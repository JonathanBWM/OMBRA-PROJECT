#pragma once
// OmbraSELib Standalone Vmcall Utilities for VMXRoot Context
// No WDK dependencies

#include "types.h"

namespace vmcall {

// Communication key storage (vmxroot global)
inline u64 g_CommunicationKey = 0;

// XOR mask for vmcall authentication
constexpr u64 VMCALL_XOR_MASK = 0xdeada55;

// Check if the provided key is a valid vmcall authentication
__forceinline bool IsVmcall(u64 r9) {
    return r9 == (g_CommunicationKey ^ VMCALL_XOR_MASK);
}

// Set the communication key
__forceinline void SetKey(u64 key) {
    g_CommunicationKey = key;
}

// Get the current communication key
__forceinline u64 GetKey() {
    return g_CommunicationKey;
}

// Validate communication key (allows initial set or matching key)
__forceinline bool ValidateCommunicationKey(u64 key) {
    return !g_CommunicationKey || g_CommunicationKey == key;
}

} // namespace vmcall
