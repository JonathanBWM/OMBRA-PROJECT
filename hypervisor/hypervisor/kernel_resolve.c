// kernel_resolve.c - Runtime Kernel Symbol Resolution
// OmbraHypervisor
//
// Resolves kernel symbols at runtime using MmGetSystemRoutineAddress.
// This allows the hypervisor to be self-contained without requiring
// the loader to pass in resolved function pointers.

#include "kernel_resolve.h"

// =============================================================================
// Global Symbol Table
// =============================================================================

KERNEL_SYMBOLS g_KernelSymbols = {0};

// =============================================================================
// Helper: Initialize UNICODE_STRING from wide string
// =============================================================================

static void InitUnicodeString(UNICODE_STRING* dest, const wchar_t* src) {
    U16 len = 0;
    while (src[len]) len++;
    dest->Length = len * sizeof(wchar_t);
    dest->MaximumLength = dest->Length + sizeof(wchar_t);
    dest->Buffer = (wchar_t*)src;
}

// =============================================================================
// Resolve Single Symbol
// =============================================================================

PVOID KernelResolveSymbol(const wchar_t* symbolName) {
    UNICODE_STRING name;

    if (!g_KernelSymbols.MmGetSystemRoutineAddress) {
        return NULL;
    }

    InitUnicodeString(&name, symbolName);
    return g_KernelSymbols.MmGetSystemRoutineAddress(&name);
}

// =============================================================================
// Bootstrap: Get MmGetSystemRoutineAddress
// =============================================================================
//
// This is the bootstrap problem: we need MmGetSystemRoutineAddress to resolve
// other symbols, but how do we get MmGetSystemRoutineAddress itself?
//
// Solution: The loader must provide this single function pointer. Everything
// else can be resolved at runtime. Alternatively, for mapped drivers, we can
// use PE import resolution if ntoskrnl imports are available.
//
// For the self-contained approach, we expect the loader to set:
//   g_KernelSymbols.MmGetSystemRoutineAddress
// before calling the hypervisor entry point.

NTSTATUS KernelResolveSymbols(void) {
    // Check if bootstrap function is available
    if (!g_KernelSymbols.MmGetSystemRoutineAddress) {
        return STATUS_NOT_FOUND;
    }

    // Resolve memory functions
    g_KernelSymbols.MmAllocateContiguousMemorySpecifyCacheNode =
        (FN_MmAllocateContiguousMemorySpecifyCacheNode)KernelResolveSymbol(
            L"MmAllocateContiguousMemorySpecifyCacheNode");

    g_KernelSymbols.MmFreeContiguousMemory =
        (FN_MmFreeContiguousMemory)KernelResolveSymbol(L"MmFreeContiguousMemory");

    g_KernelSymbols.ExAllocatePool =
        (FN_ExAllocatePool)KernelResolveSymbol(L"ExAllocatePool");

    g_KernelSymbols.ExFreePool =
        (FN_ExFreePool)KernelResolveSymbol(L"ExFreePool");

    g_KernelSymbols.MmGetPhysicalAddress =
        (FN_MmGetPhysicalAddress)KernelResolveSymbol(L"MmGetPhysicalAddress");

    g_KernelSymbols.MmGetVirtualForPhysical =
        (FN_MmGetVirtualForPhysical)KernelResolveSymbol(L"MmGetVirtualForPhysical");

    // Resolve IPI / processor functions
    g_KernelSymbols.KeIpiGenericCall =
        (FN_KeIpiGenericCall)KernelResolveSymbol(L"KeIpiGenericCall");

    g_KernelSymbols.KeQueryActiveProcessorCountEx =
        (FN_KeQueryActiveProcessorCountEx)KernelResolveSymbol(L"KeQueryActiveProcessorCountEx");

    g_KernelSymbols.KeGetCurrentProcessorNumberEx =
        (FN_KeGetCurrentProcessorNumberEx)KernelResolveSymbol(L"KeGetCurrentProcessorNumberEx");

    g_KernelSymbols.KeGetProcessorNumberFromIndex =
        (FN_KeGetProcessorNumberFromIndex)KernelResolveSymbol(L"KeGetProcessorNumberFromIndex");

    g_KernelSymbols.KeSetSystemGroupAffinityThread =
        (FN_KeSetSystemGroupAffinityThread)KernelResolveSymbol(L"KeSetSystemGroupAffinityThread");

    g_KernelSymbols.KeRevertToUserGroupAffinityThread =
        (FN_KeRevertToUserGroupAffinityThread)KernelResolveSymbol(L"KeRevertToUserGroupAffinityThread");

    // Resolve thread functions
    g_KernelSymbols.PsCreateSystemThread =
        (FN_PsCreateSystemThread)KernelResolveSymbol(L"PsCreateSystemThread");

    g_KernelSymbols.PsTerminateSystemThread =
        (FN_PsTerminateSystemThread)KernelResolveSymbol(L"PsTerminateSystemThread");

    g_KernelSymbols.ZwClose =
        (FN_ZwClose)KernelResolveSymbol(L"ZwClose");

    // Resolve process functions
    g_KernelSymbols.PsGetCurrentProcess =
        (FN_PsGetCurrentProcess)KernelResolveSymbol(L"PsGetCurrentProcess");

    g_KernelSymbols.PsGetProcessId =
        (FN_PsGetProcessId)KernelResolveSymbol(L"PsGetProcessId");

    // Resolve system info functions
    g_KernelSymbols.ZwQuerySystemInformation =
        (FN_ZwQuerySystemInformation)KernelResolveSymbol(L"ZwQuerySystemInformation");

    // Validate critical symbols are resolved
    if (!g_KernelSymbols.MmAllocateContiguousMemorySpecifyCacheNode ||
        !g_KernelSymbols.MmFreeContiguousMemory ||
        !g_KernelSymbols.MmGetPhysicalAddress ||
        !g_KernelSymbols.KeIpiGenericCall ||
        !g_KernelSymbols.KeQueryActiveProcessorCountEx ||
        !g_KernelSymbols.KeGetCurrentProcessorNumberEx) {
        return STATUS_NOT_FOUND;
    }

    g_KernelSymbols.Initialized = true;
    return STATUS_SUCCESS;
}

// =============================================================================
// Memory Allocation Helpers
// =============================================================================

// Pool types
#define NonPagedPool 0
#define NonPagedPoolNx 512

// Cache types
#define MmNonCached 0
#define MmCached 1
#define MmWriteCombined 2

PVOID KernelAllocateContiguous(U64 size) {
    if (!g_KernelSymbols.Initialized ||
        !g_KernelSymbols.MmAllocateContiguousMemorySpecifyCacheNode) {
        return NULL;
    }

    // Allocate contiguous non-cached memory
    // Lowest: 0, Highest: max physical, Boundary: 0, Cache: NonCached, Node: any
    return g_KernelSymbols.MmAllocateContiguousMemorySpecifyCacheNode(
        size,
        0,                          // LowestAcceptableAddress
        0xFFFFFFFFFFFFFFFFULL,      // HighestAcceptableAddress
        0,                          // BoundaryAddressMultiple (no boundary)
        MmNonCached,                // CacheType
        0xFFFFFFFF                  // PreferredNode (any node)
    );
}

PVOID KernelAllocatePageAligned(U64 size) {
    if (!g_KernelSymbols.Initialized || !g_KernelSymbols.ExAllocatePool) {
        return NULL;
    }

    // Round up to page boundary
    size = (size + 0xFFF) & ~0xFFFULL;

    return g_KernelSymbols.ExAllocatePool(NonPagedPoolNx, size);
}

void KernelFreeContiguous(PVOID ptr) {
    if (!g_KernelSymbols.Initialized || !g_KernelSymbols.MmFreeContiguousMemory) {
        return;
    }

    g_KernelSymbols.MmFreeContiguousMemory(ptr);
}

U64 KernelGetPhysicalAddress(PVOID virtualAddress) {
    if (!g_KernelSymbols.Initialized || !g_KernelSymbols.MmGetPhysicalAddress) {
        return 0;
    }

    return g_KernelSymbols.MmGetPhysicalAddress(virtualAddress);
}

// =============================================================================
// Processor Helpers
// =============================================================================

U32 KernelGetProcessorCount(void) {
    if (!g_KernelSymbols.Initialized ||
        !g_KernelSymbols.KeQueryActiveProcessorCountEx) {
        return 1;
    }

    // 0xFFFF = ALL_PROCESSOR_GROUPS
    return g_KernelSymbols.KeQueryActiveProcessorCountEx(0xFFFF);
}

// IPI context for per-CPU execution
typedef struct _IPI_CONTEXT {
    NTSTATUS (*Callback)(PVOID);
    PVOID UserContext;
    volatile long SuccessCount;
    volatile long FailCount;
} IPI_CONTEXT;

// IPI worker function
static U64 __stdcall IpiWorker(U64 context) {
    IPI_CONTEXT* ctx = (IPI_CONTEXT*)context;
    NTSTATUS status;

    status = ctx->Callback(ctx->UserContext);

    if (NT_SUCCESS(status)) {
        _InterlockedIncrement(&ctx->SuccessCount);
    } else {
        _InterlockedIncrement(&ctx->FailCount);
    }

    return 0;
}

NTSTATUS KernelExecuteOnEachProcessor(NTSTATUS(*callback)(PVOID), PVOID context) {
    IPI_CONTEXT ipiCtx;
    U32 cpuCount;

    if (!g_KernelSymbols.Initialized || !g_KernelSymbols.KeIpiGenericCall) {
        return STATUS_NOT_FOUND;
    }

    cpuCount = KernelGetProcessorCount();

    ipiCtx.Callback = callback;
    ipiCtx.UserContext = context;
    ipiCtx.SuccessCount = 0;
    ipiCtx.FailCount = 0;

    // Broadcast IPI to all processors
    g_KernelSymbols.KeIpiGenericCall((void*)IpiWorker, (U64)&ipiCtx);

    // Check results
    if (ipiCtx.SuccessCount == 0) {
        return (NTSTATUS)0xC0000001L;  // STATUS_UNSUCCESSFUL
    }

    if (ipiCtx.FailCount > 0) {
        // Partial success - some CPUs failed
        return (NTSTATUS)0x8000000DL;  // STATUS_PARTIAL_COPY
    }

    return STATUS_SUCCESS;
}

// =============================================================================
// EPROCESS Offset Discovery (Cross-Version Compatibility)
// =============================================================================
//
// Discovers EPROCESS structure offsets at runtime by pattern scanning
// kernel functions. This allows the hypervisor to work across different
// Windows versions without hardcoded offsets.
//
// Patterns used:
// - PsGetProcessId:     mov rax, [rcx + XX] -> UniqueProcessId offset
// - PsGetProcessPeb:    mov rax, [rcx + XX] -> Peb offset (can derive others)
// - ActiveProcessLinks: UniqueProcessId + 8 (always follows UniqueProcessId)

// Global offset storage
KERNEL_OFFSETS g_KernelOffsets = {0};

// Pattern: 48 8B 81 XX XX XX XX  (mov rax, [rcx+disp32])
// Pattern: 48 8B 41 XX           (mov rax, [rcx+disp8])
static bool ScanForMovRaxRcxDisp(PVOID function, U32 maxScan, U64* outOffset) {
    U8* code = (U8*)function;

    for (U32 i = 0; i < maxScan - 7; i++) {
        // Check for REX.W prefix (48)
        if (code[i] != 0x48) continue;

        // MOV r64, [r64+disp32]: 48 8B 81 XX XX XX XX
        if (code[i+1] == 0x8B && code[i+2] == 0x81) {
            *outOffset = *(U32*)&code[i+3];
            return true;
        }

        // MOV r64, [r64+disp8]: 48 8B 41 XX
        if (code[i+1] == 0x8B && code[i+2] == 0x41) {
            *outOffset = (U64)code[i+3];
            return true;
        }
    }

    return false;
}

NTSTATUS KernelDiscoverOffsets(void) {
    PVOID psGetProcessId;
    U64 uniqueProcessIdOffset = 0;

    if (g_KernelOffsets.Discovered) {
        return STATUS_SUCCESS;
    }

    // Default offsets for Windows 10 22H2 / Windows 11 (fallback)
    g_KernelOffsets.UniqueProcessId = 0x440;
    g_KernelOffsets.ActiveProcessLinks = 0x448;
    g_KernelOffsets.DirectoryTableBase = 0x28;
    g_KernelOffsets.ImageFileName = 0x5A8;

    // Try to resolve PsGetProcessId for dynamic discovery
    psGetProcessId = KernelResolveSymbol(L"PsGetProcessId");
    if (psGetProcessId) {
        // Scan the function prologue for the offset load
        // PsGetProcessId is: mov rax, [rcx+UniqueProcessId]; ret
        if (ScanForMovRaxRcxDisp(psGetProcessId, 32, &uniqueProcessIdOffset)) {
            g_KernelOffsets.UniqueProcessId = uniqueProcessIdOffset;
            // ActiveProcessLinks always follows UniqueProcessId (+8 on x64)
            g_KernelOffsets.ActiveProcessLinks = uniqueProcessIdOffset + 8;
        }
    }

    // DirectoryTableBase is at fixed offset 0x28 in KPROCESS (stable across versions)
    // ImageFileName offset could be discovered from PsGetProcessImageFileName if needed

    g_KernelOffsets.Discovered = true;
    return STATUS_SUCCESS;
}

// Accessor functions for offsets
U64 KernelGetUniqueProcessIdOffset(void) {
    if (!g_KernelOffsets.Discovered) KernelDiscoverOffsets();
    return g_KernelOffsets.UniqueProcessId;
}

U64 KernelGetActiveProcessLinksOffset(void) {
    if (!g_KernelOffsets.Discovered) KernelDiscoverOffsets();
    return g_KernelOffsets.ActiveProcessLinks;
}

U64 KernelGetDirectoryTableBaseOffset(void) {
    if (!g_KernelOffsets.Discovered) KernelDiscoverOffsets();
    return g_KernelOffsets.DirectoryTableBase;
}

U64 KernelGetImageFileNameOffset(void) {
    if (!g_KernelOffsets.Discovered) KernelDiscoverOffsets();
    return g_KernelOffsets.ImageFileName;
}
