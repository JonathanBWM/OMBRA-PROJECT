#ifndef PE_IMPORTS_H
#define PE_IMPORTS_H

#include <stdint.h>
#include <stdbool.h>
#include "pe_parser.h"
#include "../driver_interface.h"

// Common kernel symbols that OmbraDriver might need
typedef struct _COMMON_SYMBOLS {
    uint64_t ExAllocatePoolWithTag;
    uint64_t ExFreePoolWithTag;
    uint64_t MmGetSystemRoutineAddress;
    uint64_t IoCreateDevice;
    uint64_t IoDeleteDevice;
    uint64_t IoCreateSymbolicLink;
    uint64_t IoDeleteSymbolicLink;
    uint64_t RtlInitUnicodeString;
    uint64_t DbgPrint;
    uint64_t KeQueryActiveProcessorCountEx;
    uint64_t KeGetCurrentProcessorNumberEx;
    uint64_t PsGetCurrentProcessId;
    uint64_t PsGetCurrentThreadId;
    uint64_t PsLookupProcessByProcessId;
    uint64_t ObDereferenceObject;
    uint64_t ZwQuerySystemInformation;
    uint64_t MmMapIoSpace;
    uint64_t MmUnmapIoSpace;
} COMMON_SYMBOLS;

bool ResolveImports(DRV_CONTEXT* drv, PE_INFO* peInfo);
bool ResolveCommonSymbols(DRV_CONTEXT* drv, COMMON_SYMBOLS* syms);

#endif
