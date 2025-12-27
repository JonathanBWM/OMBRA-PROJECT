# Phase 1: Hypervisor Loader Implementation Plan

## Overview

Fix the broken hypervisor loading chain. Currently, `HvLoad` allocates memory and attempts to execute via `DrvExecuteOnCpu`, but this doesn't work because SUPDrv's `CALL_VMMR0` expects a properly loaded module via `LDR_OPEN`/`LDR_LOAD`.

**Goal:** Load the hypervisor payload correctly via SUPDrv, virtualize all CPUs using `KeIpiGenericCall`, return with hypervisor active.

---

## Architecture Decisions

### 1. CPU Virtualization Strategy
**Decision:** Single entry point with internal IPI broadcast.

The loader calls the hypervisor entry once. The hypervisor uses `KeIpiGenericCall` (resolved by loader, passed in params) to broadcast initialization to all CPUs.

**Rationale:**
- SUPDrv's `LDR_LOAD` calls entry point once, not per-CPU
- Keeps hypervisor payload pure (minimal Windows dependencies)
- Standard pattern used by HyperPlatform, hvpp, and similar projects
- Errors during IPI broadcast are handled inside hypervisor where we have full context

### 2. Memory Allocation Strategy
**Decision:** Allocate all memory from loader via existing `DrvAllocContiguous`/`DrvAllocPages`.

The loader allocates VMXON regions, VMCS regions, host stacks, MSR bitmap, EPT tables, and debug buffer before loading the hypervisor. Addresses are passed via `HV_INIT_PARAMS`.

**Rationale:**
- Allocation errors surface in usermode (debuggable)
- Hypervisor stays pure (no `MmAllocateContiguousMemory` dependency)
- Pre-initialization (zeroing, revision IDs) can be done from usermode
- Already have working `DrvAllocContiguous`/`DrvAllocPages` implementations

### 3. Parameter Passing Strategy
**Decision:** Custom `.ombra` PE section in hypervisor image.

Loader parses hypervisor PE, finds `.ombra` section, patches `ParamsPtr` field with R0 address of `HV_INIT_PARAMS`. Hypervisor reads params from this known location.

**Rationale:**
- LDPlayer's Ld9BoxSup.sys is a closed-source fork; callback signature unknown
- Parameter-agnostic approach doesn't depend on what SUPDrv passes to entry
- Custom section survives recompilation (no manual offset tracking)
- Loader can validate section exists before patching

### 4. Kernel Symbol Resolution
**Decision:** Use SUPDrv's `LDR_GET_SYMBOL` via existing `DrvGetSymbol`.

Loader resolves `KeIpiGenericCall`, `KeQueryActiveProcessorCountEx`, `KeGetCurrentProcessorNumberEx` before loading hypervisor. Addresses passed in `HV_INIT_PARAMS`.

**Rationale:**
- Already implemented and working
- Errors surface in usermode
- Hypervisor doesn't need PE parsing or export walking code

---

## Data Structures

### HV_INIT_PARAMS (loader → hypervisor)

```c
typedef struct _HV_INIT_PARAMS {
    // Magic and version for validation
    U64     Magic;              // 0x4F4D4252 ('OMBR')
    U32     Version;            // 1
    U32     CpuCount;           // Number of CPUs to virtualize

    // Per-CPU regions (arrays, indexed by CPU number)
    // Each VMXON/VMCS is one 4KB page
    void*   VmxonRegionsVirt;   // Array base: CpuCount pages
    U64     VmxonRegionsPhys;   // Physical address of array base
    void*   VmcsRegionsVirt;    // Array base: CpuCount pages
    U64     VmcsRegionsPhys;    // Physical address of array base

    // Host stacks (array, each stack is HOST_STACK_SIZE bytes)
    void*   HostStacksBase;     // Array base
    U32     HostStackSize;      // Size per stack (e.g., 0x4000 = 16KB)

    // Shared structures
    void*   MsrBitmapVirt;      // 4KB MSR bitmap
    U64     MsrBitmapPhys;

    // EPT tables
    void*   EptTablesVirt;      // EPT table region base
    U64     EptTablesPhys;      // Physical address
    U32     EptTablesPages;     // Number of pages allocated

    // Debug buffer
    void*   DebugBufferVirt;
    U64     DebugBufferPhys;
    U64     DebugBufferSize;

    // Resolved kernel functions
    U64     KeIpiGenericCall;
    U64     KeQueryActiveProcessorCountEx;
    U64     KeGetCurrentProcessorNumberEx;

    // Pre-read VMX capability MSRs
    U64     VmxBasic;
    U64     VmxPinbasedCtls;
    U64     VmxProcbasedCtls;
    U64     VmxProcbasedCtls2;
    U64     VmxExitCtls;
    U64     VmxEntryCtls;
    U64     VmxTruePinbasedCtls;
    U64     VmxTrueProcbasedCtls;
    U64     VmxTrueExitCtls;
    U64     VmxTrueEntryCtls;
    U64     VmxCr0Fixed0;
    U64     VmxCr0Fixed1;
    U64     VmxCr4Fixed0;
    U64     VmxCr4Fixed1;
    U64     VmxEptVpidCap;

    // VMCALL authentication
    U64     VmcallKey;

    // Flags
    U32     Flags;
    U32     Reserved;
} HV_INIT_PARAMS;
```

### OMBRA_BOOTSTRAP (in .ombra section)

```c
typedef struct _OMBRA_BOOTSTRAP {
    U64     Magic;          // 0x524D424F ('OMBR' little-endian)
    U64     Version;        // 1
    U64     ParamsPtr;      // Loader patches: R0 pointer to HV_INIT_PARAMS
    U64     Reserved[5];    // Future expansion
} OMBRA_BOOTSTRAP;
```

### HV_MEMORY_LAYOUT (loader internal tracking)

```c
typedef struct _HV_MEMORY_LAYOUT {
    // Per-CPU (contiguous, need physical addresses)
    ALLOC_INFO  VmxonRegions;       // CpuCount pages
    ALLOC_INFO  VmcsRegions;        // CpuCount pages
    ALLOC_INFO  HostStacks;         // CpuCount * STACK_PAGES pages

    // Shared (contiguous)
    ALLOC_INFO  MsrBitmap;          // 1 page
    ALLOC_INFO  EptTables;          // EPT_TABLES_PAGES pages

    // Params page
    ALLOC_INFO  ParamsPage;         // 1 page for HV_INIT_PARAMS

    // Debug (non-paged OK)
    ALLOC_INFO  DebugBuffer;        // DEBUG_BUFFER_PAGES pages
} HV_MEMORY_LAYOUT;
```

---

## Implementation Tasks

### Task 1: Create loader/hv_loader.h

Define public interface for hypervisor loading.

```c
#ifndef HV_LOADER_H
#define HV_LOADER_H

#include "driver_interface.h"
#include "../shared/types.h"

// Configuration
#define HOST_STACK_SIZE     0x4000      // 16KB per CPU
#define HOST_STACK_PAGES    (HOST_STACK_SIZE / 0x1000)
#define EPT_TABLES_PAGES    512         // 2MB for identity mapping
#define DEBUG_BUFFER_PAGES  16          // 64KB debug ring buffer

// Context for loaded hypervisor
typedef struct _HV_LOADER_CTX {
    DRV_CONTEXT         Driver;
    U32                 CpuCount;
    HV_MEMORY_LAYOUT    Memory;
    HV_INIT_PARAMS      Params;

    // Hypervisor module
    void*               ImageBase;      // Kernel address from LDR_OPEN
    U32                 ImageSize;

    // State
    BOOL                Loaded;
    BOOL                Running;
} HV_LOADER_CTX;

// Public API
BOOL HvLoaderInit(HV_LOADER_CTX* ctx, const wchar_t* driverPath);
BOOL HvLoaderLoad(HV_LOADER_CTX* ctx, const void* hvImage, U32 hvImageSize);
BOOL HvLoaderUnload(HV_LOADER_CTX* ctx);
BOOL HvLoaderIsRunning(HV_LOADER_CTX* ctx);
void HvLoaderCleanup(HV_LOADER_CTX* ctx);

// Debug buffer access
void* HvLoaderGetDebugBuffer(HV_LOADER_CTX* ctx);
U64   HvLoaderGetDebugBufferSize(HV_LOADER_CTX* ctx);

#endif // HV_LOADER_H
```

**File:** `hypervisor/loader/hv_loader.h`

---

### Task 2: Create loader/pe_utils.h and pe_utils.c

PE parsing utilities for finding `.ombra` section.

**Header (pe_utils.h):**
```c
#ifndef PE_UTILS_H
#define PE_UTILS_H

#include <stdint.h>
#include <stdbool.h>

// Find a named section in a PE image
// Returns true if found, fills offset and size
bool PeFindSection(
    const void* peImage,
    const char* sectionName,    // e.g., ".ombra"
    uint32_t* outFileOffset,    // Offset in file
    uint32_t* outSize           // Size of section
);

// Get entry point RVA from PE
bool PeGetEntryPoint(
    const void* peImage,
    uint32_t* outEntryRva
);

// Validate PE image
bool PeValidate(const void* peImage, uint32_t imageSize);

#endif // PE_UTILS_H
```

**Implementation (pe_utils.c):**

```c
#include "pe_utils.h"
#include <Windows.h>
#include <string.h>

bool PeValidate(const void* peImage, uint32_t imageSize) {
    if (imageSize < sizeof(IMAGE_DOS_HEADER)) return false;

    const IMAGE_DOS_HEADER* dos = (const IMAGE_DOS_HEADER*)peImage;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return false;

    if ((uint32_t)dos->e_lfanew + sizeof(IMAGE_NT_HEADERS64) > imageSize) return false;

    const IMAGE_NT_HEADERS64* nt = (const IMAGE_NT_HEADERS64*)((uint8_t*)peImage + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return false;

    if (nt->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) return false;

    return true;
}

bool PeFindSection(
    const void* peImage,
    const char* sectionName,
    uint32_t* outFileOffset,
    uint32_t* outSize)
{
    const IMAGE_DOS_HEADER* dos = (const IMAGE_DOS_HEADER*)peImage;
    const IMAGE_NT_HEADERS64* nt = (const IMAGE_NT_HEADERS64*)((uint8_t*)peImage + dos->e_lfanew);

    const IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(nt);
    uint16_t numSections = nt->FileHeader.NumberOfSections;

    size_t nameLen = strlen(sectionName);
    if (nameLen > 8) nameLen = 8;  // Section names max 8 chars

    for (uint16_t i = 0; i < numSections; i++) {
        if (memcmp(sections[i].Name, sectionName, nameLen) == 0) {
            *outFileOffset = sections[i].PointerToRawData;
            *outSize = sections[i].SizeOfRawData;
            return true;
        }
    }

    return false;
}

bool PeGetEntryPoint(const void* peImage, uint32_t* outEntryRva) {
    const IMAGE_DOS_HEADER* dos = (const IMAGE_DOS_HEADER*)peImage;
    const IMAGE_NT_HEADERS64* nt = (const IMAGE_NT_HEADERS64*)((uint8_t*)peImage + dos->e_lfanew);

    *outEntryRva = nt->OptionalHeader.AddressOfEntryPoint;
    return true;
}
```

**File:** `hypervisor/loader/pe_utils.h`, `hypervisor/loader/pe_utils.c`

---

### Task 3: Create loader/hv_loader.c - Symbol Resolution

Resolve required kernel symbols via SUPDrv.

```c
// In hv_loader.c

typedef struct _KERNEL_SYMBOLS {
    U64     KeIpiGenericCall;
    U64     KeQueryActiveProcessorCountEx;
    U64     KeGetCurrentProcessorNumberEx;
} KERNEL_SYMBOLS;

static BOOL ResolveKernelSymbols(DRV_CONTEXT* drv, KERNEL_SYMBOLS* syms) {
    void* addr;

    if (DrvGetSymbol(drv, "KeIpiGenericCall", &addr) != DRV_SUCCESS) {
        printf("[-] Failed to resolve KeIpiGenericCall\n");
        return FALSE;
    }
    syms->KeIpiGenericCall = (U64)addr;
    printf("[+] KeIpiGenericCall @ 0x%llX\n", syms->KeIpiGenericCall);

    if (DrvGetSymbol(drv, "KeQueryActiveProcessorCountEx", &addr) != DRV_SUCCESS) {
        printf("[-] Failed to resolve KeQueryActiveProcessorCountEx\n");
        return FALSE;
    }
    syms->KeQueryActiveProcessorCountEx = (U64)addr;
    printf("[+] KeQueryActiveProcessorCountEx @ 0x%llX\n", syms->KeQueryActiveProcessorCountEx);

    if (DrvGetSymbol(drv, "KeGetCurrentProcessorNumberEx", &addr) != DRV_SUCCESS) {
        printf("[-] Failed to resolve KeGetCurrentProcessorNumberEx\n");
        return FALSE;
    }
    syms->KeGetCurrentProcessorNumberEx = (U64)addr;
    printf("[+] KeGetCurrentProcessorNumberEx @ 0x%llX\n", syms->KeGetCurrentProcessorNumberEx);

    return TRUE;
}
```

---

### Task 4: Create loader/hv_loader.c - Memory Allocation

Allocate all required memory regions.

```c
static BOOL AllocateHypervisorMemory(HV_LOADER_CTX* ctx) {
    DRV_CONTEXT* drv = &ctx->Driver;
    U32 cpuCount = ctx->CpuCount;
    HV_MEMORY_LAYOUT* mem = &ctx->Memory;

    printf("[*] Allocating memory for %u CPUs\n", cpuCount);

    // VMXON regions: one 4KB page per CPU, must be contiguous
    if (DrvAllocContiguous(drv, cpuCount, &mem->VmxonRegions) != DRV_SUCCESS) {
        printf("[-] Failed to allocate VMXON regions\n");
        return FALSE;
    }
    printf("[+] VMXON regions @ R0=0x%p, Phys=0x%llX\n",
           mem->VmxonRegions.R0, mem->VmxonRegions.Physical);

    // VMCS regions: one 4KB page per CPU, must be contiguous
    if (DrvAllocContiguous(drv, cpuCount, &mem->VmcsRegions) != DRV_SUCCESS) {
        printf("[-] Failed to allocate VMCS regions\n");
        return FALSE;
    }
    printf("[+] VMCS regions @ R0=0x%p, Phys=0x%llX\n",
           mem->VmcsRegions.R0, mem->VmcsRegions.Physical);

    // Host stacks: HOST_STACK_PAGES per CPU
    U32 totalStackPages = HOST_STACK_PAGES * cpuCount;
    if (DrvAllocPages(drv, totalStackPages, TRUE, &mem->HostStacks) != DRV_SUCCESS) {
        printf("[-] Failed to allocate host stacks\n");
        return FALSE;
    }
    printf("[+] Host stacks @ R0=0x%p (%u pages)\n",
           mem->HostStacks.R0, totalStackPages);

    // MSR bitmap: single 4KB page
    if (DrvAllocContiguous(drv, 1, &mem->MsrBitmap) != DRV_SUCCESS) {
        printf("[-] Failed to allocate MSR bitmap\n");
        return FALSE;
    }
    printf("[+] MSR bitmap @ R0=0x%p, Phys=0x%llX\n",
           mem->MsrBitmap.R0, mem->MsrBitmap.Physical);

    // EPT tables
    if (DrvAllocContiguous(drv, EPT_TABLES_PAGES, &mem->EptTables) != DRV_SUCCESS) {
        printf("[-] Failed to allocate EPT tables\n");
        return FALSE;
    }
    printf("[+] EPT tables @ R0=0x%p, Phys=0x%llX (%u pages)\n",
           mem->EptTables.R0, mem->EptTables.Physical, EPT_TABLES_PAGES);

    // Params page
    if (DrvAllocContiguous(drv, 1, &mem->ParamsPage) != DRV_SUCCESS) {
        printf("[-] Failed to allocate params page\n");
        return FALSE;
    }
    printf("[+] Params page @ R0=0x%p, Phys=0x%llX\n",
           mem->ParamsPage.R0, mem->ParamsPage.Physical);

    // Debug buffer
    if (DrvAllocPages(drv, DEBUG_BUFFER_PAGES, TRUE, &mem->DebugBuffer) != DRV_SUCCESS) {
        printf("[-] Failed to allocate debug buffer\n");
        return FALSE;
    }
    printf("[+] Debug buffer @ R0=0x%p (%u pages)\n",
           mem->DebugBuffer.R0, DEBUG_BUFFER_PAGES);

    return TRUE;
}

static void FreeHypervisorMemory(HV_LOADER_CTX* ctx) {
    DRV_CONTEXT* drv = &ctx->Driver;
    HV_MEMORY_LAYOUT* mem = &ctx->Memory;

    if (mem->VmxonRegions.R3)  DrvFreeContiguous(drv, &mem->VmxonRegions);
    if (mem->VmcsRegions.R3)   DrvFreeContiguous(drv, &mem->VmcsRegions);
    if (mem->HostStacks.R3)    DrvFreePages(drv, &mem->HostStacks);
    if (mem->MsrBitmap.R3)     DrvFreeContiguous(drv, &mem->MsrBitmap);
    if (mem->EptTables.R3)     DrvFreeContiguous(drv, &mem->EptTables);
    if (mem->ParamsPage.R3)    DrvFreeContiguous(drv, &mem->ParamsPage);
    if (mem->DebugBuffer.R3)   DrvFreePages(drv, &mem->DebugBuffer);

    memset(mem, 0, sizeof(*mem));
}
```

---

### Task 5: Create loader/hv_loader.c - Structure Preparation

Zero and initialize VMX structures from usermode.

```c
static void PrepareVmxStructures(HV_LOADER_CTX* ctx) {
    U32 vmcsRevision = (U32)(ctx->Driver.VmxMsrs.Basic & 0x7FFFFFFF);
    U32 cpuCount = ctx->CpuCount;
    HV_MEMORY_LAYOUT* mem = &ctx->Memory;

    printf("[*] Preparing VMX structures (revision 0x%X)\n", vmcsRevision);

    // Zero and set revision ID for each VMXON region
    for (U32 i = 0; i < cpuCount; i++) {
        void* vmxon = (U8*)mem->VmxonRegions.R3 + (i * 0x1000);
        memset(vmxon, 0, 0x1000);
        *(U32*)vmxon = vmcsRevision;
    }

    // Zero and set revision ID for each VMCS region
    for (U32 i = 0; i < cpuCount; i++) {
        void* vmcs = (U8*)mem->VmcsRegions.R3 + (i * 0x1000);
        memset(vmcs, 0, 0x1000);
        *(U32*)vmcs = vmcsRevision;
    }

    // Zero host stacks
    memset(mem->HostStacks.R3, 0, HOST_STACK_PAGES * cpuCount * 0x1000);

    // Zero MSR bitmap (pass-through all MSRs initially)
    memset(mem->MsrBitmap.R3, 0, 0x1000);

    // Zero EPT tables
    memset(mem->EptTables.R3, 0, EPT_TABLES_PAGES * 0x1000);

    // Zero debug buffer
    memset(mem->DebugBuffer.R3, 0, DEBUG_BUFFER_PAGES * 0x1000);

    // Zero params page
    memset(mem->ParamsPage.R3, 0, 0x1000);
}
```

---

### Task 6: Create loader/hv_loader.c - Build Init Params

Construct `HV_INIT_PARAMS` from allocated regions and resolved symbols.

```c
static void BuildInitParams(HV_LOADER_CTX* ctx, KERNEL_SYMBOLS* syms) {
    HV_MEMORY_LAYOUT* mem = &ctx->Memory;
    HV_INIT_PARAMS* params = (HV_INIT_PARAMS*)mem->ParamsPage.R3;

    // Magic and version
    params->Magic = 0x4F4D4252;  // 'OMBR'
    params->Version = 1;
    params->CpuCount = ctx->CpuCount;

    // Per-CPU regions
    params->VmxonRegionsVirt = mem->VmxonRegions.R0;
    params->VmxonRegionsPhys = mem->VmxonRegions.Physical;
    params->VmcsRegionsVirt = mem->VmcsRegions.R0;
    params->VmcsRegionsPhys = mem->VmcsRegions.Physical;

    // Host stacks
    params->HostStacksBase = mem->HostStacks.R0;
    params->HostStackSize = HOST_STACK_SIZE;

    // Shared structures
    params->MsrBitmapVirt = mem->MsrBitmap.R0;
    params->MsrBitmapPhys = mem->MsrBitmap.Physical;

    // EPT
    params->EptTablesVirt = mem->EptTables.R0;
    params->EptTablesPhys = mem->EptTables.Physical;
    params->EptTablesPages = EPT_TABLES_PAGES;

    // Debug
    params->DebugBufferVirt = mem->DebugBuffer.R0;
    params->DebugBufferPhys = mem->DebugBuffer.Physical;
    params->DebugBufferSize = DEBUG_BUFFER_PAGES * 0x1000;

    // Kernel symbols
    params->KeIpiGenericCall = syms->KeIpiGenericCall;
    params->KeQueryActiveProcessorCountEx = syms->KeQueryActiveProcessorCountEx;
    params->KeGetCurrentProcessorNumberEx = syms->KeGetCurrentProcessorNumberEx;

    // VMX MSRs (from driver context, pre-read during DrvInitialize)
    params->VmxBasic = ctx->Driver.VmxMsrs.Basic;
    params->VmxPinbasedCtls = ctx->Driver.VmxMsrs.PinCtls;
    params->VmxProcbasedCtls = ctx->Driver.VmxMsrs.ProcCtls;
    params->VmxProcbasedCtls2 = ctx->Driver.VmxMsrs.ProcCtls2;
    params->VmxExitCtls = ctx->Driver.VmxMsrs.ExitCtls;
    params->VmxEntryCtls = ctx->Driver.VmxMsrs.EntryCtls;
    params->VmxTruePinbasedCtls = ctx->Driver.VmxMsrs.TruePin;
    params->VmxTrueProcbasedCtls = ctx->Driver.VmxMsrs.TrueProc;
    params->VmxTrueExitCtls = ctx->Driver.VmxMsrs.TrueExit;
    params->VmxTrueEntryCtls = ctx->Driver.VmxMsrs.TrueEntry;
    params->VmxCr0Fixed0 = ctx->Driver.VmxMsrs.Cr0Fixed0;
    params->VmxCr0Fixed1 = ctx->Driver.VmxMsrs.Cr0Fixed1;
    params->VmxCr4Fixed0 = ctx->Driver.VmxMsrs.Cr4Fixed0;
    params->VmxCr4Fixed1 = ctx->Driver.VmxMsrs.Cr4Fixed1;
    params->VmxEptVpidCap = ctx->Driver.VmxMsrs.EptVpidCap;

    // VMCALL key (generate random or use fixed for now)
    params->VmcallKey = 0xDEADBEEFCAFEBABE;  // TODO: randomize

    params->Flags = 0;
    params->Reserved = 0;

    printf("[+] HV_INIT_PARAMS built at R0=0x%p\n", mem->ParamsPage.R0);
}
```

---

### Task 7: Create loader/hv_loader.c - Patch Bootstrap Section

Find and patch `.ombra` section in hypervisor PE.

```c
static BOOL PatchBootstrapSection(void* hvImage, U32 hvImageSize, U64 paramsR0) {
    U32 sectionOffset, sectionSize;

    // Validate PE
    if (!PeValidate(hvImage, hvImageSize)) {
        printf("[-] Invalid PE image\n");
        return FALSE;
    }

    // Find .ombra section
    if (!PeFindSection(hvImage, ".ombra", &sectionOffset, &sectionSize)) {
        printf("[-] .ombra section not found\n");
        return FALSE;
    }

    printf("[*] Found .ombra section at offset 0x%X, size %u\n", sectionOffset, sectionSize);

    if (sectionSize < sizeof(OMBRA_BOOTSTRAP)) {
        printf("[-] .ombra section too small: %u < %zu\n", sectionSize, sizeof(OMBRA_BOOTSTRAP));
        return FALSE;
    }

    // Get bootstrap structure
    OMBRA_BOOTSTRAP* bootstrap = (OMBRA_BOOTSTRAP*)((U8*)hvImage + sectionOffset);

    // Validate magic
    if (bootstrap->Magic != 0x524D424F) {  // 'OMBR'
        printf("[-] .ombra magic mismatch: 0x%llX\n", bootstrap->Magic);
        return FALSE;
    }

    if (bootstrap->Version != 1) {
        printf("[-] .ombra version mismatch: %llu\n", bootstrap->Version);
        return FALSE;
    }

    // Patch params pointer
    bootstrap->ParamsPtr = paramsR0;

    printf("[+] Patched .ombra ParamsPtr = 0x%llX\n", paramsR0);
    return TRUE;
}
```

---

### Task 8: Create loader/hv_loader.c - Load via LDR_OPEN/LDR_LOAD

Use SUPDrv's module loading interface.

```c
static BOOL LoadHypervisorModule(HV_LOADER_CTX* ctx, void* hvImage, U32 hvImageSize) {
    PSUPDRV_CTX supdrv = &ctx->Driver.SupDrv;  // Assuming we expose SUPDrv context

    // Get entry point RVA
    U32 entryRva;
    if (!PeGetEntryPoint(hvImage, &entryRva)) {
        printf("[-] Failed to get entry point from PE\n");
        return FALSE;
    }
    printf("[*] Entry point RVA: 0x%X\n", entryRva);

    // LDR_OPEN: request kernel memory for image
    void* imageBase = NULL;
    if (!SupDrv_LdrOpen(supdrv, hvImageSize, &imageBase)) {
        printf("[-] LDR_OPEN failed: %s\n", SupDrv_GetLastError(supdrv));
        return FALSE;
    }
    printf("[+] LDR_OPEN: imageBase = 0x%p\n", imageBase);
    ctx->ImageBase = imageBase;
    ctx->ImageSize = hvImageSize;

    // Calculate entry point kernel address
    void* entryPoint = (U8*)imageBase + entryRva;
    printf("[*] Entry point @ 0x%p\n", entryPoint);

    // LDR_LOAD: copy image and register entry point
    if (!SupDrv_LdrLoad(supdrv, imageBase, hvImage, hvImageSize, entryPoint)) {
        printf("[-] LDR_LOAD failed: %s\n", SupDrv_GetLastError(supdrv));
        // TODO: LDR_FREE to release imageBase
        return FALSE;
    }
    printf("[+] LDR_LOAD: hypervisor loaded and entry point called\n");

    return TRUE;
}
```

---

### Task 9: Create loader/hv_loader.c - Main Load Function

Orchestrate the full loading flow.

```c
BOOL HvLoaderInit(HV_LOADER_CTX* ctx, const wchar_t* driverPath) {
    memset(ctx, 0, sizeof(*ctx));

    printf("[*] Initializing hypervisor loader\n");

    // Initialize driver interface
    if (DrvInitialize(&ctx->Driver, driverPath) != DRV_SUCCESS) {
        printf("[-] Failed to initialize driver interface\n");
        return FALSE;
    }

    // Get CPU count
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    ctx->CpuCount = sysInfo.dwNumberOfProcessors;
    printf("[+] System has %u CPUs\n", ctx->CpuCount);

    return TRUE;
}

BOOL HvLoaderLoad(HV_LOADER_CTX* ctx, const void* hvImage, U32 hvImageSize) {
    KERNEL_SYMBOLS syms = {0};

    printf("[*] Loading hypervisor (%u bytes)\n", hvImageSize);

    // 1. Resolve kernel symbols
    printf("[*] Resolving kernel symbols...\n");
    if (!ResolveKernelSymbols(&ctx->Driver, &syms)) {
        return FALSE;
    }

    // 2. Allocate memory
    printf("[*] Allocating hypervisor memory...\n");
    if (!AllocateHypervisorMemory(ctx)) {
        return FALSE;
    }

    // 3. Prepare VMX structures
    printf("[*] Preparing VMX structures...\n");
    PrepareVmxStructures(ctx);

    // 4. Build init params
    printf("[*] Building init params...\n");
    BuildInitParams(ctx, &syms);

    // 5. Make a copy of image to patch
    void* hvImageCopy = malloc(hvImageSize);
    if (!hvImageCopy) {
        printf("[-] Failed to allocate image copy\n");
        FreeHypervisorMemory(ctx);
        return FALSE;
    }
    memcpy(hvImageCopy, hvImage, hvImageSize);

    // 6. Patch bootstrap section
    printf("[*] Patching bootstrap section...\n");
    U64 paramsR0 = (U64)ctx->Memory.ParamsPage.R0;
    if (!PatchBootstrapSection(hvImageCopy, hvImageSize, paramsR0)) {
        free(hvImageCopy);
        FreeHypervisorMemory(ctx);
        return FALSE;
    }

    // 7. Load hypervisor module
    printf("[*] Loading hypervisor module...\n");
    if (!LoadHypervisorModule(ctx, hvImageCopy, hvImageSize)) {
        free(hvImageCopy);
        FreeHypervisorMemory(ctx);
        return FALSE;
    }

    free(hvImageCopy);

    ctx->Loaded = TRUE;
    ctx->Running = TRUE;

    printf("[+] Hypervisor loaded successfully!\n");
    return TRUE;
}

void HvLoaderCleanup(HV_LOADER_CTX* ctx) {
    if (ctx->Running) {
        HvLoaderUnload(ctx);
    }

    FreeHypervisorMemory(ctx);
    DrvCleanup(&ctx->Driver);

    memset(ctx, 0, sizeof(*ctx));
}
```

---

### Task 10: Modify hypervisor/hypervisor/entry.c - Add Bootstrap

Add `.ombra` section and `OmbraModuleInit` wrapper.

```c
// === Add to entry.c ===

// Bootstrap section
#pragma section(".ombra", read, write)
#pragma comment(linker, "/SECTION:.ombra,RW")

typedef struct _OMBRA_BOOTSTRAP {
    U64     Magic;          // 0x524D424F ('OMBR')
    U64     Version;        // 1
    U64     ParamsPtr;      // Loader patches this
    U64     Reserved[5];
} OMBRA_BOOTSTRAP;

__declspec(allocate(".ombra"))
volatile OMBRA_BOOTSTRAP g_Bootstrap = {
    .Magic = 0x524D424F,
    .Version = 1,
    .ParamsPtr = 0,
    .Reserved = {0}
};

// Module init entry point (called by SUPDrv after LDR_LOAD)
__declspec(dllexport)
int OmbraModuleInit(void* ignored) {
    // Read params from bootstrap section
    if (g_Bootstrap.Magic != 0x524D424F) {
        return -1;  // Bootstrap magic invalid
    }

    if (g_Bootstrap.Version != 1) {
        return -2;  // Bootstrap version mismatch
    }

    if (g_Bootstrap.ParamsPtr == 0) {
        return -3;  // Params not patched
    }

    HV_INIT_PARAMS* params = (HV_INIT_PARAMS*)g_Bootstrap.ParamsPtr;

    // Validate params structure
    if (params->Magic != 0x4F4D4252) {  // 'OMBR'
        return -4;  // Params magic invalid
    }

    if (params->Version != 1) {
        return -5;  // Params version mismatch
    }

    // Call actual initialization
    return OmbraInitialize(params);
}
```

---

### Task 11: Modify hypervisor/hypervisor/entry.c - IPI Broadcast

Update `OmbraInitialize` to use `KeIpiGenericCall` for multi-CPU init.

```c
// === Modify OmbraInitialize in entry.c ===

// IPI callback signature
typedef ULONG_PTR (NTAPI *KIPI_BROADCAST_WORKER)(ULONG_PTR Argument);
typedef ULONG_PTR (NTAPI *FN_KeIpiGenericCall)(KIPI_BROADCAST_WORKER, ULONG_PTR);
typedef ULONG (NTAPI *FN_KeQueryActiveProcessorCountEx)(USHORT);
typedef ULONG (NTAPI *FN_KeGetCurrentProcessorNumberEx)(void*);

// Global for IPI callback access
static HV_INIT_PARAMS* g_InitParams = NULL;
static volatile U32 g_SuccessCount = 0;
static volatile U32 g_FailCount = 0;

// IPI callback - runs on each CPU
static ULONG_PTR NTAPI VirtualizeThisCpu(ULONG_PTR Argument) {
    HV_INIT_PARAMS* params = (HV_INIT_PARAMS*)Argument;

    // Get current CPU number
    FN_KeGetCurrentProcessorNumberEx getCpuNum =
        (FN_KeGetCurrentProcessorNumberEx)params->KeGetCurrentProcessorNumberEx;
    U32 cpuId = getCpuNum(NULL);

    // Calculate per-CPU addresses
    void* vmxonVirt = (U8*)params->VmxonRegionsVirt + (cpuId * 0x1000);
    U64 vmxonPhys = params->VmxonRegionsPhys + (cpuId * 0x1000);
    void* vmcsVirt = (U8*)params->VmcsRegionsVirt + (cpuId * 0x1000);
    U64 vmcsPhys = params->VmcsRegionsPhys + (cpuId * 0x1000);
    void* stackTop = (U8*)params->HostStacksBase + ((cpuId + 1) * params->HostStackSize);

    // Initialize this CPU
    OMBRA_STATUS status = InitializeCpuVmx(
        cpuId,
        vmxonVirt, vmxonPhys,
        vmcsVirt, vmcsPhys,
        stackTop,
        params->MsrBitmapVirt, params->MsrBitmapPhys,
        params
    );

    if (OMBRA_SUCCESS == status) {
        _InterlockedIncrement((volatile long*)&g_SuccessCount);
    } else {
        _InterlockedIncrement((volatile long*)&g_FailCount);
    }

    return status;
}

__declspec(dllexport)
int OmbraInitialize(HV_INIT_PARAMS* params) {
    // Initialize debug logging first (if buffer provided)
    if (params->DebugBufferVirt && params->DebugBufferSize > 0) {
        DbgInitialize(params->DebugBufferVirt, params->DebugBufferSize);
    }

    INFO("OmbraHypervisor starting...");
    INFO("CPU count: %u", params->CpuCount);

    // Initialize EPT (once, shared across CPUs)
    OMBRA_STATUS status = EptInitialize(
        &g_EptState,
        params->EptTablesVirt,
        params->EptTablesPhys,
        params->EptTablesPages
    );
    if (OMBRA_FAILED(status)) {
        ERR("EPT initialization failed: 0x%X", status);
        return (int)status;
    }
    INFO("EPT initialized");

    // Initialize hook manager
    status = HookManagerInit(&g_HookManager, &g_EptState);
    if (OMBRA_FAILED(status)) {
        ERR("Hook manager init failed: 0x%X", status);
        return (int)status;
    }
    INFO("Hook manager initialized");

    // Store params globally for IPI callback
    g_InitParams = params;
    g_SuccessCount = 0;
    g_FailCount = 0;

    // Broadcast to all CPUs
    INFO("Broadcasting VMX init to all CPUs...");

    FN_KeIpiGenericCall ipiCall = (FN_KeIpiGenericCall)params->KeIpiGenericCall;
    ipiCall(VirtualizeThisCpu, (ULONG_PTR)params);

    // Check results
    INFO("VMX init complete: %u success, %u failed", g_SuccessCount, g_FailCount);

    if (g_SuccessCount == 0) {
        ERR("No CPUs virtualized!");
        return -100;
    }

    if (g_FailCount > 0) {
        WARN("Some CPUs failed to virtualize");
        // Continue anyway - partial virtualization may be acceptable
    }

    INFO("OmbraHypervisor active on %u CPUs", g_SuccessCount);
    return 0;
}
```

---

### Task 12: Update shared/types.h - Add HV_INIT_PARAMS

Add the `HV_INIT_PARAMS` structure definition to shared header.

```c
// === Add to shared/types.h ===

// Hypervisor initialization parameters
// Passed from loader to hypervisor via .ombra bootstrap section
typedef struct _HV_INIT_PARAMS {
    U64     Magic;              // 0x4F4D4252 ('OMBR')
    U32     Version;            // 1
    U32     CpuCount;

    // Per-CPU regions
    void*   VmxonRegionsVirt;
    U64     VmxonRegionsPhys;
    void*   VmcsRegionsVirt;
    U64     VmcsRegionsPhys;
    void*   HostStacksBase;
    U32     HostStackSize;
    U32     _Pad1;

    // Shared structures
    void*   MsrBitmapVirt;
    U64     MsrBitmapPhys;
    void*   EptTablesVirt;
    U64     EptTablesPhys;
    U32     EptTablesPages;
    U32     _Pad2;

    // Debug
    void*   DebugBufferVirt;
    U64     DebugBufferPhys;
    U64     DebugBufferSize;

    // Kernel symbols
    U64     KeIpiGenericCall;
    U64     KeQueryActiveProcessorCountEx;
    U64     KeGetCurrentProcessorNumberEx;

    // VMX MSRs
    U64     VmxBasic;
    U64     VmxPinbasedCtls;
    U64     VmxProcbasedCtls;
    U64     VmxProcbasedCtls2;
    U64     VmxExitCtls;
    U64     VmxEntryCtls;
    U64     VmxTruePinbasedCtls;
    U64     VmxTrueProcbasedCtls;
    U64     VmxTrueExitCtls;
    U64     VmxTrueEntryCtls;
    U64     VmxCr0Fixed0;
    U64     VmxCr0Fixed1;
    U64     VmxCr4Fixed0;
    U64     VmxCr4Fixed1;
    U64     VmxEptVpidCap;

    // Auth
    U64     VmcallKey;

    // Flags
    U32     Flags;
    U32     Reserved;
} HV_INIT_PARAMS;
```

---

## File Summary

### New Files
| File | Purpose |
|------|---------|
| `loader/hv_loader.h` | Public API for hypervisor loading |
| `loader/hv_loader.c` | Main loader implementation |
| `loader/pe_utils.h` | PE parsing declarations |
| `loader/pe_utils.c` | PE section finding, validation |

### Modified Files
| File | Changes |
|------|---------|
| `hypervisor/entry.c` | Add `.ombra` section, `OmbraModuleInit`, IPI broadcast |
| `shared/types.h` | Add `HV_INIT_PARAMS` structure |

### Potentially Modified Files
| File | Changes |
|------|---------|
| `usermode/main.c` | Update to use new `HvLoaderLoad` API |
| `usermode/byovd/supdrv.c` | Verify `LDR_OPEN`/`LDR_LOAD` work correctly |
| `usermode/driver_interface.h` | Expose SUPDrv context if needed |

---

## Testing Strategy

### Test 1: Symbol Resolution
Verify `DrvGetSymbol` successfully resolves all three kernel functions.

### Test 2: Memory Allocation
Verify all allocations succeed and return valid R0/R3/Physical addresses.

### Test 3: PE Parsing
Test `PeFindSection` with a minimal PE that has `.ombra` section.

### Test 4: Bootstrap Patching
Verify magic validation and patching works correctly.

### Test 5: Single-CPU VM Launch
Test on a single-CPU VM first to verify VMXON/VMLAUNCH works.

### Test 6: Multi-CPU VM Launch
Test IPI broadcast on multi-CPU system.

### Test 7: Error Cases
- Missing `.ombra` section
- Wrong magic values
- Failed memory allocation
- Failed symbol resolution

---

## Risk Areas

1. **LDR_GET_SYMBOL availability** - May not work on all Ld9BoxSup versions. Fallback: manual export walking.

2. **IPI at DISPATCH_LEVEL** - `KeIpiGenericCall` runs at HIGH_LEVEL. VMX operations should be fine, but verify no blocking calls.

3. **SUPDrv callback convention** - We're ignoring the callback parameter. If SUPDrv passes something we need, this breaks.

4. **VMLAUNCH timing** - All CPUs must complete VMLAUNCH before any returns. Current design handles this via IPI synchronization.

5. **Partial virtualization** - If some CPUs fail, system is in mixed state. May need to abort and clean up.
