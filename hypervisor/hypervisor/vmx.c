// vmx.c — VMX Operations Implementation
// OmbraHypervisor

#include "vmx.h"
#include "vmcs.h"
#include "ept.h"
#include "../shared/msr_defs.h"
#include "../shared/cpu_defs.h"
#include "../shared/vmcs_fields.h"
#include <intrin.h>

// =============================================================================
// Global State
// =============================================================================

OMBRA_STATE g_Ombra = {0};

// =============================================================================
// VMX Support Check
// =============================================================================

OMBRA_STATUS VmxCheckSupport(void) {
    int info[4];

    // Check CPUID.1:ECX.VMX[bit 5]
    __cpuid(info, 1);
    if (!(info[2] & CPUID_FEAT_ECX_VMX)) {
        return OMBRA_ERROR_NOT_SUPPORTED;
    }

    // Check if hypervisor already running (bit 31)
    if (info[2] & CPUID_FEAT_ECX_HV) {
        return OMBRA_ERROR_ALREADY_RUNNING;
    }

    // Check IA32_FEATURE_CONTROL
    U64 featCtrl = __readmsr(MSR_IA32_FEATURE_CONTROL);

    // If locked and VMX disabled, we can't enable it
    if ((featCtrl & FEATURE_CONTROL_LOCK) && !(featCtrl & FEATURE_CONTROL_VMX)) {
        return OMBRA_ERROR_VMX_DISABLED;
    }

    return OMBRA_SUCCESS;
}

bool VmxIsSupported(void) {
    return VmxCheckSupport() == OMBRA_SUCCESS;
}

// =============================================================================
// Control Adjustment
// =============================================================================

// Adjust 32-bit controls based on capability MSR
// Low 32 bits = allowed 0-settings
// High 32 bits = allowed 1-settings
U32 AdjustControls(U32 requested, U32 msr) {
    U64 cap = __readmsr(msr);
    U32 allowed0 = (U32)cap;        // Bits that CAN be 0
    U32 allowed1 = (U32)(cap >> 32); // Bits that CAN be 1

    // Set bits that MUST be 1
    requested |= allowed0;

    // Clear bits that MUST be 0
    requested &= allowed1;

    return requested;
}

// Same for 64-bit controls
U64 AdjustControls64(U64 requested, U32 msr) {
    U64 cap = __readmsr(msr);
    U64 allowed0 = cap & 0xFFFFFFFF;
    U64 allowed1 = cap >> 32;

    requested |= allowed0;
    requested &= allowed1;

    return requested;
}

// =============================================================================
// Segment Helpers
// =============================================================================

static SEGMENT_DESCRIPTOR* GetGdtEntry(U16 selector) {
    DESCRIPTOR_TABLE_REG gdtr;
    AsmReadGdtr(&gdtr);

    // Selector format: Index[15:3], TI[2], RPL[1:0]
    U16 index = (selector >> 3) & 0x1FFF;

    if (index == 0) {
        return NULL;  // Null selector
    }

    return (SEGMENT_DESCRIPTOR*)(gdtr.Base + index * 8);
}

U64 GetSegmentBase(U16 selector) {
    SEGMENT_DESCRIPTOR* desc = GetGdtEntry(selector);
    if (!desc) return 0;

    U64 base = desc->BaseLow | ((U64)desc->BaseMiddle << 16) | ((U64)desc->BaseHigh << 24);

    // Check if this is a system segment (TSS, LDT) - they have extended base in 64-bit mode
    if (!(desc->Access & 0x10)) {  // S bit = 0 means system segment
        SEGMENT_DESCRIPTOR_64* desc64 = (SEGMENT_DESCRIPTOR_64*)desc;
        base |= ((U64)desc64->BaseUpper << 32);
    }

    return base;
}

U32 GetSegmentLimit(U16 selector) {
    SEGMENT_DESCRIPTOR* desc = GetGdtEntry(selector);
    if (!desc) return 0;

    U32 limit = desc->LimitLow | ((U32)(desc->LimitHighAndFlags & 0x0F) << 16);

    // Check granularity bit
    if (desc->LimitHighAndFlags & 0x80) {
        limit = (limit << 12) | 0xFFF;
    }

    return limit;
}

U32 GetSegmentAccessRights(U16 selector) {
    if (selector == 0) {
        return SEG_ACCESS_UNUSABLE;
    }

    SEGMENT_DESCRIPTOR* desc = GetGdtEntry(selector);
    if (!desc) {
        return SEG_ACCESS_UNUSABLE;
    }

    // Build access rights from descriptor
    // Bits 7:0 from Access byte
    // Bits 15:12 from flags (high nibble of LimitHighAndFlags)
    U32 ar = desc->Access;
    ar |= ((U32)(desc->LimitHighAndFlags & 0xF0) << 8);

    // Clear undefined bits
    ar &= 0xF0FF;

    return ar;
}

void GetSegmentState(U16 selector, SEGMENT_STATE* state) {
    state->Selector = selector;
    state->Base = GetSegmentBase(selector);
    state->Limit = GetSegmentLimit(selector);
    state->AccessRights = GetSegmentAccessRights(selector);
}

// =============================================================================
// VMX Enable/Disable
// =============================================================================

OMBRA_STATUS VmxEnable(VMX_CPU* cpu) {
    // Set CR4.VMXE
    U64 cr4 = __readcr4();
    cr4 |= CR4_VMXE;
    __writecr4(cr4);

    // Adjust CR0 per VMX requirements
    U64 cr0 = __readcr0();
    cr0 |= __readmsr(MSR_IA32_VMX_CR0_FIXED0);
    cr0 &= __readmsr(MSR_IA32_VMX_CR0_FIXED1);
    __writecr0(cr0);

    // Adjust CR4 per VMX requirements (already has VMXE set)
    cr4 = __readcr4();
    cr4 |= __readmsr(MSR_IA32_VMX_CR4_FIXED0);
    cr4 &= __readmsr(MSR_IA32_VMX_CR4_FIXED1);
    __writecr4(cr4);

    // Write VMCS revision ID to VMXON region
    U64 vmxBasic = __readmsr(MSR_IA32_VMX_BASIC);
    *(U32*)cpu->VmxonRegion = (U32)vmxBasic;

    // Execute VMXON
    U8 error = __vmx_on(&cpu->VmxonPhysical);
    if (error) {
        // Restore CR4
        __writecr4(__readcr4() & ~CR4_VMXE);
        return OMBRA_ERROR_VMXON_FAILED;
    }

    cpu->VmxEnabled = true;
    return OMBRA_SUCCESS;
}

OMBRA_STATUS VmxDisable(VMX_CPU* cpu) {
    if (!cpu->VmxEnabled) {
        return OMBRA_SUCCESS;
    }

    // Execute VMXOFF
    __vmx_off();

    // Clear CR4.VMXE
    __writecr4(__readcr4() & ~CR4_VMXE);

    cpu->VmxEnabled = false;
    cpu->VmcsLoaded = false;

    return OMBRA_SUCCESS;
}

// =============================================================================
// VMCS Read/Write
// =============================================================================

U64 VmcsRead(U32 field) {
    U64 value = 0;
    __vmx_vmread(field, &value);
    return value;
}

void VmcsWrite(U32 field, U64 value) {
    __vmx_vmwrite(field, value);
}

// =============================================================================
// Per-CPU Initialization
// =============================================================================

OMBRA_STATUS VmxInitializeCpu(HV_INIT_PARAMS* params) {
    VMX_CPU* cpu;
    OMBRA_STATUS status;

    // Allocate VMX_CPU structure
    // In a real implementation, this would use non-paged pool
    // For now, we assume the caller has pre-allocated this
    static VMX_CPU cpuStorage[MAX_CPUS];
    cpu = &cpuStorage[params->CpuId];

    // Initialize structure
    cpu->CpuId = params->CpuId;
    cpu->Virtualized = false;

    cpu->VmxonRegion = params->VmxonVirtual;
    cpu->VmxonPhysical = params->VmxonPhysical;

    cpu->VmcsRegion = params->VmcsVirtual;
    cpu->VmcsPhysical = params->VmcsPhysical;

    cpu->HostStackTop = (void*)params->HostStackTop;
    cpu->HostStackSize = 0x2000;  // 8KB

    cpu->MsrBitmap = params->MsrBitmapVirtual;
    cpu->MsrBitmapPhysical = params->MsrBitmapPhysical;

    cpu->VmxEnabled = false;
    cpu->VmcsLoaded = false;

    cpu->TscOffset = 0;
    cpu->LastTsc = 0;
    cpu->VmexitCount = 0;

    cpu->Self = cpu;

    // Store in global array
    g_Ombra.Cpus[params->CpuId] = cpu;

    // Check VMX support
    status = VmxCheckSupport();
    if (OMBRA_FAILED(status)) {
        return status;
    }

    // Enable VMX
    status = VmxEnable(cpu);
    if (OMBRA_FAILED(status)) {
        return status;
    }

    // Initialize VMCS
    status = VmcsInitialize(cpu, params);
    if (OMBRA_FAILED(status)) {
        VmxDisable(cpu);
        return status;
    }

    // Launch VM
    status = VmxLaunchCpu(cpu);
    if (OMBRA_FAILED(status)) {
        VmxDisable(cpu);
        return status;
    }

    cpu->Virtualized = true;
    return OMBRA_SUCCESS;
}

// =============================================================================
// VM Launch
// =============================================================================

OMBRA_STATUS VmxLaunchCpu(VMX_CPU* cpu) {
    // The VMCS should already be loaded and configured
    // Guest RIP and RSP need to be set to continue execution after VMLAUNCH

    // Get current RSP and RIP for guest state
    // The trick: after VMLAUNCH succeeds, we continue as guest at the next instruction
    // So we set guest RIP to the return address from this function

    // This is handled in assembly - AsmVmxLaunch captures the state
    U64 result = AsmVmxLaunch();

    if (result != 0) {
        // VMLAUNCH failed
        // Read error code
        U64 errorCode = VmcsRead(VMCS_EXIT_INSTRUCTION_ERROR);
        (void)errorCode;  // Could log this

        return OMBRA_ERROR_VMLAUNCH_FAILED;
    }

    // If we get here, we're running as guest
    // (VMLAUNCH succeeded and we continued as guest)
    return OMBRA_SUCCESS;
}

// =============================================================================
// Get Current CPU
// =============================================================================

VMX_CPU* VmxGetCurrentCpu(void) {
    // In a multi-CPU scenario, we'd read the processor ID
    // For now, simple implementation using CPUID
    int info[4];
    __cpuid(info, 1);
    U32 apicId = (info[1] >> 24) & 0xFF;

    // Map APIC ID to our CPU index
    // This is simplified - real implementation would build a proper mapping
    for (U32 i = 0; i < MAX_CPUS; i++) {
        if (g_Ombra.Cpus[i] && g_Ombra.Cpus[i]->CpuId == apicId) {
            return g_Ombra.Cpus[i];
        }
    }

    // Fallback: assume sequential CPU IDs
    if (apicId < MAX_CPUS && g_Ombra.Cpus[apicId]) {
        return g_Ombra.Cpus[apicId];
    }

    return NULL;
}
