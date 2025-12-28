// ept.h — Extended Page Tables Management
// OmbraHypervisor

#ifndef OMBRA_EPT_H
#define OMBRA_EPT_H

#include "../shared/types.h"
#include "../shared/ept_defs.h"

// =============================================================================
// Configuration
// =============================================================================

// Use 1GB pages for identity map (set to 0 for 2MB pages)
#define EPT_USE_1GB_PAGES       1

// Total physical memory to map (512GB)
#define EPT_IDENTITY_MAP_SIZE   (512ULL * 1024 * 1024 * 1024)

// =============================================================================
// EPT State Structure
// =============================================================================

typedef struct _EPT_STATE {
    // PML4 table (root of EPT hierarchy)
    EPT_PML4E*  Pml4;
    U64         Pml4Physical;

    // PDPT table(s) - 1 for 512GB with 1GB pages
    EPT_PDPTE*  Pdpt;
    U64         PdptPhysical;

    // Pre-computed EPTP value
    U64         Eptp;

    // Split page tables
    // When a 1GB page is split, we allocate a PD table (512 x 2MB entries)
    // When a 2MB page is split, we allocate a PT table (512 x 4KB entries)
    // These arrays track which PDPT entries have been split
    void*       SplitPdTables[EPT_ENTRIES_PER_TABLE];   // Virtual addresses of PD tables
    void*       SplitPtTables[EPT_ENTRIES_PER_TABLE];   // Virtual addresses of PT tables (future)

    // Tracking for hook support
    U32         HookCount;
    bool        Initialized;

    // Page splitting counters (for hook framework)
    U32         SplitPdCount;   // Number of 1GB pages split to 2MB (PDs allocated)
    U32         SplitPtCount;   // Number of 2MB pages split to 4KB (PTs allocated)

    // Memory allocation tracking (for safe page splitting)
    void*       EptMemoryBase;          // Base virtual address of EPT memory pool
    U64         EptMemoryPhysical;      // Base physical address of EPT memory pool
    U32         TotalPagesAllocated;    // Total 4KB pages allocated for EPT region
    U32         PagesUsed;              // Pages currently in use (PML4 + PDPT + splits)
} EPT_STATE;

// =============================================================================
// EPT Initialization
// =============================================================================

// Initialize EPT with identity mapping
// The caller must provide pre-allocated page-aligned memory:
//   - pml4Virtual/pml4Physical: 4KB for PML4
//   - pdptVirtual/pdptPhysical: 4KB for PDPT (512 entries)
//   - totalPagesAllocated: Total 4KB pages in the EPT memory region
//     (must be >= 2 for PML4+PDPT, recommended 512 for splits)
OMBRA_STATUS EptInitialize(
    EPT_STATE* ept,
    void* pml4Virtual,
    U64 pml4Physical,
    void* pdptVirtual,
    U64 pdptPhysical,
    U32 totalPagesAllocated
);

// Destroy EPT (does not free memory - caller's responsibility)
void EptDestroy(EPT_STATE* ept);

// =============================================================================
// EPT Manipulation
// =============================================================================

// Get the EPT pointer value for VMCS
U64 EptGetEptp(EPT_STATE* ept);

// Modify EPT entry for a specific GPA
// Returns the old entry value
OMBRA_STATUS EptModifyPage(
    EPT_STATE* ept,
    U64 guestPhysical,
    U64 hostPhysical,
    U64 permissions,
    U64 memoryType
);

// Split a large page into smaller pages (for fine-grained hooks)
// Not implemented in Phase 4 - needed for Phase 6 (Hook framework)
OMBRA_STATUS EptSplitLargePage(EPT_STATE* ept, U64 guestPhysical);

// Split a 1GB page into 512 x 2MB pages
// This allocates a PD table from the EPT memory pool and updates the PDPTE
OMBRA_STATUS EptSplit1GbTo2Mb(EPT_STATE* ept, U64 guestPhysical);

// Split a 2MB page into 512 x 4KB pages
// This allocates a PT table from the EPT memory pool and updates the PDE
// The 1GB page containing this 2MB page must be split to 2MB first
OMBRA_STATUS EptSplit2MbTo4Kb(EPT_STATE* ept, U64 guestPhysical);

// =============================================================================
// EPT 4KB Page Operations
// =============================================================================

// Get the EPT PTE for a 4KB page
// Returns NULL if not split to 4KB level yet
EPT_PTE* EptGet4KbEntry(EPT_STATE* ept, U64 guestPhysical);

// Set EPT permissions on a specific 4KB page
// Automatically splits large pages if needed
// permissions: EPT_READ, EPT_WRITE, EPT_EXECUTE flags (or EPT_X_ONLY for execute-only)
OMBRA_STATUS EptSetPagePermissions(
    EPT_STATE* ept,
    U64 guestPhysical,
    U32 permissions
);

// Map guest physical to different host physical
// Used for EPT-only memory where we redirect GPAs to our physical pool
// Automatically splits to 4KB pages
OMBRA_STATUS EptMapGuestToHost(
    EPT_STATE* ept,
    U64 guestPhysical,
    U64 hostPhysical,
    U32 permissions,
    U8 memoryType
);

// Remove EPT mapping (set entry to not-present)
// Automatically splits to 4KB pages
OMBRA_STATUS EptUnmapPage(EPT_STATE* ept, U64 guestPhysical);

// =============================================================================
// EPT Query Functions
// =============================================================================

// Get EPT entry for a GPA (returns NULL if not mapped)
U64* EptGetEntry(EPT_STATE* ept, U64 guestPhysical);

// Check if EPT supports 1GB pages
bool EptSupports1GbPages(void);

// Check if EPT supports execute-only pages (R=0, W=0, X=1)
// Required for stealth mode - without this, payloads are readable by memory scans
bool EptSupportsExecuteOnly(void);

// Get safe execute permission for payload pages
// Returns EPT_EXECUTE (X-only) if supported, EPT_READ|EPT_EXECUTE (RX) if not
// ALWAYS use this instead of hardcoding EPT_EXECUTE for stealth-critical code
U32 EptGetSafeExecutePermission(void);

// =============================================================================
// EPT Self-Protection
// =============================================================================

// Hide hypervisor memory from guest reads
// Maps HV pages to blank page with execute-only permission
OMBRA_STATUS EptProtectSelf(
    EPT_STATE* ept,
    U64 hvPhysBase,
    U64 hvPhysSize,
    U64 blankPagePhys
);

// =============================================================================
// INVEPT Support
// =============================================================================

// INVEPT descriptor for single-context or all-context invalidation
typedef struct _INVEPT_DESCRIPTOR {
    U64 EptPointer;
    U64 Reserved;
} INVEPT_DESCRIPTOR;

#define INVEPT_TYPE_SINGLE_CONTEXT  1
#define INVEPT_TYPE_ALL_CONTEXT     2

// Invalidate EPT mappings
void EptInvalidate(EPT_STATE* ept, U64 inveptType);

#endif // OMBRA_EPT_H
