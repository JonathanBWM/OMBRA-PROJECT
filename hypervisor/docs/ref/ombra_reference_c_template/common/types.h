/**
 * types.h - Common type definitions for OmbraHypervisor
 * 
 * Pure C11 - NO C++ constructs
 * MSVC compatible
 */

#ifndef OMBRA_TYPES_H
#define OMBRA_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/*===========================================================================
 * Basic Integer Types
 *===========================================================================*/

typedef uint8_t   U8;
typedef uint16_t  U16;
typedef uint32_t  U32;
typedef uint64_t  U64;

typedef int8_t    I8;
typedef int16_t   I16;
typedef int32_t   I32;
typedef int64_t   I64;

/*===========================================================================
 * Address Types
 *===========================================================================*/

typedef U64 GVA;    /* Guest Virtual Address */
typedef U64 GPA;    /* Guest Physical Address */
typedef U64 HVA;    /* Host Virtual Address */
typedef U64 HPA;    /* Host Physical Address */

/*===========================================================================
 * Common Constants
 *===========================================================================*/

#define PAGE_SIZE       0x1000ULL
#define PAGE_SHIFT      12
#define PAGE_MASK       (PAGE_SIZE - 1)

#define KB(x)           ((x) * 1024ULL)
#define MB(x)           ((x) * 1024ULL * 1024ULL)
#define GB(x)           ((x) * 1024ULL * 1024ULL * 1024ULL)

#define BIT(n)          (1ULL << (n))

/*===========================================================================
 * Alignment Macros
 *===========================================================================*/

#define ALIGN_UP(x, a)      (((x) + ((a) - 1)) & ~((a) - 1))
#define ALIGN_DOWN(x, a)    ((x) & ~((a) - 1))
#define IS_ALIGNED(x, a)    (((x) & ((a) - 1)) == 0)

/*===========================================================================
 * Status Codes
 *===========================================================================*/

typedef enum _OMBRA_STATUS {
    OMBRA_SUCCESS = 0,
    OMBRA_ERROR_NOT_SUPPORTED,
    OMBRA_ERROR_VMX_DISABLED,
    OMBRA_ERROR_ALREADY_RUNNING,
    OMBRA_ERROR_VMXON_FAILED,
    OMBRA_ERROR_VMCS_FAILED,
    OMBRA_ERROR_VMLAUNCH_FAILED,
    OMBRA_ERROR_INVALID_PARAM,
    OMBRA_ERROR_NO_MEMORY,
    OMBRA_ERROR_PAGE_NOT_PRESENT,
    OMBRA_ERROR_ACCESS_DENIED,
} OMBRA_STATUS;

#define OMBRA_SUCCESS(s)    ((s) == OMBRA_SUCCESS)
#define OMBRA_FAILED(s)     ((s) != OMBRA_SUCCESS)

/*===========================================================================
 * VMX Instruction Results
 *===========================================================================*/

typedef enum _VMX_RESULT {
    VMX_OK = 0,                 /* Success */
    VMX_FAIL_INVALID = 1,       /* Fail with no error code */
    VMX_FAIL_VALID = 2,         /* Fail with error code in VMCS */
} VMX_RESULT;

/*===========================================================================
 * CPU Vendor Detection
 *===========================================================================*/

typedef enum _CPU_VENDOR {
    CPU_VENDOR_UNKNOWN = 0,
    CPU_VENDOR_INTEL,
    CPU_VENDOR_AMD,
} CPU_VENDOR;

/*===========================================================================
 * Guest Register Context
 * 
 * Layout must match assembly save/restore order exactly!
 *===========================================================================*/

typedef struct _GUEST_REGS {
    U64 Rax;
    U64 Rcx;
    U64 Rdx;
    U64 Rbx;
    U64 Rsp;    /* Not used - RSP is in VMCS */
    U64 Rbp;
    U64 Rsi;
    U64 Rdi;
    U64 R8;
    U64 R9;
    U64 R10;
    U64 R11;
    U64 R12;
    U64 R13;
    U64 R14;
    U64 R15;
} GUEST_REGS;

/*===========================================================================
 * Segment Descriptor
 *===========================================================================*/

typedef struct _SEGMENT_DESCRIPTOR {
    U16 Selector;
    U64 Base;
    U32 Limit;
    U32 AccessRights;
} SEGMENT_DESCRIPTOR;

/*===========================================================================
 * Per-CPU VMX State
 *===========================================================================*/

#define MAX_CPUS 256

typedef struct _VMX_CPU {
    /* VMX Region Pointers */
    void*   VmxonRegion;        /* Virtual address of VMXON region */
    U64     VmxonPhysical;      /* Physical address of VMXON region */
    void*   VmcsRegion;         /* Virtual address of VMCS region */
    U64     VmcsPhysical;       /* Physical address of VMCS region */
    
    /* State Flags */
    bool    VmxEnabled;         /* VMXON executed successfully */
    bool    VmcsActive;         /* VMCS is current on this CPU */
    bool    Launched;           /* VMLAUNCH executed (use VMRESUME) */
    
    /* Statistics */
    U64     VmexitCount;        /* Total VMExits on this CPU */
    
    /* Timing Compensation */
    I64     TscOffset;          /* Current TSC offset for stealth */
    
    /* CPU Identification */
    U32     ApicId;             /* APIC ID of this CPU */
    U32     CpuIndex;           /* Index in global CPU array */
    
} VMX_CPU;

/*===========================================================================
 * Global Hypervisor State
 *===========================================================================*/

typedef struct _OMBRA_STATE {
    /* Per-CPU Contexts */
    VMX_CPU*    Cpus[MAX_CPUS];
    U32         CpuCount;
    
    /* EPT State */
    void*       EptState;       /* Pointer to EPT_STATE */
    
    /* Hook Manager */
    void*       HookManager;    /* Pointer to HOOK_MANAGER */
    
    /* Global Flags */
    bool        Initialized;
    bool        Running;
    
} OMBRA_STATE;

extern OMBRA_STATE g_Ombra;

/*===========================================================================
 * Compiler Intrinsics (MSVC)
 * 
 * These are provided by <intrin.h> in MSVC
 *===========================================================================*/

#ifdef _MSC_VER
#include <intrin.h>
#else
/* For non-MSVC compilers, declare intrinsics manually */
void __cpuid(int[4], int);
void __cpuidex(int[4], int, int);
U64  __readmsr(U32);
void __writemsr(U32, U64);
U64  __readcr0(void);
U64  __readcr3(void);
U64  __readcr4(void);
void __writecr0(U64);
void __writecr3(U64);
void __writecr4(U64);
U64  __rdtsc(void);
void _mm_pause(void);
long _InterlockedCompareExchange(volatile long*, long, long);
long _InterlockedExchange(volatile long*, long);
#endif

#endif /* OMBRA_TYPES_H */
