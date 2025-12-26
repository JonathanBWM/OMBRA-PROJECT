# Kernel-Bridge Pattern Extraction

**Source**: `Refs/codebases/Kernel-Bridge/`
**Date**: 2025-12-20
**Purpose**: Comprehensive kernel memory access framework patterns

---

## 1. MEMORY READ/WRITE

### Physical Memory Access

**Direct Physical Read/Write**
- `Kernel-Bridge/API/MemoryUtils.cpp:389-416` - ReadPhysicalMemory
- `Kernel-Bridge/API/MemoryUtils.cpp:419-446` - WritePhysicalMemory
- Pattern: Try `GetVirtualForPhysical()` first, fall back to `MapPhysicalMemory()` on failure
- Exception handling with `__try/__except` wraps all physical operations
- Cache type parameter: `MmNonCached`, `MmCached`, `MmWriteCombined`

**Physical Address Translation**
- `Kernel-Bridge/API/MemoryUtils.cpp:368-382` - GetPhysicalAddress
- Uses `MmGetPhysicalAddress()` after validating with `MmIsAddressValid()`
- Process context switch via `KeStackAttachProcess/KeUnstackDetachProcess`
- Returns NULL if address invalid, avoiding exceptions

**Physical Memory Mapping**
- `Kernel-Bridge/API/MemoryUtils.cpp:358-365` - MapPhysicalMemory/UnmapPhysicalMemory
- `MmMapIoSpace()` for mapping physical ranges to virtual
- `MmUnmapIoSpace()` for cleanup
- Specify caching type explicitly

**DMI Memory Reading**
- `Kernel-Bridge/API/MemoryUtils.cpp:315-325` - ReadDmiMemory
- Fixed address `0xF0000`, size 65536 bytes
- Maps with `MmNonCached` attribute
- Pattern for reading BIOS region

### Virtual Memory Access

**Pool Allocation**
- `Kernel-Bridge/API/MemoryUtils.cpp:21-33` - AllocFromPool
- Uses `ExDefaultNonPagedPoolType` (requires `POOL_NX_OPTIN=1` definition)
- Pool tag `'KBLI'` for tracking
- Zero-fills on request, always touches first and last byte
- Executable pool: `NonPagedPoolExecute` type (line 36-43)

**Memory Copy with Validation**
- `Kernel-Bridge/API/MemoryUtils.cpp:75-117` - CopyMemory
- Size-optimized switch for 1/2/4/8 byte copies (direct assignment)
- `RtlMoveMemory()` for overlapping ranges, `RtlCopyMemory()` otherwise
- Optional presence check via `IsMemoryRangePresent()`
- Checks kernel addresses for page presence before copy

**Memory Presence Checking**
- `Kernel-Bridge/API/MemoryUtils.cpp:184-191` - IsMemoryRangePresent
- Page-aligned iteration, checks each page with `IsPagePresent()`
- `IsPagePresent()` uses `GetPhysicalAddress() || MmIsAddressValid()`
- Critical for avoiding page faults in arbitrary memory access

**User Memory Validation**
- `Kernel-Bridge/API/MemoryUtils.cpp:194-201` - CheckUserMemoryReadable
- Uses `ProbeForRead()` in `__try/__except` block
- `Kernel-Bridge/API/MemoryUtils.cpp:213-220` - CheckUserMemoryWriteable
- Uses `ProbeForWrite()` in `__try/__except` block
- Process-aware variants attach to target process first (lines 204-229)

**Memory Securing (Locking)**
- `Kernel-Bridge/API/MemoryUtils.cpp:120-130` - SecureMemory
- `MmSecureVirtualMemory()` prevents user memory from being freed
- Returns HANDLE to secured region, must call `MmUnsecureVirtualMemory()`
- Process variant (lines 133-152) uses `KeStackAttachProcess` for cross-process

### MDL-Based Mapping

**MDL Allocation and Locking**
- `Kernel-Bridge/API/MemoryUtils.cpp:451-471` - AllocMdlAndLockPages
- `IoAllocateMdl()` for MDL creation
- `MmProbeAndLockPages()` for current process
- `MmProbeAndLockProcessPages()` for target process
- Exception-safe: frees MDL on probe/lock failure

**MDL Mapping to Address Space**
- `Kernel-Bridge/API/MemoryUtils.cpp:481-543` - MapMdl
- `MmMapLockedPagesSpecifyCache()` for mapping MDL
- `MmProtectMdlSystemAddress()` sets protection (PAGE_READWRITE, PAGE_READONLY, etc.)
- Supports KernelMode or UserMode mapping
- `UserRequestedAddress` for fixed-address user mappings
- Process context switching via `KeStackAttachProcess` if `DestProcess` specified

**Complete Memory Mapping Flow**
- `Kernel-Bridge/API/MemoryUtils.cpp:552-590` - MapMemory
- Allocates MDL, probes/locks pages, maps to address space in one call
- Returns `MAPPING_INFO` struct with MDL and BaseAddress
- `UnmapMemory()` (lines 593-597) reverses: unmap, unlock, free MDL

**MDL Cleanup**
- `Kernel-Bridge/API/MemoryUtils.cpp:474-478` - UnlockPagesAndFreeMdl
- `MmUnlockPages()` then `IoFreeMdl()`
- Must be called in reverse order of allocation

---

## 2. PROCESS MANIPULATION

### EPROCESS Traversal

**Process Lookup**
- `Kernel-Bridge/API/ProcessesUtils.cpp:10-15` - GetEPROCESS
- `PsLookupProcessByProcessId()` returns referenced EPROCESS
- **Must dereference** with `ObDereferenceObject()` after use
- Returns NULL on failure

**Thread Lookup**
- `Kernel-Bridge/API/ProcessesUtils.cpp:18-23` - GetETHREAD
- `PsLookupThreadByThreadId()` returns referenced ETHREAD
- **Must dereference** with `ObDereferenceObject()` after use

**Thread to Process Conversion**
- `ProcessesUtils.h:50` - ThreadToProcess
- `IoThreadToProcess()` - does NOT require dereference (inline helper)

### Process Handle Opening

**OpenProcess by PID**
- `Kernel-Bridge/API/ProcessesUtils.cpp:26-40` - OpenProcess
- Builds `CLIENT_ID` with ProcessId
- `InitializeObjectAttributes()` with attributes (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE)
- `ZwOpenProcess()` returns handle, **must close** with `ZwClose()`

**OpenProcess by Pointer**
- `Kernel-Bridge/API/ProcessesUtils.cpp:43-59` - OpenProcessByPointer
- `ObOpenObjectByPointer()` on EPROCESS
- Uses `*PsProcessType` for object type
- Allows specifying `KernelMode` or `UserMode` processor mode

**OpenThread Patterns**
- `Kernel-Bridge/API/ProcessesUtils.cpp:62-86` - OpenThread (by ThreadId)
- Dynamically resolves `ZwOpenThread` via `Importer::GetKernelProcAddress()`
- `Kernel-Bridge/API/ProcessesUtils.cpp:89-105` - OpenThreadByPointer
- Uses `*PsThreadType` with `ObOpenObjectByPointer()`

### Address Space Switching

**Attach to Process**
- `Kernel-Bridge/API/ProcessesUtils.cpp:110-124` - AttachToProcess/AttachToProcessByPid
- `KeStackAttachProcess()` switches address space
- **Critical**: Own usermode memory/handles become invalid in target context
- Must call `KeUnstackDetachProcess()` to restore (line 127-129)

**Pattern**:
```c
KAPC_STATE ApcState;
KeStackAttachProcess(TargetProcess, &ApcState);
// ... access target process usermode memory ...
KeUnstackDetachProcess(&ApcState);
```

### Process Termination

**Terminate by PID**
- `Kernel-Bridge/API/ProcessesUtils.cpp:134-142` - TerminateProcessByPid
- Opens process, calls `ZwTerminateProcess()`, closes handle
- Returns `NTSTATUS` for error handling

**Terminate by Handle**
- `Kernel-Bridge/API/ProcessesUtils.cpp:145-147` - TerminateProcess
- Direct `ZwTerminateProcess()` call

### Thread Manipulation

**Context Get/Set**
- `Kernel-Bridge/API/ProcessesUtils.cpp:152-163` - GetContextThread
- Resolves `PsGetContextThread` dynamically
- `Kernel-Bridge/API/ProcessesUtils.cpp:166-177` - SetContextThread
- Resolves `PsSetContextThread` dynamically
- Both require `PETHREAD`, `PCONTEXT`, `KPROCESSOR_MODE`

**Suspend/Resume**
- `Kernel-Bridge/API/ProcessesUtils.cpp:180-189` - SuspendProcess
- Resolves `PsSuspendProcess` dynamically
- `Kernel-Bridge/API/ProcessesUtils.cpp:192-201` - ResumeProcess
- Resolves `PsResumeProcess` dynamically

**Thread Creation**
- `Kernel-Bridge/API/ProcessesUtils.cpp:204-240` - CreateUserThread
- Resolves `RtlCreateUserThread` dynamically
- Parameters: hProcess, StartAddress, Argument, CreateSuspended
- Returns hThread and ClientId

- `Kernel-Bridge/API/ProcessesUtils.cpp:243-253` - CreateSystemThread
- `PsCreateSystemThread()` with OBJ_KERNEL_HANDLE attributes
- Can attach to specific process or NULL for system process

### Process Memory Operations

**Read Process Memory**
- `Kernel-Bridge/API/ProcessesUtils.cpp:499-506` - ReadProcessMemory
- Uses `OperateProcessMemory()` helper (lines 406-496)
- Secures both process and buffer memory with `MmSecureVirtualMemory()`
- Maps both via MDL for safe copy
- Exception-safe with `__try/__finally` cleanup

**Write Process Memory**
- `Kernel-Bridge/API/ProcessesUtils.cpp:509-516` - WriteProcessMemory
- Same pattern as read, opposite direction
- Validates kernel address presence, secures user addresses
- MDL mapping for both source and destination

**Memory Securing Pattern** (lines 428-442):
1. Check if address range present
2. Secure process memory (`MmSecureVirtualMemory`)
3. Secure buffer memory if usermode
4. Map process memory via MDL
5. Map buffer memory via MDL
6. Perform copy in `__try` block
7. Cleanup in `__finally`: unmap, unsecure

### APC Queueing

**User APC Injection**
- `Kernel-Bridge/API/ProcessesUtils.cpp:521-599` - QueueUserApc
- Resolves `KeInitializeApc`, `KeInsertQueueApc` dynamically
- Allocates `KAPC` from pool
- User APC: `OriginalApcEnvironment`, `UserMode`, calls `NormalRoutine`
- Kernel APC: Forces delivery via `KeTestAlertThread(UserMode)`
- Handles Wow64 with `PsWrapApcWow64Thread()` on x64 systems
- Frees APC structures in kernel routines after delivery

**APC Pattern**:
1. Allocate user KAPC
2. Initialize with kernel routine that handles Wow64 wrapping
3. Allocate kernel KAPC (delivery enforcer)
4. Initialize with routine that calls `KeTestAlertThread`
5. Insert user APC first, then kernel APC
6. Kernel routines free their own KAPC allocations

---

## 3. DRIVER COMMUNICATION

### IOCTL Interface Design

**IOCTL Code Definition**
- `Kernel-Bridge/Kernel-Bridge/IOCTLs.h:3-6` - IOCTL macro
```c
#define IOCTL(Code, Method) (CTL_CODE(0x8000, (Code), Method, FILE_ANY_ACCESS))
#define EXTRACT_CTL_CODE(Ioctl)   ((unsigned short)(((Ioctl) & 0b0011111111111100) >> 2))
#define EXTRACT_CTL_METHOD(Ioctl) ((unsigned short)((Ioctl) & 0b11))
#define CTL_BASE (0x800)
```
- Custom device type `0x8000`
- Supports METHOD_BUFFERED, METHOD_IN_DIRECT, METHOD_OUT_DIRECT, METHOD_NEITHER

**IOCTL Handler Pattern**
- `Kernel-Bridge/Kernel-Bridge/IOCTLHandlers.cpp:36-45` - Example handler signature
```c
NTSTATUS FASTCALL KbGetDriverApiVersion(
    IN PIOCTL_INFO RequestInfo,
    OUT PSIZE_T ResponseLength
)
```
- `IOCTL_INFO` contains: InputBuffer, OutputBuffer, InputBufferSize, OutputBufferSize
- Handlers validate buffer sizes, return `STATUS_INFO_LENGTH_MISMATCH` on mismatch
- Set `*ResponseLength` to actual bytes returned

**Size Validation Pattern** (repeated throughout handlers):
```c
if (RequestInfo->InputBufferSize != sizeof(KB_XXX_IN))
    return STATUS_INFO_LENGTH_MISMATCH;
if (RequestInfo->OutputBufferSize != sizeof(KB_XXX_OUT))
    return STATUS_INFO_LENGTH_MISMATCH;
```

**Buffer Null Check**:
```c
if (!RequestInfo->InputBuffer || !RequestInfo->OutputBuffer)
    return STATUS_INVALID_PARAMETER;
```

### Request/Response Protocol

**Input/Output Structure Pattern**
- Structures named `KB_<OPERATION>_IN` and `KB_<OPERATION>_OUT`
- Cast buffers to typed pointers for access
- Example (lines 118-147):
```c
auto Input = static_cast<PKB_READ_PORT_IN>(RequestInfo->InputBuffer);
auto Output = static_cast<PKB_READ_PORT_OUT>(RequestInfo->OutputBuffer);
```

**Version Check Handler**
- `IOCTLHandlers.cpp:36-45` - Returns `KB_API_VERSION` constant
- Allows user-mode to validate driver compatibility

**Handle Count Tracking**
- `IOCTLHandlers.cpp:47-56` - Returns volatile global `KbHandlesCount`
- Driver maintains reference count of open handles

**Granularity-Based Operations** (port read/write):
- Input specifies granularity: `sizeof(UCHAR)`, `sizeof(USHORT)`, `sizeof(ULONG)`
- Switch on granularity to call appropriate function (BYTE/WORD/DWORD variants)
- Example: `IOCTLHandlers.cpp:125-143`

### Shared Memory Patterns

**MDL-Based Shared Memory**
- User allocates buffer, driver maps via MDL
- `MapMemory()` allows mapping to user or kernel space
- `UserRequestedAddress` parameter for fixed user mappings
- Driver can share read-only or read-write

**Direct Buffer Access (METHOD_BUFFERED)**
- System copies user buffers to/from kernel automatically
- Handlers access via `RequestInfo->InputBuffer/OutputBuffer`
- Safe but slower for large transfers

**Direct I/O (METHOD_IN_DIRECT/METHOD_OUT_DIRECT)**
- System locks user pages, provides MDL
- Driver accesses via `MmGetSystemAddressForMdlSafe()`
- Faster for bulk transfers, user buffer must remain valid

---

## 4. PE UTILITIES

### PE Header Parsing

**PE Validation**
- `User-Bridge/API/PEUtils/PEAnalyzer.cpp:7-11` - Signature constants
```c
constexpr unsigned short MZ_SIGNATURE = 0x5A4D; // MZ
constexpr unsigned short PE_SIGNATURE = 0x4550; // PE
constexpr unsigned short PE32_SIGNATURE = 0x010B;
constexpr unsigned short PE64_SIGNATURE = 0x020B;
```

**Header Extraction**
- `PEAnalyzer.h:87-89` - Header pointers
```c
PIMAGE_DOS_HEADER m_dosHeader;
PIMAGE_NT_HEADERS m_ntHeaders;
PIMAGE_OPTIONAL_HEADER m_optionalHeader;
```
- Validate DOS signature at offset 0 (`e_magic`)
- NT headers at `dosHeader + dosHeader->e_lfanew`
- Validate PE signature at NT headers

**Image Size Calculation**
- `PEAnalyzer.h:91` - `m_imageSize` from `OptionalHeader.SizeOfImage`
- `m_imageBase` from `OptionalHeader.ImageBase`
- `m_entryPoint` = ImageBase + `OptionalHeader.AddressOfEntryPoint`

### Section Enumeration

**Section Iteration**
- `PEAnalyzer.cpp:60-78` - fillSectionsInfo
- `IMAGE_FIRST_SECTION(ntHeaders)` gets first section header
- Iterate `FileHeader.NumberOfSections` times
- Extract per section:
  - `VirtualAddress` (RVA in memory)
  - `PointerToRawData` (offset in file)
  - `Misc.VirtualSize` (size in memory)
  - `SizeOfRawData` (size on disk)
  - `Characteristics` (permissions)
  - `Name` (8 bytes, null-terminate to 9)

**RVA to File Offset Conversion**
- `PEAnalyzer.cpp:23-58` - rvaToOffset
- Find section containing RVA
- Formula: `SectionFileOffset + (RVA - SectionRVA)`
- Handles alignment: file vs section alignment
- Returns 0 if RVA not in any section

**Alignment Helpers**
- `PEAnalyzer.cpp:13-21` - alignDown/alignUp
```c
alignDown(value, factor) = value & ~(factor - 1)
alignUp(value, factor)   = alignDown(value - 1, factor) + factor
```

### Export/Import Resolution

**Export Directory Parsing**
- `PEAnalyzer.cpp:193-243` - fillExportsInfo
- `DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]` gets export directory RVA
- `IMAGE_EXPORT_DIRECTORY` structure at RVA
- Arrays to process:
  - `AddressOfFunctions` - function RVAs (indexed by ordinal - base)
  - `AddressOfNames` - name RVAs (indexed by name number)
  - `AddressOfNameOrdinals` - ordinals (indexed by name number)

**Export Enumeration Pattern**:
1. Build ordinal-to-name map from `AddressOfNameOrdinals`
2. Iterate `NumberOfFunctions` in `AddressOfFunctions`
3. For each: check if ordinal in map (named) or ordinal-only export
4. Store: VA, RVA, ordinal, name (if present)

**Import Directory Parsing**
- `PEAnalyzer.cpp:144-165` - fillImportsInfo
- `DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]` gets import directory RVA
- Array of `IMAGE_IMPORT_DESCRIPTOR`, iterate until `FirstThunk == 0`
- Per DLL:
  - Name at `imports->Name` RVA
  - `OriginalFirstThunk` - array of `IMAGE_THUNK_DATA` (import names/ordinals)
  - `FirstThunk` - IAT addresses (patched by loader)

**Import Entry Decoding**
- `PEAnalyzer.cpp:108-142` - fillImportsSet
- High bit of thunk determines ordinal vs name import
- Ordinal mask: `0x7FFFFFFF` (32-bit) or `0x7FFFFFFFFFFFFFFF` (64-bit)
- Named import: `IMAGE_IMPORT_BY_NAME` at thunk RVA (Hint + Name)

**Delayed Import Parsing**
- `PEAnalyzer.cpp:167-191` - fillDelayedImportsInfo
- `DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT]`
- `IMAGE_DELAYLOAD_DESCRIPTOR` array
- Fields: `DllNameRVA`, `ModuleHandleRVA`, `ImportAddressTableRVA`, `ImportNameTableRVA`
- Same thunk parsing as standard imports

### Relocation Parsing

**Relocation Directory**
- `PEAnalyzer.cpp:80-106` - fillRelocsInfo
- `DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC]`
- Array of `IMAGE_BASE_RELOCATION` blocks
- Each block: `VirtualAddress` (page RVA), `SizeOfBlock`
- Entries: WORD array, high 4 bits = type, low 12 bits = offset in page

**Relocation Entry Decoding**:
```c
DWORD pageRva = relocs->VirtualAddress;
DWORD count = (relocs->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
PWORD entry = (PWORD)((PBYTE)relocs + sizeof(IMAGE_BASE_RELOCATION));
for (i = 0; i < count; i++) {
    BYTE type = entry[i] >> 12;
    WORD offset = entry[i] & 0x0FFF;
    DWORD rva = pageRva + offset;
}
```

**Relocation Types**:
- `IMAGE_REL_BASED_ABSOLUTE` (0) - skip
- `IMAGE_REL_BASED_HIGHLOW` (3) - 32-bit address
- `IMAGE_REL_BASED_DIR64` (10) - 64-bit address

---

## 5. WINDOWS VERSION HANDLING

### Build Number Detection

**Version Initialization**
- `Kernel-Bridge/API/OSVersion.cpp:8-12` - Initialize
- `PsGetVersion(&Major, &Minor, NULL, NULL)` gets OS version
- Static members cache result: `_OSVersion::Major`, `_OSVersion::Minor`
- `Initialized` flag prevents re-query

**Version Comparison**
- `OSVersion.cpp:14-17` - IsGreaterThan
- Compares: `(Major > X) || (Major == X && Minor >= Y)`
- All version checks use this helper

### Windows Version Checks

**Predefined Checks**:
- `OSVersion.cpp:19-21` - IsWindowsXPOrGreater: `(5, 1)`
- `OSVersion.cpp:23-25` - IsWindowsXP64OrGreater: `(5, 2)`
- `OSVersion.cpp:27-29` - IsWindowsVistaOrGreater: `(6, 0)`
- `OSVersion.cpp:31-33` - IsWindows7OrGreater: `(6, 1)`
- `OSVersion.cpp:35-37` - IsWindows8OrGreater: `(6, 2)`
- `OSVersion.cpp:39-41` - IsWindows81OrGreater: `(6, 3)`
- `OSVersion.cpp:43-45` - IsWindows10OrGreater: `(10, 0)`

**Version Table**:
| OS | Major | Minor |
|----|-------|-------|
| Windows XP | 5 | 1 |
| Windows XP x64 / Server 2003 | 5 | 2 |
| Windows Vista / Server 2008 | 6 | 0 |
| Windows 7 / Server 2008 R2 | 6 | 1 |
| Windows 8 / Server 2012 | 6 | 2 |
| Windows 8.1 / Server 2012 R2 | 6 | 3 |
| Windows 10 / Server 2016+ | 10 | 0 |

### Offset Tables per Version

**Pattern for Version-Specific Offsets**:
```c
struct EPROCESS_OFFSETS {
    ULONG UniqueProcessId;
    ULONG ActiveProcessLinks;
    ULONG ImageFileName;
    // ... other fields
};

static EPROCESS_OFFSETS GetEPROCESSOffsets() {
    if (OSVersion::IsWindows10OrGreater()) {
        return { 0x440, 0x448, 0x5A8 }; // Win10 offsets
    } else if (OSVersion::IsWindows81OrGreater()) {
        return { 0x2E0, 0x2E8, 0x438 }; // Win8.1 offsets
    } else if (OSVersion::IsWindows7OrGreater()) {
        return { 0x180, 0x188, 0x2E0 }; // Win7 offsets
    }
    // ... fallback
}
```

**Dynamic Function Resolution**:
- Used for undocumented functions that may not export on all versions
- Example: `Importer::GetKernelProcAddress(L"PsSuspendProcess")`
- Check for NULL before calling
- Return `STATUS_NOT_IMPLEMENTED` if unavailable

### 32-bit vs 64-bit Handling

**Architecture Detection**:
- Compile-time: `#ifdef _AMD64_` for 64-bit specific code
- Runtime: `ProcessesUtils.cpp:365-376` - Is32BitProcess
  - On x64: Query `ProcessWow64Information` to check for WOW64
  - On x86: Always returns TRUE

**PTE Structure Differences**:
- `PteUtils.cpp:35-69` - x64 paging (PML4 -> PDPE -> PDE -> PTE)
- `PteUtils.cpp:71-136` - x32 paging (PAE vs non-PAE)
- CR4.PAE bit determines 32-bit paging mode
- Separate structures for x32/x64: `PML4E::x64`, `PDE::x32`, etc.

**Intrinsic Differences**:
- x64: `__readcr3()` returns `unsigned long long`
- x32: `__readcr3()` returns `unsigned long`
- Similar for `__readcr4()`, handled via `#ifdef _AMD64_`

---

## Key Patterns Summary

### Memory Access Safety
1. Always validate address presence before access
2. Use `__try/__except` for arbitrary memory operations
3. Secure user memory before kernel access
4. Use MDL mapping for cross-process copies
5. Attach to target process for usermode memory access

### Process Context
1. `KeStackAttachProcess()` for address space switch
2. KAPC_STATE must be preserved and restored
3. Own usermode memory invalid in target context
4. Reference objects obtained via Ps/Ob lookup functions
5. Dereference with `ObDereferenceObject()` when done

### Driver Communication
1. Validate input/output buffer sizes before access
2. Return correct NTSTATUS codes for error conditions
3. Set `*ResponseLength` to actual bytes written
4. Use typed structures for IOCTL data
5. Support granularity-based operations where applicable

### PE Parsing
1. Validate signatures (MZ, PE) before parsing
2. Convert RVAs to file offsets for raw PE
3. Align values per file/section alignment
4. Build lookup maps for export ordinal-to-name resolution
5. Handle both ordinal and named imports

### Version Compatibility
1. Cache version check results in static variables
2. Use comparison helpers for version ranges
3. Dynamically resolve undocumented functions
4. Maintain offset tables per major OS version
5. Compile-time separation for x86/x64 differences

---

**Pattern Quality**: Production-grade, exception-safe, version-aware
**Suitability for Ombra**: High - demonstrates kernel memory manipulation fundamentals
**Security Considerations**: All techniques are dual-use; require kernel-mode execution

**Extracted by**: ENI (Hypervisor Research Agent)
**For**: Ombra Hypervisor V2 Development
