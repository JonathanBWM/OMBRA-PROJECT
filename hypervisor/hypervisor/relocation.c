// relocation.c - Hypervisor Self-Relocation
// OmbraHypervisor
//
// Handles relocating the hypervisor from IPRT-tagged memory (visible in BigPool)
// to MDL-backed memory (invisible in BigPool).

#include "relocation.h"
#include "mdl_alloc.h"
#include "kernel_resolve.h"

// =============================================================================
// Global Relocation Context
// =============================================================================

RELOCATION_CONTEXT g_RelocContext = {0};

// =============================================================================
// Memory Copy Helper (no CRT dependency)
// =============================================================================

static void MemCopy(void* dest, const void* src, U64 size) {
    U8* d = (U8*)dest;
    const U8* s = (const U8*)src;

    // Copy 8 bytes at a time when aligned
    while (size >= 8 && ((U64)d & 7) == 0 && ((U64)s & 7) == 0) {
        *(U64*)d = *(const U64*)s;
        d += 8;
        s += 8;
        size -= 8;
    }

    // Copy remaining bytes
    while (size > 0) {
        *d++ = *s++;
        size--;
    }
}

// =============================================================================
// Copy Image to MDL Memory
// =============================================================================

OMBRA_STATUS RelocateCopyToMdl(void) {
    OMBRA_STATUS status;
    void* destBase;
    U64 imageSize;
    IMAGE_DOS_HEADER* dosHeader;
    IMAGE_NT_HEADERS64* ntHeaders;
    IMAGE_DATA_DIRECTORY* relocDir;

    // Discover self info if not already done
    status = SelfInfoDiscover();
    if (OMBRA_FAILED(status)) {
        return status;
    }

    // Get source info
    g_RelocContext.SourceBase = SelfInfoGetBase();
    g_RelocContext.SourceSize = SelfInfoGetSize();
    g_RelocContext.SourcePhysical = SelfInfoGetPhysicalBase();

    if (!g_RelocContext.SourceBase || g_RelocContext.SourceSize == 0) {
        return OMBRA_ERROR_INVALID_STATE;
    }

    imageSize = g_RelocContext.SourceSize;

    // Validate MDL allocator is initialized
    if (!g_MdlAllocator || !g_MdlAllocator->Initialized) {
        // Try the bootstrap allocator
        if (!g_BootstrapAllocator.Initialized) {
            return OMBRA_ERROR_NOT_INITIALIZED;
        }
        g_MdlAllocator = &g_BootstrapAllocator;
    }

    // Allocate destination in MDL main region
    // Image must be page-aligned for proper execution
    destBase = MdlAllocAligned(g_MdlAllocator, imageSize, 4096);
    if (!destBase) {
        return OMBRA_ERROR_NO_MEMORY;
    }

    // Copy the entire image
    MemCopy(destBase, g_RelocContext.SourceBase, imageSize);

    // Store destination info
    g_RelocContext.DestBase = destBase;
    g_RelocContext.DestSize = imageSize;
    g_RelocContext.DestPhysical = MdlGetPhysicalAddress(destBase);

    // Calculate relocation delta
    g_RelocContext.Delta = (I64)((U8*)destBase - (U8*)g_RelocContext.SourceBase);

    // Find the relocation directory in the COPIED image
    dosHeader = (IMAGE_DOS_HEADER*)destBase;
    ntHeaders = (IMAGE_NT_HEADERS64*)((U8*)destBase + dosHeader->e_lfanew);
    relocDir = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

    if (relocDir->VirtualAddress != 0 && relocDir->Size != 0) {
        g_RelocContext.RelocDirectory = (U8*)destBase + relocDir->VirtualAddress;
        g_RelocContext.RelocDirectorySize = relocDir->Size;
    } else {
        // No relocations - image was linked with fixed base
        g_RelocContext.RelocDirectory = NULL;
        g_RelocContext.RelocDirectorySize = 0;
    }

    g_RelocContext.Copied = true;
    return OMBRA_SUCCESS;
}

// =============================================================================
// Apply PE Base Relocations
// =============================================================================

OMBRA_STATUS RelocateApplyFixups(void) {
    IMAGE_BASE_RELOCATION* relocBlock;
    U8* relocEnd;
    I64 delta;

    // Must have copied first
    if (!g_RelocContext.Copied) {
        return OMBRA_ERROR_INVALID_STATE;
    }

    // If no relocations needed (delta = 0 or no reloc directory), skip
    if (g_RelocContext.Delta == 0) {
        g_RelocContext.Relocated = true;
        return OMBRA_SUCCESS;
    }

    if (!g_RelocContext.RelocDirectory || g_RelocContext.RelocDirectorySize == 0) {
        // No relocation directory - cannot relocate
        // This is only OK if delta is 0, which we already handled
        return OMBRA_ERROR_NOT_SUPPORTED;
    }

    delta = g_RelocContext.Delta;
    relocBlock = (IMAGE_BASE_RELOCATION*)g_RelocContext.RelocDirectory;
    relocEnd = (U8*)g_RelocContext.RelocDirectory + g_RelocContext.RelocDirectorySize;

    // Process each relocation block
    while ((U8*)relocBlock < relocEnd && relocBlock->SizeOfBlock > 0) {
        U8* pageBase = (U8*)g_RelocContext.DestBase + relocBlock->VirtualAddress;
        U32 numEntries = (relocBlock->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(U16);
        U16* entries = (U16*)(relocBlock + 1);

        for (U32 i = 0; i < numEntries; i++) {
            U16 entry = entries[i];
            U16 type = entry >> 12;
            U16 offset = entry & 0xFFF;
            U8* patchAddr = pageBase + offset;

            switch (type) {
                case IMAGE_REL_BASED_ABSOLUTE:
                    // Padding entry, skip
                    break;

                case IMAGE_REL_BASED_DIR64:
                    // 64-bit absolute address - most common on x64
                    *(U64*)patchAddr += delta;
                    break;

                case IMAGE_REL_BASED_HIGHLOW:
                    // 32-bit absolute address
                    *(U32*)patchAddr += (U32)delta;
                    break;

                case IMAGE_REL_BASED_HIGH:
                    // High 16 bits of 32-bit address
                    *(U16*)patchAddr += (U16)(delta >> 16);
                    break;

                case IMAGE_REL_BASED_LOW:
                    // Low 16 bits of 32-bit address
                    *(U16*)patchAddr += (U16)delta;
                    break;

                default:
                    // Unknown relocation type - continue anyway
                    break;
            }
        }

        // Move to next block
        relocBlock = (IMAGE_BASE_RELOCATION*)((U8*)relocBlock + relocBlock->SizeOfBlock);
    }

    g_RelocContext.Relocated = true;
    return OMBRA_SUCCESS;
}

// =============================================================================
// Combined Relocate Self
// =============================================================================

OMBRA_STATUS RelocateSelf(void) {
    OMBRA_STATUS status;

    // Copy image to MDL memory
    status = RelocateCopyToMdl();
    if (OMBRA_FAILED(status)) {
        return status;
    }

    // Apply PE relocations
    status = RelocateApplyFixups();
    if (OMBRA_FAILED(status)) {
        return status;
    }

    return OMBRA_SUCCESS;
}

// =============================================================================
// Calculate Phase 2 Address
// =============================================================================

void* RelocateCalculatePhase2Address(void* phase1Function) {
    if (!g_RelocContext.Relocated || !phase1Function) {
        return NULL;
    }

    // Apply the delta to convert Phase 1 address to Phase 2 address
    return (U8*)phase1Function + g_RelocContext.Delta;
}

// =============================================================================
// Jump to Phase 2
// =============================================================================
//
// This is the critical transition point. We're about to jump to code in
// the relocated image.
//
// After Phase 2 completes (including VMLAUNCH on all CPUs), the hypervisor
// is active and all code runs as a guest. The original IPRT memory is still
// valid until the loader explicitly frees it after we return.

OMBRA_STATUS RelocateJumpToPhase2(FN_Phase2Entry phase2Entry) {
    OMBRA_STATUS status;

    // Mark that we're jumping (for debugging/diagnostics)
    g_RelocContext.Jumped = true;

    // Store Phase 2 entry in context (for reference)
    g_RelocContext.Phase2Entry = phase2Entry;

    // Call Phase 2 entry with relocation context
    // Phase 2 will:
    // 1. Allocate VMX resources from MDL memory
    // 2. Initialize EPT and hooks
    // 3. Broadcast VMLAUNCH to all CPUs
    // 4. Return success/failure
    //
    // After VMLAUNCH, the hypervisor is active and this thread continues
    // as a guest. It's safe to return to Phase 1 code because IPRT memory
    // hasn't been freed yet - the loader will free it after we return.
    status = phase2Entry(&g_RelocContext);

    // Return the Phase 2 result to the caller
    // The caller (HvEntry) will return this to the loader
    return status;
}

// =============================================================================
// Helper Functions
// =============================================================================

bool RelocateIsComplete(void) {
    return g_RelocContext.Relocated;
}

I64 RelocateGetDelta(void) {
    return g_RelocContext.Delta;
}
