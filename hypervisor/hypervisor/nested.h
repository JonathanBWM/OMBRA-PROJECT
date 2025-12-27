// nested.h — Nested Virtualization Support (L1/L2 handling)
// OmbraHypervisor
// Handles detection and coexistence with L0 hypervisors (Hyper-V)

#ifndef OMBRA_NESTED_H
#define OMBRA_NESTED_H

#include "../shared/types.h"
#include "vmx.h"

// =============================================================================
// Hypervisor Detection
// =============================================================================

// Detected hypervisor types
typedef enum _HYPERVISOR_TYPE {
    HV_TYPE_NONE = 0,           // No hypervisor detected
    HV_TYPE_HYPERV,             // Microsoft Hyper-V
    HV_TYPE_VMWARE,             // VMware
    HV_TYPE_KVM,                // KVM
    HV_TYPE_XEN,                // Xen
    HV_TYPE_UNKNOWN             // Unknown hypervisor present
} HYPERVISOR_TYPE;

// Hypervisor vendor signatures (CPUID 0x40000000.EBX:EDX:ECX)
#define HV_VENDOR_HYPERV    "Microsoft Hv"     // Hyper-V
#define HV_VENDOR_VMWARE    "VMwareVMware"     // VMware
#define HV_VENDOR_KVM       "KVMKVMKVM\0\0\0"  // KVM
#define HV_VENDOR_XEN       "XenVMMXenVMM"     // Xen

// Hyper-V CPUID leaves
#define CPUID_HV_VENDOR_NEUTRAL     0x40000000  // Hypervisor vendor ID
#define CPUID_HV_INTERFACE          0x40000001  // Hypervisor interface signature
#define CPUID_HV_VERSION            0x40000002  // Hypervisor version
#define CPUID_HV_FEATURES           0x40000003  // Feature identification
#define CPUID_HV_ENLIGHTENMENTS     0x40000004  // Implementation recommendations
#define CPUID_HV_LIMITS             0x40000005  // Implementation limits
#define CPUID_HV_HARDWARE_FEATURES  0x40000006  // Hardware features

// Hyper-V hypercall MSRs
#define HV_X64_MSR_GUEST_OS_ID      0x40000000
#define HV_X64_MSR_HYPERCALL        0x40000001
#define HV_X64_MSR_VP_INDEX         0x40000002
#define HV_X64_MSR_RESET            0x40000003

// =============================================================================
// Nested Virtualization State
// =============================================================================

typedef struct _NESTED_STATE {
    // Detection results
    bool            IsNested;           // Running under another hypervisor
    HYPERVISOR_TYPE L0Type;             // Type of L0 hypervisor
    char            L0Vendor[13];       // Vendor string (12 chars + null)

    // Hyper-V specific
    bool            IsHyperV;           // Specifically Hyper-V
    U64             HyperVInterface;    // Interface signature
    U64             HyperVVersion;      // Version info
    U64             HyperVFeatures;     // Feature bits
    U64             HypercallPage;      // Hypercall page physical address

    // VMCS shadowing support
    bool            VmcsShadowingAvailable;
    void*           VmreadBitmap;       // Bitmap for shadow VMREAD (4KB)
    U64             VmreadBitmapPhys;
    void*           VmwriteBitmap;      // Bitmap for shadow VMWRITE (4KB)
    U64             VmwriteBitmapPhys;

    // Shadow VMCS for L2 guests
    void*           ShadowVmcs;         // Shadow VMCS region
    U64             ShadowVmcsPhys;
    bool            ShadowActive;

    // Nested exit tracking
    U64             L0ExitCount;        // Exits to L0
    U64             L1ExitCount;        // Exits handled by us (L1)

} NESTED_STATE;

// =============================================================================
// Function Declarations
// =============================================================================

// Detection
OMBRA_STATUS NestedDetectHypervisor(NESTED_STATE* state, void* vmreadBitmapVa, U64 vmreadBitmapPhys,
                                     void* vmwriteBitmapVa, U64 vmwriteBitmapPhys,
                                     void* shadowVmcsVa, U64 shadowVmcsPhys);
void NestedCleanup(void);  // Cleanup shadow resources
bool NestedIsRunningNested(void);
HYPERVISOR_TYPE NestedGetL0Type(void);

// Hyper-V specific detection
OMBRA_STATUS NestedDetectHyperV(NESTED_STATE* state);
bool NestedIsHyperV(void);
U64 NestedGetHyperVFeatures(void);

// VMCS shadowing
OMBRA_STATUS NestedInitVmcsShadowing(VMX_CPU* cpu, NESTED_STATE* state);
OMBRA_STATUS NestedEnableVmcsShadowing(VMX_CPU* cpu);
void NestedDisableVmcsShadowing(VMX_CPU* cpu);

// Shadow VMCS management
OMBRA_STATUS NestedCreateShadowVmcs(NESTED_STATE* state);
void NestedDestroyShadowVmcs(NESTED_STATE* state);
OMBRA_STATUS NestedSyncShadowVmcs(VMX_CPU* cpu, NESTED_STATE* state);

// L1/L2 exit handling
bool NestedShouldForwardToL0(U32 exitReason, U64 qualification);
OMBRA_STATUS NestedHandleVmxInstruction(VMX_CPU* cpu, U32 exitReason);

// Hyper-V coexistence
OMBRA_STATUS NestedSetupHyperVCoexistence(NESTED_STATE* state);
OMBRA_STATUS NestedHookHyperVHypercall(NESTED_STATE* state);
void NestedUnhookHyperVHypercall(NESTED_STATE* state);

// Utilities
void NestedCpuid(U32 leaf, U32 subleaf, U32* eax, U32* ebx, U32* ecx, U32* edx);
const char* NestedGetHypervisorName(HYPERVISOR_TYPE type);

#endif // OMBRA_NESTED_H
