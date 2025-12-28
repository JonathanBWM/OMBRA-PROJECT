/**
 * @file supdrv.c
 * @brief Ld9BoxSup.sys (VirtualBox SUPDrv) interface implementation
 *
 * C port from C++ reference implementation.
 */

#include "supdrv.h"
#include "nt_defs.h"
#include "../obfuscate.h"
#include <stdio.h>
#include <string.h>

// Obfuscated module identifiers (decrypted at runtime)
static void GetModuleName(char* buf, size_t len) {
    // Returns "SysCore" or similar innocuous name
    const char* dec = DEC_OMBRAHV();
    strncpy_s(buf, len, dec, _TRUNCATE);
}

static void GetModulePath(char* buf, size_t len) {
    // Returns "C:\Windows\System32\drivers\syscore.sys" pattern
    snprintf(buf, len, "C:\\Windows\\System32\\drivers\\%s.sys", DEC_OMBRAHV());
}

//=============================================================================
// Error Handling
//=============================================================================

static void SupDrv_SetError(PSUPDRV_CTX ctx, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(ctx->szLastError, sizeof(ctx->szLastError), format, args);
    va_end(args);
    DbgLog("ERROR: %s", ctx->szLastError);
}

//=============================================================================
// Initialization / Cleanup
//=============================================================================

void SupDrv_Init(PSUPDRV_CTX ctx) {
    memset(ctx, 0, sizeof(SUPDRV_CTX));
    ctx->hDevice = INVALID_HANDLE_VALUE;
}

void SupDrv_Cleanup(PSUPDRV_CTX ctx) {
    if (ctx->hDevice != INVALID_HANDLE_VALUE) {
        CloseHandle(ctx->hDevice);
        ctx->hDevice = INVALID_HANDLE_VALUE;
    }
    ctx->bInitialized = false;
    ctx->Cookie = 0;
    ctx->SessionCookie = 0;
}

bool SupDrv_IsInitialized(PSUPDRV_CTX ctx) {
    return ctx->bInitialized;
}

const char* SupDrv_GetLastError(PSUPDRV_CTX ctx) {
    return ctx->szLastError;
}

//=============================================================================
// IOCTL Helpers
//=============================================================================

bool SupDrv_DoIoctl(PSUPDRV_CTX ctx, DWORD dwIoctl,
                    void* pIn, UINT32 cbIn,
                    void* pOut, UINT32 cbOut) {
    DWORD dwReturned = 0;
    BOOL result = DeviceIoControl(
        ctx->hDevice,
        dwIoctl,
        pIn, cbIn,
        pOut, cbOut,
        &dwReturned,
        NULL
    );
    return result != FALSE;
}

void SupDrv_FillHeader(PSUPDRV_CTX ctx, PSUPREQHDR pHdr, UINT32 cbIn, UINT32 cbOut) {
    pHdr->u32Cookie = ctx->Cookie;
    pHdr->u32SessionCookie = ctx->SessionCookie;
    pHdr->cbIn = cbIn;
    pHdr->cbOut = cbOut;
    pHdr->fFlags = SUPREQHDR_FLAGS_MAGIC;
    pHdr->rc = 0;
}

//=============================================================================
// Device Opening
//=============================================================================

bool SupDrv_TryOpenDevice(PSUPDRV_CTX ctx) {
    // Initialize NT functions
    if (!NtInit()) {
        SupDrv_SetError(ctx, "Failed to initialize NT functions");
        return false;
    }

    // Device paths to try (in order of preference)
    static const wchar_t* devicePaths[] = {
        NT_DEVICE_NAME_LDPLAYER,       // \Device\Ld9BoxDrv
        NT_DEVICE_NAME_LDPLAYER_USER,  // \Device\Ld9BoxDrvU
        NT_DEVICE_NAME_VBOX,           // \Device\VBoxDrv
    };
    static const size_t devicePathsCount = sizeof(devicePaths) / sizeof(devicePaths[0]);

    for (size_t i = 0; i < devicePathsCount; i++) {
        DbgLog("SupDrv_TryOpenDevice: Trying %ls", devicePaths[i]);

        HANDLE hDevice = NtOpenDevice(devicePaths[i]);
        if (hDevice != INVALID_HANDLE_VALUE) {
            ctx->hDevice = hDevice;
            wcscpy_s(ctx->wszDeviceName, sizeof(ctx->wszDeviceName) / sizeof(wchar_t), devicePaths[i]);
            DbgLog("SupDrv_TryOpenDevice: Opened %ls", devicePaths[i]);
            return true;
        }
    }

    SupDrv_SetError(ctx, "Failed to open any SUPDrv device");
    return false;
}

//=============================================================================
// Cookie Handshake
//=============================================================================

bool SupDrv_TryCookie(PSUPDRV_CTX ctx, UINT32 u32Version) {
    SUPCOOKIE req;
    memset(&req, 0, sizeof(req));

    // Fill header - no cookies yet, this is our first call
    req.Hdr.cbIn = COOKIE_SIZE_IN;
    req.Hdr.cbOut = COOKIE_SIZE_OUT;
    req.Hdr.fFlags = SUPREQHDR_FLAGS_MAGIC;
    req.Hdr.u32Cookie = SUPCOOKIE_INITIAL_COOKIE;  // "tori" for LDPlayer
    req.Hdr.u32SessionCookie = SUPCOOKIE_INITIAL_COOKIE;

    // Fill magic and version
    memcpy(req.u.In.szMagic, SUPCOOKIE_MAGIC, SUPCOOKIE_MAGIC_LEN);
    req.u.In.u32ReqVersion = u32Version;
    req.u.In.u32MinVersion = u32Version;

    DbgLog("SupDrv_TryCookie: Trying version 0x%08X", u32Version);

    if (!SupDrv_DoIoctl(ctx, SUP_IOCTL_COOKIE, &req, COOKIE_SIZE_IN, &req, COOKIE_SIZE_OUT)) {
        DbgLog("SupDrv_TryCookie: IOCTL failed, error %lu", GetLastError());
        return false;
    }

    if (req.Hdr.rc != VINF_SUCCESS) {
        DbgLog("SupDrv_TryCookie: Driver returned rc=%d", req.Hdr.rc);
        return false;
    }

    // Success! Save session info
    ctx->Cookie = req.u.Out.u32Cookie;
    ctx->SessionCookie = req.u.Out.u32SessionCookie;
    ctx->pSession = req.u.Out.pSession;
    ctx->DetectedVersion = u32Version;

    DbgLog("SupDrv_TryCookie: SUCCESS!");
    DbgLog("  Cookie: 0x%08X", ctx->Cookie);
    DbgLog("  SessionCookie: 0x%08X", ctx->SessionCookie);
    DbgLog("  pSession: 0x%016llX", ctx->pSession);
    DbgLog("  SessionVersion: 0x%08X", req.u.Out.u32SessionVersion);
    DbgLog("  DriverVersion: 0x%08X", req.u.Out.u32DriverVersion);
    DbgLog("  cFunctions: %u", req.u.Out.cFunctions);

    return true;
}

bool SupDrv_ProbeVersion(PSUPDRV_CTX ctx) {
    DbgLog("SupDrv_ProbeVersion: Probing %zu known versions...", KNOWN_VERSIONS_COUNT);

    for (size_t i = 0; i < KNOWN_VERSIONS_COUNT; i++) {
        if (SupDrv_TryCookie(ctx, KNOWN_VERSIONS[i])) {
            return true;
        }
    }

    SupDrv_SetError(ctx, "Cookie handshake failed for all known versions");
    return false;
}

//=============================================================================
// Full Initialization
//=============================================================================

bool SupDrv_Initialize(PSUPDRV_CTX ctx) {
    DbgLog("SupDrv_Initialize: Starting...");

    if (ctx->bInitialized) {
        DbgLog("SupDrv_Initialize: Already initialized");
        return true;
    }

    // Step 1: Open device
    if (ctx->hDevice == INVALID_HANDLE_VALUE) {
        if (!SupDrv_TryOpenDevice(ctx)) {
            return false;
        }
    }

    // Step 2: Cookie handshake
    if (!SupDrv_ProbeVersion(ctx)) {
        CloseHandle(ctx->hDevice);
        ctx->hDevice = INVALID_HANDLE_VALUE;
        return false;
    }

    ctx->bInitialized = true;
    DbgLog("SupDrv_Initialize: SUCCESS");
    return true;
}

//=============================================================================
// Memory Allocation
//=============================================================================

bool SupDrv_PageAllocEx(PSUPDRV_CTX ctx, UINT32 cPages, void** ppvR3, void** ppvR0) {
    if (!ctx->bInitialized) {
        SupDrv_SetError(ctx, "Not initialized");
        return false;
    }

    DbgLog("SupDrv_PageAllocEx: Allocating %u pages", cPages);

    // Calculate output buffer size (includes physical addresses)
    UINT32 cbOut = (UINT32)SUP_IOCTL_PAGE_ALLOC_EX_SIZE_OUT(cPages);

    // Allocate output buffer
    PVOID pOutBuf = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbOut);
    if (!pOutBuf) {
        SupDrv_SetError(ctx, "HeapAlloc failed for output buffer");
        return false;
    }

    // Build request
    SUPPAGEALLOCEX req;
    memset(&req, 0, sizeof(req));
    SupDrv_FillHeader(ctx, &req.Hdr, SUP_IOCTL_PAGE_ALLOC_EX_SIZE_IN, cbOut);
    req.u.In.cPages = cPages;
    req.u.In.fKernelMapping = 1;  // We need R0 mapping
    req.u.In.fUserMapping = 1;    // We need R3 mapping

    if (!SupDrv_DoIoctl(ctx, SUP_IOCTL_PAGE_ALLOC_EX,
                        &req, SUP_IOCTL_PAGE_ALLOC_EX_SIZE_IN,
                        pOutBuf, cbOut)) {
        SupDrv_SetError(ctx, "PAGE_ALLOC_EX IOCTL failed: %lu", GetLastError());
        HeapFree(GetProcessHeap(), 0, pOutBuf);
        return false;
    }

    // Parse output
    PSUPREQHDR pHdr = (PSUPREQHDR)pOutBuf;
    if (pHdr->rc != VINF_SUCCESS) {
        SupDrv_SetError(ctx, "PAGE_ALLOC_EX returned rc=%d", pHdr->rc);
        HeapFree(GetProcessHeap(), 0, pOutBuf);
        return false;
    }

    // Output structure follows header
    PSUPPAGEALLOCEX_OUT pOut = (PSUPPAGEALLOCEX_OUT)((UINT8*)pOutBuf + sizeof(SUPREQHDR));

    *ppvR3 = (void*)pOut->pvR3;
    *ppvR0 = (void*)pOut->pvR0;

    DbgLog("SupDrv_PageAllocEx: SUCCESS");
    DbgLog("  pvR3 (usermode): %p", *ppvR3);
    DbgLog("  pvR0 (kernel):   %p", *ppvR0);

    HeapFree(GetProcessHeap(), 0, pOutBuf);
    return true;
}

bool SupDrv_PageFree(PSUPDRV_CTX ctx, void* pvR3) {
    if (!ctx->bInitialized) {
        SupDrv_SetError(ctx, "Not initialized");
        return false;
    }

    DbgLog("SupDrv_PageFree: Freeing %p", pvR3);

    SUPPAGEFREE req;
    memset(&req, 0, sizeof(req));
    SupDrv_FillHeader(ctx, &req.Hdr, SUP_IOCTL_PAGE_FREE_SIZE_IN, SUP_IOCTL_PAGE_FREE_SIZE_OUT);
    req.u.In.pvR3 = (UINT64)pvR3;

    if (!SupDrv_DoIoctl(ctx, SUP_IOCTL_PAGE_FREE,
                        &req, SUP_IOCTL_PAGE_FREE_SIZE_IN,
                        &req, SUP_IOCTL_PAGE_FREE_SIZE_OUT)) {
        SupDrv_SetError(ctx, "PAGE_FREE IOCTL failed: %lu", GetLastError());
        return false;
    }

    if (req.Hdr.rc != VINF_SUCCESS) {
        SupDrv_SetError(ctx, "PAGE_FREE returned rc=%d", req.Hdr.rc);
        return false;
    }

    DbgLog("SupDrv_PageFree: SUCCESS");
    return true;
}

//=============================================================================
// Module Loading
//=============================================================================

bool SupDrv_LdrOpen(PSUPDRV_CTX ctx, UINT32 cbImage, void** ppvImageBase) {
    if (!ctx->bInitialized) {
        SupDrv_SetError(ctx, "Not initialized");
        return false;
    }

    DbgLog("SupDrv_LdrOpen: Requesting %u bytes", cbImage);

    // DEBUG: Verify ctx integrity
    DbgLog("[DEBUG LDR_OPEN] Entry - ctx=%p", ctx);
    DbgLog("[DEBUG LDR_OPEN]   hDevice=%p", ctx->hDevice);
    DbgLog("[DEBUG LDR_OPEN]   Cookie=0x%08X", ctx->Cookie);
    DbgLog("[DEBUG LDR_OPEN]   SessionCookie=0x%08X", ctx->SessionCookie);
    DbgLog("[DEBUG LDR_OPEN]   pSession=0x%llX", (unsigned long long)ctx->pSession);
    DbgLog("[DEBUG LDR_OPEN]   bInitialized=%d", ctx->bInitialized);
    DbgLog("[DEBUG LDR_OPEN]   cbImage=%u", cbImage);

    SUPLDROPEN req;
    memset(&req, 0, sizeof(req));
    SupDrv_FillHeader(ctx, &req.Hdr, LDR_OPEN_SIZE_IN, LDR_OPEN_SIZE_OUT);

    // CRITICAL: Driver requires cbImageBits < cbImageWithTabs (Ghidra analysis Dec 2025)
    // cbImageWithTabs = total allocation size (page-aligned + buffer for metadata)
    // cbImageBits = actual image data size
    // The driver checks: if (cbImageBits >= cbImageWithTabs) return error 87
    UINT32 cbAligned = (cbImage + 0xFFF) & ~0xFFF;  // Page-align
    req.u.In.cbImageWithTabs = cbAligned + 0x1000;  // Add 1 page for tabs/metadata
    req.u.In.cbImageBits = cbImage;                 // Actual image size (must be < cbImageWithTabs)
    GetModuleName(req.u.In.szName, sizeof(req.u.In.szName));
    // Use existing system file to trigger different error code
    // Native loader path returns -610 for non-existent files (STATUS_OBJECT_NAME_NOT_FOUND)
    // But fallback only triggers on -37. Try using existing file for different status.
    // DEBUG: Test with ntdll.dll (exists, not a driver, should return different error)
    strncpy_s(req.u.In.szFilename, sizeof(req.u.In.szFilename),
              "C:\\Windows\\System32\\ntdll.dll", _TRUNCATE);

    // DEBUG: Print request structure
    DbgLog("[DEBUG LDR_OPEN] Request structure:");
    DbgLog("[DEBUG LDR_OPEN]   Hdr.u32Cookie=0x%08X", req.Hdr.u32Cookie);
    DbgLog("[DEBUG LDR_OPEN]   Hdr.u32SessionCookie=0x%08X", req.Hdr.u32SessionCookie);
    DbgLog("[DEBUG LDR_OPEN]   Hdr.cbIn=0x%X (%u)", req.Hdr.cbIn, req.Hdr.cbIn);
    DbgLog("[DEBUG LDR_OPEN]   Hdr.cbOut=0x%X (%u)", req.Hdr.cbOut, req.Hdr.cbOut);
    DbgLog("[DEBUG LDR_OPEN]   Hdr.fFlags=0x%08X", req.Hdr.fFlags);
    DbgLog("[DEBUG LDR_OPEN]   In.cbImageWithTabs=%u", req.u.In.cbImageWithTabs);
    DbgLog("[DEBUG LDR_OPEN]   In.cbImageBits=%u", req.u.In.cbImageBits);
    DbgLog("[DEBUG LDR_OPEN]   In.szName='%s'", req.u.In.szName);

    if (!SupDrv_DoIoctl(ctx, SUP_IOCTL_LDR_OPEN,
                        &req, LDR_OPEN_SIZE_IN,
                        &req, LDR_OPEN_SIZE_OUT)) {
        DWORD err = GetLastError();
        DbgLog("[DEBUG LDR_OPEN] DeviceIoControl FAILED, error=%lu", err);
        SupDrv_SetError(ctx, "LDR_OPEN IOCTL failed: %lu", err);
        return false;
    }

    if (req.Hdr.rc != VINF_SUCCESS) {
        SupDrv_SetError(ctx, "LDR_OPEN returned rc=%d (rc=-618 means flags not set!)", req.Hdr.rc);
        return false;
    }

    if (!IsKernelAddress(req.u.Out.pvImageBase)) {
        SupDrv_SetError(ctx, "LDR_OPEN returned invalid kernel address: %p", req.u.Out.pvImageBase);
        return false;
    }

    *ppvImageBase = req.u.Out.pvImageBase;
    DbgLog("SupDrv_LdrOpen: SUCCESS, pvImageBase=%p", *ppvImageBase);
    return true;
}

bool SupDrv_LdrLoad(PSUPDRV_CTX ctx, void* pvImageBase, const void* pvImage,
                    UINT32 cbImage, void* pfnEntry) {
    if (!ctx->bInitialized) {
        SupDrv_SetError(ctx, "Not initialized");
        return false;
    }

    DbgLog("SupDrv_LdrLoad: Loading %u bytes at %p, entry=%p", cbImage, pvImageBase, pfnEntry);

    // Allocate request buffer (variable size)
    UINT32 cbIn = (UINT32)SUP_IOCTL_LDR_LOAD_SIZE_IN(cbImage);
    UINT32 cbOut = (UINT32)SUP_IOCTL_LDR_LOAD_SIZE_OUT;

    PVOID pReq = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbIn);
    if (!pReq) {
        SupDrv_SetError(ctx, "HeapAlloc failed");
        return false;
    }

    PSUPLDRLOAD pLoad = (PSUPLDRLOAD)pReq;
    SupDrv_FillHeader(ctx, &pLoad->Hdr, cbIn, cbOut);

    pLoad->u.In.pvImageBase = pvImageBase;
    // Match the allocation sizes from LDR_OPEN (cbImageBits < cbImageWithTabs)
    UINT32 cbAligned = (cbImage + 0xFFF) & ~0xFFF;
    pLoad->u.In.cbImageWithTabs = cbAligned + 0x1000;
    pLoad->u.In.cbImageBits = cbImage;
    pLoad->u.In.offSymbols = 0;
    pLoad->u.In.cSymbols = 0;
    pLoad->u.In.offStrTab = 0;
    pLoad->u.In.cbStrTab = 0;
    pLoad->u.In.eEPType = pfnEntry ? SUPLDRLOADEP_SERVICE : SUPLDRLOADEP_NOTHING;
    pLoad->u.In.pfnModuleInit = pfnEntry;
    pLoad->u.In.pfnModuleTerm = NULL;
    pLoad->u.In.pvVMMR0 = NULL;
    pLoad->u.In.pvVMMR0EntryFast = NULL;
    pLoad->u.In.pvVMMR0EntryEx = NULL;

    // Copy image data
    memcpy(pLoad->u.In.abImage, pvImage, cbImage);

    // Allocate output buffer
    SUPLDRLOAD_OUT outBuf;
    memset(&outBuf, 0, sizeof(outBuf));

    if (!SupDrv_DoIoctl(ctx, SUP_IOCTL_LDR_LOAD, pReq, cbIn, &outBuf, cbOut)) {
        SupDrv_SetError(ctx, "LDR_LOAD IOCTL failed: %lu", GetLastError());
        HeapFree(GetProcessHeap(), 0, pReq);
        return false;
    }

    // Check header rc
    PSUPREQHDR pHdr = (PSUPREQHDR)pReq;
    if (pHdr->rc != VINF_SUCCESS) {
        SupDrv_SetError(ctx, "LDR_LOAD returned rc=%d", pHdr->rc);
        HeapFree(GetProcessHeap(), 0, pReq);
        return false;
    }

    DbgLog("SupDrv_LdrLoad: SUCCESS");
    HeapFree(GetProcessHeap(), 0, pReq);
    return true;
}

//=============================================================================
// MSR Access (DISABLED in LDPlayer)
//=============================================================================

bool SupDrv_MsrRead(PSUPDRV_CTX ctx, UINT32 uMsr, UINT64* puValue, UINT32 idCpu) {
    if (!ctx->bInitialized) {
        SupDrv_SetError(ctx, "Not initialized");
        return false;
    }

    DbgLog("SupDrv_MsrRead: MSR 0x%08X, CPU %u", uMsr, idCpu);

    SUPMSRPROBER req;
    memset(&req, 0, sizeof(req));
    SupDrv_FillHeader(ctx, &req.Hdr,
                      sizeof(SUPREQHDR) + sizeof(SUPMSRPROBER_IN),
                      sizeof(SUPREQHDR) + sizeof(SUPMSRPROBER_OUT));

    req.u.In.enmOp = SUPMSRPROBEROP_READ;
    req.u.In.uMsr = uMsr;
    req.u.In.idCpu = idCpu;

    if (!SupDrv_DoIoctl(ctx, SUP_IOCTL_MSR_PROBER,
                        &req, sizeof(SUPREQHDR) + sizeof(SUPMSRPROBER_IN),
                        &req, sizeof(SUPREQHDR) + sizeof(SUPMSRPROBER_OUT))) {
        SupDrv_SetError(ctx, "MSR_PROBER IOCTL failed: %lu", GetLastError());
        return false;
    }

    if (req.Hdr.rc != VINF_SUCCESS) {
        // rc=-12 (VERR_NOT_SUPPORTED) means MSR_PROBER is disabled in LDPlayer
        SupDrv_SetError(ctx, "MSR_PROBER returned rc=%d (rc=-12 means disabled)", req.Hdr.rc);
        return false;
    }

    if (req.u.Out.uResults.Read.fGp) {
        SupDrv_SetError(ctx, "MSR read caused #GP");
        return false;
    }

    *puValue = req.u.Out.uResults.Read.uValue;
    DbgLog("SupDrv_MsrRead: MSR 0x%08X = 0x%016llX", uMsr, *puValue);
    return true;
}

bool SupDrv_MsrWrite(PSUPDRV_CTX ctx, UINT32 uMsr, UINT64 uValue, UINT32 idCpu) {
    if (!ctx->bInitialized) {
        SupDrv_SetError(ctx, "Not initialized");
        return false;
    }

    DbgLog("SupDrv_MsrWrite: MSR 0x%08X = 0x%016llX, CPU %u", uMsr, uValue, idCpu);

    SUPMSRPROBER req;
    memset(&req, 0, sizeof(req));
    SupDrv_FillHeader(ctx, &req.Hdr,
                      sizeof(SUPREQHDR) + sizeof(SUPMSRPROBER_IN),
                      sizeof(SUPREQHDR) + sizeof(SUPMSRPROBER_OUT));

    req.u.In.enmOp = SUPMSRPROBEROP_WRITE;
    req.u.In.uMsr = uMsr;
    req.u.In.idCpu = idCpu;
    req.u.In.uArgs.Write.uToWrite = uValue;

    if (!SupDrv_DoIoctl(ctx, SUP_IOCTL_MSR_PROBER,
                        &req, sizeof(SUPREQHDR) + sizeof(SUPMSRPROBER_IN),
                        &req, sizeof(SUPREQHDR) + sizeof(SUPMSRPROBER_OUT))) {
        SupDrv_SetError(ctx, "MSR_PROBER IOCTL failed: %lu", GetLastError());
        return false;
    }

    if (req.Hdr.rc != VINF_SUCCESS) {
        SupDrv_SetError(ctx, "MSR_PROBER returned rc=%d (rc=-12 means disabled)", req.Hdr.rc);
        return false;
    }

    if (req.u.Out.uResults.Write.fGp) {
        SupDrv_SetError(ctx, "MSR write caused #GP");
        return false;
    }

    DbgLog("SupDrv_MsrWrite: SUCCESS");
    return true;
}

//=============================================================================
// Symbol Resolution (LDR_GET_SYMBOL)
//=============================================================================

bool SupDrv_GetSymbol(PSUPDRV_CTX ctx, const char* szSymbol, void** ppvSymbol) {
    if (!ctx->bInitialized) {
        SupDrv_SetError(ctx, "Not initialized");
        return false;
    }

    DbgLog("SupDrv_GetSymbol: Looking up '%s'", szSymbol);

    // Structure layout from Ghidra analysis of Ld9BoxSup.sys IOCTL handler:
    // - cbIn = 0x60 (96 bytes), cbOut = 0x20 (32 bytes)
    // - szSymbol at offset 0x20, max 0x40 (64) bytes
    // Layout: SUPREQHDR (24) + pvImageBase (8) + szSymbol (64) = 96 bytes
    struct {
        SUPREQHDR Hdr;          // 24 bytes (offset 0x00-0x17)
        void* pvImageBase;       // 8 bytes  (offset 0x18-0x1F)
        char szSymbol[64];       // 64 bytes (offset 0x20-0x5F) - MUST be 64, not 128!
    } reqIn;  // Total: 96 bytes (0x60)

    struct {
        SUPREQHDR Hdr;
        void* pvSymbol;
    } reqOut;

    memset(&reqIn, 0, sizeof(reqIn));
    memset(&reqOut, 0, sizeof(reqOut));

    // Verify structure sizes match Ghidra-discovered values
    DbgLog("SupDrv_GetSymbol: sizeof(reqIn)=%zu (expected 96), sizeof(reqOut)=%zu (expected 32)",
           sizeof(reqIn), sizeof(reqOut));

    SupDrv_FillHeader(ctx, &reqIn.Hdr, sizeof(reqIn), sizeof(reqOut));
    reqIn.pvImageBase = NULL;  // NULL = ntoskrnl
    strncpy_s(reqIn.szSymbol, sizeof(reqIn.szSymbol), szSymbol, _TRUNCATE);

    if (!SupDrv_DoIoctl(ctx, SUP_IOCTL_LDR_GET_SYMBOL,
                        &reqIn, sizeof(reqIn),
                        &reqOut, sizeof(reqOut))) {
        SupDrv_SetError(ctx, "LDR_GET_SYMBOL IOCTL failed: %lu", GetLastError());
        return false;
    }

    if (reqOut.Hdr.rc != VINF_SUCCESS) {
        SupDrv_SetError(ctx, "LDR_GET_SYMBOL returned rc=%d", reqOut.Hdr.rc);
        return false;
    }

    if (!reqOut.pvSymbol) {
        SupDrv_SetError(ctx, "Symbol '%s' not found", szSymbol);
        return false;
    }

    *ppvSymbol = reqOut.pvSymbol;
    DbgLog("SupDrv_GetSymbol: %s = %p", szSymbol, *ppvSymbol);
    return true;
}
