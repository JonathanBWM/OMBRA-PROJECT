// mdl_alloc.h - MDL-Based Stealth Memory Allocator
// OmbraHypervisor
//
// Allocates memory via MmAllocatePagesForMdl which does NOT appear in BigPool.
// This eliminates the 'IPRT' pool tag detection vector used by anti-cheat.
//
// Usage:
// 1. After relocation, call MdlAllocatorInit() to create MDL regions
// 2. Use MdlAlloc/MdlAllocAligned for general allocations
// 3. Use MdlAllocVmcs/MdlAllocEptTable for VMX structures (4KB aligned)
// 4. Physical addresses are stable (pages are locked in memory)

#ifndef MDL_ALLOC_H
#define MDL_ALLOC_H

#include "../shared/types.h"
#include "kernel_resolve.h"

// =============================================================================
// Pre-allocated Region Sizes
// =============================================================================
// These are allocated upfront to avoid runtime kernel calls after VMX is active.

#define MDL_REGION_HYPERVISOR    (2 * 1024 * 1024)   // 2MB for HV code/data
#define MDL_REGION_VMCS          (64 * 1024)         // 64KB for VMCS (4KB * 16 cores max)
#define MDL_REGION_EPT           (4 * 1024 * 1024)   // 4MB for EPT tables
#define MDL_REGION_STACKS        (256 * 1024)        // 256KB for host stacks (16KB * 16 cores)
#define MDL_REGION_MSR_BITMAPS   (64 * 1024)         // 64KB for MSR bitmaps
#define MDL_REGION_MISC          (1 * 1024 * 1024)   // 1MB misc

#define MDL_TOTAL_SIZE           (MDL_REGION_HYPERVISOR + MDL_REGION_VMCS + \
                                  MDL_REGION_EPT + MDL_REGION_STACKS + \
                                  MDL_REGION_MSR_BITMAPS + MDL_REGION_MISC)

// =============================================================================
// MDL Region Structure
// =============================================================================
// Each region is a contiguous chunk of MDL-allocated memory with a bump allocator.

typedef struct _MDL_REGION {
    void*   Mdl;                // PMDL - stored as void* for header independence
    void*   VirtualAddress;     // Kernel VA mapping
    U64     PhysicalAddress;    // Physical base (for contiguous regions)
    U64     TotalSize;          // Total bytes allocated
    U64     UsedSize;           // Bytes allocated from this region
    U64     SpinLock;           // Simple spinlock for thread safety
} MDL_REGION;

// =============================================================================
// MDL Allocator State
// =============================================================================
// Contains multiple regions for different allocation types.
// Separate regions allow different alignment requirements.

typedef struct _MDL_ALLOCATOR {
    MDL_REGION  MainRegion;     // Primary hypervisor memory (code/data after relocation)
    MDL_REGION  VmcsRegion;     // VMCS structures (require 4KB alignment)
    MDL_REGION  EptRegion;      // EPT tables (require 4KB alignment)
    MDL_REGION  StackRegion;    // Host stacks
    MDL_REGION  MsrRegion;      // MSR bitmaps
    MDL_REGION  MiscRegion;     // Miscellaneous allocations
    bool        Initialized;
} MDL_ALLOCATOR;

// =============================================================================
// Global Allocator Instance
// =============================================================================
// This is set after relocation when the hypervisor is running from MDL memory.
// All VMX resource allocations then use this instead of kernel pool.

extern MDL_ALLOCATOR* g_MdlAllocator;

// =============================================================================
// Bootstrap Allocator
// =============================================================================
// Static allocator struct that lives in .data section - used before relocation.
// After relocation, this struct is part of the MDL-backed image.

extern MDL_ALLOCATOR g_BootstrapAllocator;

// =============================================================================
// Initialization and Destruction
// =============================================================================

// Initialize the MDL allocator by allocating all regions.
// Must be called after kernel symbols are resolved (needs MmAllocatePagesForMdlEx).
// Returns STATUS_SUCCESS on success, error code on failure.
NTSTATUS MdlAllocatorInit(MDL_ALLOCATOR* Allocator);

// Destroy the allocator and free all MDL regions.
// Should only be called during hypervisor unload.
void MdlAllocatorDestroy(MDL_ALLOCATOR* Allocator);

// =============================================================================
// Allocation Functions
// =============================================================================

// Allocate memory from the main region.
// Returns NULL if insufficient space.
void* MdlAlloc(MDL_ALLOCATOR* Allocator, U64 Size);

// Allocate aligned memory from the main region.
// Alignment must be power of 2.
void* MdlAllocAligned(MDL_ALLOCATOR* Allocator, U64 Size, U64 Alignment);

// Allocate a VMCS region (4KB aligned) from the VMCS region.
// Returns virtual address, fills outPhysical with physical address.
void* MdlAllocVmcs(MDL_ALLOCATOR* Allocator, U64* outPhysical);

// Allocate an EPT table (4KB aligned) from the EPT region.
// Returns virtual address, fills outPhysical with physical address.
void* MdlAllocEptTable(MDL_ALLOCATOR* Allocator, U64* outPhysical);

// Allocate from stack region (for host stacks).
void* MdlAllocStack(MDL_ALLOCATOR* Allocator, U64 Size, U64* outPhysical);

// Allocate MSR bitmap (4KB).
void* MdlAllocMsrBitmap(MDL_ALLOCATOR* Allocator, U64* outPhysical);

// Allocate from misc region.
void* MdlAllocMisc(MDL_ALLOCATOR* Allocator, U64 Size);

// =============================================================================
// Physical Address Helpers
// =============================================================================

// Get physical address for a virtual address within MDL regions.
// Returns 0 if address is not in any MDL region.
U64 MdlGetPhysicalAddress(void* VirtualAddress);

// Check if a virtual address is within MDL-managed memory.
bool MdlIsInRegion(MDL_ALLOCATOR* Allocator, void* VirtualAddress);

// =============================================================================
// Statistics
// =============================================================================

// Get allocation statistics for monitoring.
typedef struct _MDL_STATS {
    U64 MainUsed;
    U64 MainTotal;
    U64 VmcsUsed;
    U64 VmcsTotal;
    U64 EptUsed;
    U64 EptTotal;
    U64 StackUsed;
    U64 StackTotal;
    U64 MsrUsed;
    U64 MsrTotal;
    U64 MiscUsed;
    U64 MiscTotal;
} MDL_STATS;

void MdlGetStats(MDL_ALLOCATOR* Allocator, MDL_STATS* Stats);

#endif // MDL_ALLOC_H
