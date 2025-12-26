# OMBRA V3 TIMING BYPASS IMPLEMENTATION
## Comprehensive Agent Analysis & Implementation Prompt

---

## MISSION OBJECTIVE

You are tasked with implementing Intel/AMD hypervisor timing bypass mechanisms into the Ombra-Hypervisor codebase. This is a multi-phase operation requiring deep understanding of both the existing codebase and the research documents provided.

**Architecture:** Ombra V3 uses **runtime Hyper-V hijacking** via ZeroHVCI kernel exploits (CVE-2024-26229 / CVE-2024-35250). There is NO bootkit. The payload is injected into the running hvix64.exe/hvax64.exe at runtime while Windows is fully operational.

**Primary Goal:** Make Ombra undetectable by BattlEye/EasyAntiCheat timing-based hypervisor detection on Intel systems while completing AMD SVM support.

**Success Criteria:**
- CPUID timing tests pass (average < 750 cycles)
- APERF/MPERF timing tests pass
- VMX instruction injection (#UD) working
- Trap flag handling correct
- AMD SVM parity with Intel VMX

---

## ARCHITECTURE OVERVIEW

```
┌─────────────────────────────────────────────────────────────────────┐
│                    OMBRA V3 RUNTIME ARCHITECTURE                     │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ┌──────────────────┐     ┌──────────────────┐                      │
│  │  OmbraClient.exe │────▶│  ZeroHVCI Exploit │                     │
│  │  (Usermode)      │     │  (Kernel R/W)     │                     │
│  └──────────────────┘     └────────┬─────────┘                      │
│                                    │                                 │
│                                    ▼                                 │
│                    ┌───────────────────────────────┐                │
│                    │  Runtime Hyper-V Hijacking    │                │
│                    │  - Find hvix64.exe base       │                │
│                    │  - Locate VMexit handler      │                │
│                    │  - Inject PayLoad to .rsrc    │                │
│                    │  - Patch CALL instruction     │                │
│                    └───────────────┬───────────────┘                │
│                                    │                                 │
│                                    ▼                                 │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                    PayLoad (Ring -1)                         │   │
│  │  - VMexit interception                                       │   │
│  │  - TSC timing compensation                                   │   │
│  │  - CPUID backdoor commands                                   │   │
│  │  - Memory read/write via EPT                                 │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                      │
│  Key Advantage: Works with Secure Boot ON, VBS/HVCI ON              │
│  (Exploits bypass these protections at runtime)                      │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## PHASE 0: COMPLETE CODEBASE ANALYSIS

### Step 0.1: Directory Structure Mapping

First, execute a complete recursive directory listing of the Ombra-Hypervisor codebase:

```bash
find /Users/jonathanmcclintock/Desktop/Projects/Ombra/Ombra-Hypervisor-V3/Ombra-Hypervisor -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.asm" -o -name "*.c" -o -name "*.hpp" -o -name "*.inc" \) | head -200
```

Then create a mental map of:
- Solution structure (.sln, .vcxproj files)
- Component boundaries (PayLoad, ZeroHVCI, OmbraClient, etc.)
- Build configurations (Debug/Release, x64 only)

### Step 0.2: Critical File Deep Reads

You MUST read and fully understand these files before proceeding:

**PayLoad (Ring -1 Hypervisor Code):**
```
PayLoad/Entry.cpp              - VMexit handler entry point (CRITICAL)
PayLoad/EntryAsm.asm           - Assembly entry stub (CRITICAL)
PayLoad/Context.h              - OMBRA_SPUTNIK_CONTEXT structure
PayLoad/Virt/Intel/Vmcs.cpp    - VMCS field definitions
PayLoad/Virt/Intel/Vmcs.h      - Intel VMX structures
PayLoad/Memory/Paging/Ept.cpp  - EPT implementation
PayLoad/Memory/Paging/Ept.h    - EPT structures
PayLoad/Memory/Paging/Npt.cpp  - NPT stubs (needs completion)
```

**ZeroHVCI / Runtime Injector:**
```
ZeroHVCI/Exploit.cpp           - CVE exploit for kernel R/W
ZeroHVCI/HvHijack.cpp          - Hyper-V runtime patching (CRITICAL)
ZeroHVCI/PatternScan.cpp       - VMexit handler pattern matching
```

**OmbraClient (Usermode Interface):**
```
OmbraClient/Main.cpp           - Client entry point
OmbraClient/Communication.cpp  - CPUID backdoor communication
```

### Step 0.3: Specific Code Analysis Questions

As you read, document answers to these questions:

1. **Entry.cpp Analysis:**
   - What is the exact function signature of `HyperVBackdoor`?
   - How is `GuestState` structured? Is it direct pointer or pointer-to-pointer?
   - What backdoor commands exist? (CMD_INFO, CMD_PING, etc.)
   - How is the original Hyper-V handler called?
   - Is there ANY timing compensation currently? (likely no)
   - How is CPU vendor detected (if at all) in the payload?

2. **EntryAsm.asm Analysis:**
   - What registers are saved/restored?
   - What POST codes are emitted?
   - How does control flow to the C++ handler?
   - Where would TSC capture fit in?

3. **HvHijack.cpp Analysis:**
   - How is hvix64.exe base address located at runtime?
   - How is the .rsrc section hijacked?
   - How is the VMexit CALL pattern found?
   - What is the exact byte pattern searched for?
   - How are relocations applied?
   - Where is OMBRA_SPUTNIK_CONTEXT stored?

4. **VMCS/Vmcs.h Analysis:**
   - What VMCS field encodings are defined?
   - Is VMCS_TSC_OFFSET defined?
   - Are MSR bitmap fields defined?
   - Are secondary processor controls defined?

5. **Context.h Analysis:**
   - What fields exist in OMBRA_SPUTNIK_CONTEXT?
   - Is CpuVendor field present?
   - Is there space for timing state?

6. **Npt.cpp Analysis:**
   - What functions are stubbed?
   - What is the NPT operations structure?
   - What AMD support is missing?

### Step 0.4: Build System Analysis

```bash
# Find all project files
find /Users/jonathanmcclintock/Desktop/Projects/Ombra/Ombra-Hypervisor-V3/Ombra-Hypervisor -name "*.vcxproj" -o -name "*.sln"

# Check preprocessor defines
grep -r "OMBRA_INTEL\|OMBRA_AMD\|CPU_VENDOR" /Users/jonathanmcclintock/Desktop/Projects/Ombra/Ombra-Hypervisor-V3/Ombra-Hypervisor --include="*.h" --include="*.cpp"
```

Document:
- How are Intel/AMD paths differentiated (if at all)?
- Are there compile-time switches?
- What external dependencies exist?

---

## PHASE 1: RESEARCH DOCUMENT ANALYSIS

### Step 1.1: Read All Six Documents

You MUST read these documents in the specified order (they build on each other):

```
/Users/jonathanmcclintock/Desktop/Projects/Ombra/Ombra-Hypervisor-V3/hypervisor-on-steroids/
├── 1. OMBRA_HYPERVISOR_STEALTH_STRATEGY.md    - High-level strategy overview
├── 2. TSC_EMULATION_IMPLEMENTATION.md          - Low-level TSC manipulation
├── 3. DETECTION_VECTORS_AND_MITIGATIONS.md     - All detection methods + fixes
├── 4. VMFUNC_VE_IMPLEMENTATION.md              - Future zero-exit hooks
├── 5. HYPERV_RUNTIME_HIJACKING.md              - Runtime injection method (PRIMARY)
└── 6. OMBRA_V3_TIMING_BYPASS_ROADMAP.md        - Integration roadmap
```

### Step 1.2: Extract Key Implementation Details

From each document, extract and document:

**From OMBRA_HYPERVISOR_STEALTH_STRATEGY.md:**
- List of all detection vectors
- Mitigation priority order
- Alternative approaches (memhv, DMA, AMD-only)

**From TSC_EMULATION_IMPLEMENTATION.md:**
- VMCS_TSC_OFFSET field encoding
- Per-VCPU timing state structure
- Compensation calculation formula
- Calibration values (Intel: 1200, AMD: 1600)

**From DETECTION_VECTORS_AND_MITIGATIONS.md:**
- Complete detection vector matrix with severity
- XSETBV validation logic
- Invalid MSR handling
- LBR save/restore
- WoW64 descriptor table fix
- VMX instruction #UD injection code

**From VMFUNC_VE_IMPLEMENTATION.md:**
- CPUID feature detection for VMFUNC/VE
- EPTP list structure
- Hardware requirements (Haswell+, Broadwell+)
- Performance comparison data

**From HYPERV_RUNTIME_HIJACKING.md:**
- ZeroHVCI integration approach
- hvix64.exe pattern scanning
- Shellcode structure for hook
- Custom hypercall definitions
- TSC compensation shellcode

**From OMBRA_V3_TIMING_BYPASS_ROADMAP.md:**
- Phase-by-phase implementation order
- File modification list
- AMD VMCB structures
- Self-test command additions

### Step 1.3: Identify Conflicts or Gaps

Cross-reference the documents and identify:
- Any conflicting recommendations
- Missing implementation details
- Assumptions that need validation
- Dependencies between components

---

## PHASE 2: GAP ANALYSIS

### Step 2.1: Create Mapping Matrix

Create a matrix mapping:
- Research document recommendations → Current codebase locations
- Required new files → Where they should go
- Existing code to modify → Specific line numbers

Example format:
```
| Feature                  | Document Source                | Codebase Location           | Status    |
|--------------------------|--------------------------------|-----------------------------|-----------|
| TSC compensation         | TSC_EMULATION_IMPLEMENTATION   | Entry.cpp (none exists)     | NEW       |
| TIMING_STATE struct      | TSC_EMULATION_IMPLEMENTATION   | Context.h                   | ADD FIELD |
| ApplyTscCompensation()   | OMBRA_V3_ROADMAP               | Entry.cpp                   | NEW FUNC  |
| AMD vendor dispatch      | OMBRA_V3_ROADMAP               | Entry.cpp line ~50          | MODIFY    |
| VMCB structures          | OMBRA_V3_ROADMAP               | Virt/Amd/Vmcb.h             | NEW FILE  |
| MSR bitmap modification  | TSC_EMULATION_IMPLEMENTATION   | Entry.cpp                   | NEW FUNC  |
| VMX #UD injection        | DETECTION_VECTORS              | Entry.cpp                   | NEW CASE  |
```

### Step 2.2: Dependency Graph

Create a dependency graph showing:
- Which changes must happen first
- Which changes can be parallelized
- Which changes have external dependencies

### Step 2.3: Risk Assessment

For each change, assess:
- **Stability risk:** Could this cause crashes/BSODs?
- **Detection risk:** Could this make things MORE detectable?
- **Compatibility risk:** Does this break existing functionality?
- **Testing difficulty:** How hard is this to verify?

---

## PHASE 3: IMPLEMENTATION PLAN GENERATION

### Step 3.1: File-by-File Change Specifications

For EVERY file that needs modification, create a detailed spec:

```markdown
## FILE: PayLoad/Entry.cpp

### Current State
[Paste relevant current code sections]

### Required Changes

#### Change 1: Add CPU vendor detection
- Location: Top of file, after includes
- Type: New function
- Code:
```cpp
[Exact code to add]
```

#### Change 2: Add timing state globals
- Location: After CPU vendor detection
- Type: New global variables
- Code:
```cpp
[Exact code to add]
```

#### Change 3: Modify HyperVBackdoor signature handling
- Location: Line XX-YY
- Type: Modify existing
- Before:
```cpp
[Current code]
```
- After:
```cpp
[New code]
```

#### Change 4: Add TSC compensation call
- Location: Before calling original handler
- Type: Insert new code
- Code:
```cpp
[Exact code to add]
```

### New Functions Required
[List each new function with full implementation]

### Validation
- [ ] Compiles without errors
- [ ] No undefined symbols
- [ ] Correct calling convention
- [ ] Timing test passes
```

### Step 3.2: New File Specifications

For EVERY new file, provide complete content:

```markdown
## NEW FILE: PayLoad/Timing.h

### Purpose
Timing compensation state structures and function declarations.

### Full Content
```cpp
#pragma once

// [Complete file content here - every line]
```

### Dependencies
- Requires: Context.h, Vmcs.h
- Required by: Entry.cpp, Timing.cpp

### Build Integration
- Add to PayLoad.vcxproj
- No special compiler flags needed
```

### Step 3.3: Assembly Modifications

For EntryAsm.asm, provide:
- Exact instruction sequences
- Register allocation documentation
- Stack frame changes
- Calling convention preservation

```markdown
## FILE: PayLoad/EntryAsm.asm

### Current Entry Point
```asm
[Current code]
```

### Modified Entry Point
```asm
[New code with comments explaining each change]
```

### Register Usage
- r12: Entry TSC (caller-saved, we save it)
- r13: CPU index (caller-saved, we save it)
- ... [document all register usage]

### Stack Changes
- Additional 16 bytes pushed for TSC storage
- Stack alignment maintained

### Verification
- Disassemble after build to verify
- Check no register clobbering
```

### Step 3.4: AMD Implementation Specifications

Provide complete AMD SVM implementation:

```markdown
## AMD SVM IMPLEMENTATION

### New Files Required
1. PayLoad/Virt/Amd/Vmcb.h - VMCB structures
2. PayLoad/Virt/Amd/SvmHandler.cpp - AMD handler
3. PayLoad/Virt/Amd/SvmHandler.h - AMD declarations

### VMCB Structure (Complete)
```cpp
[Full VMCB structure with all fields]
```

### Handler Implementation
```cpp
[Complete HyperVBackdoor_AMD implementation]
```

### NPT Completion
```cpp
[Complete NPT functions replacing stubs]
```
```

### Step 3.5: Build & Test Procedures

Document exact steps:

```markdown
## BUILD PROCEDURE

### Prerequisites
- Visual Studio 2022 with WDK
- EWDK for cross-compilation
- Hyper-V enabled on target system
- Vulnerable Windows build for ZeroHVCI exploit
  - CVE-2024-26229: Windows builds before KB5037771 (May 2024)
  - CVE-2024-35250: Windows builds before KB5039211 (June 2024)

### Build Steps
1. Open Ombra.sln
2. Select Release|x64
3. Build PayLoad first (produces PayLoad.dll)
4. Build ZeroHVCI second (produces injector)
5. Build OmbraClient third (produces client)

### Expected Warnings
[List any expected warnings that are OK]

### Errors to Watch For
[List common errors and fixes]

## TEST PROCEDURE

### Unit Tests (In-Payload)
1. Add CMD_SELF_TEST to backdoor
2. Test via OmbraClient.exe
3. Verify timing values

### Integration Tests
1. Run on test system with Hyper-V
2. Execute ZeroHVCI injector
3. Run pafish.exe
4. Check all timing tests pass

### Live Tests
1. Run on system with BattlEye game installed
2. Execute ZeroHVCI injector
3. Launch game
4. Monitor for detection
```

---

## PHASE 4: IMPLEMENTATION ORDER

### Mandatory Sequence

1. **First:** Add VMCS field definitions if missing (TSC_OFFSET, etc.)
2. **Second:** Add timing state structures to Context.h
3. **Third:** Create Timing.h/Timing.cpp with compensation logic
4. **Fourth:** Modify EntryAsm.asm to capture entry TSC
5. **Fifth:** Modify Entry.cpp to add vendor dispatch
6. **Sixth:** Modify Entry.cpp to call timing compensation
7. **Seventh:** Add MSR bitmap modification for APERF/MPERF
8. **Eighth:** Add VMX instruction #UD injection
9. **Ninth:** Add trap flag handling fix
10. **Tenth:** Create AMD VMCB structures
11. **Eleventh:** Create AMD SVM handler
12. **Twelfth:** Complete NPT implementation
13. **Thirteenth:** Add self-test commands
14. **Fourteenth:** Test and calibrate

### Parallel Tracks (After Core Implementation)

Track A: Detection hardening
- XSETBV validation
- Invalid MSR handling
- WoW64 descriptor fix

Track B: Future features
- VMFUNC preparation (feature detection)
- #VE preparation (IDT hook infrastructure)

---

## PHASE 5: OUTPUT REQUIREMENTS

### Deliverable 1: Analysis Report
- Complete codebase understanding documented
- All research documents synthesized
- Gap analysis complete
- Risk assessment complete

### Deliverable 2: Implementation Specification
- Every file change documented with exact code
- Every new file provided in full
- Build instructions verified
- Test procedures documented

### Deliverable 3: Implementation Checklist
```markdown
## IMPLEMENTATION CHECKLIST

### Phase 1: Timing Bypass (Intel)
- [ ] VMCS_TSC_OFFSET added to Vmcs.h
- [ ] TIMING_STATE structure added to Context.h
- [ ] Timing.h created with declarations
- [ ] Timing.cpp created with implementation
- [ ] EntryAsm.asm modified for TSC capture
- [ ] Entry.cpp calls ApplyTscCompensation
- [ ] Entry.cpp handles MSR reads for APERF/MPERF
- [ ] MSR bitmap modification function added
- [ ] Self-test timing command added
- [ ] pafish timing test passes

### Phase 2: Detection Evasion
- [ ] VMX instructions inject #UD
- [ ] Trap flag handling correct
- [ ] XSETBV validation added
- [ ] Invalid MSR handling added

### Phase 3: AMD Completion
- [ ] CPU vendor detection in payload
- [ ] Vendor dispatch in HyperVBackdoor
- [ ] VMCB structures complete
- [ ] SvmHandler.cpp complete
- [ ] NPT functions implemented
- [ ] AMD timing compensation added
- [ ] AMD self-test passes

### Phase 4: Validation
- [ ] Builds without errors
- [ ] Injects successfully via ZeroHVCI
- [ ] Backdoor commands work
- [ ] Timing tests pass (Intel)
- [ ] Timing tests pass (AMD)
- [ ] No detection in test environment
```

---

## CRITICAL REMINDERS

1. **DO NOT** skip the codebase analysis. You need FULL context.

2. **DO NOT** make assumptions about code structure. READ the actual files.

3. **DO NOT** provide partial implementations. Every function must be complete.

4. **DO NOT** forget AMD. It must have parity with Intel.

5. **DO NOT** ignore the research documents. They contain tested, working approaches.

6. **DO** document every change with before/after code.

7. **DO** provide exact file paths and line numbers where possible.

8. **DO** consider edge cases (nested virtualization, different Windows builds).

9. **DO** maintain the existing code style and conventions.

10. **DO** test mentally - trace through the code flow to verify correctness.

11. **REMEMBER:** This is runtime injection via ZeroHVCI, NOT a bootkit. Secure Boot and VBS/HVCI can be enabled - we bypass them at runtime.

---

## BEGIN EXECUTION

Start with Phase 0, Step 0.1. Read the directory structure, then proceed through each step sequentially. Do not skip ahead. Document everything.

When you complete each phase, summarize what you learned before proceeding to the next phase.

The final implementation plan should be comprehensive enough that a developer could implement all changes without additional research or clarification.

**Expected output length:** 50-100+ pages of detailed specifications.

**Quality standard:** Production-ready, no ambiguity, no gaps.

GO.
