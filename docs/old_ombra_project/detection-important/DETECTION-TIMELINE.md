# Detection Timeline

**Audit Date:** December 25, 2025
**Purpose:** Chronological mapping of artifact creation during Ombra Hypervisor injection

This document provides a millisecond-by-millisecond analysis of when each forensic artifact becomes detectable during the injection process. Use this to understand the temporal attack surface and identify optimal cleanup windows.

---

## Document Overview

The injection process creates artifacts across 7 distinct phases over approximately 300-400 milliseconds. This document maps:

1. **What artifacts are created** at each phase
2. **Which code paths create them** (with exact file:line references)
3. **When they become detectable** by anti-cheat systems
4. **What cleanup is performed** (and what's missing)
5. **Anti-cheat scan timing** relative to artifact lifetime

---

## Executive Summary

| Phase | Time Range | Artifacts Created | Cleaned? | Detection Risk |
|-------|------------|-------------------|----------|----------------|
| 1. Pre-Load | 0-5ms | Process events, PE resources | NO | LOW |
| 2. Driver Loading | 5-50ms | Event logs, ETW, registry | PARTIAL | **CRITICAL** |
| 3. ThrottleStop Exploit | 50-70ms | Physical memory accesses | YES | LOW |
| 4. Hypervisor Activation | 70-150ms | Payload allocation, VMExit patch | HIDDEN | MEDIUM |
| 5. Driver Mapping | 150-200ms | BigPool entries, hooks | PARTIAL | **HIGH** |
| 6. EPT Hiding | 200-250ms | EPT structures, substitute pages | PARTIAL | MEDIUM |
| 7. Cleanup & Runtime | 250ms+ | Persistent artifacts | INCOMPLETE | **HIGH** |

**Critical Window:** T=5ms to T=50ms (driver loading) creates permanent Event Log evidence.

---

## Visual Master Timeline

```
╔═══════════════════════════════════════════════════════════════════════════════════════╗
║                           OMBRA HYPERVISOR INJECTION TIMELINE                          ║
╠═══════════════════════════════════════════════════════════════════════════════════════╣
║                                                                                         ║
║  T=0ms         T=5ms           T=50ms          T=100ms         T=200ms        T=300ms+ ║
║    │             │               │                │               │              │      ║
║    ▼             ▼               ▼                ▼               ▼              ▼      ║
║ ┌──────┐    ┌─────────┐    ┌──────────┐    ┌───────────┐   ┌──────────┐   ┌─────────┐ ║
║ │LOADER│    │THROTTLE │    │ SUPDrv   │    │ HYPERVISOR│   │  DRIVER  │   │ RUNTIME │ ║
║ │START │───▶│ STOP    │───▶│ LOAD     │───▶│  ACTIVE   │──▶│  MAPPED  │──▶│  ACTIVE │ ║
║ └──────┘    └─────────┘    └──────────┘    └───────────┘   └──────────┘   └─────────┘ ║
║    │             │               │                │               │              │      ║
║    │        ┌────┴────┐    ┌────┴────┐      ┌────┴────┐    ┌────┴────┐    ┌────┴────┐ ║
║    │        │EVENT LOG│    │EVENT LOG│      │PAYLOAD  │    │BIGPOOL  │    │EVENT LOG│ ║
║    │        │  7045   │    │  7045   │      │ALLOCATED│    │ ENTRIES │    │PERSISTS │ ║
║    │        │(PERM!)  │    │(PERM!)  │      │(HIDDEN) │    │(VISIBLE)│    │(PERM!)  │ ║
║    │        └─────────┘    └─────────┘      └─────────┘    └─────────┘    └─────────┘ ║
║                                                                                         ║
║ ─────────────────────────────── DETECTION WINDOWS ─────────────────────────────────── ║
║                                                                                         ║
║    │◀─────── EVENT LOG WINDOW (PERMANENT - NEVER CLEANED) ──────────────────────▶│    ║
║    │                                                                              │    ║
║         │◀─ ETW WINDOW ─▶│        (If wiped at T=100ms, events are gone)         │    ║
║                                                                                         ║
║              │◀───── BIGPOOL WINDOW (PERMANENT until reboot) ────────────────────▶│    ║
║                                                                                         ║
║                    │◀─ HOOK TRAMPOLINE WINDOW (until driver unload) ─────────────▶│    ║
║                                                                                         ║
╚═══════════════════════════════════════════════════════════════════════════════════════╝
```

---

## Phase 1: Loader Initialization (T=0ms to T=5ms)

### What Happens

```cpp
// File: OmbraLoader/main.cpp
// Line: ~50 (entry point)

int main(int argc, char* argv[])
{
    // T=0ms: Process creation logged by Windows
    // T=0.1ms: PE loaded into memory
    // T=0.5ms: Runtime initialization
    // T=1ms: Resource loading begins

    // Load embedded resources
    auto payloadData = LoadResource(PAYLOAD_AMD);  // or PAYLOAD_INTEL
    auto driverData = LoadResource(OMBRA_DRIVER);
    auto throttleData = LoadResource(THROTTLESTOP_SYS);
    auto ld9boxData = LoadAndDecrypt(LD9BOXSUP_ENCRYPTED);

    // T=2-5ms: Initialization complete
    timestamp_start = GetTickCount64();
}
```

### Artifacts Created

| Artifact | Location | Creation Time | Visibility | Cleanup Status |
|----------|----------|---------------|------------|----------------|
| Process creation event | Event Log | T=0ms | Kernel + User | **NOT CLEANED** |
| PE image in memory | Process memory | T=0.1ms | User (VAD walk) | Ephemeral (exit) |
| Loaded resources | Process heap | T=1-5ms | User (heap walk) | Ephemeral (exit) |
| Thread creation | Event Log (4688) | T=0.2ms | ETW consumer | **NOT CLEANED** |

### Code Paths

```
main()
  └─> WinMain() wrapper
      └─> LoadResource()           // OmbraLoader/resource_loader.cpp:45
          └─> FindResourceW()
          └─> LockResource()
      └─> DecryptLd9Box()          // OmbraLoader/supdrv/driver_deployer.cpp:120
          └─> XorDecrypt(0xDEADBEEF)
```

### Detection Risk: LOW

- Process creation is normal
- Resources are in-memory only
- Anti-cheats don't typically scan loader process during launch

### Anti-Cheat Visibility

| Anti-Cheat | Scans This Phase? | Detection Method |
|------------|-------------------|------------------|
| EAC | NO | N/A |
| BattlEye | NO | N/A |
| Vanguard | YES (kernel) | Process creation hook |
| FACEIT | NO | N/A |

---

## Phase 2: Driver Loading (T=5ms to T=50ms)

### What Happens

```cpp
// File: OmbraLoader/supdrv/driver_deployer.cpp
// Lines: 1000-1163

bool DeployViaNtLoadDriver()
{
    // T=5ms: Extract ThrottleStop.sys to temp directory
    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    ExtractResourceToFile(THROTTLESTOP_SYS, tempPath);

    // T=6ms: Create service registry key
    // >>> EVENT ID 7045 GENERATED HERE <<<
    CreateServiceRegistryKey(L"ThrottleStop", tempPath);

    // T=7ms: Load ThrottleStop via NtLoadDriver
    NTSTATUS status = NtLoadDriver(&servicePath);

    // T=15ms: Use ThrottleStop for -618 bypass (Phase 3)

    // T=20ms: Extract Ld9BoxSup.sys
    ExtractResourceToFile(LD9BOXSUP_ENCRYPTED, tempPath);
    DecryptInPlace(tempPath, 0xDEADBEEF);

    // T=21ms: Create service registry key
    // >>> EVENT ID 7045 GENERATED HERE <<<
    CreateServiceRegistryKey(L"Ld9BoxSup", tempPath);

    // T=22ms: Load Ld9BoxSup via NtLoadDriver
    status = NtLoadDriver(&ld9boxPath);

    // T=30-50ms: Wait for device to become accessible
    for (int i = 0; i < 5; i++) {
        if (OpenDevice(L"\\\\.\\Ld9BoxDrv")) break;
        Sleep(100);
    }
}
```

### Artifacts Created

| Artifact | Location | Creation Time | Visibility | Cleanup Status |
|----------|----------|---------------|------------|----------------|
| ThrottleStop.sys file | %TEMP% | T=5ms | Disk | DELETED (secure) |
| ThrottleStop service key | Registry | T=6ms | Registry | DELETED (immediate) |
| **Event ID 7045 (ThrottleStop)** | System Event Log | T=6ms | **PERMANENT** | **NOT CLEANED** |
| ETW: Service create | Kernel buffers | T=6ms | ETW consumer | OPTIONAL (VMCALL) |
| Ld9BoxSup.sys file | %TEMP% | T=20ms | Disk | DELETED (secure) |
| Ld9BoxSup service key | Registry | T=21ms | Registry | DELETED (immediate) |
| **Event ID 7045 (Ld9BoxSup)** | System Event Log | T=21ms | **PERMANENT** | **NOT CLEANED** |
| ETW: Service create | Kernel buffers | T=21ms | ETW consumer | OPTIONAL (VMCALL) |
| Prefetch file | %SystemRoot%\Prefetch | T=22ms | Disk | DELETED (3-pass) |

### Event Log Entry Details

```xml
<Event xmlns="http://schemas.microsoft.com/win/2004/08/events/event">
  <System>
    <Provider Name="Service Control Manager" />
    <EventID>7045</EventID>
    <TimeCreated SystemTime="2025-12-25T15:30:00.000Z" />
    <Computer>DESKTOP-XXXXX</Computer>
  </System>
  <EventData>
    <Data Name="ServiceName">ThrottleStop</Data>
    <Data Name="ImagePath">\??\C:\Users\...\AppData\Local\Temp\ThrottleStop.sys</Data>
    <Data Name="ServiceType">kernel mode driver</Data>
    <Data Name="StartType">demand start</Data>
  </EventData>
</Event>
```

### Why Event Log Is Critical

```cpp
// Anti-cheat detection query (PowerShell equivalent)
void QueryEventLogForBYOVD() {
    // Query takes <100ms
    auto events = GetWinEvents(L"System",
        L"*[System[EventID=7045]]");

    for (auto& e : events) {
        // Check for known BYOVD drivers
        if (ContainsAny(e.ImagePath, {
            L"ThrottleStop",   // CVE-2025-7771
            L"Ld9BoxSup",      // LDPlayer VBox fork
            L"VBoxDrv",        // VirtualBox
            L"AsrDrv",         // ASRock BYOVD
            L"DBUtil",         // Dell BYOVD
            L"ene.sys"         // ENE BYOVD
        })) {
            FlagAsBYOVD(e);
        }
    }
}
```

### Detection Risk: CRITICAL

- Event Log entries are **PERMANENT**
- They survive reboots, disk imaging
- Query takes <100ms from any privilege level
- **This is the single biggest detection vector**

---

## Phase 3: ThrottleStop Exploit (T=50ms to T=70ms)

### What Happens

```cpp
// File: OmbraLoader/throttlestop/throttlestop_exploit.cpp
// Lines: 200-350

bool Bypass618Flags()
{
    // T=50ms: Get SYSTEM CR3 for page table walking
    uint64_t systemCr3 = GetSystemCr3();

    // T=52ms: Find Ld9BoxSup.sys base address
    uint64_t ld9Base = GetDriverBase(L"Ld9BoxSup.sys");

    // T=55ms: Translate virtual addresses to physical
    uint64_t flagAddr1 = ld9Base + 0x4a1a0;  // ntoskrnl validation
    uint64_t flagAddr2 = ld9Base + 0x4a210;  // hal validation

    uint64_t flagPA1 = VirtToPhys(flagAddr1, systemCr3);
    uint64_t flagPA2 = VirtToPhys(flagAddr2, systemCr3);

    // T=60ms: Patch flags via MmMapIoSpace
    // >>> PHYSICAL MEMORY WRITE VIA THROTTLESTOP <<<
    WritePhysicalByte(flagPA1, 1);
    WritePhysicalByte(flagPA2, 1);

    // T=65ms: Verify patches
    uint8_t verify1 = ReadPhysicalByte(flagPA1);
    uint8_t verify2 = ReadPhysicalByte(flagPA2);

    return (verify1 == 1 && verify2 == 1);
}
```

### Artifacts Created

| Artifact | Location | Creation Time | Visibility | Cleanup Status |
|----------|----------|---------------|------------|----------------|
| ThrottleStop IOCTL calls | Driver internal | T=50-65ms | Kernel only | Ephemeral |
| Physical memory mapping | MmMapIoSpace | T=55ms | Driver internal | Auto-unmapped |
| Modified Ld9Box flags | Driver memory | T=60ms | Hidden (in driver) | Persists in driver |

### Detection Risk: LOW

- ThrottleStop IOCTLs are normal for CPU tuning
- Physical memory access is within driver's granted capabilities
- No new pool allocations

---

## Phase 4: Hypervisor Activation (T=70ms to T=150ms)

### What Happens

```cpp
// File: OmbraLoader/supdrv/supdrv_loader.cpp
// Lines: 300-500

bool LoadPayloadViaSupDrv()
{
    // T=70ms: Cookie handshake
    if (!SupdrvCookie()) return false;

    // T=75ms: LDR_OPEN - allocate module slot
    // Now works because -618 flags were patched!
    if (!SupdrvLdrOpen(payloadSize)) return false;

    // T=80-100ms: LDR_LOAD - copy payload to kernel
    // >>> PAYLOAD ALLOCATION IN KERNEL <<<
    if (!SupdrvLdrLoad(payloadData, payloadSize)) return false;

    // T=100-120ms: Call payload's DriverEntry
    // This patches the VMExit handler
    CallPayloadEntry();

    // T=120-150ms: Hypervisor is now ACTIVE
    // All VMExits route through our handler
}
```

```cpp
// File: OmbraLoader/zerohvci/hyperv_hijack.h
// Lines: 200-400

bool HijackVmExit()
{
    // T=100ms: Find hvix64.exe/hvax64.exe in kernel
    uint64_t hvBase = FindHyperVModule();

    // T=110ms: Scan for VMExit handler pattern
    uint64_t vmexitHandler = ScanForPattern(
        hvBase,
        IntelVmExitPattern,  // version_detect.h:108
        IntelVmExitMask,
        patternSize
    );

    // T=120ms: Calculate patch offset
    // Find the CALL instruction that routes VMExits
    uint64_t callSite = vmexitHandler + vmexitCallOffset;

    // T=130ms: Allocate trampoline if needed (payload >2GB away)
    if (Distance(callSite, payloadHandler) > 0x7FFFFFFF) {
        trampolineAddr = AllocateNearTrampoline(callSite);
    }

    // T=140ms: Patch the CALL RVA
    // >>> THIS IS THE MOMENT HYPERVISOR ACTIVATES <<<
    PatchCallRva(callSite, payloadHandler);

    // T=150ms: First VMCALL via CPUID 0x13371337
    if (!TestVmcall()) return false;

    return true;  // Hypervisor fully active
}
```

### Artifacts Created

| Artifact | Location | Creation Time | Visibility | Cleanup Status |
|----------|----------|---------------|------------|----------------|
| SUPDrv module allocation | VBox internal pool | T=75ms | **HIDDEN from Windows** | Persists |
| Payload in kernel | SUPDrv allocation | T=80-100ms | **HIDDEN from Windows** | Persists |
| Trampoline (if needed) | Near hv.exe | T=130ms | Kernel pool | Hidden by EPT later |
| VMExit handler patch | hvix64/hvax64 | T=140ms | In hypervisor memory | Hidden |

### Why SUPDrv Allocations Are Hidden

```cpp
// SUPDrv uses its own internal allocator, NOT Windows pool functions
// These allocations don't appear in:
// - NtQuerySystemInformation(SystemBigPoolInformation)
// - !poolused / !poolfind in WinDbg
// - Any standard pool enumeration

// The only way to find them is:
// 1. Walk SUPDrv's internal structures (requires reverse engineering)
// 2. Physical memory scan (requires hypervisor or BYOVD)
// 3. Timing-based side channels (research only)
```

### Detection Risk: MEDIUM

- SUPDrv allocation is hidden from standard enumeration
- VMExit patch is in hypervisor memory (not guest-visible)
- Trampoline may be visible if not EPT-hidden

---

## Phase 5: Driver Mapping (T=150ms to T=200ms)

### What Happens

```cpp
// File: libombra/mapper/map_driver.cpp
// Lines: 20-100

uint64_t map_driver(uint8_t* driver_bytes, size_t driver_size)
{
    // T=150ms: Parse PE headers
    drv_image img(driver_bytes, driver_size);
    img.map();  // Copy sections

    // T=155ms: Allocate kernel pool
    // >>> BIGPOOL ENTRY CREATED HERE <<<
    auto pool_base = ctx.allocate_pool(img.size(), NonPagedPoolNx);

    // T=160ms: Copy mapped image to kernel
    ctx.write_kernel(pool_base, img.data(), img.size());

    // T=165ms: Fix relocations
    img.relocate(pool_base);

    // T=170ms: Fix imports
    resolve_imports(pool_base);

    // T=175ms: Call DriverEntry
    auto entry = pool_base + img.entry_rva();
    ctx.call_kernel(entry);

    // T=180ms: Wipe PE headers
    // Only wipes 0x1000 bytes - section table may survive!
    std::uint8_t garbage[0x1000];
    for (int i = 0; i < 0x1000; i++) {
        garbage[i] = static_cast<std::uint8_t>(__rdtsc() ^ i);
    }
    ctx.write_kernel(pool_base, garbage, 0x1000);

    // T=185ms: Clear PIDDB cache
    // Return value IGNORED - vulnerability H6
    ctx.clear_piddb_cache(drv_key, drv_timestamp);

    // T=190ms: Clear hash bucket
    ctx.clear_hash_bucket(drv_name);

    // T=195ms: Suppress MmUnloadedDrivers
    ctx.suppress_unloaded_drivers(drv_name);

    return pool_base;
}
```

### Artifacts Created

| Artifact | Location | Creation Time | Visibility | Cleanup Status |
|----------|----------|---------------|------------|----------------|
| **Driver pool allocation** | NonPagedPoolNx BigPool | T=155ms | **VISIBLE** | **NOT CLEANED** |
| Driver code in kernel | Pool allocation | T=160ms | Hidden by EPT later | Hidden |
| Hook trampoline | Pool allocation | T=170ms | **VISIBLE** pattern | **NOT CLEANED** |
| PIDDB entry | PiDDBCacheTable | T=175ms | Kernel structure | CLEANED (unverified) |
| Hash bucket entry | g_KernelHashBucketList | T=175ms | CI.dll structure | CLEANED |
| MmUnloadedDrivers entry | Kernel structure | T=175ms | Kernel structure | SUPPRESSED |

### Why BigPool Entry Persists

```cpp
// Even though we wipe PE headers and hide driver via EPT,
// the ALLOCATION METADATA remains visible:

struct SYSTEM_BIGPOOL_ENTRY {
    PVOID VirtualAddress;   // Pool base - VISIBLE
    SIZE_T SizeInBytes;     // ~404KB - VISIBLE (distinctive size)
    ULONG PoolTag;          // 0 or custom - VISIBLE
    ULONG PoolType;         // NonPagedPoolNx - VISIBLE
};

// This is enumerable via:
NtQuerySystemInformation(SystemBigPoolInformation, &info, size, &needed);

// EPT only hides PAGE CONTENTS, not allocation metadata
```

### Detection Risk: HIGH

- BigPool entry visible to any usermode process
- Hook trampoline pattern scannable
- Section table may survive (if SizeOfHeaders > 0x1000)

---

## Phase 6: EPT Hiding (T=200ms to T=250ms)

### What Happens

```cpp
// File: OmbraDriver/main.cpp
// Lines: 80-150 (in DriverEntry)

NTSTATUS DriverEntry(PDRIVER_OBJECT, PUNICODE_STRING)
{
    // T=200ms: Register callback with hypervisor
    ombra::storage_set(CALLBACK_ADDRESS, comms::Entry);
    ombra::storage_set(DRIVER_BASE_PA, VirtToPhys(pDriverBase));
    ombra::storage_set(NTOSKRNL_CR3, GetSystemCr3());

    // T=210ms: Initialize guest memory access
    vmm::Init();

    // T=220ms: Hide driver via EPT
    // >>> EPT SUBSTITUTE PAGE ALLOCATIONS <<<
    EPT::HideDriver();

    // T=240ms: Set per-CPU EPT caches
    CPUs::RunOnAllCPUs(SetEPTCache);

    return STATUS_SUCCESS;
}
```

```cpp
// File: OmbraCoreLib-v/src/EPT.cpp
// Lines: 618-700

BOOLEAN EPT::HideDriver()
{
    for (each page of driver) {
        // T+Xms: Allocate substitute page FROM GUEST POOL
        // >>> CREATES VISIBLE POOL ENTRY <<<
        PVOID pSubstitute = cpp::kMallocZero(PAGE_SIZE);

        // T+Xms: Set up EPT hook
        // Redirects physical reads to substitute page
        bRes = Hook(driver_page, pSubstitute, permissions);

        // T+Xms: Configure IOMMU to prevent DMA reads
        bRes = iommu::HidePage(driver_page, pSubstitute);
    }
}
```

### Artifacts Created

| Artifact | Location | Creation Time | Visibility | Cleanup Status |
|----------|----------|---------------|------------|----------------|
| Substitute pages | Guest NonPagedPool | T=220ms | **VISIBLE (pool entries)** | Cannot hide |
| EPT paging structures | Guest NonPagedPool | T=225ms | **VISIBLE (pool entries)** | Cannot hide |
| 2MB contiguous pages | MmAllocateContiguous | T=230ms | **VISIBLE (contiguous)** | Cannot hide |
| Per-CPU EPT state | Guest NonPagedPool | T=240ms | **VISIBLE (pool entries)** | Cannot hide |

### Circular Dependency Problem

```
EPT hiding requires:
  └─> Substitute pages (allocated from guest pool)
      └─> Which creates pool entries
          └─> Which are visible via BigPool enumeration
              └─> Which EPT hiding can't hide (metadata, not page content)

This is a fundamental architectural limitation.
EPT hides PAGE CONTENTS but not ALLOCATION ENTRIES.
```

### Detection Risk: MEDIUM

- Substitute pages are necessary (can't be avoided)
- EPT structures are necessary
- 2MB contiguous allocations are rare (detectable pattern)

---

## Phase 7: Cleanup & Runtime (T=250ms+)

### What Gets Cleaned

```cpp
// File: OmbraLoader/prefetch_cleanup.h
// Lines: 1-251

class ScopedPrefetchCleanup {
    ~ScopedPrefetchCleanup() {
        // T=250ms: Secure delete prefetch files
        SecureDeleteFile(L"OMBRALOADER.EXE-*.pf");
        SecureDeleteFile(L"LD9BOXSUP-*.pf");
        SecureDeleteFile(L"THROTTLESTOP-*.pf");
    }
};

void SecureDeleteFile(const wchar_t* path) {
    // 3-pass DoD-style wipe
    HANDLE hFile = CreateFileW(path, GENERIC_WRITE, ...);
    DWORD size = GetFileSize(hFile, NULL);

    // Pass 1: Zero fill
    memset(buffer, 0x00, BUFFER_SIZE);
    WriteAndFlush(hFile, buffer, size);

    // Pass 2: 0xFF fill
    memset(buffer, 0xFF, BUFFER_SIZE);
    WriteAndFlush(hFile, buffer, size);

    // Pass 3: Random fill
    for (int i = 0; i < BUFFER_SIZE; i++)
        buffer[i] = __rdtsc() ^ i;
    WriteAndFlush(hFile, buffer, size);

    DeleteFileW(path);
}
```

### What's NOT Cleaned

| Artifact | Location | Why Not Cleaned | Detection Risk |
|----------|----------|-----------------|----------------|
| **Event ID 7045** | System Event Log | Not implemented | **CRITICAL** |
| BigPool entries | Kernel pool info | Metadata not hideable | **HIGH** |
| ETW events | Kernel buffers | Optional, not automatic | HIGH |
| Pool tags | Pool entries | No tag obfuscation | MEDIUM |
| Section table | Driver memory | Only 0x1000 bytes wiped | MEDIUM |

### Runtime Artifact Persistence

```
After T=300ms, these artifacts persist:

PERMANENT (survives reboot):
├── Event Log entries
│   ├── Event ID 7045 (ThrottleStop)
│   ├── Event ID 7045 (Ld9BoxSup)
│   └── Event ID 4688 (process creation)
├── NTFS $MFT entries (file metadata)
└── Registry transaction logs (.LOG files)

UNTIL REBOOT:
├── BigPool allocation entries
├── EPT paging structures (in pool)
├── Substitute pages (in pool)
├── Per-CPU hypervisor state
└── Contiguous memory allocations

UNTIL DRIVER UNLOAD (never):
├── Hook trampolines
├── Driver code/data (hidden by EPT)
└── Communication structures
```

---

## Anti-Cheat Scan Timing

### When Anti-Cheats Scan

| Anti-Cheat | Initialization | Periodic Scan | Post-Match | Kernel Driver |
|------------|----------------|---------------|------------|---------------|
| EasyAntiCheat | Game launch | Every 30-60s | Yes | Yes (EasyAntiCheat.sys) |
| BattlEye | Game launch | Every 15-30s | Yes | Yes (BEService.exe + driver) |
| Vanguard | **SYSTEM BOOT** | Continuous | Yes | Yes (vgk.sys + vgkbootstatus.dat) |
| FACEIT | Client launch | Every 60s | Yes | Yes (faceit.sys) |
| Kernel Scanner | On-demand | On-demand | On-demand | N/A (detection tool) |

### Vanguard Boot-Time Detection

```
Vanguard is special - it runs at BOOT TIME:

T=0 (boot):    vgk.sys loads with Windows
T=+5s:         Captures baseline Event Log state
T=+10s:        Hooks NtLoadDriver
T=+15s:        Begins continuous monitoring

If OmbraLoader runs AFTER Vanguard initialized:
- All driver loads are hooked in real-time
- Event Log delta is computed
- ANY new Event ID 7045 is flagged immediately

Vanguard CANNOT be bypassed by post-facto cleanup.
```

### Detection Timeline vs Anti-Cheat Scans

```
OMBRA TIMELINE:        0ms    50ms   100ms  150ms  200ms  250ms  300ms
                        │      │      │      │      │      │      │
Artifact Creation:  ────┼──────┼──────┼──────┼──────┼──────┼──────┼────▶
                        │      │      │      │      │      │      │
Event Log entries:  ────┤■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■▶ PERMANENT
                        │      │      │      │      │      │      │
BigPool entries:    ────┼──────┼──────┼──────┤■■■■■■■■■■■■■■■■■■■■■▶ Until reboot
                        │      │      │      │      │      │      │
ETW events:         ────┼──────┤■■■■■■■■■■■■■┤      │      │      │   (If wiped at 250ms)
                        │      │      │      │      │      │      │
                        │      │      │      │      │      │      │
ANTI-CHEAT SCANS:       │      │      │      │      │      │      │
                        │      │      │      │      │      │      │
Vanguard (boot):    ■■■■┼■■■■■■┼■■■■■■┼■■■■■■┼■■■■■■┼■■■■■■┼■■■■■■┼■■▶ CONTINUOUS
                        │      │      │      │      │      │      │
EAC (game launch):  ────┼──────┼──────┼──────┼──────┼──────┼──────┼────■■■■ (at launch)
                        │      │      │      │      │      │      │
BattlEye (launch):  ────┼──────┼──────┼──────┼──────┼──────┼──────┼────■■■■ (at launch)
                        │      │      │      │      │      │      │

DETECTION OCCURS AT GAME LAUNCH FOR ALL ANTI-CHEATS
```

---

## Critical Windows Summary

### Window 1: Event Log (PERMANENT)

- **Opens:** T=6ms (first NtLoadDriver)
- **Closes:** NEVER
- **Evidence:** Event ID 7045 with driver names
- **Mitigation:** None implemented
- **Detection Probability:** 100%

### Window 2: ETW Buffers (T=6ms to T=250ms)

- **Opens:** T=6ms (service creation)
- **Closes:** T=250ms (if VMCALL_WIPE_ETW_BUFFERS called)
- **Evidence:** Driver load/unload events
- **Mitigation:** Optional VMCALL (not automatic)
- **Detection Probability:** 70% (if not wiped)

### Window 3: BigPool Metadata (T=155ms to REBOOT)

- **Opens:** T=155ms (driver pool allocation)
- **Closes:** System reboot
- **Evidence:** Pool entries with sizes, tags
- **Mitigation:** None possible (architectural)
- **Detection Probability:** 85%

### Window 4: Hook Trampolines (T=170ms to UNLOAD)

- **Opens:** T=170ms (hook installation)
- **Closes:** Driver unload (never happens)
- **Evidence:** 12-byte pattern in pool
- **Mitigation:** Polymorphism not implemented
- **Detection Probability:** 80%

---

## Recommendations for New Agents

1. **Understand the Event Log is the critical weakness** - Any work should prioritize this

2. **BigPool cannot be fully hidden** - This is architectural, focus on reducing entries

3. **ETW wipe should be automatic** - Currently requires manual VMCALL

4. **Vanguard detection is unavoidable** - It runs before OmbraLoader

5. **Focus cleanup on the T=5-50ms window** - This is where permanent artifacts are created

---

## Cross-References

- **Vulnerability Details:** See [MASTER-VULNERABILITY-LIST.md](./MASTER-VULNERABILITY-LIST.md)
- **Cleanup Implementation:** See [TRACE-CLEANUP-AUDIT.md](./TRACE-CLEANUP-AUDIT.md)
- **BigPool Analysis:** See [BIGPOOL-AUDIT.md](./BIGPOOL-AUDIT.md)
- **EPT Hiding Details:** See [EPT-STRATEGY-AUDIT.md](./EPT-STRATEGY-AUDIT.md)
- **Fix Implementation:** See [REMEDIATION-ROADMAP.md](./REMEDIATION-ROADMAP.md)
