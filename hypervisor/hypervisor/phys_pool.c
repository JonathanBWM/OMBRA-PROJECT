// phys_pool.c — Hypervisor Physical Page Pool Implementation
// OmbraHypervisor
//
// Manages hypervisor-private physical pages that Windows has no knowledge of.
// Used for EPT-only driver mapping where allocations must not appear in BigPool.
//
// Thread-safe bitmap-based allocator using first-fit strategy.

#include "phys_pool.h"
#include "debug.h"
#include <intrin.h>
#include <string.h>

// =============================================================================
// Internal Spinlock Helpers
// =============================================================================

static inline void PoolSpinLockAcquire(volatile U32* lock) {
    while (_InterlockedCompareExchange((volatile long*)lock, 1, 0) != 0) {
        _mm_pause();
    }
}

static inline void PoolSpinLockRelease(volatile U32* lock) {
    _InterlockedExchange((volatile long*)lock, 0);
}

// =============================================================================
// Bitmap Helpers
// =============================================================================

// Check if bit is set (1 = allocated, 0 = free)
static inline bool BitmapTestBit(U64* bitmap, U32 bitIndex) {
    U32 qwordIndex = bitIndex / 64;
    U32 bitOffset = bitIndex % 64;
    return (bitmap[qwordIndex] & (1ULL << bitOffset)) != 0;
}

// Set bit (mark page as allocated)
static inline void BitmapSetBit(U64* bitmap, U32 bitIndex) {
    U32 qwordIndex = bitIndex / 64;
    U32 bitOffset = bitIndex % 64;
    bitmap[qwordIndex] |= (1ULL << bitOffset);
}

// Clear bit (mark page as free)
static inline void BitmapClearBit(U64* bitmap, U32 bitIndex) {
    U32 qwordIndex = bitIndex / 64;
    U32 bitOffset = bitIndex % 64;
    bitmap[qwordIndex] &= ~(1ULL << bitOffset);
}

// Find N contiguous clear bits starting from startBit
// Returns bit index of first clear bit, or totalBits if not found
static U32 BitmapFindContiguousClear(U64* bitmap, U32 totalBits, U32 count, U32 startBit) {
    U32 consecutiveFree = 0;
    U32 candidateStart = 0;
    U32 i;

    if (count == 0 || count > totalBits) {
        return totalBits;
    }

    for (i = startBit; i < totalBits; i++) {
        if (!BitmapTestBit(bitmap, i)) {
            // Free bit found
            if (consecutiveFree == 0) {
                candidateStart = i;
            }
            consecutiveFree++;

            if (consecutiveFree == count) {
                return candidateStart;
            }
        } else {
            // Allocated bit - reset counter
            consecutiveFree = 0;
        }
    }

    return totalBits;  // Not found
}

// Set N contiguous bits starting from startBit
static void BitmapSetRange(U64* bitmap, U32 startBit, U32 count) {
    U32 i;
    for (i = 0; i < count; i++) {
        BitmapSetBit(bitmap, startBit + i);
    }
}

// Clear N contiguous bits starting from startBit
static void BitmapClearRange(U64* bitmap, U32 startBit, U32 count) {
    U32 i;
    for (i = 0; i < count; i++) {
        BitmapClearBit(bitmap, startBit + i);
    }
}

// Count free pages in bitmap
static U32 BitmapCountFree(U64* bitmap, U32 totalBits) {
    U32 freeCount = 0;
    U32 i;

    for (i = 0; i < totalBits; i++) {
        if (!BitmapTestBit(bitmap, i)) {
            freeCount++;
        }
    }

    return freeCount;
}

// Find largest contiguous free region
static U32 BitmapFindLargestFreeRegion(U64* bitmap, U32 totalBits) {
    U32 maxFree = 0;
    U32 currentFree = 0;
    U32 i;

    for (i = 0; i < totalBits; i++) {
        if (!BitmapTestBit(bitmap, i)) {
            currentFree++;
            if (currentFree > maxFree) {
                maxFree = currentFree;
            }
        } else {
            currentFree = 0;
        }
    }

    return maxFree;
}

// =============================================================================
// Initialization
// =============================================================================

OMBRA_STATUS HvPoolInitialize(
    HV_PHYS_POOL* pool,
    void* baseVirtual,
    U64 basePhysical,
    U64 totalSize
) {
    U64 totalPages;
    U32 bitmapQwords;
    U32 bitmapBytes;
    U64 bitmapPages;
    U64 usablePages;
    U32 i;

    if (!pool || !baseVirtual || basePhysical == 0 || totalSize == 0) {
        ERR("HvPool: Invalid parameters");
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Validate alignment
    if ((basePhysical & (PAGE_SIZE - 1)) != 0) {
        ERR("HvPool: Base physical address 0x%llx not page-aligned", basePhysical);
        return OMBRA_ERROR_INVALID_PARAM;
    }

    if ((totalSize & (PAGE_SIZE - 1)) != 0) {
        ERR("HvPool: Total size 0x%llx not page-aligned", totalSize);
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Calculate total pages
    totalPages = totalSize / PAGE_SIZE;
    if (totalPages > HV_POOL_MAX_PAGES) {
        ERR("HvPool: Pool size %llu pages exceeds maximum %llu",
            totalPages, (U64)HV_POOL_MAX_PAGES);
        return OMBRA_ERROR_INVALID_PARAM;
    }

    INFO("HvPool: Initializing with %llu pages (%llu KB)",
         totalPages, totalSize / 1024);

    // Calculate bitmap size
    // Need 1 bit per page, round up to U64 boundary
    bitmapQwords = (U32)((totalPages + 63) / 64);
    bitmapBytes = bitmapQwords * sizeof(U64);
    bitmapPages = (bitmapBytes + PAGE_SIZE - 1) / PAGE_SIZE;

    TRACE("HvPool: Bitmap requires %u qwords (%u bytes, %llu pages)",
          bitmapQwords, bitmapBytes, bitmapPages);

    // Bitmap is stored at the start of the pool
    // Usable memory starts after the bitmap
    if (bitmapPages >= totalPages) {
        ERR("HvPool: Bitmap too large (%llu pages) for pool (%llu pages)",
            bitmapPages, totalPages);
        return OMBRA_ERROR_INVALID_PARAM;
    }

    usablePages = totalPages - bitmapPages;
    TRACE("HvPool: Usable pages = %llu (after %llu bitmap pages)",
          usablePages, bitmapPages);

    // Initialize pool structure
    pool->BasePhysical = basePhysical;
    pool->BaseVirtual = baseVirtual;
    pool->TotalSize = totalSize;
    pool->TotalPages = (U64)usablePages;
    pool->UsedPages = 0;
    pool->PeakUsedPages = 0;
    pool->Bitmap = (U64*)baseVirtual;
    pool->BitmapQwords = bitmapQwords;
    pool->Lock = 0;
    pool->Initialized = false;

    // Zero bitmap (all pages free)
    for (i = 0; i < bitmapQwords; i++) {
        pool->Bitmap[i] = 0;
    }

    // Mark bitmap pages as allocated (they're not available for use)
    for (i = 0; i < (U32)bitmapPages; i++) {
        BitmapSetBit(pool->Bitmap, i);
    }

    pool->UsedPages = bitmapPages;  // Bitmap pages count as used
    pool->Initialized = true;

    INFO("HvPool: Initialized at phys=0x%llx virt=%p, %llu usable pages",
         basePhysical, baseVirtual, usablePages);

    return OMBRA_SUCCESS;
}

void HvPoolDestroy(HV_PHYS_POOL* pool) {
    if (!pool) {
        return;
    }

    if (pool->Initialized) {
        INFO("HvPool: Destroying pool (used=%llu, peak=%llu)",
             pool->UsedPages, pool->PeakUsedPages);
    }

    pool->Initialized = false;
    pool->Bitmap = NULL;
}

// =============================================================================
// Allocation
// =============================================================================

U64 HvPoolAllocatePages(
    HV_PHYS_POOL* pool,
    U32 pageCount
) {
    U32 totalBits;
    U32 bitIndex;
    U64 physicalAddr;

    if (!pool || !pool->Initialized) {
        ERR("HvPool: Pool not initialized");
        return 0;
    }

    if (pageCount == 0) {
        ERR("HvPool: Invalid page count 0");
        return 0;
    }

    if (pageCount > pool->TotalPages) {
        ERR("HvPool: Requested %u pages exceeds pool size %llu",
            pageCount, pool->TotalPages);
        return 0;
    }

    PoolSpinLockAcquire(&pool->Lock);

    // Check if enough free pages exist
    if ((pool->TotalPages - pool->UsedPages) < pageCount) {
        ERR("HvPool: Out of memory (free=%llu, requested=%u)",
            pool->TotalPages - pool->UsedPages, pageCount);
        PoolSpinLockRelease(&pool->Lock);
        return 0;
    }

    // Find contiguous free region using first-fit
    totalBits = (U32)pool->TotalPages;
    bitIndex = BitmapFindContiguousClear(pool->Bitmap, totalBits, pageCount, 0);

    if (bitIndex >= totalBits) {
        ERR("HvPool: No contiguous region of %u pages available (fragmented)",
            pageCount);
        PoolSpinLockRelease(&pool->Lock);
        return 0;
    }

    // Mark pages as allocated
    BitmapSetRange(pool->Bitmap, bitIndex, pageCount);

    // Update statistics
    pool->UsedPages += pageCount;
    if (pool->UsedPages > pool->PeakUsedPages) {
        pool->PeakUsedPages = pool->UsedPages;
    }

    // Calculate physical address
    // First page of usable memory is at BasePhysical (bitmap is in pool but not counted in TotalPages)
    physicalAddr = pool->BasePhysical + ((U64)bitIndex * PAGE_SIZE);

    TRACE("HvPool: Allocated %u pages at bit %u (phys=0x%llx, used=%llu/%llu)",
          pageCount, bitIndex, physicalAddr, pool->UsedPages, pool->TotalPages);

    PoolSpinLockRelease(&pool->Lock);

    return physicalAddr;
}

OMBRA_STATUS HvPoolFreePages(
    HV_PHYS_POOL* pool,
    U64 physicalAddr,
    U32 pageCount
) {
    U64 offset;
    U32 bitIndex;
    U32 i;

    if (!pool || !pool->Initialized) {
        ERR("HvPool: Pool not initialized");
        return OMBRA_ERROR_INVALID_STATE;
    }

    if (pageCount == 0) {
        ERR("HvPool: Invalid page count 0");
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Check if address is within pool
    if (physicalAddr < pool->BasePhysical ||
        physicalAddr >= pool->BasePhysical + pool->TotalSize) {
        ERR("HvPool: Physical address 0x%llx outside pool range", physicalAddr);
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Check alignment
    if ((physicalAddr & (PAGE_SIZE - 1)) != 0) {
        ERR("HvPool: Physical address 0x%llx not page-aligned", physicalAddr);
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Calculate bit index
    offset = physicalAddr - pool->BasePhysical;
    bitIndex = (U32)(offset / PAGE_SIZE);

    if (bitIndex + pageCount > pool->TotalPages) {
        ERR("HvPool: Free range [%u, %u) exceeds pool size %llu",
            bitIndex, bitIndex + pageCount, pool->TotalPages);
        return OMBRA_ERROR_INVALID_PARAM;
    }

    PoolSpinLockAcquire(&pool->Lock);

    // Verify all pages are currently allocated
    for (i = 0; i < pageCount; i++) {
        if (!BitmapTestBit(pool->Bitmap, bitIndex + i)) {
            ERR("HvPool: Page at bit %u already free (double free?)", bitIndex + i);
            PoolSpinLockRelease(&pool->Lock);
            return OMBRA_ERROR_INVALID_STATE;
        }
    }

    // Clear bitmap bits
    BitmapClearRange(pool->Bitmap, bitIndex, pageCount);

    // Update statistics
    pool->UsedPages -= pageCount;

    TRACE("HvPool: Freed %u pages at bit %u (phys=0x%llx, used=%llu/%llu)",
          pageCount, bitIndex, physicalAddr, pool->UsedPages, pool->TotalPages);

    PoolSpinLockRelease(&pool->Lock);

    return OMBRA_SUCCESS;
}

// =============================================================================
// Query Functions
// =============================================================================

void HvPoolGetStats(HV_PHYS_POOL* pool, HV_POOL_STATS* stats) {
    U32 largestFree;

    if (!pool || !stats) {
        return;
    }

    if (!pool->Initialized) {
        memset(stats, 0, sizeof(HV_POOL_STATS));
        return;
    }

    PoolSpinLockAcquire(&pool->Lock);

    stats->TotalPages = pool->TotalPages;
    stats->UsedPages = pool->UsedPages;
    stats->FreePages = pool->TotalPages - pool->UsedPages;
    stats->PeakUsedPages = pool->PeakUsedPages;

    // Find largest contiguous free region
    largestFree = BitmapFindLargestFreeRegion(pool->Bitmap, (U32)pool->TotalPages);
    stats->LargestFreeBlock = (U64)largestFree;

    PoolSpinLockRelease(&pool->Lock);
}

bool HvPoolContainsPhysical(HV_PHYS_POOL* pool, U64 physicalAddr) {
    if (!pool || !pool->Initialized) {
        return false;
    }

    return (physicalAddr >= pool->BasePhysical &&
            physicalAddr < pool->BasePhysical + pool->TotalSize);
}

void* HvPoolPhysToVirt(HV_PHYS_POOL* pool, U64 physicalAddr) {
    U64 offset;

    if (!pool || !pool->Initialized) {
        return NULL;
    }

    if (!HvPoolContainsPhysical(pool, physicalAddr)) {
        return NULL;
    }

    offset = physicalAddr - pool->BasePhysical;
    return (U8*)pool->BaseVirtual + offset;
}

U64 HvPoolVirtToPhys(HV_PHYS_POOL* pool, void* virtualAddr) {
    U8* base;
    U8* addr;
    U64 offset;

    if (!pool || !pool->Initialized || !virtualAddr) {
        return 0;
    }

    base = (U8*)pool->BaseVirtual;
    addr = (U8*)virtualAddr;

    // Check if address is within pool virtual range
    if (addr < base || addr >= base + pool->TotalSize) {
        return 0;
    }

    offset = addr - base;
    return pool->BasePhysical + offset;
}
