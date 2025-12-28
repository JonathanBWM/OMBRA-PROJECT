# ept.c Implementation Details

## VERIFICATION STATUS
```
VERIFICATION:
- [x] I read the entire file (658 lines)
- [x] I verified all function signatures
- [x] I traced all code paths
- [x] I verified EPT structure formats against Intel SDM
- [ ] I tested execution

UNVERIFIED CLAIMS:
- INVEPT instruction behavior
- Memory type assignment correctness for MMIO
- Thread safety during split operations

ASSUMPTIONS:
- 1GB page support available (fails otherwise)
- Contiguous memory from loader is physically contiguous
- No concurrent EPT modifications during split
```

## DOCUMENTED FROM
```
File: hypervisor/hypervisor/ept.c
Git hash: 73853be
Lines: 658
Date: 2025-12-27
```

---

## File Purpose

Extended Page Tables management: identity mapping, page splitting (1GB→2MB→4KB), permission modification, and EPTP construction.

---

## EPT Structure Hierarchy

```
EPT_STATE
    │
    ├── EPTP (64-bit)           ─→ Points to PML4
    │       ├── Bits 2:0  = Memory type (6 = WB)
    │       ├── Bits 5:3  = Page walk length - 1 (3)
    │       ├── Bit 6     = Accessed/dirty enable
    │       └── Bits 51:12 = PML4 physical address
    │
    ├── PML4 (512 entries, 4KB)
    │       └── Entry 0 points to PDPT
    │
    ├── PDPT (512 entries, 4KB)
    │       └── 512 × 1GB large pages = 512GB identity map
    │
    ├── [Split PD Tables]  (on demand)
    │       └── 512 × 2MB large pages
    │
    └── [Split PT Tables]  (on demand)
            └── 512 × 4KB pages
```

---

## Function-by-Function Analysis

### EptSupports1GbPages (lines 14-19)

**Purpose**: Check if CPU supports 1GB EPT pages.

```c
bool EptSupports1GbPages(void) {
    U64 eptCap = __readmsr(MSR_IA32_VMX_EPT_VPID_CAP);
    return (eptCap & (1ULL << 17)) != 0;  // Bit 17
}
```

**VERIFIED**: Bit 17 per Intel SDM Vol 3C Appendix A.10.

---

### BuildEptp (lines 39-57)

**Purpose**: Construct EPTP (EPT Pointer) value.

```c
static U64 BuildEptp(U64 pml4Physical, bool accessedDirty) {
    U64 eptp = 0;

    eptp |= EPT_MEMORY_TYPE_WB;         // Bits 2:0 = 6 (WB)
    eptp |= (3ULL << 3);                 // Bits 5:3 = 3 (4-level - 1)
    if (accessedDirty && EptSupportsAccessedDirty()) {
        eptp |= (1ULL << 6);             // Bit 6 = A/D enable
    }
    eptp |= (pml4Physical & 0x000FFFFFFFFFF000ULL);  // Bits 51:12

    return eptp;
}
```

**VERIFIED**: Format matches Intel SDM Vol 3C Section 24.6.11.

---

### EptInitialize (lines 63-182)

**Purpose**: Create 512GB identity map with 1GB pages.

**Algorithm**:
1. Validate parameters (ept, pml4, pdpt pointers)
2. Check 1GB page support (fails if not available)
3. Initialize EPT_STATE structure
4. Setup memory pool tracking (2 pages used: PML4 + PDPT)
5. Zero PML4 and PDPT tables
6. Set PML4[0] to point to PDPT with RWX permissions
7. Fill PDPT with 512 × 1GB identity-mapped pages
8. Build EPTP and mark initialized

**Key Code - 1GB Page Entry Setup**:
```c
for (i = 0; i < EPT_ENTRIES_PER_TABLE; i++) {
    U64 physAddr = (U64)i * EPT_PAGE_SIZE_1G;  // 1GB per entry

    pdpt[i].Value = 0;
    pdpt[i].LargePage.Read = 1;
    pdpt[i].LargePage.Write = 1;
    pdpt[i].LargePage.Execute = 1;
    pdpt[i].LargePage.ExecuteUser = 1;
    pdpt[i].LargePage.LargePage = 1;           // Mark as 1GB page
    pdpt[i].LargePage.MemoryType = EPT_MEMORY_TYPE_WB;
    pdpt[i].LargePage.PagePhysAddr = physAddr >> 30;  // Bits 51:30
}
```

**Memory Layout After Init**:
- Page 0: PML4 (only entry 0 used)
- Page 1: PDPT (all 512 entries used as 1GB pages)
- Remaining pages: Reserved for splits

**VERIFIED**: Correct 4-level paging structure per SDM.

---

### EptAllocatePage (lines 190-223)

**Purpose**: Allocate 4KB page from contiguous pool.

**Algorithm**:
```c
if (ept->PagesUsed >= ept->TotalPagesAllocated) {
    return OMBRA_ERROR_NO_MEMORY;
}

offset = ept->PagesUsed * EPT_PAGE_SIZE_4K;
*outVirtual = baseVirtual + offset;
*outPhysical = ept->EptMemoryPhysical + offset;
ept->PagesUsed++;
```

**IMPORTANT**: Relies on contiguous physical memory from loader.

---

### EptSplit1GbTo2Mb (lines 244-352)

**Purpose**: Split a 1GB page into 512 × 2MB pages.

**Algorithm**:
1. Get PDPT index from GPA
2. Verify it's a large page (1GB)
3. Capture original properties (physical base, memory type)
4. Allocate new PD table (4KB)
5. Fill PD with 512 × 2MB entries (inherit permissions)
6. Convert PDPTE from large page to pointer
7. Track in SplitPdTables array
8. Invalidate EPT

**Key Code - Permission Inheritance**:
```c
for (i = 0; i < EPT_ENTRIES_PER_TABLE; i++) {
    U64 page2MbPhysical = base1GbPhysical + ((U64)i * EPT_PAGE_SIZE_2M);

    pd[i].LargePage.Read = pdpte->LargePage.Read;
    pd[i].LargePage.Write = pdpte->LargePage.Write;
    pd[i].LargePage.Execute = pdpte->LargePage.Execute;
    pd[i].LargePage.ExecuteUser = pdpte->LargePage.ExecuteUser;
    pd[i].LargePage.LargePage = 1;
    pd[i].LargePage.MemoryType = memoryType;
    pd[i].LargePage.PagePhysAddr = page2MbPhysical >> 21;
}
```

---

### EptSplit2MbTo4Kb (lines 424-567)

**Purpose**: Split a 2MB page into 512 × 4KB pages.

**Prerequisite**: 1GB must be split to 2MB first.

**Algorithm**:
1. Walk EPT to find PDE (PML4→PDPT→PD)
2. Verify it's a 2MB large page
3. Capture original properties
4. Allocate new PT table (4KB)
5. Fill PT with 512 × 4KB entries
6. Convert PDE from large page to pointer
7. Invalidate EPT

**Key Code - 4KB Entry Setup**:
```c
for (i = 0; i < EPT_ENTRIES_PER_TABLE; i++) {
    U64 page4KbPhysical = base2MbPhysical + ((U64)i * EPT_PAGE_SIZE_4K);

    pt[i].Read = pde->LargePage.Read;
    pt[i].Write = pde->LargePage.Write;
    pt[i].Execute = pde->LargePage.Execute;
    pt[i].ExecuteUser = pde->LargePage.ExecuteUser;
    pt[i].MemoryType = memoryType;
    pt[i].PagePhysAddr = page4KbPhysical >> 12;
}
```

---

### EptSplitLargePage (lines 573-613)

**Purpose**: Generic split to 4KB (handles both 1GB and 2MB).

**Algorithm**:
```c
if (pdpte->LargePage.LargePage) {
    // 1GB page - split to 2MB first
    status = EptSplit1GbTo2Mb(ept, guestPhysical);
    if (OMBRA_FAILED(status)) return status;
    // Then split 2MB to 4KB
    return EptSplit2MbTo4Kb(ept, guestPhysical);
} else {
    // Already 2MB, just split to 4KB
    return EptSplit2MbTo4Kb(ept, guestPhysical);
}
```

---

### EptInvalidate (lines 645-657)

**Purpose**: Invalidate EPT TLB entries.

```c
void EptInvalidate(EPT_STATE* ept, U64 inveptType) {
    INVEPT_DESCRIPTOR desc;
    desc.EptPointer = ept->Eptp;
    desc.Reserved = 0;
    AsmInvept(inveptType, &desc);  // Assembly wrapper
}
```

**INVEPT Types**:
- `INVEPT_TYPE_SINGLE_CONTEXT` (1): Invalidate for single EPTP
- `INVEPT_TYPE_ALL_CONTEXT` (2): Invalidate all EPT contexts

---

## Memory Pool Tracking

```c
ept->EptMemoryBase = pml4Virtual;
ept->EptMemoryPhysical = pml4Physical;
ept->TotalPagesAllocated = totalPagesAllocated;
ept->PagesUsed = 2;  // PML4 + PDPT initially

// Split tracking
ept->SplitPdTables[pdptIndex] = pdVirtual;  // PD for 1GB→2MB split
ept->SplitPtCount++;                        // Count of 4KB splits
```

---

## Address Index Extraction

```c
#define EPT_PML4_INDEX(addr)  (((addr) >> 39) & 0x1FF)  // Bits 47:39
#define EPT_PDPT_INDEX(addr)  (((addr) >> 30) & 0x1FF)  // Bits 38:30
#define EPT_PD_INDEX(addr)    (((addr) >> 21) & 0x1FF)  // Bits 29:21
#define EPT_PT_INDEX(addr)    (((addr) >> 12) & 0x1FF)  // Bits 20:12
```

---

## Data Structures

### EPT_STATE (from ept.h)

```c
typedef struct _EPT_STATE {
    EPT_PML4E*  Pml4;
    U64         Pml4Physical;
    EPT_PDPTE*  Pdpt;
    U64         PdptPhysical;
    U64         Eptp;
    U32         HookCount;
    bool        Initialized;

    // Memory pool
    void*       EptMemoryBase;
    U64         EptMemoryPhysical;
    U32         TotalPagesAllocated;
    U32         PagesUsed;

    // Split tracking
    void*       SplitPdTables[512];  // PD tables from 1GB splits
    U32         SplitPdCount;
    U32         SplitPtCount;
} EPT_STATE;
```

### EPT Entry Unions (INFERRED from usage)

```c
union EPT_PDPTE {
    U64 Value;
    struct {    // Large page (1GB)
        U64 Read : 1;
        U64 Write : 1;
        U64 Execute : 1;
        U64 MemoryType : 3;
        U64 IgnorePat : 1;
        U64 LargePage : 1;     // Set to 1 for 1GB page
        U64 Accessed : 1;
        U64 Dirty : 1;
        U64 ExecuteUser : 1;
        U64 Reserved1 : 19;
        U64 PagePhysAddr : 22; // Bits 51:30
        U64 Reserved2 : 12;
    } LargePage;
    struct {    // Pointer to PD
        U64 Read : 1;
        U64 Write : 1;
        U64 Execute : 1;
        U64 Reserved1 : 5;
        U64 Accessed : 1;
        U64 Reserved2 : 2;
        U64 ExecuteUser : 1;
        U64 PdPhysAddr : 40;   // Bits 51:12
        U64 Reserved3 : 12;
    } Pointer;
};
```

---

## Constants

| Constant | Value | Meaning |
|----------|-------|---------|
| `EPT_PAGE_SIZE_4K` | 0x1000 | 4KB |
| `EPT_PAGE_SIZE_2M` | 0x200000 | 2MB |
| `EPT_PAGE_SIZE_1G` | 0x40000000 | 1GB |
| `EPT_ENTRIES_PER_TABLE` | 512 | Entries in each table |
| `EPT_MEMORY_TYPE_WB` | 6 | Write-back caching |

---

## MSR Usage

| MSR | Bit | Purpose |
|-----|-----|---------|
| `MSR_IA32_VMX_EPT_VPID_CAP` | 14 | WB memory type support |
| `MSR_IA32_VMX_EPT_VPID_CAP` | 17 | 1GB page support |
| `MSR_IA32_VMX_EPT_VPID_CAP` | 21 | A/D flags support |

---

## Split Operation Example

**Splitting GPA 0x1_0000_0000 (4GB) to 4KB:**

1. PML4 index = 0 (within first 512GB)
2. PDPT index = 4 (4GB / 1GB = 4)
3. PD index = 0 (within first 2MB of the 1GB region)

**Step 1**: EptSplit1GbTo2Mb
- Allocates page 2 for PD table
- PDPT[4] converted from 1GB page to PD pointer
- PD filled with 512 × 2MB entries

**Step 2**: EptSplit2MbTo4Kb
- Allocates page 3 for PT table
- PD[0] converted from 2MB page to PT pointer
- PT filled with 512 × 4KB entries

**Result**: 4 pages used (PML4, PDPT, PD, PT)

---

## CONCERNS

### Thread Safety
- No locking around split operations
- INVEPT after modification may race with other CPUs

### Memory Limits
- Fixed SplitPdTables array (512 entries)
- No dynamic PT tracking (only count)

### Error Recovery
- If split fails mid-way, EPT may be in inconsistent state
- No rollback mechanism

### MMIO Handling
- All memory mapped as WB
- MMIO regions (below 1MB, above RAM) should be UC

---

## GAPS AND UNKNOWNS

- [ ] How are MMIO regions handled with correct memory type?
- [ ] What prevents concurrent splits?
- [ ] How does EptModifyPage work after splitting?
- [ ] What's the maximum number of splits supported?
- [ ] Is there PT table tracking for granular permission modification?

---

*Implementation documentation generated 2025-12-27*
*CONFIDENCE: HIGH for structure, MEDIUM for thread safety*
