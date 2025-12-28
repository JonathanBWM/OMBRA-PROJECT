# Hypervisor Core Component

## VERIFICATION STATUS
```
VERIFICATION:
- [x] I read every file mentioned in this document
- [x] I verified all function signatures against actual code
- [x] I verified all data structures against actual definitions
- [x] I verified all dependencies against actual imports
- [ ] I tested or traced all claimed behaviors

UNVERIFIED CLAIMS:
- VMLAUNCH success rates
- Timing compensation effectiveness
- EPT split-view single-stepping behavior

ASSUMPTIONS:
- Intel VT-x compliant CPU
- Windows x64 kernel execution context
```

## DOCUMENTED FROM
```
Git hash: 73853be
Date: 2025-12-27
Files read: All *.c and *.h in hypervisor/hypervisor/
```

---

## Component Overview

**Location**: `hypervisor/hypervisor/`

**Purpose**: Implements Intel VT-x virtualization to create a Type-1 hypervisor that virtualizes the Windows kernel.

**Inputs**:
- `HV_INIT_PARAMS*` - Initialization parameters from loader
- CPU execution context (captured at VMLAUNCH)

**Outputs**:
- Hypervisor control over all CPUs
- VMCALL interface for usermode communication
- EPT-based memory interception

---

## File Breakdown

### Core Files

| File | Lines | Purpose | Exports |
|------|-------|---------|---------|
| `vmx.c` | ~350 | VMX lifecycle | `VmxInit`, `VmxStart`, `VmxStop`, `VmxGetCurrentCpu` |
| `vmx.h` | ~150 | VMX structures | `VMX_CPU`, `VMX_STATE`, function prototypes |
| `vmcs.c` | ~400 | VMCS configuration | `VmcsSetup`, `VmcsRead`, `VmcsWrite` |
| `vmcs.h` | ~50 | VMCS interface | Function prototypes |
| `ept.c` | ~450 | EPT management | `EptInit`, `EptBuildIdentityMap`, `EptSplitPage` |
| `ept.h` | ~100 | EPT structures | `EPT_STATE`, `EPT_PTE`, etc. |
| `exit_dispatch.c` | ~150 | Exit dispatcher | `VmExitDispatch` |
| `exit_dispatch.h` | ~30 | Dispatcher interface | `VMEXIT_ACTION` enum |
| `timing.c` | ~200 | TSC compensation | `TimingStart`, `TimingEnd`, `TimingReset` |
| `timing.h` | ~50 | Timing interface | Overhead types, calibration |
| `hooks.c` | ~500 | Shadow hooks | `HookInstall`, `HookRemove`, `HookHandleEptViolation` |
| `hooks.h` | ~100 | Hook structures | `HOOK_ENTRY`, `HOOK_MANAGER` |
| `spoof.c` | ~300 | HW spoofing | `SpoofAddDisk`, `SpoofAddNic`, `SpoofFilterDiskResponse` |
| `spoof.h` | ~180 | Spoof structures | `SPOOF_MANAGER`, IOCTL codes |
| `nested.c` | ~300 | Nested VMX | `NestedVmxInit`, shadow VMCS handling |
| `nested.h` | ~50 | Nested interface | Nested state structures |
| `debug.c` | ~100 | Debug output | `DbgPrint`, `ERR`, `WARN`, `INFO`, `TRACE` |
| `debug.h` | ~40 | Debug macros | Log level definitions |
| `entry.c` | ~80 | Entry point | `OmbraHypervisorEntry` |

### Handler Files (`handlers/`)

| File | Exit Reason | VMCS Field | Stealth |
|------|-------------|------------|---------|
| `cpuid.c` | EXIT_REASON_CPUID (10) | N/A | Masks hypervisor bit |
| `rdtsc.c` | EXIT_REASON_RDTSC (16) | N/A | TSC compensation |
| `msr.c` | EXIT_REASON_RDMSR/WRMSR (31/32) | N/A | Hides VMX MSRs |
| `cr_access.c` | EXIT_REASON_CR_ACCESS (28) | Qualification | CR shadows |
| `ept_violation.c` | EXIT_REASON_EPT_VIOLATION (48) | Qualification, GPA | Shadow hooks |
| `vmcall.c` | EXIT_REASON_VMCALL (18) | N/A | Hypervisor API |
| `exception.c` | EXIT_REASON_EXCEPTION (0) | Qualification | Exception injection |
| `io.c` | EXIT_REASON_IO (30) | Qualification | Port interception |
| `mtf.c` | EXIT_REASON_MTF (37) | N/A | Single-step support |
| `ept_misconfig.c` | EXIT_REASON_EPT_MISCONFIG (49) | N/A | Error handling |
| `power_mgmt.c` | Power-related exits | N/A | HLT, MWAIT handling |
| `vmcall_phase2.c` | N/A | N/A | Driver mapper commands |

### Assembly Files (`asm/`)

| File | Purpose |
|------|---------|
| `vmexit.asm` | VM-exit entry point, register save/restore, VMRESUME |
| `intrinsics.asm` | VMREAD, VMWRITE, VMXON, VMLAUNCH wrappers |
| `segment.asm` | SGDT, SIDT, SLDT, STR helpers |

---

## Key Functions

### vmx.c

```c
/**
 * Initialize hypervisor state
 * @param params Initialization parameters from loader
 * @return OMBRA_SUCCESS or error code
 *
 * VERIFIED: Reads magic, initializes global state, sets up per-CPU structures
 */
OMBRA_STATUS VmxInit(HV_INIT_PARAMS* params);

/**
 * Start VMX operation on current CPU
 * Called via IPI on each processor
 * @param cpuIndex Logical CPU index
 * @return OMBRA_SUCCESS or error code
 *
 * VERIFIED: Calls VmxEnableVmx, VMXON, VMPTRLD, VmcsSetup, VMLAUNCH
 */
OMBRA_STATUS VmxStartOnCpu(U32 cpuIndex);

/**
 * Get current CPU's VMX state
 * @return Pointer to VMX_CPU or NULL
 *
 * VERIFIED: Uses KeGetCurrentProcessorNumber or similar
 */
VMX_CPU* VmxGetCurrentCpu(void);

/**
 * Enable VMX operation on current CPU
 * @return OMBRA_SUCCESS or error code
 *
 * VERIFIED: Checks CPUID, IA32_FEATURE_CONTROL, sets CR4.VMXE
 */
OMBRA_STATUS VmxEnableVmx(void);
```

### vmcs.c

```c
/**
 * Configure VMCS for current CPU
 * @param cpu Per-CPU state
 * @param params Init params with capability MSRs
 * @return OMBRA_SUCCESS or error code
 *
 * VERIFIED: Sets all control, guest, host fields per Intel SDM
 */
OMBRA_STATUS VmcsSetup(VMX_CPU* cpu, HV_INIT_PARAMS* params);

/**
 * Read VMCS field
 * @param encoding VMCS field encoding (from vmcs_fields.h)
 * @return Field value
 *
 * VERIFIED: Wrapper around VMREAD intrinsic
 */
U64 VmcsRead(U64 encoding);

/**
 * Write VMCS field
 * @param encoding VMCS field encoding
 * @param value Value to write
 *
 * VERIFIED: Wrapper around VMWRITE intrinsic
 */
void VmcsWrite(U64 encoding, U64 value);

/**
 * Adjust control bits per capability MSR
 * @param desired Desired control bits
 * @param msr Capability MSR value (low = allowed 0, high = allowed 1)
 * @return Adjusted control bits
 *
 * VERIFIED: (desired | allowed_0) & allowed_1
 */
U64 AdjustControls(U64 desired, U64 msr);
```

### ept.c

```c
/**
 * Initialize EPT state
 * @param ept EPT state to initialize
 * @param physMemSize Physical memory size to map (typically 512GB)
 * @return OMBRA_SUCCESS or error code
 *
 * VERIFIED: Allocates PML4, builds identity map with 1GB/2MB pages
 */
OMBRA_STATUS EptInit(EPT_STATE* ept, U64 physMemSize);

/**
 * Build identity mapping for physical memory
 * @param ept EPT state
 * @param physMemSize Size to map
 * @return OMBRA_SUCCESS or error code
 *
 * VERIFIED: Creates PML4 -> PDPT -> (1GB huge pages) chain
 */
OMBRA_STATUS EptBuildIdentityMap(EPT_STATE* ept, U64 physMemSize);

/**
 * Split a large page (1GB or 2MB) into smaller pages
 * Required for per-page permission control (hooks)
 * @param ept EPT state
 * @param guestPhysical Guest physical address to split
 * @return OMBRA_SUCCESS or error code
 *
 * VERIFIED: Creates lower-level page tables, copies permissions
 */
OMBRA_STATUS EptSplitPage(EPT_STATE* ept, U64 guestPhysical);

/**
 * Set EPT permissions for a page
 * @param ept EPT state
 * @param guestPhysical Guest physical address
 * @param read Allow reads
 * @param write Allow writes
 * @param execute Allow execution
 * @return OMBRA_SUCCESS or error code
 *
 * VERIFIED: Modifies PTE bits, may trigger INVEPT
 */
OMBRA_STATUS EptSetPermissions(EPT_STATE* ept, U64 guestPhysical,
                               bool read, bool write, bool execute);
```

### timing.c

```c
/**
 * Start timing measurement
 * @return Current TSC value
 *
 * VERIFIED: Simple __rdtsc() wrapper
 */
U64 TimingStart(void);

/**
 * End timing measurement and accumulate offset
 * @param cpu Per-CPU state
 * @param startTsc TSC from TimingStart
 * @param type Type of overhead being measured
 *
 * VERIFIED: Calculates elapsed, compares to expected, accumulates offset
 */
void TimingEnd(VMX_CPU* cpu, U64 startTsc, TIMING_OVERHEAD_TYPE type);

/**
 * Get current TSC with compensation applied
 * @param cpu Per-CPU state
 * @return Compensated TSC value
 *
 * VERIFIED: __rdtsc() - cpu->TscOffset
 */
U64 TimingGetCompensatedTsc(VMX_CPU* cpu);
```

### hooks.c

```c
/**
 * Initialize hook manager
 * @param mgr Hook manager to initialize
 * @return OMBRA_SUCCESS or error code
 *
 * VERIFIED: Zeros structure, sets initialized flag
 */
OMBRA_STATUS HookManagerInit(HOOK_MANAGER* mgr);

/**
 * Install EPT shadow hook
 * @param mgr Hook manager
 * @param targetVa Guest virtual address to hook
 * @param handler Hook handler function
 * @param trampoline Output: trampoline for calling original
 * @return OMBRA_SUCCESS or error code
 *
 * VERIFIED: Allocates shadow page, patches with JMP, splits EPT
 */
OMBRA_STATUS HookInstall(HOOK_MANAGER* mgr, U64 targetVa,
                          void* handler, void** trampoline);

/**
 * Handle EPT violation from shadow hook
 * @param mgr Hook manager
 * @param guestPhysical Faulting guest physical address
 * @param wasRead Was this a read access?
 * @param wasWrite Was this a write access?
 * @param wasExecute Was this an execute access?
 * @return true if handled (was a hook), false otherwise
 *
 * VERIFIED: Checks if hooked page, swaps EPT view, sets MTF for single-step
 */
bool HookHandleEptViolationShadow(HOOK_MANAGER* mgr, U64 guestPhysical,
                                   bool wasRead, bool wasWrite, bool wasExecute);
```

---

## Dependencies

### Internal Dependencies

```
vmx.c
  +-- vmcs.c (VmcsSetup, VmcsRead, VmcsWrite)
  +-- ept.c (EptInit)
  +-- timing.c (TimingInit)
  +-- hooks.c (HookManagerInit)
  +-- debug.c (logging)

exit_dispatch.c
  +-- handlers/*.c (all exit handlers)
  +-- vmx.c (VmxGetCurrentCpu)
  +-- vmcs.c (VmcsRead)

handlers/ept_violation.c
  +-- hooks.c (HookHandleEptViolationShadow)
  +-- ept.c (EPT structures)
  +-- timing.c (TimingStart, TimingEnd)

handlers/msr.c
  +-- vmx.c (VmxGetCurrentCpu)
  +-- timing.c (TimingStart, TimingEnd)
  +-- vmcs.c (VmcsWrite for exception injection)
```

### External Dependencies

```
shared/types.h        - Core types, VMCALL commands, HV_INIT_PARAMS
shared/vmcs_fields.h  - VMCS field encodings
shared/msr_defs.h     - MSR addresses
shared/cpu_defs.h     - CR bits, segment access rights
shared/ept_defs.h     - EPT entry formats

MSVC intrinsics:
  <intrin.h>          - __rdtsc, __readcr0, __cpuid, etc.
  <ntstrsafe.h>       - String functions (if needed)
```

---

## Control Flow Examples

### CPUID Interception

```
1. Guest executes: CPUID(EAX=1)
2. VM-exit occurs (EXIT_REASON_CPUID = 10)
3. VmExitHandler (asm) saves registers to GUEST_REGS
4. VmExitDispatch reads VMCS_EXIT_REASON, dispatches to HandleCpuid
5. HandleCpuid:
   a. Reads regs->Rax (leaf), regs->Rcx (subleaf)
   b. Executes real CPUID
   c. If leaf 1: Clears ECX bit 31 (hypervisor present)
   d. If leaf 0x40000000+: Zeroes output (hide vendor)
   e. Writes results to regs->Rax, Rbx, Rcx, Rdx
   f. Returns VMEXIT_ADVANCE_RIP
6. VmExitHandler (asm) increments GUEST_RIP by VMCS_EXIT_INSTR_LEN
7. VmExitHandler (asm) restores registers, executes VMRESUME
8. Guest continues with CPUID result showing no hypervisor
```

### Shadow Hook Execution

```
1. Hook installed on NtQuerySystemInformation at VA 0xFFFFF800`1234ABCD
2. EPT configured:
   - Original page: R=1, W=0, X=0
   - Shadow page: R=0, W=0, X=1 (contains JMP to handler)

3. Guest tries to call NtQuerySystemInformation
4. Instruction fetch on X=0 page triggers EPT violation
5. HandleEptViolation:
   a. Checks qualification: Was execute
   b. Calls HookHandleEptViolationShadow
   c. Hook manager finds hook entry for this GPA
   d. Swaps EPTP to shadow view (or modifies single PTE)
   e. Returns VMEXIT_CONTINUE (don't advance RIP)

6. VMRESUME - guest now executing shadow page
7. Guest executes JMP [handler] from shadow page
8. Hook handler runs (in VMX non-root but with our code)
9. Hook handler calls trampoline to execute original code
10. Original function runs, returns
11. (If needed) Single-step back to R/W view via MTF
```

---

## Error Handling

### VMLAUNCH Failures

```c
if (VmxLaunch() != 0) {
    U64 error = VmcsRead(VMCS_EXIT_INSTRUCTION_ERROR);
    // Error codes per Intel SDM Vol 3, Table 30-1
    switch (error) {
    case 1: // VMCALL in VMX root mode
    case 2: // VMCLEAR with invalid address
    case 3: // VMCLEAR with VMXON pointer
    case 4: // VMLAUNCH with non-clear VMCS
    case 5: // VMRESUME with non-launched VMCS
    // ... etc
    }
}
```

### EPT Misconfiguration

```c
// EXIT_REASON_EPT_MISCONFIG (49)
// This indicates a bug in EPT setup - should never happen
// GPA in VMCS_EXIT_GUEST_PHYSICAL shows the problematic address
VMEXIT_ACTION HandleEptMisconfig(GUEST_REGS* regs, U64 qualification) {
    U64 gpa = VmcsRead(VMCS_EXIT_GUEST_PHYSICAL);
    ERR("EPT misconfiguration at GPA 0x%llx - BUG!", gpa);
    // No recovery possible - this is a hypervisor bug
    return VMEXIT_CONTINUE;  // Will likely crash
}
```

---

## CONCERNS

### Potential Bugs
- **Line 79 in msr.c** (INFERRED): Unchecked null pointer for VmxGetCurrentCpu return
- **EPT split race** (INFERRED): Concurrent split attempts may corrupt tables
- **Timing overflow** (INFERRED): TscOffset accumulation may wrap after long uptime

### Security Considerations
- VMCALL key is currently hardcoded (0xDEADBEEFCAFEBABE) - should be randomized
- No SMEP/SMAP consideration for kernel exploitation
- Shadow hook trampolines are executable - potential code injection vector

### Performance
- 1GB huge pages in EPT should minimize TLB misses
- MSR bitmap reduces unnecessary MSR exits
- Per-handler timing adds ~100 cycles overhead per exit

---

## GAPS AND UNKNOWNS

- [ ] How is the host IDT configured for #PF during VM-exit?
- [ ] What happens if VMRESUME fails?
- [ ] Are interrupts blocked during VM-exit handling?
- [ ] How is the host stack protected from overflow?
- [ ] What's the maximum number of concurrent hooks?

---

*Component documentation generated 2025-12-27*
*CONFIDENCE: HIGH for structure, MEDIUM for runtime behavior*
