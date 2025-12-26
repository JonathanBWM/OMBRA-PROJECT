#pragma once
// OmbraSELib Identity mapping wrapper
// Provides identity:: namespace for both UEFI bootloader and vmxroot contexts

#ifdef _VMXROOT_MODE
// In vmxroot context, use the standalone headers
#include "vmxroot/identity.h"
#else
// For UEFI bootloader context, define minimal identity namespace
// using EDK2-compatible types

#include <Uefi.h>

namespace identity {

// Page constants
#ifndef PAGE_SIZE
#define PAGE_SIZE 0x1000
#endif

#ifndef PAGE_2MB
#define PAGE_2MB (PAGE_SIZE * 512)
#endif

// PDPTE structure for identity mapping
typedef union {
    struct {
        UINT64 Present : 1;
        UINT64 Write : 1;
        UINT64 Supervisor : 1;
        UINT64 PageLevelWriteThrough : 1;
        UINT64 PageLevelCacheDisable : 1;
        UINT64 Accessed : 1;
        UINT64 Reserved1 : 1;
        UINT64 LargePage : 1;
        UINT64 Ignored1 : 4;
        UINT64 PageFrameNumber : 40;
        UINT64 Ignored2 : 11;
        UINT64 ExecuteDisable : 1;
    };
    UINT64 Flags;
} PDPTE_64;

// PDE_2MB structure for identity mapping (2MB large pages)
typedef union {
    struct {
        UINT64 Present : 1;
        UINT64 Write : 1;
        UINT64 Supervisor : 1;
        UINT64 PageLevelWriteThrough : 1;
        UINT64 PageLevelCacheDisable : 1;
        UINT64 Accessed : 1;
        UINT64 Dirty : 1;
        UINT64 LargePage : 1;
        UINT64 Global : 1;
        UINT64 Ignored1 : 3;
        UINT64 Pat : 1;
        UINT64 Reserved1 : 8;
        UINT64 PageFrameNumber : 31;
        UINT64 Ignored2 : 7;
        UINT64 ProtectionKey : 4;
        UINT64 ExecuteDisable : 1;
    };
    UINT64 Flags;
} PDE_2MB_64;

// PML4E structure for identity mapping
typedef union {
    struct {
        UINT64 Present : 1;
        UINT64 Write : 1;
        UINT64 Supervisor : 1;
        UINT64 PageLevelWriteThrough : 1;
        UINT64 PageLevelCacheDisable : 1;
        UINT64 Accessed : 1;
        UINT64 Reserved1 : 1;
        UINT64 PageSize : 1;
        UINT64 Ignored1 : 4;
        UINT64 PageFrameNumber : 40;
        UINT64 Ignored2 : 11;
        UINT64 ExecuteDisable : 1;
    };
    UINT64 Flags;
} PML4E_64;

// Identity mapping structure - maps first 512GB of physical memory
// using 2MB large pages for performance
struct IDENTITY_MAPPING {
    __declspec(align(0x1000)) PML4E_64 pml4[512];
    __declspec(align(0x1000)) PDPTE_64 pdpt[512];
    __declspec(align(0x1000)) PDE_2MB_64 pdt[512][512];
    UINT64 pa; // Physical address of PDPT

    // Initialize the identity mapping page tables
    // Maps 512GB of physical memory using 2MB large pages
    inline void Init() {
        // Zero the entire structure first
        for (int i = 0; i < 512; i++) {
            pml4[i].Flags = 0;
            pdpt[i].Flags = 0;
        }

        // Initialize PML4 entry 0 -> PDPT
        pml4[0].Present = 1;
        pml4[0].Write = 1;
        pml4[0].Supervisor = 1;

        // Initialize PDPT entries -> PDT arrays
        for (UINT64 pdpt_idx = 0; pdpt_idx < 512; pdpt_idx++) {
            pdpt[pdpt_idx].Present = 1;
            pdpt[pdpt_idx].Write = 1;
            pdpt[pdpt_idx].Supervisor = 1;

            // Initialize PDT entries with 2MB large pages
            for (UINT64 pdt_idx = 0; pdt_idx < 512; pdt_idx++) {
                UINT64 phys_addr = (pdpt_idx * 512 + pdt_idx) * PAGE_2MB;
                pdt[pdpt_idx][pdt_idx].Flags = 0;
                pdt[pdpt_idx][pdt_idx].Present = 1;
                pdt[pdpt_idx][pdt_idx].Write = 1;
                pdt[pdpt_idx][pdt_idx].Supervisor = 1;
                pdt[pdpt_idx][pdt_idx].LargePage = 1;
                pdt[pdpt_idx][pdt_idx].PageFrameNumber = phys_addr / PAGE_2MB;
            }
        }

        pa = 0; // Physical address set later by caller
    }
};

using PIDENTITY_MAPPING = IDENTITY_MAPPING*;

} // namespace identity

#endif // _VMXROOT_MODE
