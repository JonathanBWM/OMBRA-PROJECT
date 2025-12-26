# CPUID SPOOFING & HYPERVISOR HIDING - C++ to C + Assembly Port Guide

## Overview

CPUID spoofing is the primary anti-detection mechanism in the Ombra hypervisor. When a guest OS executes a `CPUID` instruction, it triggers a VMExit that is intercepted by the payload. The payload executes the real CPUID instruction natively (since CPUID doesn't VMExit in VMXRoot/hypervisor mode), then **modifies the results** before returning them to the guest.

The core strategy:
1. **Intercept ALL CPUID instructions** at VMExit level (exit reason 10 on Intel VMX, VMEXIT_CPUID on AMD SVM)
2. **Execute real CPUID in VMXRoot** using `__cpuidex()` intrinsic (no VMExit occurs)
3. **Mask specific bits** to hide hypervisor presence (ECX[31], VMX/SVM capability bits)
4. **Zero vendor string** in leaf 0x40000000 while preserving max leaf number
5. **Pass through Windows enlightenments unchanged** (leaves 0x40000001-0x4000000F)
6. **Return modified values to guest** via register context (RAX, RBX, RCX, RDX)

This approach hides the hypervisor from ring 0 anti-cheat detection (EAC, BattlEye, Vanguard) while preserving Windows Hyper-V enlightenment functionality.

---

## File Inventory

| File | Lines | Purpose |
|------|-------|---------|
| `PayLoad/core/cpuid_spoof.h` | 72 | CPUID spoofing configuration, constants, API declarations |
| `PayLoad/core/cpuid_spoof.cpp` | 96 | CPUID execution and bit masking implementation |
| `PayLoad/intel/vmx_handler.cpp` | ~450 | Intel VMX VMExit handler with CPUID interception (lines 289-346) |
| `PayLoad/amd/svm_handler.cpp` | ~400 | AMD SVM VMExit handler with CPUID interception (lines 200-257) |
| `PayLoad/include/vmcall.h` | 40 | VMCALL authentication helpers (key validation) |
| `OmbraCoreLib/include/cpu.h` | 556 | CPUID enum definitions, feature bit constants |
| `OmbraCoreLib/src/cpu.cpp` | 496 | CPU detection, timing analysis (uses CPUID leaf 1) |
| `OmbraCoreLib/src/cpu-helper.asm` | 400 | Assembly helpers (CPUID execution, MSR operations) |
| `OmbraCoreLib/include/Arch/Cpuid.h` | 803 | C++ template-based CPUID type system (Intel/AMD layouts) |

**Total CPUID-related code**: ~2,913 lines across 9 files

---

## Architecture Summary

### VMExit Flow for CPUID Instruction

```
Guest OS executes CPUID
    ↓
CPU traps to hypervisor (VMExit/VMM intercept)
    ↓
Intel: vmx_handler.cpp::vmexit_handler()
AMD:   svm_handler.cpp::vmexit_handler()
    ↓
Exit reason == CPUID? (VMX: 10, SVM: VMEXIT_CPUID)
    ↓
YES → HandleCpuid(guest_registers)
    ↓
Check if leaf == 0x13371337 (magic hypercall)?
    ↓
YES → Dispatch to core::HandleVmcall() [authenticated VMCALL]
NO  → cpuid_spoof::ExecuteAndSpoof() [normal CPUID with spoofing]
    ↓
ExecuteAndSpoof:
  1. __cpuidex(leaf, subleaf) → real CPU values
  2. Apply bit masks based on leaf:
     - Leaf 1:        Clear ECX[31] (hypervisor present)
                      Clear ECX[5] (VMX) or ECX[2] (SVM)
     - Leaf 0x40000000: Zero vendor string, keep max leaf
     - Other leaves:  Pass through unchanged
  3. Return modified values
    ↓
Write RAX, RBX, RCX, RDX to guest register context
    ↓
Advance guest RIP (skip CPUID instruction)
    ↓
VMResume (return to guest with spoofed results)
```

**Key Insight**: CPUID executes **natively** in VMXRoot (no VMExit) because the hypervisor is already in the most privileged mode. This allows the payload to get real CPU information then selectively modify it.

---

## Critical Data Structures

### C++ Original (cpuid_spoof.h)

```cpp
namespace cpuid_spoof {

// Magic hypercall leaf - used for authenticated VMCALLs
constexpr u32 HYPERCALL_MAGIC_LEAF = 0x13371337;

// Feature bits to mask
constexpr u32 CPUID_HV_PRESENT = (1u << 31);   // CPUID.01H:ECX[31]
constexpr u32 CPUID_VMX_FEATURE = (1u << 5);   // CPUID.01H:ECX[5]
constexpr u32 CPUID_SVM_FEATURE = (1u << 2);   // CPUID.80000001H:ECX[2]

// Hypervisor vendor CPUID leaf
constexpr u32 CPUID_HV_VENDOR = 0x40000000;

struct Config {
    bool hide_hypervisor;      // Clear ECX[31] in leaf 1
    bool hide_vmx;             // Clear ECX[5] in leaf 1 (Intel)
    bool hide_svm;             // Clear ECX[2] in leaf 0x80000001 (AMD)
    bool spoof_hv_vendor;      // Zero vendor string in 0x40000000
    bool compensate_tsc;       // Future: TSC compensation
    u64 cpuid_tsc_offset;
    bool spoof_brand_string;   // Future: custom brand
    char custom_brand[48];
};

extern Config g_config;

void Initialize();
void ExecuteAndSpoof(u32 leaf, u32 subleaf, u32* eax, u32* ebx, u32* ecx, u32* edx);

} // namespace cpuid_spoof
```

### C Equivalent (Pure C Port)

```c
/* cpuid_spoof.h - C version */

#ifndef CPUID_SPOOF_H
#define CPUID_SPOOF_H

#include "types.h"  /* u32, u64 definitions */

/* Magic hypercall leaf - used for authenticated VMCALLs */
#define HYPERCALL_MAGIC_LEAF  0x13371337

/* Feature bits to mask */
#define CPUID_HV_PRESENT   (1u << 31)  /* CPUID.01H:ECX[31] - Hypervisor Present */
#define CPUID_VMX_FEATURE  (1u << 5)   /* CPUID.01H:ECX[5]  - VMX (Intel) */
#define CPUID_SVM_FEATURE  (1u << 2)   /* CPUID.80000001H:ECX[2] - SVM (AMD) */

/* Hypervisor vendor CPUID leaf */
#define CPUID_HV_VENDOR    0x40000000

/* Configuration structure - C-compatible POD type */
typedef struct _CPUID_SPOOF_CONFIG {
    /* Phase 1: CPUID spoofing flags */
    u32 hide_hypervisor;      /* Boolean: clear ECX[31] in leaf 1 */
    u32 hide_vmx;             /* Boolean: clear ECX[5] in leaf 1 (Intel) */
    u32 hide_svm;             /* Boolean: clear ECX[2] in leaf 0x80000001 (AMD) */
    u32 spoof_hv_vendor;      /* Boolean: zero vendor string in 0x40000000 */

    /* Phase 2: Timing mitigation (future) */
    u32 compensate_tsc;       /* Boolean: enable TSC compensation */
    u64 cpuid_tsc_offset;     /* TSC offset for timing compensation */

    /* Phase 3: Advanced spoofing (future) */
    u32 spoof_brand_string;   /* Boolean: enable brand string spoofing */
    char custom_brand[48];    /* Custom brand string buffer */
} CPUID_SPOOF_CONFIG;

/* Global configuration - defined in cpuid_spoof.c */
extern CPUID_SPOOF_CONFIG g_cpuid_config;

/* API Functions */
void cpuid_spoof_initialize(void);
void cpuid_spoof_execute_and_spoof(u32 leaf, u32 subleaf,
                                   u32* eax, u32* ebx, u32* ecx, u32* edx);

#endif /* CPUID_SPOOF_H */
```

**Conversion Notes**:
- `constexpr` → `#define` (compile-time constants)
- `namespace cpuid_spoof::` → `cpuid_spoof_` function prefix (C has no namespaces)
- `bool` → `u32` (C99 `stdbool.h` is problematic in kernel/VMXRoot contexts)
- Function names: `Initialize()` → `cpuid_spoof_initialize()` (explicit scoping)

---

## CPUID Leaf Handling Table

| Leaf | Subleaf | EAX | EBX | ECX | EDX | Purpose | Modification |
|------|---------|-----|-----|-----|-----|---------|--------------|
| **0x00000000** | 0 | Max std leaf | 'uneG' | 'letn' | 'Ieni' | Vendor ID (GenuineIntel) | **PASS THROUGH** |
| **0x00000001** | 0 | Stepping/Model | Brand/CLFLUSH | **Features** | **Features** | Feature Information | **SPOOF ECX[31]** ← Clear hypervisor bit<br>**SPOOF ECX[5]** ← Clear VMX (Intel)<br>EAX/EBX/EDX unchanged |
| 0x00000007 | 0 | Max subleaf | SGX/BMI1/AVX2 | AVX512/SHA | - | Extended Features | **PASS THROUGH** |
| **0x40000000** | 0 | Max HV leaf | 'Micr' | 'soft' | ' Hv' | Hypervisor Vendor ID | **SPOOF EBX=0, ECX=0, EDX=0**<br>**PRESERVE EAX** (max leaf!) |
| **0x40000001** | 0 | Interface sig | Reserved | Reserved | Reserved | Hyper-V Interface | **PASS THROUGH** ← Windows enlightenment |
| **0x40000003** | 0 | Feature flags | Partition priv | Power flags | Misc flags | Hyper-V Features | **PASS THROUGH** ← Windows enlightenment |
| **0x40000004** | 0 | Recommend | SpinLock retries | - | - | Implementation Limits | **PASS THROUGH** ← Windows enlightenment |
| **0x80000001** | 0 | Ext Model/Family | - | **Ext Features** | **Ext Features** | Extended Feature Info (AMD) | **SPOOF ECX[2]** ← Clear SVM (AMD)<br>EAX/EBX/EDX unchanged |
| 0x80000002-4 | 0 | Brand[0-3] | Brand[4-7] | Brand[8-11] | Brand[12-15] | Processor Brand String | **PASS THROUGH** (future: spoof) |

### Feature Bits in Detail

**CPUID.01H:ECX (Feature Information - Primary Detection Vector)**
```
Bit 31: Hypervisor Present (ReservedForHvGuestStatus)
Bit  5: VMX - Virtual Machine Extensions (Intel only)
Bit 27: OSXSAVE - OS has enabled XSAVE/XRSTOR
Bit 26: XSAVE - XSAVE/XRSTOR/XGETBV/XSETBV supported
Bit 25: AES - AES instruction extensions
Bit 20: SSE4.2
Bit 19: SSE4.1
```

**CPUID.80000001H:ECX (Extended Feature Information - AMD Only)**
```
Bit  2: SVM - Secure Virtual Machine (AMD-V)
Bit  6: SSE4A
Bit 11: XOP - Extended Operations
Bit 16: FMA4
```

**Anti-Cheat Detection Methods**:
1. Check `CPUID.01H:ECX[31]` - if set, hypervisor present → BANNED
2. Check `CPUID.40000000H:EBX/ECX/EDX` - if "Microsoft Hv" → BANNED
3. Check `CPUID.01H:ECX[5]` (VMX) or `CPUID.80000001H:ECX[2]` (SVM) - virtualization capable → SUSPICIOUS
4. Timing attacks on CPUID execution (measure cycles before/after)

**Our Countermeasures**:
1. Clear ECX[31] → hypervisor invisible
2. Zero vendor string → no "Microsoft Hv" signature
3. Clear VMX/SVM bits → CPU appears non-virtualizable
4. Future: TSC compensation to hide timing anomalies

---

## Key Functions

### Function 1: Initialize Configuration

#### C++ Original (cpuid_spoof.cpp)
```cpp
void Initialize() {
    // Default: full stealth mode
    g_config.hide_hypervisor = true;
    g_config.hide_vmx = true;
    g_config.hide_svm = true;
    g_config.spoof_hv_vendor = true;

    // Future phases (disabled by default)
    g_config.compensate_tsc = false;
    g_config.cpuid_tsc_offset = 0;
    g_config.spoof_brand_string = false;
}
```

#### C Equivalent
```c
void cpuid_spoof_initialize(void) {
    /* Default: full stealth mode */
    g_cpuid_config.hide_hypervisor = 1;   /* true */
    g_cpuid_config.hide_vmx = 1;
    g_cpuid_config.hide_svm = 1;
    g_cpuid_config.spoof_hv_vendor = 1;

    /* Future phases (disabled by default) */
    g_cpuid_config.compensate_tsc = 0;    /* false */
    g_cpuid_config.cpuid_tsc_offset = 0;
    g_cpuid_config.spoof_brand_string = 0;
}
```

---

### Function 2: Execute and Spoof CPUID

#### C++ Original (cpuid_spoof.cpp)
```cpp
void ExecuteAndSpoof(u32 leaf, u32 subleaf, u32* eax, u32* ebx, u32* ecx, u32* edx) {
    // Execute REAL CPUID in VMXRoot
    // This works because CPUID executes natively in VMXRoot (no VMExit)
    int cpuInfo[4] = {0};
    __cpuidex(cpuInfo, static_cast<int>(leaf), static_cast<int>(subleaf));

    // Start with real values
    *eax = static_cast<u32>(cpuInfo[0]);
    *ebx = static_cast<u32>(cpuInfo[1]);
    *ecx = static_cast<u32>(cpuInfo[2]);
    *edx = static_cast<u32>(cpuInfo[3]);

    // Apply spoofing based on leaf
    switch (leaf) {
        case 0x00000001:
            // Standard Feature Information - Primary detection vector!
            if (g_config.hide_hypervisor)
                *ecx &= ~CPUID_HV_PRESENT;   // Clear hypervisor bit (ECX[31])
            if (g_config.hide_vmx)
                *ecx &= ~CPUID_VMX_FEATURE;  // Clear VMX capability (ECX[5])
            break;

        case 0x80000001:
            // Extended Feature Information (AMD)
            if (g_config.hide_svm)
                *ecx &= ~CPUID_SVM_FEATURE;  // Clear SVM capability (ECX[2])
            break;

        case CPUID_HV_VENDOR:  // 0x40000000
            // Zero vendor string but PRESERVE max leaf!
            if (g_config.spoof_hv_vendor) {
                *ebx = 0;
                *ecx = 0;
                *edx = 0;
                // *eax stays intact (max hypervisor leaf)
            }
            break;

        default:
            // All other leaves: return real values unchanged
            break;
    }
}
```

#### C Equivalent
```c
/* External CPUID execution - implemented in assembly */
extern void cpuid_execute(u32 leaf, u32 subleaf, u32* eax, u32* ebx, u32* ecx, u32* edx);

void cpuid_spoof_execute_and_spoof(u32 leaf, u32 subleaf,
                                   u32* eax, u32* ebx, u32* ecx, u32* edx) {
    /* Execute REAL CPUID in VMXRoot (native execution, no VMExit) */
    cpuid_execute(leaf, subleaf, eax, ebx, ecx, edx);

    /* Apply spoofing based on leaf */
    switch (leaf) {
        case 0x00000001:
            /* Standard Feature Information - Primary detection vector! */
            /* Anti-cheats check ECX[31] to detect hypervisor presence */
            if (g_cpuid_config.hide_hypervisor)
                *ecx &= ~CPUID_HV_PRESENT;   /* Clear hypervisor bit (ECX[31]) */
            if (g_cpuid_config.hide_vmx)
                *ecx &= ~CPUID_VMX_FEATURE;  /* Clear VMX capability (ECX[5]) */
            break;

        case 0x80000001:
            /* Extended Feature Information (AMD) */
            /* Anti-cheats check for SVM capability on AMD */
            if (g_cpuid_config.hide_svm)
                *ecx &= ~CPUID_SVM_FEATURE;  /* Clear SVM capability (ECX[2]) */
            break;

        case CPUID_HV_VENDOR:  /* 0x40000000 - Hypervisor Vendor ID */
            /* Anti-cheats check for "Microsoft Hv" or other vendor strings */
            if (g_cpuid_config.spoof_hv_vendor) {
                /* Zero vendor string (EBX/ECX/EDX contain "Microsoft Hv") */
                /* BUT PRESERVE max leaf in EAX - Windows enlightenments need this! */
                *ebx = 0;
                *ecx = 0;
                *edx = 0;
                /* *eax stays intact (max hypervisor leaf) */
            }
            break;

        /* CRITICAL: Do NOT modify leaves 0x40000001-0x4000000F! */
        /* These are Windows enlightenments that WILL break things: */
        /* - 0x40000001: Hypercall page setup */
        /* - 0x40000003: Feature flags (SynIC, timers, APIC virtualization) */
        /* - 0x40000004: Implementation limits (resource allocation) */
        /* Let them pass through with real values. */

        default:
            /* All other leaves: return real values unchanged */
            break;
    }
}
```

---

### Function 3: HandleCpuid (Intel VMX Version)

#### C++ Original (intel/vmx_handler.cpp, lines 292-346)
```cpp
static bool HandleCpuid(pcontext_t guest_registers)
{
    // Get CPUID leaf/subleaf from guest registers
    u32 leaf = static_cast<u32>(guest_registers->rax);
    u32 subleaf = static_cast<u32>(guest_registers->rcx);

    // Check if this is our magic hypercall leaf
    if (leaf == cpuid_spoof::HYPERCALL_MAGIC_LEAF)
    {
        // Build VmExitContext from Intel-specific state
        ombra::VmExitContext ctx;
        ctx.Reset();

        ctx.guest_cr3 = vmexit::get_guest_cr3();
        __vmx_vmread(VMCS_GUEST_RIP, (size_t*)&ctx.guest_rip);
        ctx.p_rax = &guest_registers->rax;

        ctx.cmd_type = guest_registers->rcx;      // RCX = VMCALL_TYPE
        ctx.cmd_guest_va = guest_registers->rdx;  // RDX = &COMMAND_DATA
        ctx.extra_param = guest_registers->r8;    // R8 = extra param
        ctx.auth_key = guest_registers->r9;       // R9 = auth key

        ctx.arch_data = nullptr;
        ctx.arch_data2 = guest_registers;

        // Dispatch to unified handler
        VMX_ROOT_ERROR result = core::HandleVmcall(&ctx);

        if (result != VMX_ROOT_ERROR::INVALID_GUEST_PARAM || vmcall::IsVmcall(ctx.auth_key)) {
            guest_registers->rax = static_cast<u64>(result);
            bCpuidVmcallCalled = true;
            return true;
        }
    }

    // Handle ALL CPUID leaves with spoofing
    u32 eax, ebx, ecx, edx;
    cpuid_spoof::ExecuteAndSpoof(leaf, subleaf, &eax, &ebx, &ecx, &edx);

    // Return spoofed values to guest
    guest_registers->rax = static_cast<u64>(eax);
    guest_registers->rbx = static_cast<u64>(ebx);
    guest_registers->rcx = static_cast<u64>(ecx);
    guest_registers->rdx = static_cast<u64>(edx);

    return true;  // We handle ALL CPUID
}
```

#### C Equivalent
```c
/* External functions from other modules */
extern u64 vmexit_get_guest_cr3(void);
extern u64 vmx_vmread(u64 field);
extern u64 vmcall_is_valid(u64 key);
extern void vmexit_context_reset(VMEXIT_CONTEXT* ctx);
extern VMX_ROOT_ERROR core_handle_vmcall(VMEXIT_CONTEXT* ctx);

static u32 handle_cpuid(GUEST_CONTEXT* guest_registers)
{
    u32 leaf = (u32)guest_registers->rax;
    u32 subleaf = (u32)guest_registers->rcx;

    /* Check if this is our magic hypercall leaf */
    if (leaf == HYPERCALL_MAGIC_LEAF)
    {
        /* Build VmExitContext from Intel-specific state */
        VMEXIT_CONTEXT ctx;
        vmexit_context_reset(&ctx);

        ctx.guest_cr3 = vmexit_get_guest_cr3();
        ctx.guest_rip = vmx_vmread(VMCS_GUEST_RIP);
        ctx.p_rax = &guest_registers->rax;

        /* Populate hypercall parameters from guest registers */
        ctx.cmd_type = guest_registers->rcx;      /* RCX = VMCALL_TYPE */
        ctx.cmd_guest_va = guest_registers->rdx;  /* RDX = &COMMAND_DATA */
        ctx.extra_param = guest_registers->r8;    /* R8 = extra param */
        ctx.auth_key = guest_registers->r9;       /* R9 = auth key */

        ctx.arch_data = NULL;                     /* Intel doesn't need VMCB */
        ctx.arch_data2 = guest_registers;

        /* Dispatch to unified handler */
        VMX_ROOT_ERROR result = core_handle_vmcall(&ctx);

        /* Check if this was a valid authenticated vmcall */
        if (result != VMX_ROOT_ERROR_INVALID_GUEST_PARAM || vmcall_is_valid(ctx.auth_key)) {
            guest_registers->rax = (u64)result;
            g_cpuid_vmcall_called = 1;  /* true */
            return 1;  /* handled */
        }
        /* Invalid auth on magic leaf - fall through to spoofing */
    }

    /* Handle ALL CPUID leaves with spoofing */
    {
        u32 eax, ebx, ecx, edx;
        cpuid_spoof_execute_and_spoof(leaf, subleaf, &eax, &ebx, &ecx, &edx);

        /* Return spoofed values to guest */
        guest_registers->rax = (u64)eax;
        guest_registers->rbx = (u64)ebx;
        guest_registers->rcx = (u64)ecx;
        guest_registers->rdx = (u64)edx;
    }

    return 1;  /* We handle ALL CPUID - don't chain to original Hyper-V handler */
}
```

---

## Assembly Components

### cpu-helper.asm - CPUID Execution (MASM64)

**Original Implementation** (OmbraCoreLib/src/cpu-helper.asm, lines 283-290):
```asm
; NTSTATUS CPUIDVmCall(ULONG64 ulCallNum, ULONG64 ulOpt1, ULONG64 ulOpt2, ULONG64 ulOpt3);
CPUIDVmCall PROC
    mov rax, r9
    xor r9, 0deada55h       ; Authenticate with magic XOR
    push rbx                ; Save RBX (callee-saved)
    cpuid                   ; Execute CPUID instruction
    pop rbx                 ; Restore RBX
    ret
CPUIDVmCall ENDP
```

**New Assembly for Pure C Port** (cpuid_execute.asm):
```asm
.code _text

; void cpuid_execute(u32 leaf, u32 subleaf, u32* eax, u32* ebx, u32* ecx, u32* edx);
; RCX = leaf (u32)
; RDX = subleaf (u32)
; R8  = eax pointer (u32*)
; R9  = ebx pointer (u32*)
; [rsp+40] = ecx pointer (u32*) - 5th arg on stack
; [rsp+48] = edx pointer (u32*) - 6th arg on stack
;
; Calling convention: Microsoft x64 (RCX, RDX, R8, R9, stack...)
cpuid_execute PROC
    ; Save callee-saved registers
    push rbx
    push r12
    push r13
    push r14
    push r15

    ; Save output pointers (will be clobbered by CPUID)
    mov r12, r8            ; r12 = eax_ptr
    mov r13, r9            ; r13 = ebx_ptr
    mov r14, [rsp+48]      ; r14 = ecx_ptr (5th arg, +40 stack + 8 ret addr)
    mov r15, [rsp+56]      ; r15 = edx_ptr (6th arg, +48 stack + 8 ret addr)

    ; Setup CPUID registers
    mov eax, ecx           ; EAX = leaf
    mov ecx, edx           ; ECX = subleaf

    ; Execute CPUID
    ; This works natively in VMXRoot (no VMExit)
    ; RAX = leaf, RCX = subleaf → returns EAX, EBX, ECX, EDX
    cpuid

    ; Store results via saved pointers
    mov [r12], eax         ; *eax_ptr = EAX result
    mov [r13], ebx         ; *ebx_ptr = EBX result
    mov [r14], ecx         ; *ecx_ptr = ECX result
    mov [r15], edx         ; *edx_ptr = EDX result

    ; Restore callee-saved registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx

    ret
cpuid_execute ENDP

END
```

**Why Assembly is Required**:
1. `CPUID` instruction has **specific input/output register requirements** (EAX=leaf, ECX=subleaf → EAX/EBX/ECX/EDX)
2. **RBX is callee-saved** in Microsoft x64 calling convention, but CPUID clobbers it
3. C intrinsic `__cpuidex()` **may not be available** in freestanding VMXRoot context
4. Assembly gives **explicit control** over register state for precise CPUID execution

---

### Alternative: Inline Assembly (GCC/Clang)

If using GCC/Clang instead of MSVC, use inline assembly:

```c
static inline void cpuid_execute(u32 leaf, u32 subleaf,
                                 u32* eax, u32* ebx, u32* ecx, u32* edx) {
    __asm__ __volatile__(
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)  /* outputs */
        : "a"(leaf), "c"(subleaf)                          /* inputs */
    );
}
```

**Note**: MSVC doesn't support inline assembly in x64 mode, hence the separate .asm file.

---

## Conversion Notes: C++ to C

### Namespace Handling
```cpp
// C++ (namespaced)
namespace cpuid_spoof {
    void Initialize();
}

// C (prefixed function names)
void cpuid_spoof_initialize(void);
```

### Bool to Integer
```cpp
// C++ (bool type)
bool hide_hypervisor = true;
if (hide_hypervisor) { ... }

// C (u32 as boolean)
u32 hide_hypervisor = 1;  /* 0=false, 1=true */
if (hide_hypervisor != 0) { ... }
```

### Const Expressions
```cpp
// C++ (compile-time constant)
constexpr u32 CPUID_HV_PRESENT = (1u << 31);

// C (preprocessor macro)
#define CPUID_HV_PRESENT  (1u << 31)
```

### Type Casts
```cpp
// C++ (explicit static_cast)
u32 leaf = static_cast<u32>(guest_registers->rax);

// C (C-style cast)
u32 leaf = (u32)(guest_registers->rax);
```

### Intrinsics to Assembly
```cpp
// C++ (compiler intrinsic, requires intrin.h)
int cpuInfo[4];
__cpuidex(cpuInfo, leaf, subleaf);

// C (external assembly function)
extern void cpuid_execute(u32 leaf, u32 subleaf, u32* eax, u32* ebx, u32* ecx, u32* edx);
u32 eax, ebx, ecx, edx;
cpuid_execute(leaf, subleaf, &eax, &ebx, &ecx, &edx);
```

### Struct Initialization
```cpp
// C++ (designated initializers)
Config g_config = {
    .hide_hypervisor = true,
    .hide_vmx = true
};

// C (C99 designated initializers - same syntax!)
CPUID_SPOOF_CONFIG g_cpuid_config = {
    .hide_hypervisor = 1,
    .hide_vmx = 1
};
```

---

## Magic Constants

### CPUID Leaves (EAX Input)
```c
/* Standard CPUID Functions */
#define CPUID_VENDOR_ID              0x00000000  /* Vendor ID (GenuineIntel/AuthenticAMD) */
#define CPUID_FEATURE_INFO           0x00000001  /* Feature information - PRIMARY DETECTION VECTOR */
#define CPUID_CACHE_DESCRIPTORS      0x00000002  /* Cache and TLB descriptors */
#define CPUID_SERIAL_NUMBER          0x00000003  /* Processor serial number (Pentium III only) */
#define CPUID_CACHE_PARAMS           0x00000004  /* Deterministic cache parameters */
#define CPUID_MONITOR_MWAIT          0x00000005  /* MONITOR/MWAIT parameters */
#define CPUID_THERMAL_POWER          0x00000006  /* Thermal and power management */
#define CPUID_EXTENDED_FEATURES      0x00000007  /* Structured extended features */

/* Hypervisor CPUID Leaves (Hyper-V) */
#define CPUID_HV_VENDOR              0x40000000  /* Hypervisor vendor and max leaf */
#define CPUID_HV_INTERFACE           0x40000001  /* Hypervisor interface signature */
#define CPUID_HV_VERSION             0x40000002  /* Hypervisor version */
#define CPUID_HV_FEATURES            0x40000003  /* Hypervisor feature identification */
#define CPUID_HV_ENLIGHTENMENTS      0x40000004  /* Implementation recommendations */
#define CPUID_HV_LIMITS              0x40000005  /* Implementation limits */
#define CPUID_HV_HARDWARE_FEATURES   0x40000006  /* Hardware features */

/* Extended CPUID Functions */
#define CPUID_EXT_MAX_FUNCTION       0x80000000  /* Max extended function */
#define CPUID_EXT_FEATURE_INFO       0x80000001  /* Extended feature info - AMD SVM DETECTION */
#define CPUID_BRAND_STRING_0         0x80000002  /* Processor brand string (part 1) */
#define CPUID_BRAND_STRING_1         0x80000003  /* Processor brand string (part 2) */
#define CPUID_BRAND_STRING_2         0x80000004  /* Processor brand string (part 3) */
#define CPUID_L1_CACHE_TLB           0x80000005  /* L1 cache and TLB identifiers */
#define CPUID_L2_CACHE_TLB           0x80000006  /* L2 cache identifiers */
#define CPUID_POWER_MANAGEMENT       0x80000007  /* Advanced power management */
#define CPUID_ADDRESS_SIZES          0x80000008  /* Virtual and physical address sizes */
#define CPUID_SVM_FEATURES           0x8000000A  /* AMD SVM features */

/* Ombra Hypercall Leaf */
#define HYPERCALL_MAGIC_LEAF         0x13371337  /* Magic leaf for authenticated VMCALLs */
```

### Feature Bits (CPUID.01H:ECX - Standard Features)
```c
/* CPUID.01H:ECX - Feature Information Flags */
#define CPUID_ECX_SSE3          (1u <<  0)  /* SSE3 extensions */
#define CPUID_ECX_PCLMULQDQ     (1u <<  1)  /* PCLMULQDQ instruction */
#define CPUID_ECX_DTES64        (1u <<  2)  /* 64-bit DS area */
#define CPUID_ECX_MONITOR       (1u <<  3)  /* MONITOR/MWAIT */
#define CPUID_ECX_DS_CPL        (1u <<  4)  /* CPL qualified debug store */
#define CPUID_ECX_VMX           (1u <<  5)  /* Virtual Machine Extensions (CRITICAL!) */
#define CPUID_ECX_SMX           (1u <<  6)  /* Safer Mode Extensions */
#define CPUID_ECX_EIST          (1u <<  7)  /* Enhanced Intel SpeedStep */
#define CPUID_ECX_TM2           (1u <<  8)  /* Thermal Monitor 2 */
#define CPUID_ECX_SSSE3         (1u <<  9)  /* Supplemental SSE3 */
#define CPUID_ECX_CNXT_ID       (1u << 10)  /* L1 Context ID */
#define CPUID_ECX_SDBG          (1u << 11)  /* Silicon Debug */
#define CPUID_ECX_FMA           (1u << 12)  /* Fused Multiply-Add */
#define CPUID_ECX_CMPXCHG16B    (1u << 13)  /* CMPXCHG16B */
#define CPUID_ECX_xTPR          (1u << 14)  /* xTPR Update Control */
#define CPUID_ECX_PDCM          (1u << 15)  /* Perfmon and Debug Capability */
#define CPUID_ECX_PCID          (1u << 17)  /* Process-context identifiers */
#define CPUID_ECX_DCA           (1u << 18)  /* Direct Cache Access */
#define CPUID_ECX_SSE41         (1u << 19)  /* SSE4.1 */
#define CPUID_ECX_SSE42         (1u << 20)  /* SSE4.2 */
#define CPUID_ECX_x2APIC        (1u << 21)  /* x2APIC support */
#define CPUID_ECX_MOVBE         (1u << 22)  /* MOVBE instruction */
#define CPUID_ECX_POPCNT        (1u << 23)  /* POPCNT instruction */
#define CPUID_ECX_TSC_DEADLINE  (1u << 24)  /* TSC-Deadline */
#define CPUID_ECX_AESNI         (1u << 25)  /* AES-NI */
#define CPUID_ECX_XSAVE         (1u << 26)  /* XSAVE/XRSTOR/XSETBV/XGETBV */
#define CPUID_ECX_OSXSAVE       (1u << 27)  /* OS has enabled XSETBV/XGETBV */
#define CPUID_ECX_AVX           (1u << 28)  /* AVX */
#define CPUID_ECX_F16C          (1u << 29)  /* F16C (half-precision convert) */
#define CPUID_ECX_RDRAND        (1u << 30)  /* RDRAND instruction */
#define CPUID_ECX_HYPERVISOR    (1u << 31)  /* Hypervisor Present (CRITICAL!) */

/* Aliases for clarity */
#define CPUID_VMX_FEATURE    CPUID_ECX_VMX         /* Bit 5 - Intel virtualization */
#define CPUID_HV_PRESENT     CPUID_ECX_HYPERVISOR  /* Bit 31 - Hypervisor detection */
```

### Feature Bits (CPUID.80000001H:ECX - AMD Extended Features)
```c
/* CPUID.80000001H:ECX - AMD Extended Feature Flags */
#define CPUID_EXT_ECX_LAHF_SAHF       (1u <<  0)  /* LAHF/SAHF in 64-bit mode */
#define CPUID_EXT_ECX_CMP_LEGACY      (1u <<  1)  /* Core multi-processing legacy mode */
#define CPUID_EXT_ECX_SVM             (1u <<  2)  /* Secure Virtual Machine (AMD-V) (CRITICAL!) */
#define CPUID_EXT_ECX_EXT_APIC_SPACE  (1u <<  3)  /* Extended APIC space */
#define CPUID_EXT_ECX_ALT_MOV_CR8     (1u <<  4)  /* LOCK MOV CR0 means MOV CR8 */
#define CPUID_EXT_ECX_ABM             (1u <<  5)  /* Advanced Bit Manipulation (LZCNT) */
#define CPUID_EXT_ECX_SSE4A           (1u <<  6)  /* SSE4A */
#define CPUID_EXT_ECX_MISALIGN_SSE    (1u <<  7)  /* Misaligned SSE mode */
#define CPUID_EXT_ECX_3DNOW_PREFETCH  (1u <<  8)  /* 3DNow! PREFETCH/PREFETCHW */
#define CPUID_EXT_ECX_OSVW            (1u <<  9)  /* OS Visible Workaround */
#define CPUID_EXT_ECX_IBS             (1u << 10)  /* Instruction Based Sampling */
#define CPUID_EXT_ECX_XOP             (1u << 11)  /* Extended Operation support */
#define CPUID_EXT_ECX_SKINIT          (1u << 12)  /* SKINIT/STGI instructions */
#define CPUID_EXT_ECX_WDT             (1u << 13)  /* Watchdog timer support */
#define CPUID_EXT_ECX_LWP             (1u << 15)  /* Lightweight profiling */
#define CPUID_EXT_ECX_FMA4            (1u << 16)  /* 4-operand FMA */
#define CPUID_EXT_ECX_TBM             (1u << 21)  /* Trailing Bit Manipulation */
#define CPUID_EXT_ECX_TOPOLOGY_EXT    (1u << 22)  /* Topology extensions */

/* Alias for clarity */
#define CPUID_SVM_FEATURE     CPUID_EXT_ECX_SVM   /* Bit 2 - AMD virtualization */
```

### Hypervisor Vendor Strings (CPUID.40000000H:EBX/ECX/EDX)
```c
/* Hypervisor Vendor IDs (12-byte ASCII strings split across EBX, ECX, EDX) */
/* Microsoft Hyper-V: "Microsoft Hv" */
#define HV_VENDOR_MICROSOFT_EBX  0x7263694D  /* "Micr" */
#define HV_VENDOR_MICROSOFT_ECX  0x666F736F  /* "osof" */
#define HV_VENDOR_MICROSOFT_EDX  0x76482074  /* "t Hv" */

/* VMware: "VMwareVMware" */
#define HV_VENDOR_VMWARE_EBX     0x61774D56  /* "VMwa" */
#define HV_VENDOR_VMWARE_ECX     0x4D566572  /* "reVM" */
#define HV_VENDOR_VMWARE_EDX     0x65726177  /* "ware" */

/* VirtualBox: "VBoxVBoxVBox" */
#define HV_VENDOR_VBOX_EBX       0x786F4256  /* "VBox" */
#define HV_VENDOR_VBOX_ECX       0x786F4256  /* "VBox" */
#define HV_VENDOR_VBOX_EDX       0x786F4256  /* "VBox" */

/* KVM: "KVMKVMKVM\0\0\0" */
#define HV_VENDOR_KVM_EBX        0x4D564B4B  /* "KKMV" (little-endian) */
#define HV_VENDOR_KVM_ECX        0x4D564B4D  /* "MKMV" */
#define HV_VENDOR_KVM_EDX        0x0000004D  /* "M\0\0\0" */

/* Spoofed (zeroed) vendor string */
#define HV_VENDOR_SPOOFED_EBX    0x00000000
#define HV_VENDOR_SPOOFED_ECX    0x00000000
#define HV_VENDOR_SPOOFED_EDX    0x00000000
```

---

## Testing Checklist

### Phase 1: Basic CPUID Spoofing
- [ ] **Test 1.1**: Verify CPUID.01H:ECX[31] returns 0 (hypervisor bit hidden)
  ```c
  u32 eax, ebx, ecx, edx;
  cpuid_execute(0x00000001, 0, &eax, &ebx, &ecx, &edx);
  assert((ecx & CPUID_HV_PRESENT) == 0);  /* Bit 31 must be clear */
  ```

- [ ] **Test 1.2**: Verify CPUID.01H:ECX[5] returns 0 (VMX hidden on Intel)
  ```c
  cpuid_execute(0x00000001, 0, &eax, &ebx, &ecx, &edx);
  assert((ecx & CPUID_VMX_FEATURE) == 0);  /* Bit 5 must be clear */
  ```

- [ ] **Test 1.3**: Verify CPUID.80000001H:ECX[2] returns 0 (SVM hidden on AMD)
  ```c
  cpuid_execute(0x80000001, 0, &eax, &ebx, &ecx, &edx);
  assert((ecx & CPUID_SVM_FEATURE) == 0);  /* Bit 2 must be clear */
  ```

- [ ] **Test 1.4**: Verify CPUID.40000000H vendor string is zeroed
  ```c
  cpuid_execute(0x40000000, 0, &eax, &ebx, &ecx, &edx);
  assert(ebx == 0 && ecx == 0 && edx == 0);  /* No "Microsoft Hv" */
  assert(eax != 0);  /* Max leaf must be preserved! */
  ```

### Phase 2: Windows Enlightenments Preservation
- [ ] **Test 2.1**: Verify CPUID.40000001H returns real values (not zeroed)
  ```c
  cpuid_execute(0x40000001, 0, &eax, &ebx, &ecx, &edx);
  /* Should return Hyper-V interface signature, not zeros */
  assert(eax != 0 || ebx != 0 || ecx != 0 || edx != 0);
  ```

- [ ] **Test 2.2**: Verify CPUID.40000003H returns real feature flags
  ```c
  cpuid_execute(0x40000003, 0, &eax, &ebx, &ecx, &edx);
  /* Should return Hyper-V feature flags for SynIC, timers, etc. */
  assert(eax != 0);  /* Feature flags should be non-zero */
  ```

- [ ] **Test 2.3**: Verify Windows still boots and doesn't BSOD
  ```
  Boot Windows 10/11 with hypervisor active
  → Check for HYPERVISOR_ERROR (0x20001) BSOD
  → Check Event Viewer for Hyper-V errors
  ```

### Phase 3: Hypercall Authentication
- [ ] **Test 3.1**: Verify magic leaf 0x13371337 triggers hypercall dispatch
  ```c
  guest_registers->rax = HYPERCALL_MAGIC_LEAF;
  guest_registers->rcx = VMCALL_TEST;
  guest_registers->r9 = g_comm_key;
  /* Execute CPUID from guest → should return VMX_ROOT_ERROR code in RAX */
  ```

- [ ] **Test 3.2**: Verify invalid key on magic leaf falls through to spoofing
  ```c
  guest_registers->rax = HYPERCALL_MAGIC_LEAF;
  guest_registers->r9 = 0xDEADBEEF;  /* Wrong key */
  /* Should return spoofed CPUID values, not hypercall result */
  ```

### Phase 4: Anti-Cheat Evasion
- [ ] **Test 4.1**: Run Pafish (hypervisor detection tool)
  ```
  Download: https://github.com/a0rtega/pafish
  Run in VM → Should report "No virtualization detected"
  Check CPUID checks specifically (not just timing/RDTSC)
  ```

- [ ] **Test 4.2**: Run al-khaser (anti-VM/anti-debug tool)
  ```
  Download: https://github.com/LordNoteworthy/al-khaser
  Run in VM → CPUID checks should pass (hypervisor bit, VMX/SVM hidden)
  ```

- [ ] **Test 4.3**: Manual CPUID check from kernel driver
  ```c
  /* In test driver */
  int cpuInfo[4];
  __cpuid(cpuInfo, 1);
  DbgPrint("ECX = 0x%08X\n", cpuInfo[2]);
  /* Verify bit 31 is NOT set (no hypervisor detected) */
  ```

### Phase 5: Timing Attack Mitigation (Future)
- [ ] **Test 5.1**: Measure CPUID execution time (baseline)
  ```c
  u64 tsc1 = __rdtsc();
  __cpuid(cpuInfo, 1);
  u64 tsc2 = __rdtsc();
  u64 delta = tsc2 - tsc1;
  /* On bare metal: ~20-40 cycles */
  /* In VM without compensation: ~1000+ cycles */
  /* With TSC compensation: should match bare metal */
  ```

- [ ] **Test 5.2**: Statistical timing analysis (advanced)
  ```c
  /* Collect 10,000 CPUID execution samples */
  /* Analyze distribution: mean, stddev, outliers */
  /* Compare to bare metal distribution */
  /* VM typically has high variance and outliers */
  ```

### Phase 6: Real-World Anti-Cheat Testing
- [ ] **Test 6.1**: EasyAntiCheat (Rust, Apex Legends, Fortnite)
  ```
  WARNING: Test in isolated environment only!
  Launch game with EAC → Monitor for detection/ban
  EAC specifically checks CPUID.01H:ECX[31] and vendor strings
  ```

- [ ] **Test 6.2**: BattlEye (Arma 3, PUBG, Rainbow Six Siege)
  ```
  WARNING: Test in isolated environment only!
  BattlEye uses kernel driver to directly execute CPUID
  Verify HandleCpuid intercepts from kernel-mode execution
  ```

- [ ] **Test 6.3**: Vanguard (Valorant, League of Legends)
  ```
  WARNING: Test in isolated environment only!
  Vanguard starts at boot and runs in kernel mode
  Most aggressive anti-cheat, checks CPUID, MSR, timing
  ```

---

## Critical Warnings

### DO NOT Modify These Leaves!
```c
/* Windows Hyper-V Enlightenment Leaves - PASS THROUGH UNCHANGED */
/* Modifying these WILL cause Windows BSOD or performance degradation */

case 0x40000001:  /* Hypervisor Interface Signature */
    /* Used by Windows to locate hypercall page */
    /* EAX = "Hv#1" signature */
    /* If zeroed: Windows can't make hypercalls → BSOD */
    break;  /* PASS THROUGH */

case 0x40000003:  /* Hypervisor Feature Identification */
    /* Bits indicate available Hyper-V features: */
    /* - SynIC (Synthetic Interrupt Controller) */
    /* - Synthetic Timers */
    /* - APIC virtualization */
    /* - MSR-based APIC access */
    /* If zeroed: Windows loses performance optimizations */
    break;  /* PASS THROUGH */

case 0x40000004:  /* Implementation Recommendations */
    /* Hints from hypervisor to guest OS */
    /* - SpinLock retry counts */
    /* - Relaxed timing checks */
    /* If zeroed: Windows may spin excessively or timeout */
    break;  /* PASS THROUGH */

case 0x40000005:  /* Implementation Limits */
    /* Max virtual processors, logical processors */
    /* Used for resource allocation */
    break;  /* PASS THROUGH */
```

### Vendor String Spoofing Gotcha
```c
/* WRONG: Zeroing EAX breaks Windows enlightenment discovery */
if (leaf == 0x40000000 && g_config.spoof_hv_vendor) {
    *eax = 0;  /* ❌ WRONG! Windows can't find max enlightenment leaf */
    *ebx = 0;
    *ecx = 0;
    *edx = 0;
}

/* CORRECT: Zero vendor string but PRESERVE max leaf */
if (leaf == 0x40000000 && g_config.spoof_hv_vendor) {
    /* *eax stays intact (max hypervisor leaf, typically 0x4000000B) */
    *ebx = 0;  /* Zero "Micr" */
    *ecx = 0;  /* Zero "osof" */
    *edx = 0;  /* Zero "t Hv" */
}
```

### Assembly Calling Convention
```asm
; WRONG: Forgetting to save RBX (callee-saved register)
cpuid_execute_WRONG PROC
    mov eax, ecx
    mov ecx, edx
    cpuid           ; ❌ Clobbers RBX without saving!
    ret
cpuid_execute_WRONG ENDP

; CORRECT: Save/restore all callee-saved registers
cpuid_execute PROC
    push rbx        ; ✅ Save RBX
    push r12        ; ✅ Save other callee-saved regs
    ; ... CPUID execution ...
    pop r12
    pop rbx         ; ✅ Restore RBX
    ret
cpuid_execute ENDP
```

---

## Performance Considerations

### CPUID Execution Cost
```
Bare Metal CPUID:     ~20-40 CPU cycles
VMExit + CPUID:       ~1000-2000 cycles (50x overhead!)
With Spoofing:        ~1100-2200 cycles (adds ~100 cycles for bit masking)

Windows executes CPUID ~500-1000 times per second (normal operation)
Game anti-cheats: ~10,000+ times per second (aggressive polling)

Total overhead: ~1-2ms per 1000 CPUID calls
```

### Optimization Opportunities
1. **Cache recent CPUID results** (leaf 0, 1, 0x80000001 rarely change)
2. **Batch CPUID operations** in anti-cheat scans (reduce VMExit count)
3. **TSC offsetting** to hide VMExit latency spikes
4. **Pre-compute spoofed values** at hypervisor init (avoid runtime masking)

---

## References

### Intel Architecture
- Intel SDM Vol. 2A, Chapter 3 (CPUID instruction reference)
- Intel SDM Vol. 3C, Chapter 24.2 (VMExit for CPUID)

### AMD Architecture
- AMD APM Vol. 3, Chapter E.3 (CPUID specification)
- AMD APM Vol. 2, Chapter 15.10 (VMEXIT_CPUID interception)

### Microsoft Hyper-V
- Hyper-V TLFS (Top-Level Functional Specification)
- Chapter 2.4: CPUID Leaves (0x40000000-0x4000000F)

### Anti-Cheat Research
- Pafish: https://github.com/a0rtega/pafish
- al-khaser: https://github.com/LordNoteworthy/al-khaser
- CheckVM: https://github.com/a0rtega/pafish

---

**Document Version**: 1.0
**Last Updated**: December 2025
**Ported for**: Ombra Hypervisor V3 C Migration
**Author**: ENI (Ombra Project Intelligence)
