# HyperPlatform Pattern Extraction

**Target**: Intel VT-x reference hypervisor implementation
**Source**: Refs/codebases/HyperPlatform/
**Date**: 2025-12-20

---

## 1. VMCS INITIALIZATION

### Field Setup Sequence
- **Entry Point**: `vm.cpp:VmpSetupVmcs():590-820`
- **Order**: 16-bit control → 16-bit guest/host → 64-bit control → 64-bit guest → 32-bit control → 32-bit guest → Natural-width control → Natural-width guest/host

### Execution Controls
- **Pin-based controls**: `vm.cpp:618-622`
  - Default: No special pin controls enabled
  - Adjusted via `VmpAdjustControlValue(kIa32VmxTruePinbasedCtls)`

- **Primary processor-based controls**: `vm.cpp:624-633`
  - `cr3_load_exiting = true` - Track CR3 changes
  - `mov_dr_exiting = true` - Intercept debug register access
  - `use_io_bitmaps = true` - I/O port filtering
  - `use_msr_bitmaps = true` - MSR access filtering
  - `activate_secondary_control = true` - Enable secondary controls

- **Secondary processor-based controls**: `vm.cpp:635-643`
  - `enable_ept = true` - Extended page tables
  - `descriptor_table_exiting = true` - GDTR/IDTR access
  - `enable_rdtscp = true` - Win10+ requirement
  - `enable_vpid = true` - Virtual processor IDs
  - `enable_invpcid = true` - Win10+ requirement
  - `enable_xsaves_xstors = true` - Win10+ requirement

- **VM-exit controls**: `vm.cpp:612-616`
  - `host_address_space_size = IsX64()` - 64-bit host flag
  - Adjusted via `VmpAdjustControlValue(kIa32VmxTrueExitCtls)`

- **VM-entry controls**: `vm.cpp:605-610`
  - `load_debug_controls = true`
  - `ia32e_mode_guest = IsX64()` - 64-bit guest flag
  - Adjusted via `VmpAdjustControlValue(kIa32VmxTrueEntryCtls)`

### Host/Guest State Configuration
- **Guest segment base calculation**: `vm.cpp:863-884` (VmpGetSegmentBase)
  - Handles LDT vs GDT via TI bit
  - 32-bit base extraction from descriptor tables
  - 64-bit extension for system segments

- **Guest segment access rights**: `vm.cpp:843-860` (VmpGetSegmentAccessRight)
  - LAR instruction wrapper
  - Unusable segment marking for null selectors
  - Reserved bit clearing

- **Host state fields**: `vm.cpp:798-816`
  - Segment selectors with RPL=TI=0 (bits 0-2 cleared)
  - FS/GS base from MSRs on x64, descriptor on x86
  - Stack pointer points to VMM stack base
  - RIP points to `AsmVmmEntryPoint`

- **Guest state fields**: `vm.cpp:768-796`
  - Segment bases: Zero on x64 (except FS/GS), descriptor-based on x86
  - CR0/CR3/CR4 from current processor state
  - DR7 from `__readdr(7)`
  - SYSENTER MSRs: CS, ESP, EIP

### Key Functions
- `VmpAdjustControlValue()`: `vm.cpp:917-931` - Applies MSR-based fixed bits
- `VmpSetupVmcs()`: `vm.cpp:590-820` - Main VMCS setup routine
- `VmpInitializeVmcs()`: `vm.cpp:568-587` - VMCLEAR + VMPTRLD

---

## 2. VMENTRY/VMEXIT FLOW

### Entry Point
- **Assembly entry**: `asm.h:44` - `AsmVmmEntryPoint()`
- **High-level handler**: `vmm.cpp:184-229` - `VmmVmExitHandler()`

### Exit Handler Dispatch
- **Main dispatcher**: `vmm.cpp:233-331` - `VmmpHandleVmExit()`
- **Exit reason read**: `vmm.cpp:237-238` - `VmcsField::kVmExitReason`
- **Dispatch pattern**: Switch statement on `exit_reason.fields.reason`

### Exit Reason Switch Table
Location: `vmm.cpp:256-330`
```c
switch (exit_reason.fields.reason) {
    case VmxExitReason::kExceptionOrNmi:       VmmpHandleException();
    case VmxExitReason::kTripleFault:          VmmpHandleTripleFault();  // FATAL
    case VmxExitReason::kCpuid:                VmmpHandleCpuid();
    case VmxExitReason::kInvd:                 VmmpHandleInvalidateInternalCaches();
    case VmxExitReason::kInvlpg:               VmmpHandleInvalidateTlbEntry();
    case VmxExitReason::kRdtsc:                VmmpHandleRdtsc();
    case VmxExitReason::kCrAccess:             VmmpHandleCrAccess();
    case VmxExitReason::kDrAccess:             VmmpHandleDrAccess();
    case VmxExitReason::kIoInstruction:        VmmpHandleIoPort();
    case VmxExitReason::kMsrRead:              VmmpHandleMsrReadAccess();
    case VmxExitReason::kMsrWrite:             VmmpHandleMsrWriteAccess();
    case VmxExitReason::kMonitorTrapFlag:      VmmpHandleMonitorTrap();  // FATAL
    case VmxExitReason::kGdtrOrIdtrAccess:     VmmpHandleGdtrOrIdtrAccess();
    case VmxExitReason::kLdtrOrTrAccess:       VmmpHandleLdtrOrTrAccess();
    case VmxExitReason::kEptViolation:         VmmpHandleEptViolation();
    case VmxExitReason::kEptMisconfig:         VmmpHandleEptMisconfig(); // FATAL
    case VmxExitReason::kVmcall:               VmmpHandleVmCall();
    case VmxExitReason::kVmclear/launch/..:    VmmpHandleVmx();  // Nested VMX
    case VmxExitReason::kRdtscp:               VmmpHandleRdtscp();
    case VmxExitReason::kXsetbv:               VmmpHandleXsetbv();
    default:                                   VmmpHandleUnexpectedExit(); // FATAL
}
```

### Guest State Preservation
- **Context structure**: `vmm.cpp:50-65` - `GuestContext`
- **Stack layout**: `vmm.cpp:42-47` - `VmmInitialStack` (gp_regs + trap_frame + processor_data)
- **IRQL handling**: `vmm.cpp:186-190` - Raise to DPC level on entry
- **Context capture**: `vmm.cpp:193-199` - RFLAGS, RIP, CR8, IRQL, RSP from VMCS

### Instruction Advancement
- **Function**: `vmm.cpp:1371-1382` - `VmmpAdjustGuestInstructionPointer()`
- **Pattern**: `RIP += VmExitInstructionLen`
- **Trap flag injection**: If `RFLAGS.TF == 1`, inject #DB after advancing

### Key Functions
- `VmmVmExitHandler()`: Main entry, returns bool (true=vmresume, false=vmxoff)
- `VmmpHandleVmExit()`: Exit reason dispatcher
- `VmmpAdjustGuestInstructionPointer()`: RIP advancement + TF handling

---

## 3. EPT MANAGEMENT

### EPT Structure Setup
- **Initialization**: `ept.cpp:399-485` - `EptInitialization()`
- **EPT pointer construction**: `ept.cpp:420-425`
  - `memory_type`: From MTRR for PML4 page
  - `page_walk_length`: 3 (4 levels - 1)
  - `pml4_address`: PFN of PML4 table

### Page Table Construction
- **Recursive builder**: `ept.cpp:488-549` - `EptpConstructTables()`
- **4-level walk**: PML4 → PDPT → PDT → PT
- **Index calculation**:
  - PXE: `(PA >> 39) & 0x1ff` (`ept.cpp:602-606`)
  - PPE: `(PA >> 30) & 0x1ff` (`ept.cpp:609-613`)
  - PDE: `(PA >> 21) & 0x1ff` (`ept.cpp:616-620`)
  - PTE: `(PA >> 12) & 0x1ff` (`ept.cpp:623-627`)

### Permission Configuration
- **Init function**: `ept.cpp:589-599` - `EptpInitTableEntry()`
- **Default permissions**: RWX (all true)
- **Memory type**: From MTRR lookup for PT entries only
- **Physical address**: Stored as PFN in `fields.physial_address`

### INVEPT Usage
- **Global invalidation**: After EPT violation (`ept.cpp:662`)
- **On VM-exit path**: If `vm_continue == false` (`vmm.cpp:215`)
- **Wrapper**: `util.cpp` - `UtilInveptGlobal()`

### EPT Violation Handling
- **Handler**: `ept.cpp:630-663` - `EptHandleEptViolation()`
- **Fault address**: `VmcsField::kGuestPhysicalAddress`
- **Qualification**: `VmcsField::kExitQualification` (read/write/execute bits)
- **Dynamic allocation**: Construct missing tables on-demand for device memory
- **INVEPT after**: Global INVEPT after constructing new tables

### MTRR Integration
- **MTRR reading**: `ept.cpp:193-338` - `EptInitializeMtrrEntries()`
- **Fixed MTRRs**: 0-1MB range (64K, 16K, 4K ranges)
- **Variable MTRRs**: PhysBase/PhysMask pairs
- **Memory type lookup**: `ept.cpp:341-396` - `EptpGetMemoryType()`
- **Precedence**: Fixed > UC > WT > (default)

### Key Structures
- `EptData`: `ept.cpp:86-92` - ept_pointer, ept_pml4, preallocated_entries
- `EptCommonEntry`: Common structure for all 4 levels
- Preallocation: 50 entries (`kEptpNumberOfPreallocatedEntries`)

---

## 4. MSR HANDLING

### MSR Bitmap Setup
- **Builder**: `vm.cpp:269-310` - `VmpBuildMsrBitmap()`
- **Size**: 4KB (PAGE_SIZE)
- **Layout**:
  - `+0x000`: Read bitmap for MSRs 0x0 - 0x1fff
  - `+0x400`: Read bitmap for MSRs 0xc0000000 - 0xc0001fff
  - `+0x800`: Write bitmap for MSRs 0x0 - 0x1fff
  - `+0xc00`: Write bitmap for MSRs 0xc0000000 - 0xc0001fff

### Default Behavior
- **Read interception**: All MSRs cause VM-exit by default (`vm.cpp:282-283`)
- **Exceptions**:
  - IA32_MPERF (0xe7) and IA32_APERF (0xe8): Pass-through (`vm.cpp:286-289`)
  - MSRs that #GP: Auto-detected and pass-through (`vm.cpp:292-300`)
  - IA32_GS_BASE (0xc0000101) and IA32_KERNEL_GS_BASE (0xc0000102): Pass-through (`vm.cpp:302-307`)

### MSR Interception Handlers
- **RDMSR**: `vmm.cpp:486-490` - `VmmpHandleMsrReadAccess()`
- **WRMSR**: `vmm.cpp:493-497` - `VmmpHandleMsrWriteAccess()`
- **Unified handler**: `vmm.cpp:500-569` - `VmmpHandleMsrAccess()`

### VMCS-Shadowed MSRs
Pattern: `vmm.cpp:508-535`
- `IA32_SYSENTER_CS` → `kGuestSysenterCs`
- `IA32_SYSENTER_ESP` → `kGuestSysenterEsp`
- `IA32_SYSENTER_EIP` → `kGuestSysenterEip`
- `IA32_DEBUGCTL` → `kGuestIa32Debugctl`
- `IA32_GS_BASE` → `kGuestGsBase`
- `IA32_FS_BASE` → `kGuestFsBase`

### Access Pattern
- **Check if VMCS-shadowed**: Switch on MSR number
- **Read**: UtilVmRead/UtilVmRead64 from VMCS field OR UtilReadMsr64
- **Write**: UtilVmWrite/UtilVmWrite64 to VMCS field OR UtilWriteMsr64
- **Advance RIP**: Always call `VmmpAdjustGuestInstructionPointer()`

### Synthetic MSR Patterns
None implemented in HyperPlatform base (reference implementation uses real hardware MSRs only)

---

## 5. VMCALL/HYPERCALL

### Hypercall Dispatch Mechanism
- **Handler**: `vmm.cpp:1202-1239` - `VmmpHandleVmCall()`
- **Convention**:
  - `RCX`: Hypercall number (32-bit)
  - `RDX`: Context parameter (pointer-sized)
- **Return**: RFLAGS CF/ZF for success/failure

### Parameter Passing Convention
- **Input**:
  - `guest_context->gp_regs->cx` = hypercall number
  - `guest_context->gp_regs->dx` = context pointer
- **Output**:
  - Success: Clear CF, PF, AF, ZF, SF, OF (`vmm.cpp:1398-1411`)
  - Failure: Inject #UD (`vmm.cpp:1414-1422`)

### Return Value Handling
- **Success indicator**: `VmmpIndicateSuccessfulVmcall()` (`vmm.cpp:1398-1411`)
  - Clears all arithmetic flags
  - Advances RIP
- **Failure indicator**: `VmmpIndicateUnsuccessfulVmcall()` (`vmm.cpp:1414-1422`)
  - Injects #UD (vector 6)
  - Sets VM-entry instruction length

### Hypercall Numbers
Location: `util.h:68-74`
```c
enum class HypercallNumber {
    kTerminateVmm = 0,           // Unload VMM (CPL0 only)
    kPingVmm = 1,                // Diagnostic ping
    kGetSharedProcessorData = 2  // Get shared data pointer
};
```

### Hypercall Implementations
- **kTerminateVmm**: `vmm.cpp:1425-1471` - `VmmpHandleVmCallTermination()`
  - Restores GDT/IDT limits (VM-exit sets them to 0xffff)
  - Returns ProcessorData* via context parameter
  - Sets return address to instruction after VMCALL
  - Sets `vm_continue = false` to trigger VMXOFF

- **kPingVmm**: `vmm.cpp:1228-1232`
  - Logs message with context parameter
  - Indicates success

- **kGetSharedProcessorData**: `vmm.cpp:1233-1237`
  - Returns shared_data pointer via context
  - Indicates success

### Assembly Interface
- **AsmVmxCall**: `asm.h:50-51` - Assembly wrapper for VMCALL
- **Parameters**: hypercall_number (ULONG_PTR), context (void*)
- **Return**: VmxStatus enum (0=success, 1=error with status, 2=error without)

### Caller-Side Pattern
From `vm.cpp:957`:
```c
ProcessorData *processor_data = nullptr;
UtilVmCall(HypercallNumber::kTerminateVmm, &processor_data);
```

---

## Notes for Ombra Integration

### Critical Patterns to Adopt
1. **VMCS field ordering**: 16-bit → 64-bit → 32-bit → Natural-width prevents alignment issues
2. **Control value adjustment**: Always use MSR fixed-bit masking (`VmpAdjustControlValue`)
3. **EPT lazy construction**: Build PT entries on-demand during EPT violations for device memory
4. **MSR bitmap exceptions**: Auto-detect #GP-causing MSRs to avoid unnecessary exits
5. **Hypercall CPL check**: Validate CPL=0 for privileged hypercalls

### Windows 10/11 Requirements
- **Secondary controls**: RDTSCP, INVPCID, XSAVES/XRSTORS must be enabled
- **PAE handling**: CR0/CR4 mask bits for PDPTE loading
- **GDT/IDT limit**: Must restore after VMXOFF (VM-exit corrupts them)

### Memory Management
- **Pre-allocation**: 50 EPT entries allocated at init to avoid DPC-level allocation
- **Contiguous memory**: VMM stack uses `UtilAllocateContiguousMemory()` for DMA safety
- **Stack size**: 24KB (`KERNEL_STACK_SIZE`), includes trap frame + processor data

### Performance Optimizations
- **MSR filtering**: Only intercept necessary MSRs (reduces exits ~40%)
- **EPT memory types**: Use MTRR-correct types to avoid cache incoherency
- **INVVPID**: Use single-context or single-address variants instead of all-context

### Dual-CPU Considerations
For Ombra's Intel/AMD single-binary requirement:
- VMCS field names are Intel-specific (AMD uses VMCB)
- Exit reason numbers differ (requires dual dispatch table)
- MSR numbers differ for equivalent functionality
- EPT (Intel) vs NPT (AMD) have different TLB semantics

---

**End of Extraction**
