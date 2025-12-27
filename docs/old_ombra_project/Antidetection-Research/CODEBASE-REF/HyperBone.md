# HyperBone VMX Pattern Extraction

**Author**: Alex Ionescu (@aionescu)
**Focus**: EPT-based page hooking, VMCS setup, VMX initialization

---

## Initialization Patterns

### CPU Feature Detection
**File**: `src/Arch/Intel/VMX.c:37-63`
- CPUID check for VMX bit (ECX bit 5)
- MSR_IA32_FEATURE_CONTROL lock handling: if unlocked, enables VMXON and locks MSR
- Returns FALSE if BIOS locked VMX off
- **Key difference**: Writes MSR_IA32_FEATURE_CONTROL if not locked (line 54)

### Feature Capability Check
**File**: `src/Arch/Intel/VMX.c:68-106`
- Reads MSR_IA32_VMX_BASIC for true MSR support
- Checks secondary controls via MSR_IA32_VMX_PROCBASED_CTLS
- EPT/VPID detection from MSR_IA32_VMX_PROCBASED_CTLS2
- Execute-only EPT check from MSR_IA32_VMX_EPT_VPID_CAP (line 94-99)
- **Novel**: Stores all features in global `g_Data->Features` structure for runtime queries

### Context Capture Technique
**File**: `src/Arch/Intel/VMX.c:133-189`
- Uses `KeSaveStateForHibernate()` for special registers/MSRs (line 139)
- Calls `RtlCaptureContext()` for general purpose registers (line 149)
- **Three-state transition pattern** via `VmxState` enum:
  - `VMX_STATE_OFF` (0) → initial entry
  - `VMX_STATE_TRANSITION` (1) → VMLAUNCH executed
  - `VMX_STATE_ON` (2) → VM running, context restored
- Returns to same RIP after VMLAUNCH via captured context (line 173)

### VMXON/VMCS Memory Allocation
**File**: `src/Arch/Intel/VMX.c:220-261`
- Allocates contiguous physical memory with `MAXULONG64` constraint
- Three allocations per CPU:
  - VMXON region (sizeof VMX_VMCS)
  - VMCS region (sizeof VMX_VMCS)
  - VMM stack (KERNEL_STACK_SIZE = 24KB)
- `UtilProtectNonpagedMemory()` sets PAGE_READWRITE on all regions (line 254-256)
- Zero-fills all regions before use

### MSR Data Caching
**File**: `src/Arch/Intel/VMX.c:228-242`
- Reads all VMX MSRs into per-CPU `MsrData[]` array
- Range: MSR_IA32_VMX_BASIC through MSR_IA32_VMX_VMCS_ENUM
- Conditionally reads:
  - Secondary controls MSR if `g_Data->Features.SecondaryControls`
  - True MSRs (pin/proc/entry/exit) if `g_Data->Features.TrueMSRs`
  - VMFUNC MSR if `g_Data->Features.VMFUNC`
- **Purpose**: Avoid repeated RDMSR in VMCS setup

### VMXON Entry
**File**: `src/Arch/Intel/VMX.c:377-445`
- Validates VMCS region size ≤ PAGE_SIZE (line 383-387)
- Validates memory type == VMX_MEM_TYPE_WRITEBACK (line 390-394)
- Requires true MSR support (line 397-401)
- Sets VMXON/VMCS revision IDs from MSR_IA32_VMX_BASIC
- Adjusts CR0/CR4 based on fixed0/fixed1 MSRs (line 408-417)
- Sequence: `__vmx_on()` → `__vmx_vmclear()` → `__vmx_vmptrld()` (line 421-441)

---

## VMCS Setup Patterns

### Control Field Configuration
**File**: `src/Arch/Intel/VMX.c:451-510`
- Uses `VmxpAdjustMsr()` helper to apply must-be-0/must-be-1 bits
- Pin-based controls: minimal (no interrupts, no NMI exiting)
- Primary controls (line 472-479):
  - `UseMSRBitmaps = TRUE`
  - `ActivateSecondaryControl = TRUE`
  - `CR3LoadExiting = TRUE` if VPID enabled (for TLB invalidation)
- Secondary controls (line 485-486):
  - `EnableRDTSCP = TRUE`
  - `EnableXSAVESXSTORS = TRUE`
- Exit controls (line 466-467):
  - `AcknowledgeInterruptOnExit = TRUE`
  - `HostAddressSpaceSize = TRUE` (x64 mode)
- Entry controls (line 462):
  - `IA32eModeGuest = TRUE` (enter in x64)

### MSR Bitmap Setup
**File**: `src/Arch/Intel/VMX.c:512-530`
- 4KB bitmap divided into 4 regions (read low/high, write low/high)
- Intercepts specific MSRs by setting bits:
  - MSR_IA32_FEATURE_CONTROL (line 522)
  - MSR_IA32_DEBUGCTL (line 523)
  - MSR_LSTAR (line 524)
  - All VMX MSRs (MSR_IA32_VMX_BASIC through MSR_IA32_VMX_VMFUNC)
- Uses `RtlInitializeBitMap()` and `RtlSetBit()` for manipulation
- Physical address written to MSR_BITMAP VMCS field (line 530)

### Exception Bitmap
**File**: `src/Arch/Intel/VMX.c:532-537`
- Intercepts only `VECTOR_BREAKPOINT_EXCEPTION` (INT3) by default
- Debug exception commented out (line 534)
- Written to EXCEPTION_BITMAP VMCS field

### Segment Descriptor Conversion
**File**: `src/Arch/Intel/VMX.c:320-354`
- `VmxpConvertGdtEntry()` helper reads GDT and converts to VMX format
- Extracts base, limit, access rights from KGDTENTRY64
- Uses `__segmentlimit()` intrinsic for segment limit (line 334)
- **Handles 64-bit base calculation** for system descriptors (line 343-344)
- Sets `Unusable` bit if segment not present (line 353)

### Guest Segment State
**File**: `src/Arch/Intel/VMX.c:539-603`
- Writes selector, base, limit, access rights for:
  - CS, SS, DS, ES, FS, GS (all segments)
  - TR (Task Register)
  - LDTR (Local Descriptor Table Register)
- **GS base special handling**: uses `MsrGsBase` from special registers (line 585-586)
- GDTR/IDTR written directly from captured state (line 606-613)

### Control Register State
**File**: `src/Arch/Intel/VMX.c:615-630`
- CR0: writes to GUEST_CR0, HOST_CR0, CR0_READ_SHADOW
- CR3:
  - Host uses `SystemDirectoryTableBase` (kernel CR3, not current process)
  - Guest uses captured CR3
- CR4:
  - GUEST_CR4, HOST_CR4 from captured state
  - `CR4_GUEST_HOST_MASK = 0x2000` (VMXE bit mask)
  - `CR4_READ_SHADOW` masks out VMXE bit (line 630)

### Guest Entry Point
**File**: `src/Arch/Intel/VMX.c:636-653`
- GUEST_RSP/RIP/RFLAGS from `ContextFrame` (return to RtlCaptureContext)
- Host entry:
  - HOST_RIP = `VmxVMEntry` (ASM exit handler)
  - HOST_RSP = top of VMM stack minus sizeof(CONTEXT)
  - Stack pre-biased for CONTEXT push in exit handler
  - **16-byte alignment enforced** for XMM operations (line 650)

---

## VMExit Handling

### Exit Dispatcher Table
**File**: `src/Arch/Intel/VmxExitHandlers.c:54-121`
- Array of 65 function pointers (`g_ExitHandler[]`)
- Indexed by exit reason (0-64)
- Notable handlers:
  - `VmExitCPUID` (reason 10)
  - `VmExitVmCall` (reason 18)
  - `VmExitCR` (reason 28)
  - `VmExitMSRRead/Write` (reasons 31-32)
  - `VmExitMTF` (reason 37) - Monitor Trap Flag for single-stepping
  - `VmExitEptViolation` (reason 48)
  - `VmExitEptMisconfig` (reason 49)
- All VMX instructions (VMREAD, VMWRITE, etc.) inject #UD (line 82-83, 277)

### Exit Entry Point
**File**: `src/Arch/Intel/VmxExitHandlers.c:138-182`
- `VmxpExitHandler()` called from `VmxVMEntry` ASM stub
- Raises IRQL to HIGH_LEVEL (line 142)
- Reconstructs RCX from stack (line 144) - saved by ASM stub
- Reads VMCS fields into `GUEST_STATE` structure:
  - GUEST_RFLAGS, GUEST_RIP, GUEST_RSP
  - VM_EXIT_REASON (masked to 16 bits)
  - EXIT_QUALIFICATION
  - GUEST_LINEAR_ADDRESS, GUEST_PHYSICAL_ADDRESS
- Dispatches via table: `g_ExitHandler[ExitReason]()`
- **Unload path**: if `ExitPending == TRUE`, restores GDTR/IDTR/CR3 and calls `__vmx_off()` (line 161-173)
- Otherwise: adjusts RSP and jumps to `VmxpResume` ASM stub (line 176-177)

### CPUID Spoofing
**File**: `src/Arch/Intel/VmxExitHandlers.c:213-224`
- Executes real CPUID via `__cpuidex()`
- Returns unmodified values to guest (RAX/RBX/RCX/RDX)
- Advances RIP via `VmxpAdvanceEIP()`
- **No hypervisor hiding implemented**

### VMCALL Hypercalls
**File**: `src/Arch/Intel/VmxExitHandlers.c:284-331`
- Hypercall number from `RCX & 0xFFFF`
- Supported calls:
  - `HYPERCALL_UNLOAD`: sets `ExitPending = TRUE` (line 292)
  - `HYPERCALL_HOOK_LSTAR`: saves original MSR_LSTAR, writes new value (line 297-298)
  - `HYPERCALL_UNHOOK_LSTAR`: restores original MSR_LSTAR (line 303)
  - `HYPERCALL_HOOK_PAGE`: calls `EptUpdateTableRecursive()`, invalidates EPT (line 308-313)
  - `HYPERCALL_UNHOOK_PAGE`: restores EPT mapping (line 317-322)

### CR Access Handling
**File**: `src/Arch/Intel/VmxExitHandlers.c:337-398`
- Parses `EXIT_QUALIFICATION` into `MOV_CR_QUALIFICATION` structure
- Calculates register pointer: `&GpRegs->Rax + Register field`
- MOV to CR: writes GUEST_CR0/3/4 and shadow fields
- **CR3 special handling**: invalidates all VPID contexts if VPID enabled (line 355-356)
- MOV from CR: reads GUEST_CR0/3/4 into target register

### MSR Virtualization
**File**: `src/Arch/Intel/VmxExitHandlers.c:405-523`

**RDMSR handling** (line 405-464):
- MSR_LSTAR: returns hooked value if `OriginalLSTAR != 0`, else real MSR (line 413)
- MSR_GS_BASE/FS_BASE: reads from VMCS guest fields
- MSR_IA32_DEBUGCTL: reads from GUEST_IA32_DEBUGCTL VMCS field
- MSR_IA32_FEATURE_CONTROL: **spoofs EnableVmxon=FALSE, Lock=TRUE** (line 428-431)
- VMX MSRs: returns cached values from `MsrData[]` array (line 435-453)
- Default: executes real RDMSR

**WRMSR handling** (line 470-523):
- MSR_LSTAR: **ignores write if hooked** (`OriginalLSTAR == 0` check, line 483-484)
- MSR_GS_BASE/FS_BASE: writes to VMCS guest fields
- MSR_IA32_DEBUGCTL: writes to both VMCS and real MSR
- VMX MSRs: **silently ignored** (no write executed)
- Default: executes real WRMSR

### Monitor Trap Flag (MTF)
**File**: `src/Arch/Intel/VmxExitHandlers.c:579-608`
- Used for EPT hook single-stepping
- Checks if `HookDispatch.pEntry != NULL`
- Handles REP-prefixed instructions by comparing RIP (line 589-590)
- Restores EPT execute-only permissions after instruction executes
- Updates EPT via `EptUpdateTableRecursive()` (line 593-598)
- **Does not invalidate EPT** - relies on page cache for split method (line 601-602)
- Clears dispatch state and disables MTF

---

## EPT Implementation

### EPT Enable Sequence
**File**: `src/Arch/Intel/EPT.c:15-44`
- Reads current secondary controls
- Sets up EPT_TABLE_POINTER (EPTP):
  - `PhysAddr = PML4 physical address >> 12`
  - `PageWalkLength = 3` (4 levels: PML4/PDPT/PD/PT)
- Writes EPTP to VMCS field EPT_POINTER
- Writes VPID (VM_VPID constant) to VIRTUAL_PROCESSOR_ID
- Enables `EnableEPT` and `EnableVPID` in secondary controls
- **Critical**: calls `__invept(INV_ALL_CONTEXTS)` (line 40-41)

### Identity Map Construction
**File**: `src/Arch/Intel/EPT.c:267-282`
- Allocates PML4 page
- Iterates physical memory runs from `g_Data->Memory->NumberOfRuns`
- For each run:
  - Extracts `BasePage` and `PageCount`
  - Calls `EptUpdateTableRecursive()` with EPT_ACCESS_ALL
  - Maps guest PFN → host PFN (identity mapping)
- **Novel**: Uses physical memory descriptor from OS to map only valid ranges

### Page Table Fill Logic
**File**: `src/Arch/Intel/EPT.c:229-260`
- Loops through physical memory runs
- Handles page alignment by chunking into EPT_TABLE_ENTRIES (512 pages)
- Accounts for partial table fills at boundaries
- Updates `hostPFN` to match guest `pfn` for identity

### Recursive Table Update
**File**: `src/Arch/Intel/EPT.c:170-221`
- Starts at PML4, descends to PTE level
- At PTE level (level == EPT_LEVEL_PTE):
  - Updates Read/Write/Execute bits from access mask
  - Sets `MemoryType = VMX_MEM_TYPE_WRITEBACK`
  - Writes host PFN to `PhysAddr` field
  - Supports updating multiple contiguous entries
- At upper levels:
  - Calculates table offset via `EptpTableOffset()`
  - Allocates new page if `PhysAddr == 0`
  - Sets Present/Write/Execute = 1 for intermediate entries
  - Recursively descends to next level

### High IRQL Allocation
**File**: `src/Arch/Intel/EPT.c:80-92`
- **Preallocated page pool**: `EPT_PREALLOC_PAGES` pages allocated at init
- At IRQL > DISPATCH_LEVEL, uses preallocated pages from `Pages[]` array
- Increments `Preallocations` counter
- Bugchecks if pool exhausted (line 91)

### PTE Lookup
**File**: `src/Arch/Intel/EPT.c:319-349`
- `EptGetPTEForPhysical()` walks EPT tables manually
- Starts at PML4, reads PhysAddr field for each level
- Uses `MmGetVirtualForPhysical()` to map physical pages
- Returns pointer to PTE entry at level 0
- Returns STATUS_NOT_FOUND if any level unmapped

### EPT Violation Handler
**File**: `src/Arch/Intel/EPT.c:355-417`
- Extracts PFN from GUEST_PHYSICAL_ADDRESS
- Parses EXIT_QUALIFICATION into `EPT_VIOLATION_DATA`
- Checks if page is hooked via `PHGetHookEntryByPFN()`
- **Hooked page logic** (line 364-405):
  - Read violation: switch to data page with RW access
  - Write violation: switch to code page with RW access
  - Execute violation: switch to code page with EXEC-only access
  - Enables MTF for single-step restore
  - Stores hook entry in `HookDispatch` for MTF handler
- **Identity page logic** (line 407-416):
  - Creates new EPT entry with EPT_ACCESS_ALL
  - Maps guest PFN to same host PFN

### EPT Misconfig Handler
**File**: `src/Arch/Intel/EPT.c:423-431`
- Logs physical address and qualification
- Bugchecks with HYPERVISOR_ERROR / BUG_CHECK_EPT_MISCONFIG
- **No recovery attempt**

---

## Page Hooking Technique

### Hook Installation
**File**: `src/Hooks/PageHook.c:140-211`
- Requires EPT and execute-only EPT support
- Checks if page already hooked (reuses code page if so)
- Allocates code page via `MmAllocateContiguousMemory()`
- Copies entire original page to code page
- Uses `PHpCopyCode()` to save original bytes with LDASM
- Constructs 13-byte jump thunk:
  ```
  PUSH <low 32 bits>
  MOV [RSP+4], <high 32 bits>
  RET
  ```
- Writes thunk to code page at function offset
- Stores hook metadata in `PAGE_HOOK_ENTRY` list
- Calls `KeGenericCallDpc()` to execute `HYPERCALL_HOOK_PAGE` on all CPUs

### LDASM Code Copying
**File**: `src/Hooks/PageHook.c:69-131`
- Disassembles instructions until 13+ bytes captured
- Adjusts relative offsets (jumps, calls) to new location
- Exits early if:
  - Invalid instruction
  - RET/INT3 encountered
  - Relative offset > 2GB
  - Total length > 128 bytes
- Appends jump thunk at end to return to original code

### Hook Removal
**File**: `src/Hooks/PageHook.c:218-251`
- Checks hook count per page
- If multiple hooks on same page: restores bytes in code page only
- If single hook: calls `HYPERCALL_UNHOOK_PAGE` to restore EPT mapping
- Frees code page and hook entry

### Hook Lookup
**File**: `src/Hooks/PageHook.c:258-337`
- Three lookup methods:
  - `PHGetHookEntry()`: by function pointer
  - `PHGetHookEntryByPage()`: by virtual address (data or code page)
  - `PHGetHookEntryByPFN()`: by physical page frame number
- All iterate global `g_PageList` linked list
- Used by EPT violation handler to identify hooked pages

---

## Vendor Abstraction

### CPU Vendor Detection
**File**: `src/Core/HVM.c:10-27`
- `HvmIsHVSupported()` calls `UtilCPUVendor()`
- Intel: calls `VmxHardSupported()`
- AMD: returns TRUE (placeholder, not implemented)

### Per-CPU Initialization
**File**: `src/Core/HVM.c:57-79`
- `HvmpHVCallbackDPC()` runs on each CPU via `KeGenericCallDpc()`
- If Context present (CR3 value): subverts CPU
- If Context NULL: restores CPU
- Calls vendor-specific functions:
  - Intel: `IntelSubvertCPU()` → `VmxInitializeCPU()`
  - AMD: `AMDSubvertCPU()` (stub, prints not supported)

### Teardown Logic
**File**: `src/Core/HVM.c:110-150`
- **Does not use DPC** - causes deadlock (line 116)
- Iterates processors manually via `KeGetProcessorNumberFromIndex()`
- Sets thread affinity to each CPU with `KeSetSystemGroupAffinityThread()`
- Calls `IntelRestoreCPU()` → `VmxShutdown()` on each
- Reverts affinity after each CPU
- **Note**: AMD teardown commented as TODO (line 140)

---

## Unique Features

### Execute-Only EPT Split
- Code page: execute-only permissions
- Data page: read/write permissions
- Violation on read/write → swap to data page, enable MTF
- MTF exit after instruction → restore execute-only

### MSR_LSTAR Hooking
- Saves original MSR_LSTAR value
- Writes new handler address via hypercall
- RDMSR returns original value (hide hook)
- WRMSR blocked if hook active (preserve hook)

### VMX Nested Virtualization Hiding
- All VMX instruction executions inject #UD
- RDMSR on VMX MSRs returns cached values
- WRMSR on VMX MSRs silently ignored
- MSR_IA32_FEATURE_CONTROL spoofed (VMXON disabled, locked)

### Preallocation for High IRQL
- EPT page tables preallocated (EPT_PREALLOC_PAGES)
- Avoids memory allocation at IRQL > DISPATCH_LEVEL
- Bugchecks if exhausted (explicit fail-fast)

### Context Capture for Seamless Launch
- `KeSaveStateForHibernate()` + `RtlCaptureContext()`
- Guest RIP = return address of RtlCaptureContext
- Three-state transition prevents loops
- Custom `VmRestoreContext()` (not RtlRestoreContext) - Win10 15063+ RSP check workaround

### Physical Memory Descriptor Usage
- Reads `g_Data->Memory->Run[]` array
- Only maps valid physical ranges to EPT
- Skips holes in physical address space

---

## File References Summary

| Component | File | Lines |
|-----------|------|-------|
| Feature detection | `src/Arch/Intel/VMX.c` | 37-106 |
| CPU initialization | `src/Arch/Intel/VMX.c` | 133-312 |
| VMCS setup | `src/Arch/Intel/VMX.c` | 451-653 |
| Exit dispatcher | `src/Arch/Intel/VmxExitHandlers.c` | 54-121, 138-182 |
| CPUID/MSR handling | `src/Arch/Intel/VmxExitHandlers.c` | 213-523 |
| MTF handler | `src/Arch/Intel/VmxExitHandlers.c` | 579-608 |
| EPT setup | `src/Arch/Intel/EPT.c` | 15-44, 267-282 |
| EPT table recursion | `src/Arch/Intel/EPT.c` | 170-221 |
| EPT violation | `src/Arch/Intel/EPT.c` | 355-417 |
| Page hooking | `src/Hooks/PageHook.c` | 140-211 |
| Code copying | `src/Hooks/PageHook.c` | 69-131 |
| Vendor abstraction | `src/Core/HVM.c` | 10-150 |

---

## Key Takeaways for Ombra

1. **Three-state launch pattern** prevents context restore loops
2. **MSR caching** reduces VMCS setup overhead
3. **Preallocated EPT pages** enable high-IRQL allocation
4. **Split EPT permissions** (exec-only vs RW) for stealthy hooks
5. **MTF single-stepping** restores permissions after hooked instruction
6. **LDASM-based code copying** handles relative offsets correctly
7. **VMX nesting prevention** via #UD injection + MSR spoofing
8. **Physical memory runs** from OS descriptor optimize EPT construction
9. **Per-CPU DPC pattern** for symmetrical multiprocessing
10. **Manual teardown** avoids DPC deadlock on unload
