// nested.c — Nested Virtualization Implementation
// OmbraHypervisor

#include "nested.h"
#include "vmcs.h"
#include "debug.h"
#include "../shared/vmcs_fields.h"
#include "../shared/msr_defs.h"
#include "../shared/cpu_defs.h"
#include <intrin.h>
#include <string.h>

// Global nested state (one per system, shared across CPUs)
NESTED_STATE g_NestedState = {0};

// =============================================================================
// CPUID Wrapper
// =============================================================================

void NestedCpuid(U32 leaf, U32 subleaf, U32* eax, U32* ebx, U32* ecx, U32* edx) {
    int cpuInfo[4];
    __cpuidex(cpuInfo, leaf, subleaf);
    if (eax) *eax = cpuInfo[0];
    if (ebx) *ebx = cpuInfo[1];
    if (ecx) *ecx = cpuInfo[2];
    if (edx) *edx = cpuInfo[3];
}

// =============================================================================
// Shadow Resource Management
// =============================================================================

// Allocates shadow VMCS resources (bitmaps and shadow VMCS region)
// Accepts pre-allocated physical addresses from HV_INIT_PARAMS
static OMBRA_STATUS NestedAllocateShadowResources(NESTED_STATE* state, void* vmreadBitmapVa, U64 vmreadBitmapPhys,
                                                    void* vmwriteBitmapVa, U64 vmwriteBitmapPhys,
                                                    void* shadowVmcsVa, U64 shadowVmcsPhys) {
    if (!state) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // VMREAD bitmap (4KB) - controls which VMREAD ops use shadow vs exit
    if (vmreadBitmapVa && vmreadBitmapPhys) {
        state->VmreadBitmap = vmreadBitmapVa;
        state->VmreadBitmapPhys = vmreadBitmapPhys;

        // Clear bitmap (all 0s = shadow all fields, no exits)
        // Setting a bit to 1 = cause VM-exit for that field instead of using shadow
        memset(state->VmreadBitmap, 0, 4096);

        INFO("VMREAD bitmap mapped: VA=0x%p PA=0x%llX", vmreadBitmapVa, vmreadBitmapPhys);
    } else {
        WARN("VMREAD bitmap not provided - shadowing unavailable");
        state->VmreadBitmap = NULL;
        state->VmreadBitmapPhys = 0;
    }

    // VMWRITE bitmap (4KB) - controls which VMWRITE ops use shadow vs exit
    if (vmwriteBitmapVa && vmwriteBitmapPhys) {
        state->VmwriteBitmap = vmwriteBitmapVa;
        state->VmwriteBitmapPhys = vmwriteBitmapPhys;

        // Clear bitmap (all 0s = shadow all writes)
        memset(state->VmwriteBitmap, 0, 4096);

        INFO("VMWRITE bitmap mapped: VA=0x%p PA=0x%llX", vmwriteBitmapVa, vmwriteBitmapPhys);
    } else {
        WARN("VMWRITE bitmap not provided - shadowing unavailable");
        state->VmwriteBitmap = NULL;
        state->VmwriteBitmapPhys = 0;
    }

    // Shadow VMCS (4KB) - used when L2 guest is launched
    if (shadowVmcsVa && shadowVmcsPhys) {
        state->ShadowVmcs = shadowVmcsVa;
        state->ShadowVmcsPhys = shadowVmcsPhys;
        state->ShadowActive = false;  // Not active yet - will activate on L2 launch

        INFO("Shadow VMCS region mapped: VA=0x%p PA=0x%llX", shadowVmcsVa, shadowVmcsPhys);
    } else {
        WARN("Shadow VMCS not provided - L2 guests unavailable");
        state->ShadowVmcs = NULL;
        state->ShadowVmcsPhys = 0;
        state->ShadowActive = false;
    }

    // Check if we have full shadowing support
    if (state->VmreadBitmap && state->VmwriteBitmap && state->ShadowVmcs) {
        INFO("Shadow VMCS resources fully initialized");
        return OMBRA_SUCCESS;
    } else {
        WARN("Shadow VMCS resources partially initialized - some features unavailable");
        return OMBRA_SUCCESS;  // Not fatal, just degraded mode
    }
}

// Frees shadow VMCS resources
// NOTE: In production, memory is owned by usermode - kernel just clears pointers
static void NestedFreeShadowResources(NESTED_STATE* state) {
    if (!state) {
        return;
    }

    // Destroy shadow VMCS if active
    if (state->ShadowActive) {
        NestedDestroyShadowVmcs(state);
    }

    // In production implementation, usermode owns the memory
    // We just clear our pointers
    state->VmreadBitmap = NULL;
    state->VmreadBitmapPhys = 0;
    state->VmwriteBitmap = NULL;
    state->VmwriteBitmapPhys = 0;
    state->ShadowVmcs = NULL;
    state->ShadowVmcsPhys = 0;

    TRACE("Shadow resources released (usermode owns actual memory)");
}

// =============================================================================
// Hypervisor Detection
// =============================================================================

OMBRA_STATUS NestedDetectHypervisor(NESTED_STATE* state, void* vmreadBitmapVa, U64 vmreadBitmapPhys,
                                     void* vmwriteBitmapVa, U64 vmwriteBitmapPhys,
                                     void* shadowVmcsVa, U64 shadowVmcsPhys) {
    U32 eax, ebx, ecx, edx;

    // Zero out state
    memset(state, 0, sizeof(NESTED_STATE));

    // Check CPUID.1.ECX[31] - hypervisor present bit
    NestedCpuid(1, 0, &eax, &ebx, &ecx, &edx);
    if (!(ecx & CPUID_FEAT_ECX_HV)) {
        INFO("No hypervisor detected - bare metal execution");
        state->IsNested = false;
        state->L0Type = HV_TYPE_NONE;
        return OMBRA_SUCCESS;
    }

    INFO("Hypervisor detected via CPUID.1.ECX[31]");
    state->IsNested = true;

    // Query hypervisor vendor ID (CPUID 0x40000000)
    NestedCpuid(CPUID_HV_VENDOR_NEUTRAL, 0, &eax, &ebx, &ecx, &edx);

    // Build vendor string from EBX, ECX, EDX
    char vendor[13] = {0};
    *(U32*)&vendor[0] = ebx;
    *(U32*)&vendor[4] = ecx;
    *(U32*)&vendor[8] = edx;
    vendor[12] = '\0';

    memcpy(state->L0Vendor, vendor, 13);
    INFO("Hypervisor vendor: %s", vendor);

    // Identify hypervisor type
    if (strncmp(vendor, "Microsoft Hv", 12) == 0) {
        state->L0Type = HV_TYPE_HYPERV;
        state->IsHyperV = true;
        INFO("Detected Microsoft Hyper-V");

        // Get Hyper-V specific details
        NestedDetectHyperV(state);

    } else if (strncmp(vendor, "VMwareVMware", 12) == 0) {
        state->L0Type = HV_TYPE_VMWARE;
        INFO("Detected VMware");

    } else if (strncmp(vendor, "KVMKVMKVM", 9) == 0) {
        state->L0Type = HV_TYPE_KVM;
        INFO("Detected KVM");

    } else if (strncmp(vendor, "XenVMMXenVMM", 12) == 0) {
        state->L0Type = HV_TYPE_XEN;
        INFO("Detected Xen");

    } else {
        state->L0Type = HV_TYPE_UNKNOWN;
        WARN("Unknown hypervisor vendor: %s", vendor);
    }

    // Store in global state
    memcpy(&g_NestedState, state, sizeof(NESTED_STATE));

    // Wire up shadow VMCS resources from loader-provided addresses
    NestedAllocateShadowResources(state, vmreadBitmapVa, vmreadBitmapPhys,
                                   vmwriteBitmapVa, vmwriteBitmapPhys,
                                   shadowVmcsVa, shadowVmcsPhys);

    return OMBRA_SUCCESS;
}

void NestedCleanup(void) {
    NestedFreeShadowResources(&g_NestedState);
    memset(&g_NestedState, 0, sizeof(NESTED_STATE));
    TRACE("Nested virtualization state cleaned up");
}

bool NestedIsRunningNested(void) {
    return g_NestedState.IsNested;
}

HYPERVISOR_TYPE NestedGetL0Type(void) {
    return g_NestedState.L0Type;
}

// =============================================================================
// Hyper-V Specific Detection
// =============================================================================

OMBRA_STATUS NestedDetectHyperV(NESTED_STATE* state) {
    U32 eax, ebx, ecx, edx;

    if (!state->IsHyperV) {
        return OMBRA_ERROR_NOT_SUPPORTED;
    }

    // CPUID 0x40000001 - Hyper-V interface signature
    NestedCpuid(CPUID_HV_INTERFACE, 0, &eax, &ebx, &ecx, &edx);
    state->HyperVInterface = eax;
    INFO("Hyper-V interface: 0x%X", eax);

    // CPUID 0x40000002 - Hyper-V version
    NestedCpuid(CPUID_HV_VERSION, 0, &eax, &ebx, &ecx, &edx);
    state->HyperVVersion = ((U64)ebx << 32) | eax;
    INFO("Hyper-V version: %u.%u (build %u.%u)",
         (ebx >> 16) & 0xFFFF, ebx & 0xFFFF,
         (eax >> 16) & 0xFFFF, eax & 0xFFFF);

    // CPUID 0x40000003 - Feature identification
    NestedCpuid(CPUID_HV_FEATURES, 0, &eax, &ebx, &ecx, &edx);
    state->HyperVFeatures = ((U64)ebx << 32) | eax;
    INFO("Hyper-V features: EAX=0x%X EBX=0x%X", eax, ebx);

    // Check if hypercall MSR is available
    if (eax & BIT(5)) {  // MSR-based hypercalls available
        // Read hypercall page MSR (if enabled)
        U64 hypercallMsr = __readmsr(HV_X64_MSR_HYPERCALL);
        if (hypercallMsr & BIT(0)) {  // Bit 0 = enabled
            state->HypercallPage = hypercallMsr & ~0xFFFULL;  // Bits 63:12 = PFN
            INFO("Hyper-V hypercall page: 0x%llX", state->HypercallPage);
        } else {
            TRACE("Hyper-V hypercall MSR not enabled");
        }
    }

    return OMBRA_SUCCESS;
}

bool NestedIsHyperV(void) {
    return g_NestedState.IsHyperV;
}

U64 NestedGetHyperVFeatures(void) {
    return g_NestedState.HyperVFeatures;
}

// =============================================================================
// VMCS Shadowing
// =============================================================================

OMBRA_STATUS NestedInitVmcsShadowing(VMX_CPU* cpu, NESTED_STATE* state) {
    // Check if VMCS shadowing is supported
    U64 procBased2 = __readmsr(MSR_IA32_VMX_PROCBASED_CTLS2);
    if (!(procBased2 & (CPU2_VMCS_SHADOWING << 32))) {  // Check allowed1 bits
        TRACE("VMCS shadowing not supported by hardware");
        state->VmcsShadowingAvailable = false;
        return OMBRA_ERROR_NOT_SUPPORTED;
    }

    INFO("VMCS shadowing is supported");
    state->VmcsShadowingAvailable = true;

    // Allocate VMREAD/VMWRITE bitmaps (4KB each)
    // In production, these would be allocated from non-paged pool
    // For now, assume pre-allocated by caller
    if (!state->VmreadBitmap || !state->VmwriteBitmap) {
        ERR("Shadow bitmaps not allocated");
        return OMBRA_ERROR_VMCS_FAILED;
    }

    // Clear bitmaps (all 0s = shadow all fields)
    // Setting a bit to 1 = cause VM-exit instead of using shadow
    memset(state->VmreadBitmap, 0, 4096);
    memset(state->VmwriteBitmap, 0, 4096);

    // Configure which fields should cause exits instead of shadowing
    // For now, shadow everything (bitmap stays 0)
    // In practice, you'd set bits for fields you want to intercept

    return OMBRA_SUCCESS;
}

OMBRA_STATUS NestedEnableVmcsShadowing(VMX_CPU* cpu) {
    NESTED_STATE* state = &g_NestedState;

    if (!state->VmcsShadowingAvailable) {
        return OMBRA_ERROR_NOT_SUPPORTED;
    }

    // Enable VMCS shadowing in secondary processor-based controls
    U32 procBased2 = (U32)VmcsRead(VMCS_CTRL_PROC_BASED2);
    procBased2 |= CPU2_VMCS_SHADOWING;
    VmcsWrite(VMCS_CTRL_PROC_BASED2, procBased2);

    // Set VMREAD bitmap address
    VmcsWrite(VMCS_CTRL_VMREAD_BITMAP, state->VmreadBitmapPhys);

    // Set VMWRITE bitmap address
    VmcsWrite(VMCS_CTRL_VMWRITE_BITMAP, state->VmwriteBitmapPhys);

    // Link to shadow VMCS (if we have one active)
    if (state->ShadowActive && state->ShadowVmcsPhys) {
        VmcsWrite(VMCS_GUEST_VMCS_LINK, state->ShadowVmcsPhys);
    }

    INFO("VMCS shadowing enabled");
    return OMBRA_SUCCESS;
}

void NestedDisableVmcsShadowing(VMX_CPU* cpu) {
    // Disable VMCS shadowing
    U32 procBased2 = (U32)VmcsRead(VMCS_CTRL_PROC_BASED2);
    procBased2 &= ~CPU2_VMCS_SHADOWING;
    VmcsWrite(VMCS_CTRL_PROC_BASED2, procBased2);

    // Reset link pointer
    VmcsWrite(VMCS_GUEST_VMCS_LINK, 0xFFFFFFFFFFFFFFFF);

    TRACE("VMCS shadowing disabled");
}

// =============================================================================
// Shadow VMCS Management
// =============================================================================

OMBRA_STATUS NestedCreateShadowVmcs(NESTED_STATE* state) {
    U64 vmxBasic;
    U8 error;

    // Shadow VMCS must be pre-allocated (4KB, 4KB aligned)
    if (!state->ShadowVmcs || !state->ShadowVmcsPhys) {
        ERR("Shadow VMCS memory not allocated");
        return OMBRA_ERROR_VMCS_FAILED;
    }

    // Get VMX revision ID
    vmxBasic = __readmsr(MSR_IA32_VMX_BASIC);

    // Write revision ID with shadow-VMCS indicator (bit 31 set)
    *(U32*)state->ShadowVmcs = ((U32)vmxBasic) | BIT(31);

    // Clear the shadow VMCS
    error = __vmx_vmclear(&state->ShadowVmcsPhys);
    if (error) {
        ERR("VMCLEAR shadow VMCS failed: %u", error);
        return OMBRA_ERROR_VMCS_FAILED;
    }

    state->ShadowActive = true;
    INFO("Shadow VMCS created at phys 0x%llX", state->ShadowVmcsPhys);

    return OMBRA_SUCCESS;
}

void NestedDestroyShadowVmcs(NESTED_STATE* state) {
    if (state->ShadowActive) {
        // Just clear the shadow VMCS
        __vmx_vmclear(&state->ShadowVmcsPhys);
        state->ShadowActive = false;
        TRACE("Shadow VMCS destroyed");
    }
}

OMBRA_STATUS NestedSyncShadowVmcs(VMX_CPU* cpu, NESTED_STATE* state) {
    // Synchronize L1 VMCS fields to shadow VMCS for L2 guest
    // This would copy relevant guest state from our VMCS to the shadow

    if (!state->ShadowActive) {
        return OMBRA_ERROR_VMCS_FAILED;
    }

    // Save current VMCS pointer
    U64 currentVmcs = cpu->VmcsPhysical;

    // Load shadow VMCS
    U8 error = __vmx_vmptrld(&state->ShadowVmcsPhys);
    if (error) {
        ERR("Failed to load shadow VMCS: %u", error);
        return OMBRA_ERROR_VMCS_FAILED;
    }

    // TODO: Copy guest state fields from L1 VMCS to shadow
    // This would be done via VMREAD from current + VMWRITE to shadow
    // For now, this is a stub

    // Restore original VMCS
    __vmx_vmptrld(&currentVmcs);

    return OMBRA_SUCCESS;
}

// =============================================================================
// L1/L2 Exit Handling
// =============================================================================

bool NestedShouldForwardToL0(U32 exitReason, U64 qualification) {
    // Determine if this VM-exit should be forwarded to L0 hypervisor
    // or handled by us (L1)

    // If not nested, we handle everything
    if (!g_NestedState.IsNested) {
        return false;
    }

    // For now, handle most exits ourselves
    // In a full nested implementation, you'd forward:
    // - VMX instruction exits (VMXON, VMXOFF, etc)
    // - Certain exceptions/interrupts
    // - External interrupt exits (if L0 manages interrupts)

    switch (exitReason) {
        // Always forward VMX instruction exits
        case 18:  // VMCALL
        case 19:  // VMCLEAR
        case 20:  // VMLAUNCH
        case 21:  // VMPTRLD
        case 22:  // VMPTRST
        case 23:  // VMREAD
        case 24:  // VMRESUME
        case 25:  // VMWRITE
        case 26:  // VMXOFF
        case 27:  // VMXON
            return true;

        // Handle everything else at L1
        default:
            return false;
    }
}

OMBRA_STATUS NestedHandleVmxInstruction(VMX_CPU* cpu, U32 exitReason) {
    NESTED_STATE* state = &g_NestedState;

    TRACE("Nested VMX instruction exit: reason %u", exitReason);

    // If shadow VMCS is active and this is VMREAD/VMWRITE, handle specially
    if (state->ShadowActive && state->ShadowVmcs) {
        switch (exitReason) {
            case 23:  // VMREAD
                // Guest executed VMREAD on L2 VMCS
                // The shadow VMCS should handle this automatically, but we got an exit
                // This means the field was marked to cause an exit in VMREAD bitmap
                TRACE("VMREAD exit while shadow VMCS active - field marked for interception");
                // Could emulate the VMREAD here if needed, or just pass through
                // For now, inject #UD as we don't support L2 guests yet
                goto inject_ud;

            case 25:  // VMWRITE
                // Guest executed VMWRITE on L2 VMCS
                TRACE("VMWRITE exit while shadow VMCS active - field marked for interception");
                // Could emulate the VMWRITE here if needed
                goto inject_ud;

            default:
                // Other VMX instructions fall through to normal handling
                break;
        }
    }

    // Handle VMX instructions based on nested scenario
    switch (exitReason) {
        case 18:  // VMCALL
            // VMCALL from guest - might be hypercall to L0 or to us
            // If we're nested under Hyper-V, this could be a Hyper-V hypercall
            if (state->IsHyperV) {
                // Check if this is a Hyper-V hypercall (RCX contains hypercall code)
                // For ZeroHVCI scenario, we intercept Hyper-V hypercalls
                TRACE("VMCALL in Hyper-V nested mode - checking if it's a hypercall");
                // For now, just advance RIP - full hypercall emulation would go here
                goto advance_rip;
            } else {
                // Not nested, or guest is calling us directly
                // This would be handled by HandleVmcall() normally
                goto advance_rip;
            }

        case 19:  // VMCLEAR
        case 20:  // VMLAUNCH
        case 21:  // VMPTRLD
        case 22:  // VMPTRST
        case 23:  // VMREAD
        case 24:  // VMRESUME
        case 25:  // VMWRITE
        case 26:  // VMXOFF
        case 27:  // VMXON
            // VMX management instructions
            // If we're running nested, we need to emulate these for L2 guests
            if (state->IsNested) {
                TRACE("VMX instruction %u from L1 guest - emulation needed for L2 support", exitReason);
                // For now, inject #UD since we don't fully support L2 guests yet
                // Full implementation would emulate these using shadow VMCS
                goto inject_ud;
            } else {
                // Bare metal - these instructions should never execute
                // We hide VMX capability via CPUID spoofing
                WARN("VMX instruction %u on bare metal - guest bypassed CPUID check", exitReason);
                goto inject_ud;
            }

        case 50:  // INVEPT
        case 53:  // INVVPID
            // EPT/VPID invalidation instructions
            if (state->IsNested) {
                // Forward to L0 or emulate for L2
                TRACE("INV%s from nested guest", exitReason == 50 ? "EPT" : "VPID");
                // For now, just advance RIP - these are NOPs for us
                goto advance_rip;
            } else {
                goto inject_ud;
            }

        default:
            WARN("Unknown VMX instruction exit reason: %u", exitReason);
            goto inject_ud;
    }

advance_rip:
    {
        U64 guestRip = VmcsRead(VMCS_GUEST_RIP);
        U64 instrLen = VmcsRead(VMCS_EXIT_INSTR_LEN);
        VmcsWrite(VMCS_GUEST_RIP, guestRip + instrLen);
        return OMBRA_SUCCESS;
    }

inject_ud:
    {
        // Inject #UD (invalid opcode exception)
        U32 intInfo = 6;                    // #UD vector
        intInfo |= (3 << 8);                // Hardware exception type
        intInfo |= (1UL << 31);             // Valid bit
        VmcsWrite(VMCS_CTRL_VMENTRY_INT_INFO, intInfo);
        VmcsWrite(VMCS_CTRL_VMENTRY_INSTR_LEN, VmcsRead(VMCS_EXIT_INSTR_LEN));
        // Don't advance RIP - exception delivery will handle it
        return OMBRA_SUCCESS;
    }
}

// =============================================================================
// Hyper-V Coexistence
// =============================================================================

OMBRA_STATUS NestedSetupHyperVCoexistence(NESTED_STATE* state) {
    if (!state->IsHyperV) {
        return OMBRA_ERROR_NOT_SUPPORTED;
    }

    INFO("Setting up Hyper-V coexistence mode");

    // In ZeroHVCI scenario, we've already hijacked Hyper-V's VMExit handler
    // Our payload is now inserted in the exit path
    // We need to:
    // 1. Intercept relevant hypercalls
    // 2. Filter CPUID to hide our presence
    // 3. Maintain timing to avoid detection

    // Check if we have access to the hypercall page
    if (!state->HypercallPage) {
        WARN("Hyper-V hypercall page not available");
        // This is okay - we're hijacking at the exit handler level
    }

    return OMBRA_SUCCESS;
}

OMBRA_STATUS NestedHookHyperVHypercall(NESTED_STATE* state) {
    // Hook Hyper-V hypercall mechanism
    // In ZeroHVCI, this is done by the loader/injector
    // This function would set up any additional interception needed

    if (!state->IsHyperV || !state->HypercallPage) {
        return OMBRA_ERROR_NOT_SUPPORTED;
    }

    // TODO: Set up EPT hooks on hypercall page if needed
    // For now, we rely on the ZeroHVCI exit handler injection

    TRACE("Hyper-V hypercall interception configured");
    return OMBRA_SUCCESS;
}

void NestedUnhookHyperVHypercall(NESTED_STATE* state) {
    // Remove hypercall hooks
    // In ZeroHVCI, the loader handles cleanup
    TRACE("Hyper-V hypercall hooks removed");
}

// =============================================================================
// Utilities
// =============================================================================

const char* NestedGetHypervisorName(HYPERVISOR_TYPE type) {
    switch (type) {
        case HV_TYPE_NONE:      return "None (Bare Metal)";
        case HV_TYPE_HYPERV:    return "Microsoft Hyper-V";
        case HV_TYPE_VMWARE:    return "VMware";
        case HV_TYPE_KVM:       return "KVM";
        case HV_TYPE_XEN:       return "Xen";
        case HV_TYPE_UNKNOWN:   return "Unknown";
        default:                return "Invalid";
    }
}
