#ifndef OMBRA_TYPES_H
#define OMBRA_TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t  U8;  typedef int8_t  I8;
typedef uint16_t U16; typedef int16_t I16;
typedef uint32_t U32; typedef int32_t I32;
typedef uint64_t U64; typedef int64_t I64;

typedef U64 GPA, GVA, HPA, HVA;

#define PAGE_SIZE   0x1000ULL
#define PAGE_SHIFT  12
#define BIT(n)      (1ULL << (n))

#define ALIGN_UP(x, a)   (((x) + ((a)-1)) & ~((a)-1))
#define ALIGN_DOWN(x, a) ((x) & ~((a)-1))

typedef enum {
    OMBRA_SUCCESS = 0,
    OMBRA_ERROR_NOT_SUPPORTED,
    OMBRA_ERROR_VMX_DISABLED,
    OMBRA_ERROR_ALREADY_RUNNING,
    OMBRA_ERROR_VMXON_FAILED,
    OMBRA_ERROR_VMCS_FAILED,
    OMBRA_ERROR_VMLAUNCH_FAILED,
    OMBRA_ERROR_INVALID_PARAM,
    OMBRA_ERROR_NO_MEMORY,
    OMBRA_ERROR_OUT_OF_MEMORY = OMBRA_ERROR_NO_MEMORY,  // Alias
    OMBRA_ERROR_NOT_FOUND,
    OMBRA_ERROR_INVALID_STATE,
    OMBRA_ERROR_INVALID_OPERATION = OMBRA_ERROR_INVALID_STATE,  // Alias
    OMBRA_ERROR_NOT_IMPLEMENTED,
    OMBRA_ERROR_DRIVER_INIT_FAILED,
    OMBRA_ERROR_NOT_INITIALIZED,
} OMBRA_STATUS;

#define OMBRA_SUCCESS(s) ((s) == OMBRA_SUCCESS)
#define OMBRA_FAILED(s)  ((s) != OMBRA_SUCCESS)

// =============================================================================
// Magic Value Obfuscation (Kernel-side)
// =============================================================================
// Prevents signature scanning of ASCII magic values in kernel code.
// Uses compile-time XOR obfuscation with runtime deobfuscation.

#define OMBRA_OBF_KEY 0x9C7A3B5E2D1F4E8AULL

// Runtime deobfuscation helper
static inline U64 ombra_deobf_magic(U64 obfuscated) {
    return obfuscated ^ OMBRA_OBF_KEY;
}

// Pre-obfuscated magic values (original XOR OMBRA_OBF_KEY)
// VMCALL magic: "OMBRALL\0" = 0x4F4D4252414C4C00
// 0x4F4D4252414C4C00 ^ 0x9C7A3B5E2D1F4E8A = 0xD337790C6C53028A
#define OMBRA_VMCALL_MAGIC_OBF  0xD337790C6C53028AULL

// HOOK magic: "HOOKHOOK" = 0x484F4F4B484F4F4B
// 0x484F4F4B484F4F4B ^ 0x9C7A3B5E2D1F4E8A = 0xD4357415655001C1
#define OMBRA_HOOK_MAGIC_OBF    0xD4357415655001C1ULL

// Command code obfuscation: hash-based values instead of sequential
// Use upper 32 bits of FNV-1a hash to avoid patterns
#define VMCALL_CMD(name_hash) (name_hash)

// Pre-computed command hashes (FNV-1a hash >> 32 to get non-sequential values)
#define VMCALL_PING             0x7C9E2A8FULL
#define VMCALL_UNLOAD           0x3F7B8D21ULL
#define VMCALL_GET_STATUS       0x92C4E6F3ULL
#define VMCALL_HOOK_INSTALL     0x5A3F9B27ULL
#define VMCALL_HOOK_REMOVE      0xB8D12E95ULL
#define VMCALL_HOOK_LIST        0x41F7C3A6ULL
#define VMCALL_READ_PHYS        0xE2A849D7ULL
#define VMCALL_WRITE_PHYS       0x6C3DF52BULL
#define VMCALL_VIRT_TO_PHYS     0xA971D8F4ULL
#define VMCALL_NESTED_GET_INFO  0x1E65AB3CULL
#define VMCALL_NESTED_ENABLE    0xD7F246E8ULL
#define VMCALL_NESTED_DISABLE   0x4B829CF1ULL

// Phase 2: Virtual memory access with cross-process support
#define VMCALL_READ_VIRT        0xD4F83A19ULL
#define VMCALL_WRITE_VIRT       0x8B2E57C6ULL
#define VMCALL_GET_PROCESS_CR3  0x2F6E9A7CULL

// NMI blocking for EAC evasion
// Block NMIs for specific CR3s to evade anti-cheat integrity checks
#define VMCALL_NMI_BLOCK_CR3    0x73AE8F15ULL
#define VMCALL_NMI_UNBLOCK_CR3  0x5D2B6C49ULL
#define VMCALL_NMI_ENABLE       0x9F14D8A2ULL
#define VMCALL_NMI_DISABLE      0xE6C35B7DULL

// Hardware spoofing via EPT shadow pages
// SMBIOS spoofing: shadow WmipSMBiosTablePhysicalAddress to randomize serials
#define VMCALL_SPOOF_SMBIOS     0xB4E21C87ULL  // Setup SMBIOS shadow page
#define VMCALL_SPOOF_REMOVE     0x6A5F3D92ULL  // Remove spoof

// Disk/NIC hardware fingerprint spoofing
// Intercepts storage IOCTLs and network OIDs to return spoofed identifiers
#define VMCALL_SPOOF_INIT       0xC7A3E518ULL  // Initialize spoof manager
#define VMCALL_SPOOF_ADD_DISK   0x39F8B2D1ULL  // Add disk serial spoof
#define VMCALL_SPOOF_DEL_DISK   0xE5D4A726ULL  // Remove disk serial spoof
#define VMCALL_SPOOF_ADD_NIC    0x8B61F4C9ULL  // Add NIC MAC spoof
#define VMCALL_SPOOF_DEL_NIC    0x4E27D8A3ULL  // Remove NIC MAC spoof
#define VMCALL_SPOOF_QUERY      0xF1829C5BULL  // Query spoof status

// Phase 2 VMCALLs - Driver Mapper
#define VMCALL_CLAIM_POOL_REGION    0x3000ULL
#define VMCALL_RELEASE_POOL_REGION  0x3001ULL
#define VMCALL_FINALIZE_DRIVER_LOAD 0x3010ULL
#define VMCALL_DRIVER_PING          0x3011ULL
#define VMCALL_CALL_DRIVER_UNLOAD   0x3012ULL
#define VMCALL_HIDE_MEMORY_RANGE    0x3020ULL
#define VMCALL_UNHIDE_MEMORY_RANGE  0x3021ULL
#define VMCALL_ALLOC_MDL_MEMORY     0x3030ULL
#define VMCALL_POLL_ALLOC_STATUS    0x3031ULL
#define VMCALL_FREE_MDL_MEMORY      0x3032ULL

// Path B Option 2: EPT-only memory - hypervisor-managed mappings invisible to Windows
#define VMCALL_CREATE_EPT_MAPPING   0x3040ULL
#define VMCALL_DESTROY_EPT_MAPPING  0x3041ULL
#define VMCALL_SET_EPT_PERMISSIONS  0x3042ULL

// Anti-detection: Memory query spoofing
// EAC uses NtQueryVirtualMemory(MemoryWorkingSetExInformation) to detect EPT memory
// SharedCount==0 and Shared==0 indicates non-backed executable memory
#define VMCALL_REGISTER_EPT_RANGE   0x3050ULL  // Register EPT range for spoof
#define VMCALL_UNREGISTER_EPT_RANGE 0x3051ULL  // Unregister EPT range

// EPT mapping flags
#define EPT_MAP_FLAG_STEALTH        (1 << 0)  // Use execute-only for .text
#define EPT_MAP_FLAG_AUTO_VA        (1 << 1)  // Auto-find kernel VA range
#define EPT_MAP_FLAG_AUTO_PHYS      (1 << 2)  // Allocate physical pages from HV pool

// VMCALL_CREATE_EPT_MAPPING structures
typedef struct _VMCALL_EPT_MAP_IN {
    U64     RequestedVA;    // Desired kernel VA (0 = auto-find)
    U64     PhysicalAddr;   // Physical pages (0 = allocate from HV pool)
    U64     Size;           // Bytes to map (page-aligned)
    U32     Permissions;    // Initial EPT permissions (EPT_READ|EPT_WRITE|EPT_EXECUTE)
    U32     Flags;          // EPT_MAP_FLAG_*
} VMCALL_EPT_MAP_IN;

typedef struct _VMCALL_EPT_MAP_OUT {
    U64     KernelVA;       // Resulting kernel virtual address
    U64     PhysicalAddr;   // Physical base address (for direct writes)
    U32     Status;         // OMBRA_STATUS code
    U32     PagesAllocated; // Number of 4KB pages allocated
} VMCALL_EPT_MAP_OUT;

// VMCALL_DESTROY_EPT_MAPPING structures
typedef struct _VMCALL_EPT_UNMAP_IN {
    U64     KernelVA;       // VA to unmap
    U64     Size;           // Size to unmap
    U32     Flags;          // Reserved
} VMCALL_EPT_UNMAP_IN;

// VMCALL_SET_EPT_PERMISSIONS structures
typedef struct _VMCALL_EPT_PERM_IN {
    U64     KernelVA;       // Start VA
    U64     Size;           // Size in bytes
    U32     Permissions;    // New EPT permissions
    U32     Flags;          // Reserved
} VMCALL_EPT_PERM_IN;

// Windows kernel offsets (for EPROCESS walking)
// These vary by Windows version - detect at runtime or hardcode for target
// NOTE: Use U32 for consistency with EPROCESS_OFFSETS_EX and future versions
typedef struct {
    U32 ActiveProcessLinks;     // Offset to LIST_ENTRY in EPROCESS
    U32 UniqueProcessId;        // Offset to PID in EPROCESS
    U32 DirectoryTableBase;     // Offset to CR3 in EPROCESS (usually 0x28)
    U32 ImageFileName;          // Offset to process name (15 chars)
} EPROCESS_OFFSETS;

// Common Windows 10/11 offsets (build 19041+)
#define EPROCESS_ACTIVEPROCESSLINKS_OFFSET  0x448
#define EPROCESS_UNIQUEPROCESSID_OFFSET     0x440
#define EPROCESS_DIRECTORYTABLEBASE_OFFSET  0x028
#define EPROCESS_IMAGEFILENAME_OFFSET       0x5A8

// =============================================================================
// Bootstrap Structure (Phase 1)
// Placed in .ombra section, patched by loader before hypervisor init
// =============================================================================

typedef struct _OMBRA_BOOTSTRAP {
    U64     Magic;          // 0x524D424F ('OMBR')
    U64     Version;        // 1
    U64     ParamsPtr;      // Loader patches this with R0 address of HV_INIT_PARAMS
    U64     Reserved[5];
} OMBRA_BOOTSTRAP;

// =============================================================================
// Hypervisor Initialization Parameters (Phase 1)
// Passed from loader to hypervisor via .ombra bootstrap section
// =============================================================================

typedef struct _HV_INIT_PARAMS {
    U64     Magic;              // 0x4F4D4252 ('OMBR')
    U32     Version;            // 1
    U32     CpuCount;

    // Per-CPU regions
    void*   VmxonRegionsVirt;
    U64     VmxonRegionsPhys;
    void*   VmcsRegionsVirt;
    U64     VmcsRegionsPhys;
    void*   HostStacksBase;
    U32     HostStackSize;
    U32     _Pad1;

    // Shared structures
    void*   MsrBitmapVirt;
    U64     MsrBitmapPhys;
    void*   EptTablesVirt;
    U64     EptTablesPhys;
    U32     EptTablesPages;
    U32     _Pad2;

    // Debug
    void*   DebugBufferVirt;
    U64     DebugBufferPhys;
    U64     DebugBufferSize;

    // Kernel symbols
    U64     KeIpiGenericCall;
    U64     KeQueryActiveProcessorCountEx;
    U64     KeGetCurrentProcessorNumberEx;

    // VMX MSRs
    U64     VmxBasic;
    U64     VmxPinbasedCtls;
    U64     VmxProcbasedCtls;
    U64     VmxProcbasedCtls2;
    U64     VmxExitCtls;
    U64     VmxEntryCtls;
    U64     VmxTruePinbasedCtls;
    U64     VmxTrueProcbasedCtls;
    U64     VmxTrueExitCtls;
    U64     VmxTrueEntryCtls;
    U64     VmxCr0Fixed0;
    U64     VmxCr0Fixed1;
    U64     VmxCr4Fixed0;
    U64     VmxCr4Fixed1;
    U64     VmxEptVpidCap;

    // Auth
    U64     VmcallKey;

    // Self-protection (memhv-style memory hiding)
    U64     HvPhysBase;         // Physical address where HV is mapped
    U64     HvPhysSize;         // Size of HV in bytes (page-aligned)
    U64     BlankPagePhys;      // Physical address of zeroed page for hiding

    // Flags
    U32     Flags;
    U32     Reserved;
} HV_INIT_PARAMS;

// =============================================================================
// Phase 2 VMCALL Structures - Driver Mapper
// =============================================================================

// Status codes for asynchronous allocations
#define ALLOC_STATUS_PENDING   0
#define ALLOC_STATUS_COMPLETE  1
#define ALLOC_STATUS_FAILED    2

// Finalization flags
#define FINALIZE_FLAG_HIDE     (1 << 0)  // Hide driver memory via EPT
#define FINALIZE_FLAG_NO_ENTRY (1 << 1)  // Skip DriverEntry call

// VMCALL_CLAIM_POOL_REGION structures
typedef struct _VMCALL_CLAIM_POOL_IN {
    U32     Offset;     // Offset within pool to claim
    U32     Size;       // Size to claim in bytes
} VMCALL_CLAIM_POOL_IN;

typedef struct _VMCALL_CLAIM_POOL_OUT {
    U64     KernelVA;   // Kernel virtual address of claimed region
    U32     Status;     // OMBRA_STATUS code
} VMCALL_CLAIM_POOL_OUT;

// VMCALL_FINALIZE_DRIVER_LOAD structures
typedef struct _VMCALL_FINALIZE_IN {
    U64     KernelVA;       // Base address of mapped driver
    U32     Size;           // Total driver size
    U32     EntryRva;       // RVA to DriverEntry
    U32     TextRva;        // RVA to .text section
    U32     TextSize;       // Size of .text section
    U32     Flags;          // FINALIZE_FLAG_*
} VMCALL_FINALIZE_IN;

typedef struct _VMCALL_FINALIZE_OUT {
    U32     Status;             // OMBRA_STATUS code
    U32     DriverEntryResult;  // NTSTATUS from DriverEntry
} VMCALL_FINALIZE_OUT;

// VMCALL_HIDE_MEMORY_RANGE / VMCALL_UNHIDE_MEMORY_RANGE structures
typedef struct _VMCALL_HIDE_RANGE_IN {
    U64     Address;    // Virtual address to hide
    U64     Size;       // Size in bytes
} VMCALL_HIDE_RANGE_IN;

// VMCALL_ALLOC_MDL_MEMORY structures
typedef struct _VMCALL_ALLOC_MDL_IN {
    U32     Size;       // Size to allocate
    U32     Flags;      // Reserved for future use
} VMCALL_ALLOC_MDL_IN;

typedef struct _VMCALL_ALLOC_MDL_OUT {
    U32     RequestId;  // Unique request ID for polling
    U32     Status;     // ALLOC_STATUS_*
} VMCALL_ALLOC_MDL_OUT;

// VMCALL_POLL_ALLOC_STATUS structures
typedef struct _VMCALL_POLL_ALLOC_IN {
    U32     RequestId;  // Request ID from VMCALL_ALLOC_MDL_MEMORY
} VMCALL_POLL_ALLOC_IN;

typedef struct _VMCALL_POLL_ALLOC_OUT {
    U32     Status;         // ALLOC_STATUS_*
    U64     KernelVA;       // Kernel virtual address (if complete)
    U64     PhysicalAddr;   // Physical address (if complete)
} VMCALL_POLL_ALLOC_OUT;

// =============================================================================
// Phase 3: OmbraDriver Kernel Component
// =============================================================================

// Command IDs for shared memory ring
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

// Command ring status codes
typedef enum _OMBRA_CMD_STATUS {
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
} OMBRA_CMD_STATUS;

// Command ring configuration
#define OMBRA_RING_SIZE           64
#define OMBRA_RING_HEADER_SIZE    192
#define OMBRA_COMMAND_SIZE        512
#define OMBRA_RESPONSE_SIZE       256
#define OMBRA_RING_MAGIC          0x524E474F4D425241ULL  // "ARMBOGNR"

// Command ring header (192 bytes, cache-line aligned)
typedef struct _OMBRA_COMMAND_RING {
    // Producer side (usermode writes) - cache line 0
    volatile U32    ProducerIndex;
    U8              _pad0[60];

    // Consumer side (driver writes) - cache line 1
    volatile U32    ConsumerIndex;
    U8              _pad1[60];

    // Read-mostly fields - cache line 2
    U32             RingSize;
    U32             CommandSize;
    U32             ResponseSize;
    U32             _pad2;
    U64             Magic;
    U64             ScratchBufferOffset;
    U64             ScratchBufferSize;
    U8              _pad3[24];

    // Commands[RingSize] follows at offset 192
    // Responses[RingSize] follows commands
} OMBRA_COMMAND_RING, *POMBRA_COMMAND_RING;

// Command structure (512 bytes)
typedef struct _OMBRA_COMMAND {
    U32             CommandId;          // OMBRA_CMD_*
    U32             Flags;
    U64             SequenceId;         // For matching responses

    union {
        // Process operations
        struct {
            U64     Cr3;
            U64     Pid;
            char    ImageName[64];
        } Process;

        // Memory operations
        struct {
            U64     Cr3;
            U64     Address;
            U64     Size;
            U32     Protection;
            char    ModuleName[64];
        } Memory;

        // Physical memory access
        struct {
            U64     PhysicalAddress;
            U64     Size;
            U64     ScratchOffset;
        } PhysicalMem;

        // Virtual memory access
        struct {
            U64     Cr3;
            U64     VirtualAddress;
            U64     Size;
            U64     ScratchOffset;
        } VirtualMem;

        // Injection operations
        struct {
            U64     Pid;
            U64     ScratchOffset;
            U64     ImageSize;
            U32     Flags;
        } Inject;

        // Window operations
        struct {
            U64     Hwnd;
            char    ProcessName[64];
        } Window;

        // Enumeration
        struct {
            U32     Flags;
            U32     _pad;
            U64     ScratchOffset;
            U64     ScratchSize;
        } Enumerate;

        // ETW operations
        struct {
            U32     Operation;
            U32     ProviderId;
            U64     BufferAddress;
        } Etw;

        // Spoofing
        struct {
            U32     SpoofType;
            U8      OriginalValue[64];
            U8      SpoofedValue[64];
            U64     TargetDriver;
        } Spoof;

        // Protection
        struct {
            U64     Cr3;
            U32     Method;
            U32     AccessMask;
        } Protection;

        // Score configuration
        struct {
            U32     Operation;
            U32     EventMask;
            U32     Threshold;
        } Score;

        U8          Raw[448];
    };
} OMBRA_COMMAND, *POMBRA_COMMAND;

// Response structure (256 bytes)
typedef struct _OMBRA_RESPONSE {
    volatile U32    Ready;
    I32             Status;
    U64             SequenceId;

    union {
        // Process info
        struct {
            U64     Cr3;
            U64     Peb;
            U64     Eprocess;
            U64     ImageBase;
            U32     Pid;
            U8      Dead;
            U8      DllInjected;
            U8      _pad[2];
        } ProcessInfo;

        // Memory result
        struct {
            U64     MappedBase;
            U64     MappedSize;
            U64     BytesTransferred;
        } MemoryResult;

        // Identity map
        struct {
            U64     IdentityBase;
        } IdentityMap;

        // Status info
        struct {
            U32     DriverVersion;
            U32     _pad;
            U64     Uptime;
            U64     CommandsProcessed;
            U64     VmexitCount;
            U32     ActiveCpus;
            U32     HooksInstalled;
        } StatusInfo;

        // Ping response
        struct {
            U32     Response;
        } Ping;

        U8          Raw[224];
    };
} OMBRA_RESPONSE, *POMBRA_RESPONSE;

// Extended EPROCESS offsets for Phase 3
typedef struct _EPROCESS_OFFSETS_EX {
    U32     ActiveProcessLinks;
    U32     DirectoryTableBase;
    U32     UniqueProcessId;
    U32     ImageFileName;
    U32     Peb;
    U32     Wow64Process;
    U32     ObjectTable;
    U32     Token;
    U32     PebImageBaseAddress;
    U32     PebLdr;
} EPROCESS_OFFSETS_EX;

// Resolved kernel function pointers
typedef struct _OMBRA_KERNEL_IMPORTS {
    void*   PsGetCurrentProcess;
    void*   PsLookupProcessByProcessId;
    void*   ObReferenceObject;
    void*   ObDereferenceObject;
    void*   KeStackAttachProcess;
    void*   KeUnstackDetachProcess;
    void*   PsGetProcessPeb;
    void*   PsGetProcessWow64Process;
    void*   KeQueryInterruptTime;
    void*   KeDelayExecutionThread;
    void*   PsCreateSystemThread;
    void*   PsTerminateSystemThread;
    void*   ExAllocatePoolWithTag;
    void*   ExFreePoolWithTag;
} OMBRA_KERNEL_IMPORTS;

// Driver initialization magic
#define OMBRA_DRIVER_INIT_MAGIC  0x494E4954445256ULL  // 'INITDRV'

// Driver initialization structure (passed from loader)
typedef struct _OMBRA_DRIVER_INIT {
    U64                     Magic;
    U64                     Version;

    // Command ring (R0 addresses)
    void*                   CommandRing;
    void*                   ScratchBuffer;
    U64                     ScratchSize;
    U64                     ScratchBufferPhys;

    // Hypervisor communication
    U64                     VmcallMagic;
    U64                     VmcallKey;

    // Process context
    U64                     LoaderCr3;
    U64                     LoaderPid;

    // EPROCESS offsets (Windows version dependent)
    EPROCESS_OFFSETS_EX     Offsets;

    // Resolved kernel functions
    OMBRA_KERNEL_IMPORTS    Imports;
} OMBRA_DRIVER_INIT, *POMBRA_DRIVER_INIT;

#endif
