// PayLoad/include/types.h
// Unified type definitions for both AMD and Intel backends
#pragma once

#include <intrin.h>
#include <xmmintrin.h>
#include <cstddef>
#include <basetsd.h>

// Windows types - conditional for VMXRoot freestanding context
#ifndef _VMXROOT_MODE
#include <Windows.h>
#include <ntstatus.h>
#else
//===----------------------------------------------------------------------===//
// VMXRoot Standalone Type Definitions (no CRT/Windows headers)
//===----------------------------------------------------------------------===//

typedef unsigned long       ULONG;
typedef unsigned long long  UINT64;
typedef unsigned long long  SIZE_T;
typedef unsigned long long  ULONG_PTR;
typedef void*               PVOID;
typedef unsigned char       BOOLEAN;
typedef void*               HANDLE;
typedef unsigned long       DWORD;
typedef unsigned long long  DWORD64;
typedef int                 BOOL;
typedef wchar_t*            PWCHAR;
typedef char*               PCHAR;
typedef unsigned char       UCHAR;
typedef unsigned short      USHORT;
typedef long long           LONGLONG;
typedef unsigned long long  ULONGLONG;
typedef BOOLEAN*            PBOOLEAN;
typedef unsigned short      WCHAR;
typedef const wchar_t*      LPCWSTR;
typedef const char*         LPCSTR;

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL  nullptr
#endif

// NTSTATUS definitions
typedef long NTSTATUS;
#define STATUS_SUCCESS              ((NTSTATUS)0x00000000L)
#define STATUS_UNSUCCESSFUL         ((NTSTATUS)0xC0000001L)
#define STATUS_NOT_IMPLEMENTED      ((NTSTATUS)0xC0000002L)
#define STATUS_INVALID_PARAMETER    ((NTSTATUS)0xC000000DL)
#define STATUS_ACCESS_DENIED        ((NTSTATUS)0xC0000022L)
#define STATUS_BUFFER_TOO_SMALL     ((NTSTATUS)0xC0000023L)

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

//===----------------------------------------------------------------------===//
// VMXRoot CRT Replacement Functions
// Using pragma intrinsic to provide our own implementations
//===----------------------------------------------------------------------===//

// Block vcruntime_string.h from declaring these - we provide our own
#ifndef _VCRUNTIME_STRING_H_DEFINED
#define _VCRUNTIME_STRING_H_DEFINED
#endif

#pragma function(memset)
#pragma function(memcpy)
#pragma function(memcmp)

// memset - fill memory with a value
extern "C" inline void* memset(void* dest, int val, unsigned long long count) {
    unsigned char* d = (unsigned char*)dest;
    while (count--) *d++ = (unsigned char)val;
    return dest;
}

// memcpy - copy memory
extern "C" inline void* memcpy(void* dest, const void* src, unsigned long long count) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    while (count--) *d++ = *s++;
    return dest;
}

// memcmp - compare memory
extern "C" inline int memcmp(const void* s1, const void* s2, unsigned long long count) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    while (count--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++; p2++;
    }
    return 0;
}

// min/max macros
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#endif

//===----------------------------------------------------------------------===//
// Base Integer Types
//===----------------------------------------------------------------------===//

using u8  = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;
using u128 = __m128;

using i8  = signed char;
using i16 = signed short;
using i32 = signed int;
using i64 = signed long long;

//===----------------------------------------------------------------------===//
// Address Types
//===----------------------------------------------------------------------===//

using guest_virt_t = u64;   // Guest virtual address
using guest_phys_t = u64;   // Guest physical address
using host_virt_t  = u64;   // Host virtual address (hypervisor)
using host_phys_t  = u64;   // Host physical address
using pfn_t = u64;          // Page frame number

//===----------------------------------------------------------------------===//
// Memory Mapping Types
//===----------------------------------------------------------------------===//

// Map type for dual-mapping operations (src/dest for memory copy)
enum class map_type_t
{
    map_src,
    map_dest
};

//===----------------------------------------------------------------------===//
// Constants
//===----------------------------------------------------------------------===//

#define VMEXIT_KEY 0xDEADBEEFDEADBEEF

constexpr u64 PAGE_4KB = 0x1000;
constexpr u64 PAGE_2MB = PAGE_4KB * 512;
constexpr u64 PAGE_1GB = PAGE_2MB * 512;
constexpr u64 PAGE_MASK = PAGE_4KB - 1;
constexpr u64 PFN_MASK = ~PAGE_MASK;

//===----------------------------------------------------------------------===//
// Utility Macros
//===----------------------------------------------------------------------===//

#define PAGE_ALIGN_DOWN(addr) ((addr) & PFN_MASK)
#define PAGE_ALIGN_UP(addr)   (((addr) + PAGE_MASK) & PFN_MASK)
#define PAGE_OFFSET(addr)     ((addr) & PAGE_MASK)
#define ADDR_TO_PFN(addr)     ((addr) >> 12)
#define PFN_TO_ADDR(pfn)      ((pfn) << 12)

//===----------------------------------------------------------------------===//
// Debug Output
//===----------------------------------------------------------------------===//

#define PORT_NUM_3 0x3E8
#define DBG_PRINT(arg) \
    __outbytestring(PORT_NUM_3, (unsigned char*)arg, sizeof arg);

//===----------------------------------------------------------------------===//
// Control Register Types
//===----------------------------------------------------------------------===//

typedef union _CR0
{
    struct
    {
        UINT64 ProtectionEnable : 1;        // PE
        UINT64 MonitorCoprocessor : 1;      // MP
        UINT64 EmulateCoprocessor : 1;      // EM
        UINT64 TaskSwitched : 1;            // TS
        UINT64 ExtensionType : 1;           // ET
        UINT64 NumericError : 1;            // NE
        UINT64 Reserved1 : 10;              // Bits 6-15
        UINT64 WriteProtect : 1;            // WP
        UINT64 Reserved2 : 1;               // Bit 17
        UINT64 AlignmentMask : 1;           // AM
        UINT64 Reserved3 : 10;              // Bits 19-28
        UINT64 NotWriteThrough : 1;         // NW
        UINT64 CacheDisable : 1;            // CD
        UINT64 Paging : 1;                  // PG
        UINT64 Reserved4 : 32;              // Upper 32 bits
    };
    UINT64 Flags;
} CR0;

typedef union _CR3
{
    struct
    {
        UINT64 Reserved1 : 3;
        UINT64 PageLevelWriteThrough : 1;
        UINT64 PageLevelCacheDisable : 1;
        UINT64 Reserved2 : 7;
        UINT64 AddressOfPageDirectory : 40;
        UINT64 Reserved3 : 12;
    };
    UINT64 Flags;
} CR3;

typedef union _CR4
{
    struct
    {
        UINT64 VirtualModeExtensions : 1;           // VME
        UINT64 ProtectedModeVirtualInterrupts : 1;  // PVI
        UINT64 TimeStampDisable : 1;                // TSD
        UINT64 DebuggingExtensions : 1;             // DE
        UINT64 PageSizeExtensions : 1;              // PSE
        UINT64 PhysicalAddressExtension : 1;        // PAE
        UINT64 MachineCheckEnable : 1;              // MCE
        UINT64 PageGlobalEnable : 1;                // PGE
        UINT64 PerformanceMonitoringCounter : 1;    // PCE
        UINT64 OsFxsaveFxrstorSupport : 1;          // OSFXSR
        UINT64 OsXmmExceptionSupport : 1;           // OSXMMEXCPT
        UINT64 UserModeInstructionPrevention : 1;   // UMIP
        UINT64 Reserved1 : 1;                       // Bit 12
        UINT64 VmxEnable : 1;                       // VMXE
        UINT64 SmxEnable : 1;                       // SMXE
        UINT64 Reserved2 : 1;                       // Bit 15
        UINT64 FsGsBaseEnable : 1;                  // FSGSBASE
        UINT64 PcidEnable : 1;                      // PCIDE
        UINT64 OsXsave : 1;                         // OSXSAVE
        UINT64 Reserved3 : 1;                       // Bit 19
        UINT64 SmepEnable : 1;                      // SMEP
        UINT64 SmapEnable : 1;                      // SMAP
        UINT64 ProtectionKeyEnable : 1;             // PKE
        UINT64 CETEnabled : 1;                      // CET
        UINT64 Reserved4 : 40;                      // Upper bits
    };
    UINT64 Flags;
} CR4;

// Alternative CR3 with pml4_pfn member for compatibility
union cr3 {
    u64 value;
    struct {
        u64 reserved1 : 3;
        u64 pwt : 1;
        u64 pcd : 1;
        u64 reserved2 : 7;
        u64 pml4_pfn : 40;
        u64 reserved3 : 12;
    };

    cr3() : value(0) {}
    cr3(u64 v) : value(v) {}

    u64 GetPfn() const { return pml4_pfn; }
    u64 GetPhysicalBase() const { return pml4_pfn << 12; }
};
