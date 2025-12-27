// diagnostics.h - Driver Diagnostics and Detection Scoring
// OmbraDriver Phase 3
//
// Provides health monitoring, statistics, and detection avoidance tracking

#ifndef OMBRA_DIAGNOSTICS_H
#define OMBRA_DIAGNOSTICS_H

#include "../shared/types.h"

// Detection events that increase score
typedef enum _DETECTION_EVENT {
    DET_TIMING_ANOMALY       = 0x0001,  // RDTSC timing check failed
    DET_CPUID_ANOMALY        = 0x0002,  // CPUID inconsistency detected
    DET_MSR_ACCESS           = 0x0004,  // Suspicious MSR read detected
    DET_MEMORY_SCAN          = 0x0008,  // Memory scan detected hooks
    DET_DRIVER_ENUM          = 0x0010,  // Driver enumeration detected us
    DET_HANDLE_ENUM          = 0x0020,  // Handle enumeration detected
    DET_THREAD_ENUM          = 0x0040,  // Thread enumeration detected
    DET_MODULE_ENUM          = 0x0080,  // Module enumeration detected
    DET_DEBUG_CHECK          = 0x0100,  // Debug detection triggered
    DET_INTEGRITY_CHECK      = 0x0200,  // Integrity check detected modification
} DETECTION_EVENT;

// Record a detection event (increments score)
void DiagnosticsRecordEvent(DETECTION_EVENT event);

// Get current detection score
U32 DiagnosticsGetScore(void);

// Reset detection score to zero
void DiagnosticsResetScore(void);

// Configure scoring behavior
// operation: 0 = set event mask, 1 = set threshold, 2 = reset all
void DiagnosticsConfigScore(U32 operation, U32 eventMask, U32 threshold);

// Check if score exceeds threshold
bool DiagnosticsIsAboveThreshold(void);

// Run hypervisor self-test
bool DiagnosticsSelfTest(void);

// Get hypervisor statistics
void DiagnosticsGetHypervisorStats(U64* vmexitCount, U64* eptViolations);

// Command handlers
I32 HandlePing(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleGetStatus(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleGetScore(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleResetScore(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleConfigScore(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleShutdown(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);

#endif // OMBRA_DIAGNOSTICS_H
