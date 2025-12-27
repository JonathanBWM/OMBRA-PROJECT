# EPT Page Splitting - Quick Reference

## API Functions

### High-Level (Use This)

```c
OMBRA_STATUS EptSplitLargePage(EPT_STATE* ept, U64 guestPhysical);
```

**What it does**: Automatically splits to 4KB granularity
- If GPA is in 1GB page → splits to 2MB, then to 4KB
- If GPA is in 2MB page → splits to 4KB
- Idempotent (safe to call multiple times)

**Example**:
```c
// Hook page at 0x12345000
status = EptSplitLargePage(&ept, 0x12345000);
if (status == OMBRA_SUCCESS) {
    // Page is now split to 4KB, ready for hooking
}
```

---

### Low-Level (Advanced)

```c
OMBRA_STATUS EptSplit1GbTo2Mb(EPT_STATE* ept, U64 guestPhysical);
```

**What it does**: Splits one 1GB PDPT entry into 512 x 2MB PD entries
- Allocates 1 page (4KB) for PD table
- Preserves identity mapping (GPA N → HPA N)
- Returns SUCCESS if already split (idempotent)

**When to use**: Fine-grained control over 2MB regions without full 4KB split

```c
OMBRA_STATUS EptSplit2MbTo4Kb(EPT_STATE* ept, U64 guestPhysical);
```

**What it does**: Splits one 2MB PD entry into 512 x 4KB PT entries
- Requires parent 1GB page already split to 2MB
- Allocates 1 page (4KB) for PT table
- Returns `OMBRA_ERROR_INVALID_STATE` if 1GB page not split yet

---

## Return Codes

| Code | Meaning |
|------|---------|
| `OMBRA_SUCCESS` | Operation succeeded (or already split) |
| `OMBRA_ERROR_INVALID_PARAM` | NULL `ept`, invalid GPA |
| `OMBRA_ERROR_NO_MEMORY` | EPT memory pool exhausted |
| `OMBRA_ERROR_INVALID_STATE` | Tried to split 2MB before splitting 1GB |
| `OMBRA_ERROR_NOT_FOUND` | EPT entry not present |

---

## Memory Requirements

```
Pages Needed = 2 (baseline) + SplitPdCount + SplitPtCount
```

| Workload | Pool Size | Pages | Memory |
|----------|-----------|-------|--------|
| Minimal (no splits) | 2 | 2 | 8 KB |
| 1 hook (4KB) | 4 | 4 | 16 KB |
| 10 hooks (same 1GB) | 13 | 13 | 52 KB |
| 10 hooks (different 1GB) | 22 | 22 | 88 KB |
| **Recommended** | **512** | **512** | **2 MB** |

---

## EPT State Access

### Check Split Status

```c
// Is PDPT[0] split to 2MB?
if (ept.SplitPdTables[0] != NULL) {
    EPT_PDE* pd = (EPT_PDE*)ept.SplitPdTables[0];
    // Access 512 x 2MB entries
}

// How many splits?
printf("1GB splits: %u\n", ept.SplitPdCount);
printf("2MB splits: %u\n", ept.SplitPtCount);
printf("Memory usage: %u/%u pages\n", ept.PagesUsed, ept.TotalPagesAllocated);
```

---

## Common Patterns

### Pattern 1: Hook Single Page

```c
// Split to 4KB and modify
status = EptSplitLargePage(&ept, hookAddress);
if (status == OMBRA_SUCCESS) {
    // TODO: Walk EPT to get PTE and modify permissions
}
```

### Pattern 2: Batch Split for Multiple Hooks

```c
U64 hooks[] = {0x12345000, 0x23456000, 0x34567000};
for (int i = 0; i < 3; i++) {
    status = EptSplitLargePage(&ept, hooks[i]);
    if (status != OMBRA_SUCCESS) {
        ERR("Failed to split hook %d: %d", i, status);
        break;
    }
}
```

### Pattern 3: Pre-Split Entire 1GB Region

```c
// Split first 1GB to 2MB granularity (for many hooks in same region)
status = EptSplit1GbTo2Mb(&ept, 0x0);

// Now split individual 2MB pages as needed
for (int i = 0; i < numHooks; i++) {
    status = EptSplit2MbTo4Kb(&ept, hookAddresses[i]);
}
```

### Pattern 4: Check Before Split

```c
U32 pdptIndex = EPT_PDPT_INDEX(gpa);
if (ept.SplitPdTables[pdptIndex] == NULL) {
    // Not yet split to 2MB
    status = EptSplit1GbTo2Mb(&ept, gpa);
}
// Now split to 4KB
status = EptSplit2MbTo4Kb(&ept, gpa);
```

---

## Index Extraction Macros

```c
#define EPT_PML4_INDEX(gpa)  (((gpa) >> 39) & 0x1FF)  // Bits 47:39
#define EPT_PDPT_INDEX(gpa)  (((gpa) >> 30) & 0x1FF)  // Bits 38:30
#define EPT_PD_INDEX(gpa)    (((gpa) >> 21) & 0x1FF)  // Bits 29:21
#define EPT_PT_INDEX(gpa)    (((gpa) >> 12) & 0x1FF)  // Bits 20:12
```

**Example**:
```c
U64 gpa = 0x123456000;  // Some guest physical address

U32 pdptIdx = EPT_PDPT_INDEX(gpa);  // Which 1GB region?
U32 pdIdx = EPT_PD_INDEX(gpa);      // Which 2MB region within that 1GB?
U32 ptIdx = EPT_PT_INDEX(gpa);      // Which 4KB page within that 2MB?

printf("GPA 0x%llx → PDPT[%u] PD[%u] PT[%u]\n", gpa, pdptIdx, pdIdx, ptIdx);
```

---

## Debugging

### Check Split State

```c
void EptDumpSplitState(EPT_STATE* ept) {
    printf("EPT Split State:\n");
    printf("  Memory: %u/%u pages used\n", ept->PagesUsed, ept->TotalPagesAllocated);
    printf("  1GB→2MB splits: %u\n", ept->SplitPdCount);
    printf("  2MB→4KB splits: %u\n", ept->SplitPtCount);

    printf("  Split PDPTs:\n");
    for (int i = 0; i < 512; i++) {
        if (ept->SplitPdTables[i]) {
            printf("    PDPT[%d] → PD at %p\n", i, ept->SplitPdTables[i]);
        }
    }
}
```

### Verify Entry After Split

```c
status = EptSplit1GbTo2Mb(&ept, 0x0);

// Verify PDPTE now points to PD
EPT_PDPTE* pdpte = &ept.Pdpt[0];
if (pdpte->LargePage.LargePage) {
    ERR("Split failed - still a large page!");
} else {
    U64 pdPhys = (U64)pdpte->Pointer.PdPhysAddr << 12;
    printf("PDPTE[0] now points to PD at phys 0x%llx\n", pdPhys);
}

// Verify PD contents
EPT_PDE* pd = (EPT_PDE*)ept.SplitPdTables[0];
for (int i = 0; i < 3; i++) {
    U64 phys = (U64)pd[i].LargePage.PagePhysAddr << 21;
    printf("PD[%d]: phys=0x%llx, large=%u, R/W/X=%u/%u/%u\n",
           i, phys,
           pd[i].LargePage.LargePage,
           pd[i].LargePage.Read,
           pd[i].LargePage.Write,
           pd[i].LargePage.Execute);
}
```

---

## Performance Tips

1. **Split on demand**: Don't split everything upfront, only split pages you're hooking
2. **Batch splits**: If hooking multiple pages in same 1GB region, split to 2MB once, then split individual 2MB pages
3. **Pool size**: 512 pages (2MB) handles most workloads. Increase if installing 500+ hooks
4. **INVEPT cost**: Each split triggers INVEPT. For many splits, consider batching modifications then single INVEPT

---

## Common Errors

### "Out of memory"

```
[ERROR] EPT: Out of memory (used=512, total=512)
```

**Solution**: Increase pool size in `EptInitialize()`:
```c
EptInitialize(&ept, ..., 1024);  // 4MB pool instead of 2MB
```

### "Cannot split 2MB - PDPTE is 1GB page"

```
[ERROR] EPT: Cannot split 2MB - PDPTE[0] is 1GB page, split to 2MB first
```

**Solution**: Use `EptSplitLargePage()` instead of `EptSplit2MbTo4Kb()` directly:
```c
// DON'T: EptSplit2MbTo4Kb(&ept, gpa);
// DO:
EptSplitLargePage(&ept, gpa);  // Handles 1GB→2MB→4KB automatically
```

### "PD table not found in split tracking"

```
[ERROR] EPT: PD table for PDPTE[1] not found in split tracking
```

**Solution**: The PDPTE was modified outside of split functions. Ensure all EPT modifications go through the API.

---

## Files

| File | Purpose |
|------|---------|
| `hypervisor/ept.h` | EPT API declarations, `EPT_STATE` structure |
| `hypervisor/ept.c` | EPT implementation, split functions |
| `shared/ept_defs.h` | EPT entry structures, macros, constants |
| `test_ept_split.c` | Test harness for split operations |
| `EPT_SPLITTING_GUIDE.md` | Detailed implementation guide |
| `EPT_SPLITTING_COMPLETE.md` | Complete feature documentation |

---

## Next Steps (Hook Framework Integration)

After splitting, you'll need to:

1. **Walk EPT to get PTE**:
   ```c
   EPT_PTE* EptWalkTo4KbPage(EPT_STATE* ept, U64 gpa) {
       // TODO: Implementation needed
       // Returns pointer to the PTE for the 4KB page containing gpa
   }
   ```

2. **Modify page permissions**:
   ```c
   EPT_PTE* pte = EptWalkTo4KbPage(&ept, hookAddr);
   pte->Execute = 1;
   pte->Read = 0;      // Execute-only page
   pte->Write = 0;
   pte->PagePhysAddr = shadowPage >> 12;  // Redirect to shadow page
   EptInvalidate(&ept, INVEPT_TYPE_SINGLE_CONTEXT);
   ```

3. **Handle EPT violations**:
   ```c
   void HandleEptViolation(VM_EXIT_CONTEXT* ctx) {
       U64 gpa = VmRead(GUEST_PHYSICAL_ADDRESS);
       U64 qual = ctx->ExitQualification;

       if (qual & EPT_VIOLATION_READ) {
           // Temporarily grant read access, swap to real page
       }
       // Resume guest
   }
   ```

---

## Summary

**Most common usage**:
```c
// Initialize with 2MB pool (512 pages)
EptInitialize(&ept, pml4, pml4Phys, pdpt, pdptPhys, 512);

// Split to 4KB for hooking
status = EptSplitLargePage(&ept, hookAddress);

// Check result
if (status == OMBRA_SUCCESS) {
    // Page is now 4KB, ready for fine-grained control
    // TODO: Walk EPT and modify PTE
}
```

That's it. Three lines to split any page from 1GB → 4KB granularity.
