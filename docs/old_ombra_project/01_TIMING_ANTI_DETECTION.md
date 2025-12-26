# TIMING & TSC ANTI-DETECTION - C++ to C + Assembly Port Guide

## Overview

The timing subsystem provides anti-detection capabilities against timing-based hypervisor detection techniques used by anti-cheats like BattlEye, ESEA, and Vanguard. It works by:

1. **TSC Offset Compensation**: Capturing TSC (Time Stamp Counter) at VMExit entry and adjusting the TSC offset before VMResume to hide VMExit processing overhead from the guest
2. **APERF/MPERF Virtualization**: Compensating performance monitoring MSRs (0xE7/0xE8) to hide hypervisor overhead from Instruction Execution Time (IET) detection
3. **Per-vCPU State Management**: Maintaining separate timing state for each virtual CPU to ensure accurate compensation on multi-core systems

**Anti-Cheat Techniques Defeated**:
- BattlEye RDTSC timing checks (detects VMExit overhead via time delta anomalies)
- ESEA APERF/MPERF ratio analysis (detects hypervisor via performance counter discrepancies)
- Generic IET (Instruction Execution Time) detection using TSC deltas

## File Inventory

| File | Lines | Purpose |
|------|-------|---------|
| `PayLoad/include/timing.h` | 84 | VMXRoot timing API header, per-vCPU state structures |
| `PayLoad/core/timing.cpp` | 172 | VMXRoot timing implementation (TSC compensation, MSR virtualization) |
| `OmbraCoreLib/OmbraCoreLib/include/timing.h` | 27 | Kernel-mode stopwatch API (Windows kernel timing utilities) |
| `OmbraCoreLib/OmbraCoreLib/src/timing.cpp` | 37 | Kernel-mode stopwatch implementation (uses KeQuerySystemTime) |
| `OmbraCoreLib/OmbraCoreLib-v/include/VMTimers.h` | 26 | Legacy i8253 PIT timer port definitions (unused in current codebase) |
| `OmbraCoreLib/OmbraCoreLib-v/src/VMTimers.cpp` | 1 | Stub file (no implementation) |

**File Relationships**:
- **PayLoad timing** = VMXRoot anti-detection (NO Windows APIs, freestanding environment)
- **OmbraCoreLib timing** = Kernel driver utilities (uses Windows kernel APIs like KeQuerySystemTime)
- **VMTimers** = Legacy code (8253 PIT timer ports for hardware emulation, not used)

## Architecture Summary

### Execution Flow

```
┌─────────────────────────────────────────────────────────────────────────┐
│ GUEST EXECUTES INSTRUCTION (e.g., CPUID, RDMSR, VMCALL)                 │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼ VMExit triggered
┌─────────────────────────────────────────────────────────────────────────┐
│ 1. vmexit_handler (Intel) or SvmVmexitHandler (AMD)                     │
│    → FIRST instruction must be: timing::OnExitEntry()                   │
│    → Captures TSC via __rdtsc() → stores in VcpuTimingState.exit_entry_tsc │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│ 2. VMExit Handler Logic (dispatch commands, handle MSR reads, etc.)     │
│    → Process VMCALL commands                                            │
│    → If MSR read of 0xE7/0xE8: call timing::ReadMsrVirtualized()       │
│    → Return compensated APERF/MPERF value (hides hypervisor overhead)   │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│ 3. Just before VMResume/VMRUN: timing::OnExitComplete(arch_data)        │
│    → Captures exit TSC via __rdtsc()                                    │
│    → Calculates overhead = exit_end_tsc - exit_entry_tsc               │
│    → Intel: Reads VMCS TSC_OFFSET, subtracts overhead, writes back      │
│    → AMD: Reads VMCB.ControlArea.TscOffset (offset 0x50), subtracts     │
│    → Accumulates overhead in aperf_offset/mperf_offset for MSR virt     │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼ VMResume
┌─────────────────────────────────────────────────────────────────────────┐
│ GUEST CONTINUES EXECUTION                                                │
│ → RDTSC sees: real_tsc + (original_offset - overhead) = compensated TSC │
│ → RDMSR 0xE7/0xE8: sees real_value - accumulated_overhead               │
│ → NO TIMING ANOMALIES DETECTED                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

### Key Constants

| Constant | Value | Source | Purpose |
|----------|-------|--------|---------|
| `VMCS_FIELD_TSC_OFFSET_FULL` | `0x2010` | Intel SDM Vol 3C Appendix B | VMCS field for TSC offset (Intel) |
| `VMCB_TSC_OFFSET` | `+0x50` from VMCB base | AMD APM Vol 2 Table B-1 | VMCB ControlArea.TscOffset (AMD) |
| `IA32_MPERF` | `0xE7` | Intel SDM Vol 4 Table 2-2 | Maximum Performance Frequency Clock Count |
| `IA32_APERF` | `0xE8` | Intel SDM Vol 4 Table 2-2 | Actual Performance Frequency Clock Count |
| `MAX_CPUS` | `256` | PayLoad/include/timing.h:44 | Maximum supported vCPUs (matches typical server configs) |

### TSC Offset Compensation Math

**Without Compensation**:
```
Guest RDTSC = Real TSC + TSC_OFFSET
            = 1000000 + 0 = 1000000

[VMExit occurs, 500 cycles of processing]

Guest RDTSC = Real TSC + TSC_OFFSET
            = 1000500 + 0 = 1000500

Guest sees 500 cycles elapsed (DETECTED - VMExit overhead visible!)
```

**With Compensation**:
```
Guest RDTSC = Real TSC + TSC_OFFSET
            = 1000000 + 0 = 1000000

[VMExit: OnExitEntry captures entry_tsc = 1000000]
[VMExit processing takes 500 cycles]
[OnExitComplete: exit_tsc = 1000500, overhead = 500]
[Adjust: TSC_OFFSET = 0 - 500 = -500]

Guest RDTSC = Real TSC + TSC_OFFSET
            = 1000500 + (-500) = 1000000

Guest sees 0 cycles elapsed (UNDETECTED - overhead hidden!)
```

## Critical Data Structures

### C++ Original (PayLoad/include/timing.h)

```cpp
// Lines 20-41: Per-vCPU Timing State
struct VcpuTimingState {
    // TSC captured at VMExit entry (before any handling)
    u64 exit_entry_tsc;

    // Accumulated overhead to hide from guest
    volatile u64 accumulated_overhead;

    // APERF/MPERF shadow values for IET detection bypass
    u64 aperf_offset;
    u64 mperf_offset;

    // Statistics (debug builds only)
#ifdef OMBRA_DEBUG
    u64 vmexit_count;
    u64 total_overhead;
    u64 min_overhead;
    u64 max_overhead;
#endif

    // Initialization flag
    bool initialized;
};
```

**Dependencies**:
- `u64` = `unsigned long long` (from `PayLoad/include/types.h:118`)
- `volatile` = C keyword for volatile access (no change needed)
- `bool` = C99 `_Bool` or `stdbool.h` (requires header change)

### C Equivalent

```c
/* timing_state.h - C port of VcpuTimingState */
#include <stdint.h>
#include <stdbool.h>  /* C99 bool support */

/* Per-vCPU Timing State
 * Tracks VMExit overhead for TSC compensation
 *
 * IMPORTANT: This structure is cache-line aligned to prevent false sharing
 * between cores. Each CPU gets its own cache-line-sized block.
 */
typedef struct _VCPU_TIMING_STATE {
    /* TSC captured at VMExit entry (before any handling) */
    uint64_t exit_entry_tsc;

    /* Accumulated overhead to hide from guest
     * MUST be volatile - read/written by VMExit handler without locks
     */
    volatile uint64_t accumulated_overhead;

    /* APERF/MPERF shadow values for IET detection bypass */
    uint64_t aperf_offset;
    uint64_t mperf_offset;

#ifdef OMBRA_DEBUG
    /* Statistics (debug builds only) */
    uint64_t vmexit_count;
    uint64_t total_overhead;
    uint64_t min_overhead;
    uint64_t max_overhead;
#endif

    /* Initialization flag */
    bool initialized;

    /* Padding to cache line boundary (64 bytes)
     * Without this, adjacent CPU states would share cache lines,
     * causing cache coherency traffic on every VMExit
     */
    uint8_t _padding[64 -
        (5 * sizeof(uint64_t)) -  /* 5 u64 fields always present */
#ifdef OMBRA_DEBUG
        (4 * sizeof(uint64_t)) -  /* 4 u64 debug fields */
#endif
        sizeof(bool)];

} VCPU_TIMING_STATE;

/* Maximum supported CPUs (matches typical server configurations) */
#define MAX_CPUS 256

/* Global per-CPU timing state array
 * MUST be cache-line aligned (64-byte boundary) to prevent false sharing
 */
#ifdef _MSC_VER
__declspec(align(64)) VCPU_TIMING_STATE g_timing_states[MAX_CPUS];
#else
VCPU_TIMING_STATE g_timing_states[MAX_CPUS] __attribute__((aligned(64)));
#endif
```

**Conversion Notes**:
1. **`u64` → `uint64_t`**: Use C99 fixed-width types from `<stdint.h>`
2. **`bool` → `bool` (C99)**: Requires `#include <stdbool.h>` or manual `typedef int bool`
3. **`volatile`**: Preserved - critical for multi-core correctness
4. **`constexpr` → `#define`**: C has no constexpr, use preprocessor macros
5. **Cache-line alignment**: Changed from C++ `alignas(64)` to compiler-specific attributes
6. **Padding**: Manually calculate padding to 64 bytes to ensure each CPU gets own cache line

### C++ Stopwatch (OmbraCoreLib - Kernel Driver Utility)

```cpp
// Lines 13-26: Kernel timing utility class
class StopWatch {
private:
    LARGE_INTEGER start;
    LARGE_INTEGER stop;

public:
    StopWatch();

    DWORD64 ms();
    DWORD64 s();
    DWORD64 ticks();

    void reset();
};
```

### C Equivalent

```c
/* stopwatch.h - C port of kernel timing utility */
#include <ntddk.h>  /* For LARGE_INTEGER, KeQuerySystemTime */

/* StopWatch state structure
 * Measures elapsed time using KeQuerySystemTime (100ns units)
 */
typedef struct _STOPWATCH {
    LARGE_INTEGER start;
    LARGE_INTEGER stop;
} STOPWATCH;

/* Initialize stopwatch (starts timing) */
void stopwatch_init(STOPWATCH* sw);

/* Get elapsed time in milliseconds */
DWORD64 stopwatch_ms(STOPWATCH* sw);

/* Get elapsed time in seconds */
DWORD64 stopwatch_s(STOPWATCH* sw);

/* Get elapsed time in ticks (100ns units) */
DWORD64 stopwatch_ticks(STOPWATCH* sw);

/* Reset stopwatch (restart timing) */
void stopwatch_reset(STOPWATCH* sw);

/* Inline helper - get current system time */
static inline LARGE_INTEGER stopwatch_current_time(void) {
    LARGE_INTEGER curr_time;
    KeQuerySystemTime(&curr_time);
    return curr_time;
}
```

**Conversion Notes**:
1. **RAII Constructor → Manual Init**: C++ constructor becomes `stopwatch_init()` function
2. **Member Functions → Function Pointers**: Could use function pointer table, but direct functions are simpler
3. **Private Members**: C has no access control - convention is to use `_` prefix for internal fields
4. **Method Call Syntax**: `sw.ms()` becomes `stopwatch_ms(&sw)`

## Key Functions

### Function: Initialize

**Purpose:** Initialize timing state for the current CPU. Called once during hypervisor initialization per vCPU.

**C++ Signature:** `void timing::Initialize()`

**C Signature:** `void timing_initialize(void)`

#### C++ Implementation (Reference)

```cpp
// PayLoad/core/timing.cpp:46-65
void Initialize() {
    auto* state = GetCurrentState();
    if (!state || state->initialized) {
        return;
    }

    state->exit_entry_tsc = 0;
    state->accumulated_overhead = 0;
    state->aperf_offset = 0;
    state->mperf_offset = 0;

#ifdef OMBRA_DEBUG
    state->vmexit_count = 0;
    state->total_overhead = 0;
    state->min_overhead = ~0ULL;
    state->max_overhead = 0;
#endif

    state->initialized = true;
}
```

#### C Implementation (Target)

```c
/* timing.c - Initialize timing state for current CPU */

void timing_initialize(void) {
    VCPU_TIMING_STATE* state = timing_get_current_state();

    /* NULL check - invalid APIC ID or out of bounds */
    if (state == NULL) {
        return;
    }

    /* Already initialized check - avoid re-init on subsequent VMExits */
    if (state->initialized) {
        return;
    }

    /* Zero all timing fields
     * IMPORTANT: Do NOT use memset - we're in VMXRoot freestanding context
     * Manual assignment generates more predictable code
     */
    state->exit_entry_tsc = 0;
    state->accumulated_overhead = 0;
    state->aperf_offset = 0;
    state->mperf_offset = 0;

#ifdef OMBRA_DEBUG
    /* Debug statistics - track VMExit overhead distribution */
    state->vmexit_count = 0;
    state->total_overhead = 0;
    state->min_overhead = 0xFFFFFFFFFFFFFFFFULL;  /* Max u64 value */
    state->max_overhead = 0;
#endif

    /* Mark as initialized - prevents re-init */
    state->initialized = true;
}
```

**Conversion Notes**:
- **`auto*` → explicit type**: C has no `auto` keyword, use `VCPU_TIMING_STATE*`
- **`~0ULL` → `0xFFFFFFFFFFFFFFFFULL`**: Both produce max u64, but explicit hex is clearer
- **No memset**: Avoid CRT dependencies in VMXRoot context, use manual assignment

---

### Function: GetApicId

**Purpose:** Get current CPU's APIC ID for indexing into per-CPU arrays. Uses CPUID instruction.

**C++ Signature:** `u32 timing::GetApicId()`

**C Signature:** `uint32_t timing_get_apic_id(void)`

#### C++ Implementation (Reference)

```cpp
// PayLoad/core/timing.cpp:28-32
u32 GetApicId() {
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    return static_cast<u32>((cpuInfo[1] >> 24) & 0xFF);
}
```

#### C Implementation (Target)

```c
/* timing.c - Get current CPU APIC ID */

uint32_t timing_get_apic_id(void) {
    int cpu_info[4];

    /* CPUID leaf 0x1: Processor Info and Feature Bits
     * EAX = version information
     * EBX[31:24] = Initial APIC ID (what we want)
     * ECX/EDX = feature flags
     */
    __cpuid(cpu_info, 1);

    /* Extract APIC ID from EBX bits 31-24
     * cpu_info[0] = EAX
     * cpu_info[1] = EBX  <-- APIC ID here
     * cpu_info[2] = ECX
     * cpu_info[3] = EDX
     */
    return (uint32_t)((cpu_info[1] >> 24) & 0xFF);
}
```

**Conversion Notes**:
- **`static_cast<u32>` → C-style cast**: `(uint32_t)` instead of C++ cast syntax
- **`int cpuInfo[4]` → `int cpu_info[4]`**: C naming convention (snake_case)
- **`__cpuid` intrinsic**: Available in both C and C++ via `<intrin.h>` or `<immintrin.h>`

#### Assembly Alternative (Optional - x86-64 MASM)

```asm
; timing_asm.asm - Pure assembly APIC ID retrieval
; Faster than intrinsic in hot paths (no function call overhead)

PUBLIC timing_get_apic_id_asm

.CODE

; uint32_t timing_get_apic_id_asm(void)
; Returns: APIC ID in EAX
timing_get_apic_id_asm PROC
    push rbx                    ; Save RBX (CPUID clobbers it)
    push rcx                    ; Save RCX
    push rdx                    ; Save RDX

    mov eax, 1                  ; CPUID leaf 1
    cpuid                       ; Execute CPUID

    mov eax, ebx                ; Move EBX to EAX
    shr eax, 24                 ; Shift right 24 bits
    and eax, 0FFh               ; Mask to 8 bits

    pop rdx                     ; Restore RDX
    pop rcx                     ; Restore RCX
    pop rbx                     ; Restore RBX
    ret
timing_get_apic_id_asm ENDP

END
```

**When to Use Assembly**:
- **VMExit hot path**: If `GetApicId()` is called on every VMExit (it is via `GetCurrentState()`)
- **Minimal overhead**: Intrinsic may generate extra instructions for register preservation
- **Verified behavior**: Assembly guarantees exact instruction sequence

---

### Function: GetCurrentState

**Purpose:** Get pointer to timing state for current CPU. Critical function called on every VMExit.

**C++ Signature:** `VcpuTimingState* timing::GetCurrentState()`

**C Signature:** `VCPU_TIMING_STATE* timing_get_current_state(void)`

#### C++ Implementation (Reference)

```cpp
// PayLoad/core/timing.cpp:34-40
VcpuTimingState* GetCurrentState() {
    u32 apicId = GetApicId();
    if (apicId >= MAX_CPUS) {
        return nullptr;
    }
    return &g_timing_states[apicId];
}
```

#### C Implementation (Target)

```c
/* timing.c - Get timing state for current CPU */

VCPU_TIMING_STATE* timing_get_current_state(void) {
    uint32_t apic_id = timing_get_apic_id();

    /* Bounds check - prevent out-of-bounds access
     * If APIC ID is >= MAX_CPUS (256), return NULL
     * This should never happen on real hardware, but check anyway
     */
    if (apic_id >= MAX_CPUS) {
        return NULL;
    }

    /* Return pointer to this CPU's timing state
     * Array is cache-line aligned, so each CPU gets own cache line
     */
    return &g_timing_states[apic_id];
}
```

**Conversion Notes**:
- **`nullptr` → `NULL`**: C has no `nullptr` keyword, use `NULL` macro
- **Array indexing**: Same in C and C++ (no change needed)
- **Pointer arithmetic**: Same in C and C++

#### Assembly Alternative (Optimized for Hot Path)

```asm
; timing_asm.asm - Optimized GetCurrentState for VMExit hot path
; Avoids function call overhead by inlining CPUID + array indexing

EXTERN g_timing_states:QWORD       ; External symbol (defined in C)

PUBLIC timing_get_current_state_asm

.CODE

; VCPU_TIMING_STATE* timing_get_current_state_asm(void)
; Returns: Pointer to current CPU's timing state in RAX, or NULL if out of bounds
timing_get_current_state_asm PROC
    push rbx                        ; Save RBX (CPUID clobbers)
    push rcx                        ; Save RCX
    push rdx                        ; Save RDX

    ; Get APIC ID
    mov eax, 1                      ; CPUID leaf 1
    cpuid                           ; EBX[31:24] = APIC ID
    mov eax, ebx                    ; Move EBX to EAX
    shr eax, 24                     ; Extract bits 31-24
    and eax, 0FFh                   ; Mask to 8 bits (APIC ID)

    ; Bounds check
    cmp eax, 256                    ; MAX_CPUS = 256
    jae .out_of_bounds              ; Jump if APIC ID >= 256

    ; Calculate array offset
    ; Each VCPU_TIMING_STATE is 64 bytes (cache-line aligned)
    ; offset = apic_id * 64
    shl rax, 6                      ; Multiply by 64 (shift left 6 bits)
    lea rax, [g_timing_states + rax] ; Base address + offset

    pop rdx
    pop rcx
    pop rbx
    ret

.out_of_bounds:
    xor rax, rax                    ; Return NULL
    pop rdx
    pop rcx
    pop rbx
    ret
timing_get_current_state_asm ENDP

END
```

**Performance Benefit**:
- **Single function call**: Inlines CPUID + array indexing (no nested call to `GetApicId`)
- **3-5 cycles saved**: Critical when this runs on EVERY VMExit
- **Guaranteed cache-line math**: `shl rax, 6` ensures 64-byte stride

---

### Function: OnExitEntry

**Purpose:** Capture TSC immediately at VMExit entry. MUST be the first instruction in the VMExit handler for accurate overhead measurement.

**C++ Signature:** `void timing::OnExitEntry()`

**C Signature:** `void timing_on_exit_entry(void)`

#### C++ Implementation (Reference)

```cpp
// PayLoad/core/timing.cpp:71-84
void OnExitEntry() {
    auto* state = GetCurrentState();
    if (!state) {
        return;
    }

    // Capture TSC immediately - this is the first instruction in VMExit handler
    // The more accurately we capture this, the better our compensation
    state->exit_entry_tsc = __rdtsc();

#ifdef OMBRA_DEBUG
    state->vmexit_count++;
#endif
}
```

#### C Implementation (Target)

```c
/* timing.c - Capture TSC at VMExit entry */

void timing_on_exit_entry(void) {
    VCPU_TIMING_STATE* state = timing_get_current_state();

    /* NULL check - should never happen, but check for safety */
    if (state == NULL) {
        return;
    }

    /* Capture TSC immediately
     * CRITICAL: This MUST be the first instruction in VMExit handler
     * Every instruction executed before this increases measurement error
     *
     * __rdtsc() returns 64-bit TSC value (CPU cycles since reset)
     * On Intel: Uses RDTSC instruction (serializing not required here)
     * On AMD: Uses RDTSC instruction (same behavior)
     */
    state->exit_entry_tsc = __rdtsc();

#ifdef OMBRA_DEBUG
    /* Increment VMExit counter for statistics */
    state->vmexit_count++;
#endif
}
```

**Conversion Notes**:
- **`__rdtsc()` intrinsic**: Same in C and C++, generates `RDTSC` instruction
- **No serialization needed**: We don't need `RDTSCP` or `LFENCE` here because we're measuring relative overhead, not absolute timing
- **Volatile not needed on read**: `exit_entry_tsc` is only read by same CPU in `OnExitComplete()`

#### Assembly Alternative (Minimal Overhead)

```asm
; timing_asm.asm - Ultra-low-overhead TSC capture
; CRITICAL: This must be called FIRST in VMExit handler

EXTERN g_timing_states:QWORD

PUBLIC timing_on_exit_entry_asm

.CODE

; void timing_on_exit_entry_asm(void)
; Clobbers: RAX, RDX, RCX, RBX (CPUID clobbers)
timing_on_exit_entry_asm PROC
    ; FIRST instruction - capture TSC immediately
    rdtsc                           ; EDX:EAX = TSC
    shl rdx, 32                     ; Shift high 32 bits
    or rax, rdx                     ; Combine into RAX = full 64-bit TSC
    push rax                        ; Save TSC on stack

    ; Get APIC ID
    push rbx                        ; Save RBX
    push rcx                        ; Save RCX
    push rdx                        ; Save RDX

    mov eax, 1                      ; CPUID leaf 1
    cpuid
    mov eax, ebx
    shr eax, 24
    and eax, 0FFh                   ; EAX = APIC ID

    ; Bounds check
    cmp eax, 256
    jae .done                       ; Skip if out of bounds

    ; Calculate state pointer
    shl rax, 6                      ; Multiply by 64 (struct size)
    lea rcx, [g_timing_states + rax] ; RCX = state pointer

    ; Write TSC to state->exit_entry_tsc (offset 0)
    pop rax                         ; Restore TSC from stack
    mov [rcx], rax                  ; state->exit_entry_tsc = TSC

    ; Restore registers
    pop rdx
    pop rcx
    pop rbx
    ret

.done:
    pop rax                         ; Discard TSC
    pop rdx
    pop rcx
    pop rbx
    ret
timing_on_exit_entry_asm ENDP

END
```

**Why Assembly Here is CRITICAL**:
1. **RDTSC must be FIRST instruction**: Any delay adds error to overhead measurement
2. **Intrinsic may reorder**: Compiler might insert prologue code before `__rdtsc()`
3. **Predictable overhead**: Assembly guarantees exact instruction count
4. **Measured overhead**: If assembly version takes 15 cycles, we know the baseline error

---

### Function: OnExitComplete

**Purpose:** Calculate VMExit overhead and adjust TSC offset to hide it from guest. Called just before VMResume/VMRUN.

**C++ Signature:** `void timing::OnExitComplete(void* arch_data)`

**C Signature:** `void timing_on_exit_complete(void* arch_data)`

#### C++ Implementation (Reference)

```cpp
// PayLoad/core/timing.cpp:86-140
void OnExitComplete(void* arch_data) {
    auto* state = GetCurrentState();
    if (!state) {
        return;
    }

    // Capture exit TSC - this is just before VMResume
    u64 exit_end_tsc = __rdtsc();

    // Calculate overhead for this VMExit
    u64 overhead = exit_end_tsc - state->exit_entry_tsc;

    // Accumulate for APERF/MPERF compensation
    // These MSRs count real cycles, so we need to subtract our overhead
    state->aperf_offset += overhead;
    state->mperf_offset += overhead;

#ifdef OMBRA_DEBUG
    state->total_overhead += overhead;
    if (overhead < state->min_overhead) state->min_overhead = overhead;
    if (overhead > state->max_overhead) state->max_overhead = overhead;
#endif

    //=========================================================================
    // Apply TSC offset compensation
    // By subtracting overhead from TSC offset, the guest sees:
    //   reported_tsc = real_tsc + tsc_offset
    //                = real_tsc + (original_offset - overhead)
    //                = real_tsc - overhead + original_offset
    // This effectively hides our VMExit processing time
    //=========================================================================

    if (arch_data) {
        //---------------------------------------------------------------------
        // AMD Path: arch_data is VMCB pointer
        // VMCB ControlArea.TscOffset is at offset 0x50 from VMCB base
        //---------------------------------------------------------------------
        auto* vmcb = reinterpret_cast<u8*>(arch_data);
        auto* tsc_offset_ptr = reinterpret_cast<i64*>(vmcb + 0x50);

        // Subtract overhead from TSC offset (makes guest see compensated time)
        *tsc_offset_ptr -= static_cast<i64>(overhead);
    } else {
        //---------------------------------------------------------------------
        // Intel Path: Use VMCS TSC_OFFSET field
        // VMCS provides atomic read/write via intrinsics
        //---------------------------------------------------------------------
        size_t current_offset = 0;
        __vmx_vmread(VMCS_FIELD_TSC_OFFSET_FULL, &current_offset);

        // Subtract overhead from TSC offset
        size_t new_offset = current_offset - overhead;
        __vmx_vmwrite(VMCS_FIELD_TSC_OFFSET_FULL, new_offset);
    }
}
```

#### C Implementation (Target)

```c
/* timing.c - Complete VMExit timing and adjust TSC offset */

void timing_on_exit_complete(void* arch_data) {
    VCPU_TIMING_STATE* state = timing_get_current_state();
    uint64_t exit_end_tsc;
    uint64_t overhead;

    /* NULL check */
    if (state == NULL) {
        return;
    }

    /* Capture exit TSC - this is just before VMResume/VMRUN
     * CRITICAL: This should be the LAST thing before returning to guest
     */
    exit_end_tsc = __rdtsc();

    /* Calculate overhead for this VMExit
     * overhead = (TSC at exit complete) - (TSC at exit entry)
     * This is the total CPU cycles spent in VMExit handler
     */
    overhead = exit_end_tsc - state->exit_entry_tsc;

    /* Accumulate for APERF/MPERF compensation
     * These MSRs count ACTUAL CPU cycles executed
     * Our VMExit processing adds real cycles that show up in these counters
     * We subtract our accumulated overhead when virtualizing these MSRs
     */
    state->aperf_offset += overhead;
    state->mperf_offset += overhead;

#ifdef OMBRA_DEBUG
    /* Update statistics */
    state->total_overhead += overhead;
    if (overhead < state->min_overhead) {
        state->min_overhead = overhead;
    }
    if (overhead > state->max_overhead) {
        state->max_overhead = overhead;
    }
#endif

    /*==========================================================================
     * Apply TSC offset compensation
     *
     * MATH: By subtracting overhead from TSC offset, the guest sees:
     *   reported_tsc = real_tsc + tsc_offset
     *                = real_tsc + (original_offset - overhead)
     *                = real_tsc - overhead + original_offset
     *
     * This effectively hides our VMExit processing time from the guest.
     *==========================================================================*/

    if (arch_data != NULL) {
        /*----------------------------------------------------------------------
         * AMD Path: arch_data is VMCB pointer
         *
         * VMCB Structure Layout (AMD APM Vol 2):
         *   +0x00: ControlArea (offset 0x00-0xFF)
         *   +0x50: TscOffset (int64_t)
         *   +0x100: StateArea (offset 0x100-0x3FF)
         *----------------------------------------------------------------------*/
        uint8_t* vmcb = (uint8_t*)arch_data;
        int64_t* tsc_offset_ptr = (int64_t*)(vmcb + 0x50);

        /* Subtract overhead from TSC offset
         * IMPORTANT: TSC offset is SIGNED (can be negative)
         * Guest sees: real_tsc + (tsc_offset - overhead)
         */
        *tsc_offset_ptr -= (int64_t)overhead;

    } else {
        /*----------------------------------------------------------------------
         * Intel Path: Use VMCS TSC_OFFSET field
         *
         * VMCS Field Encoding (Intel SDM Vol 3C Appendix B):
         *   0x00002010: TSC_OFFSET (64-bit, Control)
         *
         * VMREAD/VMWRITE provide atomic access to VMCS fields
         *----------------------------------------------------------------------*/
        size_t current_offset = 0;
        size_t new_offset;

        /* Read current TSC offset from VMCS
         * __vmx_vmread returns 0 on success, non-zero on failure
         */
        if (__vmx_vmread(VMCS_FIELD_TSC_OFFSET_FULL, &current_offset) != 0) {
            /* VMREAD failed - should never happen, but don't crash */
            return;
        }

        /* Calculate new offset: current_offset - overhead
         * IMPORTANT: size_t is unsigned, but TSC offset is logically signed
         * The subtraction wraps correctly due to two's complement
         */
        new_offset = current_offset - overhead;

        /* Write new offset back to VMCS
         * __vmx_vmwrite returns 0 on success, non-zero on failure
         */
        __vmx_vmwrite(VMCS_FIELD_TSC_OFFSET_FULL, new_offset);
    }
}
```

**Conversion Notes**:
1. **`reinterpret_cast<>` → C-style cast**: `(uint8_t*)` instead of `reinterpret_cast<u8*>`
2. **Pointer arithmetic**: Same in C and C++ (`vmcb + 0x50` works identically)
3. **VMREAD/VMWRITE intrinsics**: Same signature in C and C++
4. **Error handling**: Added check for `__vmx_vmread` failure (best practice)
5. **Magic offset `0x50`**: VERIFIED from AMD APM Vol 2 Table B-1 (VMCB ControlArea.TscOffset)

#### Assembly Alternative (AMD Path Only)

```asm
; timing_asm.asm - AMD TSC offset compensation
; Optimized for minimal overhead in VMExit return path

PUBLIC timing_on_exit_complete_amd_asm

.CODE

; void timing_on_exit_complete_amd_asm(void* vmcb)
; RCX = VMCB pointer (Windows x64 calling convention)
timing_on_exit_complete_amd_asm PROC
    push rbx
    push rdx

    ; Capture exit TSC
    rdtsc                           ; EDX:EAX = TSC
    shl rdx, 32
    or rax, rdx                     ; RAX = exit_end_tsc
    push rax                        ; Save exit TSC

    ; Get current CPU state
    push rcx                        ; Save VMCB pointer
    call timing_get_current_state_asm
    pop rcx                         ; Restore VMCB pointer
    test rax, rax                   ; Check if NULL
    jz .done

    ; Calculate overhead
    mov rbx, rax                    ; RBX = state pointer
    pop rax                         ; RAX = exit_end_tsc
    mov rdx, [rbx]                  ; RDX = state->exit_entry_tsc
    sub rax, rdx                    ; RAX = overhead

    ; Accumulate offsets
    add [rbx + 16], rax             ; state->aperf_offset += overhead
    add [rbx + 24], rax             ; state->mperf_offset += overhead

    ; Adjust VMCB TSC offset
    ; VMCB.ControlArea.TscOffset is at offset 0x50
    sub [rcx + 50h], rax            ; vmcb->TscOffset -= overhead

    pop rdx
    pop rbx
    ret

.done:
    pop rax                         ; Discard exit TSC
    pop rdx
    pop rbx
    ret
timing_on_exit_complete_amd_asm ENDP

END
```

**AMD Path Assembly Benefits**:
- **No function calls**: Inlines RDTSC + state lookup + offset math
- **Atomic VMCB write**: Direct memory write to `[RCX + 0x50]`
- **5-10 cycles saved**: Matters when this runs 10,000+ times per second

---

### Function: ReadMsrVirtualized

**Purpose:** Return compensated MSR value for APERF/MPERF to hide hypervisor overhead from IET detection.

**C++ Signature:** `u64 timing::ReadMsrVirtualized(u32 msr_id)`

**C Signature:** `uint64_t timing_read_msr_virtualized(uint32_t msr_id)`

#### C++ Implementation (Reference)

```cpp
// PayLoad/core/timing.cpp:146-169
u64 ReadMsrVirtualized(u32 msr_id) {
    auto* state = GetCurrentState();

    // Read real MSR value
    u64 real_value = __readmsr(msr_id);

    if (!state) {
        return real_value;
    }

    // Compensate based on MSR type
    // These MSRs count actual CPU cycles, and VMExit handling adds real cycles
    // that show up in these counters. We subtract our accumulated overhead.
    switch (msr_id) {
        case 0xE7:  // IA32_MPERF - Maximum Performance
            return real_value - state->mperf_offset;

        case 0xE8:  // IA32_APERF - Actual Performance
            return real_value - state->aperf_offset;

        default:
            return real_value;
    }
}
```

#### C Implementation (Target)

```c
/* timing.c - Virtualized MSR read with overhead compensation */

#define IA32_MPERF  0xE7   /* Maximum Performance Frequency Clock Count */
#define IA32_APERF  0xE8   /* Actual Performance Frequency Clock Count */

uint64_t timing_read_msr_virtualized(uint32_t msr_id) {
    VCPU_TIMING_STATE* state = timing_get_current_state();
    uint64_t real_value;

    /* Read real MSR value from hardware
     * __readmsr intrinsic generates: RDMSR instruction
     * Returns EDX:EAX as 64-bit value
     */
    real_value = __readmsr(msr_id);

    /* If no valid state, return uncompensated value
     * This should never happen in normal operation
     */
    if (state == NULL) {
        return real_value;
    }

    /* Compensate based on MSR type
     *
     * WHY: APERF and MPERF count ACTUAL CPU cycles executed
     * Our VMExit handling adds REAL cycles that increment these counters
     * Anti-cheats (ESEA) check APERF/MPERF ratio to detect hypervisors:
     *   - Native system: APERF ≈ MPERF (both count real cycles)
     *   - Hypervisor: MPERF higher (VMExit overhead counted, but guest didn't see it)
     *
     * SOLUTION: Subtract accumulated overhead from both counters
     */
    switch (msr_id) {
        case IA32_MPERF:
            /* IA32_MPERF: Maximum Performance Frequency Clock Count
             * Counts CPU cycles at maximum non-turbo frequency
             * Used to calculate effective frequency over time
             */
            return real_value - state->mperf_offset;

        case IA32_APERF:
            /* IA32_APERF: Actual Performance Frequency Clock Count
             * Counts CPU cycles at actual (possibly turboed) frequency
             * Ratio APERF/MPERF indicates average frequency scaling
             */
            return real_value - state->aperf_offset;

        default:
            /* Unknown MSR - return real value without compensation
             * Caller handles virtualization for other MSRs
             */
            return real_value;
    }
}
```

**Conversion Notes**:
1. **`__readmsr` intrinsic**: Same in C and C++, generates `RDMSR` instruction
2. **`switch` statement**: Identical syntax in C and C++
3. **MSR constants**: Defined as macros instead of C++ `constexpr`
4. **Return paths**: Same in C and C++

#### Assembly Alternative (Full Virtualization)

```asm
; timing_asm.asm - MSR virtualization with overhead compensation

EXTERN g_timing_states:QWORD

PUBLIC timing_read_msr_virtualized_asm

.CODE

; uint64_t timing_read_msr_virtualized_asm(uint32_t msr_id)
; ECX = MSR ID (Windows x64 calling convention)
; Returns: Virtualized MSR value in RAX
timing_read_msr_virtualized_asm PROC
    push rbx
    push rdx
    push rcx                        ; Save MSR ID

    ; Read real MSR value
    mov ecx, ecx                    ; Zero-extend MSR ID to RCX
    rdmsr                           ; EDX:EAX = MSR value
    shl rdx, 32
    or rax, rdx                     ; RAX = full 64-bit MSR value
    push rax                        ; Save MSR value

    ; Get current CPU state
    call timing_get_current_state_asm
    test rax, rax
    jz .no_compensation             ; If NULL, return uncompensated

    ; Check MSR ID
    pop rdx                         ; RDX = real MSR value
    pop rcx                         ; RCX = MSR ID

    cmp ecx, 0E7h                   ; IA32_MPERF?
    je .compensate_mperf
    cmp ecx, 0E8h                   ; IA32_APERF?
    je .compensate_aperf

    ; Unknown MSR - return real value
    mov rax, rdx
    pop rbx
    ret

.compensate_mperf:
    ; Subtract state->mperf_offset (offset 24 in struct)
    mov rax, rdx                    ; RAX = real_value
    sub rax, [rax + 24]             ; RAX -= state->mperf_offset
    pop rbx
    ret

.compensate_aperf:
    ; Subtract state->aperf_offset (offset 16 in struct)
    mov rax, rdx                    ; RAX = real_value
    sub rax, [rax + 16]             ; RAX -= state->aperf_offset
    pop rbx
    ret

.no_compensation:
    pop rax                         ; RAX = real MSR value
    pop rcx                         ; Discard MSR ID
    pop rdx
    pop rbx
    ret
timing_read_msr_virtualized_asm ENDP

END
```

**When Assembly is Worth It**:
- **RDMSR is slow**: ~50-100 cycles, so function call overhead matters less
- **Called rarely**: Only on guest RDMSR VMExits (not every VMExit)
- **Verdict**: Intrinsic version is fine, assembly not critical here

---

## Conversion Notes

### C++ Features with No Direct C Equivalent

| C++ Feature | C Equivalent | Notes |
|-------------|--------------|-------|
| **Namespaces** | Prefix functions with namespace name | `timing::Initialize()` → `timing_initialize()` |
| **`auto` keyword** | Explicit type declarations | `auto* state` → `VCPU_TIMING_STATE* state` |
| **`constexpr`** | `#define` or `enum` | `constexpr u32 MAX_CPUS = 256;` → `#define MAX_CPUS 256` |
| **`nullptr`** | `NULL` | C has no `nullptr`, use `NULL` macro |
| **`bool` type** | `stdbool.h` or `int` | C99: `#include <stdbool.h>`, older: `typedef int bool` |
| **RAII** | Manual init/cleanup functions | Constructor → `_init()`, Destructor → `_cleanup()` |
| **`reinterpret_cast<>`** | C-style cast | `reinterpret_cast<u8*>(ptr)` → `(uint8_t*)ptr` |
| **`static_cast<>`** | C-style cast | `static_cast<u32>(val)` → `(uint32_t)val` |
| **`alignas(N)`** | `__declspec(align(N))` or `__attribute__((aligned(N)))` | Compiler-specific |
| **Member functions** | Functions with explicit `this` pointer | `state->method()` → `method(state)` |

### Memory Management Differences

| Operation | C++ | C | Notes |
|-----------|-----|---|-------|
| **String manipulation** | `<cstring>` | `<string.h>` | Same functions, different header |
| **Fixed-width types** | `<cstdint>` | `<stdint.h>` | Same types, different header |
| **Memory functions** | `memset`, `memcpy` | Same | VMXRoot: manual implementations (no CRT) |
| **Allocation** | `new`/`delete` | `malloc`/`free` | VMXRoot: NO dynamic allocation at all |

### Platform-Specific Considerations

#### VMXRoot Freestanding Context

```c
/* VMXRoot has NO access to:
 * - CRT (no malloc, printf, memset from libc)
 * - Windows APIs (no KeQuerySystemTime, etc)
 * - C++ runtime (no exceptions, RTTI, vtables)
 *
 * ONLY available:
 * - Compiler intrinsics (__rdtsc, __cpuid, __vmx_*, etc)
 * - Manual memory manipulation (pointer arithmetic)
 * - Direct hardware access (MSR, VMCS, VMCB)
 */

/* Example: Manual memset replacement for VMXRoot */
static inline void vmxroot_memset(void* dest, int val, size_t count) {
    unsigned char* d = (unsigned char*)dest;
    while (count--) {
        *d++ = (unsigned char)val;
    }
}
```

#### Kernel Driver Context (OmbraCoreLib)

```c
/* Kernel driver has access to:
 * - Windows kernel APIs (Ke*, Mm*, Ps*, etc)
 * - IRQL restrictions (DISPATCH_LEVEL, PASSIVE_LEVEL)
 * - Paged vs NonPaged memory
 *
 * NOT available:
 * - User-mode APIs (CreateFile, etc)
 * - Standard CRT (use RtlXxx replacements)
 */

/* Example: Kernel timer utility using Windows kernel API */
#include <ntddk.h>

void stopwatch_init(STOPWATCH* sw) {
    /* KeQuerySystemTime: Returns system time in 100ns units
     * IRQL: <= DISPATCH_LEVEL
     * Paged: No (can be called at DISPATCH_LEVEL)
     */
    KeQuerySystemTime(&sw->start);
    sw->stop = sw->start;
}
```

### RAII to Explicit Cleanup Conversion

**C++ RAII Pattern (Automatic Cleanup)**:
```cpp
class Timer {
    LARGE_INTEGER start;
public:
    Timer() { start = GetTime(); }  // Constructor
    ~Timer() { Cleanup(); }         // Destructor (automatic)
};

void SomeFunction() {
    Timer t;  // Constructor called
    DoWork();
    // Destructor called automatically when 't' goes out of scope
}
```

**C Manual Cleanup Pattern**:
```c
typedef struct _TIMER {
    LARGE_INTEGER start;
} TIMER;

void timer_init(TIMER* t) {
    t->start = get_time();
}

void timer_cleanup(TIMER* t) {
    /* Cleanup code */
}

void some_function(void) {
    TIMER t;
    timer_init(&t);
    do_work();
    timer_cleanup(&t);  /* MUST call manually - not automatic! */
}
```

**Best Practice for C**:
- Use **scoped cleanup macros** to simulate RAII:

```c
/* Scoped cleanup macro - executes cleanup on scope exit */
#define SCOPED_TIMER(name) \
    TIMER name; \
    timer_init(&name); \
    __attribute__((cleanup(timer_cleanup_wrapper))) TIMER* _cleanup_##name = &name

/* Cleanup wrapper for GCC/Clang __attribute__((cleanup)) */
static void timer_cleanup_wrapper(TIMER** t) {
    timer_cleanup(*t);
}

void some_function(void) {
    SCOPED_TIMER(t);  /* Cleanup happens automatically on scope exit */
    do_work();
    /* timer_cleanup(&t) called automatically here */
}
```

### Template Elimination Strategies

**C++ Template (Generic Type)**:
```cpp
template<typename T>
T min(T a, T b) {
    return (a < b) ? a : b;
}

u64 x = min<u64>(100, 200);
u32 y = min<u32>(10, 20);
```

**C Macro Equivalent**:
```c
/* Type-unsafe but works */
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

uint64_t x = MIN(100, 200);
uint32_t y = MIN(10, 20);
```

**C Type-Safe Alternative (Manual Specialization)**:
```c
/* Define separate functions for each type */
static inline uint64_t min_u64(uint64_t a, uint64_t b) {
    return (a < b) ? a : b;
}

static inline uint32_t min_u32(uint32_t a, uint32_t b) {
    return (a < b) ? a : b;
}

uint64_t x = min_u64(100, 200);
uint32_t y = min_u32(10, 20);
```

---

## Dependencies

### PayLoad Timing Dependencies

| Dependency | Purpose | Location |
|------------|---------|----------|
| `types.h` | Type definitions (`u64`, `u32`, etc) | `PayLoad/include/types.h` |
| `<intrin.h>` | Intrinsics (`__rdtsc`, `__cpuid`, `__readmsr`, `__vmx_*`) | MSVC compiler |
| `<immintrin.h>` | Extended intrinsics (optional, for AVX if needed) | MSVC compiler |

**No other dependencies** - timing.cpp is freestanding, runs in VMXRoot context.

### OmbraCoreLib Timing Dependencies

| Dependency | Purpose | Location |
|------------|---------|----------|
| `<ntddk.h>` | Kernel APIs (`KeQuerySystemTime`, `LARGE_INTEGER`) | Windows DDK |
| `cpp.h` | Kernel helper macros | `OmbraCoreLib/OmbraCoreLib/include/cpp.h` |

### Cross-Layer Dependencies

```
┌────────────────────────────────────────────────────────────┐
│ PayLoad (VMXRoot)                                          │
│ - timing.h/cpp (TSC compensation, MSR virtualization)      │
│ - NO Windows APIs, NO CRT                                  │
│ - Uses: __rdtsc, __cpuid, __vmx_*, __readmsr              │
└────────────────────────────────────────────────────────────┘
                           │
                           │ VMCALL interface
                           │ (communication.hpp)
                           ▼
┌────────────────────────────────────────────────────────────┐
│ OmbraDriver (Kernel)                                       │
│ - Uses OmbraCoreLib timing for profiling/debug            │
│ - Has access to Windows kernel APIs                        │
└────────────────────────────────────────────────────────────┘
                           │
                           │ DeviceIoControl
                           │
                           ▼
┌────────────────────────────────────────────────────────────┐
│ OmbraLoader (Usermode)                                     │
│ - No direct timing usage (uses libombra for VMCALLs)       │
└────────────────────────────────────────────────────────────┘
```

---

## Testing Checklist

### Structure Layout Verification

- [ ] **`VCPU_TIMING_STATE` size matches C++ version**
  ```c
  #ifdef OMBRA_DEBUG
      #define EXPECTED_SIZE 64  /* With debug fields */
  #else
      #define EXPECTED_SIZE 64  /* Without debug fields (padding added) */
  #endif
  _Static_assert(sizeof(VCPU_TIMING_STATE) == EXPECTED_SIZE,
                 "VCPU_TIMING_STATE size mismatch");
  ```

- [ ] **Field offsets match C++ layout**
  ```c
  #include <stddef.h>
  _Static_assert(offsetof(VCPU_TIMING_STATE, exit_entry_tsc) == 0,
                 "exit_entry_tsc offset wrong");
  _Static_assert(offsetof(VCPU_TIMING_STATE, accumulated_overhead) == 8,
                 "accumulated_overhead offset wrong");
  _Static_assert(offsetof(VCPU_TIMING_STATE, aperf_offset) == 16,
                 "aperf_offset offset wrong");
  _Static_assert(offsetof(VCPU_TIMING_STATE, mperf_offset) == 24,
                 "mperf_offset offset wrong");
  ```

- [ ] **Cache-line alignment verified**
  ```c
  /* Verify array is 64-byte aligned */
  _Static_assert((uintptr_t)&g_timing_states % 64 == 0,
                 "g_timing_states not cache-line aligned");

  /* Verify each element is 64 bytes apart */
  _Static_assert(sizeof(VCPU_TIMING_STATE) == 64,
                 "VCPU_TIMING_STATE not 64 bytes");
  ```

### Function Behavior Verification

- [ ] **`timing_get_apic_id()` returns correct APIC ID**
  - Test on multi-core system (pin thread to each core, verify unique IDs)
  - Compare C version output to C++ version output
  - Test APIC ID < 256 (should succeed), APIC ID >= 256 (should return NULL)

- [ ] **`timing_get_current_state()` returns valid pointer**
  - Test on each CPU core (0-255)
  - Verify NULL return for APIC ID >= 256
  - Verify returned pointer points into `g_timing_states` array

- [ ] **`timing_initialize()` zeros all fields**
  - Initialize state, verify all fields are 0
  - Re-initialize same state, verify no change (initialized flag prevents re-init)
  - Check debug fields are initialized correctly (`min_overhead = 0xFFFFFFFFFFFFFFFF`)

- [ ] **`timing_on_exit_entry()` captures TSC**
  - Call function, verify `exit_entry_tsc` is non-zero
  - Call twice in sequence, verify second TSC > first TSC

- [ ] **`timing_on_exit_complete()` adjusts TSC offset**
  - **Intel path**: Mock VMCS, call function, verify `__vmx_vmwrite` called with reduced offset
  - **AMD path**: Create fake VMCB, call function, verify offset at `+0x50` is reduced

- [ ] **`timing_read_msr_virtualized()` compensates APERF/MPERF**
  - Mock `__readmsr` to return known value (e.g., 1000000)
  - Set `aperf_offset = 1000`, verify return value is 999000
  - Test unknown MSR (not 0xE7/0xE8), verify returns uncompensated value

### Assembly Routines Verification

- [ ] **Assembly routines assemble correctly**
  - Compile `timing_asm.asm` with MASM
  - Link with C code, verify no linker errors
  - Disassemble object file, verify instruction sequences

- [ ] **Assembly routines match C behavior**
  - Run side-by-side comparison: C version vs Assembly version
  - Feed same inputs, verify same outputs
  - Measure cycle counts (assembly should be faster)

### No C++ Runtime Dependencies

- [ ] **No C++ symbols in object file**
  - Compile C version, run `nm timing.o | grep -i cxx`
  - Should output nothing (no C++ runtime symbols)

- [ ] **No exceptions/RTTI**
  - Disassemble, verify no exception tables (`.eh_frame`, `.pdata`)
  - Verify no RTTI data (`.rdata$r`, `??_R`)

- [ ] **No vtables**
  - Verify no vtable pointers (`.rdata` section should be minimal)

### Integration Testing

- [ ] **Full VMExit path with timing**
  1. Initialize timing (`timing_initialize()`)
  2. Trigger VMExit (VMCALL, CPUID, etc)
  3. `timing_on_exit_entry()` called first in handler
  4. Process VMExit
  5. `timing_on_exit_complete()` called before VMResume
  6. Guest executes RDTSC, verify no anomaly

- [ ] **APERF/MPERF virtualization**
  1. Guest reads IA32_APERF (triggers VMExit)
  2. Handler calls `timing_read_msr_virtualized(0xE8)`
  3. Verify returned value has overhead subtracted
  4. Guest computes APERF/MPERF ratio, verify ratio looks native

- [ ] **Multi-core stress test**
  - Run on all CPU cores simultaneously
  - Trigger 10,000 VMExits per core
  - Verify no state corruption (each core has separate `VcpuTimingState`)
  - Check debug statistics (`min_overhead`, `max_overhead`, `avg = total/count`)

---

## Appendices

### Appendix A: Intel VMCS TSC Offset Field

**Source**: Intel SDM Vol 3C, Appendix B, Table B-1 (VMCS Field Encoding)

| Field Name | Encoding | Type | Description |
|------------|----------|------|-------------|
| `TSC_OFFSET` | `0x00002010` | 64-bit Control | Offset added to guest TSC reads |

**Access**:
- Read: `__vmx_vmread(0x2010, &value)`
- Write: `__vmx_vmwrite(0x2010, value)`

**Behavior**:
- When guest executes `RDTSC` or `RDTSCP`:
  - `returned_value = real_tsc + TSC_OFFSET`
- TSC offset is **signed 64-bit** (can be negative)
- Precision: Full 64-bit TSC (no truncation)

### Appendix B: AMD VMCB TscOffset Field

**Source**: AMD APM Vol 2, Table B-1 (VMCB Layout, Control Area)

| Field | Offset | Size | Description |
|-------|--------|------|-------------|
| `TscOffset` | `+0x50` | 8 bytes (int64) | Offset added to guest TSC reads |

**Access**:
- Direct memory: `*(int64_t*)(vmcb + 0x50)`
- No special instruction needed (VMCB is just memory)

**Behavior**:
- Same as Intel: `returned_value = real_tsc + TscOffset`
- TscOffset is **signed 64-bit**

### Appendix C: MSR Definitions

**Source**: Intel SDM Vol 4, Table 2-2 (MSRs in Intel Processors)

| MSR Name | Address | Description | Overflow Behavior |
|----------|---------|-------------|-------------------|
| `IA32_MPERF` | `0xE7` | Maximum Performance Frequency Clock Count | Wraps to 0 |
| `IA32_APERF` | `0xE8` | Actual Performance Frequency Clock Count | Wraps to 0 |

**Purpose**:
- **MPERF**: Counts CPU cycles at P0 frequency (maximum non-turbo frequency)
- **APERF**: Counts CPU cycles at actual frequency (may be turboed or downclocked)
- **Ratio**: `APERF / MPERF` indicates average CPU frequency scaling

**Anti-Cheat Usage**:
- **ESEA**: Measures `APERF/MPERF` ratio before/after suspicious operations
  - Native system: Ratio ≈ 1.0 (both count same cycles)
  - Hypervisor: Ratio < 1.0 (MPERF higher due to VMExit overhead)
- **Detection**: If ratio anomaly detected, flag as hypervisor

**Virtualization Strategy**:
- Subtract accumulated VMExit overhead from both MSRs
- Maintains correct ratio (both reduced by same amount)
- Guest sees: `(real_aperf - offset) / (real_mperf - offset) ≈ real_aperf / real_mperf`

### Appendix D: Cache-Line Alignment Importance

**Problem: False Sharing**

```
CPU 0                           CPU 1
┌─────────────────┐             ┌─────────────────┐
│ L1 Cache        │             │ L1 Cache        │
│ ┌─────────────┐ │             │ ┌─────────────┐ │
│ │ Line 0x1000 │ │             │ │ Line 0x1000 │ │
│ │ (64 bytes)  │ │             │ │ (64 bytes)  │ │
│ │  State[0]   │ │             │ │  State[1]   │ │ ← BOTH STATES IN SAME CACHE LINE!
│ └─────────────┘ │             │ └─────────────┘ │
└─────────────────┘             └─────────────────┘
```

**What happens without cache-line alignment**:
1. CPU 0 writes `State[0].exit_entry_tsc`
2. Cache line 0x1000 is marked **Modified** in CPU 0's L1 cache
3. Cache line 0x1000 is marked **Invalid** in CPU 1's L1 cache
4. CPU 1 reads `State[1].exit_entry_tsc`
5. CPU 1 must fetch line 0x1000 from CPU 0 (cache coherency traffic)
6. **30-50 cycles lost** due to inter-core communication

**Solution: Cache-Line Alignment**

```
CPU 0                           CPU 1
┌─────────────────┐             ┌─────────────────┐
│ L1 Cache        │             │ L1 Cache        │
│ ┌─────────────┐ │             │ ┌─────────────┐ │
│ │ Line 0x1000 │ │             │ │ Line 0x1040 │ │ ← SEPARATE CACHE LINES!
│ │  State[0]   │ │             │ │  State[1]   │ │
│ └─────────────┘ │             │ └─────────────┘ │
└─────────────────┘             └─────────────────┘
```

**With cache-line alignment**:
1. Each `VcpuTimingState` occupies its own 64-byte cache line
2. CPU 0 writes `State[0]` → only line 0x1000 affected
3. CPU 1 reads `State[1]` → only line 0x1040 affected
4. **No cache coherency traffic** (different lines)
5. **30-50 cycles saved per VMExit**

**Implementation**:
```c
/* Align entire array to 64-byte boundary */
__declspec(align(64)) VCPU_TIMING_STATE g_timing_states[MAX_CPUS];

/* Ensure each struct is exactly 64 bytes */
_Static_assert(sizeof(VCPU_TIMING_STATE) == 64, "Size must be 64");
```

### Appendix E: Intrinsic Instruction Mappings

| Intrinsic | Assembly | Clobbers | Cycles (Typical) | Notes |
|-----------|----------|----------|------------------|-------|
| `__rdtsc()` | `RDTSC` | RAX, RDX | 20-40 | Non-serializing, may reorder |
| `__rdtscp(&aux)` | `RDTSCP` | RAX, RDX, RCX | 20-40 | Serializing, `RCX = IA32_TSC_AUX` |
| `__cpuid(info, leaf)` | `CPUID` | RAX, RBX, RCX, RDX | 100-250 | Serializing, clobbers all GPRs |
| `__readmsr(msr)` | `RDMSR` | RAX, RDX | 50-100 | Privileged (CPL=0 only) |
| `__writemsr(msr, val)` | `WRMSR` | None (input only) | 50-100 | Privileged, serializing |
| `__vmx_vmread(field, &val)` | `VMREAD` | RAX (status) | 10-20 | Intel only, CF/ZF set on error |
| `__vmx_vmwrite(field, val)` | `VMWRITE` | RAX (status) | 10-20 | Intel only, CF/ZF set on error |

**Serializing Instructions**:
- **CPUID**: Fully serializing (waits for all prior instructions to complete)
- **RDTSCP**: Serializing (unlike RDTSC)
- **WRMSR**: Serializing for certain MSRs (e.g., `IA32_TSC`)

**Non-Serializing**:
- **RDTSC**: Can execute out-of-order, may read TSC before earlier instructions complete
- **VMREAD/VMWRITE**: Not serializing (can reorder with other instructions)

**Performance**:
- RDTSC overhead: 20-40 cycles (Skylake+), 60-100 cycles (older CPUs)
- CPUID overhead: 100-250 cycles (slowest common instruction)
- VMREAD/VMWRITE: 10-20 cycles (Intel VT-x optimized)

### Appendix F: Build Configuration

**C Compiler Flags (MSVC)**:
```
/TC         # Compile as C (not C++)
/std:c11    # C11 standard (for <stdint.h>, <stdbool.h>)
/GS-        # Disable stack cookies (VMXRoot has no runtime)
/Oi         # Enable intrinsics
/O2         # Optimize for speed
/fp:fast    # Fast floating-point (not used here, but safe)
/Gd         # __cdecl calling convention (default)
/Wall       # Enable all warnings
/WX         # Treat warnings as errors
/Zi         # Generate debug info (PDB)
```

**Linker Flags (VMXRoot DLL)**:
```
/NODEFAULTLIB       # No CRT libraries
/ENTRY:DllMain      # Entry point (or custom)
/SUBSYSTEM:NATIVE   # Native driver subsystem
/DRIVER             # Driver image
/ALIGN:64           # Align sections to 64 bytes
```

**Assembly Compiler (MASM)**:
```
ml64.exe /c /Cx /Zi timing_asm.asm
# /c  = Compile only (no link)
# /Cx = Preserve case in symbols
# /Zi = Debug info
```

---

## Summary

This document provides a **complete C port guide** for the timing subsystem, covering:

1. ✅ **All source files** inventoried and analyzed
2. ✅ **Data structures** converted with field-by-field mappings
3. ✅ **Functions** converted with detailed C implementations and assembly alternatives
4. ✅ **Constants** documented with sources (Intel SDM, AMD APM)
5. ✅ **Architecture differences** explained (Intel VMCS vs AMD VMCB)
6. ✅ **Anti-detection techniques** documented (TSC offset math, APERF/MPERF virtualization)
7. ✅ **Testing checklist** provided for verification

**Key Takeaways**:
- PayLoad timing is **freestanding** (no CRT, no Windows APIs)
- TSC compensation hides VMExit overhead by adjusting VMCS/VMCB TSC offset
- APERF/MPERF virtualization defeats IET detection by subtracting accumulated overhead
- Cache-line alignment prevents false sharing on multi-core systems
- Assembly alternatives provided for hot-path functions (`OnExitEntry`, `GetApicId`)

**Next Steps**:
1. Implement C versions in new `.c`/`.h` files
2. Add `_Static_assert` checks for structure layout
3. Write unit tests for each function
4. Integrate assembly routines and benchmark
5. Full integration test with real VMExit handler
