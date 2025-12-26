// PayLoad/core/storage.cpp
// Per-CPU storage slot implementation
// Phase 4: Fixed race condition with per-CPU arrays and atomic global access
#include "../include/storage.h"

namespace storage {

//===----------------------------------------------------------------------===//
// Storage Arrays
//===----------------------------------------------------------------------===//

// Per-CPU storage - cache-line aligned to prevent false sharing
// Each CPU has its own 128-slot array indexed by APIC ID
alignas(64) CpuStorage g_per_cpu_storage[MAX_CPUS] = {};

// Global storage for shared slots (0-15)
// All CPUs read/write the same values atomically
alignas(64) u64 g_global_storage[SLOT_COUNT] = {};

//===----------------------------------------------------------------------===//
// Initialization
//===----------------------------------------------------------------------===//

void Initialize() {
    u32 cpu_id = GetCurrentCpuId();
    if (cpu_id >= MAX_CPUS) return;

    // Zero all per-CPU slots for this CPU
    for (u32 i = 0; i < SLOT_COUNT; i++) {
        g_per_cpu_storage[cpu_id].slots[i] = 0;
    }
}

void InitializeGlobal() {
    // Zero all global slots (only called once during hypervisor init)
    for (u32 i = 0; i < SLOT_COUNT; i++) {
        g_global_storage[i] = 0;
    }
}

//===----------------------------------------------------------------------===//
// Primary Storage Operations
//===----------------------------------------------------------------------===//

u64 Query(u32 index) {
    if (!IsValidSlot(index)) {
        return 0;
    }

    // Global slots (0-15): read from shared global storage
    if (IsGlobalSlot(index)) {
        // Use volatile read to ensure we get current value
        return *reinterpret_cast<volatile u64*>(&g_global_storage[index]);
    }

    // Per-CPU slots (16-127): read from current CPU's array
    u32 cpu_id = GetCurrentCpuId();
    if (cpu_id >= MAX_CPUS) {
        return 0;
    }

    return g_per_cpu_storage[cpu_id].slots[index];
}

void Set(u32 index, u64 value) {
    if (!IsValidSlot(index)) {
        return;
    }

    // Global slots (0-15): atomic write to shared storage
    if (IsGlobalSlot(index)) {
        // Use interlocked exchange for atomic write
        // This ensures visibility across all CPUs
        _InterlockedExchange64(
            reinterpret_cast<volatile long long*>(&g_global_storage[index]),
            static_cast<long long>(value)
        );
        return;
    }

    // Per-CPU slots (16-127): write to current CPU's array
    u32 cpu_id = GetCurrentCpuId();
    if (cpu_id >= MAX_CPUS) {
        return;
    }

    g_per_cpu_storage[cpu_id].slots[index] = value;
}

//===----------------------------------------------------------------------===//
// Explicit Global Operations
//===----------------------------------------------------------------------===//

u64 QueryGlobal(u32 index) {
    if (!IsValidSlot(index)) {
        return 0;
    }

    return *reinterpret_cast<volatile u64*>(&g_global_storage[index]);
}

void SetGlobal(u32 index, u64 value) {
    if (!IsValidSlot(index)) {
        return;
    }

    _InterlockedExchange64(
        reinterpret_cast<volatile long long*>(&g_global_storage[index]),
        static_cast<long long>(value)
    );
}

//===----------------------------------------------------------------------===//
// Cross-CPU Operations
//===----------------------------------------------------------------------===//

u64 QueryCpu(u32 cpu_id, u32 index) {
    if (cpu_id >= MAX_CPUS || !IsValidSlot(index)) {
        return 0;
    }

    return g_per_cpu_storage[cpu_id].slots[index];
}

void SetCpu(u32 cpu_id, u32 index, u64 value) {
    if (cpu_id >= MAX_CPUS || !IsValidSlot(index)) {
        return;
    }

    // Use interlocked for cross-CPU writes to ensure atomicity
    _InterlockedExchange64(
        reinterpret_cast<volatile long long*>(&g_per_cpu_storage[cpu_id].slots[index]),
        static_cast<long long>(value)
    );
}

//===----------------------------------------------------------------------===//
// Array Access
//===----------------------------------------------------------------------===//

u64* GetStorageArray() {
    u32 cpu_id = GetCurrentCpuId();
    if (cpu_id >= MAX_CPUS) {
        // Fallback to CPU 0's array (shouldn't happen on valid systems)
        return g_per_cpu_storage[0].slots;
    }

    return g_per_cpu_storage[cpu_id].slots;
}

u64* GetGlobalStorageArray() {
    return g_global_storage;
}

} // namespace storage
