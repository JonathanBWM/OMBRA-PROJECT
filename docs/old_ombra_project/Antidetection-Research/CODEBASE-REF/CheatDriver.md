# CheatDriver Pattern Extraction

**Source**: `Refs/codebases/CheatDriver/`
**Analysis Date**: 2025-12-20
**Pattern Categories**: Memory Access, Signature Scanning, Driver Communication

---

## 1. MEMORY PATTERNS

### 1.1 Process Memory Read/Write via VMCALL

**File**: `CheatDriver/src/comms.cpp:127`

```cpp
// Uses hypervisor VMCALL for cross-CR3 memory writes
ULONG64 cr3 = PsProcessDirBase(Process);
NTSTATUS ntStatus = CPU::CPUIDVmCall(VMCALL_WRITE_VIRT, (ULONG64)writeData, cr3, vmcall::GetCommunicationKey());
```

**Pattern**: Hypervisor-assisted memory operations bypass PatchGuard by executing writes from VMX root mode
- Line 127: CR3 extraction from target process EPROCESS
- Line 127: `VMCALL_WRITE_VIRT` executed with communication key for authentication
- Uses `WRITE_DATA` structure (communication.hpp:222-226) with target pointer, input buffer, length

**File**: `SharedCheatLibrary/SharedCheatLibrary/communication.hpp:216-226`

```cpp
typedef struct _WRITE_DATA {
    PVOID pInBuf;
    PVOID pTarget;
    UINT64 length;
} WRITE_DATA, *PWRITE_DATA;

typedef struct _READ_DATA {
    PVOID pOutBuf;
    PVOID pTarget;
    UINT64 length;
} READ_DATA, *PREAD_DATA;
```

**Pattern**: Separate read/write structures for bidirectional memory access
- `pTarget`: Virtual address in guest address space
- `pInBuf`/`pOutBuf`: Kernel buffer for data transfer
- `length`: Transfer size in bytes

### 1.2 EPT-Based Memory Hiding

**File**: `CheatDriver/src/comms.cpp:595-610`

```cpp
HOOK_SECONDARY_INFO hkSecondaryInfo = { 0 };
PAGE_PERMISSIONS pgPermission = { 0 };
hkSecondaryInfo.pSubstitutePage = pBufferShadow;
pgPermission.Exec = true;

if (!EPT::HookRange(pBuffer, szBuffer, pBufferShadow, hkSecondaryInfo, pgPermission)) {
    DbgMsg("[DRIVER] Failed EPT shadowing injected module: %p - 0x%llx", pBuffer, szBuffer);
    // Cleanup...
    return ntStatus;
}
vTrackedHiddenRanges->emplace_back(pBuffer, szBuffer, pEprocess);
```

**Pattern**: Execute-only page shadowing for code injection hiding
- Line 598: Substitute page allocated for read access (shows clean memory)
- Line 599: Execute permission set on original page
- Line 602: `EPT::HookRange` applies split permissions across multiple pages
- Line 611: Track hidden ranges for automatic cleanup on process exit

### 1.3 Memory Locking to Prevent Paging

**File**: `CheatDriver/src/comms.cpp:583-591`

```cpp
// Lock virtual memory in RAM to prevent page faults during EPT shadowing
adjustStatus = winternl::NtLockVirtualMemory(NtCurrentProcess(), (PVOID*)&pBuffer, (SIZE_T*)&szBuffer, 1);
if (!NT_SUCCESS(adjustStatus)) {
    DbgMsg("[DRIVER] Failed locking injected module at %p for current process: 0x%x", pBuffer, adjustStatus);
    // Rollback allocation...
}
```

**Pattern**: Working set adjustment + memory locking for EPT stability
- Line 568-581: Increase `MaximumWorkingSetSize` to 1GB before locking
- Line 583: `NtLockVirtualMemory` prevents memory from being paged out
- Required for EPT hooks - page faults on shadowed pages cause BSOD

### 1.4 Physical Memory Access via MmMapIoSpace

**File**: `CheatDriver/src/comms.cpp:1085-1098`

```cpp
PHYSICAL_ADDRESS pa = { 0 };
cr3.Flags = *kernelRequest.procInfo.cr3;
pa.QuadPart = (DWORD64)paging::GuestVirtToPhy(pBase, (PVOID)(cr3.AddressOfPageDirectory * PAGE_SIZE));
if (!pa.QuadPart) {
    DbgMsg("[DRIVER] Failed getting page pa for shadowing: %p", pBase);
    return STATUS_FAIL_CHECK;
}
PVOID pTarget = MmMapIoSpace(pa, PAGE_SIZE, MmNonCached);
// ... EPT hook setup ...
MmUnmapIoSpace(pTarget, PAGE_SIZE);
```

**Pattern**: Physical address translation for cross-process EPT hooks
- Line 1087: Virtual-to-physical translation using target CR3
- Line 1093: `MmMapIoSpace` maps physical page into kernel virtual space
- Line 1099: `MmNonCached` prevents CPU caching issues with EPT modifications
- Line 1106: Immediate unmap after EPT hook installation

---

## 2. SIGNATURE PATTERNS

### 2.1 PDB Path Extraction for Process Identification

**File**: `CheatDriver/src/comms.cpp:233-272`

```cpp
PE pe(pBase);
string pdbPath(pe.pdbPath());
if (!pdbPath.Length()) {
    // Fallback to short name from image filename
    string shortName(winternl::GetProcessImageFileName(pEprocess));
    for (auto& blocked : *vBlockedProcesses) {
        if (strstr(blocked.name.c_str(), shortName.to_lower())) {
            // Block/score process
        }
    }
}
pdbPath.to_lower();
if (pdbPath.last_of('\\'))
    pdbPath = pdbPath.substring(pdbPath.last_of('\\'));
```

**Pattern**: PE parsing to extract compile-time PDB path for signature matching
- Line 233: Parse PE headers from process base address
- Line 234: Extract PDB path from debug directory
- Line 236-242: Fallback to image filename if no PDB
- Line 270-272: Normalize path (lowercase, strip directory)
- Used for detection evasion scoring (lines 274-287)

### 2.2 Digital Signature Absence Detection

**File**: `CheatDriver/src/comms.cpp:301-307`

```cpp
if (pe.DataDir(IMAGE_DIRECTORY_ENTRY_SECURITY) == pBase) {
    DbgMsg("[HOOK] Process created without signature in PE: %s", pdbPath.c_str());
    PROC_INFO info;
    info.pEprocess = (DWORD64)pEprocess;
    info.imageBase = (DWORD64)pBase;
    vRestrictedProcesses->Append(info);
}
```

**Pattern**: Flag unsigned executables for restricted access
- Line 301: Check if security data directory points to base (indicates no signature)
- Line 304-306: Track unsigned processes separately from blocked processes
- Used in handle stripping mechanism (not shown in minimal build)

### 2.3 Module Name Scanning in PEB

**File**: `CheatDriver/src/comms.cpp:637-655`

```cpp
PPEB_SKLIB CurrentPEB = (PPEB_SKLIB)PsGetProcessPeb(pEprocess);
PLIST_ENTRY pListEntry = CurrentPEB->Ldr->MemoryOrder.Flink;

for (; pListEntry != &CurrentPEB->Ldr->MemoryOrder;) {
    auto moduleEntry = CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY, LoadOrder);
    string modName(&moduleEntry->ModuleName);

    int strOffset = 0;
    while(pProcInfo->dllsToShadow[strOffset] != 0) {
        char* dll = &pProcInfo->dllsToShadow[strOffset];
        strOffset += strlen(dll) + 1;
        if (modName != dll) {
            continue;
        }
        // Found module to shadow...
    }
}
```

**Pattern**: PEB->Ldr linked list traversal for module enumeration
- Line 637: Access PEB from EPROCESS
- Line 638: Walk `MemoryOrder.Flink` linked list
- Line 644: Extract `LDR_DATA_TABLE_ENTRY` via `CONTAINING_RECORD` macro
- Line 645: Convert UNICODE_STRING to C++ string
- Lines 648-652: Multi-string search (null-terminated array of DLL names)

---

## 3. DRIVER COMMUNICATION

### 3.1 VMCALL-Based Entry Point Registration

**File**: `CheatDriver/src/comms.cpp:781-786`

```cpp
NTSTATUS CommsVmcallHandler(ULONG64& ulOpt1, ULONG64& ulOpt2, ULONG64& ulOpt3) {
    PULONG64 pMem = (PULONG64)paging::vmmhost::MapGuestToHost(vmm::GetGuestCR3().Flags, (PVOID)ulOpt1);
    if(pMem)
        *pMem = (ULONG64)comms::Entry;
    return STATUS_SUCCESS;
}
```

**File**: `CheatDriver/src/comms.cpp:888-890`

```cpp
vmcall::InsertHandler(CommsVmcallHandler, VMCALL_GET_CALLBACK);
vmcall::InsertHandler(CommsHvFlagsVmcallHandler, VMCALL_GET_HV_BUILD_FLAGS);
vmcall::InsertHandler(CommsGetInfoVmcallHandler, VMCALL_GET_INFO);
```

**Pattern**: Hypervisor VMCALL dispatch for kernel callback exposure
- Line 888: Register `VMCALL_GET_CALLBACK` (0x9898) handler
- Line 782: Map guest virtual address to host physical
- Line 784: Write `comms::Entry` function pointer to user-mode buffer
- User-mode calls `comms::Entry` directly via function pointer (no IOCTL)

### 3.2 Instruction ID Dispatch Table

**File**: `CheatDriver/src/comms.cpp:926-1615`
**File**: `SharedCheatLibrary/SharedCheatLibrary/communication.hpp:9-41`

```cpp
// Dispatch structure
NTSTATUS comms::Entry(KERNEL_REQUEST* pKernelRequest) {
    KERNEL_REQUEST kernelRequest = *pKernelRequest;

    switch (kernelRequest.instructionID) {
    case INST_GET_INFO:         // Line 1298
    case INST_SUBSCRIBE_GAME:   // Line 1320
    case INST_IDENTITY_MAP:     // Line 1445
    case INST_SHADOW:           // Line 1031
    case INST_LOCK_MODULE:      // Line 1115
    // ... 30+ instruction codes
    }

    *pKernelRequest = kernelRequest;  // Write back results
    return ntStatus;
}
```

**Pattern**: Single entry point with switch-based instruction dispatch
- Line 940: Validate `KERNEL_REQUEST` pointer
- Line 944: Copy request to stack to prevent TOCTOU attacks
- Line 946: Switch on `instructionID` (enum from communication.hpp:9-41)
- Line 1607: Write modified request back to user-mode buffer
- No IOCTL codes - direct function call from user-mode

### 3.3 Request Structure Layout

**File**: `SharedCheatLibrary/SharedCheatLibrary/communication.hpp:154-180`

```cpp
typedef struct _KERNEL_REQUEST {
    DWORD64 identifier;               // Magic: 0xdeaddeadbeefbeef
    MEMORY_INFO memoryInfo;           // Read/write params
    PROC_INFO procInfo;               // Process subscription data
    COMM_CODE instructionID;          // Dispatch code
    BUGCHECK_INFO_EX bugCheckInfo;    // Custom BSOD data
    SCORE_INFO scoreInfo;             // Detection scoring
    BLOCK_INFO blockInfo;             // Process blocking
    union {
        unsigned long long seed;
        unsigned long long pIdentityMapping;
    };
} KERNEL_REQUEST, *PKERNEL_REQUEST;
```

**Pattern**: Union-based request structure for multiple command types
- Line 156: Magic identifier for validation (IsValid() at line 177-179)
- Line 157-162: Different info structures depending on `instructionID`
- Line 163-167: Union for context-dependent return values
- Shared between kernel and user-mode (no marshalling required)

### 3.4 Process Hooking via EPT Function Hooks

**File**: `CheatDriver/src/comms.cpp:892-920`

```cpp
HOOK_SECONDARY_INFO hkSecondaryInfo = { 0 };
PAGE_PERMISSIONS pgPermissions = { 0 };

hkSecondaryInfo.pOrigFn = (PVOID*)&pPspInsertThreadOrig;
if (!EPT::Hook((PVOID)winternl::PspInsertThread, PspInsertThread, hkSecondaryInfo, pgPermissions)) {
    DbgMsg("[DRIVER] Failed hooking PspInsertThread");
    return false;
}

hkSecondaryInfo.pOrigFn = (PVOID*)&pPspInsertProcessOrig;
if (!EPT::Hook((PVOID)winternl::PspInsertProcess, PspInsertProcess, hkSecondaryInfo, pgPermissions)) {
    DbgMsg("[DRIVER] Failed hooking PspInsertProcess");
    return false;
}

hkSecondaryInfo.pOrigFn = (PVOID*)&pPspRundownSingleProcessOrig;
if (!EPT::Hook((PVOID)winternl::PspRundownSingleProcess, PspRundownSingleProcess, hkSecondaryInfo, pgPermissions)) {
    DbgMsg("[DRIVER] Failed hooking PspRundownSingleProcess");
    return false;
}
```

**Pattern**: Internal kernel function hooking via EPT for process lifecycle monitoring
- Line 895: Store original function pointer in `hkSecondaryInfo.pOrigFn`
- Line 896: `EPT::Hook` installs split-page hook (read shows original, execute jumps to hook)
- Hooks kernel internals: `PspInsertThread`, `PspInsertProcess`, `PspRundownSingleProcess`
- Used for DLL injection timing (line 459-713) and cleanup (line 318-443)

---

## 4. CROSS-REFERENCE PATTERNS FOR OMBRA

### 4.1 Memory Access Strategy

**Comparison**:
- **CheatDriver**: Uses VMCALL for all cross-CR3 reads/writes from kernel callbacks
- **Ombra Equivalent**: Should use `VmcallDispatch.cpp` handlers for memory operations
- **Implementation**: Add `VMCALL_READ_VIRT`/`VMCALL_WRITE_VIRT` to `OmbraPayload/Vmcall/VmcallDispatch.cpp`

**Code Location**: `communication.hpp:200-201`
```cpp
VMCALL_READ_VIRT,
VMCALL_WRITE_VIRT,
```

**Required for Ombra**:
- EPT walker for virtual-to-physical translation (currently missing)
- Guest physical memory mapping in VMX root mode
- CR3 parameter passing to identify target address space

### 4.2 EPT Hooking Architecture

**Comparison**:
- **CheatDriver**: `EPT::Hook()` and `EPT::HookRange()` with substitute pages
- **Ombra**: Has basic EPT setup but no hooking mechanism yet
- **Gap**: Need execute-only page implementation + substitute page management

**Key Pattern from CheatDriver**:
```cpp
// comms.cpp:1094-1098
HOOK_SECONDARY_INFO hkSecondaryInfo = { 0 };
PAGE_PERMISSIONS pgPermissions = { 0 };
pgPermissions.Exec = true;
hkSecondaryInfo.pSubstitutePage = pSubstitute;
EPT::Hook(pTarget, (PVOID)pa.QuadPart, hkSecondaryInfo, pgPermissions);
```

**Ombra Needs**:
- Split EPT permissions (RWX separate for read vs execute)
- Substitute page tracking structure
- EPT violation handler to swap pages on access type

### 4.3 Process Tracking via Internal Hooks

**CheatDriver Pattern**:
- Hooks `PspInsertProcess` to catch process creation
- Hooks `PspInsertThread` to inject DLLs at first thread
- Hooks `PspRundownSingleProcess` for cleanup

**Ombra Equivalent**:
- Hook UEFI boot services (already in `OmbraBoot/Hooks/`)
- Need kernel callback registration for process monitoring
- Alternative: `PsSetCreateProcessNotifyRoutineEx` (less stealthy)

**Advantage of EPT Hooks**:
- No callback structure registration (invisible to scanners)
- No kernel structure modification
- Survives PatchGuard scans

---

## 5. KEY TAKEAWAYS FOR OMBRA

### Memory Operations
1. **VMCALL-based R/W**: CheatDriver never uses `MmCopyVirtualMemory` - always VMCALL from VMX root
2. **Page locking**: EPT hooks require `NtLockVirtualMemory` + working set adjustment
3. **Physical mapping**: `MmMapIoSpace` for cross-CR3 page access during EPT setup

### Communication
1. **No IOCTL**: Function pointer exposure via VMCALL (`VMCALL_GET_CALLBACK`)
2. **Magic validation**: `0xdeaddeadbeefbeef` identifier in request structure
3. **Single dispatch**: One entry function + instruction ID enum

### Detection Evasion
1. **PDB scanning**: Extract compile-time paths for more reliable process ID
2. **Signature checking**: Flag unsigned processes for restricted access
3. **Score-based blocking**: Accumulate detection risk instead of hard blocks

### EPT Patterns
1. **Execute-only shadowing**: Read returns clean, execute runs hooks
2. **Range tracking**: Vector of `RANGE_INFO` for automatic cleanup
3. **Substitute pages**: Pre-allocated clean pages for read access

---

**Files Referenced**:
- `CheatDriver/src/comms.cpp` - Main dispatch and memory operations
- `CheatDriver/include/comms.h` - Structure definitions
- `CheatDriver/main.cpp` - Driver entry and initialization
- `SharedCheatLibrary/SharedCheatLibrary/communication.hpp` - Shared structures
- `CheatDriver/Calls.md` - API documentation

**Total Lines Analyzed**: ~1,615 (comms.cpp) + 287 (communication.hpp) + 108 (main.cpp)
