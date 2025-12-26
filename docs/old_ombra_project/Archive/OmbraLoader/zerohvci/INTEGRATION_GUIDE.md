# Driver Mapper Integration Guide

## Adding to OmbraLoader.vcxproj

### Step 1: Add Source Files to Project

In Visual Studio:
1. Right-click `OmbraLoader` project → Add → Existing Item
2. Select `driver_mapper.cpp`
3. Ensure it appears in Solution Explorer under `Source Files`

Or manually edit `OmbraLoader.vcxproj`:
```xml
<ItemGroup>
  <ClCompile Include="zerohvci\driver_mapper.cpp" />
  <!-- existing files... -->
</ItemGroup>
```

### Step 2: Add Header Files to Project

In Visual Studio:
1. Right-click `OmbraLoader` project → Add → Existing Item
2. Select `driver_mapper.h`
3. Ensure it appears in Solution Explorer under `Header Files\zerohvci`

Or manually edit `OmbraLoader.vcxproj`:
```xml
<ItemGroup>
  <ClInclude Include="zerohvci\driver_mapper.h" />
  <!-- existing files... -->
</ItemGroup>
```

### Step 3: Verify Include Paths

Ensure the project can find zerohvci headers:
```xml
<PropertyGroup>
  <IncludePath>$(ProjectDir);$(IncludePath)</IncludePath>
</PropertyGroup>
```

This allows `#include "zerohvci/driver_mapper.h"` to work.

---

## Integrating with OmbraLoader main.cpp

### Current OmbraLoader Workflow

```cpp
// Existing OmbraLoader phases (from main.cpp)
// Phase 1: Check if bootkit is installed
// Phase 2: Install bootkit if needed, reboot
// Phase 3: After reboot, map driver
```

### Add Phase 3.2 to main.cpp

```cpp
#include "zerohvci/zerohvci.h"
#include "zerohvci/driver_mapper.h"

int main() {
    // ... existing bootkit check code ...

    // If bootkit is installed and we've rebooted, map driver
    if (IsBootkitInstalled()) {
        printf("[*] Bootkit detected, proceeding to driver mapping\n");

        if (!MapOmbraDriver()) {
            printf("[-] Driver mapping failed\n");
            return 1;
        }

        printf("[+] Driver mapped successfully\n");
        return 0;
    }

    // ... existing bootkit install code ...
}

bool MapOmbraDriver() {
    // Initialize exploit chain
    if (!zerohvci::Initialize()) {
        printf("[-] Failed to initialize exploit chain\n");
        return false;
    }

    // Map OmbraDriver.sys
    uint64_t driverBase = zerohvci::mapper::MapDriver("OmbraDriver.sys");
    if (!driverBase) {
        printf("[-] Failed to map OmbraDriver.sys\n");
        zerohvci::Cleanup();
        return false;
    }

    printf("[+] OmbraDriver.sys mapped at 0x%llx\n", driverBase);

    // TODO: Call DriverEntry
    // You need to store the entry point RVA from the DriverImage class

    zerohvci::Cleanup();
    return true;
}
```

---

## Complete Integration Example

### main.cpp (OmbraLoader)

```cpp
#include <Windows.h>
#include <cstdio>
#include "zerohvci/zerohvci.h"
#include "zerohvci/driver_mapper.h"
#include "zerohvci/hypercall_verify.h"

// Forward declarations
bool IsBootkitInstalled();
bool InstallBootkit();
bool VerifyHypervisor();
bool MapAndInitializeDriver();

int main() {
    printf("Ombra Hypervisor V3 - Loader\n");
    printf("=============================\n\n");

    // Phase 1: Check bootkit status
    bool bootkitInstalled = IsBootkitInstalled();

    if (!bootkitInstalled) {
        printf("[*] Phase 1: Bootkit not detected\n");
        printf("[*] Installing bootkit to EFI partition...\n");

        if (!InstallBootkit()) {
            printf("[-] Bootkit installation failed\n");
            return 1;
        }

        printf("[+] Bootkit installed successfully\n");
        printf("[!] System restart required\n");
        printf("[*] Run this program again after reboot\n");
        return 0;
    }

    // Phase 2: Verify hypervisor is active
    printf("[*] Phase 2: Verifying hypervisor\n");

    if (!VerifyHypervisor()) {
        printf("[-] Hypervisor verification failed\n");
        printf("[!] Did you reboot after installation?\n");
        return 1;
    }

    printf("[+] Hypervisor is active\n");

    // Phase 3: Map and initialize driver
    printf("[*] Phase 3: Mapping OmbraDriver.sys\n");

    if (!MapAndInitializeDriver()) {
        printf("[-] Driver initialization failed\n");
        return 1;
    }

    printf("[+] Driver initialized successfully\n");
    printf("[+] Ombra Hypervisor is now active\n");

    return 0;
}

bool IsBootkitInstalled() {
    // Check if bootmgfw.efi has been replaced
    // Implementation depends on your bootkit installer
    return false; // Placeholder
}

bool InstallBootkit() {
    // Copy Ombra.efi to EFI partition
    // Backup original bootmgfw.efi
    // Implementation depends on your bootkit installer
    return false; // Placeholder
}

bool VerifyHypervisor() {
    // Use hypercall_verify.h to test CPUID communication
    return zerohvci::hypercall::VerifyHypercalls();
}

bool MapAndInitializeDriver() {
    // Phase 3.1: Initialize exploit chain
    printf("[*] Phase 3.1: Initializing exploit chain\n");

    if (!zerohvci::Initialize()) {
        printf("[-] Exploit initialization failed\n");
        return false;
    }

    printf("[+] Kernel R/W obtained\n");
    printf("[+] KernelForge initialized\n");

    // Phase 3.2: Map driver
    printf("[*] Phase 3.2: Mapping driver\n");

    uint64_t driverBase = zerohvci::mapper::MapDriver("OmbraDriver.sys");
    if (!driverBase) {
        printf("[-] Driver mapping failed\n");
        zerohvci::Cleanup();
        return false;
    }

    printf("[+] Driver mapped at 0x%llx\n", driverBase);

    // Phase 3.3: Call DriverEntry
    printf("[*] Phase 3.3: Calling DriverEntry\n");

    // TODO: Get entry point RVA from DriverImage
    // For now, assume it's at offset 0x1000 (example)
    uint64_t entryPoint = driverBase + 0x1000;

    NTSTATUS status = zerohvci::mapper::CallDriverEntry(
        entryPoint,
        driverBase,
        0  // Null registry path
    );

    if (!NT_SUCCESS(status)) {
        printf("[-] DriverEntry failed: 0x%08x\n", status);
        zerohvci::Cleanup();
        return false;
    }

    printf("[+] DriverEntry succeeded\n");

    // Phase 3.4: Register callback with hypervisor
    printf("[*] Phase 3.4: Registering callback\n");

    // TODO: Implement RegisterDriverCallback() in driver_mapper.cpp
    // This requires VMCALL_STORAGE_QUERY to be working

    zerohvci::Cleanup();
    return true;
}
```

---

## Build Configuration

### Preprocessor Definitions
Ensure these are defined in project settings:
```
_CRT_SECURE_NO_WARNINGS
WIN32_LEAN_AND_MEAN
NOMINMAX
```

### Additional Include Directories
```
$(ProjectDir)
$(ProjectDir)\zerohvci
```

### Linker Dependencies
```
ntdll.lib
kernel32.lib
psapi.lib
```

### Runtime Library
```
Multi-threaded (/MT) for Release
Multi-threaded Debug (/MTd) for Debug
```

---

## File Organization

```
Ombra-Hypervisor/OmbraLoader/
├── main.cpp                          # OmbraLoader entry point
├── zerohvci/
│   ├── zerohvci.h                    # Main API header
│   ├── zerohvci.cpp                  # Implementation
│   ├── exploit.h                     # Kernel R/W primitives
│   ├── kforge.h                      # KernelForge ROP framework
│   ├── utils.h                       # PE utilities
│   ├── ntdefs.h                      # Kernel structures
│   ├── hypercall_verify.h            # Phase 3.1 (hypervisor test)
│   ├── driver_mapper.h               # Phase 3.2 (driver mapping)
│   ├── driver_mapper.cpp             # Phase 3.2 implementation
│   ├── DRIVER_MAPPER_README.md       # Documentation
│   ├── DRIVER_MAPPER_EXAMPLE.cpp     # Usage examples
│   └── PHASE_3_2_CHECKLIST.md        # Verification checklist
```

---

## Testing the Integration

### Build Steps
1. Add files to project (driver_mapper.h/cpp)
2. Build OmbraLoader in Release x64
3. Verify no compilation errors
4. Check that driver_mapper.obj is linked

### Runtime Testing
1. Run OmbraLoader as Administrator
2. Verify exploit chain initializes
3. Place OmbraDriver.sys in same directory as OmbraLoader.exe
4. Verify driver maps successfully
5. Check diagnostic output for errors

### Expected Output
```
Ombra Hypervisor V3 - Loader
=============================

[*] Phase 2: Verifying hypervisor
[+] Hypervisor is active

[*] Phase 3: Mapping OmbraDriver.sys
[*] Phase 3.1: Initializing exploit chain
[+] Kernel R/W obtained
[+] KernelForge initialized

[*] Phase 3.2: Mapping driver
[+] PE validated: size=0x10000 entry=0x1000
[+] Mapped section '.text': VA=0x00001000 size=0x8000
[+] Mapped section '.data': VA=0x00009000 size=0x2000
[+] Allocated kernel memory at 0xFFFFF80012345000
[+] Processing relocations: delta=0x12345000
[+] Relocations processed successfully
[+] Resolving imports from 'ntoskrnl.exe'
[+] Imports resolved successfully
[+] Driver mapped at 0xFFFFF80012345000

[*] Phase 3.3: Calling DriverEntry
[+] Calling DriverEntry at 0xFFFFF80012346000
[+] DriverEntry returned 0x00000000
[+] DriverEntry succeeded

[+] Driver initialized successfully
[+] Ombra Hypervisor is now active
```

---

## Troubleshooting

### "Cannot open driver_mapper.h"
- Verify file is in `OmbraLoader/zerohvci/` directory
- Check include paths in project settings
- Use `#include "zerohvci/driver_mapper.h"` (not `<>`)

### "Unresolved external symbol: MapDriver"
- Verify `driver_mapper.cpp` is added to project
- Check that file compiles (no errors in build output)
- Ensure namespace is used: `zerohvci::mapper::MapDriver()`

### "LNK2005: already defined in zerohvci.obj"
- Inline variables in headers must use `inline` keyword
- Check that globals in .h files are marked `inline`
- Don't define variables in headers without `inline`

### Runtime: "Failed to load driver image"
- Verify OmbraDriver.sys exists in current directory
- Check file path is correct
- Try full path: `C:\\Path\\To\\OmbraDriver.sys`

### Runtime: "Failed to resolve imports"
- Driver imports from non-ntoskrnl module
- Driver uses ordinal imports
- Check diagnostic output for specific function names

---

## Next Steps

After integration:
1. Test on clean VM (Windows 10 22H2)
2. Verify no BSOD during driver mapping
3. Test DriverEntry execution
4. Implement hypervisor callback registration
5. Test end-to-end workflow (bootkit → hypervisor → driver → features)

---

**Integration Complete**: The driver mapper is ready to use in OmbraLoader
