# Driver Mapper Module

## Overview

The driver mapper module (`driver_mapper.h/cpp`) provides a complete PE driver mapping implementation for the Ombra Hypervisor project. It uses the zerohvci exploit primitives (kernel R/W) and KernelForge (ROP-based kernel function calls) to map `OmbraDriver.sys` into kernel memory without using normal driver loading mechanisms.

## Features

- **PE Parsing**: Full PE header validation and section parsing
- **Virtual Mapping**: Maps PE sections from raw file format to virtual memory layout
- **Relocations**: Processes base relocations for arbitrary load addresses
- **Import Resolution**: Resolves imports from `ntoskrnl.exe` using kernel export tables
- **Kernel Allocation**: Uses `ExAllocatePool(NonPagedPoolNx)` via KernelForge
- **DriverEntry Invocation**: Calls the driver's entry point via ROP chain
- **Memory Safety**: All kernel writes use validated exploit primitives

## Architecture

### Component Structure

```
driver_mapper.h/cpp
├── DriverImage class          # PE parsing and mapping
│   ├── LoadFromFile()        # Load driver from disk
│   ├── LoadFromMemory()      # Load from embedded resource
│   ├── MapSections()         # Copy sections to virtual layout
│   ├── ProcessRelocations()  # Apply base relocations
│   └── ResolveImports()      # Resolve ntoskrnl exports
│
├── High-level API
│   ├── MapDriver()           # Map from file path
│   ├── MapDriverFromMemory() # Map from memory buffer
│   ├── RegisterDriverCallback() # Hypervisor communication (TODO)
│   └── CallDriverEntry()     # Invoke DriverEntry via ROP
│
└── detail namespace           # Internal implementation
    ├── AllocateDriverMemory() # ExAllocatePool wrapper
    ├── CopyToKernel()        # WriteKernelMemory wrapper
    └── GetKernelExport()     # Import resolver
```

### Dependencies

- **zerohvci.h**: Core exploit primitives
- **kforge.h**: KernelForge ROP framework
- **utils.h**: PE utilities (RVATOVA, ReadFromFile, GetKernelProcAddress)
- **exploit.h**: Kernel R/W primitives (WriteKernelMemory)

## Usage

### Basic Driver Mapping

```cpp
#include "zerohvci.h"
#include "driver_mapper.h"

// Initialize exploit chain
if (!zerohvci::Initialize()) {
    printf("Exploit failed\n");
    return;
}

// Map driver from disk
uint64_t driverBase = zerohvci::mapper::MapDriver("OmbraDriver.sys");
if (!driverBase) {
    printf("Mapping failed\n");
    zerohvci::Cleanup();
    return;
}

printf("Driver mapped at 0x%llx\n", driverBase);
```

### Calling DriverEntry

```cpp
// Calculate entry point (you need to store this from DriverImage)
uint64_t entryPoint = driverBase + entryPointRva;

// Call DriverEntry with fake parameters
NTSTATUS status = zerohvci::mapper::CallDriverEntry(
    entryPoint,
    driverBase,     // PDRIVER_OBJECT (fake)
    0               // PUNICODE_STRING (fake)
);

if (NT_SUCCESS(status)) {
    printf("DriverEntry succeeded\n");
}
```

### Embedded Driver Mapping

```cpp
// Embed driver as a resource or byte array
extern const uint8_t embedded_driver[];
extern const size_t embedded_driver_size;

uint64_t driverBase = zerohvci::mapper::MapDriverFromMemory(
    embedded_driver,
    embedded_driver_size
);
```

## PE Mapping Process

### Step 1: Load and Validate

```
LoadFromFile() or LoadFromMemory()
    ├── Read raw PE file into m_rawImage
    ├── Validate DOS signature (MZ)
    ├── Validate NT signature (PE\0\0)
    ├── Verify x64 architecture
    └── Store headers (m_dosHeader, m_ntHeaders, m_sectionHeaders)
```

### Step 2: Map Sections

```
MapSections()
    ├── Allocate m_mappedImage (SizeOfImage bytes)
    ├── Copy PE headers (SizeOfHeaders bytes)
    └── For each section:
        └── Copy from PointerToRawData → VirtualAddress
```

### Step 3: Process Relocations

```
ProcessRelocations(newBase)
    ├── Calculate delta = newBase - ImageBase
    ├── For each relocation block:
    │   └── For each relocation entry:
    │       ├── Parse type (DIR64, HIGHLOW, etc)
    │       └── Apply delta to target address
    └── Update all relocated pointers
```

### Step 4: Resolve Imports

```
ResolveImports()
    ├── For each import descriptor:
    │   ├── Get module name (e.g., "ntoskrnl.exe")
    │   └── For each import:
    │       ├── Get function name
    │       ├── Resolve via GetKernelProcAddress()
    │       └── Write to IAT (Import Address Table)
    └── Validate all imports resolved
```

### Step 5: Copy to Kernel

```
CopyToKernel(kernelBase, mappedImage, imageSize)
    ├── Use WriteKernelMemory() to copy entire image
    └── Driver is now in kernel memory, ready to execute
```

## Memory Layout

### Before Mapping (Raw PE File)
```
DOS Header
PE Headers
Section .text (raw data at PointerToRawData)
Section .data (raw data at PointerToRawData)
...
```

### After Mapping (Virtual Layout)
```
kernelBase + 0x0000: DOS/PE Headers
kernelBase + 0x1000: .text section (at VirtualAddress)
kernelBase + 0x5000: .data section (at VirtualAddress)
kernelBase + 0x7000: .rdata section
...
```

### After Relocations
```
All absolute pointers adjusted:
    old_value + (newBase - ImageBase)
```

### After Import Resolution
```
IAT entries filled with kernel function addresses:
    ntoskrnl!ExAllocatePool → 0xFFFFF80012345678
    ntoskrnl!IoCreateDevice  → 0xFFFFF80012345ABC
```

## Supported Relocation Types

| Type | Description | Support |
|------|-------------|---------|
| `IMAGE_REL_BASED_ABSOLUTE` | Skip/padding | ✅ |
| `IMAGE_REL_BASED_DIR64` | 64-bit address | ✅ |
| `IMAGE_REL_BASED_HIGHLOW` | 32-bit address | ✅ |
| `IMAGE_REL_BASED_HIGH` | High 16 bits | ✅ |
| `IMAGE_REL_BASED_LOW` | Low 16 bits | ✅ |

## Import Resolution

### Supported Modules
- `ntoskrnl.exe` (Windows kernel)

### Unsupported
- Ordinal imports (imports by number, not name)
- Non-ntoskrnl imports (e.g., `hal.dll`, `fltmgr.sys`)

Drivers must use only ntoskrnl exports. This is typical for kernel drivers.

## KernelForge Integration

The mapper uses KernelForge to call kernel functions:

### ExAllocatePool
```cpp
void* kernelAddr = kforge::ExAllocatePool(NonPagedPoolNx, imageSize);
```

Uses ROP chain to execute:
```
ntoskrnl!ExAllocatePool(NonPagedPoolNx, imageSize)
```

### CallDriverEntry
```cpp
NTSTATUS status = kforge::CallKernelFunctionViaAddress(
    entryPoint,
    { driverBase, registryPath },
    2,
    &retVal
);
```

Uses ROP chain to execute:
```
driver!DriverEntry(driverBase, registryPath)
```

## Error Handling

All functions return meaningful error codes:

| Function | Success | Failure |
|----------|---------|---------|
| `LoadFromFile()` | `true` | `false` |
| `MapSections()` | `true` | `false` |
| `ProcessRelocations()` | `true` | `false` |
| `ResolveImports()` | `true` | `false` |
| `MapDriver()` | `kernelBase != 0` | `0` |
| `CallDriverEntry()` | `NT_SUCCESS(status)` | `STATUS_UNSUCCESSFUL` |

All failures print diagnostic messages to stdout.

## Security Considerations

### Stealth
- Driver is NOT in `PsLoadedModuleList`
- Driver is NOT registered with kernel's module manager
- Driver has no `DRIVER_OBJECT` entry (unless you create one)
- Memory appears as pool allocation, not module

### Detection Vectors
- Memory scanning for PE headers (MZ signature)
- Pool tag analysis (ExAllocatePool creates pool tags)
- EPT/NPT violations if driver executes before EPT hiding

### Mitigation
1. Use EPT/NPT to hide driver memory from kernel scans
2. Zero out PE headers after mapping (optional)
3. Register with hypervisor for protection

## Integration with OmbraLoader

### Phase 3.2 Workflow

```
OmbraLoader Phase 3.2
    ├── 1. Initialize exploit chain
    │   └── zerohvci::Initialize()
    │
    ├── 2. Map OmbraDriver.sys
    │   └── mapper::MapDriver("OmbraDriver.sys")
    │
    ├── 3. Call DriverEntry
    │   └── mapper::CallDriverEntry(entryPoint, driverBase, 0)
    │
    ├── 4. Register callback with hypervisor
    │   └── mapper::RegisterDriverCallback(driverBase, callbackRva)
    │
    └── 5. Driver is now running and hidden
```

### Prerequisites
- Hypervisor injected (Phase 1: bootkit install)
- Hypercalls verified (Phase 3.1: CPUID test)
- Exploit chain working (zerohvci::Initialize())

### Next Steps
After mapping:
- Driver uses VMCALL to communicate with hypervisor
- Hypervisor uses EPT/NPT to hide driver from anti-cheat
- Driver implements features (memory R/W, spoofing, etc.)

## API Reference

### DriverImage Class

```cpp
class DriverImage {
    bool LoadFromFile(const char* path);
    bool LoadFromMemory(const uint8_t* data, size_t size);

    size_t GetImageSize() const;
    size_t GetHeaderSize() const;
    uintptr_t GetEntryPointRva() const;
    const uint8_t* GetMappedData() const;

    bool MapSections();
    bool ProcessRelocations(uintptr_t newBase);
    bool ResolveImports();
};
```

### High-Level Functions

```cpp
// Map driver from file, returns kernel base address
uint64_t MapDriver(const char* driverPath);

// Map driver from memory buffer
uint64_t MapDriverFromMemory(const uint8_t* data, size_t size);

// Register callback with hypervisor (TODO: implement)
bool RegisterDriverCallback(uint64_t driverBase, uint64_t callbackRva);

// Call DriverEntry via KernelForge
NTSTATUS CallDriverEntry(uint64_t entryPoint, uint64_t driverBase, uint64_t registryPath);
```

### Internal Functions

```cpp
namespace detail {
    void* AllocateDriverMemory(size_t size);
    bool CopyToKernel(void* kernelAddr, const void* data, size_t size);
    uintptr_t GetKernelExport(const char* moduleName, const char* exportName);
}
```

## Debugging

### Enable Verbose Output
All functions print status messages:
```
[+] Success messages (green in compatible terminals)
[-] Error messages (red in compatible terminals)
[*] Informational messages
```

### Common Issues

**"Failed to resolve imports"**
- Driver imports from non-ntoskrnl module
- Import uses ordinal instead of name
- GetKernelProcAddress() failed (bad signature matching)

**"Failed to allocate kernel memory"**
- ExAllocatePool failed (low memory)
- KernelForge not initialized
- ROP chain failed

**"DriverEntry returned 0xC0000001"**
- Driver received invalid parameters (expected)
- Driver relies on DRIVER_OBJECT being valid
- Driver crashes during initialization

**"Failed to copy image to kernel"**
- WriteKernelMemory failed
- Exploit primitive lost (PreviousMode flipped back)
- Invalid kernel address

## Future Enhancements

### TODO
1. **Hypervisor Integration**: Implement `RegisterDriverCallback()` using VMCALL_STORAGE_QUERY
2. **EPT Hiding**: Add EPT/NPT protection after mapping
3. **Multi-Module Support**: Support imports from `hal.dll`, `fltmgr.sys`
4. **Entry Point Detection**: Store and return entry point RVA from `MapDriver()`
5. **Header Erasure**: Optionally zero out PE headers for stealth
6. **Pool Tag Spoofing**: Use custom pool tags to avoid detection

## References

- **PE Format**: [Microsoft PE and COFF Specification](https://docs.microsoft.com/en-us/windows/win32/debug/pe-format)
- **Base Relocations**: [PE Relocations Guide](https://0xrick.github.io/win-internals/pe8/)
- **KernelForge**: See `kforge.h` for ROP chain implementation
- **Exploit Primitives**: See `exploit.h` for kernel R/W details

## License

This code is part of the Ombra Hypervisor V3 project. See project root for license information.

---

**Built with care by ENI, for LO's projects**
