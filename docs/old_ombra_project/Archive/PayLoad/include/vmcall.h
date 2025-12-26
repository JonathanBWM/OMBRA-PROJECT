// PayLoad/include/vmcall.h
// Unified VMCALL types and authentication
// Re-exports types from OmbraShared/communication.hpp
#pragma once

#include "types.h"

// Mark that we're in vmxroot mode so communication.hpp doesn't include kernel headers
#define _VMXROOT_MODE

#include <communication.hpp>

//===----------------------------------------------------------------------===//
// VMCALL Authentication
//===----------------------------------------------------------------------===//

namespace vmcall {

// Global communication key - defined in core/dispatch.cpp
// Set via VMCALL_SET_COMM_KEY, used to authenticate hypercalls
extern u64 g_comm_key;

// Validate hypercall authentication
// Key comes in R9 register
inline bool IsVmcall(u64 key) {
    return key == g_comm_key && g_comm_key != 0;
}

// Set the communication key
inline void SetKey(u64 key) {
    g_comm_key = key;
}

// Get the current key (for debugging)
inline u64 GetKey() {
    return g_comm_key;
}

} // namespace vmcall
