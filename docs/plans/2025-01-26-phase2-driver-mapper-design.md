# Phase 2: Manual PE Mapper for OmbraDriver

## Overview

Map OmbraDriver.sys into kernel memory without using SCM (Service Control Manager), hide it via EPT, and establish communication channels. This phase executes after the hypervisor is active (Phase 1 complete).

**Goal:** Load a custom kernel driver that bridges Ring 3 ↔ Hypervisor communication, provides process tracking, and manages EPT-based memory hiding.

**Critical Constraint:** Must avoid detection by anti-cheat systems that scan BigPool, PE signatures, pool tags, and event logs.

---

## Detection Vectors and Mitigations

### Vector 1: BigPool / PoolBigPageTable

**Threat:** Kernel allocations >4KB are tracked in `PoolBigPageTable`. Anti-cheats scan this table for suspicious allocations with identifiable pool tags.

**Mitigation:** Requires empirical testing (see Test Harness section). Design supports two paths based on results.

### Vector 2: PE Header Fingerprinting

**Threat:** Anti-cheats scan memory for MZ/PE signatures in unexpected regions.

**Mitigation:** Zero entire PE header region after import resolution.

### Vector 3: Compiler Signatures

**Threat:** Default MSVC includes signatured CRT routines (`memset`, `memcpy`, `_GSHandlerCheckCommon`).

**Mitigation:** Hardened build configuration with custom implementations.

### Vector 4: Event Log / DeleteService

**Threat:** `DeleteService` creates event log entries that EAC monitors.

**Mitigation:** Alternative cleanup strategy - leave service entry or corrupt registry.

### Vector 5: Pool Tag Fingerprinting

**Threat:** 'enoN' (default) and common tags are red flags.

**Mitigation:** OmbraDriver uses inconspicuous tags; avoid default allocations.

### Vector 6: Paged Pool

**Threat:** Executable code in paged pool is detectable.

**Mitigation:** All allocations use nonpaged pool.

---

## Architecture Decision: Two-Path Design

### Decision Point

**Question:** Does Ld9BoxSup's `PageAllocEx` appear in `PoolBigPageTable`?

This must be verified empirically before implementation. The test harness (Task 1) determines which path to implement.

### Path A: Clean Path (BigPool NOT Visible)

If `PageAllocEx` allocations do NOT appear in `PoolBigPageTable` (likely if SUPDrv uses `MmAllocatePagesForMdl` internally):

- Use existing `PageAllocEx` for driver pool
- Allocate pool during Phase 1 while SUPDrv is active
- Direct R3→R0 writes via dual mapping
- Simplified VMCALL interface

### Path B: Mitigation Path (BigPool IS Visible)

If allocations DO appear in `PoolBigPageTable`:

- **Option B1:** MDL-based allocation via deferred VMCALL
- **Option B2:** EPT-only memory (hypervisor-private mappings)

Path B adds complexity but provides genuine invisibility.

---

## Execution Flow

### Pre-Phase 2 (While SUPDrv Still Active)

```
1. Parse OmbraDriver.sys PE
   - Validate DOS/NT headers
   - Extract section table
   - Calculate required memory size (aligned)
   - Build import table list

2. Resolve all imports via DrvGetSymbol()
   - For each import in IAT
   - Store resolved addresses locally
   - Fail if any import unresolved

3. Allocate driver pool via PageAllocEx
   - Size: MAX(imageSize, 2MB) for growth
   - Returns R3 and R0 addresses
   - Both paths need this (Path B may reallocate later)

4. Store allocation info for Phase 2
   - poolR3, poolR0, poolSize
   - Resolved imports array
   - PE metadata

5. DO NOT unload SUPDrv yet
   - Memory lifetime depends on session
   - Unload after driver is verified running
```

### Phase 2 (Via Hypervisor VMCALLs)

```
Path A (Clean):
  1. VMCALL_CLAIM_POOL_REGION(offset, size)
     → Hypervisor tracks allocation, returns kernelVA

  2. Loader writes directly to poolR3:
     - Copy PE sections to correct RVAs
     - Apply relocations using kernelVA as base
     - Patch IAT with pre-resolved imports
     - Zero PE headers

  3. VMCALL_FINALIZE_DRIVER_LOAD(kernelVA, size, entryRva)
     - Set .text to RX, .data to RW
     - Call DriverEntry(NULL, NULL)
     - Configure EPT hiding

Path B (Mitigation):
  1. VMCALL_ALLOC_MDL_MEMORY(size)
     → Queues deferred allocation

  2. VMCALL_POLL_ALLOC_STATUS()
     → Returns PENDING or COMPLETE with kernelVA

  3-5. Same as Path A steps 2-3
```

### Post-Phase 2 (Cleanup)

```
1. Verify OmbraDriver is running
   - VMCALL_DRIVER_PING or shared memory handshake

2. Close SUPDrv handle
   - CloseHandle(hDevice)
   - Service may auto-unload

3. DO NOT call DeleteService
   - Leaves event log traces
   - Leave service entry intact (LDPlayer cover)
   - Or corrupt registry entry to look like data corruption

4. Clear loader memory
   - Zero PE buffer
   - Zero import arrays
   - Zero any secrets
```

---

## Data Structures

### DRIVER_MAP_CONTEXT (Loader Internal)

```c
typedef struct _DRIVER_MAP_CONTEXT {
    // PE parsing results
    void*       ImageBuffer;        // Original PE in usermode
    U32         ImageSize;          // Total image size
    U32         SizeOfHeaders;      // Header size to zero
    U32         EntryPointRva;      // AddressOfEntryPoint
    U32         ImageBase;          // Preferred base (for reloc calc)

    // Section info
    U32         SectionCount;
    PE_SECTION  Sections[16];       // Section metadata

    // Import resolution
    U32         ImportCount;
    RESOLVED_IMPORT Imports[256];   // Pre-resolved imports

    // Pool allocation (from SUPDrv)
    void*       PoolR3;             // Usermode mapping
    void*       PoolR0;             // Kernel mapping
    U64         PoolPhysical;       // Physical address
    U32         PoolSize;           // Allocated size

    // Mapped driver info
    void*       MappedBase;         // Final kernel address
    U32         MappedSize;         // Final size
    BOOL        Mapped;             // Successfully mapped
    BOOL        Running;            // DriverEntry returned success
} DRIVER_MAP_CONTEXT;

typedef struct _PE_SECTION {
    char        Name[8];
    U32         VirtualAddress;     // RVA
    U32         VirtualSize;
    U32         RawDataOffset;      // File offset
    U32         RawDataSize;
    U32         Characteristics;    // Section flags
} PE_SECTION;

typedef struct _RESOLVED_IMPORT {
    char        ModuleName[64];     // e.g., "ntoskrnl.exe"
    char        FunctionName[128];  // e.g., "ExAllocatePoolWithTag"
    U64         Address;            // Resolved kernel address
    U32         IatRva;             // RVA of IAT entry to patch
} RESOLVED_IMPORT;
```

### VMCALL Structures

```c
// VMCALL_CLAIM_POOL_REGION
typedef struct _VMCALL_CLAIM_POOL_IN {
    U32         Offset;             // Offset into pool
    U32         Size;               // Size to claim
} VMCALL_CLAIM_POOL_IN;

typedef struct _VMCALL_CLAIM_POOL_OUT {
    U64         KernelVA;           // Resulting kernel address
    U32         Status;             // 0 = success
} VMCALL_CLAIM_POOL_OUT;

// VMCALL_FINALIZE_DRIVER_LOAD
typedef struct _VMCALL_FINALIZE_IN {
    U64         KernelVA;           // Base address
    U32         Size;               // Total size
    U32         EntryRva;           // Entry point RVA
    U32         TextRva;            // .text section RVA
    U32         TextSize;           // .text section size
    U32         Flags;              // Options
} VMCALL_FINALIZE_IN;

typedef struct _VMCALL_FINALIZE_OUT {
    U32         Status;             // 0 = success
    U32         DriverEntryResult;  // Return from DriverEntry
} VMCALL_FINALIZE_OUT;

// VMCALL_ALLOC_MDL_MEMORY (Path B only)
typedef struct _VMCALL_ALLOC_MDL_IN {
    U32         Size;               // Bytes to allocate
    U32         Flags;              // Allocation flags
} VMCALL_ALLOC_MDL_IN;

typedef struct _VMCALL_ALLOC_MDL_OUT {
    U32         RequestId;          // ID for polling
    U32         Status;             // PENDING or immediate result
} VMCALL_ALLOC_MDL_OUT;

// VMCALL_POLL_ALLOC_STATUS (Path B only)
typedef struct _VMCALL_POLL_IN {
    U32         RequestId;
} VMCALL_POLL_IN;

typedef struct _VMCALL_POLL_OUT {
    U32         Status;             // PENDING, COMPLETE, FAILED
    U64         KernelVA;           // Valid if COMPLETE
    U64         PhysicalAddr;       // Valid if COMPLETE
} VMCALL_POLL_OUT;
```

### VMCALL IDs

```c
// Phase 2 VMCALLs (add to existing VMCALL enum)
typedef enum _VMCALL_PHASE2 {
    // Pool management
    VMCALL_CLAIM_POOL_REGION    = 0x3000,
    VMCALL_RELEASE_POOL_REGION  = 0x3001,

    // Driver loading
    VMCALL_FINALIZE_DRIVER_LOAD = 0x3010,
    VMCALL_DRIVER_PING          = 0x3011,
    VMCALL_CALL_DRIVER_UNLOAD   = 0x3012,

    // EPT hiding
    VMCALL_HIDE_MEMORY_RANGE    = 0x3020,
    VMCALL_UNHIDE_MEMORY_RANGE  = 0x3021,

    // Path B: Deferred allocation
    VMCALL_ALLOC_MDL_MEMORY     = 0x3030,
    VMCALL_POLL_ALLOC_STATUS    = 0x3031,
    VMCALL_FREE_MDL_MEMORY      = 0x3032,

    // Path B Option 2: EPT-only memory
    VMCALL_CREATE_EPT_MAPPING   = 0x3040,
    VMCALL_DESTROY_EPT_MAPPING  = 0x3041,
} VMCALL_PHASE2;
```

---

## Implementation Tasks

### Task 1: BigPool Test Harness

Create a test to determine which path to implement.

**File:** `hypervisor/tests/bigpool_test.c`

```c
// Test procedure - runs after hypervisor is loaded
// Can be triggered via special VMCALL or debug interface

#include "../shared/types.h"

// Pool tracker structure (from Windows internals)
typedef struct _POOL_TRACKER_BIG_PAGES {
    void*       Va;
    U32         Key;            // Pool tag
    U32         PoolType;
    U64         NumberOfBytes;
} POOL_TRACKER_BIG_PAGES;

// Symbols to resolve
static POOL_TRACKER_BIG_PAGES** g_PoolBigPageTable = NULL;
static U64* g_PoolBigPageTableSize = NULL;

BOOL BigPoolTest_Init(U64 fnGetSymbol) {
    // Resolve symbols
    // Note: These are internal symbols, may need alternative resolution
    g_PoolBigPageTable = (POOL_TRACKER_BIG_PAGES**)
        ResolveInternalSymbol("PoolBigPageTable");
    g_PoolBigPageTableSize = (U64*)
        ResolveInternalSymbol("PoolBigPageTableSize");

    if (!g_PoolBigPageTable || !g_PoolBigPageTableSize) {
        return FALSE;
    }
    return TRUE;
}

typedef struct _BIGPOOL_TEST_RESULT {
    BOOL        AllocationVisible;
    U32         PoolTag;
    U32         PoolType;
    U64         EntryCount;
} BIGPOOL_TEST_RESULT;

BOOL BigPoolTest_CheckAllocation(void* kernelVA, BIGPOOL_TEST_RESULT* result) {
    if (!g_PoolBigPageTable || !g_PoolBigPageTableSize) {
        return FALSE;
    }

    POOL_TRACKER_BIG_PAGES* table = *g_PoolBigPageTable;
    U64 count = *g_PoolBigPageTableSize;

    result->AllocationVisible = FALSE;
    result->EntryCount = count;

    for (U64 i = 0; i < count; i++) {
        if (table[i].Va == kernelVA) {
            result->AllocationVisible = TRUE;
            result->PoolTag = table[i].Key;
            result->PoolType = table[i].PoolType;
            return TRUE;
        }
    }

    return TRUE;  // Test completed, allocation not found
}

// Tag to readable string
void TagToString(U32 tag, char* buf) {
    buf[0] = (char)(tag & 0xFF);
    buf[1] = (char)((tag >> 8) & 0xFF);
    buf[2] = (char)((tag >> 16) & 0xFF);
    buf[3] = (char)((tag >> 24) & 0xFF);
    buf[4] = '\0';
}
```

**Test Execution (from loader):**

```c
// After Phase 1 complete, before proceeding with Phase 2

BOOL TestBigPoolVisibility(HV_LOADER_CTX* hvCtx, DRIVER_MAP_CONTEXT* drvCtx) {
    printf("[*] Testing BigPool visibility...\n");

    // Allocate test memory via SUPDrv
    ALLOC_INFO testAlloc = {0};
    if (DrvAllocContiguous(&hvCtx->Driver, 512, &testAlloc) != DRV_SUCCESS) {
        printf("[-] Test allocation failed\n");
        return FALSE;
    }
    printf("[+] Test allocation: R0=%p\n", testAlloc.R0);

    // Issue VMCALL to check BigPool
    BIGPOOL_TEST_RESULT result = {0};
    VMCALL_STATUS status = VmcallBigPoolTest(testAlloc.R0, &result);

    if (status != VMCALL_SUCCESS) {
        printf("[-] BigPool test VMCALL failed\n");
        DrvFreeContiguous(&hvCtx->Driver, &testAlloc);
        return FALSE;
    }

    // Report results
    printf("[*] BigPool test results:\n");
    printf("    Entry count: %llu\n", result.EntryCount);

    if (result.AllocationVisible) {
        char tagStr[5];
        TagToString(result.PoolTag, tagStr);
        printf("    VISIBLE: Yes\n");
        printf("    Pool tag: '%s' (0x%08X)\n", tagStr, result.PoolTag);
        printf("    Pool type: %u\n", result.PoolType);
        printf("\n[!] WARNING: Allocations visible in BigPool!\n");
        printf("[!] Path B (mitigation) required.\n");
        drvCtx->UseMitigationPath = TRUE;
    } else {
        printf("    VISIBLE: No\n");
        printf("\n[+] Allocations NOT visible in BigPool.\n");
        printf("[+] Path A (clean) can be used.\n");
        drvCtx->UseMitigationPath = FALSE;
    }

    // Cleanup test allocation
    DrvFreeContiguous(&hvCtx->Driver, &testAlloc);

    return TRUE;
}
```

---

### Task 2: PE Parser (loader/pe_parser.c)

Parse OmbraDriver PE for mapping.

**Header (pe_parser.h):**

```c
#ifndef PE_PARSER_H
#define PE_PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include <Windows.h>

#define MAX_SECTIONS 16
#define MAX_IMPORTS 256

typedef struct _PE_SECTION_INFO {
    char        Name[8];
    uint32_t    Rva;
    uint32_t    VirtualSize;
    uint32_t    FileOffset;
    uint32_t    FileSize;
    uint32_t    Characteristics;
} PE_SECTION_INFO;

typedef struct _PE_IMPORT_INFO {
    char        ModuleName[64];
    char        FunctionName[128];
    uint32_t    IatRva;             // RVA of IAT entry
    uint64_t    ResolvedAddress;    // Filled by resolver
} PE_IMPORT_INFO;

typedef struct _PE_INFO {
    // Validation
    bool        Valid;

    // Basic info
    uint64_t    ImageBase;          // Preferred base
    uint32_t    ImageSize;          // SizeOfImage
    uint32_t    HeadersSize;        // SizeOfHeaders
    uint32_t    EntryPointRva;      // AddressOfEntryPoint
    uint32_t    FileAlignment;
    uint32_t    SectionAlignment;

    // Sections
    uint32_t    SectionCount;
    PE_SECTION_INFO Sections[MAX_SECTIONS];

    // Imports
    uint32_t    ImportCount;
    PE_IMPORT_INFO Imports[MAX_IMPORTS];

    // Relocations
    uint32_t    RelocRva;
    uint32_t    RelocSize;
} PE_INFO;

// Parse PE image
bool PeParse(const void* peImage, uint32_t imageSize, PE_INFO* info);

// Get section by name
PE_SECTION_INFO* PeGetSection(PE_INFO* info, const char* name);

// Calculate required allocation size (aligned)
uint32_t PeCalculateImageSize(PE_INFO* info);

#endif // PE_PARSER_H
```

**Implementation (pe_parser.c):**

```c
#include "pe_parser.h"
#include <string.h>
#include <stdio.h>

bool PeParse(const void* peImage, uint32_t imageSize, PE_INFO* info) {
    memset(info, 0, sizeof(*info));

    // Validate DOS header
    const IMAGE_DOS_HEADER* dos = (const IMAGE_DOS_HEADER*)peImage;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
        printf("[-] Invalid DOS signature\n");
        return false;
    }

    if ((uint32_t)dos->e_lfanew + sizeof(IMAGE_NT_HEADERS64) > imageSize) {
        printf("[-] NT headers beyond image size\n");
        return false;
    }

    // Validate NT headers
    const IMAGE_NT_HEADERS64* nt =
        (const IMAGE_NT_HEADERS64*)((uint8_t*)peImage + dos->e_lfanew);

    if (nt->Signature != IMAGE_NT_SIGNATURE) {
        printf("[-] Invalid NT signature\n");
        return false;
    }

    if (nt->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) {
        printf("[-] Not x64 PE\n");
        return false;
    }

    // Extract basic info
    info->ImageBase = nt->OptionalHeader.ImageBase;
    info->ImageSize = nt->OptionalHeader.SizeOfImage;
    info->HeadersSize = nt->OptionalHeader.SizeOfHeaders;
    info->EntryPointRva = nt->OptionalHeader.AddressOfEntryPoint;
    info->FileAlignment = nt->OptionalHeader.FileAlignment;
    info->SectionAlignment = nt->OptionalHeader.SectionAlignment;

    // Parse sections
    const IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(nt);
    info->SectionCount = nt->FileHeader.NumberOfSections;

    if (info->SectionCount > MAX_SECTIONS) {
        printf("[-] Too many sections: %u\n", info->SectionCount);
        return false;
    }

    for (uint32_t i = 0; i < info->SectionCount; i++) {
        memcpy(info->Sections[i].Name, sections[i].Name, 8);
        info->Sections[i].Rva = sections[i].VirtualAddress;
        info->Sections[i].VirtualSize = sections[i].Misc.VirtualSize;
        info->Sections[i].FileOffset = sections[i].PointerToRawData;
        info->Sections[i].FileSize = sections[i].SizeOfRawData;
        info->Sections[i].Characteristics = sections[i].Characteristics;
    }

    // Parse imports
    IMAGE_DATA_DIRECTORY* importDir =
        &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

    if (importDir->VirtualAddress && importDir->Size) {
        if (!ParseImports(peImage, imageSize, importDir, info)) {
            printf("[-] Failed to parse imports\n");
            return false;
        }
    }

    // Get relocation info
    IMAGE_DATA_DIRECTORY* relocDir =
        &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

    info->RelocRva = relocDir->VirtualAddress;
    info->RelocSize = relocDir->Size;

    info->Valid = true;
    return true;
}

static bool ParseImports(const void* peImage, uint32_t imageSize,
                         IMAGE_DATA_DIRECTORY* importDir, PE_INFO* info) {
    // Find section containing imports
    uint32_t importRva = importDir->VirtualAddress;
    uint32_t importFileOffset = RvaToFileOffset(info, importRva);

    if (importFileOffset == 0 || importFileOffset >= imageSize) {
        return false;
    }

    const IMAGE_IMPORT_DESCRIPTOR* imp =
        (const IMAGE_IMPORT_DESCRIPTOR*)((uint8_t*)peImage + importFileOffset);

    info->ImportCount = 0;

    while (imp->Name && info->ImportCount < MAX_IMPORTS) {
        // Get module name
        uint32_t nameOffset = RvaToFileOffset(info, imp->Name);
        if (nameOffset == 0 || nameOffset >= imageSize) break;

        const char* moduleName = (const char*)((uint8_t*)peImage + nameOffset);

        // Get thunk data (IAT)
        uint32_t thunkRva = imp->OriginalFirstThunk ?
                           imp->OriginalFirstThunk : imp->FirstThunk;
        uint32_t thunkOffset = RvaToFileOffset(info, thunkRva);
        uint32_t iatRva = imp->FirstThunk;

        if (thunkOffset == 0 || thunkOffset >= imageSize) {
            imp++;
            continue;
        }

        const IMAGE_THUNK_DATA64* thunk =
            (const IMAGE_THUNK_DATA64*)((uint8_t*)peImage + thunkOffset);

        uint32_t iatIndex = 0;
        while (thunk->u1.AddressOfData && info->ImportCount < MAX_IMPORTS) {
            PE_IMPORT_INFO* import = &info->Imports[info->ImportCount];

            strncpy_s(import->ModuleName, sizeof(import->ModuleName),
                     moduleName, _TRUNCATE);

            if (!(thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG64)) {
                // Import by name
                uint32_t hintOffset = RvaToFileOffset(info,
                    (uint32_t)thunk->u1.AddressOfData);

                if (hintOffset && hintOffset < imageSize) {
                    const IMAGE_IMPORT_BY_NAME* hint =
                        (const IMAGE_IMPORT_BY_NAME*)((uint8_t*)peImage + hintOffset);
                    strncpy_s(import->FunctionName, sizeof(import->FunctionName),
                             hint->Name, _TRUNCATE);
                }
            } else {
                // Import by ordinal
                snprintf(import->FunctionName, sizeof(import->FunctionName),
                        "#%llu", thunk->u1.Ordinal & 0xFFFF);
            }

            import->IatRva = iatRva + (iatIndex * sizeof(uint64_t));
            import->ResolvedAddress = 0;

            info->ImportCount++;
            thunk++;
            iatIndex++;
        }

        imp++;
    }

    return true;
}

static uint32_t RvaToFileOffset(PE_INFO* info, uint32_t rva) {
    for (uint32_t i = 0; i < info->SectionCount; i++) {
        PE_SECTION_INFO* sec = &info->Sections[i];
        if (rva >= sec->Rva && rva < sec->Rva + sec->VirtualSize) {
            return sec->FileOffset + (rva - sec->Rva);
        }
    }
    return 0;
}

PE_SECTION_INFO* PeGetSection(PE_INFO* info, const char* name) {
    for (uint32_t i = 0; i < info->SectionCount; i++) {
        if (strncmp(info->Sections[i].Name, name, 8) == 0) {
            return &info->Sections[i];
        }
    }
    return NULL;
}

uint32_t PeCalculateImageSize(PE_INFO* info) {
    // Return SizeOfImage aligned to section alignment
    uint32_t align = info->SectionAlignment;
    return (info->ImageSize + align - 1) & ~(align - 1);
}
```

---

### Task 3: Import Resolver (loader/pe_imports.c)

Resolve imports via SUPDrv before unload.

```c
#include "pe_imports.h"
#include "pe_parser.h"
#include "../usermode/driver_interface.h"
#include <stdio.h>
#include <string.h>

// Common ntoskrnl imports for OmbraDriver
// Resolved proactively to ensure we don't miss any
static const char* COMMON_IMPORTS[] = {
    "ExAllocatePoolWithTag",
    "ExFreePoolWithTag",
    "MmGetSystemRoutineAddress",
    "IoCreateDevice",
    "IoDeleteDevice",
    "IoCreateSymbolicLink",
    "IoDeleteSymbolicLink",
    "RtlInitUnicodeString",
    "DbgPrint",
    "KeQueryActiveProcessorCountEx",
    "KeGetCurrentProcessorNumberEx",
    "KeSetSystemAffinityThread",
    "KeRevertToUserAffinityThread",
    "MmMapIoSpace",
    "MmUnmapIoSpace",
    "MmAllocateContiguousMemory",
    "MmFreeContiguousMemory",
    "ZwQuerySystemInformation",
    "PsGetCurrentProcessId",
    "PsGetCurrentThreadId",
    "PsLookupProcessByProcessId",
    "ObDereferenceObject",
    NULL
};

bool ResolveImports(DRV_CONTEXT* drv, PE_INFO* peInfo) {
    printf("[*] Resolving %u imports...\n", peInfo->ImportCount);

    uint32_t resolved = 0;
    uint32_t failed = 0;

    for (uint32_t i = 0; i < peInfo->ImportCount; i++) {
        PE_IMPORT_INFO* imp = &peInfo->Imports[i];

        // Only resolve ntoskrnl imports
        // Other modules (HAL, etc.) need different handling
        if (_stricmp(imp->ModuleName, "ntoskrnl.exe") != 0 &&
            _stricmp(imp->ModuleName, "ntkrnlpa.exe") != 0 &&
            _stricmp(imp->ModuleName, "ntkrnlmp.exe") != 0) {
            printf("[!] Non-ntoskrnl import: %s!%s\n",
                   imp->ModuleName, imp->FunctionName);
            // Try to resolve anyway via LDR_GET_SYMBOL
        }

        void* addr = NULL;
        DRV_STATUS status = DrvGetSymbol(drv, imp->FunctionName, &addr);

        if (status == DRV_SUCCESS && addr != NULL) {
            imp->ResolvedAddress = (uint64_t)addr;
            resolved++;
        } else {
            printf("[-] Failed to resolve: %s\n", imp->FunctionName);
            failed++;
        }
    }

    printf("[*] Resolved: %u, Failed: %u\n", resolved, failed);

    if (failed > 0) {
        printf("[-] Some imports could not be resolved\n");
        return false;
    }

    return true;
}

// Pre-resolve common symbols that OmbraDriver might dynamically need
bool ResolveCommonSymbols(DRV_CONTEXT* drv, COMMON_SYMBOLS* syms) {
    memset(syms, 0, sizeof(*syms));

    printf("[*] Pre-resolving common symbols...\n");

    for (int i = 0; COMMON_IMPORTS[i] != NULL; i++) {
        void* addr = NULL;
        DRV_STATUS status = DrvGetSymbol(drv, COMMON_IMPORTS[i], &addr);

        if (status == DRV_SUCCESS && addr != NULL) {
            // Store in appropriate field based on name
            StoreSymbol(syms, COMMON_IMPORTS[i], (uint64_t)addr);
        }
    }

    return true;
}
```

---

### Task 4: Relocation Processor (loader/pe_relocs.c)

Apply relocations after knowing kernel base.

```c
#include "pe_relocs.h"
#include "pe_parser.h"
#include <stdio.h>

#define IMAGE_REL_BASED_DIR64 10

bool ApplyRelocations(void* mappedImage, PE_INFO* peInfo, uint64_t newBase) {
    if (peInfo->RelocRva == 0 || peInfo->RelocSize == 0) {
        printf("[*] No relocations to apply\n");
        return true;
    }

    int64_t delta = (int64_t)(newBase - peInfo->ImageBase);

    if (delta == 0) {
        printf("[*] Image loaded at preferred base, no relocations needed\n");
        return true;
    }

    printf("[*] Applying relocations (delta = 0x%llX)\n", delta);

    uint8_t* relocBase = (uint8_t*)mappedImage + peInfo->RelocRva;
    uint8_t* relocEnd = relocBase + peInfo->RelocSize;

    uint32_t blockCount = 0;
    uint32_t relocCount = 0;

    while (relocBase < relocEnd) {
        IMAGE_BASE_RELOCATION* block = (IMAGE_BASE_RELOCATION*)relocBase;

        if (block->SizeOfBlock == 0) {
            break;
        }

        uint32_t entryCount = (block->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / 2;
        uint16_t* entries = (uint16_t*)(block + 1);

        for (uint32_t i = 0; i < entryCount; i++) {
            uint16_t entry = entries[i];
            uint16_t type = entry >> 12;
            uint16_t offset = entry & 0xFFF;

            if (type == IMAGE_REL_BASED_ABSOLUTE) {
                // Padding, skip
                continue;
            }

            uint8_t* targetAddr = (uint8_t*)mappedImage + block->VirtualAddress + offset;

            if (type == IMAGE_REL_BASED_DIR64) {
                // 64-bit relocation
                int64_t* target = (int64_t*)targetAddr;
                *target += delta;
                relocCount++;
            } else if (type == IMAGE_REL_BASED_HIGHLOW) {
                // 32-bit relocation (rare in x64)
                int32_t* target = (int32_t*)targetAddr;
                *target += (int32_t)delta;
                relocCount++;
            } else {
                printf("[!] Unknown relocation type: %u\n", type);
            }
        }

        relocBase += block->SizeOfBlock;
        blockCount++;
    }

    printf("[+] Applied %u relocations in %u blocks\n", relocCount, blockCount);
    return true;
}
```

---

### Task 5: IAT Patcher (loader/pe_iat.c)

Patch IAT with resolved addresses.

```c
#include "pe_iat.h"
#include "pe_parser.h"
#include <stdio.h>

bool PatchIAT(void* mappedImage, PE_INFO* peInfo) {
    printf("[*] Patching IAT with %u entries...\n", peInfo->ImportCount);

    uint32_t patched = 0;

    for (uint32_t i = 0; i < peInfo->ImportCount; i++) {
        PE_IMPORT_INFO* imp = &peInfo->Imports[i];

        if (imp->ResolvedAddress == 0) {
            printf("[-] Unresolved import: %s!%s\n",
                   imp->ModuleName, imp->FunctionName);
            return false;
        }

        // Patch IAT entry
        uint64_t* iatEntry = (uint64_t*)((uint8_t*)mappedImage + imp->IatRva);
        *iatEntry = imp->ResolvedAddress;
        patched++;
    }

    printf("[+] Patched %u IAT entries\n", patched);
    return true;
}
```

---

### Task 6: PE Header Destruction (loader/pe_wipe.c)

Zero PE headers to avoid signature detection.

```c
#include "pe_wipe.h"
#include <stdio.h>
#include <string.h>

void WipePeHeaders(void* mappedImage, uint32_t headerSize) {
    printf("[*] Wiping PE headers (%u bytes)...\n", headerSize);

    // Use volatile to prevent optimization
    volatile uint8_t* p = (volatile uint8_t*)mappedImage;

    // Zero entire header region
    for (uint32_t i = 0; i < headerSize; i++) {
        p[i] = 0;
    }

    printf("[+] PE headers wiped\n");
}

// More aggressive: wipe headers and fill with random data
void WipePeHeadersRandom(void* mappedImage, uint32_t headerSize) {
    printf("[*] Wiping PE headers with random data...\n");

    volatile uint8_t* p = (volatile uint8_t*)mappedImage;
    uint32_t seed = (uint32_t)__rdtsc();

    for (uint32_t i = 0; i < headerSize; i++) {
        seed = seed * 1103515245 + 12345;
        p[i] = (uint8_t)(seed >> 16);
    }

    printf("[+] PE headers wiped with random data\n");
}
```

---

### Task 7: Section Mapper (loader/pe_mapper.c)

Copy sections to kernel memory.

```c
#include "pe_mapper.h"
#include "pe_parser.h"
#include "pe_relocs.h"
#include "pe_iat.h"
#include "pe_wipe.h"
#include <stdio.h>
#include <string.h>

bool MapDriver(
    const void* peImage,
    void* targetR3,           // R3 mapping of kernel memory
    uint64_t targetR0,        // R0 address (for relocations)
    PE_INFO* peInfo
) {
    printf("[*] Mapping driver to R3=%p (R0=0x%llX)\n", targetR3, targetR0);

    // Zero target region first
    memset(targetR3, 0, peInfo->ImageSize);

    // Copy headers (will be wiped later)
    memcpy(targetR3, peImage, peInfo->HeadersSize);
    printf("[+] Copied headers (%u bytes)\n", peInfo->HeadersSize);

    // Copy each section
    for (uint32_t i = 0; i < peInfo->SectionCount; i++) {
        PE_SECTION_INFO* sec = &peInfo->Sections[i];

        if (sec->FileSize == 0) {
            printf("[*] Section %.8s: virtual only (%u bytes)\n",
                   sec->Name, sec->VirtualSize);
            continue;
        }

        void* dst = (uint8_t*)targetR3 + sec->Rva;
        const void* src = (const uint8_t*)peImage + sec->FileOffset;
        uint32_t copySize = min(sec->FileSize, sec->VirtualSize);

        memcpy(dst, src, copySize);
        printf("[+] Section %.8s: copied %u bytes to RVA 0x%X\n",
               sec->Name, copySize, sec->Rva);
    }

    // Apply relocations
    if (!ApplyRelocations(targetR3, peInfo, targetR0)) {
        printf("[-] Relocation failed\n");
        return false;
    }

    // Patch IAT
    if (!PatchIAT(targetR3, peInfo)) {
        printf("[-] IAT patching failed\n");
        return false;
    }

    // Wipe PE headers
    WipePeHeaders(targetR3, peInfo->HeadersSize);

    printf("[+] Driver mapped successfully\n");
    return true;
}
```

---

### Task 8: VMCALL Handlers (hypervisor/handlers/vmcall_phase2.c)

Hypervisor-side VMCALL handlers for Phase 2.

```c
#include "vmcall.h"
#include "../ept.h"
#include "../../shared/types.h"

// Pool tracking
typedef struct _POOL_REGION {
    U64     KernelVA;
    U64     PhysicalAddr;
    U32     Size;
    BOOL    InUse;
} POOL_REGION;

#define MAX_POOL_REGIONS 16
static POOL_REGION g_PoolRegions[MAX_POOL_REGIONS] = {0};
static U64 g_PoolBase = 0;
static U64 g_PoolSize = 0;
static U32 g_PoolOffset = 0;

void Phase2_InitPool(U64 poolBase, U64 poolSize) {
    g_PoolBase = poolBase;
    g_PoolSize = poolSize;
    g_PoolOffset = 0;
}

// VMCALL_CLAIM_POOL_REGION handler
OMBRA_STATUS HandleClaimPoolRegion(VMCALL_CLAIM_POOL_IN* in, VMCALL_CLAIM_POOL_OUT* out) {
    U32 alignedSize = (in->Size + 0xFFF) & ~0xFFF;  // Page align

    if (g_PoolOffset + alignedSize > g_PoolSize) {
        out->Status = OMBRA_ERROR_OUT_OF_MEMORY;
        return OMBRA_ERROR_OUT_OF_MEMORY;
    }

    // Find free region slot
    POOL_REGION* region = NULL;
    for (int i = 0; i < MAX_POOL_REGIONS; i++) {
        if (!g_PoolRegions[i].InUse) {
            region = &g_PoolRegions[i];
            break;
        }
    }

    if (!region) {
        out->Status = OMBRA_ERROR_OUT_OF_MEMORY;
        return OMBRA_ERROR_OUT_OF_MEMORY;
    }

    region->KernelVA = g_PoolBase + g_PoolOffset;
    region->Size = alignedSize;
    region->InUse = TRUE;

    out->KernelVA = region->KernelVA;
    out->Status = OMBRA_SUCCESS;

    g_PoolOffset += alignedSize;

    return OMBRA_SUCCESS;
}

// VMCALL_FINALIZE_DRIVER_LOAD handler
OMBRA_STATUS HandleFinalizeDriverLoad(VMCALL_FINALIZE_IN* in, VMCALL_FINALIZE_OUT* out) {
    // Set memory protections via EPT
    // .text section: Execute + Read (no write)
    U64 textStart = in->KernelVA + in->TextRva;
    U64 textPages = (in->TextSize + 0xFFF) / 0x1000;

    for (U64 i = 0; i < textPages; i++) {
        U64 pageAddr = textStart + (i * 0x1000);
        EptSetPagePermissions(pageAddr, EPT_READ | EPT_EXECUTE);
    }

    // Call DriverEntry
    typedef NTSTATUS (*FN_DriverEntry)(void*, void*);
    FN_DriverEntry entry = (FN_DriverEntry)(in->KernelVA + in->EntryRva);

    NTSTATUS ntStatus = entry(NULL, NULL);

    out->DriverEntryResult = (U32)ntStatus;

    if (ntStatus < 0) {
        out->Status = OMBRA_ERROR_DRIVER_INIT_FAILED;
        return OMBRA_ERROR_DRIVER_INIT_FAILED;
    }

    // Configure EPT hiding if requested
    if (in->Flags & FINALIZE_FLAG_HIDE) {
        EptHideMemoryRange(in->KernelVA, in->Size);
    }

    out->Status = OMBRA_SUCCESS;
    return OMBRA_SUCCESS;
}

// VMCALL_HIDE_MEMORY_RANGE handler
OMBRA_STATUS HandleHideMemoryRange(U64 address, U64 size) {
    // Configure EPT to make memory:
    // - Executable from kernel context
    // - NOT readable/writable from kernel context
    // - Reads return zeros or cause #PF

    U64 numPages = (size + 0xFFF) / 0x1000;

    for (U64 i = 0; i < numPages; i++) {
        U64 pageAddr = address + (i * 0x1000);

        // Execute-only: allows code execution but reads/writes fault
        EptSetPagePermissions(pageAddr, EPT_EXECUTE);

        // Or use shadow page technique:
        // EptInstallShadowPage(pageAddr, zeroPage, EPT_READ);
    }

    return OMBRA_SUCCESS;
}
```

---

### Task 9: Main Driver Loader (loader/drv_loader.c)

Orchestrate the full driver mapping flow.

```c
#include "drv_loader.h"
#include "pe_parser.h"
#include "pe_imports.h"
#include "pe_mapper.h"
#include "bigpool_test.h"
#include <stdio.h>

bool DrvLoaderInit(DRIVER_MAP_CONTEXT* ctx) {
    memset(ctx, 0, sizeof(*ctx));
    return true;
}

bool DrvLoaderPrepare(
    DRIVER_MAP_CONTEXT* ctx,
    DRV_CONTEXT* supdrv,
    const void* driverImage,
    uint32_t driverSize
) {
    printf("[*] Preparing driver for mapping...\n");

    // Parse PE
    if (!PeParse(driverImage, driverSize, &ctx->PeInfo)) {
        printf("[-] PE parsing failed\n");
        return false;
    }
    printf("[+] PE parsed: %u sections, %u imports\n",
           ctx->PeInfo.SectionCount, ctx->PeInfo.ImportCount);

    // Resolve imports via SUPDrv
    if (!ResolveImports(supdrv, &ctx->PeInfo)) {
        printf("[-] Import resolution failed\n");
        return false;
    }

    // Allocate driver pool
    uint32_t requiredSize = PeCalculateImageSize(&ctx->PeInfo);
    uint32_t poolSize = max(requiredSize, 2 * 1024 * 1024);  // Min 2MB

    if (DrvAllocContiguous(supdrv, poolSize / 0x1000, &ctx->Pool) != DRV_SUCCESS) {
        printf("[-] Pool allocation failed\n");
        return false;
    }
    printf("[+] Pool allocated: R3=%p, R0=%p, Size=%u\n",
           ctx->Pool.R3, ctx->Pool.R0, poolSize);

    // Store image buffer
    ctx->ImageBuffer = malloc(driverSize);
    if (!ctx->ImageBuffer) {
        DrvFreeContiguous(supdrv, &ctx->Pool);
        return false;
    }
    memcpy(ctx->ImageBuffer, driverImage, driverSize);
    ctx->ImageSize = driverSize;

    ctx->Prepared = TRUE;
    return true;
}

bool DrvLoaderMap(DRIVER_MAP_CONTEXT* ctx, HV_LOADER_CTX* hvCtx) {
    if (!ctx->Prepared) {
        printf("[-] Driver not prepared\n");
        return false;
    }

    printf("[*] Mapping driver via hypervisor...\n");

    // Step 1: Claim pool region via VMCALL
    VMCALL_CLAIM_POOL_IN claimIn = {
        .Offset = 0,
        .Size = ctx->PeInfo.ImageSize
    };
    VMCALL_CLAIM_POOL_OUT claimOut = {0};

    if (!VmcallClaimPoolRegion(&claimIn, &claimOut)) {
        printf("[-] VMCALL_CLAIM_POOL_REGION failed\n");
        return false;
    }

    ctx->KernelBase = claimOut.KernelVA;
    printf("[+] Claimed kernel region at 0x%llX\n", ctx->KernelBase);

    // Step 2: Map driver via direct R3 write
    if (!MapDriver(ctx->ImageBuffer, ctx->Pool.R3, ctx->KernelBase, &ctx->PeInfo)) {
        printf("[-] Driver mapping failed\n");
        return false;
    }

    // Step 3: Finalize via VMCALL (set protections, call entry, hide)
    PE_SECTION_INFO* textSec = PeGetSection(&ctx->PeInfo, ".text");

    VMCALL_FINALIZE_IN finalIn = {
        .KernelVA = ctx->KernelBase,
        .Size = ctx->PeInfo.ImageSize,
        .EntryRva = ctx->PeInfo.EntryPointRva,
        .TextRva = textSec ? textSec->Rva : 0,
        .TextSize = textSec ? textSec->VirtualSize : 0,
        .Flags = FINALIZE_FLAG_HIDE
    };
    VMCALL_FINALIZE_OUT finalOut = {0};

    if (!VmcallFinalizeDriverLoad(&finalIn, &finalOut)) {
        printf("[-] VMCALL_FINALIZE_DRIVER_LOAD failed\n");
        return false;
    }

    if (finalOut.DriverEntryResult != 0) {
        printf("[-] DriverEntry returned 0x%X\n", finalOut.DriverEntryResult);
        return false;
    }

    printf("[+] Driver loaded and hidden successfully!\n");
    ctx->Mapped = TRUE;
    ctx->Running = TRUE;

    return true;
}

void DrvLoaderCleanup(DRIVER_MAP_CONTEXT* ctx, DRV_CONTEXT* supdrv) {
    if (ctx->ImageBuffer) {
        // Secure wipe
        volatile uint8_t* p = (volatile uint8_t*)ctx->ImageBuffer;
        for (uint32_t i = 0; i < ctx->ImageSize; i++) {
            p[i] = 0;
        }
        free(ctx->ImageBuffer);
        ctx->ImageBuffer = NULL;
    }

    // Note: Don't free Pool - it's now used by the driver
    // Pool cleanup happens on unload via VMCALL

    memset(ctx, 0, sizeof(*ctx));
}
```

---

### Task 10: Build Hardening

Configure hardened build to avoid compiler signatures.

**Compiler flags (Visual Studio project or Makefile):**

```makefile
# OmbraDriver build flags
CFLAGS_DRIVER = \
    /GS-                    # Disable buffer security checks
    /Oi                     # Enable intrinsic functions
    /Os                     # Favor size
    /W4                     # Warning level 4
    /WX                     # Warnings as errors
    /Zl                     # Omit default library name
    /nologo                 # Suppress banner
    /D_AMD64_               # x64 target
    /DKERNEL_MODE           # Kernel mode define
    /kernel                 # Kernel mode compilation

# Linker flags
LDFLAGS_DRIVER = \
    /DRIVER                 # Driver subsystem
    /ENTRY:DriverEntry      # Entry point
    /NODEFAULTLIB           # No default libraries
    /DYNAMICBASE:NO         # Disable ASLR
    /FIXED                  # Fixed base address
    /MERGE:.rdata=.text     # Merge read-only data
    /SECTION:.text,ERW      # Section attributes
```

**Custom CRT replacements (driver/crt_stubs.c):**

```c
// Custom memset - avoid CRT signature
#pragma function(memset)
void* __cdecl memset(void* dest, int c, size_t count) {
    volatile unsigned char* p = (volatile unsigned char*)dest;
    while (count--) {
        *p++ = (unsigned char)c;
    }
    return dest;
}

// Custom memcpy - avoid CRT signature
#pragma function(memcpy)
void* __cdecl memcpy(void* dest, const void* src, size_t count) {
    volatile unsigned char* d = (volatile unsigned char*)dest;
    const volatile unsigned char* s = (const volatile unsigned char*)src;
    while (count--) {
        *d++ = *s++;
    }
    return dest;
}

// Custom memcmp
#pragma function(memcmp)
int __cdecl memcmp(const void* s1, const void* s2, size_t count) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    while (count--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

// Custom strlen
size_t __cdecl strlen(const char* str) {
    const char* s = str;
    while (*s) s++;
    return s - str;
}
```

---

## Path B: Mitigation Path (If BigPool Visible)

### Option B1: MDL-Based Allocation

If BigPool testing shows allocations are visible, use MDL-based allocation which bypasses pool tracking.

**Additional VMCALL handler (hypervisor/handlers/vmcall_mdl.c):**

```c
#include "vmcall.h"
#include "../../shared/types.h"

// Deferred allocation request tracking
typedef struct _DEFERRED_ALLOC {
    U32     RequestId;
    U32     Size;
    U32     Status;         // PENDING, COMPLETE, FAILED
    U64     ResultVA;
    U64     ResultPhys;
    BOOL    InUse;
} DEFERRED_ALLOC;

#define MAX_DEFERRED_ALLOCS 8
static DEFERRED_ALLOC g_DeferredAllocs[MAX_DEFERRED_ALLOCS] = {0};
static U32 g_NextRequestId = 1;

// Resolved MDL functions (from HV_INIT_PARAMS)
static U64 g_MmAllocatePagesForMdl = 0;
static U64 g_MmMapLockedPagesSpecifyCache = 0;
static U64 g_MmFreePagesFromMdl = 0;

void MdlAlloc_Init(HV_INIT_PARAMS* params) {
    // These would need to be resolved and passed in params
    // Or resolved dynamically
}

// Worker function - runs at PASSIVE_LEVEL via APC
void MdlAllocWorker(DEFERRED_ALLOC* req) {
    typedef PMDL (*FN_MmAllocatePagesForMdl)(
        PHYSICAL_ADDRESS, PHYSICAL_ADDRESS, PHYSICAL_ADDRESS, SIZE_T);
    typedef PVOID (*FN_MmMapLockedPagesSpecifyCache)(
        PMDL, KPROCESSOR_MODE, MEMORY_CACHING_TYPE, PVOID, ULONG, ULONG);

    FN_MmAllocatePagesForMdl allocPages =
        (FN_MmAllocatePagesForMdl)g_MmAllocatePagesForMdl;
    FN_MmMapLockedPagesSpecifyCache mapPages =
        (FN_MmMapLockedPagesSpecifyCache)g_MmMapLockedPagesSpecifyCache;

    PHYSICAL_ADDRESS low = {0};
    PHYSICAL_ADDRESS high = {.QuadPart = -1};
    PHYSICAL_ADDRESS skip = {0};

    PMDL mdl = allocPages(low, high, skip, req->Size);
    if (!mdl) {
        req->Status = ALLOC_FAILED;
        return;
    }

    PVOID va = mapPages(mdl, KernelMode, MmCached, NULL, FALSE, NormalPagePriority);
    if (!va) {
        // Free MDL
        req->Status = ALLOC_FAILED;
        return;
    }

    req->ResultVA = (U64)va;
    req->Status = ALLOC_COMPLETE;
}

// VMCALL_ALLOC_MDL_MEMORY handler
OMBRA_STATUS HandleAllocMdlMemory(VMCALL_ALLOC_MDL_IN* in, VMCALL_ALLOC_MDL_OUT* out) {
    // Find free slot
    DEFERRED_ALLOC* req = NULL;
    for (int i = 0; i < MAX_DEFERRED_ALLOCS; i++) {
        if (!g_DeferredAllocs[i].InUse) {
            req = &g_DeferredAllocs[i];
            break;
        }
    }

    if (!req) {
        out->Status = ALLOC_FAILED;
        return OMBRA_ERROR_OUT_OF_MEMORY;
    }

    req->RequestId = g_NextRequestId++;
    req->Size = in->Size;
    req->Status = ALLOC_PENDING;
    req->InUse = TRUE;

    // Queue APC to perform allocation at safe IRQL
    // This requires a system thread context
    QueueAllocationApc(req);

    out->RequestId = req->RequestId;
    out->Status = ALLOC_PENDING;

    return OMBRA_SUCCESS;
}

// VMCALL_POLL_ALLOC_STATUS handler
OMBRA_STATUS HandlePollAllocStatus(VMCALL_POLL_IN* in, VMCALL_POLL_OUT* out) {
    for (int i = 0; i < MAX_DEFERRED_ALLOCS; i++) {
        if (g_DeferredAllocs[i].InUse &&
            g_DeferredAllocs[i].RequestId == in->RequestId) {

            out->Status = g_DeferredAllocs[i].Status;
            out->KernelVA = g_DeferredAllocs[i].ResultVA;
            out->PhysicalAddr = g_DeferredAllocs[i].ResultPhys;

            if (out->Status != ALLOC_PENDING) {
                g_DeferredAllocs[i].InUse = FALSE;
            }

            return OMBRA_SUCCESS;
        }
    }

    out->Status = ALLOC_FAILED;
    return OMBRA_ERROR_NOT_FOUND;
}
```

### Option B2: EPT-Only Memory

Most sophisticated approach - create mappings Windows has no knowledge of.

```c
// VMCALL_CREATE_EPT_MAPPING handler
OMBRA_STATUS HandleCreateEptMapping(
    U64 requestedVA,    // Desired kernel VA (or 0 for auto)
    U64 physicalAddr,   // Physical pages to map (or 0 to allocate)
    U64 size,
    U64* resultVA
) {
    // 1. Find available VA range in kernel space if not specified
    if (requestedVA == 0) {
        requestedVA = FindUnusedKernelVaRange(size);
        if (requestedVA == 0) {
            return OMBRA_ERROR_OUT_OF_MEMORY;
        }
    }

    // 2. Find or allocate physical pages
    if (physicalAddr == 0) {
        physicalAddr = AllocatePhysicalPages(size / 0x1000);
        if (physicalAddr == 0) {
            return OMBRA_ERROR_OUT_OF_MEMORY;
        }
    }

    // 3. Create EPT mapping
    U64 numPages = (size + 0xFFF) / 0x1000;
    for (U64 i = 0; i < numPages; i++) {
        U64 guestPhys = VaToPa(requestedVA + i * 0x1000);  // Guest physical
        U64 hostPhys = physicalAddr + i * 0x1000;          // Host physical

        // Create EPT entry: Guest PA -> Host PA
        EptMapPage(guestPhys, hostPhys, EPT_READ | EPT_WRITE | EPT_EXECUTE);
    }

    // 4. Flush TLB
    InveptAllContexts();

    *resultVA = requestedVA;
    return OMBRA_SUCCESS;
}
```

---

## File Summary

### New Files (Loader)

| File | Purpose |
|------|---------|
| `loader/drv_loader.h` | Driver loader public API |
| `loader/drv_loader.c` | Main driver loading orchestration |
| `loader/pe_parser.h` | PE parsing declarations |
| `loader/pe_parser.c` | PE header/section/import parsing |
| `loader/pe_imports.h` | Import resolution declarations |
| `loader/pe_imports.c` | Resolve imports via SUPDrv |
| `loader/pe_relocs.h` | Relocation declarations |
| `loader/pe_relocs.c` | Apply PE relocations |
| `loader/pe_iat.h` | IAT patching declarations |
| `loader/pe_iat.c` | Patch IAT with resolved addresses |
| `loader/pe_wipe.h` | Header destruction declarations |
| `loader/pe_wipe.c` | Zero PE headers |
| `loader/pe_mapper.h` | Section mapping declarations |
| `loader/pe_mapper.c` | Copy sections, orchestrate mapping |
| `loader/bigpool_test.h` | BigPool test declarations |
| `loader/bigpool_test.c` | BigPool visibility testing |

### New Files (Hypervisor)

| File | Purpose |
|------|---------|
| `hypervisor/handlers/vmcall_phase2.c` | Phase 2 VMCALL handlers |
| `hypervisor/handlers/vmcall_mdl.c` | MDL allocation handlers (Path B) |
| `hypervisor/tests/bigpool_test.c` | BigPool test implementation |

### New Files (Driver)

| File | Purpose |
|------|---------|
| `driver/crt_stubs.c` | Custom CRT implementations |

### Modified Files

| File | Changes |
|------|---------|
| `shared/types.h` | Add Phase 2 VMCALL IDs and structures |
| `hypervisor/handlers/vmcall.c` | Add Phase 2 VMCALL dispatch |
| `loader/hv_loader.c` | Add driver pool allocation |

---

## Testing Strategy

### Test 1: BigPool Visibility
Run BigPool test before any Phase 2 implementation to determine path.

### Test 2: PE Parsing
Test parser with known good driver (e.g., null driver).

### Test 3: Import Resolution
Verify all standard ntoskrnl imports resolve correctly.

### Test 4: Relocation
Test with driver that has many relocations.

### Test 5: Mapping (Path A)
Map minimal test driver, verify execution.

### Test 6: EPT Hiding
Verify hidden memory is not readable from kernel debugger.

### Test 7: Detection Scan
Run EAC/BE detection tests after full implementation.

---

## Risk Areas

1. **BigPool uncertainty** - Entire path selection depends on test results. Design both paths.

2. **IRQL constraints** - MDL allocation requires PASSIVE_LEVEL. Deferred execution adds complexity.

3. **Import coverage** - Missing imports cause crashes. Pre-resolve common symbols.

4. **Relocation edge cases** - Unusual relocation types may not be handled.

5. **EPT hiding completeness** - Ensure all pages are hidden, including dynamically allocated.

6. **Compiler signatures** - Custom CRT may not cover all compiler-generated code.

7. **Event log forensics** - Even without DeleteService, other traces may exist.
