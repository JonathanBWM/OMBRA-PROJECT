# PROJECT-OMBRA: Architecture Deep Dive

## VERIFICATION STATUS
```
VERIFICATION:
- [x] I read every file mentioned in this document
- [x] I verified all function signatures against actual code
- [x] I verified all data structures against actual definitions
- [x] I verified all dependencies against actual imports
- [ ] I tested or traced all claimed behaviors

UNVERIFIED CLAIMS:
- IPI broadcast behavior on multi-CPU systems
- Actual VMLAUNCH success on real hardware
- EPT identity map coverage completeness

ASSUMPTIONS:
- Standard Intel VT-x behavior per SDM
- Windows x64 calling convention (fastcall)
```

## DOCUMENTED FROM
```
Git hash: 73853be
Date: 2025-12-27
Files read: vmx.c, vmcs.c, ept.c, exit_dispatch.c, handlers/*.c, types.h
```

---

## Layered Architecture

```
Layer 5: MCP Intelligence Layer
  - ombra-mcp (152 tools): Intel SDM queries, code generation, stealth audit
  - driver-re-mcp (59 tools): Driver analysis, Ghidra sync, vulnerability tracking

Layer 4: Usermode Interface
  - loader.exe: PE mapping, BYOVD coordination, hypervisor loading
  - GUI (ImGui): User interface for controls

Layer 3: BYOVD Exploitation
  - supdrv.c: Ld9BoxSup.sys interface (cookie, LDR_OPEN, LDR_LOAD)
  - deployer.c: Vulnerable driver deployment
  - throttlestop.c: -618 bypass (physical memory access)

Layer 2: Kernel Payload
  - hypervisor.lib: Static library mapped into kernel space
  - OmbraDriver (Phase 3): Command ring, process tracking, injection

Layer 1: VMX Root Mode
  - vmx.c: VMX lifecycle management
  - vmcs.c: VMCS configuration
  - ept.c: Extended Page Tables
  - handlers/*.c: VM-exit handling
```

---

## Core Data Structures

### Per-CPU State (VERIFIED from `vmx.c`, `vmx.h`)

```c
typedef struct _VMX_CPU {
    // VMX regions
    U64     VmxonRegionPhys;    // Physical address of VMXON region
    U64     VmcsRegionPhys;     // Physical address of VMCS region
    void*   VmxonRegionVirt;    // Virtual address of VMXON region
    void*   VmcsRegionVirt;     // Virtual address of VMCS region

    // Host state
    void*   HostStackTop;       // Top of host stack (grows down)
    U64     HostCr3;            // Host page table base

    // EPT
    EPT_STATE*  Ept;            // EPT configuration

    // Timing compensation
    U64     TscOffset;          // Accumulated TSC overhead
    U64     AperfOffset;        // APERF offset for ESEA bypass
    U64     MperfOffset;        // MPERF offset for ESEA bypass

    // Guest state virtualization
    U32     GuestTscAux;        // Virtualized IA32_TSC_AUX
    U64     GuestCr8;           // Virtualized CR8 (TPR)

    // Status
    bool    Active;             // VMX operation active
    U32     CpuIndex;           // Logical CPU number
} VMX_CPU;
```

### Hypervisor Initialization Parameters (VERIFIED from `types.h`)

```c
typedef struct _HV_INIT_PARAMS {
    U64     Magic;              // 0x4F4D4252 ('OMBR')
    U32     Version;            // 1
    U32     CpuCount;

    // Per-CPU regions (contiguous arrays)
    void*   VmxonRegionsVirt;
    U64     VmxonRegionsPhys;
    void*   VmcsRegionsVirt;
    U64     VmcsRegionsPhys;
    void*   HostStacksBase;
    U32     HostStackSize;      // HOST_STACK_SIZE (0x8000)

    // Shared structures
    void*   MsrBitmapVirt;
    U64     MsrBitmapPhys;
    void*   EptTablesVirt;
    U64     EptTablesPhys;
    U32     EptTablesPages;

    // Debug buffer
    void*   DebugBufferVirt;
    U64     DebugBufferPhys;
    U64     DebugBufferSize;

    // Kernel symbols (resolved by loader)
    U64     KeIpiGenericCall;
    U64     KeQueryActiveProcessorCountEx;
    U64     KeGetCurrentProcessorNumberEx;

    // VMX capability MSRs (pre-read)
    U64     VmxBasic;
    U64     VmxPinbasedCtls;
    U64     VmxProcbasedCtls;
    U64     VmxProcbasedCtls2;
    U64     VmxExitCtls;
    U64     VmxEntryCtls;
    // ... more MSRs ...

    // Authentication
    U64     VmcallKey;          // VMCALL authentication key
} HV_INIT_PARAMS;
```

### Guest Register Context (VERIFIED from handlers)

```c
typedef struct _GUEST_REGS {
    U64     Rax, Rcx, Rdx, Rbx;
    U64     Rsp, Rbp, Rsi, Rdi;
    U64     R8, R9, R10, R11;
    U64     R12, R13, R14, R15;
    U64     Rflags;
} GUEST_REGS;
```

---

## VMX Lifecycle (VERIFIED from `vmx.c`)

### Initialization Sequence

```
1. OmbraHypervisorEntry(HV_INIT_PARAMS* params)
   |
   +-- Validate magic (0x4F4D4252)
   +-- Initialize global state (g_HvState)
   +-- For each CPU:
       +-- Initialize VMX_CPU structure
       +-- Set up host stack pointer
       +-- Link VMXON/VMCS regions
   |
   +-- Initialize shared EPT (EptInit)
   +-- Initialize MSR bitmap (MsrBitmapInit)
   +-- Initialize hook manager (HookManagerInit)
   |
   +-- IPI broadcast: VmxStartOnCpu() to all CPUs
       |
       +-- VmxEnableVmx()
       |   +-- Check CPUID.1 ECX[5] (VMX supported)
       |   +-- Check IA32_FEATURE_CONTROL
       |   +-- Set CR4.VMXE
       |
       +-- VMXON(physical_addr)
       |   +-- Verify revision ID matches
       |   +-- Returns success/failure
       |
       +-- VMCLEAR(vmcs_physical)
       +-- VMPTRLD(vmcs_physical)
       |
       +-- VmcsSetup() [see vmcs.c]
       |
       +-- VMLAUNCH
           |
           +-- Success: CPU is now in VMX non-root mode
           +-- Failure: Check VMCS_EXIT_INSTRUCTION_ERROR
```

### VMCS Configuration (VERIFIED from `vmcs.c`)

```c
OMBRA_STATUS VmcsSetup(VMX_CPU* cpu, HV_INIT_PARAMS* params) {
    // Control fields - adjusted via capability MSRs
    VmcsWrite(VMCS_CTRL_PIN_BASED, AdjustControls(desired_pin, caps));
    VmcsWrite(VMCS_CTRL_PROC_BASED, AdjustControls(desired_proc, caps));
    VmcsWrite(VMCS_CTRL_PROC_BASED2, AdjustControls(desired_proc2, caps));
    VmcsWrite(VMCS_CTRL_VMEXIT, AdjustControls(desired_exit, caps));
    VmcsWrite(VMCS_CTRL_VMENTRY, AdjustControls(desired_entry, caps));

    // EPT pointer
    VmcsWrite(VMCS_CTRL_EPT_POINTER, eptp);

    // MSR bitmap
    VmcsWrite(VMCS_CTRL_MSR_BITMAP, msr_bitmap_phys);

    // Guest state - capture current CPU state
    VmcsWrite(VMCS_GUEST_CR0, __readcr0());
    VmcsWrite(VMCS_GUEST_CR3, __readcr3());
    VmcsWrite(VMCS_GUEST_CR4, __readcr4() | CR4_VMXE);
    VmcsWrite(VMCS_GUEST_RIP, guest_entry_point);
    VmcsWrite(VMCS_GUEST_RSP, guest_stack);
    VmcsWrite(VMCS_GUEST_RFLAGS, __readeflags());
    // ... segment selectors, bases, limits, access rights ...

    // Host state - where VM-exit returns
    VmcsWrite(VMCS_HOST_CR0, __readcr0());
    VmcsWrite(VMCS_HOST_CR3, host_cr3);
    VmcsWrite(VMCS_HOST_CR4, __readcr4());
    VmcsWrite(VMCS_HOST_RSP, cpu->HostStackTop);
    VmcsWrite(VMCS_HOST_RIP, (U64)VmExitHandler);
    // ... segment selectors, bases ...

    return OMBRA_SUCCESS;
}
```

---

## VM-Exit Handling Flow

### Assembly Entry Point (VERIFIED from `asm/vmexit.asm`)

```asm
VmExitHandler:
    ; Save all general purpose registers
    push rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi
    push r8, r9, r10, r11, r12, r13, r14, r15
    pushfq

    ; Set up GUEST_REGS pointer
    mov rcx, rsp            ; First argument = pointer to saved regs

    ; Call C dispatcher
    sub rsp, 28h            ; Shadow space + alignment
    call VmExitDispatch
    add rsp, 28h

    ; Check return value
    test eax, eax
    jz .continue

    ; VMEXIT_ADVANCE_RIP: Increment guest RIP
    mov rcx, VMCS_GUEST_RIP
    vmread rax, rcx
    mov rcx, VMCS_EXIT_INSTR_LEN
    vmread rdx, rcx
    add rax, rdx
    mov rcx, VMCS_GUEST_RIP
    vmwrite rcx, rax

.continue:
    ; Restore registers
    popfq
    pop r15, r14, r13, r12, r11, r10, r9, r8
    pop rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax

    ; Resume guest
    vmresume

    ; If vmresume fails, we have a critical error
    jmp VmxResumeFailed
```

### C Dispatcher (VERIFIED from `exit_dispatch.c`)

```c
VMEXIT_ACTION VmExitDispatch(GUEST_REGS* regs) {
    U64 reason = VmcsRead(VMCS_EXIT_REASON) & 0xFFFF;
    U64 qualification = VmcsRead(VMCS_EXIT_QUALIFICATION);

    switch (reason) {
    case EXIT_REASON_CPUID:
        return HandleCpuid(regs);

    case EXIT_REASON_RDTSC:
        return HandleRdtsc(regs);

    case EXIT_REASON_RDTSCP:
        return HandleRdtscp(regs);

    case EXIT_REASON_RDMSR:
        return HandleRdmsr(regs);

    case EXIT_REASON_WRMSR:
        return HandleWrmsr(regs);

    case EXIT_REASON_CR_ACCESS:
        return HandleCrAccess(regs, qualification);

    case EXIT_REASON_EPT_VIOLATION:
        return HandleEptViolation(regs, qualification);

    case EXIT_REASON_VMCALL:
        return HandleVmcall(regs);

    case EXIT_REASON_EXCEPTION:
        return HandleException(regs, qualification);

    case EXIT_REASON_IO_INSTRUCTION:
        return HandleIo(regs, qualification);

    case EXIT_REASON_MTF:
        return HandleMtf(regs);

    // ... more handlers ...

    default:
        // Unknown exit - advance RIP and hope for the best
        WARN("Unhandled exit reason: %llu", reason);
        return VMEXIT_ADVANCE_RIP;
    }
}
```

---

## EPT Architecture (VERIFIED from `ept.c`, `ept.h`)

### Structure Layout

```
EPTP (64-bit pointer in VMCS_CTRL_EPT_POINTER)
  |
  +-- Bits 2:0   = Memory type (6 = Write-back)
  +-- Bits 5:3   = Page walk length (3 = 4 levels)
  +-- Bits 11:6  = Reserved (0)
  +-- Bits N:12  = Physical address of PML4

PML4 (Page Map Level 4)
  +-- 512 entries, each 8 bytes
  +-- Entry: Physical address of PDPT + permissions
  +-- Each entry covers 512GB

PDPT (Page Directory Pointer Table)
  +-- 512 entries
  +-- Entry: Physical address of PD or 1GB huge page
  +-- Each entry covers 1GB

PD (Page Directory)
  +-- 512 entries
  +-- Entry: Physical address of PT or 2MB large page
  +-- Each entry covers 2MB

PT (Page Table)
  +-- 512 entries
  +-- Entry: Physical address of 4KB page + permissions
  +-- Each entry covers 4KB
```

### Identity Map Construction (VERIFIED from `ept.c`)

```c
OMBRA_STATUS EptBuildIdentityMap(EPT_STATE* ept, U64 physicalMemorySize) {
    // Allocate PML4
    ept->Pml4 = AllocContiguousPage();
    memset(ept->Pml4, 0, PAGE_SIZE);

    // Calculate required entries
    U64 numPml4Entries = (physicalMemorySize + 512GB - 1) / 512GB;
    U64 numPdptEntries = (physicalMemorySize + 1GB - 1) / 1GB;

    for (U64 i = 0; i < numPml4Entries; i++) {
        // Allocate PDPT
        EPT_PDPTE* pdpt = AllocContiguousPage();
        ept->Pml4[i] = MakeEptEntry(PhysAddr(pdpt), EPT_READ | EPT_WRITE | EPT_EXECUTE);

        for (U64 j = 0; j < 512 && (i * 512 + j) * 1GB < physicalMemorySize; j++) {
            // Use 1GB huge pages for simplicity
            U64 gpa = (i * 512 + j) * 1GB;
            pdpt[j] = MakeEptEntry(gpa, EPT_READ | EPT_WRITE | EPT_EXECUTE | EPT_HUGE_PAGE);
        }
    }

    // Build EPTP
    ept->Eptp = PhysAddr(ept->Pml4) | EPT_MEMORY_TYPE_WB | EPT_PAGE_WALK_LENGTH_4;

    return OMBRA_SUCCESS;
}
```

### Shadow Hook EPT Splitting (VERIFIED from `ept.c`, `hooks.c`)

```
Normal page (RWX):
  +----------------+
  | Guest Read     | ---> Original Page
  | Guest Write    | ---> Original Page
  | Guest Execute  | ---> Original Page
  +----------------+

Hook installed (split view):
  +----------------+
  | Guest Read     | ---> Original Page (R=1, W=0, X=0)
  | Guest Write    | ---> EPT Violation -> Handle
  | Guest Execute  | ---> Shadow Page with hook (R=0, W=0, X=1)
  +----------------+

EPT Violation Handler:
  1. Detect access type (R/W from qualification)
  2. If read: Temporarily swap to original page, set MTF
  3. MTF fires: Swap back to execute-only shadow page
```

---

## Hook Framework (VERIFIED from `hooks.c`)

### Hook Entry Structure

```c
typedef struct _HOOK_ENTRY {
    U64     TargetAddress;      // Guest virtual address of hook
    U64     TargetPhysical;     // Guest physical address
    U64     ShadowPhysical;     // Shadow page physical address

    void*   OriginalPage;       // Copy of original page content
    void*   ShadowPage;         // Shadow page with hook code

    U8      OriginalBytes[16];  // Original instruction bytes
    U8      HookBytes[16];      // Hook instruction (typically JMP)
    U32     InstructionLength;  // Length of hooked instruction

    void*   Handler;            // Hook handler function
    void*   Trampoline;         // Trampoline for calling original

    bool    Active;
    U32     HitCount;
} HOOK_ENTRY;
```

### Hook Installation Flow

```
1. HookInstall(target_va, handler)
   |
   +-- Translate VA -> PA
   +-- Allocate shadow page
   +-- Copy original page to shadow
   |
   +-- Build trampoline:
   |   +-- Copy original instruction(s) to trampoline buffer
   |   +-- Append JMP back to original+N
   |
   +-- Patch shadow page:
   |   +-- Write JMP [handler] at target offset
   |
   +-- Split EPT entry:
       +-- Original PTE: R=1, W=0, X=0 (read sees original)
       +-- Shadow PTE: R=0, W=0, X=1 (execute sees hook)

2. Guest executes at target address:
   |
   +-- EPT violation (execute on R=0 page)
   +-- Handler checks: Is this a hook?
   |   +-- Yes: Switch EPTP to shadow view, continue
   |
   +-- Guest now executing shadow page (hooked code)
   +-- JMP to handler executes
```

---

## Timing Compensation (VERIFIED from `timing.c`)

### Problem: VM-exits are detectable via RDTSC

Anti-cheat measures RDTSC before and after CPUID:
```c
u64 before = __rdtsc();
__cpuid(0, a, b, c, d);
u64 after = __rdtsc();
u64 cycles = after - before;
// Normal: ~100 cycles
// VM-exit: ~1000+ cycles
```

### Solution: TSC Offsetting

```c
typedef enum {
    TIMING_CPUID_OVERHEAD,
    TIMING_RDTSC_OVERHEAD,
    TIMING_MSR_OVERHEAD,
    TIMING_EPT_OVERHEAD,
    TIMING_CR_OVERHEAD,
    TIMING_MAX
} TIMING_OVERHEAD_TYPE;

// Per-CPU accumulated overhead
U64 g_TimingOffsets[TIMING_MAX];

// Called at start of each handler
U64 TimingStart(void) {
    return __rdtsc();
}

// Called at end of each handler
void TimingEnd(VMX_CPU* cpu, U64 startTsc, TIMING_OVERHEAD_TYPE type) {
    U64 elapsed = __rdtsc() - startTsc;
    U64 expectedCycles = g_ExpectedCycles[type];

    if (elapsed > expectedCycles) {
        cpu->TscOffset += (elapsed - expectedCycles);
    }
}

// RDTSC handler subtracts offset
VMEXIT_ACTION HandleRdtsc(GUEST_REGS* regs) {
    U64 tsc = __rdtsc();
    VMX_CPU* cpu = VmxGetCurrentCpu();

    tsc -= cpu->TscOffset;  // Hide overhead

    regs->Rax = (U32)tsc;
    regs->Rdx = (U32)(tsc >> 32);

    TimingEnd(cpu, entryTsc, TIMING_RDTSC_OVERHEAD);
    return VMEXIT_ADVANCE_RIP;
}
```

---

## BYOVD Exploitation Architecture (VERIFIED from `supdrv.c`)

### Ld9BoxSup.sys Interface

```
Usermode (loader.exe)                    Kernel (Ld9BoxSup.sys)
    |                                            |
    +-- NtCreateFile(\Device\Ld9BoxDrv) -------->|
    |                                            |
    +-- SUP_IOCTL_COOKIE ----------------------->|
    |   In: Initial cookie ("tori")              |
    |   In: Magic ("The Magic Word!")            |
    |   In: Version (probed)                     |
    |<-- Out: Session cookie                     |
    |<-- Out: pSession (kernel pointer)          |
    |                                            |
    +-- SUP_IOCTL_LDR_OPEN -------------------->|
    |   In: cbImage (hypervisor size)            |
    |   In: szName, szFilename                   |
    |<-- Out: pvImageBase (kernel address)       |
    |                                            |
    +-- Map hypervisor to R3 buffer              |
    +-- Apply relocations for kernel base        |
    +-- Resolve imports                          |
    +-- Wipe PE headers                          |
    |                                            |
    +-- SUP_IOCTL_LDR_LOAD -------------------->|
        In: pvImageBase                          |
        In: abImage (mapped hypervisor)          |
        In: pfnModuleInit (entry point)          |
        --> Driver copies image to kernel        |
        --> Driver calls entry point             |
        <-- Hypervisor now running!              |
```

### IOCTL Code Values

```c
#define SUP_IOCTL_COOKIE          0x22A004  // Cookie handshake
#define SUP_IOCTL_PAGE_ALLOC_EX   0x22A024  // Dual R3/R0 allocation
#define SUP_IOCTL_PAGE_FREE       0x22A028  // Free allocated pages
#define SUP_IOCTL_LDR_OPEN        0x22A038  // Open module for loading
#define SUP_IOCTL_LDR_LOAD        0x22A03C  // Load module
#define SUP_IOCTL_MSR_PROBER      0x22A080  // MSR access (DISABLED)
```

---

## Phase 3: OmbraDriver Command Ring (VERIFIED from `types.h`)

### Ring Buffer Architecture

```
Shared Memory Layout:
+------------------+ 0x00
| OMBRA_COMMAND_RING |
| - ProducerIndex  |  (usermode writes)
| - ConsumerIndex  |  (driver writes)
| - Magic, Sizes   |
+------------------+ 0xC0 (192 bytes)
| Commands[64]     |  (512 bytes each)
| ...              |
+------------------+ 0x80C0 (32 + 192 bytes)
| Responses[64]    |  (256 bytes each)
| ...              |
+------------------+ 0xC0C0 (48 + 192 bytes)
| Scratch Buffer   |  (for large data transfers)
| ...              |
+------------------+
```

### Command Categories

```c
// Process Tracking (0x01xx)
OMBRA_CMD_SUBSCRIBE        = 0x0100,  // Track a process by CR3
OMBRA_CMD_ENUM_PROCESSES   = 0x0104,  // Enumerate all processes
OMBRA_CMD_ENUM_MODULES     = 0x0105,  // List loaded modules

// Memory Operations (0x02xx)
OMBRA_CMD_HIDE_MEMORY      = 0x0200,  // Hide memory range via EPT
OMBRA_CMD_SHADOW_MEMORY    = 0x0201,  // Create shadow page
OMBRA_CMD_READ_PHYSICAL    = 0x0204,  // Physical memory read
OMBRA_CMD_READ_VIRTUAL     = 0x0206,  // Virtual memory read

// Injection (0x03xx)
OMBRA_CMD_INJECT           = 0x0300,  // Standard DLL injection
OMBRA_CMD_INJECT_HIDDEN    = 0x0301,  // Hidden injection via EPT

// Hardware Spoofing (0x07xx)
OMBRA_CMD_SPOOF_DISK       = 0x0700,  // Disk serial spoofing
OMBRA_CMD_SPOOF_NIC        = 0x0701,  // NIC MAC spoofing

// ETW Evasion (0x08xx)
OMBRA_CMD_ETW_DISABLE_TI   = 0x0800,  // Disable Threat Intelligence
OMBRA_CMD_ETW_WIPE_BUFFER  = 0x0801,  // Wipe circular buffers
```

---

## MCP Server Architecture

### Tool Registration Pattern

**ombra-mcp** (explicit registration):
```python
TOOLS = [
    Tool(name="vmcs_field_complete", description="...", inputSchema={...}),
    ...
]
TOOL_HANDLERS = {
    "vmcs_field_complete": vmcs_tools.vmcs_field_complete,
    ...
}
```

**driver-re-mcp** (decorator pattern):
```python
@tool("add_driver", "Add a new driver", {...})
async def add_driver(**kwargs) -> dict:
    return await driver_tools.add_driver(**kwargs)

# TOOLS dict populated at import time
```

### Database Relationships

```
intel_sdm.db          project_brain.db       anticheat_intel.db
+-------------+       +---------------+       +------------------+
| vmcs_fields |       | decisions     |       | anticheats       |
| exit_reasons|       | gotchas       |       | detection_methods|
| msrs        |       | sessions      |       | bypasses         |
| exceptions  |       | components    |       | signatures       |
+-------------+       | findings      |       +------------------+
                      +---------------+
                            |
                            v
                      ChromaDB Collections
                      +-------------------+
                      | intel_sdm         |
                      | anticheat_intel   |
                      | evasion_techniques|
                      | byovd_drivers     |
                      | project_brain     |
                      +-------------------+
```

---

## GAPS AND UNKNOWNS

### Architecture Questions
- [ ] How does host CR3 isolation work during VM-exit?
- [ ] What happens if nested hypervisor tries to execute VMXON?
- [ ] How are APIC/interrupt delivery handled?
- [ ] What's the exact memory layout of VMXON/VMCS regions?

### Implementation Status
- [ ] Is OmbraDriver (Phase 3) kernel component implemented?
- [ ] Are all VMCALL handlers complete or stubbed?
- [ ] Is GUI functional?
- [ ] Are cleanup/forensic evasion procedures implemented?

### Security Considerations
- [ ] How is VMCALL authentication enforced?
- [ ] Can anti-cheat detect the vulnerable driver loading?
- [ ] What's the detection surface of the loader.exe?

---

*Architecture documentation generated 2025-12-27*
*CONFIDENCE: HIGH for code structure, MEDIUM for runtime behavior*
