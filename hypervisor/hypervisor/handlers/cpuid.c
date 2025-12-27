// cpuid.c — CPUID Exit Handler
// OmbraHypervisor

#include "handlers.h"
#include "../debug.h"
#include "../timing.h"
#include "../vmx.h"
#include "../../shared/cpu_defs.h"
#include <intrin.h>

// =============================================================================
// CPUID Feature Bits for Stealth
// =============================================================================

#define CPUID_1_ECX_VMX     (1 << 5)    // VMX capability bit
#define CPUID_1_ECX_HV      (1 << 31)   // Hypervisor present bit

// AMD Extended Features (leaf 0x80000001)
#define CPUID_80000001_ECX_SVM  (1 << 2)   // SVM (AMD-V) capability bit

// =============================================================================
// CPUID Handler
// =============================================================================

VMEXIT_ACTION HandleCpuid(GUEST_REGS* r) {
    int info[4];
    U32 leaf = (U32)r->Rax;
    U32 subleaf = (U32)r->Rcx;
    VMX_CPU* cpu;
    U64 entryTsc;

    // TIMING COMPENSATION: Capture TSC at handler entry
    // Anti-cheat often measures: RDTSC; CPUID; RDTSC to detect hypervisors
    // We compensate for VM-exit overhead to appear native
    entryTsc = TimingStart();

    // Execute real CPUID
    __cpuidex(info, (int)leaf, (int)subleaf);

    // =========================================================================
    // STEALTH: Hide hypervisor and VMX presence
    // =========================================================================

    switch (leaf) {
    case 0x01:
        // CPUID.1.ECX - Feature Information
        // Clear hypervisor present bit (bit 31)
        info[2] &= ~CPUID_1_ECX_HV;
        // Clear VMX capability bit (bit 5) - pretend CPU doesn't support VMX
        info[2] &= ~CPUID_1_ECX_VMX;
        break;

    case 0x80000001:
        // AMD Extended Features - hide SVM (AMD-V) capability
        // AMD anti-cheat checks this bit to detect AMD virtualization
        // MCP verified: get_amd_evasion_info()["cpuid_leaves"]["0x80000001"]
        info[2] &= ~CPUID_80000001_ECX_SVM;
        break;

    case 0x40000000:
        // Hypervisor vendor leaf
        // CRITICAL: Preserve EAX (max hypervisor leaf count)
        // Windows enlightenments NEED EAX to be valid
        // Only zero the vendor string (EBX, ECX, EDX)
        // MCP verified: check_cpuid_safety(0x40000000) -> MEDIUM risk
        info[1] = info[2] = info[3] = 0;  // Zero vendor string
        // info[0] (EAX) stays intact - max hypervisor leaf
        break;

    // CRITICAL: Do NOT modify leaves 0x40000001-0x4000000F!
    // These are Windows Hyper-V enlightenments that WILL break things:
    // - Hypercall page setup
    // - SynIC (synthetic interrupt controller)
    // - Synthetic timers
    // - APIC virtualization hints
    // MCP verified: check_cpuid_safety(0x40000001) -> CRITICAL
    // Let them fall through to default (pass through unchanged)

    default:
        // Pass through other leaves unmodified
        // This includes 0x40000001+ for Windows enlightenments
        break;
    }

    // Return results
    r->Rax = info[0];
    r->Rbx = info[1];
    r->Rcx = info[2];
    r->Rdx = info[3];

    // TIMING COMPENSATION: Apply compensation before returning to guest
    // This adjusts VMCS TSC_OFFSET to hide the VM-exit overhead
    cpu = VmxGetCurrentCpu();
    if (cpu) {
        TimingEnd(cpu, entryTsc, TIMING_CPUID_OVERHEAD);
    }

    return VMEXIT_ADVANCE_RIP;
}
