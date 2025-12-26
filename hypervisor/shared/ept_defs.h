// ept_defs.h — Extended Page Table Definitions
// OmbraHypervisor
// Reference: Intel SDM Volume 3, Chapter 28.3

#ifndef OMBRA_EPT_DEFS_H
#define OMBRA_EPT_DEFS_H

#include "types.h"

// =============================================================================
// EPT Permission Bits
// =============================================================================

#define EPT_READ            (1ULL << 0)
#define EPT_WRITE           (1ULL << 1)
#define EPT_EXECUTE         (1ULL << 2)
#define EPT_RWX             (EPT_READ | EPT_WRITE | EPT_EXECUTE)
#define EPT_RW              (EPT_READ | EPT_WRITE)
#define EPT_RX              (EPT_READ | EPT_EXECUTE)
#define EPT_X_ONLY          (EPT_EXECUTE)  // Execute-only (mode-based EPT)

// =============================================================================
// EPT Entry Flags
// =============================================================================

#define EPT_LARGE_PAGE      (1ULL << 7)   // 1GB or 2MB page
#define EPT_ACCESSED        (1ULL << 8)   // If A/D enabled
#define EPT_DIRTY           (1ULL << 9)   // If A/D enabled
#define EPT_EXECUTE_USER    (1ULL << 10)  // Execute for user-mode

// =============================================================================
// EPT Memory Types (bits 5:3 for leaf entries)
// =============================================================================

#define EPT_MEMORY_TYPE_UC      0   // Uncacheable
#define EPT_MEMORY_TYPE_WC      1   // Write Combining
#define EPT_MEMORY_TYPE_WT      4   // Write Through
#define EPT_MEMORY_TYPE_WP      5   // Write Protected
#define EPT_MEMORY_TYPE_WB      6   // Write Back

#define EPT_MEMORY_TYPE_SHIFT   3
#define EPT_MEMORY_TYPE_MASK    (7ULL << EPT_MEMORY_TYPE_SHIFT)

// =============================================================================
// Page Sizes
// =============================================================================

#define EPT_PAGE_SIZE_4K        0x1000ULL           // 4KB
#define EPT_PAGE_SIZE_2M        0x200000ULL         // 2MB
#define EPT_PAGE_SIZE_1G        0x40000000ULL       // 1GB

// =============================================================================
// GPA to Index Conversion
// =============================================================================

#define EPT_PML4_INDEX(gpa)     (((gpa) >> 39) & 0x1FF)
#define EPT_PDPT_INDEX(gpa)     (((gpa) >> 30) & 0x1FF)
#define EPT_PD_INDEX(gpa)       (((gpa) >> 21) & 0x1FF)
#define EPT_PT_INDEX(gpa)       (((gpa) >> 12) & 0x1FF)

// Number of entries per table
#define EPT_ENTRIES_PER_TABLE   512

// =============================================================================
// EPT Entry Structures
// =============================================================================

// PML4 Entry (points to PDPT)
typedef union _EPT_PML4E {
    U64 Value;
    struct {
        U64 Read            : 1;    // Bit 0
        U64 Write           : 1;    // Bit 1
        U64 Execute         : 1;    // Bit 2
        U64 Reserved1       : 5;    // Bits 7:3 (MBZ for non-leaf)
        U64 Accessed        : 1;    // Bit 8
        U64 Ignored1        : 1;    // Bit 9
        U64 ExecuteUser     : 1;    // Bit 10
        U64 Ignored2        : 1;    // Bit 11
        U64 PdptPhysAddr    : 40;   // Bits 51:12 (physical address >> 12)
        U64 Ignored3        : 12;   // Bits 63:52
    };
} EPT_PML4E;

// PDPT Entry - can point to PD or be a 1GB page
typedef union _EPT_PDPTE {
    U64 Value;
    struct {
        U64 Read            : 1;    // Bit 0
        U64 Write           : 1;    // Bit 1
        U64 Execute         : 1;    // Bit 2
        U64 MemoryType      : 3;    // Bits 5:3 (for 1GB pages)
        U64 IgnorePat       : 1;    // Bit 6 (for 1GB pages)
        U64 LargePage       : 1;    // Bit 7 (1 = 1GB page)
        U64 Accessed        : 1;    // Bit 8
        U64 Dirty           : 1;    // Bit 9 (for 1GB pages)
        U64 ExecuteUser     : 1;    // Bit 10
        U64 Ignored1        : 1;    // Bit 11
        U64 Reserved1       : 18;   // Bits 29:12 (MBZ for 1GB pages)
        U64 PagePhysAddr    : 22;   // Bits 51:30 (1GB page phys >> 30)
        U64 Ignored2        : 11;   // Bits 62:52
        U64 SuppressVE      : 1;    // Bit 63
    } LargePage;
    struct {
        U64 Read            : 1;
        U64 Write           : 1;
        U64 Execute         : 1;
        U64 Reserved1       : 5;
        U64 Accessed        : 1;
        U64 Ignored1        : 1;
        U64 ExecuteUser     : 1;
        U64 Ignored2        : 1;
        U64 PdPhysAddr      : 40;   // PD physical address >> 12
        U64 Ignored3        : 12;
    } Pointer;
} EPT_PDPTE;

// PD Entry - can point to PT or be a 2MB page
typedef union _EPT_PDE {
    U64 Value;
    struct {
        U64 Read            : 1;
        U64 Write           : 1;
        U64 Execute         : 1;
        U64 MemoryType      : 3;
        U64 IgnorePat       : 1;
        U64 LargePage       : 1;    // 1 = 2MB page
        U64 Accessed        : 1;
        U64 Dirty           : 1;
        U64 ExecuteUser     : 1;
        U64 Ignored1        : 1;
        U64 Reserved1       : 9;    // MBZ for 2MB pages
        U64 PagePhysAddr    : 31;   // 2MB page phys >> 21
        U64 Ignored2        : 11;
        U64 SuppressVE      : 1;
    } LargePage;
    struct {
        U64 Read            : 1;
        U64 Write           : 1;
        U64 Execute         : 1;
        U64 Reserved1       : 5;
        U64 Accessed        : 1;
        U64 Ignored1        : 1;
        U64 ExecuteUser     : 1;
        U64 Ignored2        : 1;
        U64 PtPhysAddr      : 40;
        U64 Ignored3        : 12;
    } Pointer;
} EPT_PDE;

// PT Entry - 4KB page
typedef union _EPT_PTE {
    U64 Value;
    struct {
        U64 Read            : 1;
        U64 Write           : 1;
        U64 Execute         : 1;
        U64 MemoryType      : 3;
        U64 IgnorePat       : 1;
        U64 Ignored1        : 1;
        U64 Accessed        : 1;
        U64 Dirty           : 1;
        U64 ExecuteUser     : 1;
        U64 Ignored2        : 1;
        U64 PagePhysAddr    : 40;   // 4KB page phys >> 12
        U64 Ignored3        : 11;
        U64 SuppressVE      : 1;
    };
} EPT_PTE;

// =============================================================================
// EPT Violation Qualification (Intel SDM Vol 3, Table 27-7)
// =============================================================================

#define EPT_VIOLATION_READ          (1ULL << 0)
#define EPT_VIOLATION_WRITE         (1ULL << 1)
#define EPT_VIOLATION_EXECUTE       (1ULL << 2)
#define EPT_VIOLATION_READABLE      (1ULL << 3)
#define EPT_VIOLATION_WRITABLE      (1ULL << 4)
#define EPT_VIOLATION_EXECUTABLE    (1ULL << 5)
#define EPT_VIOLATION_GPA_VALID     (1ULL << 7)
#define EPT_VIOLATION_LINEAR_VALID  (1ULL << 8)

#endif // OMBRA_EPT_DEFS_H
