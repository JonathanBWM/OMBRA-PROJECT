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
