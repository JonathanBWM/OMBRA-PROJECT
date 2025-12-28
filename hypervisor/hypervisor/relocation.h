// relocation.h - Hypervisor Self-Relocation
// OmbraHypervisor
//
// Handles relocating the hypervisor from IPRT-tagged memory (visible in BigPool)
// to MDL-backed memory (invisible in BigPool). After relocation, the IPRT memory
// can be freed to eliminate the detection vector.
//
// Flow:
// 1. Phase 1 runs from IPRT memory (allocated by Ld9BoxSup.sys)
// 2. MdlAllocatorInit() creates MDL regions
// 3. RelocateSelf() copies image to MDL memory and applies PE relocations
// 4. RelocateJumpToPhase2() transfers control to the relocated image
// 5. Phase 2 runs from MDL memory, signals loader to free IPRT memory

#ifndef RELOCATION_H
#define RELOCATION_H

#include "../shared/types.h"
#include "self_info.h"

// =============================================================================
// PE Base Relocation Structures
// =============================================================================

#pragma pack(push, 1)

// Base relocation block header
typedef struct _IMAGE_BASE_RELOCATION {
    U32 VirtualAddress;     // Page RVA
    U32 SizeOfBlock;        // Total size of block including entries
    // Followed by WORD entries: (type << 12) | offset
} IMAGE_BASE_RELOCATION;

#pragma pack(pop)

// Relocation types (from PE specification)
#define IMAGE_REL_BASED_ABSOLUTE        0   // Padding, skip
#define IMAGE_REL_BASED_HIGH            1   // High 16 bits
#define IMAGE_REL_BASED_LOW             2   // Low 16 bits
#define IMAGE_REL_BASED_HIGHLOW         3   // Full 32-bit
#define IMAGE_REL_BASED_HIGHADJ         4   // High 16 bits + adjustment
#define IMAGE_REL_BASED_DIR64           10  // Full 64-bit (x64)

// Data directory index for base relocations
#define IMAGE_DIRECTORY_ENTRY_BASERELOC 5

// =============================================================================
// Relocation Context
// =============================================================================
// Passed between phases to track relocation state

typedef struct _RELOCATION_CONTEXT {
    // Source (IPRT memory)
    void*   SourceBase;             // Original image base in IPRT memory
    U64     SourceSize;             // Original image size
    U64     SourcePhysical;         // Physical address of source

    // Destination (MDL memory)
    void*   DestBase;               // New image base in MDL memory
    U64     DestSize;               // Size copied (should match SourceSize)
    U64     DestPhysical;           // Physical address of destination

    // Relocation delta
    I64     Delta;                  // DestBase - SourceBase (for fixups)

    // PE info
    void*   RelocDirectory;         // Pointer to .reloc directory in dest
    U32     RelocDirectorySize;     // Size of relocation directory

    // Phase 2 entry point
    void*   Phase2Entry;            // Address of Phase 2 entry in dest image

    // State flags
    bool    Copied;                 // Image has been copied to MDL
    bool    Relocated;              // Relocations have been applied
    bool    Jumped;                 // Control transferred to Phase 2
} RELOCATION_CONTEXT;

// =============================================================================
// Global Relocation Context
// =============================================================================

extern RELOCATION_CONTEXT g_RelocContext;

// =============================================================================
// Phase 2 Entry Prototype
// =============================================================================
// Phase 2 entry function signature - called after relocation

typedef OMBRA_STATUS (*FN_Phase2Entry)(RELOCATION_CONTEXT* ctx);

// =============================================================================
// Functions
// =============================================================================

// Copy the hypervisor image to MDL memory.
// Uses SelfInfoDiscover() to find current image, MDL allocator for destination.
// Does NOT apply relocations yet.
// Returns OMBRA_SUCCESS if copy succeeded.
OMBRA_STATUS RelocateCopyToMdl(void);

// Apply PE base relocations to the copied image.
// Patches all absolute addresses with the relocation delta.
// Must be called after RelocateCopyToMdl().
// Returns OMBRA_SUCCESS if relocations applied successfully.
OMBRA_STATUS RelocateApplyFixups(void);

// Combine copy + fixups in one call.
// Returns OMBRA_SUCCESS if both stages succeeded.
OMBRA_STATUS RelocateSelf(void);

// Calculate the Phase 2 entry point address in the relocated image.
// phase1Function: Address of a function in Phase 1 image
// Returns: Corresponding address in relocated image, or NULL on error
void* RelocateCalculatePhase2Address(void* phase1Function);

// Jump to Phase 2 entry point in the relocated image.
// phase2Entry: Function to call in relocated image (use RelocateCalculatePhase2Address)
// Returns: The status returned by Phase 2 entry function
//
// NOTE: After Phase 2 completes and this function returns, the hypervisor is active
// and all code is running as a guest. The caller can safely return to the loader,
// which should then free the original IPRT memory.
OMBRA_STATUS RelocateJumpToPhase2(FN_Phase2Entry phase2Entry);

// Check if relocation has been completed.
bool RelocateIsComplete(void);

// Get the relocation delta (new_base - old_base).
// Useful for manually adjusting pointers after relocation.
I64 RelocateGetDelta(void);

#endif // RELOCATION_H
