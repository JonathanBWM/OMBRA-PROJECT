# Hypervisor Component - Developer Context

This CLAUDE.md provides hypervisor-specific context. See root `/CLAUDE.MD` for project-wide conventions, ENI identity, and MCP tool usage.

## Quick Stats

| Metric | Value |
|--------|-------|
| **Total LOC** | 34,780 (excluding ImGui) |
| **Language** | Pure C + x64 MASM |
| **Compiler** | MSVC cl.exe (VS2019+) |
| **Assembler** | ml64.exe |
| **Build** | `build/build.bat` |

---

## Directory Structure

```
hypervisor/
├── build/                    # Build system
│   ├── build.bat            # Main build script (5KB, uses vswhere)
│   └── Makefile             # Stub makefile
├── hypervisor/              # VMX Core (kernel-mode library)
│   ├── asm/                 # Assembly intrinsics
│   │   ├── vmexit.asm      # VM-exit entry/resume (~7.6KB)
│   │   ├── intrinsics.asm  # vmread/vmwrite wrappers
│   │   └── segment.asm     # GDT/IDT helpers
│   ├── handlers/            # Per-exit-reason handlers
│   │   ├── cpuid.c         # CPUID virtualization + spoofing
│   │   ├── msr.c           # RDMSR/WRMSR handlers
│   │   ├── rdtsc.c         # RDTSC/RDTSCP timing compensation
│   │   ├── ept_violation.c # EPT fault handling
│   │   ├── ept_misconfig.c # EPT misconfiguration
│   │   ├── cr_access.c     # CR0/CR3/CR4 access
│   │   ├── exception.c     # Exception/NMI injection
│   │   ├── io.c            # I/O port virtualization
│   │   ├── vmcall.c        # VMCALL interface (~53KB - largest)
│   │   ├── vmcall_phase2.c # Extended VMCALL operations
│   │   ├── power_mgmt.c    # Power state transitions
│   │   ├── mtf.c           # Monitor Trap Flag
│   │   └── handlers.h      # Handler declarations
│   ├── vmx.c               # VMX enable/vmxon/vmxoff (~12.5KB)
│   ├── vmcs.c              # VMCS setup + field management (~23KB)
│   ├── ept.c               # EPT tables + identity mapping (~20KB)
│   ├── exit_dispatch.c     # Exit reason dispatcher (~7KB)
│   ├── nested.c            # Nested virtualization (~21KB)
│   ├── hooks.c             # Shadow hook management (~45KB - hot path)
│   ├── timing.c            # TSC compensation (~17KB)
│   ├── spoof.c             # Hardware ID spoofing (~9KB)
│   ├── entry.c             # Hypervisor entry point (~12KB)
│   └── debug.c/h           # Kernel debugging utilities
├── driver/                  # Windows kernel driver
│   ├── init.c              # DriverEntry, device creation
│   ├── dispatch.c          # IRP dispatch (~14KB)
│   ├── vmcall.c            # Driver-to-HV communication (~9KB)
│   ├── memory_ops.c        # Physical/virtual memory ops (~7.5KB)
│   ├── protection.c        # Memory protection (~6.7KB)
│   ├── shadow.c            # Shadow page management (~6.4KB)
│   ├── spoof.c             # Driver-level HW spoofing (~8.5KB)
│   ├── etw.c               # ETW trace management (~6KB)
│   ├── module_lock.c       # Module enumeration protection (~7.5KB)
│   ├── command_ring.c      # Ring buffer for commands
│   ├── subscription.c      # Event subscription system
│   ├── diagnostics.c       # Runtime diagnostics
│   └── worker.c            # Background worker threads
├── shared/                  # Headers shared across components
│   ├── types.h             # Core type definitions (~19KB)
│   ├── vmcs_fields.h       # VMCS field encodings (~21KB, Intel SDM)
│   ├── ept_defs.h          # EPT structure definitions (~7KB)
│   ├── exit_reasons.h      # Exit reason enum (~5KB)
│   ├── msr_defs.h          # MSR addresses
│   └── cpu_defs.h          # CPU feature flags
├── usermode/                # Usermode components
│   ├── main.c              # Application entry
│   ├── loader/             # PE manual mapper
│   │   ├── hv_loader.c     # Hypervisor loading logic
│   │   ├── pe_parser.c     # PE header parsing
│   │   ├── pe_mapper.c     # Memory mapping
│   │   ├── pe_relocs.c     # Relocation processing
│   │   ├── pe_iat.c        # Import table resolution
│   │   ├── pe_imports.c    # Import handling
│   │   ├── pe_wipe.c       # PE header wiping (anti-forensics)
│   │   └── drv_loader.c    # Driver loading
│   ├── byovd/              # BYOVD exploitation
│   │   ├── supdrv.c/h      # Ld9BoxSup.sys interface
│   │   ├── throttlestop.c/h # ThrottleStop phys mem R/W
│   │   ├── deployer.c/h    # Driver deployment
│   │   ├── crypto.c/h      # Driver decryption
│   │   └── supdrv_types.h  # IOCTL structures
│   ├── tests/              # Detection testing
│   │   ├── bigpool_test.c  # BigPool visibility test
│   │   └── detection_baseline.c
│   └── gui/                # Dear ImGui (vendored)
├── docs/                    # Hypervisor-specific docs
└── resources/               # Embedded resources
```

---

## DANGER ZONES

### Critical Files (Break Everything)

| File | Risk | Notes |
|------|------|-------|
| `hypervisor/vmcs.c` | **CRITICAL** | Wrong VMCS fields = VMLAUNCH failure or BSOD |
| `hypervisor/asm/vmexit.asm` | **CRITICAL** | Assembly VM-exit handler - wrong register save = corruption |
| `hypervisor/ept.c` | **HIGH** | EPT misconfiguration = triple fault |
| `hypervisor/exit_dispatch.c` | **HIGH** | Exit dispatcher - missing case = unhandled exit crash |
| `hypervisor/hooks.c` | **HIGH** | Largest file (~45KB), shadow hook hot path |
| `shared/vmcs_fields.h` | **CRITICAL** | VMCS encodings from Intel SDM - must match exactly |

### Anti-Detection Sensitive

| File | Detection Vector |
|------|-----------------|
| `handlers/cpuid.c` | Hypervisor bit detection, CPUID timing |
| `handlers/rdtsc.c` | TSC timing-based detection |
| `timing.c` | Overall timing compensation |
| `spoof.c` (both) | Hardware fingerprint detection |
| `usermode/byovd/` | Driver signature/blocklist detection |

---

## Build System

### Building

```batch
cd hypervisor\build
build.bat
```

The build script:
1. Uses `vswhere` to locate Visual Studio
2. Compiles C files with MSVC (`cl.exe`)
3. Assembles ASM with MASM (`ml64.exe`)
4. Links into `hypervisor.lib` static library
5. Builds `loader.exe` usermode component

### Required Tools
- Visual Studio 2019+ with Desktop C++ workload
- Windows SDK (for kernel headers)
- WDK (for driver development)

---

## Key Implementation Details

### VMCS Setup Flow (`vmcs.c`)

```
vmcs_alloc_region() → vmcs_initialize() → vmcs_setup_controls() → vmcs_setup_guest() → vmcs_setup_host()
```

All control fields adjusted via capability MSRs (`IA32_VMX_*`).

### Exit Dispatch Flow (`exit_dispatch.c`)

```
vmexit_stub (asm) → exit_dispatch() → handlers/xxx.c → vmresume (asm)
```

Exit reason read from VMCS, dispatched to handler, then resume.

### EPT Structure (`ept.c`)

- Identity mapping: GPA == HPA for all physical memory
- 2MB large pages by default for performance
- Split to 4KB only when shadow hooks needed
- VPID enabled for TLB management

### Shadow Hooks (`hooks.c`)

```c
// EPT split-view: different pages for read vs execute
// Execute: Original code with trap
// Read: Clean code (passes integrity checks)
shadow_hook_install(target_va, hook_fn);
```

### Timing Compensation (`timing.c`)

- TSC offsetting to hide VM-exit latency
- RDTSC/RDTSCP handler adjusts returned value
- Per-exit compensation calibrated at init

---

## Modification Guide

### Adding a New Exit Handler

1. Create `handlers/new_exit.c`:
```c
#include "handlers.h"
#include "../shared/types.h"

BOOLEAN handle_new_exit(PGUEST_STATE guest) {
    // Read exit qualification if needed
    UINT64 qual = vmcs_read(VMCS_EXIT_QUALIFICATION);

    // Handle the exit...

    // Advance RIP if instruction-based exit
    guest->rip += vmcs_read(VMCS_VMEXIT_INSTRUCTION_LENGTH);
    return TRUE;
}
```

2. Add declaration to `handlers/handlers.h`

3. Add case to `exit_dispatch.c`:
```c
case VMX_EXIT_REASON_NEW:
    handled = handle_new_exit(guest);
    break;
```

4. **Use OmbraMCP** to get correct exit reason details:
```bash
mcp-cli call ombra/exit_reason_complete '{"reason": XX}'
```

### Adding a New VMCALL

1. Define VMCALL number in `shared/types.h`:
```c
#define VMCALL_NEW_OPERATION 0x1234
```

2. Add handler in `handlers/vmcall.c`:
```c
case VMCALL_NEW_OPERATION:
    result = handle_new_operation(guest->rcx, guest->rdx);
    break;
```

3. Add driver-side caller in `driver/vmcall.c`

### Modifying VMCS Fields

**ALWAYS** use OmbraMCP first:
```bash
mcp-cli call ombra/vmcs_field_complete '{"field_name": "FIELD_NAME"}'
mcp-cli call ombra/validate_vmcs_setup '{"vmcs_values": {...}}'
```

Do NOT hardcode VMCS encodings without verifying against Intel SDM.

---

## Testing

### BigPool Visibility Test
```bash
# Tests if allocated memory is visible in BigPool scan
hypervisor/usermode/tests/bigpool_test.c
```

### Detection Baseline
```bash
# Establishes baseline for timing-based detection
hypervisor/usermode/tests/detection_baseline.c
```

---

## MCP Tools for Hypervisor Development

Always use these OmbraMCP tools when working on hypervisor code:

| Task | MCP Tool |
|------|----------|
| VMCS field details | `vmcs_field_complete` |
| Exit reason handling | `exit_reason_complete` |
| MSR information | `get_msr_info` |
| Code generation | `generate_exit_handler`, `generate_vmcs_setup` |
| VMCS validation | `validate_vmcs_setup` |
| Stealth audit | `audit_stealth`, `get_detection_vectors` |
| Timing compensation | `generate_timing_compensation` |

Example workflow:
```bash
# Before implementing EPT violation handler
mcp-cli info ombra/exit_reason_complete
mcp-cli call ombra/exit_reason_complete '{"reason": 48}'

# Generate handler skeleton
mcp-cli call ombra/generate_exit_handler '{"reason": 48, "stealth": true}'

# Audit for detection risks
mcp-cli call ombra/audit_stealth '{"code": "<your code>"}'
```

---

## Known Issues / Gotchas

1. **Nested VMX**: `nested.c` implements L1/L2 guest support but has edge cases with certain hypervisors
2. **VMCS shadowing**: Not fully implemented - some nested operations fall back to trap-and-emulate
3. **hooks.c backups**: `.bak` and `.bak2` files exist - may contain useful previous implementations
4. **Large VMCALL handler**: `handlers/vmcall.c` at 53KB is the largest - consider refactoring

---

## File Ownership

| Component | Primary Files | Owner |
|-----------|--------------|-------|
| VMX Core | vmx.c, vmcs.c, entry.c | Hypervisor Team |
| Exit Handlers | handlers/*.c | Hypervisor Team |
| EPT/Memory | ept.c, memory_ops.c | Memory Team |
| Stealth | timing.c, spoof.c, hooks.c | Stealth Team |
| BYOVD | usermode/byovd/*.c | Loader Team |
| PE Mapper | usermode/loader/*.c | Loader Team |

---

## Quick Reference: Exit Reasons

Common exit reasons handled:

| Reason | Handler | File |
|--------|---------|------|
| 10 | CPUID | handlers/cpuid.c |
| 28 | CR Access | handlers/cr_access.c |
| 31 | RDMSR | handlers/msr.c |
| 32 | WRMSR | handlers/msr.c |
| 16 | RDTSC | handlers/rdtsc.c |
| 18 | VMCALL | handlers/vmcall.c |
| 48 | EPT Violation | handlers/ept_violation.c |
| 49 | EPT Misconfig | handlers/ept_misconfig.c |
| 0 | Exception/NMI | handlers/exception.c |
| 30 | I/O | handlers/io.c |
| 37 | MTF | handlers/mtf.c |

For complete exit reason documentation, use:
```bash
mcp-cli call ombra/exit_reason_complete '{"reason": <number>}'
```

---

## Anti-Cheat Evasion Intel (Community Research)

This section contains critical intelligence gathered from the anti-cheat research community about detection mechanisms and evasion techniques. **This is required reading before implementing any stealth features.**

### Memory API Truths

**MmCopyMemory/MmMapIoSpace Hooking:**
- **MmCopyMemory and MmMapIoSpace are NOT hooked by EAC** - Both are PatchGuard protected
- **MmCopyMemory for virtual memory** → Traced to ETW (use physical reads instead)
- **MmCopyMemory for physical memory** → Safe, no ETW visibility
- Anti-cheats cannot hook these APIs without triggering PatchGuard
- However, they can trace specific usage patterns via ETW

**Practical Implications:**
- Physical memory operations via MmCopyMemory are stealth-safe
- Virtual memory copies trigger ETW events - avoid or use EPT instead
- MmMapIoSpace is safe but creates kernel VA mappings that may be enumerated
- For maximum stealth, use custom PTE manipulation (see Custom Physical Memory Read Pattern below)

### EAC Detection Mechanisms

**KiPageFault Heuristics:**
- EAC uses kdTrap hook at IDT entry 0x14 (Ki(Shadow)PageFault)
- Detection is about **WHEN and HOW** you allocate, not WHICH API
- Page fault patterns every microsecond give behavioral fingerprint
- EAC monitors page fault frequency, target addresses, and fault reasons

**What This Means for Hypervisor:**
- EPT violations are invisible to guest (no #PF in guest IDT)
- Guest page faults still visible to EAC's hook
- Avoid creating unusual page fault patterns during memory scans
- Use EPT read/write/execute permissions to minimize guest-visible faults

**Timing Considerations:**
- Normal game page faults: sporadic, tied to code execution flow
- Cheat page faults: high frequency, linear address patterns (scanning)
- Solution: Batch memory operations, use EPT for single-step execution

### Memory Permission Rules

**Critical Permission Constraints:**
- **NEVER use RWX (Read-Write-Execute) permissions** - Anti-cheats flag executable+writable memory
- Only use **R/W** or **R/X** page permissions in guest-visible memory
- For code modification, use EPT execute-only pages or separate views
- EAC/BE/Vanguard all scan for RWX pages via:
  - VirtualQuery loops
  - Working set enumeration
  - PTE scanning

**EPT Permission Strategy:**
- Execute-only EPT pages require CPU support (check IA32_VMX_EPT_VPID_CAP[0])
- Use EPT split-view: Read view (R/W) vs Execute view (R/X)
- Shadow hooks: Execute = original+trap, Read = clean code
- Guest never sees RWX, anti-cheat integrity checks pass

### Custom Physical Memory Read Pattern

This pattern is faster than MmCopyMemory and avoids ETW entirely:

```
1. Allocate a block of memory (NonPagedPool)
2. Get the PTE of the allocated memory
3. Write the PTE's PFN (Page Frame Number) to target physical address
4. Copy from that block of memory (now mapped to target physical page)
5. Restore original PFN
```

**Why This Works:**
- No API calls that trigger ETW
- Direct PTE manipulation in hypervisor context
- Guest anti-cheat cannot observe the mapping
- Faster than MmCopyMemory due to fewer checks

**Hypervisor Implementation:**
- EPT controls guest's view of physical memory
- Hypervisor has access to host physical memory directly
- Use EPT identity map + temporary PTE swap for arbitrary phys reads

### SPOOF_FUNC Usage

**What It Is:**
- SPOOF_FUNC is a technique to spoof call stack return addresses
- Used to make usermode calls to game engine functions appear legitimate

**When to Use:**
- **Only for game engine function calls** from usermode internals
- Anti-cheat monitors call origins to game functions (StaticFindObject, GetWorld, etc.)
- Spoofing return address prevents detection of external module calls

**When NOT to Use:**
- **Useless in kernel context** - kernel API calls don't need return address spoofing
- Anti-cheats don't validate kernel stack traces (would be too expensive)
- Pasting SPOOF_FUNC everywhere is a noob signature
- Hypervisor operations have no usermode call stack

**Hypervisor Context:**
- SPOOF_FUNC is irrelevant for VMX root operations
- Guest call stacks are managed via VMCS (GUEST_RSP, GUEST_RIP)
- If injecting usermode code via EPT hooks, consider SPOOF_FUNC at injection site

### Hypervisor Advantage

Operating at VMX root bypasses most kernel-mode detection:

**ETW Invisibility:**
- No ETW visibility into VMX non-root operations
- Guest kernel's ETW providers cannot trace hypervisor activity
- Hypervisor memory operations invisible to guest

**Execute-Only Memory:**
- EPT provides true execute-only pages (if CPU supports it)
- Guest cannot read code that it executes (passes signature checks)
- Split-view EPT: Execute sees original, Read sees modified

**Page Fault Bypass:**
- Page fault heuristics can't see hypervisor activity
- EPT violations handled in VMX root, guest never sees #PF
- Single-step execution via MTF (Monitor Trap Flag) instead of guest #DB

**Timing Compensation:**
- Timing compensation hides VMExit overhead
- TSC offsetting via VMCS field (no guest-visible API)
- RDTSC/RDTSCP exits compensated with pre-calculated deltas

**Physical Memory Control:**
- EPT controls guest's physical address space
- Can hide memory from guest by unmapping GPA ranges
- PML4E scan evasion: Don't create orphaned executable pages in guest page tables

### BYOVD Exploitation Chain (Ld9BoxSup.sys)

**Two-Driver Architecture:**

- **Driver 1 (Ld9BoxSup.sys)** - LDPlayer VirtualBox fork, provides kernel code loading
- **Driver 2 (ThrottleStop)** - CPU tuning utility, provides physical memory R/W
- Driver 1 returns error -618 in nested virtualization due to failed module enumeration check
- Driver 2 patches validation flags via physical memory, then unloads (~10-15ms exploit window)

**Ld9BoxSup.sys Critical Values (Verified Dec 2025):**

| Value | Description |
|-------|-------------|
| `0x69726F74` | Initial cookie ("tori" - NOT stock VBox "Bori") |
| `"The Magic Word!"` | Auth magic string at offset 0x36d58 |
| `0x42000042` | Required header flags |
| `0x320000` | Driver version |
| `0x4a1a0` | ntoskrnl validation flag offset |
| `0x4a210` | hal validation flag offset |
| 48/56 | COOKIE buffer sizes (in/out) |
| 328/40 | LDR_OPEN buffer sizes (in/out) |

**Key IOCTLs:**

- `SUP_IOCTL_COOKIE` - Establish session (required first call)
- `SUP_IOCTL_LDR_OPEN` - Allocate executable kernel memory
- `SUP_IOCTL_LDR_LOAD` - Copy code + call entry point (Ring-0 execution!)
- `SUP_IOCTL_MSR_PROBER` - DISABLED in this build (returns error -12)

**-618 Error Bypass Process:**

1. Load Driver 2 (ThrottleStop)
2. Scan physical memory 0x1a2000-0x1b0000 for SYSTEM EPROCESS signature
3. Read CR3 from DirectoryTableBase field (+0x28 offset)
4. Walk page tables to translate validation flag VAs to physical addresses
5. Write 0x01 to both ntoskrnl and hal validation flags
6. Unload Driver 2 immediately
7. Driver 1 LDR_OPEN now succeeds (bypass complete)

**Post-Exploitation Cleanup (from Ring-0):**

- Clear `MmUnloadedDrivers` linked list entry (prevent forensic detection)
- Clear `PiDDBCacheTable` hash bucket entry (driver object cache)
- Wipe ETW circular buffers in timestamp window (remove load event)
- Delete prefetch files (.pf) from `C:\Windows\Prefetch\`

**Hypervisor Integration:**

- Use BYOVD to load initial hypervisor payload into kernel
- Hypervisor then takes over memory operations via EPT
- BYOVD driver can be unloaded after hypervisor initialization
- Cleanup is critical - leave no traces of BYOVD exploitation

### EAC PML4E Scanning (Critical)

**The Most Dangerous Detection Vector:**

- EAC scans **ALL kernel PML4 entries** for executable pages without backing drivers
- BigPool scan happens ~20 minutes after start, but PML4E scan is **CONTINUOUS**
- Detection requires **ZERO execution** - just having orphaned executable pages = flagged
- MDL allocation bypasses BigPool tracking but **NOT PML4E scan**

**What This Means:**

- Any executable kernel memory must have a backing driver object OR be invisible
- Traditional driver mapping creates orphaned exec pages → instant detection
- Allocating NonPagedPoolExecute shows up in page tables → detected
- Even if you never execute the code, presence of orphaned RX pages = ban

**Evasion Strategy:**

1. **EPT Split-View (Recommended):**
   - Guest PML4E points to non-executable page (R/W only)
   - EPT maps same GPA to executable page for execute access
   - Guest sees R/W, executes via EPT as R/X
   - PML4E scan sees no orphaned executable pages

2. **Physical Memory Hiding:**
   - Allocate memory in reserved physical regions
   - Map via EPT only (no guest VA mapping)
   - Execute code from hypervisor context, not guest

3. **Temporary Mapping:**
   - Only create executable guest mappings during execution
   - Clear execute bit immediately after use
   - High overhead, not practical for hot paths

**Implementation in hooks.c:**

- Shadow hooks use EPT split-view by default
- Execute view: Original code with trap instruction
- Read view: Clean code (passes integrity checks)
- Guest page tables never show execute permission on hooked page
- PML4E scan sees R/W page, passes

**Testing PML4E Visibility:**

```c
// Test if memory is visible in PML4E scan
// Located in: hypervisor/usermode/tests/bigpool_test.c
// Scans PML4 entries looking for our allocation
// If found = detection risk
```

**Key Takeaway:**

**EPT split-view is not optional for stealth - it is MANDATORY to evade PML4E scanning.**

---
