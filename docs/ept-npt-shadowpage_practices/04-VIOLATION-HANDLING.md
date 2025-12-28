# Violation Handling

> **Purpose**: Handling EPT violations (Intel) and NPT faults (AMD) - the VM-exit events that make shadow page hooking work.

> **Implementation target**: OmbraPayload (Ring -1, C++ with restrictions, BOTH Intel & AMD)

---

## Overview

EPT violations and NPT faults are the mechanism that makes shadow page hooks work. When the guest accesses memory in a way that violates the configured permissions (e.g., reading an execute-only page), the CPU triggers a VM-exit. The hypervisor then handles this exit, potentially switching page mappings or injecting exceptions.

This document covers the detailed format of violation information on both vendors, the handling logic for shadow hooks, and the single-step mechanisms (MTF for Intel, alternatives for AMD) required to maintain view consistency.

---

## Intel EPT Violation

### Exit Reason

**VM-Exit Reason**: 48 (0x30) - `EXIT_REASON_EPT_VIOLATION`

EPT violations occur when:
- Guest accesses a page with insufficient permissions
- Guest accesses an address with EPT misconfiguration

### Exit Qualification

The Exit Qualification field (VMCS 0x6400) contains detailed information about the violation:

```cpp
union EptViolationQualification {
    struct {
        u64 read_access : 1;           // Bit 0: Access was a read
        u64 write_access : 1;          // Bit 1: Access was a write
        u64 execute_access : 1;        // Bit 2: Access was instruction fetch
        u64 ept_readable : 1;          // Bit 3: EPT entry allows read
        u64 ept_writable : 1;          // Bit 4: EPT entry allows write
        u64 ept_executable : 1;        // Bit 5: EPT entry allows execute
        u64 ept_exec_user : 1;         // Bit 6: EPT entry allows user-mode exec
        u64 gva_valid : 1;             // Bit 7: Guest linear address is valid
        u64 translation_fault : 1;     // Bit 8: Fault during translation
        u64 user_mode_linear : 1;      // Bit 9: User-mode linear address
        u64 rw_linear : 1;             // Bit 10: R/W page table entry
        u64 nx_linear : 1;             // Bit 11: NX page table entry
        u64 nmi_unblocking : 1;        // Bit 12: NMI unblocking due to IRET
        u64 shadow_stack : 1;          // Bit 13: Shadow stack access
        u64 supervisor_shadow : 1;     // Bit 14: Supervisor shadow stack
        u64 reserved : 49;             // Bits 15-63: Reserved
    };
    u64 value;
};
```

### Key VMCS Fields

| Field | Encoding | Description |
|-------|----------|-------------|
| Exit Qualification | 0x6400 | Violation details (above) |
| Guest Physical Address | 0x2400 | Faulting GPA |
| Guest Linear Address | 0x6402 | Faulting GVA (if bit 7 set) |
| Exit Reason | 0x4402 | Should be 48 |

### Reading Violation Information

```cpp
void HandleEptViolation() {
    // Read exit qualification
    EptViolationQualification qual;
    qual.value = VmcsRead64(VMCS_EXIT_QUALIFICATION);

    // Read faulting addresses
    u64 faulting_gpa = VmcsRead64(VMCS_GUEST_PHYSICAL_ADDRESS);
    u64 faulting_gva = qual.gva_valid ? VmcsRead64(VMCS_GUEST_LINEAR_ADDRESS) : 0;

    // Determine access type
    bool was_read = qual.read_access && !qual.write_access && !qual.execute_access;
    bool was_write = qual.write_access;
    bool was_execute = qual.execute_access;

    // Determine what was allowed
    bool allowed_read = qual.ept_readable;
    bool allowed_write = qual.ept_writable;
    bool allowed_execute = qual.ept_executable;

    // Handle based on access type and permissions
    HandleViolation(faulting_gpa, was_read, was_write, was_execute,
                   allowed_read, allowed_write, allowed_execute);
}
```

### Intel Violation Handling Flow

```cpp
void HandleEptViolation(VcpuContext* ctx) {
    EptViolationQualification qual;
    qual.value = VmcsRead64(VMCS_EXIT_QUALIFICATION);
    u64 gpa = VmcsRead64(VMCS_GUEST_PHYSICAL_ADDRESS);

    // Is this a hooked page?
    HookEntry* hook = FindHookByGpa(gpa);
    if (!hook) {
        // Not our page - could be MMIO or actual fault
        HandleUnexpectedViolation(ctx, gpa, qual);
        return;
    }

    // Execute-only page was read/written
    if (!qual.execute_access) {
        // Guest tried to read or write execute-only page
        // Switch to RW view (clean page)
        SwitchToRwView(ctx, hook);

        // Enable MTF for single-step
        EnableMtf(ctx);

        // Save hook info for MTF handler
        ctx->per_cpu->last_hook = hook;

        // Resume - guest will execute one instruction
        return;
    }

    // This shouldn't happen if EPT is configured correctly
    // Execute access on execute-only page should succeed
    DbgPrint("Unexpected execute violation on hooked page\n");
}

void SwitchToRwView(VcpuContext* ctx, HookEntry* hook) {
    EptPte* pte = GetEptPte(ctx->ept_state, hook->target_gpa);

    // Allow read/write, show clean page
    pte->read = 1;
    pte->write = 1;
    pte->execute = 0;  // Prevent re-entry via exec
    pte->pfn = hook->rw_shadow_hpa >> 12;

    InvalidateEptSingleContext(ctx->eptp);
}

void SwitchToExecView(VcpuContext* ctx, HookEntry* hook) {
    EptPte* pte = GetEptPte(ctx->ept_state, hook->target_gpa);

    // Execute-only, show hooked page
    pte->read = 0;
    pte->write = 0;
    pte->execute = 1;
    pte->pfn = hook->exec_shadow_hpa >> 12;

    InvalidateEptSingleContext(ctx->eptp);
}
```

---

## AMD NPT Fault

### Exit Code

**VM-Exit Code**: 0x400 - `VMEXIT_NPF` (Nested Page Fault)

NPT faults occur when:
- Guest accesses a page marked not present
- Guest accesses a page with insufficient permissions
- Guest violates NX bit on instruction fetch

### VMCB Fields

| Offset | Field | Description |
|--------|-------|-------------|
| 0x070 | EXITCODE | Should be 0x400 |
| 0x078 | EXITINFO1 | Error code (see below) |
| 0x080 | EXITINFO2 | Faulting GPA |

### Exit Info 1 (NPF Error Code)

```cpp
union NpfErrorCode {
    struct {
        u64 present : 1;          // Bit 0: Page was present (1=protection, 0=not present)
        u64 write : 1;            // Bit 1: Write access (0=read)
        u64 user : 1;             // Bit 2: User-mode access (always 1 for NPT!)
        u64 reserved_bit : 1;     // Bit 3: Reserved bit violation
        u64 execute : 1;          // Bit 4: Instruction fetch
        u64 reserved1 : 27;       // Bits 5-31: Reserved
        u64 final_phys_addr : 1;  // Bit 32: Final physical address (1=guest, 0=nested)
        u64 page_table_access : 1;// Bit 33: Fault during page table access
        u64 reserved2 : 30;       // Bits 34-63: Reserved
    };
    u64 value;
};
```

**Key difference from Intel**: AMD has separate `present` and `execute` bits, while Intel combines access type into single bit fields.

### Reading NPT Fault Information

```cpp
void HandleNptFault(VMCB* vmcb) {
    // Read from VMCB
    NpfErrorCode error_code;
    error_code.value = vmcb->control.exitinfo1;
    u64 faulting_gpa = vmcb->control.exitinfo2;

    // Determine access type
    bool was_execute = error_code.execute;
    bool was_write = error_code.write && !was_execute;
    bool was_read = !error_code.write && !was_execute;

    // Was page present?
    bool page_present = error_code.present;

    // Handle the fault
    if (!page_present) {
        // Page not present - might need to build sub-tables
        HandleNptMissingEntry(vmcb, faulting_gpa);
    } else {
        // Permission violation
        HandleNptPermissionViolation(vmcb, faulting_gpa, was_read, was_write, was_execute);
    }
}
```

### AMD State Machine Handling

```cpp
void HandleNptFault(VcpuContext* ctx, VMCB* vmcb) {
    NpfErrorCode error;
    error.value = vmcb->control.exitinfo1;
    u64 gpa = vmcb->control.exitinfo2;

    // Not-present fault - build sub-tables
    if (!error.present) {
        BuildNptSubTables(ctx->npt_state, gpa);
        return;
    }

    // Permission violation - must be execute attempt (NX bit set)
    if (!error.execute) {
        // Read/write fault on hooked page shouldn't happen
        // (hooked pages allow RW in state 1)
        HandleUnexpectedFault(ctx, gpa, error);
        return;
    }

    // Execute violation - transition state
    HookEntry* hook = FindHookByGpa(gpa);

    if (ctx->npt_state == NptHookEnabledInvisible) {
        if (hook) {
            // Entering hooked page - transition to visible state
            TransitionState1To2(ctx, hook);
        } else {
            // Execute on non-hooked page in invisible state
            // Shouldn't happen if state machine is correct
            HandleUnexpectedFault(ctx, gpa, error);
        }
    } else if (ctx->npt_state == NptHookEnabledVisible) {
        // Leaving current hooked page
        TransitionState2To1(ctx);

        // Check if entering another hooked page
        if (hook && hook != ctx->active_hook) {
            TransitionState1To2(ctx, hook);
        }
    }
}

void TransitionState1To2(VcpuContext* ctx, HookEntry* hook) {
    // Make all pages NX
    SetAllPagesNx(ctx->npt_state, true);

    // Switch hook page to exec shadow and allow execute
    NptPte* pte = GetNptPte(ctx->npt_state, hook->target_gpa);
    pte->pfn = hook->exec_shadow_hpa >> 12;
    pte->nx = 0;  // Allow execute

    // Update state
    ctx->active_hook = hook;
    ctx->npt_state = NptHookEnabledVisible;

    // Flush TLB
    vmcb->control.tlb_control = TLB_CONTROL_FLUSH_GUEST;
}

void TransitionState2To1(VcpuContext* ctx) {
    // Make all pages executable except hooked ones
    SetAllPagesNx(ctx->npt_state, false);

    // Set hooked pages back to NX
    for (u32 i = 0; i < g_hook_count; i++) {
        NptPte* pte = GetNptPte(ctx->npt_state, g_hooks[i].target_gpa);
        pte->nx = 1;
        pte->pfn = g_hooks[i].original_hpa >> 12;  // Back to original
    }

    // Update state
    ctx->active_hook = nullptr;
    ctx->npt_state = NptHookEnabledInvisible;

    // Flush TLB
    vmcb->control.tlb_control = TLB_CONTROL_FLUSH_GUEST;
}
```

---

## Single-Step Mechanisms

### Intel: Monitor Trap Flag (MTF)

MTF causes a VM-exit after exactly one guest instruction executes.

**Enabling MTF**:
```cpp
void EnableMtf(VcpuContext* ctx) {
    u64 proc_ctls = VmcsRead64(VMCS_PRIMARY_PROCESSOR_BASED_VM_EXEC_CONTROLS);
    proc_ctls |= (1 << 27);  // Monitor trap flag
    VmcsWrite64(VMCS_PRIMARY_PROCESSOR_BASED_VM_EXEC_CONTROLS, proc_ctls);
}

void DisableMtf(VcpuContext* ctx) {
    u64 proc_ctls = VmcsRead64(VMCS_PRIMARY_PROCESSOR_BASED_VM_EXEC_CONTROLS);
    proc_ctls &= ~(1 << 27);
    VmcsWrite64(VMCS_PRIMARY_PROCESSOR_BASED_VM_EXEC_CONTROLS, proc_ctls);
}
```

**MTF Exit Handling**:
```cpp
void HandleMtfExit(VcpuContext* ctx) {
    // Exit Reason: 37 (MTF)

    HookEntry* hook = ctx->per_cpu->last_hook;
    if (!hook) {
        // MTF without associated hook - shouldn't happen
        DisableMtf(ctx);
        return;
    }

    // Restore execute-only view
    SwitchToExecView(ctx, hook);

    // Disable MTF
    DisableMtf(ctx);

    // Clear saved state
    ctx->per_cpu->last_hook = nullptr;

    // Resume guest
}
```

### AMD: Single-Step Alternatives

AMD SVM does not have MTF. Alternatives:

**Option 1: RFLAGS.TF (Trap Flag)**

```cpp
void EnableSingleStep(VMCB* vmcb) {
    // Set trap flag in guest RFLAGS
    vmcb->state.rflags |= RFLAGS_TF;

    // Intercept #DB exception
    vmcb->control.intercept_exceptions |= (1 << 1);  // #DB
}

void HandleDebugException(VcpuContext* ctx, VMCB* vmcb) {
    // Check if this is our single-step
    if (ctx->awaiting_single_step) {
        // Clear trap flag
        vmcb->state.rflags &= ~RFLAGS_TF;

        // Restore hook state
        RestoreHookState(ctx);

        ctx->awaiting_single_step = false;
    } else {
        // Not ours - inject to guest
        InjectException(vmcb, EXCEPTION_DEBUG, 0);
    }
}
```

**Option 2: Instruction Emulation**

Instead of single-stepping, emulate the faulting instruction:

```cpp
void EmulateInstruction(VcpuContext* ctx, VMCB* vmcb, u64 gpa) {
    // Decode instruction at guest RIP
    u8 instr[15];
    ReadGuestMemory(vmcb->state.rip, instr, 15);

    ZydisDecodedInstruction decoded;
    if (!DecodeInstruction(instr, &decoded)) {
        // Can't decode - fall back to single-step
        EnableSingleStep(vmcb);
        return;
    }

    // For simple MOV instructions, emulate directly
    if (decoded.mnemonic == ZYDIS_MNEMONIC_MOV) {
        EmulateMove(ctx, vmcb, &decoded);
        vmcb->state.rip += decoded.length;
        return;
    }

    // Complex instruction - fall back to single-step
    EnableSingleStep(vmcb);
}
```

---

## Breakpoint Handling

Both Intel and AMD must handle INT3 (#BP) exceptions from breakpoint-style hooks.

### Intel #BP Handling

```cpp
void HandleExceptionExit(VcpuContext* ctx) {
    u32 interrupt_info = VmcsRead32(VMCS_VM_EXIT_INTERRUPTION_INFO);

    u8 vector = interrupt_info & 0xFF;
    bool valid = (interrupt_info >> 31) & 1;

    if (!valid) return;

    if (vector == EXCEPTION_BREAKPOINT) {
        u64 guest_rip = VmcsRead64(VMCS_GUEST_RIP);

        // Is this our breakpoint?
        HookEntry* hook = FindHookByRip(guest_rip);
        if (hook) {
            // Redirect to hook handler
            VmcsWrite64(VMCS_GUEST_RIP, (u64)hook->handler);
            return;
        }

        // Not ours - inject to guest
        InjectEvent(ctx, EXCEPTION_BREAKPOINT, INTERRUPT_TYPE_HARDWARE_EXCEPTION, 0, false);
    }
}
```

### AMD #BP Handling

```cpp
void HandleExceptionIntercept(VcpuContext* ctx, VMCB* vmcb) {
    u8 vector = vmcb->control.exitinfo1;

    if (vector == EXCEPTION_BREAKPOINT) {
        u64 guest_rip = vmcb->state.rip;

        HookEntry* hook = FindHookByRip(guest_rip);
        if (hook) {
            // Redirect to handler
            vmcb->state.rip = (u64)hook->handler;
            return;
        }

        // Not ours - inject to guest
        EventInject event = {};
        event.vector = EXCEPTION_BREAKPOINT;
        event.type = EVENT_TYPE_EXCEPTION;
        event.valid = 1;
        vmcb->control.eventinj = event.value;

        // Use NRIP for original RIP
        vmcb->state.rip = vmcb->control.nrip;
    }
}
```

---

## Unified Handler Pattern

```cpp
namespace Ombra::Violations {

struct ViolationInfo {
    u64 gpa;
    u64 gva;
    bool read_access;
    bool write_access;
    bool execute_access;
    bool page_present;
};

void HandleViolation(VcpuContext* ctx, const ViolationInfo& info) {
    HookEntry* hook = FindHookByGpa(info.gpa);

    if (!hook) {
        // Not a hooked page
        HandleUnexpectedViolation(ctx, info);
        return;
    }

    if (g_vendor == VendorIntel) {
        // Intel: Use MTF for single-step
        if (!info.execute_access) {
            // Read/write on execute-only page
            SwitchToRwView(ctx, hook);
            EnableMtf(ctx);
            ctx->per_cpu->last_hook = hook;
        }
    } else {
        // AMD: Use state machine
        if (info.execute_access) {
            // Execute violation - transition state
            TransitionNptState(ctx, hook);
        }
    }
}

// Called from vendor-specific exit handler
void OnEptViolation(VcpuContext* ctx) {
    ViolationInfo info;
    EptViolationQualification qual;
    qual.value = VmcsRead64(VMCS_EXIT_QUALIFICATION);

    info.gpa = VmcsRead64(VMCS_GUEST_PHYSICAL_ADDRESS);
    info.gva = qual.gva_valid ? VmcsRead64(VMCS_GUEST_LINEAR_ADDRESS) : 0;
    info.read_access = qual.read_access;
    info.write_access = qual.write_access;
    info.execute_access = qual.execute_access;
    info.page_present = true;  // EPT violation implies entry exists

    HandleViolation(ctx, info);
}

void OnNpfFault(VcpuContext* ctx, VMCB* vmcb) {
    ViolationInfo info;
    NpfErrorCode error;
    error.value = vmcb->control.exitinfo1;

    info.gpa = vmcb->control.exitinfo2;
    info.gva = 0;  // NPT doesn't provide GVA directly
    info.read_access = !error.write && !error.execute;
    info.write_access = error.write;
    info.execute_access = error.execute;
    info.page_present = error.present;

    if (!info.page_present) {
        BuildNptSubTables(ctx->npt_state, info.gpa);
        return;
    }

    HandleViolation(ctx, info);
}

} // namespace Ombra::Violations
```

---

## Critical Values & Constants

### Intel Exit Reasons

| Value | Name | Description |
|-------|------|-------------|
| 37 | MTF | Monitor Trap Flag |
| 48 | EPT_VIOLATION | EPT permission violation |
| 49 | EPT_MISCONFIGURATION | Invalid EPT entry |

### AMD Exit Codes

| Value | Name | Description |
|-------|------|-------------|
| 0x400 | VMEXIT_NPF | Nested Page Fault |
| 0x41 | VMEXIT_EXCEPTION_BP | Breakpoint exception (#BP) |

### VMCS Fields (Intel)

| Encoding | Name |
|----------|------|
| 0x4402 | VM_EXIT_REASON |
| 0x4404 | VM_EXIT_INTERRUPTION_INFO |
| 0x6400 | EXIT_QUALIFICATION |
| 0x2400 | GUEST_PHYSICAL_ADDRESS |
| 0x6402 | GUEST_LINEAR_ADDRESS |

### VMCB Offsets (AMD)

| Offset | Name |
|--------|------|
| 0x070 | EXITCODE |
| 0x078 | EXITINFO1 |
| 0x080 | EXITINFO2 |
| 0x0A8 | EVENTINJ |
| 0x0C8 | NRIP |

---

## Common Pitfalls

### 1. Not Checking Exit Reason Before Handling

**What goes wrong**: Code handles wrong exit type.
- **Both**: Always verify exit reason/code before processing.

### 2. Forgetting to Disable MTF

**What goes wrong**: Continuous MTF exits.
- **Intel**: ALWAYS disable MTF in handler before resuming.

### 3. AMD: Not Flushing TLB After State Transition

**What goes wrong**: Stale translations cause wrong pages.
- **AMD**: Set `tlb_control` in VMCB after every NPT modification.

### 4. Incorrect Exception Injection

**What goes wrong**: Guest sees wrong exception or crashes.
- **Both**: Carefully construct exception injection with correct type and error code.

### 5. Not Handling EPT Misconfiguration

**What goes wrong**: System hangs on invalid EPT entry.
- **Intel**: Exit reason 49 indicates bug in EPT setup - must debug and fix.

---

## Cross-References

- **01-SECOND-LEVEL-PAGING-FUNDAMENTALS.md**: EPT/NPT permission bits
- **03-SHADOW-PAGE-HOOKING.md**: Hook installation and shadow pages
- **05-TLB-MANAGEMENT.md**: TLB invalidation requirements

---

*This document synthesizes violation handling patterns from DdiMon, SimpleSvmHook, NoirVisor, and HyperPlatform reference implementations.*
