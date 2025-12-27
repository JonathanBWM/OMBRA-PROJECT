/**
 * @file loader_api.h
 * @brief C API for OmbraHypervisor Loader (C++ GUI Bridge)
 *
 * This header provides a C interface to the hypervisor loader,
 * allowing C++ GUI code to interact with the C loader core
 * without ABI compatibility issues.
 *
 * Architecture:
 * - Opaque LoaderContext handle (no struct exposure)
 * - Callback-based logging (GUI integrates into UI)
 * - Explicit error codes with human-readable strings
 * - Phased loading (pre-flight, session, hypervisor, driver, comms)
 * - Status queries (hypervisor active, CPUs virtualized, etc)
 */

#ifndef LOADER_API_H
#define LOADER_API_H

#include <stdint.h>
#include <stdbool.h>
#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// Error Codes
//=============================================================================

typedef enum {
    LOADER_OK = 0,

    // Environment validation errors
    LOADER_ERR_NOT_ADMIN,
    LOADER_ERR_VMX_UNSUPPORTED,
    LOADER_ERR_VMX_DISABLED,
    LOADER_ERR_HYPERVISOR_PRESENT,
    LOADER_ERR_WINDOWS_VERSION,
    LOADER_ERR_DRIVER_NOT_FOUND,
    LOADER_ERR_PAYLOAD_NOT_FOUND,

    // Session errors
    LOADER_ERR_DEVICE_OPEN,
    LOADER_ERR_COOKIE_HANDSHAKE,
    LOADER_ERR_SESSION_INIT,

    // Allocation errors
    LOADER_ERR_ALLOC_VMXON,
    LOADER_ERR_ALLOC_VMCS,
    LOADER_ERR_ALLOC_STACK,
    LOADER_ERR_ALLOC_EPT,
    LOADER_ERR_ALLOC_CODE,

    // Launch errors
    LOADER_ERR_PAYLOAD_COPY,
    LOADER_ERR_VMLAUNCH_FAILED,
    LOADER_ERR_CPU_INIT_FAILED,

    // Phase 2/3 placeholders
    LOADER_ERR_DRIVER_MAP,
    LOADER_ERR_COMMS_INIT,

    // Runtime errors
    LOADER_ERR_NOT_LOADED,
    LOADER_ERR_NOT_RUNNING,
    LOADER_ERR_PING_FAILED,

    // Generic
    LOADER_ERR_INVALID_CONTEXT,
    LOADER_ERR_UNKNOWN,
} LoaderError;

//=============================================================================
// Log Levels
//=============================================================================

typedef enum {
    LOG_INFO,       // [*] Informational
    LOG_SUCCESS,    // [+] Success
    LOG_WARNING,    // [!] Warning
    LOG_ERROR,      // [-] Error
} LogLevel;

//=============================================================================
// Logging Callback
//=============================================================================

/**
 * Log callback function.
 *
 * @param level Log level
 * @param message Log message (null-terminated UTF-8)
 * @param userdata User data pointer passed to LoaderSetLogCallback
 */
typedef void (*LoaderLogCallback)(LogLevel level, const char* message, void* userdata);

//=============================================================================
// Status Structures
//=============================================================================

/**
 * Loader runtime status
 */
typedef struct {
    bool        hypervisorLoaded;   // Hypervisor code loaded into kernel
    bool        hypervisorActive;   // At least one CPU virtualized
    uint32_t    numCpus;            // Total CPU count
    uint32_t    numVirtualized;     // CPUs successfully virtualized

    bool        driverMapped;       // Phase 2: Driver mapped into kernel
    bool        commsReady;         // Phase 3: Communication channel active

    void*       debugBuffer;        // Ring buffer for hypervisor logs (R3 mapping)
    uint32_t    debugBufferSize;    // Size in bytes

    // Progress reporting for GUI
    const char* currentPhase;       // Current loading phase name
    float       progress;           // Loading progress 0-100
    char        lastError[256];     // Last error message if any
} LoaderStatus;

/**
 * Environment validation results
 */
typedef struct {
    bool    isValid;            // Overall validity
    bool    isAdmin;            // Running with admin privileges
    bool    vmxSupported;       // VMX supported by CPU
    bool    vmxEnabled;         // VMX enabled in BIOS/firmware
    bool    hypervisorPresent;  // Hyper-V or other hypervisor running

    uint32_t windowsBuildNumber;
    char     windowsVersion[64];

    bool    driverFileExists;
    bool    payloadFileExists;

    char    errorMessage[256];  // Human-readable error if !isValid
} EnvironmentInfo;

//=============================================================================
// Opaque Context Handle
//=============================================================================

/**
 * Opaque loader context.
 * Allocated by LoaderCreate, freed by LoaderDestroy.
 */
typedef struct LoaderContext LoaderContext;

//=============================================================================
// Lifecycle Functions
//=============================================================================

/**
 * Create loader context.
 *
 * @return Opaque context handle, or NULL on failure
 */
LoaderContext* LoaderCreate(void);

/**
 * Destroy loader context.
 * Automatically cleans up any active sessions/hypervisors.
 *
 * @param ctx Context to destroy
 */
void LoaderDestroy(LoaderContext* ctx);

/**
 * Set log callback.
 * All loader operations will invoke this callback for logging.
 *
 * @param ctx Context
 * @param callback Callback function (or NULL to disable)
 * @param userdata User data pointer passed to callback
 */
void LoaderSetLogCallback(LoaderContext* ctx, LoaderLogCallback callback, void* userdata);

//=============================================================================
// Pre-Flight Checks
//=============================================================================

/**
 * Check environment compatibility.
 * Validates:
 * - Admin privileges
 * - VMX support and enablement
 * - No conflicting hypervisor
 * - Windows version compatibility
 * - Required files exist
 *
 * @param ctx Context
 * @param driverPath Path to Ld9BoxSup.sys
 * @param payloadPath Path to hypervisor binary (or NULL if embedded)
 * @param[out] info Environment information (detailed results)
 * @return LOADER_OK if environment is valid
 */
LoaderError LoaderCheckEnvironment(
    LoaderContext* ctx,
    const wchar_t* driverPath,
    const wchar_t* payloadPath,
    EnvironmentInfo* info
);

//=============================================================================
// Loading Stages
//=============================================================================

/**
 * Stage 1: Open driver session.
 * Loads Ld9BoxSup.sys, opens device, performs cookie handshake.
 *
 * @param ctx Context
 * @param driverPath Path to Ld9BoxSup.sys
 * @return LOADER_OK on success
 */
LoaderError LoaderOpenSession(LoaderContext* ctx, const wchar_t* driverPath);

/**
 * Stage 1a: Run Big Pool test (optional pre-flight).
 * Allocates/frees test memory to verify driver functionality.
 *
 * @param ctx Context (must have session open)
 * @return LOADER_OK if test passed
 */
LoaderError LoaderRunBigPoolTest(LoaderContext* ctx);

/**
 * Stage 1b: Load hypervisor.
 * Allocates VMX structures, copies payload, launches on all CPUs.
 *
 * @param ctx Context (must have session open)
 * @param payload Hypervisor binary (or NULL if embedded)
 * @param payloadSize Size of payload in bytes
 * @return LOADER_OK on success
 */
LoaderError LoaderLoadHypervisor(
    LoaderContext* ctx,
    const void* payload,
    uint32_t payloadSize
);

/**
 * Stage 2: Map driver (TODO).
 * Maps OmbraDriver.sys into kernel via hypervisor.
 * (Not yet implemented - returns LOADER_ERR_DRIVER_MAP)
 *
 * @param ctx Context (must have hypervisor loaded)
 * @param driverImage Driver PE image
 * @param driverSize Size in bytes
 * @return LOADER_OK on success
 */
LoaderError LoaderMapDriver(
    LoaderContext* ctx,
    const void* driverImage,
    uint32_t driverSize
);

/**
 * Stage 3: Initialize communication channel (TODO).
 * Establishes shared memory / VMCALL interface.
 * (Not yet implemented - returns LOADER_ERR_COMMS_INIT)
 *
 * @param ctx Context (must have driver mapped)
 * @return LOADER_OK on success
 */
LoaderError LoaderInitComms(LoaderContext* ctx);

/**
 * Load everything in one shot.
 * Convenience function that calls:
 * 1. LoaderOpenSession
 * 2. LoaderLoadHypervisor
 *
 * @param ctx Context
 * @param driverPath Path to Ld9BoxSup.sys
 * @param payload Hypervisor binary
 * @param payloadSize Size in bytes
 * @return LOADER_OK if all stages succeeded
 */
LoaderError LoaderLoadAll(
    LoaderContext* ctx,
    const wchar_t* driverPath,
    const void* payload,
    uint32_t payloadSize
);

//=============================================================================
// Runtime Functions
//=============================================================================

/**
 * Get current loader status.
 *
 * @param ctx Context
 * @param[out] status Current status
 * @return LOADER_OK on success
 */
LoaderError LoaderGetStatus(LoaderContext* ctx, LoaderStatus* status);

/**
 * Ping hypervisor.
 * Sends VMCALL to verify hypervisor is responsive.
 *
 * @param ctx Context
 * @return LOADER_OK if hypervisor responded
 */
LoaderError LoaderPing(LoaderContext* ctx);

/**
 * Shutdown hypervisor.
 * Sends VMXOFF to all CPUs and cleans up resources.
 *
 * @param ctx Context
 * @return LOADER_OK on success
 */
LoaderError LoaderShutdown(LoaderContext* ctx);

//=============================================================================
// Error Handling
//=============================================================================

/**
 * Get human-readable error string.
 *
 * @param error Error code
 * @return Error description (static string)
 */
const char* LoaderErrorString(LoaderError error);

/**
 * Get last OS error code.
 * Returns GetLastError() from most recent Win32 API call.
 *
 * @param ctx Context
 * @return Last Win32 error code
 */
uint32_t LoaderGetLastOsError(LoaderContext* ctx);

#ifdef __cplusplus
}
#endif

#endif // LOADER_API_H
