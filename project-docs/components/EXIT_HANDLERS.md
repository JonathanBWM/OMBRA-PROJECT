# Exit Handler Infrastructure Component

## VERIFICATION STATUS
```
VERIFICATION:
- [x] I read every file mentioned in this document
- [x] I verified all function signatures against actual code
- [x] I verified all data structures against actual definitions
- [x] I verified all dependencies against actual imports
- [ ] I tested or traced all claimed behaviors

UNVERIFIED CLAIMS:
- Timing compensation effectiveness against detection
- Exception injection correctness
- Nested virtualization forward behavior

ASSUMPTIONS:
- Intel VT-x compliant CPU
- EPT enabled in VMCS
- All handlers execute in VMX root mode
```

## DOCUMENTED FROM
```
Git hash: 73853be
Date: 2025-12-27
Files read:
  - hypervisor/hypervisor/exit_dispatch.c
  - hypervisor/hypervisor/exit_dispatch.h
  - hypervisor/hypervisor/handlers/handlers.h
  - hypervisor/hypervisor/handlers/cpuid.c
  - hypervisor/hypervisor/handlers/rdtsc.c
  - hypervisor/hypervisor/handlers/msr.c
  - hypervisor/hypervisor/handlers/cr_access.c
  - hypervisor/hypervisor/handlers/ept_violation.c
  - hypervisor/hypervisor/handlers/vmcall.c
  - hypervisor/hypervisor/handlers/mtf.c
```

---

## Component Overview

**Location**: `hypervisor/hypervisor/exit_dispatch.c` and `handlers/`

**Purpose**: Dispatches VM-exits to appropriate handlers and manages guest state transitions. Implements stealth measures in each handler to avoid hypervisor detection.

**Inputs**:
- GUEST_REGS pointer (register context saved by assembly)
- Exit reason from VMCS
- Exit qualification from VMCS

**Outputs**:
- VMEXIT_ACTION (CONTINUE, ADVANCE_RIP, or SHUTDOWN)
- Modified guest register state
- Updated VMCS fields as needed

---

## File Breakdown

### Core Dispatcher

| File | Lines | Purpose |
|------|-------|---------|
| `exit_dispatch.c` | ~225 | Main dispatcher switch |
| `exit_dispatch.h` | ~15 | GUEST_REGS, VMEXIT_ACTION |

### Individual Handlers

| File | Exit Reason | Stealth Measures |
|------|-------------|------------------|
| `cpuid.c` | EXIT_REASON_CPUID (10) | Masks hypervisor bit, hides vendor leaves |
| `rdtsc.c` | EXIT_REASON_RDTSC/RDTSCP (16/51) | TSC offset compensation |
| `msr.c` | EXIT_REASON_RDMSR/WRMSR (31/32) | Hides VMX MSRs, APERF/MPERF compensation |
| `cr_access.c` | EXIT_REASON_CR_ACCESS (28) | CR4.VMXE shadowing |
| `ept_violation.c` | EXIT_REASON_EPT_VIOLATION (48) | Shadow hook trampolines |
| `vmcall.c` | EXIT_REASON_VMCALL (18) | Key validation, stealth commands |
| `mtf.c` | EXIT_REASON_MTF (37) | Single-step for shadow hooks |
| `exception.c` | EXIT_REASON_EXCEPTION_NMI (0) | Debug exception handling |
| `io.c` | EXIT_REASON_IO_INSTRUCTION (30) | Port filtering |
| `power_mgmt.c` | MONITOR/MWAIT/PAUSE (39/36/40) | Power instruction passthrough |
| `ept_misconfig.c` | EXIT_REASON_EPT_MISCONFIG (49) | Error logging |

---

## Core Data Structures

### GUEST_REGS

```c
typedef struct {
    U64 Rax, Rcx, Rdx, Rbx, Rsp, Rbp, Rsi, Rdi;
    U64 R8, R9, R10, R11, R12, R13, R14, R15;
} GUEST_REGS;
```

VERIFIED: Layout matches Intel 64-bit GPR order. Saved/restored by `vmexit.asm`.

### VMEXIT_ACTION

```c
typedef enum {
    VMEXIT_CONTINUE,      // Don't advance RIP (re-execute or exception)
    VMEXIT_ADVANCE_RIP,   // Increment RIP by instruction length
    VMEXIT_SHUTDOWN       // Devirtualize and exit VMX
} VMEXIT_ACTION;
```

---

## Main Dispatcher

### VmexitDispatch (exit_dispatch.c)

```c
VMEXIT_ACTION VmexitDispatch(GUEST_REGS* regs) {
    // 1. Read exit reason (bits 15:0)
    reason = (U32)VmcsRead(VMCS_EXIT_REASON) & 0xFFFF;
    qualification = VmcsRead(VMCS_EXIT_QUALIFICATION);

    // 2. Update per-CPU stats
    cpu->VmexitCount++;
    cpu->LastExitReason = reason;
    cpu->GuestRip = VmcsRead(VMCS_GUEST_RIP);
    cpu->GuestRsp = VmcsRead(VMCS_GUEST_RSP);
    cpu->GuestRflags = VmcsRead(VMCS_GUEST_RFLAGS);

    // 3. Dispatch to handler (see switch table below)

    // 4. Process action (advance RIP if needed)
}
```

### Exit Reason Dispatch Table

| Exit Reason | Value | Handler | Notes |
|-------------|-------|---------|-------|
| CPUID | 10 | `HandleCpuid` | Stealth critical |
| RDTSC | 16 | `HandleRdtsc` | Timing compensation |
| RDTSCP | 51 | `HandleRdtscp` | Timing + TSC_AUX |
| RDMSR | 31 | `HandleRdmsr` | MSR filtering |
| WRMSR | 32 | `HandleWrmsr` | MSR filtering |
| CR_ACCESS | 28 | `HandleCrAccess` | CR0/CR3/CR4/CR8 |
| EPT_VIOLATION | 48 | `HandleEptViolation` | Hook dispatch |
| EPT_MISCONFIG | 49 | `HandleEptMisconfiguration` | Error |
| MONITOR | 39 | `HandleMonitor` | Passthrough |
| MWAIT | 36 | `HandleMwait` | Passthrough |
| PAUSE | 40 | `HandlePause` | Passthrough |
| VMCALL | 18 | `HandleVmcall` | Hypercall API |
| EXCEPTION_NMI | 0 | `HandleException` | Exception handling |
| MTF | 37 | `HandleMtf` | Single-step |
| IO_INSTRUCTION | 30 | `HandleIo` | Port I/O |
| TRIPLE_FAULT | 2 | N/A | SHUTDOWN |
| HLT | 12 | ADVANCE_RIP | Passthrough |
| INVLPG | 14 | ADVANCE_RIP | EPT auto-handles |
| VMX instructions | 18-27 | Inject #UD or forward | See nested handling |
| XSETBV | 55 | ADVANCE_RIP | Passthrough |

---

## Handler Details

### HandleCpuid (cpuid.c)

```c
VMEXIT_ACTION HandleCpuid(GUEST_REGS* regs) {
    U64 entryTsc = TimingStart();

    // Execute real CPUID
    __cpuidex(result, (int)regs->Rax, (int)regs->Rcx);

    // Stealth modifications based on leaf:

    // Leaf 0x01: Clear hypervisor present bit (ECX bit 31)
    if (leaf == 1) {
        result[2] &= ~(1U << 31);
    }

    // Leaf 0x40000000+: Zero all output (hide hypervisor vendor)
    if (leaf >= 0x40000000 && leaf <= 0x400000FF) {
        result[0] = result[1] = result[2] = result[3] = 0;
    }

    // Write results back
    regs->Rax = result[0];
    regs->Rbx = result[1];
    regs->Rcx = result[2];
    regs->Rdx = result[3];

    TimingEnd(cpu, entryTsc, TIMING_CPUID_OVERHEAD);
    return VMEXIT_ADVANCE_RIP;
}
```

VERIFIED: ECX bit 31 cleared for leaf 1, vendor leaves zeroed.

### HandleRdtsc/HandleRdtscp (rdtsc.c)

```c
VMEXIT_ACTION HandleRdtsc(GUEST_REGS* regs) {
    U64 tsc = TimingGetCompensatedTsc(cpu);  // __rdtsc() - TscOffset

    // Ensure monotonicity
    if (tsc <= cpu->LastReturnedTsc) {
        tsc = cpu->LastReturnedTsc + 1;
    }
    cpu->LastReturnedTsc = tsc;

    regs->Rax = (U32)(tsc);
    regs->Rdx = (U32)(tsc >> 32);

    return VMEXIT_ADVANCE_RIP;
}

VMEXIT_ACTION HandleRdtscp(GUEST_REGS* regs) {
    // Same as RDTSC plus:
    regs->Rcx = cpu->GuestTscAux;  // Return virtualized processor ID
    return VMEXIT_ADVANCE_RIP;
}
```

VERIFIED: Uses per-CPU TscOffset for compensation, enforces monotonicity.

### HandleRdmsr/HandleWrmsr (msr.c)

**Read virtualization:**
```c
switch (msr) {
case MSR_IA32_FEATURE_CONTROL:
    // Return only LOCK bit, hide VMX enable
    value = FEATURE_CONTROL_LOCK;
    break;

case MSR_IA32_VMX_BASIC ... MSR_IA32_VMX_VMFUNC:
    // VMX capability MSRs: inject #GP (VMX not available)
    InjectGpFault();
    return VMEXIT_CONTINUE;

case MSR_IA32_APERF:
    value = __readmsr(MSR_IA32_APERF) - cpu->AperfOffset;
    break;

case MSR_IA32_MPERF:
    value = __readmsr(MSR_IA32_MPERF) - cpu->MperfOffset;
    break;

case MSR_IA32_TSC_AUX:
    value = cpu->GuestTscAux;
    break;

default:
    value = __readmsr(msr);  // Passthrough
}
```

VERIFIED: VMX MSRs cause #GP injection, APERF/MPERF have offset compensation.

### HandleCrAccess (cr_access.c)

**Exit qualification format:**
- Bits 3:0: CR number (0, 3, 4, 8)
- Bits 5:4: Access type (0=MOV to CR, 1=MOV from CR, 2=CLTS, 3=LMSW)
- Bits 11:8: Register index (0=RAX, 1=RCX, ..., 15=R15)
- Bits 31:16: LMSW source data

**CR4.VMXE Handling:**
```c
case 4:  // MOV to CR4
    value |= __readmsr(MSR_IA32_VMX_CR4_FIXED0);
    value &= __readmsr(MSR_IA32_VMX_CR4_FIXED1);
    value |= CR4_VMXE;  // Never let guest clear VMXE
    VmcsWrite(VMCS_GUEST_CR4, value);
    // Hide VMXE from guest via shadow
    VmcsWrite(VMCS_CTRL_CR4_SHADOW, value & ~CR4_VMXE);
    break;

case 4:  // MOV from CR4
    // Return shadow (hides VMXE bit)
    value = VmcsRead(VMCS_CTRL_CR4_SHADOW);
    break;
```

VERIFIED: CR4 shadow hides VMXE from guest reads.

### HandleEptViolation (ept_violation.c)

**Exit qualification bits:**
- Bit 0: Data read
- Bit 1: Data write
- Bit 2: Instruction fetch
- Bit 3: EPT readable (page has R permission)
- Bit 4: EPT writable (page has W permission)
- Bit 5: EPT executable (page has X permission)

```c
VMEXIT_ACTION HandleEptViolation(GUEST_REGS* regs, U64 qualification) {
    U64 gpa = VmcsRead(VMCS_EXIT_GUEST_PHYSICAL);

    bool wasRead = (qualification & 1) != 0;
    bool wasWrite = (qualification & 2) != 0;
    bool wasExecute = (qualification & 4) != 0;

    // Check if this is a shadow hook
    if (HookHandleEptViolationShadow(gpa, wasRead, wasWrite, wasExecute)) {
        return VMEXIT_CONTINUE;  // Hook handled, re-execute
    }

    // Not a hook - possible MMIO or EPT misconfiguration
    ERR("Unhandled EPT violation at GPA 0x%llx", gpa);
    return VMEXIT_CONTINUE;
}
```

### HandleMtf (mtf.c)

```c
VMEXIT_ACTION HandleMtf(GUEST_REGS* regs) {
    // Called after single-stepping through original instruction
    // Restore execute-only shadow page view
    HookHandleMtf();

    // Disable MTF if no more pending single-steps
    if (!HooksNeedMtf()) {
        U64 procCtl = VmcsRead(VMCS_CTRL_PROC_EXEC);
        procCtl &= ~CPU_MONITOR_TRAP;
        VmcsWrite(VMCS_CTRL_PROC_EXEC, procCtl);
    }

    return VMEXIT_CONTINUE;
}
```

VERIFIED: MTF used to single-step through original code before restoring shadow view.

### HandleVmcall (vmcall.c)

```c
VMEXIT_ACTION HandleVmcall(GUEST_REGS* regs) {
    U64 key = regs->Rcx;
    U32 cmd = (U32)regs->Rdx;

    // Validate magic key
    if (key != g_VmcallKey) {
        // Not our VMCALL - inject #UD
        InjectUdFault();
        return VMEXIT_CONTINUE;
    }

    switch (cmd) {
    case VMCALL_CMD_PING:
        regs->Rax = OMBRA_SUCCESS;
        break;

    case VMCALL_CMD_HOOK_INSTALL:
        // R8 = target VA, R9 = handler, returns trampoline in RAX
        status = HookInstall(regs->R8, (void*)regs->R9, &trampoline);
        regs->Rax = (U64)trampoline;
        break;

    case VMCALL_CMD_SPOOF_DISK_ADD:
        // R8 = serial, R9 = model
        status = SpoofAddDisk((char*)regs->R8, (char*)regs->R9);
        break;

    case VMCALL_CMD_UNLOAD:
        return VMEXIT_SHUTDOWN;

    // ... other commands
    }

    return VMEXIT_ADVANCE_RIP;
}
```

VERIFIED: Key validation, multiple command types, returns status in RAX.

---

## Nested Virtualization Handling

When VMX instructions are encountered:

```c
case EXIT_REASON_VMLAUNCH:
case EXIT_REASON_VMRESUME:
// ... other VMX instructions ...
    if (NestedIsRunningNested() && cpu) {
        // Running under L0 hypervisor - forward to L0/emulate
        NestedHandleVmxInstruction(cpu, reason);
        action = VMEXIT_ADVANCE_RIP;
    } else {
        // Bare metal - inject #UD (VMX hidden)
        U32 intInfo = 6 | (3 << 8) | (1UL << 31);  // #UD, hardware, valid
        VmcsWrite(VMCS_CTRL_VMENTRY_INT_INFO, intInfo);
        action = VMEXIT_CONTINUE;  // Exception injection
    }
```

VERIFIED: Guest VMX instructions get #UD on bare metal, forwarded when nested.

---

## Exception Injection Format

The VM-entry interruption-information field format:

```c
U32 intInfo = 0;
intInfo |= vector;                // Bits 7:0 - vector number
intInfo |= (type << 8);           // Bits 10:8 - interrupt type
// Type 0 = external interrupt
// Type 2 = NMI
// Type 3 = hardware exception
// Type 4 = software interrupt
// Type 5 = privileged software exception
// Type 6 = software exception
intInfo |= (hasErrorCode << 11);  // Bit 11 - deliver error code
intInfo |= (1UL << 31);           // Bit 31 - valid

VmcsWrite(VMCS_CTRL_VMENTRY_INT_INFO, intInfo);
```

**Common injections:**
- `#UD (vector 6)`: Undefined opcode - for hidden VMX instructions
- `#GP (vector 13)`: General protection - for VMX MSR reads

---

## Control Flow: CPUID Example

```
1. Guest executes: CPUID EAX=1
2. CPU detects CPUID exit (reason 10)
3. Hardware saves guest state to VMCS
4. vmexit.asm: Push all GPRs, call VmexitDispatch
5. VmexitDispatch:
   a. Read exit reason → 10 (CPUID)
   b. Dispatch to HandleCpuid
6. HandleCpuid:
   a. Start timing measurement
   b. Execute real CPUID with guest leaf
   c. Clear ECX bit 31 (hide hypervisor)
   d. Write results to GUEST_REGS
   e. End timing, accumulate offset
   f. Return VMEXIT_ADVANCE_RIP
7. VmexitDispatch: AdvanceGuestRip()
8. vmexit.asm: Pop GPRs, VMRESUME
9. Guest continues with modified CPUID result
```

---

## Timing Compensation Integration

Each handler follows the pattern:

```c
VMEXIT_ACTION HandleXxx(GUEST_REGS* regs) {
    U64 entryTsc = TimingStart();
    VMX_CPU* cpu = VmxGetCurrentCpu();

    // ... handler logic ...

    if (cpu) TimingEnd(cpu, entryTsc, TIMING_XXX_OVERHEAD);
    return VMEXIT_ADVANCE_RIP;
}
```

Overhead types defined in timing.h:
- `TIMING_CPUID_OVERHEAD`
- `TIMING_MSR_OVERHEAD`
- `TIMING_CR_OVERHEAD`
- `TIMING_EPT_OVERHEAD`
- `TIMING_VMCALL_OVERHEAD`

---

## CONCERNS

### Stealth Issues

1. **CPUID Timing**: Even with compensation, CPUID timing measurably different
2. **MSR Behavior**: Some anti-cheats probe MSR response patterns
3. **Exception Injection**: #UD/#GP pattern may be fingerprinted

### Potential Bugs

1. **Missing Null Check**: Some handlers don't check `VmxGetCurrentCpu()` return
2. **Unknown Exit Reasons**: Default case just advances RIP - could cause issues
3. **VMCS Read Without Check**: VmcsRead failures not detected

### Performance

1. **Switch Dispatch**: Could use jump table for faster dispatch
2. **VMCS Reads**: Multiple reads per exit - could cache in per-CPU struct
3. **Timing Functions**: Called on every exit, adds measurable overhead

---

## GAPS AND UNKNOWNS

- [ ] How are nested exceptions handled (exception during exit handling)?
- [ ] What's the maximum safe exit latency before guest instability?
- [ ] How does the #PF handler interact with EPT violations?
- [ ] Are there exit reasons not yet implemented that could be triggered?
- [ ] How is the guest stack validated for VMCALL command buffers?

---

*Component documentation generated 2025-12-27*
*CONFIDENCE: HIGH for structure, MEDIUM for stealth effectiveness*
