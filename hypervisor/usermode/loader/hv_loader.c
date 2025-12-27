// hv_loader.c — Hypervisor Loader Implementation
// OmbraHypervisor - Phase 1 Loader

#include "hv_loader.h"
#include "pe_utils.h"
#include "../byovd/supdrv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// OMBRA_BOOTSTRAP defined in shared/types.h (included via hv_loader.h)

// =============================================================================
// Kernel Symbols
// =============================================================================

typedef struct _KERNEL_SYMBOLS {
    U64     KeIpiGenericCall;
    U64     KeQueryActiveProcessorCountEx;
    U64     KeGetCurrentProcessorNumberEx;
} KERNEL_SYMBOLS;

// =============================================================================
// Symbol Resolution
// =============================================================================

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

// =============================================================================
// Memory Allocation
// =============================================================================

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

// =============================================================================
// Structure Preparation
// =============================================================================

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

// =============================================================================
// Build Init Params
// =============================================================================

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

// =============================================================================
// Patch Bootstrap Section
// =============================================================================

static BOOL PatchBootstrapSection(void* hvImage, U32 hvImageSize, U64 paramsR0) {
    uint32_t sectionOffset, sectionSize;

    // Validate PE
    if (!PeValidate(hvImage, hvImageSize)) {
        printf("[-] Invalid PE image\n");
        return FALSE;
    }

    // Find .ombra section
    if (!PeFindSection(hvImage, hvImageSize, ".ombra", &sectionOffset, &sectionSize)) {
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

// =============================================================================
// Load Hypervisor Module
// =============================================================================

static BOOL LoadHypervisorModule(HV_LOADER_CTX* ctx, void* hvImage, U32 hvImageSize) {
    // Get entry point RVA
    uint32_t entryRva;
    if (!PeGetEntryPoint(hvImage, hvImageSize, &entryRva)) {
        printf("[-] Failed to get entry point from PE\n");
        return FALSE;
    }
    printf("[*] Entry point RVA: 0x%X\n", entryRva);

    // LDR_OPEN: request kernel memory for image
    void* imageBase = NULL;
    DRV_STATUS status = DrvLdrOpen(&ctx->Driver, hvImageSize, &imageBase);
    if (status != DRV_SUCCESS) {
        printf("[-] LDR_OPEN failed: %s\n", DrvStatusString(status));
        return FALSE;
    }
    printf("[+] LDR_OPEN: imageBase = 0x%p\n", imageBase);

    ctx->ImageBase = imageBase;
    ctx->ImageSize = hvImageSize;

    // Calculate entry point kernel address
    void* entryPoint = (U8*)imageBase + entryRva;
    printf("[*] Entry point @ 0x%p\n", entryPoint);

    // LDR_LOAD: copy image and invoke entry point
    status = DrvLdrLoad(&ctx->Driver, imageBase, hvImage, hvImageSize, entryPoint);
    if (status != DRV_SUCCESS) {
        printf("[-] LDR_LOAD failed: %s\n", DrvStatusString(status));
        DrvLdrFree(&ctx->Driver, imageBase);
        ctx->ImageBase = NULL;
        return FALSE;
    }
    printf("[+] LDR_LOAD: hypervisor loaded and entry point called\n");

    return TRUE;
}

// =============================================================================
// Public API Implementation
// =============================================================================

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

BOOL HvLoaderUnload(HV_LOADER_CTX* ctx) {
    if (!ctx->Loaded) {
        return FALSE;
    }

    // TODO: Issue VMCALL_UNLOAD to each CPU to gracefully exit VMX
    printf("[*] Unloading hypervisor...\n");

    ctx->Running = FALSE;

    // Free the loaded kernel module
    if (ctx->ImageBase) {
        DRV_STATUS status = DrvLdrFree(&ctx->Driver, ctx->ImageBase);
        if (status != DRV_SUCCESS) {
            printf("[!] Warning: DrvLdrFree failed: %s\n", DrvStatusString(status));
            // Continue cleanup even if this fails
        }
        ctx->ImageBase = NULL;
        ctx->ImageSize = 0;
    }

    ctx->Loaded = FALSE;

    return TRUE;
}

BOOL HvLoaderIsRunning(HV_LOADER_CTX* ctx) {
    return ctx->Running;
}

void HvLoaderCleanup(HV_LOADER_CTX* ctx) {
    if (ctx->Running) {
        HvLoaderUnload(ctx);
    }

    FreeHypervisorMemory(ctx);
    DrvCleanup(&ctx->Driver);

    memset(ctx, 0, sizeof(*ctx));
}

void* HvLoaderGetDebugBuffer(HV_LOADER_CTX* ctx) {
    return ctx->Memory.DebugBuffer.R3;
}

U64 HvLoaderGetDebugBufferSize(HV_LOADER_CTX* ctx) {
    return DEBUG_BUFFER_PAGES * 0x1000;
}
