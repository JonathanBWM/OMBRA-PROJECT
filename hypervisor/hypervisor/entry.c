// entry.c — Hypervisor Entry Point (Phase 1 Redesign)
// OmbraHypervisor
//
// New architecture: Single entry via LDR_LOAD, parameters via .ombra section,
// IPI broadcast for multi-CPU initialization.

#include "../shared/types.h"
#include "../shared/vmcs_fields.h"
#include "../shared/msr_defs.h"
#include "../shared/cpu_defs.h"
#include "vmx.h"
#include "vmcs.h"
#include "ept.h"
#include "hooks.h"
#include "debug.h"
#include <intrin.h>

// Windows kernel types for function pointers (kernel-mode compatible)
#ifndef ULONG_PTR
typedef U64 ULONG_PTR;
#endif
#ifndef ULONG
typedef U32 ULONG;
#endif
#ifndef USHORT
typedef U16 USHORT;
#endif
#ifndef NTAPI
#define NTAPI __stdcall
#endif

// =============================================================================
// Bootstrap Section - Loader patches ParamsPtr before LDR_LOAD
// =============================================================================

#pragma section(".ombra", read, write)
#pragma comment(linker, "/SECTION:.ombra,RW")

// OMBRA_BOOTSTRAP defined in shared/types.h

__declspec(allocate(".ombra"))
volatile OMBRA_BOOTSTRAP g_Bootstrap = {
    .Magic = 0x524D424F,    // 'OMBR'
    .Version = 1,
    .ParamsPtr = 0,
    .Reserved = {0}
};

// =============================================================================
// Global State
// =============================================================================

static VMX_CPU g_CpuContexts[256] = {0};
static EPT_STATE g_EptState = {0};
static volatile U32 g_SuccessCount = 0;
static volatile U32 g_FailCount = 0;
static bool g_DebugInitialized = false;
static HV_INIT_PARAMS* g_InitParams = NULL;

// External hook manager
extern HOOK_MANAGER g_HookManager;

// =============================================================================
// IPI Callback Types (Windows kernel function signatures)
// =============================================================================

typedef ULONG_PTR (NTAPI *KIPI_BROADCAST_WORKER)(ULONG_PTR Argument);
typedef ULONG_PTR (NTAPI *FN_KeIpiGenericCall)(KIPI_BROADCAST_WORKER, ULONG_PTR);
typedef ULONG (NTAPI *FN_KeQueryActiveProcessorCountEx)(USHORT);
typedef ULONG (NTAPI *FN_KeGetCurrentProcessorNumberEx)(void*);

// =============================================================================
// Per-CPU VMX Initialization
// =============================================================================

static OMBRA_STATUS InitializeCpuVmx(
    U32 cpuId,
    void* vmxonVirt, U64 vmxonPhys,
    void* vmcsVirt, U64 vmcsPhys,
    void* stackTop,
    void* msrBitmapVirt, U64 msrBitmapPhys,
    HV_INIT_PARAMS* params)
{
    VMX_CPU* cpu;
    OMBRA_STATUS status;
    U64 cr0, cr4;
    U8 vmxError;

    TRACE("CPU %u: Starting VMX initialization", cpuId);

    // Get CPU context
    cpu = &g_CpuContexts[cpuId];
    cpu->CpuId = cpuId;

    // Store addresses
    cpu->VmxonPhysical = vmxonPhys;
    cpu->VmcsPhysical = vmcsPhys;
    cpu->MsrBitmapPhysical = msrBitmapPhys;
    cpu->HostStackTop = stackTop;
    cpu->VmxonRegion = vmxonVirt;
    cpu->VmcsRegion = vmcsVirt;
    cpu->MsrBitmap = msrBitmapVirt;
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
    cr0 |= params->VmxCr0Fixed0;
    cr0 &= params->VmxCr0Fixed1;
    __writecr0(cr0);

    cr4 = __readcr4();
    cr4 |= params->VmxCr4Fixed0;
    cr4 &= params->VmxCr4Fixed1;
    __writecr4(cr4);

    // Write revision ID to VMXON region (should already be set by loader)
    U32 revisionId = (U32)(params->VmxBasic & 0x7FFFFFFF);
    *(U32*)vmxonVirt = revisionId;

    // VMXON
    vmxError = __vmx_on(&vmxonPhys);
    if (vmxError) {
        ERR("CPU %u: VMXON failed (error %u)", cpuId, vmxError);
        return OMBRA_ERROR_VMXON_FAILED;
    }
    cpu->VmxEnabled = true;
    INFO("CPU %u: VMXON successful", cpuId);

    // Initialize VMCS - create HV_PER_CPU_PARAMS from the Phase 1 params
    HV_PER_CPU_PARAMS cpuParams = {0};
    cpuParams.CpuId = cpuId;
    cpuParams.TotalCpus = params->CpuCount;
    cpuParams.VmxonPhysical = vmxonPhys;
    cpuParams.VmcsPhysical = vmcsPhys;
    cpuParams.HostStackTop = (U64)stackTop;
    cpuParams.MsrBitmapPhysical = msrBitmapPhys;
    cpuParams.VmxonVirtual = vmxonVirt;
    cpuParams.VmcsVirtual = vmcsVirt;
    cpuParams.MsrBitmapVirtual = msrBitmapVirt;
    cpuParams.EptPml4Physical = params->EptTablesPhys;
    cpuParams.EptPml4Virtual = params->EptTablesVirt;
    // Copy VMX MSR values
    cpuParams.VmxBasic = params->VmxBasic;
    cpuParams.VmxPinCtls = params->VmxPinbasedCtls;
    cpuParams.VmxProcCtls = params->VmxProcbasedCtls;
    cpuParams.VmxProcCtls2 = params->VmxProcbasedCtls2;
    cpuParams.VmxExitCtls = params->VmxExitCtls;
    cpuParams.VmxEntryCtls = params->VmxEntryCtls;
    cpuParams.VmxTruePin = params->VmxTruePinbasedCtls;
    cpuParams.VmxTrueProc = params->VmxTrueProcbasedCtls;
    cpuParams.VmxTrueExit = params->VmxTrueExitCtls;
    cpuParams.VmxTrueEntry = params->VmxTrueEntryCtls;
    cpuParams.VmxCr0Fixed0 = params->VmxCr0Fixed0;
    cpuParams.VmxCr0Fixed1 = params->VmxCr0Fixed1;
    cpuParams.VmxCr4Fixed0 = params->VmxCr4Fixed0;
    cpuParams.VmxCr4Fixed1 = params->VmxCr4Fixed1;
    cpuParams.VmxEptVpidCap = params->VmxEptVpidCap;
    cpuParams.DebugBufferPhysical = params->DebugBufferPhys;
    cpuParams.DebugBufferVirtual = params->DebugBufferVirt;
    cpuParams.DebugBufferSize = params->DebugBufferSize;

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

static ULONG_PTR NTAPI VirtualizeThisCpu(ULONG_PTR Argument) {
    HV_INIT_PARAMS* params = (HV_INIT_PARAMS*)Argument;

    // Get current CPU number
    FN_KeGetCurrentProcessorNumberEx getCpuNum =
        (FN_KeGetCurrentProcessorNumberEx)params->KeGetCurrentProcessorNumberEx;
    U32 cpuId = getCpuNum(NULL);

    // Calculate per-CPU addresses
    void* vmxonVirt = (U8*)params->VmxonRegionsVirt + (cpuId * 0x1000);
    U64 vmxonPhys = params->VmxonRegionsPhys + (cpuId * 0x1000);
    void* vmcsVirt = (U8*)params->VmcsRegionsVirt + (cpuId * 0x1000);
    U64 vmcsPhys = params->VmcsRegionsPhys + (cpuId * 0x1000);
    void* stackTop = (U8*)params->HostStacksBase + ((cpuId + 1) * params->HostStackSize);

    // Initialize this CPU
    OMBRA_STATUS status = InitializeCpuVmx(
        cpuId,
        vmxonVirt, vmxonPhys,
        vmcsVirt, vmcsPhys,
        stackTop,
        params->MsrBitmapVirt, params->MsrBitmapPhys,
        params
    );

    if (OMBRA_SUCCESS == status) {
        _InterlockedIncrement((volatile long*)&g_SuccessCount);
    } else {
        _InterlockedIncrement((volatile long*)&g_FailCount);
    }

    return status;
}

// =============================================================================
// Main Initialization (called after IPI broadcast setup)
// =============================================================================

static int OmbraInitialize(HV_INIT_PARAMS* params) {
    OMBRA_STATUS status;

    // Initialize debug logging first
    if (params->DebugBufferVirt && params->DebugBufferSize > 0) {
        status = DbgInitialize(params->DebugBufferVirt, params->DebugBufferSize);
        if (OMBRA_SUCCESS == status) {
            g_DebugInitialized = true;
        }
    }

    INFO("OmbraHypervisor starting...");
    INFO("CPU count: %u", params->CpuCount);

    // Initialize EPT (once, shared across CPUs)
    // EPT memory layout: Page 0 = PML4, Page 1 = PDPT, remaining = for splits
    void* pdptVirt = (U8*)params->EptTablesVirt + 4096;
    U64 pdptPhys = params->EptTablesPhys + 4096;

    status = EptInitialize(
        &g_EptState,
        params->EptTablesVirt,
        params->EptTablesPhys,
        pdptVirt, pdptPhys,
        params->EptTablesPages
    );
    if (OMBRA_FAILED(status)) {
        ERR("EPT initialization failed: 0x%X", status);
        return (int)status;
    }
    INFO("EPT initialized");

    // Self-protection: Hide hypervisor memory from guest
    if (params->HvPhysBase != 0 && params->BlankPagePhys != 0) {
        status = EptProtectSelf(
            &g_EptState,
            params->HvPhysBase,
            params->HvPhysSize,
            params->BlankPagePhys
        );
        if (OMBRA_FAILED(status)) {
            WARN("EPT self-protection failed: 0x%X (continuing anyway)", status);
            // Don't fail init - self-protection is optional enhancement
        } else {
            INFO("EPT self-protection enabled");
        }
    }

    // Initialize hook manager
    status = HookManagerInit(&g_HookManager, &g_EptState);
    if (OMBRA_FAILED(status)) {
        ERR("Hook manager init failed: 0x%X", status);
        return (int)status;
    }
    INFO("Hook manager initialized");

    // Store params globally for IPI callback
    g_InitParams = params;
    g_SuccessCount = 0;
    g_FailCount = 0;

    // Broadcast to all CPUs
    INFO("Broadcasting VMX init to all CPUs...");

    FN_KeIpiGenericCall ipiCall = (FN_KeIpiGenericCall)params->KeIpiGenericCall;
    ipiCall(VirtualizeThisCpu, (ULONG_PTR)params);

    // Check results
    INFO("VMX init complete: %u success, %u failed", g_SuccessCount, g_FailCount);

    if (g_SuccessCount == 0) {
        ERR("No CPUs virtualized!");
        return -100;
    }

    if (g_FailCount > 0) {
        WARN("Some CPUs failed to virtualize");
    }

    INFO("OmbraHypervisor active on %u CPUs", g_SuccessCount);
    return 0;
}

// =============================================================================
// Module Entry Point - Called by SUPDrv LDR_LOAD
// =============================================================================

__declspec(dllexport)
int OmbraModuleInit(void* ignored) {
    (void)ignored;  // SUPDrv may pass something, we don't need it

    // Read params from bootstrap section
    if (g_Bootstrap.Magic != 0x524D424F) {
        return -1;  // Bootstrap magic invalid
    }

    if (g_Bootstrap.Version != 1) {
        return -2;  // Bootstrap version mismatch
    }

    if (g_Bootstrap.ParamsPtr == 0) {
        return -3;  // Params not patched
    }

    HV_INIT_PARAMS* params = (HV_INIT_PARAMS*)g_Bootstrap.ParamsPtr;

    // Validate params structure
    if (params->Magic != 0x4F4D4252) {  // 'OMBR'
        return -4;  // Params magic invalid
    }

    if (params->Version != 1) {
        return -5;  // Params version mismatch
    }

    // Call actual initialization
    return OmbraInitialize(params);
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
    for (U32 i = 0; i < 256; i++) {
        VMX_CPU* cpu = &g_CpuContexts[i];
        if (cpu->VmxEnabled && cpu->VmxLaunched) {
            activeCpus++;
        }
    }

    if (activeCpus > 0) {
        WARN("Shutdown requested with %u active CPUs", activeCpus);
        WARN("VMXOFF must be executed per-CPU via VMCALL_UNLOAD");

        for (U32 i = 0; i < 256; i++) {
            VMX_CPU* cpu = &g_CpuContexts[i];
            if (cpu->VmxEnabled) {
                cpu->ShutdownPending = true;
            }
        }
    }

    DbgShutdown();
    INFO("Shutdown initiated - awaiting per-CPU VMXOFF");
}

// =============================================================================
// Helper: Get CPU Context (local to entry.c)
// =============================================================================
//
// NOTE: This is a LOCAL helper that accesses g_CpuContexts (entry.c's local array).
// The EXTERNAL VmxGetCurrentCpu is defined in vmx.c and accesses g_Ombra.Cpus.
// Making this static avoids duplicate symbol errors during linking.

static VMX_CPU* GetLocalCpuContext(void) {
    int cpuInfo[4];
    __cpuidex(cpuInfo, 0x1, 0);

    // Initial APIC ID is in bits 24-31 of EBX
    U32 apicId = (cpuInfo[1] >> 24) & 0xFF;

    if (apicId < 256) {
        return &g_CpuContexts[apicId];
    }

    return NULL;
}
