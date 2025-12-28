# PROJECT-OMBRA: Project Overview

## VERIFICATION STATUS
```
VERIFICATION:
- [x] I read every file mentioned in this document
- [x] I verified all function signatures against actual code
- [x] I verified all data structures against actual definitions
- [x] I verified all dependencies against actual imports
- [ ] I tested or traced all claimed behaviors

UNVERIFIED CLAIMS:
- Actual runtime behavior of hypervisor on target systems
- Anti-cheat detection evasion effectiveness
- Windows version-specific offset compatibility

ASSUMPTIONS:
- MSVC toolchain compatibility based on build.bat
- Target is Windows 10/11 x64 based on EPROCESS offsets
```

## DOCUMENTED FROM
```
Git hash: 73853be (feat: Add BigPool visibility test with live logging)
Date: 2025-12-27
Files read: 50+ hypervisor source files, MCP server code, build scripts
```

---

## What is PROJECT-OMBRA?

**VERIFIED**: PROJECT-OMBRA is a Type-1 (bare-metal) hypervisor targeting Windows x64 systems. It operates at Intel VMX Ring -1 privilege level, virtualizing the existing operating system to gain complete control over CPU execution, memory access, and hardware interaction.

### Primary Capabilities (VERIFIED from code analysis)

1. **VMX-Based Virtualization** (`hypervisor/hypervisor/vmx.c`, `vmcs.c`)
   - Full Intel VT-x implementation
   - Per-CPU VMXON/VMCS region management
   - Guest/host state separation

2. **Extended Page Table (EPT) Control** (`hypervisor/hypervisor/ept.c`)
   - 512GB identity-mapped physical memory
   - Execute-only page support for stealth hooks
   - EPT violation-based memory interception

3. **Anti-Detection Stealth** (`hypervisor/hypervisor/handlers/*.c`, `timing.c`, `spoof.c`)
   - CPUID leaf masking to hide hypervisor presence
   - RDTSC/RDTSCP timing compensation
   - APERF/MPERF ratio virtualization (ESEA detection bypass)
   - VMX MSR hiding via #GP injection

4. **Hardware Fingerprint Spoofing** (`hypervisor/hypervisor/spoof.c`)
   - Disk serial number spoofing (IOCTL interception)
   - NIC MAC address spoofing (OID filtering)
   - Random serial/MAC generation

5. **Shadow Hook Framework** (`hypervisor/hypervisor/hooks.c`)
   - EPT-based execute-only page hooks
   - Split-view memory (different data for read vs execute)
   - Inline hook support with trampoline generation

6. **BYOVD (Bring Your Own Vulnerable Driver)** (`hypervisor/usermode/byovd/supdrv.c`)
   - Ld9BoxSup.sys (LDPlayer VirtualBox fork) exploitation
   - Cookie handshake with version probing
   - LDR_OPEN/LDR_LOAD for kernel code injection
   - PageAllocEx for dual R3/R0 memory mapping

---

## Who is This For?

**CONFIDENCE: INFERRED from CLAUDE.md and codebase structure**

This project appears designed for:
- Security researchers studying hypervisor internals
- Anti-cheat evasion research
- Kernel-level debugging and instrumentation
- Hardware fingerprint obfuscation

---

## Major Components

### 1. Hypervisor Core (`hypervisor/hypervisor/`)

| File | Purpose | Lines | Status |
|------|---------|-------|--------|
| `vmx.c` | VMX lifecycle (vmxon, vmlaunch, vmresume) | ~350 | VERIFIED |
| `vmcs.c` | VMCS field setup and management | ~400 | VERIFIED |
| `ept.c` | EPT table construction and management | ~450 | VERIFIED |
| `exit_dispatch.c` | VM-exit reason dispatcher | ~150 | VERIFIED |
| `timing.c` | TSC compensation for stealth | ~200 | VERIFIED |
| `hooks.c` | EPT-based shadow hooks | ~500 | VERIFIED |
| `spoof.c` | Hardware ID spoofing | ~300 | VERIFIED |
| `nested.c` | Nested virtualization support | ~300 | VERIFIED |

### 2. Exit Handlers (`hypervisor/hypervisor/handlers/`)

| File | Exit Reason | Stealth Features |
|------|-------------|------------------|
| `cpuid.c` | CPUID instruction | Hides hypervisor bit, spoofs vendor |
| `rdtsc.c` | RDTSC/RDTSCP | Compensates for VM-exit timing |
| `msr.c` | RDMSR/WRMSR | Hides VMX MSRs, virtualizes TSC_AUX |
| `cr_access.c` | CR0/CR3/CR4 access | Maintains CR shadows |
| `ept_violation.c` | EPT faults | Drives shadow hooks |
| `vmcall.c` | VMCALL | Hypervisor API gateway |
| `exception.c` | Exceptions | Guest exception injection |
| `io.c` | I/O port access | Port interception |
| `mtf.c` | Monitor trap flag | Single-stepping support |

### 3. Usermode Loader (`hypervisor/usermode/`)

| Component | Purpose |
|-----------|---------|
| `loader/hv_loader.c` | Hypervisor loading orchestration |
| `loader/pe_mapper.c` | PE image mapping to kernel |
| `loader/pe_relocs.c` | Relocation processing |
| `loader/pe_iat.c` | Import resolution |
| `loader/pe_wipe.c` | PE header wiping |
| `byovd/supdrv.c` | Ld9BoxSup.sys interface |
| `byovd/deployer.c` | Vulnerable driver deployment |

### 4. MCP Server Layer (`ombra_mcp_server/`, `driver-re-mcp/`)

| Server | Tools | Purpose |
|--------|-------|---------|
| ombra-mcp | 152 | Intel SDM reference, code generation, stealth auditing |
| driver-re-mcp | 59 | Driver reverse engineering, IOCTL analysis, Ghidra sync |

### 5. Shared Definitions (`hypervisor/shared/`)

| File | Contents |
|------|----------|
| `types.h` | Core types, VMCALL commands, phase structures |
| `vmcs_fields.h` | Complete VMCS field encodings |
| `msr_defs.h` | MSR addresses and definitions |
| `cpu_defs.h` | CR bits, segment access rights |
| `ept_defs.h` | EPT entry formats and flags |

---

## Technology Stack

### Hypervisor
- **Language**: Pure C (no C++ in core)
- **Assembly**: x64 MASM (ml64.exe)
- **Compiler**: MSVC (Visual Studio 2019+)
- **Target**: Windows x64 kernel mode

### MCP Servers
- **Language**: Python 3.10+
- **Framework**: MCP SDK 1.0+
- **Databases**: SQLite (8 databases), ChromaDB (vector search)
- **Embeddings**: all-MiniLM-L6-v2 via ONNX

### Build System
- **Windows**: `hypervisor/build/build.bat` (auto-detects VS via vswhere)
- **Output**: `loader.exe`, `hypervisor.lib`

---

## Architecture Diagram

```
+-----------------------------------------------------------------------------------+
|                              USERMODE (Ring 3)                                     |
|  +-------------+     +------------------+     +---------------------------+        |
|  | loader.exe  |---->| SupDrv Interface |---->| Ld9BoxSup.sys (BYOVD)     |        |
|  | (PE Mapper) |     | (Cookie/LDR)     |     | LDR_OPEN -> kernel alloc  |        |
|  +-------------+     +------------------+     | LDR_LOAD -> copy + exec   |        |
|        |                                      +---------------------------+        |
+--------|--------------------------------------------------------------------------+
         | Manual map + patch .ombra section
         v
+-----------------------------------------------------------------------------------+
|                              KERNEL MODE (Ring 0)                                  |
|  +--------------------------------+                                                |
|  | hypervisor.lib (Entry Point)  |                                                 |
|  |   - OmbraHypervisorEntry()    |                                                 |
|  |   - Reads HV_INIT_PARAMS      |                                                 |
|  |   - IPI to all CPUs           |                                                 |
|  +--------------------------------+                                                |
|              |                                                                     |
+--------------|--------------------------------------------------------------------+
               | Per-CPU: VMXON + VMLAUNCH
               v
+-----------------------------------------------------------------------------------+
|                              VMX ROOT MODE (Ring -1)                               |
|  +----------------+  +----------------+  +----------------+                        |
|  | VMX_CPU #0     |  | VMX_CPU #1     |  | VMX_CPU #N     |                        |
|  | - VMXON region |  | - VMXON region |  | - VMXON region |                        |
|  | - VMCS region  |  | - VMCS region  |  | - VMCS region  |                        |
|  | - Host stack   |  | - Host stack   |  | - Host stack   |                        |
|  +----------------+  +----------------+  +----------------+                        |
|              |               |               |                                     |
|              +-------+-------+               |                                     |
|                      |                       |                                     |
|              +-------v-----------------------v-------+                             |
|              |          Shared EPT Tables            |                             |
|              |  - 512GB identity map                 |                             |
|              |  - Hook entries (execute-only)        |                             |
|              |  - Shadow pages (split view)          |                             |
|              +---------------------------------------+                             |
|              |          MSR Bitmap (4KB)             |                             |
|              |  - Intercept specific MSRs            |                             |
|              +---------------------------------------+                             |
+-----------------------------------------------------------------------------------+

VM-EXIT FLOW:
  Guest executes CPUID/RDTSC/etc
          |
          v
  +------------------+
  | VmExitHandler()  |  <-- asm/vmexit.asm saves GUEST_REGS
  +------------------+
          |
          v
  +--------------------+
  | exit_dispatch.c    |
  | - Read exit reason |
  | - Read qualification|
  | - Dispatch handler |
  +--------------------+
          |
          +--> HandleCpuid()      --> Mask leaves, hide hypervisor
          +--> HandleRdtsc()      --> Compensate timing
          +--> HandleMsr()        --> Hide VMX MSRs
          +--> HandleEptViolation()--> Shadow hook logic
          +--> HandleVmcall()     --> Hypervisor API
          |
          v
  +-------------------+
  | VMEXIT_CONTINUE   |  or  | VMEXIT_ADVANCE_RIP |
  +-------------------+       +--------------------+
          |
          v
  asm/vmexit.asm restores GUEST_REGS, executes VMRESUME
```

---

## VMCALL API (VERIFIED from `shared/types.h`)

### Phase 1: Core Operations
| Command | Value | Purpose |
|---------|-------|---------|
| `VMCALL_PING` | 0x7C9E2A8F | Verify hypervisor presence |
| `VMCALL_UNLOAD` | 0x3F7B8D21 | Graceful shutdown |
| `VMCALL_GET_STATUS` | 0x92C4E6F3 | Get hypervisor status |

### Phase 1: Memory Operations
| Command | Value | Purpose |
|---------|-------|---------|
| `VMCALL_READ_PHYS` | 0xE2A849D7 | Physical memory read |
| `VMCALL_WRITE_PHYS` | 0x6C3DF52B | Physical memory write |
| `VMCALL_VIRT_TO_PHYS` | 0xA971D8F4 | VA to PA translation |
| `VMCALL_READ_VIRT` | 0xD4F83A19 | Virtual memory read (cross-process) |
| `VMCALL_WRITE_VIRT` | 0x8B2E57C6 | Virtual memory write |
| `VMCALL_GET_PROCESS_CR3` | 0x2F6E9A7C | Get process page table base |

### Phase 1: Hooks
| Command | Value | Purpose |
|---------|-------|---------|
| `VMCALL_HOOK_INSTALL` | 0x5A3F9B27 | Install EPT shadow hook |
| `VMCALL_HOOK_REMOVE` | 0xB8D12E95 | Remove hook |
| `VMCALL_HOOK_LIST` | 0x41F7C3A6 | List active hooks |

### Phase 1: Nested VMX
| Command | Value | Purpose |
|---------|-------|---------|
| `VMCALL_NESTED_GET_INFO` | 0x1E65AB3C | Get nested VMX state |
| `VMCALL_NESTED_ENABLE` | 0xD7F246E8 | Enable nested virtualization |
| `VMCALL_NESTED_DISABLE` | 0x4B829CF1 | Disable nested virtualization |

### Phase 1: Hardware Spoofing
| Command | Value | Purpose |
|---------|-------|---------|
| `VMCALL_SPOOF_INIT` | 0xC7A3E518 | Initialize spoof manager |
| `VMCALL_SPOOF_ADD_DISK` | 0x39F8B2D1 | Add disk serial spoof |
| `VMCALL_SPOOF_DEL_DISK` | 0xE5D4A726 | Remove disk spoof |
| `VMCALL_SPOOF_ADD_NIC` | 0x8B61F4C9 | Add NIC MAC spoof |
| `VMCALL_SPOOF_DEL_NIC` | 0x4E27D8A3 | Remove NIC spoof |

### Phase 2: Driver Mapper
| Command | Value | Purpose |
|---------|-------|---------|
| `VMCALL_CLAIM_POOL_REGION` | 0x3000 | Claim hypervisor pool memory |
| `VMCALL_FINALIZE_DRIVER_LOAD` | 0x3010 | Complete driver loading |
| `VMCALL_HIDE_MEMORY_RANGE` | 0x3020 | Hide memory via EPT |
| `VMCALL_ALLOC_MDL_MEMORY` | 0x3030 | Async MDL allocation |

---

## BYOVD Exploitation Chain (VERIFIED from `supdrv.c`, `supdrv.h`)

### Target Driver
**Ld9BoxSup.sys** - LDPlayer's fork of VirtualBox support driver

### Exploitation Steps

1. **Device Open** (NtCreateFile required - no DOS symlink)
   - Paths tried: `\Device\Ld9BoxDrv`, `\Device\Ld9BoxDrvU`, `\Device\VBoxDrv`

2. **Cookie Handshake** (`SUP_IOCTL_COOKIE`)
   - Initial cookie: `0x69726F74` ("tori" - LDPlayer variant)
   - Magic string: `"The Magic Word!"`
   - Version probing across known versions

3. **Memory Allocation** (`SUP_IOCTL_PAGE_ALLOC_EX`)
   - Returns dual mapping: R3 (usermode writable) + R0 (kernel executable)
   - Key primitive for code injection

4. **Module Loading**
   - `SUP_IOCTL_LDR_OPEN`: Allocates kernel memory
   - `SUP_IOCTL_LDR_LOAD`: Copies image and calls entry point

### Known Limitations
- **MSR_PROBER disabled**: Returns `VERR_NOT_SUPPORTED` (-12)
- **-618 Error**: LDR_OPEN fails in nested virtualization (validation flag issue)

---

## Build Process (VERIFIED from `build/build.bat`)

### Prerequisites
- Visual Studio 2019+ with "Desktop development with C++" workload
- MASM (ml64.exe) included with VS

### Build Steps
```batch
cd hypervisor/build
build.bat
```

### Output Artifacts
- `usermode/loader.exe` - Hypervisor loader with BYOVD support
- `hypervisor/hypervisor.lib` - Static library for kernel payload

### Compilation Flow
1. **Usermode loader**: `cl.exe` compiles all loader/*.c and byovd/*.c
2. **Assembly modules**: `ml64.exe` assembles vmexit.asm, intrinsics.asm, segment.asm
3. **Hypervisor C**: `cl.exe` compiles vmx.c, vmcs.c, ept.c, handlers/*.c
4. **Library creation**: `lib.exe` creates hypervisor.lib from all .obj files

---

## Stealth Features Summary

### Timing Compensation (VERIFIED from `timing.c`, `rdtsc.c`)
- TSC offset tracking per handler type
- Subtracts VM-exit overhead from guest-visible TSC
- APERF/MPERF virtualization to maintain expected ratio

### CPUID Masking (VERIFIED from `cpuid.c`)
- Clears hypervisor present bit (leaf 0x1, ECX bit 31)
- Zeroes hypervisor vendor leaves (0x40000000-0x40000010)
- Passes through hardware CPUID results for other leaves

### MSR Virtualization (VERIFIED from `msr.c`)
- `IA32_FEATURE_CONTROL`: Returns locked + VMX disabled
- VMX capability MSRs (0x480-0x491): Injects #GP
- `IA32_TSC_AUX`: Virtualized per-CPU for RDTSCP consistency
- `IA32_APERF/MPERF`: Offset compensation

### EPT Shadow Hooks (VERIFIED from `hooks.c`, `ept_violation.c`)
- Execute-only pages (R=0, W=0, X=1)
- Read/write triggers EPT violation -> temporary page swap
- Single-step (MTF) -> restore execute-only view

---

## Database Reference

### MCP Server Databases (`ombra_mcp_server/src/ombra_mcp/data/`)

| Database | Size | Purpose |
|----------|------|---------|
| `intel_sdm.db` | 225KB | VMCS fields (167), exit reasons (66), MSRs (35) |
| `project_brain.db` | 14MB | Decisions, gotchas, sessions, findings |
| `anticheat_intel.db` | 102KB | Detection methods, bypasses |
| `evasion_techniques.db` | 110KB | Bypass chains, cleanup procedures |
| `byovd_drivers.db` | 61KB | Vulnerable driver catalog |
| `vergilius.db` | 3.2MB | Windows kernel structures |
| `mslearn_reference.db` | 905KB | MS Learn API docs |
| `driver_re.db` | 168KB | Driver RE analysis data |

---

## GAPS AND UNKNOWNS

### Things I Could Not Determine
- [ ] Actual anti-cheat evasion effectiveness in production
- [ ] Windows version compatibility matrix (only 19041+ offsets verified)
- [ ] Whether -618 bypass with ThrottleStop is implemented
- [ ] GUI component status (Dear ImGui mentioned but not read)
- [ ] OmbraDriver kernel component implementation status

### Things That Need Human Clarification
- [ ] Is the project currently functional end-to-end?
- [ ] What Windows versions have been tested?
- [ ] Are there additional BYOVD drivers supported beyond Ld9BoxSup?
- [ ] What is the current development phase (1, 2, or 3)?

---

*Documentation generated 2025-12-27 by Claude Code*
*CONFIDENCE LEVEL: HIGH for structure and interfaces, MEDIUM for runtime behavior*
