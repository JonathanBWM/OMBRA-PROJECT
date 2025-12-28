# B5: Driver Mapping from Ring -1 - Complete Technical Reference

## Executive Summary

This document provides a comprehensive technical reference for mapping Windows kernel drivers from the hypervisor (ring -1) context. The techniques described enable loading drivers into kernel space without using standard driver loading mechanisms (MmLoadSystemImage, IoCreateDriver), thus bypassing PatchGuard, DSE, and anti-cheat detection systems.

The key architectural difference from traditional mapping: Sputnik maps from usermode using syscall hooks, while Ombra maps from the hypervisor during/after boot - invisible to the kernel entirely.

---

## 1. Hypercall Interface

### 1.1 VMCALL_TYPE Enumeration

The core hypercall types used by Sputnik, which Ombra will extend.

**Source:** `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/Sputnik/SKLib/SKLib/SKLib-v/include/Vmcall.h`

```cpp
enum VMCALL_TYPE {
    VMCALL_TEST = 0x1,          // Hypervisor presence check
    VMCALL_VMXOFF,              // Disable virtualization
    VMCALL_INVEPT_CONTEXT,      // Invalidate EPT
    VMCALL_HOOK_PAGE,           // EPT hook single page
    VMCALL_UNHOOK_PAGE,         // Remove EPT hook
    VMCALL_HOOK_PAGE_RANGE,     // EPT hook page range
    VMCALL_HOOK_PAGE_INDEX,     // EPT hook by index
    VMCALL_SUBSTITUTE_PAGE,     // Page substitution
    VMCALL_CRASH,               // Test VMCALL
    VMCALL_PROBE,               // Test VMCALL
    VMCALL_READ_VIRT,           // Read virtual memory
    VMCALL_WRITE_VIRT,          // Write virtual memory
    VMCALL_READ_PHY,            // Read physical memory
    VMCALL_WRITE_PHY,           // Write physical memory
    VMCALL_DISABLE_EPT,         // Disable EPT
    VMCALL_SET_COMM_KEY,        // Set communication key
    VMCALL_GET_CR3,             // Get guest CR3
    VMCALL_GET_EPT_BASE,        // Get EPT base address
    VMCALL_VIRT_TO_PHY,         // Translate VA to PA
    VMCALL_STORAGE_QUERY        // 128-entry u64 storage array
};
```

### 1.2 Payload Command Enumeration (Intel)

The payload uses a separate command set for VMExit handling.

**Source:** `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/Sputnik/PayLoad (Intel)/types.h`

```cpp
enum class vmexit_command_t
{
    init_page_tables,     // Initialize mm:: page tables
    read_guest_phys,      // Read guest physical memory
    write_guest_phys,     // Write guest physical memory
    copy_guest_virt,      // Cross-process virtual memory copy
    get_dirbase,          // Get current guest CR3
    translate             // Translate guest VA to PA
};
```

### 1.3 COMMAND_DATA / command_t Structure

The structure passed via R8 register during CPUID-based hypercalls.

**Source:** `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/Sputnik/PayLoad (Intel)/types.h`

```cpp
typedef union _command_t
{
    struct _copy_phys
    {
        host_phys_t  phys_addr;   // Source/dest physical address
        guest_virt_t buffer;       // Buffer virtual address
        u64 size;                  // Size to read/write
    } copy_phys;

    struct _copy_virt
    {
        guest_phys_t dirbase_src;  // Source process CR3
        guest_virt_t virt_src;     // Source virtual address
        guest_phys_t dirbase_dest; // Dest process CR3
        guest_virt_t virt_dest;    // Dest virtual address
        u64 size;                  // Size to copy
    } copy_virt;

    struct _translate_virt
    {
        guest_virt_t virt_src;     // VA to translate
        guest_phys_t phys_addr;    // Resulting PA
    } translate_virt;

    guest_phys_t dirbase;          // Simple CR3 return

} command_t, * pcommand_t;
```

### 1.4 SKLib COMMAND_DATA Structure

Used by the kernel-mode SKLib library.

**Source:** `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/Sputnik/SKLib/SKLib/SKLib-v/include/VMMDef.h`

```cpp
namespace vmm {
    typedef struct _READ_DATA {
        PVOID pOutBuf;      // Destination buffer
        PVOID pTarget;      // Source address to read
        DWORD64 length;     // Bytes to read
    } READ_DATA, * PREAD_DATA;

    typedef struct _WRITE_DATA {
        PVOID pInBuf;       // Source buffer
        PVOID pTarget;      // Destination address to write
        DWORD64 length;     // Bytes to write
    } WRITE_DATA, * PWRITE_DATA;

    typedef struct _TRANSLATION_DATA {
        PVOID va;           // Virtual address to translate
        DWORD64 pa;         // Resulting physical address
    } TRANSLATION_DATA, *PTRANSLATION_DATA;
}
```

### 1.5 Register Convention

**Intel VT-x (via CPUID exit):**
| Register | Purpose |
|----------|---------|
| RCX | VMEXIT_KEY (magic value for auth) |
| RDX | Command ID (vmexit_command_t) |
| R8 | Pointer to command_t structure |
| R9 | Authentication key (XOR'd with marker) |
| RAX | Return value (status code) |

**Authentication Check (Sputnik):**
```cpp
#define VMEXIT_KEY 0xDEADBEEFDEADBEEF

// In vmexit handler:
if (guest_registers->rcx == VMEXIT_KEY)
{
    // Process command...
}
```

---

## 2. Physical Memory Access from Ring -1

### 2.1 Memory Manager Initialization

The hypervisor payload must initialize its own page tables for mapping guest physical memory into its virtual address space.

**Source:** `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/Sputnik/PayLoad (Intel)/mm.cpp`

```cpp
auto mm::init() -> vmxroot_error_t
{
    const auto pdpt_phys = translate(reinterpret_cast<u64>(pdpt));
    const auto pd_phys = translate(reinterpret_cast<u64>(pd));
    const auto pt_phys = translate(reinterpret_cast<u64>(pt));

    if (!pdpt_phys || !pd_phys || !pt_phys)
        return vmxroot_error_t::invalid_host_virtual;

    // Setup mapping page table entries
    hyperv_pml4[MAPPING_PML4_IDX].present = true;
    hyperv_pml4[MAPPING_PML4_IDX].pfn = pdpt_phys >> 12;
    hyperv_pml4[MAPPING_PML4_IDX].user_supervisor = false;
    hyperv_pml4[MAPPING_PML4_IDX].writeable = true;

    pdpt[511].present = true;
    pdpt[511].pfn = pd_phys >> 12;
    pdpt[511].user_supervisor = false;
    pdpt[511].rw = true;

    pd[511].present = true;
    pd[511].pfn = pt_phys >> 12;
    pd[511].user_supervisor = false;
    pd[511].rw = true;

    // Each core will have its own mapping entry
    for (auto idx = 0u; idx < 512; ++idx)
    {
        pt[idx].present = true;
        pt[idx].user_supervisor = false;
        pt[idx].rw = true;
    }

    // Verify setup
    const auto mapped_pml4 = reinterpret_cast<ppml4e>(mm::map_page(__readcr3()));
    if (translate((u64)mapped_pml4) != __readcr3())
        return vmxroot_error_t::vmxroot_translate_failure;

    return vmxroot_error_t::error_success;
}
```

### 2.2 Page Table Structures

**Source:** `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/Sputnik/PayLoad (Intel)/mm.h`

```cpp
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
} pml4e, * ppml4e;

// Similar structures for pdpte, pde, pte...

typedef union _virt_addr_t
{
    u64 value;
    struct
    {
        u64 offset_4kb : 12;
        u64 pt_index : 9;
        u64 pd_index : 9;
        u64 pdpt_index : 9;
        u64 pml4_index : 9;
        u64 reserved : 16;
    };
} virt_addr_t, * pvirt_addr_t;
```

### 2.3 Mapping Guest Physical to Host Virtual

**Source:** `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/Sputnik/PayLoad (Intel)/mm.cpp`

```cpp
auto mm::map_page(host_phys_t phys_addr, map_type_t map_type) -> u64
{
    cpuid_eax_01 cpuid_value;
    __cpuid((int*)&cpuid_value, 1);

    // Use APIC ID to get per-core mapping slot
    mm::pt[(cpuid_value
        .cpuid_additional_information
        .initial_apic_id * 2)
            + (unsigned)map_type].pfn = phys_addr >> 12;

    __invlpg(reinterpret_cast<void*>(
        get_map_virt(virt_addr_t{ phys_addr }.offset_4kb, map_type)));

    return get_map_virt(virt_addr_t{ phys_addr }.offset_4kb, map_type);
}

auto mm::map_guest_phys(guest_phys_t phys_addr, map_type_t map_type) -> u64
{
    const auto host_phys = translate_guest_physical(phys_addr, map_type);
    if (!host_phys)
        return {};
    return map_page(host_phys, map_type);
}
```

### 2.4 Guest Virtual Address Translation

**Source:** `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/Sputnik/PayLoad (Intel)/mm.cpp`

```cpp
auto mm::translate_guest_virtual(guest_phys_t dirbase, guest_virt_t guest_virt,
                                  map_type_t map_type) -> u64
{
    virt_addr_t virt_addr{ guest_virt };

    const auto pml4 = reinterpret_cast<pml4e*>(map_guest_phys(dirbase, map_type));
    if (!pml4[virt_addr.pml4_index].present)
        return {};

    const auto pdpt = reinterpret_cast<pdpte*>(map_guest_phys(
        pml4[virt_addr.pml4_index].pfn << 12, map_type));
    if (!pdpt[virt_addr.pdpt_index].present)
        return {};

    // Handle 1GB large pages
    if (pdpt[virt_addr.pdpt_index].large_page)
        return (pdpt[virt_addr.pdpt_index].pfn << 12) + virt_addr.offset_1gb;

    const auto pd = reinterpret_cast<pde*>(map_guest_phys(
        pdpt[virt_addr.pdpt_index].pfn << 12, map_type));
    if (!pd[virt_addr.pd_index].present)
        return {};

    // Handle 2MB large pages
    if (pd[virt_addr.pd_index].large_page)
        return (pd[virt_addr.pd_index].pfn << 12) + virt_addr.offset_2mb;

    const auto pt = reinterpret_cast<pte*>(map_guest_phys(
        pd[virt_addr.pd_index].pfn << 12, map_type));
    if (!pt[virt_addr.pt_index].present)
        return {};

    return (pt[virt_addr.pt_index].pfn << 12) + virt_addr.offset_4kb;
}
```

### 2.5 EPT Translation (Nested Virtualization)

When running under Hyper-V, guest physical addresses must be translated through EPT to get host physical addresses.

**Source:** `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/Sputnik/PayLoad (Intel)/mm.cpp`

```cpp
auto mm::translate_guest_physical(guest_phys_t phys_addr, map_type_t map_type) -> u64
{
    ept_pointer eptp;
    phys_addr_t guest_phys{ phys_addr };
    __vmx_vmread(VMCS_CTRL_EPT_POINTER, (size_t*)&eptp);

    const auto epml4 = reinterpret_cast<ept_pml4e*>(
        map_page(eptp.page_frame_number << 12, map_type));

    const auto epdpt_large = reinterpret_cast<ept_pdpte_1gb*>(map_page(
        epml4[guest_phys.pml4_index].page_frame_number << 12, map_type));

    // Handle 1GB EPT page
    if (epdpt_large[guest_phys.pdpt_index].large_page)
        return (epdpt_large[guest_phys.pdpt_index].page_frame_number
            * 0x1000 * 0x200 * 0x200) + EPT_LARGE_PDPTE_OFFSET(phys_addr);

    const auto epdpt = reinterpret_cast<ept_pdpte*>(epdpt_large);
    const auto epd_large = reinterpret_cast<epde_2mb*>(map_page(
        epdpt[guest_phys.pdpt_index].page_frame_number << 12, map_type));

    // Handle 2MB EPT page
    if (epd_large[guest_phys.pd_index].large_page)
        return (epd_large[guest_phys.pd_index].page_frame_number
            * 0x1000 * 0x200) + EPT_LARGE_PDE_OFFSET(phys_addr);

    const auto epd = reinterpret_cast<ept_pde*>(epd_large);
    const auto ept = reinterpret_cast<ept_pte*>(map_page(
        epd[guest_phys.pd_index].page_frame_number << 12, map_type));

    return ept[guest_phys.pt_index].page_frame_number << 12;
}
```

### 2.6 Cross-Process Virtual Memory Copy

This is the primitive that enables reading from any process's address space.

**Source:** `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/Sputnik/PayLoad (Intel)/mm.cpp`

```cpp
auto mm::copy_guest_virt(guest_phys_t dirbase_src, guest_virt_t virt_src,
    guest_virt_t dirbase_dest, guest_virt_t virt_dest, u64 size) -> vmxroot_error_t
{
    while (size)
    {
        // Handle page boundary crossing
        auto dest_size = PAGE_4KB - virt_addr_t{ virt_dest }.offset_4kb;
        if (size < dest_size)
            dest_size = size;

        auto src_size = PAGE_4KB - virt_addr_t{ virt_src }.offset_4kb;
        if (size < src_size)
            src_size = size;

        const auto mapped_src = reinterpret_cast<void*>(
            map_guest_virt(dirbase_src, virt_src, map_type_t::map_src));
        if (!mapped_src)
            return vmxroot_error_t::invalid_guest_virtual;

        const auto mapped_dest = reinterpret_cast<void*>(
            map_guest_virt(dirbase_dest, virt_dest, map_type_t::map_dest));
        if (!mapped_dest)
            return vmxroot_error_t::invalid_guest_virtual;

        auto current_size = min(dest_size, src_size);
        memcpy(mapped_dest, mapped_src, current_size);

        virt_src += current_size;
        virt_dest += current_size;
        size -= current_size;
    }

    return vmxroot_error_t::error_success;
}
```

---

## 3. PE Loading from Hypervisor

### 3.1 PE Image Class

The `drv_image` class handles PE parsing and preparation.

**Source:** `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/Sputnik/libsputnik/mapper/drv_image.h`

```cpp
class drv_image
{
    std::vector<uint8_t> m_image;         // Raw PE file
    std::vector<uint8_t> m_image_mapped;  // Mapped image
    PIMAGE_DOS_HEADER m_dos_header = nullptr;
    PIMAGE_NT_HEADERS64 m_nt_headers = nullptr;
    PIMAGE_SECTION_HEADER m_section_header = nullptr;
public:
    explicit drv_image(std::vector<uint8_t> image);
    void map();                            // Map sections
    void* data();                          // Get mapped image
    size_t size() const;                   // Get SizeOfImage
    size_t header_size();                  // Get SizeOfHeaders
    uintptr_t entry_point() const;         // Get AddressOfEntryPoint
    void relocate(void* base) const;       // Apply relocations
    void fix_imports(const std::function<uintptr_t(const char*, const char*)> get_function);
};
```

### 3.2 Header Parsing

**Source:** `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/Sputnik/libsputnik/mapper/drv_image.cpp`

```cpp
drv_image::drv_image(std::vector<uint8_t> image) : m_image(std::move(image))
{
    m_dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(m_image.data());
    m_nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS64>(
        (uintptr_t)m_dos_header + m_dos_header->e_lfanew);
    m_section_header = reinterpret_cast<IMAGE_SECTION_HEADER*>(
        (uintptr_t)(&m_nt_headers->OptionalHeader) +
        m_nt_headers->FileHeader.SizeOfOptionalHeader);
}

size_t drv_image::size() const
{
    return m_nt_headers->OptionalHeader.SizeOfImage;
}

uintptr_t drv_image::entry_point() const
{
    return m_nt_headers->OptionalHeader.AddressOfEntryPoint;
}
```

### 3.3 Section Mapping

**Source:** `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/Sputnik/libsputnik/mapper/drv_image.cpp`

```cpp
void drv_image::map()
{
    m_image_mapped.clear();
    m_image_mapped.resize(m_nt_headers->OptionalHeader.SizeOfImage);

    // Copy headers
    std::copy_n(m_image.begin(),
        m_nt_headers->OptionalHeader.SizeOfHeaders,
        m_image_mapped.begin());

    // Copy each section to its virtual address
    for (size_t i = 0; i < m_nt_headers->FileHeader.NumberOfSections; ++i)
    {
        const auto& section = m_section_header[i];
        const auto target = (uintptr_t)m_image_mapped.data() + section.VirtualAddress;
        const auto source = (uintptr_t)m_dos_header + section.PointerToRawData;

        std::copy_n(m_image.begin() + section.PointerToRawData,
            section.SizeOfRawData,
            m_image_mapped.begin() + section.VirtualAddress);
    }
}
```

### 3.4 Relocation Processing

**Source:** `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/Sputnik/libsputnik/mapper/drv_image.cpp`

```cpp
void drv_image::relocate(void* base) const
{
    if (m_nt_headers->FileHeader.Characteristics & IMAGE_FILE_RELOCS_STRIPPED)
        return;

    ULONG total_count_bytes;
    const auto nt_headers = ImageNtHeader((void*)m_image_mapped.data());

    auto relocation_directory = (PIMAGE_BASE_RELOCATION)::ImageDirectoryEntryToData(
        nt_headers, TRUE, IMAGE_DIRECTORY_ENTRY_BASERELOC, &total_count_bytes);

    auto image_base_delta = static_cast<uintptr_t>(
        reinterpret_cast<uintptr_t>(base) - nt_headers->OptionalHeader.ImageBase);

    if (!(image_base_delta != 0 && total_count_bytes > 0))
        return;

    void* relocation_end = reinterpret_cast<uint8_t*>(relocation_directory) +
        total_count_bytes;

    while (relocation_directory < relocation_end)
    {
        auto relocation_base = ::ImageRvaToVa(nt_headers,
            (void*)m_image_mapped.data(),
            relocation_directory->VirtualAddress, nullptr);

        auto num_relocs = (relocation_directory->SizeOfBlock - 8) >> 1;
        auto relocation_data = reinterpret_cast<PWORD>(relocation_directory + 1);

        for (unsigned long i = 0; i < num_relocs; ++i, ++relocation_data)
            process_relocation(image_base_delta, *relocation_data,
                (uint8_t*)relocation_base);

        relocation_directory = reinterpret_cast<PIMAGE_BASE_RELOCATION>(relocation_data);
    }
}

bool drv_image::process_relocation(uintptr_t image_base_delta,
    uint16_t data, uint8_t* relocation_base)
{
#define IMR_RELOFFSET(x) (x & 0xFFF)
    switch (data >> 12 & 0xF)
    {
    case IMAGE_REL_BASED_HIGH:
    {
        const auto raw_address = reinterpret_cast<int16_t*>(
            relocation_base + IMR_RELOFFSET(data));
        *raw_address += static_cast<unsigned long>(HIWORD(image_base_delta));
        break;
    }
    case IMAGE_REL_BASED_LOW:
    {
        const auto raw_address = reinterpret_cast<int16_t*>(
            relocation_base + IMR_RELOFFSET(data));
        *raw_address += static_cast<unsigned long>(LOWORD(image_base_delta));
        break;
    }
    case IMAGE_REL_BASED_HIGHLOW:
    {
        const auto raw_address = reinterpret_cast<size_t*>(
            relocation_base + IMR_RELOFFSET(data));
        *raw_address += static_cast<size_t>(image_base_delta);
        break;
    }
    case IMAGE_REL_BASED_DIR64:
    {
        auto UNALIGNED raw_address = reinterpret_cast<DWORD_PTR UNALIGNED*>(
            relocation_base + IMR_RELOFFSET(data));
        *raw_address += image_base_delta;
        break;
    }
    case IMAGE_REL_BASED_ABSOLUTE:
    case IMAGE_REL_BASED_HIGHADJ:
        break;  // No action required
    default:
        return false;
    }
#undef IMR_RELOFFSET
    return true;
}
```

---

## 4. Import Resolution

### 4.1 Import Table Walking

**Source:** `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/Sputnik/libsputnik/mapper/drv_image.cpp`

```cpp
void drv_image::fix_imports(
    const std::function<uintptr_t(const char*, const char*)> get_function)
{
    ULONG size;
    auto import_descriptors = static_cast<PIMAGE_IMPORT_DESCRIPTOR>(
        ::ImageDirectoryEntryToData(m_image.data(), FALSE,
            IMAGE_DIRECTORY_ENTRY_IMPORT, &size));

    if (!import_descriptors)
        return;

    for (; import_descriptors->Name; import_descriptors++)
    {
        IMAGE_THUNK_DATA* image_thunk_data;
        const auto module_name = get_rva<char>(import_descriptors->Name);

        if (import_descriptors->OriginalFirstThunk)
            image_thunk_data = get_rva<IMAGE_THUNK_DATA>(
                import_descriptors->OriginalFirstThunk);
        else
            image_thunk_data = get_rva<IMAGE_THUNK_DATA>(
                import_descriptors->FirstThunk);

        auto image_func_data = get_rva<IMAGE_THUNK_DATA64>(
            import_descriptors->FirstThunk);

        for (; image_thunk_data->u1.AddressOfData; image_thunk_data++, image_func_data++)
        {
            uintptr_t function_address;
            const auto ordinal = (image_thunk_data->u1.Ordinal & IMAGE_ORDINAL_FLAG64) != 0;
            const auto image_import_by_name = get_rva<IMAGE_IMPORT_BY_NAME>(
                *(DWORD*)image_thunk_data);
            const auto name_of_import = static_cast<char*>(image_import_by_name->Name);

            function_address = get_function(module_name, name_of_import);
            image_func_data->u1.Function = function_address;
        }
    }
}
```

### 4.2 Finding ntoskrnl Exports Without PsLoadedModuleList

From the hypervisor, we must locate ntoskrnl and walk its export table directly.

**Option A: Signature-based kernel discovery**
```cpp
// Find ntoskrnl by scanning for known patterns
// Example: MZ header followed by PE signature at consistent offset

auto find_ntoskrnl_base() -> u64
{
    // Scan from typical kernel load addresses
    const u64 kernel_start = 0xFFFFF80000000000;
    const u64 kernel_end   = 0xFFFFF88000000000;

    for (u64 addr = kernel_start; addr < kernel_end; addr += 0x1000)
    {
        auto mapped = mm::map_guest_phys(translate_guest_virtual(
            system_cr3, addr));
        if (!mapped) continue;

        auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(mapped);
        if (dos->e_magic != IMAGE_DOS_SIGNATURE) continue;

        auto nt = reinterpret_cast<IMAGE_NT_HEADERS64*>(
            (u64)dos + dos->e_lfanew);
        if (nt->Signature != IMAGE_NT_SIGNATURE) continue;

        // Verify it's ntoskrnl by checking export names
        // Look for "NtCreateFile", "PsGetCurrentProcess", etc.
        if (verify_ntoskrnl_exports(addr))
            return addr;
    }
    return 0;
}
```

**Option B: IDT-based discovery**
```cpp
// Read IDTR to find IDT base
// IDT entries point to kernel routines in ntoskrnl
// Walk back to find module base

auto find_ntoskrnl_from_idt() -> u64
{
    // Read guest IDTR
    u64 idtr_base;
    __vmx_vmread(VMCS_GUEST_IDTR_BASE, &idtr_base);

    // Read interrupt 0 handler address (divide error)
    auto idt = mm::map_guest_virt(system_cr3, idtr_base);
    auto entry = reinterpret_cast<KIDTENTRY64*>(idt);

    u64 handler = ((u64)entry->OffsetHigh << 32) |
                  ((u64)entry->OffsetMiddle << 16) |
                  entry->OffsetLow;

    // Walk back to find MZ header
    return find_module_base(handler);
}
```

### 4.3 Export Table Walking

**Reference:** A8_DRIVER_MAPPER_KNOWLEDGE.md

```cpp
auto get_export(u8* base, const char* export_name) -> u64
{
    auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE)
        return 0;

    auto nt = reinterpret_cast<IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);

    u32 exports_rva = nt->OptionalHeader.DataDirectory[
        IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    if (!exports_rva)
        return 0;

    auto exports = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(base + exports_rva);
    auto names = reinterpret_cast<u32*>(base + exports->AddressOfNames);

    for (u32 i = 0; i < exports->NumberOfNames; ++i)
    {
        const char* func_name = reinterpret_cast<const char*>(base + names[i]);

        if (strcmp(func_name, export_name) == 0)
        {
            auto funcs = reinterpret_cast<u32*>(base + exports->AddressOfFunctions);
            auto ordinals = reinterpret_cast<u16*>(base + exports->AddressOfNameOrdinals);
            return (u64)base + funcs[ordinals[i]];
        }
    }

    return 0;
}
```

### 4.4 Handling Forwarded Exports

```cpp
auto resolve_forwarded_export(u8* base, u32 func_rva,
    u32 export_dir_rva, u32 export_dir_size) -> u64
{
    // Check if the RVA falls within the export directory
    if (func_rva >= export_dir_rva &&
        func_rva < export_dir_rva + export_dir_size)
    {
        // This is a forwarded export
        // Format: "MODULE.FunctionName"
        const char* forward = reinterpret_cast<const char*>(base + func_rva);

        // Parse module name and function name
        char module_name[64] = {0};
        char func_name[64] = {0};

        const char* dot = strchr(forward, '.');
        if (!dot) return 0;

        memcpy(module_name, forward, dot - forward);
        strcpy(func_name, dot + 1);

        // Resolve from forwarded module
        u64 forwarded_module = get_module_base(module_name);
        if (!forwarded_module) return 0;

        return get_export((u8*)forwarded_module, func_name);
    }

    return (u64)base + func_rva;
}
```

---

## 5. Memory Allocation

### 5.1 Sputnik Approach (Pool Allocation via Syscall)

**Source:** `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/Sputnik/libsputnik/mapper/kernel_ctx.cpp`

```cpp
void* kernel_ctx::allocate_pool(std::size_t size, POOL_TYPE pool_type)
{
    static const auto ex_alloc_pool =
        util::get_kernel_export("ntoskrnl.exe", "ExAllocatePool");

    return syscall<ExAllocatePool>(
        ex_alloc_pool,
        pool_type,
        size
    );
}

void* kernel_ctx::allocate_pool(std::size_t size, ULONG pool_tag,
    POOL_TYPE pool_type)
{
    static const auto ex_alloc_pool_with_tag =
        util::get_kernel_export("ntoskrnl.exe", "ExAllocatePoolWithTag");

    return syscall<ExAllocatePoolWithTag>(
        ex_alloc_pool_with_tag,
        pool_type,
        size,
        pool_tag
    );
}
```

### 5.2 Hypervisor Approach (Direct PTE Manipulation)

For Ombra, we allocate memory by manipulating kernel page tables directly from ring -1.

```cpp
// Find free physical pages (high memory, beyond normal RAM)
auto find_free_physical_pages(u64 count) -> u64
{
    // Option 1: Use reserved high memory region
    // Physical addresses above actual RAM but below 4GB/MMIO

    // Option 2: Scan MmPfnDatabase for Free/Standby pages
    // This requires knowing the PFN database address

    // Option 3: Use memory range not mapped by Windows
    // Scan EPT to find unmapped guest physical ranges

    return free_base_pa;
}

// Find unmapped kernel VA space
auto find_free_kernel_va(u64 size) -> u64
{
    // System PTE region: 0xFFFFF88000000000 - 0xFFFFF8FFFFFFFFFF
    const u64 system_pte_start = 0xFFFFF88000000000;
    const u64 system_pte_end   = 0xFFFFF8FFFFFFFFFF;

    // Scan for consecutive pages with non-present PTEs
    for (u64 va = system_pte_start; va < system_pte_end; va += PAGE_SIZE)
    {
        bool all_free = true;
        for (u64 offset = 0; offset < size; offset += PAGE_SIZE)
        {
            auto pte = get_pte_for_va(system_cr3, va + offset);
            if (pte && pte->present)
            {
                all_free = false;
                break;
            }
        }
        if (all_free)
            return va;
    }
    return 0;
}

// Create PTEs mapping physical pages to kernel VA
auto create_kernel_mapping(u64 kernel_va, u64 physical_base, u64 size) -> bool
{
    for (u64 offset = 0; offset < size; offset += PAGE_SIZE)
    {
        auto pte_addr = get_pte_address(system_cr3, kernel_va + offset);

        pte new_pte = {0};
        new_pte.present = 1;
        new_pte.rw = 1;
        new_pte.user_supervisor = 0;  // Kernel only
        new_pte.pfn = (physical_base + offset) >> 12;

        // Write PTE via guest physical memory
        auto pte_pa = translate_guest_virtual(system_cr3, pte_addr);
        auto mapped_pte = mm::map_guest_phys(pte_pa);
        *reinterpret_cast<pte*>(mapped_pte) = new_pte;
    }

    // Invalidate TLB (will happen naturally via IPI or context switch)
    return true;
}
```

### 5.3 Allocation Strategy Comparison

| Method | Pros | Cons |
|--------|------|------|
| Pool via syscall | Standard, tracked | Leaves traces, requires syscall hook |
| Direct PTE manipulation | No traces, invisible | Complex, TLB management |
| Reserved physical range | Simple, predictable | Wastes memory, may conflict |
| MmPfnDatabase | Uses free pages | Requires PFN database address |

---

## 6. Entry Point Invocation

### 6.1 Driver Entry Prototype

```cpp
typedef NTSTATUS (*PDRIVER_INITIALIZE)(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath
);
```

### 6.2 Sputnik Approach (Syscall Hook)

**Source:** `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/Sputnik/libsputnik/mapper/map_driver.cpp`

```cpp
NTSTATUS mapper::map_driver(const std::vector<std::uint8_t>& driver,
    uintptr_t param1, uintptr_t param2,
    bool bAllocationPtrParam1, bool bAllocationSizeParam2, uintptr_t* allocBase)
{
    mapper::drv_image image(driver);
    mapper::kernel_ctx ctx;

    // Resolve imports
    const auto _get_export_name = [&](const char* base, const char* name) {
        return reinterpret_cast<std::uintptr_t>(
            util::get_kernel_export(base, name));
    };
    image.fix_imports(_get_export_name);
    image.map();

    // Allocate kernel pool
    void* pool_base = ctx.allocate_pool(image.size(), NonPagedPool);
    *allocBase = (uintptr_t)pool_base;

    // Apply relocations and write to kernel
    image.relocate(pool_base);
    ctx.write_kernel(pool_base, image.data(), image.size());

    // Calculate entry point
    auto entry_point = reinterpret_cast<std::uintptr_t>(pool_base) +
        image.entry_point();

    // Call entry via syscall hook
    auto result = ctx.syscall<DRIVER_INITIALIZE>(
        (PVOID)entry_point,
        bAllocationPtrParam1 ? (uintptr_t)pool_base : param1,
        bAllocationSizeParam2 ? (uintptr_t)image.size() : param2
    );

    return result;
}
```

### 6.3 Hypervisor Entry Invocation Methods

**Method A: Kernel Function Hook**

Hook a frequently-called kernel function to redirect to driver entry.

```cpp
// Hook ExAllocatePool or similar
auto hook_kernel_function_for_entry(u64 entry_point) -> bool
{
    // Find a suitable kernel function
    u64 target_func = get_export(ntoskrnl_base, "ExAllocatePool");

    // Save original bytes
    u8 original_bytes[16];
    mm::copy_guest_virt(system_cr3, target_func,
        caller_cr3, (u64)original_bytes, 16);

    // Create trampoline that calls our entry then original
    u8 trampoline[] = {
        0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0,  // mov rax, entry_point
        0xFF, 0xD0,                            // call rax
        0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0,  // mov rax, original_func
        0xFF, 0xE0                             // jmp rax
    };
    *reinterpret_cast<u64*>(&trampoline[2]) = entry_point;
    *reinterpret_cast<u64*>(&trampoline[14]) = /* original handler */;

    // Install hook
    // ...
}
```

**Method B: APC Injection**

Queue an APC in the System process that calls driver entry.

```cpp
auto inject_apc_for_entry(u64 entry_point) -> bool
{
    // Find System process EPROCESS
    u64 system_eprocess = get_system_eprocess();

    // Allocate KAPC structure in kernel memory
    // Set APC routine to our entry point
    // Insert into System thread's APC queue

    // The entry will be called at PASSIVE_LEVEL during next
    // kernel-to-user transition
}
```

**Method C: VMExit RIP Injection**

Modify guest RIP during a VMExit to call driver entry.

```cpp
// In VMExit handler, when a specific trigger occurs:
auto inject_via_vmexit(u64 entry_point, pcontext_t context) -> void
{
    // Only do this once, on a specific trigger
    if (!entry_called && /* trigger condition */)
    {
        // Save original state
        saved_rip = context->rip;
        saved_rcx = context->rcx;
        saved_rdx = context->rdx;

        // Setup call to entry point
        // RCX = fake DRIVER_OBJECT (or NULL)
        // RDX = NULL registry path
        context->rcx = 0;  // Or pointer to minimal DRIVER_OBJECT
        context->rdx = 0;

        // Redirect execution
        size_t rip;
        __vmx_vmread(VMCS_GUEST_RIP, &rip);
        __vmx_vmwrite(VMCS_GUEST_RIP, entry_point);

        // Install return hook to restore state
        entry_called = true;
    }
}
```

---

## 7. Complete Mapping Flow

### 7.1 Ombra Driver Mapping Sequence

```
1. Usermode (OmbraMapper.exe)
   ├── Load driver PE file into memory
   ├── Lock memory pages (VirtualLock)
   └── Issue hypercall: OMBRA_HYPERCALL_MAP_DRIVER

2. Hypervisor (OmbraPayload)
   ├── Validate request (authentication key, caller PID)
   ├── Read driver image from caller's address space
   │   └── mm::copy_guest_virt(caller_cr3, driver_va, hv_cr3, hv_buffer, size)
   │
   ├── Allocate kernel memory
   │   ├── find_free_physical_pages(image_size)
   │   ├── find_free_kernel_va(image_size)
   │   └── create_kernel_mapping(kernel_va, physical_base, image_size)
   │
   ├── Parse and map PE
   │   ├── Validate DOS/NT headers
   │   ├── Copy headers to kernel VA
   │   └── Copy sections to kernel VA
   │
   ├── Apply relocations
   │   └── For each relocation: adjust absolute addresses
   │
   ├── Resolve imports
   │   ├── Find ntoskrnl base (signature scan or IDT)
   │   ├── For each import: walk export table
   │   └── Write resolved addresses to IAT
   │
   ├── Call driver entry
   │   └── Via hook/APC/VMExit injection
   │
   └── Return result to usermode
       └── guest_registers->rax = mapped_base or error

3. Return to Usermode
   └── OmbraMapper.exe receives mapped base address
```

### 7.2 VMExit Handler Integration

```cpp
void vmexit_handler(pcontext_t* context, void* unknown)
{
    pcontext_t guest_registers = *context;

    size_t vmexit_reason;
    __vmx_vmread(VMCS_EXIT_REASON, &vmexit_reason);

    if (vmexit_reason == VMX_EXIT_REASON_EXECUTE_CPUID)
    {
        if (guest_registers->rcx == OMBRA_VMEXIT_KEY)
        {
            switch ((ombra_command_t)guest_registers->rdx)
            {
            case ombra_command_t::map_driver:
            {
                auto cmd = get_command(guest_registers->r8);

                // Read driver from caller's address space
                u64 caller_cr3;
                __vmx_vmread(VMCS_GUEST_CR3, &caller_cr3);

                std::vector<u8> driver_image(cmd.map_driver.size);
                mm::copy_guest_virt(
                    caller_cr3, cmd.map_driver.image_va,
                    /* hv_cr3 */, (u64)driver_image.data(),
                    cmd.map_driver.size);

                // Perform mapping
                auto result = ombra::map_driver_internal(
                    driver_image.data(),
                    driver_image.size());

                cmd.map_driver.out_base = result.base;
                cmd.map_driver.out_entry = result.entry;
                set_command(guest_registers->r8, cmd);

                guest_registers->rax = (u64)result.status;
                break;
            }
            // ... other commands
            }

            // Advance RIP past CPUID
            size_t rip, exec_len;
            __vmx_vmread(VMCS_GUEST_RIP, &rip);
            __vmx_vmread(VMCS_VMEXIT_INSTRUCTION_LENGTH, &exec_len);
            __vmx_vmwrite(VMCS_GUEST_RIP, rip + exec_len);
            return;
        }
    }

    // Call original handler
    reinterpret_cast<vmexit_handler_t>(
        reinterpret_cast<u64>(&vmexit_handler) -
            ombra_context.vmexit_handler_rva)(context, unknown);
}
```

---

## 8. Stealth Considerations

### 8.1 What to Avoid

After mapping, the driver should NOT appear in:

| Detection Vector | Standard Drivers | Manually Mapped |
|------------------|------------------|-----------------|
| PsLoadedModuleList | Listed | Not listed |
| PiDDBCacheTable | Entry created | No entry |
| MmUnloadedDrivers | Entry on unload | No entry |
| Pool tags | Trackable | Custom/None |
| VAD structures | Present | Not present |
| ETW events | Logged | Not logged |
| Driver certificates | Verified | N/A |

### 8.2 Detection Evasion Checklist

```
[ ] No PsLoadedModuleList entry (unless added for stability)
[ ] No PiDDBCacheTable entry
[ ] No MmUnloadedDrivers entry
[ ] Pool allocations use untraceable method
[ ] No registry entries created
[ ] No kernel callbacks via standard APIs
[ ] Import resolution done internally
[ ] Entry called via non-standard method
[ ] No usermode component after mapping
```

### 8.3 Optional: Adding to PsLoadedModuleList

For stability with some kernel APIs, you may need to add the driver to PsLoadedModuleList:

```cpp
// This is optional and creates a detection vector
// Only do this if absolutely necessary for stability

auto add_to_loaded_modules(u64 base, u64 size, const wchar_t* name) -> bool
{
    // Find PsLoadedModuleList head
    // Allocate LDR_DATA_TABLE_ENTRY
    // Fill in fields: DllBase, SizeOfImage, FullDllName, BaseDllName
    // Insert into list (acquire lock first)
    // This makes the driver visible to MmGetSystemRoutineAddress, etc.
}
```

---

## 9. Proposed Ombra Hypercall Interface

### 9.1 Command Enumeration

```cpp
enum class ombra_hypercall_t : u64 {
    // Core operations
    OMBRA_INIT              = 0x4F4D4252'00000001,  // "OMBR" + 1
    OMBRA_GET_CR3           = 0x4F4D4252'00000002,
    OMBRA_READ_PHYS         = 0x4F4D4252'00000003,
    OMBRA_WRITE_PHYS        = 0x4F4D4252'00000004,
    OMBRA_READ_VIRT         = 0x4F4D4252'00000005,
    OMBRA_WRITE_VIRT        = 0x4F4D4252'00000006,
    OMBRA_TRANSLATE         = 0x4F4D4252'00000007,

    // Driver mapping
    OMBRA_MAP_DRIVER        = 0x4F4D4252'00000010,
    OMBRA_UNMAP_DRIVER      = 0x4F4D4252'00000011,
    OMBRA_CALL_DRIVER_ENTRY = 0x4F4D4252'00000012,

    // EPT/NPT hooks
    OMBRA_HOOK_PAGE         = 0x4F4D4252'00000020,
    OMBRA_UNHOOK_PAGE       = 0x4F4D4252'00000021,

    // Auth
    OMBRA_SET_AUTH_KEY      = 0x4F4D4252'00000030,
    OMBRA_SET_AUTH_PID      = 0x4F4D4252'00000031,
};
```

### 9.2 Request Structures

```cpp
struct ombra_map_driver_request {
    u64 image_va;           // Usermode buffer with PE image
    u64 image_size;         // Size of PE image
    u64 out_mapped_base;    // Output: kernel VA of mapped driver
    u64 out_entry_point;    // Output: VA of driver entry
    u64 status;             // Output: status code
};

struct ombra_memory_request {
    u64 target_va;          // Target virtual address
    u64 buffer_va;          // Buffer virtual address
    u64 size;               // Size to read/write
    u64 target_cr3;         // Target process CR3 (0 = current)
};

struct ombra_translate_request {
    u64 virtual_address;    // VA to translate
    u64 cr3;                // Page table base (0 = current)
    u64 physical_address;   // Output: resulting PA
};
```

---

## 10. Error Codes

```cpp
enum class ombra_error_t : u64 {
    success                 = 0,
    invalid_key             = 1,
    invalid_pid             = 2,
    invalid_command         = 3,
    invalid_parameter       = 4,
    translate_failure       = 5,
    mapping_failure         = 6,
    allocation_failure      = 7,
    relocation_failure      = 8,
    import_failure          = 9,
    entry_call_failure      = 10,
    already_initialized     = 11,
    not_initialized         = 12,
};
```

---

## 11. Reference Files

| File | Purpose |
|------|---------|
| `/Refs/Sputnik/PayLoad (Intel)/vmexit_handler.cpp` | VMExit dispatch implementation |
| `/Refs/Sputnik/PayLoad (Intel)/mm.cpp` | Physical memory access from VMX root |
| `/Refs/Sputnik/PayLoad (Intel)/mm.h` | Page table structures |
| `/Refs/Sputnik/PayLoad (Intel)/types.h` | Command structures, error codes |
| `/Refs/Sputnik/PayLoad (Intel)/vmexit.cpp` | Command get/set helpers |
| `/Refs/Sputnik/libsputnik/libsputnik.hpp` | Usermode hypercall interface |
| `/Refs/Sputnik/libsputnik/libsputnik.cpp` | Usermode hypercall implementation |
| `/Refs/Sputnik/libsputnik/mapper/map_driver.cpp` | Driver mapping orchestration |
| `/Refs/Sputnik/libsputnik/mapper/drv_image.cpp` | PE parsing and section mapping |
| `/Refs/Sputnik/libsputnik/mapper/kernel_ctx.cpp` | Kernel memory operations |
| `/Refs/Sputnik/SKLib/SKLib/SKLib-v/include/Vmcall.h` | VMCALL_TYPE enum, RW class |
| `/Refs/Sputnik/SKLib/SKLib/SKLib-v/include/VMMDef.h` | VMM structures, READ_DATA, WRITE_DATA |

---

## 12. Implementation Phases for Ombra

### Phase 1: Physical Memory Access
- Implement `mm::init()` in OmbraPayload
- Implement guest VA translation through EPT
- Test by reading ntoskrnl headers

### Phase 2: Kernel Discovery
- Implement ntoskrnl base discovery (IDT or signature)
- Implement export table walking
- Cache ntoskrnl base and common exports

### Phase 3: Memory Allocation
- Implement physical page allocation strategy
- Implement kernel VA space finding
- Implement PTE creation from ring -1

### Phase 4: PE Loading
- Port drv_image class to payload context
- Implement section mapping via mm::copy_guest_virt
- Implement relocation processing

### Phase 5: Import Resolution
- Implement full export table walking
- Handle forwarded exports
- Support ordinal imports

### Phase 6: Entry Invocation
- Choose entry call method (hook/APC/VMExit)
- Implement entry trigger mechanism
- Handle return value propagation

### Phase 7: Hypercall Integration
- Define stable OMBRA_HYPERCALL_MAP_DRIVER interface
- Implement request validation
- Return mapped base to usermode

---

*Document Version: 1.0*
*Generated for OmbraHypervisor Project*
*Primary Reference: Sputnik libsputnik/mapper and PayLoad (Intel)*
