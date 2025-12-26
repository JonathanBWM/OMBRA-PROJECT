# OmbraHypervisor — Claude Code Implementation Prompt (With MCP)

## MCP Server Available

You have access to the **OmbraMCP** server with the following tools:

### Intel SDM Reference
- `vmcs_field(field_name)` — Get VMCS field encoding (e.g., "GUEST_RIP" → 0x681E)
- `vmcs_fields_by_category(category)` — Get all fields: "guest_state", "host_state", "control", "exit_info", "all"
- `exit_reason(reason)` — Get exit reason details (0-65)
- `exit_qualification_format(reason)` — Get qualification bit layout
- `msr_info(msr)` — Get MSR by name or number
- `search_sdm(query)` — Search Intel SDM content

### Code Generation
- `generate_vmcs_header()` — Complete vmcs_fields.h
- `generate_msr_header()` — Complete msr_defs.h  
- `generate_exit_handler_skeleton()` — exit_dispatch.c with all cases
- `generate_handler_template(reason)` — Handler for specific exit
- `generate_ept_structures()` — EPT type definitions
- `generate_asm_stub(type)` — Assembly: "vmexit" or "intrinsics"

### Driver Interface
- `ld9boxsup_ioctl_info(ioctl)` — Get IOCTL specification
- `list_available_ioctls()` — List all IOCTLs
- `get_driver_workflow()` — Recommended usage steps
- `generate_ioctl_wrapper(ioctl)` — Generate C wrapper function

### Detection Avoidance
- `check_for_signatures(code)` — Scan for known signatures
- `get_timing_thresholds()` — Detection timing limits
- `get_cpuid_detection_info()` — CPUID spoofing requirements
- `get_detection_recommendations()` — Best practices
- `generate_random_pool_tag()` — Safe pool tag

### Project Memory
- `get_implementation_status()` — Track progress
- `update_status(file, status, notes)` — Update file status
- `add_implementation_note(component, note)` — Document decisions
- `get_known_issues()` — Track blockers

**USE THESE TOOLS** instead of guessing encodings or structures. Example:
```
Need VMCS_GUEST_RIP encoding? → Call vmcs_field("GUEST_RIP")
Need exit qualification bits? → Call exit_qualification_format(48)
Need to generate header? → Call generate_vmcs_header()
```

---

## Project Overview

You are building **OmbraHypervisor**, a custom Intel VT-x hypervisor for Windows 10 x64.

### Requirements
1. **Completely original code** — No copied patterns from public repos
2. **Undetected** — CPUID spoofing, RDTSC timing < 150 cycles
3. **C + Assembly only** — No C++, no STL
4. **Loaded via Ld9BoxSup.sys** — VirtualBox SUPDrv vulnerable driver

---

## Architecture

### Project Structure
```
OmbraHypervisor/
├── usermode/
│   ├── main.c                      # Entry point
│   ├── driver_interface.c          # Ld9BoxSup IOCTL wrappers
│   └── payload_loader.c            # Copy & trigger hypervisor
│
├── hypervisor/
│   ├── entry.c                     # Per-CPU virtualization
│   ├── vmx.c                       # VMXON, setup
│   ├── vmcs.c                      # VMCS configuration
│   ├── ept.c                       # EPT identity map
│   ├── exit_dispatch.c             # Exit handler
│   ├── handlers/
│   │   ├── cpuid.c                 # CPUID + timing
│   │   ├── rdtsc.c                 # RDTSC + offset
│   │   ├── msr.c                   # MSR filtering
│   │   ├── cr_access.c             # CR shadowing
│   │   ├── ept_violation.c
│   │   └── vmcall.c                # Hypercall interface
│   ├── timing.c
│   └── asm/
│       ├── vmexit.asm              # Exit entry point
│       └── intrinsics.asm          # VMX instructions
│
├── shared/
│   ├── vmcs_fields.h               # → generate_vmcs_header()
│   ├── msr_defs.h                  # → generate_msr_header()
│   ├── exit_reasons.h
│   └── ept_defs.h                  # → generate_ept_structures()
│
└── docs/
```

### Execution Flow
```
USERMODE                           KERNEL (via Ld9BoxSup.sys)
────────                           ────────────────────────────
1. Open \\.\Ld9BoxSup
2. SUP_IOCTL_COOKIE
3. SUP_IOCTL_GET_HWVIRT_MSRS       → Get VMX capability MSRs
4. SUP_IOCTL_CONT_ALLOC × 3        → VMXON, VMCS, EPT regions
5. SUP_IOCTL_PAGE_ALLOC_EX         → Hypervisor code + stacks
6. Copy hypervisor to kernel
7. SUP_IOCTL_CALL_VMMR0            → entry()
                                      │
                                      ▼
                                   8. For each CPU:
                                      - CR4.VMXE = 1
                                      - VMXON
                                      - VMPTRLD
                                      - Configure VMCS
                                      - Setup EPT
                                      - VMLAUNCH
                                      │
                                      ▼
                                   9. CPU runs as guest
                                   
RUNTIME (every VM-exit):
────────────────────────
vmexit.asm → Save GPRs → exit_dispatch() → Handler → VMRESUME
```

---

## Critical: Timing Evasion

```c
// Anti-cheat detection method:
uint64_t start = __rdtsc();
__cpuid(regs, 1);
uint64_t end = __rdtsc();
if (end - start > 150) DETECTED;  // Hypervisor present

// Your solution:
typedef struct {
    uint64_t tsc_offset;      // Accumulated VM-exit overhead
} TIMING_STATE;

void handle_cpuid(VCPU* vcpu) {
    uint64_t tsc_start = __rdtsc();
    
    // Execute CPUID, spoof results
    __cpuidex(regs, vcpu->rax, vcpu->rcx);
    
    if (vcpu->rax == 1) {
        regs[2] &= ~(1 << 31);  // Clear hypervisor bit
        regs[2] &= ~(1 << 5);   // Clear VMX bit
    }
    
    if (vcpu->rax == 0x40000000) {
        regs[0] = regs[1] = regs[2] = regs[3] = 0;  // Hide vendor
    }
    
    // Track timing
    uint64_t tsc_end = __rdtsc();
    vcpu->timing.tsc_offset += (tsc_end - tsc_start);
    
    vcpu->rip += 2;
}

void handle_rdtsc(VCPU* vcpu) {
    uint64_t real_tsc = __rdtsc();
    uint64_t adjusted = real_tsc - vcpu->timing.tsc_offset;
    
    vcpu->rax = (uint32_t)adjusted;
    vcpu->rdx = (uint32_t)(adjusted >> 32);
    vcpu->rip += 2;
}
```

---

## Implementation Order

### Phase 1: Headers (Use MCP Tools)
```
Call: generate_vmcs_header()     → shared/vmcs_fields.h
Call: generate_msr_header()      → shared/msr_defs.h
Call: generate_ept_structures()  → shared/ept_defs.h
```

### Phase 2: Driver Interface
```
Call: get_driver_workflow()
Call: generate_ioctl_wrapper("SUP_IOCTL_COOKIE")
Call: generate_ioctl_wrapper("SUP_IOCTL_CONT_ALLOC")
Call: generate_ioctl_wrapper("SUP_IOCTL_GET_HWVIRT_MSRS")
Call: generate_ioctl_wrapper("SUP_IOCTL_CALL_VMMR0")
```

### Phase 3: Core VMX
```
Call: generate_asm_stub("intrinsics")  → hypervisor/asm/intrinsics.asm
Call: vmcs_fields_by_category("control")
Call: vmcs_fields_by_category("guest_state")
Call: vmcs_fields_by_category("host_state")
```

### Phase 4: Exit Handlers
```
Call: generate_exit_handler_skeleton()  → hypervisor/exit_dispatch.c
Call: generate_handler_template(10)     → handlers/cpuid.c
Call: generate_handler_template(16)     → handlers/rdtsc.c
Call: generate_handler_template(48)     → handlers/ept_violation.c
Call: generate_asm_stub("vmexit")       → hypervisor/asm/vmexit.asm
```

### Phase 5: EPT
```
Call: generate_ept_structures()
Call: search_sdm("EPT identity map")
```

### Phase 6: Integration & Testing
```
Call: get_detection_recommendations()
Call: check_for_signatures("<your code>")
Call: get_timing_thresholds()
```

---

## Detection Avoidance Checklist

| Vector | Mitigation | MCP Tool |
|--------|------------|----------|
| CPUID.1:ECX[31] | Clear bit | get_cpuid_detection_info() |
| CPUID timing | TSC offset | get_timing_thresholds() |
| Pool tags | Random | generate_random_pool_tag() |
| Code patterns | Unique | check_for_signatures() |
| VMX MSRs | Inject #GP | msr_info() |

---

## Build Configuration

```makefile
CC = cl.exe
AS = ml64.exe

CFLAGS = /kernel /GS- /Oi /W4 /O2 /Zl /GR- /EHs-c-
ASFLAGS = /c /Cx

hypervisor.sys: $(OBJS)
    link /DRIVER /ENTRY:DriverEntry /SUBSYSTEM:NATIVE \
         /NODEFAULTLIB ntoskrnl.lib hal.lib $(OBJS) /OUT:$@
```

---

## Success Criteria

1. ✅ All CPUs virtualized without BSOD
2. ✅ CPUID.1:ECX[31] returns 0
3. ✅ RDTSC timing < 150 cycles
4. ✅ No signature matches
5. ✅ Hypercall interface working

---

## Begin

Start with Phase 1. Generate headers using MCP tools, then proceed to driver interface.

**Remember:** Always use MCP tools for encodings and specifications. Don't guess.
