# Shadow Page Hooking

> **Purpose**: The core EPT/NPT technique - using permission splits to create invisible hooks that execute modified code while appearing unmodified to memory scanners.

> **Implementation target**: OmbraPayload (Ring -1, C++ with restrictions, BOTH Intel & AMD)

---

## Overview

Shadow page hooking is the crown jewel of hypervisor-based stealth. The technique exploits EPT/NPT permissions to present different memory contents based on access type: when something reads a hooked page, it sees the original unmodified bytes; when something executes from that page, it runs our hook code.

This invisibility makes shadow hooks undetectable to:
- Code integrity scanners (see clean bytes)
- PatchGuard (pages appear unmodified)
- Anti-cheat memory scans (no byte differences found)

Intel's EPT supports this natively with execute-only pages. AMD's NPT lacks execute-only, requiring a more complex dual-NPT or state machine approach.

---

## The Shadow Page Concept

### Core Principle: Dual Memory Views

For each hooked page, maintain two physical backing pages:

```
                    Guest Physical Address (GPA)
                           0x12345000
                               │
            ┌──────────────────┴──────────────────┐
            │                                     │
       READ/WRITE                             EXECUTE
            │                                     │
            ▼                                     ▼
    ┌───────────────┐                   ┌───────────────┐
    │ Original Page │                   │ Shadow Page   │
    │   (Clean)     │                   │  (Hooked)     │
    │               │                   │               │
    │ push rbp      │                   │ int3          │
    │ mov rbp,rsp   │                   │ mov rbp,rsp   │
    │ sub rsp,0x20  │                   │ sub rsp,0x20  │
    │ ...           │                   │ ...           │
    └───────────────┘                   └───────────────┘
         HPA A                               HPA B

    Memory scanner                      Code execution
    sees clean code                     hits our hook
```

### What Each Page Contains

**Original Page (RW Shadow)**:
- Exact copy of the original, unmodified page
- Shown to any code that attempts to read or write
- Passes all integrity checks
- Used for disassembly, debugging, scanning

**Shadow Page (Exec Shadow)**:
- Copy of original with hook installed
- Only accessed during execution
- Contains breakpoint (0xCC) or jump to handler
- Never visible to memory reads

---

## Intel EPT: Execute-Only Approach

### Native Execute-Only Support

Intel EPT uniquely supports execute-only pages: `R=0, W=0, X=1`

```
EPT PTE Permissions:
┌───┬───┬───┐
│ R │ W │ X │
├───┼───┼───┤
│ 0 │ 0 │ 1 │  Execute-Only
└───┴───┴───┘

Read attempt  → EPT Violation (VM-exit)
Write attempt → EPT Violation (VM-exit)
Execute       → Allowed (maps to shadow page)
```

This is the cleanest implementation: a single EPT configuration provides the split behavior.

### Intel Hook Installation Flow

```
1. Identify target function virtual address
           │
           ▼
2. Translate VA → GPA → HPA (original page)
           │
           ▼
3. Split 2MB large page to 4KB if needed
           │
           ▼
4. Allocate RW shadow page (copy of original)
           │
           ▼
5. Allocate Exec shadow page (copy of original)
           │
           ▼
6. Install hook (INT3) on Exec shadow page
           │
           ▼
7. Build trampoline for calling original
           │
           ▼
8. Configure EPT PTE:
   - Read = 0, Write = 0, Execute = 1
   - PFN = Exec shadow page physical
           │
           ▼
9. Invalidate TLB (INVEPT)
           │
           ▼
10. Hook active and invisible
```

### Intel EPT Configuration

**From DdiMon shadow_hook.cpp pattern**:

```cpp
// Enable execute-only view (hook active)
void EnableExecOnlyView(EptState* state, u64 gpa, u64 exec_shadow_hpa) {
    EptPte* pte = Ept::GetPte(state, gpa);

    pte->read = 0;     // Deny read
    pte->write = 0;    // Deny write
    pte->execute = 1;  // Allow execute
    pte->pfn = exec_shadow_hpa >> 12;

    InvalidateEpt();
}

// Enable RW view (for handling read/write violations)
void EnableRwView(EptState* state, u64 gpa, u64 rw_shadow_hpa) {
    EptPte* pte = Ept::GetPte(state, gpa);

    pte->read = 1;     // Allow read
    pte->write = 1;    // Allow write
    pte->execute = 0;  // Deny execute (prevent re-entry)
    pte->pfn = rw_shadow_hpa >> 12;

    InvalidateEpt();
}
```

### Intel Violation Handling Flow

When guest attempts to read hooked page:

```
1. Guest reads from hooked GPA
           │
           ▼
2. EPT Violation (R=0, W=0, X=1 blocks read)
   Exit Qualification shows read attempt
           │
           ▼
3. Handler identifies hooked page
           │
           ▼
4. Switch EPT to RW view (clean page)
           │
           ▼
5. Enable Monitor Trap Flag (MTF)
           │
           ▼
6. Resume guest (VMRESUME)
           │
           ▼
7. Guest executes ONE instruction (reads clean byte)
           │
           ▼
8. MTF VM-exit fires
           │
           ▼
9. Handler restores execute-only view
           │
           ▼
10. Disable MTF, resume guest
```

---

## AMD NPT: State Machine Approach

### The Execute-Only Problem

AMD NPT uses standard AMD64 page table format with no execute-only capability:

```
NPT Entry Permissions:
┌─────────┬───────┬─────┐
│ Present │ Write │ NX  │  Meaning
├─────────┼───────┼─────┤
│    0    │   -   │  -  │  No access at all
│    1    │   0   │  0  │  Read + Execute
│    1    │   1   │  0  │  Read + Write + Execute
│    1    │   0   │  1  │  Read only
│    1    │   1   │  1  │  Read + Write
└─────────┴───────┴─────┘

Cannot express: Execute without Read (--X)
```

### AMD Solution: NPT State Machine

Use a three-state system that transitions based on memory access patterns:

```
State Machine:
                          Page Type
State                     Current : Hooked : Other
─────────────────────────────────────────────────────────
0) NptDefault             RWX(O)  : RWX(O) : RWX(O)
1) NptHookEnabledInvisible RWX(O)  : RW-(O) : RWX(O)
2) NptHookEnabledVisible   RWX(E)  : RW-(O) : RW-(O)

Legend:
  Current = Page processor is currently executing on
  Hooked  = Pages with hooks installed (not currently executing)
  Other   = All other pages
  (O)     = Original physical page (clean bytes)
  (E)     = Exec physical page (with hooks)
  RWX     = Read/Write/Execute
  RW-     = Read/Write only (No Execute via NX bit)
```

### State Transitions

**State 1 (Invisible) → State 2 (Visible)**:
Triggered when guest tries to execute a hooked page.

```cpp
// From SimpleSvmHook pattern
void TransitionState1To2(NptState* state, HookEntry* hook) {
    // Step 1: Make ALL pages non-executable
    SetAllPagesNx(state, true);

    // Step 2: Get NPT entry for hook page
    NptPte* pte = Npt::GetPte(state, hook->target_gpa);

    // Step 3: Switch backing to exec shadow (with hook)
    pte->pfn = hook->exec_shadow_hpa >> 12;

    // Step 4: Make only this page executable
    pte->nx = 0;

    // Update state
    g_active_hook = hook;
    g_npt_state = NptHookEnabledVisible;

    FlushTlb();
}
```

**State 2 (Visible) → State 1 (Invisible)**:
Triggered when execution leaves hooked page.

```cpp
void TransitionState2To1(NptState* state) {
    // Step 1: Make all pages executable
    SetAllPagesNx(state, false);

    // Step 2: Make all hooked pages NX again
    for (auto& hook : g_hooks) {
        NptPte* pte = Npt::GetPte(state, hook.target_gpa);
        pte->nx = 1;
    }

    // Step 3: Switch current page back to original
    NptPte* pte = Npt::GetPte(state, g_active_hook->target_gpa);
    pte->pfn = g_active_hook->original_hpa >> 12;

    // Update state
    g_active_hook = nullptr;
    g_npt_state = NptHookEnabledInvisible;

    FlushTlb();
}
```

### AMD Performance Impact

Each hook invocation causes multiple NPT faults:

```
Hook A calls Hook B:
  1. Enter Hook A: State 1→2 (NPF, full TLB flush)
  2. Exit Hook A:  State 2→1 (NPF, full TLB flush)
  3. Enter Hook B: State 1→2 (NPF, full TLB flush)
  4. Exit Hook B:  State 2→1 (NPF, full TLB flush)

Total: 4+ VM-exits per hook chain
       Each: ~1000+ CPU cycles
```

**Optimization**: Minimize hook-to-hook calls, or use function emulation instead of detours.

---

## Hook Installation Implementation

### Data Structures

```cpp
struct HookEntry {
    // Target identification
    void* target_va;           // Virtual address being hooked
    u64 target_gpa;            // Guest physical address
    u64 original_hpa;          // Original host physical address

    // Shadow pages
    void* rw_shadow_va;        // RW shadow (clean copy)
    u64 rw_shadow_hpa;         // Physical address of RW shadow
    void* exec_shadow_va;      // Exec shadow (hooked copy)
    u64 exec_shadow_hpa;       // Physical address of exec shadow

    // Hook mechanics
    void* handler;             // Our hook handler function
    void* trampoline;          // Calls original function
    u32 stolen_bytes;          // Size of overwritten instructions

    // State
    bool enabled;
};

struct ShadowHookManager {
    HookEntry hooks[MAX_HOOKS];
    u32 hook_count;

    // Per-CPU state for MTF handling (Intel)
    HookEntry* last_hook[MAX_CPUS];

    // Global state for NPT (AMD)
    HookEntry* active_hook;
    NptState npt_state;
};
```

### Shadow Page Allocation

```cpp
bool AllocateShadowPages(HookEntry* entry, void* target_va) {
    // Get page-aligned base
    void* page_base = (void*)((u64)target_va & ~0xFFF);
    u64 page_offset = (u64)target_va & 0xFFF;

    // Allocate RW shadow (clean copy)
    entry->rw_shadow_va = AllocatePoolAligned(PAGE_SIZE, PAGE_SIZE);
    if (!entry->rw_shadow_va) return false;

    // Allocate Exec shadow (will contain hook)
    entry->exec_shadow_va = AllocatePoolAligned(PAGE_SIZE, PAGE_SIZE);
    if (!entry->exec_shadow_va) {
        FreePool(entry->rw_shadow_va);
        return false;
    }

    // Copy original page to both shadows
    memcpy(entry->rw_shadow_va, page_base, PAGE_SIZE);
    memcpy(entry->exec_shadow_va, page_base, PAGE_SIZE);

    // Get physical addresses
    entry->rw_shadow_hpa = VirtualToPhysical(entry->rw_shadow_va);
    entry->exec_shadow_hpa = VirtualToPhysical(entry->exec_shadow_va);

    return true;
}
```

### Shadow Page Reuse

When hooking multiple functions on the same 4KB page, reuse existing shadows:

```cpp
HookEntry* FindExistingPageHook(void* target_va) {
    u64 page_base = (u64)target_va & ~0xFFF;

    for (u32 i = 0; i < g_hook_count; i++) {
        u64 hook_page = (u64)g_hooks[i].target_va & ~0xFFF;
        if (hook_page == page_base) {
            return &g_hooks[i];
        }
    }
    return nullptr;
}

bool InstallHook(void* target, void* handler, void** trampoline) {
    HookEntry* entry = &g_hooks[g_hook_count++];

    // Check for existing hook on same page
    HookEntry* existing = FindExistingPageHook(target);
    if (existing) {
        // Reuse shadow pages
        entry->rw_shadow_va = existing->rw_shadow_va;
        entry->rw_shadow_hpa = existing->rw_shadow_hpa;
        entry->exec_shadow_va = existing->exec_shadow_va;
        entry->exec_shadow_hpa = existing->exec_shadow_hpa;
    } else {
        // Allocate new shadow pages
        if (!AllocateShadowPages(entry, target)) {
            return false;
        }
    }

    // Continue with hook installation...
}
```

---

## Trampoline Construction

### Purpose

The trampoline allows hook handlers to call the original function:

```
Original Function:
┌──────────────────────┐
│ push rbp             │ ← Stolen bytes (overwritten by hook)
│ mov rbp, rsp         │
│ sub rsp, 0x20        │ ← Execution continues from here
│ ...                  │
└──────────────────────┘

Trampoline:
┌──────────────────────┐
│ push rbp             │ ← Copied stolen bytes
│ mov rbp, rsp         │
│ jmp [OrigFunc+5]     │ ← Jump back after stolen bytes
└──────────────────────┘

Hook Handler:
┌──────────────────────┐
│ // Pre-hook logic    │
│ call trampoline      │ ← Calls original function
│ // Post-hook logic   │
│ ret                  │
└──────────────────────┘
```

### Trampoline Structure

```cpp
#pragma pack(push, 1)
struct TrampolineCode {
    u8 stolen_bytes[16];     // Copied from original
    u8 nop;                  // 0x90 - padding
    u8 jmp_opcode[6];        // FF 25 00 00 00 00
    u64 return_address;      // Original + stolen_size
};
#pragma pack(pop)
static_assert(sizeof(TrampolineCode) <= 32);

// jmp qword ptr [rip+0] encoding:
// FF 25 00 00 00 00 = jmp qword ptr cs:[rip+0]
// Followed by 8-byte address
```

### Building the Trampoline

```cpp
void* BuildTrampoline(void* target, u32 stolen_size) {
    // Allocate executable memory
    TrampolineCode* tramp = (TrampolineCode*)AllocateExecutablePool(sizeof(TrampolineCode));
    if (!tramp) return nullptr;

    // Copy stolen bytes
    memcpy(tramp->stolen_bytes, target, stolen_size);

    // NOP padding
    tramp->nop = 0x90;

    // jmp qword ptr [rip+0]
    tramp->jmp_opcode[0] = 0xFF;
    tramp->jmp_opcode[1] = 0x25;
    tramp->jmp_opcode[2] = 0x00;
    tramp->jmp_opcode[3] = 0x00;
    tramp->jmp_opcode[4] = 0x00;
    tramp->jmp_opcode[5] = 0x00;

    // Return address = original + stolen bytes
    tramp->return_address = (u64)target + stolen_size;

    return tramp;
}
```

### Determining Stolen Byte Size

Use a length disassembler to find instruction boundaries:

**Zydis (recommended)**:
```cpp
u32 GetStolenSize(void* address, u32 min_size) {
    ZydisDecoder decoder;
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

    ZydisDecodedInstruction instruction;
    ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];

    u32 offset = 0;
    while (offset < min_size) {
        if (!ZYAN_SUCCESS(ZydisDecoderDecodeFull(&decoder,
            (u8*)address + offset, 15, &instruction, operands))) {
            return 0;  // Decode error
        }
        offset += instruction.length;
    }
    return offset;
}
```

**Pattern Matching (fallback)**:
```cpp
// Common Windows x64 prologues
static const struct { u8 bytes[8]; u32 len; } known_patterns[] = {
    { {0x40, 0x53}, 2 },                    // push rbx
    { {0x40, 0x55}, 2 },                    // push rbp
    { {0x48, 0x83, 0xEC}, 4 },              // sub rsp, imm8
    { {0x48, 0x89, 0x5C, 0x24}, 5 },        // mov [rsp+xx], rbx
    { {0x48, 0x8B, 0xC4}, 3 },              // mov rax, rsp
    { {0x33, 0xD2}, 2 },                    // xor edx, edx
    // ... more patterns
};
```

---

## Breakpoint vs Jump Hooks

### INT3 Breakpoint Method

**Installation**: Write single byte `0xCC` to exec shadow

```cpp
void InstallBreakpointHook(HookEntry* entry) {
    u32 page_offset = (u64)entry->target_va & 0xFFF;

    // Write INT3 to exec shadow
    u8* hook_location = (u8*)entry->exec_shadow_va + page_offset;
    *hook_location = 0xCC;
}
```

**Handling**: Catch #BP exception in VM-exit handler

```cpp
bool HandleBreakpoint(u64 guest_rip) {
    // Find hook by address
    for (u32 i = 0; i < g_hook_count; i++) {
        if ((u64)g_hooks[i].target_va == guest_rip) {
            // Redirect to handler
            VmWrite(GUEST_RIP, (u64)g_hooks[i].handler);
            return true;
        }
    }
    // Not our breakpoint - inject to guest
    return false;
}
```

**Advantages**:
- Only 1 byte modification
- Works on functions of any size
- No instruction boundary concerns

### JMP Method

**Installation**: Write 14-byte absolute jump

```cpp
void InstallJumpHook(HookEntry* entry) {
    u32 page_offset = (u64)entry->target_va & 0xFFF;
    u8* hook_location = (u8*)entry->exec_shadow_va + page_offset;

    // ff 25 00 00 00 00 = jmp qword ptr [rip+0]
    hook_location[0] = 0xFF;
    hook_location[1] = 0x25;
    hook_location[2] = 0x00;
    hook_location[3] = 0x00;
    hook_location[4] = 0x00;
    hook_location[5] = 0x00;

    // 8-byte address follows
    *(u64*)(hook_location + 6) = (u64)entry->handler;
}
```

**Advantages**:
- No exception handling overhead
- Faster execution path

**Disadvantages**:
- Requires 14 bytes minimum
- Must respect instruction boundaries

---

## Multi-Hook Management

### Hook Table

```cpp
struct ShadowHookManager {
    // All registered hooks
    HookEntry hooks[MAX_HOOKS];
    u32 hook_count;

    // Fast lookup by GPA (for violation handling)
    struct {
        u64 gpa;
        u32 hook_index;
    } gpa_lookup[MAX_HOOKS];

    // Fast lookup by RIP (for breakpoint handling)
    struct {
        u64 rip;
        u32 hook_index;
    } rip_lookup[MAX_HOOKS];

    HookEntry* FindByGpa(u64 gpa) {
        u64 page_gpa = gpa & ~0xFFF;
        for (u32 i = 0; i < hook_count; i++) {
            if ((hooks[i].target_gpa & ~0xFFF) == page_gpa) {
                return &hooks[i];
            }
        }
        return nullptr;
    }

    HookEntry* FindByRip(u64 rip) {
        for (u32 i = 0; i < hook_count; i++) {
            if ((u64)hooks[i].target_va == rip) {
                return &hooks[i];
            }
        }
        return nullptr;
    }
};
```

### Per-CPU State

For Intel MTF handling, track which hook caused the violation on each CPU:

```cpp
struct PerCpuHookState {
    HookEntry* last_hook;     // For MTF restoration
    u64 original_gpa;         // GPA that triggered violation
};

PerCpuHookState g_per_cpu_state[MAX_CPUS];

void HandleEptViolation(u32 cpu, u64 faulting_gpa) {
    HookEntry* hook = g_manager.FindByGpa(faulting_gpa);
    if (!hook) return;

    // Save for MTF handler
    g_per_cpu_state[cpu].last_hook = hook;
    g_per_cpu_state[cpu].original_gpa = faulting_gpa;

    // Switch to RW view
    EnableRwView(hook);

    // Enable MTF for single-step
    EnableMtf();
}

void HandleMtf(u32 cpu) {
    HookEntry* hook = g_per_cpu_state[cpu].last_hook;

    // Restore execute-only view
    EnableExecOnlyView(hook);

    // Disable MTF
    DisableMtf();

    g_per_cpu_state[cpu].last_hook = nullptr;
}
```

---

## Unified Patterns

### Pattern: Vendor-Abstracted Hook Manager

```cpp
namespace Ombra::Hooks {

class IShadowHookManager {
public:
    virtual bool Initialize() = 0;
    virtual bool InstallHook(void* target, void* handler, void** trampoline) = 0;
    virtual bool RemoveHook(void* target) = 0;
    virtual void HandleViolation(u64 gpa, u64 exit_qual) = 0;
    virtual void HandleBreakpoint(u64 guest_rip) = 0;
};

class IntelHookManager : public IShadowHookManager {
    // Uses execute-only EPT pages
    // Single EPT structure
    // MTF for read consistency
};

class AmdHookManager : public IShadowHookManager {
    // Uses NPT state machine
    // Three-state transitions
    // Full TLB flush on state change
};

// Runtime selection
IShadowHookManager* CreateHookManager() {
    if (g_vendor == VendorIntel) {
        return new IntelHookManager();
    } else {
        return new AmdHookManager();
    }
}

} // namespace Ombra::Hooks
```

---

## Critical Values & Constants

| Name | Value | Purpose |
|------|-------|---------|
| `INT3_OPCODE` | 0xCC | Breakpoint instruction |
| `JMP_ABS_SIZE` | 14 | ff 25 + 8-byte address |
| `MAX_STOLEN_BYTES` | 16 | Maximum prologue size |
| `PAGE_SIZE` | 0x1000 | Shadow page size |

---

## Common Pitfalls

### 1. Not Reusing Shadow Pages

**What goes wrong**: Memory exhaustion when hooking multiple functions on same page.
- **Both**: Check for existing hooks on page before allocating new shadows.

### 2. AMD: Forgetting State Machine Transitions

**What goes wrong**: Hooks don't fire or cause infinite loops.
- **AMD**: Must properly transition between states 1 and 2 based on which page is being executed.

### 3. Intel: Not Saving Hook Info for MTF

**What goes wrong**: MTF handler doesn't know which hook to restore.
- **Intel**: Must save `last_hook` per-CPU before enabling MTF.

### 4. RIP-Relative Instruction in Stolen Bytes

**What goes wrong**: Trampoline references wrong memory location.
- **Both**: Detect and fix up RIP-relative instructions, or avoid hooking such functions.

### 5. Cross-Page Hooks

**What goes wrong**: Hook spans page boundary, second page not shadowed.
- **Both**: If stolen bytes cross 4KB boundary, must hook both pages.

---

## Cross-References

- **01-SECOND-LEVEL-PAGING-FUNDAMENTALS.md**: EPT/NPT permission bits
- **02-PAGE-TABLE-CONSTRUCTION.md**: Large page splitting for hooks
- **04-VIOLATION-HANDLING.md**: EPT violation / NPT fault handlers
- **05-TLB-MANAGEMENT.md**: TLB flush after page modifications

---

*This document synthesizes shadow hook techniques from DdiMon, SimpleSvmHook, Sputnik, and HyperHide reference implementations.*
