/**
 * @file nt_defs.h
 * @brief NT kernel structures and function pointer types
 *
 * These structures are needed for NtCreateFile (device open without DOS symlink),
 * NtLoadDriver (driver loading without SCM), and NtQuerySystemInformation.
 */

#ifndef BYOVD_NT_DEFS_H
#define BYOVD_NT_DEFS_H

#include "types.h"

//=============================================================================
// NTSTATUS Values
//=============================================================================

typedef LONG NTSTATUS;

#define STATUS_SUCCESS              ((NTSTATUS)0x00000000L)
#define STATUS_IMAGE_ALREADY_LOADED ((NTSTATUS)0xC000010EL)
#define STATUS_OBJECT_NAME_NOT_FOUND ((NTSTATUS)0xC0000034L)
#define STATUS_ACCESS_DENIED        ((NTSTATUS)0xC0000022L)
#define STATUS_INVALID_IMAGE_HASH   ((NTSTATUS)0xC0000428L)

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

//=============================================================================
// NT Structures
//=============================================================================

typedef struct _NT_UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} NT_UNICODE_STRING, *PNT_UNICODE_STRING;

typedef struct _NT_OBJECT_ATTRIBUTES {
    ULONG           Length;
    HANDLE          RootDirectory;
    PNT_UNICODE_STRING ObjectName;
    ULONG           Attributes;
    PVOID           SecurityDescriptor;
    PVOID           SecurityQualityOfService;
} NT_OBJECT_ATTRIBUTES, *PNT_OBJECT_ATTRIBUTES;

typedef struct _NT_IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID    Pointer;
    };
    ULONG_PTR Information;
} NT_IO_STATUS_BLOCK, *PNT_IO_STATUS_BLOCK;

// Object Attributes flags
#define OBJ_CASE_INSENSITIVE    0x00000040L

// File creation disposition
#define FILE_OPEN               0x00000001
#define FILE_NON_DIRECTORY_FILE 0x00000040

// InitializeObjectAttributes macro
#define InitializeObjectAttributes(p, n, a, r, s) { \
    (p)->Length = sizeof(NT_OBJECT_ATTRIBUTES);     \
    (p)->RootDirectory = r;                         \
    (p)->Attributes = a;                            \
    (p)->ObjectName = n;                            \
    (p)->SecurityDescriptor = s;                    \
    (p)->SecurityQualityOfService = NULL;           \
}

//=============================================================================
// NtQuerySystemInformation Structures
//=============================================================================

#define SystemModuleInformation     11
#define SystemBigPoolInformation    0x42  // 66

typedef struct _RTL_PROCESS_MODULE_INFORMATION {
    HANDLE Section;
    PVOID MappedBase;
    PVOID ImageBase;
    ULONG ImageSize;
    ULONG Flags;
    USHORT LoadOrderIndex;
    USHORT InitOrderIndex;
    USHORT LoadCount;
    USHORT OffsetToFileName;
    UCHAR  FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES {
    ULONG NumberOfModules;
    RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;

//=============================================================================
// BigPool Information (SystemBigPoolInformation = 0x42)
//=============================================================================

// Pool types
#define NonPagedPool        0
#define PagedPool           1
#define NonPagedPoolNx      512

// Big pool entry - returned by NtQuerySystemInformation(SystemBigPoolInformation)
typedef struct _SYSTEM_BIGPOOL_ENTRY {
    union {
        PVOID VirtualAddress;
        ULONG_PTR NonPaged : 1;  // Low bit indicates NonPagedPool
    };
    SIZE_T SizeInBytes;
    union {
        UCHAR Tag[4];
        ULONG TagUlong;
    };
} SYSTEM_BIGPOOL_ENTRY, *PSYSTEM_BIGPOOL_ENTRY;

typedef struct _SYSTEM_BIGPOOL_INFORMATION {
    ULONG Count;
    SYSTEM_BIGPOOL_ENTRY AllocatedInfo[1];  // Variable length array
} SYSTEM_BIGPOOL_INFORMATION, *PSYSTEM_BIGPOOL_INFORMATION;

//=============================================================================
// NTDLL Function Pointer Types
//=============================================================================

// NtCreateFile - Open device without DOS symlink
typedef NTSTATUS (NTAPI *PFN_NtCreateFile)(
    PHANDLE FileHandle,
    ACCESS_MASK DesiredAccess,
    PNT_OBJECT_ATTRIBUTES ObjectAttributes,
    PNT_IO_STATUS_BLOCK IoStatusBlock,
    PLARGE_INTEGER AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    PVOID EaBuffer,
    ULONG EaLength
);

// NtLoadDriver - Load driver without SCM
typedef NTSTATUS (NTAPI *PFN_NtLoadDriver)(
    PNT_UNICODE_STRING DriverServiceName
);

// NtUnloadDriver - Unload driver
typedef NTSTATUS (NTAPI *PFN_NtUnloadDriver)(
    PNT_UNICODE_STRING DriverServiceName
);

// RtlInitUnicodeString - Initialize UNICODE_STRING
typedef VOID (NTAPI *PFN_RtlInitUnicodeString)(
    PNT_UNICODE_STRING DestinationString,
    PCWSTR SourceString
);

// NtQuerySystemInformation - Get system information
typedef NTSTATUS (NTAPI *PFN_NtQuerySystemInformation)(
    ULONG SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
);

// RtlNtStatusToDosError - Convert NTSTATUS to Win32 error
typedef ULONG (NTAPI *PFN_RtlNtStatusToDosError)(
    NTSTATUS Status
);

//=============================================================================
// NTDLL Function Resolver
//=============================================================================

// Global function pointers (initialized in nt_helpers.c)
extern PFN_NtCreateFile             g_NtCreateFile;
extern PFN_NtLoadDriver             g_NtLoadDriver;
extern PFN_NtUnloadDriver           g_NtUnloadDriver;
extern PFN_RtlInitUnicodeString     g_RtlInitUnicodeString;
extern PFN_NtQuerySystemInformation g_NtQuerySystemInformation;
extern PFN_RtlNtStatusToDosError    g_RtlNtStatusToDosError;

// Initialize NTDLL functions
bool NtInit(void);

// Get driver base address via NtQuerySystemInformation
UINT64 NtGetDriverBase(const wchar_t* wszDriverName);

// Open device using NtCreateFile (required for drivers without DOS symlink)
HANDLE NtOpenDevice(const wchar_t* wszNtDevicePath);

#endif // BYOVD_NT_DEFS_H
