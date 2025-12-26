// vmcs_fields.h — VMCS Field Encodings
// OmbraHypervisor
// Reference: Intel SDM Volume 3, Appendix B

#ifndef OMBRA_VMCS_FIELDS_H
#define OMBRA_VMCS_FIELDS_H

// =============================================================================
// VMCS Field Encoding Format (Intel SDM Vol 3, 24.11.2)
// =============================================================================
// Bits 0:0   - Access type (0 = full, 1 = high)
// Bits 9:1   - Index
// Bits 11:10 - Type (0 = control, 1 = exit info, 2 = guest, 3 = host)
// Bits 14:13 - Width (0 = 16-bit, 1 = 64-bit, 2 = 32-bit, 3 = natural)
// Bit 15     - Reserved

// =============================================================================
// 16-bit Control Fields
// =============================================================================
#define VMCS_CTRL_VPID                      0x0000
#define VMCS_CTRL_POSTED_INT_NOTIFY         0x0002
#define VMCS_CTRL_EPTP_INDEX                0x0004

// =============================================================================
// 16-bit Guest State Fields
// =============================================================================
#define VMCS_GUEST_ES_SEL                   0x0800
#define VMCS_GUEST_CS_SEL                   0x0802
#define VMCS_GUEST_SS_SEL                   0x0804
#define VMCS_GUEST_DS_SEL                   0x0806
#define VMCS_GUEST_FS_SEL                   0x0808
#define VMCS_GUEST_GS_SEL                   0x080A
#define VMCS_GUEST_LDTR_SEL                 0x080C
#define VMCS_GUEST_TR_SEL                   0x080E
#define VMCS_GUEST_INT_STATUS               0x0810
#define VMCS_GUEST_PML_INDEX                0x0812

// =============================================================================
// 16-bit Host State Fields
// =============================================================================
#define VMCS_HOST_ES_SEL                    0x0C00
#define VMCS_HOST_CS_SEL                    0x0C02
#define VMCS_HOST_SS_SEL                    0x0C04
#define VMCS_HOST_DS_SEL                    0x0C06
#define VMCS_HOST_FS_SEL                    0x0C08
#define VMCS_HOST_GS_SEL                    0x0C0A
#define VMCS_HOST_TR_SEL                    0x0C0C

// =============================================================================
// 64-bit Control Fields
// =============================================================================
#define VMCS_CTRL_IO_BITMAP_A               0x2000
#define VMCS_CTRL_IO_BITMAP_A_HIGH          0x2001
#define VMCS_CTRL_IO_BITMAP_B               0x2002
#define VMCS_CTRL_IO_BITMAP_B_HIGH          0x2003
#define VMCS_CTRL_MSR_BITMAP                0x2004
#define VMCS_CTRL_MSR_BITMAP_HIGH           0x2005
#define VMCS_CTRL_VMEXIT_MSR_STORE          0x2006
#define VMCS_CTRL_VMEXIT_MSR_STORE_HIGH     0x2007
#define VMCS_CTRL_VMEXIT_MSR_LOAD           0x2008
#define VMCS_CTRL_VMEXIT_MSR_LOAD_HIGH      0x2009
#define VMCS_CTRL_VMENTRY_MSR_LOAD          0x200A
#define VMCS_CTRL_VMENTRY_MSR_LOAD_HIGH     0x200B
#define VMCS_CTRL_EXECUTIVE_VMCS            0x200C
#define VMCS_CTRL_EXECUTIVE_VMCS_HIGH       0x200D
#define VMCS_CTRL_PML_ADDRESS               0x200E
#define VMCS_CTRL_PML_ADDRESS_HIGH          0x200F
#define VMCS_CTRL_TSC_OFFSET                0x2010
#define VMCS_CTRL_TSC_OFFSET_HIGH           0x2011
#define VMCS_CTRL_VIRTUAL_APIC              0x2012
#define VMCS_CTRL_VIRTUAL_APIC_HIGH         0x2013
#define VMCS_CTRL_APIC_ACCESS               0x2014
#define VMCS_CTRL_APIC_ACCESS_HIGH          0x2015
#define VMCS_CTRL_POSTED_INT_DESC           0x2016
#define VMCS_CTRL_POSTED_INT_DESC_HIGH      0x2017
#define VMCS_CTRL_VMFUNC_CONTROLS           0x2018
#define VMCS_CTRL_VMFUNC_CONTROLS_HIGH      0x2019
#define VMCS_CTRL_EPT_POINTER               0x201A
#define VMCS_CTRL_EPT_POINTER_HIGH          0x201B
#define VMCS_CTRL_EOI_EXIT_BITMAP_0         0x201C
#define VMCS_CTRL_EOI_EXIT_BITMAP_0_HIGH    0x201D
#define VMCS_CTRL_EOI_EXIT_BITMAP_1         0x201E
#define VMCS_CTRL_EOI_EXIT_BITMAP_1_HIGH    0x201F
#define VMCS_CTRL_EOI_EXIT_BITMAP_2         0x2020
#define VMCS_CTRL_EOI_EXIT_BITMAP_2_HIGH    0x2021
#define VMCS_CTRL_EOI_EXIT_BITMAP_3         0x2022
#define VMCS_CTRL_EOI_EXIT_BITMAP_3_HIGH    0x2023
#define VMCS_CTRL_EPTP_LIST                 0x2024
#define VMCS_CTRL_EPTP_LIST_HIGH            0x2025
#define VMCS_CTRL_VMREAD_BITMAP             0x2026
#define VMCS_CTRL_VMREAD_BITMAP_HIGH        0x2027
#define VMCS_CTRL_VMWRITE_BITMAP            0x2028
#define VMCS_CTRL_VMWRITE_BITMAP_HIGH       0x2029
#define VMCS_CTRL_VE_INFO_ADDRESS           0x202A
#define VMCS_CTRL_VE_INFO_ADDRESS_HIGH      0x202B
#define VMCS_CTRL_XSS_EXITING_BITMAP        0x202C
#define VMCS_CTRL_XSS_EXITING_BITMAP_HIGH   0x202D
#define VMCS_CTRL_ENCLS_EXITING_BITMAP      0x202E
#define VMCS_CTRL_ENCLS_EXITING_BITMAP_HIGH 0x202F
#define VMCS_CTRL_TSC_MULTIPLIER            0x2032
#define VMCS_CTRL_TSC_MULTIPLIER_HIGH       0x2033

// =============================================================================
// 64-bit Read-Only Data Fields
// =============================================================================
#define VMCS_EXIT_GUEST_PHYSICAL            0x2400
#define VMCS_EXIT_GUEST_PHYSICAL_HIGH       0x2401

// =============================================================================
// 64-bit Guest State Fields
// =============================================================================
#define VMCS_GUEST_VMCS_LINK                0x2800
#define VMCS_GUEST_VMCS_LINK_HIGH           0x2801
#define VMCS_GUEST_DEBUGCTL                 0x2802
#define VMCS_GUEST_DEBUGCTL_HIGH            0x2803
#define VMCS_GUEST_PAT                      0x2804
#define VMCS_GUEST_PAT_HIGH                 0x2805
#define VMCS_GUEST_EFER                     0x2806
#define VMCS_GUEST_EFER_HIGH                0x2807
#define VMCS_GUEST_PERF_GLOBAL_CTRL         0x2808
#define VMCS_GUEST_PERF_GLOBAL_CTRL_HIGH    0x2809
#define VMCS_GUEST_PDPTE0                   0x280A
#define VMCS_GUEST_PDPTE0_HIGH              0x280B
#define VMCS_GUEST_PDPTE1                   0x280C
#define VMCS_GUEST_PDPTE1_HIGH              0x280D
#define VMCS_GUEST_PDPTE2                   0x280E
#define VMCS_GUEST_PDPTE2_HIGH              0x280F
#define VMCS_GUEST_PDPTE3                   0x2810
#define VMCS_GUEST_PDPTE3_HIGH              0x2811
#define VMCS_GUEST_BNDCFGS                  0x2812
#define VMCS_GUEST_BNDCFGS_HIGH             0x2813

// =============================================================================
// 64-bit Host State Fields
// =============================================================================
#define VMCS_HOST_PAT                       0x2C00
#define VMCS_HOST_PAT_HIGH                  0x2C01
#define VMCS_HOST_EFER                      0x2C02
#define VMCS_HOST_EFER_HIGH                 0x2C03
#define VMCS_HOST_PERF_GLOBAL_CTRL          0x2C04
#define VMCS_HOST_PERF_GLOBAL_CTRL_HIGH     0x2C05

// =============================================================================
// 32-bit Control Fields
// =============================================================================
#define VMCS_CTRL_PIN_BASED                 0x4000
#define VMCS_CTRL_PROC_BASED                0x4002
#define VMCS_CTRL_EXCEPTION_BITMAP          0x4004
#define VMCS_CTRL_PAGE_FAULT_MASK           0x4006
#define VMCS_CTRL_PAGE_FAULT_MATCH          0x4008
#define VMCS_CTRL_CR3_TARGET_COUNT          0x400A
#define VMCS_CTRL_VMEXIT                    0x400C
#define VMCS_CTRL_VMEXIT_MSR_STORE_COUNT    0x400E
#define VMCS_CTRL_VMEXIT_MSR_LOAD_COUNT     0x4010
#define VMCS_CTRL_VMENTRY                   0x4012
#define VMCS_CTRL_VMENTRY_MSR_LOAD_COUNT    0x4014
#define VMCS_CTRL_VMENTRY_INT_INFO          0x4016
#define VMCS_CTRL_VMENTRY_EXCEPTION_CODE    0x4018
#define VMCS_CTRL_VMENTRY_INSTR_LEN         0x401A
#define VMCS_CTRL_TPR_THRESHOLD             0x401C
#define VMCS_CTRL_PROC_BASED2               0x401E
#define VMCS_CTRL_PLE_GAP                   0x4020
#define VMCS_CTRL_PLE_WINDOW                0x4022

// =============================================================================
// 32-bit Read-Only Data Fields
// =============================================================================
#define VMCS_EXIT_INSTRUCTION_ERROR         0x4400
#define VMCS_EXIT_REASON                    0x4402
#define VMCS_EXIT_INT_INFO                  0x4404
#define VMCS_EXIT_INT_ERROR_CODE            0x4406
#define VMCS_EXIT_IDT_VECTOR_INFO           0x4408
#define VMCS_EXIT_IDT_VECTOR_ERROR_CODE     0x440A
#define VMCS_EXIT_INSTR_LEN                 0x440C
#define VMCS_EXIT_INSTR_INFO                0x440E

// =============================================================================
// 32-bit Guest State Fields
// =============================================================================
#define VMCS_GUEST_ES_LIMIT                 0x4800
#define VMCS_GUEST_CS_LIMIT                 0x4802
#define VMCS_GUEST_SS_LIMIT                 0x4804
#define VMCS_GUEST_DS_LIMIT                 0x4806
#define VMCS_GUEST_FS_LIMIT                 0x4808
#define VMCS_GUEST_GS_LIMIT                 0x480A
#define VMCS_GUEST_LDTR_LIMIT               0x480C
#define VMCS_GUEST_TR_LIMIT                 0x480E
#define VMCS_GUEST_GDTR_LIMIT               0x4810
#define VMCS_GUEST_IDTR_LIMIT               0x4812
#define VMCS_GUEST_ES_ACCESS                0x4814
#define VMCS_GUEST_CS_ACCESS                0x4816
#define VMCS_GUEST_SS_ACCESS                0x4818
#define VMCS_GUEST_DS_ACCESS                0x481A
#define VMCS_GUEST_FS_ACCESS                0x481C
#define VMCS_GUEST_GS_ACCESS                0x481E
#define VMCS_GUEST_LDTR_ACCESS              0x4820
#define VMCS_GUEST_TR_ACCESS                0x4822
#define VMCS_GUEST_INTERRUPTIBILITY         0x4824
#define VMCS_GUEST_ACTIVITY                 0x4826
#define VMCS_GUEST_SMBASE                   0x4828
#define VMCS_GUEST_SYSENTER_CS              0x482A
#define VMCS_GUEST_PREEMPTION_TIMER         0x482E

// =============================================================================
// 32-bit Host State Fields
// =============================================================================
#define VMCS_HOST_SYSENTER_CS               0x4C00

// =============================================================================
// Natural-Width Control Fields
// =============================================================================
#define VMCS_CTRL_CR0_MASK                  0x6000
#define VMCS_CTRL_CR4_MASK                  0x6002
#define VMCS_CTRL_CR0_SHADOW                0x6004
#define VMCS_CTRL_CR4_SHADOW                0x6006
#define VMCS_CTRL_CR3_TARGET_0              0x6008
#define VMCS_CTRL_CR3_TARGET_1              0x600A
#define VMCS_CTRL_CR3_TARGET_2              0x600C
#define VMCS_CTRL_CR3_TARGET_3              0x600E

// =============================================================================
// Natural-Width Read-Only Data Fields
// =============================================================================
#define VMCS_EXIT_QUALIFICATION             0x6400
#define VMCS_EXIT_IO_RCX                    0x6402
#define VMCS_EXIT_IO_RSI                    0x6404
#define VMCS_EXIT_IO_RDI                    0x6406
#define VMCS_EXIT_IO_RIP                    0x6408
#define VMCS_EXIT_GUEST_LINEAR              0x640A

// =============================================================================
// Natural-Width Guest State Fields
// =============================================================================
#define VMCS_GUEST_CR0                      0x6800
#define VMCS_GUEST_CR3                      0x6802
#define VMCS_GUEST_CR4                      0x6804
#define VMCS_GUEST_ES_BASE                  0x6806
#define VMCS_GUEST_CS_BASE                  0x6808
#define VMCS_GUEST_SS_BASE                  0x680A
#define VMCS_GUEST_DS_BASE                  0x680C
#define VMCS_GUEST_FS_BASE                  0x680E
#define VMCS_GUEST_GS_BASE                  0x6810
#define VMCS_GUEST_LDTR_BASE                0x6812
#define VMCS_GUEST_TR_BASE                  0x6814
#define VMCS_GUEST_GDTR_BASE                0x6816
#define VMCS_GUEST_IDTR_BASE                0x6818
#define VMCS_GUEST_DR7                      0x681A
#define VMCS_GUEST_RSP                      0x681C
#define VMCS_GUEST_RIP                      0x681E
#define VMCS_GUEST_RFLAGS                   0x6820
#define VMCS_GUEST_PENDING_DEBUG            0x6822
#define VMCS_GUEST_SYSENTER_ESP             0x6824
#define VMCS_GUEST_SYSENTER_EIP             0x6826

// =============================================================================
// Natural-Width Host State Fields
// =============================================================================
#define VMCS_HOST_CR0                       0x6C00
#define VMCS_HOST_CR3                       0x6C02
#define VMCS_HOST_CR4                       0x6C04
#define VMCS_HOST_FS_BASE                   0x6C06
#define VMCS_HOST_GS_BASE                   0x6C08
#define VMCS_HOST_TR_BASE                   0x6C0A
#define VMCS_HOST_GDTR_BASE                 0x6C0C
#define VMCS_HOST_IDTR_BASE                 0x6C0E
#define VMCS_HOST_SYSENTER_ESP              0x6C10
#define VMCS_HOST_SYSENTER_EIP              0x6C12
#define VMCS_HOST_RSP                       0x6C14
#define VMCS_HOST_RIP                       0x6C16

// =============================================================================
// Pin-Based VM-Execution Controls (Intel SDM Vol 3, 24.6.1)
// =============================================================================
#define PIN_EXTERNAL_INT_EXIT               (1ULL << 0)
#define PIN_NMI_EXIT                        (1ULL << 3)
#define PIN_VIRTUAL_NMI                     (1ULL << 5)
#define PIN_PREEMPTION_TIMER                (1ULL << 6)
#define PIN_POSTED_INTERRUPTS               (1ULL << 7)

// =============================================================================
// Primary Processor-Based VM-Execution Controls (Intel SDM Vol 3, 24.6.2)
// =============================================================================
#define CPU_INT_WINDOW_EXIT                 (1ULL << 2)
#define CPU_TSC_OFFSETTING                  (1ULL << 3)
#define CPU_HLT_EXIT                        (1ULL << 7)
#define CPU_INVLPG_EXIT                     (1ULL << 9)
#define CPU_MWAIT_EXIT                      (1ULL << 10)
#define CPU_RDPMC_EXIT                      (1ULL << 11)
#define CPU_RDTSC_EXIT                      (1ULL << 12)
#define CPU_CR3_LOAD_EXIT                   (1ULL << 15)
#define CPU_CR3_STORE_EXIT                  (1ULL << 16)
#define CPU_CR8_LOAD_EXIT                   (1ULL << 19)
#define CPU_CR8_STORE_EXIT                  (1ULL << 20)
#define CPU_TPR_SHADOW                      (1ULL << 21)
#define CPU_NMI_WINDOW_EXIT                 (1ULL << 22)
#define CPU_MOV_DR_EXIT                     (1ULL << 23)
#define CPU_UNCOND_IO_EXIT                  (1ULL << 24)
#define CPU_IO_BITMAP                       (1ULL << 25)
#define CPU_MONITOR_TRAP                    (1ULL << 27)
#define CPU_MSR_BITMAP                      (1ULL << 28)
#define CPU_MONITOR_EXIT                    (1ULL << 29)
#define CPU_PAUSE_EXIT                      (1ULL << 30)
#define CPU_SECONDARY_CONTROLS              (1ULL << 31)

// =============================================================================
// Secondary Processor-Based VM-Execution Controls (Intel SDM Vol 3, 24.6.2)
// =============================================================================
#define CPU2_VIRTUALIZE_APIC                (1ULL << 0)
#define CPU2_EPT                            (1ULL << 1)
#define CPU2_DESC_TABLE_EXIT                (1ULL << 2)
#define CPU2_RDTSCP                         (1ULL << 3)
#define CPU2_VIRTUALIZE_X2APIC              (1ULL << 4)
#define CPU2_VPID                           (1ULL << 5)
#define CPU2_WBINVD_EXIT                    (1ULL << 6)
#define CPU2_UNRESTRICTED_GUEST             (1ULL << 7)
#define CPU2_APIC_REGISTER_VIRT             (1ULL << 8)
#define CPU2_VIRTUAL_INT_DELIVERY           (1ULL << 9)
#define CPU2_PAUSE_LOOP_EXIT                (1ULL << 10)
#define CPU2_RDRAND_EXIT                    (1ULL << 11)
#define CPU2_INVPCID                        (1ULL << 12)
#define CPU2_VMFUNC                         (1ULL << 13)
#define CPU2_VMCS_SHADOWING                 (1ULL << 14)
#define CPU2_ENCLS_EXIT                     (1ULL << 15)
#define CPU2_RDSEED_EXIT                    (1ULL << 16)
#define CPU2_PML                            (1ULL << 17)
#define CPU2_EPT_VE                         (1ULL << 18)
#define CPU2_CONCEAL_VMX                    (1ULL << 19)
#define CPU2_XSAVES                         (1ULL << 20)
#define CPU2_MODE_BASED_EPT                 (1ULL << 22)
#define CPU2_TSC_SCALING                    (1ULL << 25)

// =============================================================================
// VM-Exit Controls (Intel SDM Vol 3, 24.7.1)
// =============================================================================
#define EXIT_SAVE_DEBUG                     (1ULL << 2)
#define EXIT_HOST_ADDR_SPACE_SIZE           (1ULL << 9)
#define EXIT_LOAD_PERF_GLOBAL_CTRL          (1ULL << 12)
#define EXIT_ACK_INT_ON_EXIT                (1ULL << 15)
#define EXIT_SAVE_PAT                       (1ULL << 18)
#define EXIT_LOAD_PAT                       (1ULL << 19)
#define EXIT_SAVE_EFER                      (1ULL << 20)
#define EXIT_LOAD_EFER                      (1ULL << 21)
#define EXIT_SAVE_PREEMPTION_TIMER          (1ULL << 22)
#define EXIT_CLEAR_BNDCFGS                  (1ULL << 23)
#define EXIT_CONCEAL_VMX                    (1ULL << 24)

// =============================================================================
// VM-Entry Controls (Intel SDM Vol 3, 24.8.1)
// =============================================================================
#define ENTRY_LOAD_DEBUG                    (1ULL << 2)
#define ENTRY_IA32E_MODE                    (1ULL << 9)
#define ENTRY_SMM                           (1ULL << 10)
#define ENTRY_DEACT_DUAL_MONITOR            (1ULL << 11)
#define ENTRY_LOAD_PERF_GLOBAL_CTRL         (1ULL << 13)
#define ENTRY_LOAD_PAT                      (1ULL << 14)
#define ENTRY_LOAD_EFER                     (1ULL << 15)
#define ENTRY_LOAD_BNDCFGS                  (1ULL << 16)
#define ENTRY_CONCEAL_VMX                   (1ULL << 17)

// =============================================================================
// Segment Access Rights
// =============================================================================
#define SEG_ACCESS_ACCESSED                 (1 << 0)
#define SEG_ACCESS_WRITABLE                 (1 << 1)    // Data segments
#define SEG_ACCESS_READABLE                 (1 << 1)    // Code segments
#define SEG_ACCESS_EXPAND_DOWN              (1 << 2)    // Data segments
#define SEG_ACCESS_CONFORMING               (1 << 2)    // Code segments
#define SEG_ACCESS_CODE                     (1 << 3)
#define SEG_ACCESS_S                        (1 << 4)    // Descriptor type (0=system, 1=code/data)
#define SEG_ACCESS_DPL_SHIFT                5
#define SEG_ACCESS_DPL_MASK                 (3 << 5)
#define SEG_ACCESS_PRESENT                  (1 << 7)
#define SEG_ACCESS_AVL                      (1 << 12)
#define SEG_ACCESS_L                        (1 << 13)   // 64-bit mode
#define SEG_ACCESS_DB                       (1 << 14)   // Default size
#define SEG_ACCESS_G                        (1 << 15)   // Granularity
#define SEG_ACCESS_UNUSABLE                 (1 << 16)

// Common access rights values
#define SEG_ACCESS_CODE_R_X_A               0x209B      // Code, readable, accessed, present, DPL=0
#define SEG_ACCESS_DATA_RW_A                0x2093      // Data, read/write, accessed, present, DPL=0
#define SEG_ACCESS_TSS_BUSY                 0x008B      // TSS busy, present, DPL=0

// =============================================================================
// Guest Activity State
// =============================================================================
#define GUEST_ACTIVITY_ACTIVE               0
#define GUEST_ACTIVITY_HLT                  1
#define GUEST_ACTIVITY_SHUTDOWN             2
#define GUEST_ACTIVITY_WAIT_SIPI            3

// =============================================================================
// Interruptibility State
// =============================================================================
#define GUEST_INT_BLOCKING_BY_STI           (1 << 0)
#define GUEST_INT_BLOCKING_BY_MOV_SS        (1 << 1)
#define GUEST_INT_BLOCKING_BY_SMI           (1 << 2)
#define GUEST_INT_BLOCKING_BY_NMI           (1 << 3)
#define GUEST_INT_ENCLAVE_INTERRUPTION      (1 << 4)

#endif // OMBRA_VMCS_FIELDS_H
