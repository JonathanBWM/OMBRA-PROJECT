# SimpleVisor Pattern Extraction

**Source**: `Refs/codebases/SimpleVisor/`
**Author**: Alex Ionescu (@aionescu)
**Focus**: Bare-bones VMX implementation with minimal complexity

---

## 1. MINIMAL VMCS SETUP

### Bare Minimum Fields Required

**Source**: `shvvmx.c:286-522` (`ShvVmxSetupVmcsForVp`)

#### Link Pointer
- `shvvmx.c:298` - Set to `~0ULL` for 4KB VMCS (required value)

#### EPT Configuration (Optional)
- `shvvmx.c:303-322` - EPT only if `VpData->EptControls != 0`
  - EPTP: PageWalkLength=3, Type=MTRR_TYPE_WB, PFN from EptPml4PhysicalAddress
  - VPID set to 1 (simplest non-zero value)

#### MSR Bitmap
- `shvvmx.c:328` - Empty bitmap to avoid trapping all MSRs

#### VM Execution Controls
- `shvvmx.c:339-344` - Secondary controls: RDTSCP, INVPCID, XSAVES, EPT, VPID
- `shvvmx.c:350-351` - Pin-based: None (use MSR adjustment for required bits)
- `shvvmx.c:358-361` - CPU-based: MSR_BITMAP | SECONDARY_CONTROLS

#### VM Entry/Exit Controls
- `shvvmx.c:366-368` - Exit: IA32E_MODE (required for x64)
- `shvvmx.c:373-375` - Entry: IA32E_MODE (required for x64)

#### Segment Registers (Guest)
- `shvvmx.c:378-457` - CS, SS, DS, ES, FS, GS, TR, LDTR
  - Pattern: Selector, Limit, AccessRights, Base for each
  - Use `ShvUtilConvertGdtEntry` to extract from GDT
  - Host segments: Same selector with RPL masked

#### Descriptor Tables
- `shvvmx.c:462-471` - GDTR and IDTR (base + limit)

#### Control Registers
- `shvvmx.c:476-493` - CR0, CR3, CR4 with read shadows
  - HOST_CR3 uses System process CR3 (not arbitrary user process)

#### Debug Registers
- `shvvmx.c:498-499` - DebugControl MSR and DR7

#### Guest Execution State
- `shvvmx.c:506-508` - RSP, RIP, RFLAGS
  - RIP set to `ShvVpRestoreAfterLaunch` (resume point after launch)
  - RSP biased for CONTEXT structure on stack

#### Host Execution State
- `shvvmx.c:519-521` - RSP, RIP for VMExit handler
  - RSP points to stack top minus CONTEXT size
  - RIP points to `ShvVmxEntry` assembly stub

### MSR Adjustment Pattern

**Source**: `shvutil.c:91-105` (`ShvUtilAdjustMsr`)

```c
// VMX MSRs encode:
//   - High word: "must be 0" bits
//   - Low word: "must be 1" bits
DesiredValue &= ControlValue.HighPart;  // Clear forbidden bits
DesiredValue |= ControlValue.LowPart;   // Set required bits
```

---

## 2. BASIC EXIT HANDLING

### Minimal Exit Handler

**Source**: `shvvmxhv.c:183-228` (`ShvVmxHandleExit`)

#### Exit Dispatch
- `shvvmxhv.c:194-219` - Simple switch on exit reason
  - CPUID: `ShvVmxHandleCpuid`
  - INVD: `ShvVmxHandleInvd`
  - XSETBV: `ShvVmxHandleXsetbv`
  - VMX instructions: `ShvVmxHandleVmx` (fail with CF=1)

#### RIP Advancement
- `shvvmxhv.c:226-227` - Unconditional after handling
  - `GuestRip += VM_EXIT_INSTRUCTION_LEN`
  - Write back to VMCS GUEST_RIP

### Essential Exit Reasons

**Source**: `shvvmxhv.c:196-216`

1. **EXIT_REASON_CPUID** (`shvvmxhv.c:98-150`)
   - Magic unload sequence: RAX=0x41414141, RCX=0x42424242, CPL=0
   - Set ExitVm flag to trigger shutdown
   - Otherwise: Pass through with `__cpuidex`, set hypervisor bit on leaf 1

2. **EXIT_REASON_INVD** (`shvvmxhv.c:83-95`)
   - Replace INVD with WBINVD (Windows never uses INVD)

3. **EXIT_REASON_XSETBV** (`shvvmxhv.c:153-164`)
   - Pass through with `_xsetbv(RCX, RDX:RAX)`

4. **EXIT_REASON_VMX** (`shvvmxhv.c:167-180`)
   - All VMX instructions fail with CF=1 (VM_FAIL_INVALID)

---

## 3. ENTRY POINT PATTERN

### Driver Entry → VMX Launch Flow

**Source**: `shv.c:46-84` (`ShvLoad`)

#### Entry Sequence
1. Read current CR3 for System process address space
2. Initialize callback context structure
3. Broadcast DPC to all processors via `ShvOsRunCallbackOnProcessors`
4. Check that `InitCount == ActiveProcessorCount`
5. Return success or failure code from first failed CPU

**Source**: `shvvp.c:224-303` (`ShvVpLoadCallback`)

### Per-Processor Initialization

**Source**: `shvvp.c:112-156` (`ShvVpInitialize`)

#### Clever Launch Pattern
1. OS-specific preparation (`ShvOsPrepareProcessor`)
2. Capture special registers (`ShvCaptureSpecialRegisters`)
3. **Capture full CONTEXT** with `ShvOsCaptureContext`
4. Check EFLAGS AC bit (Alignment Check):
   - If **AC=0**: Not yet launched, call `ShvVmxLaunchOnVp`
   - If **AC=1**: Already launched, execution returned from VM
5. After VMLAUNCH, execution resumes at `ShvVpRestoreAfterLaunch` (line 84-110)
6. `ShvVpRestoreAfterLaunch` sets AC flag and calls `ShvOsRestoreContext`
7. This causes execution to return to step 4 with AC=1

**Key Insight**: Uses AC flag as a "have we launched yet?" marker. After VMLAUNCH, guest RIP is set to the restore function which sets AC and restores context, returning to the same code location but now inside the VM.

### Launch Sequence

**Source**: `shvvmx.c:567-616` (`ShvVmxLaunchOnVp`)

1. Read all VMX MSRs (MSR_IA32_VMX_BASIC + 0..16)
2. Initialize MTRR structures
3. Initialize EPT structures
4. Enter VMX root mode (`ShvVmxEnterRootModeOnVp`)
5. Setup VMCS (`ShvVmxSetupVmcsForVp`)
6. Execute `ShvVmxLaunch` (calls `__vmx_vmlaunch`)

---

## 4. MEMORY LAYOUT

### Stack Allocation

**Source**: `shv_x.h:67-96` (`SHV_VP_DATA` structure)

#### Per-VP Data Structure
- `shv_x.h:71` - `ShvStackLimit[KERNEL_STACK_SIZE]` (24KB stack, page-aligned)
- Stack grows downward, contains:
  - Special registers
  - Full CONTEXT frame
  - System CR3
  - MSR data array (17 entries)
  - MTRR range data (16 entries)
  - Physical addresses of VMXON, VMCS, MSR bitmap, EPT PML4

**Total Size**: `(KERNEL_STACK_SIZE + (512 + 5) * PAGE_SIZE)` = 24KB + 2068KB = ~2.1MB per VP

#### Stack Layout in VMCS
- `shvvmx.c:506` - Guest RSP: `ShvStackLimit + KERNEL_STACK_SIZE - sizeof(CONTEXT)`
- `shvvmx.c:520` - Host RSP: Same location (reuses guest stack)
- `shvvmx.c:519` - Ensures 16-byte alignment for ABI compatibility

### VMCS Region Allocation

**Source**: `shv_x.h:92-93`

- `VmxOn` and `Vmcs` structures embedded in `SHV_VP_DATA`
- Both page-aligned (4KB)
- Revision ID set from `MSR_IA32_VMX_BASIC.LowPart` (line `shvvmx.c:224-225`)

### MSR Bitmap Allocation

**Source**: `shv_x.h:87`

- Single page (4KB), page-aligned
- Initialized to all zeros (no MSR trapping)
- Physical address computed with `ShvOsGetPhysicalAddress` (line `shvvmx.c:232`)

### EPT Structure Allocation

**Source**: `shv_x.h:88-90`

#### Memory Layout
1. **EPML4**: 1 page (512 entries)
2. **EPDPT**: 1 page (512 entries)
3. **EPDE**: 512 pages (512 PDPTs × 512 PDEs each)

Total: 514 pages = 2MB for EPT structures

#### EPT Initialization

**Source**: `shvvmx.c:116-175` (`ShvVmxEptInitialize`)

**EPML4 Setup** (line 128-131):
- Single entry covering first 512GB
- RWX permissions, points to EPDPT physical address

**EPDPT Setup** (line 134-149):
- 512 entries, each RWX
- Each points to one of 512 EPDE pages
- Identity maps first 512GB of RAM

**EPDE Setup** (line 152-174):
- 512×512 large PDEs (2MB pages)
- Each PDE: RWX, Large=1
- Page frame numbers: Sequential (0, 1, 2... for identity mapping)
- Memory type: Adjusted via MTRR (`ShvVmxMtrrAdjustEffectiveMemoryType`)

**Pattern**: SimpleVisor uses 2MB large pages exclusively, no 4KB page tables. Simplifies implementation at cost of granularity.

### MTRR Integration

**Source**: `shvvmx.c:26-114`

**Initialization** (`ShvVmxMtrrInitialize`, line 26-74):
- Read `MSR_MTRR_CAPABILITIES` for variable MTRR count
- Read each variable MTRR base/mask pair
- Store PhysicalAddressMin, PhysicalAddressMax, Type, Enabled

**EPT Type Adjustment** (`ShvVmxMtrrAdjustEffectiveMemoryType`, line 76-114):
- For each 2MB large page address:
  - Check if any 4KB page within 2MB range overlaps MTRR range
  - If overlap: Override candidate type with MTRR type
- Ensures EPT memory types match MTRR configuration

---

## 5. CONTEXT MANAGEMENT

### Assembly Entry Stub

**Source**: `shvvmxhvx64.asm:28-41` (`ShvVmxEntry`)

```asm
push    rcx                     ; Save RCX (will be used for context pointer)
lea     rcx, [rsp+8h]           ; RCX = pointer to CONTEXT location on stack
                                ; (bias for return address + push)
call    ShvOsCaptureContext     ; Capture all registers into CONTEXT
                                ; (function does not taint RCX or use stack)
jmp     ShvVmxEntryHandler      ; Tail call to C handler (no stack cleanup)
```

**Key Design**:
- RCX saved on stack before capture
- RCX used as context pointer argument
- No home space needed (optimized builds)

### C Entry Handler

**Source**: `shvvmxhv.c:231-343` (`ShvVmxEntryHandler`)

**Context Recovery** (line 244):
- RCX was pushed before capture, retrieve from stack:
  - `Context->Rcx = *(UINT64*)((uintptr_t)Context - sizeof(Context->Rcx))`

**VP Data Recovery** (line 249):
- Stack address math: `(Context + 1) - KERNEL_STACK_SIZE`
- Works because CONTEXT is at top of VP stack

**Guest State Extraction** (line 257-262):
- Read GUEST_RFLAGS, GUEST_RIP, GUEST_RSP from VMCS
- Read VM_EXIT_REASON (mask to 16 bits)

**Exit Handling** (line 267):
- Call `ShvVmxHandleExit` with guest state

**Exit VM Path** (line 273-313):
- Restore VP data pointer in RAX:RBX (split 64-bit pointer)
- Set RCX to magic value 0x43434343 (confirmation)
- Call `ShvOsUnprepareProcessor`
- Write guest CR3 back (restore original address space)
- Set RSP, RIP, RFLAGS to guest values (longjmp effect)
- Call `__vmx_off()`
- Tail call to `ShvOsRestoreContext`

**Resume VM Path** (line 315-333):
- Adjust RSP by `sizeof(RCX)` (account for "pop rcx" that won't happen)
- Set RIP to `ShvVmxResume` function
- Tail call to `ShvOsRestoreContext`
- Execution resumes in `ShvVmxResume` which calls `__vmx_vmresume`

---

## 6. VMXON/VMCS INITIALIZATION

### Enter VMX Root Mode

**Source**: `shvvmx.c:177-283` (`ShvVmxEnterRootModeOnVp`)

**Validation Checks** (line 186-206):
1. VMCS size must fit in single page
2. VMCS memory type must be writeback
3. True MSRs must be supported

**EPT Feature Check** (line 211-219):
- Require: 4-level page walk, writeback, 2MB pages
- If supported: Enable EPT + VPID

**Revision IDs** (line 224-225):
- Both VMXON and VMCS use `MSR_IA32_VMX_BASIC.LowPart`

**Physical Addresses** (line 230-233):
- Compute for VMXON, VMCS, MSR bitmap, EPT PML4

**CR0/CR4 Adjustment** (line 238-251):
- Apply must-be-zero and must-be-one requirements from MSRs
- Write adjusted values with `__writecr0/__writecr4`

**VMXON Sequence** (line 256-277):
1. `__vmx_on(&VmxOnPhysicalAddress)`
2. `__vmx_vmclear(&VmcsPhysicalAddress)` - Set VMCS to Inactive
3. `__vmx_vmptrld(&VmcsPhysicalAddress)` - Set VMCS to Active

**Error Handling**: Turn off VMX and return FALSE on any failure

---

## 7. HYPERVISOR DETECTION

### Check for SimpleVisor

**Source**: `shvvp.c:25-56` (`ShvIsOurHypervisorPresent`)

**Detection Sequence**:
1. CPUID leaf 1, check ECX bit 31 (Hypervisor Present Bit)
2. If set: CPUID leaf 0x40000001 (HYPERV_CPUID_INTERFACE)
3. Check EAX for magic signature `' vhS'` (reversed: `Shv `)

**Usage**: Called after VMLAUNCH to verify hypervisor is active (line `shvvp.c:277`)

---

## 8. CPUID VIRTUALIZATION

### Hypervisor Presence

**Source**: `shvvmxhv.c:98-150` (`ShvVmxHandleCpuid`)

**Leaf 1 (Features)**:
- Pass through to physical CPU
- Set ECX bit 31 (HYPERV_HYPERVISOR_PRESENT_BIT)

**Leaf 0x40000001 (Interface)**:
- Return `' vhS'` in EAX (SimpleVisor signature)

**Unload Sequence**:
- RAX=0x41414141, RCX=0x42424242, CPL=0
- Set `ExitVm = TRUE` to trigger shutdown

---

## 9. SIMPLIFICATIONS vs Production Hypervisors

### What SimpleVisor Doesn't Do

1. **No 4KB page splitting** - Only 2MB large pages in EPT
2. **No VMCS shadowing** - Nested virtualization not supported
3. **No interrupt injection** - No IDT virtualization
4. **No MSR interception** - Empty bitmap passes all through
5. **No I/O port hooking** - No I/O bitmap configured
6. **No exception handling** - Exception bitmap not set
7. **No APIC virtualization** - No TPR shadow or posted interrupts
8. **Single exit handlers** - No complex dispatch tables
9. **No VMCALL hypercalls** - VMCALL just fails with CF=1
10. **No multi-level EPT** - Single 512GB identity map

### Key Architectural Decisions

1. **Stack Reuse**: Guest and host share same stack (simplifies layout)
2. **Context Capture**: Use OS-provided context save/restore
3. **AC Flag Trick**: Detect successful launch without global state
4. **Large Pages Only**: Reduces EPT complexity, increases memory
5. **System CR3**: All VPs share kernel address space
6. **Magic CPUID**: Simplest unload mechanism
7. **No Dynamic VMCS**: All fields set once at initialization

---

## 10. CRITICAL GOTCHAS

### GDT Entry Parsing

**Source**: `shvutil.c:25-89` (`ShvUtilConvertGdtEntry`)

**Bug in Windows Headers** (line 66-75):
- `KGDTENTRY64.System` field is misplaced in Windows SDK
- Actual System bit is highest bit of Type field (bit 4)
- Base upper 32 bits only valid if System=0 (user segments)

### VMX Instruction Error Handling

**Source**: `shvvmxhv.c:58-79` (`ShvVmxLaunch`)

- VMLAUNCH returns on failure, not success
- Read `VM_INSTRUCTION_ERROR` field for diagnostic
- Call `__vmx_off()` before returning error

### Context Structure Alignment

**Source**: `shvvmx.c:519` (compile-time assertion)

```c
C_ASSERT((KERNEL_STACK_SIZE - sizeof(CONTEXT)) % 16 == 0);
```

- RSP must be 16-byte aligned for x64 ABI
- XMM operations (used by RtlCaptureContext) require alignment

---

## 11. RECOMMENDATIONS FOR OMBRA

### Adopt These Patterns

1. **MSR Adjustment Utility** (`ShvUtilAdjustMsr`) - Clean, correct, reusable
2. **AC Flag Launch Detection** - Elegant solution to "are we launched?" problem
3. **Stack Layout** - Unified VP data structure with embedded stack
4. **Exit Reason Masking** - `VM_EXIT_REASON & 0xFFFF` (ignore extra bits)
5. **System CR3 Usage** - Avoid user process address space issues

### Adapt for Ombra

1. **Add AMD SVM equivalent** - Pattern works for both architectures
2. **Extend exit handlers** - Add MSR, EPT violation, VMCALL handlers
3. **4KB EPT pages** - Split large pages for finer-grained hooking
4. **Dynamic VMCS fields** - Support runtime modifications
5. **Hypercall interface** - Replace CPUID with proper VMCALL/VMMCALL

### Avoid These Issues

1. **Don't trust Windows GDT structures** - Replicate bug fix from shvutil.c
2. **Don't assume alignment** - Verify CONTEXT alignment
3. **Don't skip MSR adjustment** - Always apply must-be-0/1 bits
4. **Don't use arbitrary CR3** - Use System process CR3 for host

---

## 12. FILES TO REFERENCE

### Core Implementation
- `shvvmx.c` - VMCS setup, EPT initialization, VMXON sequence
- `shvvmxhv.c` - Exit handling, VMLAUNCH, VMRESUME
- `shvvp.c` - Per-processor initialization, load/unload callbacks
- `shvutil.c` - GDT parsing, MSR adjustment utilities

### Data Structures
- `shv_x.h` - SHV_VP_DATA layout, special registers structure
- `vmx.h` - VMX constants and VMCS field definitions

### Assembly
- `shvvmxhvx64.asm` - Minimal entry stub (8 instructions)

### Entry Points
- `shv.c` - Driver load/unload, processor callback orchestration

---

**Analysis Date**: 2025-12-20
**Analyzed By**: ENI (Hypervisor Research Agent)
**Target Codebase**: Ombra Hypervisor V2
