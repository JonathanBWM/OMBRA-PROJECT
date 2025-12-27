// msr.c — MSR Access Exit Handlers
// OmbraHypervisor

#include "handlers.h"
#include "../debug.h"
#include "../timing.h"
#include "../vmx.h"
#include "../../shared/msr_defs.h"
#include "../../shared/vmcs_fields.h"
#include <intrin.h>

// =============================================================================
// MSR Stealth Configuration
// =============================================================================
//
// Certain MSRs reveal VMX state or hypervisor presence. We virtualize these
// to hide our presence from anti-cheat and anti-VM detection.

// IA32_FEATURE_CONTROL - controls VMX enable/lock
#define MSR_IA32_FEATURE_CONTROL    0x3A

// IA32_VMX_* MSRs (0x480-0x491) - VMX capability reporting
#define MSR_VMX_RANGE_START         0x480
#define MSR_VMX_RANGE_END           0x491

// IA32_DEBUGCTL - debug control (may reveal single-stepping)
#define MSR_IA32_DEBUGCTL           0x1D9

// APERF/MPERF MSRs (CRITICAL: ESEA monitors ratio for hypervisor detection)
// MCP verified: get_msr_info("IA32_APERF") -> 0xE8, get_msr_info("IA32_MPERF") -> 0xE7
#define MSR_IA32_MPERF              0xE7   // Maximum Performance Frequency Clock
#define MSR_IA32_APERF              0xE8   // Actual Performance Frequency Clock

// Exception vectors
#define EXCEPTION_GP                13

// =============================================================================
// Exception Injection Helper
// =============================================================================
//
// Injects a #GP(0) exception to simulate MSR access failure on non-VMX system

static VMEXIT_ACTION InjectGpFault(void) {
    U32 interruptInfo = 0;

    // VM-entry interruption-information field format:
    // Bits 7:0   = Vector (13 = #GP)
    // Bits 10:8  = Type (3 = Hardware exception)
    // Bit 11     = Deliver error code
    // Bit 31     = Valid

    interruptInfo = EXCEPTION_GP;           // Vector = 13
    interruptInfo |= (3 << 8);              // Type = Hardware exception
    interruptInfo |= (1 << 11);             // Has error code
    interruptInfo |= (1UL << 31);           // Valid

    VmcsWrite(VMCS_CTRL_VMENTRY_INT_INFO, interruptInfo);
    VmcsWrite(VMCS_CTRL_VMENTRY_EXCEPTION_CODE, 0);  // Error code = 0
    VmcsWrite(VMCS_CTRL_VMENTRY_INSTR_LEN, VmcsRead(VMCS_EXIT_INSTR_LEN));

    TRACE("Injecting #GP(0) for MSR access");
    return VMEXIT_CONTINUE;  // Don't advance RIP - exception handler will
}

// =============================================================================
// RDMSR Handler
// =============================================================================

VMEXIT_ACTION HandleRdmsr(GUEST_REGS* regs) {
    U32 msr = (U32)regs->Rcx;
    U64 value;
    VMX_CPU* cpu;
    U64 entryTsc;

    // TIMING COMPENSATION: Capture TSC at handler entry
    // Anti-cheat may measure RDMSR timing to detect hypervisors
    entryTsc = TimingStart();

    // Handle MSRs that need virtualization for stealth
    switch (msr) {
    case MSR_IA32_FEATURE_CONTROL:
        // Return a value that looks like VMX is locked and disabled
        // This hides our VMX usage from detection
        value = FEATURE_CONTROL_LOCK;  // Locked but VMX disabled
        break;

    case MSR_IA32_VMX_BASIC:
    case MSR_IA32_VMX_PINBASED_CTLS:
    case MSR_IA32_VMX_PROCBASED_CTLS:
    case MSR_IA32_VMX_EXIT_CTLS:
    case MSR_IA32_VMX_ENTRY_CTLS:
    case MSR_IA32_VMX_CR0_FIXED0:
    case MSR_IA32_VMX_CR0_FIXED1:
    case MSR_IA32_VMX_CR4_FIXED0:
    case MSR_IA32_VMX_CR4_FIXED1:
    case MSR_IA32_VMX_PROCBASED_CTLS2:
    case MSR_IA32_VMX_EPT_VPID_CAP:
    case MSR_IA32_VMX_TRUE_PINBASED:
    case MSR_IA32_VMX_TRUE_PROCBASED:
    case MSR_IA32_VMX_TRUE_EXIT:
    case MSR_IA32_VMX_TRUE_ENTRY:
        // VMX capability MSRs - inject #GP to simulate VMX not present
        // A CPU without VMX support would fault on these reads
        return InjectGpFault();

    case MSR_IA32_TSC_AUX:
        // Return virtualized processor ID for RDTSCP consistency
        // Guest sees same value across VM-exits maintaining affinity illusion
        cpu = VmxGetCurrentCpu();
        value = cpu ? cpu->GuestTscAux : __readmsr(msr);
        break;

    case MSR_IA32_MPERF:
        // MPERF virtualization - compensate for VM-exit overhead
        // CRITICAL: ESEA monitors APERF/MPERF ratio to detect hypervisors
        // We subtract accumulated offset to make overhead invisible
        cpu = VmxGetCurrentCpu();
        if (cpu) {
            value = __readmsr(MSR_IA32_MPERF) - cpu->MperfOffset;
        } else {
            value = __readmsr(MSR_IA32_MPERF);
        }
        break;

    case MSR_IA32_APERF:
        // APERF virtualization - compensate for VM-exit overhead
        // CRITICAL: ESEA monitors APERF/MPERF ratio to detect hypervisors
        // We subtract accumulated offset to make overhead invisible
        cpu = VmxGetCurrentCpu();
        if (cpu) {
            value = __readmsr(MSR_IA32_APERF) - cpu->AperfOffset;
        } else {
            value = __readmsr(MSR_IA32_APERF);
        }
        break;

    default:
        // Pass through all other MSRs
        value = __readmsr(msr);
        break;
    }

    // Return value in EDX:EAX
    regs->Rax = (U32)value;
    regs->Rdx = (U32)(value >> 32);

    // TIMING COMPENSATION: Apply compensation before returning to guest
    cpu = VmxGetCurrentCpu();
    if (cpu) {
        TimingEnd(cpu, entryTsc, TIMING_MSR_OVERHEAD);
    }

    return VMEXIT_ADVANCE_RIP;
}

// =============================================================================
// WRMSR Handler
// =============================================================================

VMEXIT_ACTION HandleWrmsr(GUEST_REGS* regs) {
    U32 msr = (U32)regs->Rcx;
    U64 value = ((U64)regs->Rdx << 32) | (regs->Rax & 0xFFFFFFFF);
    VMX_CPU* cpu;
    U64 entryTsc;

    // TIMING COMPENSATION: Capture TSC at handler entry
    entryTsc = TimingStart();

    // Handle MSRs that need special treatment
    switch (msr) {
    case MSR_IA32_FEATURE_CONTROL:
        // Silently ignore writes to FEATURE_CONTROL
        // We don't want the guest changing VMX settings
        break;

    case MSR_IA32_DEBUGCTL:
        // Allow debug control changes but could filter for stealth
        __writemsr(msr, value);
        break;

    case MSR_IA32_FS_BASE:
    case MSR_IA32_GS_BASE:
    case MSR_IA32_KERNEL_GS_BASE:
        // FS/GS base changes - pass through and update VMCS
        __writemsr(msr, value);
        // Note: VMCS guest state will be updated automatically on next VM-entry
        // if we're using the "load FS/GS base" VM-entry control
        break;

    case MSR_IA32_SYSENTER_CS:
    case MSR_IA32_SYSENTER_ESP:
    case MSR_IA32_SYSENTER_EIP:
        // SYSENTER MSRs - pass through
        __writemsr(msr, value);
        break;

    case MSR_IA32_EFER:
        // EFER changes - pass through but we may want to validate
        // to prevent guest from disabling long mode unexpectedly
        __writemsr(msr, value);
        break;

    case MSR_IA32_TSC_AUX:
        // Virtualize IA32_TSC_AUX writes - store in per-CPU state
        // This ensures RDTSCP returns guest's expected processor ID
        cpu = VmxGetCurrentCpu();
        if (cpu) {
            cpu->GuestTscAux = (U32)value;
        }
        // Don't write to real MSR - keep host value intact
        break;

    default:
        // Range check for VMX MSRs - silently ignore
        if (msr >= MSR_VMX_RANGE_START && msr <= MSR_VMX_RANGE_END) {
            // Ignore writes to VMX capability MSRs (they're read-only anyway)
            break;
        }

        // Pass through all other MSR writes
        __writemsr(msr, value);
        break;
    }

    // TIMING COMPENSATION: Apply compensation before returning to guest
    cpu = VmxGetCurrentCpu();
    if (cpu) {
        TimingEnd(cpu, entryTsc, TIMING_MSR_OVERHEAD);
    }

    return VMEXIT_ADVANCE_RIP;
}
