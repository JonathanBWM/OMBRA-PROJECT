// debug.h — Debug Logging Infrastructure
// OmbraHypervisor
//
// Shared memory ring buffer for hypervisor -> usermode logging.
// The ring buffer lives in memory allocated by usermode and shared
// with the hypervisor. Usermode polls the buffer to display logs.

#ifndef OMBRA_DEBUG_H
#define OMBRA_DEBUG_H

#include "../shared/types.h"

// =============================================================================
// Configuration
// =============================================================================

// Enable/disable debug logging (set to 0 for release builds)
#define DEBUG_ENABLED           1

// Maximum log message length
#define DEBUG_MAX_MESSAGE_LEN   256

// Number of entries in ring buffer (must be power of 2)
#define DEBUG_RING_SIZE         256
#define DEBUG_RING_MASK         (DEBUG_RING_SIZE - 1)

// =============================================================================
// Log Levels
// =============================================================================

typedef enum _DEBUG_LEVEL {
    DBG_TRACE   = 0,    // Verbose tracing
    DBG_INFO    = 1,    // Informational
    DBG_WARN    = 2,    // Warnings
    DBG_ERROR   = 3,    // Errors
    DBG_FATAL   = 4     // Fatal errors (will halt)
} DEBUG_LEVEL;

// =============================================================================
// Log Entry Structure
// =============================================================================

#pragma pack(push, 1)
typedef struct _DEBUG_ENTRY {
    U64         Timestamp;      // TSC at time of log
    U32         CpuId;          // Which CPU logged this
    U32         Level;          // DEBUG_LEVEL
    U32         Line;           // Source line number
    U32         Reserved;       // Alignment padding
    char        File[32];       // Source file (truncated)
    char        Message[DEBUG_MAX_MESSAGE_LEN];
} DEBUG_ENTRY;
#pragma pack(pop)

// =============================================================================
// Ring Buffer Structure (Shared Memory Header)
// =============================================================================
//
// Memory Layout:
//   Offset 0x0000: DEBUG_RING_BUFFER header
//   Offset 0x1000: DEBUG_ENTRY[DEBUG_RING_SIZE] entries
//
// Total size: 4KB header + (sizeof(DEBUG_ENTRY) * DEBUG_RING_SIZE)

#pragma pack(push, 1)
typedef struct _DEBUG_RING_BUFFER {
    // Magic signature for validation
    U64         Magic;          // 'OMBRADBG'
    U32         Version;        // Structure version

    // Ring buffer state
    volatile U32 WriteIndex;    // Next write position (producer)
    volatile U32 ReadIndex;     // Next read position (consumer)
    U32         EntryCount;     // DEBUG_RING_SIZE
    U32         EntrySize;      // sizeof(DEBUG_ENTRY)

    // Statistics
    volatile U64 TotalWrites;   // Total entries written
    volatile U64 Overflows;     // Entries lost due to full buffer

    // Configuration
    U32         MinLevel;       // Minimum level to log
    U32         Flags;          // Debug flags

    // Reserved for future use
    U8          Reserved[4096 - 56];

    // Entries follow immediately after (at offset 0x1000)
    // DEBUG_ENTRY Entries[DEBUG_RING_SIZE];
} DEBUG_RING_BUFFER;
#pragma pack(pop)

#define DEBUG_MAGIC             0x474244415242454FULL  // "OMBRADBG"
#define DEBUG_VERSION           1

// Flags
#define DEBUG_FLAG_TIMESTAMPS   (1 << 0)    // Include timestamps
#define DEBUG_FLAG_CPU_ID       (1 << 1)    // Include CPU ID
#define DEBUG_FLAG_FILE_LINE    (1 << 2)    // Include file/line

// =============================================================================
// Debug State
// =============================================================================

typedef struct _DEBUG_STATE {
    DEBUG_RING_BUFFER*  Buffer;
    DEBUG_ENTRY*        Entries;
    bool                Initialized;
} DEBUG_STATE;

// Global debug state
extern DEBUG_STATE g_Debug;

// =============================================================================
// Initialization
// =============================================================================

// Initialize debug system with pre-allocated shared memory
// bufferVirtual should point to at least:
//   sizeof(DEBUG_RING_BUFFER) + sizeof(DEBUG_ENTRY) * DEBUG_RING_SIZE bytes
OMBRA_STATUS DbgInitialize(void* bufferVirtual, U64 bufferSize);

// Shutdown debug system
void DbgShutdown(void);

// =============================================================================
// Logging Functions
// =============================================================================

// Core logging function (use macros below instead)
void DbgLogEntry(DEBUG_LEVEL level, const char* file, U32 line,
                 const char* format, ...);

// Log with format string
void DbgPrint(const char* format, ...);

// =============================================================================
// Logging Macros
// =============================================================================

#if DEBUG_ENABLED

#define DBG_TRACE_MSG(fmt, ...) \
    DbgLogEntry(DBG_TRACE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define DBG_INFO_MSG(fmt, ...) \
    DbgLogEntry(DBG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define DBG_WARN_MSG(fmt, ...) \
    DbgLogEntry(DBG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define DBG_ERROR_MSG(fmt, ...) \
    DbgLogEntry(DBG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define DBG_FATAL_MSG(fmt, ...) \
    DbgLogEntry(DBG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

// Convenience aliases
#define TRACE(fmt, ...)     DBG_TRACE_MSG(fmt, ##__VA_ARGS__)
#define INFO(fmt, ...)      DBG_INFO_MSG(fmt, ##__VA_ARGS__)
#define WARN(fmt, ...)      DBG_WARN_MSG(fmt, ##__VA_ARGS__)
#define ERR(fmt, ...)       DBG_ERROR_MSG(fmt, ##__VA_ARGS__)
#define FATAL(fmt, ...)     DBG_FATAL_MSG(fmt, ##__VA_ARGS__)

#else

// No-op when debugging disabled
#define DBG_TRACE_MSG(fmt, ...)
#define DBG_INFO_MSG(fmt, ...)
#define DBG_WARN_MSG(fmt, ...)
#define DBG_ERROR_MSG(fmt, ...)
#define DBG_FATAL_MSG(fmt, ...)
#define TRACE(fmt, ...)
#define INFO(fmt, ...)
#define WARN(fmt, ...)
#define ERR(fmt, ...)
#define FATAL(fmt, ...)

#endif

// =============================================================================
// Utility Functions
// =============================================================================

// Get number of entries available to read
U32 DbgGetPendingCount(void);

// Check if buffer is initialized
bool DbgIsInitialized(void);

#endif // OMBRA_DEBUG_H
