#pragma once
#include <Windows.h>
#include <winternl.h>
#include "version_detect.h"  // For dynamic KTHREAD offset selection

namespace zerohvci {

// KTHREAD structure offsets (static, stable across versions)
#define KTHREAD_StackBase                       0x38
#define KTHREAD_KernelStack                     0x58

// KTHREAD->PreviousMode offsets by Windows version
// Note: These are still here for reference, but prefer using GetKThreadPreviousModeOffset()
constexpr ULONG KTHREAD_PreviousMode_Win10_1809 = 0x232;
constexpr ULONG KTHREAD_PreviousMode_Win10_1903 = 0x232;
constexpr ULONG KTHREAD_PreviousMode_Win10_2004 = 0x232;
constexpr ULONG KTHREAD_PreviousMode_Win10_21H1 = 0x232;
constexpr ULONG KTHREAD_PreviousMode_Win10_22H2 = 0x232;
constexpr ULONG KTHREAD_PreviousMode_Win11_21H2 = 0x232;
constexpr ULONG KTHREAD_PreviousMode_Win11_22H2 = 0x232;
constexpr ULONG KTHREAD_PreviousMode_Win11_23H2 = 0x232;

// Default fallback offset (stable across Windows 10/11 versions)
#define KTHREAD_PreviousMode_DEFAULT 0x232

// Dynamic offset lookup - prefer this over the static define
// Returns the PreviousMode offset for the current Windows version
inline ULONG GetKThreadPreviousModeOffset()
{
    // If version detection is initialized, use the database
    if (version::g_VersionInfo.BuildNumber != 0)
    {
        return version::GetKThreadPreviousModeOffset();
    }
    // Fallback to default
    return KTHREAD_PreviousMode_DEFAULT;
}

// Legacy define for backwards compatibility - uses dynamic lookup
#define KTHREAD_PreviousMode GetKThreadPreviousModeOffset()

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO
{
    USHORT UniqueProcessId;
    USHORT CreatorBackTraceIndex;
    UCHAR ObjectTypeIndex;
    UCHAR HandleAttributes;
    USHORT HandleValue;
    PVOID Object;
    ULONG GrantedAccess;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO, * PSYSTEM_HANDLE_TABLE_ENTRY_INFO;

typedef struct _SYSTEM_HANDLE_INFORMATION
{
    ULONG NumberOfHandles;
    SYSTEM_HANDLE_TABLE_ENTRY_INFO Handles[1];
} SYSTEM_HANDLE_INFORMATION, * PSYSTEM_HANDLE_INFORMATION;

typedef struct _KSYSTEM_PROCESS_INFORMATION
{
    ULONG NextEntryOffset;
    ULONG NumberOfThreads;
    LARGE_INTEGER WorkingSetPrivateSize;
    ULONG HardFaultCount;
    ULONG NumberOfThreadsHighWatermark;
    ULONGLONG CycleTime;
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER KernelTime;
    UNICODE_STRING ImageName;
    ULONG BasePriority;
    HANDLE UniqueProcessId;
    HANDLE InheritedFromUniqueProcessId;
    ULONG HandleCount;
    ULONG SessionId;
    ULONG_PTR UniqueProcessKey;
    SIZE_T PeakVirtualSize;
    SIZE_T VirtualSize;
    ULONG PageFaultCount;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
    SIZE_T PrivatePageCount;
    LARGE_INTEGER ReadOperationCount;
    LARGE_INTEGER WriteOperationCount;
    LARGE_INTEGER OtherOperationCount;
    LARGE_INTEGER ReadTransferCount;
    LARGE_INTEGER WriteTransferCount;
    LARGE_INTEGER OtherTransferCount;
    SYSTEM_THREAD_INFORMATION Threads[1];
} KSYSTEM_PROCESS_INFORMATION,
* PKSYSTEM_PROCESS_INFORMATION;

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
    HANDLE Section;
    PVOID MappedBase;
    PVOID ImageBase;
    ULONG ImageSize;
    ULONG Flags;
    USHORT LoadOrderIndex;
    USHORT InitOrderIndex;
    USHORT LoadCount;
    USHORT OffsetToFileName;
    UCHAR FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

typedef enum _KTHREAD_STATE
{
    Initialized,
    Ready,
    Running,
    Standby,
    Terminated,
    Waiting,
    Transition,
    DeferredReady,
    GateWaitObsolete,
    WaitingForProcessInSwap,
    MaximumThreadState
} KTHREAD_STATE,
* PKTHREAD_STATE;

typedef enum _POOL_TYPE {
    NonPagedPool,
    NonPagedPoolExecute,
    PagedPool,
    NonPagedPoolMustSucceed,
    DontUseThisType,
    NonPagedPoolCacheAligned,
    PagedPoolCacheAligned,
    NonPagedPoolCacheAlignedMustS,
    MaxPoolType,
    NonPagedPoolBase,
    NonPagedPoolBaseMustSucceed,
    NonPagedPoolBaseCacheAligned,
    NonPagedPoolBaseCacheAlignedMustS,
    NonPagedPoolSession,
    PagedPoolSession,
    NonPagedPoolMustSucceedSession,
    DontUseThisTypeSession,
    NonPagedPoolCacheAlignedSession,
    PagedPoolCacheAlignedSession,
    NonPagedPoolCacheAlignedMustSSession,
    NonPagedPoolNx,
    NonPagedPoolNxCacheAligned,
    NonPagedPoolSessionNx
} POOL_TYPE;

typedef struct _RTL_PROCESS_MODULES
{
    ULONG NumberOfModules;
    RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;

typedef NTSTATUS(NTAPI* PNtFsControlFile)(
    HANDLE FileHandle,
    HANDLE Event,
    PIO_APC_ROUTINE ApcRoutine,
    PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock,
    ULONG FsControlCode,
    PVOID InputBuffer,
    ULONG InputBufferLength,
    PVOID OutputBuffer,
    ULONG OutputBufferLength);

// FIX: Use SIZE_T/PSIZE_T instead of ULONG/PULONG for x64 compatibility
typedef NTSTATUS(NTAPI* pNtReadVirtualMemory)(
    HANDLE ProcessHandle,
    PVOID BaseAddress,
    PVOID Buffer,
    SIZE_T NumberOfBytesToRead,
    PSIZE_T NumberOfBytesRead
    );

typedef NTSTATUS(NTAPI* pNtWriteVirtualMemory)(
    HANDLE ProcessHandle,
    PVOID BaseAddress,
    PVOID Buffer,
    SIZE_T NumberOfBytesToWrite,
    PSIZE_T NumberOfBytesWritten
    );

// Global function pointers - use inline to avoid ODR violations
inline pNtReadVirtualMemory NtReadVirtualMemory = nullptr;
inline pNtWriteVirtualMemory NtWriteVirtualMemory = nullptr;
inline PNtFsControlFile NtFsControlFile = nullptr;

} // namespace zerohvci
