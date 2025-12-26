#pragma once
#include "../include/types.h"

namespace cpuid_spoof {

//===----------------------------------------------------------------------===//
// Constants
//===----------------------------------------------------------------------===//

// Magic hypercall leaf - used for authenticated VMCALLs
constexpr u32 HYPERCALL_MAGIC_LEAF = 0x13371337;

// Feature bits to mask
constexpr u32 CPUID_HV_PRESENT = (1u << 31);   // CPUID.01H:ECX[31] - Hypervisor Present
constexpr u32 CPUID_VMX_FEATURE = (1u << 5);   // CPUID.01H:ECX[5]  - VMX (Intel)
constexpr u32 CPUID_SVM_FEATURE = (1u << 2);   // CPUID.80000001H:ECX[2] - SVM (AMD)

// Hypervisor vendor CPUID leaf
constexpr u32 CPUID_HV_VENDOR = 0x40000000;

//===----------------------------------------------------------------------===//
// CRITICAL: Windows Enlightenment Leaves - DO NOT MODIFY!
//===----------------------------------------------------------------------===//
// Leaves 0x40000001-0x4000000F are Windows enlightenments that WILL break things:
//
// | Leaf       | Feature                | Impact if Zeroed                           |
// |------------|------------------------|---------------------------------------------|
// | 0x40000001 | Hypervisor interface   | Breaks hypercall page                       |
// | 0x40000003 | Feature flags          | Breaks SynIC, timers, APIC virtualization   |
// | 0x40000004 | Implementation limits  | Resource allocation issues                  |
//
// These are passed through unchanged - only 0x40000000 vendor string is spoofed.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Configuration
//===----------------------------------------------------------------------===//

struct Config {
    // Phase 1: CPUID spoofing
    bool hide_hypervisor = true;   // Clear ECX[31] in leaf 1
    bool hide_vmx = true;          // Clear ECX[5] in leaf 1 (Intel)
    bool hide_svm = true;          // Clear ECX[2] in leaf 0x80000001 (AMD)
    bool spoof_hv_vendor = true;   // Zero vendor string in 0x40000000 (preserve max leaf!)

    // Phase 2: Timing mitigation (future)
    bool compensate_tsc = false;
    u64 cpuid_tsc_offset = 0;

    // Phase 3: Advanced spoofing (future)
    bool spoof_brand_string = false;
    char custom_brand[48] = {0};
};

extern Config g_config;

//===----------------------------------------------------------------------===//
// API
//===----------------------------------------------------------------------===//

// Initialize with default config (full stealth mode)
void Initialize();

// Execute real CPUID in VMXRoot and apply spoofing modifications
// This function:
// 1. Executes __cpuidex() natively (no VMExit in VMXRoot)
// 2. Applies configured bit masking
// 3. Returns modified results to caller
void ExecuteAndSpoof(u32 leaf, u32 subleaf, u32* eax, u32* ebx, u32* ecx, u32* edx);

} // namespace cpuid_spoof
