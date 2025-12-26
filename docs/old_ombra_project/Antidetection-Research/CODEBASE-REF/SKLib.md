# SKLib - Kernel Utilities Library

**Source**: `Refs/codebases/SKLib/`
**Purpose**: Comprehensive kernel-mode utility library for Windows driver development
**Context**: Used by Sputnik bootkit, provides memory manipulation, pattern scanning, process utilities, and Windows version-specific offset management

---

## 1. KERNEL UTILITIES

### Memory Manipulation Functions

**Virtual Memory Operations**
- `ReadVirtualMemory()` - `src/MemoryEx.cpp:4-9`
  - Wraps `MmCopyVirtualMemory` for cross-process reads
  - Source process → current process transfer
  - Returns `NTSTATUS` for error handling

- `WriteVirtualMemory()` - `src/MemoryEx.cpp:11-16`
  - Wraps `MmCopyVirtualMemory` for cross-process writes
  - Current process → target process transfer
  - Uses `KernelMode` processor mode

- `CopyPhysicalMemory()` - `src/MemoryEx.cpp:18-27`
  - Uses `MM_COPY_ADDRESS` with physical address
  - `MmCopyMemory` with `MM_COPY_MEMORY_PHYSICAL` flag
  - Direct physical memory access

**Virtual/Physical Translation**
- `VirtToPhy()` - `src/MemoryEx.cpp:77-80`
  - `MmGetPhysicalAddress(Va).QuadPart`
  - Simple wrapper, returns PA as UINT64

- `PhyToVirt()` - `src/MemoryEx.cpp:82-88`
  - `MmGetVirtualForPhysical()` conversion
  - Returns virtual address for given PA

**Write Protection Bypass**
- `WriteProtected()` - `src/MemoryEx.cpp:448-455`
  - Disables interrupts via `CPU::DisableInterrupts()`
  - Disables CR0.WP via `CPU::DisableWriteProtection()`
  - Copies memory, re-enables protections
  - Atomic write operation

- `VirtualProtect()` - `src/MemoryEx.cpp:29-75`
  - Allocates MDL pages via `MmAllocatePagesForMdl`
  - Builds partial MDL for target region
  - Applies protection flags via `MmProtectMdlSystemAddress`
  - Used for changing page permissions

### Process Utilities

**Process Enumeration**
- `GetProcess()` - `src/MemoryEx.cpp:137-171`
  - `ZwQuerySystemInformation(SystemProcessInformation)`
  - Walks process list by `NextEntryDelta`
  - Compares `UNICODE_STRING` process names
  - Returns `PSYSTEM_PROCESSES` structure

- `GetKernelAddress()` - `src/MemoryEx.cpp:90-135`
  - `ZwQuerySystemInformation(SystemModuleInformation)`
  - Iterates loaded modules via `SYSTEM_MODULE_INFORMATION`
  - Matches module name via `strstr()`
  - Returns module base address

**Process Name Retrieval**
- `GetDriverNameForAddress()` - `src/MemoryEx.cpp:173-221`
  - Queries `SystemModuleInformation`
  - Checks if address falls within module range
  - Returns allocated copy of `ImageName`

- `GetDriverNameForProcess()` - `src/MemoryEx.cpp:232-306`
  - `ObOpenObjectByPointer()` to get handle
  - `ZwQueryInformationProcess(ProcessImageFileName)`
  - Resolves function pointer dynamically via `MmGetSystemRoutineAddress`
  - Returns `PUNICODE_STRING` with full image path

**Process Attachment**
- `AttachToProcessId()` - `src/MemoryEx.cpp:433-441`
  - `PsLookupProcessByProcessId()` to get `PEPROCESS`
  - Allocates `RKAPC_STATE` structure
  - `KeStackAttachProcess()` to switch context
  - Returns state pointer for later detach

- `DetachFromProcess()` - `src/MemoryEx.cpp:443-446`
  - `KeUnstackDetachProcess((PRKAPC_STATE)pRkapcState)`
  - Frees allocated state structure

### WinInternals Integration

**Undocumented Function Resolution** (`src/winternlex.cpp:449-606`)
- `PsEnumProcesses` - offset-based resolution from ntoskrnl
- `PspSetQuotaLimits` - quota manipulation function
- `MmQueryVirtualMemory` - memory query internals
- `PspInsertProcess/Thread` - process/thread creation hooks
- `PspTerminateProcess` - termination internal
- `PsQueryFullProcessImageName` - full path retrieval
- All resolved via: `(fnType)((DWORD64)ntoskrnlBase + offsets.FunctionName)`

**Cleanup Functions** (`src/winternlex.cpp`)
- `ClearPIDDBCacheTable()` - `186-246`
  - Acquires `PiDDBLock` via `ExAcquireResourceExclusiveLite`
  - `RtlLookupElementGenericTableAvl` to find entry
  - Unlinks from doubly-linked list (Flink/Blink)
  - `RtlDeleteElementGenericTableAvl` to remove
  - Decrements `DeleteCount`

- `ClearKernelHashBucketList()` - `248-307`
  - Locks `HashCacheLock` in CI.dll
  - Walks `HASH_BUCKET_ENTRY` linked list
  - Matches driver name via `wcsstr()`
  - Unlinks and frees entry

- `ClearMmUnloadedDrivers()` - `309-394`
  - `ZwQuerySystemInformation(SystemExtendedHandleInformation)`
  - Walks handle table to find device object
  - Traverses: `Object→DeviceObject→DriverObject→DriverSection`
  - Zeroes `UNICODE_STRING.Length` to prevent recording

---

## 2. PATTERN SCANNING

### Signature Scanning Implementation

**Basic Pattern Matching**
- `CheckMask()` - `src/MemoryEx.cpp:308-316`
  - Iterates pattern/mask simultaneously
  - 'x' in mask = must match, other = wildcard
  - Returns `TRUE` on full match

- `FindPattern()` - `src/MemoryEx.cpp:318-328`
  - `length -= strlen(mask)` for bounds checking
  - Loops through memory calling `CheckMask()`
  - Returns first match address or 0

**Image-Specific Scanning**
- `FindPatternImage()` - `src/MemoryEx.cpp:356-373`
  - Parses PE headers to get section table
  - Searches only PAGE/.text sections
  - Calls `FindPattern()` per section
  - Avoids scanning data/resource sections

**Section Utilities**
- `FindSection()` - `src/MemoryEx.cpp:330-345`
  - Iterates `IMAGE_SECTION_HEADER` array
  - `memcmp(section->Name, pSectionName, strlen(pSectionName))`
  - Returns section VA: `pImageBase + section->VirtualAddress`

**Function Boundary Detection**
- `FindFunctionStart()` - `src/MemoryEx.cpp:347-354`
  - Walks backwards from address
  - Searches for `0xcccc` (INT3 padding)
  - Returns `currPtr + 2` (first instruction after padding)

- `FindDriverBase()` - `src/MemoryEx.cpp:423-431`
  - Page-aligns address via `PAGE_ALIGN()`
  - Walks backwards by page
  - Checks for MZ header (`0x5a4d`)
  - Returns base of PE image

**Byte Series Scanning**
- `FindByteSeries()` - `src/MemoryEx.cpp:375-395`
  - Finds consecutive bytes of specific value
  - Resets counter when mismatch
  - Returns start of series

- `FindByteSeriesSafe()` - `src/MemoryEx.cpp:397-421`
  - Same as above but checks `MmIsAddressValid()` each byte
  - Prevents page faults on invalid memory

### Wildcard Handling

**Template-Based Physical Memory Scanning** (`include/MemoryEx.h:145-246`)

- `ForEachBytePatternInPhy<F>()` - `145-177`
  - Iterates `MmGetPhysicalMemoryRanges()`
  - Maps each page via `paging::MapToGuest()`
  - Skips driver addresses via `winternl::IsDriverAddress()`
  - Calls callback for each pattern match
  - Template allows custom lambda handling

- `ForEachBytePatternInPhyDevice<F>()` - `179-212`
  - Uses MTRR ranges instead of `MmGetPhysicalMemoryRanges()`
  - Limits scan to first 256MB (`currAddress > SIZE_2_MB * 0x10`)
  - Device memory-specific scanning

- `ForEachBytePatternInPhyUncached<F>()` - `214-246`
  - Scans only uncached MTRR regions
  - `GetMemoryRangeDescriptors()` for MTRR iteration
  - Useful for device memory/firmware scanning

**Generic Pattern Iteration** (`include/MemoryEx.h:132-143`)
- `ForEachBytePattern<F>()` - Template callback approach
  - `memcmp(pBase + i, pPattern, szPattern)`
  - Invokes callback `fnCallback(pBase + i)` on match
  - Bounds: `maxLen - szPattern`

---

## 3. WINDOWS VERSION DETECTION

### Build Number Retrieval

**NTOSKRNL Base Resolution** (`src/winternlex.cpp:159-173`)
- `RtlPcToFileHeader()` - Dynamic resolution via `MmGetSystemRoutineAddress`
- Calls with self-reference to get ntoskrnl base
- Caches result in `ntoskrnlBase` global

**Image Info Initialization** (`src/winternlex.cpp:449-456`)
- `InitImageInfo(PVOID pImageBase)`
  - Sets `winternl::pDriverBase` to provided base
  - `PE(pImageBase).imageSize()` for driver size
  - `GetNtoskrnlBaseAddress()` cached lookup
  - Logs: "Driver base: %p", "NTOSKRNL base: %p"

### Version-Specific Offsets

**Offset Structure** (`include/data.h:30-104`)
```cpp
struct OffsetDump {
    // RAID identifiers
    ULONG64 RaidIdentity;
    ULONG64 RaidSerialNumber;
    ULONG64 RaidUnitRegInterface;
    ULONG64 ScsiSerialNumber;

    // Network interface
    ULONG64 NdisGlobalFilterList;
    ULONG64 FilterBlockNextFilter;
    ULONG64 MiniportBlockInterfaceGuid;

    // SMBIOS
    ULONG64 WmipSMBiosTableLength;

    // TEB/Client info
    ULONG64 ClientInfo;
    ULONG64 HwndCache;

    // CI.dll code integrity
    ULONG64 g_KernelHashBucketList;
    ULONG64 g_HashCacheLock;

    // Function offsets (from ntoskrnl base)
    ULONG64 PsEnumProcesses;
    ULONG64 PspInsertProcess;
    ULONG64 PspInsertThread;
    ULONG64 PspTerminateProcess;
    ULONG64 MmQueryVirtualMemory;
    ULONG64 BgpFwQueryBootGraphicsInformation;
    ULONG64 PsQueryFullProcessImageName;
    ULONG64 ZwSetInformationProcess;

    // Data structure offsets
    ULONG64 ExpBootEnvironmentInformation;
    ULONG64 WmipSMBiosTablePhysicalAddress;
    ULONG64 PiDDBLock;
    ULONG64 PiDDBCacheTable;
    ULONG64 KiNmiInterruptStart;
    ULONG64 NtLockVirtualMemory;
};
```

**Offset Usage Pattern**
- `VALID_OFFSET(offset)` macro checks for `offset && (offset != MAXULONG64)`
- Example: `winternl::PsEnumProcesses = (fnType)((ULONG64)ntoskrnlBase + offsets.PsEnumProcesses)`
- All internal functions resolved this way

**USERMODE_INFO Structure** (`include/data.h:113-133`)
- Shared between usermode and kernel
- Contains `OffsetDump offsets` member
- Passed during initialization
- Includes VMCALL key, driver base/size, cleanup data

---

## 4. MEMORY UTILITIES

### Physical/Virtual Mapping

**Manual Page Mapping** (`src/paging.cpp:418-442`)
- `MapToGuest(PVOID pa)` - `418-441`
  - Allocates buffer page on first call
  - Gets PTE via `paging::GetPPTE()`
  - Stores original PTE, sets Write=1, ExecuteDisable=0
  - `pGuestPageToSwap->PageFrameNumber = (DWORD64)pa >> 12`
  - `__invlpg()` to flush TLB
  - Preserves page offset: `result.offset_4kb = VIRT_ADD{pa}.offset_4kb`
  - Returns virtual address mapping to physical page

- `MapManually(PVOID pa)` - `443-462`
  - Creates new mapping per call (not reusing buffer)
  - Same technique: allocate page, get PTE, swap PFN
  - Useful for concurrent mappings

- `RestoreMapPage()` - `464-469`
  - Restores original PTE: `*pGuestPageToSwap = pgOrig`
  - Cleanup after `MapToGuest` usage

**Physical Address Translation** (`src/paging.cpp:322-385`)
- `GuestVirtToPhy()` - `322-385`
  - Uses `identity::PhysicalAccess` for direct physical reads
  - Manual page table walk: PML4E → PDPTE → PDE → PTE
  - Handles large pages (1GB via `PDPTE_1GB_64`, 2MB via `PDE_2MB_64`)
  - Sets `*pIsLargePage` if encountered
  - Returns PA + offset within page
  - Disables interrupts during walk

- `ProcessVirtToPhy()` - `387-397`
  - Gets process CR3 via `PsProcessDirBase(pEprocess)`
  - Maps PML4 via `MapPML4Base(cr3)`
  - Gets PTE, returns `PageFrameNumber * PAGE_SIZE`

**Page Table Manipulation** (`src/paging.cpp:114-191`)
- `MapPage(ppml4t, va, pa, manualMapTracking)` - `114-191`
  - 4-level page table setup:
    - PML4E: Check if PDPT exists, allocate if not
    - PDPTE: Check if PDT exists, allocate if not
    - PDE: Check if PT exists, allocate if not
    - PTE: Set PFN to physical address
  - Each level: `PhyToVirt(entry.PageFrameNumber * PAGE_SIZE)`
  - Sets Present=1, Write=1, Supervisor=1
  - Tracks allocations in `MANUAL_PAGED_TABLES` structure
  - Returns TRUE on success

- `GetPPTE(ppml4t, va, bMap)` - `src/paging.cpp:211-251`
  - Walks page tables to get PTE pointer
  - `bMap=TRUE`: uses `MapToGuest()` for each level
  - `bMap=FALSE`: uses `PhyToVirt()` for each level
  - Returns `&pOrigPt->entry[virtAddMap.Level1]`

### Safe Read/Write

**Safe Byte Series** - `src/MemoryEx.cpp:397-421`
- `FindByteSeriesSafe()` validates each byte via `MmIsAddressValid()`
- Returns `nullptr` on invalid address
- Prevents crashes during memory scanning

**MDL-Based Operations** (`src/paging.cpp:257-282`)
- `LockRange(pBase, size)` - `257-276`
  - `IoAllocateMdl()` for range
  - `MmProbeAndLockPages()` with `IoModifyAccess`
  - `MmBuildMdlForNonPagedPool()`
  - Ensures pages stay in memory

- `UnlockRange(pMdl)` - `278-282`
  - `MmUnlockPages(pMdl)`
  - `IoFreeMdl(pMdl)`

**Protected Memory Write** - `src/MemoryEx.cpp:448-455`
- Atomic sequence: disable interrupts → disable WP → copy → restore WP → restore interrupts
- Uses `CPU::DisableWriteProtection()` which handles both CR0.WP and CET

### MTRR Memory Range Detection

**Range Building** (`src/MemoryEx.cpp:460-502`)
- `BuildMemoryRanges()` - `460-502`
  - Reads `MSR_IA32_MTRR_CAPABILITIES` for count
  - Iterates variable MTRR pairs (PHYSBASE + PHYSMASK)
  - `MSR_IA32_MTRR_PHYSBASE0 + (CurrentRegister * 2)`
  - Checks `CurrentPhysMask.Valid` flag
  - Calculates size via `_BitScanForward64()` on mask
  - Formula: `PhysicalEndAddress = BaseAddress + ((1ULL << bits) - 1ULL)`
  - Filters to `MEMORY_TYPE_UNCACHEABLE` only
  - Stores in global `MemoryRanges[9]` array

**Range Query Functions** (`src/MemoryEx.cpp:524-547`)
- `GetMemoryRangeDescriptors()` - `524-530`
  - Builds ranges on first call
  - Returns pointer to `MemoryRanges` array

- `GetMemoryRangeDescriptorsLength()` - `532-534`
  - Returns `NumberOfEnabledMemoryRanges`

- `IsInMemoryRanges(pBase)` - `536-547`
  - Checks if address falls within any MTRR range
  - Compares against `PhysicalBaseAddress` and `PhysicalEndAddress`

**Page Iteration Templates** (`include/MemoryEx.h:248-346`)
- `ForEachPageInPhy<F>()` - Uses `MmGetPhysicalMemoryRanges()`
- `ForEachPageUncachedInPhy<F>()` - Uses MTRR uncached ranges
- `ForEachPageInPhyDevice<F>()` - Device memory specific (limits to 256MB)
- `ForEachPageMapped<F>()` - Walks all mapped pages via `SystemBasicInformation`
- All use `paging::MapToGuest()` to access physical pages

---

## 5. CPU DETECTION & UTILITIES

### Intel vs AMD Detection (`src/cpu.cpp:20-28`)
- `IsIntelCPU()` - Uses `Cpuid::Generic::MaximumFunctionNumberAndVendorId`
- Sets global `bIntelCPU` flag
- Logs: "[CPU] Is Intel" or "[CPU] Is not Intel"

### Virtualization Support (`src/cpu.cpp:359-384`)
- `IsVirtSupported()`
  - **Intel path:**
    - Reads `IA32_FEATURE_CONTROL` MSR
    - Calls `EnableVmx()`, checks `IsVmxEnabled()`
    - Verifies lock bit (bit 0) and VMX enable bit (bit 2)
    - Enables if unlocked, returns FALSE if locked incorrectly
  - **AMD path:**
    - Calls `CheckForSvmFeatures()` (line 349-357)
    - Checks: SVM supported, NPT supported, ASID flush, SVMDIS not set

### Write Protection Control (`src/cpu.cpp:386-426`)
- `DisableWriteProtection()` - `386-393`
  - Calls `DisableCET()` first
  - Reads CR0, sets `WriteProtect = false`
  - `__writecr0(cr0.Flags)`
  - Returns CET state for restore

- `EnableWriteProtection(bEnableCET)` - `395-402`
  - Reads CR0, sets `WriteProtect = true`
  - Optionally re-enables CET

- `DisableCET()/EnableCET()` - `404-426`
  - Checks `bCETSupported` flag
  - Modifies `CR4.CETEnabled` bit

### Hypervisor Detection (`src/cpu.cpp:49-301`)
**Timing-Based Detection:**
- `GetTSCRate()` - Averages RDTSC delta across CPUID calls
- `DetectTimeStampBasic/Advanced()` - RDTSC checks with/without IRQL elevation
- `DetectMPERFBasic/Advanced()` - MSR 0xE7 (IA32_MPERF) checks
- `DetectAPERFBasic/Advanced()` - MSR 0xE8 (IA32_APERF) checks

**Detection Thresholds:**
- Suspicious if average > 1000 cycles or < 0x33 cycles
- Checks for backwards TSC (tick1 > tick2)
- Advanced variants raise IRQL to `HIGH_LEVEL` to reduce noise

### CPU Index/Count (`src/cpu.cpp:321-334`)
- `GetCPUIndex(bVmxRoot)` - `321-328`
  - Reads APIC ID via `Cpuid::Generic::FeatureInformation`
  - Maps to processor index via `procMap[apicId]`
  - Updates map if not in VMX root

- `GetCPUCount()` - `330-334`
  - `KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS)`
  - Limits to 64 CPUs (static cache)

---

## 6. NOTABLE PATTERNS FOR OMBRA

### Pattern 1: Physical Memory Direct Access
**Source**: `src/paging.cpp:418-441` (MapToGuest)
**Technique**: Temporary PTE manipulation
- Allocate scratch page
- Get PTE for scratch page
- Swap PTE's PFN to target physical address
- Invalidate TLB entry
- Access via scratch VA
- **Ombra Application**: EPT manipulation, physical memory scanning without triggering PatchGuard

### Pattern 2: Offset-Based Internal Function Resolution
**Source**: `src/winternlex.cpp:507-605`
**Technique**: Hardcoded offsets from ntoskrnl base
- Pre-calculated offsets for Windows build
- `(functionType)((DWORD64)ntoskrnlBase + offset)`
- No import table, no inline hooks detectable
- **Ombra Application**: Accessing internal scheduler functions, process manipulation without obvious hooks

### Pattern 3: MTRR-Based Device Memory Scanning
**Source**: `src/MemoryEx.cpp:460-502`, `include/MemoryEx.h:179-212`
**Technique**: Read MTRR MSRs to identify uncached device memory
- Parse variable MTRRs (MSR 0x200-0x20F)
- Filter to uncacheable regions only
- Scan for firmware/device structures
- **Ombra Application**: Finding UEFI runtime services, ACPI tables, SMM regions

### Pattern 4: Process Context Switching
**Source**: `src/MemoryEx.cpp:433-446`
**Technique**: `KeStackAttachProcess` for usermode access
- Get `PEPROCESS` from PID
- Allocate `RKAPC_STATE` context
- Attach to process context
- Perform operations (VA reads become valid in target process)
- Detach and free context
- **Ombra Application**: Injecting into game process from hypervisor, reading game memory structures

### Pattern 5: Driver Trace Cleanup
**Source**: `src/winternlex.cpp:186-394`
**Technique**: Multi-layered driver artifact removal
- Clear PiDDBCacheTable (driver database)
- Clear CI.dll hash cache
- Clear MmUnloadedDrivers list
- **Ombra Application**: Hide OmbraPayload from forensic tools, evade driver signature enforcement checks

### Pattern 6: Template-Based Physical Scanning
**Source**: `include/MemoryEx.h:145-246`
**Technique**: Callback-driven pattern matching
```cpp
Memory::ForEachBytePatternInPhy(pattern, size, [](char* match) {
    // Process match
});
```
- **Ombra Application**: Finding Hyper-V structures in physical memory, locating vmbus pages, identifying VMCS regions

### Pattern 7: Safe Memory Traversal
**Source**: `src/MemoryEx.cpp:397-421`
**Technique**: `MmIsAddressValid()` at every access
- Prevents page faults during blind scanning
- Essential for physical memory iteration
- **Ombra Application**: Scanning physical memory without crashing when hitting MMIO holes

### Pattern 8: PE Section-Targeted Scanning
**Source**: `src/MemoryEx.cpp:356-373`
**Technique**: Parse PE headers, scan only executable sections
- Avoids false positives from data sections
- Faster than full image scan
- **Ombra Application**: Finding PatchGuard patterns in ntoskrnl, locating hypervisor detection code in anti-cheat

---

## KEY TAKEAWAYS FOR OMBRA

1. **Physical Memory Mapping**: `MapToGuest()` technique perfect for EPT manipulation without driver signatures
2. **Offset Management**: Need similar structure for Windows 10 vs 11 internal offsets
3. **Pattern Scanning**: Template-based approach allows flexible callback handling during scans
4. **Process Manipulation**: `KeStackAttachProcess` pattern for usermode injection from HV context
5. **Cleanup Functions**: Multi-layered artifact removal essential for stealth
6. **MTRR Parsing**: Device memory detection via MSRs, useful for UEFI runtime service location
7. **Safe Iteration**: Always validate before access during physical memory scanning
8. **PE-Aware Scanning**: Section-based scanning reduces false positives and improves performance

---

**Extracted**: 2025-12-20
**Target Project**: Ombra Hypervisor V2
**Next Steps**: Apply physical mapping patterns to EPT manager, implement offset structure for dual Windows version support
