# SimpleSvmHook - Pattern Extraction

**Codebase**: `Refs/codebases/SimpleSvmHook/`
**Focus**: AMD SVM with syscall/function hooking via NPT-based techniques
**Extracted**: 2025-12-20

---

## 1. SYSCALL INTERCEPTION

### Approach: NPT-Based Function Hooking (Not Direct MSR Interception)

SimpleSvmHook does **NOT** intercept syscalls via MSR_LSTAR or EFER.SCE directly. Instead, it hooks **kernel-exported functions** using NPT-based page permission manipulation.

#### Key Pattern: Breakpoint Injection + NPT State Machine

**File**: `HookKernelRegistration.cpp:408-409`
```c
// Install a breakpoint to the exec page so that the hypervisor can tell
// when it is being executed.
*hookAddrInExecPage = 0xcc;
```

**Analysis**:
- Hooks are installed by placing `0xcc` (INT3) on a shadow "exec page"
- The exec page is a copy of the original page with hooks installed
- NPT controls which physical page is visible during execution

#### VMSAVE/VMLOAD for State Preservation

**File**: `Virtualization.cpp:509-522`
```c
// Save some of the current state on VMCB. Some of those states are:
// - FS, GS, TR, LDTR (including all hidden state)
// - KernelGsBase
// - STAR, LSTAR, CSTAR, SFMASK
// - SYSENTER_CS, SYSENTER_ESP, SYSENTER_EIP
__svm_vmsave(guestVmcbPa.QuadPart);
```

**Analysis**:
- `LSTAR` (syscall entry point) is saved/restored via VMSAVE/VMLOAD
- No active interception of MSR_LSTAR writes
- Guest's syscall infrastructure remains untouched

---

## 2. NPT-BASED HOOKING

### Three-State NPT Permission Model

**File**: `HookVmmCommon.cpp:10-41`
```
State                     : Page Type
                          : Current : Hooked : Other
0)NptDefault              : RWX(O)  : RWX(O) : RWX(O)
1)NptHookEnabledInvisible : RWX(O)  : RW-(O) : RWX(O)
2)NptHookEnabledVisible   : RWX(E)  : RW-(O) : RW-(O)

    Current= The page the processor is currently executing on.
    Hooked = The pages hooks are installed into and not being executed.
    Other  = The rest of pages.

    (O)= The page is backed by the original physical page where no hook exists.
    (E)= The page is backed by the exec physical page where hooks exist.

Transition:
0 -> 1 on enabling hooks (via CPUID)
1 -> 2 on execution access against any of hooked pages
2 -> 1 on execution access against any of non hooked pages
```

**Analysis**:
- Hooks are invisible until executed (state 1: NX bit set on hooked pages)
- On execute attempt, NPT fault occurs → switch to exec page with hooks
- When execution leaves hooked page, revert to original page

### Hook Registration Structure

**File**: `HookCommon.hpp:17-49`
```c
typedef struct _HOOK_ENTRY
{
    PVOID HookAddress;                    // Where hook is installed
    PVOID Handler;                        // Handler function to execute
    PVOID PageBaseForExecution;           // Virtual address of exec page
    ULONG64 PhyPageBase;                  // Physical address of original page
    ULONG64 PhyPageBaseForExecution;      // Physical address of exec page
    PVOID OriginalCallStub;               // Stub to call original code
} HOOK_ENTRY, *PHOOK_ENTRY;
```

**File**: `HookCommon.cpp:18-47`
```c
HOOK_REGISTRATION_ENTRY g_HookRegistrationEntries[] =
{
    { RTL_CONSTANT_STRING(L"ZwQuerySystemInformation"), HandleZwQuerySystemInformation },
    { RTL_CONSTANT_STRING(L"ExAllocatePoolWithTag"),    HandleExAllocatePoolWithTag },
    { RTL_CONSTANT_STRING(L"ExFreePoolWithTag"),        HandleExFreePoolWithTag },
    { RTL_CONSTANT_STRING(L"ExFreePool"),               HandleExFreePool },
};
```

**Analysis**:
- Hooks target kernel-exported functions (resolved via `MmGetSystemRoutineAddress`)
- Not syscall entry points themselves
- Two-page technique: original page + exec page with `0xcc` breakpoints

### Exec Page Creation

**File**: `HookKernelRegistration.cpp:131-139`
```c
execPage = ExAllocatePoolWithTag(NonPagedPool, PAGE_SIZE, k_PoolTag);
if (execPage == nullptr) {
    status = STATUS_INSUFFICIENT_RESOURCES;
    goto Exit;
}
NT_ASSERT(execPage == PAGE_ALIGN(execPage));
RtlCopyMemory(execPage, hookAddressBase, PAGE_SIZE);
```

**Analysis**:
- Exec page is a full copy of the original page
- `0xcc` is written to specific function offsets within exec page
- Original page remains unmodified

### NPT Fault Handling

**File**: `HookVmmCommon.cpp:348-393`
```c
VOID HandleNestedPageFault(PVMCB GuestVmcb, PHOOK_DATA HookData)
{
    NPF_EXITINFO1 exitInfo;
    ULONG64 faultingPa;
    PPT_ENTRY_4KB nptEntry;

    faultingPa = GuestVmcb->ControlArea.ExitInfo2;
    exitInfo.AsUInt64 = GuestVmcb->ControlArea.ExitInfo1;

    if (exitInfo.Fields.Valid == FALSE) {
        // MMIO access - build NPT entry on-demand
        nptEntry = BuildSubTables(HookData->Pml4Table, faultingPa, HookData);
        goto Exit;
    }

    // Execute attempt on hooked page - transition NPT state
    NT_ASSERT(exitInfo.Fields.Execute != FALSE);
    TransitionNptState(HookData, faultingPa);

Exit:
    return;
}
```

**Analysis**:
- NPT faults occur on execute access to hooked pages (NX bit set)
- Fault handler switches physical page backing from original → exec page
- Makes exec page executable, all other pages non-executable

### State Transition 1 → 2 (Enable Hooks on Page)

**File**: `HookVmmCommon.cpp:113-179`
```c
static VOID TransitionNptState1To2(PHOOK_DATA HookData, const HOOK_ENTRY* CurrentHookEntry)
{
    PPT_ENTRY_4KB nptEntry;

    // Make all pages non-executable
    ChangePermissionsOfAllPages(HookData->Pml4Table, 0, TRUE, HookData->MaxNptPdpEntriesUsed);

    // Get NPT entry for the hooked page
    nptEntry = GetNestedPageTableEntry(HookData->Pml4Table, CurrentHookEntry->PhyPageBase);
    NT_ASSERT(nptEntry->Fields.NoExecute != FALSE);
    NT_ASSERT(nptEntry->Fields.PageFrameNumber == GetPfnFromPa(CurrentHookEntry->PhyPageBase));

    // Switch to exec physical page with hooks and make executable
    nptEntry->Fields.PageFrameNumber = GetPfnFromPa(CurrentHookEntry->PhyPageBaseForExecution);
    ChangePermissionOfPage(HookData->Pml4Table, CurrentHookEntry->PhyPageBase, FALSE);

    HookData->ActiveHookEntry = CurrentHookEntry;
    HookData->NptState = NptHookEnabledVisible;
}
```

**Analysis**:
- First disable execute on ALL pages (prevents guest from bypassing hooks)
- Then swap PFN to point to exec page with `0xcc` breakpoints
- Enable execute only on the current hooked page

### State Transition 2 → 1 (Hide Hooks)

**File**: `HookVmmCommon.cpp:186-280`
```c
static VOID TransitionNtpState2To1(PHOOK_DATA HookData)
{
    PPT_ENTRY_4KB nptEntry;

    // Make all pages executable (except hooked pages)
    ChangePermissionsOfAllPages(HookData->Pml4Table,
                                HookData->ActiveHookEntry->PhyPageBase,
                                FALSE,
                                HookData->MaxNptPdpEntriesUsed);

    // Make all hooked pages non-executable
    for (const auto& registration : g_HookRegistrationEntries) {
        ChangePermissionOfPage(HookData->Pml4Table,
                               registration.HookEntry.PhyPageBase, TRUE);
    }

    // Switch current page back to original physical page
    nptEntry = GetNestedPageTableEntry(HookData->Pml4Table,
                                       HookData->ActiveHookEntry->PhyPageBase);
    nptEntry->Fields.PageFrameNumber = GetPfnFromPa(HookData->ActiveHookEntry->PhyPageBase);

    HookData->ActiveHookEntry = nullptr;
    HookData->NptState = NptHookEnabledInvisible;
}
```

**Analysis**:
- Restore execute permissions on all non-hooked pages
- Swap PFN back to original page (without `0xcc`)
- Set hooked pages to NX to trigger fault on next execute

### Breakpoint Exception Handling

**File**: `HookVmmCommon.cpp:567-596`
```c
VOID HandleBreakPointException(PVMCB GuestVmcb, PHOOK_DATA HookData)
{
    const HOOK_ENTRY* entry;

    entry = FindHookEntryByAddress(reinterpret_cast<PVOID>(GuestVmcb->StateSaveArea.Rip));

    if (entry != nullptr) {
        // Transfer to the hook handler
        GuestVmcb->StateSaveArea.Rip = reinterpret_cast<ULONG64>(entry->Handler);
    } else {
        // Pass through to guest
        InjectBreakPointException(GuestVmcb);
    }
}
```

**Analysis**:
- When `0xcc` executes, #BP causes VMEXIT
- If RIP matches a hook address, redirect to handler
- Otherwise, inject #BP back to guest (legitimate breakpoint)

### Permission Manipulation

**File**: `HookVmmAlwaysOptimized.cpp:24-148`
```c
VOID ChangePermissionOfPage(PPML4_ENTRY_4KB Pml4Table,
                             ULONG64 PhysicalAddress,
                             BOOLEAN DisallowExecution)
{
    // Traverse PML4 → PDPT → PDT → PT
    pxeIndex = GetPxeIndex(PhysicalAddress);
    pml4Entry = &Pml4Table[pxeIndex];
    pageDirectoryPointerTable = GetVaFromPfn(pml4Entry->Fields.PageFrameNumber);

    ppeIndex = GetPpeIndex(PhysicalAddress);
    pdptEntry = &pageDirectoryPointerTable[ppeIndex];
    pageDirectoryTable = GetVaFromPfn(pdptEntry->Fields.PageFrameNumber);

    // If making executable and parent PDPT is NX, must make PDPT executable
    // then mark all 512 PDT entries as NX to maintain previous state
    if ((DisallowExecution == FALSE) && (pdptEntry->Fields.NoExecute != FALSE)) {
        pdptEntry->Fields.NoExecute = FALSE;
        for (pdeIndex = 0; pdeIndex < 512; ++pdeIndex) {
            pageDirectoryTable[pdeIndex].Fields.NoExecute = TRUE;
        }
    }

    // Same pattern for PDT → PT
    pdeIndex = GetPdeIndex(PhysicalAddress);
    pdtEntry = &pageDirectoryTable[pdeIndex];
    pageTable = GetVaFromPfn(pdtEntry->Fields.PageFrameNumber);

    if ((DisallowExecution == FALSE) && (pdtEntry->Fields.NoExecute != FALSE)) {
        pdtEntry->Fields.NoExecute = FALSE;
        for (pteIndex = 0; pteIndex < 512; ++pteIndex) {
            pageTable[pteIndex].Fields.NoExecute = TRUE;
        }
    }

    // Finally set the leaf page permission
    pteIndex = GetPteIndex(PhysicalAddress);
    ptEntry = &pageTable[pteIndex];
    ptEntry->Fields.NoExecute = DisallowExecution;
}
```

**Analysis**:
- NX inheritance: child pages inherit parent's NX bit via AND logic
- To make a page executable when parent is NX, must:
  1. Clear parent NX bit
  2. Set NX on all 512 child entries
  3. Clear NX only on target child
- This is **slow** (512-iteration overhead) but maintains granular control

---

## 3. AMD-SPECIFIC PATTERNS

### SVM vs VMX Differences

#### MSR Access Interception

**File**: `VmmMain.cpp:163-260`
```c
static VOID HandleMsrAccess(PVIRTUAL_PROCESSOR_DATA VpData, PGUEST_CONTEXT GuestContext)
{
    UINT32 msr = GuestContext->VpRegs->Rcx & MAXUINT32;
    BOOLEAN writeAccess = (VpData->GuestVmcb.ControlArea.ExitInfo1 != 0);

    if (msr == IA32_MSR_EFER) {
        // Only EFER is explicitly intercepted
        value.QuadPart = (GuestContext->VpRegs->Rdx << 32) | GuestContext->VpRegs->Rax;
        if ((value.QuadPart & EFER_SVME) == 0) {
            InjectGeneralProtectionException(VpData);  // Protect SVME bit
        }
        VpData->GuestVmcb.StateSaveArea.Efer = value.QuadPart;
    } else {
        // Pass through all other MSRs
        if (writeAccess) {
            value.QuadPart = (GuestContext->VpRegs->Rdx << 32) | GuestContext->VpRegs->Rax;
            __writemsr(msr, value.QuadPart);
        } else {
            value.QuadPart = __readmsr(msr);
            GuestContext->VpRegs->Rax = value.LowPart;
            GuestContext->VpRegs->Rdx = value.HighPart;
        }
    }
    VpData->GuestVmcb.StateSaveArea.Rip = VpData->GuestVmcb.ControlArea.NRip;
}
```

**Analysis**:
- **Intel VMX**: Uses MSR bitmap for granular per-MSR interception control
- **AMD SVM**: MSR permission map covers ranges, individual MSRs like LSTAR not intercepted
- EFER.SVME must be protected (disabling SVM mid-guest causes undefined behavior)
- No interception of LSTAR/STAR/CSTAR - syscall infrastructure is transparent

#### NPT vs EPT Terminology

| AMD SVM (NPT)               | Intel VMX (EPT)            |
|-----------------------------|----------------------------|
| Nested Page Tables (NPT)    | Extended Page Tables (EPT) |
| VMCB.NCr3 (NPT PML4 base)   | VMCS.EPTP (EPT PML4 base)  |
| NPF (Nested Page Fault)     | EPT Violation              |
| ExitInfo1/ExitInfo2         | Exit Qualification         |
| NoExecute bit (NX)          | Execute-disable (XD)       |

#### VMEXIT Reason Codes

**File**: `VmmMain.cpp:352-376`
```c
switch (VpData->GuestVmcb.ControlArea.ExitCode)
{
case VMEXIT_CPUID:
    HandleCpuid(VpData, &guestContext);
    break;
case VMEXIT_MSR:
    HandleMsrAccess(VpData, &guestContext);
    break;
case VMEXIT_VMRUN:
    HandleVmrun(VpData, &guestContext);
    break;
case VMEXIT_EXCEPTION_BP:
    HandleBreakPointException(&VpData->GuestVmcb, VpData->HookData);
    break;
case VMEXIT_NPF:
    HandleNestedPageFault(&VpData->GuestVmcb, VpData->HookData);
    break;
default:
    SIMPLESVMHOOK_BUG_CHECK();
}
```

**Analysis**:
- AMD uses `ExitCode` field in VMCB Control Area
- Intel uses VM-Exit Reason in VMCS
- NPT faults are essential for hooking (equivalent to EPT violations)

### VMCB Structure Access

**File**: `HookVmmCommon.cpp:360-362`
```c
faultingPa = GuestVmcb->ControlArea.ExitInfo2;
exitInfo.AsUInt64 = GuestVmcb->ControlArea.ExitInfo1;
```

**Analysis**:
- **ExitInfo1**: Fault type (read/write/execute, valid bit)
- **ExitInfo2**: Faulting guest physical address
- Intel equivalent: VM-Exit Qualification + Guest-Physical Address (GPA)

### ASID Management

SimpleSvmHook does **not** implement ASID (Address Space ID) management. The VMCB's ASID field is left at default (likely 0), meaning TLB flushes occur on every VMRUN.

**Implication for Ombra**:
- Consider setting unique ASID per guest to avoid TLB flushes
- ASID is analogous to Intel's VPID (Virtual Processor ID)

---

## Recommendations for Ombra

### For Syscall Interception

**Option 1: NPT-Based Hook (SimpleSvmHook approach)**
- **Pros**: Stealthy, hard to detect, works for any kernel function
- **Cons**: Complex state machine, performance overhead from NPT faults
- **Use Case**: Hook specific syscalls (e.g., NtQuerySystemInformation)

**Option 2: MSR Interception (Alternative)**
- **Pros**: Direct control over syscall entry point, simpler
- **Cons**: More detectable, requires MSR permission map setup
- **Use Case**: Monitor/modify all syscalls globally

**Recommended Approach**:
```c
// For Ombra, combine both:
// 1. NPT-based hooks for specific high-value syscalls (stealth)
// 2. Optional MSR_LSTAR interception for global syscall monitoring (debug builds)
```

### NPT Implementation Patterns to Adopt

1. **Two-Page Technique**: Original page + exec page with modifications
2. **State Machine**: Track per-processor NPT state (default/invisible/visible)
3. **Permission Cascade**: When changing leaf NX, update parent tables
4. **Pre-allocated NPT Entries**: Avoid allocations during VMEXIT

**File**: `HookCommon.hpp:94-108`
```c
typedef struct _HOOK_DATA
{
    PPML4_ENTRY_4KB Pml4Table;
    PVOID PreAllocatedNptEntries[50];  // Pre-allocate to avoid runtime alloc
    volatile LONG UsedPreAllocatedEntriesCount;
    ULONG MaxNptPdpEntriesUsed;
    const HOOK_ENTRY* ActiveHookEntry;
    NPT_STATE NptState;
} HOOK_DATA, *PHOOK_DATA;
```

### AMD-Specific Considerations

1. **VMSAVE/VMLOAD**: Use for fast guest state save/restore (LSTAR, STAR, etc.)
2. **MSR Permission Map**: Only intercept critical MSRs (EFER), pass through rest
3. **ASID**: Implement to avoid TLB flushes (SimpleSvmHook doesn't)
4. **NRip Field**: Use `VMCB.ControlArea.NRip` for instruction length after VMEXIT

---

## Files to Modify in Ombra

### For NPT-Based Syscall Hooks

1. **`OmbraPayload/Svm/NptManager.cpp`** - Implement NPT state machine
2. **`OmbraPayload/Svm/VmexitHandlers.cpp`** - Add NPT fault handler
3. **`OmbraPayload/Hooks/SyscallHooks.cpp`** - Define hook entries and handlers
4. **`OmbraPayload/Hooks/HookRegistry.cpp`** - Registration system (similar to `HookKernelRegistration.cpp`)

### For MSR Interception (Optional)

1. **`OmbraPayload/Svm/MsrHandlers.cpp`** - Add LSTAR/STAR interception
2. **`OmbraPayload/Svm/VmcbSetup.cpp`** - Configure MSR permission map

---

## Risks & Mitigations

| Risk                                      | Mitigation                                          |
|-------------------------------------------|-----------------------------------------------------|
| TLB coherency issues (stale entries)      | Invalidate TLB after NPT modifications via INVLPGA  |
| Performance hit from NPT faults           | Minimize state transitions, pre-build NPT entries   |
| Hook detection via memory scanning        | Keep hooks on exec page only, invisible at rest     |
| EFER.SVME cleared by guest → crash        | Intercept EFER writes, inject #GP if SVME cleared   |
| MMIO access causing NPT fault loops       | Build NPT entries on-demand for invalid ranges      |
| Parent NX bit prevents child execution    | Implement cascading NX propagation (expensive)      |

---

## Cross-Reference: Intel Equivalents

For dual-CPU implementation, reference these Intel patterns:

- **NPT** → **EPT** (Refs/codebases/HyperPlatform/HyperPlatform/ept.cpp)
- **VMEXIT_NPF** → **EPT Violation** (HyperPlatform/vm.cpp)
- **ExitInfo1/2** → **Exit Qualification + GPA** (VMCS fields)
- **VMCB** → **VMCS** (different field layout)

---

**Pattern Extraction Complete**
**Next Step**: Review by vmexit-handler-agent for integration into Ombra's VMEXIT dispatch table
