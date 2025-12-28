// driver_interface.h — Driver Service Management (SCM Only)
// OmbraHypervisor BYOVD Loader
//
// This file now ONLY declares Windows Service Control Manager (SCM) operations.
// All IOCTL operations are handled by supdrv.h/supdrv.c which use
// verified-correct structure definitions from supdrv_types.h.
//
// MIGRATION GUIDE:
// - Session establishment: use SupDrv_Initialize()
// - Module loading: use SupDrv_LdrOpen() and SupDrv_LdrLoad()
// - Symbol resolution: use SupDrv_GetSymbol()
// - Memory allocation: use SupDrv_PageAllocEx()
// - MSR access: use SupDrv_MsrRead() and SupDrv_MsrWrite()

#ifndef DRIVER_INTERFACE_H
#define DRIVER_INTERFACE_H

#include <Windows.h>
#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// Error Codes
// =============================================================================

typedef enum {
    DRV_SUCCESS = 0,
    DRV_ERROR_DRIVER_NOT_FOUND,
    DRV_ERROR_OPEN_DEVICE,
    DRV_ERROR_SERVICE_CREATE,
    DRV_ERROR_SERVICE_START,
    DRV_ERROR_SESSION_INIT,
    DRV_ERROR_ALLOC_FAILED,
    DRV_ERROR_EXEC_FAILED,
    DRV_ERROR_IOCTL_FAILED,
    DRV_ERROR_VMX_NOT_SUPPORTED,
    DRV_ERROR_ALREADY_LOADED,
} DRV_STATUS;

// =============================================================================
// Driver Context — SCM state only (IOCTL state in SUPDRV_CTX)
// =============================================================================

typedef struct _DRV_CONTEXT {
    HANDLE      hDevice;            // Device handle (legacy, use SUPDRV_CTX)
    HANDLE      hService;           // SCM service handle
    SC_HANDLE   hSCM;               // Service Control Manager
    bool        ServiceCreated;     // We created the service
} DRV_CONTEXT;

// =============================================================================
// Function Declarations — SCM Operations Only
// =============================================================================

// Initialization / Cleanup (deprecated - use supdrv.c for IOCTL operations)
DRV_STATUS  DrvInitialize(DRV_CONTEXT* ctx, const wchar_t* driverPath);
void        DrvCleanup(DRV_CONTEXT* ctx);

// Service Management (split for -618 bypass timing)
DRV_STATUS  DrvInstallDriver(DRV_CONTEXT* ctx, const wchar_t* driverPath);  // Install only
DRV_STATUS  DrvStartDriver(DRV_CONTEXT* ctx);                                // Start only
DRV_STATUS  DrvLoadDriver(DRV_CONTEXT* ctx, const wchar_t* driverPath);      // Install+start (legacy)
DRV_STATUS  DrvUnloadDriver(DRV_CONTEXT* ctx);

// Utility
const char* DrvStatusString(DRV_STATUS status);

// =============================================================================
// DEPRECATED - Use supdrv.h instead
// =============================================================================
// The following functions have been removed from driver_interface.c.
// Use the corresponding functions from supdrv.h:
//
// Session:
//   DrvOpenDevice()       -> SupDrv_TryOpenDevice()
//   DrvEstablishSession() -> SupDrv_Initialize()
//
// Module Loading:
//   DrvLdrOpen()          -> SupDrv_LdrOpen()
//   DrvLdrLoad()          -> SupDrv_LdrLoad()
//   DrvLdrFree()          -> (TODO: implement SupDrv_LdrFree)
//   DrvGetSymbol()        -> SupDrv_GetSymbol()
//
// Memory:
//   DrvAllocContiguous()  -> (use SupDrv_PageAllocEx for kernel memory)
//   DrvFreeContiguous()   -> SupDrv_PageFree()
//   DrvAllocPages()       -> SupDrv_PageAllocEx()
//   DrvFreePages()        -> SupDrv_PageFree()
//
// MSR:
//   DrvReadMsr()          -> SupDrv_MsrRead()
//   DrvWriteMsr()         -> SupDrv_MsrWrite()
//
// VMX (removed - not used):
//   DrvQueryVmxCaps()     -> (removed)
//   DrvGetVmxMsrs()       -> (removed)
//
// Execution (removed - use LDR_LOAD entry point):
//   DrvExecuteRing0()     -> (use SupDrv_LdrLoad with entry point)
//   DrvExecuteOnCpu()     -> (use SupDrv_LdrLoad with entry point)
//
// =============================================================================

#endif // DRIVER_INTERFACE_H
