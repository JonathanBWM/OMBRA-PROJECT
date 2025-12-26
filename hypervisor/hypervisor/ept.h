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

    // For 2MB pages, we'd also need PD tables
    // For 4KB pages, we'd also need PT tables

    // Tracking for hook support
    U32         HookCount;
    bool        Initialized;

    // Page splitting counters (for hook framework)
    U32         SplitPdCount;   // Number of 1GB pages split to 2MB (PDs allocated)
    U32         SplitPtCount;   // Number of 2MB pages split to 4KB (PTs allocated)
} EPT_STATE;

// =============================================================================
// EPT Initialization
// =============================================================================

// Initialize EPT with identity mapping
// The caller must provide pre-allocated page-aligned memory:
//   - pml4Virtual/pml4Physical: 4KB for PML4
//   - pdptVirtual/pdptPhysical: 4KB for PDPT (512 entries)
OMBRA_STATUS EptInitialize(
    EPT_STATE* ept,
    void* pml4Virtual,
    U64 pml4Physical,
    void* pdptVirtual,
    U64 pdptPhysical
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

// =============================================================================
// EPT Query Functions
// =============================================================================

// Get EPT entry for a GPA (returns NULL if not mapped)
U64* EptGetEntry(EPT_STATE* ept, U64 guestPhysical);

// Check if EPT supports 1GB pages
bool EptSupports1GbPages(void);

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
