/**
 * 02_vmcs_setup.c - VMCS Configuration Reference (Complete)
 * 
 * Pure C implementation of VMCS field setup.
 * Reference: Intel SDM Volume 3, Chapter 24
 */

#include "../common/types.h"
#include "../common/vmcs_defs.h"
#include "../common/msr_defs.h"

/* Assembly functions declared externally */
extern VMX_RESULT AsmVmclear(U64 vmcsPhysical);
extern VMX_RESULT AsmVmptrld(U64 vmcsPhysical);
extern VMX_RESULT AsmVmwrite(U64 field, U64 value);
extern VMX_RESULT AsmVmread(U64 field, U64* value);
extern void AsmReadGdtr(void* gdtr);
extern void AsmReadIdtr(void* idtr);
extern U16  AsmReadTr(void);
extern U16  AsmReadLdtr(void);

#define VmcsWrite(field, value)  AsmVmwrite((field), (value))

typedef struct _DESCRIPTOR_TABLE_REG {
    U16 Limit;
    U64 Base;
} __attribute__((packed)) DESCRIPTOR_TABLE_REG;

/*===========================================================================
 * Control Adjustment Helper
 *===========================================================================*/
static U32 AdjustControls(U32 desiredValue, U32 capabilityMsr)
{
    U64 capability = __readmsr(capabilityMsr);
    U32 allowed0 = (U32)(capability & 0xFFFFFFFF);
    U32 allowed1 = (U32)(capability >> 32);
    U32 result = desiredValue;
    result |= ~allowed0;
    result &= allowed1;
    return result;
}

/*===========================================================================
 * VMCS Initialization
 *===========================================================================*/
OMBRA_STATUS VmcsInitialize(void* vmcsVirtual, U64 vmcsPhysical)
{
    U32 revisionId = GetVmcsRevisionId();
    U8* region = (U8*)vmcsVirtual;
    U32 i;
    VMX_RESULT result;
    
    for (i = 0; i < PAGE_SIZE; i++) region[i] = 0;
    *(U32*)vmcsVirtual = revisionId & 0x7FFFFFFF;
    
    result = AsmVmclear(vmcsPhysical);
    if (result != VMX_OK) return OMBRA_ERROR_VMCS_FAILED;
    
    result = AsmVmptrld(vmcsPhysical);
    if (result != VMX_OK) return OMBRA_ERROR_VMCS_FAILED;
    
    return OMBRA_SUCCESS;
}

/*===========================================================================
 * Control Field Setup
 *===========================================================================*/
static void VmcsSetupControls(U64 eptPointer, U64 msrBitmapPhys)
{
    U32 pinBased, procBased, procBased2, exitCtls, entryCtls;
    U32 pinMsr, procMsr, exitMsr, entryMsr;
    bool useTrueCtls = UseTrueVmxControls();
    
    /* Select MSRs based on TRUE controls support */
    pinMsr = useTrueCtls ? MSR_IA32_VMX_TRUE_PINBASED_CTLS : MSR_IA32_VMX_PINBASED_CTLS;
    procMsr = useTrueCtls ? MSR_IA32_VMX_TRUE_PROCBASED_CTLS : MSR_IA32_VMX_PROCBASED_CTLS;
    exitMsr = useTrueCtls ? MSR_IA32_VMX_TRUE_EXIT_CTLS : MSR_IA32_VMX_EXIT_CTLS;
    entryMsr = useTrueCtls ? MSR_IA32_VMX_TRUE_ENTRY_CTLS : MSR_IA32_VMX_ENTRY_CTLS;
    
    /* Pin-based controls */
    pinBased = PIN_EXTERNAL_INT_EXIT | PIN_NMI_EXIT;
    pinBased = AdjustControls(pinBased, pinMsr);
    VmcsWrite(VMCS_CTRL_PIN_BASED, pinBased);
    
    /* Primary processor-based controls */
    procBased = CPU_MSR_BITMAP | CPU_SECONDARY_CONTROLS | CPU_TSC_OFFSETTING;
    procBased = AdjustControls(procBased, procMsr);
    VmcsWrite(VMCS_CTRL_PROC_BASED, procBased);
    
    /* Secondary processor-based controls */
    procBased2 = CPU2_EPT | CPU2_VPID | CPU2_RDTSCP | CPU2_UNRESTRICTED_GUEST;
    procBased2 = AdjustControls(procBased2, MSR_IA32_VMX_PROCBASED_CTLS2);
    VmcsWrite(VMCS_CTRL_PROC_BASED2, procBased2);
    
    /* VM-exit controls */
    exitCtls = EXIT_HOST_ADDR_SPACE_SIZE | EXIT_SAVE_EFER | EXIT_LOAD_EFER;
    exitCtls = AdjustControls(exitCtls, exitMsr);
    VmcsWrite(VMCS_CTRL_VMEXIT, exitCtls);
    
    /* VM-entry controls */
    entryCtls = ENTRY_IA32E_MODE | ENTRY_LOAD_EFER;
    entryCtls = AdjustControls(entryCtls, entryMsr);
    VmcsWrite(VMCS_CTRL_VMENTRY, entryCtls);
    
    /* EPT and VPID */
    VmcsWrite(VMCS_CTRL_EPT_POINTER, eptPointer);
    VmcsWrite(VMCS_CTRL_VPID, 1);
    
    /* MSR bitmap and TSC offset */
    VmcsWrite(VMCS_CTRL_MSR_BITMAP, msrBitmapPhys);
    VmcsWrite(VMCS_CTRL_TSC_OFFSET, 0);
    
    /* CR masks (0 = don't intercept) */
    VmcsWrite(VMCS_CTRL_CR0_MASK, 0);
    VmcsWrite(VMCS_CTRL_CR4_MASK, 0);
    VmcsWrite(VMCS_CTRL_CR0_SHADOW, __readcr0());
    VmcsWrite(VMCS_CTRL_CR4_SHADOW, __readcr4());
    
    /* Exception bitmap (0 = don't intercept) */
    VmcsWrite(VMCS_CTRL_EXCEPTION_BITMAP, 0);
}

/*===========================================================================
 * Guest State Setup
 *===========================================================================*/
static void VmcsSetupGuestState(U64 guestRsp, U64 guestRip)
{
    DESCRIPTOR_TABLE_REG gdtr, idtr;
    U16 tr;
    
    AsmReadGdtr(&gdtr);
    AsmReadIdtr(&idtr);
    tr = AsmReadTr();
    
    /* Control registers */
    VmcsWrite(VMCS_GUEST_CR0, __readcr0());
    VmcsWrite(VMCS_GUEST_CR3, __readcr3());
    VmcsWrite(VMCS_GUEST_CR4, __readcr4());
    VmcsWrite(VMCS_GUEST_DR7, 0x400);
    
    /* RSP, RIP, RFLAGS */
    VmcsWrite(VMCS_GUEST_RSP, guestRsp);
    VmcsWrite(VMCS_GUEST_RIP, guestRip);
    VmcsWrite(VMCS_GUEST_RFLAGS, 0x2);  /* Bit 1 must be set */
    
    /* Segment registers - simplified (use actual values in production) */
    VmcsWrite(VMCS_GUEST_CS_SEL, 0x10);
    VmcsWrite(VMCS_GUEST_CS_BASE, 0);
    VmcsWrite(VMCS_GUEST_CS_LIMIT, 0xFFFFFFFF);
    VmcsWrite(VMCS_GUEST_CS_ACCESS, 0xA09B);
    
    VmcsWrite(VMCS_GUEST_SS_SEL, 0x18);
    VmcsWrite(VMCS_GUEST_SS_BASE, 0);
    VmcsWrite(VMCS_GUEST_SS_LIMIT, 0xFFFFFFFF);
    VmcsWrite(VMCS_GUEST_SS_ACCESS, 0xC093);
    
    VmcsWrite(VMCS_GUEST_DS_SEL, 0x2B);
    VmcsWrite(VMCS_GUEST_DS_BASE, 0);
    VmcsWrite(VMCS_GUEST_DS_LIMIT, 0xFFFFFFFF);
    VmcsWrite(VMCS_GUEST_DS_ACCESS, 0xC093);
    
    VmcsWrite(VMCS_GUEST_ES_SEL, 0x2B);
    VmcsWrite(VMCS_GUEST_ES_BASE, 0);
    VmcsWrite(VMCS_GUEST_ES_LIMIT, 0xFFFFFFFF);
    VmcsWrite(VMCS_GUEST_ES_ACCESS, 0xC093);
    
    VmcsWrite(VMCS_GUEST_FS_SEL, 0x53);
    VmcsWrite(VMCS_GUEST_FS_BASE, __readmsr(MSR_IA32_FS_BASE));
    VmcsWrite(VMCS_GUEST_FS_LIMIT, 0x3C00);
    VmcsWrite(VMCS_GUEST_FS_ACCESS, 0x40F3);
    
    VmcsWrite(VMCS_GUEST_GS_SEL, 0x2B);
    VmcsWrite(VMCS_GUEST_GS_BASE, __readmsr(MSR_IA32_GS_BASE));
    VmcsWrite(VMCS_GUEST_GS_LIMIT, 0xFFFFFFFF);
    VmcsWrite(VMCS_GUEST_GS_ACCESS, 0xC093);
    
    /* TR - must be busy TSS (type 0x0B) */
    VmcsWrite(VMCS_GUEST_TR_SEL, tr);
    VmcsWrite(VMCS_GUEST_TR_BASE, 0);  /* Get from GDT in production */
    VmcsWrite(VMCS_GUEST_TR_LIMIT, 0x67);
    VmcsWrite(VMCS_GUEST_TR_ACCESS, 0x008B);  /* Busy 64-bit TSS */
    
    /* LDTR - unusable */
    VmcsWrite(VMCS_GUEST_LDTR_SEL, 0);
    VmcsWrite(VMCS_GUEST_LDTR_BASE, 0);
    VmcsWrite(VMCS_GUEST_LDTR_LIMIT, 0);
    VmcsWrite(VMCS_GUEST_LDTR_ACCESS, SEG_ACCESS_UNUSABLE);
    
    /* GDTR / IDTR */
    VmcsWrite(VMCS_GUEST_GDTR_BASE, gdtr.Base);
    VmcsWrite(VMCS_GUEST_GDTR_LIMIT, gdtr.Limit);
    VmcsWrite(VMCS_GUEST_IDTR_BASE, idtr.Base);
    VmcsWrite(VMCS_GUEST_IDTR_LIMIT, idtr.Limit);
    
    /* MSRs */
    VmcsWrite(VMCS_GUEST_SYSENTER_CS, __readmsr(MSR_IA32_SYSENTER_CS));
    VmcsWrite(VMCS_GUEST_SYSENTER_ESP, __readmsr(MSR_IA32_SYSENTER_ESP));
    VmcsWrite(VMCS_GUEST_SYSENTER_EIP, __readmsr(MSR_IA32_SYSENTER_EIP));
    VmcsWrite(VMCS_GUEST_EFER, __readmsr(MSR_IA32_EFER));
    
    /* VMCS link pointer (no nested VMX) */
    VmcsWrite(VMCS_GUEST_VMCS_LINK, 0xFFFFFFFFFFFFFFFF);
    
    /* Activity and interruptibility */
    VmcsWrite(VMCS_GUEST_ACTIVITY, GUEST_ACTIVITY_ACTIVE);
    VmcsWrite(VMCS_GUEST_INTERRUPTIBILITY, 0);
    VmcsWrite(VMCS_GUEST_PENDING_DEBUG, 0);
}

/*===========================================================================
 * Host State Setup
 *===========================================================================*/
static void VmcsSetupHostState(U64 hostRsp, U64 hostRip)
{
    DESCRIPTOR_TABLE_REG gdtr, idtr;
    
    AsmReadGdtr(&gdtr);
    AsmReadIdtr(&idtr);
    
    /* Control registers */
    VmcsWrite(VMCS_HOST_CR0, __readcr0());
    VmcsWrite(VMCS_HOST_CR3, __readcr3());
    VmcsWrite(VMCS_HOST_CR4, __readcr4());
    
    /* Segment selectors (RPL = 0) */
    VmcsWrite(VMCS_HOST_CS_SEL, 0x10);
    VmcsWrite(VMCS_HOST_SS_SEL, 0x18);
    VmcsWrite(VMCS_HOST_DS_SEL, 0x2B);
    VmcsWrite(VMCS_HOST_ES_SEL, 0x2B);
    VmcsWrite(VMCS_HOST_FS_SEL, 0x53);
    VmcsWrite(VMCS_HOST_GS_SEL, 0x2B);
    VmcsWrite(VMCS_HOST_TR_SEL, AsmReadTr() & ~0x7);
    
    /* Base addresses */
    VmcsWrite(VMCS_HOST_FS_BASE, __readmsr(MSR_IA32_FS_BASE));
    VmcsWrite(VMCS_HOST_GS_BASE, __readmsr(MSR_IA32_GS_BASE));
    VmcsWrite(VMCS_HOST_TR_BASE, 0);  /* Get from GDT in production */
    VmcsWrite(VMCS_HOST_GDTR_BASE, gdtr.Base);
    VmcsWrite(VMCS_HOST_IDTR_BASE, idtr.Base);
    
    /* MSRs */
    VmcsWrite(VMCS_HOST_SYSENTER_CS, __readmsr(MSR_IA32_SYSENTER_CS));
    VmcsWrite(VMCS_HOST_SYSENTER_ESP, __readmsr(MSR_IA32_SYSENTER_ESP));
    VmcsWrite(VMCS_HOST_SYSENTER_EIP, __readmsr(MSR_IA32_SYSENTER_EIP));
    VmcsWrite(VMCS_HOST_EFER, __readmsr(MSR_IA32_EFER));
    
    /* RSP and RIP - VMExit handler */
    VmcsWrite(VMCS_HOST_RSP, hostRsp);
    VmcsWrite(VMCS_HOST_RIP, hostRip);
}

/*===========================================================================
 * Complete VMCS Setup
 *===========================================================================*/
OMBRA_STATUS VmcsSetupAll(
    VMX_CPU* cpu,
    U64 guestRsp,
    U64 guestRip,
    U64 hostRsp,
    U64 hostRip,
    U64 eptPointer,
    U64 msrBitmapPhys)
{
    OMBRA_STATUS status;
    
    status = VmcsInitialize(cpu->VmcsRegion, cpu->VmcsPhysical);
    if (OMBRA_FAILED(status)) return status;
    
    VmcsSetupControls(eptPointer, msrBitmapPhys);
    VmcsSetupGuestState(guestRsp, guestRip);
    VmcsSetupHostState(hostRsp, hostRip);
    
    cpu->VmcsActive = true;
    return OMBRA_SUCCESS;
}
