// debug_reader.c — Usermode Debug Log Reader Implementation
// OmbraHypervisor

#include "debug_reader.h"
#include <stdio.h>

// =============================================================================
// Console Colors (Windows)
// =============================================================================

static HANDLE g_ConsoleHandle = NULL;

static void InitConsole(void) {
    if (!g_ConsoleHandle) {
        g_ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    }
}

static void SetConsoleColor(WORD color) {
    InitConsole();
    if (g_ConsoleHandle) {
        SetConsoleTextAttribute(g_ConsoleHandle, color);
    }
}

static void ResetConsoleColor(void) {
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

// =============================================================================
// Initialization
// =============================================================================

bool DbgReaderInit(DebugReader* reader, void* sharedMemory, size_t size) {
    DebugRingBuffer* header;
    size_t requiredSize;

    if (!reader || !sharedMemory) {
        return false;
    }

    // Calculate required size
    requiredSize = sizeof(DebugRingBuffer) + (sizeof(DebugEntry) * DEBUG_RING_SIZE);
    if (size < requiredSize) {
        printf("[-] Debug buffer too small: %zu < %zu\n", size, requiredSize);
        return false;
    }

    header = (DebugRingBuffer*)sharedMemory;

    // Validate magic
    if (header->Magic != DEBUG_MAGIC) {
        printf("[-] Invalid debug buffer magic: 0x%llx\n", header->Magic);
        return false;
    }

    // Initialize reader
    reader->Buffer = header;
    reader->Entries = (DebugEntry*)((uint8_t*)header + sizeof(DebugRingBuffer));
    reader->SharedMemory = sharedMemory;
    reader->SharedSize = size;
    reader->Initialized = true;
    reader->LastReadIndex = header->ReadIndex;

    printf("[+] Debug reader initialized (entries=%u, size=%u)\n",
           header->EntryCount, header->EntrySize);

    return true;
}

void DbgReaderShutdown(DebugReader* reader) {
    if (reader) {
        reader->Buffer = NULL;
        reader->Entries = NULL;
        reader->Initialized = false;
    }
}

// =============================================================================
// Reading Entries
// =============================================================================

uint32_t DbgReaderPending(DebugReader* reader) {
    uint32_t write, read;

    if (!reader || !reader->Initialized) {
        return 0;
    }

    write = reader->Buffer->WriteIndex;
    read = reader->Buffer->ReadIndex;

    if (write >= read) {
        return write - read;
    } else {
        return DEBUG_RING_SIZE - (read - write);
    }
}

bool DbgReaderReadOne(DebugReader* reader, DebugEntry* entry) {
    uint32_t readIdx;

    if (!reader || !reader->Initialized || !entry) {
        return false;
    }

    // Check if there's anything to read
    if (reader->Buffer->ReadIndex == reader->Buffer->WriteIndex) {
        return false;
    }

    // Get the entry
    readIdx = reader->Buffer->ReadIndex & DEBUG_RING_MASK;
    *entry = reader->Entries[readIdx];

    // Advance read index (atomic)
    InterlockedIncrement((volatile LONG*)&reader->Buffer->ReadIndex);

    return true;
}

uint32_t DbgReaderReadAll(DebugReader* reader, DebugLogCallback callback, void* context) {
    DebugEntry entry;
    uint32_t count = 0;

    if (!reader || !reader->Initialized || !callback) {
        return 0;
    }

    while (DbgReaderReadOne(reader, &entry)) {
        callback(&entry, context);
        count++;
    }

    return count;
}

// =============================================================================
// Statistics
// =============================================================================

uint64_t DbgReaderGetTotalWrites(DebugReader* reader) {
    if (!reader || !reader->Initialized) {
        return 0;
    }
    return reader->Buffer->TotalWrites;
}

uint64_t DbgReaderGetOverflows(DebugReader* reader) {
    if (!reader || !reader->Initialized) {
        return 0;
    }
    return reader->Buffer->Overflows;
}

void DbgReaderSetMinLevel(DebugReader* reader, DebugLevel level) {
    if (reader && reader->Initialized) {
        reader->Buffer->MinLevel = level;
    }
}

// =============================================================================
// Formatting
// =============================================================================

const char* DbgLevelToString(DebugLevel level) {
    switch (level) {
    case DBG_TRACE: return "TRACE";
    case DBG_INFO:  return "INFO ";
    case DBG_WARN:  return "WARN ";
    case DBG_ERROR: return "ERROR";
    case DBG_FATAL: return "FATAL";
    default:        return "?????";
    }
}

static WORD GetLevelColor(DebugLevel level) {
    switch (level) {
    case DBG_TRACE:
        return FOREGROUND_INTENSITY;  // Dark gray
    case DBG_INFO:
        return FOREGROUND_GREEN | FOREGROUND_INTENSITY;  // Bright green
    case DBG_WARN:
        return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;  // Yellow
    case DBG_ERROR:
        return FOREGROUND_RED | FOREGROUND_INTENSITY;  // Bright red
    case DBG_FATAL:
        return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;  // Magenta
    default:
        return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    }
}

void DbgPrintEntry(const DebugEntry* entry) {
    if (!entry) return;

    // Format: [TSC] [CPU] [LEVEL] file:line - message

    // Timestamp (convert TSC to something readable - just show lower bits)
    printf("[%08llx] ", entry->Timestamp & 0xFFFFFFFF);

    // CPU ID
    printf("[CPU%u] ", entry->CpuId);

    // Level with color
    SetConsoleColor(GetLevelColor((DebugLevel)entry->Level));
    printf("[%s] ", DbgLevelToString((DebugLevel)entry->Level));
    ResetConsoleColor();

    // File and line
    if (entry->Line > 0) {
        SetConsoleColor(FOREGROUND_INTENSITY);
        printf("%s:%u ", entry->File, entry->Line);
        ResetConsoleColor();
    }

    // Message
    printf("- %s\n", entry->Message);
}

// =============================================================================
// Debug Monitor Thread Helper
// =============================================================================

// Example usage in a thread:
//
// DWORD WINAPI DebugMonitorThread(LPVOID param) {
//     DebugReader* reader = (DebugReader*)param;
//     DebugEntry entry;
//
//     while (g_Running) {
//         while (DbgReaderReadOne(reader, &entry)) {
//             DbgPrintEntry(&entry);
//         }
//         Sleep(10);  // Poll every 10ms
//     }
//     return 0;
// }
