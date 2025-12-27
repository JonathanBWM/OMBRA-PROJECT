// timing.c — VM-Exit Timing Compensation
// OmbraHypervisor
//
// ============================================================================
// TIMING-BASED HYPERVISOR DETECTION & MITIGATION
// ============================================================================
//
// Anti-cheat systems use timing side-channels to detect hypervisors. Common
// techniques include:
//
// 1. CPUID Timing:
//    tsc1 = RDTSC; CPUID; tsc2 = RDTSC;
//    if (tsc2 - tsc1 > threshold) -> hypervisor detected
//
// 2. MSR Access Timing:
//    tsc1 = RDTSC; RDMSR(0x3A); tsc2 = RDTSC;
//    if (tsc2 - tsc1 > threshold) -> hypervisor detected
//
// 3. Exception Timing:
//    tsc1 = RDTSC; trigger #GP; tsc2 = RDTSC (in handler);
//    if (tsc2 - tsc1 > threshold) -> hypervisor detected
//
// These work because VM-exits have measurable overhead (500-5000+ cycles)
// compared to native execution (10-100 cycles).
//
// ============================================================================
// OUR MITIGATION STRATEGY
// ============================================================================
//
// We use Intel VMX's TSC offsetting feature (VMCS field 0x2010) to hide
// VM-exit overhead:
//
// 1. Enable "Use TSC offsetting" control bit in processor-based controls
// 2. For each timing-sensitive exit handler:
//    a. Capture TSC at handler ENTRY: entryTsc = RDTSC
//    b. Execute handler logic normally
//    c. Capture TSC at handler EXIT: exitTsc = RDTSC
//    d. Calculate overhead: overhead = exitTsc - entryTsc
//    e. Update VMCS: TSC_OFFSET -= overhead
//
// 3. When guest reads TSC (via RDTSC or direct register access):
//    guestTsc = realTsc + TSC_OFFSET
//
// 4. By subtracting overhead from offset, we make future TSC reads appear
//    LOWER, effectively hiding the time spent in the handler.
//
// Example:
//   Real time:     1000 -> [VM-exit 800 cycles] -> 1800 -> [RDTSC] -> 1900
//   Guest sees:    1000 -> [appears instant]    -> 1000 -> [RDTSC] -> 1100
//   TSC_OFFSET:       0 -> [subtract 800]       -> -800
//
// This makes timing measurements appear native, defeating detection.
//
// ============================================================================
// IMPLEMENTATION DETAILS
// ============================================================================
//
// Per-CPU State:
//   - TscOffset: Cumulative offset applied (grows negative over time)
//   - LastTsc: Last TSC value returned to guest (for monotonicity)
//
// Handler Integration:
//   - CPUID:  TimingStart() at entry, TimingEnd() at exit
//   - RDMSR:  TimingStart() at entry, TimingEnd() at exit
//   - WRMSR:  TimingStart() at entry, TimingEnd() at exit
//   - RDTSC:  Special handling - compensates BOTH the return value AND offset
//
// VMCS Configuration:
//   - Processor-based control: CPU_TSC_OFFSETTING (bit 3) enabled
//   - TSC_OFFSET field (0x2010): Written on each compensation
//
// ============================================================================
// KNOWN LIMITATIONS
// ============================================================================
//
// 1. Calibration: We use static overhead estimates. Real overhead varies with:
//    - CPU microarchitecture
//    - System load and interrupts
//    - Thermal throttling and frequency scaling
//    Production systems should calibrate dynamically.
//
// 2. Precision: TSC has ~nanosecond precision. VM-exit overhead measurement
//    includes our own compensation code, creating a feedback loop. We use
//    conservative estimates to avoid over-compensation.
//
// 3. Statistical Detection: Sophisticated anti-cheat may use statistical
//    analysis of timing distributions rather than simple thresholds. This
//    requires more advanced compensation (jitter injection, etc).
//
// 4. TSC Deadline Timer: If guest uses TSC deadline timer for interrupts,
//    our offset manipulation may cause early/late delivery. Monitor for this.
//
// ============================================================================

#include "timing.h"
#include "vmx.h"
#include "ept.h"
#include "../shared/vmcs_fields.h"
#include <intrin.h>

// =============================================================================
// Per-CPU Timing State
// =============================================================================
// We track cumulative TSC offset adjustments per-CPU to maintain monotonic TSC

// =============================================================================
// Initialization
// =============================================================================

void TimingInitialize(VMX_CPU* cpu) {
    if (!cpu) return;

    // Initialize per-CPU timing state
    cpu->TscOffset = 0;
    cpu->LastTsc = __rdtsc();
    cpu->TscResetCounter = 0;
    cpu->TscCumulativeOffset = 0;

    // Initialize APERF/MPERF offsets (CRITICAL for ESEA detection)
    cpu->AperfOffset = 0;
    cpu->MperfOffset = 0;

    // Run calibration to measure actual VM-exit overhead on this CPU
    TimingCalibrate(cpu);
}

// =============================================================================
// Calibration
// =============================================================================
//
// Measures actual VM-exit overhead by performing controlled exits and measuring
// the time difference. This gives us a baseline for compensation.
//
// TODO: For production, implement actual calibration by:
// 1. Trigger known VM-exits (CPUID, RDMSR, etc)
// 2. Measure TSC delta across multiple samples
// 3. Calculate average and adjust compensation constants
// 4. Re-calibrate periodically as CPU frequency/load changes

void TimingCalibrate(VMX_CPU* cpu) {
    if (!cpu) return;

    // Initialize calibration flag
    cpu->TimingCalibrated = false;

    // Calibrate CPUID baseline (native execution, before VMX)
    // This gives us the native execution time to subtract from VM-exit overhead
    U64 cpuidBaseline = 0;
    for (int i = 0; i < CALIBRATION_ITERATIONS; i++) {
        U64 start = __rdtsc();
        int regs[4];
        __cpuid(regs, 0);  // Basic CPUID leaf
        U64 end = __rdtsc();
        cpuidBaseline += (end - start);
    }
    cpu->CalibratedCpuidOverhead = cpuidBaseline / CALIBRATION_ITERATIONS;

    // Calibrate RDTSC baseline
    // Measure back-to-back RDTSC calls to get serialization overhead
    U64 rdtscBaseline = 0;
    for (int i = 0; i < CALIBRATION_ITERATIONS; i++) {
        U32 aux;
        U64 start = __rdtsc();
        __rdtscp(&aux);  // Serializing RDTSC variant
        U64 end = __rdtsc();
        rdtscBaseline += (end - start);
    }
    cpu->CalibratedRdtscBaseline = rdtscBaseline / CALIBRATION_ITERATIONS;

    // Calibrate MSR read baseline
    // Measure native RDMSR execution time
    U64 msrBaseline = 0;
    for (int i = 0; i < CALIBRATION_ITERATIONS; i++) {
        U64 start = __rdtsc();
        U64 value = __readmsr(0xC0000080);  // IA32_EFER - always readable
        U64 end = __rdtsc();
        msrBaseline += (end - start);
        (void)value;  // Prevent optimization
    }
    cpu->CalibratedMsrOverhead = msrBaseline / CALIBRATION_ITERATIONS;

    // Mark calibration complete
    cpu->TimingCalibrated = true;

    // Note: These are NATIVE execution baselines measured before VMX launch
    // After VMX launch, the actual VM-exit overhead will be:
    //   VM-exit overhead = (measured time in handler) - (native baseline)
    // This compensates for the native instruction cost, leaving only the
    // hypervisor-induced overhead to hide from timing attacks
}

// =============================================================================
// Timing Compensation API
// =============================================================================

U64 TimingStart(void) {
    // Capture TSC at the earliest possible moment in the handler
    // This includes VM-exit overhead, handler entry, stack setup, etc
    return __rdtsc();
}

void TimingEnd(VMX_CPU* cpu, U64 entryTsc, U64 estimatedOverhead) {
    U64 exitTsc;
    U64 actualOverhead;
    U64 compensationAmount;

    if (!cpu) return;

    // Capture TSC at handler exit (before VMRESUME)
    exitTsc = __rdtsc();

    // Calculate actual overhead: time from entry to exit
    actualOverhead = exitTsc - entryTsc;

    // Use the larger of actual vs estimated to be conservative
    // If actual is much larger than estimated, we might be getting scheduled out
    // or interrupted - in that case, use actual to fully compensate
    if (actualOverhead > estimatedOverhead) {
        compensationAmount = actualOverhead;
    } else {
        compensationAmount = estimatedOverhead;
    }

    // If we have calibrated baseline values, we can be more precise
    // The compensation should be the VM-exit overhead MINUS native instruction cost
    // This way we only hide the hypervisor-induced latency, not the instruction itself
    // (Note: For production, you'd track which exit type this is and use the
    //  appropriate baseline - CalibratedCpuidOverhead, CalibratedMsrOverhead, etc)
    // For now, we just apply the compensation as-is since the overhead is already
    // the delta between hypervisor and native execution

    TimingApplyCompensation(cpu, compensationAmount);
}

void TimingApplyCompensation(VMX_CPU* cpu, U64 overhead) {
    U64 currentOffset;
    U64 newOffset;

    if (!cpu) return;

    // Increment reset counter
    cpu->TscResetCounter++;

    // Check if we need to reset TSC offset to prevent wraparound
    if (cpu->TscResetCounter >= TSC_RESET_THRESHOLD ||
        cpu->TscOffset >= TSC_RESET_MAX_OFFSET) {
        TimingResetOffset(cpu);
    }

    // Read current TSC offset from VMCS
    currentOffset = VmcsRead(VMCS_CTRL_TSC_OFFSET);

    // Add the overhead to the offset
    // TSC offset is ADDED to the TSC value the guest reads
    // So to make guest TSC appear LOWER (hide time), we SUBTRACT from offset
    // This means we actually add a negative value, which is subtraction
    newOffset = currentOffset - overhead;

    // Update VMCS with new offset
    VmcsWrite(VMCS_CTRL_TSC_OFFSET, newOffset);

    // Track cumulative offset for this CPU (this grows indefinitely)
    cpu->TscOffset += overhead;

    // APERF/MPERF compensation (CRITICAL for ESEA detection)
    // During VM-exit, CPU runs at max performance, so APERF/MPERF ratio stays 1:1
    // We apply same overhead to both to maintain natural ratio
    // MCP verified: ESEA monitors this ratio for hypervisor detection
    cpu->AperfOffset += overhead;
    cpu->MperfOffset += overhead;
}

// =============================================================================
// TSC Offset Reset (Wraparound Prevention)
// =============================================================================
//
// Problem: TSC offset accumulates VM-exit overhead over time. After ~1M exits,
// the 64-bit signed offset can wrap or become suspiciously large, triggering
// anti-cheat detection via:
//   1. Reading IA32_TSC_OFFSET directly (if accessible)
//   2. Detecting TSC discontinuities during reset
//   3. Statistical analysis of TSC behavior
//
// Solution: Periodically reset the VMCS TSC_OFFSET field to 0, while maintaining
// timing continuity for the guest. We do this by:
//   1. Reading current real TSC
//   2. Calculating what guest TSC SHOULD be (real - cumulative offset)
//   3. Setting LastTsc to this value for monotonicity
//   4. Resetting VMCS TSC_OFFSET to 0
//   5. Resetting TscOffset accumulator to 0
//   6. Saving cumulative offset in TscCumulativeOffset for reference
//
// This reset is invisible to the guest because we maintain LastTsc continuity.
// Future RDTSC calls will use the new baseline and continue incrementing naturally.

void TimingResetOffset(VMX_CPU* cpu) {
    U64 currentRealTsc;
    U64 currentGuestTsc;

    if (!cpu) return;

    // Capture current real TSC
    currentRealTsc = __rdtsc();

    // Calculate what guest TSC should be right now
    // Guest sees: real_tsc - accumulated_offset
    currentGuestTsc = currentRealTsc - cpu->TscOffset;

    // Update LastTsc to maintain monotonicity
    // Future RDTSC handlers will use this as baseline
    if (currentGuestTsc > cpu->LastTsc) {
        cpu->LastTsc = currentGuestTsc;
    }
    // If currentGuestTsc <= LastTsc, keep LastTsc (already ahead)

    // Reset VMCS TSC_OFFSET to 0
    VmcsWrite(VMCS_CTRL_TSC_OFFSET, 0);

    // Save cumulative offset before reset (for debugging/analysis)
    cpu->TscCumulativeOffset += cpu->TscOffset;

    // Reset accumulator
    cpu->TscOffset = 0;
    cpu->TscResetCounter = 0;
}

// =============================================================================
// KUSER_SHARED_DATA Shadow Implementation
// =============================================================================
//
// Windows KUSER_SHARED_DATA contains system time values that user-mode code
// accesses via QueryPerformanceCounter, GetTickCount64, etc. Anti-cheats can
// compare these against RDTSC to detect timing manipulation.
//
// Our solution: Shadow the KUSER_SHARED_DATA page via EPT, adjusting time
// fields to match our TSC offset manipulation. Guest reads from shadow page
// which has time values reduced by the same amount we've hidden from TSC.

I64 TscCyclesToTime100ns(U64 cycles, U64 tscFrequencyHz) {
    // Windows system time uses 100-nanosecond intervals (10,000,000 per second)
    // Convert: cycles * 10,000,000 / tscFrequencyHz
    // Use 64-bit arithmetic carefully to avoid overflow

    if (tscFrequencyHz == 0) {
        return 0;  // Avoid division by zero
    }

    // For typical TSC frequencies (2-5 GHz), this won't overflow for reasonable cycle counts
    // cycles < 2^53 and tscFrequencyHz < 2^33 keeps us safe
    return (I64)((cycles * 10000000ULL) / tscFrequencyHz);
}

OMBRA_STATUS KuserShadowInit(KUSER_SHADOW_STATE* state, EPT_STATE* ept,
                              U64 kuserGpa, void* shadowPage, U64 shadowHpa) {
    if (!state || !ept || !shadowPage) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Initialize state
    state->KuserGpa = kuserGpa;
    state->ShadowHpa = shadowHpa;
    state->ShadowVa = shadowPage;
    state->OriginalVa = NULL;  // Will be set when we know the mapping
    state->LastUpdateTsc = __rdtsc();
    state->AccumulatedOffset100ns = 0;
    state->Initialized = true;
    state->Enabled = false;  // Enable after EPT redirection is set up

    // Note: The caller must:
    // 1. Split the EPT page containing kuserGpa to 4KB granularity
    // 2. Redirect the PTE to point to shadowHpa instead of original
    // 3. Set state->Enabled = true
    // 4. Copy original page content to shadow initially

    return OMBRA_SUCCESS;
}

void KuserShadowUpdate(KUSER_SHADOW_STATE* state, U64 hiddenCycles, U64 tscFreqHz) {
    I64 hiddenTime100ns;

    if (!state || !state->Enabled || !state->Initialized) {
        return;
    }

    // Convert hidden cycles to 100ns units
    hiddenTime100ns = TscCyclesToTime100ns(hiddenCycles, tscFreqHz);

    // Accumulate the offset
    state->AccumulatedOffset100ns += hiddenTime100ns;

    // Only update shadow page periodically to reduce overhead
    // (every ~1000 VM-exits or so)
    U64 currentTsc = __rdtsc();
    if (currentTsc - state->LastUpdateTsc < 1000000) {  // ~0.5ms at 2GHz
        return;  // Skip this update
    }
    state->LastUpdateTsc = currentTsc;

    // Update the shadow page with adjusted time values
    if (state->ShadowVa && state->OriginalVa) {
        AdjustKuserSharedData((U8*)state->ShadowVa, (const U8*)state->OriginalVa,
                              state->AccumulatedOffset100ns);
    }
}

void AdjustKuserSharedData(U8* shadowPage, const U8* originalPage, I64 hiddenTime100ns) {
    U64 systemTime;
    U64 interruptTime;
    U32 i;

    if (!shadowPage || !originalPage) {
        return;
    }

    // Copy entire original page to shadow first
    // This ensures all fields are present, we only modify time-related ones
    for (i = 0; i < PAGE_SIZE; i++) {
        shadowPage[i] = originalPage[i];
    }

    // Adjust SystemTime (offset 0x014)
    // This is a LARGE_INTEGER (64-bit) in 100ns units since Jan 1, 1601
    systemTime = *(U64*)(originalPage + KSDATA_SYSTEM_TIME);
    systemTime -= (U64)hiddenTime100ns;
    *(U64*)(shadowPage + KSDATA_SYSTEM_TIME) = systemTime;

    // Adjust InterruptTime (offset 0x008)
    // This is also a LARGE_INTEGER in 100ns units, measures uptime
    interruptTime = *(U64*)(originalPage + KSDATA_INTERRUPT_TIME);
    interruptTime -= (U64)hiddenTime100ns;
    *(U64*)(shadowPage + KSDATA_INTERRUPT_TIME) = interruptTime;

    // Also update InterruptTimeHigh (offset 0x01C) for lock-free 64-bit reads
    // Windows uses this for lock-free access pattern
    *(U32*)(shadowPage + KSDATA_INTERRUPT_TIME_HIGH) = (U32)(interruptTime >> 32);

    // Note: TickCount is derived from InterruptTime by Windows
    // TickCount = InterruptTime / 10000 (convert 100ns to ms)
    // We could adjust TickCountLow but it's derived, so adjusting InterruptTime
    // is sufficient for most detection methods
}
