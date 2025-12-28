/**
 * 01_ept_structures.h - EPT Entry Structures and Definitions
 * 
 * Pure C definitions for Extended Page Tables.
 * Reference: Intel SDM Volume 3, Chapter 28
 * 
 * CONCEPTS COVERED:
 * - EPT entry format (PML4E, PDPTE, PDE, PTE)
 * - EPTP (EPT Pointer) format
 * - Permission bits
 * - Memory types
 * - Index extraction macros
 */

#ifndef OMBRA_EPT_STRUCTURES_H
#define OMBRA_EPT_STRUCTURES_H

#include "../common/types.h"

/*===========================================================================
 * EPT Entry Permission Bits
 *===========================================================================*/

#define EPT_READ                BIT(0)
#define EPT_WRITE               BIT(1)
#define EPT_EXECUTE             BIT(2)

/* Combined permissions */
#define EPT_NONE                0
#define EPT_RO                  (EPT_READ)
#define EPT_WO                  (EPT_WRITE)
#define EPT_XO                  (EPT_EXECUTE)       /* Execute-only (Intel only!) */
#define EPT_RW                  (EPT_READ | EPT_WRITE)
#define EPT_RX                  (EPT_READ | EPT_EXECUTE)
#define EPT_WX                  (EPT_WRITE | EPT_EXECUTE)
#define EPT_RWX                 (EPT_READ | EPT_WRITE | EPT_EXECUTE)

/*===========================================================================
 * EPT Memory Types (bits 5:3 of PTE)
 *===========================================================================*/

#define EPT_MT_UC               0   /* Uncacheable */
#define EPT_MT_WC               1   /* Write Combining */
#define EPT_MT_WT               4   /* Write Through */
#define EPT_MT_WP               5   /* Write Protected */
#define EPT_MT_WB               6   /* Write Back (default for RAM) */

/*===========================================================================
 * EPT Page Sizes
 *===========================================================================*/

#define EPT_PAGE_4KB            (4ULL * 1024)
#define EPT_PAGE_2MB            (2ULL * 1024 * 1024)
#define EPT_PAGE_1GB            (1ULL * 1024 * 1024 * 1024)

#define EPT_ENTRIES_PER_TABLE   512

/*===========================================================================
 * GPA Index Extraction Macros
 *===========================================================================
 * 48-bit guest physical address breakdown:
 * [47:39] PML4 index  (9 bits, 512 entries)
 * [38:30] PDPT index  (9 bits, 512 entries)
 * [29:21] PD index    (9 bits, 512 entries)
 * [20:12] PT index    (9 bits, 512 entries)
 * [11:0]  Page offset (12 bits, 4KB)
 */

#define EPT_PML4_INDEX(gpa)     (((gpa) >> 39) & 0x1FF)
#define EPT_PDPT_INDEX(gpa)     (((gpa) >> 30) & 0x1FF)
#define EPT_PD_INDEX(gpa)       (((gpa) >> 21) & 0x1FF)
#define EPT_PT_INDEX(gpa)       (((gpa) >> 12) & 0x1FF)
#define EPT_PAGE_OFFSET(gpa)    ((gpa) & 0xFFF)

/* For 2MB pages */
#define EPT_2MB_OFFSET(gpa)     ((gpa) & 0x1FFFFF)

/* For 1GB pages */
#define EPT_1GB_OFFSET(gpa)     ((gpa) & 0x3FFFFFFF)

/*===========================================================================
 * EPT PML4 Entry (PML4E)
 *===========================================================================
 * Points to a Page Directory Pointer Table (PDPT)
 */

typedef union _EPT_PML4E {
    U64 Value;
    struct {
        U64 Read            : 1;    /* Bit 0: Read access */
        U64 Write           : 1;    /* Bit 1: Write access */
        U64 Execute         : 1;    /* Bit 2: Execute access */
        U64 Reserved1       : 5;    /* Bits 7:3: Reserved (MBZ) */
        U64 Accessed        : 1;    /* Bit 8: Accessed (if enabled) */
        U64 Ignored1        : 1;    /* Bit 9: Ignored */
        U64 ExecuteUser     : 1;    /* Bit 10: User-mode execute (MBEC) */
        U64 Ignored2        : 1;    /* Bit 11: Ignored */
        U64 PdptPhysAddr    : 40;   /* Bits 51:12: PDPT physical address >> 12 */
        U64 Ignored3        : 12;   /* Bits 63:52: Ignored */
    };
} EPT_PML4E;

/*===========================================================================
 * EPT PDPT Entry (PDPTE)
 *===========================================================================
 * Can be either:
 * - Pointer to Page Directory (LargePage = 0)
 * - 1GB page mapping (LargePage = 1)
 */

typedef union _EPT_PDPTE {
    U64 Value;
    
    /* Non-large page (points to PD) */
    struct {
        U64 Read            : 1;
        U64 Write           : 1;
        U64 Execute         : 1;
        U64 Reserved1       : 5;
        U64 Accessed        : 1;
        U64 Ignored1        : 1;
        U64 ExecuteUser     : 1;
        U64 Ignored2        : 1;
        U64 PdPhysAddr      : 40;   /* PD physical address >> 12 */
        U64 Ignored3        : 12;
    };
    
    /* 1GB large page */
    struct {
        U64 ReadLarge       : 1;
        U64 WriteLarge      : 1;
        U64 ExecuteLarge    : 1;
        U64 MemoryType      : 3;    /* Bits 5:3: Memory type */
        U64 IgnorePat       : 1;    /* Bit 6: Ignore PAT */
        U64 LargePage       : 1;    /* Bit 7: 1 = 1GB page */
        U64 AccessedLarge   : 1;
        U64 Dirty           : 1;    /* Bit 9: Dirty (if enabled) */
        U64 ExecuteUserLrg  : 1;
        U64 Ignored4        : 1;
        U64 Reserved2       : 18;   /* Bits 29:12: Reserved (MBZ for 1GB) */
        U64 PagePhysAddr    : 22;   /* Bits 51:30: Physical address >> 30 */
        U64 Ignored5        : 12;
    } LargePage;
    
} EPT_PDPTE;

/*===========================================================================
 * EPT PD Entry (PDE)
 *===========================================================================
 * Can be either:
 * - Pointer to Page Table (LargePage = 0)
 * - 2MB page mapping (LargePage = 1)
 */

typedef union _EPT_PDE {
    U64 Value;
    
    /* Non-large page (points to PT) */
    struct {
        U64 Read            : 1;
        U64 Write           : 1;
        U64 Execute         : 1;
        U64 Reserved1       : 5;
        U64 Accessed        : 1;
        U64 Ignored1        : 1;
        U64 ExecuteUser     : 1;
        U64 Ignored2        : 1;
        U64 PtPhysAddr      : 40;   /* PT physical address >> 12 */
        U64 Ignored3        : 12;
    };
    
    /* 2MB large page */
    struct {
        U64 ReadLarge       : 1;
        U64 WriteLarge      : 1;
        U64 ExecuteLarge    : 1;
        U64 MemoryType      : 3;
        U64 IgnorePat       : 1;
        U64 LargePage       : 1;    /* Bit 7: 1 = 2MB page */
        U64 AccessedLarge   : 1;
        U64 Dirty           : 1;
        U64 ExecuteUserLrg  : 1;
        U64 Ignored4        : 1;
        U64 Reserved2       : 9;    /* Bits 20:12: Reserved (MBZ for 2MB) */
        U64 PagePhysAddr    : 31;   /* Bits 51:21: Physical address >> 21 */
        U64 Ignored5        : 12;
    } LargePage;
    
} EPT_PDE;

/*===========================================================================
 * EPT PT Entry (PTE) - 4KB Page
 *===========================================================================
 */

typedef union _EPT_PTE {
    U64 Value;
    struct {
        U64 Read            : 1;    /* Bit 0: Read access */
        U64 Write           : 1;    /* Bit 1: Write access */
        U64 Execute         : 1;    /* Bit 2: Execute access */
        U64 MemoryType      : 3;    /* Bits 5:3: Memory type */
        U64 IgnorePat       : 1;    /* Bit 6: Ignore PAT */
        U64 Ignored1        : 1;    /* Bit 7: Ignored */
        U64 Accessed        : 1;    /* Bit 8: Accessed */
        U64 Dirty           : 1;    /* Bit 9: Dirty */
        U64 ExecuteUser     : 1;    /* Bit 10: User-mode execute */
        U64 Ignored2        : 1;    /* Bit 11: Ignored */
        U64 PagePhysAddr    : 40;   /* Bits 51:12: Physical address >> 12 */
        U64 Ignored3        : 11;   /* Bits 62:52: Ignored */
        U64 SuppressVe      : 1;    /* Bit 63: Suppress #VE */
    };
} EPT_PTE;

/*===========================================================================
 * EPTP (EPT Pointer) Format
 *===========================================================================
 * Stored in VMCS field VMCS_CTRL_EPT_POINTER (0x201A)
 */

typedef union _EPT_POINTER {
    U64 Value;
    struct {
        U64 MemoryType      : 3;    /* Bits 2:0: Memory type for EPT walk */
        U64 PageWalkLength  : 3;    /* Bits 5:3: Page walk length - 1 (3 = 4 levels) */
        U64 AccessedDirty   : 1;    /* Bit 6: Enable accessed/dirty flags */
        U64 SupervisorSS    : 1;    /* Bit 7: Supervisor shadow stack */
        U64 Reserved        : 4;    /* Bits 11:8: Reserved (MBZ) */
        U64 Pml4PhysAddr    : 52;   /* Bits 63:12: PML4 physical address >> 12 */
    };
} EPT_POINTER;

/**
 * Build EPTP value
 * 
 * @param pml4Physical  Physical address of PML4 table (4KB aligned)
 * @param enableAD      Enable accessed/dirty flags
 * @return EPTP value for VMCS
 */
static inline U64 EptBuildPointer(U64 pml4Physical, bool enableAD)
{
    EPT_POINTER eptp;
    
    eptp.Value = 0;
    eptp.MemoryType = EPT_MT_WB;        /* Write-back for EPT walk */
    eptp.PageWalkLength = 3;            /* 4-level paging (3 = 4-1) */
    eptp.AccessedDirty = enableAD ? 1 : 0;
    eptp.Pml4PhysAddr = pml4Physical >> 12;
    
    return eptp.Value;
}

/*===========================================================================
 * EPT State Structure
 *===========================================================================*/

typedef struct _EPT_STATE {
    /* Page table pointers (virtual addresses) */
    EPT_PML4E*  Pml4;
    EPT_PDPTE*  Pdpt;       /* For identity map: single PDPT for first 512GB */
    
    /* Physical addresses */
    U64         Pml4Physical;
    U64         PdptPhysical;
    
    /* EPTP value */
    U64         Eptp;
    
    /* State */
    bool        Initialized;
    
    /* Statistics */
    U32         SplitCount;     /* Number of large pages split to 4KB */
    
} EPT_STATE;

/*===========================================================================
 * INVEPT Definitions
 *===========================================================================*/

typedef struct _INVEPT_DESC {
    U64 EptPointer;
    U64 Reserved;
} INVEPT_DESC;

#define INVEPT_TYPE_SINGLE_CONTEXT  1
#define INVEPT_TYPE_ALL_CONTEXTS    2

/* Assembly function */
extern void AsmInvept(U64 type, INVEPT_DESC* descriptor);

/**
 * Invalidate EPT translations
 */
static inline void EptInvalidate(U64 eptp, U64 type)
{
    INVEPT_DESC desc;
    desc.EptPointer = eptp;
    desc.Reserved = 0;
    AsmInvept(type, &desc);
}

#endif /* OMBRA_EPT_STRUCTURES_H */
