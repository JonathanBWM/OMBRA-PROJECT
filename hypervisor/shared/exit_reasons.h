// exit_reasons.h — VM-Exit Reason Codes
// OmbraHypervisor
// Reference: Intel SDM Volume 3, Appendix C

#ifndef OMBRA_EXIT_REASONS_H
#define OMBRA_EXIT_REASONS_H

// =============================================================================
// Basic Exit Reasons (Intel SDM Vol 3, Table C-1)
// =============================================================================

#define EXIT_REASON_EXCEPTION_NMI           0   // Exception or NMI
#define EXIT_REASON_EXTERNAL_INT            1   // External interrupt
#define EXIT_REASON_TRIPLE_FAULT            2   // Triple fault
#define EXIT_REASON_INIT                    3   // INIT signal
#define EXIT_REASON_SIPI                    4   // Start-up IPI (SIPI)
#define EXIT_REASON_SMI                     5   // I/O system-management interrupt
#define EXIT_REASON_OTHER_SMI               6   // Other SMI
#define EXIT_REASON_INT_WINDOW              7   // Interrupt window
#define EXIT_REASON_NMI_WINDOW              8   // NMI window
#define EXIT_REASON_TASK_SWITCH             9   // Task switch
#define EXIT_REASON_CPUID                   10  // CPUID
#define EXIT_REASON_GETSEC                  11  // GETSEC
#define EXIT_REASON_HLT                     12  // HLT
#define EXIT_REASON_INVD                    13  // INVD
#define EXIT_REASON_INVLPG                  14  // INVLPG
#define EXIT_REASON_RDPMC                   15  // RDPMC
#define EXIT_REASON_RDTSC                   16  // RDTSC
#define EXIT_REASON_RSM                     17  // RSM
#define EXIT_REASON_VMCALL                  18  // VMCALL
#define EXIT_REASON_VMCLEAR                 19  // VMCLEAR
#define EXIT_REASON_VMLAUNCH                20  // VMLAUNCH
#define EXIT_REASON_VMPTRLD                 21  // VMPTRLD
#define EXIT_REASON_VMPTRST                 22  // VMPTRST
#define EXIT_REASON_VMREAD                  23  // VMREAD
#define EXIT_REASON_VMRESUME                24  // VMRESUME
#define EXIT_REASON_VMWRITE                 25  // VMWRITE
#define EXIT_REASON_VMXOFF                  26  // VMXOFF
#define EXIT_REASON_VMXON                   27  // VMXON
#define EXIT_REASON_CR_ACCESS               28  // Control-register accesses
#define EXIT_REASON_DR_ACCESS               29  // MOV DR
#define EXIT_REASON_IO_INSTRUCTION          30  // I/O instruction
#define EXIT_REASON_RDMSR                   31  // RDMSR
#define EXIT_REASON_WRMSR                   32  // WRMSR
#define EXIT_REASON_ENTRY_FAIL_GUEST        33  // VM-entry failure due to invalid guest state
#define EXIT_REASON_ENTRY_FAIL_MSR          34  // VM-entry failure due to MSR loading
#define EXIT_REASON_MWAIT                   36  // MWAIT
#define EXIT_REASON_MTF                     37  // Monitor trap flag
#define EXIT_REASON_MONITOR                 39  // MONITOR
#define EXIT_REASON_PAUSE                   40  // PAUSE
#define EXIT_REASON_ENTRY_FAIL_MACHINE_CHK  41  // VM-entry failure due to machine-check event
#define EXIT_REASON_TPR_BELOW_THRESHOLD     43  // TPR below threshold
#define EXIT_REASON_APIC_ACCESS             44  // APIC access
#define EXIT_REASON_VIRTUALIZED_EOI         45  // Virtualized EOI
#define EXIT_REASON_GDTR_IDTR_ACCESS        46  // Access to GDTR or IDTR
#define EXIT_REASON_LDTR_TR_ACCESS          47  // Access to LDTR or TR
#define EXIT_REASON_EPT_VIOLATION           48  // EPT violation
#define EXIT_REASON_EPT_MISCONFIG           49  // EPT misconfiguration
#define EXIT_REASON_INVEPT                  50  // INVEPT
#define EXIT_REASON_RDTSCP                  51  // RDTSCP
#define EXIT_REASON_PREEMPTION_TIMER        52  // VMX-preemption timer expired
#define EXIT_REASON_INVVPID                 53  // INVVPID
#define EXIT_REASON_WBINVD                  54  // WBINVD or WBNOINVD
#define EXIT_REASON_XSETBV                  55  // XSETBV
#define EXIT_REASON_APIC_WRITE              56  // APIC write
#define EXIT_REASON_RDRAND                  57  // RDRAND
#define EXIT_REASON_INVPCID                 58  // INVPCID
#define EXIT_REASON_VMFUNC                  59  // VMFUNC
#define EXIT_REASON_ENCLS                   60  // ENCLS
#define EXIT_REASON_RDSEED                  61  // RDSEED
#define EXIT_REASON_PML_FULL                62  // Page-modification log full
#define EXIT_REASON_XSAVES                  63  // XSAVES
#define EXIT_REASON_XRSTORS                 64  // XRSTORS
#define EXIT_REASON_SPP_EVENT               66  // SPP-related event
#define EXIT_REASON_UMWAIT                  67  // UMWAIT
#define EXIT_REASON_TPAUSE                  68  // TPAUSE
#define EXIT_REASON_LOADIWKEY               69  // LOADIWKEY

// =============================================================================
// Exit Reason Bit Flags
// =============================================================================

#define EXIT_REASON_MASK                    0x0000FFFF  // Basic exit reason
#define EXIT_REASON_ENCLAVE_MODE            (1UL << 27) // Exit from enclave
#define EXIT_REASON_PENDING_MTF             (1UL << 28) // Pending MTF VM-exit
#define EXIT_REASON_VMX_ROOT                (1UL << 29) // Exit from VMX root
#define EXIT_REASON_VM_ENTRY_FAIL           (1UL << 31) // VM-entry failure

#endif // OMBRA_EXIT_REASONS_H
