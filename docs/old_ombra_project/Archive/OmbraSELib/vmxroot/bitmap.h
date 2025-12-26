#pragma once
// OmbraSELib Standalone Bitmap Utilities for VMXRoot Context
// No WDK dependencies

#include "types.h"

namespace bitmap {

typedef struct _LARGE_BITMAP {
    u64 low;
    u64 high;
} LARGE_BITMAP, *PLARGE_BITMAP;

// EPT_OS_INIT_BITMAP spans 9 slots (slots 2-10), supporting 576 cores max
// This matches EPT_OS_INIT_BITMAP_END = EPT_OS_INIT_BITMAP + 8 in communication.hpp
constexpr u32 MAX_BITMAP_SLOTS = 9;
constexpr u32 MAX_SUPPORTED_CORES = MAX_BITMAP_SLOTS * 64;  // 576 cores

__forceinline void SetBit(void* va, u32 bit, bool bSet) {
    u64* pBitmap = static_cast<u64*>(va);
    u32 index = bit / 64;
    u32 offset = bit % 64;

    // Bounds check to prevent buffer overflow on high core count systems
    if (index >= MAX_BITMAP_SLOTS) {
        return;  // Silently ignore - core ID exceeds supported range
    }

    if (bSet) {
        pBitmap[index] |= (1ULL << offset);
    } else {
        pBitmap[index] &= ~(1ULL << offset);
    }
}

__forceinline bool GetBit(void* va, u32 bit) {
    u64* pBitmap = static_cast<u64*>(va);
    u32 index = bit / 64;
    u32 offset = bit % 64;

    // Bounds check to prevent buffer overflow on high core count systems
    if (index >= MAX_BITMAP_SLOTS) {
        return false;  // Core ID exceeds supported range, treat as uninitialized
    }

    return (pBitmap[index] & (1ULL << offset)) != 0;
}

template<typename T>
__forceinline T bits(u64 value, u64 start, u64 end) {
    if (end <= start)
        return 0;
    u64 bitmask = 0;
    for (u32 i = 0; i < sizeof(bitmask) * 8; i++) {
        if (i <= end && i >= start)
            SetBit(&bitmask, i, true);
    }

    return static_cast<T>((value & bitmask) >> start);
}

} // namespace bitmap
