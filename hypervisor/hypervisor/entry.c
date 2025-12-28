// entry.c - Hypervisor Entry Point (Self-Contained Design)
// OmbraHypervisor
//
// Self-contained initialization following memhv pattern.
// Resolves kernel symbols at runtime, allocates VMX resources internally.
// Loader only needs to provide MmGetSystemRoutineAddress and call HvEntry.

#include "../shared/types.h"
#include "../shared/vmcs_fields.h"
#include "../shared/msr_defs.h"
#include "../shared/cpu_defs.h"
#include "vmx.h"
#include "vmcs.h"
#include "ept.h"
#include "hooks.h"
#include "debug.h"
#include "kernel_resolve.h"
#include "self_info.h"
#include "mdl_alloc.h"
#include "relocation.h"
#include <intrin.h>

// =============================================================================
// Configuration Constants
// =============================================================================

#define HV_HOST_STACK_SIZE      0x8000      // 32KB per-CPU stack
#define HV_DEBUG_BUFFER_SIZE    0x10000     // 64KB debug buffer
#define HV_EPT_PAGES            512         // EPT table pages
#define HV_MAX_CPUS             256
#define HV_SPLIT_POOL_COUNT     32          // Pre-allocated pools for EPT splitting (memhv pattern)
#define HV_SPLIT_POOL_SIZE      0x1000      // 4KB per split pool

// =============================================================================
// Global State
// =============================================================================

static VMX_CPU g_CpuContexts[HV_MAX_CPUS] = {0};
static EPT_STATE g_EptState = {0};
static volatile U32 g_SuccessCount = 0;
static volatile U32 g_FailCount = 0;
static bool g_Initialized = false;
static bool g_UseMdlAllocation = true;      // Use MDL-based stealth allocation
static bool g_Phase2Complete = false;        // Set after Phase 2 relocation completes

// External hook manager
extern HOOK_MANAGER g_HookManager;

// =============================================================================
// Allocated Resources (self-managed)
// =============================================================================

typedef struct _HV_RESOURCES {
    // Per-CPU regions (contiguous arrays)
    void*   VmxonRegions;           // CpuCount * 4KB
    U64     VmxonRegionsPhys;
    void*   VmcsRegions;            // CpuCount * 4KB
    U64     VmcsRegionsPhys;
    void*   HostStacks;             // CpuCount * HV_HOST_STACK_SIZE

    // Shared resources
    void*   MsrBitmap;              // 4KB
    U64     MsrBitmapPhys;
    void*   EptTables;              // HV_EPT_PAGES * 4KB
    U64     EptTablesPhys;
    void*   DebugBuffer;            // HV_DEBUG_BUFFER_SIZE
    void*   BlankPage;              // 4KB (for EPT self-protection)
    U64     BlankPagePhys;

    // VMX capability MSRs (read at init time)
    U64     VmxBasic;
    U64     VmxPinbasedCtls;
    U64     VmxProcbasedCtls;
    U64     VmxProcbasedCtls2;
    U64     VmxExitCtls;
    U64     VmxEntryCtls;
    U64     VmxTruePinbasedCtls;
    U64     VmxTrueProcbasedCtls;
    U64     VmxTrueExitCtls;
    U64     VmxTrueEntryCtls;
    U64     VmxCr0Fixed0;
    U64     VmxCr0Fixed1;
    U64     VmxCr4Fixed0;
    U64     VmxCr4Fixed1;
    U64     VmxEptVpidCap;

    // Counts
    U32     CpuCount;
    bool    Allocated;

    // Pre-allocated split pools (memhv pattern)
    // Used for on-demand EPT page table allocation during runtime
    // Avoids calling kernel allocator during EPT violation handling
    void*   SplitPools[HV_SPLIT_POOL_COUNT];
    U64     SplitPoolsPhys[HV_SPLIT_POOL_COUNT];
    volatile U32 SplitPoolIndex;
    U32     SplitPoolCount;

    // Runtime kernel offsets (cross-version compatibility)
    struct {
        U64     UniqueProcessId;        // EPROCESS.UniqueProcessId offset
        U64     ActiveProcessLinks;     // EPROCESS.ActiveProcessLinks offset
        U64     DirectoryTableBase;     // EPROCESS.DirectoryTableBase offset
        U64     ImageFileName;          // EPROCESS.ImageFileName offset
        bool    Discovered;
    } Offsets;
} HV_RESOURCES;

static HV_RESOURCES g_Resources = {0};

// =============================================================================
// Read VMX Capability MSRs
// =============================================================================

static void ReadVmxCapabilities(void) {
    g_Resources.VmxBasic = __readmsr(MSR_IA32_VMX_BASIC);
    g_Resources.VmxPinbasedCtls = __readmsr(MSR_IA32_VMX_PINBASED_CTLS);
    g_Resources.VmxProcbasedCtls = __readmsr(MSR_IA32_VMX_PROCBASED_CTLS);
    g_Resources.VmxExitCtls = __readmsr(MSR_IA32_VMX_EXIT_CTLS);
    g_Resources.VmxEntryCtls = __readmsr(MSR_IA32_VMX_ENTRY_CTLS);

    // Read true controls if supported (bit 55 of VMX_BASIC)
    if (g_Resources.VmxBasic & (1ULL << 55)) {
        g_Resources.VmxTruePinbasedCtls = __readmsr(MSR_IA32_VMX_TRUE_PINBASED);
        g_Resources.VmxTrueProcbasedCtls = __readmsr(MSR_IA32_VMX_TRUE_PROCBASED);
        g_Resources.VmxTrueExitCtls = __readmsr(MSR_IA32_VMX_TRUE_EXIT);
        g_Resources.VmxTrueEntryCtls = __readmsr(MSR_IA32_VMX_TRUE_ENTRY);
    } else {
        g_Resources.VmxTruePinbasedCtls = g_Resources.VmxPinbasedCtls;
        g_Resources.VmxTrueProcbasedCtls = g_Resources.VmxProcbasedCtls;
        g_Resources.VmxTrueExitCtls = g_Resources.VmxExitCtls;
        g_Resources.VmxTrueEntryCtls = g_Resources.VmxEntryCtls;
    }

    // Secondary controls
    U32 procCtls = (U32)(g_Resources.VmxProcbasedCtls >> 32);
    if (procCtls & (1 << 31)) {  // Activate secondary controls
        g_Resources.VmxProcbasedCtls2 = __readmsr(MSR_IA32_VMX_PROCBASED_CTLS2);
    }

    // EPT/VPID capabilities
    U32 procCtls2 = (U32)(g_Resources.VmxProcbasedCtls2 >> 32);
    if ((procCtls2 & (1 << 1)) || (procCtls2 & (1 << 5))) {  // EPT or VPID
        g_Resources.VmxEptVpidCap = __readmsr(MSR_IA32_VMX_EPT_VPID_CAP);
    }

    // CR fixed bits
    g_Resources.VmxCr0Fixed0 = __readmsr(MSR_IA32_VMX_CR0_FIXED0);
    g_Resources.VmxCr0Fixed1 = __readmsr(MSR_IA32_VMX_CR0_FIXED1);
    g_Resources.VmxCr4Fixed0 = __readmsr(MSR_IA32_VMX_CR4_FIXED0);
    g_Resources.VmxCr4Fixed1 = __readmsr(MSR_IA32_VMX_CR4_FIXED1);
}

// =============================================================================
// Zero memory helper (no CRT)
// =============================================================================

static void ZeroMem(void* dst, U64 size) {
    U8* p = (U8*)dst;
    while (size--) *p++ = 0;
}

// =============================================================================
// Allocate VMX Resources Using MDL Allocator (Stealth Mode)
// =============================================================================
// Uses MdlAlloc* functions which do NOT create BigPool entries.
// Called in Phase 2 after self-relocation is complete.

static OMBRA_STATUS AllocateResourcesMdl(void) {
    U32 cpuCount;
    MDL_ALLOCATOR* alloc = g_MdlAllocator;

    if (!alloc || !alloc->Initialized) {
        return OMBRA_ERROR_NOT_INITIALIZED;
    }

    if (g_Resources.Allocated) {
        return OMBRA_SUCCESS;
    }

    // Get CPU count
    cpuCount = KernelGetProcessorCount();
    if (cpuCount == 0 || cpuCount > HV_MAX_CPUS) {
        return OMBRA_ERROR_INVALID_PARAM;
    }
    g_Resources.CpuCount = cpuCount;

    // Allocate VMXON regions from MDL (4KB aligned)
    g_Resources.VmxonRegions = MdlAllocAligned(alloc, cpuCount * 0x1000, 0x1000);
    if (!g_Resources.VmxonRegions) {
        return OMBRA_ERROR_NO_MEMORY;
    }
    ZeroMem(g_Resources.VmxonRegions, cpuCount * 0x1000);
    g_Resources.VmxonRegionsPhys = MdlGetPhysicalAddress(g_Resources.VmxonRegions);

    // Allocate VMCS regions from dedicated VMCS region
    // Each CPU gets its own VMCS from the pre-allocated VMCS pool
    g_Resources.VmcsRegions = MdlAllocAligned(alloc, cpuCount * 0x1000, 0x1000);
    if (!g_Resources.VmcsRegions) {
        return OMBRA_ERROR_NO_MEMORY;
    }
    ZeroMem(g_Resources.VmcsRegions, cpuCount * 0x1000);
    g_Resources.VmcsRegionsPhys = MdlGetPhysicalAddress(g_Resources.VmcsRegions);

    // Allocate host stacks from stack region
    U64 stackSize = cpuCount * HV_HOST_STACK_SIZE;
    g_Resources.HostStacks = MdlAllocStack(alloc, stackSize, NULL);
    if (!g_Resources.HostStacks) {
        return OMBRA_ERROR_NO_MEMORY;
    }
    ZeroMem(g_Resources.HostStacks, stackSize);

    // Allocate MSR bitmap (4KB, from VMCS or misc region)
    g_Resources.MsrBitmap = MdlAllocMsrBitmap(alloc, &g_Resources.MsrBitmapPhys);
    if (!g_Resources.MsrBitmap) {
        return OMBRA_ERROR_NO_MEMORY;
    }
    ZeroMem(g_Resources.MsrBitmap, 0x1000);

    // Allocate EPT tables from EPT region
    U64 eptSize = HV_EPT_PAGES * 0x1000;
    g_Resources.EptTables = MdlAllocEptTable(alloc, &g_Resources.EptTablesPhys);
    if (!g_Resources.EptTables) {
        return OMBRA_ERROR_NO_MEMORY;
    }
    // Allocate more EPT pages - the first one just gives us a start
    // We need to allocate additional pages for the full EPT structure
    for (U32 i = 1; i < HV_EPT_PAGES; i++) {
        U64 phys;
        void* page = MdlAllocEptTable(alloc, &phys);
        if (!page) break;
        // Pages are contiguous in the EPT region, just keep allocating
    }

    // Allocate debug buffer from misc region
    g_Resources.DebugBuffer = MdlAllocMisc(alloc, HV_DEBUG_BUFFER_SIZE);
    if (g_Resources.DebugBuffer) {
        ZeroMem(g_Resources.DebugBuffer, HV_DEBUG_BUFFER_SIZE);
    }

    // Allocate blank page for EPT hiding
    U64 blankPhys;
    g_Resources.BlankPage = MdlAllocAligned(alloc, 0x1000, 0x1000);
    if (g_Resources.BlankPage) {
        ZeroMem(g_Resources.BlankPage, 0x1000);
        g_Resources.BlankPagePhys = MdlGetPhysicalAddress(g_Resources.BlankPage);
    }

    // Pre-allocate split pools from MDL
    g_Resources.SplitPoolCount = 0;
    g_Resources.SplitPoolIndex = 0;
    for (U32 i = 0; i < HV_SPLIT_POOL_COUNT; i++) {
        g_Resources.SplitPools[i] = MdlAllocMisc(alloc, HV_SPLIT_POOL_SIZE);
        if (g_Resources.SplitPools[i]) {
            ZeroMem(g_Resources.SplitPools[i], HV_SPLIT_POOL_SIZE);
            g_Resources.SplitPoolsPhys[i] = MdlGetPhysicalAddress(g_Resources.SplitPools[i]);
            g_Resources.SplitPoolCount++;
        } else {
            break;
        }
    }

    // Write revision ID to all VMXON/VMCS regions
    U32 revisionId = (U32)(g_Resources.VmxBasic & 0x7FFFFFFF);
    for (U32 i = 0; i < cpuCount; i++) {
        U32* vmxon = (U32*)((U8*)g_Resources.VmxonRegions + (i * 0x1000));
        *vmxon = revisionId;

        U32* vmcs = (U32*)((U8*)g_Resources.VmcsRegions + (i * 0x1000));
        *vmcs = revisionId;
    }

    g_Resources.Allocated = true;
    return OMBRA_SUCCESS;
}

// =============================================================================
// Get Next Pre-allocated Split Pool (memhv pattern)
// =============================================================================
//
// Returns the next available pre-allocated pool for EPT page table allocation.
// This avoids calling kernel allocator during EPT violation handling.
// Thread-safe via interlocked increment.
//
// Returns: Virtual address of 4KB pool, or NULL if exhausted

void* GetPreallocatedPool(U64* outPhysical) {
    U32 index = (U32)_InterlockedIncrement((volatile long*)&g_Resources.SplitPoolIndex) - 1;

    if (index >= g_Resources.SplitPoolCount) {
        // Pool exhausted - caller should fall back to EPT's contiguous pool
        if (outPhysical) *outPhysical = 0;
        return NULL;
    }

    if (outPhysical) {
        *outPhysical = g_Resources.SplitPoolsPhys[index];
    }

    return g_Resources.SplitPools[index];
}

// Get count of remaining pre-allocated pools
U32 GetPreallocatedPoolsRemaining(void) {
    U32 used = g_Resources.SplitPoolIndex;
    if (used >= g_Resources.SplitPoolCount) return 0;
    return g_Resources.SplitPoolCount - used;
}

// =============================================================================
// Allocate VMX Resources
// =============================================================================

static OMBRA_STATUS AllocateResources(void) {
    U32 cpuCount;
    U64 vmxonSize, vmcsSize, stackSize, eptSize;

    if (g_Resources.Allocated) {
        return OMBRA_SUCCESS;
    }

    // Get CPU count
    cpuCount = KernelGetProcessorCount();
    if (cpuCount == 0 || cpuCount > HV_MAX_CPUS) {
        return OMBRA_ERROR_INVALID_PARAM;
    }
    g_Resources.CpuCount = cpuCount;

    // Calculate sizes
    vmxonSize = cpuCount * 0x1000;      // 4KB per CPU
    vmcsSize = cpuCount * 0x1000;       // 4KB per CPU
    stackSize = cpuCount * HV_HOST_STACK_SIZE;
    eptSize = HV_EPT_PAGES * 0x1000;

    // Allocate VMXON regions (contiguous, 4KB aligned)
    g_Resources.VmxonRegions = KernelAllocateContiguous(vmxonSize);
    if (!g_Resources.VmxonRegions) {
        return OMBRA_ERROR_NO_MEMORY;
    }
    ZeroMem(g_Resources.VmxonRegions, vmxonSize);
    g_Resources.VmxonRegionsPhys = KernelGetPhysicalAddress(g_Resources.VmxonRegions);

    // Allocate VMCS regions (contiguous, 4KB aligned)
    g_Resources.VmcsRegions = KernelAllocateContiguous(vmcsSize);
    if (!g_Resources.VmcsRegions) {
        return OMBRA_ERROR_NO_MEMORY;
    }
    ZeroMem(g_Resources.VmcsRegions, vmcsSize);
    g_Resources.VmcsRegionsPhys = KernelGetPhysicalAddress(g_Resources.VmcsRegions);

    // Allocate host stacks (contiguous for simplicity)
    g_Resources.HostStacks = KernelAllocateContiguous(stackSize);
    if (!g_Resources.HostStacks) {
        return OMBRA_ERROR_NO_MEMORY;
    }
    ZeroMem(g_Resources.HostStacks, stackSize);

    // Allocate MSR bitmap (shared, 4KB)
    g_Resources.MsrBitmap = KernelAllocateContiguous(0x1000);
    if (!g_Resources.MsrBitmap) {
        return OMBRA_ERROR_NO_MEMORY;
    }
    ZeroMem(g_Resources.MsrBitmap, 0x1000);
    g_Resources.MsrBitmapPhys = KernelGetPhysicalAddress(g_Resources.MsrBitmap);

    // Allocate EPT tables (contiguous)
    g_Resources.EptTables = KernelAllocateContiguous(eptSize);
    if (!g_Resources.EptTables) {
        return OMBRA_ERROR_NO_MEMORY;
    }
    ZeroMem(g_Resources.EptTables, eptSize);
    g_Resources.EptTablesPhys = KernelGetPhysicalAddress(g_Resources.EptTables);

    // Allocate debug buffer
    g_Resources.DebugBuffer = KernelAllocatePageAligned(HV_DEBUG_BUFFER_SIZE);
    if (!g_Resources.DebugBuffer) {
        // Debug buffer is optional - continue without it
        g_Resources.DebugBuffer = NULL;
    } else {
        ZeroMem(g_Resources.DebugBuffer, HV_DEBUG_BUFFER_SIZE);
    }

    // Allocate blank page for EPT hiding
    g_Resources.BlankPage = KernelAllocateContiguous(0x1000);
    if (g_Resources.BlankPage) {
        ZeroMem(g_Resources.BlankPage, 0x1000);
        g_Resources.BlankPagePhys = KernelGetPhysicalAddress(g_Resources.BlankPage);
    }

    // Pre-allocate split pools (memhv pattern)
    // These avoid runtime allocation during EPT violation handling
    g_Resources.SplitPoolCount = 0;
    g_Resources.SplitPoolIndex = 0;
    for (U32 i = 0; i < HV_SPLIT_POOL_COUNT; i++) {
        g_Resources.SplitPools[i] = KernelAllocateContiguous(HV_SPLIT_POOL_SIZE);
        if (g_Resources.SplitPools[i]) {
            ZeroMem(g_Resources.SplitPools[i], HV_SPLIT_POOL_SIZE);
            g_Resources.SplitPoolsPhys[i] = KernelGetPhysicalAddress(g_Resources.SplitPools[i]);
            g_Resources.SplitPoolCount++;
        } else {
            // Partial allocation is OK, just track how many we got
            break;
        }
    }
    // Note: Split pools are optional - EPT has its own contiguous pool as fallback

    // Write revision ID to all VMXON regions
    U32 revisionId = (U32)(g_Resources.VmxBasic & 0x7FFFFFFF);
    for (U32 i = 0; i < cpuCount; i++) {
        U32* vmxon = (U32*)((U8*)g_Resources.VmxonRegions + (i * 0x1000));
        *vmxon = revisionId;

        U32* vmcs = (U32*)((U8*)g_Resources.VmcsRegions + (i * 0x1000));
        *vmcs = revisionId;
    }

    g_Resources.Allocated = true;
    return OMBRA_SUCCESS;
}

// =============================================================================
// Free VMX Resources
// =============================================================================

static void FreeResources(void) {
    if (!g_Resources.Allocated) return;

    if (g_Resources.VmxonRegions) KernelFreeContiguous(g_Resources.VmxonRegions);
    if (g_Resources.VmcsRegions) KernelFreeContiguous(g_Resources.VmcsRegions);
    if (g_Resources.HostStacks) KernelFreeContiguous(g_Resources.HostStacks);
    if (g_Resources.MsrBitmap) KernelFreeContiguous(g_Resources.MsrBitmap);
    if (g_Resources.EptTables) KernelFreeContiguous(g_Resources.EptTables);
    if (g_Resources.BlankPage) KernelFreeContiguous(g_Resources.BlankPage);

    // Free pre-allocated split pools
    for (U32 i = 0; i < g_Resources.SplitPoolCount; i++) {
        if (g_Resources.SplitPools[i]) {
            KernelFreeContiguous(g_Resources.SplitPools[i]);
        }
    }

    // Debug buffer uses ExAllocatePool, would need ExFreePool

    ZeroMem(&g_Resources, sizeof(g_Resources));
}

// =============================================================================
// Per-CPU VMX Initialization
// =============================================================================

static OMBRA_STATUS InitializeCpuVmx(U32 cpuId) {
    VMX_CPU* cpu;
    OMBRA_STATUS status;
    U64 cr0, cr4;
    U8 vmxError;

    TRACE("CPU %u: Starting VMX initialization", cpuId);

    // Get CPU context
    cpu = &g_CpuContexts[cpuId];
    cpu->CpuId = cpuId;

    // Calculate per-CPU addresses
    void* vmxonVirt = (U8*)g_Resources.VmxonRegions + (cpuId * 0x1000);
    U64 vmxonPhys = g_Resources.VmxonRegionsPhys + (cpuId * 0x1000);
    void* vmcsVirt = (U8*)g_Resources.VmcsRegions + (cpuId * 0x1000);
    U64 vmcsPhys = g_Resources.VmcsRegionsPhys + (cpuId * 0x1000);
    void* stackTop = (U8*)g_Resources.HostStacks + ((cpuId + 1) * HV_HOST_STACK_SIZE);

    // Store addresses
    cpu->VmxonPhysical = vmxonPhys;
    cpu->VmcsPhysical = vmcsPhys;
    cpu->MsrBitmapPhysical = g_Resources.MsrBitmapPhys;
    cpu->HostStackTop = stackTop;
    cpu->VmxonRegion = vmxonVirt;
    cpu->VmcsRegion = vmcsVirt;
    cpu->MsrBitmap = g_Resources.MsrBitmap;
    cpu->Ept = &g_EptState;

    // Check VMX support
    status = VmxCheckSupport();
    if (OMBRA_FAILED(status)) {
        ERR("CPU %u: VMX not supported (0x%x)", cpuId, status);
        return status;
    }

    // Enable VMX in CR4
    cr4 = __readcr4();
    if (!(cr4 & CR4_VMXE)) {
        cr4 |= CR4_VMXE;
        __writecr4(cr4);
    }

    // Apply CR0/CR4 fixed bits
    cr0 = __readcr0();
    cr0 |= g_Resources.VmxCr0Fixed0;
    cr0 &= g_Resources.VmxCr0Fixed1;
    __writecr0(cr0);

    cr4 = __readcr4();
    cr4 |= g_Resources.VmxCr4Fixed0;
    cr4 &= g_Resources.VmxCr4Fixed1;
    __writecr4(cr4);

    // VMXON
    vmxError = __vmx_on(&vmxonPhys);
    if (vmxError) {
        ERR("CPU %u: VMXON failed (error %u)", cpuId, vmxError);
        return OMBRA_ERROR_VMXON_FAILED;
    }
    cpu->VmxEnabled = true;
    INFO("CPU %u: VMXON successful", cpuId);

    // Build HV_PER_CPU_PARAMS for VMCS initialization
    HV_PER_CPU_PARAMS cpuParams = {0};
    cpuParams.CpuId = cpuId;
    cpuParams.TotalCpus = g_Resources.CpuCount;
    cpuParams.VmxonPhysical = vmxonPhys;
    cpuParams.VmcsPhysical = vmcsPhys;
    cpuParams.HostStackTop = (U64)stackTop;
    cpuParams.MsrBitmapPhysical = g_Resources.MsrBitmapPhys;
    cpuParams.VmxonVirtual = vmxonVirt;
    cpuParams.VmcsVirtual = vmcsVirt;
    cpuParams.MsrBitmapVirtual = g_Resources.MsrBitmap;
    cpuParams.EptPml4Physical = g_Resources.EptTablesPhys;
    cpuParams.EptPml4Virtual = g_Resources.EptTables;
    cpuParams.VmxBasic = g_Resources.VmxBasic;
    cpuParams.VmxPinCtls = g_Resources.VmxPinbasedCtls;
    cpuParams.VmxProcCtls = g_Resources.VmxProcbasedCtls;
    cpuParams.VmxProcCtls2 = g_Resources.VmxProcbasedCtls2;
    cpuParams.VmxExitCtls = g_Resources.VmxExitCtls;
    cpuParams.VmxEntryCtls = g_Resources.VmxEntryCtls;
    cpuParams.VmxTruePin = g_Resources.VmxTruePinbasedCtls;
    cpuParams.VmxTrueProc = g_Resources.VmxTrueProcbasedCtls;
    cpuParams.VmxTrueExit = g_Resources.VmxTrueExitCtls;
    cpuParams.VmxTrueEntry = g_Resources.VmxTrueEntryCtls;
    cpuParams.VmxCr0Fixed0 = g_Resources.VmxCr0Fixed0;
    cpuParams.VmxCr0Fixed1 = g_Resources.VmxCr0Fixed1;
    cpuParams.VmxCr4Fixed0 = g_Resources.VmxCr4Fixed0;
    cpuParams.VmxCr4Fixed1 = g_Resources.VmxCr4Fixed1;
    cpuParams.VmxEptVpidCap = g_Resources.VmxEptVpidCap;
    if (g_Resources.DebugBuffer) {
        cpuParams.DebugBufferVirtual = g_Resources.DebugBuffer;
        cpuParams.DebugBufferSize = HV_DEBUG_BUFFER_SIZE;
    }

    status = VmcsInitialize(cpu, &cpuParams);
    if (OMBRA_FAILED(status)) {
        ERR("CPU %u: VMCS initialization failed (0x%x)", cpuId, status);
        __vmx_off();
        cpu->VmxEnabled = false;
        return status;
    }

    // VMLAUNCH
    INFO("CPU %u: Executing VMLAUNCH...", cpuId);
    U64 launchResult = AsmVmxLaunch();

    if (launchResult == 0) {
        cpu->VmxLaunched = true;
        INFO("CPU %u: VMLAUNCH successful - hypervisor active!", cpuId);
        return OMBRA_SUCCESS;
    } else {
        U64 errorCode = 0;
        __vmx_vmread(VMCS_EXIT_INSTRUCTION_ERROR, &errorCode);
        ERR("CPU %u: VMLAUNCH failed - error code %llu", cpuId, errorCode);
        __vmx_off();
        cpu->VmxEnabled = false;
        return OMBRA_ERROR_VMLAUNCH_FAILED;
    }
}

// =============================================================================
// IPI Callback - Runs on Each CPU
// =============================================================================

static U64 __stdcall VirtualizeThisCpu(U64 Argument) {
    (void)Argument;

    // Get current CPU ID via resolved function
    U32 cpuId = 0;
    if (g_KernelSymbols.KeGetCurrentProcessorNumberEx) {
        cpuId = g_KernelSymbols.KeGetCurrentProcessorNumberEx(NULL);
    }

    // Initialize this CPU
    OMBRA_STATUS status = InitializeCpuVmx(cpuId);

    if (OMBRA_SUCCESS == status) {
        _InterlockedIncrement((volatile long*)&g_SuccessCount);
    } else {
        _InterlockedIncrement((volatile long*)&g_FailCount);
    }

    return (U64)status;
}

// =============================================================================
// Main Initialization
// =============================================================================

static OMBRA_STATUS OmbraInitialize(void) {
    OMBRA_STATUS status;

    if (g_Initialized) {
        return OMBRA_SUCCESS;
    }

    // Initialize debug logging first
    if (g_Resources.DebugBuffer) {
        status = DbgInitialize(g_Resources.DebugBuffer, HV_DEBUG_BUFFER_SIZE);
        if (OMBRA_SUCCESS != status) {
            // Continue without debug - not critical
        }
    }

    INFO("OmbraHypervisor starting (self-contained mode)...");
    INFO("CPU count: %u", g_Resources.CpuCount);

    // Discover self info for EPT protection
    status = SelfInfoDiscover();
    if (OMBRA_FAILED(status)) {
        WARN("Self info discovery failed: 0x%X", status);
        // Continue - self-protection is optional
    }

    // Initialize EPT (once, shared across CPUs)
    void* pdptVirt = (U8*)g_Resources.EptTables + 0x1000;
    U64 pdptPhys = g_Resources.EptTablesPhys + 0x1000;

    status = EptInitialize(
        &g_EptState,
        g_Resources.EptTables,
        g_Resources.EptTablesPhys,
        pdptVirt, pdptPhys,
        HV_EPT_PAGES
    );
    if (OMBRA_FAILED(status)) {
        ERR("EPT initialization failed: 0x%X", status);
        return status;
    }
    INFO("EPT initialized");

    // Self-protection: Hide hypervisor memory from guest
    U64 hvPhysBase, hvPhysSize;
    if (OMBRA_SUCCESS == SelfInfoGetPhysicalRange(&hvPhysBase, &hvPhysSize)) {
        if (g_Resources.BlankPagePhys != 0) {
            status = EptProtectSelf(
                &g_EptState,
                hvPhysBase,
                hvPhysSize,
                g_Resources.BlankPagePhys
            );
            if (OMBRA_FAILED(status)) {
                WARN("EPT self-protection failed: 0x%X (continuing)", status);
            } else {
                INFO("EPT self-protection enabled");
            }
        }
    }

    // Initialize hook manager
    status = HookManagerInit(&g_HookManager, &g_EptState);
    if (OMBRA_FAILED(status)) {
        ERR("Hook manager init failed: 0x%X", status);
        return status;
    }
    INFO("Hook manager initialized");

    // Reset counters
    g_SuccessCount = 0;
    g_FailCount = 0;

    // Broadcast to all CPUs via IPI
    INFO("Broadcasting VMX init to all CPUs...");

    if (g_KernelSymbols.KeIpiGenericCall) {
        g_KernelSymbols.KeIpiGenericCall((void*)VirtualizeThisCpu, 0);
    } else {
        ERR("KeIpiGenericCall not available!");
        return OMBRA_ERROR_INVALID_STATE;
    }

    // Check results
    INFO("VMX init complete: %u success, %u failed", g_SuccessCount, g_FailCount);

    if (g_SuccessCount == 0) {
        ERR("No CPUs virtualized!");
        return OMBRA_ERROR_VMXON_FAILED;
    }

    if (g_FailCount > 0) {
        WARN("Some CPUs failed to virtualize");
    }

    g_Initialized = true;
    INFO("OmbraHypervisor active on %u CPUs", g_SuccessCount);
    return OMBRA_SUCCESS;
}

// =============================================================================
// Phase 2 Entry Point (Runs from MDL Memory)
// =============================================================================
//
// Called after self-relocation from IPRT memory to MDL memory.
// At this point:
// - Hypervisor code is running from MDL-backed memory (invisible in BigPool)
// - MDL allocator is initialized with pre-allocated regions
// - All VMX resources should be allocated from MDL regions
//
// The relocation context contains information about the source (IPRT) memory
// that should be freed by the loader after Phase 2 returns successfully.

static OMBRA_STATUS HvEntryPhase2(RELOCATION_CONTEXT* ctx) {
    OMBRA_STATUS status;

    // Mark Phase 2 as active
    g_Phase2Complete = false;

    INFO("Phase 2 starting from MDL memory at 0x%p", ctx->DestBase);
    INFO("  Source (IPRT): 0x%p (%llu bytes)", ctx->SourceBase, ctx->SourceSize);
    INFO("  Dest (MDL):    0x%p (%llu bytes)", ctx->DestBase, ctx->DestSize);
    INFO("  Delta:         0x%llx", ctx->Delta);

    // At this point, we're running from MDL memory
    // The MDL allocator (g_MdlAllocator) should already be set up

    // Verify MDL allocator is available
    if (!g_MdlAllocator || !g_MdlAllocator->Initialized) {
        ERR("Phase 2: MDL allocator not available!");
        return OMBRA_ERROR_NOT_INITIALIZED;
    }

    // Read VMX capability MSRs (if not already done)
    if (g_Resources.VmxBasic == 0) {
        ReadVmxCapabilities();
    }

    // Allocate VMX resources using MDL allocator (stealth mode)
    status = AllocateResourcesMdl();
    if (OMBRA_FAILED(status)) {
        ERR("Phase 2: MDL resource allocation failed: 0x%X", status);
        return status;
    }
    INFO("Phase 2: Resources allocated from MDL memory");

    // Print MDL allocation stats
    MDL_STATS stats;
    MdlGetStats(g_MdlAllocator, &stats);
    INFO("  Main:  %llu / %llu bytes used", stats.MainUsed, stats.MainTotal);
    INFO("  VMCS:  %llu / %llu bytes used", stats.VmcsUsed, stats.VmcsTotal);
    INFO("  EPT:   %llu / %llu bytes used", stats.EptUsed, stats.EptTotal);
    INFO("  Stack: %llu / %llu bytes used", stats.StackUsed, stats.StackTotal);
    INFO("  MSR:   %llu / %llu bytes used", stats.MsrUsed, stats.MsrTotal);
    INFO("  Misc:  %llu / %llu bytes used", stats.MiscUsed, stats.MiscTotal);

    // Continue with normal hypervisor initialization
    status = OmbraInitialize();
    if (OMBRA_FAILED(status)) {
        ERR("Phase 2: OmbraInitialize failed: 0x%X", status);
        return status;
    }

    g_Phase2Complete = true;
    INFO("Phase 2 complete - hypervisor running from stealth memory");

    // Return success - caller (loader) can now free IPRT memory
    return OMBRA_SUCCESS;
}

// =============================================================================
// Entry Point - Called by Loader (Phase 1)
// =============================================================================
//
// Two-Phase Initialization for Stealth Memory:
//
// Phase 1 (this function, runs from IPRT memory):
//   1. Resolve kernel symbols (including MDL functions)
//   2. Initialize MDL allocator (creates stealth memory regions)
//   3. Copy hypervisor image to MDL memory
//   4. Apply PE base relocations
//   5. Jump to Phase 2 in the relocated image
//
// Phase 2 (HvEntryPhase2, runs from MDL memory):
//   1. Allocate VMX resources from MDL memory (no BigPool footprint)
//   2. Initialize EPT, hooks, and VMX on all CPUs
//   3. Return success to loader, which can free IPRT memory
//
// The loader must:
//   1. Set g_KernelSymbols.MmGetSystemRoutineAddress before calling
//   2. Call HvEntry(NULL) or HvEntry(MmGetSystemRoutineAddress)
//   3. Returns 0 on success, negative on failure
//   4. On success, free the original IPRT memory allocation

__declspec(dllexport)
int HvEntry(void* MmGetSystemRoutineAddress) {
    OMBRA_STATUS status;

    // Accept MmGetSystemRoutineAddress from loader
    if (MmGetSystemRoutineAddress) {
        g_KernelSymbols.MmGetSystemRoutineAddress =
            (FN_MmGetSystemRoutineAddress)MmGetSystemRoutineAddress;
    }

    // Must have bootstrap function
    if (!g_KernelSymbols.MmGetSystemRoutineAddress) {
        return -1;  // No bootstrap function
    }

    // Resolve all kernel symbols (including MDL functions)
    status = KernelResolveSymbols();
    if (OMBRA_FAILED(status)) {
        return -2;  // Symbol resolution failed
    }

    // Read VMX capability MSRs (needed for resource allocation)
    ReadVmxCapabilities();

    // Check if MDL-based stealth allocation is enabled and available
    if (g_UseMdlAllocation &&
        g_KernelSymbols.MmAllocatePagesForMdlEx &&
        g_KernelSymbols.MmMapLockedPagesSpecifyCache) {

        // =====================================================================
        // PHASE 1: MDL Allocation and Self-Relocation
        // =====================================================================

        // Initialize the bootstrap MDL allocator
        // This creates all the stealth memory regions
        status = MdlAllocatorInit(&g_BootstrapAllocator);
        if (OMBRA_FAILED(status)) {
            // MDL allocation failed - fall back to legacy mode
            g_UseMdlAllocation = false;
            goto legacy_mode;
        }

        // Set the global allocator pointer
        g_MdlAllocator = &g_BootstrapAllocator;

        // Perform self-relocation to MDL memory
        // This copies the hypervisor image and applies PE relocations
        status = RelocateSelf();
        if (OMBRA_FAILED(status)) {
            // Relocation failed - clean up and fall back to legacy mode
            MdlAllocatorDestroy(&g_BootstrapAllocator);
            g_MdlAllocator = NULL;
            g_UseMdlAllocation = false;
            goto legacy_mode;
        }

        // Calculate the Phase 2 entry address in the relocated image
        FN_Phase2Entry phase2 = (FN_Phase2Entry)RelocateCalculatePhase2Address(
            (void*)HvEntryPhase2
        );

        if (!phase2) {
            // Failed to calculate Phase 2 address
            MdlAllocatorDestroy(&g_BootstrapAllocator);
            g_MdlAllocator = NULL;
            g_UseMdlAllocation = false;
            goto legacy_mode;
        }

        // =====================================================================
        // JUMP TO PHASE 2 (in MDL memory)
        // =====================================================================
        // After this call, we're running from stealth memory.
        // Phase 2 will allocate VMX resources and initialize the hypervisor.
        // When Phase 2 returns, the hypervisor is active and we're running
        // as a guest. The loader can then free the IPRT memory.

        status = RelocateJumpToPhase2(phase2);

        if (OMBRA_SUCCESS == status && g_Phase2Complete) {
            // Success - hypervisor running from stealth memory
            // The loader should now free the original IPRT allocation
            return 0;
        }

        // Phase 2 failed - fall back to legacy mode is not possible at this point
        // because we've already corrupted state by relocating
        return -5;  // Phase 2 failed
    }

legacy_mode:
    // =========================================================================
    // LEGACY MODE: Pool-based allocation (visible in BigPool)
    // =========================================================================
    // This is the fallback when MDL allocation is not available or fails.
    // VMX resources will be allocated using KernelAllocateContiguous which
    // creates visible BigPool entries.

    // Allocate VMX resources using legacy pool allocation
    status = AllocateResources();
    if (OMBRA_FAILED(status)) {
        return -3;  // Resource allocation failed
    }

    // Initialize hypervisor
    status = OmbraInitialize();
    if (OMBRA_FAILED(status)) {
        FreeResources();
        return -4;  // Initialization failed
    }

    return 0;  // Success (legacy mode)
}

// =============================================================================
// Legacy Entry Point - For backwards compatibility with .ombra section
// =============================================================================

#pragma section(".ombra", read, write)
#pragma comment(linker, "/SECTION:.ombra,RW")

__declspec(allocate(".ombra"))
volatile OMBRA_BOOTSTRAP g_Bootstrap = {
    .Magic = 0x524D424F,    // 'OMBR'
    .Version = 1,
    .ParamsPtr = 0,
    .Reserved = {0}
};

__declspec(dllexport)
int OmbraModuleInit(void* ignored) {
    (void)ignored;

    // Check if .ombra section was patched with MmGetSystemRoutineAddress
    // In the self-contained design, the loader stores MmGetSystemRoutineAddress
    // directly in ParamsPtr (not as a pointer to HV_INIT_PARAMS).
    if (g_Bootstrap.Magic == 0x524D424F &&
        g_Bootstrap.Version == 1 &&
        g_Bootstrap.ParamsPtr != 0) {

        // ParamsPtr contains MmGetSystemRoutineAddress directly
        // Store it in g_KernelSymbols for HvEntry to use
        g_KernelSymbols.MmGetSystemRoutineAddress =
            (FN_MmGetSystemRoutineAddress)g_Bootstrap.ParamsPtr;
    }

    // Call HvEntry - it will use g_KernelSymbols.MmGetSystemRoutineAddress
    // If .ombra was patched correctly, MmGetSystemRoutineAddress is already set
    return HvEntry(NULL);
}

// =============================================================================
// Shutdown Entry Point
// =============================================================================

__declspec(dllexport)
void OmbraShutdown(void) {
    INFO("OmbraHypervisor shutting down...");

    // Shutdown hook manager first
    if (g_HookManager.Initialized) {
        HookManagerShutdown(&g_HookManager);
    }

    // Count active CPUs
    U32 activeCpus = 0;
    for (U32 i = 0; i < HV_MAX_CPUS; i++) {
        VMX_CPU* cpu = &g_CpuContexts[i];
        if (cpu->VmxEnabled && cpu->VmxLaunched) {
            activeCpus++;
        }
    }

    if (activeCpus > 0) {
        WARN("Shutdown requested with %u active CPUs", activeCpus);
        WARN("VMXOFF must be executed per-CPU via VMCALL_UNLOAD");

        for (U32 i = 0; i < HV_MAX_CPUS; i++) {
            VMX_CPU* cpu = &g_CpuContexts[i];
            if (cpu->VmxEnabled) {
                cpu->ShutdownPending = true;
            }
        }
    }

    DbgShutdown();

    // Free resources only after all CPUs have executed VMXOFF
    // This would typically happen in response to VMCALL_UNLOAD completion
    // FreeResources();  // Deferred

    INFO("Shutdown initiated - awaiting per-CPU VMXOFF");
    g_Initialized = false;
}

// =============================================================================
// Helper: Get CPU Context (local to entry.c)
// =============================================================================

static VMX_CPU* GetLocalCpuContext(void) {
    int cpuInfo[4];
    __cpuidex(cpuInfo, 0x1, 0);

    // Initial APIC ID is in bits 24-31 of EBX
    U32 apicId = (cpuInfo[1] >> 24) & 0xFF;

    if (apicId < HV_MAX_CPUS) {
        return &g_CpuContexts[apicId];
    }

    return NULL;
}
