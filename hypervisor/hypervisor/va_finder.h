// va_finder.h — Kernel VA Range Finder
// OmbraHypervisor
//
// Finds unused kernel virtual address ranges for EPT-only memory.
// Scans kernel page tables to find unmapped regions.

#ifndef OMBRA_VA_FINDER_H
#define OMBRA_VA_FINDER_H

#include "../shared/types.h"

// =============================================================================
// Helper Macros
// =============================================================================

#ifndef PAGE_SIZE
#define PAGE_SIZE           0x1000ULL
#endif

#ifndef ALIGN_UP
#define ALIGN_UP(val, align) (((val) + ((align) - 1)) & ~((align) - 1))
#endif

// =============================================================================
// Configuration
// =============================================================================

// Kernel VA range to search (safe range typically unused)
#define KERNEL_VA_SEARCH_START  0xFFFF888000000000ULL
#define KERNEL_VA_SEARCH_END    0xFFFFF00000000000ULL
#define KERNEL_VA_ALIGNMENT     0x200000ULL  // 2MB aligned

// Find an unused kernel VA range of the specified size
// Returns 0 if no suitable range found
U64 VaFindUnusedRange(
    U64 cr3,            // Kernel CR3 for page table walk
    U64 sizeNeeded,     // Bytes needed (will be page-aligned)
    U64 alignment       // Alignment requirement (typically 2MB)
);

// Check if a specific VA range is unmapped in kernel page tables
bool VaIsRangeUnmapped(
    U64 cr3,
    U64 startVa,
    U64 size
);

// Walk kernel page tables to check if VA is mapped
// Returns true if PTE exists and is present
bool VaIsMapped(U64 cr3, U64 virtualAddr);

#endif // OMBRA_VA_FINDER_H
