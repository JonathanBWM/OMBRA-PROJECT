# InfinityHook - ETW-Based Syscall Hooking Pattern Extraction

**Source**: `Refs/codebases/InfinityHook/`
**Extracted**: 2025-12-20
**Purpose**: ETW abuse for PatchGuard-safe syscall interception without kernel modification

---

## 1. ETW HOOK MECHANISM

### Core Concept: Hijacking GetCpuClock Callback
- **Target**: `_WMI_LOGGER_CONTEXT.GetCpuClock` function pointer at offset +0x28
- **Source**: `infinityhook.cpp:311-321`
  ```c
  // We care about overwriting the GetCpuClock (+0x28) pointer in this structure.
  PVOID* AddressOfEtwpGetCycleCount = (PVOID*)((uintptr_t)CkclWmiLoggerContext + OFFSET_WMI_LOGGER_CONTEXT_CPU_CYCLE_CLOCK);

  // Replace this function pointer with our own. Each time syscall is logged by ETW, it will invoke our new timing function.
  *AddressOfEtwpGetCycleCount = IfhpInternalGetCpuClock;
  ```
- **Offset Definition**: `infinityhook.cpp:63`
  ```c
  #define OFFSET_WMI_LOGGER_CONTEXT_CPU_CYCLE_CLOCK 0x28
  ```

### Circular Kernel Context Logger (CKCL) Resolution
- **CKCL Index**: `infinityhook.cpp:88`
  ```c
  #define INDEX_CKCL_LOGGER 2
  ```
- **EtwpDebuggerData Pattern Search**: `infinityhook.cpp:51-58, 376-394`
  ```c
  // Works from Windows 7+. Signature in .data or .rdata section
  UCHAR EtwpDebuggerDataPattern[] = { 0x2c, 0x08, 0x04, 0x38, 0x0c };

  // Search .data section first
  EtwpDebuggerData = MmSearchMemory(SectionBase, SizeOfSection, EtwpDebuggerDataPattern, RTL_NUMBER_OF(EtwpDebuggerDataPattern));

  // Fallback to .rdata for Windows 7
  if (!EtwpDebuggerData) {
      SectionBase = ImgGetImageSection(NtBaseAddress, ".rdata", &SizeOfSection);
      EtwpDebuggerData = MmSearchMemory(SectionBase, SizeOfSection, EtwpDebuggerDataPattern, RTL_NUMBER_OF(EtwpDebuggerDataPattern));
  }

  // Adjust for signature offset
  EtwpDebuggerData = (PVOID)((uintptr_t)EtwpDebuggerData - 2);
  ```

- **Extracting CKCL Context**: `infinityhook.cpp:404-409`
  ```c
  #define OFFSET_ETW_DEBUGGER_DATA_SILO 0x10

  // Get the silos of EtwpDebuggerData.
  PVOID* EtwpDebuggerDataSilo = *(PVOID**)((uintptr_t)EtwpDebuggerData + OFFSET_ETW_DEBUGGER_DATA_SILO);

  // Pull out the circular kernel context logger.
  CkclWmiLoggerContext = EtwpDebuggerDataSilo[INDEX_CKCL_LOGGER];
  ```

### WMI Trace Registration & Control
- **GUID Definition**: `ntint.h:393`
  ```c
  /* 54dea73a-ed1f-42a4-af713e63d056f174 */
  const GUID CkclSessionGuid = { 0x54dea73a, 0xed1f, 0x42a4, { 0xaf, 0x71, 0x3e, 0x63, 0xd0, 0x56, 0xf1, 0x74 } };
  ```

- **Trace Property Structure**: `infinityhook.cpp:34-38`
  ```c
  typedef struct _CKCL_TRACE_PROPERIES: EVENT_TRACE_PROPERTIES {
      ULONG64 Unknown[3];
      UNICODE_STRING ProviderName;
  } CKCL_TRACE_PROPERTIES, *PCKCL_TRACE_PROPERTIES;
  ```

- **Starting CKCL Session**: `infinityhook.cpp:429-475`
  ```c
  // Setup trace properties
  Property->Wnode.BufferSize = PAGE_SIZE;
  Property->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
  Property->ProviderName = RTL_CONSTANT_STRING(L"Circular Kernel Context Logger");
  Property->Wnode.Guid = CkclSessionGuid;
  Property->Wnode.ClientContext = 1;
  Property->BufferSize = sizeof(ULONG);
  Property->MinimumBuffers = Property->MaximumBuffers = 2;
  Property->LogFileMode = EVENT_TRACE_BUFFERING_MODE;

  // Enable syscall tracing
  case CKCL_TRACE_SYSCALL:
      Property->EnableFlags = EVENT_TRACE_FLAG_SYSTEMCALL;  // 0x00000080
      Status = ZwTraceControl(EtwpUpdateTrace, Property, PAGE_SIZE, Property, PAGE_SIZE, &ReturnLength);
  ```

- **Trace Operations**: `infinityhook.cpp:24-29, ntint.h:19-23`
  ```c
  enum CKCL_TRACE_OPERATION {
      CKCL_TRACE_START,
      CKCL_TRACE_SYSCALL,
      CKCL_TRACE_END
  };

  #define EtwpStartTrace   1
  #define EtwpStopTrace    2
  #define EtwpUpdateTrace  4
  ```

---

## 2. SYSCALL INTERCEPTION

### GetCpuClock Replacement Callback
- **Callback Signature**: `infinityhook.h:22`
  ```c
  typedef void (__fastcall* INFINITYHOOKCALLBACK)(_In_ unsigned int SystemCallIndex, _Inout_ void** SystemCallFunction);
  ```

- **Main Interception Logic**: `infinityhook.cpp:490-567`
  ```c
  static ULONG64 IfhpInternalGetCpuClock() {
      // Skip kernel-mode calls
      if (ExGetPreviousMode() == KernelMode) {
          return __rdtsc();
      }

      // Extract system call index from KTHREAD
      PKTHREAD CurrentThread = (PKTHREAD)__readgsqword(OFFSET_KPCR_CURRENT_THREAD);
      unsigned int SystemCallIndex = *(unsigned int*)((uintptr_t)CurrentThread + OFFSET_KTHREAD_SYSTEM_CALL_NUMBER);

      // Get stack boundaries
      PVOID* StackMax = (PVOID*)__readgsqword(OFFSET_KPCR_RSP_BASE);
      PVOID* StackFrame = (PVOID*)_AddressOfReturnAddress();

      // Walk stack backwards to find magic values
      for (PVOID* StackCurrent = StackMax; StackCurrent > StackFrame; --StackCurrent) {
          PULONG AsUlong = (PULONG)StackCurrent;
          if (*AsUlong != INFINITYHOOK_MAGIC_1) continue;

          --StackCurrent;
          PUSHORT AsShort = (PUSHORT)StackCurrent;
          if (*AsShort != INFINITYHOOK_MAGIC_2) continue;

          // Found magic - now walk forward to find syscall entry return address
          for (; StackCurrent < StackMax; ++StackCurrent) {
              PULONGLONG AsUlonglong = (PULONGLONG)StackCurrent;

              // Check if value points to syscall entry page (KiSystemCall64 or KiSystemServiceUser)
              if (!(PAGE_ALIGN(*AsUlonglong) >= SystemCallEntryPage &&
                    PAGE_ALIGN(*AsUlonglong) < (PVOID)((uintptr_t)SystemCallEntryPage + (PAGE_SIZE * 2)))) {
                  continue;
              }

              // StackCurrent[9] contains the syscall function pointer
              void** SystemCallFunction = &StackCurrent[9];

              if (IfhpCallback) {
                  IfhpCallback(SystemCallIndex, SystemCallFunction);
              }
              break;
          }
          break;
      }

      return __rdtsc();
  }
  ```

### Critical Offsets & Magic Values
- **KPCR Offsets**: `infinityhook.cpp:68-78`
  ```c
  #define OFFSET_KPCR_RSP_BASE 0x1A8              // _KPCR.Prcb.RspBase
  #define OFFSET_KPCR_CURRENT_THREAD 0x188        // _KPCR.Prcb.CurrentThread
  #define OFFSET_KTHREAD_SYSTEM_CALL_NUMBER 0x80  // _KTHREAD.SystemCallNumber
  ```

- **Stack Magic Values**: `infinityhook.cpp:94-95`
  ```c
  #define INFINITYHOOK_MAGIC_1 ((ULONG)0x501802)
  #define INFINITYHOOK_MAGIC_2 ((USHORT)0xF33)
  ```

### Syscall Entry Page Resolution
- **KiSystemCall64 / KiSystemServiceUser Lookup**: `img.cpp:156-229`
  ```c
  PVOID ImgGetSyscallEntry() {
      PVOID NtBaseAddress = ImgGetBaseAddress(NULL, NULL);

      // Read LSTAR MSR (IA32_LSTAR_MSR = 0xC0000082)
      PVOID SyscallEntry = (PVOID)__readmsr(IA32_LSTAR_MSR);

      // Check if KVA shadowing exists (KVASCODE section)
      PVOID SectionBase = ImgGetImageSection(NtBaseAddress, "KVASCODE", &SizeOfSection);
      if (!SectionBase) {
          return SyscallEntry;  // No KVA shadowing
      }

      // If LSTAR points to KVASCODE, it's KiSystemCall64Shadow
      if (!(SyscallEntry >= SectionBase && SyscallEntry < (PVOID)((uintptr_t)SectionBase + SizeOfSection))) {
          return SyscallEntry;
      }

      // Disassemble KiSystemCall64Shadow to find jmp to KiSystemServiceUser
      hde64s HDE;
      for (PCHAR KiSystemServiceUser = (PCHAR)SyscallEntry; ; KiSystemServiceUser += HDE.len) {
          if (!hde64_disasm(KiSystemServiceUser, &HDE)) break;

          if (HDE.opcode != OPCODE_JMP_NEAR) continue;  // 0xE9

          PVOID PossibleSyscallEntry = (PVOID)((intptr_t)KiSystemServiceUser + (int)HDE.len + (int)HDE.imm.imm32);

          // Ignore jmps within KVASCODE
          if (PossibleSyscallEntry >= SectionBase && PossibleSyscallEntry < (PVOID)((uintptr_t)SectionBase + SizeOfSection)) {
              continue;
          }

          SyscallEntry = PossibleSyscallEntry;  // Found KiSystemServiceUser
          break;
      }

      return SyscallEntry;
  }
  ```

- **Page Alignment**: `infinityhook.cpp:414`
  ```c
  SystemCallEntryPage = PAGE_ALIGN(ImgGetSyscallEntry());
  ```

### Parameter Access & Return Value Modification
- **Callback Implementation Example**: `entry.cpp:90-116`
  ```c
  void __fastcall SyscallStub(_In_ unsigned int SystemCallIndex, _Inout_ void** SystemCallFunction) {
      // SystemCallIndex contains the syscall number from KTHREAD
      // SystemCallFunction is a pointer to the stack slot containing the function pointer

      if (*SystemCallFunction == OriginalNtCreateFile) {
          // Replace the function pointer on the stack to redirect execution
          *SystemCallFunction = DetourNtCreateFile;
      }
  }
  ```

- **Detour Function Example**: `entry.cpp:122-175`
  ```c
  NTSTATUS DetourNtCreateFile(_Out_ PHANDLE FileHandle, _In_ ACCESS_MASK DesiredAccess, ...) {
      // Access all original syscall parameters
      // Implement custom logic

      if (wcsstr(ObjectName, IfhMagicFileName)) {
          // Deny access or modify behavior
          return STATUS_ACCESS_DENIED;
      }

      // Call original function with original parameters
      return OriginalNtCreateFile(FileHandle, DesiredAccess, ObjectAttributes, ...);
  }
  ```

---

## 3. PATCHGUARD SAFETY

### Why This Avoids PatchGuard
1. **No Kernel Code Modification**: `infinityhook.cpp:311-321`
   - Only modifies **data structure** (`_WMI_LOGGER_CONTEXT.GetCpuClock` pointer)
   - Does NOT patch SSDT, IDT, GDT, or any kernel code pages
   - Does NOT hook KiSystemCall64 or modify syscall entry points

2. **Legitimate ETW Infrastructure Abuse**:
   - Uses officially exported `ZwTraceControl` API: `ntint.h:399-406`
   - Operates within documented ETW trace properties
   - CKCL session is a legitimate Windows component

3. **Stack-Based Interception**:
   - Reads syscall index from `KTHREAD.SystemCallNumber` (offset +0x80)
   - Walks stack to find return address pointing to syscall entry
   - Modifies **stack memory** (StackCurrent[9]), not kernel code

4. **No Critical Structure Modification**:
   - `_WMI_LOGGER_CONTEXT` is not PatchGuard-protected
   - GetCpuClock pointer is intended to be customizable for ETW providers
   - Changes appear as legitimate ETW timing customization

### Cleanup & Restoration
- **Graceful Teardown**: `infinityhook.cpp:332-345`
  ```c
  void IfhRelease() {
      if (!IfhpInitialized) return;

      // Stop CKCL session (restores original GetCpuClock)
      if (NT_SUCCESS(IfhpModifyTraceSettings(CKCL_TRACE_END))) {
          IfhpModifyTraceSettings(CKCL_TRACE_START);
      }

      IfhpInitialized = false;
  }
  ```

### Limitations & Detection Risks
1. **ETW-Dependent**:
   - Requires CKCL session to be active/startable
   - Fails if ETW is disabled or restricted

2. **Kernel Structure Dependency**:
   - Hardcoded offsets (KTHREAD, KPCR, WMI_LOGGER_CONTEXT)
   - Requires signature scanning for `EtwpDebuggerData`
   - Breaks across Windows versions if offsets change

3. **Detection Vectors**:
   - `ZwTraceControl` calls with syscall tracing flags
   - Modified `GetCpuClock` pointer in CKCL context
   - Performance anomalies (RDTSC on every usermode syscall)

---

## Implementation Notes for Ombra

### Applicability to Hypervisor Context
- **NOT DIRECTLY APPLICABLE**: InfinityHook operates in **kernel mode** without virtualization
- **Conceptual Value**: Demonstrates syscall interception without modifying kernel code pages
- **Hypervisor Alternative**: Use EPT hooks on SSDT or syscall entry points with RWX permission split

### Useful Patterns to Extract
1. **Syscall Entry Resolution** (`img.cpp:156-229`):
   - Hypervisor needs to locate `KiSystemCall64` / `KiSystemServiceUser` for VMCS setup
   - KVA shadow detection logic applicable

2. **Stack Walking Technique** (`infinityhook.cpp:509-564`):
   - Could be adapted for VMExit handler to identify syscall context
   - Magic value approach useful for filtering spurious exits

3. **Offset Management**:
   - Shows which KTHREAD/KPCR offsets are critical for syscall tracking
   - Hypervisor can use same offsets to read guest syscall numbers

### Ombra Recommendation
- **Do NOT implement ETW hooking in hypervisor**
- **DO leverage**:
  - Syscall entry page resolution logic
  - KTHREAD.SystemCallNumber offset for VMExit filtering
  - Stack walking pattern for context identification
- **Alternative**: EPT-based SSDT/syscall entry hooking from hypervisor context

---

## Cross-Reference Files
- **Core Implementation**: `infinityhook.cpp`
- **Header Definitions**: `infinityhook.h`, `ntint.h`
- **Helper Functions**: `img.cpp` (syscall entry resolution), `mm.cpp` (pattern search)
- **Example Usage**: `entry.cpp` (NtCreateFile hook demonstration)

**End of Extraction**
