# A8: Driver Mapper Knowledge Base

## Executive Summary

This document provides comprehensive knowledge for implementing manual driver mapping from a hypervisor context. The techniques analyzed are extracted from multiple reference implementations including umap, Kernel-Bridge, Shark, and Sputnik. Manual mapping bypasses standard driver loading mechanisms (MmLoadSystemImage, IoCreateDriver) and avoids detection by PatchGuard/DSE through direct memory manipulation.

---

## 1. Manual Mapping Overview

### 1.1 Complete Process Flow

Manual mapping involves these sequential steps executed from ring -1:

```
1. Parse PE headers from driver image
2. Allocate kernel memory for mapped image
3. Map PE headers to allocated base
4. Map each section to correct virtual addresses
5. Process base relocations for new image base
6. Resolve imports from kernel modules (ntoskrnl, hal)
7. Execute TLS callbacks (if present)
8. Call driver entry point
```

### 1.2 Key Differences: Hypervisor vs Kernel Context

| Aspect | Kernel-Mode Mapping | Hypervisor (Ring -1) Mapping |
|--------|--------------------|-----------------------------|
| Memory Allocation | ExAllocatePool, MmAllocateContiguous | Direct PTE manipulation, physical page reservation |
| Module Enumeration | ZwQuerySystemInformation | PsLoadedModuleList traversal via guest physical memory |
| Import Resolution | MmGetSystemRoutineAddress | Export table walking through guest VA translation |
| Entry Point Call | Direct function call | VMExit injection or kernel hook |
| Detection Surface | Pool allocation tracked | Completely invisible to ring 0 |

Reference: `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/umap/boot/main.c` (lines 273-431)

---

## 2. PE Parser Implementation

### 2.1 DOS Header Validation

Every PE file begins with a DOS header containing the magic number 0x5A4D ("MZ"):

```c
// Reference: umap/mapper/main.c:62-66
PIMAGE_DOS_HEADER dosHeaders = (PIMAGE_DOS_HEADER)buffer;
if (dosHeaders->e_magic != IMAGE_DOS_SIGNATURE) {
    sprintf(error, "Image does not have DOS signature");
    return STATUS_NOT_SUPPORTED;
}
```

The `e_lfanew` field provides the offset to the NT headers.

### 2.2 NT Headers Parsing

```c
// Reference: umap/mapper/main.c:68-74
PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(buffer + dosHeaders->e_lfanew);

// Critical fields:
// - FileHeader.NumberOfSections: Count of sections to map
// - FileHeader.SizeOfOptionalHeader: Offset to section headers
// - OptionalHeader.SizeOfImage: Total memory needed
// - OptionalHeader.ImageBase: Preferred load address (for relocations)
// - OptionalHeader.AddressOfEntryPoint: RVA of DriverEntry
```

### 2.3 Section Enumeration

Section headers immediately follow the optional header:

```c
// Reference: umap/boot/main.c:313-324
IMAGE_SECTION_HEADER *sections =
    (IMAGE_SECTION_HEADER *)((UINT8 *)&ntHeaders->OptionalHeader +
        ntHeaders->FileHeader.SizeOfOptionalHeader);

for (UINT16 i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i) {
    IMAGE_SECTION_HEADER *section = &sections[i];
    if (section->SizeOfRawData) {
        MemCopy(mapperBase + section->VirtualAddress,
            mapperBuffer + section->PointerToRawData,
            section->SizeOfRawData);
    }
}
```

### 2.4 Data Directory Access

The optional header contains an array of data directories for special structures:

```c
// Important directories:
#define IMAGE_DIRECTORY_ENTRY_EXPORT    0  // Export table
#define IMAGE_DIRECTORY_ENTRY_IMPORT    1  // Import table
#define IMAGE_DIRECTORY_ENTRY_BASERELOC 5  // Base relocations

UINT32 importsRva = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
```

---

## 3. Kernel Memory Allocation

### 3.1 Traditional Kernel Allocation

From kernel mode, allocation uses pool APIs:

```c
// Reference: umap/mapper/main.c:70-74
PBYTE base = ExAllocatePool(NonPagedPoolExecute, ntHeaders->OptionalHeader.SizeOfImage);
if (!base) {
    sprintf(error, "Failed to allocate pool of size 0x%X", ntHeaders->OptionalHeader.SizeOfImage);
    return STATUS_NO_MEMORY;
}
```

### 3.2 Boot-Time Allocation (Winload Context)

The umap bootkit hooks `BlImgAllocateImageBuffer` to allocate memory in the Windows boot context before kernel initialization:

```c
// Reference: umap/boot/main.c:201-231
EFI_STATUS EFIAPI BlImgAllocateImageBufferHook(VOID **imageBuffer,
    UINTN imageSize,
    UINT32 memoryType,
    UINT32 attributes, VOID *unused,
    UINT32 flags) {

    // Allocate mapper's buffer alongside kernel image
    if (memoryType == BL_MEMORY_TYPE_APPLICATION) {
        mapper.AllocatedBufferStatus = BlImgAllocateImageBuffer(
            &mapper.AllocatedBuffer, MAPPER_BUFFER_SIZE, memoryType,
            BL_MEMORY_ATTRIBUTE_RWX, unused, 0);
    }
}
```

### 3.3 Hypervisor-Level Allocation

From ring -1, memory allocation requires direct page table manipulation:

```c
// Conceptual approach for Ombra:
// 1. Find free physical pages (scan MmPfnDatabase or use reserved range)
// 2. Find unmapped kernel VA space (System PTE region: 0xFFFFF88000000000)
// 3. Create PTEs mapping physical pages to kernel VA
// 4. Write driver image to physical pages

// Page table structure (from Sputnik mm.h:59-143)
typedef union _pte {
    u64 value;
    struct {
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
```

Reference: `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/Sputnik/PayLoad (Intel)/mm.h` (lines 122-143)

---

## 4. Section Mapping

### 4.1 Section Copy Process

Each PE section must be copied to its virtual address offset from the image base:

```c
// Reference: umap/mapper/main.c:79-86
PIMAGE_SECTION_HEADER sections = (PIMAGE_SECTION_HEADER)
    ((PBYTE)&ntHeaders->OptionalHeader + ntHeaders->FileHeader.SizeOfOptionalHeader);

for (USHORT i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i) {
    PIMAGE_SECTION_HEADER section = &sections[i];
    if (section->SizeOfRawData) {
        memcpy(base + section->VirtualAddress,
               buffer + section->PointerToRawData,
               section->SizeOfRawData);
    }
}
```

### 4.2 Section Characteristics

Section flags determine memory protection requirements:

```c
// Common characteristics:
#define IMAGE_SCN_MEM_EXECUTE  0x20000000  // Executable
#define IMAGE_SCN_MEM_READ     0x40000000  // Readable
#define IMAGE_SCN_MEM_WRITE    0x80000000  // Writeable
#define IMAGE_SCN_MEM_DISCARDABLE 0x02000000  // Can be discarded after load
```

For stealth, initially map all pages as RWX during loading, then adjust protections to match intended characteristics.

### 4.3 Header Mapping

PE headers must also be copied to the image base:

```c
// Reference: umap/boot/main.c:310
MemCopy(mapperBase, mapperBuffer, ntHeaders->OptionalHeader.SizeOfHeaders);
```

---

## 5. Relocation Processing

### 5.1 Base Relocation Theory

When an image loads at a different address than its preferred `ImageBase`, all absolute addresses in the code/data must be adjusted. The relocation directory contains entries specifying which addresses need adjustment.

### 5.2 Relocation Block Structure

```c
// Each relocation block:
typedef struct _IMAGE_BASE_RELOCATION {
    ULONG VirtualAddress;  // Page RVA
    ULONG SizeOfBlock;     // Total block size including header
} IMAGE_BASE_RELOCATION;

// Following the header are WORD entries:
// High 4 bits = relocation type
// Low 12 bits = offset within page
```

### 5.3 Relocation Processing Code

```c
// Reference: umap/mapper/main.c:121-155
PIMAGE_DATA_DIRECTORY baseRelocDir = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
if (baseRelocDir->VirtualAddress) {
    PIMAGE_BASE_RELOCATION reloc = (PIMAGE_BASE_RELOCATION)(base + baseRelocDir->VirtualAddress);

    for (UINT32 currentSize = 0; currentSize < baseRelocDir->Size; ) {
        ULONG relocCount = (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(USHORT);
        PUSHORT relocData = (PUSHORT)((PBYTE)reloc + sizeof(IMAGE_BASE_RELOCATION));
        PBYTE relocBase = base + reloc->VirtualAddress;

        for (UINT32 i = 0; i < relocCount; ++i, ++relocData) {
            USHORT data = *relocData;
            USHORT type = data >> 12;
            USHORT offset = data & 0xFFF;

            switch (type) {
                case IMAGE_REL_BASED_ABSOLUTE:
                    break;  // Padding, skip
                case IMAGE_REL_BASED_DIR64: {
                    // 64-bit relocation: add delta to QWORD at offset
                    PULONG64 rva = (PULONG64)(relocBase + offset);
                    *rva = (ULONG64)(base + (*rva - ntHeaders->OptionalHeader.ImageBase));
                    break;
                }
                default:
                    return STATUS_NOT_SUPPORTED;
            }
        }

        currentSize += reloc->SizeOfBlock;
        reloc = (PIMAGE_BASE_RELOCATION)relocData;
    }
}
```

### 5.4 Relocation Types

| Type | Value | Description |
|------|-------|-------------|
| IMAGE_REL_BASED_ABSOLUTE | 0 | Padding, no operation |
| IMAGE_REL_BASED_HIGHLOW | 3 | 32-bit address (x86) |
| IMAGE_REL_BASED_DIR64 | 10 | 64-bit address (x64) |

---

## 6. Import Resolution (Critical)

### 6.1 Import Directory Structure

```c
typedef struct _IMAGE_IMPORT_DESCRIPTOR {
    ULONG OriginalFirstThunk;  // RVA to Import Lookup Table (names/ordinals)
    ULONG TimeDateStamp;
    ULONG ForwarderChain;
    ULONG Name;                // RVA to module name (ASCII)
    ULONG FirstThunk;          // RVA to Import Address Table (to be filled)
} IMAGE_IMPORT_DESCRIPTOR;
```

### 6.2 Import Resolution Process

```c
// Reference: umap/mapper/main.c:88-119
ULONG importsRva = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
if (importsRva) {
    PIMAGE_IMPORT_DESCRIPTOR importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)(base + importsRva);

    for (; importDescriptor->FirstThunk; ++importDescriptor) {
        PCHAR moduleName = (PCHAR)(base + importDescriptor->Name);
        PVOID module = GetModuleBaseAddress(moduleName);
        if (!module) {
            sprintf(error, "Failed to find module %s", moduleName);
            return STATUS_NOT_FOUND;
        }

        PIMAGE_THUNK_DATA64 thunk = (PIMAGE_THUNK_DATA64)(base + importDescriptor->FirstThunk);
        PIMAGE_THUNK_DATA64 thunkOriginal = (PIMAGE_THUNK_DATA64)(base + importDescriptor->OriginalFirstThunk);

        for (; thunk->u1.AddressOfData; ++thunk, ++thunkOriginal) {
            PCHAR importName = ((PIMAGE_IMPORT_BY_NAME)(base + thunkOriginal->u1.AddressOfData))->Name;
            ULONG64 import = GetExport(module, importName);
            if (!import) {
                sprintf(error, "Failed to find export %s in module %s", importName, moduleName);
                return STATUS_NOT_FOUND;
            }

            thunk->u1.Function = import;
        }
    }
}
```

### 6.3 Export Table Walking

To resolve imports, we must walk the export table of the target module:

```c
// Reference: umap/boot/util.c:62-97
UINT64 GetExport(UINT8 *base, CHAR8 *export) {
    IMAGE_DOS_HEADER *dosHeaders = (IMAGE_DOS_HEADER *)base;
    if (dosHeaders->e_magic != IMAGE_DOS_SIGNATURE) {
        return 0;
    }

    IMAGE_NT_HEADERS64 *ntHeaders = (IMAGE_NT_HEADERS64 *)(base + dosHeaders->e_lfanew);

    UINT32 exportsRva = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    if (!exportsRva) {
        return 0;
    }

    IMAGE_EXPORT_DIRECTORY *exports = (IMAGE_EXPORT_DIRECTORY *)(base + exportsRva);

    UINT32 *nameRva = (UINT32 *)(base + exports->AddressOfNames);

    for (UINT32 i = 0; i < exports->NumberOfNames; ++i) {
        CHAR8 *func = (CHAR8 *)(base + nameRva[i]);

        if (AsciiStrCmp(func, export) == 0) {
            UINT32 *funcRva = (UINT32 *)(base + exports->AddressOfFunctions);
            UINT16 *ordinalRva = (UINT16 *)(base + exports->AddressOfNameOrdinals);

            return (UINT64)base + funcRva[ordinalRva[i]];
        }
    }

    return 0;
}
```

### 6.4 Ordinal Imports

For imports by ordinal (high bit set in thunk value):

```c
if (thunkOriginal->u1.Ordinal & IMAGE_ORDINAL_FLAG64) {
    USHORT ordinal = (USHORT)(thunkOriginal->u1.Ordinal & 0xFFFF);
    // Lookup by ordinal - exports->Base is the ordinal base
    UINT32 *funcRva = (UINT32 *)(base + exports->AddressOfFunctions);
    return (UINT64)base + funcRva[ordinal - exports->Base];
}
```

### 6.5 Forwarded Exports

Forwarded exports point to another module:

```c
// If the function RVA falls within the export directory, it's forwarded
UINT32 exportDirStart = exportsRva;
UINT32 exportDirEnd = exportsRva + ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

UINT32 funcRvaValue = funcRva[ordinalRva[i]];
if (funcRvaValue >= exportDirStart && funcRvaValue < exportDirEnd) {
    // funcRvaValue points to "MODULE.FunctionName" string
    CHAR8 *forwardName = (CHAR8 *)(base + funcRvaValue);
    // Parse and resolve from forwarded module
}
```

---

## 7. Finding Kernel Modules

### 7.1 ZwQuerySystemInformation Method (Kernel Mode)

```c
// Reference: umap/mapper/util.c:52-86
PVOID GetModuleBaseAddress(PCHAR name) {
    PVOID addr = 0;

    ULONG size = 0;
    NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, 0, 0, &size);
    if (STATUS_INFO_LENGTH_MISMATCH != status) {
        return addr;
    }

    PSYSTEM_MODULE_INFORMATION modules = ExAllocatePool(NonPagedPool, size);
    if (!modules) {
        return addr;
    }

    if (!NT_SUCCESS(ZwQuerySystemInformation(SystemModuleInformation, modules, size, 0))) {
        ExFreePool(modules);
        return addr;
    }

    for (ULONG i = 0; i < modules->NumberOfModules; ++i) {
        SYSTEM_MODULE m = modules->Modules[i];

        if (strstr((PCHAR)m.FullPathName, name)) {
            addr = m.ImageBase;
            break;
        }
    }

    ExFreePool(modules);
    return addr;
}
```

### 7.2 PsLoadedModuleList Traversal (Boot/Hypervisor)

During boot, use the LoaderParameterBlock:

```c
// Reference: umap/boot/util.c:45-60
KLDR_DATA_TABLE_ENTRY *GetModuleEntry(LIST_ENTRY *list, CHAR16 *name) {
    for (LIST_ENTRY *entry = list->ForwardLink; entry != list;
        entry = entry->ForwardLink) {

        KLDR_DATA_TABLE_ENTRY *module =
            CONTAINING_RECORD(entry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

        if (module && StrnCmp(name, module->BaseImageName.Buffer,
            module->BaseImageName.Length) == 0) {

            return module;
        }
    }

    return NULL;
}
```

### 7.3 Hypervisor Module Discovery

From ring -1, we must read PsLoadedModuleList through guest physical memory:

```c
// Conceptual Ombra approach:
// 1. Find PsLoadedModuleList address (from KdDebuggerDataBlock or signature scan)
// 2. Read guest CR3 from VMCS_GUEST_CR3
// 3. Translate PsLoadedModuleList VA to PA using guest page tables
// 4. Read LIST_ENTRY and traverse module list
// 5. For each KLDR_DATA_TABLE_ENTRY, read DllBase for module addresses

// Reference: Sputnik PayLoad mm.cpp for guest memory access patterns
```

Reference: `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/Shark/Projects/Shark/Reload.c` (lines 734-773)

---

## 8. Entry Point Invocation

### 8.1 Standard Driver Entry Prototype

```c
typedef NTSTATUS (*PDRIVER_INITIALIZE)(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath
);
```

### 8.2 Direct Call Method

The simplest approach passes a fake or null driver object:

```c
// Reference: umap/mapper/main.c:157
return ((PDRIVER_INITIALIZE)(base + ntHeaders->OptionalHeader.AddressOfEntryPoint))
    ((PDRIVER_OBJECT)base, NULL);
```

### 8.3 Boot-Time Entry Hooking (umap approach)

umap hooks a legitimate driver's entry point to execute the mapper:

```c
// Reference: umap/boot/main.c:289-296
// Copy hook stub to target driver's entry point
MemCopy(targetModule->EntryPoint, "\x4C\x8D\x05\xF9\xFF\xFF\xFF", 7);  // lea r8, [rip - 7]

// Install trampoline to our mapper entry
TrampolineHook(mapperEntryPoint, (UINT8 *)targetModule->EntryPoint + 7, NULL);
```

The hook stub saves the original entry point address in R8, allowing the mapper to call it after completing mapping operations.

### 8.4 Entry Point from Hypervisor

Three viable approaches for calling driver entry from ring -1:

**A. Kernel Function Hook**
Modify a frequently-called kernel function to check for a trigger and call the driver entry.

**B. Kernel APC Injection**
Queue an APC in the System process that executes the driver entry.

**C. VMExit-Based RIP Modification**
On specific VMExit condition, modify guest RIP to point to driver entry with appropriate register setup.

---

## 9. Hypervisor-to-Kernel Mapping Specifics

### 9.1 Guest Physical Memory Access

The Sputnik payload demonstrates guest memory access from VMX root mode:

```c
// Reference: Sputnik PayLoad mm.cpp:3-25
auto mm::map_guest_phys(guest_phys_t phys_addr, map_type_t map_type) -> u64
{
    const auto host_phys = translate_guest_physical(phys_addr, map_type);
    if (!host_phys)
        return {};
    return map_page(host_phys, map_type);
}

auto mm::map_guest_virt(guest_phys_t dirbase, guest_virt_t virt_addr, map_type_t map_type) -> u64
{
    const auto guest_phys = translate_guest_virtual(dirbase, virt_addr, map_type);
    if (!guest_phys)
        return {};
    return map_guest_phys(guest_phys, map_type);
}
```

### 9.2 Guest Virtual Address Translation

```c
// Reference: Sputnik PayLoad mm.cpp:94-133
auto mm::translate_guest_virtual(guest_phys_t dirbase, guest_virt_t guest_virt, map_type_t map_type) -> u64
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

### 9.3 EPT Translation for Nested Virtualization

When running under existing hypervisor (Hyper-V), guest physical addresses must be translated through EPT:

```c
// Reference: Sputnik PayLoad mm.cpp:135-174
auto mm::translate_guest_physical(guest_phys_t phys_addr, map_type_t map_type) -> u64
{
    ept_pointer eptp;
    phys_addr_t guest_phys{ phys_addr };
    __vmx_vmread(VMCS_CTRL_EPT_POINTER, (size_t*)&eptp);

    const auto epml4 = reinterpret_cast<ept_pml4e*>(
        map_page(eptp.page_frame_number << 12, map_type));

    const auto epdpt_large = reinterpret_cast<ept_pdpte_1gb*>(map_page(
        epml4[guest_phys.pml4_index].page_frame_number << 12, map_type));

    // Handle 1GB EPT pages
    if (epdpt_large[guest_phys.pdpt_index].large_page)
        return (epdpt_large[guest_phys.pdpt_index].page_frame_number
            * 0x1000 * 0x200 * 0x200) + EPT_LARGE_PDPTE_OFFSET(phys_addr);

    // Continue through PDPT -> PD -> PT as needed
    // ...
}
```

### 9.4 Memory Copy Across Guest Boundaries

```c
// Reference: Sputnik PayLoad mm.cpp:324-360
auto mm::copy_guest_virt(guest_phys_t dirbase_src, guest_virt_t virt_src,
    guest_virt_t dirbase_dest, guest_virt_t virt_dest, u64 size) -> vmxroot_error_t
{
    while (size)
    {
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

## 10. Cleanup and Unload Support

### 10.1 Module Management Structure

```c
// Reference: Kernel-Bridge LoadableModules.h:3-6
namespace LoadableModules {
    using _OnLoad = NTSTATUS(NTAPI*)(PVOID hModule, LPCWSTR Name);
    using _OnUnload = NTSTATUS(NTAPI*)();
    using _OnDeviceControl = NTSTATUS(NTAPI*)(ULONG CtlCode, PVOID Argument);
}
```

### 10.2 Reference Counting for Safe Unload

```c
// Reference: Kernel-Bridge LoadableModules.cpp:147-187
NTSTATUS Unload(PVOID hModule) {
    // Mark module as unloading
    Module->Unloading = true;

    // Wait for all references to be released
    KeWaitForSingleObject(TargetModule->CompletionEvent, Executive, KernelMode, FALSE, NULL);

    // Call unload callback
    if (TargetModule->OnUnload) TargetModule->OnUnload();

    // Free memory
    VirtualMemory::FreePoolMemory(hModule);
    return STATUS_SUCCESS;
}
```

### 10.3 Stealth Unload Considerations

For mapped drivers that should remain invisible:
- Do NOT add to PsLoadedModuleList
- Do NOT register with PnP
- Implement custom unload via hypercall
- Zero memory before freeing to prevent forensic recovery

---

## 11. Integration with Hypercall Interface

### 11.1 VMExit Command Dispatch

```c
// Reference: Sputnik PayLoad vmexit_handler.cpp:1-132
void vmexit_handler(pcontext_t* context, void* unknown)
{
    pcontext_t guest_registers = *context;

    size_t vmexit_reason;
    __vmx_vmread(VMCS_EXIT_REASON, &vmexit_reason);

    if (vmexit_reason == VMX_EXIT_REASON_EXECUTE_CPUID)
    {
        if (guest_registers->rcx == VMEXIT_KEY)
        {
            switch ((vmexit_command_t)guest_registers->rdx)
            {
            case vmexit_command_t::init_page_tables:
                guest_registers->rax = (u64) mm::init();
                break;

            case vmexit_command_t::get_dirbase:
                // Get guest CR3
                __vmx_vmread(VMCS_GUEST_CR3, &guest_dirbase);
                command_data.dirbase = cr3{ guest_dirbase }.pml4_pfn << 12;
                break;

            case vmexit_command_t::copy_guest_virt:
                // Cross-process memory copy
                guest_registers->rax = (u64)mm::copy_guest_virt(...);
                break;
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
    reinterpret_cast<vmexit_handler_t>(original_handler)(context, unknown);
}
```

### 11.2 Proposed Ombra Hypercall Interface

```c
// Hypercall codes for driver mapping
enum class ombra_hypercall_t : u64 {
    map_driver          = 0x4F4D4252'00000001,  // "OMBR" + 1
    unmap_driver        = 0x4F4D4252'00000002,
    get_kernel_base     = 0x4F4D4252'00000003,
    translate_address   = 0x4F4D4252'00000004,
    read_kernel_memory  = 0x4F4D4252'00000005,
    write_kernel_memory = 0x4F4D4252'00000006,
};

// Map driver request structure
struct map_driver_request {
    u64 driver_image_va;      // Usermode buffer containing PE image
    u64 driver_image_size;    // Size of PE image
    u64 caller_dirbase;       // CR3 of calling process
    u64 out_mapped_base;      // Output: kernel VA of mapped driver
    u64 out_entry_point;      // Output: VA of driver entry
};
```

---

## 12. Complete Reference Table

| Source File | Key Content | Lines |
|-------------|-------------|-------|
| umap/boot/main.c | Boot-time hook chain, PE mapping | 273-431 |
| umap/mapper/main.c | Kernel-mode manual mapper | 61-158 |
| umap/boot/util.c | Export resolution, module lookup | 45-97 |
| umap/mapper/util.c | ZwQuerySystemInformation, pattern scan | 52-138 |
| Kernel-Bridge/LoadableModules.cpp | Module lifecycle management | 58-229 |
| Kernel-Bridge/PteUtils.cpp | Page table operations | 18-139 |
| Kernel-Bridge/PTE.h | Complete PTE structure definitions | 1-450 |
| Kernel-Bridge/MemoryUtils.cpp | Memory allocation utilities | 21-598 |
| Sputnik/PayLoad/mm.cpp | Guest memory access from hypervisor | 1-360 |
| Sputnik/PayLoad/mm.h | Page table type definitions | 18-165 |
| Sputnik/PayLoad/vmexit_handler.cpp | VMExit command dispatch | 1-132 |
| Shark/Projects/Shark/Reload.c | PsLoadedModuleList traversal | 734-815 |

---

## 13. Security Considerations

### 13.1 Detection Vectors (What to Avoid)

| Detection Method | How We Evade |
|------------------|--------------|
| PiDDBCacheTable | Not using MmLoadSystemImage |
| MmLoadedUserImageList | Not loading via standard APIs |
| Pool tag tracking | Direct PTE manipulation, no pool allocations |
| Process handle table | No usermode handle to kernel object |
| Syscall hooks | All operations from ring -1 |
| ETW events | Pre-boot injection before ETW starts |

### 13.2 Stealth Checklist

After mapping driver via hypervisor:

- [ ] No entry in PsLoadedModuleList (unless intentionally added)
- [ ] No MmUnloadedDrivers entry
- [ ] No pool allocations traceable to driver
- [ ] No registry keys created
- [ ] No kernel callbacks registered through standard APIs
- [ ] Memory pages not tagged with identifiable pool tags
- [ ] Import resolution done internally (no MmGetSystemRoutineAddress calls)

---

## 14. Implementation Recommendations for Ombra

### 14.1 Phase 1: Physical Memory Access
- Implement `mm::init()` for hypervisor page table setup
- Implement guest VA translation through EPT chain
- Test reading ntoskrnl headers from hypervisor

### 14.2 Phase 2: Kernel Discovery
- Find PsLoadedModuleList by signature scanning ntoskrnl
- Implement module enumeration through guest memory
- Cache ntoskrnl base and export table location

### 14.3 Phase 3: Memory Allocation
- Identify free physical pages (high memory or reserved range)
- Find unmapped kernel VA space in system PTE region
- Implement PTE creation in kernel page tables from ring -1

### 14.4 Phase 4: Driver Mapping
- Copy embedded driver to allocated physical pages
- Apply relocations based on kernel VA base
- Resolve imports from ntoskrnl export table

### 14.5 Phase 5: Entry Invocation
- Implement VMExit-triggered entry point call
- Create minimal DRIVER_OBJECT structure
- Handle driver return value

### 14.6 Phase 6: Hypercall Integration
- Define stable hypercall interface for OmbraMapper.exe
- Implement request validation in VMExit handler
- Return mapped base and entry point to usermode

---

*Document Version: 1.0*
*Generated for OmbraHypervisor Project*
*Reference Repositories: umap, Kernel-Bridge, Shark, Sputnik*
