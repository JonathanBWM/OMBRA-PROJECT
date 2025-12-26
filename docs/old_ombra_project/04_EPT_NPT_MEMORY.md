# EPT/NPT MEMORY VIRTUALIZATION - C++ to C + Assembly Port Guide

## Overview

This document provides comprehensive guidance for porting EPT (Extended Page Tables) and NPT (Nested Page Tables) memory virtualization code from C++ to pure C with inline assembly. EPT/NPT are hardware-assisted second-level address translation mechanisms used in Intel VT-x and AMD SVM respectively.

**Purpose**: Enable hypervisors to control guest physical-to-host physical memory mappings, providing memory isolation, page-level hooks, and execute-only permissions.

**Key Difference**: Intel EPT and AMD NPT differ in structure details, permission semantics, and TLB invalidation mechanisms.

---

## File Inventory

### Current C++ Implementation

#### Intel VMX Memory Management
- **PayLoad/intel/mm.h** - Intel EPT interface (395 lines)
- **PayLoad/intel/mm.cpp** - Intel EPT implementation (405 lines)

#### AMD SVM Memory Management
- **PayLoad/amd/mm.h** - AMD NPT interface (264 lines)
- **PayLoad/amd/mm.cpp** - AMD NPT implementation (442 lines)

#### Kernel Library Paging
- **OmbraCoreLib/OmbraCoreLib/include/paging.h** - Kernel paging API (145 lines)
- **OmbraCoreLib/OmbraCoreLib/src/paging.cpp** - Kernel paging implementation (470 lines)
- **OmbraCoreLib/OmbraCoreLib/include/Arch/Pte.h** - Page table entry definitions (1561 lines)

#### EPT/NPT Advanced Features
- **OmbraCoreLib/OmbraCoreLib-v/include/EPT.h** - EPT hooking interface (23 lines)
- **OmbraCoreLib/OmbraCoreLib-v/src/EPT.cpp** - EPT hooking implementation (1659 lines)

---

## Architecture Summary

### Intel EPT (Extended Page Tables)

EPT provides guest-physical to host-physical address translation via a 4-level page table structure:

```
Guest Virtual → Guest Page Tables (CR3) → Guest Physical → EPT → Host Physical
```

**EPT Walk**: `EPTP → EPT PML4 → EPT PDPT → EPT PD → EPT PT → Host Physical Page`

**Key Features**:
- Execute-only permissions (if supported by CPU)
- Memory type control (UC, WC, WT, WP, WB)
- Access and dirty flag tracking
- Per-page permissions (Read, Write, Execute)

### AMD NPT (Nested Page Tables)

NPT provides similar functionality using a 4-level nested page table:

```
Guest Virtual → Guest Page Tables (gCR3) → Guest Physical → NPT → System Physical
```

**NPT Walk**: `nCR3 → NPT PML4 → NPT PDPT → NPT PD → NPT PT → System Physical Page`

**Key Features**:
- Standard page table entry format
- User/supervisor bits
- Cache control bits
- NX (No Execute) bit

### Ombra's Dual EPT/NPT Strategy

Ombra uses **TWO** EPT/NPT tables per core:

1. **Primary EPT** (`vmm::vGuestStates[core].eptState`) - Normal execution, hooks active
2. **Shadow EPT** (`vmm::eptShadow[core]`) - All pages execute-disabled except whitelisted kernel code

**Hook Mechanism**:
- Primary EPT: Target page → Hook page (execute-disabled, causes EPT violation)
- Shadow EPT: Target page → Original page (execute-enabled for kernel)
- EPT violation → Toggle between Primary/Shadow → Single-step → Restore

---

## EPT/NPT Page Table Structures

### Intel EPT Structures

#### EPT Pointer (EPTP) - 64 bits
```c
typedef union _ept_pointer {
    u64 flags;
    struct {
        u64 memory_type : 3;                  // 0=UC, 6=WB
        u64 page_walk_length : 3;             // Value is 1 less than walk (3=4-level)
        u64 enable_access_and_dirty_flags : 1;
        u64 reserved1 : 5;
        u64 page_frame_number : 36;           // Physical address >> 12
        u64 reserved2 : 16;
    };
} ept_pointer;
```

#### EPT PML4 Entry - 64 bits
```c
typedef union _ept_pml4e {
    u64 flags;
    struct {
        u64 read_access : 1;
        u64 write_access : 1;
        u64 execute_access : 1;
        u64 reserved1 : 5;
        u64 accessed : 1;
        u64 reserved2 : 1;
        u64 user_mode_execute : 1;
        u64 reserved3 : 1;
        u64 page_frame_number : 36;           // Next level table physical address >> 12
        u64 reserved4 : 16;
    };
} ept_pml4e;
```

#### EPT PDPTE (1GB Large Page) - 64 bits
```c
typedef union _ept_pdpte_1gb {
    u64 flags;
    struct {
        u64 read_access : 1;
        u64 write_access : 1;
        u64 execute_access : 1;
        u64 memory_type : 3;                  // MTRR memory type
        u64 ignore_pat : 1;
        u64 large_page : 1;                   // Must be 1 for 1GB page
        u64 accessed : 1;
        u64 dirty : 1;
        u64 user_mode_execute : 1;
        u64 reserved1 : 19;
        u64 page_frame_number : 18;           // 1GB aligned (bits 47:30)
        u64 reserved2 : 15;
        u64 suppress_ve : 1;
    };
} ept_pdpte_1gb;
```

#### EPT PDPTE (Table Reference) - 64 bits
```c
typedef union _ept_pdpte {
    u64 flags;
    struct {
        u64 read_access : 1;
        u64 write_access : 1;
        u64 execute_access : 1;
        u64 reserved1 : 5;
        u64 accessed : 1;
        u64 reserved2 : 1;
        u64 user_mode_execute : 1;
        u64 reserved3 : 1;
        u64 page_frame_number : 36;           // PD physical address >> 12
        u64 reserved4 : 16;
    };
} ept_pdpte;
```

#### EPT PDE (2MB Large Page) - 64 bits
```c
typedef union _epde_2mb {
    u64 flags;
    struct {
        u64 read_access : 1;
        u64 write_access : 1;
        u64 execute_access : 1;
        u64 memory_type : 3;
        u64 ignore_pat : 1;
        u64 large_page : 1;                   // Must be 1 for 2MB page
        u64 accessed : 1;
        u64 dirty : 1;
        u64 user_mode_execute : 1;
        u64 reserved1 : 10;
        u64 page_frame_number : 27;           // 2MB aligned (bits 47:21)
        u64 reserved2 : 15;
        u64 suppress_ve : 1;
    };
} epde_2mb;
```

#### EPT PDE (Table Reference) - 64 bits
```c
typedef union _ept_pde {
    u64 flags;
    struct {
        u64 read_access : 1;
        u64 write_access : 1;
        u64 execute_access : 1;
        u64 reserved1 : 5;
        u64 accessed : 1;
        u64 reserved2 : 1;
        u64 user_mode_execute : 1;
        u64 reserved3 : 1;
        u64 page_frame_number : 36;           // PT physical address >> 12
        u64 reserved4 : 16;
    };
} ept_pde;
```

#### EPT PTE (4KB Page) - 64 bits
```c
typedef union _ept_pte {
    u64 flags;
    struct {
        u64 read_access : 1;
        u64 write_access : 1;
        u64 execute_access : 1;
        u64 memory_type : 3;
        u64 ignore_pat : 1;
        u64 reserved1 : 1;
        u64 accessed : 1;
        u64 dirty : 1;
        u64 user_mode_execute : 1;
        u64 reserved2 : 1;
        u64 page_frame_number : 36;           // Physical page address >> 12
        u64 reserved3 : 15;
        u64 suppress_ve : 1;
    };
} ept_pte;
```

### AMD NPT Structures

#### NPT PML4E/PDPTE/PDE (Table References) - 64 bits
```c
typedef struct _npt_pml4e {
    union {
        u64 value;
        struct {
            u64 present : 1;
            u64 writeable : 1;
            u64 user : 1;
            u64 write_through : 1;
            u64 cache_disable : 1;
            u64 accessed : 1;
            u64 reserved1 : 3;
            u64 avl : 3;                      // Available to software
            u64 pfn : 40;                     // Next level table physical address >> 12
            u64 reserved2 : 11;
            u64 nx : 1;                       // No Execute
        };
    };
} npt_pml4e, npt_pdpte, npt_pde;
```

#### NPT PTE (4KB Page) - 64 bits
```c
typedef struct _npt_pte {
    union {
        u64 value;
        struct {
            u64 present : 1;
            u64 writeable : 1;
            u64 user : 1;
            u64 write_through : 1;
            u64 cache_disable : 1;
            u64 accessed : 1;
            u64 dirty : 1;
            u64 pat : 1;                      // Page Attribute Table
            u64 global : 1;
            u64 avl : 3;
            u64 pfn : 40;                     // Physical page address >> 12
            u64 reserved : 11;
            u64 nx : 1;
        };
    };
} npt_pte;
```

#### NPT PDE (2MB Large Page) - 64 bits
```c
typedef struct _npt_pde_2mb {
    union {
        u64 value;
        struct {
            u64 present : 1;
            u64 writeable : 1;
            u64 user : 1;
            u64 write_through : 1;
            u64 cache_disable : 1;
            u64 accessed : 1;
            u64 dirty : 1;
            u64 large_page : 1;               // Must be 1 for 2MB page
            u64 global : 1;
            u64 avl : 3;
            u64 pat : 1;
            u64 reserved1 : 8;
            u64 pfn : 31;                     // 2MB aligned (bits 47:21)
            u64 reserved2 : 11;
            u64 nx : 1;
        };
    };
} npt_pde_2mb;
```

---

## Key Functions

### EPT Table Construction (OmbraCoreLib-v/EPT.cpp)

#### BuildMtrrMap - Construct MTRR Memory Range Map

**Purpose**: Enumerate CPU MTRR (Memory Type Range Registers) to determine cache attributes for EPT entries.

**C++ Implementation**:
```cpp
BOOLEAN EPT::BuildMtrrMap(EPT_STATE* pEptState)
{
    IA32_MTRR_CAPABILITIES_REGISTER MTRRCap;
    IA32_MTRR_PHYSBASE_REGISTER CurrentPhysBase;
    IA32_MTRR_PHYSMASK_REGISTER CurrentPhysMask;
    PMTRR_RANGE_DESCRIPTOR Descriptor;
    ULONG CurrentRegister;
    ULONG NumberOfBitsInMask;

    MTRRCap.Flags = __readmsr(MSR_IA32_MTRR_CAPABILITIES);

    for (CurrentRegister = 0; CurrentRegister < MTRRCap.VariableRangeCount; CurrentRegister++)
    {
        CurrentPhysBase.Flags = __readmsr(MSR_IA32_MTRR_PHYSBASE0 + (CurrentRegister * 2));
        CurrentPhysMask.Flags = __readmsr(MSR_IA32_MTRR_PHYSMASK0 + (CurrentRegister * 2));

        if (CurrentPhysMask.Valid)
        {
            Descriptor = &pEptState->MemoryRanges[pEptState->NumberOfEnabledMemoryRanges++];
            Descriptor->PhysicalBaseAddress = CurrentPhysBase.PageFrameNumber * PAGE_SIZE;

            _BitScanForward64(&NumberOfBitsInMask, CurrentPhysMask.PageFrameNumber * PAGE_SIZE);
            Descriptor->PhysicalEndAddress = Descriptor->PhysicalBaseAddress + ((1ULL << NumberOfBitsInMask) - 1ULL);
            Descriptor->MemoryType = (UCHAR)CurrentPhysBase.Type;

            if (Descriptor->MemoryType == MEMORY_TYPE_WRITE_BACK)
                pEptState->NumberOfEnabledMemoryRanges--;
        }
    }

    return TRUE;
}
```

**C Conversion Notes**:
- Replace `__readmsr` with inline assembly: `__asm__ __volatile__("rdmsr" : "=a"(low), "=d"(high) : "c"(msr))`
- Replace `_BitScanForward64` with GCC builtin: `__builtin_ctzll()` or manual loop
- Manual memory management for `pEptState->MemoryRanges` array

#### CreatePageTable - Construct 4-Level EPT/NPT Table

**Purpose**: Allocate and initialize identity-mapped EPT/NPT tables covering 512GB.

**C++ Implementation** (Simplified):
```cpp
PVMM_EPT_PAGE_TABLE EPT::CreatePageTable(EPT_STATE* pEptState, PML2E_2MB PML2EntryTemplate, ULONG pml4Index)
{
    PVMM_EPT_PAGE_TABLE PageTable;
    SIZE_T EntryGroupIndex;
    SIZE_T EntryIndex;

    PageTable = (PVMM_EPT_PAGE_TABLE)cpp::kMalloc(sizeof(VMM_EPT_PAGE_TABLE));
    RtlZeroMemory(PageTable, sizeof(VMM_EPT_PAGE_TABLE));

    // Mark first 512GB PML4 entry as present
    PageTable->PML4[0].SetPFN(Memory::VirtToPhy(&PageTable->PML3[0]) / PAGE_SIZE);
    PageTable->PML4[0].SetReadWrite(true);
    PageTable->PML4[0].SetExecute(true);
    PageTable->PML4[0].SetValid(true);

    // Initialize all 512 PML3 entries (each covers 1GB)
    for (EntryIndex = 0; EntryIndex < VMM_EPT_PML3E_COUNT; EntryIndex++)
    {
        PageTable->PML3[EntryIndex].SetPFN(Memory::VirtToPhy(&PageTable->PML2[EntryIndex][0]) / PAGE_SIZE);
        PageTable->PML3[EntryIndex].SetReadWrite(true);
        PageTable->PML3[EntryIndex].SetExecute(true);
        PageTable->PML3[EntryIndex].SetValid(true);
    }

    // Initialize all PML2 entries (2MB large pages)
    __stosq((SIZE_T*)&PageTable->PML2[0], PML2EntryTemplate.Flags, VMM_EPT_PML3E_COUNT * VMM_EPT_PML2E_COUNT);

    for (EntryGroupIndex = 0; EntryGroupIndex < VMM_EPT_PML3E_COUNT; EntryGroupIndex++)
    {
        for (EntryIndex = 0; EntryIndex < VMM_EPT_PML2E_COUNT; EntryIndex++)
        {
            SetupPML2Entry(pEptState, &PageTable->PML2[EntryGroupIndex][EntryIndex],
                          (((pml4Index * 512) + EntryGroupIndex) * VMM_EPT_PML2E_COUNT) + EntryIndex);
        }
    }

    return PageTable;
}
```

**C Conversion Notes**:
- Replace `cpp::kMalloc` with `malloc()` or custom allocator
- Replace `RtlZeroMemory` with `memset(ptr, 0, size)`
- Replace `__stosq` with manual loop or `memset_pattern8()` equivalent:
  ```c
  for (size_t i = 0; i < count; i++) {
      dest[i] = pattern;
  }
  ```
- Replace C++ method calls (`SetPFN`, `SetReadWrite`) with direct bit manipulation:
  ```c
  entry.flags = 0;
  entry.page_frame_number = pfn;
  entry.read_access = 1;
  entry.write_access = 1;
  entry.execute_access = 1;
  ```

#### SetupPML2Entry - Initialize 2MB Page Entry with MTRR Cache Type

**C++ Implementation**:
```cpp
VOID SetupPML2Entry(EPT_STATE* pEptState, PPML2E_2MB NewEntry, SIZE_T PageFrameNumber)
{
    SIZE_T AddressOfPage;
    SIZE_T CurrentMtrrRange;
    SIZE_T TargetMemoryType;

    NewEntry->SetPFN(PageFrameNumber);
    NewEntry->SetValid(true);
    NewEntry->SetReadWrite(true);

    AddressOfPage = PageFrameNumber * SIZE_2_MB;

    if (PageFrameNumber == 0)
    {
        NewEntry->SetPATWriteback(false);  // First MB is uncacheable (MMIO)
        return;
    }

    TargetMemoryType = MEMORY_TYPE_WRITE_BACK;

    // Check MTRR ranges
    for (CurrentMtrrRange = 0; CurrentMtrrRange < pEptState->NumberOfEnabledMemoryRanges; CurrentMtrrRange++)
    {
        if (AddressOfPage <= pEptState->MemoryRanges[CurrentMtrrRange].PhysicalEndAddress
            && (AddressOfPage + SIZE_2_MB - 1) >= pEptState->MemoryRanges[CurrentMtrrRange].PhysicalBaseAddress)
        {
            TargetMemoryType = pEptState->MemoryRanges[CurrentMtrrRange].MemoryType;

            if (TargetMemoryType == MEMORY_TYPE_UNCACHEABLE)
                break;  // UC always takes precedence
        }
    }

    NewEntry->SetPATWriteback(TargetMemoryType == MEMORY_TYPE_WRITE_BACK);
}
```

**C Conversion**:
```c
void setup_pml2_entry(ept_state_t* ept_state, epde_2mb* new_entry, u64 page_frame_number)
{
    u64 address_of_page;
    u64 current_mtrr_range;
    u64 target_memory_type;

    new_entry->flags = 0;
    new_entry->page_frame_number = page_frame_number;
    new_entry->read_access = 1;
    new_entry->write_access = 1;
    new_entry->execute_access = 1;
    new_entry->large_page = 1;
    new_entry->user_mode_execute = 1;

    address_of_page = page_frame_number * SIZE_2_MB;

    if (page_frame_number == 0) {
        new_entry->memory_type = MEMORY_TYPE_UNCACHEABLE;
        return;
    }

    target_memory_type = MEMORY_TYPE_WRITE_BACK;

    for (current_mtrr_range = 0; current_mtrr_range < ept_state->num_enabled_memory_ranges; current_mtrr_range++)
    {
        if (address_of_page <= ept_state->memory_ranges[current_mtrr_range].physical_end_address
            && (address_of_page + SIZE_2_MB - 1) >= ept_state->memory_ranges[current_mtrr_range].physical_base_address)
        {
            target_memory_type = ept_state->memory_ranges[current_mtrr_range].memory_type;

            if (target_memory_type == MEMORY_TYPE_UNCACHEABLE)
                break;
        }
    }

    new_entry->memory_type = target_memory_type;
}
```

### Page Splitting (OmbraCoreLib-v/EPT.cpp)

#### SplitLargePage - Convert 2MB Page to 512x4KB Pages

**Purpose**: Split a 2MB large page into 512 individual 4KB pages for fine-grained permission control.

**C++ Implementation**:
```cpp
BOOLEAN EPT::SplitLargePage(PVMM_EPT_PAGE_TABLE pEpt, PVOID PBuf, SIZE_T pa, BOOLEAN bVmxRoot)
{
    PVMM_EPT_DYNAMIC_SPLIT NewSplit;
    PML1E EntryTemplate = { 0 };
    SIZE_T EntryIndex;
    PPML2E_2MB TargetEntry;
    PPML2E Target;
    PML2E NewPointer = { 0 };

    // Find the PML2 entry currently used
    TargetEntry = (PPML2E_2MB)GetPml2Entry(pEpt, pa);
    if (!TargetEntry)
        return FALSE;

    Target = (PPML2E)TargetEntry;

    // If already split, return success
    if (!TargetEntry->GetLarge())
        return TRUE;

    // Allocate PML1 entries (512 * 4KB pages)
    NewSplit = (PVMM_EPT_DYNAMIC_SPLIT)PBuf;
    if (!NewSplit)
        return FALSE;

    RtlZeroMemory(NewSplit, sizeof(VMM_EPT_DYNAMIC_SPLIT));

    // Create template for RWX 4KB pages
    EntryTemplate.SetReadWrite(true);
    EntryTemplate.SetExecute(true);
    EntryTemplate.SetPATWriteback(true);
    EntryTemplate.SetValid(true);

    // Set page frame numbers for identity mapping
    for (EntryIndex = 0; EntryIndex < VMM_EPT_PML1E_COUNT; EntryIndex++)
    {
        EntryTemplate.SetPFN(((TargetEntry->GetPFN() * SIZE_2_MB) / PAGE_SIZE) + EntryIndex);
        NewSplit->PML1[EntryIndex].Flags = EntryTemplate.Flags;
    }

    // Create new pointer to replace 2MB entry
    NewPointer.Flags = 0;
    NewPointer.SetReadWrite(true);
    NewPointer.SetValid(true);
    NewPointer.SetExecute(true);

    if (bVmxRoot) {
        CR3 guestCR3 = vmm::GetGuestCR3();
        NewPointer.SetPFN(paging::vmmhost::GuestVirtToPhy(&NewSplit->PML1[0], (PVOID)(guestCR3.AddressOfPageDirectory * PAGE_SIZE)) / PAGE_SIZE);
    } else {
        NewPointer.SetPFN(Memory::VirtToPhy(&NewSplit->PML1[0]) / PAGE_SIZE);
    }

    // Replace 2MB entry with pointer to PML1 table
    RtlCopyMemory(TargetEntry, &NewPointer, sizeof(NewPointer));

    return TRUE;
}
```

**C Conversion**:
```c
bool split_large_page(vmm_ept_page_table* ept, void* buffer, u64 physical_address, bool vmx_root)
{
    vmm_ept_dynamic_split* new_split;
    ept_pte entry_template;
    u64 entry_index;
    epde_2mb* target_entry;
    ept_pde* target;
    ept_pde new_pointer;

    target_entry = (epde_2mb*)get_pml2_entry(ept, physical_address);
    if (!target_entry)
        return false;

    target = (ept_pde*)target_entry;

    if (!target_entry->large_page)
        return true;  // Already split

    new_split = (vmm_ept_dynamic_split*)buffer;
    if (!new_split)
        return false;

    memset(new_split, 0, sizeof(vmm_ept_dynamic_split));

    // Initialize template
    entry_template.flags = 0;
    entry_template.read_access = 1;
    entry_template.write_access = 1;
    entry_template.execute_access = 1;
    entry_template.memory_type = MEMORY_TYPE_WRITE_BACK;

    // Set page frame numbers
    for (entry_index = 0; entry_index < 512; entry_index++)
    {
        entry_template.page_frame_number = ((target_entry->page_frame_number * SIZE_2_MB) / PAGE_SIZE) + entry_index;
        new_split->pml1[entry_index].flags = entry_template.flags;
    }

    // Create new PDE pointer
    new_pointer.flags = 0;
    new_pointer.read_access = 1;
    new_pointer.write_access = 1;
    new_pointer.execute_access = 1;

    if (vmx_root) {
        u64 guest_cr3 = get_guest_cr3();
        new_pointer.page_frame_number = guest_virt_to_phys(&new_split->pml1[0], (void*)(guest_cr3 * PAGE_SIZE)) / PAGE_SIZE;
    } else {
        new_pointer.page_frame_number = virt_to_phys(&new_split->pml1[0]) / PAGE_SIZE;
    }

    memcpy(target_entry, &new_pointer, sizeof(new_pointer));

    return true;
}
```

### TLB Invalidation

#### Intel INVEPT - Invalidate EPT TLB Entries

**C++ Implementation**:
```cpp
VMX_ERROR EPT::InvalidateEPT(DWORD dwCore)
{
    VMX_ERROR res;
    INVEPT_DESCRIPTOR Descriptor = { 0, 0 };
    res = CPU::InveptContext(InveptAllContext, &Descriptor);
    return res;
}
```

**Assembly Implementation** (Intel syntax):
```asm
; INVEPT - Invalidate EPT mappings
; Input: rcx = INVEPT type (1=single-context, 2=all-context)
;        rdx = pointer to INVEPT_DESCRIPTOR
invept_all_context:
    mov rax, 2              ; InveptAllContext
    invept rax, [rdx]       ; INVEPT instruction
    jnc .success            ; CF=0 on success
    mov rax, 1              ; VMX_ERROR_CODE_FAILED
    ret
.success:
    xor rax, rax            ; VMX_ERROR_CODE_SUCCESS
    ret
```

**C Wrapper**:
```c
typedef struct _invept_descriptor {
    u64 eptp;
    u64 reserved;
} invept_descriptor;

typedef enum _invept_type {
    invept_single_context = 1,
    invept_all_context = 2
} invept_type;

u32 invalidate_ept(invept_type type, invept_descriptor* descriptor)
{
    u32 result;
    __asm__ __volatile__(
        "invept %1, %2\n\t"
        "jnc 1f\n\t"
        "mov $1, %0\n\t"
        "jmp 2f\n\t"
        "1: xor %0, %0\n\t"
        "2:\n\t"
        : "=r"(result)
        : "r"((u64)type), "m"(*descriptor)
        : "cc"
    );
    return result;
}
```

#### AMD INVLPGA - Invalidate NPT TLB Entries

**C++ Implementation**:
```cpp
void SVM::ClearEntireTLB(SVM_STATE* SvmState)
{
    SvmState->GuestVmcb->ControlArea.TlbControl = TLB_CONTROL_FLUSH_ENTIRE_ASID;
}
```

**C Implementation**:
```c
typedef enum _tlb_control {
    TLB_CONTROL_DO_NOTHING = 0,
    TLB_CONTROL_FLUSH_ENTIRE_ASID = 1,
    TLB_CONTROL_FLUSH_THIS_ASID = 3,
    TLB_CONTROL_FLUSH_ALL_ASID_NON_GLOBALS = 7
} tlb_control;

void clear_entire_tlb(svm_state* svm_state)
{
    svm_state->guest_vmcb->control_area.tlb_control = TLB_CONTROL_FLUSH_ENTIRE_ASID;
}
```

**Alternative: INVLPGA Instruction** (AMD-specific):
```asm
; INVLPGA - Invalidate TLB entry for virtual address
; Input: rax = virtual address
;        ecx = ASID
invlpga_addr:
    invlpga                 ; INVLPGA instruction (implicit rax, ecx)
    ret
```

**C Wrapper**:
```c
void invlpga(u64 virtual_address, u32 asid)
{
    __asm__ __volatile__(
        "invlpga"
        :
        : "a"(virtual_address), "c"(asid)
        : "memory"
    );
}
```

### Address Translation

#### Intel EPT Translation - Guest Physical to Host Physical

**C++ Implementation**:
```cpp
auto translate_guest_physical(guest_phys_t phys_addr, map_type_t map_type) -> u64
{
    ept_pointer eptp;
    phys_addr_t guest_phys{ phys_addr };
    __vmx_vmread(VMCS_CTRL_EPT_POINTER, (size_t*)&eptp);

    // Map EPT PML4
    const auto epml4 = reinterpret_cast<ept_pml4e*>(
        map_page(eptp.page_frame_number << 12, map_type));

    // Map EPDPT and check for 1GB large page
    const auto epdpt_large = reinterpret_cast<ept_pdpte_1gb*>(map_page(
        epml4[guest_phys.pml4_index].page_frame_number << 12, map_type));

    if (epdpt_large[guest_phys.pdpt_index].large_page)
        return (epdpt_large[guest_phys.pdpt_index].page_frame_number
            * 0x1000 * 0x200 * 0x200) + EPT_LARGE_PDPTE_OFFSET(phys_addr);

    const auto epdpt = reinterpret_cast<ept_pdpte*>(epdpt_large);

    // Map EPD and check for 2MB large page
    const auto epd_large = reinterpret_cast<epde_2mb*>(map_page(
        epdpt[guest_phys.pdpt_index].page_frame_number << 12, map_type));

    if (epd_large[guest_phys.pd_index].large_page)
        return (epd_large[guest_phys.pd_index].page_frame_number
            * 0x1000 * 0x200) + EPT_LARGE_PDE_OFFSET(phys_addr);

    const auto epd = reinterpret_cast<ept_pde*>(epd_large);

    // Map EPT (4KB entry)
    const auto ept = reinterpret_cast<ept_pte*>(map_page(
        epd[guest_phys.pd_index].page_frame_number << 12, map_type));

    return ept[guest_phys.pt_index].page_frame_number << 12;
}
```

**C Conversion**:
```c
u64 translate_guest_physical(u64 guest_physical, map_type_t map_type)
{
    ept_pointer eptp;
    virt_addr_t guest_phys;
    ept_pml4e* epml4;
    ept_pdpte_1gb* epdpt_large;
    ept_pdpte* epdpt;
    epde_2mb* epd_large;
    ept_pde* epd;
    ept_pte* ept;

    guest_phys.value = guest_physical;

    // Read EPTP from VMCS
    __vmx_vmread(VMCS_CTRL_EPT_POINTER, &eptp.flags);

    // Walk EPT PML4
    epml4 = (ept_pml4e*)map_page(eptp.page_frame_number << 12, map_type);

    // Walk EPT PDPT
    epdpt_large = (ept_pdpte_1gb*)map_page(
        epml4[guest_phys.pml4_index].page_frame_number << 12, map_type);

    // Check for 1GB large page
    if (epdpt_large[guest_phys.pdpt_index].large_page) {
        u64 offset = guest_physical & ((0x1000 * 0x200 * 0x200) - 1);
        return (epdpt_large[guest_phys.pdpt_index].page_frame_number * 0x40000000ULL) + offset;
    }

    epdpt = (ept_pdpte*)epdpt_large;

    // Walk EPT PD
    epd_large = (epde_2mb*)map_page(
        epdpt[guest_phys.pdpt_index].page_frame_number << 12, map_type);

    // Check for 2MB large page
    if (epd_large[guest_phys.pd_index].large_page) {
        u64 offset = guest_physical & ((0x1000 * 0x200) - 1);
        return (epd_large[guest_phys.pd_index].page_frame_number * 0x200000ULL) + offset;
    }

    epd = (ept_pde*)epd_large;

    // Walk EPT PT
    ept = (ept_pte*)map_page(
        epd[guest_phys.pd_index].page_frame_number << 12, map_type);

    return (ept[guest_phys.pt_index].page_frame_number << 12) + guest_phys.offset_4kb;
}
```

#### AMD NPT Translation - Guest Physical to System Physical

**C++ Implementation**:
```cpp
auto translate_guest_physical(guest_phys_t phys_addr, map_type_t map_type) -> u64
{
    phys_addr_t guest_phys{ phys_addr };
    const auto vmcb = amd::get_vmcb();

    // Walk NPT to translate guest physical to system physical
    const auto npt_pml4 =
        reinterpret_cast<pnpt_pml4e>(
            map_page(vmcb->NestedPageTableCr3(), map_type));

    if (!npt_pml4[guest_phys.pml4_index].present)
        return 0;

    const auto npt_pdpt =
        reinterpret_cast<pnpt_pdpte>(
            map_page(npt_pml4[guest_phys.pml4_index].pfn << 12, map_type));

    if (!npt_pdpt[guest_phys.pdpt_index].present)
        return 0;

    const auto npt_pd =
        reinterpret_cast<pnpt_pde>(
            map_page(npt_pdpt[guest_phys.pdpt_index].pfn << 12, map_type));

    if (!npt_pd[guest_phys.pd_index].present)
        return 0;

    // Handle 2MB large page
    if (reinterpret_cast<pnpt_pde_2mb>(npt_pd)[guest_phys.pd_index].large_page)
        return (reinterpret_cast<pnpt_pde_2mb>(npt_pd)
            [guest_phys.pd_index].pfn << 21) + guest_phys.offset_2mb;

    const auto npt_pt =
        reinterpret_cast<pnpt_pte>(
            map_page(npt_pd[guest_phys.pd_index].pfn << 12, map_type));

    if (!npt_pt[guest_phys.pt_index].present)
        return 0;

    return (npt_pt[guest_phys.pt_index].pfn << 12) + guest_phys.offset_4kb;
}
```

**C Conversion**:
```c
u64 translate_guest_physical_amd(u64 guest_physical, map_type_t map_type)
{
    virt_addr_t guest_phys;
    vmcb_t* vmcb;
    npt_pml4e* npt_pml4;
    npt_pdpte* npt_pdpt;
    npt_pde* npt_pd;
    npt_pde_2mb* npt_pd_2mb;
    npt_pte* npt_pt;

    guest_phys.value = guest_physical;
    vmcb = get_vmcb();

    // Walk NPT PML4
    npt_pml4 = (npt_pml4e*)map_page(vmcb->control_area.nested_page_table_cr3, map_type);

    if (!npt_pml4[guest_phys.pml4_index].present)
        return 0;

    // Walk NPT PDPT
    npt_pdpt = (npt_pdpte*)map_page(npt_pml4[guest_phys.pml4_index].pfn << 12, map_type);

    if (!npt_pdpt[guest_phys.pdpt_index].present)
        return 0;

    // Walk NPT PD
    npt_pd = (npt_pde*)map_page(npt_pdpt[guest_phys.pdpt_index].pfn << 12, map_type);

    if (!npt_pd[guest_phys.pd_index].present)
        return 0;

    // Check for 2MB large page
    npt_pd_2mb = (npt_pde_2mb*)npt_pd;
    if (npt_pd_2mb[guest_phys.pd_index].large_page) {
        u64 offset = guest_physical & ((0x1000 * 0x200) - 1);
        return (npt_pd_2mb[guest_phys.pd_index].pfn << 21) + offset;
    }

    // Walk NPT PT
    npt_pt = (npt_pte*)map_page(npt_pd[guest_phys.pd_index].pfn << 12, map_type);

    if (!npt_pt[guest_phys.pt_index].present)
        return 0;

    return (npt_pt[guest_phys.pt_index].pfn << 12) + guest_phys.offset_4kb;
}
```

#### Guest Virtual to Guest Physical Translation (Shared)

**C++ Implementation**:
```cpp
auto translate_guest_virtual(guest_phys_t dirbase, guest_virt_t guest_virt, map_type_t map_type) -> u64
{
    virt_addr_t virt_addr{ guest_virt };

    const auto pml4 = reinterpret_cast<pml4e*>(map_guest_phys(dirbase, map_type));
    if (!pml4[virt_addr.pml4_index].present)
        return 0;

    const auto pdpt = reinterpret_cast<pdpte*>(map_guest_phys(
        pml4[virt_addr.pml4_index].pfn << 12, map_type));
    if (!pdpt[virt_addr.pdpt_index].present)
        return 0;

    // Handle 1GB pages
    if (pdpt[virt_addr.pdpt_index].large_page)
        return (pdpt[virt_addr.pdpt_index].pfn << 30) + virt_addr.offset_1gb;

    const auto pd = reinterpret_cast<pde*>(map_guest_phys(
        pdpt[virt_addr.pdpt_index].pfn << 12, map_type));
    if (!pd[virt_addr.pd_index].present)
        return 0;

    // Handle 2MB pages
    if (pd[virt_addr.pd_index].large_page)
        return (pd[virt_addr.pd_index].pfn << 21) + virt_addr.offset_2mb;

    const auto pt = reinterpret_cast<pte*>(map_guest_phys(
        pd[virt_addr.pd_index].pfn << 12, map_type));
    if (!pt[virt_addr.pt_index].present)
        return 0;

    return (pt[virt_addr.pt_index].pfn << 12) + virt_addr.offset_4kb;
}
```

**C Conversion**:
```c
u64 translate_guest_virtual(u64 dirbase, u64 guest_virtual, map_type_t map_type)
{
    virt_addr_t virt_addr;
    pml4e* pml4;
    pdpte* pdpt;
    pde* pd;
    pte* pt;

    virt_addr.value = guest_virtual;

    // Walk guest PML4
    pml4 = (pml4e*)map_guest_phys(dirbase, map_type);
    if (!pml4[virt_addr.pml4_index].present)
        return 0;

    // Walk guest PDPT
    pdpt = (pdpte*)map_guest_phys(pml4[virt_addr.pml4_index].pfn << 12, map_type);
    if (!pdpt[virt_addr.pdpt_index].present)
        return 0;

    // Check for 1GB large page
    if (pdpt[virt_addr.pdpt_index].large_page)
        return (pdpt[virt_addr.pdpt_index].pfn << 30) + virt_addr.offset_1gb;

    // Walk guest PD
    pd = (pde*)map_guest_phys(pdpt[virt_addr.pdpt_index].pfn << 12, map_type);
    if (!pd[virt_addr.pd_index].present)
        return 0;

    // Check for 2MB large page
    if (pd[virt_addr.pd_index].large_page)
        return (pd[virt_addr.pd_index].pfn << 21) + virt_addr.offset_2mb;

    // Walk guest PT
    pt = (pte*)map_guest_phys(pd[virt_addr.pd_index].pfn << 12, map_type);
    if (!pt[virt_addr.pt_index].present)
        return 0;

    return (pt[virt_addr.pt_index].pfn << 12) + virt_addr.offset_4kb;
}
```

---

## Critical Constants

### Page Sizes
```c
#define PAGE_4KB            0x1000          // 4096 bytes
#define PAGE_2MB            0x200000        // 2097152 bytes
#define PAGE_1GB            0x40000000      // 1073741824 bytes

#define SIZE_2_MB           0x200000
#define SIZE_1_GB           0x40000000

#define PAGE_SIZE           PAGE_4KB
```

### Page Alignment
```c
#define PAGE_ALIGN(addr)    ((void*)((u64)(addr) & ~(PAGE_SIZE - 1)))
#define PAGE_OFFSET(addr)   ((u64)(addr) & (PAGE_SIZE - 1))

#define IS_ALIGNED_2MB(addr) (((u64)(addr) & (PAGE_2MB - 1)) == 0)
#define IS_ALIGNED_1GB(addr) (((u64)(addr) & (PAGE_1GB - 1)) == 0)
```

### PML4 Configuration
```c
#define SELF_REF_PML4_IDX   510     // Self-referencing PML4 entry for host page table access
#define MAPPING_PML4_IDX    100     // PML4 entry for guest physical mapping

#define MAPPING_ADDRESS_BASE 0x0000327FFFE00000ULL
#define SELF_REF_PML4        0xFFFFFF7FBFDFE000ULL
```

### Virtual Address Parsing Macros
```c
#define PML4_INDEX(addr)    (((u64)(addr) >> 39) & 0x1FF)
#define PDPT_INDEX(addr)    (((u64)(addr) >> 30) & 0x1FF)
#define PD_INDEX(addr)      (((u64)(addr) >> 21) & 0x1FF)
#define PT_INDEX(addr)      (((u64)(addr) >> 12) & 0x1FF)

#define OFFSET_4KB(addr)    ((u64)(addr) & 0xFFF)
#define OFFSET_2MB(addr)    ((u64)(addr) & 0x1FFFFF)
#define OFFSET_1GB(addr)    ((u64)(addr) & 0x3FFFFFFF)
```

### EPT Large Page Offset Extraction
```c
#define EPT_LARGE_PDPTE_OFFSET(addr) (((u64)(addr)) & ((0x1000 * 0x200 * 0x200) - 1))  // 1GB offset mask
#define EPT_LARGE_PDE_OFFSET(addr)   (((u64)(addr)) & ((0x1000 * 0x200) - 1))          // 2MB offset mask
```

### Memory Types (MTRR)
```c
#define MEMORY_TYPE_UNCACHEABLE     0   // UC - Uncacheable
#define MEMORY_TYPE_WRITE_COMBINING 1   // WC - Write Combining
#define MEMORY_TYPE_WRITE_THROUGH   4   // WT - Write Through
#define MEMORY_TYPE_WRITE_PROTECTED 5   // WP - Write Protected
#define MEMORY_TYPE_WRITE_BACK      6   // WB - Write Back
```

### Table Sizes
```c
#define VMM_EPT_PML4E_COUNT     512
#define VMM_EPT_PML3E_COUNT     512
#define VMM_EPT_PML2E_COUNT     512
#define VMM_EPT_PML1E_COUNT     512

#define PT_ENTRIES              512
```

### VMCS Field Encodings (Intel)
```c
#define VMCS_CTRL_EPT_POINTER   0x0000201A
#define VMCS_GUEST_CR3          0x00006802
#define EPT_POINTER_LOW         VMCS_CTRL_EPT_POINTER
```

### MSR Addresses
```c
#define MSR_IA32_MTRR_CAPABILITIES  0x000000FE
#define MSR_IA32_MTRR_DEF_TYPE      0x000002FF
#define MSR_IA32_MTRR_PHYSBASE0     0x00000200
#define MSR_IA32_MTRR_PHYSMASK0     0x00000201
#define MSR_IA32_VMX_EPT_VPID_CAP   0x0000048C
```

---

## Intel vs AMD Differences

| Feature | Intel EPT | AMD NPT | Notes |
|---------|-----------|---------|-------|
| **Structure Name** | Extended Page Tables | Nested Page Tables | Marketing names |
| **Root Pointer** | EPTP (in VMCS) | nCR3 (in VMCB) | Both 64-bit |
| **Entry Format** | Custom EPT format | Standard x86-64 PTE format | AMD reuses existing structures |
| **Execute-Only** | Supported (if CPUID reports) | **NOT SUPPORTED** | Intel: R=0,W=0,X=1 valid; AMD: requires 3-state machine |
| **Memory Type** | Per-page `memory_type` field | PAT + cache control bits | Intel more flexible |
| **Accessed/Dirty** | Optional (EPTP.EnableAccessAndDirtyFlags) | Always present | AMD follows standard paging |
| **TLB Invalidation** | INVEPT instruction | VMCB.TlbControl or INVLPGA | Different mechanisms |
| **1GB Page Support** | Yes (PDPTE.PS=1) | Yes (PDPTE.PS=1) | Both support |
| **2MB Page Support** | Yes (PDE.PS=1) | Yes (PDE.PS=1) | Both support |
| **User Mode Execute** | `user_mode_execute` bit | Standard `user` bit | Different semantics |
| **Cache Control** | MTRR + `memory_type` field | PAT + PWT/PCD bits | Intel overrides PAT |
| **Suppress #VE** | `suppress_ve` bit (optional) | N/A | Intel-specific feature |

### Execute-Only Implementation Difference

**Intel**:
```c
// Execute-only page (if CPU supports it)
ept_pte entry;
entry.read_access = 0;
entry.write_access = 0;
entry.execute_access = 1;  // Valid configuration
```

**AMD**:
```c
// AMD does NOT support execute-only
// Must use shadow page tables:
// - Primary NPT: page execute-disabled (triggers #VMEXIT on execute)
// - Shadow NPT: page execute-enabled, read/write-disabled
// Toggle between tables to emulate execute-only behavior

npt_pte primary_entry;
primary_entry.present = 1;
primary_entry.writeable = 1;
primary_entry.nx = 1;  // No execute

npt_pte shadow_entry;
shadow_entry.present = 1;
shadow_entry.writeable = 0;  // No write
shadow_entry.nx = 0;         // Execute allowed
```

### TLB Invalidation Mechanisms

**Intel INVEPT**:
```c
typedef enum _invept_type {
    invept_single_context = 1,  // Invalidate mappings for one EPTP
    invept_all_context = 2      // Invalidate all EPT mappings
} invept_type;

void invalidate_ept_intel(invept_type type, u64 eptp)
{
    invept_descriptor desc;
    desc.eptp = eptp;
    desc.reserved = 0;

    __asm__ __volatile__(
        "invept %0, %1"
        :
        : "r"((u64)type), "m"(desc)
        : "cc", "memory"
    );
}
```

**AMD TLB Control**:
```c
typedef enum _tlb_control {
    TLB_CONTROL_DO_NOTHING = 0,
    TLB_CONTROL_FLUSH_ENTIRE_ASID = 1,
    TLB_CONTROL_FLUSH_THIS_ASID = 3,
    TLB_CONTROL_FLUSH_ALL_ASID_NON_GLOBALS = 7
} tlb_control;

void invalidate_npt_amd(svm_state* svm_state)
{
    // Set TlbControl field in VMCB - flushed on next VMRUN
    svm_state->guest_vmcb->control_area.tlb_control = TLB_CONTROL_FLUSH_ENTIRE_ASID;
}
```

**AMD INVLPGA** (alternative):
```c
// Invalidate specific virtual address in specific ASID
void invlpga_addr(u64 virtual_address, u32 asid)
{
    __asm__ __volatile__(
        "invlpga"
        :
        : "a"(virtual_address), "c"(asid)
        : "memory"
    );
}
```

---

## C Conversion Notes

### Replace C++ Language Features

#### 1. Namespaces → Prefix Naming Convention

**C++**:
```cpp
namespace core {
namespace mm {
    auto init() -> VMX_ROOT_ERROR;
    auto map_page(host_phys_t phys_addr, map_type_t map_type) -> u64;
}
}
```

**C**:
```c
// Use module_function naming convention
vmx_root_error core_mm_init(void);
u64 core_mm_map_page(u64 phys_addr, map_type_t map_type);
```

#### 2. Function Overloading → Unique Function Names

**C++**:
```cpp
auto map_guest_phys(guest_phys_t phys_addr) -> u64;
auto map_guest_phys(guest_phys_t phys_addr, map_type_t map_type) -> u64;
```

**C**:
```c
u64 map_guest_phys(u64 phys_addr);
u64 map_guest_phys_ex(u64 phys_addr, map_type_t map_type);
```

#### 3. Templates → Manual Type Implementation

**C++ Template**:
```cpp
template <Mode mode>
union LinearAddress;

template <>
union LinearAddress<Mode::longMode4Level> {
    unsigned long long raw;
    // ...
};
```

**C**:
```c
// Manual implementation for each mode
typedef union _linear_address_long_mode_4level {
    u64 raw;
    // ...
} linear_address_long_mode_4level;

typedef union _linear_address_legacy_pae {
    u32 raw;
    // ...
} linear_address_legacy_pae;
```

#### 4. auto → Explicit Types

**C++**:
```cpp
const auto epml4 = reinterpret_cast<ept_pml4e*>(map_page(eptp.page_frame_number << 12));
```

**C**:
```c
ept_pml4e* epml4 = (ept_pml4e*)map_page(eptp.page_frame_number << 12);
```

#### 5. Constructors/Destructors → Init/Cleanup Functions

**C++**:
```cpp
class EPT {
public:
    EPT() { /* constructor */ }
    ~EPT() { /* destructor */ }
};
```

**C**:
```c
typedef struct _ept_state {
    // Members
} ept_state;

void ept_state_init(ept_state* state);
void ept_state_cleanup(ept_state* state);
```

### Replace STL Containers

#### 1. std::vector → Manual Dynamic Array

**C++**:
```cpp
std::vector<HOOK_DATA> hooks;
hooks.push_back(hook_data);
```

**C**:
```c
typedef struct _hook_array {
    HOOK_DATA* data;
    size_t count;
    size_t capacity;
} hook_array;

void hook_array_init(hook_array* arr) {
    arr->data = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

void hook_array_push(hook_array* arr, HOOK_DATA* hook) {
    if (arr->count >= arr->capacity) {
        size_t new_capacity = arr->capacity == 0 ? 8 : arr->capacity * 2;
        HOOK_DATA* new_data = realloc(arr->data, new_capacity * sizeof(HOOK_DATA));
        arr->data = new_data;
        arr->capacity = new_capacity;
    }
    arr->data[arr->count++] = *hook;
}

void hook_array_cleanup(hook_array* arr) {
    free(arr->data);
    arr->data = NULL;
    arr->count = 0;
    arr->capacity = 0;
}
```

### Replace Intrinsics

#### 1. __vmx_vmread / __vmx_vmwrite

**C++ (Windows)**:
```cpp
__vmx_vmread(VMCS_CTRL_EPT_POINTER, &eptp.flags);
__vmx_vmwrite(EPT_POINTER_LOW, eptp.flags);
```

**C + Inline Assembly (GCC)**:
```c
static inline u8 vmx_vmread(u64 field, u64* value)
{
    u8 error;
    __asm__ __volatile__(
        "vmread %2, %1\n\t"
        "setna %0"
        : "=q"(error), "=r"(*value)
        : "r"(field)
        : "cc"
    );
    return error;
}

static inline u8 vmx_vmwrite(u64 field, u64 value)
{
    u8 error;
    __asm__ __volatile__(
        "vmwrite %1, %2\n\t"
        "setna %0"
        : "=q"(error)
        : "r"(value), "r"(field)
        : "cc"
    );
    return error;
}
```

#### 2. __readmsr / __writemsr

**C++ (Windows)**:
```cpp
u64 msr_value = __readmsr(MSR_IA32_MTRR_CAPABILITIES);
__writemsr(MSR_IA32_MTRR_DEF_TYPE, new_value);
```

**C + Inline Assembly (GCC)**:
```c
static inline u64 rdmsr(u32 msr)
{
    u32 low, high;
    __asm__ __volatile__(
        "rdmsr"
        : "=a"(low), "=d"(high)
        : "c"(msr)
    );
    return ((u64)high << 32) | low;
}

static inline void wrmsr(u32 msr, u64 value)
{
    u32 low = (u32)value;
    u32 high = (u32)(value >> 32);
    __asm__ __volatile__(
        "wrmsr"
        :
        : "a"(low), "d"(high), "c"(msr)
    );
}
```

#### 3. __readcr3 / __writecr3

**C++ (Windows)**:
```cpp
u64 cr3 = __readcr3();
__writecr3(new_cr3);
```

**C + Inline Assembly (GCC)**:
```c
static inline u64 read_cr3(void)
{
    u64 cr3;
    __asm__ __volatile__(
        "mov %%cr3, %0"
        : "=r"(cr3)
        :
        : "memory"
    );
    return cr3;
}

static inline void write_cr3(u64 cr3)
{
    __asm__ __volatile__(
        "mov %0, %%cr3"
        :
        : "r"(cr3)
        : "memory"
    );
}
```

#### 4. __cpuid

**C++ (Windows)**:
```cpp
cpuid_eax_01 cpuid_value;
__cpuid((int*)&cpuid_value, 1);
```

**C + Inline Assembly (GCC)**:
```c
static inline void cpuid(u32 leaf, u32* eax, u32* ebx, u32* ecx, u32* edx)
{
    __asm__ __volatile__(
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf)
    );
}
```

#### 5. __invlpg

**C++ (Windows)**:
```cpp
__invlpg(reinterpret_cast<void*>(virtual_address));
```

**C + Inline Assembly (GCC)**:
```c
static inline void invlpg(void* virtual_address)
{
    __asm__ __volatile__(
        "invlpg (%0)"
        :
        : "r"(virtual_address)
        : "memory"
    );
}
```

#### 6. __stosq (Fill 64-bit Values)

**C++ (Windows)**:
```cpp
__stosq((SIZE_T*)&PageTable->PML2[0], PML2EntryTemplate.Flags, VMM_EPT_PML3E_COUNT * VMM_EPT_PML2E_COUNT);
```

**C**:
```c
void stosq(u64* dest, u64 value, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        dest[i] = value;
    }
}
```

**C + Inline Assembly (Optimized)**:
```c
static inline void stosq(u64* dest, u64 value, size_t count)
{
    __asm__ __volatile__(
        "rep stosq"
        : "=D"(dest), "=c"(count)
        : "a"(value), "0"(dest), "1"(count)
        : "memory"
    );
}
```

#### 7. _BitScanForward64

**C++ (Windows)**:
```cpp
ULONG NumberOfBitsInMask;
_BitScanForward64(&NumberOfBitsInMask, CurrentPhysMask.PageFrameNumber * PAGE_SIZE);
```

**C (GCC)**:
```c
// Count trailing zeros (equivalent to bit scan forward)
u32 number_of_bits_in_mask = __builtin_ctzll(current_phys_mask.page_frame_number * PAGE_SIZE);
```

**C (Manual Implementation)**:
```c
static inline u32 bsf64(u64 value)
{
    if (value == 0)
        return 64;

    u32 pos = 0;
    while ((value & 1) == 0) {
        value >>= 1;
        pos++;
    }
    return pos;
}
```

### Replace Memory Management

#### 1. RtlZeroMemory / RtlCopyMemory

**C++ (Windows)**:
```cpp
RtlZeroMemory(PageTable, sizeof(VMM_EPT_PAGE_TABLE));
RtlCopyMemory(TargetEntry, &NewPointer, sizeof(NewPointer));
```

**C**:
```c
#include <string.h>

memset(page_table, 0, sizeof(vmm_ept_page_table));
memcpy(target_entry, &new_pointer, sizeof(new_pointer));
```

#### 2. Custom Allocators → Standard malloc/free

**C++ (Custom)**:
```cpp
PageTable = (PVMM_EPT_PAGE_TABLE)cpp::kMalloc(sizeof(VMM_EPT_PAGE_TABLE));
```

**C**:
```c
#include <stdlib.h>

vmm_ept_page_table* page_table = malloc(sizeof(vmm_ept_page_table));
// ...
free(page_table);
```

#### 3. Aligned Allocation

**C (POSIX)**:
```c
#include <stdlib.h>

void* aligned_alloc_4kb(size_t size)
{
    void* ptr;
    if (posix_memalign(&ptr, 4096, size) != 0)
        return NULL;
    return ptr;
}
```

---

## Testing Checklist

### EPT/NPT Table Construction

- [ ] Verify PML4[0] points to valid PDPT
- [ ] Verify all 512 PDPT entries point to valid PD tables
- [ ] Verify all 512x512 PD entries contain 2MB large pages
- [ ] Verify PFN calculation: `physical_address = pfn * page_size`
- [ ] Verify 1GB page alignment (30-bit offset)
- [ ] Verify 2MB page alignment (21-bit offset)
- [ ] Verify 4KB page alignment (12-bit offset)

### MTRR Integration

- [ ] Verify MTRR capabilities register read
- [ ] Verify variable MTRR range enumeration
- [ ] Verify PHYSBASE and PHYSMASK decoding
- [ ] Verify memory type assignment (UC, WC, WT, WP, WB)
- [ ] Verify UC precedence over other memory types
- [ ] Verify first 1MB marked as UC (MMIO region)

### Page Splitting

- [ ] Verify 2MB page split into 512x4KB entries
- [ ] Verify identity mapping preserved across split
- [ ] Verify permissions preserved across split
- [ ] Verify memory type preserved across split
- [ ] Verify PDE.large_page cleared after split
- [ ] Verify PDE.pfn points to new PT table

### Address Translation

#### Guest Virtual → Guest Physical
- [ ] Test 4KB page translation
- [ ] Test 2MB large page translation
- [ ] Test 1GB huge page translation
- [ ] Test cross-page-boundary addresses
- [ ] Test non-present page handling

#### Guest Physical → Host Physical (EPT)
- [ ] Test EPT walk with 4KB pages
- [ ] Test EPT walk with 2MB large pages
- [ ] Test EPT walk with 1GB huge pages
- [ ] Test EPT pointer extraction from VMCS
- [ ] Test cross-EPT-table-boundary addresses

#### Guest Physical → System Physical (NPT)
- [ ] Test NPT walk with 4KB pages
- [ ] Test NPT walk with 2MB large pages
- [ ] Test nested CR3 extraction from VMCB
- [ ] Test NPT non-present page handling

### TLB Invalidation

#### Intel INVEPT
- [ ] Test INVEPT single-context invalidation
- [ ] Test INVEPT all-context invalidation
- [ ] Test INVEPT after permission changes
- [ ] Test INVEPT after PFN changes
- [ ] Test INVEPT descriptor format

#### AMD TLB Control
- [ ] Test VMCB.TlbControl field write
- [ ] Test TLB flush on next VMRUN
- [ ] Test FLUSH_ENTIRE_ASID mode
- [ ] Test FLUSH_THIS_ASID mode
- [ ] Test INVLPGA instruction (if available)

### Permission Enforcement

#### Intel EPT
- [ ] Test read-only enforcement (R=1, W=0)
- [ ] Test write-only (invalid combination)
- [ ] Test execute-only (if supported: R=0, W=0, X=1)
- [ ] Test no-access (R=0, W=0, X=0)
- [ ] Test read-write-execute (R=1, W=1, X=1)

#### AMD NPT
- [ ] Test present=1, writeable=0 (read-only)
- [ ] Test present=1, nx=1 (no-execute)
- [ ] Test present=0 (page fault on access)
- [ ] Test execute-only emulation (shadow table toggle)

### Memory Type Handling

- [ ] Test UC (Uncacheable) pages
- [ ] Test WB (Write-Back) pages
- [ ] Test WC (Write-Combining) for framebuffers
- [ ] Test memory type precedence (UC > WB)
- [ ] Test MTRR override of PAT

### Edge Cases

- [ ] Test NULL pointer translation
- [ ] Test canonical address violations
- [ ] Test reserved bit violations
- [ ] Test maximum physical address (MAXPHYADDR)
- [ ] Test self-referencing PML4 entry
- [ ] Test recursive page table modifications

### Performance

- [ ] Measure EPT/NPT walk latency
- [ ] Measure INVEPT/TLB flush overhead
- [ ] Measure page split overhead
- [ ] Measure memory mapping throughput
- [ ] Test TLB hit ratio before/after invalidation

### Multicore Safety

- [ ] Test per-core EPT/NPT tables
- [ ] Test concurrent page splits on different cores
- [ ] Test race conditions during table modifications
- [ ] Test TLB coherency across cores
- [ ] Test INVEPT broadcast to all cores

### Integration with Hypervisor

- [ ] Test EPT violation handling
- [ ] Test NPT fault handling
- [ ] Test EPT misconfiguration detection
- [ ] Test primary/shadow EPT toggle
- [ ] Test hook page substitution

---

## Critical Implementation Notes

### 1. VMXRoot Context Limitations

**Problem**: VMXRoot context has NO exception handling. Page faults = triple fault = system crash.

**Solution**: Always validate addresses before dereferencing:

```c
u64 translate_guest_physical(u64 guest_physical, map_type_t map_type)
{
    ept_pml4e* epml4 = (ept_pml4e*)map_page(eptp.page_frame_number << 12, map_type);

    // CRITICAL: Check validity before dereferencing
    if (!epml4)
        return 0;

    if (!epml4[guest_phys.pml4_index].read_access)
        return 0;  // Not present - would cause EPT violation

    // Safe to continue...
}
```

### 2. Physical Address Maximum (MAXPHYADDR)

**Problem**: Not all 52 bits of physical address are usable. MAXPHYADDR varies by CPU (typically 36-48 bits).

**Solution**: Query CPUID and mask PFN fields:

```c
u32 get_maxphyaddr(void)
{
    u32 eax, ebx, ecx, edx;
    cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
    return (u32)(eax & 0xFF);  // Bits 7:0 = physical address width
}

u64 mask_pfn(u64 pfn, u32 maxphyaddr)
{
    u64 mask = (1ULL << (maxphyaddr - 12)) - 1;
    return pfn & mask;
}
```

### 3. Self-Referencing PML4 Entry

**Purpose**: Allow hypervisor to modify its own page tables without separate mappings.

**Implementation**:
```c
// At PML4 index 510, point to PML4 itself
pml4e self_ref;
self_ref.present = 1;
self_ref.writeable = 1;
self_ref.pfn = virt_to_phys(pml4_base) >> 12;

pml4[SELF_REF_PML4_IDX] = self_ref;

// Now can access any page table entry via fixed virtual addresses:
// PML4[i] = *(ppml4e)(SELF_REF_PML4 + i * 8)
// PDPT[i][j] = *(ppdpte)(SELF_REF_PML4 + (i << 12) + j * 8)
// etc.
```

### 4. EPT Execute-Only Compatibility

**Problem**: Not all Intel CPUs support execute-only EPT pages.

**Solution**: Check capability and provide fallback:

```c
bool is_exec_only_supported(void)
{
    u64 ept_vpid_cap = rdmsr(MSR_IA32_VMX_EPT_VPID_CAP);
    return (ept_vpid_cap & (1ULL << 0)) != 0;  // Bit 0 = execute-only supported
}

void set_execute_only(ept_pte* entry, bool exec_only_supported)
{
    if (exec_only_supported) {
        entry->read_access = 0;
        entry->write_access = 0;
        entry->execute_access = 1;
    } else {
        // Fallback: use shadow page tables
        // Primary: R=1, W=0, X=0 (causes EPT violation on execute)
        // Shadow:  R=0, W=0, X=1 (execute allowed)
        entry->read_access = 1;
        entry->write_access = 0;
        entry->execute_access = 0;
    }
}
```

### 5. AMD NPT Execute-Only Emulation

**Problem**: AMD NPT does NOT support execute-only pages natively.

**Solution**: Implement 3-state machine with shadow tables:

```c
typedef enum _npt_state {
    NPT_STATE_PRIMARY,   // Normal execution, hooks active
    NPT_STATE_SHADOW     // Single-step mode, original code visible
} npt_state;

// Primary NPT: Execute triggers #VMEXIT
npt_pte primary_entry;
primary_entry.present = 1;
primary_entry.writeable = 1;
primary_entry.nx = 1;  // No execute
primary_entry.pfn = hook_page_pfn;

// Shadow NPT: Execute allowed, R/W triggers #VMEXIT
npt_pte shadow_entry;
shadow_entry.present = 1;
shadow_entry.writeable = 0;  // No write
shadow_entry.nx = 0;         // Execute OK
shadow_entry.pfn = original_page_pfn;

// On #VMEXIT due to execute attempt:
void handle_npt_violation_execute(u64 guest_rip)
{
    // Switch to shadow NPT
    vmcb->control_area.nested_page_table_cr3 = shadow_npt_cr3;

    // Enable single-step (RFLAGS.TF)
    vmcb->state_save_area.rflags |= (1ULL << 8);

    // On next #VMEXIT (single-step):
    // - Restore primary NPT
    // - Clear RFLAGS.TF
}
```

### 6. Memory Ordering and Barriers

**Problem**: CPU may reorder EPT/NPT table modifications, causing stale TLB entries.

**Solution**: Use memory barriers after table modifications:

```c
void update_ept_entry(ept_pte* entry, u64 new_flags)
{
    // Update entry
    entry->flags = new_flags;

    // Memory barrier to ensure update visible to all cores
    __asm__ __volatile__("mfence" ::: "memory");

    // Invalidate TLB
    invept_descriptor desc;
    desc.eptp = current_eptp;
    desc.reserved = 0;
    invalidate_ept_intel(invept_single_context, desc.eptp);
}
```

### 7. Large Page Splitting Atomicity

**Problem**: During page split, there's a window where entry is invalid, causing EPT violations.

**Solution**: Prepare new table BEFORE modifying PDE:

```c
bool split_large_page_atomic(ept_pde* pde, epde_2mb* large_page)
{
    // 1. Allocate and populate new PT (512 entries)
    ept_pte* new_pt = allocate_pt();
    for (int i = 0; i < 512; i++) {
        new_pt[i].flags = 0;
        new_pt[i].read_access = large_page->read_access;
        new_pt[i].write_access = large_page->write_access;
        new_pt[i].execute_access = large_page->execute_access;
        new_pt[i].memory_type = large_page->memory_type;
        new_pt[i].page_frame_number = (large_page->page_frame_number << 9) + i;
    }

    // 2. Memory barrier
    __asm__ __volatile__("mfence" ::: "memory");

    // 3. ATOMICALLY update PDE (single 8-byte write is atomic)
    ept_pde new_pde;
    new_pde.flags = 0;
    new_pde.read_access = 1;
    new_pde.write_access = 1;
    new_pde.execute_access = 1;
    new_pde.page_frame_number = virt_to_phys(new_pt) >> 12;

    pde->flags = new_pde.flags;  // Atomic 8-byte write

    // 4. Invalidate TLB
    invept_all();

    return true;
}
```

---

## Appendix: Complete Structure Definitions

### Virtual Address Union (All Modes)

```c
typedef union _virt_addr_t {
    u64 value;

    // 4KB page decomposition
    struct {
        u64 offset_4kb : 12;
        u64 pt_index : 9;
        u64 pd_index : 9;
        u64 pdpt_index : 9;
        u64 pml4_index : 9;
        u64 reserved : 16;
    };

    // 2MB page decomposition
    struct {
        u64 offset_2mb : 21;
        u64 pd_index_2mb : 9;
        u64 pdpt_index_2mb : 9;
        u64 pml4_index_2mb : 9;
        u64 reserved_2mb : 16;
    };

    // 1GB page decomposition
    struct {
        u64 offset_1gb : 30;
        u64 pdpt_index_1gb : 9;
        u64 pml4_index_1gb : 9;
        u64 reserved_1gb : 16;
    };
} virt_addr_t;
```

### EPT Page Table Structure

```c
typedef struct _vmm_ept_page_table {
    ept_pml4e pml4[512];               // 4KB - Level 4
    ept_pdpte pml3[512];               // 4KB - Level 3 (each entry covers 1GB)
    ept_pde   pml2[512][512];          // 2MB - Level 2 (each entry is 2MB large page)
    // No PML1 allocated by default (2MB large pages used)
} vmm_ept_page_table;

typedef struct _vmm_ept_dynamic_split {
    ept_pte pml1[512];                 // 4KB - Level 1 (for split 2MB pages)
} vmm_ept_dynamic_split;
```

### MTRR Range Descriptor

```c
typedef struct _mtrr_range_descriptor {
    u64 physical_base_address;
    u64 physical_end_address;
    u8  memory_type;
} mtrr_range_descriptor;

typedef struct _ept_state {
    vmm_ept_page_table* ept_page_table[512];  // Per-PML4-index tables
    ept_pointer         ept_pointer;
    mtrr_range_descriptor memory_ranges[256];
    u32                 num_enabled_memory_ranges;
    // AMD-specific
    union {
        u64 flags;
        struct {
            u64 reserved1 : 12;
            u64 address_of_page_directory : 40;
            u64 reserved2 : 12;
        };
    } ncr3;
} ept_state;
```

---

**END OF DOCUMENT**
