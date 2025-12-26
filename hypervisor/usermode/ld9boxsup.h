// ld9boxsup.h — Ld9BoxSup.sys (VirtualBox SUPDrv) IOCTL Definitions
// OmbraHypervisor BYOVD Interface
//
// This driver is a VirtualBox SUPDrv derivative signed by LDPlayer.
// IOCTLs match VirtualBox 6.1.x SUPDrv interface.

#ifndef LD9BOXSUP_H
#define LD9BOXSUP_H

#include <Windows.h>
#include <stdint.h>

#pragma pack(push, 1)

// =============================================================================
// Device Path
// =============================================================================

#define LD9BOXSUP_DEVICE_PATH   L"\\\\.\\Ld9BoxSup"
#define LD9BOXSUP_SERVICE_NAME  L"Ld9BoxSup"

// =============================================================================
// IOCTL Codes
// =============================================================================
// CTL_CODE(FILE_DEVICE_UNKNOWN, 0xA00 + n, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define SUP_IOCTL_COOKIE            0x22A000    // Establish session
#define SUP_IOCTL_LDR_OPEN          0x22A004    // Open module for loading
#define SUP_IOCTL_LDR_LOAD          0x22A008    // Load module into kernel
#define SUP_IOCTL_LDR_FREE          0x22A00C    // Free loaded module
#define SUP_IOCTL_LDR_GET_SYMBOL    0x22A010    // Resolve kernel symbol
#define SUP_IOCTL_CONT_ALLOC        0x22A014    // Allocate contiguous memory
#define SUP_IOCTL_CONT_FREE         0x22A018    // Free contiguous memory
#define SUP_IOCTL_LOW_ALLOC         0x22A01C    // Allocate below 4GB
#define SUP_IOCTL_LOW_FREE          0x22A020    // Free low memory
#define SUP_IOCTL_GIP_MAP           0x22A024    // Map Global Info Page
#define SUP_IOCTL_PAGE_ALLOC_EX     0x22A028    // Allocate pages with options
#define SUP_IOCTL_PAGE_MAP_KERNEL   0x22A02C    // Map pages to kernel
#define SUP_IOCTL_PAGE_PROTECT      0x22A030    // Change page protection
#define SUP_IOCTL_PAGE_FREE         0x22A034    // Free page allocation
#define SUP_IOCTL_CALL_VMMR0        0x22A03C    // Execute in Ring-0
#define SUP_IOCTL_PAGE_LOCK         0x22A048    // Lock pages in memory
#define SUP_IOCTL_PAGE_UNLOCK       0x22A04C    // Unlock pages
#define SUP_IOCTL_VT_CAPS           0x22A050    // Query VT-x capabilities
#define SUP_IOCTL_GET_HWVIRT_MSRS   0x22A054    // Get VMX MSR values
#define SUP_IOCTL_TSC_DELTA_MEASURE 0x22A05C    // Measure TSC delta
#define SUP_IOCTL_MSR_PROBER        0x22A068    // Read/Write arbitrary MSRs

// =============================================================================
// Common Header
// =============================================================================

typedef struct _SUPREQHDR {
    uint32_t    u32Cookie;          // Session cookie from SUP_IOCTL_COOKIE
    uint32_t    u32SessionCookie;   // Session-specific cookie
    uint32_t    cbIn;               // Input size
    uint32_t    cbOut;              // Output size
    uint32_t    fFlags;             // Flags
    int32_t     rc;                 // Return code
} SUPREQHDR;

// =============================================================================
// SUP_IOCTL_COOKIE — Establish Session
// =============================================================================

#define SUP_COOKIE_MAGIC    "The Magic Word!"

typedef struct _SUPCOOKIE_IN {
    SUPREQHDR   Hdr;
    char        szMagic[16];        // Must be "The Magic Word!"
    uint32_t    u32ReqVersion;      // Requested interface version
    uint32_t    u32MinVersion;      // Minimum version we accept
} SUPCOOKIE_IN;

typedef struct _SUPCOOKIE_OUT {
    SUPREQHDR   Hdr;
    uint32_t    u32Cookie;          // Save this
    uint32_t    u32SessionCookie;   // Save this
    uint32_t    u32SessionVersion;  // Interface version
    uint32_t    u32DriverVersion;   // Driver version
    void*       pSession;           // Session handle
    uint32_t    cFunctions;         // Number of functions
    void*       pvReserved;
} SUPCOOKIE_OUT;

// =============================================================================
// SUP_IOCTL_CONT_ALLOC — Allocate Contiguous Memory
// =============================================================================

typedef struct _SUPCONTALLOC_IN {
    SUPREQHDR   Hdr;
    uint32_t    cPages;             // Number of 4KB pages
} SUPCONTALLOC_IN;

typedef struct _SUPCONTALLOC_OUT {
    SUPREQHDR   Hdr;
    void*       pvR3;               // Ring-3 (usermode) mapping
    void*       pvR0;               // Ring-0 (kernel) mapping
    uint64_t    HCPhys;             // Physical address
} SUPCONTALLOC_OUT;

// =============================================================================
// SUP_IOCTL_PAGE_ALLOC_EX — Allocate Pages with Options
// =============================================================================

typedef struct _SUPPAGEALLOC_IN {
    SUPREQHDR   Hdr;
    uint32_t    cPages;             // Number of pages
    uint32_t    fKernelMapping : 1; // Create kernel mapping
    uint32_t    fUserMapping : 1;   // Create user mapping
    uint32_t    fReserved : 1;
    uint32_t    fInterProcessShared : 1;
    uint32_t    fReserved2 : 28;
} SUPPAGEALLOC_IN;

typedef struct _SUPPAGEALLOC_OUT {
    SUPREQHDR   Hdr;
    void*       pvR3;               // Ring-3 mapping (if requested)
    void*       pvR0;               // Ring-0 mapping (if requested)
    // Followed by cPages * uint64_t of physical addresses
} SUPPAGEALLOC_OUT;

// =============================================================================
// SUP_IOCTL_CALL_VMMR0 — Execute in Ring-0
// =============================================================================

typedef struct _SUPCALLVMMR0_IN {
    SUPREQHDR   Hdr;
    void*       pVMR0;              // VM handle (can be NULL)
    uint32_t    idCpu;              // Target CPU (-1 for current)
    uint32_t    uOperation;         // Operation code
    uint64_t    u64Arg;             // 64-bit argument
    // Followed by variable data
} SUPCALLVMMR0_IN;

typedef struct _SUPCALLVMMR0_OUT {
    SUPREQHDR   Hdr;
    int32_t     iResult;            // Result from called function
} SUPCALLVMMR0_OUT;

// =============================================================================
// SUP_IOCTL_GET_HWVIRT_MSRS — Get VMX MSR Values
// =============================================================================

typedef struct _SUPHWVIRTMSRS_IN {
    SUPREQHDR   Hdr;
    uint32_t    fCaps;              // Request flags
} SUPHWVIRTMSRS_IN;

typedef struct _SUPHWVIRTMSRS_OUT {
    SUPREQHDR   Hdr;
    // VMX MSRs
    uint64_t    u64FeatCtrl;        // IA32_FEATURE_CONTROL
    uint64_t    u64Basic;           // IA32_VMX_BASIC
    uint64_t    u64PinCtls;         // IA32_VMX_PINBASED_CTLS
    uint64_t    u64ProcCtls;        // IA32_VMX_PROCBASED_CTLS
    uint64_t    u64ProcCtls2;       // IA32_VMX_PROCBASED_CTLS2
    uint64_t    u64ExitCtls;        // IA32_VMX_EXIT_CTLS
    uint64_t    u64EntryCtls;       // IA32_VMX_ENTRY_CTLS
    uint64_t    u64TruePin;         // IA32_VMX_TRUE_PINBASED_CTLS
    uint64_t    u64TrueProc;        // IA32_VMX_TRUE_PROCBASED_CTLS
    uint64_t    u64TrueExit;        // IA32_VMX_TRUE_EXIT_CTLS
    uint64_t    u64TrueEntry;       // IA32_VMX_TRUE_ENTRY_CTLS
    uint64_t    u64Misc;            // IA32_VMX_MISC
    uint64_t    u64Cr0Fixed0;       // IA32_VMX_CR0_FIXED0
    uint64_t    u64Cr0Fixed1;       // IA32_VMX_CR0_FIXED1
    uint64_t    u64Cr4Fixed0;       // IA32_VMX_CR4_FIXED0
    uint64_t    u64Cr4Fixed1;       // IA32_VMX_CR4_FIXED1
    uint64_t    u64VmcsEnum;        // IA32_VMX_VMCS_ENUM
    uint64_t    u64EptVpidCap;      // IA32_VMX_EPT_VPID_CAP
    uint64_t    u64VmFunc;          // IA32_VMX_VMFUNC (if supported)
} SUPHWVIRTMSRS_OUT;

// =============================================================================
// SUP_IOCTL_VT_CAPS — Query VT-x Capabilities
// =============================================================================

typedef struct _SUPVTCAPS_IN {
    SUPREQHDR   Hdr;
} SUPVTCAPS_IN;

typedef struct _SUPVTCAPS_OUT {
    SUPREQHDR   Hdr;
    uint32_t    fCaps;              // Capability flags
} SUPVTCAPS_OUT;

// VT capability flags
#define SUPVTCAPS_VT_X              (1 << 0)    // VT-x supported
#define SUPVTCAPS_NESTED_PAGING     (1 << 1)    // EPT supported
#define SUPVTCAPS_VT_X_UNRESTRICTED (1 << 2)    // Unrestricted guest

// =============================================================================
// SUP_IOCTL_LDR_OPEN — Open Module for Loading
// =============================================================================

typedef struct _SUPLDROPEN_IN {
    SUPREQHDR   Hdr;
    uint32_t    cbImageWithEverything;  // Total image size
    uint32_t    cbImageBits;            // Size of image bits
    char        szName[32];             // Module name
    char        szFilename[260];        // File path
} SUPLDROPEN_IN;

typedef struct _SUPLDROPEN_OUT {
    SUPREQHDR   Hdr;
    void*       pvImageBase;            // Allocated image base
    uint32_t    fNeedsLoading;          // Needs LDR_LOAD call
} SUPLDROPEN_OUT;

// =============================================================================
// SUP_IOCTL_LDR_LOAD — Load Module into Kernel
// =============================================================================

typedef struct _SUPLDRLOAD_IN {
    SUPREQHDR   Hdr;
    void*       pvImageBase;            // From LDR_OPEN
    void*       pfnModuleInit;          // Entry point
    void*       pfnModuleTerm;          // Cleanup function
    uint32_t    offStrTab;              // String table offset
    uint32_t    cbStrTab;               // String table size
    uint32_t    offSymbols;             // Symbol table offset
    uint32_t    cSymbols;               // Symbol count
    uint32_t    cbImageBits;            // Image size
    // Followed by image data
} SUPLDRLOAD_IN;

typedef struct _SUPLDRLOAD_OUT {
    SUPREQHDR   Hdr;
    void*       pvMod;                  // Module handle
    int32_t     eEPType;                // Entry point type
} SUPLDRLOAD_OUT;

// =============================================================================
// SUP_IOCTL_LDR_GET_SYMBOL — Resolve Kernel Symbol
// =============================================================================

typedef struct _SUPLDRGETSYMBOL_IN {
    SUPREQHDR   Hdr;
    void*       pvImageBase;            // Module handle (NULL for ntoskrnl)
    char        szSymbol[128];          // Symbol name
} SUPLDRGETSYMBOL_IN;

typedef struct _SUPLDRGETSYMBOL_OUT {
    SUPREQHDR   Hdr;
    void*       pvSymbol;               // Symbol address
} SUPLDRGETSYMBOL_OUT;

// =============================================================================
// SUP_IOCTL_MSR_PROBER — Read/Write Arbitrary MSRs
// =============================================================================

#define SUP_MSR_OP_READ     0
#define SUP_MSR_OP_WRITE    1
#define SUP_MSR_OP_MODIFY   2

typedef struct _SUPMSRPROBER_IN {
    SUPREQHDR   Hdr;
    uint32_t    uMsr;                   // MSR number
    uint32_t    idCpu;                  // Target CPU
    uint32_t    uOp;                    // Operation (read/write/modify)
    uint64_t    u64In;                  // Value for write
    uint64_t    u64AndMask;             // AND mask for modify
    uint64_t    u64OrMask;              // OR mask for modify
} SUPMSRPROBER_IN;

typedef struct _SUPMSRPROBER_OUT {
    SUPREQHDR   Hdr;
    uint64_t    u64Out;                 // Read value
} SUPMSRPROBER_OUT;

// =============================================================================
// SUP_IOCTL_CONT_FREE — Free Contiguous Memory
// =============================================================================

typedef struct _SUPCONTFREE_IN {
    SUPREQHDR   Hdr;
    void*       pvR3;                   // Ring-3 mapping to free
} SUPCONTFREE_IN;

typedef struct _SUPCONTFREE_OUT {
    SUPREQHDR   Hdr;
} SUPCONTFREE_OUT;

// =============================================================================
// SUP_IOCTL_PAGE_FREE — Free Page Allocation
// =============================================================================

typedef struct _SUPPAGEFREE_IN {
    SUPREQHDR   Hdr;
    void*       pvR3;                   // Ring-3 mapping to free
} SUPPAGEFREE_IN;

typedef struct _SUPPAGEFREE_OUT {
    SUPREQHDR   Hdr;
} SUPPAGEFREE_OUT;

#pragma pack(pop)

// =============================================================================
// Error Codes (VirtualBox style)
// =============================================================================

#define VINF_SUCCESS            0
#define VERR_GENERAL_FAILURE    (-1)
#define VERR_INVALID_PARAMETER  (-2)
#define VERR_NO_MEMORY          (-3)
#define VERR_NOT_SUPPORTED      (-4)

#endif // LD9BOXSUP_H
