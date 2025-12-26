#include "cpuid_spoof.h"
#include <intrin.h>

namespace cpuid_spoof {

//===----------------------------------------------------------------------===//
// Global Configuration
//===----------------------------------------------------------------------===//

Config g_config;

//===----------------------------------------------------------------------===//
// Initialization
//===----------------------------------------------------------------------===//

void Initialize() {
    // Default: full stealth mode
    g_config.hide_hypervisor = true;
    g_config.hide_vmx = true;
    g_config.hide_svm = true;
    g_config.spoof_hv_vendor = true;

    // Future phases (disabled by default)
    g_config.compensate_tsc = false;
    g_config.cpuid_tsc_offset = 0;
    g_config.spoof_brand_string = false;
}

//===----------------------------------------------------------------------===//
// CPUID Execution and Spoofing
//===----------------------------------------------------------------------===//

void ExecuteAndSpoof(u32 leaf, u32 subleaf, u32* eax, u32* ebx, u32* ecx, u32* edx) {
    // Execute REAL CPUID in VMXRoot
    // This works because CPUID executes natively in VMXRoot (no VMExit)
    // Confirmed by existing usage at vmx_handler.cpp:64 for APIC ID detection
    int cpuInfo[4] = {0};
    __cpuidex(cpuInfo, static_cast<int>(leaf), static_cast<int>(subleaf));

    // Start with real values
    *eax = static_cast<u32>(cpuInfo[0]);
    *ebx = static_cast<u32>(cpuInfo[1]);
    *ecx = static_cast<u32>(cpuInfo[2]);
    *edx = static_cast<u32>(cpuInfo[3]);

    // Apply spoofing based on leaf
    switch (leaf) {
        case 0x00000001:
            // Standard Feature Information - Primary detection vector!
            // Anti-cheats check ECX[31] to detect hypervisor presence
            if (g_config.hide_hypervisor)
                *ecx &= ~CPUID_HV_PRESENT;   // Clear hypervisor bit (ECX[31])
            if (g_config.hide_vmx)
                *ecx &= ~CPUID_VMX_FEATURE;  // Clear VMX capability (ECX[5])
            break;

        case 0x80000001:
            // Extended Feature Information (AMD)
            // Anti-cheats check for SVM capability on AMD
            if (g_config.hide_svm)
                *ecx &= ~CPUID_SVM_FEATURE;  // Clear SVM capability (ECX[2])
            break;

        case CPUID_HV_VENDOR:  // 0x40000000 - Hypervisor Vendor ID
            // Anti-cheats check for "Microsoft Hv" or other vendor strings
            if (g_config.spoof_hv_vendor) {
                // Zero vendor string (EBX/ECX/EDX contain "Microsoft Hv")
                // BUT PRESERVE max leaf in EAX - Windows enlightenments need this!
                // Without max leaf, Windows can't query enlightenment features
                *ebx = 0;
                *ecx = 0;
                *edx = 0;
                // *eax stays intact (max hypervisor leaf)
            }
            break;

        // CRITICAL: Do NOT modify leaves 0x40000001-0x4000000F!
        // These are Windows enlightenments that WILL break things:
        // - 0x40000001: Hypercall page setup
        // - 0x40000003: Feature flags (SynIC, timers, APIC virtualization)
        // - 0x40000004: Implementation limits (resource allocation)
        // Let them pass through with real values.

        default:
            // All other leaves: return real values unchanged
            // This includes:
            // - 0x00000000: Vendor ID (GenuineIntel/AuthenticAMD) - unchanged
            // - 0x00000007: Extended features - unchanged
            // - 0x80000002-4: Processor brand string - unchanged
            // - 0x40000001+: Windows enlightenments - PRESERVED
            break;
    }
}

} // namespace cpuid_spoof
