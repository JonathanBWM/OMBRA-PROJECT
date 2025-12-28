#ifndef HV_LOADER_H
#define HV_LOADER_H

#include "../driver_interface.h"
#include "../../shared/types.h"

// Configuration
#define HOST_STACK_SIZE     0x4000      // 16KB per CPU
#define HOST_STACK_PAGES    (HOST_STACK_SIZE / 0x1000)
#define EPT_TABLES_PAGES    512         // 2MB for identity mapping
#define DEBUG_BUFFER_PAGES  16          // 64KB debug ring buffer

// Memory layout tracking
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

    // Self-protection
    ALLOC_INFO  BlankPage;          // 1 page, zeroed for memory hiding

    // Debug (non-paged OK)
    ALLOC_INFO  DebugBuffer;        // DEBUG_BUFFER_PAGES pages
} HV_MEMORY_LAYOUT;

// Context for loaded hypervisor
typedef struct _HV_LOADER_CTX {
    DRV_CONTEXT         Driver;
    U32                 CpuCount;
    HV_MEMORY_LAYOUT    Memory;

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
