/**
 * @file stealth_scanner.h
 * @brief Stealth Verification Scanner - Header
 *
 * Provides comprehensive verification of hypervisor stealth profile:
 * - BigPool visibility scanning to detect 'IPRT' and VirtualBox pool tags
 * - Timing measurement to establish baseline before VMX activation
 * - Real-time detection status for pre/during/post hypervisor load
 *
 * Usage:
 *   STEALTH_SCAN_RESULT result;
 *   StealthScanBigPool(&result, SCAN_FILTER_ALL);
 *   StealthPrintResults(&result);
 *
 *   STEALTH_TIMING_RESULT timing;
 *   StealthMeasureTimingWindow(&timing);
 *   StealthPrintTimingResults(&timing);
 */

#ifndef STEALTH_SCANNER_H
#define STEALTH_SCANNER_H

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>
#include <intrin.h>

//=============================================================================
// Configuration Constants
//=============================================================================

// Maximum suspicious entries to track
#define MAX_SUSPICIOUS_ENTRIES      64

// Number of timing samples for baseline measurement
#define TIMING_SAMPLES              100

//=============================================================================
// Scan Filter Flags
//=============================================================================

// Filter for 'IPRT' pool tag specifically (VirtualBox Runtime)
#define SCAN_FILTER_IPRT            0x00000001

// Filter for all VirtualBox-related pool tags
#define SCAN_FILTER_VBOX            0x00000002

// Filter for large allocations (>1MB)
#define SCAN_FILTER_LARGE           0x00000004

// Include all suspicious tags (default behavior)
#define SCAN_FILTER_ALL             0x0000000F

//=============================================================================
// Known Pool Tag Structure
//=============================================================================

typedef struct _STEALTH_POOL_TAG {
    ULONG       Tag;            // Pool tag as DWORD (little-endian)
    const char* TagString;      // Human-readable tag
    const char* Description;    // What this tag indicates
} STEALTH_POOL_TAG;

//=============================================================================
// Suspicious Entry Structure
//=============================================================================

typedef struct _STEALTH_SUSPICIOUS_ENTRY {
    ULONG_PTR   VirtualAddress; // Kernel virtual address of allocation
    SIZE_T      Size;           // Allocation size in bytes
    ULONG       Tag;            // Pool tag as DWORD
    char        TagString[8];   // Printable tag string
    BOOL        NonPaged;       // TRUE if NonPagedPool
} STEALTH_SUSPICIOUS_ENTRY;

//=============================================================================
// BigPool Scan Result Structure
//=============================================================================

typedef struct _STEALTH_SCAN_RESULT {
    // Query status
    BOOL        QuerySucceeded;     // TRUE if NtQuerySystemInformation succeeded
    DWORD       ErrorCode;          // Last error code if failed

    // Pool statistics
    UINT32      TotalEntries;       // Total BigPool entries scanned
    UINT32      IprtTagCount;       // Number of 'IPRT' tagged allocations
    UINT64      IprtTotalBytes;     // Total bytes in IPRT allocations
    UINT32      VboxTagCount;       // Number of VBox-related tags
    UINT32      SuspiciousCount;    // Number of suspicious entries found

    // Timing
    UINT64      ScanDurationUs;     // Scan time in microseconds

    // Detailed suspicious entries
    STEALTH_SUSPICIOUS_ENTRY SuspiciousEntries[MAX_SUSPICIOUS_ENTRIES];
} STEALTH_SCAN_RESULT;

//=============================================================================
// Timing Measurement Result Structure
//=============================================================================

typedef struct _STEALTH_TIMING_RESULT {
    // CPUID timing (nanoseconds)
    UINT64      CpuidNativeNs;      // Average native CPUID time
    UINT64      CpuidMinNs;         // Minimum observed
    UINT64      CpuidMaxNs;         // Maximum observed

    // RDTSC timing (cycles)
    UINT64      RdtscNativeCycles;  // Average native RDTSC overhead
    UINT64      RdtscMinCycles;     // Minimum observed
    UINT64      RdtscMaxCycles;     // Maximum observed

    // Status
    BOOL        MeasurementsComplete; // TRUE if measurement succeeded
} STEALTH_TIMING_RESULT;

//=============================================================================
// Quick Check Status Enum
//=============================================================================

typedef enum _STEALTH_STATUS {
    STEALTH_CLEAN        = 0,       // No detection vectors found
    STEALTH_IPRT_VISIBLE = 1,       // 'IPRT' pool tag detected (BYOVD visible)
    STEALTH_VBOX_VISIBLE = 2,       // VirtualBox signatures detected
    STEALTH_ERROR        = 3        // Scan failed
} STEALTH_STATUS;

//=============================================================================
// Function Declarations
//=============================================================================

/**
 * @brief Scan BigPool for suspicious pool tags
 *
 * Uses NtQuerySystemInformation(SystemBigPoolInformation) to enumerate
 * all kernel BigPool allocations and check for detection indicators.
 *
 * @param result    Output structure for scan results
 * @param filterFlags   SCAN_FILTER_* flags to control what's reported
 * @return          TRUE on success, FALSE on query failure
 *
 * @note This function allocates memory dynamically and may block for
 *       significant time on systems with large pool usage.
 */
bool StealthScanBigPool(STEALTH_SCAN_RESULT* result, UINT32 filterFlags);

/**
 * @brief Print formatted scan results to console
 *
 * Displays a formatted table with detection status, pool tag counts,
 * and detailed suspicious entries with actionable recommendations.
 *
 * @param result    Scan results from StealthScanBigPool()
 */
void StealthPrintResults(const STEALTH_SCAN_RESULT* result);

/**
 * @brief Measure native CPUID and RDTSC timing baseline
 *
 * Collects timing samples before VMX activation to establish baseline.
 * After hypervisor load, compare new measurements to detect overhead.
 *
 * @param result    Output structure for timing results
 * @return          TRUE on success, FALSE on measurement failure
 *
 * @note Run this BEFORE loading hypervisor to establish baseline.
 *       Run again AFTER to measure VM-exit induced overhead.
 */
bool StealthMeasureTimingWindow(STEALTH_TIMING_RESULT* result);

/**
 * @brief Print formatted timing results to console
 *
 * Displays timing baseline with detection threshold analysis.
 *
 * @param result    Timing results from StealthMeasureTimingWindow()
 */
void StealthPrintTimingResults(const STEALTH_TIMING_RESULT* result);

/**
 * @brief Quick stealth status check
 *
 * Performs fast BigPool scan and returns simple status enum.
 * Use for real-time status indicators.
 *
 * @return          STEALTH_STATUS indicating detection state
 */
STEALTH_STATUS StealthQuickCheck(void);

/**
 * @brief Get human-readable status string
 *
 * @param status    Status enum value
 * @return          Static string describing the status
 */
const char* StealthStatusString(STEALTH_STATUS status);

#endif // STEALTH_SCANNER_H
