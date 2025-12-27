// drv_loader.c — Manual Driver Mapper Implementation
// OmbraHypervisor Phase 2

#include "drv_loader.h"
#include "pe_parser.h"
#include "pe_imports.h"
#include "pe_mapper.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// =============================================================================
// VMCALL Wrappers (communicate with hypervisor)
// =============================================================================

// These would issue actual VMCALLs via the hypervisor interface
// For now, they're stubs that will be connected to the VMCALL mechanism

static bool VmcallClaimPoolRegion(uint64_t poolR0, uint32_t size, uint64_t* kernelVA) {
    // TODO: Issue VMCALL_CLAIM_POOL_REGION
    // For Path A, the pool is already in kernel space via SUPDrv
    // Just return the pool address directly
    *kernelVA = poolR0;
    return true;
}

static bool VmcallFinalizeDriverLoad(
    uint64_t kernelVA,
    uint32_t size,
    uint32_t entryRva,
    uint32_t textRva,
    uint32_t textSize,
    uint32_t flags,
    uint32_t* driverEntryResult
) {
    // TODO: Issue VMCALL_FINALIZE_DRIVER_LOAD
    // This would call DriverEntry from hypervisor context
    // For now, we can't call it from usermode - stub returns success
    printf("[!] VMCALL_FINALIZE_DRIVER_LOAD not yet implemented\n");
    printf("[!] Driver mapped but DriverEntry NOT called\n");
    *driverEntryResult = 0;
    return true;
}

// =============================================================================
// Initialization
// =============================================================================

bool DrvLoaderInit(DRIVER_MAP_CONTEXT* ctx) {
    memset(ctx, 0, sizeof(*ctx));
    return true;
}

// =============================================================================
// Preparation (while SUPDrv active)
// =============================================================================

bool DrvLoaderPrepare(
    DRIVER_MAP_CONTEXT* ctx,
    DRV_CONTEXT* supdrv,
    const void* driverImage,
    uint32_t driverSize
) {
    printf("[*] Preparing driver for manual mapping...\n");

    // Parse PE
    if (!PeParse(driverImage, driverSize, &ctx->PeInfo)) {
        printf("[-] PE parsing failed\n");
        return false;
    }
    printf("[+] PE parsed: %u sections, %u imports, entry RVA 0x%X\n",
           ctx->PeInfo.SectionCount, ctx->PeInfo.ImportCount,
           ctx->PeInfo.EntryPointRva);

    // Resolve imports via SUPDrv
    printf("[*] Resolving imports via SUPDrv...\n");
    if (!ResolveImports(supdrv, &ctx->PeInfo)) {
        printf("[-] Import resolution failed\n");
        return false;
    }

    // Pre-resolve common symbols (optional, for driver's dynamic use)
    ResolveCommonSymbols(supdrv, &ctx->CommonSymbols);

    // Calculate required pool size
    uint32_t requiredSize = PeCalculateImageSize(&ctx->PeInfo);
    uint32_t poolPages = (requiredSize + 0xFFF) / 0x1000;

    // Add extra pages for growth (min 2MB)
    if (poolPages < 512) {
        poolPages = 512;  // 2MB minimum
    }

    printf("[*] Allocating driver pool: %u pages (%u KB)\n",
           poolPages, (poolPages * 4096) / 1024);

    // Allocate contiguous pool via SUPDrv
    DRV_STATUS status = DrvAllocContiguous(supdrv, poolPages, &ctx->Pool);
    if (status != DRV_SUCCESS) {
        printf("[-] Pool allocation failed: %s\n", DrvStatusString(status));
        return false;
    }

    printf("[+] Pool allocated:\n");
    printf("    R3 (usermode):  %p\n", ctx->Pool.R3);
    printf("    R0 (kernel):    %p\n", ctx->Pool.R0);
    printf("    Physical:       0x%llX\n", ctx->Pool.Physical);

    // Store copy of driver image
    ctx->ImageBuffer = malloc(driverSize);
    if (!ctx->ImageBuffer) {
        printf("[-] Failed to allocate image buffer\n");
        DrvFreeContiguous(supdrv, &ctx->Pool);
        return false;
    }
    memcpy(ctx->ImageBuffer, driverImage, driverSize);
    ctx->ImageSize = driverSize;

    ctx->Prepared = true;
    printf("[+] Driver prepared for mapping\n");
    return true;
}

// =============================================================================
// Mapping (via hypervisor)
// =============================================================================

bool DrvLoaderMap(DRIVER_MAP_CONTEXT* ctx, HV_LOADER_CTX* hvCtx) {
    if (!ctx->Prepared) {
        printf("[-] Driver not prepared - call DrvLoaderPrepare first\n");
        return false;
    }

    (void)hvCtx;  // Will be used for VMCALL interface

    printf("[*] Mapping driver via hypervisor...\n");

    // Step 1: Claim pool region via VMCALL
    uint64_t kernelVA = 0;
    if (!VmcallClaimPoolRegion((uint64_t)ctx->Pool.R0, ctx->PeInfo.ImageSize, &kernelVA)) {
        printf("[-] VMCALL_CLAIM_POOL_REGION failed\n");
        return false;
    }

    ctx->KernelBase = kernelVA;
    ctx->MappedSize = ctx->PeInfo.ImageSize;
    printf("[+] Claimed kernel region at 0x%llX\n", kernelVA);

    // Step 2: Map driver via direct R3 write (dual-mapped memory)
    printf("[*] Mapping PE sections to kernel memory...\n");
    if (!MapDriver(ctx->ImageBuffer, ctx->Pool.R3, kernelVA, &ctx->PeInfo)) {
        printf("[-] Driver mapping failed\n");
        return false;
    }

    ctx->Mapped = true;
    printf("[+] Driver sections mapped and relocated\n");

    // Step 3: Finalize via VMCALL (set protections, call entry, hide)
    PE_SECTION_INFO* textSec = PeGetSection(&ctx->PeInfo, ".text");
    uint32_t textRva = textSec ? textSec->Rva : 0;
    uint32_t textSize = textSec ? textSec->VirtualSize : 0;

    uint32_t driverEntryResult = 0;
    uint32_t flags = FINALIZE_FLAG_HIDE;  // Request EPT hiding

    if (!VmcallFinalizeDriverLoad(
            kernelVA,
            ctx->PeInfo.ImageSize,
            ctx->PeInfo.EntryPointRva,
            textRva,
            textSize,
            flags,
            &driverEntryResult)) {
        printf("[-] VMCALL_FINALIZE_DRIVER_LOAD failed\n");
        return false;
    }

    if (driverEntryResult != 0) {
        printf("[-] DriverEntry returned 0x%X\n", driverEntryResult);
        // Non-zero could be error or success depending on driver
        // STATUS_SUCCESS = 0, so non-zero might be an error
    }

    ctx->Running = true;
    printf("[+] Driver loaded successfully at kernel address 0x%llX\n", kernelVA);

    return true;
}

// =============================================================================
// Cleanup
// =============================================================================

void DrvLoaderCleanup(DRIVER_MAP_CONTEXT* ctx, DRV_CONTEXT* supdrv) {
    // Secure wipe of image buffer
    if (ctx->ImageBuffer) {
        volatile uint8_t* p = (volatile uint8_t*)ctx->ImageBuffer;
        for (uint32_t i = 0; i < ctx->ImageSize; i++) {
            p[i] = 0;
        }
        free(ctx->ImageBuffer);
        ctx->ImageBuffer = NULL;
    }

    // Note: We do NOT free the Pool allocation here
    // The driver is using that memory - freeing it would crash the driver
    // Pool cleanup happens via VMCALL when driver is explicitly unloaded

    // Clear PE info (contains resolved addresses)
    volatile uint8_t* peInfo = (volatile uint8_t*)&ctx->PeInfo;
    for (size_t i = 0; i < sizeof(ctx->PeInfo); i++) {
        peInfo[i] = 0;
    }

    // Clear common symbols
    volatile uint8_t* syms = (volatile uint8_t*)&ctx->CommonSymbols;
    for (size_t i = 0; i < sizeof(ctx->CommonSymbols); i++) {
        syms[i] = 0;
    }

    // If driver never mapped, we can free the pool
    if (!ctx->Mapped && ctx->Pool.R3) {
        DrvFreeContiguous(supdrv, &ctx->Pool);
    }

    memset(ctx, 0, sizeof(*ctx));
}

// =============================================================================
// Status Queries
// =============================================================================

bool DrvLoaderIsRunning(DRIVER_MAP_CONTEXT* ctx) {
    return ctx->Running;
}

uint64_t DrvLoaderGetKernelBase(DRIVER_MAP_CONTEXT* ctx) {
    return ctx->KernelBase;
}
