// cpuid.c — CPUID Exit Handler
// OmbraHypervisor

#include "handlers.h"
#include "../debug.h"
#include "../../shared/cpu_defs.h"
#include <intrin.h>

// =============================================================================
// CPUID Feature Bits for Stealth
// =============================================================================

#define CPUID_1_ECX_VMX     (1 << 5)    // VMX capability bit
#define CPUID_1_ECX_HV      (1 << 31)   // Hypervisor present bit

// =============================================================================
// CPUID Handler
// =============================================================================

VMEXIT_ACTION HandleCpuid(GUEST_REGS* r) {
    int info[4];
    U32 leaf = (U32)r->Rax;
    U32 subleaf = (U32)r->Rcx;

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

    case 0x40000000:
    case 0x40000001:
    case 0x40000002:
    case 0x40000003:
    case 0x40000004:
    case 0x40000005:
    case 0x40000006:
        // Hypervisor information leaves (used by VMware, Hyper-V, KVM, etc.)
        // Return zeros to deny hypervisor presence
        info[0] = info[1] = info[2] = info[3] = 0;
        break;

    default:
        // Pass through other leaves unmodified
        break;
    }

    // Return results
    r->Rax = info[0];
    r->Rbx = info[1];
    r->Rcx = info[2];
    r->Rdx = info[3];

    return VMEXIT_ADVANCE_RIP;
}
