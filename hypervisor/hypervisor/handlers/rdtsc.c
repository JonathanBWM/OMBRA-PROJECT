// rdtsc.c — RDTSC/RDTSCP Exit Handlers
// OmbraHypervisor

#include "handlers.h"
#include "../timing.h"
#include "../vmx.h"
#include "../debug.h"
#include <intrin.h>
#include <emmintrin.h>  // For _mm_lfence()

// =============================================================================
// RDTSC Handler
// =============================================================================
//
// RDTSC is special: guest is explicitly reading TSC, so we can't use VMCS offset
// compensation like we do for other exits. Instead, we:
// 1. Capture TSC at entry
// 2. Return a compensated value directly to guest registers
// 3. Update LastTsc for monotonicity
// 4. Apply compensation for this exit to benefit future TSC reads

VMEXIT_ACTION HandleRdtsc(GUEST_REGS* regs) {
    VMX_CPU* cpu;
    U64 entryTsc;
    U64 adjustedTsc;
    I64 netCompensation;

    // Capture TSC at handler entry with serialization
    // LFENCE ensures all prior instructions complete before RDTSC
    _mm_lfence();
    entryTsc = __rdtsc();

    // Get per-CPU state for TSC tracking
    cpu = VmxGetCurrentCpu();
    if (cpu) {
        // Calculate net compensation: accumulated offset + this exit's overhead
        // Use signed arithmetic to handle cases where overhead exceeds TSC delta
        netCompensation = (I64)cpu->TscOffset + (I64)TIMING_RDTSC_OVERHEAD;

        // Apply compensation, but prevent underflow
        if ((I64)entryTsc >= netCompensation) {
            adjustedTsc = entryTsc - (U64)netCompensation;
        } else {
            // If compensation exceeds TSC (shouldn't happen, but be safe),
            // just return LastTsc + 1 to maintain monotonicity
            adjustedTsc = cpu->LastTsc + 1;
        }

        // Ensure monotonic: never return a value less than last returned
        // This prevents detection via backwards TSC values
        if (adjustedTsc <= cpu->LastTsc) {
            adjustedTsc = cpu->LastTsc + 1;
        }

        cpu->LastTsc = adjustedTsc;

        // Apply timing compensation for this exit
        // This will adjust VMCS TSC offset for future RDTSC reads
        TimingEnd(cpu, entryTsc, TIMING_RDTSC_OVERHEAD);
    } else {
        // Fallback if we can't get CPU state
        // Just return raw TSC minus overhead if safe, otherwise raw TSC
        if (entryTsc > TIMING_RDTSC_OVERHEAD) {
            adjustedTsc = entryTsc - TIMING_RDTSC_OVERHEAD;
        } else {
            adjustedTsc = entryTsc;
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
//
// RDTSCP reads TSC and IA32_TSC_AUX (processor ID) atomically
// We apply the same compensation as RDTSC, plus virtualize the processor ID

VMEXIT_ACTION HandleRdtscp(GUEST_REGS* regs) {
    VMX_CPU* cpu;
    U64 entryTsc;
    U64 adjustedTsc;
    U32 aux;
    I64 netCompensation;

    // Capture TSC with serialization
    // RDTSCP is already serializing, but we need LFENCE before it to ensure
    // all prior instructions complete before timing measurement
    _mm_lfence();
    entryTsc = __rdtsc();  // Use __rdtsc() instead of __rdtscp() for consistent timing

    // Get per-CPU state for TSC tracking
    cpu = VmxGetCurrentCpu();
    if (cpu) {
        // Calculate net compensation: accumulated offset + this exit's overhead
        netCompensation = (I64)cpu->TscOffset + (I64)TIMING_RDTSC_OVERHEAD;

        // Apply compensation, but prevent underflow
        if ((I64)entryTsc >= netCompensation) {
            adjustedTsc = entryTsc - (U64)netCompensation;
        } else {
            // If compensation exceeds TSC, return LastTsc + 1 for monotonicity
            adjustedTsc = cpu->LastTsc + 1;
        }

        // Ensure monotonic
        if (adjustedTsc <= cpu->LastTsc) {
            adjustedTsc = cpu->LastTsc + 1;
        }

        cpu->LastTsc = adjustedTsc;

        // Apply timing compensation for this exit
        TimingEnd(cpu, entryTsc, TIMING_RDTSC_OVERHEAD);

        // Virtualize processor ID from IA32_TSC_AUX
        // Return guest's expected value - this MSR is fully virtualized
        // to maintain consistent processor affinity across VM-exits
        aux = cpu->GuestTscAux;
    } else {
        // Fallback without CPU state
        if (entryTsc > TIMING_RDTSC_OVERHEAD) {
            adjustedTsc = entryTsc - TIMING_RDTSC_OVERHEAD;
        } else {
            adjustedTsc = entryTsc;
        }
        // Read actual processor ID as fallback
        __rdtscp(&aux);
    }

    // Return TSC in EDX:EAX, IA32_TSC_AUX in ECX
    regs->Rax = (U32)adjustedTsc;
    regs->Rdx = (U32)(adjustedTsc >> 32);
    regs->Rcx = aux;

    return VMEXIT_ADVANCE_RIP;
}
