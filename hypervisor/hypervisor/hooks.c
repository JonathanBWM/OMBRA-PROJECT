// hooks.c — EPT Hook Framework Implementation
// OmbraHypervisor

#include "hooks.h"
#include "ept.h"
#include "vmx.h"
#include "debug.h"
#include "../shared/ept_defs.h"

// =============================================================================
// Internal Helpers
// =============================================================================

static void ZeroMemory(void* ptr, U64 size) {
    U8* p = (U8*)ptr;
    while (size--) *p++ = 0;
}

static void CopyMemory(void* dst, const void* src, U64 size) {
    U8* d = (U8*)dst;
    const U8* s = (const U8*)src;
    while (size--) *d++ = *s++;
}

// =============================================================================
// Hook Manager Initialization
// =============================================================================

OMBRA_STATUS HookManagerInit(HOOK_MANAGER* mgr, EPT_STATE* ept) {
    if (!mgr || !ept) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    INFO("Initializing hook manager");

    ZeroMemory(mgr, sizeof(HOOK_MANAGER));
    mgr->Ept = ept;
    mgr->Initialized = true;

    // Note: Shadow page pool must be allocated by caller before hooks can work
    // This requires contiguous physical memory from the driver

    INFO("Hook manager initialized (max hooks=%u)", MAX_HOOKS);
    return OMBRA_SUCCESS;
}

void HookManagerShutdown(HOOK_MANAGER* mgr) {
    if (!mgr) return;

    INFO("Shutting down hook manager (active hooks=%u)", mgr->HookCount);

    // Remove all active hooks
    for (U32 i = 0; i < MAX_HOOKS; i++) {
        if (mgr->Hooks[i].Active) {
            HookRemove(mgr, &mgr->Hooks[i]);
        }
    }

    mgr->Initialized = false;
}

// =============================================================================
// EPT Page Splitting - 1GB to 2MB
// =============================================================================
//
// Splits a 1GB large page (PDPTE) into 512 x 2MB pages (PD entries).
// This is required before we can do fine-grained 4KB hooks.

OMBRA_STATUS EptSplit1GbTo2Mb(EPT_STATE* ept, U64 guestPhysical) {
    U32 pdptIndex;
    EPT_PDPTE* pdpte;
    EPT_PDE* pd;
    U64 baseAddr;
    U32 i;

    if (!ept || !ept->Initialized) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Get PDPT index for this address
    pdptIndex = EPT_PDPT_INDEX(guestPhysical);
    if (pdptIndex >= EPT_ENTRIES_PER_TABLE) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    pdpte = &ept->Pdpt[pdptIndex];

    // Check if it's a 1GB page
    if (!pdpte->LargePage.LargePage) {
        TRACE("PDPTE %u is not a large page, already split?", pdptIndex);
        return OMBRA_SUCCESS;  // Already split
    }

    // We need a new page for the PD table
    // In a real implementation, this would allocate from the shadow pool
    // For now, we'll just use a pre-allocated area from EptTables

    // TODO: Allocate PD table from pool
    // For now, we store split PDs after the PDPT
    // ept->EptTables has EPT_TABLES_PAGES (512) pages
    // PML4 = page 0, PDPT = page 1, PDs = pages 2+

    if (ept->SplitPdCount >= 510) {  // Leave room for PML4 + PDPT
        ERR("No more space for PD tables");
        return OMBRA_ERROR_NO_MEMORY;
    }

    // Calculate address for new PD (after PDPT)
    U64 pdOffset = (2 + ept->SplitPdCount) * 4096;
    pd = (EPT_PDE*)((U8*)ept->Pml4 + pdOffset);
    U64 pdPhysical = ept->Pml4Physical + pdOffset;

    INFO("Splitting 1GB page at PDPT[%u] into 2MB pages (PD at 0x%llx)",
         pdptIndex, pdPhysical);

    // Get base physical address of the 1GB page
    baseAddr = pdpte->LargePage.PagePhysAddr << 30;

    // Create 512 x 2MB entries
    for (i = 0; i < EPT_ENTRIES_PER_TABLE; i++) {
        U64 physAddr = baseAddr + (i * EPT_PAGE_SIZE_2M);

        pd[i].Value = 0;
        pd[i].LargePage.Read = 1;
        pd[i].LargePage.Write = 1;
        pd[i].LargePage.Execute = 1;
        pd[i].LargePage.ExecuteUser = 1;
        pd[i].LargePage.LargePage = 1;  // 2MB large page
        pd[i].LargePage.MemoryType = EPT_MEMORY_TYPE_WB;
        pd[i].LargePage.PagePhysAddr = physAddr >> 21;  // bits 51:21
    }

    // Update PDPTE to point to new PD (no longer a large page)
    pdpte->Value = 0;
    pdpte->Pointer.Read = 1;
    pdpte->Pointer.Write = 1;
    pdpte->Pointer.Execute = 1;
    pdpte->Pointer.ExecuteUser = 1;
    pdpte->Pointer.PdPhysAddr = pdPhysical >> 12;

    ept->SplitPdCount++;

    // Invalidate EPT
    EptInvalidate(ept, INVEPT_TYPE_SINGLE_CONTEXT);

    TRACE("Split complete, now have %u split 1GB pages", ept->SplitPdCount);
    return OMBRA_SUCCESS;
}

// =============================================================================
// EPT Page Splitting - 2MB to 4KB
// =============================================================================
//
// Splits a 2MB large page (PDE) into 512 x 4KB pages (PT entries).
// This is required for per-page hook granularity.

OMBRA_STATUS EptSplit2MbTo4Kb(EPT_STATE* ept, U64 guestPhysical) {
    U32 pdptIndex, pdIndex;
    EPT_PDPTE* pdpte;
    EPT_PDE* pde;
    EPT_PTE* pt;
    U64 baseAddr;
    U32 i;

    if (!ept || !ept->Initialized) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // First ensure the 1GB page is split
    OMBRA_STATUS status = EptSplit1GbTo2Mb(ept, guestPhysical);
    if (OMBRA_FAILED(status)) {
        return status;
    }

    // Get indices
    pdptIndex = EPT_PDPT_INDEX(guestPhysical);
    pdIndex = EPT_PD_INDEX(guestPhysical);

    // Get PDPTE (now should point to a PD)
    pdpte = &ept->Pdpt[pdptIndex];
    if (pdpte->LargePage.LargePage) {
        ERR("PDPTE is still a large page after split?");
        return OMBRA_ERROR_INVALID_STATE;
    }

    // Get PD address
    U64 pdPhysical = pdpte->Directory.PdPhysAddr << 12;
    // Convert physical to virtual (assuming identity mapping for now)
    EPT_PDE* pd = (EPT_PDE*)(pdPhysical);  // Simplified - need proper VA translation

    // Actually, we stored PDs in our EptTables region, so we can calculate
    // This is hacky - proper implementation would track allocations
    U64 pdOffset = pdPhysical - ept->Pml4Physical;
    pd = (EPT_PDE*)((U8*)ept->Pml4 + pdOffset);

    pde = &pd[pdIndex];

    // Check if it's a 2MB page
    if (!pde->LargePage.LargePage) {
        TRACE("PDE is not a large page, already split");
        return OMBRA_SUCCESS;  // Already split
    }

    // Allocate page table
    // Similar to above, use pre-allocated space
    // For a real implementation, we'd have a proper allocator

    // Store PTs after PDs (rough estimate: 512 possible PDs, then PTs)
    // This is simplified - real implementation needs proper memory management
    if (ept->SplitPtCount >= 256) {
        ERR("No more space for PT tables");
        return OMBRA_ERROR_NO_MEMORY;
    }

    U64 ptOffset = (2 + 510 + ept->SplitPtCount) * 4096;  // After PML4, PDPT, max PDs
    pt = (EPT_PTE*)((U8*)ept->Pml4 + ptOffset);
    U64 ptPhysical = ept->Pml4Physical + ptOffset;

    INFO("Splitting 2MB page at PD[%u][%u] into 4KB pages (PT at 0x%llx)",
         pdptIndex, pdIndex, ptPhysical);

    // Get base physical address of the 2MB page
    baseAddr = pde->LargePage.PagePhysAddr << 21;

    // Create 512 x 4KB entries
    for (i = 0; i < EPT_ENTRIES_PER_TABLE; i++) {
        U64 physAddr = baseAddr + (i * EPT_PAGE_SIZE_4K);

        pt[i].Value = 0;
        pt[i].Read = 1;
        pt[i].Write = 1;
        pt[i].Execute = 1;
        pt[i].ExecuteUser = 1;
        pt[i].MemoryType = EPT_MEMORY_TYPE_WB;
        pt[i].PagePhysAddr = physAddr >> 12;
    }

    // Update PDE to point to new PT (no longer a large page)
    pde->Value = 0;
    pde->Pointer.Read = 1;
    pde->Pointer.Write = 1;
    pde->Pointer.Execute = 1;
    pde->Pointer.ExecuteUser = 1;
    pde->Pointer.PtPhysAddr = ptPhysical >> 12;

    ept->SplitPtCount++;

    // Invalidate EPT
    EptInvalidate(ept, INVEPT_TYPE_SINGLE_CONTEXT);

    TRACE("Split complete, now have %u split 2MB pages", ept->SplitPtCount);
    return OMBRA_SUCCESS;
}

// =============================================================================
// Hook Installation - EPT Execute-Only
// =============================================================================

OMBRA_STATUS HookInstallEpt(
    HOOK_MANAGER* mgr,
    U64 targetVirtual,
    U64 targetPhysical,
    void* handlerAddress,
    EPT_HOOK** outHook
) {
    EPT_HOOK* hook = NULL;
    U32 i;

    if (!mgr || !mgr->Initialized || !handlerAddress) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    INFO("Installing EPT hook at VA=0x%llx PA=0x%llx", targetVirtual, targetPhysical);

    // Find free hook slot
    for (i = 0; i < MAX_HOOKS; i++) {
        if (!mgr->Hooks[i].Active) {
            hook = &mgr->Hooks[i];
            break;
        }
    }

    if (!hook) {
        ERR("No free hook slots available");
        return OMBRA_ERROR_NO_MEMORY;
    }

    // Split EPT pages to get 4KB granularity
    OMBRA_STATUS status = EptSplit2MbTo4Kb(mgr->Ept, targetPhysical);
    if (OMBRA_FAILED(status)) {
        ERR("Failed to split EPT pages for hook");
        return status;
    }

    // Allocate shadow page
    U64 shadowPhysical;
    void* shadowVirtual = HookAllocateShadowPage(mgr, &shadowPhysical);
    if (!shadowVirtual) {
        ERR("Failed to allocate shadow page");
        return OMBRA_ERROR_NO_MEMORY;
    }

    // Copy original page content to shadow page
    void* targetPage = (void*)(targetPhysical);  // Simplified VA translation
    CopyMemory(shadowVirtual, targetPage, 4096);

    // Install jump to handler at target offset in shadow page
    U32 offset = targetVirtual & 0xFFF;
    U8* hookPoint = (U8*)shadowVirtual + offset;

    // Save original bytes
    CopyMemory(hook->OriginalBytes, hookPoint, 16);

    // Write JMP to handler (64-bit absolute jump)
    // FF 25 00 00 00 00 [8-byte address]
    hookPoint[0] = 0xFF;
    hookPoint[1] = 0x25;
    hookPoint[2] = 0x00;
    hookPoint[3] = 0x00;
    hookPoint[4] = 0x00;
    hookPoint[5] = 0x00;
    *(U64*)(hookPoint + 6) = (U64)handlerAddress;
    hook->OriginalLength = 14;  // Size of our JMP

    // Setup hook structure
    hook->Magic = HOOK_MAGIC;
    hook->TargetVirtual = targetVirtual;
    hook->TargetPhysical = targetPhysical;
    hook->TargetOffset = offset;
    hook->ShadowPhysical = shadowPhysical;
    hook->ShadowVirtual = shadowVirtual;
    hook->HandlerAddress = handlerAddress;
    hook->Type = HOOK_TYPE_EPT_EXECUTE;
    hook->Active = true;
    hook->HitCount = 0;

    // Modify EPT entry for the target page
    // Execute: shadow page (with hook)
    // Read/Write: original page
    //
    // For execute-only hooks:
    // 1. Set original page to Read+Write only (no Execute)
    // 2. On execute, EPT violation triggers, we swap to shadow page
    //
    // Alternative: use mode-based execute control if available

    // Get EPT PTE for target page
    U64* pte = EptGetEntry(mgr->Ept, targetPhysical);
    if (!pte) {
        ERR("Failed to get EPT entry for target");
        hook->Active = false;
        return OMBRA_ERROR_NOT_FOUND;
    }

    // Store original PTE value
    // Then set to Execute-Only (R=0, W=0, X=1)
    // This will cause EPT violation on read/write, allowing us to show original

    // Actually, simpler approach for stealth:
    // - Execute-only for shadow page
    // - On read fault, temporarily make readable, single-step, restore

    // For now, just make the page point to shadow for all access
    // Full stealth implementation would use MTF (monitor trap flag)

    EPT_PTE* entry = (EPT_PTE*)pte;
    entry->PagePhysAddr = shadowPhysical >> 12;

    // Invalidate EPT
    EptInvalidate(mgr->Ept, INVEPT_TYPE_SINGLE_CONTEXT);

    mgr->HookCount++;

    if (outHook) {
        *outHook = hook;
    }

    INFO("EPT hook installed successfully (slot %u, count=%u)", i, mgr->HookCount);
    return OMBRA_SUCCESS;
}

// =============================================================================
// Hook Removal
// =============================================================================

OMBRA_STATUS HookRemove(HOOK_MANAGER* mgr, EPT_HOOK* hook) {
    if (!mgr || !hook || hook->Magic != HOOK_MAGIC) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    if (!hook->Active) {
        return OMBRA_SUCCESS;  // Already removed
    }

    INFO("Removing hook at VA=0x%llx", hook->TargetVirtual);

    // Restore original EPT entry
    U64* pte = EptGetEntry(mgr->Ept, hook->TargetPhysical);
    if (pte) {
        EPT_PTE* entry = (EPT_PTE*)pte;
        entry->PagePhysAddr = hook->TargetPhysical >> 12;
        EptInvalidate(mgr->Ept, INVEPT_TYPE_SINGLE_CONTEXT);
    }

    // Free shadow page
    if (hook->ShadowVirtual) {
        HookFreeShadowPage(mgr, hook->ShadowVirtual);
    }

    // Clear hook structure
    hook->Active = false;
    hook->Magic = 0;
    mgr->HookCount--;

    INFO("Hook removed (remaining=%u)", mgr->HookCount);
    return OMBRA_SUCCESS;
}

// =============================================================================
// Hook Lookup
// =============================================================================

EPT_HOOK* HookFindByPhysical(HOOK_MANAGER* mgr, U64 physicalAddress) {
    U64 pageBase = physicalAddress & ~0xFFFULL;

    for (U32 i = 0; i < MAX_HOOKS; i++) {
        EPT_HOOK* hook = &mgr->Hooks[i];
        if (hook->Active && hook->Magic == HOOK_MAGIC) {
            if ((hook->TargetPhysical & ~0xFFFULL) == pageBase ||
                (hook->ShadowPhysical & ~0xFFFULL) == pageBase) {
                return hook;
            }
        }
    }
    return NULL;
}

EPT_HOOK* HookFindByVirtual(HOOK_MANAGER* mgr, U64 virtualAddress) {
    U64 pageBase = virtualAddress & ~0xFFFULL;

    for (U32 i = 0; i < MAX_HOOKS; i++) {
        EPT_HOOK* hook = &mgr->Hooks[i];
        if (hook->Active && hook->Magic == HOOK_MAGIC) {
            if ((hook->TargetVirtual & ~0xFFFULL) == pageBase) {
                return hook;
            }
        }
    }
    return NULL;
}

// =============================================================================
// EPT Violation Handler for Hooks
// =============================================================================

bool HookHandleEptViolation(
    HOOK_MANAGER* mgr,
    U64 guestPhysical,
    U64 qualification,
    void* guestRegs
) {
    (void)guestRegs;  // For now

    if (!mgr || !mgr->Initialized) {
        return false;
    }

    // Check if this is a hooked page
    EPT_HOOK* hook = HookFindByPhysical(mgr, guestPhysical);
    if (!hook) {
        return false;  // Not our violation
    }

    bool isRead = (qualification & EPT_VIOLATION_READ) != 0;
    bool isWrite = (qualification & EPT_VIOLATION_WRITE) != 0;
    bool isExecute = (qualification & EPT_VIOLATION_EXECUTE) != 0;

    TRACE("EPT violation on hooked page: PA=0x%llx R=%d W=%d X=%d",
          guestPhysical, isRead, isWrite, isExecute);

    hook->HitCount++;

    // For a full implementation, we would:
    // 1. On execute violation: switch to shadow page, set MTF
    // 2. On read violation: switch to original page, set MTF, single-step
    // 3. On MTF exit: restore EPT mapping

    // For now, just count the hit
    return true;
}

// =============================================================================
// Shadow Page Management (Simplified)
// =============================================================================

void* HookAllocateShadowPage(HOOK_MANAGER* mgr, U64* outPhysical) {
    if (!mgr || !mgr->ShadowPool || mgr->ShadowPoolUsed >= mgr->ShadowPoolSize) {
        return NULL;
    }

    U32 pageIndex = mgr->ShadowPoolUsed++;
    void* page = (U8*)mgr->ShadowPool + (pageIndex * 4096);
    *outPhysical = mgr->ShadowPoolPhysical + (pageIndex * 4096);

    return page;
}

void HookFreeShadowPage(HOOK_MANAGER* mgr, void* page) {
    (void)mgr;
    (void)page;
    // Simplified - real implementation would track free pages
    // For now, shadow pages are not reclaimed
}

// =============================================================================
// Hook Enable/Disable
// =============================================================================

OMBRA_STATUS HookEnable(EPT_HOOK* hook) {
    if (!hook || hook->Magic != HOOK_MAGIC) {
        return OMBRA_ERROR_INVALID_PARAM;
    }
    hook->Active = true;
    return OMBRA_SUCCESS;
}

OMBRA_STATUS HookDisable(EPT_HOOK* hook) {
    if (!hook || hook->Magic != HOOK_MAGIC) {
        return OMBRA_ERROR_INVALID_PARAM;
    }
    hook->Active = false;
    return OMBRA_SUCCESS;
}
