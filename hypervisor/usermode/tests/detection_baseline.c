/**
 * @file detection_baseline.c
 * @brief Detection baseline capture implementation
 */

#include "detection_baseline.h"
#include <stdio.h>
#include <stdlib.h>

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
 * Count NonPagedPool allocations >4KB
 * TODO: Implement using NtQuerySystemInformation(SystemBigPoolInformation)
 */
uint32_t CountBigPoolEntries(void)
{
    // Stub implementation - returns 0 until NtQuerySystemInformation is integrated
    // Real implementation will:
    //   1. Call NtQuerySystemInformation with SystemBigPoolInformation (0x42)
    //   2. Enumerate SYSTEM_BIGPOOL_ENTRY structures
    //   3. Count entries with Size >= 4096 and Type == NonPagedPool
    //   4. Filter out known Windows allocations to reduce noise
    return 0;
}

/**
 * Count VirtualBox signature tags in memory
 * TODO: Implement pool tag scanning or registry enumeration
 */
uint32_t CountVBoxTags(void)
{
    // Stub implementation - returns 0 until scanning is implemented
    // Real implementation will:
    //   1. Scan SystemBigPoolInformation for VBox-related tags:
    //      - 'VBox', 'vbgl', 'VbglR0', 'VBoxGuest', 'VBoxSF'
    //   2. Check registry keys:
    //      - HKLM\SYSTEM\CurrentControlSet\Services\VBoxGuest
    //      - HKLM\HARDWARE\ACPI\DSDT\VBOX__
    //   3. Scan loaded modules for vbox*.sys
    //   4. Return total count of artifacts found
    return 0;
}

/**
 * Count loaded kernel drivers
 * TODO: Implement using NtQuerySystemInformation(SystemModuleInformation)
 */
uint32_t CountLoadedDrivers(void)
{
    // Stub implementation - returns 0 until NtQuerySystemInformation is integrated
    // Real implementation will:
    //   1. Call NtQuerySystemInformation with SystemModuleInformation (0x0B)
    //   2. Enumerate RTL_PROCESS_MODULES structure
    //   3. Count total number of modules in kernel space
    //   4. Return module count
    return 0;
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
