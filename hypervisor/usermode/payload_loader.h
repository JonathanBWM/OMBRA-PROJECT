// payload_loader.h — Hypervisor Payload Loader
// OmbraHypervisor

#ifndef PAYLOAD_LOADER_H
#define PAYLOAD_LOADER_H

#include "driver_interface.h"

// =============================================================================
// Per-CPU Structure (mirrors hypervisor's VMX_CPU)
// =============================================================================

typedef struct _CPU_CONTEXT {
    uint32_t    CpuId;
    bool        Virtualized;

    // VMXON Region (4KB, 4KB-aligned, contiguous)
    ALLOC_INFO  VmxonRegion;

    // VMCS Region (4KB, 4KB-aligned, contiguous)
    ALLOC_INFO  VmcsRegion;

    // Host Stack (8KB minimum, contiguous)
    ALLOC_INFO  HostStack;

    // MSR Bitmap (4KB, 4KB-aligned, contiguous)
    ALLOC_INFO  MsrBitmap;

    // IO Bitmap A+B (4KB each, contiguous)
    ALLOC_INFO  IoBitmapA;
    ALLOC_INFO  IoBitmapB;
} CPU_CONTEXT;

// =============================================================================
// Hypervisor Context
// =============================================================================

#define MAX_CPUS 256
#define HOST_STACK_PAGES 2      // 8KB
#define EPT_TABLES_PAGES 512    // 2MB for identity map
#define DEBUG_BUFFER_PAGES 16   // 64KB for debug ring buffer

typedef struct _HV_CONTEXT {
    DRV_CONTEXT     Driver;
    uint32_t        NumCpus;
    CPU_CONTEXT     Cpus[MAX_CPUS];

    // Hypervisor code region
    ALLOC_INFO      HypervisorCode;

    // EPT tables (shared across CPUs)
    ALLOC_INFO      EptTables;

    // Debug ring buffer (shared with usermode)
    ALLOC_INFO      DebugBuffer;

    // Entry point in kernel space
    void*           EntryPoint;

    // State
    bool            Loaded;
    bool            Running;
} HV_CONTEXT;

// =============================================================================
// Function Declarations
// =============================================================================

// Load and start hypervisor
bool HvLoad(HV_CONTEXT* ctx, const wchar_t* driverPath, const void* payload, size_t payloadSize);

// Stop and unload hypervisor
bool HvUnload(HV_CONTEXT* ctx);

// Check if hypervisor is running (via VMCALL)
bool HvIsRunning(HV_CONTEXT* ctx);

// Internal helpers
bool HvAllocatePerCpu(HV_CONTEXT* ctx);
bool HvFreePerCpu(HV_CONTEXT* ctx);
bool HvAllocateEpt(HV_CONTEXT* ctx);
bool HvAllocateDebugBuffer(HV_CONTEXT* ctx);
bool HvCopyPayload(HV_CONTEXT* ctx, const void* payload, size_t size);
bool HvLaunchOnCpu(HV_CONTEXT* ctx, uint32_t cpuId);
bool HvLaunchAll(HV_CONTEXT* ctx);

// Debug buffer access
void* HvGetDebugBuffer(HV_CONTEXT* ctx);
size_t HvGetDebugBufferSize(HV_CONTEXT* ctx);

#endif // PAYLOAD_LOADER_H
