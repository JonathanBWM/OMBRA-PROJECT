// PayLoad/include/storage.h
// Per-CPU storage slot management for hypervisor state
// Phase 4: Fixed race condition with per-CPU arrays
#pragma once

#include "types.h"
#include <communication.hpp>

//===----------------------------------------------------------------------===//
// Storage Configuration
//===----------------------------------------------------------------------===//

namespace storage {

// Total number of storage slots per CPU
constexpr u32 SLOT_COUNT = 128;

// Maximum supported CPUs (covers high-end server systems)
constexpr u32 MAX_CPUS = 256;

// Predefined slot indices - imported from communication.hpp VMX_ROOT_STORAGE enum
// CALLBACK_ADDRESS = 0      // Driver callback function pointer (GLOBAL)
// EPT_HANDLER_ADDRESS = 1   // EPT violation handler (GLOBAL)
// EPT_OS_INIT_BITMAP = 2    // Per-core EPT initialization flags (2-10) (PER-CPU)
// DRIVER_BASE_PA = 11       // Physical address of hidden driver (GLOBAL)
// NTOSKRNL_CR3 = 12         // System process CR3 (GLOBAL)
// CURRENT_CONTROLLER_PROCESS = 13  // Current controller EPROCESS (GLOBAL)
// PAYLOAD_BASE = 14         // SUPDrv-loaded payload base (GLOBAL)
// MAX_STORAGE = 127         // Maximum slot index

//===----------------------------------------------------------------------===//
// Per-CPU Storage Structure
//===----------------------------------------------------------------------===//

// Cache-line aligned to prevent false sharing between CPUs
// Each CPU gets its own 128-slot array
struct alignas(64) CpuStorage {
    u64 slots[SLOT_COUNT];
};

// Per-CPU storage arrays - indexed by APIC ID
extern CpuStorage g_per_cpu_storage[MAX_CPUS];

// Global storage for slots shared across all CPUs
// Uses atomic access to prevent race conditions
extern u64 g_global_storage[SLOT_COUNT];

//===----------------------------------------------------------------------===//
// CPU Identification
//===----------------------------------------------------------------------===//

// Get current CPU's APIC ID (used as array index)
inline u32 GetCurrentCpuId() {
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    // APIC ID is in bits 31:24 of EBX
    return static_cast<u32>((cpuInfo[1] >> 24) & 0xFF);
}

//===----------------------------------------------------------------------===//
// Slot Classification
//===----------------------------------------------------------------------===//

// Slots 0-15 are global (same value across all CPUs)
// Slots 16-127 are per-CPU (each CPU has its own copy)
constexpr u32 GLOBAL_SLOT_MAX = 15;

inline bool IsGlobalSlot(u32 index) {
    return index <= GLOBAL_SLOT_MAX;
}

inline bool IsPerCpuSlot(u32 index) {
    return index > GLOBAL_SLOT_MAX && index < SLOT_COUNT;
}

//===----------------------------------------------------------------------===//
// Storage Operations
//===----------------------------------------------------------------------===//

// Validate slot index
inline bool IsValidSlot(u32 index) {
    return index <= VMX_ROOT_STORAGE::MAX_STORAGE;
}

// Query a storage slot value
// Global slots return same value for all CPUs
// Per-CPU slots return value specific to current CPU
u64 Query(u32 index);

// Set a storage slot value
// Global slots are set atomically (visible to all CPUs)
// Per-CPU slots are set for current CPU only
void Set(u32 index, u64 value);

//===----------------------------------------------------------------------===//
// Explicit Global/Per-CPU Operations
//===----------------------------------------------------------------------===//

// Force query from global storage (regardless of slot classification)
u64 QueryGlobal(u32 index);

// Force set to global storage (regardless of slot classification)
void SetGlobal(u32 index, u64 value);

// Query from specific CPU's per-CPU storage
u64 QueryCpu(u32 cpu_id, u32 index);

// Set to specific CPU's per-CPU storage
void SetCpu(u32 cpu_id, u32 index, u64 value);

//===----------------------------------------------------------------------===//
// Array Access (for EPT handler bitmap operations)
//===----------------------------------------------------------------------===//

// Get pointer to current CPU's storage array
u64* GetStorageArray();

// Get pointer to global storage array
u64* GetGlobalStorageArray();

//===----------------------------------------------------------------------===//
// Initialization
//===----------------------------------------------------------------------===//

// Initialize storage for current CPU
// Should be called during VMExit handler first execution on each CPU
void Initialize();

// Initialize global storage (call once during hypervisor setup)
void InitializeGlobal();

} // namespace storage
