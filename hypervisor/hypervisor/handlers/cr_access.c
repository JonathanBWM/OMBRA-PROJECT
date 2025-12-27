// cr_access.c — Control Register Access Handler
// OmbraHypervisor

#include "handlers.h"
#include "../vmx.h"
#include "../timing.h"
#include "../../shared/vmcs_fields.h"
#include "../../shared/cpu_defs.h"
#include <intrin.h>

// =============================================================================
// Exit Qualification Format for CR Access (Intel SDM Vol 3, Table 27-3)
// =============================================================================
//
// Bits 3:0   - Control register number (0, 3, 4, or 8)
// Bits 5:4   - Access type:
//                0 = MOV to CR
//                1 = MOV from CR
//                2 = CLTS
//                3 = LMSW
// Bits 7:6   - Reserved
// Bits 11:8  - Source/destination register (for MOV to/from CR):
//                0=RAX, 1=RCX, 2=RDX, 3=RBX, 4=RSP, 5=RBP, 6=RSI, 7=RDI
//                8=R8, 9=R9, 10=R10, 11=R11, 12=R12, 13=R13, 14=R14, 15=R15
// Bits 15:12 - Reserved
// Bits 31:16 - LMSW source data (for LMSW only)

#define CR_ACCESS_CR_MASK       0x0F
#define CR_ACCESS_TYPE_SHIFT    4
#define CR_ACCESS_TYPE_MASK     0x03
#define CR_ACCESS_REG_SHIFT     8
#define CR_ACCESS_REG_MASK      0x0F
#define CR_ACCESS_LMSW_SHIFT    16

#define CR_ACCESS_TYPE_MOV_TO   0
#define CR_ACCESS_TYPE_MOV_FROM 1
#define CR_ACCESS_TYPE_CLTS     2
#define CR_ACCESS_TYPE_LMSW     3

// =============================================================================
// Helper: Get register value from GUEST_REGS by index
// =============================================================================

static U64 GetRegisterValue(GUEST_REGS* regs, U32 regIndex) {
    switch (regIndex) {
    case 0:  return regs->Rax;
    case 1:  return regs->Rcx;
    case 2:  return regs->Rdx;
    case 3:  return regs->Rbx;
    case 4:  return regs->Rsp;  // Note: This is the placeholder, not real RSP
    case 5:  return regs->Rbp;
    case 6:  return regs->Rsi;
    case 7:  return regs->Rdi;
    case 8:  return regs->R8;
    case 9:  return regs->R9;
    case 10: return regs->R10;
    case 11: return regs->R11;
    case 12: return regs->R12;
    case 13: return regs->R13;
    case 14: return regs->R14;
    case 15: return regs->R15;
    default: return 0;
    }
}

static void SetRegisterValue(GUEST_REGS* regs, U32 regIndex, U64 value) {
    switch (regIndex) {
    case 0:  regs->Rax = value; break;
    case 1:  regs->Rcx = value; break;
    case 2:  regs->Rdx = value; break;
    case 3:  regs->Rbx = value; break;
    case 4:  regs->Rsp = value; break;
    case 5:  regs->Rbp = value; break;
    case 6:  regs->Rsi = value; break;
    case 7:  regs->Rdi = value; break;
    case 8:  regs->R8  = value; break;
    case 9:  regs->R9  = value; break;
    case 10: regs->R10 = value; break;
    case 11: regs->R11 = value; break;
    case 12: regs->R12 = value; break;
    case 13: regs->R13 = value; break;
    case 14: regs->R14 = value; break;
    case 15: regs->R15 = value; break;
    }
}

// =============================================================================
// CR Access Handler
// =============================================================================

VMEXIT_ACTION HandleCrAccess(GUEST_REGS* regs, U64 qualification) {
    U64 entryTsc = TimingStart();
    VMX_CPU* cpu = VmxGetCurrentCpu();
    U32 crNumber;
    U32 accessType;
    U32 regIndex;
    U64 value;
    U64 guestCr0, guestCr4;

    // Parse exit qualification
    crNumber = (U32)(qualification & CR_ACCESS_CR_MASK);
    accessType = (U32)((qualification >> CR_ACCESS_TYPE_SHIFT) & CR_ACCESS_TYPE_MASK);
    regIndex = (U32)((qualification >> CR_ACCESS_REG_SHIFT) & CR_ACCESS_REG_MASK);

    switch (accessType) {
    case CR_ACCESS_TYPE_MOV_TO:
        // MOV to CR
        value = GetRegisterValue(regs, regIndex);

        switch (crNumber) {
        case 0:
            // MOV to CR0
            // Apply fixed bits from VMX requirements
            value |= __readmsr(MSR_IA32_VMX_CR0_FIXED0);
            value &= __readmsr(MSR_IA32_VMX_CR0_FIXED1);

            // Update both the real CR0 and VMCS guest CR0
            VmcsWrite(VMCS_GUEST_CR0, value);

            // Update shadow to match (for bits we're not intercepting)
            VmcsWrite(VMCS_CTRL_CR0_SHADOW, value);
            break;

        case 3:
            // MOV to CR3 - update page tables
            // EPT handles translation, so just update VMCS
            VmcsWrite(VMCS_GUEST_CR3, value);
            break;

        case 4:
            // MOV to CR4
            // Apply fixed bits and ensure VMXE stays set
            value |= __readmsr(MSR_IA32_VMX_CR4_FIXED0);
            value &= __readmsr(MSR_IA32_VMX_CR4_FIXED1);
            value |= CR4_VMXE;  // Never let guest clear VMXE

            VmcsWrite(VMCS_GUEST_CR4, value);

            // Update shadow (hiding VMXE from guest)
            VmcsWrite(VMCS_CTRL_CR4_SHADOW, value & ~CR4_VMXE);
            break;

        case 8:
            // MOV to CR8 (TPR - Task Priority Register)
            // CR8 controls interrupt priority threshold in 64-bit mode
            // Without APIC virtualization, we shadow it in VMX_CPU state
            cpu->GuestCr8 = value & 0x0F;  // Only lower 4 bits valid (priority 0-15)

            // If interrupts are pending and new TPR allows them, update pending debug
            // This is a simplified implementation - full TPR virtualization would
            // integrate with local APIC TPR (0x80) and check pending interrupt priority
            break;
        }
        break;

    case CR_ACCESS_TYPE_MOV_FROM:
        // MOV from CR
        switch (crNumber) {
        case 0:
            // Return the shadow value (what guest thinks CR0 is)
            value = VmcsRead(VMCS_CTRL_CR0_SHADOW);
            break;

        case 3:
            value = VmcsRead(VMCS_GUEST_CR3);
            break;

        case 4:
            // Return shadow (hides VMXE bit)
            value = VmcsRead(VMCS_CTRL_CR4_SHADOW);
            break;

        case 8:
            // MOV from CR8 - read current TPR
            value = cpu->GuestCr8 & 0x0F;  // Return shadowed value
            break;

        default:
            value = 0;
            break;
        }

        SetRegisterValue(regs, regIndex, value);
        break;

    case CR_ACCESS_TYPE_CLTS:
        // CLTS - Clear Task-Switched flag in CR0
        guestCr0 = VmcsRead(VMCS_GUEST_CR0);
        guestCr0 &= ~CR0_TS;  // Clear TS bit (bit 3)
        VmcsWrite(VMCS_GUEST_CR0, guestCr0);
        VmcsWrite(VMCS_CTRL_CR0_SHADOW, guestCr0);
        break;

    case CR_ACCESS_TYPE_LMSW:
        // LMSW - Load Machine Status Word (lower 16 bits of CR0)
        value = (qualification >> CR_ACCESS_LMSW_SHIFT) & 0xFFFF;
        guestCr0 = VmcsRead(VMCS_GUEST_CR0);

        // LMSW can only set PE, MP, EM, TS bits (bits 0-3)
        // It cannot clear PE once set
        guestCr0 = (guestCr0 & 0xFFFFFFFFFFFFFFF0ULL) | (value & 0x0F);

        // Apply VMX fixed bits
        guestCr0 |= __readmsr(MSR_IA32_VMX_CR0_FIXED0);
        guestCr0 &= __readmsr(MSR_IA32_VMX_CR0_FIXED1);

        VmcsWrite(VMCS_GUEST_CR0, guestCr0);
        VmcsWrite(VMCS_CTRL_CR0_SHADOW, guestCr0);
        break;
    }

    if (cpu) TimingEnd(cpu, entryTsc, TIMING_CR_OVERHEAD);
    return VMEXIT_ADVANCE_RIP;
}
