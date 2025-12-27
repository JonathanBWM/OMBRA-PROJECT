// vmcall_phase2.c — Phase 2 VMCALL Handlers (Driver Mapper)
// OmbraHypervisor

#include "vmcall_phase2.h"
#include "handlers.h"
#include "../ept.h"

// =============================================================================
// Pool Region Tracking
// =============================================================================

typedef struct _POOL_REGION {
    U64     KernelVA;
    U64     PhysicalAddr;
    U32     Size;
    bool    InUse;
} POOL_REGION;

#define MAX_POOL_REGIONS 16
static POOL_REGION g_PoolRegions[MAX_POOL_REGIONS] = {0};
static U64 g_PoolBase = 0;
static U32 g_PoolSize = 0;
static U32 g_PoolOffset = 0;

// =============================================================================
// Initialization
// =============================================================================

void Phase2_InitPool(U64 poolBase, U64 poolSize) {
    g_PoolBase = poolBase;
    g_PoolSize = (U32)poolSize;
    g_PoolOffset = 0;

    for (int i = 0; i < MAX_POOL_REGIONS; i++) {
        g_PoolRegions[i].InUse = false;
    }
}

// =============================================================================
// VMCALL Handlers
// =============================================================================

// VMCALL_CLAIM_POOL_REGION - Claim a region from pre-allocated pool
static OMBRA_STATUS HandleClaimPoolRegion(VMCALL_CLAIM_POOL_IN* in, VMCALL_CLAIM_POOL_OUT* out) {
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
    region->InUse = true;

    out->KernelVA = region->KernelVA;
    out->Status = OMBRA_SUCCESS;

    g_PoolOffset += alignedSize;

    return OMBRA_SUCCESS;
}

// VMCALL_RELEASE_POOL_REGION - Release a claimed region
static OMBRA_STATUS HandleReleasePoolRegion(U64 kernelVA) {
    for (int i = 0; i < MAX_POOL_REGIONS; i++) {
        if (g_PoolRegions[i].InUse && g_PoolRegions[i].KernelVA == kernelVA) {
            g_PoolRegions[i].InUse = false;
            return OMBRA_SUCCESS;
        }
    }
    return OMBRA_ERROR_NOT_FOUND;
}

// VMCALL_FINALIZE_DRIVER_LOAD - Set protections, call entry, optionally hide
static OMBRA_STATUS HandleFinalizeDriverLoad(VMCALL_FINALIZE_IN* in, VMCALL_FINALIZE_OUT* out) {
    // Note: EPT permission changes would require page splitting
    // For now, we just track the regions and call DriverEntry

    U64 textStart = in->KernelVA + in->TextRva;
    (void)textStart;  // Would use for EPT RX permissions

    // Call DriverEntry unless NO_ENTRY flag set
    if (!(in->Flags & FINALIZE_FLAG_NO_ENTRY)) {
        typedef long (*FN_DriverEntry)(void*, void*);
        FN_DriverEntry entry = (FN_DriverEntry)(in->KernelVA + in->EntryRva);

        // Call with NULL parameters (no driver object for manual map)
        long ntStatus = entry(NULL, NULL);

        out->DriverEntryResult = (U32)ntStatus;

        if (ntStatus < 0) {
            out->Status = OMBRA_ERROR_DRIVER_INIT_FAILED;
            return OMBRA_ERROR_DRIVER_INIT_FAILED;
        }
    } else {
        out->DriverEntryResult = 0;
    }

    // EPT hiding would be configured here if FINALIZE_FLAG_HIDE is set
    // This makes pages execute-only, causing reads/writes to fault

    out->Status = OMBRA_SUCCESS;
    return OMBRA_SUCCESS;
}

// VMCALL_HIDE_MEMORY_RANGE - Configure EPT to hide memory from kernel
static OMBRA_STATUS HandleHideMemoryRange(VMCALL_HIDE_RANGE_IN* in) {
    // Configure EPT to make memory execute-only
    // Reads/writes from kernel will cause EPT violation
    // Handler can return zeros or inject #PF

    U64 numPages = (in->Size + 0xFFF) / 0x1000;

    for (U64 i = 0; i < numPages; i++) {
        U64 pageAddr = in->Address + (i * 0x1000);
        // Execute-only: code can run but reads/writes fault
        // EptSetPagePermissions(pageAddr, EPT_EXECUTE);
        (void)pageAddr;  // Placeholder until EPT integration
    }

    return OMBRA_SUCCESS;
}

// VMCALL_UNHIDE_MEMORY_RANGE - Restore normal EPT permissions
static OMBRA_STATUS HandleUnhideMemoryRange(VMCALL_HIDE_RANGE_IN* in) {
    U64 numPages = (in->Size + 0xFFF) / 0x1000;

    for (U64 i = 0; i < numPages; i++) {
        U64 pageAddr = in->Address + (i * 0x1000);
        // Restore RWX permissions
        // EptSetPagePermissions(pageAddr, EPT_READ | EPT_WRITE | EPT_EXECUTE);
        (void)pageAddr;
    }

    return OMBRA_SUCCESS;
}

// VMCALL_DRIVER_PING - Check if driver is responding
static OMBRA_STATUS HandleDriverPing(U32* response) {
    *response = 0x504F4E47;  // 'PONG'
    return OMBRA_SUCCESS;
}

// =============================================================================
// Main Dispatch
// =============================================================================

OMBRA_STATUS Phase2_HandleVmcall(U64 vmcallId, void* inputBuf, void* outputBuf) {
    switch (vmcallId) {
        case VMCALL_CLAIM_POOL_REGION:
            return HandleClaimPoolRegion(
                (VMCALL_CLAIM_POOL_IN*)inputBuf,
                (VMCALL_CLAIM_POOL_OUT*)outputBuf
            );

        case VMCALL_RELEASE_POOL_REGION: {
            U64* va = (U64*)inputBuf;
            return HandleReleasePoolRegion(*va);
        }

        case VMCALL_FINALIZE_DRIVER_LOAD:
            return HandleFinalizeDriverLoad(
                (VMCALL_FINALIZE_IN*)inputBuf,
                (VMCALL_FINALIZE_OUT*)outputBuf
            );

        case VMCALL_DRIVER_PING:
            return HandleDriverPing((U32*)outputBuf);

        case VMCALL_HIDE_MEMORY_RANGE:
            return HandleHideMemoryRange((VMCALL_HIDE_RANGE_IN*)inputBuf);

        case VMCALL_UNHIDE_MEMORY_RANGE:
            return HandleUnhideMemoryRange((VMCALL_HIDE_RANGE_IN*)inputBuf);

        // Path B handlers (MDL allocation) - stubbed for now
        case VMCALL_ALLOC_MDL_MEMORY:
        case VMCALL_POLL_ALLOC_STATUS:
        case VMCALL_FREE_MDL_MEMORY:
            return OMBRA_ERROR_NOT_IMPLEMENTED;

        default:
            return OMBRA_ERROR_INVALID_OPERATION;
    }
}
