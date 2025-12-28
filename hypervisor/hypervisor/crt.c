// crt.c - Minimal CRT replacements for position-independent hypervisor blob
// OmbraHypervisor
//
// This file provides minimal implementations of CRT functions required by
// compiled code but unavailable when linking with /NODEFAULTLIB.
//
// These implementations prioritize correctness over performance since they
// are typically used during initialization, not in hot paths.

#include "../shared/types.h"

// Use U64 for size_t since we're in a no-CRT environment
typedef U64 size_t;

// Disable intrinsics for all functions we're defining
// This allows us to provide custom implementations
#pragma function(memcpy)
#pragma function(memset)
#pragma function(strlen)
#pragma function(memmove)
#pragma function(memcmp)

// =============================================================================
// memcpy - Copy memory block
// =============================================================================
//
// Standard memcpy implementation. Uses byte-by-byte copy for simplicity.
// For larger copies in performance-critical paths, consider using __movsb
// intrinsic directly.

void* memcpy(void* dest, const void* src, size_t count) {
    U8* d = (U8*)dest;
    const U8* s = (const U8*)src;

    while (count--) {
        *d++ = *s++;
    }

    return dest;
}

// =============================================================================
// memset - Fill memory block
// =============================================================================
//
// Standard memset implementation. Uses byte-by-byte fill.
// For larger fills in performance-critical paths, consider using __stosb
// intrinsic directly.

void* memset(void* dest, int value, size_t count) {
    U8* d = (U8*)dest;
    U8 v = (U8)value;

    while (count--) {
        *d++ = v;
    }

    return dest;
}

// =============================================================================
// strncmp - Compare strings with length limit
// =============================================================================
//
// Standard strncmp implementation.

int strncmp(const char* s1, const char* s2, size_t count) {
    while (count--) {
        if (*s1 != *s2) {
            return (*(U8*)s1 - *(U8*)s2);
        }
        if (*s1 == '\0') {
            return 0;
        }
        s1++;
        s2++;
    }
    return 0;
}

// =============================================================================
// strlen - String length (may be needed by some code paths)
// =============================================================================

size_t strlen(const char* str) {
    const char* s = str;
    while (*s) {
        s++;
    }
    return (size_t)(s - str);
}

// =============================================================================
// memmove - Copy memory with overlap handling
// =============================================================================
//
// Unlike memcpy, handles overlapping source and destination.

void* memmove(void* dest, const void* src, size_t count) {
    U8* d = (U8*)dest;
    const U8* s = (const U8*)src;

    if (d < s) {
        // Copy forward
        while (count--) {
            *d++ = *s++;
        }
    } else if (d > s) {
        // Copy backward
        d += count;
        s += count;
        while (count--) {
            *--d = *--s;
        }
    }

    return dest;
}

// =============================================================================
// memcmp - Compare memory blocks
// =============================================================================

int memcmp(const void* s1, const void* s2, size_t count) {
    const U8* p1 = (const U8*)s1;
    const U8* p2 = (const U8*)s2;

    while (count--) {
        if (*p1 != *p2) {
            return (*p1 - *p2);
        }
        p1++;
        p2++;
    }

    return 0;
}
