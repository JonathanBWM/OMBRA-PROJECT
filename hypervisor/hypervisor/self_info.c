// self_info.c - PE Image Self-Discovery
// OmbraHypervisor
//
// Discovers the hypervisor's own PE base and size at runtime using __ImageBase.
// This allows the hypervisor to protect itself via EPT without loader assistance.

#include "self_info.h"
#include "kernel_resolve.h"

// =============================================================================
// __ImageBase - Linker-provided symbol for PE base address
// =============================================================================
//
// The linker automatically provides __ImageBase which points to the DOS header
// of the loaded PE image. This works for both standard drivers and manually
// mapped images (as long as the PE headers are preserved during mapping).

extern IMAGE_DOS_HEADER __ImageBase;

// =============================================================================
// Global Self-Info Structure
// =============================================================================

SELF_INFO g_SelfInfo = {0};

// =============================================================================
// String comparison helper (no CRT dependency)
// =============================================================================

static bool StrEqualN(const char* a, const char* b, U32 n) {
    for (U32 i = 0; i < n; i++) {
        if (a[i] != b[i]) return false;
        if (a[i] == 0) return true;
    }
    return true;
}

// =============================================================================
// Discover Self Information
// =============================================================================

OMBRA_STATUS SelfInfoDiscover(void) {
    IMAGE_DOS_HEADER* dosHeader;
    IMAGE_NT_HEADERS64* ntHeaders;
    IMAGE_OPTIONAL_HEADER64* optHeader;

    // Already discovered?
    if (g_SelfInfo.Valid) {
        return OMBRA_SUCCESS;
    }

    // Get PE base from linker symbol
    dosHeader = &__ImageBase;
    g_SelfInfo.ImageBase = dosHeader;

    // Validate DOS header
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        return OMBRA_ERROR_INVALID_PARAMETER;
    }

    // Get NT headers
    ntHeaders = (IMAGE_NT_HEADERS64*)((U8*)dosHeader + dosHeader->e_lfanew);

    // Validate NT signature
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
        return OMBRA_ERROR_INVALID_PARAMETER;
    }

    // Validate PE64
    optHeader = &ntHeaders->OptionalHeader;
    if (optHeader->Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        return OMBRA_ERROR_INVALID_PARAMETER;
    }

    // Extract info
    g_SelfInfo.ImageSize = optHeader->SizeOfImage;
    g_SelfInfo.EntryPoint = (U8*)dosHeader + optHeader->AddressOfEntryPoint;
    g_SelfInfo.NumberOfSections = ntHeaders->FileHeader.NumberOfSections;

    // Physical base requires kernel symbols - will be filled in later
    g_SelfInfo.PhysicalBase = 0;

    g_SelfInfo.Valid = true;
    return OMBRA_SUCCESS;
}

// =============================================================================
// Accessors
// =============================================================================

void* SelfInfoGetBase(void) {
    if (!g_SelfInfo.Valid) {
        SelfInfoDiscover();
    }
    return g_SelfInfo.ImageBase;
}

U64 SelfInfoGetSize(void) {
    if (!g_SelfInfo.Valid) {
        SelfInfoDiscover();
    }
    return g_SelfInfo.ImageSize;
}

U64 SelfInfoGetPhysicalBase(void) {
    if (!g_SelfInfo.Valid) {
        SelfInfoDiscover();
    }

    // Resolve physical base on first call (requires kernel symbols)
    if (g_SelfInfo.PhysicalBase == 0 && g_SelfInfo.ImageBase != NULL) {
        g_SelfInfo.PhysicalBase = KernelGetPhysicalAddress(g_SelfInfo.ImageBase);
    }

    return g_SelfInfo.PhysicalBase;
}

// =============================================================================
// Find Section by Name
// =============================================================================

IMAGE_SECTION_HEADER* SelfInfoFindSection(const char* name) {
    IMAGE_DOS_HEADER* dosHeader;
    IMAGE_NT_HEADERS64* ntHeaders;
    IMAGE_SECTION_HEADER* section;
    U16 numSections;

    if (!g_SelfInfo.Valid) {
        if (OMBRA_FAILED(SelfInfoDiscover())) {
            return NULL;
        }
    }

    dosHeader = (IMAGE_DOS_HEADER*)g_SelfInfo.ImageBase;
    ntHeaders = (IMAGE_NT_HEADERS64*)((U8*)dosHeader + dosHeader->e_lfanew);
    numSections = ntHeaders->FileHeader.NumberOfSections;

    // Section headers follow the optional header
    section = (IMAGE_SECTION_HEADER*)((U8*)&ntHeaders->OptionalHeader +
                                       ntHeaders->FileHeader.SizeOfOptionalHeader);

    for (U16 i = 0; i < numSections; i++) {
        if (StrEqualN((const char*)section[i].Name, name, 8)) {
            return &section[i];
        }
    }

    return NULL;
}

// =============================================================================
// Get Physical Range for EPT Protection
// =============================================================================

OMBRA_STATUS SelfInfoGetPhysicalRange(U64* outPhysBase, U64* outPhysSize) {
    if (!outPhysBase || !outPhysSize) {
        return OMBRA_ERROR_INVALID_PARAMETER;
    }

    if (!g_SelfInfo.Valid) {
        OMBRA_STATUS status = SelfInfoDiscover();
        if (OMBRA_FAILED(status)) {
            return status;
        }
    }

    // Get physical base (requires kernel symbols)
    U64 physBase = SelfInfoGetPhysicalBase();
    if (physBase == 0) {
        return OMBRA_ERROR_NOT_INITIALIZED;
    }

    *outPhysBase = physBase;
    *outPhysSize = g_SelfInfo.ImageSize;

    return OMBRA_SUCCESS;
}
