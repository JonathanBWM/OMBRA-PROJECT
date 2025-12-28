// driver_interface.c — Ld9BoxSup.sys Driver Interface Implementation
// OmbraHypervisor BYOVD Loader

#include "driver_interface.h"
#include "ld9boxsup.h"
#include <stdio.h>

// =============================================================================
// Internal Helpers
// =============================================================================

static bool DoIoctl(HANDLE hDevice, uint32_t code, void* in, uint32_t inSize,
                    void* out, uint32_t outSize, uint32_t* bytesReturned) {
    DWORD bytes = 0;
    BOOL result = DeviceIoControl(
        hDevice,
        code,
        in, inSize,
        out, outSize,
        &bytes,
        NULL
    );
    if (bytesReturned) *bytesReturned = bytes;
    return result != FALSE;
}

static void FillHeader(SUPREQHDR* hdr, DRV_CONTEXT* ctx, uint32_t cbIn, uint32_t cbOut) {
    hdr->u32Cookie = ctx->Cookie;
    hdr->u32SessionCookie = ctx->SessionCookie;
    hdr->cbIn = cbIn;
    hdr->cbOut = cbOut;
    hdr->fFlags = 0;
    hdr->rc = 0;
}

// =============================================================================
// Service Management
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
// Session Management
// =============================================================================

DRV_STATUS DrvOpenDevice(DRV_CONTEXT* ctx) {
    // Try multiple device paths (driver may create different device names)
    static const wchar_t* devicePaths[] = {
        L"\\\\.\\Ld9BoxDrv",   // Primary LDPlayer device
        L"\\\\.\\Ld9BoxDrvU",  // LDPlayer user device
        L"\\\\.\\VBoxDrv",     // VirtualBox fallback
    };
    static const int devicePathCount = sizeof(devicePaths) / sizeof(devicePaths[0]);

    for (int i = 0; i < devicePathCount; i++) {
        ctx->hDevice = CreateFileW(
            devicePaths[i],
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (ctx->hDevice != INVALID_HANDLE_VALUE) {
            printf("[+] Opened device: %ls\n", devicePaths[i]);
            return DRV_SUCCESS;
        }
    }

    ctx->hDevice = NULL;
    return DRV_ERROR_OPEN_DEVICE;
}

DRV_STATUS DrvEstablishSession(DRV_CONTEXT* ctx) {
    SUPCOOKIE_IN in = {0};
    SUPCOOKIE_OUT out = {0};

    // Fill header (no cookies yet - this is our first call)
    in.Hdr.cbIn = sizeof(in);
    in.Hdr.cbOut = sizeof(out);

    // Magic string
    memcpy(in.szMagic, SUP_COOKIE_MAGIC, sizeof(SUP_COOKIE_MAGIC));

    // Request version (VirtualBox 6.1.x compatible)
    in.u32ReqVersion = 0x00290000;  // 6.1.x
    in.u32MinVersion = 0x00230000;  // 6.0.0

    if (!DoIoctl(ctx->hDevice, SUP_IOCTL_COOKIE, &in, sizeof(in), &out, sizeof(out), NULL)) {
        return DRV_ERROR_SESSION_INIT;
    }

    if (out.Hdr.rc != VINF_SUCCESS) {
        return DRV_ERROR_SESSION_INIT;
    }

    // Save session info
    ctx->Cookie = out.u32Cookie;
    ctx->SessionCookie = out.u32SessionCookie;
    ctx->pSession = out.pSession;
    ctx->Initialized = true;

    return DRV_SUCCESS;
}

// =============================================================================
// VMX Capabilities
// =============================================================================

DRV_STATUS DrvQueryVmxCaps(DRV_CONTEXT* ctx, uint32_t* caps) {
    if (!ctx->Initialized) return DRV_ERROR_SESSION_INIT;

    SUPVTCAPS_IN in = {0};
    SUPVTCAPS_OUT out = {0};

    FillHeader(&in.Hdr, ctx, sizeof(in), sizeof(out));

    if (!DoIoctl(ctx->hDevice, SUP_IOCTL_VT_CAPS, &in, sizeof(in), &out, sizeof(out), NULL)) {
        return DRV_ERROR_IOCTL_FAILED;
    }

    if (out.Hdr.rc != VINF_SUCCESS) {
        return DRV_ERROR_VMX_NOT_SUPPORTED;
    }

    *caps = out.fCaps;
    return DRV_SUCCESS;
}

DRV_STATUS DrvGetVmxMsrs(DRV_CONTEXT* ctx, VMX_MSRS* msrs) {
    if (!ctx->Initialized) return DRV_ERROR_SESSION_INIT;

    SUPHWVIRTMSRS_IN in = {0};
    SUPHWVIRTMSRS_OUT out = {0};

    FillHeader(&in.Hdr, ctx, sizeof(in), sizeof(out));
    in.fCaps = 0;

    if (!DoIoctl(ctx->hDevice, SUP_IOCTL_GET_HWVIRT_MSRS, &in, sizeof(in), &out, sizeof(out), NULL)) {
        return DRV_ERROR_IOCTL_FAILED;
    }

    if (out.Hdr.rc != VINF_SUCCESS) {
        return DRV_ERROR_IOCTL_FAILED;
    }

    // Copy MSR values
    msrs->FeatCtrl = out.u64FeatCtrl;
    msrs->Basic = out.u64Basic;
    msrs->PinCtls = out.u64PinCtls;
    msrs->ProcCtls = out.u64ProcCtls;
    msrs->ProcCtls2 = out.u64ProcCtls2;
    msrs->ExitCtls = out.u64ExitCtls;
    msrs->EntryCtls = out.u64EntryCtls;
    msrs->TruePin = out.u64TruePin;
    msrs->TrueProc = out.u64TrueProc;
    msrs->TrueExit = out.u64TrueExit;
    msrs->TrueEntry = out.u64TrueEntry;
    msrs->Misc = out.u64Misc;
    msrs->Cr0Fixed0 = out.u64Cr0Fixed0;
    msrs->Cr0Fixed1 = out.u64Cr0Fixed1;
    msrs->Cr4Fixed0 = out.u64Cr4Fixed0;
    msrs->Cr4Fixed1 = out.u64Cr4Fixed1;
    msrs->VmcsEnum = out.u64VmcsEnum;
    msrs->EptVpidCap = out.u64EptVpidCap;

    // Cache in context
    memcpy(&ctx->VmxMsrs, msrs, sizeof(VMX_MSRS));

    return DRV_SUCCESS;
}

// =============================================================================
// Memory Allocation
// =============================================================================

DRV_STATUS DrvAllocContiguous(DRV_CONTEXT* ctx, uint32_t pages, ALLOC_INFO* info) {
    if (!ctx->Initialized) return DRV_ERROR_SESSION_INIT;

    SUPCONTALLOC_IN in = {0};
    SUPCONTALLOC_OUT out = {0};

    FillHeader(&in.Hdr, ctx, sizeof(in), sizeof(out));
    in.cPages = pages;

    if (!DoIoctl(ctx->hDevice, SUP_IOCTL_CONT_ALLOC, &in, sizeof(in), &out, sizeof(out), NULL)) {
        return DRV_ERROR_ALLOC_FAILED;
    }

    if (out.Hdr.rc != VINF_SUCCESS || !out.pvR3) {
        return DRV_ERROR_ALLOC_FAILED;
    }

    info->R3 = out.pvR3;
    info->R0 = out.pvR0;
    info->Physical = out.HCPhys;
    info->Pages = pages;
    info->Contiguous = true;

    return DRV_SUCCESS;
}

DRV_STATUS DrvFreeContiguous(DRV_CONTEXT* ctx, ALLOC_INFO* info) {
    if (!ctx->Initialized) return DRV_ERROR_SESSION_INIT;
    if (!info->Contiguous || !info->R3) return DRV_SUCCESS;

    SUPCONTFREE_IN in = {0};
    SUPCONTFREE_OUT out = {0};

    FillHeader(&in.Hdr, ctx, sizeof(in), sizeof(out));
    in.pvR3 = info->R3;

    DoIoctl(ctx->hDevice, SUP_IOCTL_CONT_FREE, &in, sizeof(in), &out, sizeof(out), NULL);

    memset(info, 0, sizeof(*info));
    return DRV_SUCCESS;
}

DRV_STATUS DrvAllocPages(DRV_CONTEXT* ctx, uint32_t pages, bool kernelMap, ALLOC_INFO* info) {
    if (!ctx->Initialized) return DRV_ERROR_SESSION_INIT;

    // Allocate buffer for request + physical addresses
    size_t outSize = sizeof(SUPPAGEALLOC_OUT) + pages * sizeof(uint64_t);
    SUPPAGEALLOC_IN in = {0};
    SUPPAGEALLOC_OUT* out = (SUPPAGEALLOC_OUT*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, outSize);
    if (!out) return DRV_ERROR_ALLOC_FAILED;

    FillHeader(&in.Hdr, ctx, sizeof(in), (uint32_t)outSize);
    in.cPages = pages;
    in.fKernelMapping = kernelMap ? 1 : 0;
    in.fUserMapping = 1;

    bool success = DoIoctl(ctx->hDevice, SUP_IOCTL_PAGE_ALLOC_EX, &in, sizeof(in), out, (uint32_t)outSize, NULL);

    if (!success || out->Hdr.rc != VINF_SUCCESS || !out->pvR3) {
        HeapFree(GetProcessHeap(), 0, out);
        return DRV_ERROR_ALLOC_FAILED;
    }

    info->R3 = out->pvR3;
    info->R0 = out->pvR0;
    info->Physical = 0;  // Pages may not be contiguous
    info->Pages = pages;
    info->Contiguous = false;

    HeapFree(GetProcessHeap(), 0, out);
    return DRV_SUCCESS;
}

DRV_STATUS DrvFreePages(DRV_CONTEXT* ctx, ALLOC_INFO* info) {
    if (!ctx->Initialized) return DRV_ERROR_SESSION_INIT;
    if (info->Contiguous || !info->R3) return DRV_SUCCESS;

    SUPPAGEFREE_IN in = {0};
    SUPPAGEFREE_OUT out = {0};

    FillHeader(&in.Hdr, ctx, sizeof(in), sizeof(out));
    in.pvR3 = info->R3;

    DoIoctl(ctx->hDevice, SUP_IOCTL_PAGE_FREE, &in, sizeof(in), &out, sizeof(out), NULL);

    memset(info, 0, sizeof(*info));
    return DRV_SUCCESS;
}

// =============================================================================
// Code Execution
// =============================================================================

DRV_STATUS DrvExecuteRing0(DRV_CONTEXT* ctx, void* func, uint64_t arg, int32_t* result) {
    return DrvExecuteOnCpu(ctx, (uint32_t)-1, func, arg, result);  // -1 = current CPU
}

DRV_STATUS DrvExecuteOnCpu(DRV_CONTEXT* ctx, uint32_t cpuId, void* func, uint64_t arg, int32_t* result) {
    if (!ctx->Initialized) return DRV_ERROR_SESSION_INIT;

    SUPCALLVMMR0_IN in = {0};
    SUPCALLVMMR0_OUT out = {0};

    FillHeader(&in.Hdr, ctx, sizeof(in), sizeof(out));
    in.pVMR0 = NULL;
    in.idCpu = cpuId;
    in.uOperation = 0;  // Direct function call
    in.u64Arg = arg;
    // The function pointer is passed via pVMR0 or embedded - driver dependent
    // For SUPDrv, we may need to use LDR_LOAD first

    // This IOCTL is tricky - may need module loaded first
    // For now, we'll use a different approach: allocate memory, copy code, call

    if (!DoIoctl(ctx->hDevice, SUP_IOCTL_CALL_VMMR0, &in, sizeof(in), &out, sizeof(out), NULL)) {
        return DRV_ERROR_EXEC_FAILED;
    }

    if (out.Hdr.rc != VINF_SUCCESS) {
        return DRV_ERROR_EXEC_FAILED;
    }

    if (result) *result = out.iResult;
    return DRV_SUCCESS;
}

// =============================================================================
// Module Loading (LDR_OPEN / LDR_LOAD)
// =============================================================================

DRV_STATUS DrvLdrOpen(DRV_CONTEXT* ctx, uint32_t imageSize, void** ppImageBase) {
    if (!ctx->Initialized) return DRV_ERROR_SESSION_INIT;

    SUPLDROPEN_IN in = {0};
    SUPLDROPEN_OUT out = {0};

    FillHeader(&in.Hdr, ctx, sizeof(in), sizeof(out));
    in.cbImageWithEverything = imageSize;
    in.cbImageBits = imageSize;
    strncpy_s(in.szName, sizeof(in.szName), "OmbraHV", _TRUNCATE);
    strncpy_s(in.szFilename, sizeof(in.szFilename), "C:\\Windows\\System32\\drivers\\ombrahv.sys", _TRUNCATE);

    if (!DoIoctl(ctx->hDevice, SUP_IOCTL_LDR_OPEN, &in, sizeof(in), &out, sizeof(out), NULL)) {
        return DRV_ERROR_IOCTL_FAILED;
    }

    if (out.Hdr.rc != VINF_SUCCESS || !out.pvImageBase) {
        return DRV_ERROR_IOCTL_FAILED;
    }

    *ppImageBase = out.pvImageBase;
    return DRV_SUCCESS;
}

DRV_STATUS DrvLdrLoad(DRV_CONTEXT* ctx, void* imageBase, const void* imageData,
                      uint32_t imageSize, void* entryPoint) {
    if (!ctx->Initialized) return DRV_ERROR_SESSION_INIT;

    // Calculate total request size (header + fixed fields + image data)
    size_t reqSize = sizeof(SUPLDRLOAD_IN) + imageSize;
    SUPLDRLOAD_IN* in = (SUPLDRLOAD_IN*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, reqSize);
    if (!in) return DRV_ERROR_ALLOC_FAILED;

    SUPLDRLOAD_OUT out = {0};

    FillHeader(&in->Hdr, ctx, (uint32_t)reqSize, sizeof(out));
    in->pvImageBase = imageBase;
    in->pfnModuleInit = entryPoint;
    in->pfnModuleTerm = NULL;
    in->cbImageBits = imageSize;
    in->offStrTab = 0;
    in->cbStrTab = 0;
    in->offSymbols = 0;
    in->cSymbols = 0;

    // Copy image data after the fixed fields
    memcpy((uint8_t*)in + sizeof(SUPLDRLOAD_IN), imageData, imageSize);

    bool success = DoIoctl(ctx->hDevice, SUP_IOCTL_LDR_LOAD, in, (uint32_t)reqSize, &out, sizeof(out), NULL);

    HeapFree(GetProcessHeap(), 0, in);

    if (!success || out.Hdr.rc != VINF_SUCCESS) {
        return DRV_ERROR_EXEC_FAILED;
    }

    return DRV_SUCCESS;
}

DRV_STATUS DrvLdrFree(DRV_CONTEXT* ctx, void* imageBase) {
    if (!ctx->Initialized) return DRV_ERROR_SESSION_INIT;
    if (!imageBase) return DRV_SUCCESS;  // Nothing to free

    SUPLDRFREE_IN in = {0};
    SUPLDRFREE_OUT out = {0};

    FillHeader(&in.Hdr, ctx, sizeof(in), sizeof(out));
    in.pvImageBase = imageBase;

    DWORD bytesReturned;
    BOOL success = DeviceIoControl(
        ctx->hDevice,
        SUP_IOCTL_LDR_FREE,
        &in, sizeof(in),
        &out, sizeof(out),
        &bytesReturned,
        NULL
    );

    if (!success || out.Hdr.rc != VINF_SUCCESS) {
        return DRV_ERROR_IOCTL_FAILED;
    }

    return DRV_SUCCESS;
}

// =============================================================================
// Symbol Resolution
// =============================================================================

DRV_STATUS DrvGetSymbol(DRV_CONTEXT* ctx, const char* name, void** addr) {
    if (!ctx->Initialized) return DRV_ERROR_SESSION_INIT;

    SUPLDRGETSYMBOL_IN in = {0};
    SUPLDRGETSYMBOL_OUT out = {0};

    FillHeader(&in.Hdr, ctx, sizeof(in), sizeof(out));
    in.pvImageBase = NULL;  // NULL = ntoskrnl
    strncpy_s(in.szSymbol, sizeof(in.szSymbol), name, _TRUNCATE);

    if (!DoIoctl(ctx->hDevice, SUP_IOCTL_LDR_GET_SYMBOL, &in, sizeof(in), &out, sizeof(out), NULL)) {
        return DRV_ERROR_IOCTL_FAILED;
    }

    if (out.Hdr.rc != VINF_SUCCESS || !out.pvSymbol) {
        return DRV_ERROR_IOCTL_FAILED;
    }

    *addr = out.pvSymbol;
    return DRV_SUCCESS;
}

// =============================================================================
// MSR Access
// =============================================================================

DRV_STATUS DrvReadMsr(DRV_CONTEXT* ctx, uint32_t msr, uint32_t cpu, uint64_t* value) {
    if (!ctx->Initialized) return DRV_ERROR_SESSION_INIT;

    SUPMSRPROBER_IN in = {0};
    SUPMSRPROBER_OUT out = {0};

    FillHeader(&in.Hdr, ctx, sizeof(in), sizeof(out));
    in.uMsr = msr;
    in.idCpu = cpu;
    in.uOp = SUP_MSR_OP_READ;

    if (!DoIoctl(ctx->hDevice, SUP_IOCTL_MSR_PROBER, &in, sizeof(in), &out, sizeof(out), NULL)) {
        return DRV_ERROR_IOCTL_FAILED;
    }

    if (out.Hdr.rc != VINF_SUCCESS) {
        return DRV_ERROR_IOCTL_FAILED;
    }

    *value = out.u64Out;
    return DRV_SUCCESS;
}

DRV_STATUS DrvWriteMsr(DRV_CONTEXT* ctx, uint32_t msr, uint32_t cpu, uint64_t value) {
    if (!ctx->Initialized) return DRV_ERROR_SESSION_INIT;

    SUPMSRPROBER_IN in = {0};
    SUPMSRPROBER_OUT out = {0};

    FillHeader(&in.Hdr, ctx, sizeof(in), sizeof(out));
    in.uMsr = msr;
    in.idCpu = cpu;
    in.uOp = SUP_MSR_OP_WRITE;
    in.u64In = value;

    if (!DoIoctl(ctx->hDevice, SUP_IOCTL_MSR_PROBER, &in, sizeof(in), &out, sizeof(out), NULL)) {
        return DRV_ERROR_IOCTL_FAILED;
    }

    if (out.Hdr.rc != VINF_SUCCESS) {
        return DRV_ERROR_IOCTL_FAILED;
    }

    return DRV_SUCCESS;
}

// =============================================================================
// High-Level Initialization
// =============================================================================

DRV_STATUS DrvInitialize(DRV_CONTEXT* ctx, const wchar_t* driverPath) {
    memset(ctx, 0, sizeof(*ctx));

    DRV_STATUS status;

    // Step 1: Load driver via SCM
    status = DrvLoadDriver(ctx, driverPath);
    if (status != DRV_SUCCESS) {
        return status;
    }

    // Step 2: Open device
    status = DrvOpenDevice(ctx);
    if (status != DRV_SUCCESS) {
        DrvUnloadDriver(ctx);
        return status;
    }

    // Step 3: Establish session
    status = DrvEstablishSession(ctx);
    if (status != DRV_SUCCESS) {
        CloseHandle(ctx->hDevice);
        DrvUnloadDriver(ctx);
        return status;
    }

    // Step 4: Cache VMX MSRs
    status = DrvGetVmxMsrs(ctx, &ctx->VmxMsrs);
    if (status != DRV_SUCCESS) {
        // Non-fatal - might not have VMX
    }

    return DRV_SUCCESS;
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
