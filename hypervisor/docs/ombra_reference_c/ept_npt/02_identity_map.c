/**
 * 02_identity_map.c - EPT Identity Mapping Construction
 * 
 * Pure C implementation of EPT identity map setup.
 * Reference: Intel SDM Volume 3, Chapter 28
 * 
 * CONCEPTS COVERED:
 * - Identity mapping (GPA == HPA)
 * - Large page usage (1GB, 2MB)
 * - Memory type handling
 * - MTRR awareness
 */

#include "01_ept_structures.h"
#include "../common/msr_defs.h"

/*===========================================================================
 * CONCEPT 1: Identity Mapping
 *===========================================================================
 * With identity mapping:
 *   Guest Physical Address (GPA) == Host Physical Address (HPA)
 * 
 * This is the simplest and most common configuration:
 * - Guest sees the same physical memory layout as host
 * - No address translation overhead
 * - Hooks are applied as modifications to this baseline
 */

/*===========================================================================
 * CONCEPT 2: Large Page Strategy
 *===========================================================================
 * For performance, use the largest page size possible:
 * - 1GB pages: Cover most physical memory efficiently
 * - 2MB pages: For regions needing finer control
 * - 4KB pages: Only where needed (hooks, protection)
 * 
 * With 1GB pages:
 * - PML4[0] → PDPT with 512 × 1GB entries = 512GB coverage
 * - Entire 512GB mapped with just 2 page tables!
 */

/*===========================================================================
 * CONCEPT 3: MTRR Memory Types
 *===========================================================================
 * Memory Type Range Registers define caching behavior for physical ranges.
 * EPT memory type combines with MTRR to determine final caching.
 * 
 * Safe defaults:
 * - Normal RAM (< ~3.5GB): Write-Back (WB)
 * - MMIO region (~0xFE000000+): Uncacheable (UC)
 * 
 * For full correctness, read MTRR MSRs and set EPT types accordingly.
 */

#define MMIO_BASE_TYPICAL   0xFE000000ULL

/**
 * Determine memory type for a physical address
 * Simplified version - use MTRR parsing for production
 */
static U8 GetMemoryType(U64 physicalAddress)
{
    /* MMIO regions should be uncacheable */
    if (physicalAddress >= MMIO_BASE_TYPICAL) {
        return EPT_MT_UC;
    }
    
    /* Normal RAM is write-back */
    return EPT_MT_WB;
}

/*===========================================================================
 * CONCEPT 4: 1GB Page Identity Map
 *===========================================================================
 * Most efficient: Use 1GB pages for entire physical memory.
 * Requires EPT 1GB page support (check MSR_IA32_VMX_EPT_VPID_CAP bit 17).
 */

/**
 * Check if 1GB pages are supported
 */
static bool EptSupports1GbPages(void)
{
    U64 cap = __readmsr(MSR_IA32_VMX_EPT_VPID_CAP);
    return (cap & EPT_CAP_1GB_PAGE) != 0;
}

/**
 * Initialize EPT with 1GB page identity map
 * 
 * @param ept           EPT state structure to initialize
 * @param pml4Virtual   Virtual address of PML4 table (4KB, zeroed)
 * @param pml4Physical  Physical address of PML4 table
 * @param pdptVirtual   Virtual address of PDPT (4KB, zeroed)
 * @param pdptPhysical  Physical address of PDPT
 * @return OMBRA_SUCCESS or error
 */
OMBRA_STATUS EptInitialize1GbPages(
    EPT_STATE* ept,
    void* pml4Virtual,
    U64 pml4Physical,
    void* pdptVirtual,
    U64 pdptPhysical)
{
    EPT_PML4E* pml4;
    EPT_PDPTE* pdpt;
    U32 i;
    
    if (!ept || !pml4Virtual || !pdptVirtual) {
        return OMBRA_ERROR_INVALID_PARAM;
    }
    
    /* Check 1GB page support */
    if (!EptSupports1GbPages()) {
        return OMBRA_ERROR_NOT_SUPPORTED;
    }
    
    /* Store pointers */
    ept->Pml4 = (EPT_PML4E*)pml4Virtual;
    ept->Pml4Physical = pml4Physical;
    ept->Pdpt = (EPT_PDPTE*)pdptVirtual;
    ept->PdptPhysical = pdptPhysical;
    
    pml4 = ept->Pml4;
    pdpt = ept->Pdpt;
    
    /* Zero both tables */
    for (i = 0; i < EPT_ENTRIES_PER_TABLE; i++) {
        pml4[i].Value = 0;
        pdpt[i].Value = 0;
    }
    
    /*
     * PML4[0] → PDPT
     * This single PML4 entry covers GPA 0 to 512GB
     */
    pml4[0].Read = 1;
    pml4[0].Write = 1;
    pml4[0].Execute = 1;
    pml4[0].ExecuteUser = 1;
    pml4[0].PdptPhysAddr = pdptPhysical >> 12;
    
    /*
     * PDPT[0..511] → 1GB identity mapped pages
     * Each entry covers 1GB: GPA N*1GB → HPA N*1GB
     */
    for (i = 0; i < EPT_ENTRIES_PER_TABLE; i++) {
        U64 physAddr = (U64)i * EPT_PAGE_1GB;
        
        pdpt[i].LargePage.ReadLarge = 1;
        pdpt[i].LargePage.WriteLarge = 1;
        pdpt[i].LargePage.ExecuteLarge = 1;
        pdpt[i].LargePage.ExecuteUserLrg = 1;
        pdpt[i].LargePage.LargePage = 1;     /* 1GB page */
        pdpt[i].LargePage.MemoryType = GetMemoryType(physAddr);
        pdpt[i].LargePage.PagePhysAddr = physAddr >> 30;  /* Bits 51:30 */
    }
    
    /* Build EPTP */
    ept->Eptp = EptBuildPointer(pml4Physical, false);
    ept->Initialized = true;
    ept->SplitCount = 0;
    
    return OMBRA_SUCCESS;
}

/*===========================================================================
 * CONCEPT 5: 2MB Page Identity Map (Alternative)
 *===========================================================================
 * If 1GB pages aren't supported, use 2MB pages.
 * Requires more page tables but still efficient.
 */

/**
 * Initialize EPT with 2MB page identity map
 * 
 * @param ept           EPT state structure
 * @param pml4Virtual   PML4 table (4KB)
 * @param pml4Physical  Physical address of PML4
 * @param pdptVirtual   PDPT (4KB)
 * @param pdptPhysical  Physical of PDPT
 * @param pdArrayVirtual    Array of PD tables (512 × 4KB = 2MB)
 * @param pdArrayPhysical   Physical base of PD array
 * @param numPdTables   Number of PD tables (512 for full 512GB)
 */
OMBRA_STATUS EptInitialize2MbPages(
    EPT_STATE* ept,
    void* pml4Virtual,
    U64 pml4Physical,
    void* pdptVirtual,
    U64 pdptPhysical,
    void* pdArrayVirtual,
    U64 pdArrayPhysical,
    U32 numPdTables)
{
    EPT_PML4E* pml4;
    EPT_PDPTE* pdpt;
    EPT_PDE* pdArray;
    U32 pdptIdx, pdIdx;
    
    if (!ept || !pml4Virtual || !pdptVirtual || !pdArrayVirtual) {
        return OMBRA_ERROR_INVALID_PARAM;
    }
    
    ept->Pml4 = (EPT_PML4E*)pml4Virtual;
    ept->Pml4Physical = pml4Physical;
    ept->Pdpt = (EPT_PDPTE*)pdptVirtual;
    ept->PdptPhysical = pdptPhysical;
    
    pml4 = ept->Pml4;
    pdpt = ept->Pdpt;
    pdArray = (EPT_PDE*)pdArrayVirtual;
    
    /* Zero all tables */
    for (pdptIdx = 0; pdptIdx < EPT_ENTRIES_PER_TABLE; pdptIdx++) {
        pml4[pdptIdx].Value = 0;
        pdpt[pdptIdx].Value = 0;
    }
    
    /* PML4[0] → PDPT */
    pml4[0].Read = 1;
    pml4[0].Write = 1;
    pml4[0].Execute = 1;
    pml4[0].PdptPhysAddr = pdptPhysical >> 12;
    
    /* Setup PDPT entries pointing to PD tables */
    for (pdptIdx = 0; pdptIdx < numPdTables && pdptIdx < EPT_ENTRIES_PER_TABLE; pdptIdx++) {
        U64 pdPhysical = pdArrayPhysical + (pdptIdx * PAGE_SIZE);
        EPT_PDE* pd = &pdArray[pdptIdx * EPT_ENTRIES_PER_TABLE];
        
        /* PDPT[i] → PD[i] */
        pdpt[pdptIdx].Read = 1;
        pdpt[pdptIdx].Write = 1;
        pdpt[pdptIdx].Execute = 1;
        pdpt[pdptIdx].PdPhysAddr = pdPhysical >> 12;
        
        /* PD entries: 512 × 2MB pages per PD */
        for (pdIdx = 0; pdIdx < EPT_ENTRIES_PER_TABLE; pdIdx++) {
            U64 physAddr = ((U64)pdptIdx * EPT_PAGE_1GB) + ((U64)pdIdx * EPT_PAGE_2MB);
            
            pd[pdIdx].LargePage.ReadLarge = 1;
            pd[pdIdx].LargePage.WriteLarge = 1;
            pd[pdIdx].LargePage.ExecuteLarge = 1;
            pd[pdIdx].LargePage.LargePage = 1;   /* 2MB page */
            pd[pdIdx].LargePage.MemoryType = GetMemoryType(physAddr);
            pd[pdIdx].LargePage.PagePhysAddr = physAddr >> 21;  /* Bits 51:21 */
        }
    }
    
    ept->Eptp = EptBuildPointer(pml4Physical, false);
    ept->Initialized = true;
    
    return OMBRA_SUCCESS;
}

/*===========================================================================
 * CONCEPT 6: Large Page Splitting
 *===========================================================================
 * For hooks, we need 4KB granularity. Split a 2MB page into 512 × 4KB pages.
 * The 4KB PT must be pre-allocated.
 */

/**
 * Split a 2MB page into 512 × 4KB pages
 * Required for installing hooks on specific addresses.
 * 
 * @param pde           Pointer to the 2MB PDE to split
 * @param ptVirtual     Pre-allocated 4KB page table (virtual)
 * @param ptPhysical    Physical address of the page table
 */
void EptSplit2MbTo4Kb(EPT_PDE* pde, EPT_PTE* ptVirtual, U64 ptPhysical)
{
    U32 i;
    U64 basePhysical;
    U8 memType;
    
    /* Get the 2MB page's physical address and memory type */
    basePhysical = (U64)pde->LargePage.PagePhysAddr << 21;
    memType = (U8)pde->LargePage.MemoryType;
    
    /* Fill PT with 512 × 4KB entries */
    for (i = 0; i < EPT_ENTRIES_PER_TABLE; i++) {
        U64 pagePhys = basePhysical + (i * PAGE_SIZE);
        
        ptVirtual[i].Value = 0;
        ptVirtual[i].Read = 1;
        ptVirtual[i].Write = 1;
        ptVirtual[i].Execute = 1;
        ptVirtual[i].MemoryType = memType;
        ptVirtual[i].PagePhysAddr = pagePhys >> 12;
    }
    
    /* Convert PDE from large page to PT pointer */
    pde->Value = 0;
    pde->Read = 1;
    pde->Write = 1;
    pde->Execute = 1;
    pde->PtPhysAddr = ptPhysical >> 12;
    /* Note: LargePage bit is now 0 (not a large page anymore) */
}

/*===========================================================================
 * CONCEPT 7: EPT Entry Lookup
 *===========================================================================
 * Walk EPT hierarchy to find the entry for a given GPA.
 */

/**
 * Get the EPT PTE for a 4KB page
 * Assumes the page has been split to 4KB granularity.
 * 
 * @param ept   EPT state
 * @param gpa   Guest physical address
 * @return Pointer to PTE, or NULL if not a 4KB page
 */
EPT_PTE* EptGetPte(EPT_STATE* ept, U64 gpa)
{
    U32 pml4Idx, pdptIdx, pdIdx, ptIdx;
    EPT_PML4E* pml4e;
    EPT_PDPTE* pdpte;
    EPT_PDE* pde;
    EPT_PTE* pt;
    
    if (!ept || !ept->Initialized) {
        return NULL;
    }
    
    pml4Idx = EPT_PML4_INDEX(gpa);
    pdptIdx = EPT_PDPT_INDEX(gpa);
    pdIdx = EPT_PD_INDEX(gpa);
    ptIdx = EPT_PT_INDEX(gpa);
    
    /* Check PML4 entry */
    pml4e = &ept->Pml4[pml4Idx];
    if (!pml4e->Read) {
        return NULL;
    }
    
    /* Get PDPT entry */
    /* Note: In production, convert physical to virtual properly */
    pdpte = (EPT_PDPTE*)((U64)pml4e->PdptPhysAddr << 12);
    pdpte = &pdpte[pdptIdx];
    
    if (!pdpte->Read) {
        return NULL;
    }
    
    /* Check for 1GB page */
    if (pdpte->LargePage.LargePage) {
        return NULL;  /* 1GB page, not 4KB */
    }
    
    /* Get PD entry */
    pde = (EPT_PDE*)((U64)pdpte->PdPhysAddr << 12);
    pde = &pde[pdIdx];
    
    if (!pde->Read) {
        return NULL;
    }
    
    /* Check for 2MB page */
    if (pde->LargePage.LargePage) {
        return NULL;  /* 2MB page, not 4KB */
    }
    
    /* Get PT entry */
    pt = (EPT_PTE*)((U64)pde->PtPhysAddr << 12);
    
    return &pt[ptIdx];
}

/*===========================================================================
 * CONCEPT 8: Permission Modification
 *===========================================================================
 * Change permissions on a 4KB page (for hooks, protection).
 */

/**
 * Set EPT permissions for a 4KB page
 * 
 * @param pte       Pointer to the PTE
 * @param read      Allow read access
 * @param write     Allow write access
 * @param execute   Allow execute access
 */
void EptSetPermissions(EPT_PTE* pte, bool read, bool write, bool execute)
{
    pte->Read = read ? 1 : 0;
    pte->Write = write ? 1 : 0;
    pte->Execute = execute ? 1 : 0;
}

/**
 * Set execute-only permissions (for shadow page hooks)
 * Intel EPT only - AMD NPT cannot do execute-only!
 */
void EptSetExecuteOnly(EPT_PTE* pte)
{
    pte->Read = 0;
    pte->Write = 0;
    pte->Execute = 1;
}

/**
 * Redirect a PTE to a different physical page
 * Used for shadow page hooks.
 */
void EptRedirectPage(EPT_PTE* pte, U64 newPhysical)
{
    pte->PagePhysAddr = newPhysical >> 12;
}
