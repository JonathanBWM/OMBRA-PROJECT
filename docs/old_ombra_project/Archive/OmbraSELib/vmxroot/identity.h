#pragma once
// OmbraSELib Standalone Identity Mapping for VMXRoot Context
// No WDK dependencies

#include "types.h"

namespace identity {

// PDPTE structure for identity mapping (4-Level paging, references page directory)
typedef union {
    struct {
        u64 Present : 1;
        u64 Write : 1;
        u64 Supervisor : 1;
        u64 PageLevelWriteThrough : 1;
        u64 PageLevelCacheDisable : 1;
        u64 Accessed : 1;
        u64 Reserved1 : 1;
        u64 LargePage : 1;
        u64 Ignored1 : 4;
        u64 PageFrameNumber : 40;
        u64 Ignored2 : 11;
        u64 ExecuteDisable : 1;
    };
    u64 Flags;
} PDPTE_64;

// PDE_2MB structure for identity mapping (2MB large pages)
typedef union {
    struct {
        u64 Present : 1;
        u64 Write : 1;
        u64 Supervisor : 1;
        u64 PageLevelWriteThrough : 1;
        u64 PageLevelCacheDisable : 1;
        u64 Accessed : 1;
        u64 Dirty : 1;
        u64 LargePage : 1;
        u64 Global : 1;
        u64 Ignored1 : 3;
        u64 Pat : 1;
        u64 Reserved1 : 8;
        u64 PageFrameNumber : 31;
        u64 Ignored2 : 7;
        u64 ProtectionKey : 4;
        u64 ExecuteDisable : 1;
    };
    u64 Flags;
} PDE_2MB_64;

// PML4E structure for identity mapping
typedef union {
    struct {
        u64 Present : 1;
        u64 Write : 1;
        u64 Supervisor : 1;
        u64 PageLevelWriteThrough : 1;
        u64 PageLevelCacheDisable : 1;
        u64 Accessed : 1;
        u64 Reserved1 : 1;
        u64 PageSize : 1;
        u64 Ignored1 : 4;
        u64 PageFrameNumber : 40;
        u64 Ignored2 : 11;
        u64 ExecuteDisable : 1;
    };
    u64 Flags;
} PML4E_64;

// Identity mapping structure - maps first 512GB of physical memory
// using 2MB large pages for performance
struct IDENTITY_MAPPING {
    __declspec(align(0x1000)) PML4E_64 pml4[512];
    __declspec(align(0x1000)) PDPTE_64 pdpt[512];
    __declspec(align(0x1000)) PDE_2MB_64 pdt[512][512];
    u64 pa; // Physical address of PDPT

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
        // PageFrameNumber will be set when we know the physical address

        // Initialize PDPT entries -> PDT arrays
        // Each PDPT entry maps 1GB
        for (u64 pdpt_idx = 0; pdpt_idx < 512; pdpt_idx++) {
            pdpt[pdpt_idx].Present = 1;
            pdpt[pdpt_idx].Write = 1;
            pdpt[pdpt_idx].Supervisor = 1;

            // Initialize PDT entries with 2MB large pages
            // Each PDT entry maps 2MB of physical memory
            for (u64 pdt_idx = 0; pdt_idx < 512; pdt_idx++) {
                u64 phys_addr = (pdpt_idx * 512 + pdt_idx) * PAGE_2MB;
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
