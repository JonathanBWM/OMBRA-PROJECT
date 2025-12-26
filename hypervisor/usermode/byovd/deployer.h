/**
 * @file deployer.h
 * @brief Portable driver deployment via SCM or NtLoadDriver (C port)
 *
 * Handles extraction, deployment, and cleanup of embedded signed drivers.
 * Supports two loading methods:
 * - SCM: Service Control Manager (CreateService/StartService)
 * - NtLoadDriver: Direct registry + ntdll!NtLoadDriver
 */

#ifndef BYOVD_DEPLOYER_H
#define BYOVD_DEPLOYER_H

#include "types.h"
#include "supdrv_types.h"

//=============================================================================
// Deployment Method
//=============================================================================

typedef enum _DEPLOY_METHOD {
    DEPLOY_SCM = 0,        // Service Control Manager
    DEPLOY_NTLOAD,         // Direct NtLoadDriver
    DEPLOY_AUTO            // Try SCM first, fallback to NtLoadDriver
} DEPLOY_METHOD;

//=============================================================================
// Driver Resource Info
//=============================================================================

typedef struct _DRIVER_RESOURCE {
    const wchar_t* wszResourceName;   // Resource name (e.g., L"LD9BOXSUP_ENCRYPTED")
    const wchar_t* wszResourceType;   // Resource type (RT_RCDATA or custom)
    const wchar_t* wszDeviceName;     // Expected device name after loading
    const wchar_t* wszServiceName;    // Service name to use
    UINT32         dwEncryptionKey;   // Encryption key (0 = use header key)
} DRIVER_RESOURCE, *PDRIVER_RESOURCE;

//=============================================================================
// Deployer Context
//=============================================================================

typedef struct _DEPLOYER_CTX {
    HANDLE        hDevice;            // Device handle
    SC_HANDLE     hService;           // SCM service handle
    SC_HANDLE     hSCManager;         // SCM handle
    bool          bDeployed;          // Driver loaded successfully
    bool          bCreatedService;    // We created the service
    bool          bCreatedRegistry;   // We created registry entries
    DEPLOY_METHOD Method;             // Method used for deployment
    wchar_t       wszDriverPath[260]; // Path to extracted driver
    wchar_t       wszServiceName[64]; // Service name used
    char          szLastError[256];   // Last error message
} DEPLOYER_CTX, *PDEPLOYER_CTX;

//=============================================================================
// Known Driver Paths
//=============================================================================

extern const wchar_t* KNOWN_DRIVER_PATHS[];
extern const size_t KNOWN_DRIVER_PATHS_COUNT;

//=============================================================================
// Default Resource
//=============================================================================

extern const DRIVER_RESOURCE DEFAULT_DRIVER_RESOURCE;

//=============================================================================
// Initialization / Cleanup
//=============================================================================

/**
 * Initialize deployer context
 * @param ctx Context to initialize
 */
void Deployer_Init(PDEPLOYER_CTX ctx);

/**
 * Cleanup deployer - stop service, delete files, remove registry
 *
 * @param ctx Context
 * @param bDeleteFiles true to delete extracted driver file
 */
void Deployer_Cleanup(PDEPLOYER_CTX ctx, bool bDeleteFiles);

//=============================================================================
// Deployment
//=============================================================================

/**
 * Deploy driver from embedded resources
 *
 * @param ctx Context
 * @param method Deployment method (SCM, NtLoadDriver, or Auto)
 * @param pResource Optional custom resource info (NULL = use default)
 * @return true if driver loaded and device opened
 */
bool Deployer_Deploy(PDEPLOYER_CTX ctx, DEPLOY_METHOD method,
                     const DRIVER_RESOURCE* pResource);

/**
 * Check if deployment succeeded
 */
bool Deployer_IsDeployed(PDEPLOYER_CTX ctx);

/**
 * Get device handle for driver communication
 * Only valid after successful Deploy()
 */
HANDLE Deployer_GetDeviceHandle(PDEPLOYER_CTX ctx);

/**
 * Release device handle ownership
 * After calling this, GetDeviceHandle() returns INVALID_HANDLE_VALUE
 *
 * @param ctx Context
 * @return The handle (caller takes ownership)
 */
HANDLE Deployer_ReleaseDeviceHandle(PDEPLOYER_CTX ctx);

/**
 * Get path to extracted driver file
 */
const wchar_t* Deployer_GetDriverPath(PDEPLOYER_CTX ctx);

/**
 * Get service name used
 */
const wchar_t* Deployer_GetServiceName(PDEPLOYER_CTX ctx);

/**
 * Get last error message
 */
const char* Deployer_GetLastError(PDEPLOYER_CTX ctx);

/**
 * Get deployment method that was used
 */
DEPLOY_METHOD Deployer_GetMethod(PDEPLOYER_CTX ctx);

//=============================================================================
// Static Helpers
//=============================================================================

/**
 * Check if a SUPDrv-compatible driver is already loaded
 * @return true if device can be opened
 */
bool Deployer_IsDriverLoaded(void);

/**
 * Check if driver service exists in SCM
 * @param wszServiceName Service name to check
 * @return true if service is registered
 */
bool Deployer_IsServiceRegistered(const wchar_t* wszServiceName);

/**
 * Get service status
 *
 * @param wszServiceName Service name to query
 * @param pStatus Output service status (optional)
 * @return true if service exists
 */
bool Deployer_GetServiceStatus(const wchar_t* wszServiceName, DWORD* pStatus);

//=============================================================================
// SCM-Based Deployment
//=============================================================================

/**
 * Deploy via Service Control Manager
 *
 * @param ctx Context
 * @param pResource Resource configuration
 * @return true on success
 */
bool Deployer_DeployViaSCM(PDEPLOYER_CTX ctx, const DRIVER_RESOURCE* pResource);

//=============================================================================
// NtLoadDriver-Based Deployment
//=============================================================================

/**
 * Deploy via NtLoadDriver
 *
 * @param ctx Context
 * @param pResource Resource configuration
 * @return true on success
 */
bool Deployer_DeployViaNtLoadDriver(PDEPLOYER_CTX ctx, const DRIVER_RESOURCE* pResource);

/**
 * Create registry entries for NtLoadDriver
 *
 * @param ctx Context
 * @param wszDriverPath Full path to .sys file
 * @param wszServiceName Service name
 * @return true on success
 */
bool Deployer_CreateDriverRegistry(PDEPLOYER_CTX ctx, const wchar_t* wszDriverPath,
                                    const wchar_t* wszServiceName);

/**
 * Delete registry entries with retry logic
 *
 * @param wszServiceName Service name
 * @return true on success
 */
bool Deployer_DeleteDriverRegistry(const wchar_t* wszServiceName);

//=============================================================================
// Extraction
//=============================================================================

/**
 * Extract encrypted driver from PE resources
 *
 * @param ctx Context
 * @param pResource Resource info
 * @param wszOutputPath Output file path
 * @return true on success
 */
bool Deployer_ExtractFromResource(PDEPLOYER_CTX ctx, const DRIVER_RESOURCE* pResource,
                                   const wchar_t* wszOutputPath);

/**
 * Extract from raw encrypted blob
 *
 * @param ctx Context
 * @param pEncrypted Pointer to encrypted data
 * @param cbEncrypted Size of encrypted data
 * @param dwKey Decryption key (0 = use header)
 * @param wszOutputPath Output file path
 * @return true on success
 */
bool Deployer_ExtractFromBlob(PDEPLOYER_CTX ctx, const UINT8* pEncrypted,
                               size_t cbEncrypted, UINT32 dwKey,
                               const wchar_t* wszOutputPath);

//=============================================================================
// Device Access
//=============================================================================

/**
 * Open device handle after driver is loaded
 * Uses NtCreateFile for NT device paths.
 *
 * @param ctx Context
 * @param wszDeviceName Device name
 * @return true on success
 */
bool Deployer_OpenDevice(PDEPLOYER_CTX ctx, const wchar_t* wszDeviceName);

//=============================================================================
// Existing Installation Detection
//=============================================================================

/**
 * Try to use existing driver installation
 *
 * @param ctx Context
 * @param pResource Resource config
 * @return true if existing driver found and opened
 */
bool Deployer_TryUseExisting(PDEPLOYER_CTX ctx, const DRIVER_RESOURCE* pResource);

/**
 * Try to start existing service
 *
 * @param ctx Context
 * @param wszServiceName Service name
 * @param wszDeviceName Device name
 * @return true on success
 */
bool Deployer_TryStartExistingService(PDEPLOYER_CTX ctx, const wchar_t* wszServiceName,
                                       const wchar_t* wszDeviceName);

/**
 * Try to use driver from known path
 *
 * @param ctx Context
 * @param wszDriverPath Path to driver
 * @param wszServiceName Service name
 * @param wszDeviceName Device name
 * @return true on success
 */
bool Deployer_TryUseFromPath(PDEPLOYER_CTX ctx, const wchar_t* wszDriverPath,
                              const wchar_t* wszServiceName,
                              const wchar_t* wszDeviceName);

//=============================================================================
// Privilege Management
//=============================================================================

/**
 * Enable a privilege for the current process
 *
 * @param wszPrivilege Privilege name (e.g., SE_LOAD_DRIVER_NAME)
 * @return true on success
 */
bool Deployer_EnablePrivilege(const wchar_t* wszPrivilege);

#endif // BYOVD_DEPLOYER_H
