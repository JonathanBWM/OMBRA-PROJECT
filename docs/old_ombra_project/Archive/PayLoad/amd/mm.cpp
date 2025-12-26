// PayLoad/amd/mm.cpp
// AMD SVM Memory Management Implementation
// Identity-mapped physical memory access for vmxroot context

#include "mm.h"
#include "debug.h"
#include <OmbraSELib/Identity.h>

#define PTI_SHIFT  12L
#define PDI_SHIFT  21L
#define PPI_SHIFT  30L
#define PXI_SHIFT  39L

// Exported identity mapping structure for boot chain
__declspec(dllexport) identity::IDENTITY_MAPPING identity_map;

// PML4 index 360 is used for host physical memory mapping
constexpr u64 mapped_host_phys_pml = 360;

// Base virtual address for identity-mapped physical memory access
// Calculated as: (pml4_index << 39) | sign extension
char* pIdentity = (char*)((mapped_host_phys_pml << PXI_SHIFT) | 0xffff000000000000);
u64 pIdentityAsU64 = (u64)((mapped_host_phys_pml << PXI_SHIFT) | 0xffff000000000000);

// Memory initialization flag
bool bMemInit = false;

namespace core {
namespace mm {

auto init() -> u64
{
    if (bMemInit)
    {
        return VMX_ROOT_ERROR::SUCCESS;
    }

    {
        auto mapping = &identity_map;

        // Set up PML4 entry at index 360 to point to our identity mapping PDPT
        hyperv_pml4[mapped_host_phys_pml].value = 0;
        hyperv_pml4[mapped_host_phys_pml].present = true;
        hyperv_pml4[mapped_host_phys_pml].writeable = true;
        hyperv_pml4[mapped_host_phys_pml].user_supervisor = true;
        hyperv_pml4[mapped_host_phys_pml].pfn = translate((UINT64)&mapping->pdpt[0]) / PAGE_4KB;

        // Initialize PDPT entries (512 entries, each covers 1GB)
        for (UINT64 EntryIndex = 0; EntryIndex < 512; EntryIndex++)
        {
            mapping->pdpt[EntryIndex].Flags = 0;
            mapping->pdpt[EntryIndex].Present = true;
            mapping->pdpt[EntryIndex].Write = true;
            mapping->pdpt[EntryIndex].Supervisor = true;
            mapping->pdpt[EntryIndex].PageFrameNumber = translate((UINT64)&mapping->pdt[EntryIndex][0]) / PAGE_4KB;
        }

        // Initialize PDT entries with 2MB large pages
        // 512 PDPT entries * 512 PDT entries = 512GB identity mapped
        for (UINT64 EntryGroupIndex = 0; EntryGroupIndex < 512; EntryGroupIndex++)
        {
            for (UINT64 EntryIndex = 0; EntryIndex < 512; EntryIndex++)
            {
                mapping->pdt[EntryGroupIndex][EntryIndex].Flags = 0;
                mapping->pdt[EntryGroupIndex][EntryIndex].Present = true;
                mapping->pdt[EntryGroupIndex][EntryIndex].Write = true;
                mapping->pdt[EntryGroupIndex][EntryIndex].LargePage = true;
                mapping->pdt[EntryGroupIndex][EntryIndex].Supervisor = true;
                // Physical address = (group * 512 + entry) * 2MB
                mapping->pdt[EntryGroupIndex][EntryIndex].PageFrameNumber = (EntryGroupIndex * 512) + EntryIndex;
            }
        }

        // Store physical address of PDPT for later use
        mapping->pa = translate((UINT64)&mapping->pdpt[0]);
    }

    // Flush TLB by rewriting CR3
    volatile CR3 cr3 = { 0 };
    cr3.Flags = __readcr3();
    __writecr3(cr3.Flags);

    // Test the mapping by reading from a known physical address
    int* p = (int*)map_guest_phys(0x200000);
    volatile int test = *p;

    bMemInit = true;

    return VMX_ROOT_ERROR::SUCCESS;
}

auto map_guest_phys(guest_phys_t phys_addr, map_type_t map_type) -> u64
{
    // Direct physical mapping through our identity-mapped region
    return map_page(phys_addr, map_type);
}

auto map_guest_virt(guest_phys_t dirbase, guest_virt_t virt_addr, map_type_t map_type) -> u64
{
    // First translate guest virtual to guest physical
    const auto guest_phys =
        translate_guest_virtual(dirbase, virt_addr, map_type);

    if (!guest_phys)
        return {};

    // Then map the guest physical address
    return map_guest_phys(guest_phys, map_type);
}

auto map_page(host_phys_t phys_addr, map_type_t map_type) -> u64
{
    // Ensure PML4 entry is valid (Hyper-V may overwrite it)
    hyperv_pml4[mapped_host_phys_pml].present = true;
    hyperv_pml4[mapped_host_phys_pml].writeable = true;
    hyperv_pml4[mapped_host_phys_pml].user_supervisor = true;
    hyperv_pml4[mapped_host_phys_pml].pfn = identity_map.pa / PAGE_4KB;

    // Return virtual address: identity base + physical offset
    return pIdentityAsU64 + phys_addr;
}

auto translate(host_virt_t host_virt) -> u64
{
    virt_addr_t virt_addr{ host_virt };
    virt_addr_t cursor{ (u64)hyperv_pml4 };

    // Walk PML4
    if (!reinterpret_cast<ppml4e>(cursor.value)[virt_addr.pml4_index].present)
        return 0;

    cursor.pt_index = virt_addr.pml4_index;

    // Walk PDPT
    if (!reinterpret_cast<ppdpte>(cursor.value)[virt_addr.pdpt_index].present)
        return 0;

    // Handle 1GB large page
    if (reinterpret_cast<ppdpte>(cursor.value)[virt_addr.pdpt_index].large_page)
        return (reinterpret_cast<ppdpte>(cursor.value)
            [virt_addr.pdpt_index].pfn << 30) + virt_addr.offset_1gb;

    cursor.pd_index = virt_addr.pml4_index;
    cursor.pt_index = virt_addr.pdpt_index;

    // Walk PD
    if (!reinterpret_cast<ppde>(cursor.value)[virt_addr.pd_index].present)
        return 0;

    // Handle 2MB large page
    if (reinterpret_cast<ppde>(cursor.value)[virt_addr.pd_index].large_page)
        return (reinterpret_cast<ppde>(cursor.value)
            [virt_addr.pd_index].pfn << 21) + virt_addr.offset_2mb;

    cursor.pdpt_index = virt_addr.pml4_index;
    cursor.pd_index = virt_addr.pdpt_index;
    cursor.pt_index = virt_addr.pd_index;

    // Walk PT
    if (!reinterpret_cast<ppte>(cursor.value)[virt_addr.pt_index].present)
        return 0;

    // Return physical address from 4KB page
    return (reinterpret_cast<ppte>(cursor.value)
        [virt_addr.pt_index].pfn << 12) + virt_addr.offset_4kb;
}

auto translate_guest_virtual(guest_phys_t dirbase, guest_virt_t guest_virt, map_type_t map_type) -> u64
{
    CR3 cr3 = { 0 };
    cr3.Flags = dirbase;
    dirbase = cr3.AddressOfPageDirectory * 0x1000;
    virt_addr_t virt_addr{ guest_virt };

    // Map and walk PML4
    const auto pml4 =
        reinterpret_cast<pml4e*>(
            map_guest_phys(dirbase, map_type));

    if (!pml4 || !pml4[virt_addr.pml4_index].present)
        return {};

    // Map and walk PDPT
    const auto pdpt =
        reinterpret_cast<pdpte*>(map_guest_phys(
            pml4[virt_addr.pml4_index].pfn << 12, map_type));

    if (!pdpt || !pdpt[virt_addr.pdpt_index].present)
        return {};

    // Handle 1GB large page
    if (pdpt[virt_addr.pdpt_index].large_page)
        return (pdpt[virt_addr.pdpt_index].pfn << 30) + virt_addr.offset_1gb;

    // Map and walk PD
    const auto pd =
        reinterpret_cast<pde*>(map_guest_phys(
            pdpt[virt_addr.pdpt_index].pfn << 12, map_type));

    if (!pd || !pd[virt_addr.pd_index].present)
        return {};

    // Handle 2MB large page
    if (pd[virt_addr.pd_index].large_page)
        return (pd[virt_addr.pd_index].pfn << 21) + virt_addr.offset_2mb;

    // Map and walk PT
    const auto pt =
        reinterpret_cast<pte*>(map_guest_phys(
            pd[virt_addr.pd_index].pfn << 12, map_type));

    if (!pt || !pt[virt_addr.pt_index].present)
        return {};

    // Return guest physical address from 4KB page
    return (pt[virt_addr.pt_index].pfn << 12) + virt_addr.offset_4kb;
}

auto get_npte(guest_phys_t phys_addr) -> pnpt_pte {
    phys_addr_t guest_phys{ phys_addr };
    const auto vmcb = amd::get_vmcb();

    // Walk NPT starting from VMCB's nested CR3
    const auto npt_pml4 =
        reinterpret_cast<pnpt_pml4e>(
            map_page(vmcb->NestedPageTableCr3()));

    if (!npt_pml4[guest_phys.pml4_index].present)
        return {};

    const auto npt_pdpt =
        reinterpret_cast<pnpt_pdpte>(
            map_page(npt_pml4[guest_phys.pml4_index].pfn << 12));

    if (!npt_pdpt[guest_phys.pdpt_index].present)
        return {};

    const auto npt_pd =
        reinterpret_cast<pnpt_pde>(
            map_page(npt_pdpt[guest_phys.pdpt_index].pfn << 12));

    if (!npt_pd[guest_phys.pd_index].present)
        return { 0 };

    // Check for 2MB large page
    if (reinterpret_cast<pnpt_pde_2mb>(npt_pd)[guest_phys.pd_index].large_page)
        return { 0 };

    const auto npt_pt =
        reinterpret_cast<pnpt_pte>(
            map_page(npt_pd[guest_phys.pd_index].pfn << 12));

    return &npt_pt[guest_phys.pt_index];
}

auto translate_guest_physical(guest_phys_t phys_addr, map_type_t map_type) -> u64
{
    phys_addr_t guest_phys{ phys_addr };
    const auto vmcb = amd::get_vmcb();

    // Walk NPT to translate guest physical to system physical
    const auto npt_pml4 =
        reinterpret_cast<pnpt_pml4e>(
            map_page(vmcb->NestedPageTableCr3(), map_type));

    if (!npt_pml4[guest_phys.pml4_index].present)
        return {};

    const auto npt_pdpt =
        reinterpret_cast<pnpt_pdpte>(
            map_page(npt_pml4[guest_phys.pml4_index].pfn << 12, map_type));

    if (!npt_pdpt[guest_phys.pdpt_index].present)
        return {};

    const auto npt_pd =
        reinterpret_cast<pnpt_pde>(
            map_page(npt_pdpt[guest_phys.pdpt_index].pfn << 12, map_type));

    if (!npt_pd[guest_phys.pd_index].present)
        return {};

    // Handle 2MB large page
    if (reinterpret_cast<pnpt_pde_2mb>(npt_pd)[guest_phys.pd_index].large_page)
        return (reinterpret_cast<pnpt_pde_2mb>(npt_pd)
            [guest_phys.pd_index].pfn << 21) + guest_phys.offset_2mb;

    const auto npt_pt =
        reinterpret_cast<pnpt_pte>(
            map_page(npt_pd[guest_phys.pd_index].pfn << 12, map_type));

    if (!npt_pt[guest_phys.pt_index].present)
        return {};

    // Return system physical address
    return (npt_pt[guest_phys.pt_index].pfn << 12) + guest_phys.offset_4kb;
}

auto read_guest_phys(guest_phys_t dirbase, guest_phys_t guest_phys,
    guest_virt_t guest_virt, u64 size) -> VMX_ROOT_ERROR
{
    // Handle reading across page boundaries
    while (size)
    {
        auto dest_current_size = PAGE_4KB -
            virt_addr_t{ guest_virt }.offset_4kb;

        if (size < dest_current_size)
            dest_current_size = size;

        auto src_current_size = PAGE_4KB -
            phys_addr_t{ guest_phys }.offset_4kb;

        if (size < src_current_size)
            src_current_size = size;

        auto current_size =
            min(dest_current_size, src_current_size);

        // Map destination (guest virtual in caller's address space)
        const auto mapped_dest =
            reinterpret_cast<void*>(
                map_guest_virt(dirbase, guest_virt, map_type_t::map_dest));

        if (!mapped_dest)
            return VMX_ROOT_ERROR::INVALID_GUEST_VIRTUAL;

        // Map source (guest physical)
        const auto mapped_src =
            reinterpret_cast<void*>(
                map_guest_phys(guest_phys, map_type_t::map_src));

        if (!mapped_src)
            return VMX_ROOT_ERROR::INVALID_GUEST_PHYSICAL;

        // VMXRoot: page faults are fatal triple faults - NPT setup must be correct
        // Null checks above validate mappings; proceed with copy
        __movsb((UINT8*)mapped_dest, (UINT8*)mapped_src, current_size);

        guest_phys += current_size;
        guest_virt += current_size;
        size -= current_size;
    }

    return VMX_ROOT_ERROR::SUCCESS;
}

auto write_guest_phys(guest_phys_t dirbase,
    guest_phys_t guest_phys, guest_virt_t guest_virt, u64 size) -> VMX_ROOT_ERROR
{
    // Handle writing across page boundaries
    while (size)
    {
        auto dest_current_size = PAGE_4KB -
            virt_addr_t{ guest_virt }.offset_4kb;

        if (size < dest_current_size)
            dest_current_size = size;

        auto src_current_size = PAGE_4KB -
            phys_addr_t{ guest_phys }.offset_4kb;

        if (size < src_current_size)
            src_current_size = size;

        auto current_size =
            min(dest_current_size, src_current_size);

        // Map source (guest virtual in caller's address space)
        const auto mapped_src =
            reinterpret_cast<void*>(
                map_guest_virt(dirbase, guest_virt, map_type_t::map_src));

        if (!mapped_src)
            return VMX_ROOT_ERROR::INVALID_GUEST_VIRTUAL;

        // Map destination (guest physical)
        const auto mapped_dest =
            reinterpret_cast<void*>(
                map_guest_phys(guest_phys, map_type_t::map_dest));

        if (!mapped_dest)
            return VMX_ROOT_ERROR::INVALID_GUEST_PHYSICAL;

        // VMXRoot: page faults are fatal triple faults - NPT setup must be correct
        // Null checks above validate mappings; proceed with copy
        __movsb((UINT8*)mapped_dest, (UINT8*)mapped_src, current_size);

        guest_phys += current_size;
        guest_virt += current_size;
        size -= current_size;
    }

    return VMX_ROOT_ERROR::SUCCESS;
}

auto copy_guest_virt(guest_phys_t dirbase_src, guest_virt_t virt_src,
    guest_virt_t dirbase_dest, guest_virt_t virt_dest, u64 size) -> VMX_ROOT_ERROR
{
    while (size)
    {
        auto dest_size = PAGE_4KB - virt_addr_t{ virt_dest }.offset_4kb;
        if (size < dest_size)
            dest_size = size;

        auto src_size = PAGE_4KB - virt_addr_t{ virt_src }.offset_4kb;
        if (size < src_size)
            src_size = size;

        // Map source virtual address
        const auto mapped_src =
            reinterpret_cast<void*>(
                map_guest_virt(dirbase_src, virt_src, map_type_t::map_src));

        if (!mapped_src)
            return VMX_ROOT_ERROR::INVALID_GUEST_VIRTUAL;

        // Map destination virtual address
        const auto mapped_dest =
            reinterpret_cast<void*>(
                map_guest_virt(dirbase_dest, virt_dest, map_type_t::map_dest));

        if (!mapped_dest)
            return VMX_ROOT_ERROR::INVALID_GUEST_VIRTUAL;

        auto current_size = min(dest_size, src_size);

        // VMXRoot: page faults are fatal triple faults - NPT setup must be correct
        // Null checks above validate mappings; proceed with copy
        __movsb((UINT8*)mapped_dest, (UINT8*)mapped_src, current_size);

        virt_src += current_size;
        virt_dest += current_size;
        size -= current_size;
    }

    return VMX_ROOT_ERROR::SUCCESS;
}

} // namespace mm
} // namespace core
