// spoof.c — Hardware Spoofing Implementation
// OmbraHypervisor

#include "spoof.h"
#include "hooks.h"
#include "debug.h"

// =============================================================================
// Global Instance
// =============================================================================

SPOOF_MANAGER g_SpoofManager = {0};

// =============================================================================
// Helpers
// =============================================================================

static void ZeroMem(void* ptr, U64 size) {
    U8* p = (U8*)ptr;
    while (size--) *p++ = 0;
}

static void CopyMem(void* dst, const void* src, U64 size) {
    U8* d = (U8*)dst;
    const U8* s = (const U8*)src;
    while (size--) *d++ = *s++;
}

static U32 StrLen(const char* s) {
    U32 len = 0;
    while (*s++) len++;
    return len;
}

static void StrCopy(char* dst, const char* src, U32 maxLen) {
    U32 i = 0;
    while (src[i] && i < maxLen - 1) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = 0;
}

// Simple LCG PRNG
static U32 g_SpoofRandState = 0xDEADBEEF;

static U32 SpoofRand(void) {
    g_SpoofRandState = g_SpoofRandState * 1103515245 + 12345;
    return (g_SpoofRandState >> 16) & 0x7FFF;
}

static void SpoofSeedRand(U32 seed) {
    g_SpoofRandState = seed ? seed : 0xDEADBEEF;
}

// =============================================================================
// Manager Lifecycle
// =============================================================================

OMBRA_STATUS SpoofManagerInit(SPOOF_MANAGER* mgr) {
    if (!mgr) return OMBRA_ERROR_INVALID_PARAM;

    INFO("Initializing spoof manager");

    ZeroMem(mgr, sizeof(SPOOF_MANAGER));
    mgr->RandomSeed = 0x12345678;
    mgr->Initialized = true;

    return OMBRA_SUCCESS;
}

void SpoofManagerShutdown(SPOOF_MANAGER* mgr) {
    if (!mgr) return;

    INFO("Shutting down spoof manager");
    mgr->Initialized = false;
}

// =============================================================================
// Random Generation
// =============================================================================

void SpoofGenerateRandomSerial(char* buffer, U32 length, U32 seed) {
    static const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    if (seed) SpoofSeedRand(seed);

    for (U32 i = 0; i < length - 1; i++) {
        buffer[i] = charset[SpoofRand() % 36];
    }
    buffer[length - 1] = 0;
}

void SpoofGenerateRandomMac(U8* buffer, U32 seed) {
    if (seed) SpoofSeedRand(seed);

    // Generate random MAC with locally administered bit set
    // Bit 1 of first octet = 1 (locally administered)
    // Bit 0 of first octet = 0 (unicast)
    buffer[0] = (U8)((SpoofRand() & 0xFC) | 0x02);  // x2, x6, xA, xE
    buffer[1] = (U8)(SpoofRand() & 0xFF);
    buffer[2] = (U8)(SpoofRand() & 0xFF);
    buffer[3] = (U8)(SpoofRand() & 0xFF);
    buffer[4] = (U8)(SpoofRand() & 0xFF);
    buffer[5] = (U8)(SpoofRand() & 0xFF);
}

// =============================================================================
// Disk Spoofing
// =============================================================================

OMBRA_STATUS SpoofAddDisk(SPOOF_MANAGER* mgr, U32 diskIndex,
                          const char* serial, const char* model) {
    if (!mgr || !mgr->Initialized) return OMBRA_ERROR_INVALID_PARAM;
    if (diskIndex >= MAX_SPOOFED_DISKS) return OMBRA_ERROR_INVALID_PARAM;

    DISK_SPOOF_ENTRY* entry = &mgr->Disks[diskIndex];

    // If serial is NULL or empty, generate random
    if (serial && serial[0]) {
        StrCopy(entry->SpoofedSerial, serial, SERIAL_MAX_LEN);
    } else {
        SpoofGenerateRandomSerial(entry->SpoofedSerial, 20, mgr->RandomSeed++);
    }

    // If model is NULL or empty, leave as original
    if (model && model[0]) {
        StrCopy(entry->SpoofedModel, model, SERIAL_MAX_LEN);
    } else {
        entry->SpoofedModel[0] = 0;  // Will use original
    }

    entry->DiskIndex = diskIndex;
    entry->Active = true;
    mgr->DiskCount++;
    mgr->DiskSpoofEnabled = true;

    INFO("Added disk spoof: index=%u serial=%s", diskIndex, entry->SpoofedSerial);
    return OMBRA_SUCCESS;
}

OMBRA_STATUS SpoofRemoveDisk(SPOOF_MANAGER* mgr, U32 diskIndex) {
    if (!mgr || !mgr->Initialized) return OMBRA_ERROR_INVALID_PARAM;
    if (diskIndex >= MAX_SPOOFED_DISKS) return OMBRA_ERROR_INVALID_PARAM;

    DISK_SPOOF_ENTRY* entry = &mgr->Disks[diskIndex];
    if (!entry->Active) return OMBRA_SUCCESS;

    entry->Active = false;
    mgr->DiskCount--;

    if (mgr->DiskCount == 0) {
        mgr->DiskSpoofEnabled = false;
    }

    INFO("Removed disk spoof: index=%u", diskIndex);
    return OMBRA_SUCCESS;
}

bool SpoofShouldFilterDiskIoctl(U32 ioctl) {
    switch (ioctl) {
    case IOCTL_STORAGE_QUERY_PROPERTY:
    case IOCTL_ATA_PASS_THROUGH:
    case IOCTL_ATA_PASS_THROUGH_DIRECT:
    case IOCTL_SCSI_PASS_THROUGH:
    case IOCTL_SCSI_PASS_THROUGH_DIRECT:
    case IOCTL_STORAGE_PROTOCOL_COMMAND:
        return true;
    default:
        return false;
    }
}

void SpoofFilterDiskResponse(SPOOF_MANAGER* mgr, U32 diskIndex,
                             void* buffer, U32 bufferSize) {
    if (!mgr || !mgr->Initialized || !buffer) return;
    if (diskIndex >= MAX_SPOOFED_DISKS) return;

    DISK_SPOOF_ENTRY* entry = &mgr->Disks[diskIndex];
    if (!entry->Active) return;

    // Try to parse as STORAGE_DEVICE_DESCRIPTOR
    if (bufferSize < sizeof(STORAGE_DEVICE_DESCRIPTOR)) return;

    STORAGE_DEVICE_DESCRIPTOR* desc = (STORAGE_DEVICE_DESCRIPTOR*)buffer;

    // Validate structure
    if (desc->Size > bufferSize) return;

    // Spoof serial number if present
    if (desc->SerialNumberOffset != 0 &&
        desc->SerialNumberOffset < desc->Size) {
        char* serial = (char*)buffer + desc->SerialNumberOffset;
        U32 serialLen = StrLen(serial);
        U32 spoofLen = StrLen(entry->SpoofedSerial);

        // Copy spoof serial (truncate if needed)
        U32 copyLen = (spoofLen < serialLen) ? spoofLen : serialLen;
        for (U32 i = 0; i < copyLen; i++) {
            serial[i] = entry->SpoofedSerial[i];
        }

        TRACE("Spoofed disk %u serial: %s", diskIndex, serial);
    }

    // Spoof product ID / model if specified
    if (entry->SpoofedModel[0] && desc->ProductIdOffset != 0 &&
        desc->ProductIdOffset < desc->Size) {
        char* model = (char*)buffer + desc->ProductIdOffset;
        U32 modelLen = StrLen(model);
        U32 spoofLen = StrLen(entry->SpoofedModel);

        U32 copyLen = (spoofLen < modelLen) ? spoofLen : modelLen;
        for (U32 i = 0; i < copyLen; i++) {
            model[i] = entry->SpoofedModel[i];
        }

        TRACE("Spoofed disk %u model: %s", diskIndex, model);
    }
}

// =============================================================================
// NIC Spoofing
// =============================================================================

OMBRA_STATUS SpoofAddNic(SPOOF_MANAGER* mgr, U32 nicIndex, const U8* mac) {
    if (!mgr || !mgr->Initialized) return OMBRA_ERROR_INVALID_PARAM;
    if (nicIndex >= MAX_SPOOFED_NICS) return OMBRA_ERROR_INVALID_PARAM;

    NIC_SPOOF_ENTRY* entry = &mgr->Nics[nicIndex];

    // If mac is NULL, generate random
    if (mac) {
        CopyMem(entry->SpoofedMac, mac, MAC_ADDR_LEN);
    } else {
        SpoofGenerateRandomMac(entry->SpoofedMac, mgr->RandomSeed++);
    }

    entry->NicIndex = nicIndex;
    entry->Active = true;
    mgr->NicCount++;
    mgr->NicSpoofEnabled = true;

    INFO("Added NIC spoof: index=%u MAC=%02X:%02X:%02X:%02X:%02X:%02X",
         nicIndex,
         entry->SpoofedMac[0], entry->SpoofedMac[1], entry->SpoofedMac[2],
         entry->SpoofedMac[3], entry->SpoofedMac[4], entry->SpoofedMac[5]);

    return OMBRA_SUCCESS;
}

OMBRA_STATUS SpoofRemoveNic(SPOOF_MANAGER* mgr, U32 nicIndex) {
    if (!mgr || !mgr->Initialized) return OMBRA_ERROR_INVALID_PARAM;
    if (nicIndex >= MAX_SPOOFED_NICS) return OMBRA_ERROR_INVALID_PARAM;

    NIC_SPOOF_ENTRY* entry = &mgr->Nics[nicIndex];
    if (!entry->Active) return OMBRA_SUCCESS;

    entry->Active = false;
    mgr->NicCount--;

    if (mgr->NicCount == 0) {
        mgr->NicSpoofEnabled = false;
    }

    INFO("Removed NIC spoof: index=%u", nicIndex);
    return OMBRA_SUCCESS;
}

bool SpoofShouldFilterNicOid(U32 oid) {
    switch (oid) {
    case OID_802_3_PERMANENT_ADDRESS:
    case OID_802_3_CURRENT_ADDRESS:
    case OID_GEN_MAC_ADDRESS:
        return true;
    default:
        return false;
    }
}

void SpoofFilterNicResponse(SPOOF_MANAGER* mgr, U32 nicIndex,
                            void* buffer, U32 bufferSize) {
    if (!mgr || !mgr->Initialized || !buffer) return;
    if (nicIndex >= MAX_SPOOFED_NICS) return;
    if (bufferSize < MAC_ADDR_LEN) return;

    NIC_SPOOF_ENTRY* entry = &mgr->Nics[nicIndex];
    if (!entry->Active) return;

    // Replace MAC address in buffer
    CopyMem(buffer, entry->SpoofedMac, MAC_ADDR_LEN);

    TRACE("Spoofed NIC %u MAC", nicIndex);
}
