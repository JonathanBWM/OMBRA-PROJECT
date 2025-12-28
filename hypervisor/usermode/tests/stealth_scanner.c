/**
 * @file stealth_scanner.c
 * @brief Stealth Verification Scanner - BigPool and Timing Analysis
 *
 * Comprehensive verification tool for hypervisor stealth:
 * - BigPool visibility scanning (pre/during/post load)
 * - Timing measurement for pre-VMX visibility window
 * - 'IPRT' pool tag detection (Ld9BoxSup.sys allocations)
 * - Real-time status output with logging
 *
 * Usage:
 *   STEALTH_SCAN_RESULT result;
 *   StealthScanBigPool(&result, SCAN_FILTER_ALL);
 *   StealthPrintResults(&result);
 */

#include "stealth_scanner.h"
#include <stdio.h>
#include <string.h>

//=============================================================================
// NtQuerySystemInformation Types (not in public headers)
//=============================================================================

typedef NTSTATUS (NTAPI *FN_NtQuerySystemInformation)(
    ULONG SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
);

#define SystemBigPoolInformation 0x42

// BigPool entry structure (from Windows internals)
typedef struct _SYSTEM_BIGPOOL_ENTRY {
    union {
        PVOID VirtualAddress;
        ULONG_PTR NonPaged : 1;  // Low bit = NonPaged flag
    };
    SIZE_T SizeInBytes;
    union {
        UCHAR Tag[4];
        ULONG TagUlong;
    };
} SYSTEM_BIGPOOL_ENTRY, *PSYSTEM_BIGPOOL_ENTRY;

typedef struct _SYSTEM_BIGPOOL_INFORMATION {
    ULONG Count;
    SYSTEM_BIGPOOL_ENTRY AllocatedInfo[1];  // Variable length array
} SYSTEM_BIGPOOL_INFORMATION, *PSYSTEM_BIGPOOL_INFORMATION;

//=============================================================================
// Known Suspicious Pool Tags
//=============================================================================

// Pool tags that indicate VirtualBox/BYOVD presence
static const STEALTH_POOL_TAG g_SuspiciousTags[] = {
    { 0x54525049, "IPRT", "VirtualBox Internal Portable Runtime" },
    { 0x786F4256, "VBox", "VirtualBox generic" },
    { 0x4D4D6F56, "VoMM", "VirtualBox Memory Manager" },
    { 0x72447053, "SpDr", "SUPDrv generic" },
    { 0x674D7053, "SpMg", "SUP Memory Manager" },
    { 0x396C644C, "Ld9B", "LDPlayer variant" },
    { 0x39786F42, "Box9", "LDPlayer VBox" },
    { 0, NULL, NULL }  // Sentinel
};

//=============================================================================
// Helper Functions
//=============================================================================

static void TagToString(ULONG tag, char* buf, size_t len) {
    if (len < 5) return;
    buf[0] = (char)((tag >> 0) & 0xFF);
    buf[1] = (char)((tag >> 8) & 0xFF);
    buf[2] = (char)((tag >> 16) & 0xFF);
    buf[3] = (char)((tag >> 24) & 0xFF);
    buf[4] = '\0';
    for (int i = 0; i < 4; i++) {
        if (buf[i] < 32 || buf[i] > 126) buf[i] = '.';
    }
}

static bool IsSuspiciousTag(ULONG tag) {
    for (int i = 0; g_SuspiciousTags[i].Tag != 0; i++) {
        if (tag == g_SuspiciousTags[i].Tag) return true;
    }
    return false;
}

static const char* GetTagDescription(ULONG tag) {
    for (int i = 0; g_SuspiciousTags[i].Tag != 0; i++) {
        if (tag == g_SuspiciousTags[i].Tag) return g_SuspiciousTags[i].Description;
    }
    return NULL;
}

//=============================================================================
// BigPool Scanner Implementation
//=============================================================================

bool StealthScanBigPool(STEALTH_SCAN_RESULT* result, UINT32 filterFlags) {
    HMODULE ntdll;
    FN_NtQuerySystemInformation NtQuerySystemInformation;
    PSYSTEM_BIGPOOL_INFORMATION bigPoolInfo = NULL;
    ULONG bufferSize = 0x100000;  // Start with 1MB
    ULONG returnLength = 0;
    NTSTATUS status;
    LARGE_INTEGER startTime, endTime, freq;

    if (!result) return false;
    memset(result, 0, sizeof(*result));

    // Get NtQuerySystemInformation
    ntdll = GetModuleHandleW(L"ntdll.dll");
    if (!ntdll) {
        result->ErrorCode = GetLastError();
        return false;
    }

    NtQuerySystemInformation = (FN_NtQuerySystemInformation)
        GetProcAddress(ntdll, "NtQuerySystemInformation");
    if (!NtQuerySystemInformation) {
        result->ErrorCode = GetLastError();
        return false;
    }

    // Start timing
    QueryPerformanceCounter(&startTime);
    QueryPerformanceFrequency(&freq);

    // Allocate buffer and query BigPool
    while (TRUE) {
        bigPoolInfo = (PSYSTEM_BIGPOOL_INFORMATION)malloc(bufferSize);
        if (!bigPoolInfo) {
            result->ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            return false;
        }

        status = NtQuerySystemInformation(
            SystemBigPoolInformation,
            bigPoolInfo,
            bufferSize,
            &returnLength
        );

        if (status == 0) {  // STATUS_SUCCESS
            break;
        }

        if (status == 0xC0000004L) {  // STATUS_INFO_LENGTH_MISMATCH
            free(bigPoolInfo);
            bufferSize = returnLength + 0x10000;  // Add margin
            if (bufferSize > 0x10000000) {  // 256MB limit
                result->ErrorCode = ERROR_BUFFER_OVERFLOW;
                return false;
            }
            continue;
        }

        // Other error
        free(bigPoolInfo);
        result->ErrorCode = (DWORD)status;
        return false;
    }

    result->TotalEntries = bigPoolInfo->Count;
    result->QuerySucceeded = TRUE;

    // Scan entries
    for (ULONG i = 0; i < bigPoolInfo->Count && result->SuspiciousCount < MAX_SUSPICIOUS_ENTRIES; i++) {
        PSYSTEM_BIGPOOL_ENTRY entry = &bigPoolInfo->AllocatedInfo[i];
        ULONG tag = entry->TagUlong;
        ULONG_PTR va = (ULONG_PTR)entry->VirtualAddress & ~1ULL;  // Clear NonPaged bit

        // Check filters
        bool include = false;

        if ((filterFlags & SCAN_FILTER_IPRT) && tag == 0x54525049) include = true;
        if ((filterFlags & SCAN_FILTER_VBOX) && IsSuspiciousTag(tag)) include = true;
        if ((filterFlags & SCAN_FILTER_LARGE) && entry->SizeInBytes >= 0x100000) include = true;
        if (filterFlags & SCAN_FILTER_ALL) include = IsSuspiciousTag(tag);

        if (include) {
            STEALTH_SUSPICIOUS_ENTRY* se = &result->SuspiciousEntries[result->SuspiciousCount];
            se->VirtualAddress = va;
            se->Size = entry->SizeInBytes;
            se->Tag = tag;
            se->NonPaged = (entry->NonPaged & 1) != 0;
            TagToString(tag, se->TagString, sizeof(se->TagString));
            result->SuspiciousCount++;
        }

        // Count specific tags
        if (tag == 0x54525049) {
            result->IprtTagCount++;
            result->IprtTotalBytes += entry->SizeInBytes;
        }
        if (IsSuspiciousTag(tag)) {
            result->VboxTagCount++;
        }
    }

    // End timing
    QueryPerformanceCounter(&endTime);
    result->ScanDurationUs = (UINT64)(((endTime.QuadPart - startTime.QuadPart) * 1000000) / freq.QuadPart);

    free(bigPoolInfo);
    return true;
}

//=============================================================================
// Result Display
//=============================================================================

void StealthPrintResults(const STEALTH_SCAN_RESULT* result) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════════╗\n");
    printf("║                    STEALTH VERIFICATION SCAN                     ║\n");
    printf("╠══════════════════════════════════════════════════════════════════╣\n");

    if (!result->QuerySucceeded) {
        printf("║  [ERROR] BigPool query failed: 0x%08X                        ║\n", result->ErrorCode);
        printf("╚══════════════════════════════════════════════════════════════════╝\n");
        return;
    }

    printf("║  Total BigPool Entries: %-8u   Scan Time: %llu µs           ║\n",
           result->TotalEntries, (unsigned long long)result->ScanDurationUs);
    printf("╠══════════════════════════════════════════════════════════════════╣\n");

    // IPRT tag status - critical indicator
    if (result->IprtTagCount > 0) {
        printf("║  ⚠️  'IPRT' POOL TAG DETECTED - BYOVD VISIBLE                   ║\n");
        printf("║      Count: %-6u   Total Size: %-10llu bytes              ║\n",
               result->IprtTagCount, (unsigned long long)result->IprtTotalBytes);
    } else {
        printf("║  ✓  No 'IPRT' pool tags found - BYOVD may be unloaded          ║\n");
    }

    // VBox tags summary
    if (result->VboxTagCount > 0) {
        printf("║  ⚠️  VirtualBox-related tags: %-6u                            ║\n", result->VboxTagCount);
    } else {
        printf("║  ✓  No VirtualBox signatures in BigPool                        ║\n");
    }

    printf("╠══════════════════════════════════════════════════════════════════╣\n");

    // Detailed suspicious entries
    if (result->SuspiciousCount > 0) {
        printf("║  SUSPICIOUS ENTRIES:                                             ║\n");
        printf("╟──────────────────────────────────────────────────────────────────╢\n");
        printf("║  %-6s  %-18s  %-10s  %-20s    ║\n", "Tag", "Address", "Size", "Description");
        printf("╟──────────────────────────────────────────────────────────────────╢\n");

        for (UINT32 i = 0; i < result->SuspiciousCount && i < 10; i++) {
            const STEALTH_SUSPICIOUS_ENTRY* e = &result->SuspiciousEntries[i];
            const char* desc = GetTagDescription(e->Tag);
            printf("║  '%-4s'  0x%016llX  %-10llu  %-20s    ║\n",
                   e->TagString,
                   (unsigned long long)e->VirtualAddress,
                   (unsigned long long)e->Size,
                   desc ? desc : "Unknown");
        }
        if (result->SuspiciousCount > 10) {
            printf("║  ... and %u more entries                                       ║\n",
                   result->SuspiciousCount - 10);
        }
    } else {
        printf("║  ✓  No suspicious entries found                                 ║\n");
    }

    printf("╠══════════════════════════════════════════════════════════════════╣\n");

    // Verdict
    if (result->IprtTagCount == 0 && result->VboxTagCount == 0) {
        printf("║  VERDICT: ✓ STEALTH - No detection vectors in BigPool          ║\n");
    } else if (result->IprtTagCount > 0) {
        printf("║  VERDICT: ✗ DETECTED - 'IPRT' allocations visible              ║\n");
        printf("║  RECOMMENDATION: Unload BYOVD driver or use EPT hiding         ║\n");
    } else {
        printf("║  VERDICT: ⚠️ WARNING - VBox signatures present                 ║\n");
    }

    printf("╚══════════════════════════════════════════════════════════════════╝\n\n");
}

//=============================================================================
// Timing Measurement
//=============================================================================

bool StealthMeasureTimingWindow(STEALTH_TIMING_RESULT* result) {
    int cpuInfo[4];
    LARGE_INTEGER start, end, freq;
    UINT64 samples[TIMING_SAMPLES];

    if (!result) return false;
    memset(result, 0, sizeof(*result));

    QueryPerformanceFrequency(&freq);

    // Measure native CPUID timing (baseline)
    for (int i = 0; i < TIMING_SAMPLES; i++) {
        QueryPerformanceCounter(&start);
        __cpuidex(cpuInfo, 0, 0);
        __cpuidex(cpuInfo, 0, 0);
        __cpuidex(cpuInfo, 0, 0);
        __cpuidex(cpuInfo, 0, 0);
        __cpuidex(cpuInfo, 0, 0);
        QueryPerformanceCounter(&end);
        samples[i] = ((end.QuadPart - start.QuadPart) * 1000000000ULL) / freq.QuadPart / 5;  // ns per CPUID
    }

    // Calculate stats (simple - could use better statistics)
    UINT64 sum = 0, min = UINT64_MAX, max = 0;
    for (int i = 0; i < TIMING_SAMPLES; i++) {
        sum += samples[i];
        if (samples[i] < min) min = samples[i];
        if (samples[i] > max) max = samples[i];
    }
    result->CpuidNativeNs = sum / TIMING_SAMPLES;
    result->CpuidMinNs = min;
    result->CpuidMaxNs = max;

    // Measure RDTSC timing
    for (int i = 0; i < TIMING_SAMPLES; i++) {
        UINT64 tsc1 = __rdtsc();
        UINT64 tsc2 = __rdtsc();
        samples[i] = tsc2 - tsc1;
    }

    sum = 0; min = UINT64_MAX; max = 0;
    for (int i = 0; i < TIMING_SAMPLES; i++) {
        sum += samples[i];
        if (samples[i] < min) min = samples[i];
        if (samples[i] > max) max = samples[i];
    }
    result->RdtscNativeCycles = sum / TIMING_SAMPLES;
    result->RdtscMinCycles = min;
    result->RdtscMaxCycles = max;

    result->MeasurementsComplete = TRUE;
    return true;
}

void StealthPrintTimingResults(const STEALTH_TIMING_RESULT* result) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════════╗\n");
    printf("║                    TIMING BASELINE MEASUREMENT                   ║\n");
    printf("╠══════════════════════════════════════════════════════════════════╣\n");

    if (!result->MeasurementsComplete) {
        printf("║  [ERROR] Timing measurement failed                              ║\n");
        printf("╚══════════════════════════════════════════════════════════════════╝\n");
        return;
    }

    printf("║  CPUID Timing (native, no hypervisor):                           ║\n");
    printf("║      Average: %-6llu ns   Min: %-6llu ns   Max: %-6llu ns       ║\n",
           (unsigned long long)result->CpuidNativeNs,
           (unsigned long long)result->CpuidMinNs,
           (unsigned long long)result->CpuidMaxNs);
    printf("╟──────────────────────────────────────────────────────────────────╢\n");
    printf("║  RDTSC Timing (native, no hypervisor):                           ║\n");
    printf("║      Average: %-6llu cyc  Min: %-6llu cyc  Max: %-6llu cyc      ║\n",
           (unsigned long long)result->RdtscNativeCycles,
           (unsigned long long)result->RdtscMinCycles,
           (unsigned long long)result->RdtscMaxCycles);
    printf("╠══════════════════════════════════════════════════════════════════╣\n");

    // Detection thresholds (typical values)
    UINT64 cpuidThreshold = 500;   // >500ns typically indicates VM
    UINT64 rdtscThreshold = 200;   // >200 cycles typically indicates VM

    if (result->CpuidNativeNs < cpuidThreshold && result->RdtscNativeCycles < rdtscThreshold) {
        printf("║  BASELINE: ✓ Native timing looks clean                          ║\n");
        printf("║  These values will increase under hypervisor - compare after    ║\n");
    } else {
        printf("║  BASELINE: ⚠️ Timing suggests existing virtualization           ║\n");
        printf("║  Expected if running in VM or with other hypervisor            ║\n");
    }

    printf("╚══════════════════════════════════════════════════════════════════╝\n\n");
}

//=============================================================================
// Quick Stealth Check
//=============================================================================

STEALTH_STATUS StealthQuickCheck(void) {
    STEALTH_SCAN_RESULT scanResult;
    STEALTH_STATUS status = STEALTH_CLEAN;

    if (!StealthScanBigPool(&scanResult, SCAN_FILTER_ALL)) {
        return STEALTH_ERROR;
    }

    if (scanResult.IprtTagCount > 0) {
        status = STEALTH_IPRT_VISIBLE;
    } else if (scanResult.VboxTagCount > 0) {
        status = STEALTH_VBOX_VISIBLE;
    }

    return status;
}

const char* StealthStatusString(STEALTH_STATUS status) {
    switch (status) {
        case STEALTH_CLEAN:        return "CLEAN - No detection vectors";
        case STEALTH_IPRT_VISIBLE: return "DETECTED - 'IPRT' pool tag visible";
        case STEALTH_VBOX_VISIBLE: return "WARNING - VirtualBox signatures visible";
        case STEALTH_ERROR:        return "ERROR - Scan failed";
        default:                   return "UNKNOWN";
    }
}
