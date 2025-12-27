# hvpp Pattern Extraction

**Target**: Refs/codebases/hvpp/
**Focus**: Minimalist C++ hypervisor - educational Intel VT-x implementation
**Date**: 2025-12-20

---

## 1. C++ VMCS ABSTRACTION

### 1.1 Encoding System - Type-Safe Field Encoding
**Source**: `Refs/codebases/hvpp/src/hvpp/hvpp/ia32/vmx/vmcs.h:10-91`

```cpp
// Constexpr encoding functions build VMCS field identifiers at compile time
static constexpr uint16_t encode(vmcs_access_type_t access_type,
                                  vmcs_type_t type,
                                  vmcs_width_t width,
                                  uint16_t index) noexcept
{
  return
    (static_cast<uint16_t>(access_type))  |
    (static_cast<uint16_t>(index) << 1)   |
    (static_cast<uint16_t>(type)  << 10)  |
    (static_cast<uint16_t>(width) << 13);
}
```

**Pattern**: Compile-time field encoding using bit manipulation instead of bitfields (Visual Studio compatibility)
**Why**: Avoids runtime overhead, ensures correct VMCS field layout

### 1.2 Field Enumeration - Strong Typing
**Source**: `Refs/codebases/hvpp/src/hvpp/hvpp/ia32/vmx/vmcs.h:106-341`

```cpp
enum class field
{
  // CONTROL fields
  ctrl_virtual_processor_identifier = detail::encode_full(type::control, width::_16_bit, 0),
  ctrl_ept_pointer                  = detail::encode_full(type::control, width::_64_bit, 13),

  // VMEXIT fields (read-only)
  vmexit_reason                     = detail::encode_full(type::vmexit, width::_32_bit, 1),
  vmexit_qualification              = detail::encode_full(type::vmexit, width::natural, 0),

  // GUEST state fields
  guest_cr0                         = detail::encode_full(type::guest, width::natural, 0),
  guest_rip                         = detail::encode_full(type::guest, width::natural, 15),

  // HOST state fields
  host_rsp                          = detail::encode_full(type::host, width::natural, 10),
  host_rip                          = detail::encode_full(type::host, width::natural, 11),
};
```

**Pattern**: Enum class with compile-time field calculation prevents magic numbers
**Benefit**: Type-safe VMCS access, impossible to use wrong field encoding

### 1.3 VMCS Structure - Page-Aligned Layout
**Source**: `Refs/codebases/hvpp/src/hvpp/hvpp/ia32/vmx/vmcs.h:94-101`

```cpp
struct alignas(page_size) vmcs_t
{
  uint32_t revision_id;
  uint32_t abort_indicator;

private:
  uint8_t  data[page_size - 2 * sizeof(uint32_t)];
  // ... field enum follows
};
```

**Pattern**: First 8 bytes are revision_id + abort_indicator, rest is opaque
**Critical**: VMCS must be page-aligned, revision ID from IA32_VMX_BASIC MSR

### 1.4 Field Arithmetic - Iteration Support
**Source**: `Refs/codebases/hvpp/src/hvpp/hvpp/ia32/vmx/vmcs.h:344-354`

```cpp
inline vmcs_t::field operator+(vmcs_t::field vmcs_field, int index) noexcept
{ return static_cast<vmcs_t::field>(static_cast<int>(vmcs_field) + index); }

inline vmcs_t::field& operator+=(vmcs_t::field& vmcs_field, int index) noexcept
{ reinterpret_cast<int&>(vmcs_field) += index; return vmcs_field; }
```

**Pattern**: Allows iteration over sequential VMCS fields (e.g., guest_pdpte0 through guest_pdpte3)
**Use Case**: Batch operations on related VMCS fields

---

## 2. VCPU ARCHITECTURE

### 2.1 Per-CPU Structure - State Machine Design
**Source**: `Refs/codebases/hvpp/src/hvpp/hvpp/vcpu.h:22-26,325-356`

```cpp
class vcpu_t final
{
public:
  vcpu_t(vmexit_handler& handler) noexcept;
  ~vcpu_t() noexcept;

  auto start() noexcept -> error_code_t;
  void stop() noexcept;

private:
  enum class state
  {
    off,          // Uninitialized
    initializing, // VMX-root mode, setting up VMCS
    launching,    // Initial VMENTRY executing
    running,      // VCPU operational
    terminating,  // Teardown in progress
    terminated,   // VMX-root mode exited
  };
```

**Pattern**: Explicit state machine prevents invalid transitions
**Safety**: Destructor asserts on invalid states (can't be in initializing/launching during destruction)

### 2.2 Stack Layout - Assembly Integration
**Source**: `Refs/codebases/hvpp/src/hvpp/hvpp/vcpu.h:363-401`

```cpp
struct stack_t
{
  static constexpr auto size = 0x8000;  // 32KB stack

  struct machine_frame_t
  {
    uint64_t rip;
    uint64_t cs;
    uint64_t eflags;
    uint64_t rsp;
    uint64_t ss;
  };

  struct shadow_space_t
  {
    uint64_t dummy[4];  // Windows x64 calling convention
  };

  union
  {
    uint8_t data[size];
    struct
    {
      uint8_t         dummy[size - sizeof(shadow_space_t)
                                 - sizeof(machine_frame_t)
                                 - sizeof(uint64_t)];
      shadow_space_t  shadow_space;
      machine_frame_t machine_frame;
      uint64_t        unused;
    };
  };
};
```

**Pattern**: Stack grows downward, top contains calling convention structures
**Critical**: 16-byte alignment for Windows x64 ABI (XMM instructions require this)
**Verification**: Static asserts check alignment at `Refs/codebases/hvpp/src/hvpp/hvpp/vcpu.cpp:88-122`

### 2.3 Context Union - Memory Optimization
**Source**: `Refs/codebases/hvpp/src/hvpp/hvpp/vcpu.h:409-417`

```cpp
stack_t stack_;

union
{
  // These two contexts never used simultaneously, share memory
  context_t context_;         // Exit context (during VM-exit handling)
  context_t launch_context_;  // Launch context (during initial VMENTRY)
};
```

**Pattern**: Union saves memory since launch happens once, then exit context used repeatedly
**Offset**: Hard-coded offsets in `vcpu.asm` - static asserts verify at compile time

### 2.4 VMCS Storage - Page-Aligned Structures
**Source**: `Refs/codebases/hvpp/src/hvpp/hvpp/vcpu.h:419-431`

```cpp
// All have alignas(PAGE_SIZE) specifier
vmx::vmcs_t       vmxon_;       // VMXON region
vmx::vmcs_t       vmcs_;        // VMCS region
vmx::msr_bitmap_t msr_bitmap_;  // MSR access bitmap (2 pages)
vmx::io_bitmap_t  io_bitmap_;   // I/O port bitmap (2 pages)

fxsave_area_t     fxsave_area_; // SSE register state preservation
```

**Pattern**: Each VCPU owns its VMCS, VMXON region, and control bitmaps
**Memory**: ~20KB per VCPU (4KB VMXON + 4KB VMCS + 8KB MSR + 8KB IO + FXSAVE)

### 2.5 Handler Reference - Polymorphic Dispatch
**Source**: `Refs/codebases/hvpp/src/hvpp/hvpp/vcpu.h:433-437`

```cpp
vmexit_handler& handler_;  // Reference to exit handler (polymorphic)
state           state_;
ept_t*          ept_;      // Optional EPT pointer (nullptr if EPT disabled)
```

**Pattern**: Single handler reference per VCPU, EPT is optional and swappable
**Flexibility**: Can change EPT pointer at runtime via `vcpu_t::ept(ept_t& new_ept)`

### 2.6 Per-VCPU Resources
**Source**: `Refs/codebases/hvpp/src/hvpp/hvpp/vcpu.h:441-465`

```cpp
context_t             resume_context_;     // Guest resume state
spinlock_queue_t      spinlock_queue_;     // Stacked spinlock tracking

mm::memory_mapper     mapper_;             // Guest physical -> host virtual
mm::memory_translator translator_;         // Guest virtual -> guest physical

uint64_t              tsc_entry_;          // TSC on VM-entry
uint64_t              tsc_delta_previous_; // Time in last VM-exit handler
uint64_t              tsc_delta_sum_;      // Cumulative handler time

interrupt_queue_t     pending_interrupt_queue_[interrupt_queue_max];

void*                 user_data_;          // Opaque pointer for handler use
bool                  suppress_rip_adjust_;
```

**Pattern**: Each VCPU has own memory translation, interrupt queue, timing
**Concurrency**: Spinlock queue enables nested lock tracking (RAII guard pattern)

---

## 3. EXIT HANDLER DESIGN

### 3.1 Handler Array Initialization
**Source**: `Refs/codebases/hvpp/src/hvpp/hvpp/vmexit.cpp:8-78`

```cpp
vmexit_handler::vmexit_handler() noexcept
  : handlers_{ {
    &vmexit_handler::handle_exception_or_nmi,           // 0
    &vmexit_handler::handle_external_interrupt,         // 1
    &vmexit_handler::handle_triple_fault,               // 2
    // ... (truncated for brevity)
    &vmexit_handler::handle_execute_cpuid,              // 10
    // ...
    &vmexit_handler::handle_mov_cr,                     // 28
    &vmexit_handler::handle_execute_io_instruction,     // 30
    &vmexit_handler::handle_execute_rdmsr,              // 31
    &vmexit_handler::handle_execute_wrmsr,              // 32
    // ...
    &vmexit_handler::handle_ept_violation,              // 48
    &vmexit_handler::handle_ept_misconfiguration,       // 49
    // ... up to 65 handlers
  } }
{}
```

**Pattern**: Fixed array of member function pointers indexed by exit reason
**Performance**: Direct array lookup, no switch statement overhead

### 3.2 Dispatch Mechanism
**Source**: `Refs/codebases/hvpp/src/hvpp/hvpp/vmexit.cpp:102-106`

```cpp
void vmexit_handler::handle(vcpu_t& vp) noexcept
{
  const auto handler_index = static_cast<int>(vp.exit_reason());
  (this->*handlers_[handler_index])(vp);
}
```

**Pattern**: Cast exit reason to index, invoke via member function pointer
**Benefit**: Single virtual call (`handle()`), then direct member function call

### 3.3 Handler Signatures - Virtual Methods
**Source**: `Refs/codebases/hvpp/src/hvpp/hvpp/vmexit.h:202-267`

```cpp
protected:
  virtual void handle_exception_or_nmi(vcpu_t& vp) noexcept;
  virtual void handle_execute_cpuid(vcpu_t& vp) noexcept;
  virtual void handle_mov_cr(vcpu_t& vp) noexcept;
  virtual void handle_execute_io_instruction(vcpu_t& vp) noexcept;
  virtual void handle_execute_rdmsr(vcpu_t& vp) noexcept;
  virtual void handle_execute_wrmsr(vcpu_t& vp) noexcept;
  virtual void handle_ept_violation(vcpu_t& vp) noexcept;
  virtual void handle_ept_misconfiguration(vcpu_t& vp) noexcept;

  virtual void handle_fallback(vcpu_t& vp) noexcept;         // Default handler
  virtual void handle_vm_fallback(vcpu_t& vp) noexcept;      // VMX instruction fallback
```

**Pattern**: All handlers take `vcpu_t&`, return void, marked noexcept
**Override**: Derived classes override only needed handlers, rest use fallback

### 3.4 Base Handler Stubs
**Source**: `Refs/codebases/hvpp/src/hvpp/hvpp/vmexit.cpp:121-186`

```cpp
void vmexit_handler::handle_exception_or_nmi(vcpu_t& vp)     noexcept { handle_fallback(vp); }
void vmexit_handler::handle_execute_cpuid(vcpu_t& vp)        noexcept { handle_fallback(vp); }
void vmexit_handler::handle_ept_violation(vcpu_t& vp)        noexcept { handle_fallback(vp); }

// VMX instructions use separate fallback
void vmexit_handler::handle_execute_vmcall(vcpu_t& vp)       noexcept { handle_vm_fallback(vp); }
void vmexit_handler::handle_execute_invept(vcpu_t& vp)       noexcept { handle_vm_fallback(vp); }

void vmexit_handler::handle_fallback(vcpu_t& vp)             noexcept { (void)(vp); }
void vmexit_handler::handle_vm_fallback(vcpu_t& vp)          noexcept { (void)(vp); }
```

**Pattern**: Default implementation does nothing (or calls fallback)
**Use Case**: Override `handle_vm_fallback()` once to inject #UD for all VMX instructions

### 3.5 Lifecycle Hooks
**Source**: `Refs/codebases/hvpp/src/hvpp/hvpp/vmexit.cpp:85-100`

```cpp
auto vmexit_handler::setup(vcpu_t& vp) noexcept -> error_code_t
{
  (void)(vp);
  return {};  // Success by default
}

void vmexit_handler::teardown(vcpu_t& vp) noexcept
{
  (void)(vp);
}

void vmexit_handler::terminate(vcpu_t& vp) noexcept
{
  (void)(vp);
}
```

**Pattern**: Three lifecycle methods - setup (VMX-root, before VMLAUNCH), teardown (after VMXOFF), terminate (initiate shutdown)
**Critical**: `terminate()` must NOT call VMXOFF directly - must cause VM-exit that leads to VMXOFF

### 3.6 Compositor Pattern - Handler Stacking
**Source**: `Refs/codebases/hvpp/src/hvpp/hvpp/vmexit.h:289-335`

```cpp
template <typename ...ARGS>
class vmexit_compositor_handler : public vmexit_handler
{
public:
  auto setup(vcpu_t& vp) noexcept -> error_code_t override
  {
    for_each_element(handlers, [&](auto&& handler, int) {
      if (auto err = handler.setup(vp))
      { hvpp_assert(0); }
    });
    return {};
  }

  void handle(vcpu_t& vp) noexcept override
  {
    for_each_element(handlers, [&](auto&& handler, int) {
      handler.handle(vp);
    });
  }

  std::tuple<ARGS...> handlers;
};
```

**Pattern**: Variadic template allows chaining multiple handlers
**Use Case**: `vmexit_compositor_handler<handler_logger, handler_ept_hook>` processes VM-exits through multiple layers

---

## 4. EPT MANAGER

### 4.1 EPT Entry Union - Flexible Access
**Source**: `Refs/codebases/hvpp/src/hvpp/hvpp/ia32/ept.h:250-352`

```cpp
struct epte_t
{
  enum class access_type : uint32_t
  {
    none               = 0b0000'0000,
    read               = 0b0000'0001,
    write              = 0b0000'0010,
    execute            = 0b0000'0100,
    read_write         = read  |  write,
    read_execute       = read  | execute,
    read_write_execute = read  |  write  | execute,
  };

  union
  {
    uint64_t          flags;

    ept_pml4e_t       pml4;
    ept_pdpte_large_t pdpt_large;  // 1GB page
    ept_pdpte_t       pdpt;
    ept_pde_large_t   pd_large;    // 2MB page
    ept_pde_t         pd;
    ept_pte_t         pt;          // 4KB page

    // Common fields across all entry types
    struct
    {
      uint64_t read_access : 1;
      uint64_t write_access : 1;
      uint64_t execute_access : 1;
      uint64_t memory_type : 3;
      uint64_t ignore_pat : 1;
      uint64_t large_page : 1;
      uint64_t accessed : 1;
      uint64_t dirty : 1;
      uint64_t user_mode_execute : 1;
      uint64_t reserved_1 : 1;
      uint64_t page_frame_number : 36;
      uint64_t reserved_2 : 15;
      uint64_t suppress_ve : 1;
    };

    struct
    {
      uint64_t access : 3;  // Combines read/write/execute bits
    };
  };
```

**Pattern**: Union allows treating same 64-bit value as different page table entry types
**Convenience**: Common fields accessible regardless of level in page table hierarchy

### 4.2 EPT Entry Manipulation
**Source**: `Refs/codebases/hvpp/src/hvpp/hvpp/ia32/ept.h:307-352`

```cpp
void update(access_type new_access) noexcept
{
  access = uint64_t(new_access);
}

void update(pa_t pa, access_type new_access = access_type::read_write_execute) noexcept
{
  update(new_access);
  page_frame_number = pa.pfn();
}

void update(pa_t pa, ia32::memory_type type, access_type new_access = ...) noexcept
{
  update(pa, new_access);
  memory_type = static_cast<uint64_t>(type);
}

epte_t* subtable() const noexcept
{
  return present()
    ? reinterpret_cast<epte_t*>(pa_t::from_pfn(page_frame_number).va())
    : nullptr;
}

bool present() const noexcept
{
  return read_access || write_access || execute_access;
}
```

**Pattern**: Overloaded `update()` methods for different configuration scenarios
**Safety**: `present()` checks any permission bit set (EPT uses RWX instead of Present bit)

### 4.3 EPT Manager Class
**Source**: `Refs/codebases/hvpp/src/hvpp/hvpp/ept.h:11-81`

```cpp
class ept_t final
{
public:
  ept_t() noexcept;
  ~ept_t() noexcept;

  // Identity mapping (guest PA = host PA)
  void map_identity(epte_t::access_type access = ...) noexcept;
  void map_identity_sparse(epte_t::access_type access = ...) noexcept;

  // Flexible mapping
  epte_t* map(pa_t guest_pa, pa_t host_pa,
              epte_t::access_type access = ...,
              pml level = pml::pt) noexcept;

  epte_t* map_4kb(pa_t guest_pa, pa_t host_pa, ...) noexcept;
  epte_t* map_2mb(pa_t guest_pa, pa_t host_pa, ...) noexcept;
  epte_t* map_1gb(pa_t guest_pa, pa_t host_pa, ...) noexcept;

  // Page splitting/joining
  void split_1gb_to_2mb(pa_t guest_pa, pa_t host_pa, ...) noexcept;
  void split_2mb_to_4kb(pa_t guest_pa, pa_t host_pa, ...) noexcept;
  void join_2mb_to_1gb(pa_t guest_pa, pa_t host_pa, ...) noexcept;
  void join_4kb_to_2mb(pa_t guest_pa, pa_t host_pa, ...) noexcept;

  epte_t*   ept_entry(pa_t guest_pa, pml level = pml::pt) noexcept;
  ept_ptr_t ept_pointer() const noexcept;

private:
  alignas(page_size)
  epte_t    epml4_[512];   // Root EPT PML4 table
  ept_ptr_t eptptr_;       // EPT pointer for VMCS
};
```

**Pattern**: Self-contained EPT manager owns PML4 root, provides high-level mapping API
**Flexibility**: Supports runtime page splitting (1GB→2MB→4KB) for fine-grained hooking

### 4.4 EPT Pointer Structure
**Source**: `Refs/codebases/hvpp/src/hvpp/hvpp/ia32/ept.h:166-186`

```cpp
struct ept_ptr_t
{
  static constexpr int page_walk_length_4 = 3;  // 4-level walk (value is N-1)

  union
  {
    uint64_t flags;

    struct
    {
      uint64_t memory_type : 3;              // EPT paging structure memory type
      uint64_t page_walk_length : 3;         // Page walk length (3 = 4 levels)
      uint64_t enable_access_and_dirty_flags : 1;
      uint64_t reserved_1 : 5;
      uint64_t page_frame_number : 36;       // PFN of EPT PML4 table
    };
  };
};
```

**Pattern**: Encapsulates EPTP value written to VMCS ctrl_ept_pointer field
**Setup**: `page_walk_length = 3` for 4-level (PML4→PDPT→PD→PT), memory_type usually WB

### 4.5 EPT Type Descriptors
**Source**: `Refs/codebases/hvpp/src/hvpp/hvpp/ia32/ept.h:193-244`

```cpp
struct ept_pt_t
{
  using        descriptor_tag = ept_descriptor_tag;
  using                 entry = ept_pte_t;
  static constexpr auto level = pml::pt;
  static constexpr auto count = uint64_t(512);
  static constexpr auto shift = uint64_t(12);           // 4KB shift
  static constexpr auto size  = uint64_t(1) << shift;   // 4KB
  static constexpr auto mask  = ~(size - 1);
};

struct ept_pd_t
{
  using                 entry = ept_pde_t;
  static constexpr auto level = pml::pd;
  static constexpr auto shift = ept_pt_t::shift + 9;    // 21 (2MB)
  static constexpr auto size  = uint64_t(1) << shift;
  // ...
};

struct ept_pdpt_t
{
  static constexpr auto shift = ept_pd_t::shift + 9;    // 30 (1GB)
  // ...
};

struct ept_pml4_t
{
  static constexpr auto shift = ept_pdpt_t::shift + 9;  // 39 (512GB)
  // ...
};
```

**Pattern**: Type descriptors define page table level characteristics at compile time
**Use Case**: Template-based page table walking code can use these descriptors generically

---

## KEY PATTERNS FOR OMBRA

### Pattern 1: Compile-Time VMCS Encoding
**Apply**: Use constexpr encoding for VMCS field identifiers
**Benefit**: Zero runtime overhead, type-safe access

### Pattern 2: VCPU State Machine
**Apply**: Explicit state enum prevents invalid VCPU transitions
**Benefit**: Easier debugging, clearer error handling

### Pattern 3: Handler Array Dispatch
**Apply**: Fixed array of function pointers indexed by exit reason
**Benefit**: O(1) dispatch, no switch statement bloat

### Pattern 4: EPT Entry Union
**Apply**: Union of all EPT entry types + common fields
**Benefit**: Single interface for all page table levels

### Pattern 5: Page Table Type Descriptors
**Apply**: Compile-time descriptor structs for PT/PD/PDPT/PML4
**Benefit**: Generic page table code works at all levels

### Pattern 6: Stack Layout Validation
**Apply**: Static asserts verify assembly offset assumptions
**Benefit**: Catch calling convention violations at compile time

---

## ARCHITECTURAL DECISIONS

### Why Member Function Pointers?
- **Pro**: Single virtual call (`handle()`), then direct dispatch
- **Pro**: Array initialization validates handler count at compile time
- **Con**: Slightly larger code than function pointer array

### Why Union for Launch/Exit Context?
- **Pro**: Saves ~512 bytes per VCPU (context_t size)
- **Pro**: Launch context only used once, exit context reused constantly
- **Safe**: Compiler enforces exclusive access via union

### Why Page-Aligned VMCS/VMXON/Bitmaps?
- **Required**: Intel specification mandates 4KB alignment
- **Implementation**: C++ `alignas(page_size)` attribute on struct members

### Why EPT Class Per-VCPU vs. Shared?
- **hvpp Choice**: Each VCPU can have different EPT (ept_t* pointer)
- **Flexibility**: Allows per-VCPU memory views or EPT swapping
- **Ombra Choice**: Likely single shared EPT for all VCPUs (simpler)

---

## FILES REFERENCED

- `Refs/codebases/hvpp/src/hvpp/hvpp/ia32/vmx/vmcs.h` - VMCS abstraction
- `Refs/codebases/hvpp/src/hvpp/hvpp/vcpu.h` - VCPU class definition
- `Refs/codebases/hvpp/src/hvpp/hvpp/vcpu.cpp` - VCPU implementation
- `Refs/codebases/hvpp/src/hvpp/hvpp/vmexit.h` - Handler base class
- `Refs/codebases/hvpp/src/hvpp/hvpp/vmexit.cpp` - Handler dispatch
- `Refs/codebases/hvpp/src/hvpp/hvpp/ept.h` - EPT manager class
- `Refs/codebases/hvpp/src/hvpp/hvpp/ia32/ept.h` - EPT entry structures
- `Refs/codebases/hvpp/src/hvpp/hvpp/hypervisor.h` - Top-level API
- `Refs/codebases/hvpp/src/hvpp/hvpp/hypervisor.cpp` - Initialization

---

**End of hvpp Extraction**
