# Phase 3: OmbraDriver Kernel Component Design

## Overview

OmbraDriver is a manually-mapped kernel driver that bridges Ring 3 (usermode) and Ring -1 (hypervisor). It provides process tracking, memory operations, EPT-based hiding, and anti-cheat evasion features.

**Goal:** Establish bidirectional communication between usermode applications and the hypervisor, enabling process monitoring, memory manipulation, and stealth operations.

**Critical Constraint:** No device objects, no IOCTL handlers, no registry presence. Communication via shared memory only.

**Blocking Prerequisite:** Phase 2 BigPool visibility test must complete before implementation. Results determine memory allocation strategy.

---

## Architecture Decision: Shared Memory Communication

### Why Shared Memory

| Approach | Detection Surface | Verdict |
|----------|------------------|---------|
| Device Object | Enumerable via IoGetDeviceObjectPointer, NtQueryDirectoryObject | Rejected |
| APC Queue | Can't queue kernel APCs from usermode | Impractical |
| Shared Memory | No kernel objects, no handles, anonymous pages | Selected |

### Memory Layout

Phase 2 allocates a contiguous pool via SUPDrv's `PageAllocEx`, providing dual R3/R0 mappings:

```
+0x00000000: Driver Image
             (size: variable, typically 64-256KB)
             (page-aligned end)

+DriverEnd:  Command Ring Header (192 bytes, cache-aligned)

+Header+192: Commands[64] (64 × 512 = 32,768 bytes)

+Header+33K: Responses[64] (64 × 256 = 16,384 bytes)

+Header+49K: Padding to page boundary

+RingEnd:    Scratch Buffer (131,072 bytes = 128KB)

Total: ~DriverSize + 180KB
```

### Signaling Mechanism

**Polling-based:** Driver's worker thread checks for pending commands every 1ms. Usermode polls for response completion after submitting commands.

**Rationale:** Zero detection surface. No kernel events, no handles, no synchronization objects. CPU cost is negligible on modern hardware.

---

## Data Structures

### Command Ring Header

Cache-line aligned to prevent false sharing between producer (usermode) and consumer (driver):

```c
typedef struct _OMBRA_COMMAND_RING {
    // Producer side (usermode writes) - cache line 0
    volatile uint32_t   ProducerIndex;
    uint8_t             _pad0[60];

    // Consumer side (driver writes) - cache line 1
    volatile uint32_t   ConsumerIndex;
    uint8_t             _pad1[60];

    // Read-mostly fields - cache line 2
    uint32_t            RingSize;
    uint32_t            CommandSize;
    uint32_t            ResponseSize;
    uint64_t            Magic;
    uint64_t            ScratchBufferOffset;
    uint64_t            ScratchBufferSize;
    uint8_t             _pad2[24];

    // Commands[RingSize] follows at offset 192
    // Responses[RingSize] follows commands
} OMBRA_COMMAND_RING;
```

### Command Structure

```c
typedef struct _OMBRA_COMMAND {
    uint32_t    CommandId;          // OMBRA_CMD_*
    uint32_t    Flags;              // OMBRA_FLAG_*
    uint64_t    SequenceId;         // For matching responses

    union {
        // Process operations
        struct {
            uint64_t    Cr3;
            uint64_t    Pid;
            char        ImageName[64];
        } Process;

        // Memory operations
        struct {
            uint64_t    Cr3;
            uint64_t    Address;
            uint64_t    Size;
            uint32_t    Protection;
            char        ModuleName[64];
        } Memory;

        // Physical memory access
        struct {
            uint64_t    PhysicalAddress;
            uint64_t    Size;
            uint64_t    ScratchOffset;
        } PhysicalMem;

        // Virtual memory access
        struct {
            uint64_t    Cr3;
            uint64_t    VirtualAddress;
            uint64_t    Size;
            uint64_t    ScratchOffset;
        } VirtualMem;

        // Injection operations
        struct {
            uint64_t    Pid;
            uint64_t    ScratchOffset;
            uint64_t    ImageSize;
            uint32_t    Flags;
        } Inject;

        // Window operations
        struct {
            uint64_t    Hwnd;
            char        ProcessName[64];
        } Window;

        // Enumeration
        struct {
            uint32_t    Flags;
            uint64_t    ScratchOffset;
            uint64_t    ScratchSize;
        } Enumerate;

        // ETW operations
        struct {
            uint32_t    Operation;
            uint32_t    ProviderId;
            uint64_t    BufferAddress;
        } Etw;

        // Spoofing
        struct {
            uint32_t    SpoofType;
            uint8_t     OriginalValue[64];
            uint8_t     SpoofedValue[64];
            uint64_t    TargetDriver;
        } Spoof;

        // Protection
        struct {
            uint64_t    Cr3;
            uint32_t    Method;
            uint32_t    AccessMask;
        } Protection;

        // Score configuration
        struct {
            uint32_t    Operation;
            uint32_t    EventMask;
            uint32_t    Threshold;
        } Score;

        uint8_t     Raw[448];
    };
} OMBRA_COMMAND;  // 512 bytes total
```

### Response Structure

```c
typedef struct _OMBRA_RESPONSE {
    volatile uint32_t   Ready;
    int32_t             Status;
    uint64_t            SequenceId;

    union {
        // Process info
        struct {
            uint64_t    Cr3;
            uint64_t    Peb;
            uint64_t    Eprocess;
            uint64_t    ImageBase;
            uint32_t    Pid;
            uint8_t     Dead;
            uint8_t     DllInjected;
        } ProcessInfo;

        // Memory result
        struct {
            uint64_t    MappedBase;
            uint64_t    MappedSize;
            uint64_t    BytesTransferred;
        } MemoryResult;

        // Identity map
        struct {
            uint64_t    IdentityBase;
        } IdentityMap;

        // Status info
        struct {
            uint32_t    DriverVersion;
            uint64_t    Uptime;
            uint64_t    CommandsProcessed;
            uint64_t    VmexitCount;
            uint32_t    ActiveCpus;
            uint32_t    HooksInstalled;
        } StatusInfo;

        uint8_t     Raw[224];
    };
} OMBRA_RESPONSE;  // 256 bytes total
```

### Command Enumeration

```c
typedef enum _OMBRA_CMD {
    // Process Tracking (0x01xx)
    OMBRA_CMD_SUBSCRIBE           = 0x0100,
    OMBRA_CMD_UNSUBSCRIBE         = 0x0101,
    OMBRA_CMD_GET_INFO            = 0x0102,
    OMBRA_CMD_RESET_INFO          = 0x0103,
    OMBRA_CMD_ENUM_PROCESSES      = 0x0104,
    OMBRA_CMD_ENUM_MODULES        = 0x0105,

    // Memory Operations (0x02xx)
    OMBRA_CMD_HIDE_MEMORY         = 0x0200,
    OMBRA_CMD_SHADOW_MEMORY       = 0x0201,
    OMBRA_CMD_LOCK_MODULE         = 0x0202,
    OMBRA_CMD_UNLOCK_MODULE       = 0x0203,
    OMBRA_CMD_READ_PHYSICAL       = 0x0204,
    OMBRA_CMD_WRITE_PHYSICAL      = 0x0205,
    OMBRA_CMD_READ_VIRTUAL        = 0x0206,
    OMBRA_CMD_WRITE_VIRTUAL       = 0x0207,

    // Injection (0x03xx)
    OMBRA_CMD_INJECT              = 0x0300,
    OMBRA_CMD_INJECT_HIDDEN       = 0x0301,

    // Window Hiding (0x04xx)
    OMBRA_CMD_SET_OVERLAY         = 0x0400,
    OMBRA_CMD_GET_OVERLAY         = 0x0401,
    OMBRA_CMD_SET_DEFAULT_OVERLAY = 0x0402,

    // Protection (0x05xx)
    OMBRA_CMD_PROTECT_PROCESS     = 0x0500,
    OMBRA_CMD_UNPROTECT_PROCESS   = 0x0501,
    OMBRA_CMD_BLOCK_IMAGE         = 0x0502,
    OMBRA_CMD_UNBLOCK_IMAGE       = 0x0503,

    // Identity Map (0x06xx)
    OMBRA_CMD_GET_IDENTITY_MAP    = 0x0600,
    OMBRA_CMD_RELEASE_IDENTITY_MAP = 0x0601,

    // Spoofing (0x07xx)
    OMBRA_CMD_SPOOF_DISK          = 0x0700,
    OMBRA_CMD_SPOOF_NIC           = 0x0701,
    OMBRA_CMD_SPOOF_VOLUME        = 0x0702,
    OMBRA_CMD_SPOOF_QUERY         = 0x0703,

    // ETW (0x08xx)
    OMBRA_CMD_ETW_DISABLE_TI      = 0x0800,
    OMBRA_CMD_ETW_WIPE_BUFFER     = 0x0801,
    OMBRA_CMD_ETW_RESTORE         = 0x0802,

    // Diagnostics (0x09xx)
    OMBRA_CMD_PING                = 0x0900,
    OMBRA_CMD_GET_STATUS          = 0x0901,
    OMBRA_CMD_GET_SCORE           = 0x0902,
    OMBRA_CMD_RESET_SCORE         = 0x0903,
    OMBRA_CMD_CONFIG_SCORE        = 0x0904,
    OMBRA_CMD_SHUTDOWN            = 0x0905,
} OMBRA_CMD;
```

### Status Codes

```c
typedef enum _OMBRA_STATUS {
    OMBRA_STATUS_SUCCESS            = 0,
    OMBRA_STATUS_INVALID_COMMAND    = 1,
    OMBRA_STATUS_INVALID_PARAMETER  = 2,
    OMBRA_STATUS_NOT_FOUND          = 3,
    OMBRA_STATUS_ACCESS_DENIED      = 4,
    OMBRA_STATUS_LIMIT_EXCEEDED     = 5,
    OMBRA_STATUS_ALREADY_EXISTS     = 6,
    OMBRA_STATUS_VMCALL_FAILED      = 7,
    OMBRA_STATUS_INTERNAL_ERROR     = 8,
    OMBRA_STATUS_SHUTTING_DOWN      = 9,
    OMBRA_STATUS_OUT_OF_MEMORY      = 10,
} OMBRA_STATUS;
```

---

## Driver Initialization

### Bootstrap Parameter Passing

The loader patches a global variable in a custom PE section before calling DriverEntry:

```c
// Driver declares this
#pragma section(".ombra", read, write)
__declspec(allocate(".ombra"))
volatile POMBRA_DRIVER_INIT g_InitParams = NULL;

// Loader patches it during Phase 2
PIMAGE_SECTION_HEADER ombraSection = FindSection(driverBase, ".ombra");
uint64_t patchOffset = ombraSection->VirtualAddress;
*(uint64_t*)(poolR3 + patchOffset) = (uint64_t)(poolR0 + initStructOffset);
```

### Initialization Structure

```c
#define OMBRA_DRIVER_INIT_MAGIC  0x494E4954445256ULL  // 'INITDRV'

typedef struct _OMBRA_DRIVER_INIT {
    uint64_t    Magic;
    uint64_t    Version;

    // Command ring (R0 addresses)
    void*       CommandRing;
    void*       ScratchBuffer;
    uint64_t    ScratchSize;
    uint64_t    ScratchBufferPhys;  // For hypervisor operations

    // Hypervisor communication
    uint64_t    VmcallMagic;
    uint64_t    VmcallKey;

    // Process context
    uint64_t    LoaderCr3;
    uint64_t    LoaderPid;

    // EPROCESS offsets (Windows version dependent)
    EPROCESS_OFFSETS Offsets;

    // Resolved kernel functions
    OMBRA_KERNEL_IMPORTS Imports;
} OMBRA_DRIVER_INIT;

typedef struct _EPROCESS_OFFSETS {
    uint32_t    ActiveProcessLinks;
    uint32_t    DirectoryTableBase;
    uint32_t    UniqueProcessId;
    uint32_t    ImageFileName;
    uint32_t    Peb;
    uint32_t    Wow64Process;
    uint32_t    ObjectTable;
    uint32_t    Token;
    uint32_t    PebImageBaseAddress;
    uint32_t    PebLdr;
} EPROCESS_OFFSETS;

typedef struct _OMBRA_KERNEL_IMPORTS {
    void*       PsGetCurrentProcess;
    void*       PsLookupProcessByProcessId;
    void*       ObReferenceObject;
    void*       ObDereferenceObject;
    void*       KeStackAttachProcess;
    void*       KeUnstackDetachProcess;
    void*       PsGetProcessPeb;
    void*       PsGetProcessWow64Process;
    void*       KeQueryInterruptTime;
    void*       KeDelayExecutionThread;
    void*       PsCreateSystemThread;
    void*       PsTerminateSystemThread;
    void*       ExAllocatePoolWithTag;
    void*       ExFreePoolWithTag;
} OMBRA_KERNEL_IMPORTS;
```

### Driver Global Context

```c
typedef struct _OMBRA_DRIVER_CONTEXT {
    // Initialization data
    uint64_t                VmcallMagic;
    uint64_t                VmcallKey;
    uint64_t                OwnerCr3;
    uint64_t                OwnerPid;

    // Communication
    POMBRA_COMMAND_RING     CommandRing;
    void*                   ScratchBuffer;
    uint64_t                ScratchSize;
    uint64_t                ScratchBufferPhys;
    uint64_t                ScratchAllocOffset;  // Bump allocator position

    // Worker thread
    HANDLE                  WorkerThread;
    volatile LONG           ShutdownRequested;

    // Process subscriptions
    PROCESS_SUBSCRIPTION    Subscriptions[16];
    uint32_t                SubscriptionCount;

    // Protection
    struct {
        uint64_t            Cr3;
        uint32_t            AccessMask;
    } ProtectedProcesses[8];
    uint32_t                ProtectedCount;

    // Scoring
    struct {
        volatile uint32_t   CurrentScore;
        uint32_t            WarningThreshold;
        uint32_t            DetectionThreshold;
        uint32_t            EventMask;
    } Scoring;

    // Timing
    uint64_t                StartTime;
    uint64_t                CommandsProcessed;

    // Window hiding
    uint64_t                DefaultOverlayHwnd;

    // EPROCESS offsets
    EPROCESS_OFFSETS        Offsets;

    // Imports
    OMBRA_KERNEL_IMPORTS    Imports;

} OMBRA_DRIVER_CONTEXT;

static OMBRA_DRIVER_CONTEXT g_Ctx = {0};
```

### DriverEntry Implementation

```c
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    // Init pointer was patched by loader
    if (!g_InitParams || g_InitParams->Magic != OMBRA_DRIVER_INIT_MAGIC) {
        return STATUS_INVALID_PARAMETER;
    }

    POMBRA_DRIVER_INIT init = (POMBRA_DRIVER_INIT)g_InitParams;

    // Validate offsets
    if (!ValidateOffsets(&init->Offsets)) {
        return STATUS_INVALID_PARAMETER;
    }

    // Validate and copy imports
    NTSTATUS status = ValidateAndCopyImports(&g_Ctx.Imports, init);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    // Copy context
    g_Ctx.VmcallMagic = init->VmcallMagic;
    g_Ctx.VmcallKey = init->VmcallKey;
    g_Ctx.OwnerCr3 = init->LoaderCr3;
    g_Ctx.OwnerPid = init->LoaderPid;
    g_Ctx.CommandRing = init->CommandRing;
    g_Ctx.ScratchBuffer = init->ScratchBuffer;
    g_Ctx.ScratchSize = init->ScratchSize;
    g_Ctx.ScratchBufferPhys = init->ScratchBufferPhys;
    g_Ctx.Offsets = init->Offsets;

    // Initialize scoring defaults
    g_Ctx.Scoring.WarningThreshold = 50;
    g_Ctx.Scoring.DetectionThreshold = 100;
    g_Ctx.Scoring.EventMask = 0xFFFFFFFF;

    // Record start time
    g_Ctx.StartTime = g_Ctx.Imports.KeQueryInterruptTime();

    // Initialize dispatch tables
    InitDispatchTables();

    // Zero init structure (prevent info leak)
    SecureZeroMemory(init, sizeof(*init));

    // Create worker thread
    status = g_Ctx.Imports.PsCreateSystemThread(
        &g_Ctx.WorkerThread,
        THREAD_ALL_ACCESS,
        NULL, NULL, NULL,
        WorkerThreadProc,
        &g_Ctx
    );

    if (!NT_SUCCESS(status)) {
        return status;
    }

    // Signal ready to hypervisor
    VmCall(VMCALL_DRIVER_READY, g_Ctx.OwnerCr3, 0, 0);

    return STATUS_SUCCESS;
}
```

---

## Worker Thread and Command Dispatch

### Dispatch Table

```c
typedef OMBRA_STATUS (*COMMAND_HANDLER)(
    POMBRA_DRIVER_CONTEXT ctx,
    POMBRA_COMMAND cmd,
    POMBRA_RESPONSE resp
);

typedef struct _COMMAND_DISPATCH_ENTRY {
    uint32_t        CommandId;
    COMMAND_HANDLER Handler;
    const char*     Name;
} COMMAND_DISPATCH_ENTRY;

// Category-indexed lookup tables (10 categories × 16 commands each)
static COMMAND_DISPATCH_ENTRY g_CategoryTables[10][16];

static void InitDispatchTables(void) {
    RtlZeroMemory(g_CategoryTables, sizeof(g_CategoryTables));

    // Process Tracking (category 1)
    g_CategoryTables[1][0] = (COMMAND_DISPATCH_ENTRY){ 0x0100, CmdSubscribe, "Subscribe" };
    g_CategoryTables[1][1] = (COMMAND_DISPATCH_ENTRY){ 0x0101, CmdUnsubscribe, "Unsubscribe" };
    g_CategoryTables[1][2] = (COMMAND_DISPATCH_ENTRY){ 0x0102, CmdGetInfo, "GetInfo" };
    g_CategoryTables[1][3] = (COMMAND_DISPATCH_ENTRY){ 0x0103, CmdResetInfo, "ResetInfo" };
    g_CategoryTables[1][4] = (COMMAND_DISPATCH_ENTRY){ 0x0104, CmdEnumProcesses, "EnumProcesses" };
    g_CategoryTables[1][5] = (COMMAND_DISPATCH_ENTRY){ 0x0105, CmdEnumModules, "EnumModules" };

    // Memory Operations (category 2)
    g_CategoryTables[2][0] = (COMMAND_DISPATCH_ENTRY){ 0x0200, CmdHideMemory, "HideMemory" };
    g_CategoryTables[2][1] = (COMMAND_DISPATCH_ENTRY){ 0x0201, CmdShadowMemory, "ShadowMemory" };
    g_CategoryTables[2][2] = (COMMAND_DISPATCH_ENTRY){ 0x0202, CmdLockModule, "LockModule" };
    g_CategoryTables[2][3] = (COMMAND_DISPATCH_ENTRY){ 0x0203, CmdUnlockModule, "UnlockModule" };
    g_CategoryTables[2][4] = (COMMAND_DISPATCH_ENTRY){ 0x0204, CmdReadPhysical, "ReadPhysical" };
    g_CategoryTables[2][5] = (COMMAND_DISPATCH_ENTRY){ 0x0205, CmdWritePhysical, "WritePhysical" };
    g_CategoryTables[2][6] = (COMMAND_DISPATCH_ENTRY){ 0x0206, CmdReadVirtual, "ReadVirtual" };
    g_CategoryTables[2][7] = (COMMAND_DISPATCH_ENTRY){ 0x0207, CmdWriteVirtual, "WriteVirtual" };

    // ... continue for all categories
}

static PCOMMAND_DISPATCH_ENTRY FindDispatchEntry(uint32_t commandId) {
    uint32_t category = (commandId >> 8) & 0xFF;
    uint32_t index = commandId & 0xFF;

    if (category < 1 || category > 9 || index > 15) {
        return NULL;
    }

    PCOMMAND_DISPATCH_ENTRY entry = &g_CategoryTables[category][index];
    return entry->Handler ? entry : NULL;
}
```

### Worker Thread

```c
static void ProcessPendingCommands(POMBRA_DRIVER_CONTEXT ctx) {
    POMBRA_COMMAND_RING ring = ctx->CommandRing;

    uint32_t producer = ring->ProducerIndex;
    MemoryBarrier();
    uint32_t consumer = ring->ConsumerIndex;

    while (consumer != producer) {
        uint32_t slot = consumer % ring->RingSize;
        POMBRA_COMMAND cmd = (POMBRA_COMMAND)((uint8_t*)ring + 192 + slot * 512);
        POMBRA_RESPONSE resp = (POMBRA_RESPONSE)((uint8_t*)ring + 192 +
                               ring->RingSize * 512 + slot * 256);

        // Clear response
        RtlZeroMemory(resp, sizeof(*resp));
        resp->SequenceId = cmd->SequenceId;

        __try {
            PCOMMAND_DISPATCH_ENTRY entry = FindDispatchEntry(cmd->CommandId);

            if (entry && entry->Handler) {
                resp->Status = entry->Handler(ctx, cmd, resp);
            } else {
                resp->Status = OMBRA_STATUS_INVALID_COMMAND;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            resp->Status = OMBRA_STATUS_INTERNAL_ERROR;
            InterlockedAdd(&ctx->Scoring.CurrentScore, 10);
        }

        ctx->CommandsProcessed++;

        MemoryBarrier();
        resp->Ready = TRUE;

        consumer++;
    }

    ring->ConsumerIndex = consumer;
}

static void DrainRemainingCommands(POMBRA_DRIVER_CONTEXT ctx) {
    POMBRA_COMMAND_RING ring = ctx->CommandRing;
    uint32_t producer = ring->ProducerIndex;
    uint32_t consumer = ring->ConsumerIndex;

    while (consumer != producer) {
        uint32_t slot = consumer % ring->RingSize;
        POMBRA_RESPONSE resp = (POMBRA_RESPONSE)((uint8_t*)ring + 192 +
                               ring->RingSize * 512 + slot * 256);

        resp->Status = OMBRA_STATUS_SHUTTING_DOWN;
        MemoryBarrier();
        resp->Ready = TRUE;

        consumer++;
    }

    ring->ConsumerIndex = consumer;
}

VOID WorkerThreadProc(PVOID Context) {
    POMBRA_DRIVER_CONTEXT ctx = (POMBRA_DRIVER_CONTEXT)Context;
    LARGE_INTEGER cmdInterval = { .QuadPart = -10000 };  // 1ms
    uint32_t pollCounter = 0;

    while (!ctx->ShutdownRequested) {
        __try {
            ProcessPendingCommands(ctx);

            // Poll for pending subscriptions every 100ms
            if (++pollCounter >= 100) {
                pollCounter = 0;
                PollPendingSubscriptions(ctx);
                UpdateSubscriptionStates(ctx);
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            InterlockedAdd(&ctx->Scoring.CurrentScore, 10);
        }

        ctx->Imports.KeDelayExecutionThread(KernelMode, FALSE, &cmdInterval);
    }

    // Drain remaining commands
    DrainRemainingCommands(ctx);

    // Cleanup
    CleanupAllSubscriptions(ctx);
    CleanupProtections(ctx);

    // Notify hypervisor
    VmCall(VMCALL_DRIVER_SHUTDOWN, 0, 0, 0);

    ctx->Imports.PsTerminateSystemThread(STATUS_SUCCESS);
}
```

---

## Process Subscription System

### Subscription State

```c
typedef enum _SUBSCRIPTION_STATE {
    SUB_STATE_EMPTY = 0,
    SUB_STATE_PENDING,
    SUB_STATE_ACTIVE,
    SUB_STATE_DEAD,
} SUBSCRIPTION_STATE;

typedef struct _LOCKED_MODULE {
    char                Name[64];
    uint64_t            TargetCr3;
    uint64_t            Base;
    uint64_t            Size;
    uint64_t*           OriginalPfns;
    uint64_t*           BackupPfns;
    uint32_t            PageCount;
    BOOLEAN             Locked;
    BOOLEAN             Shadowed;
    BOOLEAN             BackupValid;
} LOCKED_MODULE;

typedef struct _PROCESS_SUBSCRIPTION {
    SUBSCRIPTION_STATE  State;
    char                ImageName[64];
    uint64_t            Cr3;
    uint64_t            Pid;
    PEPROCESS           Process;

    uint64_t            Peb;
    uint64_t            Peb32;
    uint64_t            ImageBase;
    uint64_t            ImageSize;

    struct {
        uint64_t        DllBase;
        uint64_t        DllSize;
        uint64_t        ScratchOffset;
        uint32_t        Flags;
        BOOLEAN         Pending;
        BOOLEAN         Complete;
        BOOLEAN         Hidden;
    } Injection;

    LOCKED_MODULE       LockedModules[8];
    uint32_t            LockedModuleCount;

    uint64_t            OverlayHwnd;
    uint64_t            NotifyCr3;

} PROCESS_SUBSCRIPTION;
```

### Process Finding

```c
static BOOLEAN MatchesImageName(const char* eprocessName, const char* targetName) {
    size_t targetLen = strlen(targetName);

    // EPROCESS.ImageFileName is max 15 chars
    if (targetLen <= 15) {
        return _strnicmp(eprocessName, targetName, 15) == 0;
    } else {
        // Compare prefix only for long names
        return _strnicmp(eprocessName, targetName, 15) == 0;
    }
}

static BOOLEAN TryFindRunningProcess(
    POMBRA_DRIVER_CONTEXT ctx,
    PPROCESS_SUBSCRIPTION sub
) {
    PEPROCESS initialProcess = PsInitialSystemProcess;
    if (!initialProcess) {
        return FALSE;
    }

    ULONG linksOffset = ctx->Offsets.ActiveProcessLinks;
    ULONG nameOffset = ctx->Offsets.ImageFileName;
    ULONG cr3Offset = ctx->Offsets.DirectoryTableBase;
    ULONG pidOffset = ctx->Offsets.UniqueProcessId;

    PLIST_ENTRY listHead = (PLIST_ENTRY)((uint8_t*)initialProcess + linksOffset);
    PLIST_ENTRY current = listHead->Flink;
    BOOLEAN found = FALSE;

    __try {
        while (current != listHead && !found) {
            PEPROCESS process = (PEPROCESS)((uint8_t*)current - linksOffset);

            if ((uint64_t)process < 0xFFFF800000000000ULL) {
                current = current->Flink;
                continue;
            }

            const char* processName = (const char*)((uint8_t*)process + nameOffset);

            if (MatchesImageName(processName, sub->ImageName)) {
                // Reference object to keep it alive
                NTSTATUS status = ObReferenceObjectByPointer(
                    process, 0, *PsProcessType, KernelMode);

                if (!NT_SUCCESS(status)) {
                    current = current->Flink;
                    continue;
                }

                sub->Process = process;
                sub->Cr3 = *(uint64_t*)((uint8_t*)process + cr3Offset);
                sub->Pid = *(uint64_t*)((uint8_t*)process + pidOffset);
                sub->Peb = (uint64_t)ctx->Imports.PsGetProcessPeb(process);
                sub->Peb32 = (uint64_t)ctx->Imports.PsGetProcessWow64Process(process);

                // Read image base from PEB
                if (sub->Peb) {
                    VmReadVirtual(ctx, sub->Cr3,
                                 sub->Peb + ctx->Offsets.PebImageBaseAddress,
                                 &sub->ImageBase, sizeof(sub->ImageBase));
                }

                found = TRUE;
            }

            current = current->Flink;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        if (found && sub->Process) {
            ctx->Imports.ObDereferenceObject(sub->Process);
            sub->Process = NULL;
            found = FALSE;
        }
    }

    return found;
}
```

### Subscription Lifecycle

```c
static void PollPendingSubscriptions(POMBRA_DRIVER_CONTEXT ctx) {
    for (uint32_t i = 0; i < 16; i++) {
        PPROCESS_SUBSCRIPTION sub = &ctx->Subscriptions[i];

        if (sub->State == SUB_STATE_PENDING) {
            if (TryFindRunningProcess(ctx, sub)) {
                sub->State = SUB_STATE_ACTIVE;

                // Tell hypervisor to watch this CR3
                VmCall(VMCALL_WATCH_CR3, sub->Cr3, 0, 0);

                // Notify usermode
                if (sub->NotifyCr3) {
                    VmWriteVirtual(ctx, ctx->OwnerCr3, sub->NotifyCr3,
                                  &sub->Cr3, sizeof(sub->Cr3));
                }
            }
        }
    }
}

static void UpdateSubscriptionStates(POMBRA_DRIVER_CONTEXT ctx) {
    for (uint32_t i = 0; i < 16; i++) {
        PPROCESS_SUBSCRIPTION sub = &ctx->Subscriptions[i];

        if (sub->State != SUB_STATE_ACTIVE) {
            continue;
        }

        PEPROCESS process = NULL;
        NTSTATUS status = ctx->Imports.PsLookupProcessByProcessId(
            (HANDLE)sub->Pid, &process);

        if (!NT_SUCCESS(status) || process != sub->Process) {
            MarkSubscriptionDead(ctx, sub);
        } else {
            ctx->Imports.ObDereferenceObject(process);
        }
    }
}

static void MarkSubscriptionDead(
    POMBRA_DRIVER_CONTEXT ctx,
    PPROCESS_SUBSCRIPTION sub
) {
    sub->State = SUB_STATE_DEAD;

    // Release locked modules
    for (uint32_t i = 0; i < sub->LockedModuleCount; i++) {
        CleanupLockedModule(ctx, &sub->LockedModules[i]);
    }

    // Release injection hiding
    if (sub->Injection.Hidden) {
        VmCall(VMCALL_UNHIDE_MEMORY, sub->Cr3,
               sub->Injection.DllBase, sub->Injection.DllSize);
    }

    // Unwatch CR3
    VmCall(VMCALL_UNWATCH_CR3, sub->Cr3, 0, 0);

    // Dereference process object
    if (sub->Process) {
        ctx->Imports.ObDereferenceObject(sub->Process);
        sub->Process = NULL;
    }

    // Notify usermode
    if (sub->NotifyCr3) {
        uint64_t zero = 0;
        VmWriteVirtual(ctx, ctx->OwnerCr3, sub->NotifyCr3, &zero, sizeof(zero));
    }
}
```

---

## Module Locking and EPT Shadowing

### EPT Split Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    EPT Split Page Flow                       │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  Default State: Read=1, Write=0, Execute=0 → Clean Page     │
│                                                              │
│  Anti-cheat reads: No violation, sees clean (original) code │
│                                                              │
│  Target executes:                                            │
│    1. Execute violation (Execute=0)                          │
│    2. Switch to dirty page, grant full access                │
│    3. Enable MTF for single-instruction step                 │
│    4. Instruction executes (sees hooked code)                │
│    5. MTF fires → restore to clean page                      │
│                                                              │
│  Result: Hooks are only visible for single instruction       │
│          Anti-cheat scans never see modifications            │
└─────────────────────────────────────────────────────────────┘
```

### Module Lock Implementation

```c
static OMBRA_STATUS LockModulePages(
    POMBRA_DRIVER_CONTEXT ctx,
    PPROCESS_SUBSCRIPTION sub,
    PLOCKED_MODULE mod
) {
    KAPC_STATE apcState;

    // Attach to target process to touch pages
    ctx->Imports.KeStackAttachProcess(sub->Process, &apcState);

    __try {
        for (uint32_t i = 0; i < mod->PageCount; i++) {
            volatile uint8_t* page = (uint8_t*)(mod->Base + i * PAGE_SIZE);
            volatile uint8_t dummy = *page;  // Force page-in
            UNREFERENCED_PARAMETER(dummy);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        ctx->Imports.KeUnstackDetachProcess(&apcState);
        return OMBRA_STATUS_INVALID_PARAMETER;
    }

    ctx->Imports.KeUnstackDetachProcess(&apcState);

    // Allocate PFN tracking from scratch buffer
    size_t pfnArraySize = mod->PageCount * sizeof(uint64_t);
    mod->OriginalPfns = ScratchAlloc(ctx, pfnArraySize);
    mod->BackupPfns = ScratchAlloc(ctx, pfnArraySize);

    if (!mod->OriginalPfns || !mod->BackupPfns) {
        return OMBRA_STATUS_OUT_OF_MEMORY;
    }

    // Translate VAs to PAs and pin pages
    for (uint32_t i = 0; i < mod->PageCount; i++) {
        uint64_t va = mod->Base + (i * PAGE_SIZE);
        uint64_t pa = 0;

        int64_t result = VmCall(VMCALL_VIRT_TO_PHYS, mod->TargetCr3, va, (uint64_t)&pa);
        if (result < 0 || pa == 0) {
            return OMBRA_STATUS_INVALID_PARAMETER;
        }

        mod->OriginalPfns[i] = pa >> PAGE_SHIFT;
        VmCall(VMCALL_PIN_PAGE, pa, 0, 0);
    }

    return OMBRA_STATUS_SUCCESS;
}

static OMBRA_STATUS CreateModuleBackup(
    POMBRA_DRIVER_CONTEXT ctx,
    PPROCESS_SUBSCRIPTION sub,
    PLOCKED_MODULE mod
) {
    for (uint32_t i = 0; i < mod->PageCount; i++) {
        uint64_t backupPa = 0;

        int64_t result = VmCall(VMCALL_ALLOC_PHYSICAL_PAGE, 0, 0, (uint64_t)&backupPa);
        if (result < 0 || backupPa == 0) {
            // Rollback
            for (uint32_t j = 0; j < i; j++) {
                VmCall(VMCALL_FREE_PHYSICAL_PAGE, mod->BackupPfns[j] << PAGE_SHIFT, 0, 0);
            }
            return OMBRA_STATUS_OUT_OF_MEMORY;
        }

        mod->BackupPfns[i] = backupPa >> PAGE_SHIFT;

        // Copy original content
        uint64_t originalPa = mod->OriginalPfns[i] << PAGE_SHIFT;
        VmCall(VMCALL_COPY_PHYSICAL_PAGE, originalPa, backupPa, 0);
    }

    return OMBRA_STATUS_SUCCESS;
}
```

### Hypervisor EPT Split Handler

```c
// In hypervisor handlers/ept_split.c

typedef struct _EPT_SPLIT_INFO {
    uint64_t    GuestPa;
    uint64_t    DirtyPfn;       // Hooked code
    uint64_t    CleanPfn;       // Original code
    uint64_t    TargetCr3;
    BOOLEAN     Active;
} EPT_SPLIT_INFO;

static EPT_SPLIT_INFO g_Splits[256];

static int64_t HandleSplitEptPage(
    VMX_CPU* cpu,
    uint64_t targetCr3,
    uint64_t guestPa,
    uint64_t cleanPa
) {
    // Find free slot
    EPT_SPLIT_INFO* split = NULL;
    for (int i = 0; i < 256; i++) {
        if (!g_Splits[i].Active) {
            split = &g_Splits[i];
            break;
        }
    }

    if (!split) {
        return VMCALL_STATUS_LIMIT_EXCEEDED;
    }

    EPT_PTE* pte = EptGetPte(cpu->Ept, guestPa);
    if (!pte) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Store original (dirty) PFN
    split->GuestPa = guestPa & ~0xFFFULL;
    split->DirtyPfn = pte->PageFrameNumber;
    split->CleanPfn = cleanPa >> PAGE_SHIFT;
    split->TargetCr3 = targetCr3;
    split->Active = TRUE;

    // Default state: read-only pointing to clean copy
    pte->ReadAccess = 1;
    pte->WriteAccess = 0;
    pte->ExecuteAccess = 0;
    pte->PageFrameNumber = split->CleanPfn;

    InveptSingleContext(cpu->Ept->EptPointer);

    return VMCALL_STATUS_SUCCESS;
}
```

### EPT Violation Handler with MTF

```c
// In handlers/ept_violation.c

VMEXIT_RESULT HandleEptViolation(VMX_CPU* cpu, GUEST_REGS* regs) {
    uint64_t guestPa = VmcsRead(VMCS_RODATA_GUEST_PHYS);
    uint64_t qualification = VmcsRead(VMCS_RODATA_EXIT_QUAL);

    BOOLEAN isExecute = (qualification & 4) != 0;

    EPT_SPLIT_INFO* split = EptFindSplit(guestPa);
    if (!split) {
        return HandleRegularEptViolation(cpu, regs, guestPa, qualification);
    }

    EPT_PTE* pte = EptGetPte(cpu->Ept, guestPa);

    if (isExecute) {
        uint64_t currentCr3 = VmcsRead(VMCS_GUEST_CR3);

        // Only show dirty page to target process
        if ((currentCr3 & ~0xFFF) == (split->TargetCr3 & ~0xFFF)) {
            // Switch to dirty page with full access
            pte->PageFrameNumber = split->DirtyPfn;
            pte->ReadAccess = 1;
            pte->WriteAccess = 1;
            pte->ExecuteAccess = 1;

            // Enable MTF for single-step
            uint64_t procCtls = VmcsRead(VMCS_CTLS_CPU_BASED);
            VmcsWrite(VMCS_CTLS_CPU_BASED, procCtls | CPU_BASED_MTF);

            cpu->PendingMtfRestore = split;
        } else {
            // Other process - show clean page but allow execute
            pte->PageFrameNumber = split->CleanPfn;
            pte->ExecuteAccess = 1;
        }
    } else {
        // Read violation - show clean page
        pte->PageFrameNumber = split->CleanPfn;
        pte->ReadAccess = 1;
        pte->WriteAccess = 0;
        pte->ExecuteAccess = 0;
    }

    InveptSingleContext(cpu->Ept->EptPointer);
    return VMEXIT_CONTINUE;
}

// MTF handler - fires after single instruction
VMEXIT_RESULT HandleMtf(VMX_CPU* cpu, GUEST_REGS* regs) {
    // Disable MTF
    uint64_t procCtls = VmcsRead(VMCS_CTLS_CPU_BASED);
    VmcsWrite(VMCS_CTLS_CPU_BASED, procCtls & ~CPU_BASED_MTF);

    if (cpu->PendingMtfRestore) {
        EPT_SPLIT_INFO* split = cpu->PendingMtfRestore;
        EPT_PTE* pte = EptGetPte(cpu->Ept, split->GuestPa);

        // Restore to default: read-only clean page
        pte->PageFrameNumber = split->CleanPfn;
        pte->ReadAccess = 1;
        pte->WriteAccess = 0;
        pte->ExecuteAccess = 0;

        InveptSingleContext(cpu->Ept->EptPointer);
        cpu->PendingMtfRestore = NULL;
    }

    return VMEXIT_CONTINUE;
}
```

---

## New VMCALL Interface

### Required VMCALLs for Phase 3

| VMCALL | Parameters | Purpose |
|--------|------------|---------|
| `VMCALL_WATCH_CR3` | cr3 | Register CR3 for monitoring |
| `VMCALL_UNWATCH_CR3` | cr3 | Remove CR3 from watch |
| `VMCALL_PIN_PAGE` | pa | Increment page refcount |
| `VMCALL_UNPIN_PAGE` | pa | Decrement page refcount |
| `VMCALL_ALLOC_PHYSICAL_PAGE` | out_pa | Allocate from backup pool |
| `VMCALL_FREE_PHYSICAL_PAGE` | pa | Return to backup pool |
| `VMCALL_COPY_PHYSICAL_PAGE` | src_pa, dst_pa | memcpy between physical pages |
| `VMCALL_SPLIT_EPT_PAGE` | cr3, gpa, clean_pa | Enable shadow page |
| `VMCALL_UNSPLIT_EPT_PAGE` | cr3, gpa | Disable shadow page |
| `VMCALL_VIRT_TO_PHYS` | cr3, va, out_pa | Translate VA using target CR3 |
| `VMCALL_READ_VIRT` | cr3, va, size, dest_pa | Read from target VA |
| `VMCALL_WRITE_VIRT` | cr3, va, size, src_val | Write to target VA |
| `VMCALL_DRIVER_READY` | owner_cr3 | Signal driver initialized |
| `VMCALL_DRIVER_SHUTDOWN` | - | Clean teardown |

---

## Implementation Tasks

### Task 3.1: Command Ring Infrastructure
**Files:** `driver/command_ring.h`, `driver/command_ring.c`
**Dependencies:** None
**Effort:** 1 task

### Task 3.2: Driver Initialization
**Files:** `driver/init.c`, `driver/context.h`
**Dependencies:** Task 3.1
**Effort:** 1 task

### Task 3.3: Worker Thread and Dispatch
**Files:** `driver/worker.c`, `driver/dispatch.c`
**Dependencies:** Task 3.1, Task 3.2
**Effort:** 1 task

### Task 3.4: VMCALL Interface
**Files:** `driver/vmcall.c`, `driver/vmcall.h`
**Dependencies:** None
**Effort:** 1 task

### Task 3.5: Process Subscription System
**Files:** `driver/subscription.c`, `driver/subscription.h`
**Dependencies:** Task 3.3, Task 3.4
**Effort:** 2 tasks

### Task 3.6: Module Locking
**Files:** `driver/module_lock.c`, `driver/module_lock.h`
**Dependencies:** Task 3.4, Task 3.5
**Effort:** 2 tasks

### Task 3.7: EPT Shadow Implementation

#### Task 3.7a: Driver-side shadow request handlers
**Files:** `driver/shadow.c`
**Dependencies:** Task 3.6
**Effort:** 1 task

#### Task 3.7b: Hypervisor VMCALL_SPLIT/UNSPLIT handlers
**Files:** `hypervisor/handlers/ept_split.c`
**Dependencies:** Existing EPT infrastructure
**Effort:** 1 task

#### Task 3.7c: EPT violation handler modifications
**Files:** `hypervisor/handlers/ept_violation.c`
**Dependencies:** Task 3.7b
**Effort:** 1 task

#### Task 3.7d: MTF handler for single-instruction window
**Files:** `hypervisor/handlers/mtf.c`
**Dependencies:** Task 3.7c
**Effort:** 1 task

### Task 3.8: Memory Operations
**Files:** `driver/memory_ops.c`
**Dependencies:** Task 3.4
**Effort:** 1 task

### Task 3.9: Injection System
**Files:** `driver/inject.c`
**Dependencies:** Task 3.5, Task 3.7
**Effort:** 2 tasks

### Task 3.10: Protection System
**Files:** `driver/protection.c`
**Dependencies:** Task 3.4, Task 3.5
**Effort:** 2 tasks

### Task 3.11: Window Hiding
**Files:** `driver/window.c`
**Dependencies:** Task 3.5
**Effort:** 1 task

### Task 3.12: ETW Manipulation
**Files:** `driver/etw.c`
**Dependencies:** Task 3.4
**Effort:** 2 tasks

### Task 3.13: Spoofing Configuration
**Files:** `driver/spoof.c`
**Dependencies:** Task 3.4
**Effort:** 1 task

### Task 3.14: Diagnostics and Shutdown
**Files:** `driver/diagnostics.c`
**Dependencies:** Task 3.5, Task 3.6
**Effort:** 1 task

### Task 3.15: Usermode API Library
**Files:** `usermode/ombra_client.h`, `usermode/ombra_client.c`
**Dependencies:** Task 3.1
**Effort:** 1 task

---

## Testing Milestones

| Milestone | Validation |
|-----------|------------|
| M1 | Driver loads, worker thread running, Ping returns |
| M2 | Subscribe to notepad.exe, GetInfo returns valid CR3/PEB |
| M3 | Read/Write physical and virtual memory works |
| M4 | Lock ntdll.dll, verify pages pinned |
| M5 | Shadow locked module, verify reads show clean, executes work |
| M6 | Inject test DLL, verify execution |
| M7 | Complete workflow: subscribe → lock → shadow → inject → hide |
| M8 | Run with target game, verify no detection |

---

## Summary

- **Total driver code:** ~3,500 LOC
- **New hypervisor VMCALLs:** ~500 LOC
- **Implementation time:** 6 weeks with focused effort
- **Highest complexity:** EPT shadowing (Task 3.7)
- **Blocking prerequisite:** Phase 2 BigPool test results
