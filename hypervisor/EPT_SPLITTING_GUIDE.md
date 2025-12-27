# EPT Page Splitting Implementation Guide

## Overview

This document describes the implementation of EPT page splitting in OmbraHypervisor. Page splitting is essential for fine-grained memory hooks, allowing the hypervisor to control individual 4KB pages within large 1GB or 2MB regions.

## Memory Hierarchy

EPT uses a 4-level page table hierarchy similar to regular x86-64 paging:

```
PML4 (512 entries, each covers 512GB)
  └─> PDPT (512 entries, each covers 1GB)
       ├─> 1GB Large Page (if bit 7 = 1)
       └─> PD (512 entries, each covers 2MB)
            ├─> 2MB Large Page (if bit 7 = 1)
            └─> PT (512 entries, each covers 4KB)
                 └─> 4KB Page
```

## Splitting Strategy

OmbraHypervisor uses **1GB pages** by default for the identity map (512 x 1GB = 512GB). To hook a specific 4KB page, we must split down:

1. **1GB → 2MB**: Convert PDPT large page entry into a PD table with 512 x 2MB entries
2. **2MB → 4KB**: Convert PD large page entry into a PT table with 512 x 4KB entries

This implementation focuses on **step 1: 1GB to 2MB splitting**.

## Implementation Details

### EPT State Modifications

Added to `EPT_STATE` structure in `ept.h`:

```c
// Memory pool tracking
void*       EptMemoryBase;              // Base virtual address of EPT pool
U64         EptMemoryPhysical;          // Base physical address of EPT pool
U32         TotalPagesAllocated;        // Total 4KB pages in pool
U32         PagesUsed;                  // Pages currently consumed

// Split table tracking
void*       SplitPdTables[512];         // Virtual addresses of split PD tables
void*       SplitPtTables[512];         // Virtual addresses of split PT tables (future)
U32         SplitPdCount;               // Number of 1GB pages split
U32         SplitPtCount;               // Number of 2MB pages split (future)
```

### Memory Pool Management

The EPT tables are allocated from a contiguous memory pool:

```
Offset 0:       PML4 table (4KB)
Offset 4KB:     PDPT table (4KB)
Offset 8KB:     First split PD table (4KB)
Offset 12KB:    Second split PD table (4KB)
...
```

The `EptAllocatePage()` function allocates from this pool:

```c
static OMBRA_STATUS EptAllocatePage(
    EPT_STATE* ept,
    void** outVirtual,
    U64* outPhysical
)
```

It:
- Checks if space is available (`PagesUsed < TotalPagesAllocated`)
- Calculates offset into pool: `offset = PagesUsed * 4KB`
- Returns virtual and physical addresses
- Increments `PagesUsed` counter

### 1GB to 2MB Split Algorithm

`EptSplit1GbTo2Mb(ept, guestPhysical)` performs the split:

#### Step 1: Validate and Locate Entry

```c
U32 pdptIndex = EPT_PDPT_INDEX(guestPhysical);  // Bits 38:30 of GPA
EPT_PDPTE* pdpte = &ept->Pdpt[pdptIndex];
```

Checks:
- Is `pdpte->LargePage.LargePage` bit set? (If not, already split)
- Is `SplitPdTables[pdptIndex]` NULL? (Verify not already tracked)

#### Step 2: Capture Original 1GB Page Properties

Before modifying the PDPTE, we save:

```c
U64 base1GbPhysical = pdpte->LargePage.PagePhysAddr << 30;
U8 memoryType = pdpte->LargePage.MemoryType;
U64 permissions = pdpte->LargePage.Read/Write/Execute/ExecuteUser;
```

#### Step 3: Allocate New PD Table

```c
EptAllocatePage(ept, &pdVirtual, &pdPhysical);
EptZeroPage(pdVirtual);  // Zero 512 entries
```

#### Step 4: Populate PD with 512 x 2MB Entries

Each 2MB entry maps a slice of the original 1GB page:

```c
for (i = 0; i < 512; i++) {
    U64 page2MbPhysical = base1GbPhysical + (i * 2MB);

    pd[i].LargePage.Read = originalPermissions;
    pd[i].LargePage.Write = originalPermissions;
    pd[i].LargePage.Execute = originalPermissions;
    pd[i].LargePage.LargePage = 1;  // 2MB large page
    pd[i].LargePage.MemoryType = memoryType;
    pd[i].LargePage.PagePhysAddr = page2MbPhysical >> 21;
}
```

**Key Point**: The identity mapping is preserved. GPA N still maps to HPA N, but now at 2MB granularity.

#### Step 5: Convert PDPTE to Pointer

Change the PDPTE from a 1GB large page to a PD table pointer:

```c
pdpte->Value = 0;  // Clear all bits
pdpte->Pointer.Read = 1;
pdpte->Pointer.Write = 1;
pdpte->Pointer.Execute = 1;
pdpte->Pointer.ExecuteUser = 1;
pdpte->Pointer.PdPhysAddr = pdPhysical >> 12;  // Bits 51:12
```

**Critical**: The `LargePage` bit (bit 7) is now **0**, indicating this is a pointer to the next level.

#### Step 6: Track and Invalidate

```c
ept->SplitPdTables[pdptIndex] = pdVirtual;  // Track for future access
ept->SplitPdCount++;
EptInvalidate(ept, INVEPT_TYPE_SINGLE_CONTEXT);  // Flush TLB
```

### Error Handling

The function handles:

1. **Invalid parameters**: NULL `ept`, uninitialized state
2. **Out of bounds GPA**: PDPT index >= 512
3. **Already split**: Returns success (idempotent operation)
4. **Out of memory**: Returns `OMBRA_ERROR_NO_MEMORY` if pool exhausted

### Logging

The implementation uses tiered logging:

- **INFO**: Major operations (split initiated, split complete)
- **TRACE**: Detailed state (physical addresses, entry counts, allocation)
- **WARN**: Unexpected but recoverable conditions (already split)
- **ERR**: Fatal errors (out of memory, invalid parameters)

Example output:

```
[INFO] EPT: Splitting 1GB page at PDPT[0] (GPA 0x0) into 512 x 2MB pages
[TRACE] EPT: 1GB page base phys=0x0, memory type=6
[TRACE] EPT: Allocated page 2 at virt=0x... phys=0x...
[TRACE] EPT: Populated PD with 512 x 2MB entries (0x0 - 0x3fffffff)
[INFO] EPT: Split complete - PDPT[0] now points to PD at phys=0x... (split count=1)
```

## Usage Example

```c
EPT_STATE ept;
OMBRA_STATUS status;

// Initialize EPT with 512 pages (2MB) memory pool
EptInitialize(&ept, pml4, pml4Phys, pdpt, pdptPhys, 512);

// Split the 1GB page containing address 0x12345000
status = EptSplit1GbTo2Mb(&ept, 0x12345000);
if (status != OMBRA_SUCCESS) {
    // Handle error
}

// Now you can access the 2MB entries
EPT_PDE* pd = (EPT_PDE*)ept.SplitPdTables[0];  // PDPT[0] was split
// Modify specific 2MB entries or further split them to 4KB
```

## Memory Requirements

- **Minimum pool size**: 2 pages (PML4 + PDPT)
- **Each 1GB split**: Requires 1 additional page (PD table)
- **Recommended pool**: 512 pages (2MB) allows splitting all 512 x 1GB regions

Memory usage formula:
```
PagesNeeded = 2 + SplitPdCount + (SplitPtCount * 512)
```

Where:
- `SplitPdCount` = Number of 1GB pages split to 2MB
- `SplitPtCount` = Number of 2MB pages split to 4KB (future work)

## Future Work: 2MB to 4KB Splitting

The next implementation phase will add `EptSplit2MbTo4Kb()`:

1. Access the PD table: `pd = SplitPdTables[pdptIndex]`
2. Get the PDE: `pde = &pd[pdIndex]`
3. Allocate a PT table (512 x 4KB entries)
4. Populate PT with 512 x 4KB pages
5. Convert PDE from 2MB large page to PT pointer
6. Track in `SplitPtTables` (indexed by `pdptIndex * 512 + pdIndex`)

## Testing

A test harness is provided in `test_ept_split.c`. It verifies:

1. Basic split operation (PDPT[0])
2. Multiple splits (PDPT[1], PDPT[2], ...)
3. Idempotent re-split (splitting already split page)
4. Memory exhaustion handling
5. PD entry correctness (physical addresses, permissions, large page bit)

Run with:
```bash
cl test_ept_split.c /I. /DTEST_MODE
./test_ept_split
```

## References

- Intel SDM Volume 3, Chapter 28.2: EPT Translation Mechanism
- Intel SDM Volume 3, Table 28-1: Format of EPTP
- Intel SDM Volume 3, Table 28-5: Format of EPT Page-Directory-Pointer-Table Entry
- Intel SDM Volume 3, Table 28-6: Format of EPT Page-Directory Entry

## Implementation Status

- [x] EPT state structure enhancements
- [x] Memory pool management (`EptAllocatePage`, `EptZeroPage`)
- [x] 1GB to 2MB splitting (`EptSplit1GbTo2Mb`)
- [x] Split table tracking
- [x] INVEPT invalidation
- [x] Comprehensive error handling
- [x] Debug logging
- [x] Test harness
- [ ] 2MB to 4KB splitting (future)
- [ ] Integration with hook framework (future)
- [ ] EPT entry modification at 4KB granularity (future)
