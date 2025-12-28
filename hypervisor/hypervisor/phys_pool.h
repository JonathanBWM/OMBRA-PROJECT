// phys_pool.h — Hypervisor Physical Page Pool
// OmbraHypervisor
//
// Manages hypervisor-private physical pages that Windows has no knowledge of.
// Used for EPT-only driver mapping where allocations must not appear in BigPool.

#ifndef OMBRA_PHYS_POOL_H
#define OMBRA_PHYS_POOL_H

#include "../shared/types.h"

// =============================================================================
// Configuration
// =============================================================================

// Maximum pages in the hypervisor pool (256MB default)
#define HV_POOL_MAX_PAGES       (256 * 1024 * 1024 / PAGE_SIZE)

// Pool allocation granularity
#define HV_POOL_BLOCK_SIZE      PAGE_SIZE

// =============================================================================
// Pool State Structure
// =============================================================================

typedef struct _HV_PHYS_POOL {
    // Pool base addresses
    U64         BasePhysical;       // Physical address of pool start
    void*       BaseVirtual;        // Virtual address (hypervisor mapping)

    // Pool size tracking
    U64         TotalSize;          // Total pool size in bytes
    U64         TotalPages;         // Total 4KB pages
    U64         UsedPages;          // Currently allocated pages
    U64         PeakUsedPages;      // Peak usage for diagnostics

    // Allocation bitmap (1 bit per 4KB page)
    U64*        Bitmap;             // Allocation bitmap
    U32         BitmapQwords;       // Number of U64s in bitmap

    // Thread safety
    volatile U32 Lock;              // Spinlock for concurrent access

    // State
    bool        Initialized;
} HV_PHYS_POOL;

// =============================================================================
// Initialization
// =============================================================================

// Initialize pool with pre-allocated memory region
// Called during Phase 1 with contiguous physical memory
OMBRA_STATUS HvPoolInitialize(
    HV_PHYS_POOL* pool,
    void* baseVirtual,
    U64 basePhysical,
    U64 totalSize
);

// Destroy pool (does not free memory - caller's responsibility)
void HvPoolDestroy(HV_PHYS_POOL* pool);

// =============================================================================
// Allocation
// =============================================================================

// Allocate contiguous physical pages from pool
// Returns physical address of allocated region, or 0 on failure
U64 HvPoolAllocatePages(
    HV_PHYS_POOL* pool,
    U32 pageCount
);

// Free previously allocated pages
OMBRA_STATUS HvPoolFreePages(
    HV_PHYS_POOL* pool,
    U64 physicalAddr,
    U32 pageCount
);

// =============================================================================
// Query Functions
// =============================================================================

// Get pool statistics
typedef struct _HV_POOL_STATS {
    U64     TotalPages;
    U64     UsedPages;
    U64     FreePages;
    U64     PeakUsedPages;
    U64     LargestFreeBlock;   // Largest contiguous free region (pages)
} HV_POOL_STATS;

void HvPoolGetStats(HV_PHYS_POOL* pool, HV_POOL_STATS* stats);

// Check if address is within pool
bool HvPoolContainsPhysical(HV_PHYS_POOL* pool, U64 physicalAddr);

// Convert pool physical address to virtual
void* HvPoolPhysToVirt(HV_PHYS_POOL* pool, U64 physicalAddr);

// Convert pool virtual address to physical
U64 HvPoolVirtToPhys(HV_PHYS_POOL* pool, void* virtualAddr);

#endif // OMBRA_PHYS_POOL_H
