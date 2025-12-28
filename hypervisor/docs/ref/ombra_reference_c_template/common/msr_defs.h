/**
 * msr_defs.h - Model Specific Register Addresses
 * 
 * Reference: Intel SDM Volume 4, Chapter 2
 * Pure C11 - NO C++ constructs
 */

#ifndef OMBRA_MSR_DEFS_H
#define OMBRA_MSR_DEFS_H

#include "types.h"

/*===========================================================================
 * Feature Control MSRs
 *===========================================================================*/
#define MSR_IA32_FEATURE_CONTROL            0x0000003A

/* IA32_FEATURE_CONTROL Bits */
#define FEATURE_CONTROL_LOCK                BIT(0)
#define FEATURE_CONTROL_VMX_SMX             BIT(1)
#define FEATURE_CONTROL_VMX_OUTSIDE_SMX     BIT(2)

/*===========================================================================
 * VMX Capability MSRs
 *===========================================================================*/
#define MSR_IA32_VMX_BASIC                  0x00000480
#define MSR_IA32_VMX_PINBASED_CTLS          0x00000481
#define MSR_IA32_VMX_PROCBASED_CTLS         0x00000482
#define MSR_IA32_VMX_EXIT_CTLS              0x00000483
#define MSR_IA32_VMX_ENTRY_CTLS             0x00000484
#define MSR_IA32_VMX_MISC                   0x00000485
#define MSR_IA32_VMX_CR0_FIXED0             0x00000486
#define MSR_IA32_VMX_CR0_FIXED1             0x00000487
#define MSR_IA32_VMX_CR4_FIXED0             0x00000488
#define MSR_IA32_VMX_CR4_FIXED1             0x00000489
#define MSR_IA32_VMX_VMCS_ENUM              0x0000048A
#define MSR_IA32_VMX_PROCBASED_CTLS2        0x0000048B
#define MSR_IA32_VMX_EPT_VPID_CAP           0x0000048C
#define MSR_IA32_VMX_TRUE_PINBASED_CTLS     0x0000048D
#define MSR_IA32_VMX_TRUE_PROCBASED_CTLS    0x0000048E
#define MSR_IA32_VMX_TRUE_EXIT_CTLS         0x0000048F
#define MSR_IA32_VMX_TRUE_ENTRY_CTLS        0x00000490
#define MSR_IA32_VMX_VMFUNC                 0x00000491

/*===========================================================================
 * IA32_VMX_BASIC Fields
 *===========================================================================*/
#define VMX_BASIC_REVISION_MASK             0x7FFFFFFFULL
#define VMX_BASIC_VMCS_SIZE_SHIFT           32
#define VMX_BASIC_VMCS_SIZE_MASK            0x1FFF
#define VMX_BASIC_PHYS_ADDR_WIDTH           BIT(48)
#define VMX_BASIC_DUAL_MONITOR              BIT(49)
#define VMX_BASIC_MEMORY_TYPE_SHIFT         50
#define VMX_BASIC_MEMORY_TYPE_MASK          0x0F
#define VMX_BASIC_TRUE_CTLS                 BIT(55)

/*===========================================================================
 * IA32_VMX_EPT_VPID_CAP Fields
 *===========================================================================*/
#define EPT_CAP_EXEC_ONLY                   BIT(0)
#define EPT_CAP_PAGE_WALK_4                 BIT(6)
#define EPT_CAP_UC_MEMORY                   BIT(8)
#define EPT_CAP_WB_MEMORY                   BIT(14)
#define EPT_CAP_2MB_PAGE                    BIT(16)
#define EPT_CAP_1GB_PAGE                    BIT(17)
#define EPT_CAP_INVEPT                      BIT(20)
#define EPT_CAP_ACCESSED_DIRTY              BIT(21)
#define EPT_CAP_ADV_EPT_INFO                BIT(22)
#define EPT_CAP_INVEPT_SINGLE               BIT(25)
#define EPT_CAP_INVEPT_ALL                  BIT(26)
#define VPID_CAP_INVVPID                    BIT(32)
#define VPID_CAP_INVVPID_INDIVIDUAL         BIT(40)
#define VPID_CAP_INVVPID_SINGLE             BIT(41)
#define VPID_CAP_INVVPID_ALL                BIT(42)
#define VPID_CAP_INVVPID_SINGLE_GLOBAL      BIT(43)

/*===========================================================================
 * System MSRs
 *===========================================================================*/
#define MSR_IA32_SYSENTER_CS                0x00000174
#define MSR_IA32_SYSENTER_ESP               0x00000175
#define MSR_IA32_SYSENTER_EIP               0x00000176
#define MSR_IA32_DEBUGCTL                   0x000001D9
#define MSR_IA32_PAT                        0x00000277
#define MSR_IA32_PERF_GLOBAL_CTRL           0x0000038F
#define MSR_IA32_EFER                       0xC0000080
#define MSR_IA32_STAR                       0xC0000081
#define MSR_IA32_LSTAR                      0xC0000082
#define MSR_IA32_CSTAR                      0xC0000083
#define MSR_IA32_SFMASK                     0xC0000084
#define MSR_IA32_FS_BASE                    0xC0000100
#define MSR_IA32_GS_BASE                    0xC0000101
#define MSR_IA32_KERNEL_GS_BASE             0xC0000102
#define MSR_IA32_TSC_AUX                    0xC0000103

/*===========================================================================
 * IA32_EFER Bits
 *===========================================================================*/
#define EFER_SCE                            BIT(0)  /* SYSCALL Enable */
#define EFER_LME                            BIT(8)  /* Long Mode Enable */
#define EFER_LMA                            BIT(10) /* Long Mode Active */
#define EFER_NXE                            BIT(11) /* No-Execute Enable */
#define EFER_SVME                           BIT(12) /* SVM Enable (AMD) */
#define EFER_LMSLE                          BIT(13) /* Long Mode Segment Limit Enable */
#define EFER_FFXSR                          BIT(14) /* Fast FXSAVE/FXRSTOR */
#define EFER_TCE                            BIT(15) /* Translation Cache Extension */

/*===========================================================================
 * CR0 Bits
 *===========================================================================*/
#define CR0_PE                              BIT(0)  /* Protected Mode Enable */
#define CR0_MP                              BIT(1)  /* Monitor Co-processor */
#define CR0_EM                              BIT(2)  /* Emulation */
#define CR0_TS                              BIT(3)  /* Task Switched */
#define CR0_ET                              BIT(4)  /* Extension Type */
#define CR0_NE                              BIT(5)  /* Numeric Error */
#define CR0_WP                              BIT(16) /* Write Protect */
#define CR0_AM                              BIT(18) /* Alignment Mask */
#define CR0_NW                              BIT(29) /* Not Write-through */
#define CR0_CD                              BIT(30) /* Cache Disable */
#define CR0_PG                              BIT(31) /* Paging */

/*===========================================================================
 * CR4 Bits
 *===========================================================================*/
#define CR4_VME                             BIT(0)  /* Virtual 8086 Mode Extensions */
#define CR4_PVI                             BIT(1)  /* Protected Virtual Interrupts */
#define CR4_TSD                             BIT(2)  /* Time Stamp Disable */
#define CR4_DE                              BIT(3)  /* Debugging Extensions */
#define CR4_PSE                             BIT(4)  /* Page Size Extension */
#define CR4_PAE                             BIT(5)  /* Physical Address Extension */
#define CR4_MCE                             BIT(6)  /* Machine Check Exception */
#define CR4_PGE                             BIT(7)  /* Page Global Enable */
#define CR4_PCE                             BIT(8)  /* Performance Counter Enable */
#define CR4_OSFXSR                          BIT(9)  /* OS FXSAVE/FXRSTOR Support */
#define CR4_OSXMMEXCPT                      BIT(10) /* OS Unmasked SIMD FP Exceptions */
#define CR4_UMIP                            BIT(11) /* User Mode Instruction Prevention */
#define CR4_VMXE                            BIT(13) /* VMX Enable */
#define CR4_SMXE                            BIT(14) /* SMX Enable */
#define CR4_FSGSBASE                        BIT(16) /* FSGSBASE Enable */
#define CR4_PCIDE                           BIT(17) /* PCID Enable */
#define CR4_OSXSAVE                         BIT(18) /* XSAVE Enable */
#define CR4_SMEP                            BIT(20) /* Supervisor Mode Exec Protection */
#define CR4_SMAP                            BIT(21) /* Supervisor Mode Access Prevention */
#define CR4_PKE                             BIT(22) /* Protection Key Enable */

/*===========================================================================
 * MTRR MSRs (Memory Type Range Registers)
 *===========================================================================*/
#define MSR_IA32_MTRRCAP                    0x000000FE
#define MSR_IA32_MTRR_DEF_TYPE              0x000002FF
#define MSR_IA32_MTRR_PHYSBASE0             0x00000200
#define MSR_IA32_MTRR_PHYSMASK0             0x00000201
/* MTRR pairs continue: 0x202/0x203, 0x204/0x205, etc. */

/* Memory Types */
#define MTRR_TYPE_UC                        0   /* Uncacheable */
#define MTRR_TYPE_WC                        1   /* Write Combining */
#define MTRR_TYPE_WT                        4   /* Write Through */
#define MTRR_TYPE_WP                        5   /* Write Protected */
#define MTRR_TYPE_WB                        6   /* Write Back */

/*===========================================================================
 * Helper Functions
 *===========================================================================*/

/**
 * Read VMX capability MSR and extract allowed-0 and allowed-1 bits
 * 
 * @param msr       The capability MSR to read
 * @param allowed0  Output: bits that must be 0
 * @param allowed1  Output: bits that must be 1
 */
static inline void ReadVmxCapabilityMsr(U32 msr, U32* allowed0, U32* allowed1)
{
    U64 value = __readmsr(msr);
    *allowed0 = (U32)(value >> 32);  /* High 32 bits: allowed-1 */
    *allowed1 = (U32)(value);         /* Low 32 bits: allowed-0 */
}

/**
 * Adjust a VMX control value based on capability MSR
 * Sets bits that must be 1, clears bits that must be 0
 * 
 * @param value     Desired control value
 * @param msr       The capability MSR (or TRUE capability MSR)
 * @return          Adjusted value with required bits set
 */
static inline U32 AdjustVmxControl(U32 value, U32 msr)
{
    U64 cap = __readmsr(msr);
    U32 allowed0 = (U32)cap;         /* Low: bits that CAN be 0 */
    U32 allowed1 = (U32)(cap >> 32); /* High: bits that CAN be 1 */
    
    /* Set bits that must be 1 (not in allowed0) */
    value |= ~allowed0;
    
    /* Clear bits that must be 0 (not in allowed1) */
    value &= allowed1;
    
    return value;
}

/**
 * Check if TRUE capability MSRs should be used
 * 
 * @return true if IA32_VMX_BASIC bit 55 is set
 */
static inline bool UseTrueVmxControls(void)
{
    U64 basic = __readmsr(MSR_IA32_VMX_BASIC);
    return (basic & VMX_BASIC_TRUE_CTLS) != 0;
}

/**
 * Get VMCS revision ID from IA32_VMX_BASIC
 * 
 * @return 31-bit revision ID (bit 31 must be 0 in VMXON/VMCS regions)
 */
static inline U32 GetVmcsRevisionId(void)
{
    U64 basic = __readmsr(MSR_IA32_VMX_BASIC);
    return (U32)(basic & VMX_BASIC_REVISION_MASK);
}

#endif /* OMBRA_MSR_DEFS_H */
