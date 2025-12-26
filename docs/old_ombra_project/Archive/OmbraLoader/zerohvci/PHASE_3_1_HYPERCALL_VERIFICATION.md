# Phase 3.1: Hypercall Verification

## Overview

After ZeroHVCI obtains kernel R/W and RuntimeHijacker injects the payload into Hyper-V, Phase 3.1 verifies that the hypercall communication channel is working correctly.

## Purpose

The hypercall channel is the only way to communicate with the Ring -1 hypervisor from usermode/kernel code. Before proceeding to map OmbraDriver.sys or perform any hypervisor operations, we must verify:

1. The payload is correctly intercepting CPUID instructions
2. The authentication key system works
3. Basic hypercalls (like reading CR3) function correctly

## Architecture

### Hypercall Mechanism

The Ombra hypervisor uses CPUID as the hypercall instruction because:

- CPUID always causes a VMExit (can't be suppressed)
- Anti-cheats don't hook CPUID (it's harmless from their perspective)
- Works identically on Intel and AMD

**Register Convention:**
```
RCX = VMCALL_TYPE (command code from communication.hpp)
RDX = &COMMAND_DATA (pointer to command structure)
R8  = Optional parameter (target_cr3, etc)
R9  = VMEXIT_KEY XOR 0xBABAB00E (obfuscated authentication key)
RAX = VMX_ROOT_ERROR (return value after VMExit)
```

**Execution Flow:**
1. Usermode/kernel code sets up registers with command + parameters
2. Executes CPUID instruction (any leaf works, we use 0x13371337 for debugging)
3. VMExit occurs - control transfers to payload's vmexit_handler
4. Payload deobfuscates R9, validates key, executes command
5. Payload sets RAX to VMX_ROOT_ERROR result
6. VMRESUME returns to guest
7. Caller reads RAX to get result

### Authentication System

The payload validates every hypercall using a session key:

1. First call: `VMCALL_SET_COMM_KEY` with key=0 (bootstrap)
   - Payload allows key=0 ONLY for this command
   - Stores new key in VMXRoot storage

2. All subsequent calls: Must provide correct session key
   - Key is XOR'd with 0xBABAB00E before CPUID
   - Payload deobfuscates and validates
   - Invalid key → hypercall ignored, returns error

This prevents anti-cheats or other code from accidentally triggering our VMExit handler.

## Implementation

### Files

- `hypercall_verify.h` - Core implementation (header-only)
- `HYPERCALL_VERIFY_EXAMPLE.cpp` - Integration examples

### API

```cpp
namespace zerohvci::hypercall {

// Generate random 64-bit session key
uint64_t GenerateSessionKey();

// Set authentication key with hypervisor (bootstrap call)
bool SetCommunicationKey(uint64_t key);

// Test hypercall by reading current CR3
bool VerifyHypercallWorks(uint64_t* outCr3, uint64_t session_key);

// Complete verification sequence (all-in-one)
uint64_t VerifyHypervisorActive();

}
```

### Usage Pattern

**After RuntimeHijacker.HijackHyperV():**

```cpp
#include "hypercall_verify.h"

// Simple verification
uint64_t session_key = zerohvci::hypercall::VerifyHypervisorActive();
if (session_key == 0) {
    printf("Hypercall verification failed\n");
    return -1;
}

// session_key is now valid for all hypercalls
// Pass it to libombra functions or use ExecuteHypercall directly
```

**Manual step-by-step:**

```cpp
// 1. Generate key
uint64_t key = zerohvci::hypercall::GenerateSessionKey();

// 2. Set key with hypervisor
if (!zerohvci::hypercall::SetCommunicationKey(key)) {
    // Payload not responding
    return -1;
}

// 3. Test with CR3 read
uint64_t cr3 = 0;
if (!zerohvci::hypercall::VerifyHypercallWorks(&cr3, key)) {
    // Hypercall failed
    return -1;
}

// All verified - use 'key' for future hypercalls
```

## Technical Details

### Key Generation

Uses RDTSC (CPU timestamp counter) combined with ProcessId for entropy:

```cpp
uint64_t rdtsc_val = __rdtsc();
uint64_t pid = static_cast<uint64_t>(GetCurrentProcessId());
uint64_t key = rdtsc_val ^ (pid << 32) ^ (pid >> 32);
```

This provides sufficient randomness for a session key. The key doesn't need cryptographic strength - it's just preventing accidental collisions.

### CPUID Inline Assembly

MSVC x64 inline assembly for the hypercall:

```cpp
__asm {
    push rbx                    // Save callee-saved register
    mov rax, obfuscated_key
    mov r9, rax                 // R9 = key XOR 0xBABAB00E

    mov rcx, code               // VMCALL_TYPE
    mov rdx, param1             // &COMMAND_DATA
    mov r8, param2              // Optional param

    mov eax, 0x13371337         // Magic CPUID leaf
    cpuid                       // VMExit here

    mov result, rax             // RAX = VMX_ROOT_ERROR
    pop rbx
}
```

### CR3 Validation

After reading CR3, we validate the result:

- CR3 must be non-zero
- CR3 must be page-aligned (low 12 bits clear)
- CR3 contains the physical address of the page directory

If these checks fail, the hypercall didn't actually work (payload returned garbage or default value).

## Debugging

### If VMCALL_SET_COMM_KEY Fails

**Symptoms:** SetCommunicationKey() returns false

**Possible Causes:**
1. Payload not injected into Hyper-V
   - Check RuntimeHijacker.GetPayloadBase() is non-zero
   - Verify payload file exists and was read correctly

2. VMExit handler not patched
   - Check RuntimeHijacker.GetHyperVInfo().VmExitCallAddr
   - Verify signature scanning found the handler

3. Payload crashed during initialization
   - Check payload's DllMain executed successfully
   - Look for null pointer dereferences in payload init code

4. Wrong CPU architecture
   - Intel payload on AMD CPU (or vice versa)
   - Check hijacker.IsIntel() matches payload filename

### If VMCALL_GET_CR3 Fails

**Symptoms:** VerifyHypercallWorks() returns false or CR3 is invalid

**Possible Causes:**
1. Key validation failing
   - XOR obfuscation not matching (check 0xBABAB00E constant)
   - Key not stored correctly in VMXRoot storage

2. COMMAND_DATA pointer issues
   - Pointer not in accessible memory
   - Payload can't translate virtual address

3. CR3 read implementation broken
   - Intel: `__vmx_vmread(VMCS_GUEST_CR3)` failing
   - AMD: VMCB CR3 field access failing

4. Return value corruption
   - RAX getting clobbered before return
   - Inline assembly register constraints wrong

### Common Mistakes

**Forgetting to set key before other hypercalls:**
```cpp
// WRONG - no key set
uint64_t cr3;
zerohvci::hypercall::VerifyHypercallWorks(&cr3, 0);  // Will fail

// RIGHT - set key first
uint64_t key = zerohvci::hypercall::GenerateSessionKey();
zerohvci::hypercall::SetCommunicationKey(key);
zerohvci::hypercall::VerifyHypercallWorks(&cr3, key);
```

**Using libombra before verification:**
```cpp
// WRONG - libombra needs key set
#include <libombra.hpp>
ombra::set_vmcall_key(some_key);  // Doesn't verify it worked
auto cr3 = ombra::current_dirbase();  // May return garbage

// RIGHT - verify first
uint64_t key = zerohvci::hypercall::VerifyHypervisorActive();
ombra::VMEXIT_KEY = key;  // Now libombra will work
auto cr3 = ombra::current_dirbase();
```

**Terminating process after hijack:**
```cpp
// WRONG - ZeroHVCI kernel R/W dies when process exits
zerohvci::hypercall::VerifyHypervisorActive();
return 0;  // Process exits, kernel R/W gone, payload orphaned

// RIGHT - keep process alive
zerohvci::hypercall::VerifyHypervisorActive();
printf("Press Enter to exit...\n");
getchar();  // Wait for user
zerohvci::Cleanup();
```

## Next Steps

After successful hypercall verification:

1. **Map OmbraDriver.sys** into kernel memory (Phase 3.2)
   - Use VMCALL_WRITE_VIRT to copy driver image
   - Fix relocations and imports
   - Call DriverEntry

2. **Register driver callback** (Phase 3.3)
   - Use VMCALL_STORAGE_QUERY to set CALLBACK_ADDRESS slot
   - Driver can now invoke hypervisor for EPT operations

3. **Hide driver with EPT** (Phase 3.4)
   - Use VMCALL_STORAGE_QUERY to set EPT_HANDLER_ADDRESS
   - Shadow driver pages so anti-cheat can't read them

## Integration with Existing Code

### libombra Compatibility

The hypercall_verify.h module is standalone but compatible with libombra:

```cpp
#include "hypercall_verify.h"
#include <libombra.hpp>

// Verify and get session key
uint64_t key = zerohvci::hypercall::VerifyHypervisorActive();

// Pass to libombra
ombra::VMEXIT_KEY = key;

// Now use libombra API
auto cr3 = ombra::current_dirbase();
auto root_cr3 = ombra::root_dirbase();
```

### OmbraLoader Integration

Add to OmbraLoader's main flow:

```cpp
// After bootkit installation or hypervisor detection
if (IsHypervisorActive()) {
    // In bootkit mode, hypervisor is active from boot
    uint64_t key = zerohvci::hypercall::VerifyHypervisorActive();

    if (key != 0) {
        // Proceed to driver mapping
        MapOmbraDriver(key);
    }
} else {
    // Runtime hijack mode
    zerohvci::Initialize();
    zerohvci::hyperv::RuntimeHijacker hijacker;
    hijacker.HijackHyperV(payload, payload_size);

    uint64_t key = zerohvci::hypercall::VerifyHypervisorActive();
    // ...
}
```

## Payload Side

The payload must handle these VMCALLs in its vmexit_handler:

**Intel (VMX):**
```cpp
case VMCALL_SET_COMM_KEY:
    if (vmexit_key == 0) {  // Bootstrap - allow key=0
        VMEXIT_KEY = param2;  // R8 has new key
        __vmx_vmwrite(VMCS_GUEST_RAX, VMX_ROOT_ERROR::SUCCESS);
    }
    break;

case VMCALL_GET_CR3:
    if (ValidateKey(vmexit_key)) {
        uint64_t cr3;
        __vmx_vmread(VMCS_GUEST_CR3, &cr3);
        pCommandData->cr3.value = cr3;
        __vmx_vmwrite(VMCS_GUEST_RAX, VMX_ROOT_ERROR::SUCCESS);
    }
    break;
```

**AMD (SVM):**
```cpp
case VMCALL_SET_COMM_KEY:
    if (IsVmcall(context)) {  // Bootstrap - IsVmcall allows invalid key initially
        VMEXIT_KEY = param2;
        context->rax = VMX_ROOT_ERROR::SUCCESS;
    }
    break;

case VMCALL_GET_CR3:
    if (IsVmcall(context)) {
        pCommandData->cr3.value = vmcb->Cr3();
        context->rax = VMX_ROOT_ERROR::SUCCESS;
    }
    break;
```

## References

- `libombra/com.asm` - Assembly hypercall implementation
- `libombra/libombra.cpp` - Reference hypercall usage
- `OmbraShared/communication.hpp` - VMCALL_TYPE definitions and COMMAND_DATA structures
- `PayLoad/core/dispatch.cpp` - Payload hypercall dispatch logic (unified architecture)
- `PayLoad/intel/vmx_handler.cpp` - Intel VMExit handler implementation
- `PayLoad/amd/svm_handler.cpp` - AMD VMExit handler implementation
