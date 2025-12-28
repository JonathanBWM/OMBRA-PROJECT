// kernel_resolve.h - Runtime Kernel Symbol Resolution
// OmbraHypervisor
//
// Resolves kernel symbols at runtime using MmGetSystemRoutineAddress.
// This allows the hypervisor to be self-contained without requiring
// the loader to pass in resolved function pointers.

#ifndef KERNEL_RESOLVE_H
#define KERNEL_RESOLVE_H

#include "../shared/types.h"

// =============================================================================
// Kernel Types
// =============================================================================

#ifndef NTSTATUS
typedef long NTSTATUS;
#endif

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#endif

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS              ((NTSTATUS)0x00000000L)
#define STATUS_NOT_FOUND            ((NTSTATUS)0xC0000225L)
#define STATUS_INSUFFICIENT_RESOURCES ((NTSTATUS)0xC000009AL)
#endif

typedef struct _UNICODE_STRING {
    U16 Length;
    U16 MaximumLength;
    wchar_t* Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _PROCESSOR_NUMBER {
    U16 Group;
    U8  Number;
    U8  Reserved;
} PROCESSOR_NUMBER, *PPROCESSOR_NUMBER;

typedef struct _GROUP_AFFINITY {
    U64 Mask;
    U16 Group;
    U16 Reserved[3];
} GROUP_AFFINITY, *PGROUP_AFFINITY;

typedef void* PEPROCESS;
typedef void* HANDLE;
typedef void* PVOID;
typedef void* PDEVICE_OBJECT;
typedef void* PDRIVER_OBJECT;
typedef void* PFILE_OBJECT;
typedef void* PMDL;

// Calling convention
#ifndef NTAPI
#define NTAPI __stdcall
#endif

// =============================================================================
// Function Pointer Types
// =============================================================================

// Memory allocation
typedef PVOID (NTAPI *FN_MmAllocateContiguousMemorySpecifyCacheNode)(
    U64 NumberOfBytes,
    U64 LowestAcceptableAddress,
    U64 HighestAcceptableAddress,
    U64 BoundaryAddressMultiple,
    U32 CacheType,
    U32 PreferredNode
);

typedef void (NTAPI *FN_MmFreeContiguousMemory)(PVOID BaseAddress);

typedef PVOID (NTAPI *FN_ExAllocatePool)(U32 PoolType, U64 NumberOfBytes);
typedef void (NTAPI *FN_ExFreePool)(PVOID P);

// Physical memory
typedef U64 (NTAPI *FN_MmGetPhysicalAddress)(PVOID BaseAddress);
typedef PVOID (NTAPI *FN_MmGetVirtualForPhysical)(U64 PhysicalAddress);

// IPI and processor
typedef U64 (NTAPI *FN_KeIpiGenericCall)(void* BroadcastFunction, U64 Context);
typedef U32 (NTAPI *FN_KeQueryActiveProcessorCountEx)(U16 GroupNumber);
typedef U32 (NTAPI *FN_KeGetCurrentProcessorNumberEx)(PPROCESSOR_NUMBER ProcNumber);
typedef NTSTATUS (NTAPI *FN_KeGetProcessorNumberFromIndex)(U32 Index, PPROCESSOR_NUMBER ProcNumber);
typedef void (NTAPI *FN_KeSetSystemGroupAffinityThread)(PGROUP_AFFINITY Affinity, PGROUP_AFFINITY PreviousAffinity);
typedef void (NTAPI *FN_KeRevertToUserGroupAffinityThread)(PGROUP_AFFINITY PreviousAffinity);

// Thread
typedef NTSTATUS (NTAPI *FN_PsCreateSystemThread)(
    HANDLE* ThreadHandle,
    U32 DesiredAccess,
    void* ObjectAttributes,
    HANDLE ProcessHandle,
    void* ClientId,
    void* StartRoutine,
    PVOID StartContext
);

typedef void (NTAPI *FN_PsTerminateSystemThread)(NTSTATUS ExitStatus);
typedef NTSTATUS (NTAPI *FN_ZwClose)(HANDLE Handle);

// Process
typedef PEPROCESS (NTAPI *FN_PsGetCurrentProcess)(void);
typedef U64 (NTAPI *FN_PsGetProcessId)(PEPROCESS Process);

// Module enumeration
typedef NTSTATUS (NTAPI *FN_ZwQuerySystemInformation)(
    U32 SystemInformationClass,
    PVOID SystemInformation,
    U32 SystemInformationLength,
    U32* ReturnLength
);

// Symbol resolution (used to bootstrap everything else)
typedef PVOID (NTAPI *FN_MmGetSystemRoutineAddress)(PUNICODE_STRING SystemRoutineName);

// =============================================================================
// Resolved Symbols Structure
// =============================================================================

typedef struct _KERNEL_SYMBOLS {
    // Memory
    FN_MmAllocateContiguousMemorySpecifyCacheNode MmAllocateContiguousMemorySpecifyCacheNode;
    FN_MmFreeContiguousMemory MmFreeContiguousMemory;
    FN_ExAllocatePool ExAllocatePool;
    FN_ExFreePool ExFreePool;
    FN_MmGetPhysicalAddress MmGetPhysicalAddress;
    FN_MmGetVirtualForPhysical MmGetVirtualForPhysical;

    // IPI / Processor
    FN_KeIpiGenericCall KeIpiGenericCall;
    FN_KeQueryActiveProcessorCountEx KeQueryActiveProcessorCountEx;
    FN_KeGetCurrentProcessorNumberEx KeGetCurrentProcessorNumberEx;
    FN_KeGetProcessorNumberFromIndex KeGetProcessorNumberFromIndex;
    FN_KeSetSystemGroupAffinityThread KeSetSystemGroupAffinityThread;
    FN_KeRevertToUserGroupAffinityThread KeRevertToUserGroupAffinityThread;

    // Thread
    FN_PsCreateSystemThread PsCreateSystemThread;
    FN_PsTerminateSystemThread PsTerminateSystemThread;
    FN_ZwClose ZwClose;

    // Process
    FN_PsGetCurrentProcess PsGetCurrentProcess;
    FN_PsGetProcessId PsGetProcessId;

    // System info
    FN_ZwQuerySystemInformation ZwQuerySystemInformation;

    // Bootstrap
    FN_MmGetSystemRoutineAddress MmGetSystemRoutineAddress;

    // Initialization status
    bool Initialized;
} KERNEL_SYMBOLS;

// =============================================================================
// Global Symbol Table
// =============================================================================

extern KERNEL_SYMBOLS g_KernelSymbols;

// =============================================================================
// Functions
// =============================================================================

// Initialize the symbol table by resolving all required kernel functions.
// This must be called early in hypervisor initialization.
// Returns STATUS_SUCCESS on success, error code on failure.
NTSTATUS KernelResolveSymbols(void);

// Helper to resolve a single symbol by name.
// Returns function pointer or NULL if not found.
PVOID KernelResolveSymbol(const wchar_t* symbolName);

// =============================================================================
// Memory Allocation Helpers
// =============================================================================

// Allocate page-aligned contiguous memory
PVOID KernelAllocateContiguous(U64 size);

// Allocate page-aligned non-contiguous memory
PVOID KernelAllocatePageAligned(U64 size);

// Free contiguous memory
void KernelFreeContiguous(PVOID ptr);

// Get physical address for virtual address
U64 KernelGetPhysicalAddress(PVOID virtualAddress);

// =============================================================================
// Processor Helpers
// =============================================================================

// Get number of active processors
U32 KernelGetProcessorCount(void);

// Execute callback on each processor
NTSTATUS KernelExecuteOnEachProcessor(NTSTATUS(*callback)(PVOID), PVOID context);

// =============================================================================
// EPROCESS Offset Discovery (Cross-Version Compatibility)
// =============================================================================

// Structure to hold discovered kernel offsets
typedef struct _KERNEL_OFFSETS {
    U64     UniqueProcessId;        // EPROCESS.UniqueProcessId offset
    U64     ActiveProcessLinks;     // EPROCESS.ActiveProcessLinks offset
    U64     DirectoryTableBase;     // EPROCESS.DirectoryTableBase (KPROCESS+0x28)
    U64     ImageFileName;          // EPROCESS.ImageFileName offset
    bool    Discovered;
} KERNEL_OFFSETS;

// Global offsets table
extern KERNEL_OFFSETS g_KernelOffsets;

// Discover EPROCESS offsets by pattern scanning kernel functions
// This should be called after KernelResolveSymbols() succeeds
NTSTATUS KernelDiscoverOffsets(void);

// Accessor functions for individual offsets
// These will auto-discover if not already done
U64 KernelGetUniqueProcessIdOffset(void);
U64 KernelGetActiveProcessLinksOffset(void);
U64 KernelGetDirectoryTableBaseOffset(void);
U64 KernelGetImageFileNameOffset(void);

#endif // KERNEL_RESOLVE_H
