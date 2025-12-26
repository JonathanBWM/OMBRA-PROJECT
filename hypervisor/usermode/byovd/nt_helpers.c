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
