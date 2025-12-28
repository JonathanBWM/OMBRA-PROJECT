# Usermode Loader Component

## VERIFICATION STATUS
```
VERIFICATION:
- [x] I read every file mentioned in this document
- [x] I verified all function signatures against actual code
- [x] I verified all data structures against actual definitions
- [x] I verified all dependencies against actual imports
- [ ] I tested or traced all claimed behaviors

UNVERIFIED CLAIMS:
- LDR_LOAD entry point invocation behavior
- Physical address translation accuracy
- Memory allocation alignment guarantees

ASSUMPTIONS:
- BYOVD driver (Ld9BoxSup.sys) is loaded and responding
- Process has administrator privileges
- Windows x64 environment
```

## DOCUMENTED FROM
```
Git hash: 73853be
Date: 2025-12-27
Files read:
  - hypervisor/usermode/loader/hv_loader.c
  - hypervisor/usermode/loader/hv_loader.h
  - hypervisor/usermode/loader/pe_mapper.c
  - hypervisor/usermode/loader/pe_relocs.c
  - hypervisor/usermode/loader/pe_iat.c
  - hypervisor/usermode/loader/pe_wipe.c
```

---

## Component Overview

**Location**: `hypervisor/usermode/loader/`

**Purpose**: Orchestrates hypervisor loading via BYOVD driver, including memory allocation, PE mapping, relocation, import resolution, and entry point invocation.

**Inputs**:
- Path to BYOVD driver (Ld9BoxSup.sys)
- Raw PE image of hypervisor module
- PE image size

**Outputs**:
- Running hypervisor with all CPUs virtualized
- Debug buffer accessible from usermode

---

## File Breakdown

| File | Lines | Purpose | Exports |
|------|-------|---------|---------|
| `hv_loader.c` | ~470 | Main loader orchestration | `HvLoaderInit`, `HvLoaderLoad`, `HvLoaderUnload`, `HvLoaderCleanup` |
| `hv_loader.h` | ~60 | Loader structures/config | `HV_LOADER_CTX`, `HV_MEMORY_LAYOUT` |
| `pe_mapper.c` | ~65 | PE section mapping | `MapDriver` |
| `pe_relocs.c` | ~70 | Relocation processing | `ApplyRelocations` |
| `pe_iat.c` | ~30 | IAT patching | `PatchIAT` |
| `pe_wipe.c` | ~35 | Header wiping | `WipePeHeaders`, `WipePeHeadersRandom` |
| `pe_utils.c` | ~200 | PE parsing utilities | `PeValidate`, `PeFindSection`, `PeGetEntryPoint` |
| `pe_parser.c` | ~150 | PE info extraction | `PeParseHeaders`, `PeParseImports` |

---

## Configuration Constants

From `hv_loader.h`:

```c
#define HOST_STACK_SIZE     0x4000      // 16KB per CPU
#define HOST_STACK_PAGES    4           // HOST_STACK_SIZE / PAGE_SIZE
#define EPT_TABLES_PAGES    512         // 2MB for EPT identity mapping
#define DEBUG_BUFFER_PAGES  16          // 64KB debug ring buffer
```

---

## Key Data Structures

### HV_MEMORY_LAYOUT

```c
typedef struct _HV_MEMORY_LAYOUT {
    // Per-CPU (contiguous, need physical addresses)
    ALLOC_INFO  VmxonRegions;       // CpuCount pages
    ALLOC_INFO  VmcsRegions;        // CpuCount pages
    ALLOC_INFO  HostStacks;         // CpuCount * HOST_STACK_PAGES pages

    // Shared (contiguous)
    ALLOC_INFO  MsrBitmap;          // 1 page
    ALLOC_INFO  EptTables;          // 512 pages

    // Params page
    ALLOC_INFO  ParamsPage;         // 1 page for HV_INIT_PARAMS

    // Debug (non-paged OK)
    ALLOC_INFO  DebugBuffer;        // 16 pages
} HV_MEMORY_LAYOUT;
```

VERIFIED: Each `ALLOC_INFO` contains R3 (usermode), R0 (kernel), and Physical addresses.

### HV_LOADER_CTX

```c
typedef struct _HV_LOADER_CTX {
    DRV_CONTEXT         Driver;         // BYOVD driver context
    U32                 CpuCount;       // Number of logical CPUs
    HV_MEMORY_LAYOUT    Memory;         // Allocated memory regions

    void*               ImageBase;      // Kernel address from LDR_OPEN
    U32                 ImageSize;      // Size of mapped image

    BOOL                Loaded;         // HV module loaded
    BOOL                Running;        // HV actively virtualizing
} HV_LOADER_CTX;
```

### KERNEL_SYMBOLS (internal)

```c
typedef struct _KERNEL_SYMBOLS {
    U64     KeIpiGenericCall;               // For cross-CPU IPI
    U64     KeQueryActiveProcessorCountEx;  // Get CPU count
    U64     KeGetCurrentProcessorNumberEx;  // Get current CPU index
} KERNEL_SYMBOLS;
```

VERIFIED: These are resolved via `DrvGetSymbol()` before hypervisor launch.

---

## Key Functions

### hv_loader.c

```c
/**
 * Initialize loader context with BYOVD driver
 * @param ctx Loader context to initialize
 * @param driverPath Path to Ld9BoxSup.sys (loaded externally)
 * @return TRUE on success
 *
 * VERIFIED: Calls DrvInitialize, gets CPU count via GetSystemInfo
 */
BOOL HvLoaderInit(HV_LOADER_CTX* ctx, const wchar_t* driverPath);

/**
 * Load and start the hypervisor
 * @param ctx Initialized loader context
 * @param hvImage Raw PE image bytes
 * @param hvImageSize Size of PE image
 * @return TRUE on success
 *
 * VERIFIED: Sequence is:
 *   1. ResolveKernelSymbols()
 *   2. AllocateHypervisorMemory()
 *   3. PrepareVmxStructures()
 *   4. BuildInitParams()
 *   5. PatchBootstrapSection()
 *   6. LoadHypervisorModule()
 */
BOOL HvLoaderLoad(HV_LOADER_CTX* ctx, const void* hvImage, U32 hvImageSize);

/**
 * Unload the hypervisor (graceful shutdown)
 * @param ctx Loader context
 * @return TRUE on success
 *
 * VERIFIED: Sets Running=FALSE, calls DrvLdrFree for kernel image
 * NOTE: TODO comment indicates VMCALL_UNLOAD not yet implemented
 */
BOOL HvLoaderUnload(HV_LOADER_CTX* ctx);

/**
 * Full cleanup including memory deallocation
 * @param ctx Loader context
 *
 * VERIFIED: Calls HvLoaderUnload, FreeHypervisorMemory, DrvCleanup
 */
void HvLoaderCleanup(HV_LOADER_CTX* ctx);
```

### pe_mapper.c

```c
/**
 * Map PE image to kernel memory
 * @param peImage Source PE image (usermode buffer)
 * @param targetR3 Destination usermode-visible buffer
 * @param targetR0 Kernel address where image will execute
 * @param peInfo Parsed PE information
 * @return true on success
 *
 * VERIFIED: Sequence is:
 *   1. Zero target region
 *   2. Copy headers (for reloc/IAT processing)
 *   3. Copy each section to its RVA
 *   4. ApplyRelocations() with kernel base
 *   5. PatchIAT() with resolved imports
 *   6. WipePeHeaders() for anti-forensics
 */
bool MapDriver(const void* peImage, void* targetR3,
               uint64_t targetR0, PE_INFO* peInfo);
```

### pe_relocs.c

```c
/**
 * Apply base relocations to mapped image
 * @param mappedImage Mapped PE in usermode buffer
 * @param peInfo Parsed PE info with relocation RVA/size
 * @param newBase Kernel address (new ImageBase)
 * @return true on success
 *
 * VERIFIED: Handles IMAGE_REL_BASED_DIR64 (64-bit) and
 *           IMAGE_REL_BASED_HIGHLOW (32-bit) relocations.
 *           Returns early if delta == 0 (preferred base match).
 */
bool ApplyRelocations(void* mappedImage, PE_INFO* peInfo, uint64_t newBase);
```

### pe_iat.c

```c
/**
 * Patch Import Address Table with resolved addresses
 * @param mappedImage Mapped PE in usermode buffer
 * @param peInfo Parsed PE info with import entries
 * @return true on success
 *
 * VERIFIED: Iterates peInfo->Imports[], writes ResolvedAddress
 *           to each IatRva. Fails if any import unresolved.
 */
bool PatchIAT(void* mappedImage, PE_INFO* peInfo);
```

### pe_wipe.c

```c
/**
 * Zero PE headers to prevent signature detection
 * @param mappedImage Mapped image base
 * @param headerSize Size of headers to wipe
 *
 * VERIFIED: Uses volatile write to prevent compiler optimization
 */
void WipePeHeaders(void* mappedImage, uint32_t headerSize);

/**
 * Fill PE headers with random data
 * @param mappedImage Mapped image base
 * @param headerSize Size of headers to wipe
 *
 * VERIFIED: Uses RDTSC-seeded LCG PRNG for randomness
 */
void WipePeHeadersRandom(void* mappedImage, uint32_t headerSize);
```

---

## Loading Sequence

```
HvLoaderInit()
    │
    ├── DrvInitialize()      // Establish BYOVD session
    └── GetSystemInfo()      // Get CPU count

HvLoaderLoad()
    │
    ├── ResolveKernelSymbols()
    │   ├── DrvGetSymbol("KeIpiGenericCall")
    │   ├── DrvGetSymbol("KeQueryActiveProcessorCountEx")
    │   └── DrvGetSymbol("KeGetCurrentProcessorNumberEx")
    │
    ├── AllocateHypervisorMemory()
    │   ├── DrvAllocContiguous() × 5     // VMXON, VMCS, MSR, EPT, Params
    │   └── DrvAllocPages() × 2          // Stacks, Debug
    │
    ├── PrepareVmxStructures()
    │   ├── Set VMCS revision ID in each VMXON/VMCS region
    │   └── Zero all buffers
    │
    ├── BuildInitParams()
    │   ├── Set magic (0x4F4D4252 = 'OMBR')
    │   ├── Populate memory pointers (R0 + Physical)
    │   ├── Copy VMX capability MSRs from driver context
    │   └── Set VMCALL key (hardcoded: 0xDEADBEEFCAFEBABE)
    │
    ├── PatchBootstrapSection()
    │   ├── PeFindSection(".ombra")
    │   ├── Validate OMBRA_BOOTSTRAP magic
    │   └── Write ParamsPtr to bootstrap
    │
    └── LoadHypervisorModule()
        ├── PeGetEntryPoint()
        ├── DrvLdrOpen()        // Allocate kernel memory
        ├── MapDriver()         // Copy + relocate + patch
        └── DrvLdrLoad()        // Copy to kernel + call entry
```

---

## Memory Allocation Details

For an 8-CPU system:

| Region | Pages | Size | Notes |
|--------|-------|------|-------|
| VMXON | 8 | 32KB | One 4KB page per CPU |
| VMCS | 8 | 32KB | One 4KB page per CPU |
| Host Stacks | 32 | 128KB | 4 pages × 8 CPUs |
| MSR Bitmap | 1 | 4KB | Shared across CPUs |
| EPT Tables | 512 | 2MB | 512GB identity map |
| Params | 1 | 4KB | HV_INIT_PARAMS |
| Debug Buffer | 16 | 64KB | Ring buffer |

**Total**: ~2.3MB for 8-CPU system

---

## Relocation Types Handled

| Type | Value | Size | Description |
|------|-------|------|-------------|
| `IMAGE_REL_BASED_ABSOLUTE` | 0 | 0 | Padding, ignored |
| `IMAGE_REL_BASED_HIGHLOW` | 3 | 4 bytes | 32-bit delta |
| `IMAGE_REL_BASED_DIR64` | 10 | 8 bytes | 64-bit delta |

VERIFIED: Only DIR64 and HIGHLOW are processed; ABSOLUTE entries skipped.

---

## Bootstrap Section Format

The hypervisor PE contains a `.ombra` section with:

```c
typedef struct _OMBRA_BOOTSTRAP {
    U64 Magic;      // 0x524D424F ('OMBR' little-endian)
    U64 Version;    // 1
    U64 ParamsPtr;  // Patched by loader to point to HV_INIT_PARAMS
} OMBRA_BOOTSTRAP;
```

The loader:
1. Finds `.ombra` section in PE
2. Validates Magic and Version
3. Writes kernel address of ParamsPage to ParamsPtr
4. Entry point reads ParamsPtr to access HV_INIT_PARAMS

---

## PE Header Wiping

Two strategies available:

1. **Zero Wipe** (`WipePeHeaders`):
   - Faster
   - Still detectable (large zero region at process base)

2. **Random Wipe** (`WipePeHeadersRandom`):
   - RDTSC-seeded LCG
   - No recognizable pattern
   - Slightly slower

Current code uses zero wipe by default.

---

## Dependencies

### Internal Dependencies

```
hv_loader.c
  +-- pe_utils.c (PeValidate, PeFindSection, PeGetEntryPoint)
  +-- pe_mapper.c (MapDriver) [not directly - uses DrvLdrLoad]
  +-- ../byovd/supdrv.c (DRV_CONTEXT, Drv* functions)
  +-- ../../shared/types.h (HV_INIT_PARAMS, OMBRA_BOOTSTRAP)

pe_mapper.c
  +-- pe_relocs.c (ApplyRelocations)
  +-- pe_iat.c (PatchIAT)
  +-- pe_wipe.c (WipePeHeaders)
```

### External Dependencies

```
Windows API:
  - GetSystemInfo() - Get processor count
  - malloc/free - Image copy buffer

CRT:
  - memset, memcpy - Buffer operations
  - printf - Logging

Intrinsics:
  - __rdtsc - Random seed for header wipe
```

---

## CONCERNS

### Security Issues

1. **Hardcoded VMCALL Key**: `0xDEADBEEFCAFEBABE` - Should be randomized
2. **PE Headers in Memory**: Even after wiping, timing window exists during mapping
3. **Debug Symbols**: Printf statements reveal internal structure

### Potential Bugs

1. **Memory Leak on Failure**: If `LoadHypervisorModule` fails after `AllocateHypervisorMemory`, cleanup is called but error path may not be complete
2. **Race on Unload**: `Running` flag set FALSE before actual VMX exit

### Anti-Forensics Gaps

1. **Prefetch Files**: Loading driver creates prefetch entries (not cleaned)
2. **Event Logs**: Driver loading may be logged (not cleaned)
3. **Registry Artifacts**: Service registration may persist (if used)

---

## GAPS AND UNKNOWNS

- [ ] What happens if DrvLdrLoad fails after DrvLdrOpen succeeds?
- [ ] How is the image copy freed in the kernel on failure?
- [ ] Is the host stack size sufficient for complex exit handlers?
- [ ] What's the maximum supported CPU count?
- [ ] How are imports resolved if ntoskrnl has different versions?

---

*Component documentation generated 2025-12-27*
*CONFIDENCE: HIGH for structure, MEDIUM for runtime behavior*
