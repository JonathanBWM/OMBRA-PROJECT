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
    U64 pdptPhysical
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
// EPT Large Page Split (Stub for Phase 6)
// =============================================================================

OMBRA_STATUS EptSplitLargePage(EPT_STATE* ept, U64 guestPhysical) {
    (void)ept;
    (void)guestPhysical;

    // This would split a 1GB page into 512 x 2MB pages
    // or a 2MB page into 512 x 4KB pages
    // Required for fine-grained EPT hooks

    return OMBRA_ERROR_NOT_IMPLEMENTED;
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
