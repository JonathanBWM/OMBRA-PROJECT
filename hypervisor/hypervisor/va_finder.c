// va_finder.c — Kernel VA Range Finder Implementation
// OmbraHypervisor
//
// Walks x64 page tables via physical memory reads to find unmapped kernel VA ranges.
// Uses EPT identity mapping (GPA == HPA) for physical memory access.

#include "va_finder.h"
#include "debug.h"
#include <string.h>

// =============================================================================
// Page Table Entry Bit Definitions
// =============================================================================

#define PTE_PRESENT         (1ULL << 0)   // Page is present
#define PTE_LARGE_PAGE      (1ULL << 7)   // 1GB/2MB page (PDPTE/PDE)
#define PT_ENTRY_MASK       0x000FFFFFFFFFF000ULL  // Physical address bits

// =============================================================================
// Helper: Read Physical Memory (via Hypervisor Identity Mapping)
// =============================================================================

// In VMX root mode, the hypervisor has its own CR3 with identity-mapped
// physical memory. The hypervisor page tables (set up in entry.c) map
// all physical memory 1:1 starting at a known base.
//
// This is NOT the same as EPT identity mapping (which is GPA→HPA).
// This is the hypervisor's own linear address space which maps physical memory.

// Hypervisor physical memory base (set during hypervisor init)
// This should be initialized to the base VA where physical memory is mapped
// in the hypervisor's page tables (typically 0xFFFF800000000000 or similar).
extern void* g_HvPhysicalMemoryBase;

static inline U64 ReadPhysical64(U64 physAddr) {
    // Validate physical address is within reasonable bounds
    // Max physical address on current x64 CPUs is typically 52 bits (4PB)
    if (physAddr > 0x000FFFFFFFFFFFFFULL) {
        return 0;  // Invalid physical address
    }

    // Calculate virtual address by adding to hypervisor's phys-to-virt base
    // The hypervisor must have set up identity mapping in its own page tables
    //
    // CRITICAL: g_HvPhysicalMemoryBase must be initialized before calling this!
    // If not initialized, fall back to assuming hypervisor identity maps at 0
    // (works if hypervisor uses low-memory identity mapping in early init)

    void* base = g_HvPhysicalMemoryBase;
    if (base == NULL) {
        // Fallback: assume hypervisor page tables have identity mapping at 0
        // This works during early init before full mapping is set up
        // DANGER: Only safe for low physical addresses < 512GB
        base = (void*)0;
    }

    U64* ptr = (U64*)((U64)base + physAddr);
    return *ptr;
}

// Weak definition - will be overridden by actual definition in entry.c or vmx.c
#pragma weak g_HvPhysicalMemoryBase
void* g_HvPhysicalMemoryBase = NULL;

// =============================================================================
// Page Table Walking
// =============================================================================

// Walk x64 page tables to check if a VA is mapped
// Returns true if the VA has a present PTE
bool VaIsMapped(U64 cr3, U64 virtualAddr) {
    U64 pml4Physical;
    U64 pdptPhysical;
    U64 pdPhysical;
    U64 ptPhysical;
    U64 pml4Index, pdptIndex, pdIndex, ptIndex;
    U64 pml4e, pdpte, pde, pte;

    // CR3 contains physical address of PML4 (bits 51:12)
    pml4Physical = cr3 & 0x000FFFFFFFFFF000ULL;

    // Extract indices from VA
    // VA format: [63:48]=sign ext, [47:39]=PML4, [38:30]=PDPT, [29:21]=PD, [20:12]=PT, [11:0]=offset
    pml4Index = (virtualAddr >> 39) & 0x1FF;
    pdptIndex = (virtualAddr >> 30) & 0x1FF;
    pdIndex   = (virtualAddr >> 21) & 0x1FF;
    ptIndex   = (virtualAddr >> 12) & 0x1FF;

    // =========================================================================
    // 1. Read PML4E
    // =========================================================================
    pml4e = ReadPhysical64(pml4Physical + (pml4Index * 8));

    // Check present bit
    if (!(pml4e & PTE_PRESENT)) {
        return false;  // PML4E not present - VA unmapped
    }

    // Get PDPT physical address
    pdptPhysical = pml4e & PT_ENTRY_MASK;

    // =========================================================================
    // 2. Read PDPTE
    // =========================================================================
    pdpte = ReadPhysical64(pdptPhysical + (pdptIndex * 8));

    if (!(pdpte & PTE_PRESENT)) {
        return false;  // PDPTE not present - VA unmapped
    }

    // Check for 1GB page
    if (pdpte & PTE_LARGE_PAGE) {
        return true;  // 1GB page present - VA mapped
    }

    // Get PD physical address
    pdPhysical = pdpte & PT_ENTRY_MASK;

    // =========================================================================
    // 3. Read PDE
    // =========================================================================
    pde = ReadPhysical64(pdPhysical + (pdIndex * 8));

    if (!(pde & PTE_PRESENT)) {
        return false;  // PDE not present - VA unmapped
    }

    // Check for 2MB page
    if (pde & PTE_LARGE_PAGE) {
        return true;  // 2MB page present - VA mapped
    }

    // Get PT physical address
    ptPhysical = pde & PT_ENTRY_MASK;

    // =========================================================================
    // 4. Read PTE
    // =========================================================================
    pte = ReadPhysical64(ptPhysical + (ptIndex * 8));

    if (!(pte & PTE_PRESENT)) {
        return false;  // PTE not present - VA unmapped
    }

    return true;  // 4KB page present - VA mapped
}

// =============================================================================
// Range Checking
// =============================================================================

// Check if an entire VA range is unmapped
bool VaIsRangeUnmapped(U64 cr3, U64 startVa, U64 size) {
    U64 currentVa;
    U64 endVa;

    if (size == 0) {
        return false;
    }

    endVa = startVa + size;

    // Scan in 2MB strides for efficiency
    // We only need to check if ANY part is mapped to reject the range
    for (currentVa = startVa; currentVa < endVa; currentVa += KERNEL_VA_ALIGNMENT) {
        if (VaIsMapped(cr3, currentVa)) {
            return false;  // Found a mapped page - range not suitable
        }
    }

    // Also check the end address if not aligned
    if ((endVa & (KERNEL_VA_ALIGNMENT - 1)) != 0) {
        if (VaIsMapped(cr3, endVa - PAGE_SIZE)) {
            return false;
        }
    }

    return true;  // Entire range is unmapped
}

// =============================================================================
// Range Finding
// =============================================================================

// Find an unused kernel VA range of the specified size
U64 VaFindUnusedRange(U64 cr3, U64 sizeNeeded, U64 alignment) {
    U64 currentVa;
    U64 alignedSize;
    U64 searchStart, searchEnd;

    // Validate parameters
    if (sizeNeeded == 0 || alignment == 0) {
        ERR("VaFinder: Invalid parameters (size=%llu, align=%llu)", sizeNeeded, alignment);
        return 0;
    }

    // Align size up to alignment boundary
    alignedSize = ALIGN_UP(sizeNeeded, alignment);

    // Use predefined search range
    searchStart = KERNEL_VA_SEARCH_START;
    searchEnd = KERNEL_VA_SEARCH_END;

    TRACE("VaFinder: Searching for %llu bytes (aligned=%llu) in range 0x%llx - 0x%llx",
          sizeNeeded, alignedSize, searchStart, searchEnd);

    // Scan the search range with alignment-sized strides
    for (currentVa = searchStart;
         currentVa + alignedSize <= searchEnd;
         currentVa += alignment) {

        // Check if this range is completely unmapped
        if (VaIsRangeUnmapped(cr3, currentVa, alignedSize)) {
            INFO("VaFinder: Found unused range at 0x%llx (size=%llu)",
                 currentVa, alignedSize);
            return currentVa;
        }
    }

    // No suitable range found
    WARN("VaFinder: No unused range found for size=%llu", alignedSize);
    return 0;
}
