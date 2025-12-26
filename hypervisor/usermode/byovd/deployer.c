/**
 * @file deployer.c
 * @brief Portable driver deployment implementation (C port)
 */

#include "deployer.h"
#include "crypto.h"
#include "nt_defs.h"
#include <stdio.h>
#include <string.h>

//=============================================================================
// Forward Declarations
//=============================================================================

static void Deployer_SetError(PDEPLOYER_CTX ctx, const char* format, ...);
static bool SecureDeleteFile(const wchar_t* wszPath);

//=============================================================================
// Known Driver Paths
//=============================================================================

const wchar_t* KNOWN_DRIVER_PATHS[] = {
    L"C:\\Program Files\\ldplayer9box\\Ld9BoxSup.sys",
    L"C:\\Program Files\\LDPlayer\\LDPlayer9\\vbox\\drivers\\Ld9BoxSup.sys",
    L"C:\\Program Files (x86)\\LDPlayer\\LDPlayer9\\vbox\\drivers\\Ld9BoxSup.sys",
    L"C:\\Program Files\\LDPlayer9\\vbox\\drivers\\Ld9BoxSup.sys",
    L"C:\\LDPlayer\\LDPlayer9\\vbox\\drivers\\Ld9BoxSup.sys",
};

const size_t KNOWN_DRIVER_PATHS_COUNT = sizeof(KNOWN_DRIVER_PATHS) / sizeof(KNOWN_DRIVER_PATHS[0]);

//=============================================================================
// Default Resource
//=============================================================================

const DRIVER_RESOURCE DEFAULT_DRIVER_RESOURCE = {
    L"LD9BOXSUP_ENCRYPTED",     // Resource name
    RT_RCDATA,                   // Resource type
    DEVICE_NAME_LDPLAYER,        // Device name
    L"Ld9BoxSup",                // Service name
    0                            // Use header key
};

//=============================================================================
// Initialization / Cleanup
//=============================================================================

void Deployer_Init(PDEPLOYER_CTX ctx) {
    if (!ctx) return;

    memset(ctx, 0, sizeof(DEPLOYER_CTX));
    ctx->hDevice = INVALID_HANDLE_VALUE;
    ctx->hService = NULL;
    ctx->hSCManager = NULL;
    ctx->bDeployed = false;
    ctx->bCreatedService = false;
    ctx->bCreatedRegistry = false;
    ctx->Method = DEPLOY_AUTO;
}

void Deployer_Cleanup(PDEPLOYER_CTX ctx, bool bDeleteFiles) {
    if (!ctx) return;

    DbgLog("Deployer_Cleanup: Starting...");

    // Close device handle
    if (ctx->hDevice != INVALID_HANDLE_VALUE) {
        CloseHandle(ctx->hDevice);
        ctx->hDevice = INVALID_HANDLE_VALUE;
    }

    if (ctx->Method == DEPLOY_SCM && ctx->hService) {
        // Stop service
        SERVICE_STATUS status;
        ControlService(ctx->hService, SERVICE_CONTROL_STOP, &status);
        Sleep(100);

        // Delete if we created it
        if (ctx->bCreatedService) {
            DeleteService(ctx->hService);
        }

        CloseServiceHandle(ctx->hService);
        ctx->hService = NULL;
    }

    if (ctx->hSCManager) {
        CloseServiceHandle(ctx->hSCManager);
        ctx->hSCManager = NULL;
    }

    if (ctx->Method == DEPLOY_NTLOAD) {
        // Unload driver
        if (g_NtUnloadDriver && g_RtlInitUnicodeString && ctx->wszServiceName[0] != L'\0') {
            wchar_t ntRegPath[320];
            swprintf_s(ntRegPath, sizeof(ntRegPath)/sizeof(wchar_t),
                       L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\%ls",
                       ctx->wszServiceName);

            NT_UNICODE_STRING usDriverPath;
            g_RtlInitUnicodeString(&usDriverPath, ntRegPath);
            g_NtUnloadDriver(&usDriverPath);
        }

        // Delete registry
        if (ctx->bCreatedRegistry && ctx->wszServiceName[0] != L'\0') {
            Deployer_DeleteDriverRegistry(ctx->wszServiceName);
        }
    }

    // Delete driver file
    if (bDeleteFiles && ctx->wszDriverPath[0] != L'\0') {
        SecureDeleteFile(ctx->wszDriverPath);
    }

    ctx->bDeployed = false;
    ctx->bCreatedService = false;
    ctx->bCreatedRegistry = false;
    ctx->wszDriverPath[0] = L'\0';
    ctx->wszServiceName[0] = L'\0';

    DbgLog("Deployer_Cleanup: Complete");
}

//=============================================================================
// Error Handling
//=============================================================================

static void Deployer_SetError(PDEPLOYER_CTX ctx, const char* format, ...) {
    if (!ctx) return;

    va_list args;
    va_start(args, format);
    vsnprintf(ctx->szLastError, sizeof(ctx->szLastError), format, args);
    va_end(args);

    DbgLog("[Deployer] ERROR: %s", ctx->szLastError);
}

//=============================================================================
// Accessors
//=============================================================================

bool Deployer_IsDeployed(PDEPLOYER_CTX ctx) {
    return ctx && ctx->bDeployed;
}

HANDLE Deployer_GetDeviceHandle(PDEPLOYER_CTX ctx) {
    return ctx ? ctx->hDevice : INVALID_HANDLE_VALUE;
}

HANDLE Deployer_ReleaseDeviceHandle(PDEPLOYER_CTX ctx) {
    if (!ctx) return INVALID_HANDLE_VALUE;
    HANDLE h = ctx->hDevice;
    ctx->hDevice = INVALID_HANDLE_VALUE;
    return h;
}

const wchar_t* Deployer_GetDriverPath(PDEPLOYER_CTX ctx) {
    return ctx ? ctx->wszDriverPath : L"";
}

const wchar_t* Deployer_GetServiceName(PDEPLOYER_CTX ctx) {
    return ctx ? ctx->wszServiceName : L"";
}

const char* Deployer_GetLastError(PDEPLOYER_CTX ctx) {
    return ctx ? ctx->szLastError : "NULL context";
}

DEPLOY_METHOD Deployer_GetMethod(PDEPLOYER_CTX ctx) {
    return ctx ? ctx->Method : DEPLOY_AUTO;
}

//=============================================================================
// Privilege Management
//=============================================================================

bool Deployer_EnablePrivilege(const wchar_t* wszPrivilege) {
    HANDLE hToken = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return false;
    }

    TOKEN_PRIVILEGES tp = {0};
    if (!LookupPrivilegeValueW(NULL, wszPrivilege, &tp.Privileges[0].Luid)) {
        CloseHandle(hToken);
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    BOOL bResult = AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
    DWORD dwError = GetLastError();
    CloseHandle(hToken);

    return bResult && (dwError == ERROR_SUCCESS);
}

//=============================================================================
// Static Helpers
//=============================================================================

bool Deployer_IsDriverLoaded(void) {
    // Try LDPlayer device
    HANDLE hDevice = CreateFileW(
        DEVICE_NAME_LDPLAYER,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hDevice != INVALID_HANDLE_VALUE) {
        CloseHandle(hDevice);
        return true;
    }

    // Try VBoxDrv
    hDevice = CreateFileW(
        DEVICE_NAME_VBOX,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hDevice != INVALID_HANDLE_VALUE) {
        CloseHandle(hDevice);
        return true;
    }

    return false;
}

bool Deployer_IsServiceRegistered(const wchar_t* wszServiceName) {
    SC_HANDLE hSCM = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCM) return false;

    SC_HANDLE hService = OpenServiceW(hSCM, wszServiceName, SERVICE_QUERY_STATUS);
    bool bExists = (hService != NULL);

    if (hService) CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);

    return bExists;
}

bool Deployer_GetServiceStatus(const wchar_t* wszServiceName, DWORD* pStatus) {
    SC_HANDLE hSCM = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCM) return false;

    SC_HANDLE hService = OpenServiceW(hSCM, wszServiceName, SERVICE_QUERY_STATUS);
    if (!hService) {
        CloseServiceHandle(hSCM);
        return false;
    }

    SERVICE_STATUS status = {0};
    BOOL bResult = QueryServiceStatus(hService, &status);

    if (bResult && pStatus) {
        *pStatus = status.dwCurrentState;
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);

    return bResult != FALSE;
}

static bool SecureDeleteFile(const wchar_t* wszPath) {
    if (!wszPath || !wszPath[0]) return true;

    const int MAX_RETRIES = 5;
    const DWORD RETRY_DELAY_MS = 100;

    for (int attempt = 0; attempt < MAX_RETRIES; attempt++) {
        if (DeleteFileW(wszPath)) {
            DbgLog("SecureDeleteFile: Deleted: %ls", wszPath);
            return true;
        }

        DWORD err = GetLastError();

        if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND) {
            DbgLog("SecureDeleteFile: File not found: %ls", wszPath);
            return true;
        }

        if (err == ERROR_ACCESS_DENIED || err == ERROR_SHARING_VIOLATION ||
            err == ERROR_LOCK_VIOLATION) {
            DbgLog("SecureDeleteFile: Retry %d - error %lu for %ls", attempt + 1, err, wszPath);
            Sleep(RETRY_DELAY_MS);
            continue;
        }

        DbgLog("SecureDeleteFile: Unexpected error %lu for %ls", err, wszPath);
        break;
    }

    // Schedule for deletion on reboot
    if (MoveFileExW(wszPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT)) {
        DbgLog("SecureDeleteFile: Scheduled for reboot deletion: %ls", wszPath);
        return true;
    }

    DbgLog("SecureDeleteFile: FAILED: %ls", wszPath);
    return false;
}

//=============================================================================
// Device Access - Uses NtCreateFile for NT paths
//=============================================================================

bool Deployer_OpenDevice(PDEPLOYER_CTX ctx, const wchar_t* wszDeviceName) {
    if (!ctx || !wszDeviceName) return false;

    DbgLog("Deployer_OpenDevice: Trying: %ls", wszDeviceName);

    // Initialize NT functions if needed
    if (!NtInit()) {
        Deployer_SetError(ctx, "Failed to initialize NT functions");
        return false;
    }

    // Convert DosDevices path to NT path if needed
    wchar_t ntPath[256];
    if (wcsncmp(wszDeviceName, L"\\\\.\\", 4) == 0) {
        swprintf_s(ntPath, sizeof(ntPath)/sizeof(wchar_t),
                   L"\\Device\\%ls", wszDeviceName + 4);
        DbgLog("Deployer_OpenDevice: Converted to NT path: %ls", ntPath);
    } else if (wcsncmp(wszDeviceName, L"\\Device\\", 8) == 0) {
        wcscpy_s(ntPath, sizeof(ntPath)/sizeof(wchar_t), wszDeviceName);
    } else {
        Deployer_SetError(ctx, "Unknown device path format: %ls", wszDeviceName);
        return false;
    }

    // Open using NtCreateFile
    ctx->hDevice = NtOpenDevice(ntPath);
    if (ctx->hDevice == INVALID_HANDLE_VALUE) {
        Deployer_SetError(ctx, "Failed to open device: %ls", ntPath);
        return false;
    }

    DbgLog("Deployer_OpenDevice: SUCCESS: %ls handle=%p", ntPath, ctx->hDevice);
    return true;
}

//=============================================================================
// Extraction
//=============================================================================

bool Deployer_ExtractFromBlob(PDEPLOYER_CTX ctx, const UINT8* pEncrypted,
                               size_t cbEncrypted, UINT32 dwKey,
                               const wchar_t* wszOutputPath) {
    if (!ctx || !pEncrypted || cbEncrypted == 0 || !wszOutputPath) {
        return false;
    }

    DbgLog("Deployer_ExtractFromBlob: Size=%zu, Key=0x%08X", cbEncrypted, dwKey);

    UINT8* pDecrypted = NULL;
    size_t cbDecrypted = 0;
    bool success = false;

    if (dwKey == 0) {
        // Use header-based decryption
        DbgLog("Deployer_ExtractFromBlob: Using header-based decryption...");
        if (!Crypto_DecryptWithHeader(pEncrypted, cbEncrypted, &pDecrypted, &cbDecrypted, NULL)) {
            Deployer_SetError(ctx, "Failed to decrypt driver blob: invalid header or key");
            return false;
        }
    } else {
        // Use provided key
        DbgLog("Deployer_ExtractFromBlob: Using explicit key...");
        if (!Crypto_Decrypt(pEncrypted, cbEncrypted, dwKey, &pDecrypted, &cbDecrypted)) {
            Deployer_SetError(ctx, "Failed to decrypt driver blob");
            return false;
        }
    }

    DbgLog("Deployer_ExtractFromBlob: Decrypted %zu bytes", cbDecrypted);

    // Validate PE signature
    if (cbDecrypted < 2 || pDecrypted[0] != 'M' || pDecrypted[1] != 'Z') {
        DbgLog("Deployer_ExtractFromBlob: Invalid PE signature");
        Deployer_SetError(ctx, "Decrypted data is not a valid PE file");
        HeapFree(GetProcessHeap(), 0, pDecrypted);
        return false;
    }

    DbgLog("Deployer_ExtractFromBlob: PE signature valid");

    // Write to disk
    DbgLog("Deployer_ExtractFromBlob: Writing to: %ls", wszOutputPath);
    HANDLE hFile = CreateFileW(
        wszOutputPath,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        Deployer_SetError(ctx, "Failed to create driver file: error %lu", err);
        HeapFree(GetProcessHeap(), 0, pDecrypted);
        return false;
    }

    DWORD dwWritten = 0;
    BOOL bResult = WriteFile(hFile, pDecrypted, (DWORD)cbDecrypted, &dwWritten, NULL);
    CloseHandle(hFile);

    HeapFree(GetProcessHeap(), 0, pDecrypted);

    if (!bResult || dwWritten != cbDecrypted) {
        Deployer_SetError(ctx, "Failed to write driver file");
        DeleteFileW(wszOutputPath);
        return false;
    }

    DbgLog("Deployer_ExtractFromBlob: SUCCESS: Wrote %lu bytes", dwWritten);
    return true;
}

bool Deployer_ExtractFromResource(PDEPLOYER_CTX ctx, const DRIVER_RESOURCE* pResource,
                                   const wchar_t* wszOutputPath) {
    if (!ctx || !pResource || !wszOutputPath) return false;

    DbgLog("Deployer_ExtractFromResource: Looking for: %ls", pResource->wszResourceName);

    // Find resource
    HRSRC hRes = FindResourceW(NULL, pResource->wszResourceName, pResource->wszResourceType);
    if (!hRes) {
        DWORD err = GetLastError();
        Deployer_SetError(ctx, "Resource not found: %ls (error %lu)",
                          pResource->wszResourceName, err);
        return false;
    }

    // Load resource
    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) {
        Deployer_SetError(ctx, "Failed to load resource");
        return false;
    }

    // Get pointer and size
    LPVOID pEncrypted = LockResource(hData);
    DWORD dwSize = SizeofResource(NULL, hRes);

    if (!pEncrypted || dwSize == 0) {
        Deployer_SetError(ctx, "Failed to access resource data");
        return false;
    }

    DbgLog("Deployer_ExtractFromResource: Resource size: %lu bytes", dwSize);

    return Deployer_ExtractFromBlob(ctx, (const UINT8*)pEncrypted, dwSize,
                                     pResource->dwEncryptionKey, wszOutputPath);
}

//=============================================================================
// Registry Management
//=============================================================================

bool Deployer_CreateDriverRegistry(PDEPLOYER_CTX ctx, const wchar_t* wszDriverPath,
                                    const wchar_t* wszServiceName) {
    if (!ctx || !wszDriverPath || !wszServiceName) return false;

    DbgLog("Deployer_CreateDriverRegistry: Creating for: %ls", wszServiceName);

    wchar_t regPath[320];
    swprintf_s(regPath, sizeof(regPath)/sizeof(wchar_t),
               L"SYSTEM\\CurrentControlSet\\Services\\%ls", wszServiceName);

    HKEY hKey = NULL;
    DWORD dwDisposition = 0;

    LONG result = RegCreateKeyExW(
        HKEY_LOCAL_MACHINE,
        regPath,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        &hKey,
        &dwDisposition
    );

    if (result != ERROR_SUCCESS) {
        Deployer_SetError(ctx, "Failed to create registry key: error %ld", result);
        return false;
    }

    // ImagePath = \??\<DriverPath>
    wchar_t imagePath[320];
    swprintf_s(imagePath, sizeof(imagePath)/sizeof(wchar_t),
               L"\\??\\%ls", wszDriverPath);

    result = RegSetValueExW(hKey, L"ImagePath", 0, REG_EXPAND_SZ,
                            (const BYTE*)imagePath,
                            (DWORD)((wcslen(imagePath) + 1) * sizeof(wchar_t)));

    if (result != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        Deployer_SetError(ctx, "Failed to set ImagePath: error %ld", result);
        return false;
    }

    // Type = SERVICE_KERNEL_DRIVER (1)
    DWORD dwType = 1;
    RegSetValueExW(hKey, L"Type", 0, REG_DWORD, (const BYTE*)&dwType, sizeof(dwType));

    // Start = SERVICE_DEMAND_START (3)
    DWORD dwStart = 3;
    RegSetValueExW(hKey, L"Start", 0, REG_DWORD, (const BYTE*)&dwStart, sizeof(dwStart));

    // ErrorControl = SERVICE_ERROR_NORMAL (1)
    DWORD dwErrorControl = 1;
    RegSetValueExW(hKey, L"ErrorControl", 0, REG_DWORD,
                   (const BYTE*)&dwErrorControl, sizeof(dwErrorControl));

    RegCloseKey(hKey);
    ctx->bCreatedRegistry = true;

    DbgLog("Deployer_CreateDriverRegistry: SUCCESS");
    return true;
}

bool Deployer_DeleteDriverRegistry(const wchar_t* wszServiceName) {
    if (!wszServiceName) return true;

    wchar_t regPath[320];
    swprintf_s(regPath, sizeof(regPath)/sizeof(wchar_t),
               L"SYSTEM\\CurrentControlSet\\Services\\%ls", wszServiceName);

    const int MAX_RETRIES = 5;
    const DWORD RETRY_DELAY_MS = 50;

    for (int attempt = 0; attempt < MAX_RETRIES; attempt++) {
        LSTATUS status = RegDeleteKeyW(HKEY_LOCAL_MACHINE, regPath);

        if (status == ERROR_SUCCESS) {
            DbgLog("Deployer_DeleteDriverRegistry: Deleted (attempt %d)", attempt + 1);
            return true;
        }

        if (status == ERROR_FILE_NOT_FOUND) {
            DbgLog("Deployer_DeleteDriverRegistry: Not found (OK)");
            return true;
        }

        if (status == ERROR_ACCESS_DENIED || status == ERROR_SHARING_VIOLATION) {
            DbgLog("Deployer_DeleteDriverRegistry: Retry %d - error %lu", attempt + 1, status);
            Sleep(RETRY_DELAY_MS);
            continue;
        }

        DbgLog("Deployer_DeleteDriverRegistry: FAILED: error %lu", status);
        return false;
    }

    return false;
}

//=============================================================================
// Existing Installation Detection
//=============================================================================

bool Deployer_TryStartExistingService(PDEPLOYER_CTX ctx, const wchar_t* wszServiceName,
                                       const wchar_t* wszDeviceName) {
    SC_HANDLE hSCM = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCM) return false;

    SC_HANDLE hService = OpenServiceW(hSCM, wszServiceName,
                                        SERVICE_START | SERVICE_QUERY_STATUS);
    if (!hService) {
        CloseServiceHandle(hSCM);
        return false;
    }

    SERVICE_STATUS status = {0};
    if (!QueryServiceStatus(hService, &status)) {
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);
        return false;
    }

    // If running, just open device
    if (status.dwCurrentState == SERVICE_RUNNING) {
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);
        return Deployer_OpenDevice(ctx, wszDeviceName);
    }

    // Try to start
    if (!StartServiceW(hService, 0, NULL)) {
        DWORD err = GetLastError();
        if (err != ERROR_SERVICE_ALREADY_RUNNING) {
            CloseServiceHandle(hService);
            CloseServiceHandle(hSCM);
            return false;
        }
    }

    Sleep(100);
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);

    return Deployer_OpenDevice(ctx, wszDeviceName);
}

bool Deployer_TryUseFromPath(PDEPLOYER_CTX ctx, const wchar_t* wszDriverPath,
                              const wchar_t* wszServiceName,
                              const wchar_t* wszDeviceName) {
    // Check if file exists
    DWORD dwAttrib = GetFileAttributesW(wszDriverPath);
    if (dwAttrib == INVALID_FILE_ATTRIBUTES) return false;

    SC_HANDLE hSCM = OpenSCManagerW(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!hSCM) return false;

    SC_HANDLE hService = CreateServiceW(
        hSCM, wszServiceName, wszServiceName,
        SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
        wszDriverPath, NULL, NULL, NULL, NULL, NULL
    );

    if (!hService) {
        if (GetLastError() == ERROR_SERVICE_EXISTS) {
            hService = OpenServiceW(hSCM, wszServiceName, SERVICE_ALL_ACCESS);
        }
        if (!hService) {
            CloseServiceHandle(hSCM);
            return false;
        }
    }

    if (!StartServiceW(hService, 0, NULL)) {
        DWORD err = GetLastError();
        if (err != ERROR_SERVICE_ALREADY_RUNNING) {
            CloseServiceHandle(hService);
            CloseServiceHandle(hSCM);
            return false;
        }
    }

    Sleep(100);
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);

    return Deployer_OpenDevice(ctx, wszDeviceName);
}

bool Deployer_TryUseExisting(PDEPLOYER_CTX ctx, const DRIVER_RESOURCE* pResource) {
    if (!ctx || !pResource) return false;

    // Strategy 1: Device already accessible
    if (Deployer_OpenDevice(ctx, pResource->wszDeviceName)) {
        ctx->bDeployed = true;
        ctx->Method = DEPLOY_AUTO;
        return true;
    }

    if (Deployer_OpenDevice(ctx, DEVICE_NAME_VBOX)) {
        ctx->bDeployed = true;
        ctx->Method = DEPLOY_AUTO;
        return true;
    }

    // Strategy 2: Service exists but not running
    if (Deployer_IsServiceRegistered(pResource->wszServiceName)) {
        if (Deployer_TryStartExistingService(ctx, pResource->wszServiceName,
                                              pResource->wszDeviceName)) {
            ctx->bDeployed = true;
            ctx->Method = DEPLOY_SCM;
            return true;
        }
    }

    if (Deployer_IsServiceRegistered(L"VBoxDrv")) {
        if (Deployer_TryStartExistingService(ctx, L"VBoxDrv", DEVICE_NAME_VBOX)) {
            ctx->bDeployed = true;
            ctx->Method = DEPLOY_SCM;
            return true;
        }
    }

    // Strategy 3: Scan known paths
    for (size_t i = 0; i < KNOWN_DRIVER_PATHS_COUNT; i++) {
        if (Deployer_TryUseFromPath(ctx, KNOWN_DRIVER_PATHS[i],
                                     pResource->wszServiceName,
                                     pResource->wszDeviceName)) {
            wcscpy_s(ctx->wszDriverPath, sizeof(ctx->wszDriverPath)/sizeof(wchar_t),
                     KNOWN_DRIVER_PATHS[i]);
            ctx->bDeployed = true;
            ctx->Method = DEPLOY_SCM;
            return true;
        }
    }

    return false;
}

//=============================================================================
// SCM-Based Deployment
//=============================================================================

bool Deployer_DeployViaSCM(PDEPLOYER_CTX ctx, const DRIVER_RESOURCE* pResource) {
    if (!ctx || !pResource) return false;

    DbgLog("Deployer_DeployViaSCM: Starting...");

    // Determine output path
    wchar_t wszTargetPath[MAX_PATH];
    if (!GetSystemDirectoryW(wszTargetPath, MAX_PATH)) {
        Deployer_SetError(ctx, "Failed to get system directory");
        return false;
    }
    wcscat_s(wszTargetPath, MAX_PATH, L"\\drivers\\");
    wcscat_s(wszTargetPath, MAX_PATH, pResource->wszServiceName);
    wcscat_s(wszTargetPath, MAX_PATH, L".sys");

    DbgLog("Deployer_DeployViaSCM: Target: %ls", wszTargetPath);

    // Extract driver
    if (!Deployer_ExtractFromResource(ctx, pResource, wszTargetPath)) {
        // Try temp directory
        if (!GetTempPathW(MAX_PATH, wszTargetPath)) {
            Deployer_SetError(ctx, "Failed to get temp path");
            return false;
        }
        wcscat_s(wszTargetPath, MAX_PATH, pResource->wszServiceName);
        wcscat_s(wszTargetPath, MAX_PATH, L".sys");

        if (!Deployer_ExtractFromResource(ctx, pResource, wszTargetPath)) {
            return false;
        }
    }

    wcscpy_s(ctx->wszDriverPath, sizeof(ctx->wszDriverPath)/sizeof(wchar_t), wszTargetPath);
    wcscpy_s(ctx->wszServiceName, sizeof(ctx->wszServiceName)/sizeof(wchar_t),
             pResource->wszServiceName);

    // Open SCM
    ctx->hSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!ctx->hSCManager) {
        DWORD err = GetLastError();
        Deployer_SetError(ctx, "Failed to open SCM: error %lu", err);
        DeleteFileW(wszTargetPath);
        return false;
    }

    // Create service
    ctx->hService = CreateServiceW(
        ctx->hSCManager, ctx->wszServiceName, L"VirtualBox Support Driver",
        SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
        wszTargetPath, NULL, NULL, NULL, NULL, NULL
    );

    if (!ctx->hService) {
        DWORD err = GetLastError();
        if (err == ERROR_SERVICE_EXISTS) {
            ctx->hService = OpenServiceW(ctx->hSCManager, ctx->wszServiceName,
                                          SERVICE_ALL_ACCESS);
            ctx->bCreatedService = false;
        }
        if (!ctx->hService) {
            Deployer_SetError(ctx, "Failed to create/open service: error %lu", err);
            DeleteFileW(wszTargetPath);
            return false;
        }
    } else {
        ctx->bCreatedService = true;
    }

    // Start service
    if (!StartServiceW(ctx->hService, 0, NULL)) {
        DWORD err = GetLastError();
        if (err != ERROR_SERVICE_ALREADY_RUNNING) {
            Deployer_SetError(ctx, "Failed to start service: error %lu", err);
            if (ctx->bCreatedService) DeleteService(ctx->hService);
            DeleteFileW(wszTargetPath);
            return false;
        }
    }

    // Try to open device with retries
    const wchar_t* deviceNames[] = {
        pResource->wszDeviceName,
        NT_DEVICE_NAME_LDPLAYER_USER,
        DEVICE_NAME_VBOX
    };

    for (int retry = 0; retry < 5; retry++) {
        Sleep(100);
        for (int i = 0; i < 3; i++) {
            if (Deployer_OpenDevice(ctx, deviceNames[i])) {
                ctx->Method = DEPLOY_SCM;
                ctx->bDeployed = true;
                DbgLog("Deployer_DeployViaSCM: SUCCESS");
                return true;
            }
        }
    }

    Deployer_SetError(ctx, "Service started but device not accessible");
    return false;
}

//=============================================================================
// NtLoadDriver-Based Deployment
//=============================================================================

bool Deployer_DeployViaNtLoadDriver(PDEPLOYER_CTX ctx, const DRIVER_RESOURCE* pResource) {
    if (!ctx || !pResource) return false;

    DbgLog("Deployer_DeployViaNtLoadDriver: Starting...");

    // Initialize NT functions
    if (!NtInit()) {
        Deployer_SetError(ctx, "Failed to initialize NT functions");
        return false;
    }

    // Determine output path (temp)
    wchar_t wszTargetPath[MAX_PATH];
    if (!GetTempPathW(MAX_PATH, wszTargetPath)) {
        Deployer_SetError(ctx, "Failed to get temp path");
        return false;
    }
    wcscat_s(wszTargetPath, MAX_PATH, pResource->wszServiceName);
    wcscat_s(wszTargetPath, MAX_PATH, L".sys");

    DbgLog("Deployer_DeployViaNtLoadDriver: Target: %ls", wszTargetPath);

    // Extract driver
    if (!Deployer_ExtractFromResource(ctx, pResource, wszTargetPath)) {
        return false;
    }

    wcscpy_s(ctx->wszDriverPath, sizeof(ctx->wszDriverPath)/sizeof(wchar_t), wszTargetPath);
    wcscpy_s(ctx->wszServiceName, sizeof(ctx->wszServiceName)/sizeof(wchar_t),
             pResource->wszServiceName);

    // Create registry
    if (!Deployer_CreateDriverRegistry(ctx, wszTargetPath, pResource->wszServiceName)) {
        SecureDeleteFile(wszTargetPath);
        return false;
    }

    // Build NT registry path
    wchar_t ntRegPath[320];
    swprintf_s(ntRegPath, sizeof(ntRegPath)/sizeof(wchar_t),
               L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\%ls",
               pResource->wszServiceName);

    NT_UNICODE_STRING usDriverPath;
    g_RtlInitUnicodeString(&usDriverPath, ntRegPath);

    // Enable privilege
    if (!Deployer_EnablePrivilege(SE_LOAD_DRIVER_NAME)) {
        Deployer_SetError(ctx, "Failed to enable SeLoadDriverPrivilege");
        Deployer_DeleteDriverRegistry(pResource->wszServiceName);
        SecureDeleteFile(wszTargetPath);
        return false;
    }

    // Load driver
    DbgLog("Deployer_DeployViaNtLoadDriver: Calling NtLoadDriver...");
    NTSTATUS status = g_NtLoadDriver(&usDriverPath);
    DbgLog("Deployer_DeployViaNtLoadDriver: NtLoadDriver returned: 0x%08X", status);

    if (status != STATUS_SUCCESS && status != STATUS_IMAGE_ALREADY_LOADED) {
        Deployer_SetError(ctx, "NtLoadDriver failed: NTSTATUS 0x%08X", status);
        Deployer_DeleteDriverRegistry(pResource->wszServiceName);
        SecureDeleteFile(wszTargetPath);
        return false;
    }

    // Try to open device with retries
    const wchar_t* deviceNames[] = {
        pResource->wszDeviceName,
        NT_DEVICE_NAME_LDPLAYER_USER,
        DEVICE_NAME_VBOX
    };

    bool deviceOpened = false;
    for (int retry = 0; retry < 5 && !deviceOpened; retry++) {
        Sleep(100);
        for (int i = 0; i < 3; i++) {
            if (Deployer_OpenDevice(ctx, deviceNames[i])) {
                deviceOpened = true;
                break;
            }
        }
    }

    if (!deviceOpened) {
        Deployer_SetError(ctx, "Driver loaded but device not accessible");
        // Unload and cleanup
        g_NtUnloadDriver(&usDriverPath);
        Deployer_DeleteDriverRegistry(pResource->wszServiceName);
        SecureDeleteFile(wszTargetPath);
        return false;
    }

    // Immediate artifact elimination
    DbgLog("Deployer_DeployViaNtLoadDriver: Eliminating artifacts...");
    Deployer_DeleteDriverRegistry(pResource->wszServiceName);

    // Flush registry
    HKEY hSystemKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM", 0, KEY_WRITE, &hSystemKey) == ERROR_SUCCESS) {
        RegFlushKey(hSystemKey);
        RegCloseKey(hSystemKey);
    }

    // Delete driver file
    if (SecureDeleteFile(wszTargetPath)) {
        ctx->wszDriverPath[0] = L'\0';
    }
    ctx->bCreatedRegistry = false;

    ctx->Method = DEPLOY_NTLOAD;
    ctx->bDeployed = true;
    DbgLog("Deployer_DeployViaNtLoadDriver: SUCCESS");
    return true;
}

//=============================================================================
// Main Deploy Function
//=============================================================================

bool Deployer_Deploy(PDEPLOYER_CTX ctx, DEPLOY_METHOD method,
                     const DRIVER_RESOURCE* pResource) {
    if (!ctx) return false;

    DbgLog("Deployer_Deploy: Starting...");
    DbgLog("Deployer_Deploy: Method=%d (0=SCM, 1=NtLoad, 2=Auto)", method);

    if (ctx->bDeployed) {
        DbgLog("Deployer_Deploy: Already deployed");
        return true;
    }

    // Use default if none provided
    if (!pResource) {
        pResource = &DEFAULT_DRIVER_RESOURCE;
    }

    // Step 1: Try existing installation
    DbgLog("Deployer_Deploy: Checking for existing driver...");
    if (Deployer_TryUseExisting(ctx, pResource)) {
        DbgLog("Deployer_Deploy: Using existing driver");
        return true;
    }

    // Step 2: Deploy from resources
    DbgLog("Deployer_Deploy: Deploying from resources...");
    switch (method) {
    case DEPLOY_SCM:
        return Deployer_DeployViaSCM(ctx, pResource);

    case DEPLOY_NTLOAD:
        return Deployer_DeployViaNtLoadDriver(ctx, pResource);

    case DEPLOY_AUTO:
    default:
        // Try SCM first
        if (Deployer_DeployViaSCM(ctx, pResource)) {
            return true;
        }
        // Fallback to NtLoadDriver
        ctx->szLastError[0] = '\0';
        return Deployer_DeployViaNtLoadDriver(ctx, pResource);
    }
}
