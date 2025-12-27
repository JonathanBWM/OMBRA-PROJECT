// drv_loader.h — Manual Driver Mapper API
// OmbraHypervisor Phase 2

#ifndef DRV_LOADER_H
#define DRV_LOADER_H

#include <stdint.h>
#include <stdbool.h>
#include <Windows.h>
#include "pe_parser.h"
#include "pe_imports.h"
#include "hv_loader.h"
#include "../driver_interface.h"

// =============================================================================
// Driver Mapping Context
// =============================================================================

typedef struct _DRIVER_MAP_CONTEXT {
    // PE parsing results
    PE_INFO         PeInfo;

    // Image buffer (copy of original PE)
    void*           ImageBuffer;
    uint32_t        ImageSize;

    // Pool allocation from SUPDrv
    ALLOC_INFO      Pool;

    // Mapped driver info
    uint64_t        KernelBase;         // R0 address after mapping
    uint32_t        MappedSize;

    // Pre-resolved common symbols
    COMMON_SYMBOLS  CommonSymbols;

    // State flags
    bool            Prepared;           // PE parsed, imports resolved, pool allocated
    bool            Mapped;             // Driver sections mapped to kernel
    bool            Running;            // DriverEntry called successfully

    // Path selection (based on BigPool test)
    bool            UseMitigationPath;  // Path B if BigPool visible
} DRIVER_MAP_CONTEXT;

// =============================================================================
// API Functions
// =============================================================================

// Initialize empty context
bool DrvLoaderInit(DRIVER_MAP_CONTEXT* ctx);

// Prepare driver for mapping (parse PE, resolve imports, allocate pool)
// Must be called while SUPDrv is still active
// driverImage: raw PE file contents
// driverSize: size of PE file
bool DrvLoaderPrepare(
    DRIVER_MAP_CONTEXT* ctx,
    DRV_CONTEXT* supdrv,
    const void* driverImage,
    uint32_t driverSize
);

// Map driver into kernel via hypervisor VMCALLs
// Must be called after hypervisor is active (Phase 1 complete)
bool DrvLoaderMap(DRIVER_MAP_CONTEXT* ctx, HV_LOADER_CTX* hvCtx);

// Cleanup context (secure wipe of sensitive data)
// Does NOT unload the driver - that requires VMCALL
void DrvLoaderCleanup(DRIVER_MAP_CONTEXT* ctx, DRV_CONTEXT* supdrv);

// Check if driver is running
bool DrvLoaderIsRunning(DRIVER_MAP_CONTEXT* ctx);

// Get driver kernel base address
uint64_t DrvLoaderGetKernelBase(DRIVER_MAP_CONTEXT* ctx);

#endif // DRV_LOADER_H
