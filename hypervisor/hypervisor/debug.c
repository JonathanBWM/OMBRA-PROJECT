// debug.c — Debug Logging Implementation
// OmbraHypervisor

#include "debug.h"
#include "vmx.h"
#include <intrin.h>
#include <stdarg.h>

// =============================================================================
// Global State
// =============================================================================

DEBUG_STATE g_Debug = {0};

// =============================================================================
// String Formatting Helpers (Minimal Implementation)
// =============================================================================
//
// We can't use standard library sprintf in Ring-0, so we implement minimal
// formatting. Supports: %s, %d, %u, %x, %llx, %p

static char* FormatHex(char* buf, char* end, U64 value, int width, bool upper) {
    const char* digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    char temp[17];
    int i = 0;

    if (value == 0) {
        temp[i++] = '0';
    } else {
        while (value && i < 16) {
            temp[i++] = digits[value & 0xF];
            value >>= 4;
        }
    }

    // Pad with zeros
    while (i < width && buf < end - 1) {
        *buf++ = '0';
        width--;
    }

    // Copy digits in reverse
    while (i > 0 && buf < end - 1) {
        *buf++ = temp[--i];
    }

    return buf;
}

static char* FormatDecimal(char* buf, char* end, I64 value, bool isSigned) {
    char temp[21];
    int i = 0;
    bool negative = false;

    if (isSigned && value < 0) {
        negative = true;
        value = -value;
    }

    U64 uval = (U64)value;
    if (uval == 0) {
        temp[i++] = '0';
    } else {
        while (uval && i < 20) {
            temp[i++] = '0' + (uval % 10);
            uval /= 10;
        }
    }

    if (negative && buf < end - 1) {
        *buf++ = '-';
    }

    while (i > 0 && buf < end - 1) {
        *buf++ = temp[--i];
    }

    return buf;
}

static char* FormatString(char* buf, char* end, const char* str) {
    if (!str) str = "(null)";
    while (*str && buf < end - 1) {
        *buf++ = *str++;
    }
    return buf;
}

static int MiniVsnprintf(char* buf, int size, const char* fmt, va_list args) {
    char* start = buf;
    char* end = buf + size - 1;

    while (*fmt && buf < end) {
        if (*fmt != '%') {
            *buf++ = *fmt++;
            continue;
        }

        fmt++;  // Skip '%'

        // Check for 'll' prefix
        bool longlong = false;
        if (fmt[0] == 'l' && fmt[1] == 'l') {
            longlong = true;
            fmt += 2;
        } else if (*fmt == 'l') {
            longlong = true;
            fmt++;
        }

        switch (*fmt) {
        case 's':
            buf = FormatString(buf, end, va_arg(args, const char*));
            break;

        case 'd':
        case 'i':
            if (longlong) {
                buf = FormatDecimal(buf, end, va_arg(args, I64), true);
            } else {
                buf = FormatDecimal(buf, end, va_arg(args, int), true);
            }
            break;

        case 'u':
            if (longlong) {
                buf = FormatDecimal(buf, end, (I64)va_arg(args, U64), false);
            } else {
                buf = FormatDecimal(buf, end, (I64)va_arg(args, unsigned int), false);
            }
            break;

        case 'x':
            if (longlong) {
                buf = FormatHex(buf, end, va_arg(args, U64), 0, false);
            } else {
                buf = FormatHex(buf, end, va_arg(args, unsigned int), 0, false);
            }
            break;

        case 'X':
            if (longlong) {
                buf = FormatHex(buf, end, va_arg(args, U64), 0, true);
            } else {
                buf = FormatHex(buf, end, va_arg(args, unsigned int), 0, true);
            }
            break;

        case 'p':
            if (buf < end - 2) {
                *buf++ = '0';
                *buf++ = 'x';
            }
            buf = FormatHex(buf, end, (U64)va_arg(args, void*), 16, false);
            break;

        case '%':
            *buf++ = '%';
            break;

        case 'c':
            *buf++ = (char)va_arg(args, int);
            break;

        default:
            // Unknown format, just copy
            *buf++ = '%';
            if (buf < end) *buf++ = *fmt;
            break;
        }

        fmt++;
    }

    *buf = '\0';
    return (int)(buf - start);
}

// =============================================================================
// Initialization
// =============================================================================

OMBRA_STATUS DbgInitialize(void* bufferVirtual, U64 bufferSize) {
    DEBUG_RING_BUFFER* header;
    U64 requiredSize;

    if (!bufferVirtual) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Calculate required size
    requiredSize = sizeof(DEBUG_RING_BUFFER) + (sizeof(DEBUG_ENTRY) * DEBUG_RING_SIZE);
    if (bufferSize < requiredSize) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    header = (DEBUG_RING_BUFFER*)bufferVirtual;

    // Initialize header
    header->Magic = DEBUG_MAGIC;
    header->Version = DEBUG_VERSION;
    header->WriteIndex = 0;
    header->ReadIndex = 0;
    header->EntryCount = DEBUG_RING_SIZE;
    header->EntrySize = sizeof(DEBUG_ENTRY);
    header->TotalWrites = 0;
    header->Overflows = 0;
    header->MinLevel = DBG_TRACE;
    header->Flags = DEBUG_FLAG_TIMESTAMPS | DEBUG_FLAG_CPU_ID | DEBUG_FLAG_FILE_LINE;

    // Store in global state
    g_Debug.Buffer = header;
    g_Debug.Entries = (DEBUG_ENTRY*)((U8*)header + sizeof(DEBUG_RING_BUFFER));
    g_Debug.Initialized = true;

    // Log initialization
    INFO("Debug logging initialized (ring size=%u)", DEBUG_RING_SIZE);

    return OMBRA_SUCCESS;
}

void DbgShutdown(void) {
    if (g_Debug.Initialized) {
        INFO("Debug logging shutdown");
    }
    g_Debug.Buffer = NULL;
    g_Debug.Entries = NULL;
    g_Debug.Initialized = false;
}

// =============================================================================
// Core Logging
// =============================================================================

static void ExtractFilename(char* dest, int destSize, const char* path) {
    const char* lastSlash = path;
    const char* p = path;

    // Find last slash or backslash
    while (*p) {
        if (*p == '/' || *p == '\\') {
            lastSlash = p + 1;
        }
        p++;
    }

    // Copy filename (truncated if needed)
    int i = 0;
    while (*lastSlash && i < destSize - 1) {
        dest[i++] = *lastSlash++;
    }
    dest[i] = '\0';
}

void DbgLogEntry(DEBUG_LEVEL level, const char* file, U32 line,
                 const char* format, ...) {
    DEBUG_RING_BUFFER* buf;
    DEBUG_ENTRY* entry;
    U32 writeIdx;
    U32 nextIdx;
    va_list args;

    if (!g_Debug.Initialized || !g_Debug.Buffer) {
        return;
    }

    buf = g_Debug.Buffer;

    // Check minimum level
    if (level < buf->MinLevel) {
        return;
    }

    // Get write slot (atomic increment)
    writeIdx = _InterlockedIncrement((volatile long*)&buf->WriteIndex) - 1;
    writeIdx &= DEBUG_RING_MASK;

    // Check for overflow (reader too slow)
    nextIdx = (writeIdx + 1) & DEBUG_RING_MASK;
    if (nextIdx == buf->ReadIndex) {
        _InterlockedIncrement64((volatile long long*)&buf->Overflows);
        // Continue anyway - overwrite old entry
    }

    entry = &g_Debug.Entries[writeIdx];

    // Fill entry
    entry->Timestamp = __rdtsc();
    entry->Level = level;
    entry->Line = line;

    // Get CPU ID
    {
        VMX_CPU* cpu = VmxGetCurrentCpu();
        entry->CpuId = cpu ? cpu->CpuId : 0xFF;
    }

    // Extract filename from path
    ExtractFilename(entry->File, sizeof(entry->File), file);

    // Format message
    va_start(args, format);
    MiniVsnprintf(entry->Message, sizeof(entry->Message), format, args);
    va_end(args);

    // Update stats
    _InterlockedIncrement64((volatile long long*)&buf->TotalWrites);
}

void DbgPrint(const char* format, ...) {
    va_list args;
    char buf[DEBUG_MAX_MESSAGE_LEN];

    va_start(args, format);
    MiniVsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    DbgLogEntry(DBG_INFO, "dbg", 0, "%s", buf);
}

// =============================================================================
// Utility Functions
// =============================================================================

U32 DbgGetPendingCount(void) {
    if (!g_Debug.Initialized || !g_Debug.Buffer) {
        return 0;
    }

    U32 write = g_Debug.Buffer->WriteIndex;
    U32 read = g_Debug.Buffer->ReadIndex;

    if (write >= read) {
        return write - read;
    } else {
        return DEBUG_RING_SIZE - (read - write);
    }
}

bool DbgIsInitialized(void) {
    return g_Debug.Initialized;
}
