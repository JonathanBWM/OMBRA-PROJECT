// driver_interface.c — Driver Service Management (SCM Only)
// OmbraHypervisor BYOVD Loader
//
// This file now ONLY handles Windows Service Control Manager (SCM) operations.
// All IOCTL operations (session establishment, memory allocation, module loading)
// are handled by supdrv.c which uses verified-correct structure definitions.
//
// DEPRECATED FUNCTIONS REMOVED:
// - DrvEstablishSession() - use SupDrv_Initialize()
// - DrvLdrOpen() - use SupDrv_LdrOpen()
// - DrvLdrLoad() - use SupDrv_LdrLoad()
// - DrvLdrFree() - use SupDrv_LdrFree() (TODO: implement if needed)
// - DrvGetSymbol() - use SupDrv_GetSymbol()
// - DrvReadMsr() - use SupDrv_MsrRead()
// - DrvWriteMsr() - use SupDrv_MsrWrite()
// - DrvQueryVmxCaps() - removed (not needed)
// - DrvGetVmxMsrs() - removed (not needed)
// - DrvCallVmmr0() - removed (not used)
// - All memory allocation functions - use SupDrv_PageAllocEx()

#include "driver_interface.h"
#include <stdio.h>

// Service name for Ld9BoxSup driver
#define LD9BOXSUP_SERVICE_NAME  L"Ld9BoxSup"

// =============================================================================
// Service Management (SCM Operations Only)
// =============================================================================

// Install driver service WITHOUT starting it
// This allows -618 bypass to be applied before DriverEntry runs
DRV_STATUS DrvInstallDriver(DRV_CONTEXT* ctx, const wchar_t* driverPath) {
    // Open Service Control Manager
    ctx->hSCM = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!ctx->hSCM) {
        // Try with less privileges - maybe service already exists
        ctx->hSCM = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);
        if (!ctx->hSCM) {
            return DRV_ERROR_SERVICE_CREATE;
        }
    }

    // Try to open existing service first
    ctx->hService = OpenServiceW(ctx->hSCM, LD9BOXSUP_SERVICE_NAME, SERVICE_ALL_ACCESS);

    if (!ctx->hService) {
        // Service doesn't exist, create it
        ctx->hService = CreateServiceW(
            ctx->hSCM,
            LD9BOXSUP_SERVICE_NAME,
            LD9BOXSUP_SERVICE_NAME,
            SERVICE_ALL_ACCESS,
            SERVICE_KERNEL_DRIVER,
            SERVICE_DEMAND_START,
            SERVICE_ERROR_NORMAL,
            driverPath,
            NULL, NULL, NULL, NULL, NULL
        );

        if (!ctx->hService) {
            DWORD err = GetLastError();
            if (err == ERROR_SERVICE_EXISTS) {
                // Race condition - try opening again
                ctx->hService = OpenServiceW(ctx->hSCM, LD9BOXSUP_SERVICE_NAME, SERVICE_ALL_ACCESS);
                if (!ctx->hService) {
                    CloseServiceHandle(ctx->hSCM);
                    ctx->hSCM = NULL;
                    return DRV_ERROR_SERVICE_CREATE;
                }
            } else {
                CloseServiceHandle(ctx->hSCM);
                ctx->hSCM = NULL;
                return DRV_ERROR_SERVICE_CREATE;
            }
        } else {
            ctx->ServiceCreated = true;
        }
    }

    // NOTE: Service is installed but NOT started
    // Caller must apply -618 bypass, then call DrvStartDriver()
    return DRV_SUCCESS;
}

// Start an already-installed driver service
// Call this AFTER applying -618 bypass
DRV_STATUS DrvStartDriver(DRV_CONTEXT* ctx) {
    if (!ctx->hService) {
        return DRV_ERROR_SERVICE_CREATE;
    }

    if (!StartServiceW(ctx->hService, 0, NULL)) {
        DWORD err = GetLastError();
        if (err != ERROR_SERVICE_ALREADY_RUNNING) {
            if (ctx->ServiceCreated) {
                DeleteService(ctx->hService);
            }
            CloseServiceHandle(ctx->hService);
            CloseServiceHandle(ctx->hSCM);
            ctx->hService = NULL;
            ctx->hSCM = NULL;
            return DRV_ERROR_SERVICE_START;
        }
    }

    return DRV_SUCCESS;
}

// Legacy function - install AND start (for backward compat)
DRV_STATUS DrvLoadDriver(DRV_CONTEXT* ctx, const wchar_t* driverPath) {
    DRV_STATUS status = DrvInstallDriver(ctx, driverPath);
    if (status != DRV_SUCCESS) {
        return status;
    }
    return DrvStartDriver(ctx);
}

DRV_STATUS DrvUnloadDriver(DRV_CONTEXT* ctx) {
    if (ctx->hService) {
        // Stop the service
        SERVICE_STATUS status;
        ControlService(ctx->hService, SERVICE_CONTROL_STOP, &status);

        // Delete if we created it
        if (ctx->ServiceCreated) {
            DeleteService(ctx->hService);
        }

        CloseServiceHandle(ctx->hService);
        ctx->hService = NULL;
    }

    if (ctx->hSCM) {
        CloseServiceHandle(ctx->hSCM);
        ctx->hSCM = NULL;
    }

    ctx->ServiceCreated = false;
    return DRV_SUCCESS;
}

// =============================================================================
// High-Level Initialization (DEPRECATED - use supdrv.c directly)
// =============================================================================

DRV_STATUS DrvInitialize(DRV_CONTEXT* ctx, const wchar_t* driverPath) {
    // This function is deprecated. Use SupDrv_Initialize() instead.
    // Kept for backward compatibility only.
    printf("[!] WARNING: DrvInitialize is deprecated, use SupDrv_Initialize()\n");

    memset(ctx, 0, sizeof(*ctx));

    // Just load the driver service
    return DrvLoadDriver(ctx, driverPath);
}

void DrvCleanup(DRV_CONTEXT* ctx) {
    if (ctx->hDevice) {
        CloseHandle(ctx->hDevice);
        ctx->hDevice = NULL;
    }

    DrvUnloadDriver(ctx);

    memset(ctx, 0, sizeof(*ctx));
}

// =============================================================================
// Utility
// =============================================================================

const char* DrvStatusString(DRV_STATUS status) {
    switch (status) {
        case DRV_SUCCESS:               return "Success";
        case DRV_ERROR_DRIVER_NOT_FOUND: return "Driver file not found";
        case DRV_ERROR_OPEN_DEVICE:     return "Failed to open device";
        case DRV_ERROR_SERVICE_CREATE:  return "Failed to create service";
        case DRV_ERROR_SERVICE_START:   return "Failed to start service";
        case DRV_ERROR_SESSION_INIT:    return "Failed to establish session";
        case DRV_ERROR_ALLOC_FAILED:    return "Memory allocation failed";
        case DRV_ERROR_EXEC_FAILED:     return "Ring-0 execution failed";
        case DRV_ERROR_IOCTL_FAILED:    return "IOCTL failed";
        case DRV_ERROR_VMX_NOT_SUPPORTED: return "VMX not supported";
        case DRV_ERROR_ALREADY_LOADED:  return "Hypervisor already loaded";
        default:                        return "Unknown error";
    }
}
