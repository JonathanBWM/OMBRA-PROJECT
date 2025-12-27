// driver_interface.h — Ld9BoxSup.sys Driver Interface
// OmbraHypervisor BYOVD Loader

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
// Allocation Info — Returned from memory allocation
// =============================================================================

typedef struct _ALLOC_INFO {
    void*       R3;         // Ring-3 (usermode) mapping
    void*       R0;         // Ring-0 (kernel) mapping
    uint64_t    Physical;   // Physical address (for contiguous allocs)
    uint32_t    Pages;      // Number of pages
    bool        Contiguous; // Was contiguous allocation
} ALLOC_INFO;

// =============================================================================
// VMX MSR Info — Retrieved from driver
// =============================================================================

typedef struct _VMX_MSRS {
    uint64_t    FeatCtrl;
    uint64_t    Basic;
    uint64_t    PinCtls;
    uint64_t    ProcCtls;
    uint64_t    ProcCtls2;
    uint64_t    ExitCtls;
    uint64_t    EntryCtls;
    uint64_t    TruePin;
    uint64_t    TrueProc;
    uint64_t    TrueExit;
    uint64_t    TrueEntry;
    uint64_t    Misc;
    uint64_t    Cr0Fixed0;
    uint64_t    Cr0Fixed1;
    uint64_t    Cr4Fixed0;
    uint64_t    Cr4Fixed1;
    uint64_t    VmcsEnum;
    uint64_t    EptVpidCap;
} VMX_MSRS;

// =============================================================================
// Driver Context — Holds session state
// =============================================================================

typedef struct _DRV_CONTEXT {
    HANDLE      hDevice;            // Device handle
    HANDLE      hService;           // SCM service handle
    SC_HANDLE   hSCM;               // Service Control Manager
    uint32_t    Cookie;             // Session cookie
    uint32_t    SessionCookie;      // Session-specific cookie
    void*       pSession;           // Driver session handle
    bool        Initialized;        // Session established
    bool        ServiceCreated;     // We created the service
    VMX_MSRS    VmxMsrs;            // Cached VMX capabilities
} DRV_CONTEXT;

// =============================================================================
// Function Declarations
// =============================================================================

// Initialization / Cleanup
DRV_STATUS  DrvInitialize(DRV_CONTEXT* ctx, const wchar_t* driverPath);
void        DrvCleanup(DRV_CONTEXT* ctx);

// Service Management
DRV_STATUS  DrvLoadDriver(DRV_CONTEXT* ctx, const wchar_t* driverPath);
DRV_STATUS  DrvUnloadDriver(DRV_CONTEXT* ctx);

// Session Management
DRV_STATUS  DrvOpenDevice(DRV_CONTEXT* ctx);
DRV_STATUS  DrvEstablishSession(DRV_CONTEXT* ctx);

// VMX Capabilities
DRV_STATUS  DrvQueryVmxCaps(DRV_CONTEXT* ctx, uint32_t* caps);
DRV_STATUS  DrvGetVmxMsrs(DRV_CONTEXT* ctx, VMX_MSRS* msrs);

// Memory Allocation
DRV_STATUS  DrvAllocContiguous(DRV_CONTEXT* ctx, uint32_t pages, ALLOC_INFO* info);
DRV_STATUS  DrvFreeContiguous(DRV_CONTEXT* ctx, ALLOC_INFO* info);
DRV_STATUS  DrvAllocPages(DRV_CONTEXT* ctx, uint32_t pages, bool kernelMap, ALLOC_INFO* info);
DRV_STATUS  DrvFreePages(DRV_CONTEXT* ctx, ALLOC_INFO* info);

// Code Execution
DRV_STATUS  DrvExecuteRing0(DRV_CONTEXT* ctx, void* func, uint64_t arg, int32_t* result);
DRV_STATUS  DrvExecuteOnCpu(DRV_CONTEXT* ctx, uint32_t cpuId, void* func, uint64_t arg, int32_t* result);

// Symbol Resolution
DRV_STATUS  DrvGetSymbol(DRV_CONTEXT* ctx, const char* name, void** addr);

// MSR Access
DRV_STATUS  DrvReadMsr(DRV_CONTEXT* ctx, uint32_t msr, uint32_t cpu, uint64_t* value);
DRV_STATUS  DrvWriteMsr(DRV_CONTEXT* ctx, uint32_t msr, uint32_t cpu, uint64_t value);

// Module Loading (LDR_OPEN / LDR_LOAD)
DRV_STATUS  DrvLdrOpen(DRV_CONTEXT* ctx, uint32_t imageSize, void** ppImageBase);
DRV_STATUS  DrvLdrLoad(DRV_CONTEXT* ctx, void* imageBase, const void* imageData,
                       uint32_t imageSize, void* entryPoint);
DRV_STATUS  DrvLdrFree(DRV_CONTEXT* ctx, void* imageBase);

// Utility
const char* DrvStatusString(DRV_STATUS status);

#endif // DRIVER_INTERFACE_H
