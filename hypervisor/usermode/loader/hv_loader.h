#ifndef HV_LOADER_H
#define HV_LOADER_H

#include "../driver_interface.h"
#include "../../shared/types.h"

// =============================================================================
// Hypervisor Loader - Self-Contained Design
// =============================================================================
//
// With the self-contained hypervisor architecture, the loader is simplified:
// 1. Load hypervisor PE into kernel memory via BYOVD
// 2. Resolve MmGetSystemRoutineAddress
// 3. Call HvEntry(MmGetSystemRoutineAddress)
//
// The hypervisor then:
// - Resolves all other kernel symbols at runtime
// - Allocates VMX structures (VMXON, VMCS, stacks, EPT, etc.)
// - Initializes VMX on all CPUs via IPI broadcast
//
// This eliminates the need for HV_MEMORY_LAYOUT and complex parameter building
// in the loader, making the system more robust and easier to debug.

// Context for loaded hypervisor
typedef struct _HV_LOADER_CTX {
    DRV_CONTEXT         Driver;         // BYOVD driver context
    U32                 CpuCount;       // CPU count (for display)

    // Hypervisor module
    void*               ImageBase;      // Kernel address from LDR_OPEN
    U32                 ImageSize;      // Size of mapped image

    // State
    BOOL                Loaded;         // HvEntry called successfully
    BOOL                Running;        // Hypervisor is active
} HV_LOADER_CTX;

// =============================================================================
// Public API
// =============================================================================

// Initialize the loader context.
// driverPath: Path to BYOVD driver (Ld9BoxSup.sys) or NULL for embedded
BOOL HvLoaderInit(HV_LOADER_CTX* ctx, const wchar_t* driverPath);

// Load and start the hypervisor.
// hvImage: Pointer to hypervisor.sys PE file in memory
// hvImageSize: Size of the PE file
BOOL HvLoaderLoad(HV_LOADER_CTX* ctx, const void* hvImage, U32 hvImageSize);

// Unload the hypervisor (issues VMCALL_UNLOAD).
BOOL HvLoaderUnload(HV_LOADER_CTX* ctx);

// Check if hypervisor is running.
BOOL HvLoaderIsRunning(HV_LOADER_CTX* ctx);

// Cleanup all resources.
void HvLoaderCleanup(HV_LOADER_CTX* ctx);

// =============================================================================
// Debug Buffer Access (deprecated with self-contained design)
// =============================================================================
//
// With self-contained design, the hypervisor allocates its own debug buffer.
// Debug output is accessed via VMCALL interface, not direct memory mapping.
// These functions return NULL/0 for compatibility.

void* HvLoaderGetDebugBuffer(HV_LOADER_CTX* ctx);
U64   HvLoaderGetDebugBufferSize(HV_LOADER_CTX* ctx);

#endif // HV_LOADER_H
