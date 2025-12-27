// ept.c — Extended Page Tables Implementation
// OmbraHypervisor

#include "ept.h"
#include "vmx.h"
#include "debug.h"
#include "../shared/msr_defs.h"
#include <intrin.h>

// =============================================================================
// EPT Capability Check
// =============================================================================

bool EptSupports1GbPages(void) {
    U64 eptCap = __readmsr(MSR_IA32_VMX_EPT_VPID_CAP);

    // Bit 17: EPT 1GB page support
    return (eptCap & (1ULL << 17)) != 0;
}

static bool EptSupportsAccessedDirty(void) {
    U64 eptCap = __readmsr(MSR_IA32_VMX_EPT_VPID_CAP);

    // Bit 21: Accessed and dirty flags support
    return (eptCap & (1ULL << 21)) != 0;
}

static bool EptSupportsWriteBack(void) {
    U64 eptCap = __readmsr(MSR_IA32_VMX_EPT_VPID_CAP);

    // Bit 14: Write-back memory type support
    return (eptCap & (1ULL << 14)) != 0;
}

// =============================================================================
// EPTP Construction
// =============================================================================

static U64 BuildEptp(U64 pml4Physical, bool accessedDirty) {
    U64 eptp = 0;

    // Memory type: Write-back (6) - bits 2:0
    eptp |= EPT_MEMORY_TYPE_WB;

    // Page-walk length minus 1 (4 levels - 1 = 3) - bits 5:3
    eptp |= (3ULL << 3);

    // Accessed/Dirty flags - bit 6
    if (accessedDirty && EptSupportsAccessedDirty()) {
        eptp |= (1ULL << 6);
    }

    // PML4 physical address (bits 51:12)
    eptp |= (pml4Physical & 0x000FFFFFFFFFF000ULL);

    return eptp;
}

// =============================================================================
// EPT Initialization - 512GB Identity Map with 1GB Pages
// =============================================================================

OMBRA_STATUS EptInitialize(
    EPT_STATE* ept,
    void* pml4Virtual,
    U64 pml4Physical,
    void* pdptVirtual,
    U64 pdptPhysical,
    U32 totalPagesAllocated
) {
    EPT_PML4E* pml4;
    EPT_PDPTE* pdpt;
    U32 i;

    INFO("EPT: Initializing identity map");
    TRACE("EPT: PML4 phys=0x%llx, PDPT phys=0x%llx", pml4Physical, pdptPhysical);

    if (!ept || !pml4Virtual || !pdptVirtual) {
        ERR("EPT: Invalid parameters");
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Validate total pages - must be at least 2 (PML4 + PDPT)
    if (totalPagesAllocated < 2) {
        ERR("EPT: totalPagesAllocated=%u too small (need >= 2)", totalPagesAllocated);
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Check 1GB page support
    if (!EptSupports1GbPages()) {
        // Fall back to 2MB pages would go here
        // For now, fail if 1GB not supported
        ERR("EPT: 1GB pages not supported");
        return OMBRA_ERROR_NOT_SUPPORTED;
    }
    TRACE("EPT: 1GB pages supported");

    // Initialize EPT state
    ept->Pml4 = (EPT_PML4E*)pml4Virtual;
    ept->Pml4Physical = pml4Physical;
    ept->Pdpt = (EPT_PDPTE*)pdptVirtual;
    ept->PdptPhysical = pdptPhysical;
    ept->HookCount = 0;
    ept->Initialized = false;

    // Initialize allocation tracking
    // Assume PML4 is at offset 0, PDPT at offset 4KB in contiguous memory pool
    ept->EptMemoryBase = pml4Virtual;
    ept->EptMemoryPhysical = pml4Physical;
    ept->TotalPagesAllocated = totalPagesAllocated;
    ept->PagesUsed = 2;  // PML4 (page 0) + PDPT (page 1)
    ept->SplitPdCount = 0;
    ept->SplitPtCount = 0;

    // Initialize split table arrays to NULL
    for (i = 0; i < EPT_ENTRIES_PER_TABLE; i++) {
        ept->SplitPdTables[i] = NULL;
        ept->SplitPtTables[i] = NULL;
    }

    TRACE("EPT: Memory region = %u pages (%u KB), %u pages available for splits",
          totalPagesAllocated, totalPagesAllocated * 4, totalPagesAllocated - 2);

    // Zero out tables
    pml4 = ept->Pml4;
    pdpt = ept->Pdpt;

    for (i = 0; i < EPT_ENTRIES_PER_TABLE; i++) {
        pml4[i].Value = 0;
        pdpt[i].Value = 0;
    }

    // =========================================================================
    // Setup PML4 Entry 0
    // =========================================================================
    // Points to PDPT, covers GPA 0x0 - 0x7F_FFFF_FFFF (512GB)

    pml4[0].Value = 0;
    pml4[0].Read = 1;
    pml4[0].Write = 1;
    pml4[0].Execute = 1;
    pml4[0].ExecuteUser = 1;  // Allow user-mode execute
    pml4[0].PdptPhysAddr = pdptPhysical >> 12;

    // =========================================================================
    // Setup PDPT Entries - 512 x 1GB Pages = 512GB Identity Map
    // =========================================================================
    // Each entry maps 1GB: GPA N*1GB -> HPA N*1GB

    for (i = 0; i < EPT_ENTRIES_PER_TABLE; i++) {
        U64 physAddr = (U64)i * EPT_PAGE_SIZE_1G;

        pdpt[i].Value = 0;

        // Set permissions (RWX for all pages)
        pdpt[i].LargePage.Read = 1;
        pdpt[i].LargePage.Write = 1;
        pdpt[i].LargePage.Execute = 1;
        pdpt[i].LargePage.ExecuteUser = 1;

        // Mark as large page (1GB)
        pdpt[i].LargePage.LargePage = 1;

        // Memory type: Write-back for most memory
        // Could use UC for MMIO regions if needed
        pdpt[i].LargePage.MemoryType = EPT_MEMORY_TYPE_WB;

        // Physical address (bits 51:30 of physical address)
        // For identity mapping, GPA == HPA
        pdpt[i].LargePage.PagePhysAddr = physAddr >> 30;
    }

    // =========================================================================
    // Build EPTP
    // =========================================================================

    ept->Eptp = BuildEptp(pml4Physical, true);
    ept->Initialized = true;

    INFO("EPT: 512GB identity map created (EPTP=0x%llx)", ept->Eptp);
    return OMBRA_SUCCESS;
}

// =============================================================================
// EPT Memory Pool Management
// =============================================================================

// Allocate a new 4KB page from the EPT memory pool
// Returns virtual and physical addresses via output parameters
static OMBRA_STATUS EptAllocatePage(
    EPT_STATE* ept,
    void** outVirtual,
    U64* outPhysical
) {
    U8* baseVirtual;
    U64 offset;

    if (!ept || !outVirtual || !outPhysical) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Check if we have space
    if (ept->PagesUsed >= ept->TotalPagesAllocated) {
        ERR("EPT: Out of memory (used=%u, total=%u)",
            ept->PagesUsed, ept->TotalPagesAllocated);
        return OMBRA_ERROR_NO_MEMORY;
    }

    // Calculate offset into contiguous pool
    baseVirtual = (U8*)ept->EptMemoryBase;
    offset = ept->PagesUsed * EPT_PAGE_SIZE_4K;

    *outVirtual = baseVirtual + offset;
    *outPhysical = ept->EptMemoryPhysical + offset;

    // Increment usage counter
    ept->PagesUsed++;

    TRACE("EPT: Allocated page %u at virt=%p phys=0x%llx",
          ept->PagesUsed - 1, *outVirtual, *outPhysical);

    return OMBRA_SUCCESS;
}

// Zero out a 4KB page
static void EptZeroPage(void* page) {
    U64* ptr = (U64*)page;
    U32 i;

    if (!page) {
        return;
    }

    // Zero 512 x 8-byte entries (4KB total)
    for (i = 0; i < EPT_ENTRIES_PER_TABLE; i++) {
        ptr[i] = 0;
    }
}

// =============================================================================
// EPT Page Splitting - 1GB to 2MB
// =============================================================================

OMBRA_STATUS EptSplit1GbTo2Mb(EPT_STATE* ept, U64 guestPhysical) {
    U32 pdptIndex;
    EPT_PDPTE* pdpte;
    EPT_PDE* pd;
    void* pdVirtual;
    U64 pdPhysical;
    U64 base1GbPhysical;
    U8 memoryType;
    U32 i;
    OMBRA_STATUS status;

    if (!ept || !ept->Initialized) {
        ERR("EPT: Invalid EPT state");
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Get PDPT index from GPA
    pdptIndex = EPT_PDPT_INDEX(guestPhysical);
    if (pdptIndex >= EPT_ENTRIES_PER_TABLE) {
        ERR("EPT: Invalid GPA 0x%llx (PDPT index %u out of range)",
            guestPhysical, pdptIndex);
        return OMBRA_ERROR_INVALID_PARAM;
    }

    pdpte = &ept->Pdpt[pdptIndex];

    // Check if it's actually a 1GB large page
    if (!pdpte->LargePage.LargePage) {
        // Already split or points to PD table
        TRACE("EPT: GPA 0x%llx already split (PDPT[%u] not a large page)",
              guestPhysical, pdptIndex);
        return OMBRA_SUCCESS;
    }

    // Check if we've already split this entry
    if (ept->SplitPdTables[pdptIndex] != NULL) {
        WARN("EPT: PDPT[%u] marked as large page but already has split PD table",
             pdptIndex);
        return OMBRA_SUCCESS;
    }

    INFO("EPT: Splitting 1GB page at PDPT[%u] (GPA 0x%llx) into 512 x 2MB pages",
         pdptIndex, guestPhysical);

    // Capture the 1GB page's properties before we overwrite the entry
    base1GbPhysical = (U64)pdpte->LargePage.PagePhysAddr << 30;
    memoryType = (U8)pdpte->LargePage.MemoryType;

    TRACE("EPT: 1GB page base phys=0x%llx, memory type=%u",
          base1GbPhysical, memoryType);

    // Allocate a new PD table (4KB = 512 entries)
    status = EptAllocatePage(ept, &pdVirtual, &pdPhysical);
    if (status != OMBRA_SUCCESS) {
        ERR("EPT: Failed to allocate PD table");
        return status;
    }

    // Zero the new PD table
    EptZeroPage(pdVirtual);

    pd = (EPT_PDE*)pdVirtual;

    // Fill PD with 512 x 2MB large page entries
    // Each 2MB page inherits permissions and memory type from the 1GB page
    for (i = 0; i < EPT_ENTRIES_PER_TABLE; i++) {
        U64 page2MbPhysical = base1GbPhysical + ((U64)i * EPT_PAGE_SIZE_2M);

        pd[i].Value = 0;

        // Set permissions (copy from original 1GB page)
        pd[i].LargePage.Read = pdpte->LargePage.Read;
        pd[i].LargePage.Write = pdpte->LargePage.Write;
        pd[i].LargePage.Execute = pdpte->LargePage.Execute;
        pd[i].LargePage.ExecuteUser = pdpte->LargePage.ExecuteUser;

        // Mark as 2MB large page
        pd[i].LargePage.LargePage = 1;

        // Set memory type
        pd[i].LargePage.MemoryType = memoryType;

        // Set physical address (bits 51:21)
        pd[i].LargePage.PagePhysAddr = page2MbPhysical >> 21;
    }

    TRACE("EPT: Populated PD with 512 x 2MB entries (0x%llx - 0x%llx)",
          base1GbPhysical, base1GbPhysical + EPT_PAGE_SIZE_1G - 1);

    // Convert PDPTE from 1GB large page to PD pointer
    pdpte->Value = 0;
    pdpte->Pointer.Read = 1;
    pdpte->Pointer.Write = 1;
    pdpte->Pointer.Execute = 1;
    pdpte->Pointer.ExecuteUser = 1;
    pdpte->Pointer.PdPhysAddr = pdPhysical >> 12;

    // Track the split PD table
    ept->SplitPdTables[pdptIndex] = pdVirtual;
    ept->SplitPdCount++;

    INFO("EPT: Split complete - PDPT[%u] now points to PD at phys=0x%llx (split count=%u)",
         pdptIndex, pdPhysical, ept->SplitPdCount);

    // Invalidate EPT to ensure changes take effect
    EptInvalidate(ept, INVEPT_TYPE_SINGLE_CONTEXT);

    return OMBRA_SUCCESS;
}

// =============================================================================
// EPT Destruction
// =============================================================================

void EptDestroy(EPT_STATE* ept) {
    if (ept) {
        ept->Initialized = false;
        // Memory is caller-managed, just clear pointers
        ept->Pml4 = NULL;
        ept->Pdpt = NULL;
        ept->Eptp = 0;
    }
}

// =============================================================================
// Get EPTP
// =============================================================================

U64 EptGetEptp(EPT_STATE* ept) {
    if (!ept || !ept->Initialized) {
        return 0;
    }
    return ept->Eptp;
}

// =============================================================================
// EPT Page Modification (for hooks)
// =============================================================================

OMBRA_STATUS EptModifyPage(
    EPT_STATE* ept,
    U64 guestPhysical,
    U64 hostPhysical,
    U64 permissions,
    U64 memoryType
) {
    U32 pdptIndex;
    EPT_PDPTE* entry;

    if (!ept || !ept->Initialized) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // For 1GB pages, we can only modify whole 1GB regions
    // Fine-grained modification requires splitting first (Phase 6)

    pdptIndex = EPT_PDPT_INDEX(guestPhysical);
    if (pdptIndex >= EPT_ENTRIES_PER_TABLE) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    entry = &ept->Pdpt[pdptIndex];

    // Modify the entry
    entry->LargePage.Read = (permissions & EPT_READ) ? 1 : 0;
    entry->LargePage.Write = (permissions & EPT_WRITE) ? 1 : 0;
    entry->LargePage.Execute = (permissions & EPT_EXECUTE) ? 1 : 0;
    entry->LargePage.MemoryType = memoryType & 7;
    entry->LargePage.PagePhysAddr = hostPhysical >> 30;

    // Invalidate EPT to ensure changes take effect
    EptInvalidate(ept, INVEPT_TYPE_SINGLE_CONTEXT);

    return OMBRA_SUCCESS;
}

// =============================================================================
// EPT Page Splitting - 2MB to 4KB
// =============================================================================

OMBRA_STATUS EptSplit2MbTo4Kb(EPT_STATE* ept, U64 guestPhysical) {
    U32 pml4Index, pdptIndex, pdIndex;
    EPT_PML4E* pml4e;
    EPT_PDPTE* pdpte;
    EPT_PDE* pde;
    EPT_PDE* pd;
    EPT_PTE* pt;
    void* ptVirtual;
    U64 ptPhysical;
    U64 base2MbPhysical;
    U8 memoryType;
    U32 i;
    OMBRA_STATUS status;

    if (!ept || !ept->Initialized) {
        ERR("EPT: Invalid EPT state");
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Get indices from GPA
    pml4Index = EPT_PML4_INDEX(guestPhysical);
    pdptIndex = EPT_PDPT_INDEX(guestPhysical);
    pdIndex = EPT_PD_INDEX(guestPhysical);

    TRACE("EPT: Splitting 2MB page at GPA 0x%llx (PML4[%u] PDPT[%u] PD[%u])",
          guestPhysical, pml4Index, pdptIndex, pdIndex);

    // Walk EPT to find the PDE
    if (pml4Index >= EPT_ENTRIES_PER_TABLE) {
        ERR("EPT: Invalid PML4 index %u", pml4Index);
        return OMBRA_ERROR_INVALID_PARAM;
    }

    pml4e = &ept->Pml4[pml4Index];
    if (!pml4e->Read) {
        ERR("EPT: PML4E[%u] not present", pml4Index);
        return OMBRA_ERROR_NOT_FOUND;
    }

    if (pdptIndex >= EPT_ENTRIES_PER_TABLE) {
        ERR("EPT: Invalid PDPT index %u", pdptIndex);
        return OMBRA_ERROR_INVALID_PARAM;
    }

    pdpte = &ept->Pdpt[pdptIndex];
    if (!pdpte->Pointer.Read) {
        ERR("EPT: PDPTE[%u] not present", pdptIndex);
        return OMBRA_ERROR_NOT_FOUND;
    }

    // Check if this is a 1GB page that needs splitting first
    if (pdpte->LargePage.LargePage) {
        ERR("EPT: Cannot split 2MB - PDPTE[%u] is 1GB page, split to 2MB first", pdptIndex);
        return OMBRA_ERROR_INVALID_STATE;
    }

    // Get the PD table (must have been split from 1GB already)
    pd = (EPT_PDE*)ept->SplitPdTables[pdptIndex];
    if (!pd) {
        ERR("EPT: PD table for PDPTE[%u] not found in split tracking", pdptIndex);
        return OMBRA_ERROR_INVALID_STATE;
    }

    if (pdIndex >= EPT_ENTRIES_PER_TABLE) {
        ERR("EPT: Invalid PD index %u", pdIndex);
        return OMBRA_ERROR_INVALID_PARAM;
    }

    pde = &pd[pdIndex];
    if (!pde->LargePage.Read) {
        ERR("EPT: PDE[%u] not present", pdIndex);
        return OMBRA_ERROR_NOT_FOUND;
    }

    // Check if already a 4KB page
    if (!pde->LargePage.LargePage) {
        TRACE("EPT: PDE[%u] is already split to 4KB pages", pdIndex);
        return OMBRA_SUCCESS;  // Already done
    }

    INFO("EPT: Splitting 2MB page at PD[%u] (GPA 0x%llx) into 512 x 4KB pages",
         pdIndex, guestPhysical);

    // Capture the 2MB page's properties before we overwrite the entry
    base2MbPhysical = (U64)pde->LargePage.PagePhysAddr << 21;
    memoryType = (U8)pde->LargePage.MemoryType;

    TRACE("EPT: 2MB page base phys=0x%llx, memory type=%u",
          base2MbPhysical, memoryType);

    // Allocate PT for 4KB pages
    status = EptAllocatePage(ept, &ptVirtual, &ptPhysical);
    if (status != OMBRA_SUCCESS) {
        ERR("EPT: Failed to allocate PT for 2MB split");
        return status;
    }

    // Zero the new PT table
    EptZeroPage(ptVirtual);

    pt = (EPT_PTE*)ptVirtual;

    // Fill PT with 512 × 4KB entries
    // Each 4KB page inherits permissions and memory type from the 2MB page
    for (i = 0; i < EPT_ENTRIES_PER_TABLE; i++) {
        U64 page4KbPhysical = base2MbPhysical + ((U64)i * EPT_PAGE_SIZE_4K);

        pt[i].Value = 0;

        // Set permissions (copy from original 2MB page)
        pt[i].Read = pde->LargePage.Read;
        pt[i].Write = pde->LargePage.Write;
        pt[i].Execute = pde->LargePage.Execute;
        pt[i].ExecuteUser = pde->LargePage.ExecuteUser;

        // Set memory type
        pt[i].MemoryType = memoryType;

        // Set physical address (bits 51:12)
        pt[i].PagePhysAddr = page4KbPhysical >> 12;
    }

    TRACE("EPT: Populated PT with 512 x 4KB entries (0x%llx - 0x%llx)",
          base2MbPhysical, base2MbPhysical + EPT_PAGE_SIZE_2M - 1);

    // Convert PDE from large page to PT pointer
    pde->Value = 0;
    pde->Pointer.Read = 1;
    pde->Pointer.Write = 1;
    pde->Pointer.Execute = 1;
    pde->Pointer.ExecuteUser = 1;
    pde->Pointer.PtPhysAddr = ptPhysical >> 12;
    // Note: LargePage bit is now 0 (not a large page anymore)

    ept->SplitPtCount++;

    INFO("EPT: Split complete - PD[%u] now points to PT at phys=0x%llx (split count=%u)",
         pdIndex, ptPhysical, ept->SplitPtCount);

    // Invalidate EPT to ensure changes take effect
    EptInvalidate(ept, INVEPT_TYPE_SINGLE_CONTEXT);

    return OMBRA_SUCCESS;
}

// =============================================================================
// Generic EPT Large Page Split
// =============================================================================

OMBRA_STATUS EptSplitLargePage(EPT_STATE* ept, U64 guestPhysical) {
    U32 pml4Index, pdptIndex;
    EPT_PDPTE* pdpte;
    OMBRA_STATUS status;

    if (!ept || !ept->Initialized) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    pml4Index = EPT_PML4_INDEX(guestPhysical);
    pdptIndex = EPT_PDPT_INDEX(guestPhysical);

    // Currently only PML4 entry 0 is valid (512GB)
    if (pml4Index != 0) {
        ERR("EPT: GPA 0x%llx outside mapped range", guestPhysical);
        return OMBRA_ERROR_INVALID_PARAM;
    }

    if (pdptIndex >= EPT_ENTRIES_PER_TABLE) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    pdpte = &ept->Pdpt[pdptIndex];

    // Determine what kind of page we're splitting
    if (pdpte->LargePage.LargePage) {
        // 1GB page - split to 2MB first, then to 4KB
        INFO("EPT: Splitting 1GB page at GPA 0x%llx to 2MB, then to 4KB", guestPhysical);

        status = EptSplit1GbTo2Mb(ept, guestPhysical);
        if (OMBRA_FAILED(status)) {
            return status;
        }

        // After splitting to 2MB, split the specific 2MB page to 4KB
        return EptSplit2MbTo4Kb(ept, guestPhysical);
    } else {
        // Already split to 2MB (or smaller), split the specific 2MB page to 4KB
        return EptSplit2MbTo4Kb(ept, guestPhysical);
    }
}

// =============================================================================
// Get EPT Entry
// =============================================================================

U64* EptGetEntry(EPT_STATE* ept, U64 guestPhysical) {
    U32 pml4Index, pdptIndex;

    if (!ept || !ept->Initialized) {
        return NULL;
    }

    pml4Index = EPT_PML4_INDEX(guestPhysical);
    pdptIndex = EPT_PDPT_INDEX(guestPhysical);

    // Currently only PML4 entry 0 is valid (512GB)
    if (pml4Index != 0) {
        return NULL;
    }

    if (pdptIndex >= EPT_ENTRIES_PER_TABLE) {
        return NULL;
    }

    return &ept->Pdpt[pdptIndex].Value;
}

// =============================================================================
// EPT Invalidation
// =============================================================================

void EptInvalidate(EPT_STATE* ept, U64 inveptType) {
    INVEPT_DESCRIPTOR desc;

    if (!ept) {
        return;
    }

    desc.EptPointer = ept->Eptp;
    desc.Reserved = 0;

    // Use the assembly INVEPT wrapper
    AsmInvept(inveptType, &desc);
}
