# A9: Communications Knowledge Base

## Overview

This document captures the essential patterns and implementation details for usermode-to-hypervisor communication extracted from Sputnik, hyper-reV, Kernel-Bridge, and NoirVisor reference implementations. It forms the foundation for OmbraHypervisor's hypercall interface design.

---

## 1. Hypercall Interface Design

### 1.1 Intel VT-x: VMCALL Instruction

The VMCALL instruction causes an unconditional VM-exit when executed in VMX non-root operation (guest mode). Parameters are passed via general-purpose registers.

**Assembly Implementation (Sputnik)**
```asm
; File: /Refs/Sputnik/SKLib/SKLib/SKLib/src/cpu-helper.asm:274-280
; NTSTATUS VMCALL(ULONG64 ulCallNum, ULONG64 ulOpt1, ULONG64 ulOpt2, ULONG64 ulOpt3);
VmxVMCALL PROC
    mov rax, r9           ; Move key to RAX
    xor r9, 0deada55h     ; XOR key for validation marker
    vmcall                ; Trigger VM-exit
    ret
VmxVMCALL ENDP
```

**CPUID-based Alternative (Sputnik)**
```asm
; File: /Refs/Sputnik/SKLib/SKLib/SKLib/src/cpu-helper.asm:282-290
; Uses CPUID as hypercall trigger instead of VMCALL
CPUIDVmCall PROC
    mov rax, r9           ; Key in RAX
    xor r9, 0deada55h     ; Validation marker
    push rbx              ; Save RBX (CPUID modifies it)
    cpuid                 ; Trigger VM-exit via CPUID intercept
    pop rbx
    ret
CPUIDVmCall ENDP
```

**Register Convention (Intel)**
| Register | Purpose |
|----------|---------|
| RCX | Command ID / First parameter |
| RDX | Parameter 1 or pointer to request struct |
| R8 | Parameter 2 |
| R9 | Authentication key (XOR'd with marker) |
| RAX | Return value (status code) |

### 1.2 AMD-V: VMMCALL Instruction

AMD-V uses the VMMCALL instruction which triggers a #VMEXIT with exit code `VMEXIT_VMMCALL` (0x81).

**VMCB Intercept Configuration**
```cpp
// File: /Refs/Kernel-Bridge/CommonTypes/SVM.h:169-170
unsigned short InterceptVmrun : 1;
unsigned short InterceptVmcall : 1;   // Enable VMMCALL interception
```

**Exit Code Reference**
```cpp
// File: /Refs/Kernel-Bridge/CommonTypes/SVM.h:573
VMEXIT_VMMCALL,  // Exit code 0x81 in SVM_EXIT_CODE enum
```

**Register Convention (AMD)**
| Register | Purpose |
|----------|---------|
| RAX | Communication key |
| RCX | Command ID |
| RDX | Parameter 1 |
| R8 | Parameter 2 |
| R9 | Parameter 3 |

---

## 2. Command Dispatch Architecture

### 2.1 Command ID Enumeration (Sputnik)

```cpp
// File: /Refs/Sputnik/SKLib/SKLib/SKLib-v/include/Vmcall.h:13-34
enum VMCALL_TYPE {
    VMCALL_TEST = 0x1,
    VMCALL_VMXOFF,
    VMCALL_INVEPT_CONTEXT,
    VMCALL_HOOK_PAGE,
    VMCALL_UNHOOK_PAGE,
    VMCALL_HOOK_PAGE_RANGE,
    VMCALL_HOOK_PAGE_INDEX,
    VMCALL_SUBSTITUTE_PAGE,
    VMCALL_CRASH,
    VMCALL_PROBE,
    VMCALL_READ_VIRT,
    VMCALL_WRITE_VIRT,
    VMCALL_READ_PHY,
    VMCALL_WRITE_PHY,
    VMCALL_DISABLE_EPT,
    VMCALL_SET_COMM_KEY,
    VMCALL_GET_CR3,
    VMCALL_GET_EPT_BASE,
    VMCALL_VIRT_TO_PHY,
    VMCALL_STORAGE_QUERY
};
```

### 2.2 Command ID Enumeration (hyper-reV)

```cpp
// File: /Refs/hyper-reV/shared/hypercall/hypercall_def.h:5-17
enum class hypercall_type_t : std::uint64_t
{
    guest_physical_memory_operation,
    guest_virtual_memory_operation,
    translate_guest_virtual_address,
    read_guest_cr3,
    add_slat_code_hook,
    remove_slat_code_hook,
    hide_guest_physical_page,
    log_current_state,
    flush_logs,
    get_heap_free_page_count
};
```

### 2.3 Dispatch Handler Pattern (Sputnik)

```cpp
// File: /Refs/Sputnik/SKLib/SKLib/SKLib-v/src/Vmcall.cpp:69-328
NTSTATUS vmcall::HandleVmcall(ULONG64 ulCallNum, ULONG64& ulOpt1,
                               ULONG64& ulOpt2, ULONG64& ulOpt3)
{
    ULONG dwCore = CPU::GetCPUIndex(true);
    NTSTATUS ntVmcallStatus = STATUS_UNSUCCESSFUL;
    lastCr3 = vmm::GetGuestCR3().Flags;

    switch (ulCallNum)
    {
    case VMCALL_SET_COMM_KEY:
    {
        if (!communicationKey || communicationKey == ulOpt2) {
            communicationKey = ulOpt1;
            ntVmcallStatus = STATUS_SUCCESS;
        }
        break;
    }
    case VMCALL_TEST:
    {
        ntVmcallStatus = (NTSTATUS)'ImON';  // Magic response
        break;
    }
    case VMCALL_GET_CR3:
    {
        DWORD64* pOutCr3 = (DWORD64*)paging::vmmhost::MapGuestToHost(
            vmm::GetGuestCR3().Flags, (PVOID)ulOpt1);
        if(pOutCr3)
            *pOutCr3 = vmm::GetGuestCR3().Flags;
        ntVmcallStatus = STATUS_SUCCESS;
        break;
    }
    // ... additional cases
    default:
    {
        fnVmcallCallback pCallback = FindHandler(ulCallNum);
        if (!pCallback) {
            ntVmcallStatus = STATUS_UNSUCCESSFUL;
        }
        else {
            ntVmcallStatus = pCallback(ulOpt1, ulOpt2, ulOpt3);
        }
        break;
    }
    }
    return ntVmcallStatus;
}
```

### 2.4 Dispatch Handler Pattern (hyper-reV)

```cpp
// File: /Refs/hyper-reV/hyperv-attachment/src/hypercall/hypercall.cpp:219-311
void hypercall::process(const hypercall_info_t hypercall_info,
                        trap_frame_t* const trap_frame)
{
    switch (hypercall_info.call_type)
    {
    case hypercall_type_t::guest_physical_memory_operation:
    {
        const auto memory_operation =
            static_cast<memory_operation_t>(hypercall_info.call_reserved_data);
        trap_frame->rax = operate_on_guest_physical_memory(trap_frame,
                                                           memory_operation);
        break;
    }
    case hypercall_type_t::read_guest_cr3:
    {
        const cr3 guest_cr3 = arch::get_guest_cr3();
        trap_frame->rax = guest_cr3.flags;
        break;
    }
    // ... additional cases
    }
}
```

---

## 3. Request/Response Data Structures

### 3.1 Memory Read/Write Structures (Sputnik)

```cpp
// File: /Refs/Sputnik/SKLib/SKLib/SKLib-v/include/VMMDef.h:274-296
namespace vmm {
    typedef struct _READ_DATA {
        PVOID pOutBuf;      // Destination buffer
        PVOID pTarget;      // Source address to read
        DWORD64 length;     // Bytes to read
    } READ_DATA, *PREAD_DATA;

    typedef struct _WRITE_DATA {
        PVOID pInBuf;       // Source buffer
        PVOID pTarget;      // Destination address to write
        DWORD64 length;     // Bytes to write
    } WRITE_DATA, *PWRITE_DATA;

    typedef struct _TRANSLATION_DATA {
        PVOID va;           // Virtual address to translate
        DWORD64 pa;         // Resulting physical address
    } TRANSLATION_DATA, *PTRANSLATION_DATA;
}
```

### 3.2 Hypercall Info Structure (hyper-reV)

```cpp
// File: /Refs/hyper-reV/shared/hypercall/hypercall_def.h:22-36
constexpr std::uint64_t hypercall_primary_key = 0x4E47;
constexpr std::uint64_t hypercall_secondary_key = 0x7F;

union hypercall_info_t
{
    std::uint64_t value;
    struct
    {
        std::uint64_t primary_key : 16;         // Authentication key part 1
        hypercall_type_t call_type : 4;         // Command ID
        std::uint64_t secondary_key : 7;        // Authentication key part 2
        std::uint64_t call_reserved_data : 37;  // Additional parameters
    };
};
```

---

## 4. Usermode Client Implementation

### 4.1 Hypervisor Presence Detection

```cpp
// File: /Refs/Sputnik/SKLib/SKLib/SKLib/include/cpu.h:367-377
__forceinline bool IsHypervOn(ULONG64 key = 0)
{
    INT32 CpuInfo[4] = { 0 };
    __cpuidex(CpuInfo, 'Hypr', 'Chck');  // Custom CPUID leaf

    if (CpuInfo[0] != 'Yass')
    {
        // Fallback to VMCALL test
        return CPUIDVmCall(0x1 /*VMCALL_TEST*/, 0, 0, key) == 'ImON';
    }
    return true;
}
```

### 4.2 Usermode Hypercall Wrapper (Sputnik libsputnik)

```cpp
// File: /Refs/Sputnik/libsputnik/libsputnik.hpp:35-41
extern "C" auto hypercall(u64 code, PCOMMAND_DATA param1,
                          u64 param2, u64 key) -> VMX_ROOT_ERROR;

template<typename T>
auto hypercall(u64 code, T param1, u64 param2, u64 key) -> VMX_ROOT_ERROR
{
    return hypercall(code, (PCOMMAND_DATA)param1, param2, key);
}
```

### 4.3 High-Level Client Functions

```cpp
// File: /Refs/Sputnik/libsputnik/libsputnik.cpp:82-127
auto sputnik::read_phys(guest_phys_t phys_addr, guest_virt_t buffer,
                        u64 size) -> VMX_ROOT_ERROR
{
    COMMAND_DATA command = { 0 };
    command.read.length = size;
    command.read.pOutBuf = (PVOID)buffer;
    command.read.pTarget = (PVOID)phys_addr;
    return hypercall(VMCALL_TYPE::VMCALL_READ_PHY, &command, 0, VMEXIT_KEY);
}

auto sputnik::read_virt(guest_virt_t virt_addr, guest_virt_t buffer,
                        u64 size, u64 target_cr3) -> VMX_ROOT_ERROR
{
    COMMAND_DATA command = { 0 };
    command.read.length = size;
    command.read.pOutBuf = (PVOID)virt_addr;
    command.read.pTarget = (PVOID)buffer;
    return hypercall(VMCALL_TYPE::VMCALL_READ_VIRT, &command, target_cr3,
                     VMEXIT_KEY);
}

auto sputnik::virt_to_phy(guest_virt_t p, u64 dirbase) -> guest_phys_t
{
    COMMAND_DATA command = { 0 };
    command.translation.va = (void*)p;
    auto status = hypercall(VMCALL_TYPE::VMCALL_VIRT_TO_PHY, &command,
                            dirbase, VMEXIT_KEY);
    if (status != VMX_ROOT_ERROR::SUCCESS) {
        return 0;
    }
    return command.translation.pa;
}
```

### 4.4 Memory Locking for Safe DMA

```cpp
// File: /Refs/Sputnik/libsputnik/libsputnik.hpp:67-83
__forceinline auto malloc_locked(u64 size) -> guest_virt_t
{
#ifndef _KERNEL_MODE
    auto p = malloc(size);
    if (!VirtualLock(p, size)) {
        // Expand working set if needed
        if (!SetProcessWorkingSetSize(
            OpenProcess(PROCESS_ALL_ACCESS | PROCESS_SET_QUOTA, FALSE,
                        GetCurrentProcessId()),
            0x200 * 0x200 * 0x1000,
            0x200 * 0x200 * 0x1000)) {
            if (p) free(p);
            return 0;
        }
        VirtualLock(p, size);
    }
    return (guest_virt_t)p;
#else
    return 0;
#endif
}
```

---

## 5. Authentication and Validation

### 5.1 Communication Key Validation (Sputnik)

```cpp
// File: /Refs/Sputnik/SKLib/SKLib/SKLib-v/src/Vmcall.cpp:54-67
ULONG64 communicationKey = 0;

bool vmcall::ValidateCommunicationKey(ULONG64 key)
{
    // Accept if no key set, or key matches
    return !communicationKey || communicationKey == key;
}

bool vmcall::IsVmcall(ULONG64 r9)
{
    // XOR marker validation (0xdeada55)
    return r9 == (communicationKey ^ 0xdeada55);
}
```

### 5.2 Setting Communication Key

```cpp
// File: /Refs/Sputnik/SKLib/SKLib/SKLib-v/src/Vmcall.cpp:78-85
case VMCALL_SET_COMM_KEY:
{
    // Only set if not already set, or if old key provided
    if (!communicationKey || communicationKey == ulOpt2) {
        communicationKey = ulOpt1;
        ntVmcallStatus = STATUS_SUCCESS;
    }
    break;
}
```

### 5.3 Dual-Key Authentication (hyper-reV)

```cpp
// File: /Refs/hyper-reV/shared/hypercall/hypercall_def.h:22-24
constexpr std::uint64_t hypercall_primary_key = 0x4E47;
constexpr std::uint64_t hypercall_secondary_key = 0x7F;

// Bitfield includes both keys for validation
struct {
    std::uint64_t primary_key : 16;    // Must match 0x4E47
    hypercall_type_t call_type : 4;
    std::uint64_t secondary_key : 7;   // Must match 0x7F
    std::uint64_t call_reserved_data : 37;
};
```

---

## 6. Secure Data Exchange

### 6.1 Guest-to-Host Address Mapping

```cpp
// File: /Refs/Sputnik/SKLib/SKLib/SKLib-v/src/Vmcall.cpp:159-196
case VMCALL_READ_VIRT:
{
    CR3 guestCr3 = vmm::GetGuestCR3();
    CR3 cr3 = { 0 };

    // Determine target CR3
    if (ulOpt2 == TARGET_CR3_SYSTEM) {
        cr3.Flags = vmm::guestCR3.Flags;
    } else if (ulOpt2 == TARGET_CR3_CURRENT) {
        cr3.Flags = guestCr3.Flags;
    } else {
        cr3.Flags = ulOpt2;
    }

    // Map guest virtual to host virtual
    vmm::PREAD_DATA readData = (vmm::PREAD_DATA)
        paging::vmmhost::MapGuestToHost(guestCr3.Flags, (PVOID)ulOpt1,
                                         MAP_TYPE::src);
    if (!readData) {
        ntVmcallStatus = EXIT_ERRORS::ERROR_CANNOT_MAP_PARAM;
        break;
    }

    // Extract parameters from mapped structure
    SIZE_T length = readData->length;
    PVOID pOutBuf = readData->pOutBuf;
    PVOID pTarget = readData->pTarget;

    if (!pOutBuf || !pTarget) {
        ntVmcallStatus = EXIT_ERRORS::ERROR_INVALID_PARAM;
        break;
    }

    ntVmcallStatus = paging::vmmhost::ReadVirtMemory(pOutBuf, pTarget,
                                                      length, cr3);
    break;
}
```

### 6.2 Cross-Page Memory Operations (hyper-reV)

```cpp
// File: /Refs/hyper-reV/hyperv-attachment/src/hypercall/hypercall.cpp:66-128
std::uint64_t operate_on_guest_virtual_memory(
    const trap_frame_t* const trap_frame,
    const memory_operation_t operation,
    const std::uint64_t address_of_page_directory)
{
    const cr3 guest_source_cr3 = {
        .address_of_page_directory = address_of_page_directory
    };
    const cr3 guest_destination_cr3 = arch::get_guest_cr3();
    const cr3 slat_cr3 = slat::hyperv_cr3();

    std::uint64_t bytes_copied = 0;
    std::uint64_t size_left_to_read = trap_frame->r9;

    while (size_left_to_read != 0)
    {
        std::uint64_t size_left_of_destination_virtual_page = UINT64_MAX;
        std::uint64_t size_left_of_source_virtual_page = UINT64_MAX;

        // Translate both source and destination
        const std::uint64_t guest_source_physical_address =
            memory_manager::translate_guest_virtual_address(
                guest_source_cr3, slat_cr3,
                { .address = guest_source_virtual_address + bytes_copied },
                &size_left_of_source_virtual_page);

        // Handle page boundaries correctly
        const std::uint64_t copy_size =
            crt::min(size_left_to_read, size_left_of_pages);

        if (copy_size == 0) break;

        crt::copy_memory(host_destination, host_source, copy_size);

        size_left_to_read -= copy_size;
        bytes_copied += copy_size;
    }

    return bytes_copied;
}
```

---

## 7. Error Handling and Status Codes

### 7.1 VMX Error Codes

```cpp
// File: /Refs/Sputnik/SKLib/SKLib/SKLib/include/cpu.h:336-341
typedef enum _VMX_ERROR
{
    VMX_ERROR_CODE_SUCCESS = 0,
    VMX_ERROR_CODE_FAILED_WITH_STATUS = 1,
    VMX_ERROR_CODE_FAILED = 2
} VMX_ERROR;
```

### 7.2 Exit Error Codes

Common patterns observed:
- `STATUS_SUCCESS` (0) - Operation completed
- `STATUS_UNSUCCESSFUL` - Generic failure
- `'ImON'` (0x496D4F4E) - Hypervisor presence confirmation
- Custom error codes for specific failures

### 7.3 Sputnik Error Pattern

```cpp
// File: /Refs/Sputnik/SKLib/SKLib/SKLib-v/src/Vmcall.cpp
// Error codes used in VMCALL handler:
EXIT_ERRORS::ERROR_CANNOT_MAP_PARAM   // Failed to map guest parameter
EXIT_ERRORS::ERROR_INVALID_PARAM      // Null or invalid parameter
EXIT_ERRORS::ERROR_CANNOT_MAP_DST     // Failed to map destination
EXIT_ERRORS::ERROR_SUCCESS            // Operation succeeded
```

---

## 8. VM-Exit Handling for Hypercalls

### 8.1 Intel VT-x Exit Reason

```cpp
// File: /Refs/Sputnik/SKLib/SKLib/SKLib-v/include/VMMDef.h:150
EXIT_REASON_VMCALL = 18,  // VM-exit due to VMCALL instruction

// File: /Refs/Kernel-Bridge/CommonTypes/VMX.h:286
EXIT_REASON_VMCALL = 18,
```

### 8.2 AMD-V Exit Code

```cpp
// File: /Refs/Kernel-Bridge/CommonTypes/SVM.h:573
VMEXIT_VMMCALL,  // Exit code when VMMCALL is intercepted
```

### 8.3 Exit Handler Integration

The hypervisor's exit handler checks the exit reason and routes to the hypercall dispatcher:

```cpp
// Conceptual pattern from analysis:
void HandleVmExit(PREGS pGuestRegs)
{
    ULONG exitReason = GetExitReason();

    switch (exitReason)
    {
    case EXIT_REASON_VMCALL:
        // Validate this is our hypercall
        if (vmcall::IsVmcall(pGuestRegs->r9)) {
            pGuestRegs->rax = vmcall::HandleVmcall(
                pGuestRegs->rcx,    // Command
                pGuestRegs->rdx,    // Param1
                pGuestRegs->r8,     // Param2
                pGuestRegs->r9      // Key
            );
            MoveRip();  // Advance past VMCALL
        }
        break;
    case EXIT_REASON_CPUID:
        // Check for CPUID-based hypercall
        // ...
        break;
    }
}
```

---

## 9. Recommended OmbraHypervisor Design

### 9.1 Command Enumeration

```cpp
namespace ombra {

enum class HypercallCommand : uint64_t {
    // Core commands
    OMBRA_INIT = 0x01,
    OMBRA_GET_CR3 = 0x02,
    OMBRA_READ_PHYS = 0x03,
    OMBRA_WRITE_PHYS = 0x04,
    OMBRA_READ_VIRT = 0x05,
    OMBRA_WRITE_VIRT = 0x06,
    OMBRA_TRANSLATE = 0x07,

    // Hook commands
    OMBRA_HOOK_PAGE = 0x08,
    OMBRA_UNHOOK_PAGE = 0x09,

    // Auth commands
    OMBRA_SET_AUTH_PID = 0x0A,
    OMBRA_SPOOF_ENABLE = 0x0B,

    // System
    OMBRA_GET_VERSION = 0x0C,
};

} // namespace ombra
```

### 9.2 Authentication Structure

```cpp
namespace ombra {

constexpr uint64_t OMBRA_COMM_KEY = 0xDEADBEEFDEADBEEF;
constexpr uint64_t OMBRA_VALIDATION_XOR = 0x0DEAD055;

struct AuthContext {
    uint64_t communicationKey;
    uint64_t authorizedPid;
    bool isInitialized;
};

bool ValidateRequest(uint64_t key, uint64_t pid,
                     const AuthContext& ctx) {
    if (key != ctx.communicationKey)
        return false;
    if (ctx.authorizedPid != 0 && pid != ctx.authorizedPid)
        return false;
    return true;
}

} // namespace ombra
```

### 9.3 Request/Response Protocol

```cpp
namespace ombra {

struct MemoryRequest {
    void* targetAddress;
    void* bufferAddress;
    uint64_t size;
    uint64_t targetCr3;
};

struct TranslationRequest {
    void* virtualAddress;
    uint64_t cr3;
    uint64_t physicalAddressOut;
};

struct HypercallResult {
    uint64_t status;
    uint64_t returnValue;
};

// Register convention:
// RAX = Communication key
// RCX = Command ID
// RDX = Pointer to request structure (or inline param)
// R8  = Secondary parameter
// R9  = Reserved / PID for auth
//
// Return:
// RAX = Status code (0 = success)
// RDX = Return value (command-specific)

} // namespace ombra
```

---

## 10. Security Considerations

1. **Key Rotation**: Consider periodic key rotation to prevent replay attacks
2. **PID Binding**: Bind communication key to a specific process ID
3. **Rate Limiting**: Implement throttling to prevent abuse
4. **Memory Validation**: Always validate guest pointers before mapping
5. **Page Boundary Handling**: Properly handle cross-page memory operations
6. **Locked Memory**: Usermode buffers must be locked (VirtualLock) before hypercall

---

## References

| File | Purpose |
|------|---------|
| `/Refs/Sputnik/SKLib/SKLib/SKLib-v/include/Vmcall.h` | VMCALL types and RW class |
| `/Refs/Sputnik/SKLib/SKLib/SKLib-v/src/Vmcall.cpp` | Command dispatch implementation |
| `/Refs/Sputnik/libsputnik/libsputnik.hpp` | Usermode hypercall interface |
| `/Refs/Sputnik/libsputnik/libsputnik.cpp` | Usermode function implementations |
| `/Refs/Sputnik/SKLib/SKLib/SKLib/src/cpu-helper.asm` | VMCALL/CPUID assembly |
| `/Refs/hyper-reV/shared/hypercall/hypercall_def.h` | Hypercall definitions |
| `/Refs/hyper-reV/hyperv-attachment/src/hypercall/hypercall.cpp` | Dispatch and memory ops |
| `/Refs/hyper-reV/usermode/src/hypercall/hypercall.h` | Usermode interface |
| `/Refs/Kernel-Bridge/CommonTypes/VMX.h` | VMX exit reasons |
| `/Refs/Kernel-Bridge/CommonTypes/SVM.h` | SVM exit codes and VMCB |
| `/Refs/Kernel-Bridge/Kernel-Bridge/API/Hypervisor.h` | KB VMCALL interface |
| `/Refs/NoirVisor/src/vt_core/vt_custom.c` | NoirVisor VT-x guest switching |
| `/Refs/NoirVisor/src/svm_core/svm_custom.c` | NoirVisor SVM guest switching |
