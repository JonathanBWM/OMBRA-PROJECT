// PayLoad/intel/mm.h
// Intel VMX Memory Management via EPT
// Provides guest memory access and EPT translation for vmxroot context
#pragma once

#include "../include/types.h"
#include "../include/vmcall.h"
#include "debug.h"

//===----------------------------------------------------------------------===//
// CPUID Structure for APIC ID extraction
// From deprecated ia32.hpp - needed for per-CPU page table indexing
//===----------------------------------------------------------------------===//

typedef union
{
    struct
    {
        // EAX - Version information
        u32 stepping : 4;
        u32 model : 4;
        u32 family : 4;
        u32 type : 2;
        u32 reserved1 : 2;
        u32 ext_model : 4;
        u32 ext_family : 8;
        u32 reserved2 : 4;
    } version_info;

    struct
    {
        u8 brand_index;
        u8 clflush_line_size;
        u8 max_addressable_ids;
        u8 initial_apic_id;
    } cpuid_additional_information;

    u32 cpuid_feature_ecx;
    u32 cpuid_feature_edx;
} cpuid_eax_01;

//===----------------------------------------------------------------------===//
// EPT Structure Definitions
// From deprecated ia32.hpp - preserves page_frame_number field names
//===----------------------------------------------------------------------===//

// EPT Pointer (EPTP) - references EPT PML4 table
typedef union
{
    struct
    {
        u64 memory_type : 3;                  // 0 = UC, 6 = WB
        u64 page_walk_length : 3;             // Value is 1 less than walk length
        u64 enable_access_and_dirty_flags : 1;
        u64 reserved1 : 5;
        u64 page_frame_number : 36;
        u64 reserved2 : 16;
    };
    u64 flags;
} ept_pointer;

// EPT PML4 Entry - references 512GB region
typedef union
{
    struct
    {
        u64 read_access : 1;
        u64 write_access : 1;
        u64 execute_access : 1;
        u64 reserved1 : 5;
        u64 accessed : 1;
        u64 reserved2 : 1;
        u64 user_mode_execute : 1;
        u64 reserved3 : 1;
        u64 page_frame_number : 36;
        u64 reserved4 : 16;
    };
    u64 flags;
} ept_pml4e;

// EPT PDPTE for 1GB large page
typedef union
{
    struct
    {
        u64 read_access : 1;
        u64 write_access : 1;
        u64 execute_access : 1;
        u64 memory_type : 3;
        u64 ignore_pat : 1;
        u64 large_page : 1;           // Must be 1 for 1GB page
        u64 accessed : 1;
        u64 dirty : 1;
        u64 user_mode_execute : 1;
        u64 reserved1 : 19;
        u64 page_frame_number : 18;   // 1GB aligned
        u64 reserved2 : 15;
        u64 suppress_ve : 1;
    };
    u64 flags;
} ept_pdpte_1gb;

// EPT PDPTE that references page directory
typedef union
{
    struct
    {
        u64 read_access : 1;
        u64 write_access : 1;
        u64 execute_access : 1;
        u64 reserved1 : 5;
        u64 accessed : 1;
        u64 reserved2 : 1;
        u64 user_mode_execute : 1;
        u64 reserved3 : 1;
        u64 page_frame_number : 36;
        u64 reserved4 : 16;
    };
    u64 flags;
} ept_pdpte;

// EPT PDE for 2MB large page
typedef union
{
    struct
    {
        u64 read_access : 1;
        u64 write_access : 1;
        u64 execute_access : 1;
        u64 memory_type : 3;
        u64 ignore_pat : 1;
        u64 large_page : 1;           // Must be 1 for 2MB page
        u64 accessed : 1;
        u64 dirty : 1;
        u64 user_mode_execute : 1;
        u64 reserved1 : 10;
        u64 page_frame_number : 27;   // 2MB aligned
        u64 reserved2 : 15;
        u64 suppress_ve : 1;
    };
    u64 flags;
} epde_2mb;

// EPT PDE that references page table
typedef union
{
    struct
    {
        u64 read_access : 1;
        u64 write_access : 1;
        u64 execute_access : 1;
        u64 reserved1 : 5;
        u64 accessed : 1;
        u64 reserved2 : 1;
        u64 user_mode_execute : 1;
        u64 reserved3 : 1;
        u64 page_frame_number : 36;
        u64 reserved4 : 16;
    };
    u64 flags;
} ept_pde;

// EPT PTE for 4KB page
typedef union
{
    struct
    {
        u64 read_access : 1;
        u64 write_access : 1;
        u64 execute_access : 1;
        u64 memory_type : 3;
        u64 ignore_pat : 1;
        u64 reserved1 : 1;
        u64 accessed : 1;
        u64 dirty : 1;
        u64 user_mode_execute : 1;
        u64 reserved2 : 1;
        u64 page_frame_number : 36;
        u64 reserved3 : 15;
        u64 suppress_ve : 1;
    };
    u64 flags;
} ept_pte;

//===----------------------------------------------------------------------===//
// PML4 Configuration
//===----------------------------------------------------------------------===//

#define SELF_REF_PML4_IDX 510
#define MAPPING_PML4_IDX 100

#define MAPPING_ADDRESS_BASE 0x0000327FFFE00000
#define SELF_REF_PML4 0xFFFFFF7FBFDFE000

// EPT large page offset macros
#define EPT_LARGE_PDPTE_OFFSET(_) (((u64)(_)) & ((0x1000 * 0x200 * 0x200) - 1))
#define EPT_LARGE_PDE_OFFSET(_) (((u64)(_)) & ((0x1000 * 0x200) - 1))

//===----------------------------------------------------------------------===//
// Section Allocations for Page Tables
//===----------------------------------------------------------------------===//

#pragma section(".pdpt", read, write)
#pragma section(".pd", read, write)
#pragma section(".pt", read, write)

namespace core {
namespace mm {

// Note: map_type_t is defined in ../include/types.h

//===----------------------------------------------------------------------===//
// Virtual Address Parsing Union
//===----------------------------------------------------------------------===//

typedef union _virt_addr_t
{
    u64 value;

    // 4KB page layout
    struct
    {
        u64 offset_4kb : 12;
        u64 pt_index : 9;
        u64 pd_index : 9;
        u64 pdpt_index : 9;
        u64 pml4_index : 9;
        u64 reserved : 16;
    };

    // 2MB page layout
    struct
    {
        u64 offset_2mb : 21;
        u64 pd_index : 9;
        u64 pdpt_index : 9;
        u64 pml4_index : 9;
        u64 reserved : 16;
    };

    // 1GB page layout
    struct
    {
        u64 offset_1gb : 30;
        u64 pdpt_index : 9;
        u64 pml4_index : 9;
        u64 reserved : 16;
    };

} virt_addr_t, *pvirt_addr_t;

using phys_addr_t = virt_addr_t;

//===----------------------------------------------------------------------===//
// Page Table Entry Structures (Host)
//===----------------------------------------------------------------------===//

typedef union _pml4e
{
    u64 value;
    struct
    {
        u64 present : 1;
        u64 writeable : 1;
        u64 user_supervisor : 1;
        u64 page_write_through : 1;
        u64 page_cache : 1;
        u64 accessed : 1;
        u64 ignore_1 : 1;
        u64 page_size : 1;
        u64 ignore_2 : 4;
        u64 pfn : 36;
        u64 reserved : 4;
        u64 ignore_3 : 11;
        u64 nx : 1;
    };
} pml4e, *ppml4e;

typedef union _pdpte
{
    u64 value;
    struct
    {
        u64 present : 1;
        u64 rw : 1;
        u64 user_supervisor : 1;
        u64 page_write_through : 1;
        u64 page_cache : 1;
        u64 accessed : 1;
        u64 ignore_1 : 1;
        u64 large_page : 1;
        u64 ignore_2 : 4;
        u64 pfn : 36;
        u64 reserved : 4;
        u64 ignore_3 : 11;
        u64 nx : 1;
    };
} pdpte, *ppdpte;

typedef union _pde
{
    u64 value;
    struct
    {
        u64 present : 1;
        u64 rw : 1;
        u64 user_supervisor : 1;
        u64 page_write_through : 1;
        u64 page_cache : 1;
        u64 accessed : 1;
        u64 ignore_1 : 1;
        u64 large_page : 1;
        u64 ignore_2 : 4;
        u64 pfn : 36;
        u64 reserved : 4;
        u64 ignore_3 : 11;
        u64 nx : 1;
    };
} pde, *ppde;

typedef union _pte
{
    u64 value;
    struct
    {
        u64 present : 1;
        u64 rw : 1;
        u64 user_supervisor : 1;
        u64 page_write_through : 1;
        u64 page_cache : 1;
        u64 accessed : 1;
        u64 dirty : 1;
        u64 access_type : 1;
        u64 global : 1;
        u64 ignore_2 : 3;
        u64 pfn : 36;
        u64 reserved : 4;
        u64 ignore_3 : 7;
        u64 pk : 4;
        u64 nx : 1;
    };
} pte, *ppte;

//===----------------------------------------------------------------------===//
// Static Page Table Allocations
// Each core gets 2 PT entries (src/dest) for concurrent mapping
//===----------------------------------------------------------------------===//

__declspec(allocate(".pdpt")) inline pdpte pdpt[512];
__declspec(allocate(".pd")) inline pde pd[512];
__declspec(allocate(".pt")) inline pte pt[512];

// Self-referencing PML4 allows walking our own page tables
inline const ppml4e hyperv_pml4{ reinterpret_cast<ppml4e>(SELF_REF_PML4) };

//===----------------------------------------------------------------------===//
// Memory Management API
//===----------------------------------------------------------------------===//

// Initialize mapping page tables and validate setup
auto init() -> VMX_ROOT_ERROR;

// Map guest physical address to host virtual
auto map_guest_phys(guest_phys_t phys_addr, map_type_t map_type = map_type_t::map_src) -> u64;

// Map guest virtual address to host virtual
auto map_guest_virt(guest_phys_t dirbase, guest_virt_t virt_addr, map_type_t map_type = map_type_t::map_src) -> u64;

// Map host physical address to host virtual (per-CPU PT entry)
auto map_page(host_phys_t phys_addr, map_type_t map_type = map_type_t::map_src) -> u64;

// Get virtual address for current CPU's mapping slot
auto get_map_virt(u16 offset = 0u, map_type_t map_type = map_type_t::map_src) -> u64;

// Translate host virtual to host physical (via self-ref PML4)
auto translate(host_virt_t host_virt) -> u64;

// Translate guest physical to host physical (via EPT)
auto translate_guest_physical(guest_phys_t guest_phys, map_type_t map_type = map_type_t::map_src) -> u64;

// Translate guest virtual to guest physical (via guest page tables)
auto translate_guest_virtual(guest_phys_t dirbase, guest_virt_t guest_virt, map_type_t map_type = map_type_t::map_src) -> u64;

// Read from guest physical to guest virtual buffer
auto read_guest_phys(guest_phys_t dirbase, guest_phys_t guest_phys, guest_virt_t guest_virt, u64 size) -> VMX_ROOT_ERROR;

// Write from guest virtual buffer to guest physical
auto write_guest_phys(guest_phys_t dirbase, guest_phys_t guest_phys, guest_virt_t guest_virt, u64 size) -> VMX_ROOT_ERROR;

// Copy between two guest virtual address spaces
auto copy_guest_virt(guest_phys_t dirbase_src, guest_virt_t virt_src, guest_virt_t dirbase_dest, guest_virt_t virt_dest, u64 size) -> VMX_ROOT_ERROR;

} // namespace mm
} // namespace core
