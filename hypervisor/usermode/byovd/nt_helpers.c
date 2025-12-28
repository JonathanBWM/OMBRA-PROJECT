/**
 * @file nt_helpers.c
 * @brief NT kernel function wrappers
 *
 * Provides NtCreateFile for opening devices without DOS symlinks,
 * NtQuerySystemInformation for finding driver base addresses.
 */

#include "nt_defs.h"
#include <stdio.h>

//=============================================================================
// Global Function Pointers
//=============================================================================

PFN_NtCreateFile             g_NtCreateFile = NULL;
PFN_NtLoadDriver             g_NtLoadDriver = NULL;
PFN_NtUnloadDriver           g_NtUnloadDriver = NULL;
PFN_RtlInitUnicodeString     g_RtlInitUnicodeString = NULL;
PFN_NtQuerySystemInformation g_NtQuerySystemInformation = NULL;
PFN_RtlNtStatusToDosError    g_RtlNtStatusToDosError = NULL;

static bool g_NtInitialized = false;

//=============================================================================
// Initialization
//=============================================================================

bool NtInit(void) {
    if (g_NtInitialized) {
        return true;
    }

    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (!hNtdll) {
        DbgLog("NtInit: Failed to get ntdll.dll handle");
        return false;
    }

    g_NtCreateFile = (PFN_NtCreateFile)GetProcAddress(hNtdll, "NtCreateFile");
    g_NtLoadDriver = (PFN_NtLoadDriver)GetProcAddress(hNtdll, "NtLoadDriver");
    g_NtUnloadDriver = (PFN_NtUnloadDriver)GetProcAddress(hNtdll, "NtUnloadDriver");
    g_RtlInitUnicodeString = (PFN_RtlInitUnicodeString)GetProcAddress(hNtdll, "RtlInitUnicodeString");
    g_NtQuerySystemInformation = (PFN_NtQuerySystemInformation)GetProcAddress(hNtdll, "NtQuerySystemInformation");
    g_RtlNtStatusToDosError = (PFN_RtlNtStatusToDosError)GetProcAddress(hNtdll, "RtlNtStatusToDosError");

    if (!g_NtCreateFile || !g_RtlInitUnicodeString || !g_NtQuerySystemInformation) {
        DbgLog("NtInit: Failed to resolve critical ntdll functions");
        return false;
    }

    g_NtInitialized = true;
    DbgLog("NtInit: NTDLL functions resolved successfully");
    return true;
}

//=============================================================================
// Open Device via NtCreateFile
//=============================================================================

HANDLE NtOpenDevice(const wchar_t* wszNtDevicePath) {
    if (!g_NtInitialized && !NtInit()) {
        return INVALID_HANDLE_VALUE;
    }

    DbgLog("NtOpenDevice: Opening %ls", wszNtDevicePath);

    // Initialize UNICODE_STRING for the device path
    NT_UNICODE_STRING usDeviceName;
    g_RtlInitUnicodeString(&usDeviceName, wszNtDevicePath);

    // Initialize OBJECT_ATTRIBUTES
    NT_OBJECT_ATTRIBUTES objAttr;
    InitializeObjectAttributes(&objAttr, &usDeviceName, OBJ_CASE_INSENSITIVE, NULL, NULL);

    // Open the device
    NT_IO_STATUS_BLOCK ioStatus = {0};
    HANDLE hDevice = INVALID_HANDLE_VALUE;

    NTSTATUS status = g_NtCreateFile(
        &hDevice,
        GENERIC_READ | GENERIC_WRITE,
        &objAttr,
        &ioStatus,
        NULL,                       // AllocationSize
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN,                  // CreateDisposition - open existing
        FILE_NON_DIRECTORY_FILE,
        NULL,                       // EaBuffer
        0                           // EaLength
    );

    if (!NT_SUCCESS(status)) {
        DbgLog("NtOpenDevice: NtCreateFile failed with NTSTATUS 0x%08X", status);
        DbgLog("  0xC0000034 = STATUS_OBJECT_NAME_NOT_FOUND");
        DbgLog("  0xC0000022 = STATUS_ACCESS_DENIED");

        if (g_RtlNtStatusToDosError) {
            SetLastError(g_RtlNtStatusToDosError(status));
        }
        return INVALID_HANDLE_VALUE;
    }

    DbgLog("NtOpenDevice: Opened %ls, handle=%p", wszNtDevicePath, hDevice);
    return hDevice;
}

//=============================================================================
// Get Driver Base Address
//=============================================================================

UINT64 NtGetDriverBase(const wchar_t* wszDriverName) {
    if (!g_NtInitialized && !NtInit()) {
        return 0;
    }

    DbgLog("NtGetDriverBase: Looking for %ls", wszDriverName);

    // Query required buffer size
    ULONG cbNeeded = 0;
    g_NtQuerySystemInformation(SystemModuleInformation, NULL, 0, &cbNeeded);
    if (cbNeeded == 0) {
        DbgLog("NtGetDriverBase: SystemModuleInformation query failed");
        return 0;
    }

    // Allocate buffer with extra space
    PVOID pBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbNeeded + 0x1000);
    if (!pBuffer) {
        DbgLog("NtGetDriverBase: HeapAlloc failed");
        return 0;
    }

    PRTL_PROCESS_MODULES pModules = (PRTL_PROCESS_MODULES)pBuffer;
    NTSTATUS status = g_NtQuerySystemInformation(
        SystemModuleInformation,
        pModules,
        cbNeeded + 0x1000,
        &cbNeeded
    );

    if (!NT_SUCCESS(status)) {
        DbgLog("NtGetDriverBase: NtQuerySystemInformation failed: 0x%08X", status);
        HeapFree(GetProcessHeap(), 0, pBuffer);
        return 0;
    }

    DbgLog("NtGetDriverBase: Found %lu loaded modules", pModules->NumberOfModules);

    // Convert wide string to narrow for comparison
    char szDriverName[256] = {0};
    WideCharToMultiByte(CP_ACP, 0, wszDriverName, -1, szDriverName, sizeof(szDriverName), NULL, NULL);
    _strlwr_s(szDriverName, sizeof(szDriverName));

    UINT64 result = 0;

    for (ULONG i = 0; i < pModules->NumberOfModules; i++) {
        const char* pFullPath = (const char*)pModules->Modules[i].FullPathName;
        const char* pFileName = pFullPath + pModules->Modules[i].OffsetToFileName;

        char szFileName[256];
        strncpy_s(szFileName, sizeof(szFileName), pFileName, _TRUNCATE);
        _strlwr_s(szFileName, sizeof(szFileName));

        if (strstr(szFileName, szDriverName) != NULL) {
            result = (UINT64)pModules->Modules[i].ImageBase;
            DbgLog("NtGetDriverBase: Found '%s' at 0x%016llX (size: 0x%X)",
                   pFileName, result, pModules->Modules[i].ImageSize);
            break;
        }
    }

    if (result == 0) {
        DbgLog("NtGetDriverBase: Driver '%ls' NOT FOUND", wszDriverName);
    }

    HeapFree(GetProcessHeap(), 0, pBuffer);
    return result;
}

//=============================================================================
// Resolve Kernel Export (from usermode)
//=============================================================================
//
// Approach:
// 1. Get kernel module base from NtQuerySystemInformation
// 2. Load module into usermode with DONT_RESOLVE_DLL_REFERENCES
// 3. Parse export directory to get function RVA
// 4. Return kernel_base + RVA
//
// This allows resolving kernel symbols without SUP_IOCTL_LDR_GET_SYMBOL.

UINT64 NtGetKernelExport(const char* szFunctionName) {
    printf("[DEBUG] NtGetKernelExport ENTRY: '%s'\n", szFunctionName);
    fflush(stdout);

    if (!g_NtInitialized && !NtInit()) {
        DbgLog("NtGetKernelExport: NtInit failed");
        return 0;
    }

    printf("[DEBUG] NtGetKernelExport: NtInit passed\n");
    fflush(stdout);

    DbgLog("NtGetKernelExport: Resolving '%s'", szFunctionName);

    // Get ntoskrnl kernel base
    UINT64 kernelBase = NtGetDriverBase(L"ntoskrnl");
    if (kernelBase == 0) {
        DbgLog("NtGetKernelExport: Failed to get ntoskrnl base");
        return 0;
    }

    DbgLog("NtGetKernelExport: ntoskrnl base = 0x%016llX", kernelBase);

    // Load ntoskrnl into usermode without resolving imports
    // This gives us a usermode copy to parse the export table
    HMODULE hKernel = LoadLibraryExW(
        L"ntoskrnl.exe",
        NULL,
        DONT_RESOLVE_DLL_REFERENCES
    );

    if (!hKernel) {
        DbgLog("NtGetKernelExport: LoadLibraryExW failed: %lu", GetLastError());
        return 0;
    }

    // Get usermode export address
    FARPROC pUserExport = GetProcAddress(hKernel, szFunctionName);
    if (!pUserExport) {
        DbgLog("NtGetKernelExport: GetProcAddress('%s') failed: %lu",
               szFunctionName, GetLastError());
        FreeLibrary(hKernel);
        return 0;
    }

    // Calculate RVA: usermode_addr - usermode_base
    UINT64 userBase = (UINT64)hKernel;
    UINT64 userAddr = (UINT64)pUserExport;
    UINT64 rva = userAddr - userBase;

    // Calculate kernel address: kernel_base + RVA
    UINT64 kernelAddr = kernelBase + rva;

    DbgLog("NtGetKernelExport: %s = 0x%016llX (RVA=0x%llX)",
           szFunctionName, kernelAddr, rva);

    FreeLibrary(hKernel);
    return kernelAddr;
}
