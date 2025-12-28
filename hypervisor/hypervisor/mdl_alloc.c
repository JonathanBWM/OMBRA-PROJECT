// mdl_alloc.c - MDL-Based Stealth Memory Allocator
// OmbraHypervisor
//
// Allocates memory via MmAllocatePagesForMdlEx which does NOT appear in BigPool.
// This eliminates the 'IPRT' pool tag detection vector used by anti-cheat.

#include "mdl_alloc.h"
#include "kernel_resolve.h"

// =============================================================================
// Global Allocator Instances
// =============================================================================

// Bootstrap allocator - lives in .data section before relocation
MDL_ALLOCATOR g_BootstrapAllocator = {0};

// Pointer to active allocator (set after relocation to MDL-backed image)
MDL_ALLOCATOR* g_MdlAllocator = NULL;

// =============================================================================
// Internal Spinlock Implementation
// =============================================================================
// Simple spinlock using interlocked operations

static void SpinLockAcquire(volatile U64* lock) {
    while (_InterlockedCompareExchange64((volatile long long*)lock, 1, 0) != 0) {
        // Spin with pause instruction to reduce contention
        _mm_pause();
    }
}

static void SpinLockRelease(volatile U64* lock) {
    _InterlockedExchange64((volatile long long*)lock, 0);
}

// =============================================================================
// Internal Helper: Allocate and Map a Single MDL Region
// =============================================================================

static NTSTATUS AllocateMdlRegion(MDL_REGION* region, U64 size) {
    PMDL mdl = NULL;
    void* va = NULL;

    // Validate kernel symbols are available
    if (!g_KernelSymbols.Initialized) {
        return STATUS_NOT_FOUND;
    }

    // Prefer MmAllocatePagesForMdlEx with contiguous chunks for large regions
    if (g_KernelSymbols.MmAllocatePagesForMdlEx) {
        mdl = g_KernelSymbols.MmAllocatePagesForMdlEx(
            0,                          // LowAddress: accept any physical address
            0xFFFFFFFFFFFFFFFFULL,      // HighAddress: full 64-bit range
            0,                          // SkipBytes: no alignment boundary
            size,                       // TotalBytes: requested size
            MmCached,                   // CacheType: cached for performance
            MM_ALLOCATE_FULLY_REQUIRED | MM_ALLOCATE_REQUIRE_CONTIGUOUS_CHUNKS
        );
    } else if (g_KernelSymbols.MmAllocatePagesForMdl) {
        // Fallback to basic version (may not be contiguous)
        mdl = g_KernelSymbols.MmAllocatePagesForMdl(
            0,                          // LowAddress
            0xFFFFFFFFFFFFFFFFULL,      // HighAddress
            0,                          // SkipBytes
            size                        // TotalBytes
        );
    } else {
        return STATUS_NOT_FOUND;
    }

    if (!mdl) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // Map the MDL pages into kernel virtual address space
    if (!g_KernelSymbols.MmMapLockedPagesSpecifyCache) {
        // Cleanup and fail
        if (g_KernelSymbols.MmFreePagesFromMdl) {
            g_KernelSymbols.MmFreePagesFromMdl(mdl);
        }
        return STATUS_NOT_FOUND;
    }

    va = g_KernelSymbols.MmMapLockedPagesSpecifyCache(
        mdl,
        KernelMode,                 // AccessMode: kernel
        MmCached,                   // CacheType: cached
        NULL,                       // RequestedAddress: any
        0,                          // BugCheckOnFailure: FALSE (graceful failure)
        HighPagePriority            // Priority: high
    );

    if (!va) {
        // Cleanup and fail
        if (g_KernelSymbols.MmFreePagesFromMdl) {
            g_KernelSymbols.MmFreePagesFromMdl(mdl);
        }
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // Zero the allocated memory
    if (g_KernelSymbols.RtlZeroMemory) {
        g_KernelSymbols.RtlZeroMemory(va, size);
    } else {
        // Manual zeroing fallback
        U8* p = (U8*)va;
        for (U64 i = 0; i < size; i++) {
            p[i] = 0;
        }
    }

    // Get physical address of the first page
    U64 physAddr = 0;
    if (g_KernelSymbols.MmGetPhysicalAddress) {
        physAddr = g_KernelSymbols.MmGetPhysicalAddress(va);
    }

    // Initialize the region structure
    region->Mdl = mdl;
    region->VirtualAddress = va;
    region->PhysicalAddress = physAddr;
    region->TotalSize = size;
    region->UsedSize = 0;
    region->SpinLock = 0;

    return STATUS_SUCCESS;
}

// =============================================================================
// Internal Helper: Free an MDL Region
// =============================================================================

static void FreeMdlRegion(MDL_REGION* region) {
    if (!region || !region->Mdl) {
        return;
    }

    // Unmap the virtual address
    if (g_KernelSymbols.MmUnmapLockedPages && region->VirtualAddress) {
        g_KernelSymbols.MmUnmapLockedPages(region->VirtualAddress, region->Mdl);
    }

    // Free the physical pages
    if (g_KernelSymbols.MmFreePagesFromMdl) {
        g_KernelSymbols.MmFreePagesFromMdl(region->Mdl);
    }

    // Free the MDL structure itself
    if (g_KernelSymbols.IoFreeMdl) {
        g_KernelSymbols.IoFreeMdl(region->Mdl);
    }

    // Zero the region structure
    region->Mdl = NULL;
    region->VirtualAddress = NULL;
    region->PhysicalAddress = 0;
    region->TotalSize = 0;
    region->UsedSize = 0;
    region->SpinLock = 0;
}

// =============================================================================
// Initialization and Destruction
// =============================================================================

NTSTATUS MdlAllocatorInit(MDL_ALLOCATOR* Allocator) {
    NTSTATUS status;

    if (!Allocator) {
        return STATUS_INVALID_PARAMETER;
    }

    // Validate required kernel functions
    if (!g_KernelSymbols.Initialized ||
        !g_KernelSymbols.MmMapLockedPagesSpecifyCache ||
        (!g_KernelSymbols.MmAllocatePagesForMdlEx && !g_KernelSymbols.MmAllocatePagesForMdl)) {
        return STATUS_NOT_FOUND;
    }

    // Clear the allocator structure
    if (g_KernelSymbols.RtlZeroMemory) {
        g_KernelSymbols.RtlZeroMemory(Allocator, sizeof(MDL_ALLOCATOR));
    }

    // Allocate the main hypervisor region (2MB)
    status = AllocateMdlRegion(&Allocator->MainRegion, MDL_REGION_HYPERVISOR);
    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    // Allocate the VMCS region (64KB, requires 4KB alignment)
    status = AllocateMdlRegion(&Allocator->VmcsRegion, MDL_REGION_VMCS);
    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    // Allocate the EPT region (4MB for EPT tables)
    status = AllocateMdlRegion(&Allocator->EptRegion, MDL_REGION_EPT);
    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    // Allocate the stack region (256KB for host stacks)
    status = AllocateMdlRegion(&Allocator->StackRegion, MDL_REGION_STACKS);
    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    // Allocate the MSR bitmap region (64KB)
    status = AllocateMdlRegion(&Allocator->MsrRegion, MDL_REGION_MSR_BITMAPS);
    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    // Allocate the misc region (1MB)
    status = AllocateMdlRegion(&Allocator->MiscRegion, MDL_REGION_MISC);
    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    Allocator->Initialized = true;
    return STATUS_SUCCESS;

cleanup:
    // Free any regions that were allocated
    FreeMdlRegion(&Allocator->MainRegion);
    FreeMdlRegion(&Allocator->VmcsRegion);
    FreeMdlRegion(&Allocator->EptRegion);
    FreeMdlRegion(&Allocator->StackRegion);
    FreeMdlRegion(&Allocator->MsrRegion);
    FreeMdlRegion(&Allocator->MiscRegion);
    return status;
}

void MdlAllocatorDestroy(MDL_ALLOCATOR* Allocator) {
    if (!Allocator || !Allocator->Initialized) {
        return;
    }

    FreeMdlRegion(&Allocator->MainRegion);
    FreeMdlRegion(&Allocator->VmcsRegion);
    FreeMdlRegion(&Allocator->EptRegion);
    FreeMdlRegion(&Allocator->StackRegion);
    FreeMdlRegion(&Allocator->MsrRegion);
    FreeMdlRegion(&Allocator->MiscRegion);

    Allocator->Initialized = false;
}

// =============================================================================
// Internal: Bump Allocator from Region
// =============================================================================

static void* BumpAlloc(MDL_REGION* region, U64 size, U64 alignment) {
    void* result = NULL;
    U64 alignedOffset;
    U64 newUsed;

    if (!region || !region->VirtualAddress || size == 0) {
        return NULL;
    }

    SpinLockAcquire(&region->SpinLock);

    // Calculate aligned offset
    alignedOffset = (region->UsedSize + alignment - 1) & ~(alignment - 1);
    newUsed = alignedOffset + size;

    // Check for overflow
    if (newUsed > region->TotalSize) {
        SpinLockRelease(&region->SpinLock);
        return NULL;
    }

    // Return the allocated address
    result = (U8*)region->VirtualAddress + alignedOffset;
    region->UsedSize = newUsed;

    SpinLockRelease(&region->SpinLock);

    return result;
}

// =============================================================================
// Allocation Functions
// =============================================================================

void* MdlAlloc(MDL_ALLOCATOR* Allocator, U64 Size) {
    if (!Allocator || !Allocator->Initialized) {
        return NULL;
    }
    return BumpAlloc(&Allocator->MainRegion, Size, 8); // 8-byte alignment default
}

void* MdlAllocAligned(MDL_ALLOCATOR* Allocator, U64 Size, U64 Alignment) {
    if (!Allocator || !Allocator->Initialized) {
        return NULL;
    }
    // Alignment must be power of 2
    if (Alignment == 0 || (Alignment & (Alignment - 1)) != 0) {
        return NULL;
    }
    return BumpAlloc(&Allocator->MainRegion, Size, Alignment);
}

void* MdlAllocVmcs(MDL_ALLOCATOR* Allocator, U64* outPhysical) {
    void* va;
    U64 offset;

    if (!Allocator || !Allocator->Initialized || !outPhysical) {
        return NULL;
    }

    // VMCS requires 4KB alignment
    va = BumpAlloc(&Allocator->VmcsRegion, 4096, 4096);
    if (!va) {
        return NULL;
    }

    // Calculate physical address
    // For contiguous regions, phys = base_phys + (va - base_va)
    offset = (U64)((U8*)va - (U8*)Allocator->VmcsRegion.VirtualAddress);
    *outPhysical = Allocator->VmcsRegion.PhysicalAddress + offset;

    return va;
}

void* MdlAllocEptTable(MDL_ALLOCATOR* Allocator, U64* outPhysical) {
    void* va;
    U64 offset;

    if (!Allocator || !Allocator->Initialized || !outPhysical) {
        return NULL;
    }

    // EPT tables require 4KB alignment
    va = BumpAlloc(&Allocator->EptRegion, 4096, 4096);
    if (!va) {
        return NULL;
    }

    // Calculate physical address
    offset = (U64)((U8*)va - (U8*)Allocator->EptRegion.VirtualAddress);
    *outPhysical = Allocator->EptRegion.PhysicalAddress + offset;

    return va;
}

void* MdlAllocStack(MDL_ALLOCATOR* Allocator, U64 Size, U64* outPhysical) {
    void* va;
    U64 offset;

    if (!Allocator || !Allocator->Initialized) {
        return NULL;
    }

    // Stacks should be page-aligned
    va = BumpAlloc(&Allocator->StackRegion, Size, 4096);
    if (!va) {
        return NULL;
    }

    // Calculate physical address if requested
    if (outPhysical) {
        offset = (U64)((U8*)va - (U8*)Allocator->StackRegion.VirtualAddress);
        *outPhysical = Allocator->StackRegion.PhysicalAddress + offset;
    }

    return va;
}

void* MdlAllocMsrBitmap(MDL_ALLOCATOR* Allocator, U64* outPhysical) {
    void* va;
    U64 offset;

    if (!Allocator || !Allocator->Initialized || !outPhysical) {
        return NULL;
    }

    // MSR bitmaps require 4KB alignment
    va = BumpAlloc(&Allocator->MsrRegion, 4096, 4096);
    if (!va) {
        return NULL;
    }

    // Calculate physical address
    offset = (U64)((U8*)va - (U8*)Allocator->MsrRegion.VirtualAddress);
    *outPhysical = Allocator->MsrRegion.PhysicalAddress + offset;

    return va;
}

void* MdlAllocMisc(MDL_ALLOCATOR* Allocator, U64 Size) {
    if (!Allocator || !Allocator->Initialized) {
        return NULL;
    }
    return BumpAlloc(&Allocator->MiscRegion, Size, 8);
}

// =============================================================================
// Physical Address Helpers
// =============================================================================

// Check if address is within a specific region
static bool IsInRegion(MDL_REGION* region, void* addr) {
    U8* start = (U8*)region->VirtualAddress;
    U8* end = start + region->TotalSize;
    U8* ptr = (U8*)addr;
    return (ptr >= start && ptr < end);
}

U64 MdlGetPhysicalAddress(void* VirtualAddress) {
    MDL_ALLOCATOR* alloc = g_MdlAllocator;
    U64 offset;

    if (!alloc || !alloc->Initialized || !VirtualAddress) {
        // Fallback to kernel function
        if (g_KernelSymbols.MmGetPhysicalAddress) {
            return g_KernelSymbols.MmGetPhysicalAddress(VirtualAddress);
        }
        return 0;
    }

    // Check each region
    if (IsInRegion(&alloc->MainRegion, VirtualAddress)) {
        offset = (U64)((U8*)VirtualAddress - (U8*)alloc->MainRegion.VirtualAddress);
        return alloc->MainRegion.PhysicalAddress + offset;
    }

    if (IsInRegion(&alloc->VmcsRegion, VirtualAddress)) {
        offset = (U64)((U8*)VirtualAddress - (U8*)alloc->VmcsRegion.VirtualAddress);
        return alloc->VmcsRegion.PhysicalAddress + offset;
    }

    if (IsInRegion(&alloc->EptRegion, VirtualAddress)) {
        offset = (U64)((U8*)VirtualAddress - (U8*)alloc->EptRegion.VirtualAddress);
        return alloc->EptRegion.PhysicalAddress + offset;
    }

    if (IsInRegion(&alloc->StackRegion, VirtualAddress)) {
        offset = (U64)((U8*)VirtualAddress - (U8*)alloc->StackRegion.VirtualAddress);
        return alloc->StackRegion.PhysicalAddress + offset;
    }

    if (IsInRegion(&alloc->MsrRegion, VirtualAddress)) {
        offset = (U64)((U8*)VirtualAddress - (U8*)alloc->MsrRegion.VirtualAddress);
        return alloc->MsrRegion.PhysicalAddress + offset;
    }

    if (IsInRegion(&alloc->MiscRegion, VirtualAddress)) {
        offset = (U64)((U8*)VirtualAddress - (U8*)alloc->MiscRegion.VirtualAddress);
        return alloc->MiscRegion.PhysicalAddress + offset;
    }

    // Not in any MDL region, use kernel function
    if (g_KernelSymbols.MmGetPhysicalAddress) {
        return g_KernelSymbols.MmGetPhysicalAddress(VirtualAddress);
    }

    return 0;
}

bool MdlIsInRegion(MDL_ALLOCATOR* Allocator, void* VirtualAddress) {
    if (!Allocator || !Allocator->Initialized || !VirtualAddress) {
        return false;
    }

    return IsInRegion(&Allocator->MainRegion, VirtualAddress) ||
           IsInRegion(&Allocator->VmcsRegion, VirtualAddress) ||
           IsInRegion(&Allocator->EptRegion, VirtualAddress) ||
           IsInRegion(&Allocator->StackRegion, VirtualAddress) ||
           IsInRegion(&Allocator->MsrRegion, VirtualAddress) ||
           IsInRegion(&Allocator->MiscRegion, VirtualAddress);
}

// =============================================================================
// Statistics
// =============================================================================

void MdlGetStats(MDL_ALLOCATOR* Allocator, MDL_STATS* Stats) {
    if (!Allocator || !Stats) {
        return;
    }

    if (g_KernelSymbols.RtlZeroMemory) {
        g_KernelSymbols.RtlZeroMemory(Stats, sizeof(MDL_STATS));
    }

    if (!Allocator->Initialized) {
        return;
    }

    Stats->MainUsed = Allocator->MainRegion.UsedSize;
    Stats->MainTotal = Allocator->MainRegion.TotalSize;

    Stats->VmcsUsed = Allocator->VmcsRegion.UsedSize;
    Stats->VmcsTotal = Allocator->VmcsRegion.TotalSize;

    Stats->EptUsed = Allocator->EptRegion.UsedSize;
    Stats->EptTotal = Allocator->EptRegion.TotalSize;

    Stats->StackUsed = Allocator->StackRegion.UsedSize;
    Stats->StackTotal = Allocator->StackRegion.TotalSize;

    Stats->MsrUsed = Allocator->MsrRegion.UsedSize;
    Stats->MsrTotal = Allocator->MsrRegion.TotalSize;

    Stats->MiscUsed = Allocator->MiscRegion.UsedSize;
    Stats->MiscTotal = Allocator->MiscRegion.TotalSize;
}
