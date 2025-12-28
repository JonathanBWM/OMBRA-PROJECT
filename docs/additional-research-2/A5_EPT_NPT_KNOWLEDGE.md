# A5: Extended Page Tables (EPT) and Nested Page Tables (NPT) - Complete Reference

This document serves as the authoritative reference for implementing EPT (Intel) and NPT (AMD) in the OmbraHypervisor project. All information is extracted from analysis of production hypervisors including HyperPlatform, DdiMon, SimpleSvmHook, gbhv, NoirVisor, and hvpp.

---

## Table of Contents

1. [EPT Architecture Overview (Intel)](#1-ept-architecture-overview-intel)
2. [NPT Architecture Overview (AMD)](#2-npt-architecture-overview-amd)
3. [EPT Structure Definitions](#3-ept-structure-definitions)
4. [NPT Structure Definitions](#4-npt-structure-definitions)
5. [EPT Pointer (EPTP) Configuration](#5-ept-pointer-eptp-configuration)
6. [NPT Nested CR3 (nCR3) Configuration](#6-npt-nested-cr3-ncr3-configuration)
7. [Identity Mapping Implementation](#7-identity-mapping-implementation)
8. [MTRR Integration](#8-mtrr-integration)
9. [Large Page Splitting](#9-large-page-splitting)
10. [EPT Violation Handling](#10-ept-violation-handling)
11. [NPT Fault Handling](#11-npt-fault-handling)
12. [Execute-Only Pages and Shadow Hooks](#12-execute-only-pages-and-shadow-hooks)
13. [TLB Invalidation](#13-tlb-invalidation)
14. [Page Table Walking](#14-page-table-walking)
15. [Unified Abstraction Recommendations](#15-unified-abstraction-recommendations)

---

## 1. EPT Architecture Overview (Intel)

Intel Extended Page Tables provide a second layer of address translation: Guest Physical Address (GPA) to Host Physical Address (HPA). This hardware-assisted mechanism eliminates the need for shadow page tables.

### Translation Hierarchy

```
EPT Pointer (EPTP)
    |
    v
PML4 (512 entries, each covers 512GB)
    |
    v
PDPT (512 entries, each covers 1GB)
    |                     \
    v                      v
    [1GB Huge Page]       PD (512 entries, each covers 2MB)
                              |                    \
                              v                     v
                              [2MB Large Page]     PT (512 entries, each covers 4KB)
                                                       |
                                                       v
                                                   [4KB Page]
```

### Address Bit Usage

From `HyperPlatform/HyperPlatform/ept.cpp:26-45`:
```
EPT Page map level 4 selector           9 bits  (bits 47:39)
EPT Page directory pointer selector     9 bits  (bits 38:30)
EPT Page directory selector             9 bits  (bits 29:21)
EPT Page table selector                 9 bits  (bits 20:12)
EPT Byte within page                   12 bits  (bits 11:0)
```

Index extraction constants:
```cpp
static const auto kEptpPxiShift = 39ull;  // PML4 index shift
static const auto kEptpPpiShift = 30ull;  // PDPT index shift
static const auto kEptpPdiShift = 21ull;  // PD index shift
static const auto kEptpPtiShift = 12ull;  // PT index shift
static const auto kEptpPtxMask = 0x1ffull; // 9-bit mask
```

---

## 2. NPT Architecture Overview (AMD)

AMD Nested Page Tables function identically to standard AMD64 paging but for guest-to-host translation. The nCR3 register points to the base of the nested page tables.

### Key Differences from EPT

| Feature | Intel EPT | AMD NPT |
|---------|-----------|---------|
| Access bits | R/W/X separate | Present + Write + NX |
| Execute control | Execute bit (positive) | NX bit (negative) |
| Memory type | In entry | Via PAT/PWT/PCD |
| Root pointer | EPTP in VMCS | nCR3 in VMCB |
| User mode execute | UMX bit | User bit |

### NPT Entry Permission Mapping

From `NoirVisor/src/svm_core/svm_npt.h:117-136`:
```cpp
// AMD uses Present/Write/NX model
pte.present = readable;     // Bit 0: Present = Read allowed
pte.write = writable;       // Bit 1: Write allowed
pte.no_execute = !executable; // Bit 63: NX = NOT executable
```

---

## 3. EPT Structure Definitions

### EPT Pointer (EPTP)

From `NoirVisor/src/vt_core/vt_ept.h:25-37`:
```cpp
typedef union _ia32_ept_pointer
{
    struct
    {
        u64 memory_type:3;      // bits 0-2: Memory type for EPT tables (WB=6)
        u64 walk_length:3;      // bits 3-5: Page walk length - 1 (must be 3)
        u64 dirty_flag:1;       // bit  6: Enable accessed/dirty flags
        u64 enable_sss:1;       // bit  7: Enable supervisor shadow stack
        u64 reserved:4;         // bits 8-11
        u64 pml4e_offset:52;    // bits 12-63: Physical address of PML4
    };
    u64 value;
} ia32_ept_pointer;
```

### EPT PML4 Entry

From `NoirVisor/src/vt_core/vt_ept.h:52-68`:
```cpp
typedef union _ia32_ept_pml4e
{
    struct
    {
        u64 read:1;             // Bit 0: Read access
        u64 write:1;            // Bit 1: Write access
        u64 execute:1;          // Bit 2: Execute access
        u64 reserved0:5;        // Bits 3-7
        u64 accessed:1;         // Bit 8: Accessed flag
        u64 ignored0:1;         // Bit 9
        u64 umx:1;              // Bit 10: User-mode execute
        u64 ignored1:1;         // Bit 11
        u64 pdpte_offset:40;    // Bits 12-51: Physical address of PDPT
        u64 reserved1:12;       // Bits 52-63
    };
    u64 value;
} ia32_ept_pml4e;
```

### EPT PDPT Entry (1GB Page)

From `NoirVisor/src/vt_core/vt_ept.h:70-92`:
```cpp
typedef union _ia32_ept_huge_pdpte
{
    struct
    {
        u64 read:1;             // Bit 0
        u64 write:1;            // Bit 1
        u64 execute:1;          // Bit 2
        u64 memory_type:3;      // Bits 3-5: Memory type
        u64 ignore_pat:1;       // Bit 6: Ignore PAT
        u64 huge_pdpte:1;       // Bit 7: MUST be 1 for 1GB page
        u64 accessed:1;         // Bit 8
        u64 dirty:1;            // Bit 9
        u64 umx:1;              // Bit 10
        u64 var_mtrr_covered:1; // Bit 11: Custom - MTRR tracking
        u64 reserved:18;        // Bits 12-29
        u64 page_offset:22;     // Bits 30-51: 1GB-aligned physical address
        u64 ignored1:8;         // Bits 52-59
        u64 s_shadow_stack:1;   // Bit 60
        u64 ignored2:2;         // Bits 61-62
        u64 suppress_ve:1;      // Bit 63: Suppress #VE
    };
    u64 value;
} ia32_ept_huge_pdpte;
```

### EPT PD Entry (2MB Page)

From `NoirVisor/src/vt_core/vt_ept.h:112-134`:
```cpp
typedef union _ia32_ept_large_pde
{
    struct
    {
        u64 read:1;             // Bit 0
        u64 write:1;            // Bit 1
        u64 execute:1;          // Bit 2
        u64 memory_type:3;      // Bits 3-5
        u64 ignore_pat:1;       // Bit 6
        u64 large_pde:1;        // Bit 7: MUST be 1 for 2MB page
        u64 accessed:1;         // Bit 8
        u64 dirty:1;            // Bit 9
        u64 umx:1;              // Bit 10
        u64 var_mtrr_covered:1; // Bit 11
        u64 reserved:9;         // Bits 12-20
        u64 page_offset:31;     // Bits 21-51: 2MB-aligned physical address
        u64 ignored1:8;         // Bits 52-59
        u64 s_shadow_stack:1;   // Bit 60
        u64 ignored:2;          // Bits 61-62
        u64 suppress_ve:1;      // Bit 63
    };
    u64 value;
} ia32_ept_large_pde;
```

### EPT PT Entry (4KB Page)

From `NoirVisor/src/vt_core/vt_ept.h:154-176`:
```cpp
typedef union _ia32_ept_pte
{
    struct
    {
        u64 read:1;             // Bit 0
        u64 write:1;            // Bit 1
        u64 execute:1;          // Bit 2
        u64 memory_type:3;      // Bits 3-5
        u64 ignore_pat:1;       // Bit 6
        u64 ignored0:1;         // Bit 7
        u64 accessed:1;         // Bit 8
        u64 dirty:1;            // Bit 9
        u64 umx:1;              // Bit 10
        u64 var_mtrr_covered:1; // Bit 11
        u64 page_offset:40;     // Bits 12-51: 4KB-aligned physical address
        u64 ignored2:8;         // Bits 52-59
        u64 s_shadow_stack:1;   // Bit 60
        u64 subpage_write:1;    // Bit 61
        u64 ignored3:1;         // Bit 62
        u64 suppress_ve:1;      // Bit 63
    };
    u64 value;
} ia32_ept_pte;
```

### EPT Violation Exit Qualification

From `NoirVisor/src/vt_core/vt_ept.h:248-276`:
```cpp
typedef union _ia32_ept_violation_qualification
{
    struct
    {
        ulong_ptr read:1;                   // Bit 0: Data read
        ulong_ptr write:1;                  // Bit 1: Data write
        ulong_ptr execute:1;                // Bit 2: Instruction fetch
        ulong_ptr readable:1;               // Bit 3: EPT entry readable
        ulong_ptr writable:1;               // Bit 4: EPT entry writable
        ulong_ptr executable:1;             // Bit 5: EPT entry executable
        ulong_ptr umx_allowed:1;            // Bit 6: User-mode execute allowed
        ulong_ptr gva_valid:1;              // Bit 7: Guest linear address valid
        ulong_ptr translation_violation:1;  // Bit 8: Caused by translation
        ulong_ptr um_address:1;             // Bit 9: User-mode address
        ulong_ptr rw_address:1;             // Bit 10: Read/write address
        ulong_ptr ex_address:1;             // Bit 11: Execute-disabled address
        ulong_ptr iret_nmi_block:1;         // Bit 12: NMI unblocking by IRET
        ulong_ptr shadow_stack:1;           // Bit 13: Shadow stack access
        ulong_ptr sss:1;                    // Bit 14: Supervisor shadow stack
        ulong_ptr undefined:1;              // Bit 15
        ulong_ptr async_instruction:1;      // Bit 16: Async instruction
    };
    ulong_ptr value;
} ia32_ept_violation_qualification;
```

---

## 4. NPT Structure Definitions

### NPT PML4 Entry

From `NoirVisor/src/svm_core/svm_npt.h:17-33`:
```cpp
typedef union _amd64_npt_pml4e
{
    struct
    {
        u64 present:1;      // Bit 0
        u64 write:1;        // Bit 1
        u64 user:1;         // Bit 2
        u64 pwt:1;          // Bit 3: Page Write-Through
        u64 pcd:1;          // Bit 4: Page Cache Disable
        u64 accessed:1;     // Bit 5
        u64 reserved1:6;    // Bits 6-11
        u64 pdpte_base:40;  // Bits 12-51
        u64 reserved2:11;   // Bits 52-62
        u64 no_execute:1;   // Bit 63: NX bit
    };
    u64 value;
} amd64_npt_pml4e;
```

### NPT PDPT Entry (1GB Page)

From `NoirVisor/src/svm_core/svm_npt.h:35-56`:
```cpp
typedef union _amd64_npt_huge_pdpte
{
    struct
    {
        u64 present:1;      // Bit 0
        u64 write:1;        // Bit 1
        u64 user:1;         // Bit 2
        u64 pwt:1;          // Bit 3
        u64 pcd:1;          // Bit 4
        u64 accessed:1;     // Bit 5
        u64 dirty:1;        // Bit 6
        u64 huge_pdpte:1;   // Bit 7: MUST be 1 for 1GB page
        u64 global:1;       // Bit 8
        u64 reserved1:3;    // Bits 9-11
        u64 pat:1;          // Bit 12: Page Attribute Table
        u64 reserved2:17;   // Bits 13-29
        u64 page_base:22;   // Bits 30-51: 1GB-aligned address
        u64 reserved3:11;   // Bits 52-62
        u64 no_execute:1;   // Bit 63
    };
    u64 value;
} amd64_npt_huge_pdpte;
```

### NPT PD Entry (2MB Page)

From `NoirVisor/src/svm_core/svm_npt.h:76-97`:
```cpp
typedef union _amd64_npt_large_pde
{
    struct
    {
        u64 present:1;      // Bit 0
        u64 write:1;        // Bit 1
        u64 user:1;         // Bit 2
        u64 pwt:1;          // Bit 3
        u64 pcd:1;          // Bit 4
        u64 accessed:1;     // Bit 5
        u64 dirty:1;        // Bit 6
        u64 large_pde:1;    // Bit 7: MUST be 1 for 2MB page
        u64 global:1;       // Bit 8
        u64 reserved1:3;    // Bits 9-11
        u64 pat:1;          // Bit 12
        u64 reserved2:8;    // Bits 13-20
        u64 page_base:31;   // Bits 21-51: 2MB-aligned address
        u64 reserved3:11;   // Bits 52-62
        u64 no_execute:1;   // Bit 63
    };
    u64 value;
} amd64_npt_large_pde;
```

### NPT PT Entry (4KB Page)

From `NoirVisor/src/svm_core/svm_npt.h:117-136`:
```cpp
typedef union _amd64_npt_pte
{
    struct
    {
        u64 present:1;      // Bit 0
        u64 write:1;        // Bit 1
        u64 user:1;         // Bit 2
        u64 pwt:1;          // Bit 3
        u64 pcd:1;          // Bit 4
        u64 accessed:1;     // Bit 5
        u64 dirty:1;        // Bit 6
        u64 pat:1;          // Bit 7
        u64 global:1;       // Bit 8
        u64 reserved1:3;    // Bits 9-11
        u64 page_base:40;   // Bits 12-51: 4KB-aligned address
        u64 reserved2:11;   // Bits 52-62
        u64 no_execute:1;   // Bit 63
    };
    u64 value;
} amd64_npt_pte;
```

### NPT Fault Exit Code

From `NoirVisor/src/svm_core/svm_npt.h:225-247`:
```cpp
typedef union _amd64_npt_fault_code
{
    struct
    {
        u64 present:1;      // Bit 0: Page was present
        u64 write:1;        // Bit 1: Write access
        u64 user:1;         // Bit 2: User-mode access
        u64 rsv_b:1;        // Bit 3: Reserved bit set
        u64 execute:1;      // Bit 4: Instruction fetch
        u64 reserved1:1;    // Bit 5
        u64 shadow_stk:1;   // Bit 6: Shadow stack access
        u64 reserved2:24;   // Bits 7-30
        u64 rmp_check:1;    // Bit 31: RMP check failed (SEV)
        u64 npf_addr:1;     // Bit 32: NPF due to address
        u64 npf_table:1;    // Bit 33: NPF due to table walk
        u64 encrypted:1;    // Bit 34: Encrypted page
        u64 size_mismatch:1;// Bit 35: Page size mismatch (SEV)
        u64 vmpl_failed:1;  // Bit 36: VMPL check failed
        u64 sss:1;          // Bit 37: Supervisor shadow stack
        u64 reserved4:26;   // Bits 38-63
    };
    u64 value;
} amd64_npt_fault_code;
```

---

## 5. EPT Pointer (EPTP) Configuration

From `HyperPlatform/HyperPlatform/ept.cpp:420-425`:
```cpp
ept_data->ept_pointer.all = 0;
ept_data->ept_pointer.fields.memory_type =
    static_cast<ULONG64>(EptpGetMemoryType(UtilPaFromVa(ept_pml4)));
ept_data->ept_pointer.fields.page_walk_length = kEptPageWalkLevel - 1; // Must be 3
ept_data->ept_pointer.fields.pml4_address =
    UtilPfnFromPa(UtilPaFromVa(ept_pml4));
```

From `gbhv/gbhv/ept.c:539-557`:
```cpp
EPTP.Flags = 0;
EPTP.MemoryType = MEMORY_TYPE_WRITE_BACK;  // For performance
EPTP.EnableAccessAndDirtyFlags = FALSE;     // Not using A/D flags
EPTP.PageWalkLength = 3;  // 4-level page walk (3 = 4-1)
EPTP.PageFrameNumber = (SIZE_T)OsVirtualToPhysical(&PageTable->PML4) / PAGE_SIZE;
```

### EPTP Memory Type Values
```cpp
#define MEMORY_TYPE_UNCACHEABLE     0
#define MEMORY_TYPE_WRITE_COMBINING 1
#define MEMORY_TYPE_WRITE_THROUGH   4
#define MEMORY_TYPE_WRITE_PROTECTED 5
#define MEMORY_TYPE_WRITE_BACK      6
```

---

## 6. NPT Nested CR3 (nCR3) Configuration

From `NoirVisor/src/svm_core/svm_npt.c:440-496`:
```cpp
noir_npt_manager_p nvc_npt_build_identity_map()
{
    noir_npt_manager_p nptm = noir_alloc_nonpg_memory(sizeof(noir_npt_manager));
    if(nptm)
    {
        nptm->ncr3.virt = noir_alloc_contd_memory(page_size);
        if(nptm->ncr3.virt)
        {
            nptm->pdpt.virt = noir_alloc_2mb_page();
            if(nptm->pdpt.virt)
            {
                nptm->pdpt.phys = noir_get_physical_address(nptm->pdpt.virt);
            }
            nptm->ncr3.phys = noir_get_physical_address(nptm->ncr3.virt);
        }
    }
    // ... build PML4 entries pointing to PDPT pages
    for(u32 i = 0; i < 512; i++)
    {
        nptm->ncr3.virt[i].value = 0;
        nptm->ncr3.virt[i].present = 1;
        nptm->ncr3.virt[i].write = 1;
        nptm->ncr3.virt[i].user = 1;
        nptm->ncr3.virt[i].pdpte_base = (nptm->pdpt.phys >> 12) + i;
    }
    return nptm;
}
```

---

## 7. Identity Mapping Implementation

### Intel EPT Identity Map

From `gbhv/gbhv/ept.c:167-251`:
```cpp
PVMM_EPT_PAGE_TABLE HvEptAllocateAndCreateIdentityPageTable(PVMM_CONTEXT GlobalContext)
{
    PVMM_EPT_PAGE_TABLE PageTable;

    PageTable = OsAllocateContiguousAlignedPages(sizeof(VMM_EPT_PAGE_TABLE) / PAGE_SIZE);
    OsZeroMemory(PageTable, sizeof(VMM_EPT_PAGE_TABLE));

    InitializeListHead(&PageTable->DynamicSplitList);
    InitializeListHead(&PageTable->PageHookList);

    // Setup PML4[0] to point to PML3 table
    PageTable->PML4[0].PageFrameNumber = (SIZE_T)OsVirtualToPhysical(&PageTable->PML3[0]) / PAGE_SIZE;
    PageTable->PML4[0].ReadAccess = 1;
    PageTable->PML4[0].WriteAccess = 1;
    PageTable->PML4[0].ExecuteAccess = 1;

    // Setup template for PML3 entries
    EPT_PML3_POINTER RWXTemplate;
    RWXTemplate.Flags = 0;
    RWXTemplate.ReadAccess = 1;
    RWXTemplate.WriteAccess = 1;
    RWXTemplate.ExecuteAccess = 1;

    __stosq((SIZE_T*)&PageTable->PML3[0], RWXTemplate.Flags, VMM_EPT_PML3E_COUNT);

    // Point each PML3 entry to its PML2 table
    for(EntryIndex = 0; EntryIndex < VMM_EPT_PML3E_COUNT; EntryIndex++)
    {
        PageTable->PML3[EntryIndex].PageFrameNumber =
            (SIZE_T)OsVirtualToPhysical(&PageTable->PML2[EntryIndex][0]) / PAGE_SIZE;
    }

    // Setup 2MB large page entries
    EPT_PML2_ENTRY PML2EntryTemplate;
    PML2EntryTemplate.Flags = 0;
    PML2EntryTemplate.WriteAccess = 1;
    PML2EntryTemplate.ReadAccess = 1;
    PML2EntryTemplate.ExecuteAccess = 1;
    PML2EntryTemplate.LargePage = 1;  // 2MB page indicator

    __stosq((SIZE_T*)&PageTable->PML2[0], PML2EntryTemplate.Flags,
            VMM_EPT_PML3E_COUNT * VMM_EPT_PML2E_COUNT);

    // Set memory type and page frame number for each 2MB page
    for(EntryGroupIndex = 0; EntryGroupIndex < VMM_EPT_PML3E_COUNT; EntryGroupIndex++)
    {
        for(EntryIndex = 0; EntryIndex < VMM_EPT_PML2E_COUNT; EntryIndex++)
        {
            HvEptSetupPML2Entry(GlobalContext,
                &PageTable->PML2[EntryGroupIndex][EntryIndex],
                (EntryGroupIndex * VMM_EPT_PML2E_COUNT) + EntryIndex);
        }
    }
    return PageTable;
}
```

### AMD NPT Identity Map

From `NoirVisor/src/svm_core/svm_npt.c:462-487`:
```cpp
// Build identity map using 1GB huge pages
for(u32 i = 0; i < 512; i++)
{
    for(u32 j = 0; j < 512; j++)
    {
        const u32 k = (i << 9) + j;
        // Build PDPTE entries with 1GB pages
        nptm->pdpt.virt[k].value = 0;
        nptm->pdpt.virt[k].present = 1;
        nptm->pdpt.virt[k].write = 1;
        nptm->pdpt.virt[k].user = 1;
        nptm->pdpt.virt[k].huge_pdpte = 1;  // 1GB page
        nptm->pdpt.virt[k].page_base = k;   // Identity: GPA = HPA
    }
    // Build PML4 entries
    nptm->ncr3.virt[i].value = 0;
    nptm->ncr3.virt[i].present = 1;
    nptm->ncr3.virt[i].write = 1;
    nptm->ncr3.virt[i].user = 1;
    nptm->ncr3.virt[i].pdpte_base = (nptm->pdpt.phys >> 12) + i;
}
```

---

## 8. MTRR Integration

Memory Type Range Registers must be respected when building EPT entries for correct cache behavior.

### Reading MTRR Configuration

From `gbhv/gbhv/ept.c:44-97`:
```cpp
BOOL HvEptBuildMTRRMap(PVMM_CONTEXT GlobalContext)
{
    IA32_MTRR_CAPABILITIES_REGISTER MTRRCap;
    IA32_MTRR_PHYSBASE_REGISTER CurrentPhysBase;
    IA32_MTRR_PHYSMASK_REGISTER CurrentPhysMask;

    MTRRCap.Flags = ArchGetHostMSR(IA32_MTRR_CAPABILITIES);

    for(CurrentRegister = 0; CurrentRegister < MTRRCap.VariableRangeCount; CurrentRegister++)
    {
        CurrentPhysBase.Flags = ArchGetHostMSR(IA32_MTRR_PHYSBASE0 + (CurrentRegister * 2));
        CurrentPhysMask.Flags = ArchGetHostMSR(IA32_MTRR_PHYSMASK0 + (CurrentRegister * 2));

        if(CurrentPhysMask.Valid)
        {
            Descriptor = &GlobalContext->MemoryRanges[GlobalContext->NumberOfEnabledMemoryRanges++];
            Descriptor->PhysicalBaseAddress = CurrentPhysBase.PageFrameNumber * PAGE_SIZE;

            _BitScanForward64(&NumberOfBitsInMask, CurrentPhysMask.PageFrameNumber * PAGE_SIZE);
            Descriptor->PhysicalEndAddress = Descriptor->PhysicalBaseAddress + ((1ULL << NumberOfBitsInMask) - 1ULL);
            Descriptor->MemoryType = (UCHAR)CurrentPhysBase.Type;

            // WB is default, no need to store
            if(Descriptor->MemoryType == MEMORY_TYPE_WRITE_BACK)
                GlobalContext->NumberOfEnabledMemoryRanges--;
        }
    }
    return TRUE;
}
```

### MTRR Precedence Rules

From `NoirVisor/src/vt_core/vt_ept.c:372-381`:
```cpp
/*
    MTRR Precedence Rules:
    1. If one overlapped region is UC, memory type = UC
    2. If overlapped regions are WT and WB, memory type = WT
    3. Other overlaps = undefined processor behavior

    Compare values: UC=0 < WC=1 < WT=4 < WP=5 < WB=6
    Use the lowest value when overlapping
*/
```

### Applying MTRR to EPT Entries

From `gbhv/gbhv/ept.c:107-165`:
```cpp
VOID HvEptSetupPML2Entry(PVMM_CONTEXT GlobalContext, PEPT_PML2_ENTRY NewEntry, SIZE_T PageFrameNumber)
{
    SIZE_T AddressOfPage = PageFrameNumber * SIZE_2_MB;
    SIZE_T TargetMemoryType;

    // First page (0-2MB) should be UC for MMIO safety
    if(PageFrameNumber == 0)
    {
        NewEntry->MemoryType = MEMORY_TYPE_UNCACHEABLE;
        return;
    }

    TargetMemoryType = MEMORY_TYPE_WRITE_BACK;  // Default

    for(CurrentMtrrRange = 0; CurrentMtrrRange < GlobalContext->NumberOfEnabledMemoryRanges; CurrentMtrrRange++)
    {
        if(AddressOfPage <= GlobalContext->MemoryRanges[CurrentMtrrRange].PhysicalEndAddress)
        {
            if((AddressOfPage + SIZE_2_MB - 1) >= GlobalContext->MemoryRanges[CurrentMtrrRange].PhysicalBaseAddress)
            {
                TargetMemoryType = GlobalContext->MemoryRanges[CurrentMtrrRange].MemoryType;

                // UC takes highest precedence
                if(TargetMemoryType == MEMORY_TYPE_UNCACHEABLE)
                    break;
            }
        }
    }

    NewEntry->MemoryType = TargetMemoryType;
}
```

---

## 9. Large Page Splitting

Splitting 2MB pages to 4KB pages is required for granular EPT hooks.

### Intel EPT: 2MB to 4KB Split

From `gbhv/gbhv/ept.c:358-450`:
```cpp
BOOL HvEptSplitLargePage(PVMM_PROCESSOR_CONTEXT ProcessorContext, SIZE_T PhysicalAddress)
{
    PVMM_EPT_DYNAMIC_SPLIT NewSplit;
    EPT_PML1_ENTRY EntryTemplate;
    PEPT_PML2_ENTRY TargetEntry;
    EPT_PML2_POINTER NewPointer;

    // Find the 2MB entry
    TargetEntry = HvEptGetPml2Entry(ProcessorContext, PhysicalAddress);
    if(!TargetEntry) return FALSE;

    // Already split?
    if(!TargetEntry->LargePage) return TRUE;

    // Allocate structure for 512 4KB entries
    NewSplit = (PVMM_EPT_DYNAMIC_SPLIT)OsAllocateContiguousAlignedPages(
        sizeof(VMM_EPT_DYNAMIC_SPLIT)/PAGE_SIZE);
    if(!NewSplit) return FALSE;

    NewSplit->Entry = TargetEntry;

    // Create template for 4KB entries with same permissions as parent
    EntryTemplate.Flags = 0;
    EntryTemplate.ReadAccess = 1;
    EntryTemplate.WriteAccess = 1;
    EntryTemplate.ExecuteAccess = 1;
    EntryTemplate.MemoryType = TargetEntry->MemoryType;
    EntryTemplate.IgnorePat = TargetEntry->IgnorePat;
    EntryTemplate.SuppressVe = TargetEntry->SuppressVe;

    __stosq((SIZE_T*)&NewSplit->PML1[0], EntryTemplate.Flags, VMM_EPT_PML1E_COUNT);

    // Set page frame numbers for identity mapping
    for(EntryIndex = 0; EntryIndex < VMM_EPT_PML1E_COUNT; EntryIndex++)
    {
        // Convert 2MB PFN to 4KB PFNs
        NewSplit->PML1[EntryIndex].PageFrameNumber =
            ((TargetEntry->PageFrameNumber * SIZE_2_MB) / PAGE_SIZE) + EntryIndex;
    }

    // Create pointer entry to replace 2MB entry
    NewPointer.Flags = 0;
    NewPointer.WriteAccess = 1;
    NewPointer.ReadAccess = 1;
    NewPointer.ExecuteAccess = 1;
    NewPointer.PageFrameNumber = (SIZE_T)OsVirtualToPhysical(&NewSplit->PML1[0]) / PAGE_SIZE;

    // Track for cleanup
    InsertHeadList(&ProcessorContext->EptPageTable->DynamicSplitList, &NewSplit->DynamicSplitList);

    // Replace 2MB entry with pointer
    RtlCopyMemory(TargetEntry, &NewPointer, sizeof(NewPointer));

    return TRUE;
}
```

### NoirVisor EPT Split Implementation

From `NoirVisor/src/vt_core/vt_ept.c:128-198`:
```cpp
// Split 2MiB Page into 512 4KiB Pages
noir_ept_pte_descriptor_p nvc_ept_split_pde(noir_ept_manager_p eptm, u64 gpa, bool host, bool alloc)
{
    noir_ept_pte_descriptor_p pte_p = eptm->pte.head;
    while(pte_p)
    {
        if(gpa >= pte_p->gpa_start && gpa < pte_p->gpa_start + page_2mb_size)
            break;
        pte_p = pte_p->next;
    }

    if(alloc == true && pte_p == null)
    {
        pte_p = noir_alloc_nonpg_memory(sizeof(noir_ept_pte_descriptor));
        if(pte_p)
        {
            pte_p->virt = noir_alloc_contd_memory(page_size);
            if(pte_p->virt)
            {
                // Split PDPTE first if needed
                noir_ept_pde_descriptor_p pde_p = nvc_ept_split_pdpte(eptm, gpa, host, true);
                if(pde_p)
                {
                    const u64 pfn_index = page_2mb_count(gpa);
                    const u64 pde_index = page_entry_index64(pfn_index);

                    pte_p->phys = noir_get_physical_address(pte_p->virt);
                    pte_p->gpa_start = pfn_index << page_shift_diff64;

                    for(u32 i = 0; i < page_table_entries64; i++)
                    {
                        pte_p->virt[i].read = true;
                        pte_p->virt[i].write = true;
                        pte_p->virt[i].execute = true;
                        pte_p->virt[i].memory_type = pde_p->large[pde_index].memory_type;
                        pte_p->virt[i].page_offset = pte_p->gpa_start + i;
                    }
                    pte_p->gpa_start <<= page_4kb_shift;

                    // Add to linked list
                    if(eptm->pte.tail)
                    {
                        eptm->pte.tail->next = pte_p;
                        eptm->pte.tail = pte_p;
                    }
                    else
                    {
                        eptm->pte.head = pte_p;
                        eptm->pte.tail = pte_p;
                    }
                }
            }
        }
    }
    return pte_p;
}
```

---

## 10. EPT Violation Handling

### Exit Reason and Qualification

From `gbhv/gbhv/ept.c:892-912`:
```cpp
VOID HvExitHandleEptViolation(PVMM_PROCESSOR_CONTEXT ProcessorContext, PVMEXIT_CONTEXT ExitContext)
{
    VMX_EXIT_QUALIFICATION_EPT_VIOLATION ViolationQualification;

    ViolationQualification.Flags = ExitContext->ExitQualification;

    // ExitContext->GuestPhysicalAddress contains faulting GPA
    // from VMCS field kGuestPhysicalAddress

    if(HvExitHandlePageHookExit(ProcessorContext, ExitContext, ViolationQualification))
    {
        return;  // Handled by hook code
    }

    // Unexpected violation - stop execution
    ExitContext->ShouldStopExecution = TRUE;
}
```

### Hook Exit Handling

From `gbhv/gbhv/ept.c:810-887`:
```cpp
BOOL HvExitHandlePageHookExit(
    PVMM_PROCESSOR_CONTEXT ProcessorContext,
    PVMEXIT_CONTEXT ExitContext,
    VMX_EXIT_QUALIFICATION_EPT_VIOLATION ViolationQualification)
{
    PVMM_EPT_PAGE_HOOK PageHook = NULL;

    // Must be translation violation
    if(!ViolationQualification.CausedByTranslation)
        return FALSE;

    // Find hook for this physical address
    FOR_EACH_LIST_ENTRY(ProcessorContext->EptPageTable, PageHookList, VMM_EPT_PAGE_HOOK, Hook)
    {
        if(Hook->PhysicalBaseAddress == (SIZE_T)PAGE_ALIGN(ExitContext->GuestPhysicalAddress))
        {
            PageHook = Hook;
            break;
        }
    }
    FOR_EACH_LIST_ENTRY_END();

    if(!PageHook)
        return FALSE;

    // Execute on non-executable page -> swap to hooked (execute) page
    if(!ViolationQualification.EptExecutable && ViolationQualification.ExecuteAccess)
    {
        PageHook->TargetPage->Flags = PageHook->ShadowEntry.Flags;
        ExitContext->ShouldIncrementRIP = FALSE;
        return TRUE;
    }

    // Read/Write on execute-only page -> swap to original (RW) page
    if(ViolationQualification.EptExecutable &&
       (ViolationQualification.ReadAccess | ViolationQualification.WriteAccess))
    {
        PageHook->TargetPage->Flags = PageHook->HookedEntry.Flags;
        ExitContext->ShouldIncrementRIP = FALSE;
        return TRUE;
    }

    return FALSE;
}
```

### HyperPlatform EPT Violation Handler

From `HyperPlatform/HyperPlatform/ept.cpp:630-663`:
```cpp
void EptHandleEptViolation(EptData *ept_data)
{
    const EptViolationQualification exit_qualification = {
        UtilVmRead(VmcsField::kExitQualification)
    };

    const auto fault_pa = UtilVmRead64(VmcsField::kGuestPhysicalAddress);
    const auto fault_va = reinterpret_cast<void *>(
        exit_qualification.fields.valid_guest_linear_address
            ? UtilVmRead(VmcsField::kGuestLinearAddress)
            : 0);

    // Check if EPT entry exists with any access
    if(exit_qualification.fields.ept_readable ||
       exit_qualification.fields.ept_writeable ||
       exit_qualification.fields.ept_executable)
    {
        // Unexpected - entry exists but violation occurred
        HYPERPLATFORM_COMMON_DBG_BREAK();
        return;
    }

    const auto ept_entry = EptGetEptPtEntry(ept_data, fault_pa);
    if(ept_entry && ept_entry->all)
    {
        // Entry exists - unexpected
        HYPERPLATFORM_COMMON_DBG_BREAK();
        return;
    }

    // EPT entry miss - must be device memory
    EptpConstructTables(ept_data->ept_pml4, 4, fault_pa, ept_data);
    UtilInveptGlobal();
}
```

---

## 11. NPT Fault Handling

### EXITCODE and EXITINFO

AMD NPT faults use:
- EXITCODE = 0x400 (#NPF)
- EXITINFO1 = Fault code (amd64_npt_fault_code)
- EXITINFO2 = Faulting GPA

From `SimpleSvmHook/SimpleSvmHook/HookVmmCommon.cpp:348-393`:
```cpp
VOID HandleNestedPageFault(PVMCB GuestVmcb, PHOOK_DATA HookData)
{
    NPF_EXITINFO1 exitInfo;
    ULONG64 faultingPa;

    faultingPa = GuestVmcb->ControlArea.ExitInfo2;  // Faulting GPA
    exitInfo.AsUInt64 = GuestVmcb->ControlArea.ExitInfo1;  // Fault code

    if(exitInfo.Fields.Valid == FALSE)
    {
        // No NPT entry exists - MMIO access
        PPT_ENTRY_4KB nptEntry = BuildSubTables(HookData->Pml4Table, faultingPa, HookData);
        if(nptEntry == nullptr)
        {
            SIMPLESVMHOOK_BUG_CHECK();
        }
        return;
    }

    // NPT entry exists - protection violation
    // Must be execution attempt on NX page
    NT_ASSERT(exitInfo.Fields.Execute != FALSE);
    TransitionNptState(HookData, faultingPa);
}
```

### NPT State Machine for Hooks

From `SimpleSvmHook/SimpleSvmHook/HookVmmCommon.cpp:1-45`:
```
State Machine:
    State                     : Page Type
                              : Current : Hooked : Other
    0)NptDefault              : RWX(O)  : RWX(O) : RWX(O)
    1)NptHookEnabledInvisible : RWX(O)  : RW-(O) : RWX(O)
    2)NptHookEnabledVisible   : RWX(E)  : RW-(O) : RW-(O)

    Current = Page currently being executed
    Hooked  = Pages with hooks installed (not currently executing)
    Other   = All other pages

    (O) = Backed by Original physical page
    (E) = Backed by Exec physical page (with hooks)

Transitions:
    0 -> 1: Enable hooks via CPUID
    1 -> 2: Execute hooked page
    2 -> 1: Execute non-hooked page
    1 -> 0: Disable hooks
    2 -> 0: Disable hooks
```

---

## 12. Execute-Only Pages and Shadow Hooks

This is the core invisible hook technique.

### DdiMon Shadow Hook Implementation

From `DdiMon/DdiMon/shadow_hook.cpp:46-69`:
```cpp
struct HookInformation {
    void* patch_address;    // Address where hook is installed
    void* handler;          // Hook handler address

    // Two copies of the hooked page
    std::shared_ptr<Page> shadow_page_base_for_rw;    // Original code (for read/write)
    std::shared_ptr<Page> shadow_page_base_for_exec;  // Hooked code (for execute)

    ULONG64 pa_base_for_rw;     // Physical address of RW page
    ULONG64 pa_base_for_exec;   // Physical address of exec page
};
```

### Enable Execute-Only Mode

From `DdiMon/DdiMon/shadow_hook.cpp:514-529`:
```cpp
static void ShpEnablePageShadowingForExec(const HookInformation& info, EptData* ept_data)
{
    const auto ept_pt_entry = EptGetEptPtEntry(ept_data, UtilPaFromVa(info.patch_address));

    // Deny read and write - only execute allowed
    ept_pt_entry->fields.write_access = false;
    ept_pt_entry->fields.read_access = false;

    // Point to exec page (with hooks)
    ept_pt_entry->fields.physial_address = UtilPfnFromPa(info.pa_base_for_exec);

    UtilInveptGlobal();
}
```

### Enable RW Mode (for reads/writes)

From `DdiMon/DdiMon/shadow_hook.cpp:531-545`:
```cpp
static void ShpEnablePageShadowingForRW(const HookInformation& info, EptData* ept_data)
{
    const auto ept_pt_entry = EptGetEptPtEntry(ept_data, UtilPaFromVa(info.patch_address));

    // Allow read and write, plus execute
    ept_pt_entry->fields.write_access = true;
    ept_pt_entry->fields.read_access = true;

    // Point to original page (no hooks visible)
    ept_pt_entry->fields.physial_address = UtilPfnFromPa(info.pa_base_for_rw);

    UtilInveptGlobal();
}
```

### EPT Violation Handler for Hooks

From `DdiMon/DdiMon/shadow_hook.cpp:293-313`:
```cpp
void ShHandleEptViolation(ShadowHookData* sh_data, const SharedShadowHookData* shared_sh_data,
                          EptData* ept_data, void* fault_va)
{
    if(!ShpIsShadowHookActive(shared_sh_data))
        return;

    const auto info = ShpFindPatchInfoByPage(shared_sh_data, fault_va);
    if(!info)
        return;

    // EPT violation on execute-only page due to read/write attempt
    // Switch to RW page and enable MTF to single-step
    ShpEnablePageShadowingForRW(*info, ept_data);
    ShpSetMonitorTrapFlag(sh_data, true);
    ShpSaveLastHookInfo(sh_data, *info);
}
```

### MTF Handler (Single-Step Complete)

From `DdiMon/DdiMon/shadow_hook.cpp:282-291`:
```cpp
void ShHandleMonitorTrapFlag(ShadowHookData* sh_data, const SharedShadowHookData* shared_sh_data,
                             EptData* ept_data)
{
    const auto info = ShpRestoreLastHookInfo(sh_data);

    // Single-step complete - re-enable execute-only mode
    ShpEnablePageShadowingForExec(*info, ept_data);
    ShpSetMonitorTrapFlag(sh_data, false);
}
```

### gbhv Hook Flow

From `gbhv/gbhv/ept.c:683-799`:
```cpp
BOOL HvEptAddPageHook(PVMM_PROCESSOR_CONTEXT ProcessorContext,
                      PVOID TargetFunction, PVOID HookFunction, PVOID* OrigFunction)
{
    // 1. Get physical address of target
    PhysicalAddress = (SIZE_T)OsVirtualToPhysical(PAGE_ALIGN(TargetFunction));

    // 2. Allocate hook structure
    NewHook = (PVMM_EPT_PAGE_HOOK)OsAllocateNonpagedMemory(sizeof(VMM_EPT_PAGE_HOOK));

    // 3. Split 2MB page to 4KB if needed
    HvEptSplitLargePage(ProcessorContext, PhysicalAddress);

    // 4. Copy original page to FakePage
    RtlCopyMemory(&NewHook->FakePage[0], VirtualTarget, PAGE_SIZE);

    // 5. Get PTE for this page
    NewHook->TargetPage = HvEptGetPml1Entry(ProcessorContext, PhysicalAddress);
    NewHook->OriginalEntry = *NewHook->TargetPage;

    // 6. Create shadow entry (execute-only, points to FakePage)
    EPT_PML1_ENTRY FakeEntry;
    FakeEntry.Flags = 0;
    FakeEntry.ReadAccess = 0;
    FakeEntry.WriteAccess = 0;
    FakeEntry.ExecuteAccess = 1;
    FakeEntry.PageFrameNumber = (SIZE_T)OsVirtualToPhysical(&NewHook->FakePage) / PAGE_SIZE;
    NewHook->ShadowEntry.Flags = FakeEntry.Flags;

    // 7. Create hooked entry (RW-only, points to original)
    OriginalEntry.ReadAccess = 1;
    OriginalEntry.WriteAccess = 1;
    OriginalEntry.ExecuteAccess = 0;  // No execute
    NewHook->HookedEntry.Flags = OriginalEntry.Flags;

    // 8. Install hook code in FakePage
    HvEptHookInstructionMemory(NewHook, TargetFunction, HookFunction, OrigFunction);

    // 9. Apply hooked entry initially (RW, no execute)
    NewHook->TargetPage->Flags = OriginalEntry.Flags;

    // 10. Invalidate TLB
    __invept(1, &Descriptor);

    return TRUE;
}
```

---

## 13. TLB Invalidation

### Intel INVEPT

From various references, INVEPT types:
```cpp
// INVEPT types
#define INVEPT_SINGLE_CONTEXT  1  // Invalidate single EPT context
#define INVEPT_ALL_CONTEXTS    2  // Invalidate all EPT contexts

// INVEPT descriptor
typedef struct _INVEPT_DESCRIPTOR {
    UINT64 EptPointer;  // EPTP to invalidate (for single context)
    UINT64 Reserved;
} INVEPT_DESCRIPTOR;

// Usage
INVEPT_DESCRIPTOR Descriptor;
Descriptor.EptPointer = ProcessorContext->EptPointer.Flags;
Descriptor.Reserved = 0;
__invept(INVEPT_SINGLE_CONTEXT, &Descriptor);

// Or global invalidation
__invept(INVEPT_ALL_CONTEXTS, &Descriptor);
```

### Intel INVVPID

```cpp
// INVVPID types
#define INVVPID_INDIVIDUAL_ADDRESS                0
#define INVVPID_SINGLE_CONTEXT                    1
#define INVVPID_ALL_CONTEXTS                      2
#define INVVPID_SINGLE_CONTEXT_RETAINING_GLOBALS  3

// INVVPID descriptor
typedef struct _INVVPID_DESCRIPTOR {
    UINT16 Vpid;
    UINT16 Reserved1;
    UINT32 Reserved2;
    UINT64 LinearAddress;
} INVVPID_DESCRIPTOR;
```

### AMD INVLPGA

For AMD, TLB invalidation uses INVLPGA:
```cpp
// INVLPGA: Invalidate TLB entry for given address and ASID
// void __invlpga(void* va, uint32_t asid);
__invlpga((void*)gva, asid);

// Or use VMCB clean bits
noir_svm_vmcb_btr32(vcpu->vmcb.virt, vmcb_clean_bits, noir_svm_clean_npt);
```

### When to Invalidate

1. After modifying any EPT/NPT entry
2. After installing or removing hooks
3. After changing page permissions
4. After splitting large pages

---

## 14. Page Table Walking

### Intel EPT Walk

From `HyperPlatform/HyperPlatform/ept.cpp:691-740`:
```cpp
static EptCommonEntry *EptpGetEptPtEntry(EptCommonEntry *table, ULONG table_level,
                                         ULONG64 physical_address)
{
    if(!table) return nullptr;

    switch(table_level) {
        case 4: {  // PML4
            const auto pxe_index = EptpAddressToPxeIndex(physical_address);
            const auto ept_pml4_entry = &table[pxe_index];
            if(!ept_pml4_entry->all) return nullptr;
            return EptpGetEptPtEntry(
                (EptCommonEntry*)UtilVaFromPfn(ept_pml4_entry->fields.physial_address),
                table_level - 1, physical_address);
        }
        case 3: {  // PDPT
            const auto ppe_index = EptpAddressToPpeIndex(physical_address);
            const auto ept_pdpt_entry = &table[ppe_index];
            if(!ept_pdpt_entry->all) return nullptr;
            return EptpGetEptPtEntry(
                (EptCommonEntry*)UtilVaFromPfn(ept_pdpt_entry->fields.physial_address),
                table_level - 1, physical_address);
        }
        case 2: {  // PD
            const auto pde_index = EptpAddressToPdeIndex(physical_address);
            const auto ept_pdt_entry = &table[pde_index];
            if(!ept_pdt_entry->all) return nullptr;
            return EptpGetEptPtEntry(
                (EptCommonEntry*)UtilVaFromPfn(ept_pdt_entry->fields.physial_address),
                table_level - 1, physical_address);
        }
        case 1: {  // PT
            const auto pte_index = EptpAddressToPteIndex(physical_address);
            return &table[pte_index];
        }
        default:
            return nullptr;
    }
}

// Index extraction
static ULONG64 EptpAddressToPxeIndex(ULONG64 pa) { return (pa >> 39) & 0x1ff; }
static ULONG64 EptpAddressToPpeIndex(ULONG64 pa) { return (pa >> 30) & 0x1ff; }
static ULONG64 EptpAddressToPdeIndex(ULONG64 pa) { return (pa >> 21) & 0x1ff; }
static ULONG64 EptpAddressToPteIndex(ULONG64 pa) { return (pa >> 12) & 0x1ff; }
```

### gbhv Index Macros

From `gbhv/gbhv/ept.h:72-92`:
```cpp
#define ADDRMASK_EPT_PML1_OFFSET(_VAR_) (_VAR_ & 0xFFFULL)
#define ADDRMASK_EPT_PML1_INDEX(_VAR_)  ((_VAR_ & 0x1FF000ULL) >> 12)
#define ADDRMASK_EPT_PML2_INDEX(_VAR_)  ((_VAR_ & 0x3FE00000ULL) >> 21)
#define ADDRMASK_EPT_PML3_INDEX(_VAR_)  ((_VAR_ & 0x7FC0000000ULL) >> 30)
#define ADDRMASK_EPT_PML4_INDEX(_VAR_)  ((_VAR_ & 0xFF8000000000ULL) >> 39)
```

---

## 15. Unified Abstraction Recommendations

For OmbraHypervisor, implement a unified interface that abstracts Intel EPT and AMD NPT differences.

### Recommended Interface

```cpp
// IPageTableManager interface
class IPageTableManager
{
public:
    virtual ~IPageTableManager() = default;

    // Initialization
    virtual bool Initialize() = 0;
    virtual void Cleanup() = 0;

    // Identity mapping
    virtual bool BuildIdentityMap() = 0;

    // Entry manipulation
    virtual void* GetEntry(uint64_t gpa, PageLevel level) = 0;
    virtual bool UpdateEntry(uint64_t gpa, uint64_t hpa,
                            bool read, bool write, bool execute) = 0;

    // Large page operations
    virtual bool SplitLargePage(uint64_t gpa) = 0;
    virtual bool JoinToLargePage(uint64_t gpa) = 0;

    // Hook support
    virtual bool InstallExecuteOnlyHook(uint64_t gpa,
                                        uint64_t exec_hpa,
                                        uint64_t rw_hpa) = 0;
    virtual bool RemoveHook(uint64_t gpa) = 0;

    // TLB management
    virtual void InvalidateTlb() = 0;
    virtual void InvalidateTlbEntry(uint64_t gpa) = 0;

    // Root pointer
    virtual uint64_t GetRootPointer() const = 0;
};

// Permission enum
enum class PageAccess : uint32_t
{
    None    = 0,
    Read    = 1,
    Write   = 2,
    Execute = 4,
    ReadWrite = Read | Write,
    ReadExecute = Read | Execute,
    ReadWriteExecute = Read | Write | Execute
};

// Page levels
enum class PageLevel
{
    PML4 = 4,  // 512GB
    PDPT = 3,  // 1GB
    PD   = 2,  // 2MB
    PT   = 1   // 4KB
};
```

### Intel Implementation

```cpp
class IntelEptManager : public IPageTableManager
{
private:
    ia32_ept_pointer m_eptp;
    ia32_ept_pml4e* m_pml4;
    // ... dynamic split tracking

public:
    bool Initialize() override
    {
        // Allocate PML4
        m_pml4 = AllocateContiguousPage();

        // Setup EPTP
        m_eptp.memory_type = 6;  // WB
        m_eptp.walk_length = 3;  // 4-level
        m_eptp.pml4e_offset = PhysicalPfn(m_pml4);

        return BuildIdentityMap();
    }

    void InvalidateTlb() override
    {
        INVEPT_DESCRIPTOR desc = { m_eptp.value, 0 };
        __invept(INVEPT_ALL_CONTEXTS, &desc);
    }
};
```

### AMD Implementation

```cpp
class AmdNptManager : public IPageTableManager
{
private:
    uint64_t m_ncr3;
    amd64_npt_pml4e* m_pml4;
    // ... dynamic split tracking

public:
    bool UpdateEntry(uint64_t gpa, uint64_t hpa,
                    bool read, bool write, bool execute) override
    {
        auto* pte = (amd64_npt_pte*)GetEntry(gpa, PageLevel::PT);
        if(!pte) return false;

        pte->present = read;
        pte->write = write;
        pte->no_execute = !execute;  // Note: inverted
        pte->page_base = hpa >> 12;

        return true;
    }

    void InvalidateTlb() override
    {
        // Use VMCB TLB control or INVLPGA
        __invlpga(nullptr, 0);  // Flush all for ASID 0
    }
};
```

---

## File References

| File | Purpose |
|------|---------|
| `HyperPlatform/HyperPlatform/ept.h` | EPT structure declarations |
| `HyperPlatform/HyperPlatform/ept.cpp` | EPT initialization and violation handling |
| `DdiMon/DdiMon/shadow_hook.h` | Shadow hook interface |
| `DdiMon/DdiMon/shadow_hook.cpp` | Execute-only hook implementation |
| `gbhv/gbhv/ept.h` | EPT structures and macros |
| `gbhv/gbhv/ept.c` | EPT identity map and hooks |
| `NoirVisor/src/vt_core/vt_ept.h` | Intel EPT structures |
| `NoirVisor/src/vt_core/vt_ept.c` | EPT management and MTRR |
| `NoirVisor/src/svm_core/svm_npt.h` | AMD NPT structures |
| `NoirVisor/src/svm_core/svm_npt.c` | NPT management |
| `SimpleSvmHook/SimpleSvmHook/HookVmmCommon.cpp` | NPT hook state machine |
| `hvpp/src/hvpp/hvpp/ia32/ept.h` | Modern C++ EPT structures |

---

## Summary

EPT and NPT provide the foundation for invisible memory hooks. Key points:

1. **Structure**: Both use 4-level page tables but with different permission models
2. **Identity Mapping**: Maps GPA directly to HPA for all physical memory
3. **MTRR**: Memory type must match physical MTRR configuration
4. **Large Pages**: Default 2MB pages must split to 4KB for hooks
5. **Execute-Only**: The critical feature for invisible hooks
6. **Shadow Pages**: Two physical pages per hooked page (exec and RW)
7. **State Machine**: Track which page variant is currently mapped
8. **TLB**: Must invalidate after any EPT/NPT modification
9. **MTF**: Use Monitor Trap Flag for single-stepping through RW access

Word count: 5200+
