// debug_reader.h — Usermode Debug Log Reader
// OmbraHypervisor

#ifndef OMBRA_DEBUG_READER_H
#define OMBRA_DEBUG_READER_H

#include <Windows.h>
#include <stdbool.h>
#include <stdint.h>

// =============================================================================
// Constants (must match hypervisor/debug.h)
// =============================================================================

#define DEBUG_MAX_MESSAGE_LEN   256
#define DEBUG_RING_SIZE         256
#define DEBUG_RING_MASK         (DEBUG_RING_SIZE - 1)
#define DEBUG_MAGIC             0x474244415242454FULL

// =============================================================================
// Log Levels
// =============================================================================

typedef enum {
    DBG_TRACE   = 0,
    DBG_INFO    = 1,
    DBG_WARN    = 2,
    DBG_ERROR   = 3,
    DBG_FATAL   = 4
} DebugLevel;

// =============================================================================
// Structures (must match hypervisor layout)
// =============================================================================

#pragma pack(push, 1)
typedef struct {
    uint64_t    Timestamp;
    uint32_t    CpuId;
    uint32_t    Level;
    uint32_t    Line;
    uint32_t    Reserved;
    char        File[32];
    char        Message[DEBUG_MAX_MESSAGE_LEN];
} DebugEntry;

typedef struct {
    uint64_t    Magic;
    uint32_t    Version;
    volatile uint32_t WriteIndex;
    volatile uint32_t ReadIndex;
    uint32_t    EntryCount;
    uint32_t    EntrySize;
    volatile uint64_t TotalWrites;
    volatile uint64_t Overflows;
    uint32_t    MinLevel;
    uint32_t    Flags;
    uint8_t     Reserved[4096 - 56];
} DebugRingBuffer;
#pragma pack(pop)

// =============================================================================
// Reader Context
// =============================================================================

typedef struct {
    DebugRingBuffer*    Buffer;
    DebugEntry*         Entries;
    void*               SharedMemory;
    size_t              SharedSize;
    bool                Initialized;
    uint32_t            LastReadIndex;
} DebugReader;

// =============================================================================
// Callback for log entries
// =============================================================================

typedef void (*DebugLogCallback)(
    const DebugEntry* entry,
    void* context
);

// =============================================================================
// Functions
// =============================================================================

// Initialize the debug reader with shared memory from driver allocation
bool DbgReaderInit(DebugReader* reader, void* sharedMemory, size_t size);

// Shutdown the reader
void DbgReaderShutdown(DebugReader* reader);

// Check if there are pending entries
uint32_t DbgReaderPending(DebugReader* reader);

// Read one entry (returns false if none available)
bool DbgReaderReadOne(DebugReader* reader, DebugEntry* entry);

// Read all pending entries with callback
uint32_t DbgReaderReadAll(DebugReader* reader, DebugLogCallback callback, void* context);

// Get statistics
uint64_t DbgReaderGetTotalWrites(DebugReader* reader);
uint64_t DbgReaderGetOverflows(DebugReader* reader);

// Set minimum log level filter
void DbgReaderSetMinLevel(DebugReader* reader, DebugLevel level);

// Print entry to console with colors
void DbgPrintEntry(const DebugEntry* entry);

// Level to string
const char* DbgLevelToString(DebugLevel level);

#endif // OMBRA_DEBUG_READER_H
