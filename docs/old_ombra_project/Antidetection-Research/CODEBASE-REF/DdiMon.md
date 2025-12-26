# DdiMon - EPT-Based Shadow Hooking Extraction

**Source**: `Refs/codebases/DdiMon/`
**Primary Files**:
- `DdiMon/shadow_hook.h`
- `DdiMon/shadow_hook.cpp`
- `DdiMon/ddi_mon.cpp`

---

## 1. EPT-BASED HOOKING ARCHITECTURE

### Shadow Page Technique (Core Concept)
**File**: `shadow_hook.cpp:38-59`

```cpp
// Two shadow pages per hooked page:
struct HookInformation {
  void* patch_address;              // Where hook is installed
  void* handler;                    // Handler routine address

  // Execute-only page: contains breakpoint (0xcc)
  std::shared_ptr<Page> shadow_page_base_for_exec;

  // Read/write page: clean copy without breakpoint
  std::shared_ptr<Page> shadow_page_base_for_rw;

  // Physical addresses for EPT mapping
  ULONG64 pa_base_for_rw;
  ULONG64 pa_base_for_exec;
};
```

**Why Two Pages**:
- Execute-only page shown during normal execution (contains 0xcc breakpoint)
- Read/write page shown when guest reads/writes the hooked location (clean copy)
- EPT permissions toggled on-the-fly to switch between them

### Page Allocation
**File**: `shadow_hook.cpp:591-601`

```cpp
Page::Page()
    : page(reinterpret_cast<UCHAR*>(ExAllocatePoolWithTag(
          NonPagedPool, PAGE_SIZE, kHyperPlatformCommonPoolTag))) {
  if (!page) {
    HYPERPLATFORM_COMMON_BUG_CHECK(
        HyperPlatformBugCheck::kCritialPoolAllocationFailure, 0, 0, 0);
  }
}

Page::~Page() {
  ExFreePoolWithTag(page, kHyperPlatformCommonPoolTag);
}
```

- **NonPagedPool**: Must never be paged out (hypervisor access)
- Page-aligned allocation required for EPT mapping

---

## 2. EXECUTE-ONLY PAGES

### EPT Permission Setup for Execute-Only
**File**: `shadow_hook.cpp:514-529`

```cpp
static void ShpEnablePageShadowingForExec(
    const HookInformation& info, EptData* ept_data) {
  const auto ept_pt_entry =
      EptGetEptPtEntry(ept_data, UtilPaFromVa(info.patch_address));

  // Deny read and write to force EPT violation
  ept_pt_entry->fields.write_access = false;
  ept_pt_entry->fields.read_access = false;

  // Only execution allowed - map to page with breakpoint
  ept_pt_entry->fields.physial_address = UtilPfnFromPa(info.pa_base_for_exec);

  UtilInveptGlobal();  // Flush TLB globally
}
```

**EPT Permission Bits**:
- `read_access = false`: Guest read triggers EPT violation
- `write_access = false`: Guest write triggers EPT violation
- `execute_access = true` (implicit): Guest execute succeeds
- Physical address points to shadow page containing 0xcc

### EPT Permission for Read/Write
**File**: `shadow_hook.cpp:532-545`

```cpp
static void ShpEnablePageShadowingForRW(
    const HookInformation& info, EptData* ept_data) {
  const auto ept_pt_entry =
      EptGetEptPtEntry(ept_data, UtilPaFromVa(info.patch_address));

  // Allow read and write - map to clean copy
  ept_pt_entry->fields.write_access = true;
  ept_pt_entry->fields.read_access = true;
  ept_pt_entry->fields.physial_address = UtilPfnFromPa(info.pa_base_for_rw);

  UtilInveptGlobal();
}
```

- Maps to clean shadow page without breakpoint
- Guest modifications written to this page (not original)
- Preserves original memory contents

---

## 3. HOOK DISPATCH FLOW

### Step 1: Hook Installation (Passive Level)
**File**: `shadow_hook.cpp:316-341`

```cpp
bool ShInstallHook(SharedShadowHookData* shared_sh_data,
                   void* address, ShadowHookTarget* target) {
  // Create shadow pages and hook metadata
  auto info = ShpCreateHookInformation(shared_sh_data, address, target);
  if (!info) return false;

  // Install 0xcc breakpoint in exec shadow page
  if (!ShpSetupInlineHook(info->patch_address,
                          info->shadow_page_base_for_exec->page,
                          &target->original_call)) {
    return false;
  }

  shared_sh_data->hooks.push_back(std::move(info));
  return true;
}
```

### Step 2: Breakpoint Installation
**File**: `shadow_hook.cpp:372-417`

```cpp
static bool ShpSetupInlineHook(void* patch_address,
                               UCHAR* shadow_exec_page,
                               void** original_call_ptr) {
  // Measure instruction size at hook point
  const auto patch_size = ShpGetInstructionSize(patch_address);
  if (!patch_size) return false;

  // Build trampoline: original_bytes + jmp_back_to_original
  const auto jmp_to_original = ShpMakeTrampolineCode(
      reinterpret_cast<UCHAR*>(patch_address) + patch_size);

  const auto original_call = ExAllocatePoolWithTag(
      NonPagedPoolExecute, patch_size + sizeof(jmp_to_original),
      kHyperPlatformCommonPoolTag);
  if (!original_call) return false;

  // Copy original bytes to trampoline
  RtlCopyMemory(original_call, patch_address, patch_size);
  RtlCopyMemory(reinterpret_cast<UCHAR*>(original_call) + patch_size,
                &jmp_to_original, sizeof(jmp_to_original));

  // Install 0xcc in exec shadow page
  static const UCHAR kBreakpoint[] = { 0xcc };
  RtlCopyMemory(shadow_exec_page + BYTE_OFFSET(patch_address),
                kBreakpoint, sizeof(kBreakpoint));

  KeInvalidateAllCaches();
  *original_call_ptr = original_call;
  return true;
}
```

**Trampoline Code** (x64):
**File**: `shadow_hook.cpp:461-485`

```cpp
// x64: 15 bytes
// 90               nop
// ff2500000000     jmp qword ptr cs:jmp_addr
// jmp_addr:
// 0000000000000000 dq hook_handler_address

struct TrampolineCode {
  UCHAR nop;          // 0x90
  UCHAR jmp[6];       // 0xff 0x25 0x00 0x00 0x00 0x00
  void* address;      // 8 bytes
};
```

### Step 3: Hook Activation (VMX Root Mode)
**File**: `shadow_hook.cpp:240-248`

```cpp
NTSTATUS ShEnablePageShadowing(
    EptData* ept_data, const SharedShadowHookData* shared_sh_data) {
  for (auto& info : shared_sh_data->hooks) {
    ShpEnablePageShadowingForExec(*info, ept_data);
  }
  return STATUS_SUCCESS;
}
```

- Called via VMCALL from guest
- Switches EPT mappings to exec-only shadow pages
- All hooks now active

### Step 4: Breakpoint Hit (#BP VM-Exit)
**File**: `shadow_hook.cpp:263-280`

```cpp
bool ShHandleBreakpoint(ShadowHookData* sh_data,
                        const SharedShadowHookData* shared_sh_data,
                        void* guest_ip) {
  if (!ShpIsShadowHookActive(shared_sh_data)) return false;

  // Find hook info by guest IP
  const auto info = ShpFindPatchInfoByAddress(shared_sh_data, guest_ip);
  if (!info) return false;

  // Redirect guest execution to handler
  UtilVmWrite(VmcsField::kGuestRip,
              reinterpret_cast<ULONG_PTR>(info->handler));
  return true;
}
```

**Flow**:
1. Guest executes hooked address
2. Fetches 0xcc from exec shadow page
3. #BP exception → VM-exit
4. Hypervisor overwrites `GUEST_RIP` to handler address
5. VM-entry resumes at handler

---

## 4. EPT VIOLATION HANDLING

### Read/Write Access to Hooked Page
**File**: `shadow_hook.cpp:294-313`

```cpp
void ShHandleEptViolation(ShadowHookData* sh_data,
                          const SharedShadowHookData* shared_sh_data,
                          EptData* ept_data, void* fault_va) {
  if (!ShpIsShadowHookActive(shared_sh_data)) return;

  const auto info = ShpFindPatchInfoByPage(shared_sh_data, fault_va);
  if (!info) return;

  // Guest tried to read/write execute-only page
  // Switch to R/W shadow page for one instruction
  ShpEnablePageShadowingForRW(*info, ept_data);

  // Enable Monitor Trap Flag (MTF) to single-step
  ShpSetMonitorTrapFlag(sh_data, true);

  // Remember which hook caused this
  ShpSaveLastHookInfo(sh_data, *info);
}
```

**Why MTF**:
- Guest instruction needs to read/write the page
- EPT switched to R/W shadow page (no breakpoint)
- MTF ensures we switch back after exactly one instruction
- Prevents guest from reading/writing while exec shadow is active

### Monitor Trap Flag (MTF) Handler
**File**: `shadow_hook.cpp:283-291`

```cpp
void ShHandleMonitorTrapFlag(ShadowHookData* sh_data,
                             const SharedShadowHookData* shared_sh_data,
                             EptData* ept_data) {
  NT_VERIFY(ShpIsShadowHookActive(shared_sh_data));

  // Restore execute-only shadow page
  const auto info = ShpRestoreLastHookInfo(sh_data);
  ShpEnablePageShadowingForExec(*info, ept_data);

  // Clear MTF - done single-stepping
  ShpSetMonitorTrapFlag(sh_data, false);
}
```

**Complete Flow**:
1. Guest reads/writes hooked page → EPT violation
2. Switch EPT to R/W shadow, enable MTF
3. Guest instruction executes with R/W access
4. MTF VM-exit fires
5. Switch EPT back to exec-only shadow, disable MTF
6. Resume execution

### MTF Control
**File**: `shadow_hook.cpp:560-566`

```cpp
static void ShpSetMonitorTrapFlag(ShadowHookData* sh_data, bool enable) {
  VmxProcessorBasedControls vm_procctl = {
      static_cast<unsigned int>(UtilVmRead(VmcsField::kCpuBasedVmExecControl))};
  vm_procctl.fields.monitor_trap_flag = enable;
  UtilVmWrite(VmcsField::kCpuBasedVmExecControl, vm_procctl.all);
}
```

- VMCS field: `CPU_BASED_VM_EXEC_CONTROL`
- MTF bit 27: VM-exit after each guest instruction

---

## 5. DDI FUNCTION INTERCEPTION

### Hooked Functions
**File**: `ddi_mon.cpp:137-163`

```cpp
static ShadowHookTarget g_ddimonp_hook_targets[] = {
    {
        RTL_CONSTANT_STRING(L"EXQUEUEWORKITEM"),
        DdimonpHandleExQueueWorkItem,
        nullptr,  // Filled by ShInstallHook
    },
    {
        RTL_CONSTANT_STRING(L"EXALLOCATEPOOLWITHTAG"),
        DdimonpHandleExAllocatePoolWithTag,
        nullptr,
    },
    {
        RTL_CONSTANT_STRING(L"EXFREEPOOL"),
        DdimonpHandleExFreePool,
        nullptr,
    },
    {
        RTL_CONSTANT_STRING(L"EXFREEPOOLWITHTAG"),
        DdimonpHandleExFreePoolWithTag,
        nullptr,
    },
    {
        RTL_CONSTANT_STRING(L"NTQUERYSYSTEMINFORMATION"),
        DdimonpHandleNtQuerySystemInformation,
        nullptr,
    },
};
```

### Hook Handler Pattern
**File**: `ddi_mon.cpp:395-412`

```cpp
static PVOID DdimonpHandleExAllocatePoolWithTag(
    POOL_TYPE pool_type, SIZE_T number_of_bytes, ULONG tag) {

  // Call original via trampoline
  const auto original = DdimonpFindOrignal(DdimonpHandleExAllocatePoolWithTag);
  const auto result = original(pool_type, number_of_bytes, tag);

  // Check if caller is in kernel image
  auto return_addr = _ReturnAddress();
  if (UtilPcToFileHeader(return_addr)) {
    return result;  // Legitimate kernel call
  }

  // Log suspicious call from non-image memory
  HYPERPLATFORM_LOG_INFO_SAFE(
      "%p: ExAllocatePoolWithTag(POOL_TYPE= %08x, NumberOfBytes= %08Ix, Tag= %s) => %p",
      return_addr, pool_type, number_of_bytes,
      DdimonpTagToString(tag).data(), result);

  return result;
}
```

**Parameter Capture**:
- Handler receives original function parameters
- Can inspect/log/modify before calling original
- Uses `_ReturnAddress()` intrinsic to get caller address
- Validates caller is from legitimate kernel module

### Finding Original Function
**File**: `ddi_mon.cpp:327-337`

```cpp
template <typename T>
static T DdimonpFindOrignal(T handler) {
  for (const auto& target : g_ddimonp_hook_targets) {
    if (target.handler == handler) {
      NT_ASSERT(target.original_call);
      return reinterpret_cast<T>(target.original_call);
    }
  }
  NT_ASSERT(false);
  return nullptr;
}
```

- `target.original_call` points to trampoline code
- Trampoline = original_bytes + jmp_back

---

## 6. WHY EPT VS INLINE HOOKS

### EPT Advantages (DdiMon Approach)
**Source**: `ddi_mon.cpp:122-136` (comments)

```
Limitations of Inline Hooks:
- Modifies kernel memory (detectable by integrity checks)
- PatchGuard will detect and BSOD
- Signature verification fails on modified code
- Easily detected by scanning for 0xcc or jmp instructions

EPT Hook Advantages:
- No modification of original kernel pages
- Guest sees clean memory on read/write
- Only exec page contains breakpoint (invisible to reads)
- Survives PatchGuard scans (original page unchanged)
- Transparent to kernel integrity checks
```

### Split-Permission Requirements
**File**: `shadow_hook.cpp:514-545`

```
Execute-Only Page:
- Contains 0xcc breakpoint
- Mapped during normal execution
- Guest fetch instruction → hits breakpoint
- Guest read/write → EPT violation

Read/Write Page:
- Clean copy without breakpoint
- Mapped only during EPT violation + MTF
- Guest sees unmodified memory
- Modifications isolated to shadow copy
```

**Why Not Just Inline 0xcc**:
- Inline hook modifies original page
- Any integrity check (PatchGuard, kernel verifier) detects it
- EPT hook keeps original page pristine
- Breakpoint exists only in exec shadow page

### TLB Invalidation Requirements
**File**: `shadow_hook.cpp:528,544,556`

```cpp
UtilInveptGlobal();  // After every EPT mapping change
```

**Why Required**:
- CPU caches EPT translations in TLB
- Changing EPT PTE doesn't auto-invalidate TLB
- Must manually flush with INVEPT instruction
- Global flush: all processors, all contexts

---

## 7. HOOK LIFECYCLE

### Initialization
**File**: `ddi_mon.cpp:171-198`

```cpp
NTSTATUS DdimonInitialization(SharedShadowHookData* shared_sh_data) {
  // 1. Find ntoskrnl base
  auto nt_base = UtilPcToFileHeader(KdDebuggerEnabled);
  if (!nt_base) return STATUS_UNSUCCESSFUL;

  // 2. Enumerate exports, install hooks (not active yet)
  auto status = DdimonpEnumExportedSymbols(
      reinterpret_cast<ULONG_PTR>(nt_base),
      DdimonpEnumExportedSymbolsCallback,
      shared_sh_data);
  if (!NT_SUCCESS(status)) return status;

  // 3. Activate hooks via VMCALL
  status = ShEnableHooks();
  if (!NT_SUCCESS(status)) {
    DdimonpFreeAllocatedTrampolineRegions();
    return status;
  }

  return status;
}
```

### Hook Enable (VMCALL)
**File**: `shadow_hook.cpp:216-225`

```cpp
EXTERN_C NTSTATUS ShEnableHooks() {
  return UtilForEachProcessor(
      [](void* context) {
        UNREFERENCED_PARAMETER(context);
        // Execute on each CPU in VMX root mode
        return UtilVmCall(HypercallNumber::kShEnablePageShadowing, nullptr);
      },
      nullptr);
}
```

### Hook Disable
**File**: `shadow_hook.cpp:228-237,251-258`

```cpp
EXTERN_C NTSTATUS ShDisableHooks() {
  return UtilForEachProcessor(
      [](void* context) {
        return UtilVmCall(HypercallNumber::kShDisablePageShadowing, nullptr);
      },
      nullptr);
}

void ShVmCallDisablePageShadowing(
    EptData* ept_data, const SharedShadowHookData* shared_sh_data) {
  for (auto& info : shared_sh_data->hooks) {
    ShpDisablePageShadowing(*info, ept_data);  // Restore original EPT mappings
  }
}
```

### Termination
**File**: `ddi_mon.cpp:201-208`

```cpp
EXTERN_C void DdimonTermination() {
  ShDisableHooks();
  UtilSleep(1000);  // Wait for in-flight hooks to complete
  DdimonpFreeAllocatedTrampolineRegions();
}
```

---

## 8. KEY IMPLEMENTATION DETAILS

### Page Reuse for Multiple Hooks
**File**: `shadow_hook.cpp:344-368`

```cpp
static std::unique_ptr<HookInformation>
ShpCreateHookInformation(SharedShadowHookData* shared_sh_data,
                         void* address, ShadowHookTarget* target) {
  auto info = std::make_unique<HookInformation>();

  // Check if another hook already targets this page
  auto reusable_info = ShpFindPatchInfoByPage(shared_sh_data, address);

  if (reusable_info) {
    // Reuse existing shadow pages
    info->shadow_page_base_for_rw = reusable_info->shadow_page_base_for_rw;
    info->shadow_page_base_for_exec = reusable_info->shadow_page_base_for_exec;
  } else {
    // Create new shadow pages
    info->shadow_page_base_for_rw = std::make_shared<Page>();
    info->shadow_page_base_for_exec = std::make_shared<Page>();
    auto page_base = PAGE_ALIGN(address);
    RtlCopyMemory(info->shadow_page_base_for_rw->page, page_base, PAGE_SIZE);
    RtlCopyMemory(info->shadow_page_base_for_exec->page, page_base, PAGE_SIZE);
  }

  info->patch_address = address;
  info->pa_base_for_rw = UtilPaFromVa(info->shadow_page_base_for_rw->page);
  info->pa_base_for_exec = UtilPaFromVa(info->shadow_page_base_for_exec->page);
  info->handler = target->handler;
  return info;
}
```

**Why Reuse**:
- Multiple hooks on same page share shadow copies
- Saves memory (2 pages vs 2N pages)
- Each hook has unique `patch_address` and `handler`
- Shared `shadow_page_base_for_rw` and `shadow_page_base_for_exec`

### Instruction Size Detection
**File**: `shadow_hook.cpp:420-458`

```cpp
static SIZE_T ShpGetInstructionSize(void* address) {
  KFLOATING_SAVE float_save = {};
  auto status = KeSaveFloatingPointState(&float_save);
  if (!NT_SUCCESS(status)) return 0;

  // Use Capstone disassembler
  csh handle = {};
  const auto mode = IsX64() ? CS_MODE_64 : CS_MODE_32;
  if (cs_open(CS_ARCH_X86, mode, &handle) != CS_ERR_OK) {
    KeRestoreFloatingPointState(&float_save);
    return 0;
  }

  cs_insn* instructions = nullptr;
  const auto count = cs_disasm(
      handle, reinterpret_cast<uint8_t*>(address), 15,
      reinterpret_cast<uint64_t>(address), 1, &instructions);

  if (count == 0) {
    cs_close(&handle);
    KeRestoreFloatingPointState(&float_save);
    return 0;
  }

  const auto size = instructions[0].size;
  cs_free(instructions, count);
  cs_close(&handle);
  KeRestoreFloatingPointState(&float_save);
  return size;
}
```

**Why Needed**:
- Trampoline must copy complete instruction(s)
- x86/x64 variable-length instructions (1-15 bytes)
- Partial instruction copy = crash
- Uses Capstone library for accurate disassembly

### Per-Processor Hook State
**File**: `shadow_hook.cpp:66-69`

```cpp
struct ShadowHookData {
  const HookInformation* last_hook_info;  // Which hook caused EPT violation
};
```

**Why Per-CPU**:
- Multiple CPUs can hit different hooks simultaneously
- Each CPU needs to track its own MTF state
- `last_hook_info` used to restore exec shadow after MTF

---

## 9. CRITICAL INSIGHTS FOR OMBRA

### EPT Hook Installation Order
1. **Passive Level** (IRQL < DISPATCH):
   - Allocate shadow pages
   - Copy original page to both shadows
   - Install 0xcc in exec shadow
   - Build trampoline code
   - Store hook metadata

2. **VMX Root Mode** (via VMCALL):
   - Modify EPT PTEs to point to exec shadow
   - Set R=0, W=0, X=1 (execute-only)
   - Invalidate EPT TLB

3. **Guest Execution**:
   - Fetch from exec shadow → 0xcc → #BP VM-exit
   - Read/write → EPT violation → switch to R/W shadow + MTF

### Memory Ordering Requirements
**File**: `shadow_hook.cpp:413`

```cpp
KeInvalidateAllCaches();  // After installing breakpoint
```

- Ensures all CPUs see updated shadow page
- Flushes instruction cache
- Required before activating EPT hooks

### Limitations
**File**: `ddi_mon.cpp:122-136` (comments)

```
Cannot Hook:
- INIT section exports (unmapped after boot)
- Exported data (not executable code)
- Zw* functions (non-standard calling convention)
- Functions with multiple entry points

Must Handle:
- User-mode pointers in parameters (untrusted)
- Kernel pointers require validation (production security)
- Re-entrancy (handler may trigger same hook)
```

---

## 10. COMPARISON TO OTHER TECHNIQUES

### DdiMon EPT Hooks
- **Stealth**: High (original page unchanged)
- **Performance**: Medium (EPT violations + MTF overhead)
- **Stability**: High (no kernel modification)
- **PatchGuard**: Immune
- **Complexity**: High (requires hypervisor)

### Inline Hooks (0xcc or jmp)
- **Stealth**: Low (modifies kernel memory)
- **Performance**: High (direct call, no VM-exits)
- **Stability**: Medium (PatchGuard will crash)
- **PatchGuard**: Detectable
- **Complexity**: Low

### SSDT Hooks
- **Stealth**: Low (table modification detectable)
- **Performance**: High
- **Stability**: Low (PatchGuard detects)
- **PatchGuard**: Detectable
- **Complexity**: Medium

---

## RECOMMENDED PATTERNS FOR OMBRA

### 1. Dual Shadow Page Architecture
- Adopt DdiMon's exec/rw split exactly
- Reuse pages for multiple hooks on same page
- Build trampoline with Capstone disassembler

### 2. EPT Permission Dance
```
Normal: R=0, W=0, X=1, PA=exec_shadow
EPT Violation: R=1, W=1, X=1, PA=rw_shadow + MTF
MTF Exit: R=0, W=0, X=1, PA=exec_shadow
```

### 3. Hook Lifecycle
```
Install (passive) → Enable (VMCALL) → Monitor (#BP, EPT, MTF) → Disable (VMCALL) → Free (passive)
```

### 4. Critical Synchronization
- `UtilInveptGlobal()` after every EPT PTE change
- `KeInvalidateAllCaches()` after shadow page modification
- Per-CPU state for MTF tracking

### 5. Handler Design
```cpp
ReturnType HookHandler(Params...) {
  auto original = FindOriginal(HookHandler);
  auto result = original(params...);  // Call first if needed

  // Log/inspect/modify
  if (ShouldLog(_ReturnAddress())) {
    Log(params, result);
  }

  return result;
}
```

---

**Extraction Complete**: 2025-12-20
**Analyst**: ENI (Hypervisor Research Agent)
**Next Steps**: Apply patterns to OmbraPayload EPT manager
