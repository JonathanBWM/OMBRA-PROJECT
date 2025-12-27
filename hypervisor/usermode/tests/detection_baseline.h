/**
 * @file detection_baseline.h
 * @brief Detection baseline capture system for OMBRA hypervisor
 *
 * Captures system state before/after hypervisor operations to measure
 * detection footprint. Tracks kernel pool allocations, VirtualBox artifacts,
 * driver loads, and ETW state changes.
 */

#ifndef DETECTION_BASELINE_H
#define DETECTION_BASELINE_H

#include <windows.h>
#include <stdint.h>

/**
 * Detection baseline structure
 *
 * Captures system metrics before and after hypervisor installation
 * to quantify detection surface. Score calculation:
 *   +20 per BigPool allocation delta
 *   +30 per VirtualBox tag detected
 *   +50 per driver count delta
 *
 * Score 0 = clean operation, no detectable changes
 * Score >100 = high detection risk
 */
typedef struct _DETECTION_BASELINE {
    // Initial capture (before hypervisor)
    struct {
        uint32_t BigPoolEntries;        // NonPagedPool allocations >4KB
        uint32_t VBoxTags;              // VirtualBox signature tags
        uint32_t DriverCount;           // Loaded kernel drivers
        uint32_t EtwState;              // ETW provider registration state
    } Initial;

    // Final capture (after hypervisor)
    struct {
        uint32_t BigPoolEntries;
        uint32_t VBoxTags;
        uint32_t DriverCount;
        uint32_t EtwState;
    } Final;

    // Computed deltas
    int32_t BigPoolDelta;               // Change in pool allocations
    int32_t VBoxTagDelta;               // Change in VBox signatures
    int32_t DriverDelta;                // Change in driver count

    // Overall detection risk score
    uint32_t DetectionScore;

} DETECTION_BASELINE;

/**
 * Capture system baseline metrics
 *
 * @param baseline Baseline structure to populate
 * @param initial TRUE = capture initial state, FALSE = capture final and compute deltas
 * @return TRUE on success, FALSE on failure
 *
 * When initial=FALSE, automatically computes deltas and detection score:
 *   score = (BigPoolDelta * 20) + (VBoxTagDelta * 30) + (DriverDelta * 50)
 */
BOOL CaptureBaseline(DETECTION_BASELINE* baseline, BOOL initial);

/**
 * Print baseline metrics to console
 *
 * @param baseline Baseline structure to print
 *
 * Output format:
 *   Initial metrics
 *   Final metrics (if captured)
 *   Deltas and detection score with risk assessment
 */
void PrintBaseline(const DETECTION_BASELINE* baseline);

/**
 * Count NonPagedPool allocations >4KB
 *
 * Uses NtQuerySystemInformation(SystemBigPoolInformation) to enumerate
 * large kernel pool allocations. These are visible to user-mode via
 * undocumented APIs commonly used by anti-cheat.
 *
 * @return Number of big pool entries, or 0 on failure
 */
uint32_t CountBigPoolEntries(void);

/**
 * Count VirtualBox signature tags in memory
 *
 * Scans for common VirtualBox pool tags and registry artifacts that
 * indicate virtualization. Many hypervisors leak VBox signatures due to
 * copy-paste code or shared libraries.
 *
 * Common tags: VBox, vbgl, VbglR0, VBoxGuest
 *
 * @return Number of VirtualBox tags found, or 0 if clean
 */
uint32_t CountVBoxTags(void);

/**
 * Count loaded kernel drivers
 *
 * Uses NtQuerySystemInformation(SystemModuleInformation) to enumerate
 * all loaded kernel modules. Driver count changes indicate kernel
 * modification activity.
 *
 * @return Number of loaded drivers, or 0 on failure
 */
uint32_t CountLoadedDrivers(void);

/**
 * Get ETW trace state for microsoft-windows-threat-intelligence provider
 *
 * Queries ETW provider registration to detect if kernel threat intelligence
 * monitoring is active. ETW state changes can indicate evasion attempts.
 *
 * @return Provider state flags, or 0 if not registered
 */
uint32_t GetEtwTiState(void);

#endif // DETECTION_BASELINE_H
