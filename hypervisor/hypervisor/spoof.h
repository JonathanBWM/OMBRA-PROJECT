// spoof.h — Hardware Spoofing Infrastructure
// OmbraHypervisor
//
// Provides EPT-based interception for hardware fingerprint spoofing.
// Defeats anti-cheat that queries disk serials, NIC MACs, etc.

#ifndef OMBRA_SPOOF_H
#define OMBRA_SPOOF_H

#include "../shared/types.h"

// =============================================================================
// Spoofing Configuration
// =============================================================================

#define MAX_SPOOFED_DISKS   8
#define MAX_SPOOFED_NICS    4
#define SERIAL_MAX_LEN      64
#define MAC_ADDR_LEN        6

// Disk serial spoof entry
typedef struct _DISK_SPOOF_ENTRY {
    bool    Active;
    U32     DiskIndex;              // Disk number (0, 1, 2, ...)
    char    OriginalSerial[SERIAL_MAX_LEN];
    char    SpoofedSerial[SERIAL_MAX_LEN];
    char    OriginalModel[SERIAL_MAX_LEN];
    char    SpoofedModel[SERIAL_MAX_LEN];
    U64     HookAddress;            // Address of hooked dispatch routine
    U64     OriginalHandler;        // Original handler for passthrough
} DISK_SPOOF_ENTRY;

// NIC MAC spoof entry
typedef struct _NIC_SPOOF_ENTRY {
    bool    Active;
    U32     NicIndex;               // NIC index
    U8      OriginalMac[MAC_ADDR_LEN];
    U8      SpoofedMac[MAC_ADDR_LEN];
    U64     HookAddress;
    U64     OriginalHandler;
} NIC_SPOOF_ENTRY;

// Global spoofing state
typedef struct _SPOOF_MANAGER {
    bool                Initialized;

    // Disk spoofing
    DISK_SPOOF_ENTRY    Disks[MAX_SPOOFED_DISKS];
    U32                 DiskCount;
    bool                DiskSpoofEnabled;

    // NIC spoofing
    NIC_SPOOF_ENTRY     Nics[MAX_SPOOFED_NICS];
    U32                 NicCount;
    bool                NicSpoofEnabled;

    // Random seed for auto-generation
    U32                 RandomSeed;
} SPOOF_MANAGER;

extern SPOOF_MANAGER g_SpoofManager;

// =============================================================================
// Windows IOCTL Codes for Disk Queries
// =============================================================================

// Storage query - returns serial, model, vendor
#define IOCTL_STORAGE_QUERY_PROPERTY        0x002D1400

// ATA pass-through for SMART data
#define IOCTL_ATA_PASS_THROUGH              0x0004D02C
#define IOCTL_ATA_PASS_THROUGH_DIRECT       0x0004D030

// SCSI pass-through
#define IOCTL_SCSI_PASS_THROUGH             0x0004D004
#define IOCTL_SCSI_PASS_THROUGH_DIRECT      0x0004D014

// NVMe pass-through
#define IOCTL_STORAGE_PROTOCOL_COMMAND      0x002D5140

// Disk geometry
#define IOCTL_DISK_GET_DRIVE_GEOMETRY       0x00070000
#define IOCTL_DISK_GET_DRIVE_GEOMETRY_EX    0x000700A0

// =============================================================================
// Storage Property Query Structures
// =============================================================================

typedef enum _STORAGE_PROPERTY_ID {
    StorageDeviceProperty = 0,
    StorageAdapterProperty = 1,
    StorageDeviceIdProperty = 2,
    StorageDeviceUniqueIdProperty = 3,
    StorageDeviceWriteCacheProperty = 4,
    StorageMiniportProperty = 5,
    StorageAccessAlignmentProperty = 6,
    StorageDeviceSeekPenaltyProperty = 7,
    StorageDeviceTrimProperty = 8,
    StorageDeviceWriteAggregationProperty = 9,
    StorageDeviceDeviceTelemetryProperty = 10,
    StorageDeviceLBProvisioningProperty = 11,
    StorageDevicePowerProperty = 12,
    StorageDeviceCopyOffloadProperty = 13,
    StorageDeviceResiliencyProperty = 14,
    StorageDeviceMediumProductType = 15,
    StorageAdapterRpmbProperty = 16,
    StorageDeviceIoCapabilityProperty = 48,
    StorageAdapterProtocolSpecificProperty = 49,
    StorageDeviceProtocolSpecificProperty = 50,
    StorageAdapterTemperatureProperty = 51,
    StorageDeviceTemperatureProperty = 52,
    StorageAdapterPhysicalTopologyProperty = 53,
    StorageDevicePhysicalTopologyProperty = 54,
    StorageDeviceAttributesProperty = 55,
} STORAGE_PROPERTY_ID;

typedef enum _STORAGE_QUERY_TYPE {
    PropertyStandardQuery = 0,
    PropertyExistsQuery = 1,
    PropertyMaskQuery = 2,
    PropertyQueryMaxDefined = 3,
} STORAGE_QUERY_TYPE;

#pragma pack(push, 1)

typedef struct _STORAGE_PROPERTY_QUERY {
    STORAGE_PROPERTY_ID PropertyId;
    STORAGE_QUERY_TYPE QueryType;
    U8 AdditionalParameters[1];
} STORAGE_PROPERTY_QUERY;

typedef struct _STORAGE_DEVICE_DESCRIPTOR {
    U32 Version;
    U32 Size;
    U8  DeviceType;
    U8  DeviceTypeModifier;
    U8  RemovableMedia;
    U8  CommandQueueing;
    U32 VendorIdOffset;         // Offset to vendor string
    U32 ProductIdOffset;        // Offset to product/model string
    U32 ProductRevisionOffset;  // Offset to revision string
    U32 SerialNumberOffset;     // Offset to serial number string
    U8  BusType;
    U32 RawPropertiesLength;
    U8  RawDeviceProperties[1];
} STORAGE_DEVICE_DESCRIPTOR;

#pragma pack(pop)

// =============================================================================
// Network OID Codes for MAC Queries
// =============================================================================

#define OID_802_3_PERMANENT_ADDRESS     0x01010101
#define OID_802_3_CURRENT_ADDRESS       0x01010102
#define OID_GEN_MAC_ADDRESS             0x00010102

// =============================================================================
// API Functions
// =============================================================================

// Initialize spoofing manager
OMBRA_STATUS SpoofManagerInit(SPOOF_MANAGER* mgr);
void SpoofManagerShutdown(SPOOF_MANAGER* mgr);

// Disk spoofing
OMBRA_STATUS SpoofAddDisk(SPOOF_MANAGER* mgr, U32 diskIndex,
                          const char* serial, const char* model);
OMBRA_STATUS SpoofRemoveDisk(SPOOF_MANAGER* mgr, U32 diskIndex);
bool SpoofShouldFilterDiskIoctl(U32 ioctl);
void SpoofFilterDiskResponse(SPOOF_MANAGER* mgr, U32 diskIndex,
                             void* buffer, U32 bufferSize);

// NIC spoofing
OMBRA_STATUS SpoofAddNic(SPOOF_MANAGER* mgr, U32 nicIndex, const U8* mac);
OMBRA_STATUS SpoofRemoveNic(SPOOF_MANAGER* mgr, U32 nicIndex);
bool SpoofShouldFilterNicOid(U32 oid);
void SpoofFilterNicResponse(SPOOF_MANAGER* mgr, U32 nicIndex,
                            void* buffer, U32 bufferSize);

// Random serial generation
void SpoofGenerateRandomSerial(char* buffer, U32 length, U32 seed);
void SpoofGenerateRandomMac(U8* buffer, U32 seed);

#endif // OMBRA_SPOOF_H
