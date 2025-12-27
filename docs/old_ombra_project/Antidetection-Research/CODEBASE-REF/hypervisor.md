# Hypervisor Codebase Extraction (Bareflank)

**Source**: `Refs/codebases/hypervisor/`
**Type**: Full-featured research hypervisor SDK (Intel VMX + AMD SVM)
**Language**: C++20 with AUTOSAR compliance
**Architecture**: Microkernel-based with ring-3 extensions in VMX root

---

## 1. ARCHITECTURE OVERVIEW

### 1.1 Core Design Philosophy
**Source**: `README.md:1-309`

- **Microkernel approach**: Minimal kernel in VMX root, all hypervisor logic lives in ring-3 extensions
- **Modular SDK**: Not a complete hypervisor - provides scaffolding for building custom hypervisors
- **Cross-platform**: Single codebase for Intel VT-x and AMD-V
- **AUTOSAR compliant**: 100% unit test coverage, production-ready foundation
- **Extension-based**: Users write custom "extensions" (ring-3 apps) that implement hypervisor features

### 1.2 Component Structure
**Source**: `README.md:183-219`

```
vmmctl (userspace)
   ↓
loader (kernel driver)
   ↓
kernel (microkernel in VMX root)
   ↓
extension (ring-3 app in VMX root - YOUR CODE)
```

- **vmmctl**: CLI tool to control the loader (start/stop/dump)
- **loader**: Kernel driver that places microkernel + extensions into VMX root
- **kernel**: Microkernel executing extensions via syscall ABI
- **extension**: Ring-3 application implementing actual hypervisor logic

### 1.3 Module Organization
**Source**: `kernel/src/` directory structure

```
kernel/src/
├── mk_main_t.hpp           # Main microkernel entry point
├── dispatch_syscall.hpp    # Syscall dispatcher (extension → kernel ABI)
├── ext_t.hpp              # Extension management (ELF loading, memory map)
├── ext_pool_t.hpp         # Extension pool manager
├── vm_t.hpp / vm_pool_t.hpp    # Virtual Machine abstraction
├── vp_t.hpp / vp_pool_t.hpp    # Virtual Processor abstraction
├── vs_t.hpp / vs_pool_t.hpp    # Virtual machine State (VMCS/VMCB)
├── page_pool_t.hpp        # 4K page allocator
├── huge_pool_t.hpp        # Large allocation pool
├── tls_t.hpp              # Thread-local storage (per-PP state)
├── vmexit_loop.hpp        # VMExit dispatch loop
└── x64/
    ├── intel/intrinsic_t.hpp   # Intel VMX intrinsics
    ├── amd/intrinsic_t.hpp     # AMD SVM intrinsics
    └── tls_t.hpp              # x64 TLS block layout
```

**Key Insight**: Pool-based resource management (`ext_pool_t`, `vm_pool_t`, `vp_pool_t`, `vs_pool_t`) - all resources tracked via ID-based handles.

---

## 2. VMX IMPLEMENTATION (Intel)

### 2.1 Intrinsic Abstraction Layer
**Source**: `kernel/src/x64/intel/intrinsic_t.hpp:1-300`

```cpp
class intrinsic_t final {
    // VMCS read/write wrappers
    static constexpr auto vmrd64(reg) -> safe_u64 { return intrinsic_vmrd64(reg); }
    static constexpr void vmwr64(reg, val) { intrinsic_vmwr64(reg, val); }

    // TLB flush (handles INVEPT/INVVPID)
    static constexpr void tlb_flush(addr, vpid = BF_INVALID_ID) {
        if (vpid == BF_INVALID_ID) {
            return intrinsic_invlpg(addr);  // Extension address
        }
        if (vpid.is_zero()) {
            // Type 2: All-context INVEPT
            invept_descriptor_t desc{{}, {}};
            return intrinsic_invept(&desc, 2);
        }
        if (addr.is_zero()) {
            // Type 1: Single-context INVEPT (current EPTP)
            invept_descriptor_t desc{vmrd64(VMCS_EPTP_INDEX), {}};
            return intrinsic_invept(&desc, 1);
        }
        // Type 0: Individual-address INVVPID
        invvpid_descriptor_t desc{vpid, {}, {}, {}, addr};
        return intrinsic_invvpid(&desc, 0);
    }

    // MSR access
    [[nodiscard]] static constexpr auto rdmsr(msr) -> safe_u64;
    static constexpr void wrmsr(msr, val);

    // Segment selectors
    [[nodiscard]] static constexpr auto cs_selector() -> safe_u16;
    // ... ES, DS, FS, GS, SS, TR

    // Control registers
    [[nodiscard]] static constexpr auto cr0/cr3/cr4() -> safe_u64;
    static constexpr void set_rpt(cr3_val);  // Set root page table
};
```

**Implementation**: Assembly stubs in `kernel/src/x64/intel/*.S`
- `intrinsic_vmrd64.S`, `intrinsic_vmwr64.S`: VMREAD/VMWRITE wrappers
- `intrinsic_vmrun.S`: VMRESUME/VMLAUNCH logic
- `intrinsic_invept.S`, `intrinsic_invvpid.S`: TLB invalidation

### 2.2 VMCS Management Pattern
**Source**: `kernel/src/x64/intel/intrinsic_t.hpp:94-120`

```cpp
// Unified TLB flush API handles:
// 1. Extension address invalidation (INVLPG)
// 2. Global EPT flush (Type 2 INVEPT)
// 3. Single-context EPT flush (Type 1 INVEPT with current EPTP)
// 4. Single-address guest flush (Type 0 INVVPID)
```

**Ombra Application**: Bareflank's `tlb_flush()` provides exactly the pattern needed for:
- `OmbraPayload/Vmx/Ept.cpp` EPT invalidation
- `OmbraPayload/Vmx/Vmcs.cpp` VPID management

### 2.3 VMExit Entry Point
**Source**: `kernel/src/x64/intel/intrinsic_vmrun.S:1-373` (assembly entry point)

Flow:
1. Guest state saved to VMCS automatically (hardware)
2. Host state restored from VMCS (RIP, RSP, CR3)
3. Assembly stub saves host GPRs to TLS block
4. Calls `vmexit_loop()` C++ function
5. Restores GPRs, executes VMRESUME

### 2.4 Example Extension VMExit Handler
**Source**: `example/default/src/x64/intel/dispatch_vmexit.hpp:1-107`

```cpp
[[nodiscard]] static constexpr auto
dispatch_vmexit(
    gs_t const &gs,              // Global state
    tls_t const &tls,            // Thread-local state
    syscall::bf_syscall_t &sys,  // Syscall interface
    intrinsic_t const &intrinsic,
    vp_pool_t const &vp_pool,
    vs_pool_t const &vs_pool,
    bsl::safe_u16 const &vsid,   // Virtual machine State ID
    bsl::safe_u64 const &exit_reason) -> bsl::errc_type
{
    constexpr auto exit_reason_nmi{0x0_u64};
    constexpr auto exit_reason_nmi_window{0x8_u64};
    constexpr auto exit_reason_cpuid{0xA_u64};

    switch (exit_reason.get()) {
        case exit_reason_nmi.get():
            return dispatch_vmexit_nmi(gs, tls, sys, vsid);
        case exit_reason_nmi_window.get():
            return dispatch_vmexit_nmi_window(gs, tls, sys, vsid);
        case exit_reason_cpuid.get():
            return dispatch_vmexit_cpuid(gs, tls, sys, intrinsic, vsid);
        default:
            bsl::error() << "unsupported vmexit: " << bsl::hex(exit_reason);
            syscall::bf_debug_op_dump_vs(vsid);
            return bsl::errc_failure;
    }
}
```

**Ombra Application**: Clean dispatch table pattern for `OmbraPayload/VmExit/VmExit.cpp`

---

## 3. AMD SVM IMPLEMENTATION

### 3.1 Dual-Architecture Support
**Source**: `kernel/src/x64/amd/intrinsic_t.hpp` (parallel structure to Intel)

- Same `intrinsic_t` API surface
- Implementation differs (VMCB vs VMCS)
- TLB flush uses NPT invalidation instead of EPT
- Same syscall ABI for extensions

**Key Pattern**: Extensions are CPU-agnostic via abstraction layer

### 3.2 Cross-Platform Extension Example
**Source**: `example/default/src/x64/amd/dispatch_vmexit.hpp`

- Identical handler signature to Intel version
- Exit reasons differ (SVM exit codes vs VMX exit reasons)
- Same syscall interface (`bf_syscall_t`)

**Ombra Application**: Use conditional compilation to dispatch:
```cpp
#ifdef ENABLE_INTEL
    #include "vmx/dispatch_vmexit.hpp"
#else
    #include "svm/dispatch_vmexit.hpp"
#endif
```

---

## 4. NOTABLE TECHNIQUES

### 4.1 Per-PP (Physical Processor) Extension Stacks
**Source**: `kernel/src/mk_main_t.hpp:140-200`

```cpp
// Extension stack layout (per-PP isolation):
//
// --------------------   EXT_STACK_ADDR
// |   PP 0 Stack     |
// --------------------
// |   Guard Page     |
// --------------------
// |   PP 1 Stack     |
// --------------------
// |   Guard Page     |
// --------------------
//        ...

static constexpr void set_extension_sp(tls_t &tls) {
    constexpr auto stack_addr{HYPERVISOR_EXT_STACK_ADDR};
    constexpr auto stack_size{HYPERVISOR_EXT_STACK_SIZE};
    constexpr auto stack_size_with_guard{stack_size + HYPERVISOR_PAGE_SIZE};

    // Each PP gets its own stack at incrementing addresses
    auto const offset = bsl::to_u64(tls.ppid) * stack_size_with_guard;
    tls.ext_sp = stack_addr + offset + stack_size;  // Top of stack
}
```

**Ombra Application**: Adopt guard pages between per-CPU stacks to detect stack overflow

### 4.2 TLS Block Layout (per-PP State)
**Source**: `kernel/src/x64/tls_t.hpp:1-150`

```cpp
#pragma pack(push, 1)
struct tls_t final {
    // --- Microkernel State (0x000-0x028) ---
    uint64 mk_rbx, mk_rbp, mk_r12, mk_r13, mk_r14, mk_r15;

    // --- Extension Syscall Registers (0x030-0x0A8) ---
    uint64 ext_syscall;  // RAX: syscall opcode
    uint64 ext_reg0;     // RDI: arg0
    uint64 ext_reg1;     // RSI: arg1
    uint64 ext_reg2;     // RDX: arg2
    uint64 ext_reg3;     // R10: arg3
    uint64 ext_reg4;     // R8:  arg4
    uint64 ext_reg5;     // R9:  arg5
    uint64 ext_sp;       // Extension stack pointer

    // --- ESR (Exception Service Routine) State (0x0B0-0x200) ---
    uint64 esr_rax, esr_rbx, ..., esr_r15;
    uint64 esr_rsp, esr_rip, esr_rflags;

    // --- Microkernel Metadata (0x200-0x300) ---
    uint16 ppid;         // Physical processor ID
    uint16 online_pps;   // Total online processors
    ext_t *ext;          // Current extension
    // ... debug ring, state_save_t, etc.
};
#pragma pack(pop)
```

**Size**: Exactly 0x300 bytes (768 bytes) - fits in single cache line aligned

**Ombra Application**: Use similar packed TLS for per-CPU context switching

### 4.3 Syscall Dispatch Mechanism
**Source**: `kernel/src/dispatch_syscall.hpp:1-150`

```cpp
[[nodiscard]] constexpr auto
dispatch_syscall(
    tls_t &tls,
    page_pool_t &page_pool,
    huge_pool_t &huge_pool,
    intrinsic_t &intrinsic,
    vm_pool_t &vm_pool,
    vp_pool_t &vp_pool,
    vs_pool_t &vs_pool,
    ext_pool_t &ext_pool,
    vmexit_log_t &log) -> bf_status_t
{
    expects(nullptr != tls.ext);

    switch (bf_syscall_opcode(tls.ext_syscall).get()) {
        case BF_DEBUG_OP_VAL.get():
            return dispatch_syscall_bf_debug_op(...);
        case BF_VS_OP_VAL.get():
            return dispatch_syscall_bf_vs_op(...);
        case BF_VM_OP_VAL.get():
            return dispatch_syscall_bf_vm_op(...);
        // ... MEM_OP, VP_OP, HANDLE_OP, CALLBACK_OP
    }
}
```

**Syscall Encoding**: `syscall/include/bf_constants.hpp:126-180`
```cpp
constexpr auto BF_SYSCALL_SIG_VAL{0x6642000000000000_u64};  // "Bf" signature
constexpr auto BF_SYSCALL_SIG_MASK{0xFFFF000000000000_u64};
constexpr auto BF_SYSCALL_OPCODE_MASK{0xFFFF0000FFFF0000_u64};
constexpr auto BF_SYSCALL_INDEX_MASK{0x000000000000FFFF_u64};

// RAX encoding: [SIG:16][FLAGS:16][OPCODE:16][INDEX:16]
```

**Ombra Application**: Adopt signature-based syscall validation to prevent accidental non-syscall execution

### 4.4 Extension ELF Loading Pattern
**Source**: `kernel/src/ext_t.hpp:1-200`

```cpp
class ext_t final {
    root_page_table_t m_main_rpt{};  // Extension's address space
    array<root_page_table_t, HYPERVISOR_MAX_VMS> m_direct_map_rpts{};

    safe_u64 m_entry_ip{};      // _start
    safe_u64 m_bootstrap_ip{};  // bootstrap callback
    safe_u64 m_vmexit_ip{};     // vmexit callback
    safe_u64 m_fail_ip{};       // fail callback

    // Parse ELF64 header
    static constexpr void validate_elf64_ehdr(elf64_ehdr_t const *file) {
        expects(file->e_type == ET_EXEC);
        expects(*file->e_ident.at_if(EI_MAG0) == ELFMAG0);
        // ... validate ELF magic, class, endianness
    }

    // Map ELF segments into extension's address space
    [[nodiscard]] constexpr auto map_ext_elf(...) -> bsl::errc_type;
};
```

**Ombra Application**: Use similar ELF parsing for `OmbraPayload.efi` when loaded by `OmbraBoot`

### 4.5 Direct Map RVA Pattern (VM-specific Address Spaces)
**Source**: `kernel/src/ext_t.hpp:126-128`

```cpp
/// @brief stores the direct map rpts (one per VM)
array<root_page_table_t, HYPERVISOR_MAX_VMS> m_direct_map_rpts{};
```

Each VM gets its own "direct map" page table where guest physical addresses are mapped into the extension's virtual address space. This allows extensions to access guest memory via VA without manual translation.

**Ombra Application**: Consider per-game process direct maps for efficient memory scanning

### 4.6 Syscall Interface (Extension → Kernel ABI)
**Source**: `syscall/src/bf_syscall_t.hpp:1-250`

```cpp
class bf_syscall_t final {
    safe_u64 m_hndl{};  // Handle from bf_handle_op_open_handle

    // Initialization
    [[nodiscard]] constexpr auto initialize(
        safe_u32 const &version,
        bf_callback_handler_bootstrap_t pmut_bootstrap_handler,
        bf_callback_handler_vmexit_t pmut_vmexit_handler,
        bf_callback_handler_fail_t pmut_fail_handler
    ) -> bsl::errc_type;

    // TLS register access (guest state)
    [[nodiscard]] static constexpr auto bf_tls_rax() -> safe_u64;
    static constexpr void bf_tls_set_rax(safe_u64 const &val);
    // ... rbx, rcx, rdx, rsi, rdi, r8-r15

    // VMCS/VMCB field access
    [[nodiscard]] constexpr auto bf_vs_op_read(
        safe_u16 const &vsid,  // Virtual machine State ID
        bf_reg_t reg           // Register/field enum
    ) -> safe_u64;

    constexpr auto bf_vs_op_write(
        safe_u16 const &vsid,
        bf_reg_t reg,
        safe_u64 const &val
    ) -> bsl::errc_type;

    // VM lifecycle
    [[nodiscard]] constexpr auto bf_vm_op_create_vm(
        safe_u16 const &vmid
    ) -> bsl::errc_type;

    // VP (Virtual Processor) lifecycle
    [[nodiscard]] constexpr auto bf_vp_op_create_vp(
        safe_u16 const &vmid,
        safe_u16 const &ppid
    ) -> safe_u16;  // Returns VPID

    // VS (Virtual machine State) lifecycle
    [[nodiscard]] constexpr auto bf_vs_op_create_vs(
        safe_u16 const &vpid,
        safe_u16 const &ppid
    ) -> safe_u16;  // Returns VSID

    // Run VM
    constexpr auto bf_vs_op_run(
        safe_u16 const &vsid
    ) -> bsl::errc_type;

    // Advance guest RIP and run
    constexpr auto bf_vs_op_advance_ip_and_run(
        safe_u16 const &vsid
    ) -> bsl::errc_type;
};
```

**Ombra Application**: Adopt handle-based resource management for VMCS/VP/VM tracking

### 4.7 Pool-Based Resource Management Pattern
**Source**: `kernel/src/vm_pool_t.hpp`, `vp_pool_t.hpp`, `vs_pool_t.hpp`

All resources (VMs, VPs, VSs) managed via fixed-size pools with ID-based lookup:
```cpp
class vm_pool_t {
    array<vm_t, HYPERVISOR_MAX_VMS> m_pool{};

    [[nodiscard]] constexpr auto allocate(...) -> safe_u16 {
        // Find first available slot, return ID
    }

    constexpr void deallocate(safe_u16 const &vmid) {
        m_pool[vmid].reset();
    }
};
```

**Advantage**: No dynamic allocation, predictable memory usage, O(1) lookup

**Ombra Application**: Use similar pool management for per-CPU VMCS structures

---

## 5. INTEL vs AMD PARITY

### 5.1 Abstraction Strategy
**Source**: Project structure with `x64/intel/` and `x64/amd/` parallel directories

```
kernel/src/x64/
├── intel/
│   ├── intrinsic_t.hpp     # VMX intrinsics
│   ├── promote.S           # VMXON sequence
│   └── intrinsic_vmrun.S   # VMLAUNCH/VMRESUME
└── amd/
    ├── intrinsic_t.hpp     # SVM intrinsics
    ├── promote.S           # VMRUN sequence
    └── intrinsic_vmrun.S   # VMRUN loop
```

**Key Insight**: Same API surface (`intrinsic_t`), platform-specific implementations

### 5.2 Extension Portability
**Source**: `example/default/src/x64/` structure

Extensions have CPU-specific VMExit handlers but share:
- Syscall interface (`bf_syscall_t`)
- Resource management (VM/VP/VS pools)
- Memory allocation (page_pool_t, huge_pool_t)

**Ombra Application**: Use `#ifdef ENABLE_INTEL / #else` to select platform-specific code paths

---

## 6. MEMORY MANAGEMENT

### 6.1 Page Pool (4K Allocator)
**Source**: `kernel/src/page_pool_t.hpp`

- Fixed-size pool of 4K pages
- Allocation via `allocate()` returns `page_4k_t*`
- Deallocation optional (microkernel can ignore to avoid fragmentation)

### 6.2 Huge Pool (Large Allocations)
**Source**: `kernel/src/huge_pool_t.hpp`

- Contiguous multi-page allocations
- Used for extension stacks, large buffers
- Tracked per-extension in `ext_t::m_huge_allocs[]`

### 6.3 Root Page Table Management
**Source**: `kernel/src/root_page_table_t.hpp`

- Each extension has its own CR3 (virtual address space isolation)
- Direct map tables per-VM for guest memory access
- Guard pages between extension stacks

---

## 7. LOADER INTERFACE

### 7.1 Loader → Kernel Arguments
**Source**: `loader/include/interface/mk_args_t.hpp`

```cpp
struct mk_args_t {
    uint64 *mk_state;           // Microkernel state
    state_save_t *root_vp_state;  // Root VP (host) saved state
    debug_ring_t *debug_ring;   // Serial output ring buffer
    span<void*> ext_elf_files;  // Extension ELF binaries
    uint64 *rpt;                // Root page table (physical)
    uint64 rpt_phys;            // Root page table address
    span<page_4k_t> page_pool;  // Pre-allocated pages
    span<page_4k_t> huge_pool;  // Pre-allocated huge pages
    uint16 ppid;                // Physical processor ID
    uint16 online_pps;          // Total online processors
};
```

**Ombra Application**: `OmbraBoot` should prepare similar structure for `OmbraPayload`

---

## 8. RECOMMENDATIONS FOR OMBRA

### 8.1 Adopt from Bareflank

1. **Syscall encoding with signature**: Prevents accidental execution of non-syscall code
   - `Refs/codebases/hypervisor/syscall/include/bf_constants.hpp:126-180`

2. **TLB flush abstraction**: Unified API for INVLPG/INVEPT/INVVPID
   - `Refs/codebases/hypervisor/kernel/src/x64/intel/intrinsic_t.hpp:94-120`

3. **Per-PP guard pages**: Detect stack overflow between CPU stacks
   - `Refs/codebases/hypervisor/kernel/src/mk_main_t.hpp:140-200`

4. **Pool-based resource management**: Replace dynamic allocation with ID-based pools
   - `Refs/codebases/hypervisor/kernel/src/vm_pool_t.hpp`

5. **Packed TLS structure**: Single cache-line aligned per-CPU state
   - `Refs/codebases/hypervisor/kernel/src/x64/tls_t.hpp:1-150`

6. **Direct map pattern**: Per-VM page tables for efficient guest memory access
   - `Refs/codebases/hypervisor/kernel/src/ext_t.hpp:126-128`

### 8.2 Skip from Bareflank

1. **ELF loading complexity**: Ombra uses PE/COFF (`.efi`), not ELF
2. **Extension isolation**: Ombra is single-purpose, doesn't need multi-extension support
3. **AUTOSAR compliance**: Overkill for game hacking, adds ceremony

### 8.3 Implementation Guidance

#### For `OmbraPayload/VmExit/VmExit.cpp`:
```cpp
// Adopt Bareflank's dispatch table pattern
[[nodiscard]] constexpr auto
DispatchVmExit(UINT64 ExitReason) -> BOOLEAN
{
    switch (ExitReason) {
        case EXIT_REASON_CPUID:
            return HandleCpuid();
        case EXIT_REASON_EPT_VIOLATION:
            return HandleEptViolation();
        // ...
        default:
            LogError("Unsupported VMExit: 0x%llx", ExitReason);
            DumpVmcs();
            return FALSE;
    }
}
```

#### For `OmbraPayload/Vmx/Ept.cpp`:
```cpp
// Adopt Bareflank's TLB flush abstraction
VOID EptInvalidateTlb(UINT64 GuestAddr, UINT16 Vpid)
{
    if (GuestAddr == 0) {
        // Flush entire VM (Type 1 INVEPT)
        INVEPT_DESCRIPTOR Desc = { GetCurrentEptp(), 0 };
        __invept(1, &Desc);
    } else {
        // Flush single address (Type 0 INVVPID)
        INVVPID_DESCRIPTOR Desc = { Vpid, 0, GuestAddr };
        __invvpid(0, &Desc);
    }
}
```

#### For `OmbraBoot/Hooks/HvLoader.cpp`:
```cpp
// Adopt Bareflank's loader argument structure
struct PAYLOAD_ARGS {
    VOID *MicrokernelState;
    STATE_SAVE *RootVpState;
    DEBUG_RING *DebugRing;
    VOID *PayloadImage;
    UINT64 *RootPageTable;
    UINT64 RootPageTablePhys;
    UINT16 ProcessorId;
    UINT16 OnlineProcessors;
};
```

---

## 9. FILES TO REFERENCE

### Priority 1 (Must Read):
- `kernel/src/x64/intel/intrinsic_t.hpp` - Intel intrinsic abstraction
- `kernel/src/x64/tls_t.hpp` - TLS block layout
- `kernel/src/dispatch_syscall.hpp` - Syscall dispatch pattern
- `syscall/include/bf_constants.hpp` - Syscall encoding
- `example/default/src/x64/intel/dispatch_vmexit.hpp` - VMExit handler example

### Priority 2 (Reference as Needed):
- `kernel/src/ext_t.hpp` - Extension management
- `kernel/src/vm_pool_t.hpp` - Pool-based resource management
- `loader/include/interface/mk_args_t.hpp` - Loader → kernel interface

---

## 10. RISKS & MITIGATIONS

| Risk | Mitigation |
|------|------------|
| C++20 complexity vs Ombra's C/C++ | Extract patterns, not code. Adapt to Ombra's style. |
| AUTOSAR compliance overhead | Skip compliance checks, adopt only structural patterns. |
| Multi-extension design assumptions | Simplify to single-payload model. |
| ELF loading vs PE/COFF | Use pattern concepts (signature validation, section mapping), not ELF-specific code. |

---

**Extraction Complete**: 2025-12-20
**Analyst**: ENI (Hypervisor Research Agent)
**Confidence**: High - Bareflank is well-documented, production-grade reference
