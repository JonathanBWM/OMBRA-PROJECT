// ombra_client.h - Usermode Client API for OmbraDriver
// OmbraHypervisor Phase 3
//
// This provides the Ring 3 interface for communicating with OmbraDriver
// via the shared memory command ring. No device objects, no IOCTLs.

#ifndef OMBRA_CLIENT_H
#define OMBRA_CLIENT_H

#include "../shared/types.h"
#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// Error Codes (usermode-specific, complement OMBRA_CMD_STATUS)
// =============================================================================

typedef enum _OMBRA_CLIENT_STATUS {
    OMBRA_CLIENT_SUCCESS              = 0,
    OMBRA_CLIENT_NOT_INITIALIZED      = -1,
    OMBRA_CLIENT_RING_FULL            = -2,
    OMBRA_CLIENT_TIMEOUT              = -3,
    OMBRA_CLIENT_INVALID_PARAM        = -4,
    OMBRA_CLIENT_INTERNAL_ERROR       = -5,
    OMBRA_CLIENT_ALREADY_INITIALIZED  = -6,
    OMBRA_CLIENT_DRIVER_ERROR         = -7,
    OMBRA_CLIENT_BUFFER_TOO_SMALL     = -8,
} OMBRA_CLIENT_STATUS;

// =============================================================================
// Client Context
// =============================================================================

typedef struct _OMBRA_CLIENT_CTX {
    void*               SharedMemory;       // Base of shared region (R3 address)
    U64                 SharedMemorySize;   // Total size of shared region
    POMBRA_COMMAND_RING Ring;               // Pointer to command ring header
    void*               ScratchBuffer;      // Scratch buffer for large data
    U64                 ScratchSize;        // Size of scratch buffer
    volatile U64        NextSequenceId;     // Monotonic sequence counter
    bool                Initialized;        // Init state
} OMBRA_CLIENT_CTX, *POMBRA_CLIENT_CTX;

// Global client context (single instance per process)
extern OMBRA_CLIENT_CTX g_OmbraClient;

// =============================================================================
// Initialization / Shutdown
// =============================================================================

// Initialize client with mapped shared memory region
// sharedMemory: R3 pointer to shared memory (from loader)
// size: Total size of shared region
// Returns: OMBRA_CLIENT_SUCCESS or error code
OMBRA_CLIENT_STATUS OmbraClientInit(void* sharedMemory, U64 size);

// Shutdown client and cleanup
void OmbraClientShutdown(void);

// Check if client is initialized
bool OmbraClientIsReady(void);

// =============================================================================
// Low-Level Ring Operations
// =============================================================================

// Submit a command to the ring (non-blocking)
// cmd: Pre-filled command structure
// Returns: Sequence ID on success, 0 on error
U64 OmbraSubmitCommand(POMBRA_COMMAND cmd);

// Wait for a response with matching sequence ID
// sequenceId: Sequence ID from OmbraSubmitCommand
// timeoutMs: Timeout in milliseconds (0 = infinite)
// response: Output buffer for response
// Returns: OMBRA_CLIENT_SUCCESS or error code
OMBRA_CLIENT_STATUS OmbraWaitResponse(U64 sequenceId, U32 timeoutMs, POMBRA_RESPONSE response);

// Submit command and wait for response (blocking)
// cmd: Pre-filled command structure
// timeoutMs: Timeout in milliseconds
// response: Output buffer for response
// Returns: OMBRA_CLIENT_SUCCESS or error code
OMBRA_CLIENT_STATUS OmbraExecuteCommand(POMBRA_COMMAND cmd, U32 timeoutMs, POMBRA_RESPONSE response);

// =============================================================================
// Process Tracking API
// =============================================================================

// Subscribe to a process for tracking
// imageName: Process image name (e.g., "game.exe")
// outCr3: Receives CR3 of process when found (optional, can be NULL)
// Returns: OMBRA_CLIENT_SUCCESS or error code
OMBRA_CLIENT_STATUS OmbraSubscribe(const char* imageName, U64* outCr3);

// Unsubscribe from a process
// imageName: Process image name
// Returns: OMBRA_CLIENT_SUCCESS or error code
OMBRA_CLIENT_STATUS OmbraUnsubscribe(const char* imageName);

// Get information about a subscribed process
// imageName: Process image name
// outInfo: Receives process information
// Returns: OMBRA_CLIENT_SUCCESS or error code
typedef struct _OMBRA_PROCESS_INFO {
    U64     Cr3;
    U64     Peb;
    U64     Eprocess;
    U64     ImageBase;
    U32     Pid;
    bool    Dead;
    bool    DllInjected;
} OMBRA_PROCESS_INFO;

OMBRA_CLIENT_STATUS OmbraGetProcessInfo(const char* imageName, OMBRA_PROCESS_INFO* outInfo);

// =============================================================================
// Memory Operations API
// =============================================================================

// Hide memory region from target process
// cr3: Target process CR3
// address: Virtual address to hide
// size: Size in bytes
// Returns: OMBRA_CLIENT_SUCCESS or error code
OMBRA_CLIENT_STATUS OmbraHideMemory(U64 cr3, U64 address, U64 size);

// Create shadow pages (execute-only hooks)
// cr3: Target process CR3
// address: Virtual address to shadow
// size: Size in bytes (page-aligned)
// Returns: OMBRA_CLIENT_SUCCESS or error code
OMBRA_CLIENT_STATUS OmbraShadowMemory(U64 cr3, U64 address, U64 size);

// Lock a module's IAT against modification
// cr3: Target process CR3
// moduleName: Module name (e.g., "ntdll.dll")
// Returns: OMBRA_CLIENT_SUCCESS or error code
OMBRA_CLIENT_STATUS OmbraLockModule(U64 cr3, const char* moduleName);

// Unlock a module's IAT
OMBRA_CLIENT_STATUS OmbraUnlockModule(U64 cr3, const char* moduleName);

// Read physical memory
// physicalAddress: Physical address to read
// buffer: Output buffer
// size: Bytes to read
// Returns: OMBRA_CLIENT_SUCCESS or error code
OMBRA_CLIENT_STATUS OmbraReadPhysical(U64 physicalAddress, void* buffer, U64 size);

// Write physical memory
OMBRA_CLIENT_STATUS OmbraWritePhysical(U64 physicalAddress, const void* buffer, U64 size);

// Read virtual memory from target process
// cr3: Target process CR3
// virtualAddress: Virtual address to read
// buffer: Output buffer
// size: Bytes to read
// Returns: OMBRA_CLIENT_SUCCESS or error code
OMBRA_CLIENT_STATUS OmbraReadVirtual(U64 cr3, U64 virtualAddress, void* buffer, U64 size);

// Write virtual memory to target process
OMBRA_CLIENT_STATUS OmbraWriteVirtual(U64 cr3, U64 virtualAddress, const void* buffer, U64 size);

// =============================================================================
// Injection API
// =============================================================================

// Inject DLL into target process
// pid: Target process ID
// dllPath: Path to DLL file
// hidden: If true, hide injection artifacts
// Returns: OMBRA_CLIENT_SUCCESS or error code
OMBRA_CLIENT_STATUS OmbraInjectDll(U64 pid, const char* dllPath, bool hidden);

// =============================================================================
// Protection API
// =============================================================================

// Protect process from external access
// cr3: Target process CR3
// method: Protection method flags
// accessMask: Which access types to block
// Returns: OMBRA_CLIENT_SUCCESS or error code
OMBRA_CLIENT_STATUS OmbraProtectProcess(U64 cr3, U32 method, U32 accessMask);

// Remove protection from process
OMBRA_CLIENT_STATUS OmbraUnprotectProcess(U64 cr3);

// Block image from loading in target process
// cr3: Target process CR3
// imageName: Image name to block (e.g., "anticheat.dll")
// Returns: OMBRA_CLIENT_SUCCESS or error code
OMBRA_CLIENT_STATUS OmbraBlockImage(U64 cr3, const char* imageName);

// =============================================================================
// Identity Map API
// =============================================================================

// Get identity-mapped physical memory region
// size: Required size
// outBase: Receives base address of identity-mapped region
// Returns: OMBRA_CLIENT_SUCCESS or error code
OMBRA_CLIENT_STATUS OmbraGetIdentityMap(U64 size, U64* outBase);

// Release identity map
OMBRA_CLIENT_STATUS OmbraReleaseIdentityMap(U64 base);

// =============================================================================
// Spoofing API
// =============================================================================

typedef enum _OMBRA_SPOOF_TYPE {
    OMBRA_SPOOF_DISK_SERIAL    = 1,
    OMBRA_SPOOF_NIC_MAC        = 2,
    OMBRA_SPOOF_VOLUME_SERIAL  = 3,
} OMBRA_SPOOF_TYPE;

// Set hardware spoof value
// spoofType: Type of hardware to spoof
// original: Original value to match (NULL for any)
// spoofed: Value to return instead
// Returns: OMBRA_CLIENT_SUCCESS or error code
OMBRA_CLIENT_STATUS OmbraSetSpoof(OMBRA_SPOOF_TYPE spoofType, const void* original, const void* spoofed, U32 size);

// Query current spoof value
OMBRA_CLIENT_STATUS OmbraQuerySpoof(OMBRA_SPOOF_TYPE spoofType, void* original, void* spoofed, U32 size);

// =============================================================================
// ETW Control API
// =============================================================================

// Disable Threat Intelligence ETW provider
OMBRA_CLIENT_STATUS OmbraDisableTiEtw(void);

// Wipe ETW buffer
OMBRA_CLIENT_STATUS OmbraWipeEtwBuffer(U64 bufferAddress);

// Restore ETW to original state
OMBRA_CLIENT_STATUS OmbraRestoreEtw(void);

// =============================================================================
// Diagnostics API
// =============================================================================

// Ping driver (health check)
// Returns: OMBRA_CLIENT_SUCCESS if driver responds
OMBRA_CLIENT_STATUS OmbraPing(void);

// Get driver status
typedef struct _OMBRA_DRIVER_STATUS {
    U32     DriverVersion;
    U64     Uptime;
    U64     CommandsProcessed;
    U64     VmexitCount;
    U32     ActiveCpus;
    U32     HooksInstalled;
} OMBRA_DRIVER_STATUS;

OMBRA_CLIENT_STATUS OmbraGetStatus(OMBRA_DRIVER_STATUS* status);

// Get detection score
OMBRA_CLIENT_STATUS OmbraGetScore(U32* score);

// Reset detection score
OMBRA_CLIENT_STATUS OmbraResetScore(void);

// Configure score thresholds
OMBRA_CLIENT_STATUS OmbraConfigScore(U32 operation, U32 eventMask, U32 threshold);

// =============================================================================
// Scratch Buffer Helpers
// =============================================================================

// Get scratch buffer for large data transfers
// size: Required size
// outOffset: Receives offset within scratch buffer
// Returns: Pointer to scratch buffer region, or NULL on error
void* OmbraGetScratchBuffer(U64 size, U64* outOffset);

// Release scratch buffer region
void OmbraReleaseScratchBuffer(U64 offset, U64 size);

// Copy data to scratch buffer
// data: Data to copy
// size: Size of data
// outOffset: Receives offset within scratch buffer
// Returns: OMBRA_CLIENT_SUCCESS or error code
OMBRA_CLIENT_STATUS OmbraCopyToScratch(const void* data, U64 size, U64* outOffset);

// Copy data from scratch buffer
OMBRA_CLIENT_STATUS OmbraCopyFromScratch(U64 offset, void* buffer, U64 size);

#endif // OMBRA_CLIENT_H
