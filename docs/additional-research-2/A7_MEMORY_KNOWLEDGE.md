# A7: Hypervisor Memory Management Knowledge

## Overview

This document captures comprehensive knowledge about hypervisor-level memory management extracted from NoirVisor, HyperPlatform, Kernel-Bridge, gbhv, and hvpp reference implementations. It covers physical memory allocation, virtual-to-physical translation, EPT/NPT identity mapping, MTRR handling, and safe guest memory access patterns.

---

## 1. Hypervisor Memory Requirements

### 1.1 Per-CPU Structures

Each logical processor requires dedicated memory regions for VMX/SVM operation:

**Intel VT-x Per-CPU Structures:**
| Structure | Size | Alignment | Allocation Method |
|-----------|------|-----------|-------------------|
| VMXON Region | 4KB | 4KB aligned | Contiguous physical memory |
| VMCS Region | 4KB | 4KB aligned | Contiguous physical memory |
| MSR Bitmap | 4KB | 4KB aligned | Contiguous physical memory |
| I/O Bitmap A | 4KB | 4KB aligned | Contiguous physical memory (optional) |
| I/O Bitmap B | 4KB | 4KB aligned | Contiguous physical memory (optional) |
| Host Stack | 8KB-32KB | 16-byte aligned | Non-paged pool |

**AMD SVM Per-CPU Structures:**
| Structure | Size | Alignment | Allocation Method |
|-----------|------|-----------|-------------------|
| VMCB | 4KB | 4KB aligned | Contiguous physical memory |
| HSAVE Area | 4KB | 4KB aligned | Contiguous physical memory |
| MSRPM | 8KB | 4KB aligned | Contiguous physical memory |
| IOPM | 12KB | 4KB aligned | Contiguous physical memory |
| Host Stack | 8KB-32KB | 16-byte aligned | Non-paged pool |

**Reference: gbhv/vmm.h:31-131**
```c
typedef struct _VMM_PROCESSOR_CONTEXT
{
    BOOL HasLaunched;
    PVMM_CONTEXT GlobalContext;
    PVMXON_REGION VmxonRegion;
    PPHYSVOID VmxonRegionPhysical;
    PVMCS VmcsRegion;
    PPHYSVOID VmcsRegionPhysical;
    PVMX_MSR_BITMAP MsrBitmap;
    PPHYSVOID MsrBitmapPhysical;
    REGISTER_CONTEXT InitialRegisters;
    IA32_SPECIAL_REGISTERS InitialSpecialRegisters;
    VMM_HOST_STACK_REGION HostStack;
    EPT_POINTER EptPointer;
    PVMM_EPT_PAGE_TABLE EptPageTable;
} VMM_PROCESSOR_CONTEXT;
```

### 1.2 EPT/NPT Page Tables

**Identity Map Memory Consumption (NoirVisor design for 256TB):**

| Level | Structure | Count | Memory |
|-------|-----------|-------|--------|
| PML4 | PML4E entries | 512 | 4KB |
| PDPT | PDPTE entries | 512 x 512 = 262,144 | 2MB |
| PDE | Large 2MB entries (base) | 512 x 512 = 262,144 | 2MB |

**Reference: NoirVisor/src/vt_core/vt_ept.c:652-662**
```c
/*
  Memory Consumption in Paging of each vCPU:
  4KB for 1 PML4E page - all 512 entries for 256TB
  2MB for 512 PDPTE pages - all 262144 entries for 256TB
  2MB for 512 PDE pages - entries for lower 512GB
  Note: we expect only lower 512GB as RAM, higher for MMIO.
*/
```

### 1.3 Dynamic Split Lists

When hooking specific 4KB pages, 2MB large pages must be split into 512 4KB entries:

**Reference: gbhv/ept.h:137-158**
```c
typedef struct _VMM_EPT_DYNAMIC_SPLIT
{
    DECLSPEC_ALIGN(PAGE_SIZE) EPT_PML1_ENTRY PML1[VMM_EPT_PML1E_COUNT];
    union {
        PEPT_PML2_ENTRY Entry;
        PEPT_PML2_POINTER Pointer;
    };
    LIST_ENTRY DynamicSplitList;
} VMM_EPT_DYNAMIC_SPLIT;
```

---

## 2. Physical Memory Allocation

### 2.1 Pre-Virtualization Allocation (Windows)

Before entering VMX/SVM operation, use OS allocators:

**Reference: NoirVisor/src/xpf_core/windows/nvsys.c:163-239**
```c
PVOID NoirAllocateContiguousMemory(IN SIZE_T Length)
{
    PHYSICAL_ADDRESS H = {0xFFFFFFFFFFFFFFFF};
    PVOID p = MmAllocateContiguousMemory(Length, H);
    if(p)
    {
        RtlZeroMemory(p, Length);
        InterlockedIncrement(&NoirAllocatedContiguousMemoryCount);
    }
    return p;
}

PVOID NoirAllocateNonPagedMemory(IN SIZE_T Length)
{
    // For WDK 20348+
    PVOID p = ExAllocatePool2(POOL_FLAG_NON_PAGED_EXECUTE, Length, 'pNvN');
    // Older WDK
    // PVOID p = ExAllocatePoolWithTag(NonPagedPool, Length, 'pNvN');
    if(p)
    {
        RtlZeroMemory(p, Length);
        InterlockedIncrement(&NoirAllocatedNonPagedPools);
    }
    return p;
}
```

**Reference: Kernel-Bridge/API/MemoryUtils.cpp:328-355**
```cpp
PVOID AllocPhysicalMemory(PVOID64 HighestAcceptableAddress, SIZE_T Size) {
    return MmAllocateContiguousMemory(
        Size,
        *reinterpret_cast<PHYSICAL_ADDRESS*>(&HighestAcceptableAddress)
    );
}

PVOID AllocPhysicalMemorySpecifyCache(
    PVOID64 LowestAcceptableAddress,
    PVOID64 HighestAcceptableAddress,
    PVOID64 BoundaryAddressMultiple,
    SIZE_T Size,
    MEMORY_CACHING_TYPE CachingType
) {
    return MmAllocateContiguousMemorySpecifyCache(
        Size,
        *reinterpret_cast<PHYSICAL_ADDRESS*>(&LowestAcceptableAddress),
        *reinterpret_cast<PHYSICAL_ADDRESS*>(&HighestAcceptableAddress),
        *reinterpret_cast<PHYSICAL_ADDRESS*>(&BoundaryAddressMultiple),
        CachingType
    );
}
```

### 2.2 Post-Virtualization Allocation Challenge

**Critical**: In VMX-root mode, standard OS allocators cannot be used directly.

**Reference: hvpp/vcpu.cpp:778-786**
```cpp
// Because we're in VMX-root mode, we can't use the system allocator
// (ExAllocatePoolWithTag/ExFreePoolWithTag).
// This line will enable "custom allocator" that will be used whenever
// "new"/"delete" operator is executed.
mm::allocator_guard _;
```

**Solutions:**
1. Pre-allocate all memory before VMXON
2. Use per-CPU memory pools
3. Implement custom hypervisor allocator
4. Use VMCALL to request allocation from guest context

### 2.3 2MB Aligned Page Allocation

For PDPT arrays and large structures:

**Reference: NoirVisor uses `noir_alloc_2mb_page()` for PDPT allocation**
```c
// From nvc_ept_build_identity_map()
eptm->pdpt.virt = noir_alloc_2mb_page();
if(eptm->pdpt.virt)
{
    eptm->pdpt.phys = noir_get_physical_address(eptm->pdpt.virt);
    alloc_success = true;
}
```

---

## 3. Virtual Address Space Management

### 3.1 Host CR3 Selection

The hypervisor must use a stable CR3 that persists across all VM-exits:

**Reference: hvpp/vcpu.cpp:609-618**
```cpp
//
// Notice how we're setting "system_cr3" as HOST_CR3.
// Because current function can be called in context of any process
// (which can die at any time), we can't use current CR3.
// We MUST use such CR3 that will live long enough - "System" process.
//
host_cr0(read<cr0_t>());
host_cr3(mm::paging_descriptor().system_cr3());
host_cr4(read<cr4_t>());
```

**Reference: gbhv/vmm.c:112-114**
```c
// Save SYSTEM process DTB
Context->SystemDirectoryTableBase = __readcr3();
```

### 3.2 Stack Management

Each VCPU needs dedicated stack space for VM-exit handling:

**Reference: hvpp/vcpu.h:363-396**
```cpp
struct stack_t
{
    static constexpr auto size = 0x8000;  // 32KB

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
        uint64_t dummy[4];  // 32 bytes for ABI compliance
    };

    union
    {
        uint8_t data[size];
        struct
        {
            uint8_t dummy[size - sizeof(shadow_space_t) - sizeof(machine_frame_t) - sizeof(uint64_t)];
            shadow_space_t shadow_space;
            machine_frame_t machine_frame;
            uint64_t unused;
        };
    };
};
```

**Reference: gbhv/vmm.h:31-45**
```c
typedef struct _VMM_HOST_STACK_REGION
{
    CHAR HostStack[VMM_SETTING_STACK_SPACE];  // Typically 0x6000
    PVMM_CONTEXT GlobalContext;  // At top of stack for easy access
} VMM_HOST_STACK_REGION;
```

---

## 4. Guest Physical to Host Physical Translation (GPA -> HPA)

### 4.1 EPT Walk Algorithm

**Reference: NoirVisor/src/vt_core/vt_ept.c - PTE lookup pattern**
```c
// Get PML2 entry for physical address
PEPT_PML2_ENTRY HvEptGetPml2Entry(PVMM_PROCESSOR_CONTEXT Ctx, SIZE_T PhysAddr)
{
    SIZE_T Directory = ADDRMASK_EPT_PML2_INDEX(PhysAddr);
    SIZE_T DirectoryPointer = ADDRMASK_EPT_PML3_INDEX(PhysAddr);
    SIZE_T PML4Entry = ADDRMASK_EPT_PML4_INDEX(PhysAddr);

    // Addresses above 512GB are invalid (> physical address bus width)
    if(PML4Entry > 0) return NULL;

    return &Ctx->EptPageTable->PML2[DirectoryPointer][Directory];
}
```

**Index Extraction Macros (gbhv/ept.h:72-92):**
```c
#define ADDRMASK_EPT_PML1_OFFSET(_VAR_) (_VAR_ & 0xFFFULL)
#define ADDRMASK_EPT_PML1_INDEX(_VAR_)  ((_VAR_ & 0x1FF000ULL) >> 12)
#define ADDRMASK_EPT_PML2_INDEX(_VAR_)  ((_VAR_ & 0x3FE00000ULL) >> 21)
#define ADDRMASK_EPT_PML3_INDEX(_VAR_)  ((_VAR_ & 0x7FC0000000ULL) >> 30)
#define ADDRMASK_EPT_PML4_INDEX(_VAR_)  ((_VAR_ & 0xFF8000000000ULL) >> 39)
```

### 4.2 NPT Walk Algorithm (AMD)

**Reference: NoirVisor/src/svm_core/svm_npt.c:269-285**
```c
bool nvc_npt_update_pte(noir_npt_manager_p nptm, u64 hpa, u64 gpa,
                        bool r, bool w, bool x, bool alloc)
{
    // Split the PDE first
    noir_npt_pte_descriptor_p pte_p = nvc_npt_split_pde(nptm, gpa, true, alloc);
    if(pte_p)
    {
        amd64_addr_translator gat;
        gat.value = gpa;

        // Update the specific PTE
        pte_p->virt[gat.pte_offset].present = r;
        pte_p->virt[gat.pte_offset].write = w;
        pte_p->virt[gat.pte_offset].no_execute = !x;
        pte_p->virt[gat.pte_offset].page_base = page_4kb_count(hpa);
        return true;
    }
    return false;
}
```

---

## 5. Guest Virtual to Host Physical Translation (GVA -> HPA)

### 5.1 Full Translation Chain

GVA -> GPA (guest page tables) -> HPA (EPT/NPT)

**Reference: hvpp/vcpu.cpp:419-432**
```cpp
auto vcpu_t::guest_va_to_pa(va_t guest_va) noexcept -> pa_t
{
    return translator_.va_to_pa(guest_va, ::detail::kernel_cr3(guest_cr3()));
}

auto vcpu_t::guest_read_memory(va_t guest_va, void* buffer, size_t size,
                                bool ignore_errors) noexcept -> va_t
{
    return translator_.read(guest_va, ::detail::kernel_cr3(guest_cr3()),
                           buffer, size, ignore_errors);
}

auto vcpu_t::guest_write_memory(va_t guest_va, const void* buffer, size_t size,
                                 bool ignore_errors) noexcept -> va_t
{
    return translator_.write(guest_va, ::detail::kernel_cr3(guest_cr3()),
                            buffer, size, ignore_errors);
}
```

### 5.2 Page Table Walking (x64)

**Reference: Kernel-Bridge/API/PteUtils.cpp:19-139**
```cpp
BOOLEAN GetPageTables(PVOID Address, OUT PAGE_TABLES_INFO* Info)
{
    VIRTUAL_ADDRESS Va = {};
    Va.Value = reinterpret_cast<unsigned long long>(Address);

    CR3 Cr3 = {};
    Cr3.Value = static_cast<unsigned long long>(__readcr3());

    // PML4E
    PVOID64 Pml4ePhys = reinterpret_cast<PVOID64>(
        PFN_TO_PAGE(Cr3.x64.Bitmap.PML4) +
        Va.x64.Generic.PageMapLevel4Offset * sizeof(PML4E::x64));
    Info->Pml4e = reinterpret_cast<PML4E*>(GetVirtualForPhysical(Pml4ePhys));
    if (!Info->Pml4e) return FALSE;
    if (!Info->Pml4e->x64.Generic.P) return TRUE;

    // PDPE
    PVOID64 PdpePhys = reinterpret_cast<PVOID64>(
        PFN_TO_PAGE(Info->Pml4e->x64.Generic.PDP) +
        Va.x64.Generic.PageDirectoryPointerOffset * sizeof(PDPE::x64));
    Info->Pdpe = reinterpret_cast<PDPE*>(GetVirtualForPhysical(PdpePhys));
    if (!Info->Pdpe) return FALSE;

    // Check for 1GB page
    if (Info->Pdpe->x64.Generic.PS)
    {
        Info->Type = PAGE_TABLES_INFO::pt64Page1Gb;
        return TRUE;
    }

    // PDE
    PVOID64 PdePhys = reinterpret_cast<PVOID64>(
        PFN_TO_PAGE(Info->Pdpe->x64.NonPageSize.Generic.PD) +
        Va.x64.NonPageSize.Generic.PageDirectoryOffset * sizeof(PDE::x64));
    Info->Pde = reinterpret_cast<PDE*>(GetVirtualForPhysical(PdePhys));
    if (!Info->Pde) return FALSE;

    // Check for 2MB page
    if (Info->Pde->x64.Generic.PS)
    {
        Info->Type = PAGE_TABLES_INFO::pt64Page2Mb;
        return TRUE;
    }

    // PTE (4KB page)
    Info->Type = PAGE_TABLES_INFO::pt64Page4Kb;
    PVOID64 PtePhys = reinterpret_cast<PVOID64>(
        PFN_TO_PAGE(Info->Pde->x64.Page4Kb.PT) +
        Va.x64.NonPageSize.Page4Kb.PageTableOffset * sizeof(PTE::x64));
    Info->Pte = reinterpret_cast<PTE*>(GetVirtualForPhysical(PtePhys));

    return TRUE;
}
```

---

## 6. Safe Memory Access Patterns

### 6.1 Physical Memory Read/Write

**Reference: Kernel-Bridge/API/MemoryUtils.cpp:389-446**
```cpp
BOOLEAN ReadPhysicalMemory(IN PVOID64 PhysicalAddress, OUT PVOID Buffer,
                           SIZE_T Length, MEMORY_CACHING_TYPE CachingType)
{
    BOOLEAN Status;

    // Try direct VA mapping first (faster)
    PVOID VirtualAddress = GetVirtualForPhysical(PhysicalAddress);
    if (VirtualAddress) {
        __try {
            VirtualMemory::CopyMemory(Buffer, VirtualAddress, Length);
            Status = TRUE;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            Status = FALSE;
        }
    } else {
        Status = FALSE;
    }

    // Fall back to MmMapIoSpace if direct mapping failed
    if (!Status) {
        PVOID MappedMemory = MapPhysicalMemory(PhysicalAddress, Length, CachingType);
        if (!MappedMemory) return FALSE;
        __try {
            VirtualMemory::CopyMemory(Buffer, MappedMemory, Length);
            Status = TRUE;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            Status = FALSE;
        }
        UnmapPhysicalMemory(MappedMemory, Length);
    }

    return Status;
}
```

### 6.2 Address Validation

**Reference: Kernel-Bridge/API/MemoryUtils.cpp:173-191**
```cpp
BOOLEAN IsAddressValid(PVOID Address) {
    return MmIsAddressValid(Address);
}

BOOLEAN IsPagePresent(PVOID Address) {
    return PhysicalMemory::GetPhysicalAddress(Address) || MmIsAddressValid(Address);
}

BOOLEAN IsMemoryRangePresent(PVOID Address, SIZE_T Size) {
    PVOID PageCounter = ALIGN_DOWN_POINTER_BY(Address, PAGE_SIZE);
    do {
        if (!IsPagePresent(PageCounter)) return FALSE;
        PageCounter = reinterpret_cast<PVOID>(
            reinterpret_cast<SIZE_T>(PageCounter) + PAGE_SIZE);
    } while (reinterpret_cast<SIZE_T>(PageCounter) <
             reinterpret_cast<SIZE_T>(Address) + Size);
    return TRUE;
}
```

### 6.3 User Memory Probing

**Reference: Kernel-Bridge/API/MemoryUtils.cpp:193-229**
```cpp
BOOLEAN CheckUserMemoryReadable(__in_data_source(USER_MODE) PVOID UserAddress,
                                SIZE_T Size) {
    __try {
        ProbeForRead(UserAddress, Size, 1);
        return TRUE;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return FALSE;
    }
}

BOOLEAN SecureMemory(PVOID UserAddress, SIZE_T Size, ULONG ProtectRights,
                     OUT PHANDLE SecureHandle) {
    if (!SecureHandle || !Size || AddressRange::IsKernelAddress(UserAddress))
        return FALSE;
    *SecureHandle = MmSecureVirtualMemory(UserAddress, Size, ProtectRights);
    return *SecureHandle != NULL;
}
```

---

## 7. EPT/NPT Identity Mapping

### 7.1 Building Identity Map (Intel EPT)

**Reference: gbhv/ept.c:167-251**
```c
PVMM_EPT_PAGE_TABLE HvEptAllocateAndCreateIdentityPageTable(PVMM_CONTEXT Ctx)
{
    PVMM_EPT_PAGE_TABLE PageTable;
    EPT_PML3_POINTER RWXTemplate;
    EPT_PML2_ENTRY PML2EntryTemplate;

    // Allocate all paging structures as 4KB aligned pages
    PageTable = OsAllocateContiguousAlignedPages(sizeof(VMM_EPT_PAGE_TABLE) / PAGE_SIZE);
    if(PageTable == NULL) return NULL;

    OsZeroMemory(PageTable, sizeof(VMM_EPT_PAGE_TABLE));
    InitializeListHead(&PageTable->DynamicSplitList);
    InitializeListHead(&PageTable->PageHookList);

    // Mark first 512GB PML4 entry as present (manages up to 512GB)
    PageTable->PML4[0].PageFrameNumber =
        (SIZE_T)OsVirtualToPhysical(&PageTable->PML3[0]) / PAGE_SIZE;
    PageTable->PML4[0].ReadAccess = 1;
    PageTable->PML4[0].WriteAccess = 1;
    PageTable->PML4[0].ExecuteAccess = 1;

    // Setup RWX template for PML3 entries
    RWXTemplate.Flags = 0;
    RWXTemplate.ReadAccess = 1;
    RWXTemplate.WriteAccess = 1;
    RWXTemplate.ExecuteAccess = 1;
    __stosq((SIZE_T*)&PageTable->PML3[0], RWXTemplate.Flags, VMM_EPT_PML3E_COUNT);

    // Link each PML3 entry to its PML2 array
    for(SIZE_T i = 0; i < VMM_EPT_PML3E_COUNT; i++)
    {
        PageTable->PML3[i].PageFrameNumber =
            (SIZE_T)OsVirtualToPhysical(&PageTable->PML2[i][0]) / PAGE_SIZE;
    }

    // Setup 2MB large page template
    PML2EntryTemplate.Flags = 0;
    PML2EntryTemplate.WriteAccess = 1;
    PML2EntryTemplate.ReadAccess = 1;
    PML2EntryTemplate.ExecuteAccess = 1;
    PML2EntryTemplate.LargePage = 1;
    __stosq((SIZE_T*)&PageTable->PML2[0], PML2EntryTemplate.Flags,
            VMM_EPT_PML3E_COUNT * VMM_EPT_PML2E_COUNT);

    // Setup memory types based on MTRRs
    for(SIZE_T i = 0; i < VMM_EPT_PML3E_COUNT; i++)
    {
        for(SIZE_T j = 0; j < VMM_EPT_PML2E_COUNT; j++)
        {
            HvEptSetupPML2Entry(Ctx, &PageTable->PML2[i][j],
                               (i * VMM_EPT_PML2E_COUNT) + j);
        }
    }

    return PageTable;
}
```

### 7.2 Building Identity Map (AMD NPT)

**Reference: NoirVisor/src/svm_core/svm_npt.c:440-496**
```c
noir_npt_manager_p nvc_npt_build_identity_map()
{
    noir_npt_manager_p nptm = noir_alloc_nonpg_memory(
        sizeof(noir_npt_manager) + sizeof(noir_hook_page) * noir_hook_pages_count);

    if(nptm)
    {
        nptm->ncr3.virt = noir_alloc_contd_memory(page_size);
        if(nptm->ncr3.virt)
        {
            nptm->pdpt.virt = noir_alloc_2mb_page();
            if(nptm->pdpt.virt)
            {
                nptm->pdpt.phys = noir_get_physical_address(nptm->pdpt.virt);
                // Build PDPTE entries (1GB huge pages)
                for(u32 i = 0; i < 512; i++)
                {
                    for(u32 j = 0; j < 512; j++)
                    {
                        const u32 k = (i << 9) + j;
                        nptm->pdpt.virt[k].value = 0;
                        nptm->pdpt.virt[k].present = 1;
                        nptm->pdpt.virt[k].write = 1;
                        nptm->pdpt.virt[k].user = 1;
                        nptm->pdpt.virt[k].huge_pdpte = 1;
                        nptm->pdpt.virt[k].page_base = k;
                    }
                    // Build PML4E entries
                    nptm->ncr3.virt[i].value = 0;
                    nptm->ncr3.virt[i].present = 1;
                    nptm->ncr3.virt[i].write = 1;
                    nptm->ncr3.virt[i].user = 1;
                    nptm->ncr3.virt[i].pdpte_base = (nptm->pdpt.phys >> 12) + i;
                }
            }
            nptm->ncr3.phys = noir_get_physical_address(nptm->ncr3.virt);
        }
    }
    return nptm;
}
```

---

## 8. Memory Type (MTRR) Handling

### 8.1 MTRR Overview

Memory Type Range Registers define cacheability attributes for physical memory regions:

| Type | Value | Description |
|------|-------|-------------|
| UC | 0 | Uncacheable |
| WC | 1 | Write Combining |
| WT | 4 | Write Through |
| WP | 5 | Write Protected |
| WB | 6 | Write Back |

### 8.2 Building MTRR Map

**Reference: gbhv/ept.c:44-97**
```c
BOOL HvEptBuildMTRRMap(PVMM_CONTEXT GlobalContext)
{
    IA32_MTRR_CAPABILITIES_REGISTER MTRRCap;
    IA32_MTRR_PHYSBASE_REGISTER CurrentPhysBase;
    IA32_MTRR_PHYSMASK_REGISTER CurrentPhysMask;
    PMTRR_RANGE_DESCRIPTOR Descriptor;
    ULONG NumberOfBitsInMask;

    MTRRCap.Flags = ArchGetHostMSR(IA32_MTRR_CAPABILITIES);

    for(ULONG i = 0; i < MTRRCap.VariableRangeCount; i++)
    {
        CurrentPhysBase.Flags = ArchGetHostMSR(IA32_MTRR_PHYSBASE0 + (i * 2));
        CurrentPhysMask.Flags = ArchGetHostMSR(IA32_MTRR_PHYSMASK0 + (i * 2));

        if(CurrentPhysMask.Valid)
        {
            Descriptor = &GlobalContext->MemoryRanges[
                GlobalContext->NumberOfEnabledMemoryRanges++];

            // Calculate base address in bytes
            Descriptor->PhysicalBaseAddress = CurrentPhysBase.PageFrameNumber * PAGE_SIZE;

            // Calculate range size from mask
            _BitScanForward64(&NumberOfBitsInMask,
                             CurrentPhysMask.PageFrameNumber * PAGE_SIZE);

            Descriptor->PhysicalEndAddress = Descriptor->PhysicalBaseAddress +
                                            ((1ULL << NumberOfBitsInMask) - 1ULL);
            Descriptor->MemoryType = (UCHAR)CurrentPhysBase.Type;

            // Skip WB ranges (default type)
            if(Descriptor->MemoryType == MEMORY_TYPE_WRITE_BACK)
                GlobalContext->NumberOfEnabledMemoryRanges--;
        }
    }
    return TRUE;
}
```

### 8.3 Applying MTRRs to EPT

**Reference: gbhv/ept.c:107-165**
```c
VOID HvEptSetupPML2Entry(PVMM_CONTEXT Ctx, PEPT_PML2_ENTRY Entry, SIZE_T PFN)
{
    SIZE_T AddressOfPage = PFN * SIZE_2_MB;
    SIZE_T TargetMemoryType;

    Entry->PageFrameNumber = PFN;

    // First page (0-2MB) should be UC for safety (MMIO in first MB)
    if(PFN == 0)
    {
        Entry->MemoryType = MEMORY_TYPE_UNCACHEABLE;
        return;
    }

    // Default to WB for performance
    TargetMemoryType = MEMORY_TYPE_WRITE_BACK;

    // Check against each MTRR range
    for(SIZE_T i = 0; i < Ctx->NumberOfEnabledMemoryRanges; i++)
    {
        if(AddressOfPage <= Ctx->MemoryRanges[i].PhysicalEndAddress)
        {
            if((AddressOfPage + SIZE_2_MB - 1) >= Ctx->MemoryRanges[i].PhysicalBaseAddress)
            {
                TargetMemoryType = Ctx->MemoryRanges[i].MemoryType;

                // UC always takes precedence (11.11.4.1 MTRR Precedences)
                if(TargetMemoryType == MEMORY_TYPE_UNCACHEABLE)
                    break;
            }
        }
    }

    Entry->MemoryType = TargetMemoryType;
}
```

### 8.4 Fixed MTRR Handling

Fixed MTRRs cover the first 1MB with finer granularity:

**Reference: NoirVisor/src/vt_core/vt_ept.c:464-541**
```c
// Fixed Range MTRRs span the first MiB
u64 fix64k_00000 = noir_rdmsr(ia32_mtrr_fix64k_00000);  // 8x64KB ranges
u64 fix16k_80000 = noir_rdmsr(ia32_mtrr_fix16k_80000);  // 8x16KB ranges
u64 fix16k_a0000 = noir_rdmsr(ia32_mtrr_fix16k_a0000);  // 8x16KB ranges
u64 fix4k_c0000  = noir_rdmsr(ia32_mtrr_fix4k_c0000);   // 8x4KB ranges
// ... and so on

// Apply to first 2MB PTE entries
noir_ept_pte_descriptor_p pte_p = nvc_ept_split_pde(eptm, 0, true, true);

// MTRR Fixed64K_00000: 8 ranges of 64KB (512KB total)
type = (u8*)&fix64k_00000;
for(u32 i = 0; i < 8; i++)
    for(u32 j = 0; j < 16; j++)  // 64KB = 16 pages
        pte_p->virt[(i << 4) + j].memory_type = type[i];
```

---

## 9. Page Splitting (2MB -> 4KB)

When hooking individual 4KB pages, large pages must be split:

**Reference: gbhv/ept.c:358-450**
```c
BOOL HvEptSplitLargePage(PVMM_PROCESSOR_CONTEXT Ctx, SIZE_T PhysicalAddress)
{
    PVMM_EPT_DYNAMIC_SPLIT NewSplit;
    EPT_PML1_ENTRY EntryTemplate;
    PEPT_PML2_ENTRY TargetEntry;
    EPT_PML2_POINTER NewPointer;

    // Find current PML2 entry
    TargetEntry = HvEptGetPml2Entry(Ctx, PhysicalAddress);
    if(!TargetEntry) return FALSE;

    // Already split if not marked as large page
    if(!TargetEntry->LargePage) return TRUE;

    // Allocate PML1 entries for the split
    NewSplit = (PVMM_EPT_DYNAMIC_SPLIT)OsAllocateContiguousAlignedPages(
        sizeof(VMM_EPT_DYNAMIC_SPLIT) / PAGE_SIZE);
    if(!NewSplit) return FALSE;

    NewSplit->Entry = TargetEntry;

    // Create RWX template
    EntryTemplate.Flags = 0;
    EntryTemplate.ReadAccess = 1;
    EntryTemplate.WriteAccess = 1;
    EntryTemplate.ExecuteAccess = 1;
    EntryTemplate.MemoryType = TargetEntry->MemoryType;
    EntryTemplate.IgnorePat = TargetEntry->IgnorePat;
    EntryTemplate.SuppressVe = TargetEntry->SuppressVe;

    __stosq((SIZE_T*)&NewSplit->PML1[0], EntryTemplate.Flags, VMM_EPT_PML1E_COUNT);

    // Set PFNs for identity mapping
    for(SIZE_T i = 0; i < VMM_EPT_PML1E_COUNT; i++)
    {
        NewSplit->PML1[i].PageFrameNumber =
            ((TargetEntry->PageFrameNumber * SIZE_2_MB) / PAGE_SIZE) + i;
    }

    // Create pointer to new PML1 array
    NewPointer.Flags = 0;
    NewPointer.WriteAccess = 1;
    NewPointer.ReadAccess = 1;
    NewPointer.ExecuteAccess = 1;
    NewPointer.PageFrameNumber =
        (SIZE_T)OsVirtualToPhysical(&NewSplit->PML1[0]) / PAGE_SIZE;

    // Track allocation for cleanup
    InsertHeadList(&Ctx->EptPageTable->DynamicSplitList, &NewSplit->DynamicSplitList);

    // Replace large page entry with pointer
    RtlCopyMemory(TargetEntry, &NewPointer, sizeof(NewPointer));

    return TRUE;
}
```

---

## 10. Hypervisor Protection

### 10.1 Protecting Critical Structures

**Reference: NoirVisor/src/vt_core/vt_ept.c:606-643**
```c
bool nvc_ept_protect_hypervisor(noir_hypervisor_p hvm, noir_ept_manager_p eptm)
{
    // Allocate blank page for redirection
    eptm->blank_page.virt = noir_alloc_contd_memory(page_size);
    if(eptm->blank_page.virt)
    {
        eptm->blank_page.phys = noir_get_physical_address(eptm->blank_page.virt);

        // Protect MSR bitmap
        nvc_ept_update_pte(eptm, hvm->relative_hvm->msr_bitmap.phys,
                          eptm->blank_page.phys, true, true, true, true, 0, true);

        for(u32 i = 0; i < hvm->cpu_count; i++)
        {
            noir_vt_vcpu_p vcpu = &hvm->virtual_cpu[i];

            // Protect VMXON region and VMCS
            nvc_ept_update_pte(eptm, vcpu->vmxon.phys,
                              eptm->blank_page.phys, true, true, true, true, 0, true);
            nvc_ept_update_pte(eptm, vcpu->vmcs.phys,
                              eptm->blank_page.phys, true, true, true, true, 0, true);

            // Protect MSR Auto List
            nvc_ept_update_pte(eptm, vcpu->msr_auto.phys,
                              eptm->blank_page.phys, true, true, true, true, 0, true);

            // Protect EPT structure itself
            nvc_ept_update_pte(eptm, eptmt->eptp.phys.value,
                              eptm->blank_page.phys, true, true, true, true, 0, true);
        }
        return true;
    }
    return false;
}
```

---

## 11. TLB Invalidation

### 11.1 INVEPT (Intel)

**Reference: gbhv/ept.c:789-796**
```c
if (ProcessorContext->HasLaunched)
{
    INVEPT_DESCRIPTOR Descriptor;
    Descriptor.EptPointer = ProcessorContext->EptPointer.Flags;
    Descriptor.Reserved = 0;
    __invept(1, &Descriptor);  // Type 1 = Single-context invalidation
}
```

### 11.2 Guidelines for INVEPT/INVVPID

**Reference: hvpp/vcpu.cpp:301-318**
```cpp
// Software can use INVVPID with "all-context" type immediately after
// VMXON or before VMXOFF to prevent retention of cached information
// between separate uses of VMX operation.
vmx::invvpid_all_contexts();

// Software can use INVEPT with "all-context" type immediately after
// VMXON or before VMXOFF to prevent retention of cached EPT info.
vmx::invept_all_contexts();
```

---

## 12. OmbraHypervisor Implementation Strategy

### 12.1 Memory Allocation Order

1. **Pre-Virtualization Phase:**
   - Allocate global VMM context
   - Allocate per-CPU contexts (VMXON, VMCS/VMCB, MSR bitmaps)
   - Build MTRR map
   - Build EPT/NPT identity map
   - Pre-allocate dynamic split pool

2. **Per-CPU Initialization:**
   - Enter VMX/SVM root mode
   - Load VMCS/VMCB
   - Set HOST_CR3 to system CR3
   - Configure host stack pointer

3. **Runtime:**
   - Use pre-allocated pools for dynamic operations
   - VMCALL for any memory allocation requests
   - Careful TLB invalidation after EPT/NPT modifications

### 12.2 Recommended Structure Sizes

```c
// OmbraHypervisor recommended allocations
#define OMBRA_HOST_STACK_SIZE       0x8000    // 32KB per CPU
#define OMBRA_DYNAMIC_SPLIT_POOL    64        // Pre-allocate 64 splits
#define OMBRA_EPT_PML3E_COUNT       512       // For 512GB coverage
#define OMBRA_EPT_PML2E_COUNT       512       // Per PML3 entry
```

### 12.3 Error Handling Patterns

Always check allocations and clean up on failure:

```c
PVOID AllocateHypervisorMemory(SIZE_T Size)
{
    PVOID p = MmAllocateContiguousMemory(Size, HighestAcceptableAddress);
    if (!p) {
        // Log error
        return NULL;
    }
    RtlZeroMemory(p, Size);
    // Track allocation for cleanup
    return p;
}
```

---

## Summary

This document provides the foundational knowledge for implementing Ring -1 memory management in OmbraHypervisor:

1. **Allocation**: Use `MmAllocateContiguousMemory` for structures requiring physical address translation
2. **Identity Mapping**: Build EPT/NPT with 2MB large pages, split only when necessary for hooks
3. **MTRRs**: Read and apply memory type information to EPT/NPT entries
4. **Translation**: Walk guest page tables (CR3 -> PML4 -> PDPT -> PD -> PT) then EPT/NPT
5. **Protection**: Redirect hypervisor structures through blank pages
6. **Invalidation**: Use INVEPT/INVVPID after any EPT/NPT modification

All code patterns are derived from production hypervisor implementations with proven stability.
