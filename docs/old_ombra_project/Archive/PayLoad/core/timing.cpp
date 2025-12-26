// PayLoad/core/timing.cpp
// TSC offset compensation implementation
//
// Anti-detection technique for defeating BattlEye's RDTSC timing checks
// and ESEA's APERF/MPERF ratio analysis.

#include "../include/timing.h"
#include <intrin.h>

//=============================================================================
// VMCS field for TSC offset (Intel)
// From OmbraShared/Arch/Vmx.h:69: VMCS_FIELD_TSC_OFFSET_FULL = 0x00002010
//=============================================================================
constexpr size_t VMCS_FIELD_TSC_OFFSET_FULL = 0x2010;

namespace timing {

//=============================================================================
// Global Per-CPU Timing State
// Cache-line aligned to prevent false sharing between cores
//=============================================================================
alignas(64) VcpuTimingState g_timing_states[MAX_CPUS];

//-----------------------------------------------------------------------------
// Utility Functions
//-----------------------------------------------------------------------------

u32 GetApicId() {
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    return static_cast<u32>((cpuInfo[1] >> 24) & 0xFF);
}

VcpuTimingState* GetCurrentState() {
    u32 apicId = GetApicId();
    if (apicId >= MAX_CPUS) {
        return nullptr;
    }
    return &g_timing_states[apicId];
}

//-----------------------------------------------------------------------------
// Initialization
//-----------------------------------------------------------------------------

void Initialize() {
    auto* state = GetCurrentState();
    if (!state || state->initialized) {
        return;
    }

    state->exit_entry_tsc = 0;
    state->accumulated_overhead = 0;
    state->aperf_offset = 0;
    state->mperf_offset = 0;

#ifdef OMBRA_DEBUG
    state->vmexit_count = 0;
    state->total_overhead = 0;
    state->min_overhead = ~0ULL;
    state->max_overhead = 0;
#endif

    state->initialized = true;
}

//-----------------------------------------------------------------------------
// VMExit Timing Hooks
//-----------------------------------------------------------------------------

void OnExitEntry() {
    auto* state = GetCurrentState();
    if (!state) {
        return;
    }

    // Capture TSC immediately - this is the first instruction in VMExit handler
    // The more accurately we capture this, the better our compensation
    state->exit_entry_tsc = __rdtsc();

#ifdef OMBRA_DEBUG
    state->vmexit_count++;
#endif
}

void OnExitComplete(void* arch_data) {
    auto* state = GetCurrentState();
    if (!state) {
        return;
    }

    // Capture exit TSC - this is just before VMResume
    u64 exit_end_tsc = __rdtsc();

    // Calculate overhead for this VMExit
    u64 overhead = exit_end_tsc - state->exit_entry_tsc;

    // Accumulate for APERF/MPERF compensation
    // These MSRs count real cycles, so we need to subtract our overhead
    state->aperf_offset += overhead;
    state->mperf_offset += overhead;

#ifdef OMBRA_DEBUG
    state->total_overhead += overhead;
    if (overhead < state->min_overhead) state->min_overhead = overhead;
    if (overhead > state->max_overhead) state->max_overhead = overhead;
#endif

    //=========================================================================
    // Apply TSC offset compensation
    // By subtracting overhead from TSC offset, the guest sees:
    //   reported_tsc = real_tsc + tsc_offset
    //                = real_tsc + (original_offset - overhead)
    //                = real_tsc - overhead + original_offset
    // This effectively hides our VMExit processing time
    //=========================================================================

    if (arch_data) {
        //---------------------------------------------------------------------
        // AMD Path: arch_data is VMCB pointer
        // VMCB ControlArea.TscOffset is at offset 0x50 from VMCB base
        //---------------------------------------------------------------------
        auto* vmcb = reinterpret_cast<u8*>(arch_data);
        auto* tsc_offset_ptr = reinterpret_cast<i64*>(vmcb + 0x50);

        // Subtract overhead from TSC offset (makes guest see compensated time)
        *tsc_offset_ptr -= static_cast<i64>(overhead);
    } else {
        //---------------------------------------------------------------------
        // Intel Path: Use VMCS TSC_OFFSET field
        // VMCS provides atomic read/write via intrinsics
        //---------------------------------------------------------------------
        size_t current_offset = 0;
        __vmx_vmread(VMCS_FIELD_TSC_OFFSET_FULL, &current_offset);

        // Subtract overhead from TSC offset
        size_t new_offset = current_offset - overhead;
        __vmx_vmwrite(VMCS_FIELD_TSC_OFFSET_FULL, new_offset);
    }
}

//-----------------------------------------------------------------------------
// MSR Virtualization
//-----------------------------------------------------------------------------

u64 ReadMsrVirtualized(u32 msr_id) {
    auto* state = GetCurrentState();

    // Read real MSR value
    u64 real_value = __readmsr(msr_id);

    if (!state) {
        return real_value;
    }

    // Compensate based on MSR type
    // These MSRs count actual CPU cycles, and VMExit handling adds real cycles
    // that show up in these counters. We subtract our accumulated overhead.
    switch (msr_id) {
        case 0xE7:  // IA32_MPERF - Maximum Performance
            return real_value - state->mperf_offset;

        case 0xE8:  // IA32_APERF - Actual Performance
            return real_value - state->aperf_offset;

        default:
            return real_value;
    }
}

} // namespace timing
