// diagnostics.c - Diagnostics and Status Reporting
// OmbraDriver Phase 3
//
// Provides driver health checking, statistics, and detection scoring

#include "context.h"
#include "command_ring.h"
#include "vmcall.h"

// =============================================================================
// Detection Score Tracking
// =============================================================================

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

static volatile U32 g_DetectionScore = 0;
static volatile U32 g_EventMask = 0xFFFF;  // Which events to track
static volatile U32 g_Threshold = 100;      // Alarm threshold

// =============================================================================
// TSC Helpers
// =============================================================================

static U64 ReadTsc(void) {
    U64 tsc;
#ifdef _MSC_VER
    __asm {
        rdtsc
        shl rdx, 32
        or rax, rdx
        mov tsc, rax
    }
#else
    __asm__ volatile ("rdtsc; shlq $32, %%rdx; orq %%rdx, %%rax" : "=a"(tsc) :: "rdx");
#endif
    return tsc;
}

// =============================================================================
// Score Management
// =============================================================================

void DiagnosticsRecordEvent(DETECTION_EVENT event) {
    if (event & g_EventMask) {
        // Atomic increment (simplified - real impl needs proper atomics)
#ifdef _MSC_VER
        _InterlockedAdd((volatile long*)&g_DetectionScore, 1);
#else
        __sync_fetch_and_add(&g_DetectionScore, 1);
#endif
    }
}

U32 DiagnosticsGetScore(void) {
    return g_DetectionScore;
}

void DiagnosticsResetScore(void) {
    g_DetectionScore = 0;
}

void DiagnosticsConfigScore(U32 operation, U32 eventMask, U32 threshold) {
    switch (operation) {
        case 0:  // Set event mask
            g_EventMask = eventMask;
            break;
        case 1:  // Set threshold
            g_Threshold = threshold;
            break;
        case 2:  // Reset all
            g_DetectionScore = 0;
            g_EventMask = 0xFFFF;
            g_Threshold = 100;
            break;
    }
}

bool DiagnosticsIsAboveThreshold(void) {
    return g_DetectionScore >= g_Threshold;
}

// =============================================================================
// Command Handlers
// =============================================================================

I32 HandlePing(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)cmd;

    // Return "PONG" magic
    resp->Ping.Response = 0x474E4F50;  // 'PONG' in little-endian
    return OMBRA_STATUS_SUCCESS;
}

I32 HandleGetStatus(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)cmd;
    POMBRA_DRIVER_CTX ctx = &g_DriverCtx;

    resp->StatusInfo.DriverVersion = 0x00030000;  // v3.0.0
    resp->StatusInfo.CommandsProcessed = ctx->CommandsProcessed;
    resp->StatusInfo.VmexitCount = ctx->VmexitCount;
    resp->StatusInfo.ActiveCpus = 1;  // TODO: query from hypervisor

    // Count installed hooks (from shadow.c)
    extern U32 ShadowGetCount(void);
    resp->StatusInfo.HooksInstalled = ShadowGetCount();

    // Calculate uptime from TSC (approximate - doesn't account for frequency)
    U64 currentTsc = ReadTsc();
    resp->StatusInfo.Uptime = currentTsc - ctx->StartTime;

    return OMBRA_STATUS_SUCCESS;
}

I32 HandleGetScore(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)cmd;

    // Return score in first 4 bytes of Raw
    *(U32*)resp->Raw = g_DetectionScore;
    return OMBRA_STATUS_SUCCESS;
}

I32 HandleResetScore(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)cmd;
    (void)resp;

    DiagnosticsResetScore();
    return OMBRA_STATUS_SUCCESS;
}

I32 HandleConfigScore(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)resp;

    DiagnosticsConfigScore(
        cmd->Score.Operation,
        cmd->Score.EventMask,
        cmd->Score.Threshold
    );

    return OMBRA_STATUS_SUCCESS;
}

I32 HandleShutdown(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)cmd;
    (void)resp;

    // Signal shutdown
    g_DriverCtx.ShutdownRequested = true;

    // Give a brief moment for response to be written
    // (worker thread will exit after this command)

    return OMBRA_STATUS_SHUTTING_DOWN;
}

// =============================================================================
// Self-Test Functions
// =============================================================================

// Verify hypervisor is responding
bool DiagnosticsSelfTest(void) {
    // Test VMCALL interface
    I64 status = VmCall(VMCALL_DRIVER_READY, g_DriverCtx.OwnerCr3, 0, 0);
    if (status < 0) return false;

    // Test memory operations
    U64 testPhys = 0;
    status = VmAllocPhysicalPage(&testPhys);
    if (status == VMCALL_STATUS_SUCCESS && testPhys != 0) {
        VmFreePhysicalPage(testPhys);
    }

    return true;
}

// Get hypervisor statistics
void DiagnosticsGetHypervisorStats(U64* vmexitCount, U64* eptViolations) {
    // These would come from hypervisor via VMCALL
    // For now, return placeholder values
    if (vmexitCount) *vmexitCount = g_DriverCtx.VmexitCount;
    if (eptViolations) *eptViolations = 0;  // TODO: track EPT violations
}
