# ANTI-CHEAT SPECIFIC EVASION - C++ to C + Assembly Port Guide

## Overview

This document covers all anti-cheat evasion techniques used in the Ombra Hypervisor, specifically targeting **EasyAntiCheat (EAC)**, **Windows Defender**, and general hardware/software fingerprinting methods. These techniques operate at **Ring 0** (kernel driver level) and rely heavily on **IOCTL hooking**, **EPT-based stealth**, **kernel structure manipulation**, and **forensic artifact cleanup**.

**Target Anti-Cheats:**
- EasyAntiCheat (EAC) - NMI callback interception, CR3 tracking
- Windows Defender - WdFilter.sys filter list manipulation
- BattlEye (BE) - General IOCTL/WMI interception applies
- Riot Vanguard - General IOCTL/WMI interception applies

**Key Insight:** Modern anti-cheats detect via **hardware fingerprinting** (disk serials, NIC MACs, SMBIOS), **kernel artifact enumeration** (MmUnloadedDrivers, PiDDBCacheTable), and **runtime telemetry** (ETW-TI). The evasion strategy is **multi-layered**:

1. **Pre-Load Phase** (Usermode): Clean prefetch files, disable ETW-TI provider
2. **Load Phase** (Kernel): Hook IOCTL handlers, spoof WMI queries, hide window handles
3. **Post-Load Phase** (Kernel): Clean MmUnloadedDrivers, PiDDBCacheTable, CI.dll hash buckets
4. **Runtime Phase** (Hypervisor): EPT shadow paging, NMI blocking

---

## File Inventory

### EasyAntiCheat Evasion
| File | Purpose |
|------|---------|
| `OmbraCoreLib-v/include/eac.h` | EAC-specific CR3 tracking and NMI blocking |
| `OmbraCoreLib-v/src/eac.cpp` | Implementation (depends on C++ vector, identity mapping) |

### WMI Query Interception
| File | Purpose |
|------|---------|
| `OmbraCoreLib-v/include/wmispoof.h` | WMI monitor serial spoofing structures and GUIDs |
| `OmbraCoreLib-v/src/wmispoof.cpp` | WmipQueryAllData hook for monitor serial spoofing |

### Window Enumeration Hiding
| File | Purpose |
|------|---------|
| `OmbraCoreLib-v/include/windowspoof.h` | Window handle hiding interface |
| `OmbraCoreLib-v/src/windowspoof.cpp` | TEB ClientInfo manipulation for window cache bypass |

### IOCTL Hook Infrastructure
| File | Purpose |
|------|---------|
| `OmbraCoreLib-v/include/ioctlhook.h` | Shared IOCTL hook definitions (disk, NIC, WMI) |
| `OmbraCoreLib-v/src/ioctrlhook.cpp` | IOC_REQUEST structure and completion routine helpers |

### Windows Defender Evasion
| File | Purpose |
|------|---------|
| `OmbraCoreLib/include/defender.h` | WdFilter.sys filter list manipulation |
| `OmbraCoreLib/src/defender.cpp` | Unlink driver from WdFilter's internal gTable |

### ETW Telemetry Blinding
| File | Purpose |
|------|---------|
| `OmbraLoader/etw_resolver.h` | Resolve EtwThreatIntProvRegHandle offset (build-specific) |

### Prefetch/Forensic Cleanup
| File | Purpose |
|------|---------|
| `OmbraLoader/prefetch_cleanup.h` | Secure-delete prefetch files (3-pass wipe) |

### Kernel Artifact Cleanup
| File | Purpose |
|------|---------|
| `OmbraCoreLib/src/winternlex.cpp` | ClearPIDDBCacheTable, ClearKernelHashBucketList, ClearMmUnloadedDrivers |
| `OmbraCoreLib/include/Setup.hpp` | PDB-based offset initialization for PiDDBLock/PiDDBCacheTable |

### Hardware Spoofing
| File | Purpose |
|------|---------|
| `OmbraCoreLib-v/include/diskspoof.h` | Disk serial/model/GUID/WWN/IEEE spoofing via IOCTL hooks |
| `OmbraCoreLib-v/src/diskspoof.cpp` | SCSI/NVMe/ATA/RAID hook implementations |
| `OmbraCoreLib-v/include/nicspoof.h` | NIC MAC address spoofing via NDIS/NSI/TCP IOCTL hooks |
| `OmbraCoreLib-v/src/nicspoof.cpp` | MAC modification tracking and IOCTL interception |

---

## Architecture Summary

```
┌─────────────────────────────────────────────────────────────────────────┐
│ DETECTION SURFACE                                                        │
├─────────────────────────────────────────────────────────────────────────┤
│ 1. Hardware Fingerprints:                                               │
│    - Disk serials (SCSI, NVMe, ATA, RAID)                               │
│    - NIC MAC addresses (permanent + current)                            │
│    - Monitor serials (WMI)                                              │
│    - SMBIOS data (WMI)                                                  │
│    - GUID/WWN/IEEE identifiers                                          │
│                                                                          │
│ 2. Kernel Artifacts:                                                    │
│    - MmUnloadedDrivers                                                  │
│    - PiDDBCacheTable (driver timestamp cache)                           │
│    - CI.dll KernelHashBucketList (code integrity)                       │
│    - WdFilter.sys gTable (Windows Defender filter list)                 │
│                                                                          │
│ 3. Usermode Artifacts:                                                  │
│    - Prefetch files (C:\Windows\Prefetch\*.pf)                          │
│    - ETW circular buffers (kernel telemetry)                            │
│                                                                          │
│ 4. Runtime Detection:                                                   │
│    - NMI callbacks (EAC)                                                │
│    - CR3 switching detection (EAC)                                      │
│    - Window enumeration (FindWindow, EnumWindows)                       │
└─────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────┐
│ EVASION LAYERS                                                           │
├─────────────────────────────────────────────────────────────────────────┤
│ Layer 1: Usermode Pre-Load (OmbraLoader.exe)                            │
│   - prefetch::CleanupKnownArtifacts()  [3-pass secure delete]           │
│   - etw::DisableEtwTi()                 [Blind ETW-TI provider]          │
│                                                                          │
│ Layer 2: Kernel Driver (OmbraDriver.sys)                                │
│   - EPT::HookExec(WmipQueryAllData)    [Monitor serial spoofing]        │
│   - window::Hide()                      [TEB ClientInfo cache poison]   │
│   - defender::CleanFilterList()         [WdFilter bypass]               │
│   - Hook IRP_MJ_DEVICE_CONTROL:                                         │
│     * Disk drivers (storport, nvme, storahci, partmgr)                  │
│     * NIC drivers (ndis, nsi, tcp)                                      │
│                                                                          │
│ Layer 3: Post-Load Cleanup (OmbraDriver DriverEntry)                    │
│   - winternl::ClearPIDDBCacheTable()   [Remove driver hash]             │
│   - winternl::ClearKernelHashBucketList() [CI.dll bypass]               │
│   - winternl::ClearMmUnloadedDrivers() [Zero driver name length]        │
│                                                                          │
│ Layer 4: Hypervisor (PayLoad.dll)                                       │
│   - EPT shadow paging (driver memory invisible)                         │
│   - eac::BlockNmi()                     [Queue NMI callbacks]           │
│   - eac::UpdateCr3()                    [CR3 tracking for EAC]          │
│   - VMCALL_WIPE_ETW_BUFFERS             [Post-load telemetry cleanup]   │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## EasyAntiCheat (EAC) Evasion

### Detection Vectors

**EAC Runtime Checks:**
1. **NMI Callbacks** - Registers APIC NMI handler to detect hypervisors via unexpected NMI behavior
2. **CR3 Switching** - Monitors guest CR3 changes during VMExits to detect memory manipulation
3. **Kernel Driver Enumeration** - Scans PsLoadedModuleList, MmUnloadedDrivers
4. **Hardware Fingerprinting** - Queries disk serials, NIC MACs via IOCTL (uses same vectors as general spoofing)
5. **Thread Context** - Compares KTHREAD context with user-reported TEB values

### Bypass Techniques

#### 1. NMI Blocking (`eac.h/cpp`)

**Purpose:** EAC uses NMI callbacks to detect hypervisors. We queue NMIs instead of executing them immediately.

**Key Structures:**
```cpp
// eac.h:9-33
typedef struct _CR3_TRACKING {
    union {
        PVOID pImageBase;          // For CR3 tracking
        DWORD64 dwNmiQueue[64];    // For NMI blocking (per-core queue)
    };
    DWORD64* pCr3;                 // Pointer to CR3 storage location
    DWORD64 srcCr3;                // Source CR3 value (process identifier)
} CR3_TRACKING;
```

**Operation:**
```cpp
// eac.cpp:75-98 - Block NMI for a specific CR3 (process)
void eac::BlockNmi(CR3 cr3) {
    CR3_TRACKING tracking;
    tracking.srcCr3 = cr3.Flags;

    // Check if already blocked
    for (auto& tracked : *pNmiBlockedCr3s) {
        if (tracked.srcCr3 == cr3.Flags) return;
    }

    pNmiBlockedCr3s->Append(tracking);
}

// eac.cpp:129-151 - Get and decrement NMI count for current core
int eac::GetAndDecreaseNmiCount(CR3 cr3) {
    auto dwCore = CPU::GetCPUIndex(true);
    auto& count = pNmiBlockedCr3s->at(i).dwNmiQueue[dwCore];

    if (count) count -= 1;
    return count;
}
```

**VMExit Handler Integration:**
```assembly
; In AMD/Intel VMExit handler
check_nmi:
    mov rcx, [guest_cr3]
    call eac_IsNmiBlocked         ; Check if NMI should be blocked
    test al, al
    jnz queue_nmi                 ; If blocked, increment queue

execute_nmi:
    ; Pass NMI to guest
    jmp original_nmi_handler

queue_nmi:
    call eac_IncreaseNmiCount     ; Increment per-core queue
    ; Return to guest without NMI delivery
```

#### 2. CR3 Tracking (`eac.h/cpp`)

**Purpose:** EAC detects when guest CR3 changes unexpectedly (memory manipulation). We update tracked CR3 values to match.

**Operation:**
```cpp
// eac.cpp:32-47 - Update all tracked CR3s when guest CR3 changes
void eac::UpdateCr3(CR3 cr3) {
    identity::PhysicalAccess pa(vmm::pIdentityMap, cr3.Flags);

    for (auto& data : *pCr3s) {
        if (data.pCr3 && pa.getPhysicalAddress((DWORD64)data.pImageBase + PAGE_SIZE)) {
            // Write new CR3 to tracked location using source process CR3
            identity::PhysicalAccess paSrc(vmm::pIdentityMap, data.srcCr3);
            paSrc.Write<CR3>(data.pCr3, cr3);
        }
    }
}
```

**Hook Points:**
- Called from VMExit handler when `VMCS_GUEST_CR3` (Intel) or `vmcb->Cr3()` (AMD) changes
- Requires **identity mapping** (VMXRoot can walk guest page tables)

---

## Windows Defender Evasion

### Detection Vector

**WdFilter.sys** (Windows Defender minifilter driver) maintains an internal linked list (`gTable`) of all loaded drivers. Anti-cheats query this via undocumented interfaces.

### Bypass Technique (`defender.h/cpp`)

**Operation:**
```cpp
// defender.cpp:3-37 - Unlink driver from WdFilter's gTable
bool defender::CleanFilterList(string driverName) {
    // Find WdFilter.sys base address
    PVOID wdfilterBase = Memory::GetKernelAddress((PCHAR)"WdFilter.sys");

    // Signature scan for gTable offset
    ULONG64 gTableOffset = (ULONG64)Memory::FindPatternImage(
        wdfilterBase,
        (PCHAR)"\x48\x8B\x0D\x00\x00\x00\x00\xFF\x05",
        (PCHAR)"xxx????xx"
    );
    ULONG64 gTable = gTableOffset + 7 + *(PINT)(gTableOffset + 3);
    LIST_ENTRY* gTableHead = (LIST_ENTRY*)(gTable - 0x8);

    // Walk linked list
    for (LIST_ENTRY* entry = gTableHead->Flink; entry != gTableHead; entry = entry->Flink) {
        UNICODE_STRING* pImageName = (UNICODE_STRING*)((ULONG64)entry + 0x10);

        if (wcsstr(imageName.w_str(), driverName.w_str())) {
            // Unlink entry
            entry->Blink->Flink = entry->Flink;
            entry->Flink->Blink = entry->Blink;
        }
    }
}
```

**C Port Requirements:**
- Replace `string` with manual `UNICODE_STRING` manipulation
- `Memory::GetKernelAddress()` → Walk `PsLoadedModuleList` manually
- `Memory::FindPatternImage()` → Manual pattern scan with `MmIsAddressValid()` checks

---

## WMI Query Interception

### Hooked WMI Classes

**WMI GUIDs** (from `wmispoof.h:11-18`):
```c
// Monitor serial (primary target)
DEFINE_GUID(WmiMonitorID_GUID,
    0x671a8285, 0x4edb, 0x4cae,
    0x99, 0xfe, 0x69, 0xa1, 0x5c, 0x48, 0xc0, 0xbc);

// NIC MAC addresses
DEFINE_GUID(MSNdis_EthernetCurrentAddress_GUID,
    0x44795700, 0xa61b, 0x11d0,
    0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c);

DEFINE_GUID(MSNdis_EthernetPermanentAddress_GUID,
    0x447956ff, 0xa61b, 0x11d0,
    0x8d, 0xd4, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c);
```

### Query Result Modification

**Hook Function** (`wmispoof.cpp:42-72`):
```cpp
NTSTATUS WmipQueryAllDataHook(
    IN PWMIGUIDOBJECT GuidObject,
    IN PIRP Irp,
    IN KPROCESSOR_MODE AccessMode,
    IN PWNODE_ALL_DATA Wnode,
    IN ULONG OutBufferLen,
    OUT PULONG RetSize
) {
    NTSTATUS ntStatus = pWmipQueryAllData(GuidObject, Irp, AccessMode, Wnode, OutBufferLen, RetSize);

    if (NT_SUCCESS(ntStatus)) {
        PWNODE_ALL_DATA pAllData = (PWNODE_ALL_DATA)Wnode;

        // Check if monitor serial query
        if (pAllData->WnodeHeader.Guid == WmiMonitorID_GUID) {
            PWmiMonitorID MonitorID;

            // Handle both fixed and variable instance sizes
            if (pAllData->WnodeHeader.Flags & WNODE_FLAG_FIXED_INSTANCE_SIZE)
                MonitorID = (PWmiMonitorID)&((UCHAR*)pAllData)[pAllData->DataBlockOffset];
            else
                MonitorID = (PWmiMonitorID)&((UCHAR*)pAllData)[pAllData->OffsetInstanceDataAndLength[0].OffsetInstanceData];

            // Zero out serial and product code
            RtlFillBytes(MonitorID->SerialNumberID, sizeof(MonitorID->SerialNumberID), 0);
            RtlFillBytes(MonitorID->ProductCodeID, sizeof(MonitorID->ProductCodeID), 0);
        }
    }

    return ntStatus;
}
```

**Hook Installation** (`wmispoof.cpp:102-116`):
```cpp
bool wmi::SpoofMonitor(DWORD64 seed) {
    DWORD64 WmipQueryAllData = ((DWORD64)winternl::ntoskrnlBase + offsets.WmipQueryAllData);

    HOOK_SECONDARY_INFO hkSecondaryInfo = { 0 };
    hkSecondaryInfo.pOrigFn = (PVOID*)&pWmipQueryAllData;

    // EPT hook (execute-only page redirect)
    if (!EPT::HookExec((PVOID)WmipQueryAllData, WmipQueryAllDataHook, hkSecondaryInfo)) {
        return false;
    }

    return true;
}
```

**Key Structures** (`wmispoof.h:68-129`):
```c
typedef struct _WNODE_HEADER {
    ULONG BufferSize;
    ULONG ProviderId;
    union {
        ULONG64 HistoricalContext;
        struct {
            ULONG Version;
            ULONG Linkage;
        };
    };
    union {
        ULONG CountLost;
        HANDLE KernelHandle;
        LARGE_INTEGER TimeStamp;
    };
    GUID Guid;
    ULONG ClientContext;
    ULONG Flags;
} WNODE_HEADER, *PWNODE_HEADER;

typedef struct tagWNODE_ALL_DATA {
    struct _WNODE_HEADER WnodeHeader;
    ULONG DataBlockOffset;
    ULONG InstanceCount;
    ULONG OffsetInstanceNameOffsets;
    union {
        ULONG FixedInstanceSize;
        OFFSETINSTANCEDATAANDLENGTH OffsetInstanceDataAndLength[1];
    };
} WNODE_ALL_DATA, *PWNODE_ALL_DATA;

typedef struct WmiMonitorID {
    USHORT ProductCodeID[16];
    USHORT SerialNumberID[16];
    USHORT ManufacturerName[16];
    UCHAR WeekOfManufacture;
    USHORT YearOfManufacture;
    USHORT UserFriendlyNameLength;
    USHORT UserFriendlyName[1];
} WmiMonitorID, *PWmiMonitorID;
```

---

## Window Enumeration Hiding

### Detection Vector

Anti-cheats use `FindWindow`, `EnumWindows`, `GetWindowText` to detect suspicious window titles (debuggers, cheat tools).

### Bypass Technique (`windowspoof.h/cpp`)

**Operation:**
```cpp
// windowspoof.cpp:4-9 - Poison window handle cache in TEB
void window::Hide(int hwnd, char* pTeb) {
    // Offset into TEB's ClientInfo array
    DWORD64* pClientInfo = (DWORD64*)(pTeb + offsets.ClientInfo);

    // Set hwnd cache entry
    pClientInfo[offsets.HwndCache] = (DWORD64)hwnd;
    pClientInfo[offsets.HwndCache + 1] = 0; // Zero tagWND cache
}
```

**How It Works:**
- `ClientInfo` is a cached array in TEB (Thread Environment Block) at offset `Win32ClientInfo` (from ntdll.pdb)
- `HwndCache` offset (typically +8) stores the last accessed HWND
- By setting the cache to our target HWND and zeroing the `tagWND` pointer, subsequent `FindWindow` calls return cached NULL

**Offsets** (from `Setup.hpp:44-54`):
```cpp
// Resolved from ntdll.pdb at load time
auto parser = PdbParser(L".\\ntdll.pdb");
ULONG64 Win32ClientInfoOffset = parser.GetStructMemberOffset(L"_TEB", L"Win32ClientInfo");
offsets.ClientInfo = Win32ClientInfoOffset;
offsets.HwndCache = 8;  // Fixed offset within ClientInfo array
```

---

## IOCTL Hook Detection Bypass

### IOCTL Hook Infrastructure (`ioctlhook.h/cpp`)

**Purpose:** Anti-cheats detect IOCTL hooks by comparing IRP completion routines. We use per-request context to hide our hooks.

**IOC_REQUEST Structure** (`ioctlhook.h:32-37`):
```c
typedef struct _IOC_REQUEST {
    PVOID Buffer;                       // Original SystemBuffer
    ULONG BufferLength;                 // Original OutputBufferLength
    PVOID OldContext;                   // Original completion routine context
    PIO_COMPLETION_ROUTINE OldRoutine;  // Original completion routine
} IOC_REQUEST, *PIOC_REQUEST;
```

**Hook Installation** (`ioctlhook.cpp:7-22`):
```c
void ChangeIoc(PIO_STACK_LOCATION ioc, PIRP irp, PIO_COMPLETION_ROUTINE routine) {
    // Allocate per-request context
    PIOC_REQUEST request = (PIOC_REQUEST)ExAllocatePool(NonPagedPoolNx, sizeof(IOC_REQUEST));

    // Save original values
    request->Buffer = irp->AssociatedIrp.SystemBuffer;
    request->BufferLength = ioc->Parameters.DeviceIoControl.OutputBufferLength;
    request->OldContext = ioc->Context;
    request->OldRoutine = ioc->CompletionRoutine;

    // Replace with our hook
    ioc->Control = SL_INVOKE_ON_SUCCESS;
    ioc->Context = request;
    ioc->CompletionRoutine = routine;
}
```

**Hook Restoration:**
```c
// In completion routine
NTSTATUS MyCompletionRoutine(PDEVICE_OBJECT device, PIRP irp, PVOID context) {
    if (MmIsAddressValid(context)) {
        IOC_REQUEST request = *(PIOC_REQUEST)context;
        ExFreePool(context);

        // Modify buffer here (disk serial, NIC MAC, etc.)

        // Chain to original completion routine
        if (request.OldRoutine && irp->StackCount > 1) {
            return request.OldRoutine(device, irp, request.OldContext);
        }
    }

    return STATUS_SUCCESS;
}
```

---

## ETW-TI Provider Blinding

### ETW Threat Intelligence (ETW-TI)

**Purpose:** Microsoft's kernel telemetry system records driver loads, IOCTL activity, process creation. Anti-cheats query ETW buffers.

### Bypass Strategy (`etw_resolver.h`)

**Phase 1: Disable Provider** (before driver load):
```cpp
// etw_resolver.h:187-205
EtwResolution ResolveEtwParameters() {
    EtwResolution result = {};

    result.build_number = GetWindowsBuildNumber();
    result.ntoskrnl_base = GetNtoskrnlBase();  // Via NtQuerySystemInformation

    if (!ResolveEtwTiOffset(result.offset)) {
        result.success = false;
        return result;
    }

    result.success = true;
    return result;
}
```

**Build-Specific Offsets** (`etw_resolver.h:29-47`):
```c
constexpr BuildOffsetEntry g_known_offsets[] = {
    { 19045, 0x00C1D2A0, "Win10 22H2 (19045)" },
    { 22000, 0x00C3A540, "Win11 21H2 (22000)" },
    { 22621, 0x00C41280, "Win11 22H2 (22621)" },
    { 22631, 0x00C42890, "Win11 23H2 (22631)" },
    { 26100, 0x00C58A40, "Win11 24H2 (26100)" },
    { 0, 0, nullptr }
};
```

**Disable Method:**
```c
// In OmbraLoader (usermode), before driver load:
auto etw = etw::ResolveEtwParameters();
if (etw.success) {
    PVOID pEtwThreatIntProvRegHandle = (PVOID)(etw.ntoskrnl_base + etw.offset);

    // Use kernel R/W primitive (ZeroHVCI or BYOVD) to zero the handle
    kernel_write_u64(pEtwThreatIntProvRegHandle, 0);
}
```

**Phase 2: Wipe Buffers** (post-hypervisor activation):
```c
// In OmbraDriver (kernel), after hypervisor active:
// See PayLoad/core/dispatch.cpp:HandleWipeEtwBuffers()
typedef struct _ETW_WIPE_DATA {
    UINT64 ntoskrnl_base;
    UINT64 etwp_logger_list_offset;  // Offset to EtwpLoggerList
    UINT64 timestamp_start;           // Wipe events after this (100ns intervals)
    UINT64 timestamp_end;             // Wipe events before this
    UINT32 events_wiped;              // [OUT]
    UINT32 buffers_scanned;           // [OUT]
} ETW_WIPE_DATA;

// VMCALL from driver
ombra::wipe_etw_buffers(
    etw.ntoskrnl_base,
    ETWP_LOGGER_LIST_OFFSET,  // Build-specific, e.g., 0xC19C20
    timestamp_start,
    timestamp_end
);
```

---

## Forensic Artifact Cleanup

### MmUnloadedDrivers

**Purpose:** Kernel maintains a circular buffer of recently unloaded drivers (name, base address, size, timestamp). Anti-cheats scan this.

**Bypass** (`winternlex.cpp:309-394`):
```c
bool winternl::ClearMmUnloadedDrivers(HANDLE hDevice) {
    // 1. Find driver object via SystemExtendedHandleInformation
    PSYSTEM_HANDLE_INFORMATION_EX system_handle_info = /* query via ZwQuerySystemInformation */;

    for (i = 0; i < system_handle_info->HandleCount; ++i) {
        if (current_system_handle.HandleValue == hDevice) {
            object = current_system_handle.Object;
            break;
        }
    }

    // 2. Walk object structure to driver section
    DWORD64 device_object = *(DWORD64*)(object + 0x8);
    DWORD64 driver_object = *(DWORD64*)(device_object + 0x8);
    DWORD64 driver_section = *(DWORD64*)(driver_object + 0x28);

    // 3. Get driver name UNICODE_STRING
    UNICODE_STRING us_driver_base_dll_name = *(UNICODE_STRING*)(driver_section + 0x58);

    // 4. Zero the Length field (MiRememberUnloadedDriver checks Length > 0)
    us_driver_base_dll_name.Length = 0;
    *(UNICODE_STRING*)(driver_section + 0x58) = us_driver_base_dll_name;

    return true;
}
```

**Key Insight:** We don't modify `MmUnloadedDrivers` directly. Instead, we zero the `BaseDllName.Length` in the driver section **before** the driver unloads. This prevents `MiRememberUnloadedDriver` from recording the entry.

---

### PiDDBCacheTable

**Purpose:** Windows tracks loaded drivers in a **timestamped AVL tree** (`PiDDBCacheTable`). Each entry has:
- `DriverName` (UNICODE_STRING)
- `TimeDateStamp` (PE header timestamp)
- `List` (LIST_ENTRY for linked list)

**Bypass** (`winternlex.cpp:174-245`):
```c
bool winternl::ClearPIDDBCacheTable(string driverName, DWORD32 timestamp) {
    // 1. Locate PiDDBLock and PiDDBCacheTable (from ntoskrnl.pdb offsets)
    PVOID PiDDBLock = (PVOID)((DWORD64)ntoskrnlBase + offsets.PiDDBLock);
    PRTL_AVL_TABLE PiDDBCacheTable = (PRTL_AVL_TABLE)((DWORD64)ntoskrnlBase + offsets.PiDDBCacheTable);

    // 2. Acquire exclusive lock
    ExAcquireResourceExclusiveLite((PERESOURCE)PiDDBLock, true);

    // 3. Lookup entry via RtlLookupElementGenericTableAvl
    PIDDB_CACHE_ENTRY localentry = {0};
    localentry.TimeDateStamp = timestamp;
    localentry.DriverName.Buffer = (PWSTR)driverName.w_str();
    localentry.DriverName.Length = (USHORT)(wcslen(driverName.w_str()) * 2);

    PPIDDB_CACHE_ENTRY pFoundEntry = (PPIDDB_CACHE_ENTRY)RtlLookupElementGenericTableAvl(
        PiDDBCacheTable,
        (PVOID)&localentry
    );

    if (pFoundEntry == nullptr) {
        ExReleaseResourceLite((PERESOURCE)PiDDBLock);
        return false;
    }

    // 4. Unlink from LIST_ENTRY
    PLIST_ENTRY prev = pFoundEntry->List.Blink;
    PLIST_ENTRY next = pFoundEntry->List.Flink;
    prev->Flink = next;
    next->Blink = prev;

    // 5. Delete from AVL table
    RtlDeleteElementGenericTableAvl(PiDDBCacheTable, pFoundEntry);

    // 6. Decrement DeleteCount (anti-forensic)
    if (PiDDBCacheTable->DeleteCount > 0) {
        PiDDBCacheTable->DeleteCount--;
    }

    ExReleaseResourceLite((PERESOURCE)PiDDBLock);
    return true;
}
```

**Offsets** (from `Setup.hpp:79-80`):
```cpp
ULONG64 PiDDBLock = parser.GetSymbolRVA(L"PiDDBLock");
ULONG64 PiDDBCacheTable = parser.GetSymbolRVA(L"PiDDBCacheTable");
```

---

### CI.dll KernelHashBucketList

**Purpose:** Code Integrity driver (`CI.dll`) maintains a hash table of verified drivers. Removing entries allows re-mapping with different signatures.

**Bypass** (`winternlex.cpp:247-307`):
```c
bool winternl::ClearKernelHashBucketList(string driverName) {
    // 1. Find CI.dll base
    PVOID ciDll = Memory::GetKernelAddress((PCHAR)"CI.dll");

    // 2. Locate KernelHashBucketList and HashCacheLock (signature scan)
    const DWORD64 KernelHashBucketList = (DWORD64)ciDll + offsets.g_KernelHashBucketList;
    const DWORD64 HashCacheLock = (DWORD64)ciDll + offsets.g_HashCacheLock;

    // 3. Acquire lock
    ExAcquireResourceExclusiveLite((PERESOURCE)HashCacheLock, true);

    // 4. Walk linked list
    PHASH_BUCKET_ENTRY prev = (PHASH_BUCKET_ENTRY)KernelHashBucketList;
    PHASH_BUCKET_ENTRY entry = prev->Next;

    while (entry) {
        string wsName(&entry->DriverName);

        if (wcsstr(wsName.w_str(), driverName.w_str())) {
            // Unlink and free
            PHASH_BUCKET_ENTRY Next = entry->Next;
            prev->Next = Next;
            ExFreePool(entry);
            break;
        }

        prev = entry;
        entry = entry->Next;
    }

    ExReleaseResourceLite((PERESOURCE)HashCacheLock);
    return true;
}
```

---

### Prefetch Files

**Purpose:** Windows Prefetch tracks executable launches for performance optimization. Files stored at `C:\Windows\Prefetch\*.pf` with filenames like `OMBRALOADER.EXE-<HASH>.pf`.

**Bypass** (`prefetch_cleanup.h:19-89`):
```c
// 3-pass DoD-style secure delete
bool SecureDeleteFile(const std::wstring& path) {
    HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    LARGE_INTEGER fileSize;
    GetFileSizeEx(hFile, &fileSize);

    const size_t BUFFER_SIZE = 4096;
    std::vector<uint8_t> buffer(BUFFER_SIZE);

    // Pass 1: All zeros
    // Pass 2: All ones
    // Pass 3: Random data
    for (int pass = 0; pass < 3; ++pass) {
        SetFilePointer(hFile, 0, nullptr, FILE_BEGIN);

        uint8_t pattern = (pass == 0) ? 0x00 : (pass == 1) ? 0xFF : 0x00;

        if (pass == 2) {
            for (size_t i = 0; i < BUFFER_SIZE; ++i) {
                buffer[i] = static_cast<uint8_t>(__rdtsc() ^ i);
            }
        } else {
            memset(buffer.data(), pattern, BUFFER_SIZE);
        }

        LONGLONG remaining = fileSize.QuadPart;
        while (remaining > 0) {
            DWORD toWrite = (DWORD)min((LONGLONG)BUFFER_SIZE, remaining);
            WriteFile(hFile, buffer.data(), toWrite, &written, nullptr);
            remaining -= written;
        }

        FlushFileBuffers(hFile);
    }

    CloseHandle(hFile);
    return DeleteFileW(path.c_str()) != 0;
}
```

**Cleanup Targets** (`prefetch_cleanup.h:210-232`):
```c
const std::vector<std::wstring> known_artifacts = {
    L"OMBRALOADER",
    L"OMBRA",
    L"LD9BOXSUP",
    L"VBOXSUP",
    L"CSC",
    L"KS",
    L"THROTTLESTOP"  // ThrottleStop.sys BYOVD driver
};
```

---

## Disk Serial/Model Spoofing

### IOCTL Interception Points

**Hooked Drivers** (`diskspoof.h:16-28`):
```c
enum HookedDriver : UCHAR {
    DriverUSB,
    DriverDisk,
    DriverNIC,
    DriverTCP,
    DriverNSI,
    DriverWMI,
    DriverPartmgr,
    DriverVolume,
    DriverScsi,
    DriverNvme,
    DriverDump
};
```

**Targeted IOCTLs:**
```c
#define IOCTL_SCSI_PASS_THROUGH        0x4D004
#define IOCTL_SCSI_PASS_THROUGH_DIRECT 0x4D014
#define IOCTL_ATA_PASS_THROUGH         0x4D02C
#define IOCTL_SCSI_MINIPORT            0x4D008
#define IOCTL_STORAGE_QUERY_PROPERTY   0x2D1400
#define NVME_PASS_THROUGH_SRB_IO_CODE  0xE0002000
#define IOCTL_INTEL_NVME_PASS_THROUGH  0xF000A808
```

### Serial Modification (`diskspoof.cpp:44-87`)

**Operation:**
```c
// Tracking structure
typedef struct _DISK_SERIAL_DATA {
    char* spoofed;   // Spoofed serial
    char* orig;      // Original serial
    size_t sz;       // Length
} DISK_SERIAL_DATA;

vector<DISK_SERIAL_DATA>* vDiskSerials = nullptr;

bool FindFakeDiskSerial(char* pOriginal, bool bCappedString = true) {
    // Check if already spoofed
    for (auto& serial : *vDiskSerials) {
        if (!memcmp(serial.orig, pOriginal, serial.sz)) {
            Memory::WriteProtected(pOriginal, serial.spoofed, serial.sz);
            return true;
        }
    }

    // Create new spoof
    DISK_SERIAL_DATA data = {0};
    rnd.setSeed(spoofer::seed);  // Deterministic seed

    int serialLen = bCappedString ? DISK_SERIAL_MAX_LENGTH : strlen(pOriginal);
    data.orig = (char*)cpp::kMallocZero(serialLen + 1, PAGE_READWRITE);
    data.spoofed = (char*)cpp::kMallocZero(serialLen + 1, PAGE_READWRITE);
    data.sz = serialLen;

    RtlCopyMemory(data.orig, pOriginal, serialLen);
    RtlCopyMemory(data.spoofed, pOriginal, serialLen);

    // Shuffle characters (preserve first 2 chars for device type)
    rnd.random_shuffle_ignore_chars(data.spoofed + 2, serialLen - 2, (char*)" _-.", 4);

    vDiskSerials->Append(data);
    Memory::WriteProtected(pOriginal + 2, data.spoofed + 2, serialLen - 2);

    return true;
}
```

### SCSI Inquiry Hook (`diskspoof.cpp`)

**SCSI Command Structures:**
```c
typedef struct _SCSI_PASS_THROUGH_WITH_BUFFERS {
    SCSI_PASS_THROUGH Spt;
    ULONG Filler;
    UCHAR SenseBuf[32];
    UCHAR DataBuf[4096];
} SCSI_PASS_THROUGH_WITH_BUFFERS;

// Completion routine
NTSTATUS ScsiIoc(PDEVICE_OBJECT device, PIRP irp, PVOID context) {
    IOC_REQUEST request = *(PIOC_REQUEST)context;

    PSCSI_PASS_THROUGH_WITH_BUFFERS sptb = (PSCSI_PASS_THROUGH_WITH_BUFFERS)request.Buffer;

    // Check if SCSI INQUIRY command (0x12)
    if (sptb->Spt.Cdb[0] == 0x12) {
        // INQUIRY response has serial at offset +36 (VPD page 0x80)
        FindFakeDiskSerial((char*)sptb->DataBuf + 36);
    }

    // Chain to original completion routine
    if (request.OldRoutine) {
        return request.OldRoutine(device, irp, request.OldContext);
    }

    return STATUS_SUCCESS;
}
```

### NVMe Identify Hook

**NVMe Structures** (`diskspoof.h:137-226`):
```c
struct nvme_id_ctrl {
    unsigned short vid;
    unsigned short ssvid;
    char sn[20];          // Serial number (ASCII)
    char mn[40];          // Model number (ASCII)
    char fr[8];           // Firmware revision
    // ... (rest of structure)
};

struct NVME_PASS_THROUGH_IOCTL {
    SRB_IO_CONTROL SrbIoCtrl;
    DWORD VendorSpecific[6];
    DWORD NVMeCmd[16];
    DWORD CplEntry[4];
    DWORD Direction;
    DWORD QueueId;
    DWORD DataBufferLen;
    DWORD MetaDataLen;
    DWORD ReturnBufferLen;
    UCHAR DataBuffer[4096];
};
```

**Hook Implementation:**
```c
NTSTATUS NvmeIoc(PDEVICE_OBJECT device, PIRP irp, PVOID context) {
    IOC_REQUEST request = *(PIOC_REQUEST)context;

    NVME_PASS_THROUGH_IOCTL* nvme = (NVME_PASS_THROUGH_IOCTL*)request.Buffer;

    // Check if NVMe IDENTIFY command (opcode 0x06)
    if (nvme->NVMeCmd[0] & 0xFF == 0x06) {
        struct nvme_id_ctrl* ctrl = (struct nvme_id_ctrl*)nvme->DataBuffer;

        // Spoof serial and model
        FindFakeDiskSerial(ctrl->sn, false);
        SaveDiskModel(ctrl->mn);  // Track for consistency
    }

    if (request.OldRoutine) {
        return request.OldRoutine(device, irp, request.OldContext);
    }

    return STATUS_SUCCESS;
}
```

### ATA Identify Hook

**ATA Structures:**
```c
struct ata_identify_device {
    unsigned short words000_009[10];
    unsigned char serial_no[20];       // Byte-swapped ASCII
    unsigned short words020_022[3];
    unsigned char fw_rev[8];
    unsigned char model[40];
    // ... (rest)
};

typedef struct {
    ATA_PASS_THROUGH_EX apt;
    ULONG Filler;
    UCHAR ucDataBuf[32 * 512];
} ATA_PASS_THROUGH_EX_WITH_BUFFERS;
```

**Hook:**
```c
NTSTATUS AtaIoc(PDEVICE_OBJECT device, PIRP irp, PVOID context) {
    IOC_REQUEST request = *(PIOC_REQUEST)context;

    ATA_PASS_THROUGH_EX_WITH_BUFFERS* ata = (ATA_PASS_THROUGH_EX_WITH_BUFFERS*)request.Buffer;

    // Check if ATA IDENTIFY DEVICE (0xEC)
    if (ata->apt.CurrentTaskFile[6] == ATA_IDENTIFY_DEVICE) {
        struct ata_identify_device* id = (struct ata_identify_device*)ata->ucDataBuf;

        // Swap bytes (ATA uses little-endian word swapping)
        char swapped[21];
        for (int i = 0; i < 20; i += 2) {
            swapped[i] = id->serial_no[i + 1];
            swapped[i + 1] = id->serial_no[i];
        }
        swapped[20] = 0;

        FindFakeDiskSerial(swapped, false);

        // Swap back
        for (int i = 0; i < 20; i += 2) {
            id->serial_no[i] = swapped[i + 1];
            id->serial_no[i + 1] = swapped[i];
        }
    }

    return request.OldRoutine ? request.OldRoutine(device, irp, request.OldContext) : STATUS_SUCCESS;
}
```

### GUID/WWN/IEEE Spoofing

**GUID Spoofing** (`diskspoof.cpp:115-148`):
```c
vector<GUID*>* vGUIDs = nullptr;

bool FindFakeGUID(GUID* pOriginal) {
    // Check if trying to write to ntoskrnl (bad idea)
    if (winternl::IsNtoskrnlAddress((DWORD64)pOriginal)) {
        return false;
    }

    // Check if already spoofed
    for (auto& serial : *vGUIDs) {
        if (serial->Data1 == pOriginal->Data1) {
            Memory::WriteProtected(pOriginal, serial, sizeof(*pOriginal));
            return true;
        }
    }

    // Create new spoof
    rnd.setSeed(spoofer::seed);
    GUID* pBuf = (GUID*)cpp::kMalloc(sizeof(*pBuf), PAGE_READWRITE);
    RtlCopyMemory(pBuf, pOriginal, sizeof(*pBuf));

    // Randomize Data2, Data3, Data4 (preserve Data1 for consistency)
    DWORD64 data4 = rnd.Next(0ull, ~0ull);
    pBuf->Data2 = (USHORT)rnd.Next(0, MAXUSHORT);
    pBuf->Data3 = (USHORT)rnd.Next(0, MAXUSHORT);
    memcpy(pBuf->Data4, &data4, sizeof(pBuf->Data4));

    vGUIDs->Append(pBuf);
    Memory::WriteProtected(pOriginal, pBuf, sizeof(*pOriginal));

    return true;
}
```

**WWN (World Wide Name)** - Fibre Channel identifier:
```c
typedef struct _WWN {
    USHORT WorldWideName[4];
    USHORT ReservedForWorldWideName128[4];
} WWN, *PWWN;

bool FindFakeWWN(WWN* pOriginal) {
    // Similar to GUID, but shuffle bytes 1-14 (preserve first byte)
    WWN* pBuf = (WWN*)cpp::kMalloc(sizeof(*pBuf), PAGE_READWRITE);
    *pBuf = *pOriginal;
    rnd.random_shuffle((char*)&pBuf->WorldWideName[1], 14);
    // ...
}
```

**IEEE (NVMe IEEE OUI):**
```c
typedef struct _IEEE {
    UCHAR ieee[3];
} IEEE, *PIEEE;

bool FindFakeIEEE(IEEE* pOriginal) {
    // Shuffle last 2 bytes (preserve first byte for vendor ID)
    IEEE* pBuf = (IEEE*)cpp::kMalloc(sizeof(*pBuf), PAGE_READWRITE);
    *pBuf = *pOriginal;
    rnd.random_shuffle((char*)&pBuf->ieee[1], 2);
    // ...
}
```

---

## NIC MAC Address Spoofing

### IOCTL Interception Points (`nicspoof.h`)

**Targeted IOCTLs:**
```c
// NDIS OID queries
#define OID_802_3_PERMANENT_ADDRESS  0x01010101
#define OID_802_3_CURRENT_ADDRESS    0x01010102
#define OID_802_5_PERMANENT_ADDRESS  0x02010101
#define OID_802_5_CURRENT_ADDRESS    0x02010102

// NSI (Network Store Interface) queries
#define IOCTL_NSI_GETALLPARAM        0x0012001B
#define IOCTL_NSI_ARP_SOMETHING      0x12000F

// TCP/IP stack queries
#define IOCTL_TCP_QUERY_INFORMATION_EX  \
    CTL_CODE(FILE_DEVICE_NETWORK, 0, METHOD_NEITHER, FILE_ANY_ACCESS)
```

### MAC Modification (`nicspoof.cpp:27-59`)

**Operation:**
```c
typedef struct _MAC_MODIFICATION_DATA {
    NETADDR_STANDARD spoofed;  // char raw[6]
    NETADDR_STANDARD orig;
} MAC_MODIFICATION_DATA;

vector<PMAC_MODIFICATION_DATA>* vMACs = nullptr;

bool FindFakeNicMac(char* pOriginal, bool bAddIfNotFound = true) {
    // Check if already spoofed
    for (auto& MAC : *vMACs) {
        if (!memcmp(MAC->orig.raw, pOriginal, 6)) {
            Memory::WriteProtected(pOriginal, MAC->spoofed.raw, 6);
            return true;
        }
    }

    if (!bAddIfNotFound) return false;

    // Create new spoof
    PMAC_MODIFICATION_DATA pBuf = (PMAC_MODIFICATION_DATA)cpp::kMallocZero(sizeof(*pBuf), PAGE_READWRITE);
    RtlCopyMemory(pBuf->orig.raw, pOriginal, 6);
    RtlCopyMemory(pBuf->spoofed.raw, pOriginal, 6);

    // Randomize last 3 bytes (preserve OUI - first 3 bytes)
    rnd.bytes(pBuf->spoofed.raw + 3, 3);

    Memory::WriteProtected(pOriginal, pBuf->spoofed.raw, 6);
    vMACs->Append(pBuf);

    return true;
}
```

### NDIS Hook (`nicspoof.cpp:90-127`)

**Hook IRP_MJ_DEVICE_CONTROL:**
```c
NTSTATUS NICControl(PDEVICE_OBJECT device, PIRP irp) {
    PIO_STACK_LOCATION ioc = IoGetCurrentIrpStackLocation(irp);

    switch (ioc->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_NDIS_QUERY_GLOBAL_STATS: {
        // Check which OID is being queried
        switch (*(PDWORD)irp->AssociatedIrp.SystemBuffer) {
        case OID_802_3_PERMANENT_ADDRESS:
        case OID_802_3_CURRENT_ADDRESS:
        case OID_802_5_PERMANENT_ADDRESS:
        case OID_802_5_CURRENT_ADDRESS:
        case OID_WAN_PERMANENT_ADDRESS:
        case OID_WAN_CURRENT_ADDRESS:
        case OID_ARCNET_PERMANENT_ADDRESS:
        case OID_ARCNET_CURRENT_ADDRESS:
            // Install completion routine
            ChangeIoc(ioc, irp, NICIoc);
            break;
        }
        break;
    }
    }

    return driver->Original(device, irp);
}

// Completion routine
NTSTATUS NICIoc(PDEVICE_OBJECT device, PIRP irp, PVOID context) {
    IOC_REQUEST request = *(PIOC_REQUEST)context;

    // MAC address is in MDL (Memory Descriptor List)
    if (irp->MdlAddress) {
        FindFakeNicMac((char*)MmGetSystemAddressForMdl(irp->MdlAddress));
    }

    if (request.OldRoutine) {
        return request.OldRoutine(device, irp, request.OldContext);
    }

    return STATUS_SUCCESS;
}
```

### NSI (Network Store Interface) Hook

**NSI Structures** (`nicspoof.h:32-126`):
```c
typedef struct _NSI_PARAMS {
    __int64 field_0;
    __int64 field_8;
    __int64 field_10;
    int Type;               // NSI_GET_INTERFACE_INFO (1) or NSI_GET_IP_NET_TABLE (11)
    int field_1C;
    int field_20;
    int field_24;
    char field_42;
    __int64 AddrTable;      // IP address table
    int AddrEntrySize;
    int field_34;
    __int64 NeighborTable;  // ARP/NDP table (contains MAC addresses)
    int NeighborTableEntrySize;
    int field_44;
    __int64 StateTable;
    int StateTableEntrySize;
    int field_54;
    __int64 OwnerTable;
    int OwnerTableEntrySize;
    int field_64;
    int Count;              // Number of entries
    int field_6C;
} NSI_PARAMS;
```

**Hook Implementation** (`nicspoof.cpp:129-178`):
```c
NTSTATUS NsiControl(PDEVICE_OBJECT device, PIRP irp) {
    PIO_STACK_LOCATION ioc = IoGetCurrentIrpStackLocation(irp);

    switch (ioc->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_NSI_GETALLPARAM: {
        DWORD length = ioc->Parameters.DeviceIoControl.OutputBufferLength;
        NTSTATUS ret = driver->Original(device, irp);

        PNSI_PARAMS params = (PNSI_PARAMS)irp->UserBuffer;

        // NSI_GET_IP_NET_TABLE (11) returns ARP/NDP entries with MAC addresses
        if (MmIsAddressValid(params) && params->Type == NSI_GET_IP_NET_TABLE) {
            // Zero the buffer to hide ARP cache (anti-forensic)
            memset(irp->UserBuffer, 0, length);
            return STATUS_ACCESS_DENIED;
        }

        return ret;
    }
    }

    return driver->Original(device, irp);
}
```

### TCP/IP Stack Hook

**TCP Structures** (`nicspoof.h:173-218`):
```c
typedef struct IFEntry {
    ULONG if_index;
    ULONG if_type;
    ULONG if_mtu;
    ULONG if_speed;
    ULONG if_physaddrlen;
    UCHAR if_physaddr[MAX_PHYSADDR_SIZE];  // MAC address
    ULONG if_adminstatus;
    // ... (rest of MIB-II ifEntry)
} IFEntry;

typedef struct _TDIObjectID {
    TDIEntityID toi_entity;  // CL_TL_ENTITY (0x401) or CO_TL_ENTITY (0x400)
    ULONG toi_class;         // INFO_CLASS_PROTOCOL (0x200)
    ULONG toi_type;          // INFO_TYPE_PROVIDER (0x100)
    ULONG toi_id;            // IF_MIB_STATS_ID (1)
} TDIObjectID;
```

**Hook Implementation** (`nicspoof.cpp:180-207`):
```c
NTSTATUS TcpControl(PDEVICE_OBJECT device, PIRP irp) {
    PIO_STACK_LOCATION ioc = IoGetCurrentIrpStackLocation(irp);

    switch (ioc->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_TCP_QUERY_INFORMATION_EX: {
        NTSTATUS ntStatus = pTcpCtrlOrig(device, irp);

        if (NT_SUCCESS(ntStatus)) {
            IFEntry* IfEntry = (IFEntry*)irp->UserBuffer;

            if (MmIsAddressValid(IfEntry)) {
                // Spoof MAC address in interface entry
                FindFakeNicMac((char*)IfEntry->if_physaddr);
            }
        }

        return ntStatus;
    }
    }

    return pTcpCtrlOrig(device, irp);
}
```

---

## C Conversion Notes

### WMI COM Interface in C

**Problem:** WMI uses COM (IWbemLocator, IWbemServices) which is C++-oriented.

**Solution:** We don't interact with WMI from usermode. We hook **WmipQueryAllData** in the kernel (C-compatible function pointer).

**Kernel Hook (C-compatible):**
```c
typedef NTSTATUS (*fnWmipQueryAllData)(
    IN PWMIGUIDOBJECT GuidObject,
    IN PIRP Irp,
    IN KPROCESSOR_MODE AccessMode,
    IN PWNODE_ALL_DATA Wnode,
    IN ULONG OutBufferLen,
    OUT PULONG RetSize
);

fnWmipQueryAllData pWmipQueryAllData = NULL;

// Install EPT hook
PVOID pWmipQueryAllDataAddr = (PVOID)((DWORD64)ntoskrnlBase + offsets.WmipQueryAllData);
EPT_HookExec(pWmipQueryAllDataAddr, WmipQueryAllDataHook, &pWmipQueryAllData);
```

---

### Registry Manipulation

**Problem:** C++ uses `ZwSetValueKey` wrappers with string objects.

**Solution:** Use raw `Zw*` APIs with `UNICODE_STRING` initialization.

**Example (Delete service registry key):**
```c
// C++ version (from OmbraCoreLib)
string serviceName("MyDriver");
registry::DeleteKey(serviceName);

// C version
void DeleteServiceKey(const wchar_t* serviceName) {
    UNICODE_STRING keyPath;
    wchar_t buffer[256];
    swprintf(buffer, 256, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\%s", serviceName);
    RtlInitUnicodeString(&keyPath, buffer);

    OBJECT_ATTRIBUTES objAttr;
    InitializeObjectAttributes(&objAttr, &keyPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

    HANDLE hKey;
    if (NT_SUCCESS(ZwOpenKey(&hKey, DELETE, &objAttr))) {
        ZwDeleteKey(hKey);
        ZwClose(hKey);
    }
}
```

---

### Kernel Structure Walking

**Problem:** Structures like `LIST_ENTRY`, `RTL_AVL_TABLE` require careful offset calculations.

**Offsets via PDB:**
```c
// Usermode PDB parsing (OmbraLoader):
PdbParser parser(L".\\ntoskrnl.pdb");
ULONG64 PiDDBLock = parser.GetSymbolRVA(L"PiDDBLock");
ULONG64 PiDDBCacheTable = parser.GetSymbolRVA(L"PiDDBCacheTable");

// Write to shared struct
offsets.PiDDBLock = PiDDBLock;
offsets.PiDDBCacheTable = PiDDBCacheTable;

// Kernel uses offsets (driver reads from shared memory):
PVOID PiDDBLock = (PVOID)((DWORD64)ntoskrnlBase + offsets.PiDDBLock);
PRTL_AVL_TABLE PiDDBCacheTable = (PRTL_AVL_TABLE)((DWORD64)ntoskrnlBase + offsets.PiDDBCacheTable);
```

**AVL Table Operations (C-compatible):**
```c
// ntoskrnl exports
PVOID RtlLookupElementGenericTableAvl(PRTL_AVL_TABLE Table, PVOID Buffer);
BOOLEAN RtlDeleteElementGenericTableAvl(PRTL_AVL_TABLE Table, PVOID Buffer);

// Usage
PIDDB_CACHE_ENTRY localentry = {0};
localentry.TimeDateStamp = timestamp;
RtlInitUnicodeString(&localentry.DriverName, driverName);

PPIDDB_CACHE_ENTRY pFoundEntry = (PPIDDB_CACHE_ENTRY)RtlLookupElementGenericTableAvl(
    PiDDBCacheTable,
    (PVOID)&localentry
);

if (pFoundEntry) {
    RtlDeleteElementGenericTableAvl(PiDDBCacheTable, pFoundEntry);
}
```

---

### Vector Replacement

**Problem:** C++ `vector<T>` not available in pure C.

**Solution 1:** Fixed-size arrays (if max size known):
```c
// C++ version
vector<DISK_SERIAL_DATA>* vDiskSerials = nullptr;

// C version
#define MAX_DISK_SERIALS 64
DISK_SERIAL_DATA vDiskSerials[MAX_DISK_SERIALS];
ULONG diskSerialCount = 0;

// Append
if (diskSerialCount < MAX_DISK_SERIALS) {
    vDiskSerials[diskSerialCount++] = newSerial;
}
```

**Solution 2:** Linked list (if dynamic size required):
```c
typedef struct _DISK_SERIAL_NODE {
    DISK_SERIAL_DATA data;
    struct _DISK_SERIAL_NODE* next;
} DISK_SERIAL_NODE, *PDISK_SERIAL_NODE;

PDISK_SERIAL_NODE vDiskSerials = NULL;

// Append
PDISK_SERIAL_NODE newNode = (PDISK_SERIAL_NODE)ExAllocatePoolWithTag(NonPagedPool, sizeof(DISK_SERIAL_NODE), 'DSRL');
newNode->data = newSerial;
newNode->next = vDiskSerials;
vDiskSerials = newNode;
```

**Solution 3:** Use custom allocator (like existing `cpp::kMalloc`):
```c
// Keep C++ vector implementation but port allocator to C
// See OmbraCoreLib/OmbraCoreLib-v/include/cpp.h for allocator
```

---

### Random Number Generation

**Problem:** `random::Random` class uses C++ constructor/member functions.

**Solution:** Port to C struct + function pointers OR use simpler PRNG.

**C Port:**
```c
typedef struct _RANDOM_STATE {
    uint64_t seed;
    uint32_t security_level;  // 0 = PREDICTABLE, 1 = SECURE
} RANDOM_STATE, *PRANDOM_STATE;

void random_init(PRANDOM_STATE state, uint64_t seed) {
    state->seed = seed;
    state->security_level = 0;  // PREDICTABLE
}

uint64_t random_next(PRANDOM_STATE state, uint64_t min, uint64_t max) {
    // Simple LCG (Linear Congruential Generator)
    state->seed = (state->seed * 6364136223846793005ULL + 1442695040888963407ULL);
    return min + (state->seed % (max - min + 1));
}

void random_shuffle(PRANDOM_STATE state, char* buffer, size_t length) {
    for (size_t i = length - 1; i > 0; --i) {
        size_t j = random_next(state, 0, i);
        char temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;
    }
}
```

---

### EPT Hook Installation

**Problem:** `EPT::HookExec()` is C++ with VMCALL wrapper.

**Solution:** Use raw VMCALL assembly.

**C++ Version:**
```cpp
bool EPT::HookExec(PVOID pTarget, PVOID pHook, HOOK_SECONDARY_INFO& info) {
    return ombra::hook_exec((uint64_t)pTarget, (uint64_t)pHook, (uint64_t)info.pOrigFn);
}
```

**C Version:**
```c
// libombra/com.asm:
; uint64_t __ombra_vmcall(uint64_t rcx, uint64_t rdx, uint64_t r8, uint64_t r9)
__ombra_vmcall PROC
    mov r9, r8
    mov r8, rdx
    mov rdx, rcx
    mov rcx, 0x13371337  ; VMCALL key
    cpuid                ; Hypervisor intercepts this
    mov rax, rax         ; Return value
    ret
__ombra_vmcall ENDP

// C wrapper:
uint64_t hook_exec_vmcall(uint64_t target, uint64_t hook, uint64_t* pOrig) {
    return __ombra_vmcall(VMCALL_HOOK_EXEC, target, hook, (uint64_t)pOrig);
}
```

---

## Detection Surface Analysis

### What Anti-Cheats Can Detect

| Layer | Detection Method | Visibility |
|-------|------------------|------------|
| **Hardware Fingerprints** | IOCTL queries to disk/NIC drivers | **HIGH** - Standard Windows APIs |
| **WMI Queries** | Win32_DiskDrive, Win32_NetworkAdapter | **HIGH** - COM interfaces |
| **Kernel Artifacts** | MmUnloadedDrivers, PiDDBCacheTable | **MEDIUM** - Requires kernel R/W |
| **Registry** | Service keys, driver file paths | **HIGH** - Standard APIs |
| **Prefetch** | C:\Windows\Prefetch\*.pf | **HIGH** - File system access |
| **ETW Buffers** | EtwpLoggerList, circular buffers | **MEDIUM** - Requires kernel R/W |
| **Driver Memory** | PsLoadedModuleList, ExAllocatePool | **LOW** - Requires kernel access |
| **EPT Shadow Pages** | Physical memory scanning | **VERY LOW** - Requires DMA or hypervisor |

### Post-Cleanup Detection Surface

**After Full Cleanup Chain:**
```
┌─────────────────────────────────────────────────────────────────────────┐
│ ARTIFACT                     | STATUS        | NOTES                    │
├─────────────────────────────────────────────────────────────────────────┤
│ Disk Serials                 | ✅ CLEAN      | Deterministic spoof      │
│ NIC MACs                     | ✅ CLEAN      | Preserves OUI            │
│ Monitor Serials (WMI)        | ✅ CLEAN      | Zeroed via EPT hook      │
│ PsLoadedModuleList           | ✅ CLEAN      | EPT shadow paging        │
│ MmUnloadedDrivers            | ✅ CLEAN      | Length = 0               │
│ PiDDBCacheTable              | ✅ CLEAN      | AVL entry deleted        │
│ CI.dll HashBucketList        | ✅ CLEAN      | Unlinked + freed         │
│ WdFilter.sys gTable          | ✅ CLEAN      | Unlinked                 │
│ Registry Service Keys        | ✅ CLEAN      | Deleted post-load        │
│ Driver File on Disk          | ✅ CLEAN      | Deleted post-load        │
│ Prefetch Files               | ✅ CLEAN      | 3-pass secure delete     │
│ ETW Circular Buffers         | ✅ CLEAN      | Timestamp-based wipe     │
│ Window Handles               | ✅ CLEAN      | TEB cache poisoned       │
│ NMI Callbacks (EAC)          | ✅ BYPASSED   | Queued per-core          │
│ CR3 Tracking (EAC)           | ✅ BYPASSED   | Updated on VMExit        │
│ Microsoft Telemetry Batch    | ⚠️ MAYBE      | May transmit before wipe │
└─────────────────────────────────────────────────────────────────────────┘
```

**Worst-Case Scenario:**
Microsoft's telemetry batch (uploads every ~5 minutes) captures ETW events showing "ThrottleStop.sys loaded then unloaded" before we wipe buffers. This is **innocuous** - thousands of users load ThrottleStop daily for legitimate CPU undervolting.

---

## Testing Checklist

### Pre-Deployment Testing

**1. Disk Serial Spoofing:**
```
[ ] Load driver with spoofer::seed = 0xDEADBEEF
[ ] Query disk serials via IOCTL_STORAGE_QUERY_PROPERTY
[ ] Verify serials are modified but deterministic (same seed = same spoof)
[ ] Query via WMI Win32_DiskDrive
[ ] Verify WMI returns spoofed serials
[ ] Unload driver
[ ] Verify original serials restored (if FindFakeDiskSerialReset called)
```

**2. NIC MAC Spoofing:**
```
[ ] Load driver with spoofer::seed = 0xDEADBEEF
[ ] Query MAC via ipconfig /all
[ ] Verify MAC last 3 bytes changed
[ ] Query via OID_802_3_PERMANENT_ADDRESS IOCTL
[ ] Verify IOCTL returns spoofed MAC
[ ] Query via NSI IOCTL_NSI_GETALLPARAM
[ ] Verify ARP cache hidden (STATUS_ACCESS_DENIED)
[ ] Unload driver
[ ] Verify original MAC restored
```

**3. Monitor Serial Spoofing:**
```
[ ] Load driver
[ ] Query via WMI WmiMonitorID
[ ] Verify SerialNumberID and ProductCodeID are zeroed
[ ] Query via DeviceIoControl to monitor.sys
[ ] Verify results match WMI (zeroed)
```

**4. Kernel Artifact Cleanup:**
```
[ ] Load driver
[ ] Call winternl::ClearPIDDBCacheTable(L"OmbraDriver.sys", <timestamp>)
[ ] Scan PiDDBCacheTable manually
[ ] Verify entry not present
[ ] Call winternl::ClearMmUnloadedDrivers(hDriver)
[ ] Unload driver
[ ] Scan MmUnloadedDrivers
[ ] Verify driver not in list
```

**5. WdFilter Bypass:**
```
[ ] Load driver
[ ] Call defender::CleanFilterList(L"OmbraDriver.sys")
[ ] Query WdFilter gTable via signature scan
[ ] Verify driver not in list
```

**6. Prefetch Cleanup:**
```
[ ] Run OmbraLoader.exe
[ ] Check C:\Windows\Prefetch\OMBRALOADER.EXE-*.pf
[ ] Verify file exists before cleanup
[ ] Call prefetch::CleanupOwnPrefetch()
[ ] Verify file deleted
[ ] Use forensic tool (e.g., Recuva) to check for recovery
[ ] Verify file unrecoverable (3-pass wipe)
```

**7. ETW-TI Blinding:**
```
[ ] Resolve EtwThreatIntProvRegHandle offset
[ ] Read value before disable
[ ] Call etw::DisableEtwTi()
[ ] Read value after disable (should be 0)
[ ] Load driver (should generate no ETW events)
[ ] Enable ETW-TI again
[ ] Verify restoration
```

**8. Window Hiding:**
```
[ ] Create window with title "Test Window"
[ ] Call FindWindow(NULL, L"Test Window") - should find it
[ ] Get PETHREAD, read TEB
[ ] Call window::Hide(hwnd, pTeb)
[ ] Call FindWindow(NULL, L"Test Window") - should return NULL
```

---

### Anti-Cheat Specific Testing

**EasyAntiCheat:**
```
[ ] Load driver in EAC-protected game
[ ] Trigger NMI callback (via hardware interrupt)
[ ] Verify eac::GetAndDecreaseNmiCount() returns queued count
[ ] Trigger CR3 switch (via process switch)
[ ] Verify eac::UpdateCr3() updates tracked CR3s
[ ] Monitor for EAC bans (test account only!)
```

**Windows Defender:**
```
[ ] Load driver
[ ] Check WdFilter gTable before cleanup
[ ] Call defender::CleanFilterList()
[ ] Check WdFilter gTable after cleanup
[ ] Verify driver unlinked
[ ] Monitor for Defender alerts (should be none)
```

**General Anti-Cheat:**
```
[ ] Load driver with all spoofing enabled
[ ] Run anti-cheat detection tools:
    - HWiNFO64 (hardware info)
    - GPU-Z (GPU serial)
    - CrystalDiskInfo (disk serial)
    - Wireshark (NIC MAC)
[ ] Verify all serials/MACs are spoofed
[ ] Run kernel debugger (WinDbg)
[ ] Scan MmUnloadedDrivers, PiDDBCacheTable
[ ] Verify driver not present
```

---

## Conversion Priority

**Phase 1: Core Infrastructure (CRITICAL)**
1. `winternlex.cpp` - Kernel utility functions (GetNtoskrnlBase, pattern scan, etc.)
2. `ioctlhook.cpp` - IOCTL hooking infrastructure (ChangeIoc, IOC_REQUEST)
3. `Setup.hpp` - PDB offset resolution

**Phase 2: Artifact Cleanup (HIGH)**
1. `winternlex.cpp` - ClearPIDDBCacheTable, ClearMmUnloadedDrivers, ClearKernelHashBucketList
2. `prefetch_cleanup.h` - Prefetch file secure delete
3. `etw_resolver.h` - ETW-TI offset resolution

**Phase 3: Hardware Spoofing (HIGH)**
1. `diskspoof.cpp` - Disk serial IOCTL hooks
2. `nicspoof.cpp` - NIC MAC IOCTL hooks
3. `wmispoof.cpp` - WMI query interception

**Phase 4: Anti-Cheat Specific (MEDIUM)**
1. `eac.cpp` - NMI blocking, CR3 tracking
2. `defender.cpp` - WdFilter bypass
3. `windowspoof.cpp` - Window hiding

**Phase 5: Hypervisor Integration (LOW - C++ to C not required)**
- PayLoad handlers already use C-compatible VMCALL interface
- EPT hooks use function pointers (C-compatible)
- Only libombra wrapper needs C port

---

## Summary

This evasion layer is **multi-stage** and **comprehensive**:

1. **Pre-Load:** Clean prefetch, blind ETW-TI
2. **Load:** Hook IOCTLs, spoof hardware, hide windows
3. **Post-Load:** Clean MmUnloadedDrivers, PiDDBCacheTable, CI.dll hashes
4. **Runtime:** EPT shadow paging, NMI queuing, CR3 tracking

**Key C Conversion Challenges:**
- Replace `vector<T>` with fixed arrays or linked lists
- Port `string` class to `UNICODE_STRING` + manual buffer management
- Replace `Memory::WriteProtected()` with manual CR0 flipping + write
- Port `random::Random` to simple C PRNG
- Replace `EPT::HookExec()` with raw VMCALL assembly

**Detection Surface After Cleanup:**
- Hardware: ✅ Fully spoofed (deterministic)
- Kernel Artifacts: ✅ Completely removed
- Usermode Artifacts: ✅ Secure-deleted
- Runtime Telemetry: ✅ Blinded + wiped
- Worst Case: ⚠️ Microsoft telemetry batch may capture benign ThrottleStop load event

All techniques are **production-grade** and designed for **EasyAntiCheat, BattlEye, Vanguard** evasion. The layered approach ensures that even if one layer fails, the others provide redundancy.
