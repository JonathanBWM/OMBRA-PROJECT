#pragma once
// OmbraSELib Standalone CPU Utilities for VMXRoot Context
// No WDK dependencies

#include "types.h"
#include <intrin.h>

// GetCs implemented in IDT.asm
extern "C" unsigned short GetCs();

namespace CPU {

// Wrapper for assembly GetCs
__forceinline u16 GetCs() {
    return static_cast<u16>(::GetCs());
}

// Get APIC ID for current processor
__forceinline u32 ApicId() {
    int cpuInfo[4] = { 0 };
    __cpuid(cpuInfo, 1);
    return static_cast<u32>((cpuInfo[1] >> 24) & 0xFF);
}

// CPU initialization (stub for vmxroot - actual init done elsewhere)
__forceinline void Init() {
    // No-op for vmxroot - initialization is handled by the hypervisor
}

// Get processor index (simple wrapper around ApicId for compatibility)
__forceinline u32 GetCPUIndex(bool = false) {
    return ApicId();
}

} // namespace CPU
