/**
 * Dual CPU Architecture Patterns (Intel VMX + AMD SVM)
 * Extracted from: NoirVisor, SimpleSvm, CheatDriver
 *
 * These patterns demonstrate unified hypervisor architecture
 * supporting both Intel VT-x and AMD-V.
 */

#include <stdint.h>

/**
 * CPU Manufacturer Detection
 */
typedef enum {
    CPU_UNKNOWN = 0,
    CPU_INTEL,
    CPU_AMD
} cpu_manufacturer_t;

typedef enum {
    CORE_NONE = 0,
    CORE_VMX,       /* Intel VT-x */
    CORE_SVM        /* AMD-V */
} core_type_t;

/**
 * Pattern 1: CPU Detection (from NoirVisor)
 *
 * Source: NoirVisor/src/xpf_core/ci.c
 */
#define CPUID_VENDOR_INTEL_EBX  0x756E6547  /* "Genu" */
#define CPUID_VENDOR_INTEL_ECX  0x6C65746E  /* "ntel" */
#define CPUID_VENDOR_INTEL_EDX  0x49656E69  /* "ineI" */

#define CPUID_VENDOR_AMD_EBX    0x68747541  /* "Auth" */
#define CPUID_VENDOR_AMD_ECX    0x444D4163  /* "cAMD" */
#define CPUID_VENDOR_AMD_EDX    0x69746E65  /* "enti" */

static cpu_manufacturer_t detect_cpu_manufacturer(void)
{
    int regs[4];
    __cpuid(regs, 0);

    if (regs[1] == CPUID_VENDOR_INTEL_EBX &&
        regs[2] == CPUID_VENDOR_INTEL_ECX &&
        regs[3] == CPUID_VENDOR_INTEL_EDX) {
        return CPU_INTEL;
    }

    if (regs[1] == CPUID_VENDOR_AMD_EBX &&
        regs[2] == CPUID_VENDOR_AMD_ECX &&
        regs[3] == CPUID_VENDOR_AMD_EDX) {
        return CPU_AMD;
    }

    return CPU_UNKNOWN;
}

/**
 * Pattern 2: Virtualization Support Check (from NoirVisor)
 *
 * Source: NoirVisor/src/xpf_core/ci.c
 */
#define CPUID_VMX_BIT   (1 << 5)    /* CPUID.1:ECX.VMX */
#define CPUID_SVM_BIT   (1 << 2)    /* CPUID.80000001:ECX.SVM */

static int check_virtualization_support(cpu_manufacturer_t cpu)
{
    int regs[4];

    if (cpu == CPU_INTEL) {
        __cpuid(regs, 1);
        return (regs[2] & CPUID_VMX_BIT) != 0;
    }

    if (cpu == CPU_AMD) {
        __cpuid(regs, 0x80000001);
        return (regs[2] & CPUID_SVM_BIT) != 0;
    }

    return 0;
}

/**
 * Pattern 3: Unified Hypervisor Structure (from NoirVisor)
 *
 * Source: NoirVisor/src/xpf_core/noirhvm.h
 */
typedef struct {
    /* Common fields */
    char vendor_string[13];
    cpu_manufacturer_t cpu_manuf;
    core_type_t selected_core;

    /* CPU count */
    uint32_t cpu_count;

    /* Core-specific data */
    union {
        struct {
            void* vmxon_region;
            void* vmcs;
            uint64_t vmx_msr_bitmap;
            /* Intel-specific fields */
        } intel;

        struct {
            void* hsave_area;
            void* vmcb;
            void* msrpm;
            /* AMD-specific fields */
        } amd;
    };

    /* Per-CPU vCPU structures */
    void** vcpu_array;

    /* EPT/NPT manager */
    void* ept_manager;      /* Intel: EPT, AMD: NPT */

    /* Configuration flags */
    int stealth_mode;
    int nested_virt;

} hypervisor_t;

/**
 * Pattern 4: Unified vCPU Structure (from NoirVisor/hvpp)
 *
 * Source: NoirVisor/src/xpf_core/noirhvm.h, hvpp/vcpu.h
 */
typedef struct {
    /* Common guest state */
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11;
    uint64_t r12, r13, r14, r15;
    uint64_t rip, rflags;
    uint64_t cr0, cr3, cr4;

    /* Control registers shadow */
    uint64_t cr0_shadow;
    uint64_t cr4_shadow;

    /* TSC tracking */
    uint64_t tsc_offset;
    uint64_t tsc_entry;

    /* Core-specific state */
    core_type_t core_type;
    union {
        struct {
            void* vmcs;
            uint64_t vmcs_pa;
        } vmx;

        struct {
            void* vmcb;
            uint64_t vmcb_pa;
        } svm;
    };

    /* Parent hypervisor */
    hypervisor_t* hv;

} vcpu_t;

/**
 * Pattern 5: Unified Hypervisor Initialization (from NoirVisor)
 *
 * Source: NoirVisor/src/xpf_core/noirhvm.c
 */
static int build_hypervisor(hypervisor_t* hv)
{
    /* Detect CPU manufacturer */
    hv->cpu_manuf = detect_cpu_manufacturer();

    if (hv->cpu_manuf == CPU_UNKNOWN) {
        return -1;  /* Unsupported CPU */
    }

    /* Check virtualization support */
    if (!check_virtualization_support(hv->cpu_manuf)) {
        return -2;  /* Virtualization not supported */
    }

    /* Select core based on CPU */
    switch (hv->cpu_manuf) {
    case CPU_INTEL:
        hv->selected_core = CORE_VMX;
        return init_vmx_core(hv);

    case CPU_AMD:
        hv->selected_core = CORE_SVM;
        return init_svm_core(hv);

    default:
        return -3;  /* Should not reach here */
    }
}

/**
 * Pattern 6: Intel VMX Core Initialization
 *
 * Source: NoirVisor/src/vt_core/vt_main.c
 */
static int init_vmx_core(hypervisor_t* hv)
{
    /* 1. Check IA32_FEATURE_CONTROL MSR */
    uint64_t feature_control = __readmsr(0x3A);
    if (!(feature_control & 0x5)) {  /* Lock and VMX outside SMX bits */
        /* Try to enable VMX */
        __writemsr(0x3A, feature_control | 0x5);
    }

    /* 2. Allocate VMXON region */
    hv->intel.vmxon_region = allocate_page_aligned(0x1000);
    if (!hv->intel.vmxon_region) {
        return -1;
    }

    /* 3. Set revision ID in VMXON region */
    uint32_t vmx_basic = (uint32_t)__readmsr(0x480);
    *(uint32_t*)hv->intel.vmxon_region = vmx_basic;

    /* 4. Allocate MSR bitmap */
    hv->intel.vmx_msr_bitmap = (uint64_t)allocate_page_aligned(0x1000);

    /* 5. Execute VMXON */
    uint64_t vmxon_pa = get_physical_address(hv->intel.vmxon_region);
    int result = __vmx_on(&vmxon_pa);

    return result == 0 ? 0 : -2;
}

/**
 * Pattern 7: AMD SVM Core Initialization
 *
 * Source: SimpleSvm/SimpleSvm.cpp, NoirVisor/src/svm_core/svm_main.c
 */
#define MSR_VM_CR           0xC0010114
#define MSR_VM_HSAVE_PA     0xC0010117
#define MSR_EFER            0xC0000080
#define EFER_SVME           (1 << 12)
#define VM_CR_SVMDIS        (1 << 4)

static int init_svm_core(hypervisor_t* hv)
{
    /* 1. Check if SVM is disabled in VM_CR MSR */
    uint64_t vm_cr = __readmsr(MSR_VM_CR);
    if (vm_cr & VM_CR_SVMDIS) {
        return -1;  /* SVM disabled by BIOS */
    }

    /* 2. Enable SVM in EFER MSR */
    uint64_t efer = __readmsr(MSR_EFER);
    __writemsr(MSR_EFER, efer | EFER_SVME);

    /* 3. Allocate host save area */
    hv->amd.hsave_area = allocate_page_aligned(0x1000);
    if (!hv->amd.hsave_area) {
        return -2;
    }

    /* 4. Set HSAVE_PA MSR */
    uint64_t hsave_pa = get_physical_address(hv->amd.hsave_area);
    __writemsr(MSR_VM_HSAVE_PA, hsave_pa);

    /* 5. Allocate MSRPM */
    hv->amd.msrpm = allocate_page_aligned(0x2000);  /* 8KB for AMD */

    return 0;
}

/**
 * Pattern 8: Unified Hypercall Dispatch (from NoirVisor)
 *
 * Source: NoirVisor/src/xpf_core/noirhvm.c
 */
typedef int (*HYPERCALL_HANDLER)(vcpu_t*, uint64_t, uint64_t);

static int dispatch_hypercall(hypervisor_t* hv, uint64_t code, uint64_t param)
{
    if (hv->selected_core == CORE_SVM) {
        /* AMD: Use VMMCALL instruction */
        return amd_vmmcall(code, param);
    } else if (hv->selected_core == CORE_VMX) {
        /* Intel: Use VMCALL instruction */
        return intel_vmcall(code, param);
    }

    return -1;
}

/**
 * Pattern 9: Unified VMExit Handler (from CheatDriver)
 *
 * Source: CheatDriver/Virtualization/Vm.cpp
 */
typedef enum {
    /* Intel VMExit reasons */
    EXIT_REASON_CPUID_INTEL = 10,
    EXIT_REASON_RDMSR_INTEL = 31,
    EXIT_REASON_WRMSR_INTEL = 32,
    EXIT_REASON_VMCALL_INTEL = 18,
    EXIT_REASON_EPT_VIOLATION_INTEL = 48,

    /* AMD VMExit reasons */
    EXIT_REASON_CPUID_AMD = 0x72,
    EXIT_REASON_MSR_AMD = 0x7C,
    EXIT_REASON_VMRUN_AMD = 0x80,
    EXIT_REASON_VMMCALL_AMD = 0x81,
    EXIT_REASON_NPF_AMD = 0x400,
} vmexit_reason_t;

static int handle_vmexit_unified(vcpu_t* vcpu, vmexit_reason_t reason)
{
    if (vcpu->core_type == CORE_VMX) {
        /* Intel VMExit handling */
        switch (reason) {
        case EXIT_REASON_CPUID_INTEL:
            return handle_cpuid(vcpu);
        case EXIT_REASON_RDMSR_INTEL:
            return handle_rdmsr(vcpu);
        case EXIT_REASON_WRMSR_INTEL:
            return handle_wrmsr(vcpu);
        case EXIT_REASON_VMCALL_INTEL:
            return handle_vmcall(vcpu);
        case EXIT_REASON_EPT_VIOLATION_INTEL:
            return handle_ept_violation(vcpu);
        default:
            return handle_unknown_exit(vcpu, reason);
        }
    } else if (vcpu->core_type == CORE_SVM) {
        /* AMD VMExit handling */
        switch (reason) {
        case EXIT_REASON_CPUID_AMD:
            return handle_cpuid(vcpu);
        case EXIT_REASON_MSR_AMD:
            return handle_msr_amd(vcpu);
        case EXIT_REASON_VMMCALL_AMD:
            return handle_vmmcall(vcpu);
        case EXIT_REASON_NPF_AMD:
            return handle_npt_fault(vcpu);
        default:
            return handle_unknown_exit(vcpu, reason);
        }
    }

    return -1;
}

/**
 * Pattern 10: Unified EPT/NPT Manager (from NoirVisor)
 *
 * Source: NoirVisor/src/xpf_core/noirhvm.h
 */
typedef struct {
    core_type_t type;

    union {
        struct {
            void* pml4;         /* EPT PML4 */
            uint64_t eptp;      /* EPT pointer for VMCS */
        } ept;

        struct {
            void* pml4;         /* NPT PML4 (nCR3) */
            uint64_t ncr3;      /* Nested CR3 for VMCB */
        } npt;
    };

    /* Common fields */
    int identity_mapped;
    void* hook_list;

} memory_manager_t;

static int init_memory_manager(memory_manager_t* mgr, core_type_t type)
{
    mgr->type = type;

    /* Allocate PML4 (same for both EPT and NPT) */
    void* pml4 = allocate_page_aligned(0x1000);
    if (!pml4) return -1;

    /* Create identity mapping */
    create_identity_map(pml4);

    if (type == CORE_VMX) {
        mgr->ept.pml4 = pml4;
        mgr->ept.eptp = build_eptp(pml4);
    } else {
        mgr->npt.pml4 = pml4;
        mgr->npt.ncr3 = get_physical_address(pml4);
    }

    mgr->identity_mapped = 1;
    return 0;
}

/**
 * Pattern 11: Conditional Compilation (from NoirVisor)
 *
 * Source: NoirVisor build system
 *
 * Build separate Intel and AMD modules.
 */
#ifdef _NOIR_VMX_CORE
    /* Intel-specific code */
    #include "vt_core/vt_main.c"
    #include "vt_core/vt_exit.c"
    #include "vt_core/vt_ept.c"
#endif

#ifdef _NOIR_SVM_CORE
    /* AMD-specific code */
    #include "svm_core/svm_main.c"
    #include "svm_core/svm_exit.c"
    #include "svm_core/svm_npt.c"
#endif

/*
 * Usage Notes:
 *
 * 1. Architecture Pattern:
 *    - Detect CPU at runtime
 *    - Use common abstractions (vcpu_t, hypervisor_t)
 *    - Dispatch to Intel or AMD specific code
 *    - Use unions for architecture-specific state
 *
 * 2. Key Differences:
 *    Intel VMX:
 *    - VMXON/VMXOFF for VMX enable/disable
 *    - VMCS for guest state
 *    - EPT for memory virtualization
 *    - VMCALL for hypercalls
 *    - MSR bitmap (4KB)
 *
 *    AMD SVM:
 *    - EFER.SVME for SVM enable
 *    - VMCB for guest state
 *    - NPT for memory virtualization
 *    - VMMCALL for hypercalls
 *    - MSRPM (8KB)
 *
 * 3. Testing:
 *    - Test on both Intel and AMD hardware
 *    - Use same detection tools on both
 *    - Verify identical behavior
 *
 * Critical: AMD NPT does not support execute-only pages like Intel EPT.
 *           Use different hook strategies for AMD.
 */
