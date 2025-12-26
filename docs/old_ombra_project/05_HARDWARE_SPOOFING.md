# HARDWARE IDENTITY SPOOFING - C++ to C + Assembly Port Guide

## Overview

This document details the comprehensive hardware identity spoofing system that intercepts I/O requests, modifies hardware identifiers, and prevents anti-cheat detection. The system operates across multiple hardware subsystems (disks, NICs, SMBIOS, GPU, volumes, USB, EFI) using IOCTL hooks and memory manipulation.

**Key Strategy**: Hook IRP_MJ_DEVICE_CONTROL handlers for driver stacks, intercept completion routines, and modify hardware serial numbers, MAC addresses, GUIDs, and SMBIOS table entries before they reach usermode.

## File Inventory

### Spoofing Implementation Files

| File | Lines | Purpose |
|------|-------|---------|
| `diskspoof.cpp` | 1,275 | Disk serial number spoofing (SATA, NVMe, SCSI) |
| `diskspoof.h` | 581 | Disk structures, IOCTL codes, NVMe/ATA definitions |
| `nicspoof.cpp` | 675 | Network interface MAC address spoofing |
| `nicspoof.h` | 218 | NIC structures, NSI/NDIS IOCTL codes |
| `smbiosspoof.cpp` | 136 | SMBIOS table modification via EPT shadowing |
| `smbiosspoof.h` | 8 | SMBIOS spoof interface |
| `gpuspoof.cpp` | 244 | NVIDIA GPU UUID randomization |
| `gpuspoof.h` | 13 | GPU IOCTL constants |
| `volumespoof.cpp` | 179 | Volume GUID randomization |
| `volumespoof.h` | 37 | Volume IOCTL structures |
| `usbspoof.cpp` | 151 | USB device serial number nullification |
| `usbspoof.h` | 12 | USB spoof interface |
| `efispoof.cpp` | 90 | EFI variable randomization |
| `efispoof.h` | 9 | EFI constants |
| `spoof.cpp` | 51 | Master orchestrator - calls all subsystems |
| `spoof.h` | 18 | Master spoof interface |

### Supporting Files

| File | Lines | Purpose |
|------|-------|---------|
| `smbios.cpp` | 508 | SMBIOS table parser |
| `smbios.h` | 368 | SMBIOS structure definitions (17 table types) |
| `identity.cpp` | 215 | Physical memory identity mapping |
| `identity.h` | 238 | Identity mapping class for phys↔virt translation |

**Total**: ~5,525 lines of C++ code to port

## Architecture Summary

```
┌─────────────────────────────────────────────────────────────────────┐
│                     Usermode Application                            │
│  DeviceIoControl(IOCTL_STORAGE_QUERY_PROPERTY, ...)                 │
└──────────────────┬──────────────────────────────────────────────────┘
                   │ IRP with IOCTL code
                   ↓
┌─────────────────────────────────────────────────────────────────────┐
│                    Driver Stack (Windows)                           │
│  ┌────────────┬─────────────┬─────────────┬─────────────┐          │
│  │ partmgr.sys│ stornvme.sys│ storahci.sys│ nsiproxy.sys│          │
│  │ (Volumes)  │ (NVMe)      │ (SATA)      │ (Network)   │          │
│  └────────────┴─────────────┴─────────────┴─────────────┘          │
└──────────────────┬──────────────────────────────────────────────────┘
                   │ Original IRP_MJ_DEVICE_CONTROL
                   ↓
┌─────────────────────────────────────────────────────────────────────┐
│                   OmbraDriver Hooks (EPT-based)                     │
│  EPT::HookExec(DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL])  │
│                                                                     │
│  ┌──────────────────────────────────────────────────────────────┐  │
│  │ Hook Function (e.g. DiskControl, NicControl)                 │  │
│  │  1. Parse IOCTL code from IO_STACK_LOCATION                  │  │
│  │  2. Identify if IOCTL requests hardware ID                   │  │
│  │  3. Call Original Handler → IRP completes                    │  │
│  │  4. ChangeIoc() - Set custom completion routine              │  │
│  └──────────────────────────────────────────────────────────────┘  │
└──────────────────┬──────────────────────────────────────────────────┘
                   │ IRP returns from device
                   ↓
┌─────────────────────────────────────────────────────────────────────┐
│              Custom Completion Routine (IoCompletion)               │
│  ┌──────────────────────────────────────────────────────────────┐  │
│  │ AtaPassIoc() / NicIoc() / PartInfoIoc() ...                  │  │
│  │  1. Extract IOC_REQUEST context                              │  │
│  │  2. Parse output buffer (IDENTIFY_DEVICE_DATA, IFEntry, ...) │  │
│  │  3. Locate serial number / MAC address field                 │  │
│  │  4. Call FindFakeDiskSerial() / FindFakeNicMac()             │  │
│  │  5. Call Old Completion Routine if exists                    │  │
│  └──────────────────────────────────────────────────────────────┘  │
└──────────────────┬──────────────────────────────────────────────────┘
                   │ Modified data
                   ↓
┌─────────────────────────────────────────────────────────────────────┐
│                  Serial/MAC Spoofing Cache                          │
│  ┌──────────────────────────────────────────────────────────────┐  │
│  │ FindFakeDiskSerial(char* pOriginal)                          │  │
│  │  1. Search vDiskSerials vector for original value            │  │
│  │  2. If found: Memory::WriteProtected(pOriginal, spoofed)     │  │
│  │  3. If not found:                                            │  │
│  │     - Allocate new DISK_SERIAL_DATA                          │  │
│  │     - rnd.random_shuffle_ignore_chars(spoofed + 2, len - 2)  │  │
│  │     - Append to vector                                       │  │
│  │     - Write spoofed value                                    │  │
│  └──────────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────────┘
```

### Hook Installation Sequence (spoof::SpoofAll)

```c
NTSTATUS spoofer::SpoofAll(DWORD64 seed) {
    spoofer::seed = seed;

    // Order matters - volumes first (used by disk enumeration)
    volumes::Spoof(seed);   // Mount manager GUID spoofing
    disks::Spoof(seed);     // Disk serials, WWNs, IEEE addresses
    nics::Spoof(seed);      // MAC addresses, interface GUIDs
    wmi::SpoofMonitor(seed);// Monitor EDIDs (not shown in excerpts)
    usb::Spoof(seed);       // USB serial numbers
    smbios::Spoof(seed);    // SMBIOS tables (EPT shadow pages)
    efi::Spoof(seed);       // EFI variables
    gpu::Spoof(seed);       // NVIDIA GPU UUIDs

    return STATUS_SUCCESS;
}
```

## Disk Serial Spoofing

### Hook Points

**Target Drivers**:
- `\Driver\Disk` - Main disk class driver (up to 255 device objects)
- `\Driver\partmgr` - Partition manager
- `\Driver\stornvme` - NVMe storage
- `\Driver\storahci` - AHCI/SATA storage
- `\Device\RaidPort0` through `\Device\RaidPort63` - RAID controllers

**Hook Method**: EPT execution hook on `MajorFunction[IRP_MJ_DEVICE_CONTROL]`

### IOCTL Codes Intercepted

| IOCTL Code | Value | Structure | Field Spoofed |
|------------|-------|-----------|---------------|
| **Partition Manager** | | | |
| `IOCTL_DISK_GET_PARTITION_INFO_EX` | 0x70048 | `PARTITION_INFORMATION_EX` | `Gpt.PartitionId` (GUID) |
| `IOCTL_DISK_GET_DRIVE_LAYOUT_EX` | 0x70050 | `DRIVE_LAYOUT_INFORMATION_EX` | `Gpt.DiskId` (GUID) |
| **Storage Query** | | | |
| `IOCTL_STORAGE_QUERY_PROPERTY` | 0x2D1400 | `STORAGE_DEVICE_DESCRIPTOR` | `RawDeviceProperties + SerialNumberOffset` |
| `IOCTL_STORAGE_QUERY_PROPERTY` | 0x2D1400 | `STORAGE_PROTOCOL_DATA_DESCRIPTOR` | NVMe controller serial |
| **ATA Pass-Through** | | | |
| `IOCTL_ATA_PASS_THROUGH` | 0x4D02C | `ATA_PASS_THROUGH_EX` + `IDENTIFY_DEVICE_DATA` | `SerialNumber[20]` |
| `IOCTL_ATA_PASS_THROUGH_DIRECT` | 0x4D030 | `ATA_PASS_THROUGH_DIRECT` | `SerialNumber[20]`, `CurrentMediaSerialNumber[30]`, `WorldWideName` |
| **SCSI Pass-Through** | | | |
| `IOCTL_SCSI_PASS_THROUGH` | 0x4D004 | `SCSI_PASS_THROUGH` + NVMe data | `nvmeIdentify->SerialNumber[20]` |
| `IOCTL_SCSI_PASS_THROUGH_DIRECT` | 0x4D014 | `SCSI_PASS_THROUGH_DIRECT` | Same as above |
| `IOCTL_SCSI_PASS_THROUGH_EX` | 0x4D040 | `SCSI_PASS_THROUGH_EX` | Same as above |
| `IOCTL_SCSI_PASS_THROUGH_DIRECT_EX` | 0x4D044 | `SCSI_PASS_THROUGH_DIRECT_EX` | Same as above |
| `IOCTL_SCSI_MINIPORT` | 0x4D008 | `SRB_IO_CONTROL` + payload | Context-dependent |
| **SCSI Miniport Sub-IOCTLs** | | | |
| `IOCTL_SCSI_MINIPORT_IDENTIFY` | 0x1B0501 | `SENDCMDOUTPARAMS` + `IDINFO` | `sSerialNumber[20]` |
| `IOCTL_INTEL_NVME_PASS_THROUGH` | 0xF000A02 | `INTEL_NVME_PASS_THROUGH` | `nvmeId->SerialNumber[20]` |
| `NVME_PASS_THROUGH_SRB_IO_CODE` | 0xE000800 | `NVME_PASS_THROUGH_IOCTL` | Blocked (returns `STATUS_NOT_SUPPORTED`) |
| **SMART** | | | |
| `SMART_RCV_DRIVE_DATA` | 0x7C088 | `SENDCMDOUTPARAMS` + `IDSECTOR` | `sSerialNumber[20]` |

**Total**: 17 IOCTL codes hooked per disk driver

### NVMe-Specific Handling

NVMe drives require special handling because they use two different command structures:

1. **Standard Storage Protocol** (via `STORAGE_PROTOCOL_SPECIFIC_DATA`):
```c
struct STORAGE_PROTOCOL_SPECIFIC_QUERY_WITH_BUFFER {
    STORAGE_PROPERTY_QUERY PropertyQuery;
    STORAGE_PROTOCOL_SPECIFIC_DATA ProtocolSpecific;
    BYTE DataBuffer[1];
};

// When ProtocolSpecific.DataType == NVMeDataTypeIdentify:
nvme_id_ctrl* ctrl = (nvme_id_ctrl*)DataBuffer;
FindFakeDiskSerial(ctrl->sn);           // Serial number (20 bytes)
FindFakeIEEE((IEEE*)ctrl->ieee);        // IEEE OUI (3 bytes)
```

2. **Intel Proprietary** (via `IOCTL_INTEL_NVME_PASS_THROUGH`):
```c
struct INTEL_NVME_PASS_THROUGH {
    SRB_IO_CONTROL SRB;
    INTEL_NVME_PAYLOAD Payload;
    BYTE DataBuffer[0x1000];
};

NVME_IDENTIFY_DEVICE* nvmeId = (NVME_IDENTIFY_DEVICE*)DataBuffer;
FindFakeDiskSerial(nvmeId->SerialNumber);
```

### Serial Number Generation Algorithm

```c
bool FindFakeDiskSerial(char* pOriginal, bool bCappedString = true) {
    // 1. Check cache
    for (auto& serial : *vDiskSerials) {
        if (!memcmp(serial.orig, pOriginal, serial.sz)) {
            Memory::WriteProtected(pOriginal, serial.spoofed, serial.sz);
            return true;
        }
    }

    // 2. Generate new spoofed value
    DISK_SERIAL_DATA data;
    rnd.setSeed(spoofer::seed);

    int serialLen = bCappedString ? DISK_SERIAL_MAX_LENGTH : strlen(pOriginal);
    data.orig = kMallocZero(serialLen + 1);
    data.spoofed = kMallocZero(serialLen + 1);
    data.sz = serialLen;

    memcpy(data.orig, pOriginal, serialLen);
    memcpy(data.spoofed, pOriginal, serialLen);

    // 3. Randomize everything EXCEPT first 2 chars and special chars
    // Keeps format consistent (e.g. "S4" prefix for Samsung, "WD" for Western Digital)
    rnd.random_shuffle_ignore_chars(
        data.spoofed + 2,    // Start at index 2
        serialLen - 2,       // Randomize rest
        " _-.",              // Preserve these characters
        4
    );

    vDiskSerials->Append(data);
    Memory::WriteProtected(pOriginal + 2, data.spoofed + 2, serialLen - 2);
    return true;
}
```

**Key Insight**: Preserve first 2 characters to maintain vendor prefixes. For example:
- `S4NENS0M123456A` (Samsung) → `S4NENS0M8F3K2P9`
- `WD-WCAV12345678` (Western Digital) → `WD-WCAV98K3L2M4`

### WWN (World Wide Name) Spoofing

```c
struct WWN {
    USHORT WorldWideName[4];              // 64-bit identifier
    USHORT ReservedForWorldWideName128[4]; // Extended 128-bit (unused)
};

bool FindFakeWWN(WWN* pOriginal) {
    // Check cache
    for (auto& wwn : *vWWNs) {
        if (wwn->WorldWideName[0] == pOriginal->WorldWideName[0]) {
            Memory::WriteProtected(pOriginal, wwn, sizeof(*pOriginal));
            return true;
        }
    }

    // Generate new - preserve first USHORT (NAA identifier), randomize rest
    WWN* pBuf = kMalloc(sizeof(*pBuf));
    *pBuf = *pOriginal;
    rnd.random_shuffle((char*)&pBuf->WorldWideName[1], 14); // 7 USHORTs = 14 bytes
    vWWNs->Append(pBuf);
    Memory::WriteProtected(pOriginal, pBuf, sizeof(*pOriginal));
    return true;
}
```

### IEEE OUI Spoofing

```c
struct IEEE {
    UCHAR ieee[3]; // Organizationally Unique Identifier
};

bool FindFakeIEEE(IEEE* pOriginal) {
    // Preserve first byte (I/G and U/L bits), randomize last 2 bytes
    IEEE* pBuf = kMalloc(sizeof(*pBuf));
    *pBuf = *pOriginal;
    rnd.random_shuffle((char*)&pBuf->ieee[1], 2);
    vIEEEs->Append(pBuf);
    Memory::WriteProtected(pOriginal, pBuf, sizeof(*pOriginal));
    return true;
}
```

### RAID Controller Spoofing

RAID controllers expose disks through `\Device\RaidPortN` device objects. The driver extension contains two serial number fields:

```c
struct _RAID_UNIT_EXTENSION {
    // +offsets.RaidIdentity
    DWORD64* pIdentity; // Points to _STOR_SCSI_IDENTITY

    // +offsets.RaidSerialNumber
    char SerialNumber[DISK_SERIAL_MAX_LENGTH];
};

struct _STOR_SCSI_IDENTITY {
    // +offsets.ScsiSerialNumber
    char SerialNumber[DISK_SERIAL_MAX_LENGTH];
};

// Hook sequence
for (int i = 0; i < 64; i++) {
    wchar_t deviceName[] = L"\\Device\\RaidPort0";
    deviceName[16] = i + '0';

    PDEVICE_OBJECT device = GetRaidDevice(deviceName);
    DWORD64* pIdentity = (DWORD64*)((DWORD64)device->DeviceExtension + offsets.RaidIdentity);
    char* pSerial1 = (char*)(*pIdentity + offsets.ScsiSerialNumber);
    char* pSerial2 = (char*)((DWORD64)device->DeviceExtension + offsets.RaidSerialNumber);

    FindFakeDiskSerial(pSerial1);
    FindFakeDiskSerial(pSerial2);

    // Hook IRP_MJ_DEVICE_CONTROL
    EPT::HookExec(device->DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL],
                  i == 0 ? Scsi0IoCtrlHook : Scsi1IoCtrlHook);
}
```

**Registry Synchronization**: After spoofing in-memory structures, call `RaidUnitRegisterInterfaces()` to update registry entries:

```c
typedef NTSTATUS(__fastcall* RaidUnitRegisterInterfaces)(PHDD_EXTENSION device_ext);
RaidUnitRegisterInterfaces pRaidUnitRegisterInterfaces =
    (RaidUnitRegisterInterfaces)((UINT64)storport_base + offsets.RaidUnitRegInterface);

pRaidUnitRegisterInterfaces(pDeviceHDD);
```

## NIC MAC Spoofing

### NDIS Filter Approach

Network adapters are managed by the NDIS (Network Driver Interface Specification) stack. The architecture uses a filter chain:

```
User Application
    ↓ DeviceIoControl
NDIS Filter Chain
    ↓
Miniport Driver (e.g. Intel e1000, Realtek RTL8139)
    ↓
Hardware NIC
```

**Target Structures**:
```c
struct _NDIS_FILTER_BLOCK {
    // +0x008
    struct _NDIS_FILTER_BLOCK* NextFilter;

    // +offsets.FilterBlockMiniport
    DWORD64 Miniport; // Points to _NDIS_MINIPORT_BLOCK

    // +offsets.FilterBlockIfBlock
    PNDIS_IF_BLOCK IfBlock;
};

struct _NDIS_IF_BLOCK {
    // +offsets.IfBlockPhy
    IF_PHYSICAL_ADDRESS_LH ifPhysAddress;      // Current MAC

    // +offsets.IfBlockPermanentPhy
    IF_PHYSICAL_ADDRESS_LH PermanentPhysAddress; // Hardware MAC
};

struct _NDIS_MINIPORT_BLOCK {
    // +offsets.MiniportBlockLowestFilter
    PNDIS_FILTER_BLOCK LowestFilter;

    // +offsets.MiniportBlockHighestFilter
    PNDIS_FILTER_BLOCK HighestFilter;

    // +offsets.MiniportBlockInterfaceGuid
    GUID InterfaceGuid;
};
```

### Hook Points

| Driver | IOCTL | Structure | Field Spoofed |
|--------|-------|-----------|---------------|
| **ndis.sys** | `IOCTL_NDIS_QUERY_GLOBAL_STATS` (0x170002) | MDL address | MAC from `MmGetSystemAddressForMdl()` |
| **nsiproxy.sys** | `IOCTL_NSI_GETALLPARAM` (0x12001B) | `NSI_PARAMS` | Blocks ARP table queries (returns `STATUS_ACCESS_DENIED`) |
| **nsi (device)** | Same as nsiproxy | Same | Same |
| **Tcp (device)** | `IOCTL_TCP_QUERY_INFORMATION_EX` (0x120003) | `IFEntry` | `if_physaddr[8]` |
| **ndiswan** | `IOCTL_NDIS_QUERY_GLOBAL_STATS` | MDL address | MAC from MDL |

### NDIS OID Codes

When `IOCTL_NDIS_QUERY_GLOBAL_STATS` is called, the OID (Object Identifier) is in `irp->AssociatedIrp.SystemBuffer`:

```c
#define OID_802_3_PERMANENT_ADDRESS     0x01010101
#define OID_802_3_CURRENT_ADDRESS       0x01010102
#define OID_802_5_PERMANENT_ADDRESS     0x02010101  // Token Ring
#define OID_802_5_CURRENT_ADDRESS       0x02010102
#define OID_WAN_PERMANENT_ADDRESS       0x04010101  // WAN adapters
#define OID_WAN_CURRENT_ADDRESS         0x04010102
#define OID_ARCNET_PERMANENT_ADDRESS    0x06010101  // ARCNET
#define OID_ARCNET_CURRENT_ADDRESS      0x06010102

NTSTATUS NICControl(PDEVICE_OBJECT device, PIRP irp) {
    PIO_STACK_LOCATION ioc = IoGetCurrentIrpStackLocation(irp);
    switch (ioc->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_NDIS_QUERY_GLOBAL_STATS: {
        switch (*(PDWORD)irp->AssociatedIrp.SystemBuffer) {
        case OID_802_3_PERMANENT_ADDRESS:
        case OID_802_3_CURRENT_ADDRESS:
            // ... (all 8 OID codes)
            ChangeIoc(ioc, irp, NICIoc);
            break;
        }
        break;
    }
    }
    return pOriginal(device, irp);
}
```

### MAC Address Generation

```c
#define MAC_MAX_LENGTH 6

bool FindFakeNicMac(char* pOriginal, bool bAddIfNotFound = true) {
    // 1. Check cache
    for (auto& MAC : *vMACs) {
        if (!memcmp(MAC->orig.raw, pOriginal, sizeof(MAC->orig))) {
            Memory::WriteProtected(pOriginal, MAC->spoofed.raw, sizeof(MAC->orig));
            return true;
        }
    }

    // 2. Generate new
    PMAC_MODIFICATION_DATA pBuf = kMallocZero(sizeof(*pBuf));
    memcpy(pBuf->orig.raw, pOriginal, sizeof(pBuf->orig));
    memcpy(pBuf->spoofed.raw, pOriginal, sizeof(pBuf->spoofed));

    // 3. Preserve first 3 bytes (OUI), randomize last 3 bytes (NIC-specific)
    rnd.bytes(pBuf->spoofed.raw + 3, sizeof(pBuf->spoofed) - 3);

    Memory::WriteProtected(pOriginal, pBuf->spoofed.raw, sizeof(pBuf->spoofed));
    vMACs->Append(pBuf);
    return true;
}
```

**Example**:
- Original: `00:1A:2B:3C:4D:5E` (Intel OUI: `00:1A:2B`)
- Spoofed: `00:1A:2B:7F:89:A3` (Preserves Intel OUI)

### NDIS Filter Enumeration

```c
PVOID ndisBase = GetDriverBase("ndis.sys");
PVOID ndisGlobalFilterList = *(PVOID*)((DWORD64)ndisBase + offsets.NdisGlobalFilterList);

// Walk filter chain
while (ndisGlobalFilterList) {
    PCHAR pCurrentFilter = (PCHAR)ndisGlobalFilterList;
    ndisGlobalFilterList = *(PVOID*)((DWORD64)ndisGlobalFilterList + offsets.FilterBlockNextFilter);

    // Get miniport block
    DWORD64 pMiniportBlock = *(DWORD64*)(pCurrentFilter + offsets.FilterBlockMiniport);

    // Spoof interface GUID
    GUID* pGuid = (PGUID)(pMiniportBlock + offsets.MiniportBlockInterfaceGuid);
    rnd.bytes((char*)&pGuid->Data2, sizeof(pGuid->Data2));
    rnd.bytes((char*)&pGuid->Data3, sizeof(pGuid->Data3));

    // Walk sub-filter chain (LowestFilter → NextFilter → ...)
    PVOID pLowestMiniportFilter = *(PVOID*)(pCurrentFilter + offsets.MiniportBlockLowestFilter);
    while (pLowestMiniportFilter) {
        PNDIS_IF_BLOCK pNdisIfBlock = *(PNDIS_IF_BLOCK*)(pLowestMiniportFilter + offsets.FilterBlockIfBlock);

        // Spoof both current and permanent MAC
        FindFakeNicMac((char*)pNdisIfBlock->ifPhysAddress.Address);
        FindFakeNicMac((char*)pNdisIfBlock->PermanentPhysAddress.Address);

        pLowestMiniportFilter = *(PVOID*)((DWORD64)pLowestMiniportFilter + offsets.FilterBlockNextFilter);
    }
}
```

### NSI (Network Store Interface) Blocking

The NSI proxy allows querying ARP tables, which could expose original MAC addresses. The solution is to **block specific NSI queries**:

```c
#define NSI_GET_INTERFACE_INFO (1)
#define NSI_GET_IP_NET_TABLE   (11) // ARP table query

NTSTATUS NsiControl(PDEVICE_OBJECT device, PIRP irp) {
    PIO_STACK_LOCATION ioc = IoGetCurrentIrpStackLocation(irp);
    switch (ioc->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_NSI_GETALLPARAM: {
        DWORD length = ioc->Parameters.DeviceIoControl.OutputBufferLength;
        NTSTATUS ret = pOriginal(device, irp);

        PNSI_PARAMS params = (PNSI_PARAMS)irp->UserBuffer;
        if (params->Type == NSI_GET_IP_NET_TABLE) {
            // Zero out ARP table response
            memset(irp->UserBuffer, 0, length);
            return STATUS_ACCESS_DENIED;
        }
        return ret;
    }
    }
    return pOriginal(device, irp);
}
```

## SMBIOS Spoofing

### Table Types Modified

SMBIOS (System Management BIOS) tables are standardized firmware structures. Anti-cheats often query these for hardware fingerprinting.

| Type | Name | Fields Spoofed |
|------|------|----------------|
| 0 | BIOS Information | (Read-only, not spoofed) |
| 1 | System Information | `UUID[16]`, `SerialNumber` |
| 2 | Baseboard Information | `SerialNumber` |
| 3 | System Enclosure | `SerialNumber` |
| 4 | Processor Information | `SerialNumber` |
| 17 | Memory Device | `SerialNumber`, `PartNumber` |

### Memory Layout

SMBIOS tables are stored in physical memory and exposed via:
1. **WMI**: `\\.\root\wmi::MSSmBios_RawSMBiosTables` (usermode accessible)
2. **Kernel Globals**:
   - `ntoskrnl!WmipSMBiosTablePhysicalAddress` (PHYSICAL_ADDRESS)
   - `ntoskrnl!WmipSMBiosTableLength` (DWORD32)
   - `ntoskrnl!WmipSMBiosVersionInfo` (RawSMBIOSData*)

**Table Format**:
```c
struct RawSMBIOSData {
    UCHAR Used20CallingMethod;
    UCHAR SMBIOSMajorVersion;
    UCHAR SMBIOSMinorVersion;
    UCHAR DmiRevision;
    DWORD32 Length;
    UCHAR SMBIOSTableData[1]; // Variable length
};

// Each table entry:
struct SMBIOS_ENTRY {
    UCHAR Type;         // Table type (0, 1, 2, 3, 4, 17, ...)
    UCHAR Length;       // Formatted section length
    USHORT Handle;      // Unique handle
    UCHAR Data[Length - 4]; // Formatted data
    // Followed by null-terminated strings
    // Terminated by double null (0x00 0x00)
};
```

### EPT Shadow Page Implementation

SMBIOS spoofing uses **EPT shadow paging** to present different data to usermode:

```c
bool smbios::Spoof(DWORD64 seed) {
    // 1. Get physical address of SMBIOS table
    PPHYSICAL_ADDRESS pWmipSMBiosTablePhysicalAddress =
        (PPHYSICAL_ADDRESS)((DWORD64)ntoskrnlBase + offsets.WmipSMBiosTablePhysicalAddress);
    DWORD32* smbiosTableLen =
        (DWORD32*)((DWORD64)ntoskrnlBase + offsets.WmipSMBiosTableLength);

    DWORD64 totLen = *smbiosTableLen + PAGE_SIZE;
    PHYSICAL_ADDRESS pa = *pWmipSMBiosTablePhysicalAddress;
    pa.QuadPart = PAGE_ALIGN(pa.QuadPart);

    // 2. Map physical memory
    char* pOrigSMBiosTable = (char*)MmMapIoSpaceEx(pa, totLen, PAGE_READWRITE);

    // 3. Create shadow copies
    pCopySMBios = kMalloc(totLen);  // Modified version
    pOrigSMBios = kMalloc(totLen);  // Backup of original
    memcpy(pCopySMBios, pOrigSMBiosTable, totLen);
    memcpy(pOrigSMBios, pOrigSMBiosTable, totLen);

    // 4. Set up EPT hooks for each page
    for (DWORD64 pageOffset = 0; pageOffset < totLen; pageOffset += PAGE_SIZE) {
        HOOK_SECONDARY_INFO hkSecondaryInfo = { 0 };
        PAGE_PERMISSIONS pgPermissions = { .Exec = true };
        hkSecondaryInfo.pSubstitutePage = pCopySMBios + pageOffset;

        EPT::Hook(pOrigSMBiosTable + pageOffset,
                  hkSecondaryInfo.pSubstitutePage,
                  hkSecondaryInfo,
                  pgPermissions);
    }

    MmUnmapIoSpace(pOrigSMBiosTable, totLen);

    // 5. Parse and modify shadow copy
    smbios::Parser parser(pCopySMBios + ADDRMASK_EPT_PML1_OFFSET(pa.QuadPart),
                          *smbiosTableLen,
                          (WmipSMBiosVersionInfo->SMBiosMajorVersion << 8) |
                          WmipSMBiosVersionInfo->SMBiosMinorVersion);

    auto entry = parser.next();
    while (entry) {
        switch (entry->type) {
        case TYPE_SYSTEM_INFO: {
            // Randomize UUID bytes 4-15 (preserve first 4 bytes)
            rnd.bytes(entry->data.sysinfo.UUID + 4, 12);

            // Randomize serial number (if not default string)
            if (strcmp("Default string", entry->data.sysinfo.SerialNumber) != 0) {
                int serialLen = strlen(entry->data.sysinfo.SerialNumber);
                rnd.random_shuffle_ignore_chars(entry->data.sysinfo.SerialNumber,
                                                serialLen, " _-.", 4);
            }
            break;
        }
        case TYPE_BASEBOARD_INFO:
        case TYPE_SYSTEM_ENCLOSURE:
        case TYPE_PROCESSOR_INFO:
        case TYPE_MEMORY_DEVICE:
            // Similar logic for each type
            break;
        }
        entry = parser.next();
    }

    // 6. Update registry (WMI data source)
    char* temp = kMalloc(*smbiosTableLen + 8);
    memcpy(temp, WmipSMBiosVersionInfo, 4);
    memcpy(temp + 4, smbiosTableLen, 4);
    memcpy(temp + 8, pCopySMBios, *smbiosTableLen);
    registry::SetKeyValueEx("SYSTEM\\CurrentControlSet\\Services\\mssmbios\\Data",
                            "SMBiosData", temp, REG_BINARY, *smbiosTableLen + 8);
    kFree(temp);

    return true;
}
```

**How EPT Shadowing Works**:
1. Guest reads SMBIOS at physical address `0x12345000`
2. EPT translates `0x12345000` → `pCopySMBios` (our modified version)
3. Guest sees spoofed serial numbers
4. Kernel still sees original data (because EPT is one-way - guest→host)

### SMBIOS Parser Implementation

The parser walks the SMBIOS table format:

```c
Entry* Parser::next() {
    if (ptr_ == nullptr)
        ptr_ = start_ = data_;
    else {
        // Skip to end of current entry
        ptr_ = start_ + entry_.length - 4; // Subtract header size

        // Find double null terminator
        while (ptr_ < data_ + size_ - 1 && !(ptr_[0] == 0 && ptr_[1] == 0))
            ptr_++;
        ptr_ += 2;

        if (ptr_ >= data_ + size_)
            return nullptr;
    }

    // Read entry header
    entry_.type = *ptr_++;
    entry_.length = *ptr_++;
    entry_.handle = *(USHORT*)ptr_;
    ptr_ += 2;
    entry_.rawdata = ptr_ - 4;
    entry_.strings = (char*)entry_.rawdata + entry_.length;
    start_ = ptr_;

    if (entry_.type == 127) // End-of-table marker
        return nullptr;

    return parseEntry();
}

char* Parser::getString(int index) const {
    if (index <= 0) return "";

    char* ptr = (char*)start_ + entry_.length - 4;
    for (int i = 1; *ptr != 0 && i < index; ++i) {
        while (*ptr != 0) ptr++;
        ptr++;
    }
    return ptr;
}
```

**String References**: SMBIOS stores strings after the formatted section:
```
[Type][Length][Handle][Data...][String1\0String2\0String3\0\0]
                         ^
                         Indexed from 1
```

Example:
```c
entry->data.sysinfo.Manufacturer_ = 0x02; // Index into string table
entry->data.sysinfo.Manufacturer = getString(0x02); // Returns "Dell Inc."
```

## GPU ID Spoofing

### NVIDIA GPU UUID Randomization

NVIDIA GPUs expose a unique UUID per device. Anti-cheats read this via `nvapi.dll` or WMI.

**Target**: `nvlddmkm.sys` (NVIDIA Kernel Mode Driver)

**Approach**: Pattern scan for `GpuMgrGetGpuFromId()` function, then iterate all GPU instances and randomize the UUID field in the GPU object.

```c
bool gpu::Spoof(DWORD64 seed) {
    PVOID pBase = GetKernelAddress("nvlddmkm.sys");

    // 1. Find GpuMgrGetGpuFromId pattern
    UINT64 Addr = Memory::FindPatternImage(pBase,
        "\xE8\xCC\xCC\xCC\xCC\x48\x8B\xD8\x48\x85\xC0\x0F\x84\xCC\xCC\xCC\xCC\x44\x8B\x80\xCC\xCC\xCC\xCC\x48\x8D\x15",
        "x????xxxxxxxx????xxx????xxx");

    UINT64 AddrOffset = 0x3B;
    if (*(UINT8*)(Addr + AddrOffset) != 0xE8)
        AddrOffset++;

    // Resolve reference to GpuMgrGetGpuFromId
    GpuMgrGetGpuFromId = (UINT64(__fastcall*)(int))(*(int*)(Addr + 1) + 5 + Addr);
    Addr += AddrOffset;
    Addr += *(int*)(Addr + 1) + 5; // gpuGetGidInfo

    // 2. Find UUID offset by scanning for "cmp [rcx + offset], dil" pattern
    UINT32 UuidValidOffset = 0;
    ZydisDecoder decoder;
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64);

    for (int i = 0; i < 0x50; i++) {
        UINT32 Opcode = *(UINT32*)Addr & 0xFFFFFF;
        if (Opcode == 0x818D4C) { // lea r9, [rcx + offset]
            UuidValidOffset = *(UINT32*)(Addr + 3) - 1;
            break;
        }
        // Decode and increment
    }

    // 3. Iterate all GPUs (max 32)
    for (int i = 0; i < 32; i++) {
        UINT64 ProbedGPU = GpuMgrGetGpuFromId(i);
        if (!ProbedGPU) continue;
        if (!*(bool*)(ProbedGPU + UuidValidOffset)) continue; // UUID not initialized

        // 4. Randomize UUID
        rnd.setSeed(seed);
        UUID* uuid = (UUID*)(ProbedGPU + UuidValidOffset + 1);
        rnd.bytes((char*)uuid, sizeof(UUID));
    }

    return true;
}
```

**Why Pattern Scanning?**:
NVIDIA's driver is closed-source and changes with each driver version. Hard-coded offsets would break immediately. Pattern scanning finds the function dynamically.

## Volume GUID Randomization

Volume GUIDs are exposed via mount manager and appear in device paths like:
```
\\?\Volume{12345678-1234-1234-1234-123456789012}\
```

### Hook Points

| Driver | IOCTL | Structure | Field Spoofed |
|--------|-------|-----------|---------------|
| `\Driver\mountmgr` | `IOCTL_MOUNTMGR_QUERY_POINTS` (0x6D0008) | `MOUNTMGR_MOUNT_POINTS` | `SymbolicLinkName` (contains GUID) |
| `\Driver\mountmgr` | `IOCTL_MOUNTDEV_QUERY_UNIQUE_ID` (0x4D0000) | `MOUNTDEV_UNIQUE_ID` | `UniqueId` (raw GUID) |

### GUID Format

```c
#define VOLUME_GUID_MAX_LENGTH (0x24) // 36 characters
#define GUID_OFFSET 22 // Offset in symbolic link name

// Example symbolic link: \\?\Volume{12345678-1234-1234-1234-123456789012}\
//                                   ^         ^    ^    ^    ^
//                        Offsets:   22        31   36   41   46

bool FindFakeVolumeGUID(wchar_t* pOriginal) {
    // 1. Check cache
    for (auto& serial : *vVolGUIDs) {
        if (!memcmp(serial.orig, pOriginal, 4)) {
            Memory::WriteProtected(pOriginal, serial.spoofed, VOLUME_GUID_MAX_LENGTH * 2);
            return true;
        }
    }

    // 2. Generate new GUID
    wchar_t* pBuf = kMalloc((VOLUME_GUID_MAX_LENGTH + 1) * 2);
    wchar_t* pBufOrig = kMalloc((VOLUME_GUID_MAX_LENGTH + 1) * 2);
    memcpy(pBuf, pOriginal, VOLUME_GUID_MAX_LENGTH * 2);
    memcpy(pBufOrig, pOriginal, VOLUME_GUID_MAX_LENGTH * 2);

    // Format: {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}
    //          01234567890123456789012345678901234567
    //          {       -    -    -    -            }

    rnd.w_str_hex(pBuf + 2, 2);   // First 2 hex digits (keep '{' and first digit)
    rnd.w_str_hex(pBuf + 9, 4);   // 4 hex digits after first '-'
    rnd.w_str_hex(pBuf + 14, 4);  // 4 hex digits after second '-'
    rnd.random_shuffle(pBuf + 19, 4); // Last section

    MOUNT_SERIAL_DATA serial = { .orig = pBufOrig, .spoofed = pBuf };
    vVolGUIDs->Append(serial);
    Memory::WriteProtected(pOriginal, pBuf, VOLUME_GUID_MAX_LENGTH * 2);
    return true;
}
```

**Completion Routine**:
```c
NTSTATUS MountPointsIoc(PDEVICE_OBJECT device, PIRP irp, PVOID context) {
    IOC_REQUEST request = *(PIOC_REQUEST)context;
    PMOUNTMGR_MOUNT_POINTS points = (PMOUNTMGR_MOUNT_POINTS)request.Buffer;

    for (DWORD32 i = 0; i < points->NumberOfMountPoints; ++i) {
        PMOUNTMGR_MOUNT_POINT point = &points->MountPoints[i];

        // Check if symbolic link contains "{" at expected offset
        wchar_t* pSymLink = (wchar_t*)((char*)points + point->SymbolicLinkNameOffset);
        if (pSymLink[GUID_OFFSET / 2 - 1] == L'{') {
            FindFakeVolumeGUID((wchar_t*)((char*)pSymLink + GUID_OFFSET));
        }
    }

    return request.OldRoutine ? request.OldRoutine(device, irp, request.OldContext) : STATUS_SUCCESS;
}
```

## USB Device Spoofing

### Approach

USB device serial numbers are exposed via USB hub IOCTLs. The strategy is to **nullify** (zero out) the `iSerialNumber` index rather than randomize it, preventing Windows from querying the serial string.

### Hook Points

| IOCTL | Structure | Field Modified |
|-------|-----------|----------------|
| `IOCTL_USB_GET_NODE_CONNECTION_INFORMATION` (0x220408) | `USB_NODE_CONNECTION_INFORMATION` | `DeviceDescriptor.iSerialNumber = 0` |
| `IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX` (0x220448) | `USB_NODE_CONNECTION_INFORMATION_EX` | `DeviceDescriptor.iSerialNumber = 0` |
| `IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION` (0x220410) | (Blocked) | Returns `STATUS_SUCCESS` without processing |

```c
NTSTATUS GetNodeConnectionInfoExIoc(PDEVICE_OBJECT device, PIRP irp, PVOID context) {
    IOC_REQUEST request = *(PIOC_REQUEST)context;
    PUSB_NODE_CONNECTION_INFORMATION_EX pUsbInfo = (PUSB_NODE_CONNECTION_INFORMATION_EX)request.Buffer;

    pUsbInfo->DeviceDescriptor.iSerialNumber = 0;
    // Optionally null out:
    // pUsbInfo->DeviceDescriptor.iProduct = 0;
    // pUsbInfo->DeviceDescriptor.iManufacturer = 0;
    // pUsbInfo->DeviceDescriptor.idProduct = 0;
    // pUsbInfo->DeviceDescriptor.idVendor = 0;

    return STATUS_SUCCESS;
}
```

**Hub Enumeration**:
```c
bool usb::Spoof(DWORD64 seed) {
    const GUID GUID_USB_HUB = { 0xf18a0e88, 0xc30c, 0x11d0,
                                 0x88, 0x15, 0x00, 0xa0, 0xc9, 0x06, 0xbe, 0xd8 };

    PWCHAR pDeviceNames = nullptr;
    IoGetDeviceInterfaces(&GUID_USB_HUB, nullptr, DEVICE_INTERFACE_INCLUDE_NONACTIVE, &pDeviceNames);

    // Parse null-terminated string list
    list<string> usbInterfaces;
    while (*pDeviceNames) {
        int strLen = wcslen(pDeviceNames);
        usbInterfaces.emplace_back(pDeviceNames);
        pDeviceNames += strLen + 1;
    }

    for (auto& interface : usbInterfaces) {
        PDEVICE_OBJECT pCurrDevObj = GetDeviceFromIName(interface);
        EPT::HookExec(pCurrDevObj->DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL],
                      UsbHubControl);
    }

    return true;
}
```

## EFI Variable Protection

### Firmware Environment Variables

Windows exposes UEFI firmware variables via:
```c
NTSTATUS ExGetFirmwareEnvironmentVariable(
    PUNICODE_STRING VariableName,
    LPGUID VendorGuid,
    PVOID Value,
    PULONG ValueLength,
    PULONG Attributes
);

NTSTATUS ExSetFirmwareEnvironmentVariable(
    PUNICODE_STRING VariableName,
    LPGUID VendorGuid,
    PVOID Value,
    ULONG ValueLength,
    ULONG Attributes
);
```

**Target GUID**: `{eaec226f-c9a3-477a-a826-ddc716cdc0e3}` (Windows Boot Environment)

### Variables Spoofed

| Variable Name | Purpose | Spoof Method |
|---------------|---------|--------------|
| `UnlockIDCopy` | TPM unlock ID | Randomize all bytes |
| `OfflineUniqueIDRandomSeed` | Unique ID seed | Randomize all bytes |
| `OfflineUniqueIDRandomSeedCRC` | CRC of seed | Randomize all bytes |
| `OfflineUniqueIDEKPub` | TPM endorsement key public | Randomize all bytes |
| `OfflineUniqueIDEKPubCRC` | CRC of EK pub | Randomize all bytes |
| `PlatformModuleData` | Platform-specific data | Randomize all bytes |

```c
bool SpoofFirmwareEntry(string entryName) {
    string guidString("{eaec226f-c9a3-477a-a826-ddc716cdc0e3}");
    GUID guid;
    RtlGUIDFromString(&guidString.unicode(), &guid);

    // 1. Read current value
    char buffer[0x100];
    ULONG length = 0xff;
    ULONG attributes = 0;
    ExGetFirmwareEnvironmentVariable(&entryName.unicode(), &guid, buffer, &length, &attributes);

    // 2. Randomize
    rnd.c_str(buffer, length);

    // 3. Write back
    attributes |= VARIABLE_ATTRIBUTE_NON_VOLATILE;
    ExSetFirmwareEnvironmentVariable(&entryName.unicode(), &guid, buffer, length, attributes);

    return true;
}
```

## Random Value Generation

### Seeded PRNG

All randomization uses a **deterministic PRNG** seeded with a global value:

```c
DWORD64 spoofer::seed = 0; // Set once at driver initialization

void GenerateSerial() {
    rnd.setSeed(spoofer::seed);
    rnd.setSecLevel(random::SecurityLevel::PREDICTABLE);

    rnd.bytes(buffer, length);               // Fill buffer with random bytes
    rnd.random_shuffle(buffer, length);      // Shuffle existing bytes
    rnd.w_str_hex(wbuffer, length);          // Generate hex wchar string
    rnd.random_shuffle_ignore_chars(buffer, len, " _-.", 4); // Shuffle but preserve chars
}
```

**Why Deterministic?**:
1. **Consistency**: Same seed = same spoofed values across reboots
2. **Reversibility**: Can revert to original values by re-running with reset logic
3. **Correlation**: All hardware IDs change in sync (no mismatched serials)

### Character Preservation

```c
void random_shuffle_ignore_chars(char* buffer, int length, char* ignoreChars, int ignoreCount) {
    for (int i = 0; i < length; i++) {
        bool bIgnore = false;
        for (int j = 0; j < ignoreCount; j++) {
            if (buffer[i] == ignoreChars[j]) {
                bIgnore = true;
                break;
            }
        }

        if (!bIgnore) {
            int swapIdx = Next(0, length - 1); // Random index
            // Swap buffer[i] with buffer[swapIdx]
            char temp = buffer[i];
            buffer[i] = buffer[swapIdx];
            buffer[swapIdx] = temp;
        }
    }
}
```

This ensures format characters like spaces, dashes, and periods remain in place:
- `"S4NENS0M 123456A"` → `"S4NENS0M 98K3L2A"` (space preserved)
- `"WD-WCAV12345678"` → `"WD-WCAV98K3L2M4"` (dash preserved)

## C Conversion Notes

### String Handling Without std::string

**C++ Code**:
```cpp
vector<DISK_SERIAL_DATA>* vDiskSerials = nullptr;

struct DISK_SERIAL_DATA {
    char* spoofed;
    char* orig;
    size_t sz;
};

for (auto& serial : *vDiskSerials) {
    if (!memcmp(serial.orig, pOriginal, serial.sz)) {
        // Found match
    }
}
```

**C Equivalent**:
```c
typedef struct {
    char* spoofed;
    char* orig;
    size_t sz;
} disk_serial_data_t;

typedef struct {
    disk_serial_data_t* data;
    size_t count;
    size_t capacity;
} disk_serial_list_t;

disk_serial_list_t* disk_serials = NULL;

// Initialization
disk_serials = ombra_malloc(sizeof(disk_serial_list_t));
disk_serials->data = ombra_malloc(64 * sizeof(disk_serial_data_t));
disk_serials->count = 0;
disk_serials->capacity = 64;

// Iteration
for (size_t i = 0; i < disk_serials->count; i++) {
    disk_serial_data_t* serial = &disk_serials->data[i];
    if (memcmp(serial->orig, original, serial->sz) == 0) {
        // Found match
    }
}

// Append
if (disk_serials->count >= disk_serials->capacity) {
    disk_serials->capacity *= 2;
    disk_serials->data = ombra_realloc(disk_serials->data,
                                       disk_serials->capacity * sizeof(disk_serial_data_t));
}
disk_serials->data[disk_serials->count++] = new_entry;
```

### Buffer Management

**C++ Code**:
```cpp
char* pBuf = (char*)cpp::kMalloc(length, PAGE_READWRITE);
RtlZeroMemory(pBuf, length);
// ... use pBuf
cpp::kFree(pBuf);
```

**C Equivalent**:
```c
char* buf = ombra_malloc_zero(length);
// ... use buf
ombra_free(buf);

// Helper functions
void* ombra_malloc(size_t size) {
    return ExAllocatePoolWithTag(NonPagedPool, size, 'arbO'); // 'Obra' reversed
}

void* ombra_malloc_zero(size_t size) {
    void* ptr = ombra_malloc(size);
    if (ptr)
        RtlZeroMemory(ptr, size);
    return ptr;
}

void ombra_free(void* ptr) {
    if (ptr)
        ExFreePoolWithTag(ptr, 'arbO');
}
```

### Hook Installation Pattern in C

**C++ Code**:
```cpp
HOOK_SECONDARY_INFO hkSecondaryInfo = { 0 };
hkSecondaryInfo.pOrigFn = (PVOID*)&PartmgrIoCtrlOrig;

EPT::HookExec(pDrivObj->MajorFunction[IRP_MJ_DEVICE_CONTROL],
              &PartmgrIoCtrlHook,
              hkSecondaryInfo);
```

**C Equivalent**:
```c
typedef NTSTATUS (*driver_dispatch_t)(PDEVICE_OBJECT, PIRP);

driver_dispatch_t partmgr_ioctl_orig = NULL;

hook_secondary_info_t hook_info = { 0 };
hook_info.orig_fn = (void**)&partmgr_ioctl_orig;

ept_hook_exec(driver_obj->MajorFunction[IRP_MJ_DEVICE_CONTROL],
              partmgr_ioctl_hook,
              &hook_info);

// Hook function
NTSTATUS partmgr_ioctl_hook(PDEVICE_OBJECT device, PIRP irp) {
    // ... custom logic
    return partmgr_ioctl_orig(device, irp);
}
```

### Completion Routine Pattern

**C++ Code**:
```cpp
void ChangeIoc(PIO_STACK_LOCATION ioc, PIRP irp, PIO_COMPLETION_ROUTINE routine) {
    PIOC_REQUEST request = (PIOC_REQUEST)ExAllocatePool(NonPagedPool, sizeof(IOC_REQUEST));
    request->Buffer = irp->UserBuffer ?: irp->AssociatedIrp.SystemBuffer;
    request->BufferLength = ioc->Parameters.DeviceIoControl.OutputBufferLength;
    request->OldRoutine = ioc->CompletionRoutine;
    request->OldContext = ioc->Context;

    ioc->Control = 0;
    ioc->Control |= SL_INVOKE_ON_SUCCESS;
    ioc->Context = request;
    ioc->CompletionRoutine = routine;
}
```

**C Equivalent**:
```c
typedef struct {
    void* buffer;
    uint32_t buffer_length;
    PIO_COMPLETION_ROUTINE old_routine;
    void* old_context;
} ioc_request_t;

void change_ioc(PIO_STACK_LOCATION ioc, PIRP irp, PIO_COMPLETION_ROUTINE routine) {
    ioc_request_t* request = ExAllocatePoolWithTag(NonPagedPool, sizeof(ioc_request_t), 'arbO');
    request->buffer = irp->UserBuffer ? irp->UserBuffer : irp->AssociatedIrp.SystemBuffer;
    request->buffer_length = ioc->Parameters.DeviceIoControl.OutputBufferLength;
    request->old_routine = ioc->CompletionRoutine;
    request->old_context = ioc->Context;

    ioc->Control = SL_INVOKE_ON_SUCCESS;
    ioc->Context = request;
    ioc->CompletionRoutine = routine;
}

NTSTATUS ata_pass_ioc(PDEVICE_OBJECT device, PIRP irp, PVOID context) {
    if (!MmIsAddressValid(context))
        return STATUS_SUCCESS;

    ioc_request_t* request = (ioc_request_t*)context;

    // Process buffer
    IDENTIFY_DEVICE_DATA* device_data = (IDENTIFY_DEVICE_DATA*)((BYTE*)request->buffer + offset);
    find_fake_disk_serial((char*)device_data->SerialNumber);

    // Free context
    ExFreePoolWithTag(context, 'arbO');

    // Call old routine
    if (request->old_routine && irp->StackCount > 1)
        return request->old_routine(device, irp, request->old_context);

    return STATUS_SUCCESS;
}
```

### Identity Mapping in C

**C++ Code**:
```cpp
identity::PhysicalAccess phys(target_cr3);
char* serial = (char*)phys.Read<char*>(serial_va);
phys.Write(serial_va, spoofed_serial, 20);
```

**C Equivalent**:
```c
typedef struct {
    char* identity_base;
    CR3 cr3;
    bool allocated;
} physical_access_t;

physical_access_t* phys_access_create(uint64_t cr3) {
    physical_access_t* phys = ombra_malloc(sizeof(physical_access_t));
    phys->cr3.Flags = cr3;
    phys->identity_base = (char*)map_identity_untracked(phys->cr3);
    phys->allocated = true;
    return phys;
}

void phys_access_destroy(physical_access_t* phys) {
    if (phys->allocated)
        reset_cache_untracked(phys->cr3);
    ombra_free(phys);
}

uint64_t phys_get_physical_address(physical_access_t* phys, uintptr_t va) {
    // Walk page tables using phys->identity_base + PFN * PAGE_SIZE
    // (See identity.cpp lines 57-131 for full implementation)
}

bool phys_read(physical_access_t* phys, void* src_va, void* dst, size_t size) {
    while (size > 0) {
        uintptr_t phys_addr = phys_get_physical_address(phys, (uintptr_t)src_va);
        if (!phys_addr)
            return false;

        size_t page_remaining = PAGE_SIZE - (phys_addr & (PAGE_SIZE - 1));
        size_t copy_size = size < page_remaining ? size : page_remaining;

        memcpy(dst, phys->identity_base + phys_addr, copy_size);

        dst = (char*)dst + copy_size;
        src_va = (char*)src_va + copy_size;
        size -= copy_size;
    }
    return true;
}
```

### SMBIOS Parser in C

**C++ Code**:
```cpp
smbios::Parser parser(data, size, version);
auto entry = parser.next();
while (entry) {
    switch (entry->type) {
    case TYPE_SYSTEM_INFO:
        // ...
        break;
    }
    entry = parser.next();
}
```

**C Equivalent**:
```c
typedef struct {
    char* data;
    size_t size;
    char* ptr;
    char* start;
    int version;
    smbios_entry_t entry;
} smbios_parser_t;

smbios_parser_t* smbios_parser_create(char* data, size_t size, int version) {
    smbios_parser_t* parser = ombra_malloc(sizeof(smbios_parser_t));
    parser->data = data;
    parser->size = size;
    parser->version = version;
    parser->ptr = NULL;
    parser->start = NULL;
    RtlZeroMemory(&parser->entry, sizeof(parser->entry));
    return parser;
}

smbios_entry_t* smbios_parser_next(smbios_parser_t* parser) {
    if (parser->ptr == NULL) {
        parser->ptr = parser->start = parser->data;
    } else {
        // Find double null terminator
        parser->ptr = parser->start + parser->entry.length - 4;
        while (parser->ptr < parser->data + parser->size - 1 &&
               !(parser->ptr[0] == 0 && parser->ptr[1] == 0))
            parser->ptr++;
        parser->ptr += 2;

        if (parser->ptr >= parser->data + parser->size)
            return NULL;
    }

    // Read header
    parser->entry.type = *parser->ptr++;
    parser->entry.length = *parser->ptr++;
    parser->entry.handle = *(uint16_t*)parser->ptr;
    parser->ptr += 2;
    parser->entry.rawdata = parser->ptr - 4;
    parser->entry.strings = (char*)parser->entry.rawdata + parser->entry.length;
    parser->start = parser->ptr;

    if (parser->entry.type == 127)
        return NULL;

    return smbios_parse_entry(parser);
}

char* smbios_get_string(smbios_parser_t* parser, int index) {
    if (index <= 0) return "";

    char* ptr = (char*)parser->start + parser->entry.length - 4;
    for (int i = 1; *ptr != 0 && i < index; ++i) {
        while (*ptr != 0) ptr++;
        ptr++;
    }
    return ptr;
}
```

## Testing Checklist

### Disk Serial Spoofing

- [ ] Query disk serial via `IOCTL_STORAGE_QUERY_PROPERTY` (usermode)
- [ ] Query via WMI `Win32_DiskDrive.SerialNumber`
- [ ] Query via `SMART_RCV_DRIVE_DATA` (HDD health tools)
- [ ] Query NVMe via `STORAGE_PROTOCOL_SPECIFIC_DATA`
- [ ] Verify WWN randomization for SAS drives
- [ ] Verify IEEE OUI randomization for NVMe
- [ ] Test with multiple drives (SATA + NVMe)
- [ ] Verify RAID controller spoofing (StorPort drivers)
- [ ] Test serial persistence across reboots (same seed)

### NIC MAC Spoofing

- [ ] Query MAC via `getmac` command
- [ ] Query via `ipconfig /all`
- [ ] Query via WMI `Win32_NetworkAdapter.MACAddress`
- [ ] Query via NDIS OID queries (driver-level)
- [ ] Verify ARP table blocking (NSI queries)
- [ ] Test with multiple NICs (Ethernet + WiFi)
- [ ] Verify interface GUID randomization
- [ ] Test MAC persistence across reboots

### SMBIOS Spoofing

- [ ] Query via WMI `Win32_ComputerSystemProduct.UUID`
- [ ] Query via WMI `Win32_BaseBoard.SerialNumber`
- [ ] Query via `wmic csproduct get uuid`
- [ ] Query via `wmic baseboard get serialnumber`
- [ ] Verify EPT shadow pages active (kernel debugger)
- [ ] Verify registry `SMBiosData` key updated
- [ ] Test with SMBIOS-reading tools (HWiNFO, CPU-Z)
- [ ] Verify all 6 table types spoofed

### GPU Spoofing

- [ ] Query via WMI `Win32_VideoController.PNPDeviceID`
- [ ] Query via NVIDIA Control Panel → System Information
- [ ] Query via `nvidia-smi -L` (if CUDA installed)
- [ ] Test with multiple NVIDIA GPUs
- [ ] Verify UUID randomization (pattern scan success)

### Volume GUID Spoofing

- [ ] Query via `mountvol` command
- [ ] Query via `Get-Volume` PowerShell cmdlet
- [ ] Enumerate via `FindFirstVolume` / `FindNextVolume` API
- [ ] Verify GUID format preservation (`{...}` braces)
- [ ] Test with multiple volumes

### USB Spoofing

- [ ] Query via Device Manager → USB Device Properties
- [ ] Query via `usbview.exe` (Windows SDK tool)
- [ ] Verify `iSerialNumber` index is 0
- [ ] Test with multiple USB devices connected

### EFI Variable Spoofing

- [ ] Query via `bcdedit /enum {bootmgr}` (check GUIDs)
- [ ] Query via PowerShell `Get-SecureBootUEFI -Name PK`
- [ ] Verify 6 EFI variables randomized
- [ ] Test on UEFI-enabled system only (not legacy BIOS)

### Integration Testing

- [ ] Run all spoofers in sequence (`spoofer::SpoofAll`)
- [ ] Verify no BSODs during hook installation
- [ ] Test with anti-cheat sandboxes (read-only monitoring)
- [ ] Verify no performance degradation (IOCTL latency)
- [ ] Test spoof reset functionality (restore originals)
- [ ] Verify deterministic spoofing (same seed = same results)
- [ ] Test on Windows 10 (multiple builds) and Windows 11

### Edge Cases

- [ ] System with no NVMe drives (SATA only)
- [ ] System with no NVIDIA GPU (Intel/AMD integrated)
- [ ] Virtual machine (Hyper-V, VMware) - some IOCTLs may differ
- [ ] Disk controller change (driver stack different)
- [ ] Network adapter disabled/enabled during runtime
- [ ] USB device hot-plug during runtime
- [ ] Multiple monitors (EDID spoofing if implemented)

---

## Summary

This hardware spoofing system operates at the kernel driver level by:

1. **Hooking IRP_MJ_DEVICE_CONTROL** handlers for disk, NIC, USB, and volume drivers
2. **Installing completion routines** that intercept IOCTL responses before they reach usermode
3. **Caching original→spoofed mappings** to ensure consistency
4. **Using deterministic RNG** seeded globally for correlated spoofing
5. **EPT shadow paging** for SMBIOS tables (presents different data to guests)
6. **Pattern scanning** for closed-source drivers (NVIDIA GPU)
7. **Physical memory identity mapping** for cross-process VA→PA translation

**Total Attack Surface**: 17 disk IOCTLs + 8 NIC OIDs + 6 SMBIOS table types + 3 USB IOCTLs + 2 volume IOCTLs + 6 EFI variables + GPU UUID = comprehensive hardware fingerprint evasion.

**C Port Complexity**: Medium-High. Requires careful manual memory management, struct-based polymorphism for SMBIOS parsing, and detailed understanding of IOCTL completion routine mechanics.
