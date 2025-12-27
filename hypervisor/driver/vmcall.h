#ifndef OMBRA_DRIVER_VMCALL_H
#define OMBRA_DRIVER_VMCALL_H

#include "../shared/types.h"

// Phase 3 VMCALL Commands
// These extend the existing VMCALL interface defined in types.h
// Command codes use the same obfuscated hash-based scheme to avoid signatures

// CR3 watching for process tracking
#define VMCALL_WATCH_CR3            0x4A8E71B2ULL
#define VMCALL_UNWATCH_CR3          0x9F3C2D4EULL

// Physical page management
#define VMCALL_PIN_PAGE             0x7B1F4C93ULL
#define VMCALL_UNPIN_PAGE           0x2D9A6E17ULL
#define VMCALL_ALLOC_PHYSICAL_PAGE  0xC6E3A941ULL
#define VMCALL_FREE_PHYSICAL_PAGE   0x5F2B8D7CULL
#define VMCALL_COPY_PHYSICAL_PAGE   0x8A4E1CF5ULL

// EPT splitting for shadow pages
#define VMCALL_SPLIT_EPT_PAGE       0x3D7F9A21ULL
#define VMCALL_UNSPLIT_EPT_PAGE     0xB4C2E68DULL

// Virtual memory operations (cross-process)
// Note: VMCALL_READ_VIRT and VMCALL_WRITE_VIRT already defined in types.h

// Driver lifecycle
#define VMCALL_DRIVER_READY         0x6F1A8D39ULL
#define VMCALL_DRIVER_SHUTDOWN      0x1E9C4B7AULL

// Memory hiding (simplified interface)
#define VMCALL_HIDE_MEMORY          0xA3F5E219ULL
#define VMCALL_UNHIDE_MEMORY        0x4D2C7B8EULL

// VMCALL status codes (extended from types.h)
#define VMCALL_STATUS_SUCCESS           0
#define VMCALL_STATUS_INVALID_MAGIC     -1
#define VMCALL_STATUS_INVALID_COMMAND   -2
#define VMCALL_STATUS_INVALID_PARAM     -3
#define VMCALL_STATUS_NOT_IMPLEMENTED   -4
#define VMCALL_STATUS_ACCESS_DENIED     -5
#define VMCALL_STATUS_LIMIT_EXCEEDED    -6
#define VMCALL_STATUS_OUT_OF_MEMORY     -7

// VMCALL wrapper interface
// All functions return I64 status codes (negative = error)

// Initialize VMCALL interface with magic and key
void VmCallInit(U64 magic, U64 key);

// Raw VMCALL with 4 parameters
// RAX = magic, RCX = command, RDX = key, R8 = param1, R9 = param2, R10 = param3
I64 VmCall(U64 command, U64 param1, U64 param2, U64 param3);

// CR3 watching (for process tracking)
I64 VmWatchCr3(U64 cr3);
I64 VmUnwatchCr3(U64 cr3);

// Physical page management
I64 VmPinPage(U64 physicalAddr);
I64 VmUnpinPage(U64 physicalAddr);
I64 VmAllocPhysicalPage(U64* outPhysical);
I64 VmFreePhysicalPage(U64 physicalAddr);
I64 VmCopyPhysicalPage(U64 srcPhysical, U64 dstPhysical);

// EPT split for shadowing
I64 VmSplitEptPage(U64 targetCr3, U64 guestPhysical, U64 cleanPhysical);
I64 VmUnsplitEptPage(U64 targetCr3, U64 guestPhysical);

// Virtual memory access (cross-process)
I64 VmReadVirtual(U64 targetCr3, U64 va, void* buffer, U64 size);
I64 VmWriteVirtual(U64 targetCr3, U64 va, const void* buffer, U64 size);
I64 VmVirtToPhys(U64 targetCr3, U64 va, U64* outPhysical);

// Driver lifecycle
I64 VmDriverReady(U64 ownerCr3);
I64 VmDriverShutdown(void);

// Hiding
I64 VmHideMemory(U64 addr, U64 size);
I64 VmUnhideMemory(U64 addr, U64 size);

#endif
