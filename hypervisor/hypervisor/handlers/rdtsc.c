// rdtsc.c — RDTSC/RDTSCP Exit Handlers
// OmbraHypervisor

#include "handlers.h"
#include "../vmx.h"
#include "../debug.h"
#include <intrin.h>

// =============================================================================
// TSC Overhead Compensation
// =============================================================================
//
// The overhead value compensates for the time spent handling the VM-exit.
// This is approximate - a real implementation would calibrate this per-CPU
// and adjust dynamically based on observed VM-exit latency.
//
// Typical VM-exit/VM-entry overhead: 500-2000 cycles on modern CPUs
// We use a conservative estimate.

#define TSC_EXIT_OVERHEAD   1500

// =============================================================================
// RDTSC Handler
// =============================================================================

VMEXIT_ACTION HandleRdtsc(GUEST_REGS* regs) {
    VMX_CPU* cpu;
    U64 realTsc;
    U64 adjustedTsc;

    // Get real TSC value
    realTsc = __rdtsc();

    // Get per-CPU state for TSC tracking
    cpu = VmxGetCurrentCpu();
    if (cpu) {
        // Apply accumulated offset (hide VM-exit overhead over time)
        adjustedTsc = realTsc - cpu->TscOffset;

        // Subtract current exit overhead
        if (adjustedTsc > TSC_EXIT_OVERHEAD) {
            adjustedTsc -= TSC_EXIT_OVERHEAD;
        }

        // Ensure monotonic: never return a value less than last returned
        if (adjustedTsc <= cpu->LastTsc) {
            adjustedTsc = cpu->LastTsc + 1;
        }

        cpu->LastTsc = adjustedTsc;
    } else {
        // Fallback if we can't get CPU state
        adjustedTsc = realTsc;
        if (adjustedTsc > TSC_EXIT_OVERHEAD) {
            adjustedTsc -= TSC_EXIT_OVERHEAD;
        }
    }

    // Return TSC in EDX:EAX
    regs->Rax = (U32)adjustedTsc;
    regs->Rdx = (U32)(adjustedTsc >> 32);

    return VMEXIT_ADVANCE_RIP;
}

// =============================================================================
// RDTSCP Handler
// =============================================================================

VMEXIT_ACTION HandleRdtscp(GUEST_REGS* regs) {
    VMX_CPU* cpu;
    U64 realTsc;
    U64 adjustedTsc;
    U32 aux;

    // Get real TSC and processor ID
    realTsc = __rdtscp(&aux);

    // Get per-CPU state for TSC tracking
    cpu = VmxGetCurrentCpu();
    if (cpu) {
        // Apply accumulated offset
        adjustedTsc = realTsc - cpu->TscOffset;

        // Subtract current exit overhead
        if (adjustedTsc > TSC_EXIT_OVERHEAD) {
            adjustedTsc -= TSC_EXIT_OVERHEAD;
        }

        // Ensure monotonic
        if (adjustedTsc <= cpu->LastTsc) {
            adjustedTsc = cpu->LastTsc + 1;
        }

        cpu->LastTsc = adjustedTsc;
    } else {
        adjustedTsc = realTsc;
        if (adjustedTsc > TSC_EXIT_OVERHEAD) {
            adjustedTsc -= TSC_EXIT_OVERHEAD;
        }
    }

    // Return TSC in EDX:EAX, IA32_TSC_AUX in ECX
    regs->Rax = (U32)adjustedTsc;
    regs->Rdx = (U32)(adjustedTsc >> 32);
    regs->Rcx = aux;

    return VMEXIT_ADVANCE_RIP;
}
