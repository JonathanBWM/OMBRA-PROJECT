/**
 * @file detection_baseline.c
 * @brief Detection baseline capture implementation
 */

#include "detection_baseline.h"
#include "../byovd/nt_defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// External NT functions from nt_helpers.c
extern PFN_NtQuerySystemInformation g_NtQuerySystemInformation;
extern bool NtInit(void);

// VirtualBox-related pool tags to detect
static const ULONG g_VBoxTags[] = {
    'xoBV',     // VBox (little-endian: 0x786F4256)
    'XoBV',     // VBoX
    'MMoV',     // VoMM - VBox memory manager
    'rDpS',     // SpDr - SUPDrv
    'gMpS',     // SpMg - SUP memory
    'B9dL',     // Ld9B - LDPlayer
    'xoBv',     // vBox variant
    'tsuG',     // Guest
    0           // Sentinel
};

/**
 * Capture system baseline metrics
 */
BOOL CaptureBaseline(DETECTION_BASELINE* baseline, BOOL initial)
{
    if (!baseline) {
        return FALSE;
    }

    if (initial) {
        // First capture - populate initial state
        baseline->Initial.BigPoolEntries = CountBigPoolEntries();
        baseline->Initial.VBoxTags = CountVBoxTags();
        baseline->Initial.DriverCount = CountLoadedDrivers();
        baseline->Initial.EtwState = GetEtwTiState();

        // Zero out final state and deltas
        baseline->Final.BigPoolEntries = 0;
        baseline->Final.VBoxTags = 0;
        baseline->Final.DriverCount = 0;
        baseline->Final.EtwState = 0;
        baseline->BigPoolDelta = 0;
        baseline->VBoxTagDelta = 0;
        baseline->DriverDelta = 0;
        baseline->DetectionScore = 0;

        printf("[+] Captured initial baseline:\n");
        printf("    BigPool entries: %u\n", baseline->Initial.BigPoolEntries);
        printf("    VBox tags: %u\n", baseline->Initial.VBoxTags);
        printf("    Loaded drivers: %u\n", baseline->Initial.DriverCount);
        printf("    ETW state: 0x%08X\n", baseline->Initial.EtwState);
    }
    else {
        // Second capture - populate final state and compute deltas
        baseline->Final.BigPoolEntries = CountBigPoolEntries();
        baseline->Final.VBoxTags = CountVBoxTags();
        baseline->Final.DriverCount = CountLoadedDrivers();
        baseline->Final.EtwState = GetEtwTiState();

        // Compute deltas (signed to detect increases/decreases)
        baseline->BigPoolDelta = (int32_t)baseline->Final.BigPoolEntries -
                                  (int32_t)baseline->Initial.BigPoolEntries;
        baseline->VBoxTagDelta = (int32_t)baseline->Final.VBoxTags -
                                  (int32_t)baseline->Initial.VBoxTags;
        baseline->DriverDelta = (int32_t)baseline->Final.DriverCount -
                                 (int32_t)baseline->Initial.DriverCount;

        // Calculate detection score
        // Only count positive deltas (increases are suspicious)
        uint32_t score = 0;
        if (baseline->BigPoolDelta > 0) {
            score += baseline->BigPoolDelta * 20;
        }
        if (baseline->VBoxTagDelta > 0) {
            score += baseline->VBoxTagDelta * 30;
        }
        if (baseline->DriverDelta > 0) {
            score += baseline->DriverDelta * 50;
        }

        baseline->DetectionScore = score;

        printf("[+] Captured final baseline:\n");
        printf("    BigPool entries: %u (delta: %+d)\n",
               baseline->Final.BigPoolEntries, baseline->BigPoolDelta);
        printf("    VBox tags: %u (delta: %+d)\n",
               baseline->Final.VBoxTags, baseline->VBoxTagDelta);
        printf("    Loaded drivers: %u (delta: %+d)\n",
               baseline->Final.DriverCount, baseline->DriverDelta);
        printf("    ETW state: 0x%08X\n", baseline->Final.EtwState);
        printf("    Detection score: %u\n", baseline->DetectionScore);
    }

    return TRUE;
}

/**
 * Print baseline metrics to console
 */
void PrintBaseline(const DETECTION_BASELINE* baseline)
{
    if (!baseline) {
        return;
    }

    printf("\n========== DETECTION BASELINE REPORT ==========\n\n");

    printf("INITIAL STATE:\n");
    printf("  BigPool entries:  %u\n", baseline->Initial.BigPoolEntries);
    printf("  VBox tags:        %u\n", baseline->Initial.VBoxTags);
    printf("  Loaded drivers:   %u\n", baseline->Initial.DriverCount);
    printf("  ETW state:        0x%08X\n", baseline->Initial.EtwState);

    // Only print final state if it was captured
    if (baseline->Final.BigPoolEntries != 0 ||
        baseline->Final.VBoxTags != 0 ||
        baseline->Final.DriverCount != 0 ||
        baseline->DetectionScore != 0) {

        printf("\nFINAL STATE:\n");
        printf("  BigPool entries:  %u\n", baseline->Final.BigPoolEntries);
        printf("  VBox tags:        %u\n", baseline->Final.VBoxTags);
        printf("  Loaded drivers:   %u\n", baseline->Final.DriverCount);
        printf("  ETW state:        0x%08X\n", baseline->Final.EtwState);

        printf("\nDELTAS:\n");
        printf("  BigPool delta:    %+d (weight: x20 = %+d)\n",
               baseline->BigPoolDelta,
               baseline->BigPoolDelta > 0 ? baseline->BigPoolDelta * 20 : 0);
        printf("  VBox tag delta:   %+d (weight: x30 = %+d)\n",
               baseline->VBoxTagDelta,
               baseline->VBoxTagDelta > 0 ? baseline->VBoxTagDelta * 30 : 0);
        printf("  Driver delta:     %+d (weight: x50 = %+d)\n",
               baseline->DriverDelta,
               baseline->DriverDelta > 0 ? baseline->DriverDelta * 50 : 0);

        printf("\nDETECTION SCORE: %u\n", baseline->DetectionScore);

        // Risk assessment
        if (baseline->DetectionScore == 0) {
            printf("\n[CLEAN] No detectable system changes. Operation is stealthy.\n");
        }
        else if (baseline->DetectionScore < 50) {
            printf("\n[LOW RISK] Minimal detection footprint. Acceptable for testing.\n");
        }
        else if (baseline->DetectionScore < 100) {
            printf("\n[MEDIUM RISK] Moderate detection footprint. May trigger basic checks.\n");
        }
        else if (baseline->DetectionScore < 200) {
            printf("\n[HIGH RISK] Significant detection footprint. Will likely trigger anti-cheat.\n");
        }
        else {
            printf("\n[CRITICAL] Massive detection footprint. Guaranteed detection by any competent AC.\n");
        }
    }
    else {
        printf("\n[INFO] Final baseline not yet captured.\n");
    }

    printf("\n===============================================\n\n");
}

/**
 * Helper: Query BigPool information from kernel
 * Returns allocated buffer (caller must free) or NULL on failure
 */
static PSYSTEM_BIGPOOL_INFORMATION QueryBigPoolInfo(void)
{
    if (!NtInit() || !g_NtQuerySystemInformation) {
        return NULL;
    }

    // Query required size
    ULONG cbNeeded = 0;
    NTSTATUS status = g_NtQuerySystemInformation(
        SystemBigPoolInformation,
        NULL,
        0,
        &cbNeeded
    );

    // STATUS_INFO_LENGTH_MISMATCH is expected on size query
    if (cbNeeded == 0) {
        // Try a reasonable default size
        cbNeeded = 1024 * 1024;  // 1MB should be plenty
    }

    // Allocate with extra margin
    ULONG cbBuffer = cbNeeded + 0x10000;
    PVOID pBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbBuffer);
    if (!pBuffer) {
        return NULL;
    }

    status = g_NtQuerySystemInformation(
        SystemBigPoolInformation,
        pBuffer,
        cbBuffer,
        &cbNeeded
    );

    if (!NT_SUCCESS(status)) {
        HeapFree(GetProcessHeap(), 0, pBuffer);
        return NULL;
    }

    return (PSYSTEM_BIGPOOL_INFORMATION)pBuffer;
}

/**
 * Count NonPagedPool allocations >4KB
 */
uint32_t CountBigPoolEntries(void)
{
    PSYSTEM_BIGPOOL_INFORMATION pInfo = QueryBigPoolInfo();
    if (!pInfo) {
        printf("[-] Failed to query BigPool information\n");
        return 0;
    }

    uint32_t count = pInfo->Count;
    HeapFree(GetProcessHeap(), 0, pInfo);

    return count;
}

/**
 * Helper: Check if a pool tag matches VBox signatures
 */
static bool IsVBoxPoolTag(ULONG tag)
{
    for (int i = 0; g_VBoxTags[i] != 0; i++) {
        if (tag == g_VBoxTags[i]) {
            return true;
        }
    }
    return false;
}

/**
 * Count VirtualBox signature tags in BigPool
 */
uint32_t CountVBoxTags(void)
{
    PSYSTEM_BIGPOOL_INFORMATION pInfo = QueryBigPoolInfo();
    if (!pInfo) {
        return 0;
    }

    uint32_t vboxCount = 0;

    for (ULONG i = 0; i < pInfo->Count; i++) {
        if (IsVBoxPoolTag(pInfo->AllocatedInfo[i].TagUlong)) {
            vboxCount++;
        }
    }

    HeapFree(GetProcessHeap(), 0, pInfo);
    return vboxCount;
}

/**
 * Count loaded kernel drivers
 */
uint32_t CountLoadedDrivers(void)
{
    if (!NtInit() || !g_NtQuerySystemInformation) {
        return 0;
    }

    // Query required size
    ULONG cbNeeded = 0;
    g_NtQuerySystemInformation(SystemModuleInformation, NULL, 0, &cbNeeded);
    if (cbNeeded == 0) {
        return 0;
    }

    // Allocate with margin
    PVOID pBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbNeeded + 0x1000);
    if (!pBuffer) {
        return 0;
    }

    NTSTATUS status = g_NtQuerySystemInformation(
        SystemModuleInformation,
        pBuffer,
        cbNeeded + 0x1000,
        &cbNeeded
    );

    if (!NT_SUCCESS(status)) {
        HeapFree(GetProcessHeap(), 0, pBuffer);
        return 0;
    }

    PRTL_PROCESS_MODULES pModules = (PRTL_PROCESS_MODULES)pBuffer;
    uint32_t count = pModules->NumberOfModules;

    HeapFree(GetProcessHeap(), 0, pBuffer);
    return count;
}

/**
 * Get ETW trace state for microsoft-windows-threat-intelligence provider
 * TODO: Implement using TraceQueryInformation or ETW enumeration
 */
uint32_t GetEtwTiState(void)
{
    // Stub implementation - returns 0 until ETW APIs are integrated
    // Real implementation will:
    //   1. Open ETW provider {6d1b249d-131b-468a-899b-fb0ad9551772} (Threat Intelligence)
    //   2. Query provider registration state
    //   3. Check if callbacks are enabled for:
    //      - Process creation (PsSetCreateProcessNotifyRoutine)
    //      - Thread creation (PsSetCreateThreadNotifyRoutine)
    //      - Image load (PsSetLoadImageNotifyRoutine)
    //   4. Return bitmask of enabled callbacks
    return 0;
}
