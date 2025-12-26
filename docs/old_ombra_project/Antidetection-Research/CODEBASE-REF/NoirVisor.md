# NoirVisor Dual-Architecture Pattern Extraction

**Target**: NoirVisor - The ONLY reference codebase that supports Intel VMX AND AMD SVM in a SINGLE BINARY
**Purpose**: Extract patterns for Ombra's dual-CPU architecture requirement
**Date**: 2025-12-20

---

## 1. CPU DETECTION & DISPATCH

### 1.1 Vendor String Detection
**Source**: `Refs/codebases/NoirVisor/src/xpf_core/noirhvm.c:33-37`
```c
void noir_get_vendor_string(char* vendor_string)
{
    noir_cpuid(0,0,null,(u32*)&vendor_string[0],(u32*)&vendor_string[8],(u32*)&vendor_string[4]);
    vendor_string[12]=0;
}
```
**Pattern**: CPUID leaf 0 returns vendor in EBX, EDX, ECX (not sequential order!)

### 1.2 CPU Manufacturer Lookup Table
**Source**: `Refs/codebases/NoirVisor/src/include/noirhvm.h:26-43`
```c
#define intel_processor        0
#define amd_processor          1
#define via_processor          2
#define zhaoxin_processor      3
#define hygon_processor        4
#define centaur_processor      5
// ... more vendors
#define unknown_processor      0xff
```

**Source**: `Refs/codebases/NoirVisor/src/include/noirhvm.h:461-501`
```c
#define known_vendor_strings   16
char* vendor_string_list[known_vendor_strings]=
{
    " Shanghai ",      // Zhaoxin
    "AMDisbetter!",    // Early ES of AMD-K5
    "AuthenticAMD",    // AMD
    "CentaurHauls",    // Centaur
    "CyrixInstead",    // Cyrix
    "GenuineIntel",    // Intel
    // ... more
};

u8 cpu_manuf_list[known_vendor_strings]=
{
    zhaoxin_processor,
    amd_processor,
    amd_processor,
    centaur_processor,
    cyrix_processor,
    intel_processor,
    // ...
};
```
**Pattern**: Binary search on sorted vendor strings, maps to manufacturer enum

### 1.3 Binary Search Manufacturer Confirmation
**Source**: `Refs/codebases/NoirVisor/src/xpf_core/noirhvm.c:47-63`
```c
u8 nvc_confirm_cpu_manufacturer(char* vendor_string)
{
    u32 min=0,max=known_vendor_strings;
    while(max>=min)
    {
        u32 mid=(min+max)>>1;
        char* vsn=vendor_string_list[mid];
        i32 cmp=strcmp(vendor_string,vsn);
        if(cmp>0)
            min=mid+1;
        else if(cmp<0)
            max=mid-1;
        else
            return cpu_manuf_list[mid];
    }
    return unknown_processor;
}
```
**Pattern**: O(log n) lookup - efficient runtime detection

### 1.4 Runtime Feature Detection Dispatch
**Source**: `Refs/codebases/NoirVisor/src/xpf_core/noirhvm.c:65-74`
```c
u32 nvc_query_physical_asid_limit(char* vendor_string)
{
    u8 manuf=nvc_confirm_cpu_manufacturer(vendor_string);
    if(manuf==intel_processor || manuf==via_processor || manuf==zhaoxin_processor)
        return nvc_vt_get_avail_vpid();  // Intel path
    else if(manuf==amd_processor || manuf==hygon_processor)
        return nvc_svm_get_avail_asid(); // AMD path
    return 0;
}
```
**Pattern**: Vendor-to-function-pointer dispatch based on CPU family grouping

---

## 2. UNIFIED ABSTRACTION LAYER

### 2.1 Central Hypervisor Structure
**Source**: `Refs/codebases/NoirVisor/src/include/noirhvm.h:199-339`
```c
typedef struct _noir_hypervisor
{
#if defined(_vt_core)
    noir_vt_vcpu_p virtual_cpu;
    noir_vt_hvm_p relative_hvm;
#elif defined(_svm_core)
    noir_svm_vcpu_p virtual_cpu;
    noir_svm_hvm_p relative_hvm;
#else
    void* virtual_cpu;      // Generic pointer at runtime
    void* relative_hvm;     // Generic pointer at runtime
#endif
    // ... COMMON FIELDS (same for Intel/AMD) ...
    struct {
        union {
            void* asid_pool;
            void* vpid_pool;
        };
        union {
            noir_reslock asid_pool_lock;
            noir_reslock vpid_pool_lock;
        };
        u32 start;
        u32 limit;
    }tlb_tagging;

    u32 cpu_count;
    char vendor_string[13];
    u8 cpu_manuf;           // Runtime CPU manufacturer ID
    u8 selected_core;       // Runtime core selection (VMX or SVM)
    u64 reserved[0x10];
}noir_hypervisor,*noir_hypervisor_p;
```

**Key Pattern**:
- Conditional compilation (`#if defined(_vt_core)`) for single-core builds
- Generic `void*` pointers when both cores compiled in
- **`selected_core` field** determines runtime behavior
- Unions for Intel/AMD equivalent structures (VPID/ASID)

### 2.2 Core Selection Constants
**Source**: `Refs/codebases/NoirVisor/src/include/noirhvm.h:45-49`
```c
#define use_nothing        0
#define use_vt_core        1
#define use_svm_core       2
#define use_unknown_core   0xff
```
**Pattern**: `selected_core` field stores runtime choice

### 2.3 Common vCPU Abstraction (CVM)
**Source**: `Refs/codebases/NoirVisor/src/include/cvm_hvm.h` (structures like `noir_cvm_virtual_cpu`)
**Pattern**: Platform-agnostic register structures used by both Intel and AMD
- `noir_gpr_state` - General purpose registers
- `segment_register` - Segment descriptors
- `noir_cvm_control_registers` - CR0, CR2, CR3, CR4, CR8
- `noir_cvm_debug_registers` - DR0-DR7
- Cache invalidation flags track which state needs sync

---

## 3. INTEL VMX PATH

### 3.1 VMX-Specific Structures
**Source**: `Refs/codebases/NoirVisor/src/include/vt_hvm.h:92-150`
```c
typedef struct _noir_vt_hvm
{
    memory_descriptor msr_bitmap;
    memory_descriptor io_bitmap_a;
    memory_descriptor io_bitmap_b;
    u32 hvm_cpuid_leaf_max;
    struct _noir_dmar_manager *dmar_manager;
}noir_vt_hvm,*noir_vt_hvm_p;

typedef struct _noir_vt_vcpu
{
    memory_descriptor vmxon;
    memory_descriptor vmcs;
    memory_descriptor msr_auto;
    struct _noir_vt_vcpu *self;
    void* hv_stack;
    noir_vt_hvm_p relative_hvm;
    // ... VMCS-specific state ...
    noir_cvm_virtual_cpu cvm_state;  // Common abstraction layer
    noir_mshv_vcpu mshvcpu;
    u32 family_ext;
    u8 status;
}noir_vt_vcpu,*noir_vt_vcpu_p;
```
**Pattern**: VMCS/VMXON regions, MSR autoload, EPT manager embedded

### 3.2 VMX Entry Point
**Source**: `Refs/codebases/NoirVisor/src/xpf_core/noirhvm.c:1989-2000`
```c
vmx_subversion:
    hvm_p->selected_core=use_vt_core;
    if(nvc_is_vt_supported())
    {
        nv_dprintf("Starting subversion with VMX Engine!\n");
        return nvc_vt_subvert_system(hvm_p);
    }
    else
    {
        nv_dprintf("Your processor does not support Intel VT-x!\n");
        return noir_vmx_not_supported;
    }
```
**Pattern**: Set `selected_core`, verify support, call Intel-specific subversion

### 3.3 VMX Hypercall Interface
**Source**: `Refs/codebases/NoirVisor/src/include/vt_hvm.h:17-23`
```c
#define noir_vt_callexit           0x1
#define noir_vt_init_custom_vmcs   0x10000
#define noir_vt_run_custom_vcpu    0x10001
#define noir_vt_dump_vcpu_vmcs     0x10002
#define noir_vt_set_vcpu_options   0x10003
```
**Pattern**: VMCALL codes for Intel path

---

## 4. AMD SVM PATH

### 4.1 SVM-Specific Structures
**Source**: `Refs/codebases/NoirVisor/src/include/svm_hvm.h:75-99`
```c
typedef struct _noir_svm_hvm
{
    memory_descriptor msrpm;
    memory_descriptor iopm;
    memory_descriptor blank_page;
    struct _noir_npt_manager* primary_nptm;
    struct _noir_npt_manager* secondary_nptm;
    struct {
        u32 asid_limit;
        u32 capabilities;
    }virt_cap;
    struct {
        u32 capabilities;
        u32 mem_virt_cap;
        u32 simultaneous;
        u32 minimum_asid;
    }sev_cap;
    u32 hvm_cpuid_leaf_max;
}noir_svm_hvm,*noir_svm_hvm_p;
```
**Pattern**: MSRPM/IOPM instead of bitmaps, NPT manager, SEV support

### 4.2 SVM Entry Point
**Source**: `Refs/codebases/NoirVisor/src/xpf_core/noirhvm.c:2001-2012`
```c
svm_subversion:
    hvm_p->selected_core=use_svm_core;
    if(nvc_is_svm_supported())
    {
        nv_dprintf("Starting subversion with SVM Engine!\n");
        return nvc_svm_subvert_system(hvm_p);
    }
    else
    {
        nv_dprintf("Your processor does not support AMD-V!\n");
        return noir_svm_not_supported;
    }
```
**Pattern**: Identical structure to VMX path, different function calls

### 4.3 SVM Hypercall Interface
**Source**: `Refs/codebases/NoirVisor/src/include/svm_hvm.h:17-28`
```c
#define noir_svm_callexit          0x1
#define noir_svm_init_custom_vmcb  0x10000
#define noir_svm_run_custom_vcpu   0x10001
#define noir_svm_dump_vcpu_vmcb    0x10002
#define noir_svm_set_vcpu_options  0x10003
```
**Pattern**: VMMCALL codes mirror VMCALL codes (same numbering scheme!)

---

## 5. SINGLE BINARY PATTERN

### 5.1 Build System - Both Cores Compiled
**Source**: `Refs/codebases/NoirVisor/build/compchk_win11x64.bat:26-30`
```batch
echo Compiling Core Engine of Intel VT-x...
for %%1 in (..\src\vt_core\*.c) do (cl %%1 ... /D"_vt_core" ...)

echo Compiling Core Engine of AMD-V...
for %%1 in (..\src\svm_core\*.c) do (cl %%1 ... /D"_svm_core" ...)
```
**Pattern**: BOTH cores compiled into SAME binary with different defines per file

### 5.2 Runtime Initialization Dispatch
**Source**: `Refs/codebases/NoirVisor/src/xpf_core/noirhvm.c:1960-2015`
```c
noir_status nvc_build_hypervisor()
{
    noir_get_vendor_string(hvm_p->vendor_string);
    hvm_p->cpu_manuf=nvc_confirm_cpu_manufacturer(hvm_p->vendor_string);
    hvm_p->options.value=noir_query_enabled_features_in_system();
    nvc_store_image_info(&hvm_p->hv_image.base,&hvm_p->hv_image.size);

    switch(hvm_p->cpu_manuf)
    {
        case intel_processor:
            goto vmx_subversion;
        case amd_processor:
            goto svm_subversion;
        case via_processor:
            goto vmx_subversion;
        case zhaoxin_processor:
            goto vmx_subversion;
        case hygon_processor:
            goto svm_subversion;
        case centaur_processor:
            goto vmx_subversion;
        default:
            return noir_unknown_processor;
    }

vmx_subversion:
    hvm_p->selected_core=use_vt_core;
    if(nvc_is_vt_supported())
        return nvc_vt_subvert_system(hvm_p);
    else
        return noir_vmx_not_supported;

svm_subversion:
    hvm_p->selected_core=use_svm_core;
    if(nvc_is_svm_supported())
        return nvc_svm_subvert_system(hvm_p);
    else
        return noir_svm_not_supported;
}
```

**CRITICAL PATTERN**:
1. Detect vendor at runtime via CPUID
2. Switch based on `cpu_manuf`
3. Set `selected_core` to `use_vt_core` or `use_svm_core`
4. Call appropriate `*_subvert_system()` function
5. **NO conditional compilation at top level** - both paths exist in binary

### 5.3 Runtime Teardown Dispatch
**Source**: `Refs/codebases/NoirVisor/src/xpf_core/noirhvm.c:2017-2043`
```c
void nvc_teardown_hypervisor()
{
    switch(hvm_p->cpu_manuf)
    {
        case intel_processor:
            goto vmx_restoration;
        case amd_processor:
            goto svm_restoration;
        // ... other vendors ...
    }

vmx_restoration:
    nvc_vt_restore_system(hvm_p);
    return;

svm_restoration:
    nvc_svm_restore_system(hvm_p);
    return;
}
```
**Pattern**: Same switch-based dispatch for teardown

### 5.4 Hypercall Runtime Dispatch
**Source**: `Refs/codebases/NoirVisor/src/xpf_core/noirhvm.c:165-182`
```c
noir_status nvc_set_guest_vcpu_options(noir_cvm_virtual_cpu_p vcpu,
                                        noir_cvm_vcpu_option_type option_type,
                                        u32 data)
{
    // ... validate parameters ...

    if(hvm_p->selected_core==use_svm_core)
    {
        st=noir_success;
        noir_svm_vmmcall(noir_cvm_set_vcpu_options,(ulong_ptr)vcpu);
    }
    else if(hvm_p->selected_core==use_vt_core)
    {
        st=noir_success;
        noir_vt_vmcall(noir_cvm_set_vcpu_options,(ulong_ptr)vcpu);
    }
    else
        st=noir_unknown_processor;

    return st;
}
```
**Pattern**: Runtime check of `selected_core` dispatches to Intel or AMD hypercall

### 5.5 Conditional Compilation Strategy
**Source**: `Refs/codebases/NoirVisor/src/include/noirhvm.h:20-24`
```c
#include "mshv_hvm.h"
#include "cvm_hvm.h"
#include "hax_hvm.h"
#if defined(_vt_core)
#include "vt_hvm.h"
#elif defined(_svm_core)
#include "svm_hvm.h"
#endif
```
**Pattern**:
- Files in `vt_core/` get `-D_vt_core` - see Intel structures
- Files in `svm_core/` get `-D_svm_core` - see AMD structures
- Cross-platform files (`xpf_core/`) get NEITHER - see generic `void*` pointers

### 5.6 Central Definition File Pattern
**Source**: `Refs/codebases/NoirVisor/src/xpf_core/noirhvm.c:38`
```c
cl ..\src\xpf_core\noirhvm.c ... /D"_central_hvm" ...
```
**Pattern**: Central HVM file gets special define, sees both cores

---

## 6. COMMON STRUCTURES

### 6.1 Platform-Agnostic Register State
**Source**: `Refs/codebases/NoirVisor/src/include/cvm_hvm.h:67-95`
```c
typedef enum _noir_cvm_register_type
{
    noir_cvm_general_purpose_register,
    noir_cvm_flags_register,
    noir_cvm_instruction_pointer,
    noir_cvm_control_register,
    noir_cvm_cr2_register,
    noir_cvm_debug_register,
    noir_cvm_segment_register,
    noir_cvm_descriptor_table,
    noir_cvm_syscall_msr_register,
    noir_cvm_sysenter_msr_register,
    // ... unified register types work on both Intel/AMD
}noir_cvm_register_type,*noir_cvm_register_type_p;
```
**Pattern**: Abstract register types hide VMCS vs VMCB differences

### 6.2 Common Intercept Codes
**Source**: `Refs/codebases/NoirVisor/src/include/cvm_hvm.h:38-65`
```c
typedef enum _noir_cvm_intercept_code
{
    cv_invalid_state=0,
    cv_shutdown_condition=1,
    cv_memory_access=2,
    cv_hlt_instruction=4,
    cv_io_instruction=5,
    cv_cpuid_instruction=6,
    cv_rdmsr_instruction=7,
    cv_wrmsr_instruction=8,
    cv_cr_access=9,
    cv_dr_access=10,
    cv_hypercall=11,
    cv_exception=12,
    // ... platform-agnostic exit reasons
}noir_cvm_intercept_code,*noir_cvm_intercept_code_p;
```
**Pattern**: Unified exit codes map to VMExit (Intel) or #VMEXIT (AMD)

### 6.3 State Cache Invalidation
**Source**: Used throughout `noirhvm.c` (lines 206-292)
```c
vcpu->state_cache.gprvalid=0;   // Invalidate GPR cache
vcpu->state_cache.cr_valid=0;   // Invalidate CR cache
vcpu->state_cache.sr_valid=0;   // Invalidate segment cache
```
**Pattern**: Generic state tracking - works for VMCS and VMCB

---

## 7. KEY IMPLEMENTATION INSIGHTS FOR OMBRA

### 7.1 Critical Success Factors
1. **Single `selected_core` field** drives ALL runtime decisions
2. **Binary search on vendor strings** is fast and extensible
3. **Unions for Intel/AMD equivalent structures** (VPID/ASID pools)
4. **Common abstraction layer** (CVM) hides arch differences from upper layers
5. **Per-file conditional compilation** - core files see their structures, cross-platform sees generic pointers

### 7.2 Function Pointer Pattern (Not Used)
**NoirVisor does NOT use function pointers** - it uses runtime `if/else` checks on `selected_core`
This is SIMPLER and avoids indirect call overhead

### 7.3 Compilation Pattern
```
vt_core/*.c     -> Compiled with -D_vt_core    (sees noir_vt_vcpu_p)
svm_core/*.c    -> Compiled with -D_svm_core   (sees noir_svm_vcpu_p)
xpf_core/*.c    -> Compiled with neither       (sees void*)
noirhvm.c       -> Compiled with -D_central_hvm (sees both)
```
All object files link into ONE binary

### 7.4 Initialization Sequence
```
1. noir_get_vendor_string()           // CPUID leaf 0
2. nvc_confirm_cpu_manufacturer()     // Binary search lookup
3. switch(cpu_manuf)                  // Intel vs AMD vs ...
4. hvm_p->selected_core = use_X_core  // Set runtime flag
5. nvc_X_subvert_system(hvm_p)        // Call arch-specific init
6. All future operations check selected_core
```

### 7.5 Memory Layout Strategy
- **Shared structures** allocated once (`noir_hypervisor`)
- **Arch-specific structures** allocated based on `selected_core`
- **Generic pointers** (`void* virtual_cpu`) cast at runtime
- **Unions** for equivalent structures reduce memory waste

---

## 8. DIRECT APPLICATION TO OMBRA

### 8.1 Recommended Structure for Ombra
```c
typedef struct _OMBRA_HYPERVISOR {
    // Runtime selection (like NoirVisor)
    UINT8 CpuManufacturer;   // intel_processor or amd_processor
    UINT8 SelectedCore;      // use_vmx_core or use_svm_core

    // Generic pointers (like NoirVisor's void*)
    PVOID VirtualCpu;        // Points to OMBRA_VMX_VCPU or OMBRA_SVM_VCPU
    PVOID RelativeHvm;       // Points to OMBRA_VMX_HVM or OMBRA_SVM_HVM

    // Common fields (like NoirVisor's tlb_tagging union)
    union {
        PVOID AsidPool;
        PVOID VpidPool;
    } TlbTagging;

    CHAR VendorString[13];
    UINT32 CpuCount;
    // ...
} OMBRA_HYPERVISOR, *POMBRA_HYPERVISOR;
```

### 8.2 Recommended Initialization Pattern
```c
NTSTATUS OmbraInitializeHypervisor(POMBRA_HYPERVISOR Hvm)
{
    // Detect vendor (NoirVisor pattern)
    OmbraGetVendorString(Hvm->VendorString);
    Hvm->CpuManufacturer = OmbraConfirmCpuManufacturer(Hvm->VendorString);

    // Dispatch based on vendor
    switch(Hvm->CpuManufacturer) {
        case OMBRA_INTEL_PROCESSOR:
            Hvm->SelectedCore = OMBRA_USE_VMX_CORE;
            return OmbraVmxSubvertSystem(Hvm);

        case OMBRA_AMD_PROCESSOR:
            Hvm->SelectedCore = OMBRA_USE_SVM_CORE;
            return OmbraSvmSubvertSystem(Hvm);

        default:
            return STATUS_NOT_SUPPORTED;
    }
}
```

### 8.3 Recommended Hypercall Pattern
```c
NTSTATUS OmbraHandleHypercall(POMBRA_HYPERVISOR Hvm, UINT64 Code, UINT64 Param)
{
    if(Hvm->SelectedCore == OMBRA_USE_VMX_CORE)
        return OmbraVmxHandleVmcall(Code, Param);
    else if(Hvm->SelectedCore == OMBRA_USE_SVM_CORE)
        return OmbraSvmHandleVmmcall(Code, Param);
    else
        return STATUS_INVALID_DEVICE_STATE;
}
```

### 8.4 Build System Recommendations
```makefile
# Intel VMX core (like NoirVisor's vt_core)
OmbraVmx/*.c: CFLAGS += -D_OMBRA_VMX_CORE

# AMD SVM core (like NoirVisor's svm_core)
OmbraSvm/*.c: CFLAGS += -D_OMBRA_SVM_CORE

# Cross-platform core (like NoirVisor's xpf_core)
OmbraCore/*.c: # No arch-specific defines

# Central coordinator
OmbraCore/OmbraHvm.c: CFLAGS += -D_OMBRA_CENTRAL_HVM
```

---

## 9. FILES TO MODIFY IN OMBRA

Based on NoirVisor's patterns:

### 9.1 Create New Files
- `OmbraPayload/OmbraHvm.h` - Central hypervisor structure (like `noirhvm.h`)
- `OmbraPayload/OmbraHvm.c` - Runtime dispatch logic (like `noirhvm.c`)
- `OmbraPayload/OmbraCvm.h` - Common VM abstraction (like `cvm_hvm.h`)
- `OmbraPayload/Intel/` - Directory for VMX-specific code
- `OmbraPayload/Amd/` - Directory for SVM-specific code

### 9.2 Modify Existing Files
- `OmbraPayload/Entry.cpp` - Add vendor detection before subversion
- `OmbraPayload/EntryAsm.asm` - May need dual entry points (VMX vs SVM)
- `OmbraBoot/Hooks/HvLoader.cpp` - Pass vendor info to payload

---

## 10. RISKS & MITIGATIONS

| Risk | Mitigation (from NoirVisor) |
|------|------------------------------|
| **Binary size doubles** | NoirVisor is ~200KB with both cores - acceptable for Ombra |
| **Wrong core selected** | Binary search is reliable; also check CPUID features after selection |
| **Generic void* casting errors** | Use inline helpers like NoirVisor's `(noir_vt_vcpu_p)hvm_p->virtual_cpu` |
| **Linker errors from duplicate symbols** | Per-file defines prevent symbol collisions |
| **Performance overhead** | NoirVisor's runtime checks are branch-predicted; negligible cost |

---

## REFERENCES

### Primary Source Files Analyzed
- `Refs/codebases/NoirVisor/src/include/noirhvm.h` - Central structures
- `Refs/codebases/NoirVisor/src/include/vt_hvm.h` - Intel VMX definitions
- `Refs/codebases/NoirVisor/src/include/svm_hvm.h` - AMD SVM definitions
- `Refs/codebases/NoirVisor/src/include/cvm_hvm.h` - Common VM abstraction
- `Refs/codebases/NoirVisor/src/xpf_core/noirhvm.c` - Runtime dispatch implementation
- `Refs/codebases/NoirVisor/build/compchk_win11x64.bat` - Build system

### Key Line References
- **CPU detection**: `noirhvm.c:33-63` (vendor string, binary search)
- **Runtime dispatch**: `noirhvm.c:1960-2015` (nvc_build_hypervisor switch)
- **Hypercall dispatch**: `noirhvm.c:165-182` (selected_core checks)
- **Structure definitions**: `noirhvm.h:199-339` (conditional compilation)
- **Build pattern**: `compchk_win11x64.bat:26-30` (both cores compiled)

---

**Extraction Complete**: 2025-12-20
**Status**: READY FOR IMPLEMENTATION IN OMBRA
**Next Agent**: vmexit-handler-agent (use this pattern for VMExit/VMEXIT unification)
