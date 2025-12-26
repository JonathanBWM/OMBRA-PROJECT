// entry.c — Hypervisor Entry Point
// OmbraHypervisor
//
// This is called by the usermode loader via DrvExecuteOnCpu for each CPU.
// It receives HV_INIT_DATA containing all physical addresses and parameters.

#include "../shared/types.h"
#include "../shared/vmcs_fields.h"
#include "../shared/msr_defs.h"
#include "vmx.h"
#include "vmcs.h"
#include "ept.h"
#include "debug.h"
#include <intrin.h>

// =============================================================================
// Initialization Data Structure (must match usermode HV_INIT_DATA)
// =============================================================================

typedef struct _HV_INIT_DATA {
    U32     CpuId;
    U32     TotalCpus;

    // Per-CPU physical addresses
    U64     VmxonPhysical;
    U64     VmcsPhysical;
    U64     HostStackTop;       // Virtual address of stack TOP
    U64     MsrBitmapPhysical;

    // Shared physical addresses
    U64     EptPml4Physical;

    // VMX MSR values (pre-fetched from driver)
    U64     VmxBasic;
    U64     VmxPinCtls;
    U64     VmxProcCtls;
    U64     VmxProcCtls2;
    U64     VmxExitCtls;
    U64     VmxEntryCtls;
    U64     VmxCr0Fixed0;
    U64     VmxCr0Fixed1;
    U64     VmxCr4Fixed0;
    U64     VmxCr4Fixed1;
    U64     VmxEptVpidCap;

    // Exit handler address
    U64     VmexitHandler;

    // Debug infrastructure
    U64     DebugBufferPhysical;
    U64     DebugBufferSize;
} HV_INIT_DATA;

// =============================================================================
// Global State
// =============================================================================

static VMX_CPU g_CpuContexts[256] = {0};
static EPT_STATE g_EptState = {0};
static volatile U32 g_InitializedCpus = 0;
static bool g_DebugInitialized = false;

// =============================================================================
// Per-CPU Initialization
// =============================================================================

static OMBRA_STATUS InitializeOnThisCpu(HV_INIT_DATA* initData) {
    VMX_CPU* cpu;
    OMBRA_STATUS status;
    U64 cr0, cr4;
    U8 vmxError;
    U32 cpuId = initData->CpuId;

    INFO("CPU %u: Starting VMX initialization", cpuId);

    // Get or allocate CPU context
    cpu = &g_CpuContexts[cpuId];
    cpu->CpuId = cpuId;

    // Store physical addresses
    cpu->VmxonPhysical = initData->VmxonPhysical;
    cpu->VmcsPhysical = initData->VmcsPhysical;
    cpu->MsrBitmapPhysical = initData->MsrBitmapPhysical;
    cpu->HostStackTop = (void*)initData->HostStackTop;

    // -------------------------------------------------------------------------
    // Step 1: Check VMX support
    // -------------------------------------------------------------------------
    status = VmxCheckSupport();
    if (OMBRA_FAILED(status)) {
        ERR("CPU %u: VMX not supported (0x%x)", cpuId, status);
        return status;
    }
    TRACE("CPU %u: VMX support verified", cpuId);

    // -------------------------------------------------------------------------
    // Step 2: Enable VMX in CR4
    // -------------------------------------------------------------------------
    cr4 = __readcr4();
    if (!(cr4 & CR4_VMXE)) {
        cr4 |= CR4_VMXE;
        __writecr4(cr4);
        TRACE("CPU %u: Enabled VMXE in CR4", cpuId);
    }

    // -------------------------------------------------------------------------
    // Step 3: Fix CR0 for VMX requirements
    // -------------------------------------------------------------------------
    cr0 = __readcr0();
    // Apply fixed bits
    cr0 |= initData->VmxCr0Fixed0;
    cr0 &= initData->VmxCr0Fixed1;
    __writecr0(cr0);

    cr4 = __readcr4();
    cr4 |= initData->VmxCr4Fixed0;
    cr4 &= initData->VmxCr4Fixed1;
    __writecr4(cr4);

    TRACE("CPU %u: Applied CR0/CR4 fixed bits", cpuId);

    // -------------------------------------------------------------------------
    // Step 4: Enable VMX operation (VMXON)
    // -------------------------------------------------------------------------
    // Write revision ID to VMXON region
    U32 revisionId = (U32)(initData->VmxBasic & 0x7FFFFFFF);
    *(U32*)cpu->VmxonRegion = revisionId;

    vmxError = __vmx_on(&cpu->VmxonPhysical);
    if (vmxError) {
        ERR("CPU %u: VMXON failed (error %u)", cpuId, vmxError);
        return OMBRA_ERROR_VMXON_FAILED;
    }
    cpu->VmxEnabled = true;
    INFO("CPU %u: VMXON successful", cpuId);

    // -------------------------------------------------------------------------
    // Step 5: Initialize VMCS
    // -------------------------------------------------------------------------
    HV_INIT_PARAMS params = {0};
    params.EptPml4Physical = initData->EptPml4Physical;
    params.DebugBufferPhysical = initData->DebugBufferPhysical;

    status = VmcsInitialize(cpu, &params);
    if (OMBRA_FAILED(status)) {
        ERR("CPU %u: VMCS initialization failed (0x%x)", cpuId, status);
        __vmx_off();
        cpu->VmxEnabled = false;
        return status;
    }
    INFO("CPU %u: VMCS initialized", cpuId);

    // -------------------------------------------------------------------------
    // Step 6: Launch VMX (VMLAUNCH)
    // -------------------------------------------------------------------------
    INFO("CPU %u: Executing VMLAUNCH...", cpuId);

    // AsmVmxLaunch will:
    // 1. Save guest registers
    // 2. Write GUEST_RSP and GUEST_RIP to VMCS
    // 3. Execute VMLAUNCH
    // 4. On success, returns to guest mode at the saved RIP
    // 5. First VM-exit will jump to VmexitHandler

    int launchResult = AsmVmxLaunch();

    if (launchResult == 0) {
        // We're now in VMX root mode as a "guest"
        cpu->VmxLaunched = true;
        INFO("CPU %u: VMLAUNCH successful - hypervisor active!", cpuId);

        // Increment initialized count
        _InterlockedIncrement((volatile long*)&g_InitializedCpus);

        return OMBRA_SUCCESS;
    } else {
        // VMLAUNCH failed
        U64 errorCode = 0;
        __vmx_vmread(VMCS_RODATA_INSTR_ERROR, &errorCode);
        ERR("CPU %u: VMLAUNCH failed - error code %llu", cpuId, errorCode);

        __vmx_off();
        cpu->VmxEnabled = false;
        return OMBRA_ERROR_VMLAUNCH_FAILED;
    }
}

// =============================================================================
// Main Entry Point
// =============================================================================
//
// Called by usermode loader via DrvExecuteOnCpu.
// Returns 0 on success, non-zero error code on failure.

__declspec(dllexport)
int OmbraInitialize(HV_INIT_DATA* initData) {
    OMBRA_STATUS status;

    if (!initData) {
        return -1;
    }

    // Initialize debug logging on first CPU only
    if (initData->CpuId == 0 && initData->DebugBufferPhysical && !g_DebugInitialized) {
        // Convert physical to virtual - for now assume identity mapped
        // In production, would need proper VA translation
        void* debugVa = (void*)initData->DebugBufferPhysical;  // Simplified

        status = DbgInitialize(debugVa, initData->DebugBufferSize);
        if (OMBRA_SUCCESS == status) {
            g_DebugInitialized = true;
            INFO("OmbraHypervisor starting...");
            INFO("Initializing on %u CPUs", initData->TotalCpus);
        }
    }

    // Initialize this CPU
    status = InitializeOnThisCpu(initData);

    if (OMBRA_FAILED(status)) {
        return (int)status;
    }

    return 0;
}

// =============================================================================
// Shutdown Entry Point
// =============================================================================

__declspec(dllexport)
void OmbraShutdown(void) {
    INFO("OmbraHypervisor shutting down...");

    // Each CPU needs to execute VMXOFF
    // This would be called via VMCALL from each virtualized CPU

    for (U32 i = 0; i < 256; i++) {
        VMX_CPU* cpu = &g_CpuContexts[i];
        if (cpu->VmxEnabled) {
            // Would need to be executed on that CPU
            // For now, just mark as disabled
            cpu->VmxEnabled = false;
            cpu->VmxLaunched = false;
        }
    }

    DbgShutdown();
    INFO("Shutdown complete");
}

// =============================================================================
// Helper: Get CPU Context
// =============================================================================

VMX_CPU* VmxGetCurrentCpu(void) {
    // Use CPUID or kernel API to get current processor number
    // For simplicity, use the FS/GS base or processor ID from CPUID

    int cpuInfo[4];
    __cpuidex(cpuInfo, 0x1, 0);

    // Initial APIC ID is in bits 24-31 of EBX
    U32 apicId = (cpuInfo[1] >> 24) & 0xFF;

    // Map APIC ID to our CPU index (simplified - may need proper mapping)
    if (apicId < 256) {
        return &g_CpuContexts[apicId];
    }

    return NULL;
}
