// PayLoad/include/timing.h
// TSC offset compensation for anti-timing detection
//
// This module provides VMExit overhead hiding by:
// 1. Capturing TSC at VMExit entry
// 2. Calculating overhead before VMResume
// 3. Adjusting TSC offset to hide overhead from guest
//
// Defeats: BattlEye RDTSC timing, ESEA APERF/MPERF ratio

#pragma once
#include "types.h"

namespace timing {

//=============================================================================
// Per-vCPU Timing State
// Tracks VMExit overhead for TSC compensation
//=============================================================================
struct VcpuTimingState {
    // TSC captured at VMExit entry (before any handling)
    u64 exit_entry_tsc;

    // Accumulated overhead to hide from guest
    volatile u64 accumulated_overhead;

    // APERF/MPERF shadow values for IET detection bypass
    u64 aperf_offset;
    u64 mperf_offset;

    // Statistics (debug builds only)
#ifdef OMBRA_DEBUG
    u64 vmexit_count;
    u64 total_overhead;
    u64 min_overhead;
    u64 max_overhead;
#endif

    // Initialization flag
    bool initialized;
};

// Maximum supported CPUs (matches typical server configurations)
constexpr u32 MAX_CPUS = 256;

// Global per-CPU timing state array
extern VcpuTimingState g_timing_states[MAX_CPUS];

//-----------------------------------------------------------------------------
// Core API
//-----------------------------------------------------------------------------

// Initialize timing state for current CPU (call during hypervisor init)
void Initialize();

// Call IMMEDIATELY at VMExit entry - captures entry TSC
// Must be the first instruction in handler for accurate measurement
void OnExitEntry();

// Call before VMResume - calculates overhead and adjusts TSC offset
// Intel: pass nullptr (uses VMCS)
// AMD: pass VMCB pointer
void OnExitComplete(void* arch_data);

// Get timing state for current CPU
VcpuTimingState* GetCurrentState();

//-----------------------------------------------------------------------------
// MSR Virtualization for APERF/MPERF
//-----------------------------------------------------------------------------

// Returns compensated MSR value (hides hypervisor overhead)
// Supports IA32_MPERF (0xE7) and IA32_APERF (0xE8)
u64 ReadMsrVirtualized(u32 msr_id);

//-----------------------------------------------------------------------------
// Utility
//-----------------------------------------------------------------------------

// Get current CPU's APIC ID (for indexing into per-CPU arrays)
u32 GetApicId();

} // namespace timing
