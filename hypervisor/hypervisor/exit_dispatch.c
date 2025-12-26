// exit_dispatch.c — VM-Exit Dispatcher
// OmbraHypervisor

#include "exit_dispatch.h"
#include "vmx.h"
#include "handlers/handlers.h"
#include "debug.h"
#include "../shared/vmcs_fields.h"
#include "../shared/exit_reasons.h"

// =============================================================================
// Helper: Advance Guest RIP by instruction length
// =============================================================================

static void AdvanceGuestRip(void) {
    U64 rip = VmcsRead(VMCS_GUEST_RIP);
    U64 len = VmcsRead(VMCS_EXIT_INSTR_LEN);
    VmcsWrite(VMCS_GUEST_RIP, rip + len);
}

// =============================================================================
// Main Exit Dispatcher
// =============================================================================

VMEXIT_ACTION VmexitDispatch(GUEST_REGS* regs) {
    U32 reason;
    U64 qualification;
    VMEXIT_ACTION action;
    VMX_CPU* cpu;

    // Get exit reason (bits 15:0)
    reason = (U32)VmcsRead(VMCS_EXIT_REASON) & 0xFFFF;
    qualification = VmcsRead(VMCS_EXIT_QUALIFICATION);

    // Get current CPU structure for stats
    cpu = VmxGetCurrentCpu();
    if (cpu) {
        cpu->VmexitCount++;
        cpu->LastExitReason = reason;

        // Cache guest state for quick access
        cpu->GuestRip = VmcsRead(VMCS_GUEST_RIP);
        cpu->GuestRsp = VmcsRead(VMCS_GUEST_RSP);
        cpu->GuestRflags = VmcsRead(VMCS_GUEST_RFLAGS);
    }

    // Dispatch to appropriate handler
    switch (reason) {
    case EXIT_REASON_CPUID:
        action = HandleCpuid(regs);
        break;

    case EXIT_REASON_RDTSC:
        action = HandleRdtsc(regs);
        break;

    case EXIT_REASON_RDTSCP:
        action = HandleRdtscp(regs);
        break;

    case EXIT_REASON_RDMSR:
        action = HandleRdmsr(regs);
        break;

    case EXIT_REASON_WRMSR:
        action = HandleWrmsr(regs);
        break;

    case EXIT_REASON_CR_ACCESS:
        action = HandleCrAccess(regs, qualification);
        break;

    case EXIT_REASON_EPT_VIOLATION:
        action = HandleEptViolation(regs, qualification);
        break;

    case EXIT_REASON_VMCALL:
        action = HandleVmcall(regs);
        break;

    case EXIT_REASON_EXCEPTION_NMI:
        action = HandleException(regs);
        break;

    case EXIT_REASON_IO_INSTRUCTION:
        action = HandleIo(regs, qualification);
        break;

    case EXIT_REASON_TRIPLE_FAULT:
        // Triple fault = unrecoverable, must shutdown
        FATAL("Triple fault! RIP=0x%llx RSP=0x%llx",
              cpu ? cpu->GuestRip : 0, cpu ? cpu->GuestRsp : 0);
        action = VMEXIT_SHUTDOWN;
        break;

    case EXIT_REASON_HLT:
        // Just advance RIP and let guest idle
        // In a real implementation, we might inject interrupts or wait
        action = VMEXIT_ADVANCE_RIP;
        break;

    case EXIT_REASON_INVLPG:
        // INVLPG - just advance RIP, TLB management is automatic with EPT
        action = VMEXIT_ADVANCE_RIP;
        break;

    case EXIT_REASON_VMLAUNCH:
    case EXIT_REASON_VMRESUME:
    case EXIT_REASON_VMREAD:
    case EXIT_REASON_VMWRITE:
    case EXIT_REASON_VMXON:
    case EXIT_REASON_VMXOFF:
    case EXIT_REASON_VMCLEAR:
    case EXIT_REASON_VMPTRLD:
    case EXIT_REASON_VMPTRST:
    case EXIT_REASON_INVEPT:
    case EXIT_REASON_INVVPID:
        // VMX instructions from guest - inject #UD (undefined opcode)
        // Since we hide VMX capability, these instructions should fault
        TRACE("Guest executed VMX instruction (reason=%u), injecting #UD", reason);
        {
            // VM-entry interruption-information field format:
            // Bits 7:0   = Vector (6 = #UD)
            // Bits 10:8  = Type (3 = Hardware exception)
            // Bit 11     = Deliver error code (0 for #UD)
            // Bit 31     = Valid
            U32 intInfo = 6;                    // #UD vector
            intInfo |= (3 << 8);                // Hardware exception
            intInfo |= (1UL << 31);             // Valid

            VmcsWrite(VMCS_CTRL_VMENTRY_INT_INFO, intInfo);
            VmcsWrite(VMCS_CTRL_VMENTRY_INSTR_LEN, VmcsRead(VMCS_EXIT_INSTR_LEN));
        }
        action = VMEXIT_CONTINUE;  // Don't advance RIP
        break;

    case EXIT_REASON_XSETBV:
        // XSETBV - just pass through for now
        action = VMEXIT_ADVANCE_RIP;
        break;

    default:
        // Unknown exit reason - advance RIP and hope for the best
        WARN("Unknown exit reason %u at RIP=0x%llx qual=0x%llx",
             reason, cpu ? cpu->GuestRip : 0, qualification);
        action = VMEXIT_ADVANCE_RIP;
        break;
    }

    // Process the action
    switch (action) {
    case VMEXIT_ADVANCE_RIP:
        AdvanceGuestRip();
        break;

    case VMEXIT_CONTINUE:
        // Don't advance RIP - handler already did what was needed
        // (e.g., re-execute instruction after fixing EPT violation)
        break;

    case VMEXIT_SHUTDOWN:
        // Will be handled by assembly to devirtualize
        INFO("VM-exit requesting shutdown, devirtualizing...");
        break;
    }

    return action;
}
