// spoof.c - Hardware Identity Spoofing
// OmbraDriver Phase 3
//
// Provides hardware serial number spoofing for anti-detection

#include "context.h"
#include "command_ring.h"
#include "vmcall.h"

// =============================================================================
// Spoof Configuration
// =============================================================================

#define MAX_SPOOF_ENTRIES 32
#define SPOOF_VALUE_SIZE 64

typedef struct _SPOOF_ENTRY {
    bool    Active;
    U32     Type;                           // OMBRA_SPOOF_TYPE
    U8      OriginalValue[SPOOF_VALUE_SIZE]; // Match pattern (or 0 for any)
    U8      SpoofedValue[SPOOF_VALUE_SIZE];  // Value to return
    U32     ValueSize;                       // Size of values
    U64     TargetDriver;                    // Optional: target specific driver
    U64     HitCount;                        // Statistics: times spoofed
} SPOOF_ENTRY, *PSPOOF_ENTRY;

static SPOOF_ENTRY g_SpoofEntries[MAX_SPOOF_ENTRIES] = {0};

// Spoof types
#define SPOOF_TYPE_DISK_SERIAL    1
#define SPOOF_TYPE_NIC_MAC        2
#define SPOOF_TYPE_VOLUME_SERIAL  3
#define SPOOF_TYPE_SMBIOS         4
#define SPOOF_TYPE_GPU_ID         5
#define SPOOF_TYPE_CPU_ID         6

// =============================================================================
// Memory Helpers
// =============================================================================

static void MemCopy(void* dst, const void* src, U32 size) {
    volatile U8* d = (volatile U8*)dst;
    const volatile U8* s = (const volatile U8*)src;
    for (U32 i = 0; i < size; i++) {
        d[i] = s[i];
    }
}

static void MemZero(void* dst, U32 size) {
    volatile U8* d = (volatile U8*)dst;
    for (U32 i = 0; i < size; i++) {
        d[i] = 0;
    }
}

static bool MemEqual(const void* a, const void* b, U32 size) {
    const volatile U8* pa = (const volatile U8*)a;
    const volatile U8* pb = (const volatile U8*)b;
    for (U32 i = 0; i < size; i++) {
        if (pa[i] != pb[i]) return false;
    }
    return true;
}

static bool MemIsZero(const void* buf, U32 size) {
    const volatile U8* p = (const volatile U8*)buf;
    for (U32 i = 0; i < size; i++) {
        if (p[i] != 0) return false;
    }
    return true;
}

// =============================================================================
// Spoof Management
// =============================================================================

static PSPOOF_ENTRY FindSpoofEntry(U32 type, const void* originalValue, U32 size) {
    for (U32 i = 0; i < MAX_SPOOF_ENTRIES; i++) {
        PSPOOF_ENTRY entry = &g_SpoofEntries[i];
        if (!entry->Active || entry->Type != type) continue;

        // If original value is zero, match any
        if (MemIsZero(entry->OriginalValue, entry->ValueSize)) {
            return entry;
        }

        // Otherwise, match exact value
        if (originalValue && size >= entry->ValueSize &&
            MemEqual(entry->OriginalValue, originalValue, entry->ValueSize)) {
            return entry;
        }
    }
    return 0;
}

I32 SpoofAdd(U32 type, const void* originalValue, const void* spoofedValue, U32 size) {
    if (!spoofedValue || size == 0 || size > SPOOF_VALUE_SIZE) {
        return OMBRA_STATUS_INVALID_PARAMETER;
    }

    // Check if already exists and update
    if (originalValue) {
        PSPOOF_ENTRY existing = FindSpoofEntry(type, originalValue, size);
        if (existing) {
            MemCopy(existing->SpoofedValue, spoofedValue, size);
            existing->ValueSize = size;
            return OMBRA_STATUS_SUCCESS;
        }
    }

    // Find free slot
    for (U32 i = 0; i < MAX_SPOOF_ENTRIES; i++) {
        if (!g_SpoofEntries[i].Active) {
            PSPOOF_ENTRY entry = &g_SpoofEntries[i];

            MemZero(entry, sizeof(SPOOF_ENTRY));
            entry->Active = true;
            entry->Type = type;
            entry->ValueSize = size;

            if (originalValue) {
                MemCopy(entry->OriginalValue, originalValue, size);
            }
            MemCopy(entry->SpoofedValue, spoofedValue, size);

            // Install hooks based on type
            // Each type requires hooking specific kernel functions
            // or intercepting specific IOCTLs
            switch (type) {
                case SPOOF_TYPE_DISK_SERIAL:
                    // Hook disk.sys IOCTL handling
                    // IOCTL_STORAGE_QUERY_PROPERTY / StorageDeviceSerialNumberProperty
                    break;

                case SPOOF_TYPE_NIC_MAC:
                    // Hook NDIS OID queries
                    // OID_802_3_PERMANENT_ADDRESS, OID_802_3_CURRENT_ADDRESS
                    break;

                case SPOOF_TYPE_VOLUME_SERIAL:
                    // Hook NtQueryVolumeInformationFile
                    // FileFsVolumeInformation
                    break;

                case SPOOF_TYPE_SMBIOS:
                    // Hook GetSystemFirmwareTable or raw SMBIOS table access
                    break;

                case SPOOF_TYPE_GPU_ID:
                    // Hook GPU driver IOCTLs
                    break;

                case SPOOF_TYPE_CPU_ID:
                    // This is handled at hypervisor level via CPUID exits
                    break;
            }

            return OMBRA_STATUS_SUCCESS;
        }
    }

    return OMBRA_STATUS_LIMIT_EXCEEDED;
}

I32 SpoofRemove(U32 type, const void* originalValue, U32 size) {
    PSPOOF_ENTRY entry = FindSpoofEntry(type, originalValue, size);
    if (!entry) {
        return OMBRA_STATUS_NOT_FOUND;
    }

    entry->Active = false;
    return OMBRA_STATUS_SUCCESS;
}

// Apply spoofing to a value
// Called from hooks when matching IOCTL/call is intercepted
bool SpoofApply(U32 type, void* buffer, U32 size) {
    PSPOOF_ENTRY entry = FindSpoofEntry(type, buffer, size);
    if (!entry) return false;

    // Replace buffer content with spoofed value
    MemCopy(buffer, entry->SpoofedValue, entry->ValueSize);
    entry->HitCount++;

    return true;
}

// Query current spoof configuration
I32 SpoofQuery(U32 type, void* originalOut, void* spoofedOut, U32 size) {
    PSPOOF_ENTRY entry = FindSpoofEntry(type, NULL, 0);
    if (!entry) {
        return OMBRA_STATUS_NOT_FOUND;
    }

    if (originalOut && size >= entry->ValueSize) {
        MemCopy(originalOut, entry->OriginalValue, entry->ValueSize);
    }
    if (spoofedOut && size >= entry->ValueSize) {
        MemCopy(spoofedOut, entry->SpoofedValue, entry->ValueSize);
    }

    return OMBRA_STATUS_SUCCESS;
}

// =============================================================================
// Command Handlers
// =============================================================================

I32 HandleSpoofDisk(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)resp;
    return SpoofAdd(SPOOF_TYPE_DISK_SERIAL,
                   MemIsZero(cmd->Spoof.OriginalValue, 64) ? NULL : cmd->Spoof.OriginalValue,
                   cmd->Spoof.SpoofedValue,
                   64);
}

I32 HandleSpoofNic(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)resp;
    return SpoofAdd(SPOOF_TYPE_NIC_MAC,
                   MemIsZero(cmd->Spoof.OriginalValue, 6) ? NULL : cmd->Spoof.OriginalValue,
                   cmd->Spoof.SpoofedValue,
                   6);  // MAC is 6 bytes
}

I32 HandleSpoofVolume(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)resp;
    return SpoofAdd(SPOOF_TYPE_VOLUME_SERIAL,
                   MemIsZero(cmd->Spoof.OriginalValue, 4) ? NULL : cmd->Spoof.OriginalValue,
                   cmd->Spoof.SpoofedValue,
                   4);  // Volume serial is 4 bytes
}

I32 HandleSpoofQuery(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    U8 original[64] = {0};
    U8 spoofed[64] = {0};

    I32 status = SpoofQuery(cmd->Spoof.SpoofType, original, spoofed, 64);
    if (status == OMBRA_STATUS_SUCCESS) {
        MemCopy(resp->Raw, original, 64);
        MemCopy(resp->Raw + 64, spoofed, 64);
    }

    return status;
}

// =============================================================================
// Initialization
// =============================================================================

void SpoofInit(void) {
    for (U32 i = 0; i < MAX_SPOOF_ENTRIES; i++) {
        g_SpoofEntries[i].Active = false;
    }
}

// Get total spoof hit count (for statistics)
U64 SpoofGetTotalHits(void) {
    U64 total = 0;
    for (U32 i = 0; i < MAX_SPOOF_ENTRIES; i++) {
        if (g_SpoofEntries[i].Active) {
            total += g_SpoofEntries[i].HitCount;
        }
    }
    return total;
}
