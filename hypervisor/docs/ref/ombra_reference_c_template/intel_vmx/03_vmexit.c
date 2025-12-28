/**
 * 03_vmexit.c - VMExit Handling Reference
 * 
 * Pure C implementation of VMExit dispatch and common handlers.
 * Reference: Intel SDM Volume 3, Chapter 25-27
 * 
 * CONCEPTS COVERED:
 * - VMExit dispatch architecture
 * - Exit reason decoding
 * - Guest state access
 * - RIP advancement
 * - Common exit handlers (CPUID, MSR, etc.)
 */

#include "../common/types.h"
#include "../common/vmcs_defs.h"
#include "../common/msr_defs.h"

/*===========================================================================
 * VMExit Action Enum
 *===========================================================================*/
typedef enum _VMEXIT_ACTION {
    VMEXIT_CONTINUE,        /* Resume guest without advancing RIP */
    VMEXIT_ADVANCE_RIP,     /* Resume guest, advance RIP past instruction */
    VMEXIT_SHUTDOWN,        /* Terminate hypervisor on this CPU */
} VMEXIT_ACTION;

/*===========================================================================
 * External Assembly Functions
 *===========================================================================*/
extern VMX_RESULT AsmVmread(U64 field, U64* value);
extern VMX_RESULT AsmVmwrite(U64 field, U64 value);
extern VMX_RESULT AsmVmresume(void);
extern VMX_RESULT AsmVmlaunch(void);

/* Helper wrappers */
static inline U64 VmcsRead(U64 field)
{
    U64 value = 0;
    AsmVmread(field, &value);
    return value;
}

static inline void VmcsWrite(U64 field, U64 value)
{
    AsmVmwrite(field, value);
}

/*===========================================================================
 * CONCEPT 1: Exit Information Extraction
 *===========================================================================
 * On VMExit, hardware saves:
 * - Exit reason (VMCS_EXIT_REASON)
 * - Exit qualification (VMCS_EXIT_QUALIFICATION) - varies by exit type
 * - Guest linear/physical address (for memory exits)
 * - Instruction length (for instruction exits)
 */

typedef struct _VMEXIT_INFO {
    U32 Reason;             /* Basic exit reason (bits 15:0) */
    bool VmEntryFailure;    /* Bit 31: VM-entry failure */
    U64 Qualification;      /* Exit qualification */
    U64 GuestLinear;        /* Guest linear address (if valid) */
    U64 GuestPhysical;      /* Guest physical address (EPT violation) */
    U32 InstructionLength;  /* Length of causing instruction */
    U32 InstructionInfo;    /* Instruction-specific info */
    U32 InterruptInfo;      /* Interrupt info (for exceptions) */
    U32 InterruptError;     /* Exception error code */
} VMEXIT_INFO;

/**
 * Read all exit information from VMCS
 */
static void VmexitReadInfo(VMEXIT_INFO* info)
{
    U64 reason;
    
    reason = VmcsRead(VMCS_EXIT_REASON);
    info->Reason = (U32)(reason & 0xFFFF);
    info->VmEntryFailure = (reason & BIT(31)) != 0;
    
    info->Qualification = VmcsRead(VMCS_EXIT_QUALIFICATION);
    info->GuestLinear = VmcsRead(VMCS_EXIT_GUEST_LINEAR);
    info->GuestPhysical = VmcsRead(VMCS_EXIT_GUEST_PHYSICAL);
    info->InstructionLength = (U32)VmcsRead(VMCS_EXIT_INSTR_LEN);
    info->InstructionInfo = (U32)VmcsRead(VMCS_EXIT_INSTR_INFO);
    info->InterruptInfo = (U32)VmcsRead(VMCS_EXIT_INTR_INFO);
    info->InterruptError = (U32)VmcsRead(VMCS_EXIT_INTR_ERROR_CODE);
}

/*===========================================================================
 * CONCEPT 2: RIP Advancement
 *===========================================================================
 * Most instruction-causing exits require advancing guest RIP past the
 * instruction before resuming. The instruction length is in VMCS.
 */

/**
 * Advance guest RIP past the current instruction
 */
static void VmexitAdvanceRip(void)
{
    U64 rip, len;
    
    rip = VmcsRead(VMCS_GUEST_RIP);
    len = VmcsRead(VMCS_EXIT_INSTR_LEN);
    
    VmcsWrite(VMCS_GUEST_RIP, rip + len);
}

/*===========================================================================
 * CONCEPT 3: CPUID Handler
 *===========================================================================
 * CPUID exit: Guest executed CPUID instruction.
 * We execute real CPUID, optionally modify results, return to guest.
 * 
 * Common modifications for stealth:
 * - Clear hypervisor present bit (CPUID.1:ECX[31])
 * - Mask VMX capability (CPUID.1:ECX[5])
 * - Handle hypervisor vendor leaf (0x40000000)
 */

static VMEXIT_ACTION HandleCpuid(GUEST_REGS* regs)
{
    int cpuInfo[4] = {0};
    U32 leaf = (U32)regs->Rax;
    U32 subleaf = (U32)regs->Rcx;
    
    /* Execute real CPUID */
    __cpuidex(cpuInfo, (int)leaf, (int)subleaf);
    
    /* Apply stealth modifications */
    switch (leaf) {
    case 0x00000001:
        /* Feature information */
        /* Clear hypervisor present bit (ECX[31]) for full hiding */
        /* cpuInfo[2] &= ~BIT(31); */
        
        /* Clear VMX capability bit (ECX[5]) */
        cpuInfo[2] &= ~BIT(5);
        break;
        
    case 0x40000000:
        /* Hypervisor vendor leaf */
        /* Option A: Return zeros (full hiding) */
        cpuInfo[0] = 0;
        cpuInfo[1] = 0;
        cpuInfo[2] = 0;
        cpuInfo[3] = 0;
        
        /* Option B: Return Hyper-V signature (if hijacking Hyper-V) */
        /* cpuInfo[1] = 'rciM'; cpuInfo[2] = 'foso'; cpuInfo[3] = 'vH t'; */
        break;
        
    case 0x80000001:
        /* Extended features (AMD) */
        /* Clear SVM capability (EDX[2]) if on AMD */
        cpuInfo[3] &= ~BIT(2);
        break;
    }
    
    /* Return results to guest */
    regs->Rax = (U64)cpuInfo[0];
    regs->Rbx = (U64)cpuInfo[1];
    regs->Rcx = (U64)cpuInfo[2];
    regs->Rdx = (U64)cpuInfo[3];
    
    return VMEXIT_ADVANCE_RIP;
}

/*===========================================================================
 * CONCEPT 4: MSR Read Handler
 *===========================================================================
 * RDMSR exit: Guest read from MSR (ECX = MSR index).
 * Virtualize sensitive MSRs for stealth.
 */

static VMEXIT_ACTION HandleMsrRead(GUEST_REGS* regs)
{
    U32 msrIndex = (U32)regs->Rcx;
    U64 value;
    
    switch (msrIndex) {
    case MSR_IA32_FEATURE_CONTROL:
        /* Hide VMX enabled bits */
        value = __readmsr(msrIndex);
        value &= ~(FEATURE_CONTROL_VMX_OUTSIDE_SMX | FEATURE_CONTROL_VMX_SMX);
        break;
        
    case MSR_IA32_VMX_BASIC:
    case MSR_IA32_VMX_PINBASED_CTLS:
    case MSR_IA32_VMX_PROCBASED_CTLS:
    case MSR_IA32_VMX_EXIT_CTLS:
    case MSR_IA32_VMX_ENTRY_CTLS:
    case MSR_IA32_VMX_MISC:
    case MSR_IA32_VMX_CR0_FIXED0:
    case MSR_IA32_VMX_CR0_FIXED1:
    case MSR_IA32_VMX_CR4_FIXED0:
    case MSR_IA32_VMX_CR4_FIXED1:
    case MSR_IA32_VMX_PROCBASED_CTLS2:
    case MSR_IA32_VMX_EPT_VPID_CAP:
        /* VMX capability MSRs - inject #GP to hide VMX */
        /* InjectException(EXCEPTION_GP, 0); */
        /* return VMEXIT_CONTINUE; */
        /* Or just return zeros */
        value = 0;
        break;
        
    default:
        /* Pass through other MSRs */
        value = __readmsr(msrIndex);
        break;
    }
    
    /* Return value in EDX:EAX */
    regs->Rax = (U32)(value & 0xFFFFFFFF);
    regs->Rdx = (U32)(value >> 32);
    
    return VMEXIT_ADVANCE_RIP;
}

/*===========================================================================
 * CONCEPT 5: MSR Write Handler
 *===========================================================================
 * WRMSR exit: Guest wrote to MSR (ECX = index, EDX:EAX = value).
 */

static VMEXIT_ACTION HandleMsrWrite(GUEST_REGS* regs)
{
    U32 msrIndex = (U32)regs->Rcx;
    U64 value = ((U64)regs->Rdx << 32) | (U32)regs->Rax;
    
    switch (msrIndex) {
    case MSR_IA32_FEATURE_CONTROL:
        /* Block writes that would affect VMX */
        /* Just ignore or inject #GP */
        break;
        
    case MSR_IA32_EFER:
        /* Allow EFER writes but update VMCS guest EFER too */
        VmcsWrite(VMCS_GUEST_EFER, value);
        __writemsr(msrIndex, value);
        break;
        
    default:
        /* Pass through other MSRs */
        __writemsr(msrIndex, value);
        break;
    }
    
    return VMEXIT_ADVANCE_RIP;
}

/*===========================================================================
 * CONCEPT 6: Control Register Access Handler
 *===========================================================================
 * CR access exit: Guest accessed CR0, CR3, CR4, or CR8.
 * Qualification bits tell us which CR and operation type.
 */

/* Exit qualification bits for CR access */
#define CR_ACCESS_CR_NUM(q)     ((q) & 0xF)
#define CR_ACCESS_TYPE(q)       (((q) >> 4) & 0x3)
#define CR_ACCESS_REG(q)        (((q) >> 8) & 0xF)

#define CR_ACCESS_TYPE_MOV_TO_CR    0
#define CR_ACCESS_TYPE_MOV_FROM_CR  1
#define CR_ACCESS_TYPE_CLTS         2
#define CR_ACCESS_TYPE_LMSW         3

static VMEXIT_ACTION HandleCrAccess(GUEST_REGS* regs, U64 qualification)
{
    U32 crNum = CR_ACCESS_CR_NUM(qualification);
    U32 accessType = CR_ACCESS_TYPE(qualification);
    U32 gpReg = CR_ACCESS_REG(qualification);
    U64* regPtr;
    U64 value;
    
    /* Get pointer to the GPR specified in qualification */
    switch (gpReg) {
    case 0: regPtr = &regs->Rax; break;
    case 1: regPtr = &regs->Rcx; break;
    case 2: regPtr = &regs->Rdx; break;
    case 3: regPtr = &regs->Rbx; break;
    case 4: regPtr = &regs->Rsp; break;
    case 5: regPtr = &regs->Rbp; break;
    case 6: regPtr = &regs->Rsi; break;
    case 7: regPtr = &regs->Rdi; break;
    case 8: regPtr = &regs->R8;  break;
    case 9: regPtr = &regs->R9;  break;
    case 10: regPtr = &regs->R10; break;
    case 11: regPtr = &regs->R11; break;
    case 12: regPtr = &regs->R12; break;
    case 13: regPtr = &regs->R13; break;
    case 14: regPtr = &regs->R14; break;
    case 15: regPtr = &regs->R15; break;
    default: regPtr = &regs->Rax; break;
    }
    
    if (accessType == CR_ACCESS_TYPE_MOV_TO_CR) {
        /* MOV to CR */
        value = *regPtr;
        
        switch (crNum) {
        case 0:
            VmcsWrite(VMCS_GUEST_CR0, value);
            VmcsWrite(VMCS_CTRL_CR0_SHADOW, value);
            break;
        case 3:
            VmcsWrite(VMCS_GUEST_CR3, value);
            break;
        case 4:
            /* For stealth: hide CR4.VMXE from shadow */
            VmcsWrite(VMCS_GUEST_CR4, value);
            VmcsWrite(VMCS_CTRL_CR4_SHADOW, value & ~CR4_VMXE);
            break;
        }
    } else if (accessType == CR_ACCESS_TYPE_MOV_FROM_CR) {
        /* MOV from CR */
        switch (crNum) {
        case 0:
            *regPtr = VmcsRead(VMCS_GUEST_CR0);
            break;
        case 3:
            *regPtr = VmcsRead(VMCS_GUEST_CR3);
            break;
        case 4:
            /* Return shadow to hide VMXE */
            *regPtr = VmcsRead(VMCS_CTRL_CR4_SHADOW);
            break;
        }
    }
    
    return VMEXIT_ADVANCE_RIP;
}

/*===========================================================================
 * CONCEPT 7: VMCALL Handler
 *===========================================================================
 * VMCALL is our hypercall mechanism. Use magic + command protocol.
 */

#define OMBRA_VMCALL_MAGIC      0x4F4D4252414C4C00ULL

#define VMCALL_PING             0x0001
#define VMCALL_UNLOAD           0x0002
#define VMCALL_READ_VIRT        0xD4F83A19
#define VMCALL_WRITE_VIRT       0x8B2E57C6

#define VMCALL_STATUS_SUCCESS           0
#define VMCALL_STATUS_INVALID_MAGIC     -1
#define VMCALL_STATUS_INVALID_COMMAND   -2

static VMEXIT_ACTION HandleVmcall(GUEST_REGS* regs)
{
    U64 magic = regs->Rax;
    U64 command = regs->Rcx;
    I64 status = VMCALL_STATUS_SUCCESS;
    
    /* Check magic signature */
    if (magic != OMBRA_VMCALL_MAGIC) {
        regs->Rax = (U64)VMCALL_STATUS_INVALID_MAGIC;
        return VMEXIT_ADVANCE_RIP;
    }
    
    /* Security: Check caller is Ring 0 */
    U64 guestCs = VmcsRead(VMCS_GUEST_CS_SEL);
    if ((guestCs & 0x3) != 0) {
        /* Not ring 0 - reject */
        regs->Rax = (U64)VMCALL_STATUS_INVALID_MAGIC;
        return VMEXIT_ADVANCE_RIP;
    }
    
    /* Dispatch command */
    switch (command) {
    case VMCALL_PING:
        regs->Rdx = 0x00010000;  /* Version 1.0 */
        break;
        
    case VMCALL_UNLOAD:
        regs->Rax = VMCALL_STATUS_SUCCESS;
        return VMEXIT_SHUTDOWN;
        
    case VMCALL_READ_VIRT:
    case VMCALL_WRITE_VIRT:
        /* Memory operations - see 04_memory_ops.c */
        status = VMCALL_STATUS_INVALID_COMMAND;
        break;
        
    default:
        status = VMCALL_STATUS_INVALID_COMMAND;
        break;
    }
    
    regs->Rax = (U64)status;
    return VMEXIT_ADVANCE_RIP;
}

/*===========================================================================
 * CONCEPT 8: Main VMExit Dispatcher
 *===========================================================================
 * Called from assembly stub after saving GPRs.
 * Returns action indicating how to resume.
 */

/**
 * Main VMExit dispatch function
 * 
 * @param regs  Pointer to saved guest registers (on stack)
 * @return Action to take (continue, advance RIP, shutdown)
 */
VMEXIT_ACTION VmexitDispatch(GUEST_REGS* regs)
{
    VMEXIT_INFO info;
    VMEXIT_ACTION action = VMEXIT_ADVANCE_RIP;
    
    /* Read exit information */
    VmexitReadInfo(&info);
    
    /* Handle VM-entry failure */
    if (info.VmEntryFailure) {
        /* Critical error - debug and shutdown */
        return VMEXIT_SHUTDOWN;
    }
    
    /* Dispatch based on exit reason */
    switch (info.Reason) {
    case EXIT_REASON_CPUID:
        action = HandleCpuid(regs);
        break;
        
    case EXIT_REASON_MSR_READ:
        action = HandleMsrRead(regs);
        break;
        
    case EXIT_REASON_MSR_WRITE:
        action = HandleMsrWrite(regs);
        break;
        
    case EXIT_REASON_CR_ACCESS:
        action = HandleCrAccess(regs, info.Qualification);
        break;
        
    case EXIT_REASON_VMCALL:
        action = HandleVmcall(regs);
        break;
        
    case EXIT_REASON_EPT_VIOLATION:
        /* See ept_npt/04_ept_violation.c */
        action = VMEXIT_CONTINUE;
        break;
        
    case EXIT_REASON_RDTSC:
        /* TSC handling for stealth - see antidetection/03_timing.c */
        regs->Rax = (U32)__rdtsc();
        regs->Rdx = (U32)(__rdtsc() >> 32);
        action = VMEXIT_ADVANCE_RIP;
        break;
        
    case EXIT_REASON_RDTSCP:
        {
            U64 tsc = __rdtsc();
            regs->Rax = (U32)tsc;
            regs->Rdx = (U32)(tsc >> 32);
            regs->Rcx = (U32)__readmsr(MSR_IA32_TSC_AUX);
        }
        action = VMEXIT_ADVANCE_RIP;
        break;
        
    case EXIT_REASON_HLT:
        /* Let guest halt - just resume */
        action = VMEXIT_ADVANCE_RIP;
        break;
        
    case EXIT_REASON_EXTERNAL_INT:
    case EXIT_REASON_NMI_WINDOW:
        /* Handle or re-inject interrupts */
        action = VMEXIT_CONTINUE;
        break;
        
    default:
        /* Unhandled exit - advance RIP and hope for the best */
        /* In production, log and potentially inject #UD */
        action = VMEXIT_ADVANCE_RIP;
        break;
    }
    
    /* Advance RIP if needed */
    if (action == VMEXIT_ADVANCE_RIP) {
        VmexitAdvanceRip();
    }
    
    return action;
}
