/**
 * @file loader_api.c
 * @brief C API implementation for OmbraHypervisor Loader
 *
 * Bridges C++ GUI to C loader core with clean interface.
 */

#include "loader_api.h"
#include "payload_loader.h"
#include "byovd/supdrv.h"
#include "byovd/nt_defs.h"
#include "obfuscate.h"

#include <Windows.h>
#include <sddl.h>
#include <intrin.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//=============================================================================
// Internal Context Structure
//=============================================================================

struct LoaderContext {
    HV_CONTEXT          hvContext;          // Hypervisor state
    LoaderLogCallback   logCallback;        // User log callback
    void*               logUserdata;        // User data for callback
    uint32_t            lastOsError;        // Last GetLastError()
    bool                sessionOpen;        // Driver session established
    bool                hypervisorLoaded;   // Hypervisor loaded
    bool                hypervisorActive;   // At least one CPU virtualized
};

//=============================================================================
// Logging Helpers
//=============================================================================

static void LogMessage(LoaderContext* ctx, LogLevel level, const char* format, ...) {
    if (!ctx || !ctx->logCallback) {
        return;
    }

    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    ctx->logCallback(level, buffer, ctx->logUserdata);
}

//=============================================================================
// Lifecycle Functions
//=============================================================================

LoaderContext* LoaderCreate(void) {
    LoaderContext* ctx = (LoaderContext*)calloc(1, sizeof(LoaderContext));
    if (!ctx) {
        return NULL;
    }

    memset(ctx, 0, sizeof(LoaderContext));
    return ctx;
}

void LoaderDestroy(LoaderContext* ctx) {
    if (!ctx) {
        return;
    }

    // Cleanup any active hypervisor
    if (ctx->hypervisorLoaded) {
        LogMessage(ctx, LOG_INFO, "Cleaning up hypervisor...");
        HvUnload(&ctx->hvContext);
    }

    // Cleanup driver session
    if (ctx->sessionOpen) {
        LogMessage(ctx, LOG_INFO, "Closing driver session...");
        DrvCleanup(&ctx->hvContext.Driver);
    }

    free(ctx);
}

void LoaderSetLogCallback(LoaderContext* ctx, LoaderLogCallback callback, void* userdata) {
    if (!ctx) {
        return;
    }

    ctx->logCallback = callback;
    ctx->logUserdata = userdata;
}

//=============================================================================
// Pre-Flight Checks
//=============================================================================

static bool IsAdmin(void) {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuth = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(&ntAuth, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                  &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }

    return isAdmin != FALSE;
}

static bool CheckVmxSupport(void) {
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    return (cpuInfo[2] & (1 << 5)) != 0;  // ECX bit 5 = VMX
}

static bool CheckVmxEnabled(void) {
    // Read IA32_FEATURE_CONTROL (MSR 0x3A)
    // Bit 0 = lock, Bit 2 = VMX outside SMX
    // NOTE: This requires ring0 access or will #GP
    // For now, we'll assume enabled and let the driver check
    return true;
}

static bool CheckHypervisorPresent(void) {
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    return (cpuInfo[2] & (1 << 31)) != 0;  // ECX bit 31 = hypervisor present
}

static bool GetWindowsVersion(char* version, size_t len, uint32_t* buildNumber) {
    OSVERSIONINFOEXW osvi;
    memset(&osvi, 0, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);

    // Use RtlGetVersion (GetVersionEx is deprecated and lies)
    typedef NTSTATUS(WINAPI* RtlGetVersionFunc)(PRTL_OSVERSIONINFOW);
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (!ntdll) {
        return false;
    }

    RtlGetVersionFunc RtlGetVersion = (RtlGetVersionFunc)GetProcAddress(ntdll, "RtlGetVersion");
    if (!RtlGetVersion) {
        return false;
    }

    if (RtlGetVersion((PRTL_OSVERSIONINFOW)&osvi) != 0) {
        return false;
    }

    *buildNumber = osvi.dwBuildNumber;
    snprintf(version, len, "Windows %lu.%lu (Build %lu)",
             osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);

    return true;
}

LoaderError LoaderCheckEnvironment(
    LoaderContext* ctx,
    const wchar_t* driverPath,
    const wchar_t* payloadPath,
    EnvironmentInfo* info
) {
    if (!ctx || !info) {
        return LOADER_ERR_INVALID_CONTEXT;
    }

    memset(info, 0, sizeof(EnvironmentInfo));

    LogMessage(ctx, LOG_INFO, "Checking environment...");

    // Check admin privileges
    info->isAdmin = IsAdmin();
    if (!info->isAdmin) {
        LogMessage(ctx, LOG_ERROR, "Not running as administrator");
        snprintf(info->errorMessage, sizeof(info->errorMessage),
                 "Administrator privileges required");
        return LOADER_ERR_NOT_ADMIN;
    }
    LogMessage(ctx, LOG_SUCCESS, "Running as administrator");

    // Check VMX support
    info->vmxSupported = CheckVmxSupport();
    if (!info->vmxSupported) {
        LogMessage(ctx, LOG_ERROR, "VMX not supported by CPU");
        snprintf(info->errorMessage, sizeof(info->errorMessage),
                 "CPU does not support Intel VT-x (VMX)");
        return LOADER_ERR_VMX_UNSUPPORTED;
    }
    LogMessage(ctx, LOG_SUCCESS, "VMX supported");

    // Check VMX enabled (best-effort - driver will verify)
    info->vmxEnabled = CheckVmxEnabled();
    if (!info->vmxEnabled) {
        LogMessage(ctx, LOG_WARNING, "VMX may be disabled in BIOS");
    }

    // Check for existing hypervisor
    info->hypervisorPresent = CheckHypervisorPresent();
    if (info->hypervisorPresent) {
        LogMessage(ctx, LOG_WARNING, "Hypervisor already present (Hyper-V?)");
        // Not a hard error - might work in nested virtualization
    }

    // Get Windows version
    if (!GetWindowsVersion(info->windowsVersion, sizeof(info->windowsVersion),
                           &info->windowsBuildNumber)) {
        LogMessage(ctx, LOG_WARNING, "Could not determine Windows version");
    } else {
        LogMessage(ctx, LOG_INFO, "Windows version: %s", info->windowsVersion);
    }

    // Check driver file
    if (driverPath) {
        DWORD attrs = GetFileAttributesW(driverPath);
        info->driverFileExists = (attrs != INVALID_FILE_ATTRIBUTES);
        if (!info->driverFileExists) {
            LogMessage(ctx, LOG_ERROR, "Driver file not found: %ls", driverPath);
            snprintf(info->errorMessage, sizeof(info->errorMessage),
                     "Driver file not found");
            return LOADER_ERR_DRIVER_NOT_FOUND;
        }
        LogMessage(ctx, LOG_SUCCESS, "Driver file found");
    }

    // Check payload file (if external)
    if (payloadPath) {
        DWORD attrs = GetFileAttributesW(payloadPath);
        info->payloadFileExists = (attrs != INVALID_FILE_ATTRIBUTES);
        if (!info->payloadFileExists) {
            LogMessage(ctx, LOG_ERROR, "Payload file not found: %ls", payloadPath);
            snprintf(info->errorMessage, sizeof(info->errorMessage),
                     "Hypervisor payload file not found");
            return LOADER_ERR_PAYLOAD_NOT_FOUND;
        }
        LogMessage(ctx, LOG_SUCCESS, "Payload file found");
    }

    info->isValid = true;
    LogMessage(ctx, LOG_SUCCESS, "Environment check passed");
    return LOADER_OK;
}

//=============================================================================
// Loading Stages
//=============================================================================

LoaderError LoaderOpenSession(LoaderContext* ctx, const wchar_t* driverPath) {
    if (!ctx) {
        return LOADER_ERR_INVALID_CONTEXT;
    }

    if (ctx->sessionOpen) {
        LogMessage(ctx, LOG_WARNING, "Session already open");
        return LOADER_OK;
    }

    LogMessage(ctx, LOG_INFO, "Opening driver session...");

    DRV_STATUS status = DrvInitialize(&ctx->hvContext.Driver, driverPath);
    if (status != DRV_SUCCESS) {
        ctx->lastOsError = GetLastError();
        LogMessage(ctx, LOG_ERROR, "Driver init failed: %s", DrvStatusString(status));

        switch (status) {
            case DRV_ERROR_DRIVER_NOT_FOUND:
                return LOADER_ERR_DRIVER_NOT_FOUND;
            case DRV_ERROR_OPEN_DEVICE:
                return LOADER_ERR_DEVICE_OPEN;
            case DRV_ERROR_SESSION_INIT:
                return LOADER_ERR_SESSION_INIT;
            default:
                return LOADER_ERR_SESSION_INIT;
        }
    }

    ctx->sessionOpen = true;
    LogMessage(ctx, LOG_SUCCESS, "Driver session established");

    // Display VMX capabilities
    uint32_t vtCaps = 0;
    if (DrvQueryVmxCaps(&ctx->hvContext.Driver, &vtCaps) == DRV_SUCCESS) {
        LogMessage(ctx, LOG_INFO, "VMX capabilities: 0x%08X", vtCaps);
    }

    return LOADER_OK;
}

LoaderError LoaderRunBigPoolTest(LoaderContext* ctx) {
    if (!ctx) {
        return LOADER_ERR_INVALID_CONTEXT;
    }

    if (!ctx->sessionOpen) {
        LogMessage(ctx, LOG_ERROR, "No session open");
        return LOADER_ERR_SESSION_INIT;
    }

    LogMessage(ctx, LOG_INFO, "Running Big Pool allocation test...");

    // Allocate 64KB test buffer
    ALLOC_INFO testAlloc = {0};
    DRV_STATUS status = DrvAllocContiguous(&ctx->hvContext.Driver, 16, &testAlloc);
    if (status != DRV_SUCCESS) {
        ctx->lastOsError = GetLastError();
        LogMessage(ctx, LOG_ERROR, "Test allocation failed: %s", DrvStatusString(status));
        return LOADER_ERR_ALLOC_VMXON;
    }

    LogMessage(ctx, LOG_INFO, "Test allocation: R3=%p R0=%p Phys=0x%llX",
               testAlloc.R3, testAlloc.R0, (unsigned long long)testAlloc.Physical);

    // Write test pattern
    if (testAlloc.R3) {
        uint32_t* p = (uint32_t*)testAlloc.R3;
        for (int i = 0; i < 16 * 1024; i++) {
            p[i] = 0xDEADBEEF + i;
        }
        LogMessage(ctx, LOG_SUCCESS, "Test pattern written");
    }

    // Free
    DrvFreeContiguous(&ctx->hvContext.Driver, &testAlloc);
    LogMessage(ctx, LOG_SUCCESS, "Big Pool test passed");

    return LOADER_OK;
}

LoaderError LoaderLoadHypervisor(
    LoaderContext* ctx,
    const void* payload,
    uint32_t payloadSize
) {
    if (!ctx) {
        return LOADER_ERR_INVALID_CONTEXT;
    }

    if (!ctx->sessionOpen) {
        LogMessage(ctx, LOG_ERROR, "No session open");
        return LOADER_ERR_SESSION_INIT;
    }

    if (ctx->hypervisorLoaded) {
        LogMessage(ctx, LOG_WARNING, "Hypervisor already loaded");
        return LOADER_OK;
    }

    if (!payload || payloadSize == 0) {
        LogMessage(ctx, LOG_ERROR, "Invalid payload");
        return LOADER_ERR_PAYLOAD_NOT_FOUND;
    }

    LogMessage(ctx, LOG_INFO, "Loading hypervisor (%u bytes)...", payloadSize);

    // Note: HvLoad calls DrvInitialize internally, but we've already called it
    // We need to pass the driver path again, but it will skip re-init
    // For now, we'll call the internal functions directly

    // Allocate per-CPU structures
    ctx->hvContext.NumCpus = 0;
    if (!HvAllocatePerCpu(&ctx->hvContext)) {
        LogMessage(ctx, LOG_ERROR, "Failed to allocate per-CPU structures");
        return LOADER_ERR_ALLOC_VMXON;
    }
    LogMessage(ctx, LOG_SUCCESS, "Allocated VMX structures for %u CPUs",
               ctx->hvContext.NumCpus);

    // Allocate EPT tables
    if (!HvAllocateEpt(&ctx->hvContext)) {
        LogMessage(ctx, LOG_ERROR, "Failed to allocate EPT tables");
        HvFreePerCpu(&ctx->hvContext);
        return LOADER_ERR_ALLOC_EPT;
    }
    LogMessage(ctx, LOG_SUCCESS, "Allocated EPT tables");

    // Allocate debug buffer
    if (!HvAllocateDebugBuffer(&ctx->hvContext)) {
        LogMessage(ctx, LOG_ERROR, "Failed to allocate debug buffer");
        DrvFreeContiguous(&ctx->hvContext.Driver, &ctx->hvContext.EptTables);
        HvFreePerCpu(&ctx->hvContext);
        return LOADER_ERR_ALLOC_CODE;
    }
    LogMessage(ctx, LOG_SUCCESS, "Allocated debug buffer");

    // Copy payload
    if (!HvCopyPayload(&ctx->hvContext, payload, payloadSize)) {
        LogMessage(ctx, LOG_ERROR, "Failed to copy payload");
        DrvFreeContiguous(&ctx->hvContext.Driver, &ctx->hvContext.DebugBuffer);
        DrvFreeContiguous(&ctx->hvContext.Driver, &ctx->hvContext.EptTables);
        HvFreePerCpu(&ctx->hvContext);
        return LOADER_ERR_PAYLOAD_COPY;
    }
    LogMessage(ctx, LOG_SUCCESS, "Payload copied to kernel memory");

    ctx->hvContext.Loaded = true;
    ctx->hypervisorLoaded = true;

    // Launch on all CPUs
    LogMessage(ctx, LOG_INFO, "Launching on all CPUs...");
    if (!HvLaunchAll(&ctx->hvContext)) {
        LogMessage(ctx, LOG_ERROR, "Failed to launch hypervisor");
        return LOADER_ERR_VMLAUNCH_FAILED;
    }

    // Count virtualized CPUs
    uint32_t virtualizedCount = 0;
    for (uint32_t i = 0; i < ctx->hvContext.NumCpus; i++) {
        if (ctx->hvContext.Cpus[i].Virtualized) {
            virtualizedCount++;
        }
    }

    ctx->hypervisorActive = (virtualizedCount > 0);
    LogMessage(ctx, LOG_SUCCESS, "Hypervisor active on %u/%u CPUs",
               virtualizedCount, ctx->hvContext.NumCpus);

    return LOADER_OK;
}

LoaderError LoaderMapDriver(
    LoaderContext* ctx,
    const void* driverImage,
    uint32_t driverSize
) {
    if (!ctx) {
        return LOADER_ERR_INVALID_CONTEXT;
    }

    if (!ctx->hypervisorLoaded) {
        LogMessage(ctx, LOG_ERROR, "Hypervisor not loaded");
        return LOADER_ERR_NOT_LOADED;
    }

    LogMessage(ctx, LOG_WARNING, "Driver mapping not yet implemented (Phase 2)");
    // TODO: Implement manual PE mapping via hypervisor
    return LOADER_ERR_DRIVER_MAP;
}

LoaderError LoaderInitComms(LoaderContext* ctx) {
    if (!ctx) {
        return LOADER_ERR_INVALID_CONTEXT;
    }

    LogMessage(ctx, LOG_WARNING, "Communication channel not yet implemented (Phase 3)");
    // TODO: Set up shared memory / VMCALL interface
    return LOADER_ERR_COMMS_INIT;
}

LoaderError LoaderLoadAll(
    LoaderContext* ctx,
    const wchar_t* driverPath,
    const void* payload,
    uint32_t payloadSize
) {
    LoaderError err;

    err = LoaderOpenSession(ctx, driverPath);
    if (err != LOADER_OK) {
        return err;
    }

    err = LoaderLoadHypervisor(ctx, payload, payloadSize);
    if (err != LOADER_OK) {
        return err;
    }

    return LOADER_OK;
}

//=============================================================================
// Runtime Functions
//=============================================================================

LoaderError LoaderGetStatus(LoaderContext* ctx, LoaderStatus* status) {
    if (!ctx || !status) {
        return LOADER_ERR_INVALID_CONTEXT;
    }

    memset(status, 0, sizeof(LoaderStatus));

    status->hypervisorLoaded = ctx->hypervisorLoaded;
    status->hypervisorActive = ctx->hypervisorActive;
    status->numCpus = ctx->hvContext.NumCpus;

    // Count virtualized CPUs
    uint32_t virtualizedCount = 0;
    for (uint32_t i = 0; i < ctx->hvContext.NumCpus; i++) {
        if (ctx->hvContext.Cpus[i].Virtualized) {
            virtualizedCount++;
        }
    }
    status->numVirtualized = virtualizedCount;

    status->driverMapped = false;   // Phase 2
    status->commsReady = false;     // Phase 3

    status->debugBuffer = HvGetDebugBuffer(&ctx->hvContext);
    status->debugBufferSize = (uint32_t)HvGetDebugBufferSize(&ctx->hvContext);

    // Progress reporting fields
    if (ctx->hypervisorActive) {
        status->currentPhase = "Active";
        status->progress = 100.0f;
    } else if (ctx->hypervisorLoaded) {
        status->currentPhase = "Starting CPUs";
        status->progress = 75.0f;
    } else if (ctx->sessionOpen) {
        status->currentPhase = "Loading Hypervisor";
        status->progress = 50.0f;
    } else {
        status->currentPhase = "Idle";
        status->progress = 0.0f;
    }
    status->lastError[0] = '\0';  // No error

    return LOADER_OK;
}

LoaderError LoaderPing(LoaderContext* ctx) {
    if (!ctx) {
        return LOADER_ERR_INVALID_CONTEXT;
    }

    if (!ctx->hypervisorActive) {
        LogMessage(ctx, LOG_ERROR, "Hypervisor not active");
        return LOADER_ERR_NOT_RUNNING;
    }

    // TODO: Send VMCALL_PING
    // For now, just check the Running flag
    if (HvIsRunning(&ctx->hvContext)) {
        LogMessage(ctx, LOG_SUCCESS, "Hypervisor is responsive");
        return LOADER_OK;
    } else {
        LogMessage(ctx, LOG_ERROR, "Hypervisor not responding");
        return LOADER_ERR_PING_FAILED;
    }
}

LoaderError LoaderShutdown(LoaderContext* ctx) {
    if (!ctx) {
        return LOADER_ERR_INVALID_CONTEXT;
    }

    if (!ctx->hypervisorLoaded) {
        LogMessage(ctx, LOG_WARNING, "Hypervisor not loaded");
        return LOADER_OK;
    }

    LogMessage(ctx, LOG_INFO, "Shutting down hypervisor...");

    if (!HvUnload(&ctx->hvContext)) {
        LogMessage(ctx, LOG_ERROR, "Shutdown failed");
        return LOADER_ERR_UNKNOWN;
    }

    ctx->hypervisorLoaded = false;
    ctx->hypervisorActive = false;

    LogMessage(ctx, LOG_SUCCESS, "Hypervisor shutdown complete");
    return LOADER_OK;
}

//=============================================================================
// Error Handling
//=============================================================================

const char* LoaderErrorString(LoaderError error) {
    switch (error) {
        case LOADER_OK:                     return "Success";
        case LOADER_ERR_NOT_ADMIN:          return "Administrator privileges required";
        case LOADER_ERR_VMX_UNSUPPORTED:    return "VMX not supported by CPU";
        case LOADER_ERR_VMX_DISABLED:       return "VMX disabled in BIOS/firmware";
        case LOADER_ERR_HYPERVISOR_PRESENT: return "Hypervisor already present";
        case LOADER_ERR_WINDOWS_VERSION:    return "Unsupported Windows version";
        case LOADER_ERR_DRIVER_NOT_FOUND:   return "Driver file not found";
        case LOADER_ERR_PAYLOAD_NOT_FOUND:  return "Payload file not found";
        case LOADER_ERR_DEVICE_OPEN:        return "Failed to open device";
        case LOADER_ERR_COOKIE_HANDSHAKE:   return "Cookie handshake failed";
        case LOADER_ERR_SESSION_INIT:       return "Session initialization failed";
        case LOADER_ERR_ALLOC_VMXON:        return "Failed to allocate VMXON region";
        case LOADER_ERR_ALLOC_VMCS:         return "Failed to allocate VMCS region";
        case LOADER_ERR_ALLOC_STACK:        return "Failed to allocate host stack";
        case LOADER_ERR_ALLOC_EPT:          return "Failed to allocate EPT tables";
        case LOADER_ERR_ALLOC_CODE:         return "Failed to allocate code region";
        case LOADER_ERR_PAYLOAD_COPY:       return "Failed to copy payload";
        case LOADER_ERR_VMLAUNCH_FAILED:    return "VMLAUNCH failed";
        case LOADER_ERR_CPU_INIT_FAILED:    return "CPU initialization failed";
        case LOADER_ERR_DRIVER_MAP:         return "Driver mapping not implemented";
        case LOADER_ERR_COMMS_INIT:         return "Communication init not implemented";
        case LOADER_ERR_NOT_LOADED:         return "Hypervisor not loaded";
        case LOADER_ERR_NOT_RUNNING:        return "Hypervisor not running";
        case LOADER_ERR_PING_FAILED:        return "Ping failed";
        case LOADER_ERR_INVALID_CONTEXT:    return "Invalid context";
        case LOADER_ERR_UNKNOWN:            return "Unknown error";
        default:                            return "Unknown error code";
    }
}

uint32_t LoaderGetLastOsError(LoaderContext* ctx) {
    if (!ctx) {
        return 0;
    }
    return ctx->lastOsError;
}
