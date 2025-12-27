# gbhv - Game-Focused Hypervisor Pattern Extraction

**Source**: `Refs/codebases/gbhv/`
**Focus**: Performance-oriented VMX implementation with EPT shadow hooking

---

## 1. VMX SETUP PATTERNS

### Initialization Sequence

**Fixed Bit Enforcement** - `vmx.c:53-73`
- Read CR0/CR4 fixed bits from `IA32_VMX_CR0_FIXED0/1` and `IA32_VMX_CR4_FIXED0/1` MSRs
- OR with FIXED0 (must be 1), AND with FIXED1 (must be 0)
- Apply before VMXON to meet processor requirements

**Root Mode Entry** - `vmx.c:85-118`
```c
// Sequence:
1. Enable CR4.VMXE (bit 13)
2. Set CR0/CR4 fixed bits
3. VMXON with physical address of VMXON region
4. VMCLEAR to initialize VMCS
5. VMPTRLD to load VMCS pointer
```

**VMCS Revision ID Setup** - `vmm.c:187-189`
- First 31 bits of VMXON/VMCS regions must match `IA32_VMX_BASIC.VmcsRevisionId`
- Bit 31 must be 0 (shadow VMCS indicator)

**True MSR Support Detection** - `vmcs.c:470-479`
```c
if (Context->GlobalContext->VmxCapabilities.VmxControls == 1) {
    // Use IA32_VMX_TRUE_* MSRs for flexible control bits
    ConfigMSR = ArchGetHostMSR(IA32_VMX_TRUE_PINBASED_CTLS);
} else {
    // Fallback to original MSRs with stricter requirements
    ConfigMSR = ArchGetHostMSR(IA32_VMX_PINBASED_CTLS);
}
```

### Segment Descriptor Translation

**GDT Entry to VMX Descriptor** - `vmx.c:156-238`
- Extract base address from three separate fields in GDT entry (Low/Middle/High)
- Handle 64-bit base for System segments (DescriptorType == 0)
- Use `__segmentlimit()` intrinsic for segment limit
- Copy access rights bit-by-bit to VMX format
- Clear RPL for host segments to ensure consistency

**Host State Configuration** - `vmcs.c:82-168`
- Host CR3 always uses `SystemDirectoryTableBase` to avoid usermode CR3 during DPC
- FS/GS base read from MSRs (IA32_FS_BASE, IA32_GS_BASE), not GDT
- Host segments clear RPL bits (ClearRPL = TRUE)

### Performance Optimizations in Setup

**MSR Bitmap Usage** - `vmcs.c:515-527`
```c
Register.UseMsrBitmaps = 1;  // Avoid exits on all MSRs
// Later: vmcs.c:433
VmxVmwriteFieldFromImmediate(VMCS_CTRL_MSR_BITMAP_ADDRESS, MsrBitmapPhysical);
// Zero bitmap = no exits on any MSR (vmm.c:254-255)
```

**Secondary Controls Activation** - `vmcs.c:513`
```c
Register.ActivateSecondaryControls = 1;  // Enable EPT, VPID, etc.
```

**VPID for TLB Performance** - `vmcs.c:585-594, 447`
```c
Register.EnableVpid = 1;  // Associate TLB entries with VPID
VmxVmwriteFieldFromImmediate(VMCS_CTRL_VIRTUAL_PROCESSOR_IDENTIFIER, 1);
// All processors use VPID=1, separates EPT TLB from OS page tables
```

**RDTSCP/INVPCID/XSAVES Support** - `vmcs.c:576-614`
- Enable these to prevent #UD on Windows 10 instructions
- Critical for stability on modern Windows

---

## 2. STEALTH TECHNIQUES

### Hypervisor Presence Hiding

**CPUID VMX Bit Masking** - `exit.c:61-66`
```c
if (ExitContext->GuestContext->GuestRAX == CPUID_VERSION_INFORMATION) {
    // Clear bit 5 in ECX (VMX feature flag)
    CPUInfo[2] = HvUtilBitClearBit(CPUInfo[2], CPUID_VMX_ENABLED_BIT);
}
```

**Intel PT Concealment** - `vmcs.c:617-624, 663-670, 723-730`
```c
// Pin-based, entry, and exit controls:
Register.ConcealVmxFromPt = 1;
// Suppresses Intel Processor Trace packets indicating virtualization
```

### EPT Shadow Hook Technique

**Execute-Only Fake Page** - `ept.c:749-761`
```c
// Shadow entry (fake page with hook):
FakeEntry.ReadAccess = 0;
FakeEntry.WriteAccess = 0;
FakeEntry.ExecuteAccess = 1;  // Execute-only triggers EPT violation on R/W
FakeEntry.PageFrameNumber = PhysicalFakePage / PAGE_SIZE;
```

**Non-Executable Original Page** - `ept.c:771-776`
```c
// Hooked entry (original page):
OriginalEntry.ReadAccess = 1;
OriginalEntry.WriteAccess = 1;
OriginalEntry.ExecuteAccess = 0;  // Non-execute triggers violation on fetch
```

**Page Swapping Logic** - `ept.c:810-886`
- Execute access + current page non-executable → swap to fake (execute-only) page
- Read/Write access + current page executable → swap to original (R/W) page
- `ShouldIncrementRIP = FALSE` to retry instruction after swap

**Inline Hook Trampoline** - `ept.c:597-680`
```c
// 14-byte absolute jump (push/ret technique):
push low32(target)
mov [rsp+4], high32(target)
ret
// Avoids reading hooked page (jmp [rip+0] would trigger R violation)
```

---

## 3. MEMORY ACCESS PATTERNS

### Guest Memory Read/Write

**Physical Address Translation** - `ept.c:281-298`
```c
// Get PML2 entry (2MB page):
Directory = ADDRMASK_EPT_PML2_INDEX(PhysicalAddress);       // Bits 21-29
DirectoryPointer = ADDRMASK_EPT_PML3_INDEX(PhysicalAddress); // Bits 30-38
PML4Entry = ADDRMASK_EPT_PML4_INDEX(PhysicalAddress);        // Bits 39-47
PML2 = &ProcessorContext->EptPageTable->PML2[DirectoryPointer][Directory];
```

**PML1 Entry Access (Split Pages)** - `ept.c:304-347`
```c
// If LargePage bit clear, PML2 is now a pointer:
PML2Pointer = (PEPT_PML2_POINTER) PML2;
PML1 = OsPhysicalToVirtual(PML2Pointer->PageFrameNumber * PAGE_SIZE);
PML1 = &PML1[ADDRMASK_EPT_PML1_INDEX(PhysicalAddress)];  // Bits 12-20
```

### Physical Memory Mapping

**Identity Mapping Strategy** - `ept.c:167-251`
- Single 512GB PML4 entry covering 0x0 to 0x8000000000
- 512 PML3 entries (1GB each), each pointing to 512 PML2 entries
- Default: 2MB large pages (LargePage=1) for entire identity map
- Total: 512 * 512 = 262,144 PML2 entries covering 512GB

**MTRR Memory Type Integration** - `ept.c:44-97, 107-165`
```c
// Build map of MTRR ranges from IA32_MTRR_PHYSBASE/MASK pairs:
for (CurrentRegister = 0; CurrentRegister < MTRRCap.VariableRangeCount; CurrentRegister++) {
    PhysBase = ArchGetHostMSR(IA32_MTRR_PHYSBASE0 + (CurrentRegister * 2));
    PhysMask = ArchGetHostMSR(IA32_MTRR_PHYSMASK0 + (CurrentRegister * 2));

    if (PhysMask.Valid) {
        Descriptor->PhysicalBaseAddress = PhysBase.PageFrameNumber * PAGE_SIZE;
        // Range size = lowest set bit in mask
        _BitScanForward64(&NumberOfBitsInMask, PhysMask.PageFrameNumber * PAGE_SIZE);
        Descriptor->PhysicalEndAddress = BaseAddress + ((1ULL << NumberOfBitsInMask) - 1);
        Descriptor->MemoryType = PhysBase.Type;
    }
}

// Apply to each 2MB entry:
for (EntryGroupIndex = 0; EntryGroupIndex < 512; EntryGroupIndex++) {
    for (EntryIndex = 0; EntryIndex < 512; EntryIndex++) {
        HvEptSetupPML2Entry(GlobalContext, &PML2[EntryGroupIndex][EntryIndex],
                            (EntryGroupIndex * 512) + EntryIndex);
    }
}
```

**First Page UC Workaround** - `ept.c:124-133`
```c
if (PageFrameNumber == 0) {
    NewEntry->MemoryType = MEMORY_TYPE_UNCACHEABLE;
    // First MB typically MMIO, avoid undefined behavior from fixed MTRRs
}
```

**Default WB, Override from MTRR** - `ept.c:136-161`
- Default all pages to `MEMORY_TYPE_WRITE_BACK`
- For each MTRR range overlapping the 2MB page, override with MTRR type
- UC (Uncacheable) takes precedence per 11.11.4.1 spec

### Dynamic Page Splitting

**2MB → 4KB Split** - `ept.c:358-450`
```c
// Allocate new split structure with 512 PML1 entries:
NewSplit = OsAllocateContiguousAlignedPages(sizeof(VMM_EPT_DYNAMIC_SPLIT)/PAGE_SIZE);

// Copy permissions from original 2MB entry:
EntryTemplate.Flags = 0;
EntryTemplate.ReadAccess = 1;
EntryTemplate.WriteAccess = 1;
EntryTemplate.ExecuteAccess = 1;
EntryTemplate.MemoryType = TargetEntry->MemoryType;
__stosq((SIZE_T*)&NewSplit->PML1[0], EntryTemplate.Flags, 512);

// Identity map each 4KB page:
for (EntryIndex = 0; EntryIndex < 512; EntryIndex++) {
    NewSplit->PML1[EntryIndex].PageFrameNumber =
        ((TargetEntry->PageFrameNumber * SIZE_2_MB) / PAGE_SIZE) + EntryIndex;
}

// Replace 2MB entry with pointer to PML1 table:
NewPointer.PageFrameNumber = OsVirtualToPhysical(&NewSplit->PML1[0]) / PAGE_SIZE;
RtlCopyMemory(TargetEntry, &NewPointer, sizeof(NewPointer));
```

**Split Tracking** - `ept.h:137-158, ept.c:442`
```c
// Linked list of dynamic splits for cleanup:
typedef struct _VMM_EPT_DYNAMIC_SPLIT {
    DECLSPEC_ALIGN(PAGE_SIZE) EPT_PML1_ENTRY PML1[512];
    union { PEPT_PML2_ENTRY Entry; PEPT_PML2_POINTER Pointer; };
    LIST_ENTRY DynamicSplitList;
} VMM_EPT_DYNAMIC_SPLIT;

InsertHeadList(&ProcessorContext->EptPageTable->DynamicSplitList, &NewSplit->DynamicSplitList);
```

---

## 4. PERFORMANCE OPTIMIZATIONS

### Exit Minimization

**Exception Bitmap Zero** - `vmcs.c:363-369`
```c
VmxVmwriteFieldFromImmediate(VMCS_CTRL_EXCEPTION_BITMAP, 0);
// No exits on any exceptions, delivered through IDT
```

**CR3-Target Count Zero** - `vmcs.c:380-392`
```c
VmxVmwriteFieldFromImmediate(VMCS_CTRL_CR3_TARGET_COUNT, 0);
// All MOV to CR3 cause exits (could optimize by whitelisting kernel CR3)
```

**Minimal VMExit Handlers** - `exit.c:104-163`
```c
switch (ExitReason.BasicExitReason) {
    case VMX_EXIT_REASON_EXECUTE_CPUID:  // CPUID spoof
    case VMX_EXIT_REASON_EXECUTE_INVD:   // WBINVD passthrough
    case VMX_EXIT_REASON_EXECUTE_XSETBV: // XSETBV passthrough
    case VMX_EXIT_REASON_EPT_VIOLATION:   // Page hook swap
    case VMX_EXIT_REASON_EPT_MISCONFIGURATION: // Fatal error
    default: // Try to continue despite unknown exit
}
```

**RIP Increment for Instruction Exits** - `exit.c:20-21, 152-159`
```c
ExitContext->ShouldIncrementRIP = TRUE;  // Default for all instruction exits
// Later:
if (ExitContext->ShouldIncrementRIP) {
    VmxVmreadFieldToImmediate(VMCS_VMEXIT_INSTRUCTION_LENGTH, &GuestInstructionLength);
    ExitContext->GuestRIP += GuestInstructionLength;
    VmxVmwriteFieldFromImmediate(VMCS_GUEST_RIP, ExitContext->GuestRIP);
}
```

### Caching Strategies

**EPT Memory Type Configuration** - `ept.c:541-542`
```c
EPTP.MemoryType = MEMORY_TYPE_WRITE_BACK;
// Allow processor to cache EPT page table walks
```

**INVEPT on Hook Installation** - `ept.c:788-796`
```c
if (ProcessorContext->HasLaunched) {
    Descriptor.EptPointer = ProcessorContext->EptPointer.Flags;
    Descriptor.Reserved = 0;
    __invept(1, &Descriptor);  // Type 1: single-context invalidation
}
// Only invalidate when hooking, not on every page swap
```

**VPID TLB Separation** - `vmcs.c:444-448`
```c
// VPID=1 for all processors
VmxVmwriteFieldFromImmediate(VMCS_CTRL_VIRTUAL_PROCESSOR_IDENTIFIER, 1);
// Separates EPT translations from OS page tables in TLB
// Avoids full TLB flush on EPT operations
```

### Allocation Patterns

**Contiguous Aligned Pages** - `vmm.c:176-179, 254-255, 280-283`
```c
// VMXON region: 4KB aligned, contiguous physical memory
VmxonRegion = OsAllocateContiguousAlignedPages(VMX_VMXON_NUMBER_PAGES);
OsZeroMemory(Region, VMX_VMXON_NUMBER_PAGES * PAGE_SIZE);

// MSR bitmap: Single 4KB page, zeroed (no exits)
Context->MsrBitmap = OsAllocateContiguousAlignedPages(1);
OsZeroMemory(Context->MsrBitmap, PAGE_SIZE);
```

**Embedded Host Stack** - `vmm.c:220-223`
```c
typedef struct _VMM_PROCESSOR_CONTEXT {
    // Stack embedded in context structure
    VMM_HOST_STACK HostStack;
    // Top of stack = &Context->HostStack.GlobalContext
} VMM_PROCESSOR_CONTEXT;
```

**Intrinsics for Bulk Operations** - `ept.c:211, 237, 419`
```c
// Fill 512 entries with template using STOSQ:
__stosq((SIZE_T*)&PageTable->PML3[0], RWXTemplate.Flags, VMM_EPT_PML3E_COUNT);
__stosq((SIZE_T*)&PageTable->PML2[0], PML2EntryTemplate.Flags, VMM_EPT_PML3E_COUNT * VMM_EPT_PML2E_COUNT);
__stosq((SIZE_T*)&NewSplit->PML1[0], EntryTemplate.Flags, VMM_EPT_PML1E_COUNT);
```

### DPC Broadcast Pattern

**Per-CPU Initialization** - `vmm.c:21-83, 327-368`
```c
// Queue DPC on all processors:
KeGenericCallDpc(HvpDPCBroadcastFunction, (PVOID)GlobalContext);

// In DPC callback:
VOID NTAPI HvpDPCBroadcastFunction(...) {
    CurrentContext = HvGetCurrentCPUContext(GlobalContext);
    if (HvBeginInitializeLogicalProcessor(CurrentContext)) {
        InterlockedIncrement(&GlobalContext->SuccessfulInitializationsCount);
    }
    KeSignalCallDpcSynchronize(SystemArgument2);
    KeSignalCallDpcDone(SystemArgument1);
}
```

**Interrupt Handling in Exit Handler** - `vmm.c:420-486`
```c
// Prevent context switch during interrupt enable:
ExitContext.SavedIRQL = KeGetCurrentIrql();
if (ExitContext.SavedIRQL < DISPATCH_LEVEL) {
    KeRaiseIrqlToDpcLevel();  // Disable dispatcher
}

// Handle exit with interrupts enabled...

if (ExitContext.SavedIRQL < DISPATCH_LEVEL) {
    KeLowerIrql(ExitContext.SavedIRQL);
}
```

---

## Key Takeaways for Ombra

### VMX Setup
1. Use True MSRs when available for flexibility
2. Apply CR0/CR4 fixed bits before VMXON
3. Zero MSR bitmap for minimal exits
4. Enable VPID, RDTSCP, INVPCID, XSAVES for stability
5. Conceal from Intel PT

### Stealth
1. Clear CPUID VMX bit in leaf 1
2. Use EPT execute-only pages for invisible hooks
3. Swap pages on EPT violations (exec vs R/W)
4. Trampoline with push/ret to avoid reading hooked page

### Memory
1. Identity map 512GB with 2MB pages by default
2. Apply MTRR memory types to each entry
3. Split to 4KB only when needed for fine-grained permissions
4. Track splits in linked list for cleanup

### Performance
1. Zero exception bitmap (no exception exits)
2. VPID separates EPT TLB from OS
3. WB memory type for EPT
4. INVEPT only on hook install, not every swap
5. Use intrinsics (__stosq) for bulk initialization
6. Raise IRQL before enabling interrupts in exit handler

---

**Analysis Date**: 2025-12-20
**Codebase Version**: gbhv (game-focused hypervisor)
**Key Files Analyzed**: vmx.c, vmcs.c, vmm.c, exit.c, ept.c, arch.c, util.c
