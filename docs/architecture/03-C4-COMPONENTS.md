# C4 Model: Component Diagram

## Hypervisor Core Components

```mermaid
C4Component
    title Hypervisor Core Component Diagram

    Container_Boundary(hv_core, "Hypervisor Core") {
        Component(vmx, "VMX Engine", "vmx.c", "VMXON/VMXOFF, per-CPU init")
        Component(vmcs, "VMCS Manager", "vmcs.c", "Field setup, validation")
        Component(entry, "Entry Point", "entry.c", "Bootstrap, IPI broadcast")
        Component(dispatch, "Exit Dispatcher", "exit_dispatch.c", "Route exits to handlers")
        Component(ept, "EPT Manager", "ept.c", "Identity map, page split")
        Component(timing, "Timing Engine", "timing.c", "TSC/APERF compensation")
        Component(hooks, "Hook Manager", "hooks.c", "Shadow page management")
        Component(nested, "Nested VMX", "nested.c", "L1/L2 guest support")
        Component(spoof, "Spoofer", "spoof.c", "CPUID/MSR spoofing")
    }

    Container_Boundary(handlers, "Exit Handlers") {
        Component(h_cpuid, "CPUID Handler", "cpuid.c", "Leaf spoofing")
        Component(h_rdtsc, "RDTSC Handler", "rdtsc.c", "Timing compensation")
        Component(h_msr, "MSR Handler", "msr.c", "MSR virtualization")
        Component(h_cr, "CR Handler", "cr_access.c", "CR0/3/4 trapping")
        Component(h_ept, "EPT Handler", "ept_violation.c", "Page fault handling")
        Component(h_vmcall, "VMCALL Handler", "vmcall.c", "Command dispatch")
    }

    Rel(entry, vmx, "Initializes")
    Rel(vmx, vmcs, "Configures")
    Rel(vmx, ept, "Sets up")
    Rel(dispatch, h_cpuid, "Routes exit 10")
    Rel(dispatch, h_rdtsc, "Routes exit 16")
    Rel(dispatch, h_msr, "Routes exit 31/32")
    Rel(dispatch, h_vmcall, "Routes exit 18")
    Rel(dispatch, h_ept, "Routes exit 48")
    Rel(h_cpuid, spoof, "Uses")
    Rel(h_rdtsc, timing, "Uses")
    Rel(h_ept, hooks, "Consults")
```

## Component Details: Hypervisor Core

| Component | File | Size | Responsibility |
|-----------|------|------|----------------|
| **VMX Engine** | vmx.c/h | 12.5KB | VMXON/VMXOFF lifecycle, per-CPU VMX_CPU structures, MSR capability checking |
| **VMCS Manager** | vmcs.c/h | 23KB | VMCS initialization, control/guest/host field setup, field validation |
| **Entry Point** | entry.c | 12KB | Hypervisor bootstrap, `HvEntry()`, multi-CPU IPI via `KeIpiGenericCall` |
| **Exit Dispatcher** | exit_dispatch.c/h | 7.2KB | Read EXIT_REASON, switch to appropriate handler, handle unimplemented exits |
| **EPT Manager** | ept.c/h | 29KB | Identity mapping, large page optimization, 4KB splitting, `INVEPT` |
| **Timing Engine** | timing.c/h | 17KB | TSC offset calculation, APERF/MPERF compensation, exit cost tracking |
| **Hook Manager** | hooks.c/h | 44KB | Shadow page allocation, execute-only pages, read/execute split |
| **Nested VMX** | nested.c/h | 21KB | L1/L2 guest support, VMCS shadowing (partial) |
| **Spoofer** | spoof.c/h | 8.8KB | CPUID leaf modification, MSR hiding, SMBIOS spoofing |

## Component Details: Exit Handlers

| Handler | Exit Reason | File | Purpose |
|---------|-------------|------|---------|
| **CPUID** | 10 | cpuid.c | Mask hypervisor bit, spoof vendor ID, modify feature flags |
| **RDTSC** | 16 | rdtsc.c | Apply TSC offset, compensate for exit latency |
| **MSR** | 31/32 | msr.c | Virtualize VMX MSRs, shadow values, passthrough optimization |
| **CR Access** | 28 | cr_access.c | Trap CR0.WP changes, CR3 updates, CR4.VMXE |
| **EPT Violation** | 48 | ept_violation.c | Handle page faults, update EPT entries, trigger hooks |
| **EPT Misconfig** | 49 | ept_misconfig.c | Debug EPT configuration errors |
| **VMCALL** | 18 | vmcall.c | Authenticate caller, dispatch 30+ command codes |
| **Exception** | 0 | exception.c | Inject exceptions, handle NMI, debug traps |
| **I/O** | 30 | io.c | Virtualize I/O ports |
| **MTF** | 37 | mtf.c | Single-step execution for debugging |
| **Power** | 1 | power_mgmt.c | Handle HLT, power state transitions |

## Driver Layer Components

```mermaid
C4Component
    title Driver Layer Component Diagram

    Container_Boundary(driver, "ombra.sys") {
        Component(init, "Init", "init.c", "DriverEntry, device creation")
        Component(d_dispatch, "Dispatcher", "dispatch.c", "IRP routing, command dispatch")
        Component(vmcall, "VMCALL Wrapper", "vmcall.c", "VMCALL with auth")
        Component(mem_ops, "Memory Ops", "memory_ops.c", "Physical/virtual R/W")
        Component(protect, "Protection", "protection.c", "Memory hiding via EPT")
        Component(shadow, "Shadow Mgr", "shadow.c", "Hook installation")
        Component(d_spoof, "Spoofer", "spoof.c", "Disk/NIC/Volume spoofing")
        Component(etw, "ETW Manager", "etw.c", "Trace wiping")
        Component(modlock, "Module Lock", "module_lock.c", "Hide from enumeration")
    }

    Rel(init, d_dispatch, "Creates device")
    Rel(d_dispatch, vmcall, "Uses for HV ops")
    Rel(d_dispatch, mem_ops, "Uses for memory")
    Rel(d_dispatch, protect, "Uses for hiding")
    Rel(protect, vmcall, "Calls")
    Rel(shadow, vmcall, "Calls")
```

## Component Details: Driver

| Component | File | Commands |
|-----------|------|----------|
| **Init** | init.c | DriverEntry, IRP_MJ_CREATE/CLOSE |
| **Dispatcher** | dispatch.c | 30+ command handlers |
| **VMCALL Wrapper** | vmcall.c | Authenticated VMCALL execution |
| **Memory Ops** | memory_ops.c | ReadPhysical, WritePhysical, ReadVirtual, WriteVirtual |
| **Protection** | protection.c | HideMemory, ShadowMemory via EPT |
| **Shadow Manager** | shadow.c | InstallShadow, RemoveShadow |
| **Spoofer** | spoof.c | SpoofDisk, SpoofNic, SpoofVolume |
| **ETW Manager** | etw.c | EtwDisableTi, EtwWipeBuffer |
| **Module Lock** | module_lock.c | Hide from PEB/LDR enumeration |

## MCP Server Components

```mermaid
C4Component
    title MCP Server Component Diagram

    Container_Boundary(ombra_mcp, "ombra-mcp") {
        Component(server, "Server", "server.py", "MCP protocol handling")
        Component(sdm, "SDM Query", "sdm_query.py", "Intel SDM tools")
        Component(codegen, "Code Gen", "code_generator.py", "VMCS/handler generation")
        Component(stealth, "Stealth", "stealth.py", "Detection auditing")
        Component(validator, "Validator", "vmcs_validator.py", "Config validation")
        Component(brain, "Project Brain", "project_brain.py", "State tracking")
        Component(semantic, "Semantic", "semantic_search.py", "ChromaDB search")
    }

    ContainerDb(intel_db, "intel_sdm.db")
    ContainerDb(project_db, "project_brain.db")
    ContainerDb(chroma, "ChromaDB")

    Rel(server, sdm, "Calls handlers")
    Rel(server, codegen, "Calls handlers")
    Rel(server, stealth, "Calls handlers")
    Rel(sdm, intel_db, "Queries")
    Rel(brain, project_db, "Reads/Writes")
    Rel(semantic, chroma, "Vector search")
```

## Component Details: ombra-mcp

| Component | File | Tools |
|-----------|------|-------|
| **Server** | server.py | list_tools, call_tool, resource handlers |
| **SDM Query** | sdm_query.py | vmcs_field_complete, exit_reason_complete, get_msr_info |
| **Code Gen** | code_generator.py | generate_vmcs_setup, generate_exit_handler, generate_ept_setup |
| **Stealth** | stealth.py | get_detection_vectors, audit_stealth, generate_timing_compensation |
| **Validator** | vmcs_validator.py | validate_vmcs_setup, get_vmcs_checklist |
| **Project Brain** | project_brain.py | add_decision, add_gotcha, get_findings |
| **Semantic** | semantic_search.py | semantic_search, rebuild_semantic_index |

## Data Structures

### VMX_CPU (Per-CPU State)

```c
typedef struct _VMX_CPU {
    // Identification
    U32 CpuId;
    BOOLEAN Active;

    // VMX Regions (page-aligned)
    void *VmxonRegion;      // 4KB VMXON region
    void *VmcsRegion;       // 4KB VMCS region

    // Host Stack
    void *HostStackBase;    // 16KB host stack
    void *HostStackTop;

    // MSR Bitmap
    void *MsrBitmap;        // 4KB MSR intercept bitmap

    // EPT (shared reference)
    struct _EPT_STATE* Ept;

    // Timing Compensation
    U64 VmexitCount;
    U64 TscOffset;
    U64 AperfOffset;
    U64 MperfOffset;
    U64 LastExitTsc;

    // Per-CPU hooks
    struct _HOOK_ENTRY* Hooks[MAX_HOOKS];
} VMX_CPU;
```

### OMBRA_COMMAND (Driver Command)

```c
typedef struct _OMBRA_COMMAND {
    U32 CommandId;          // Command code
    U32 Status;             // Result status
    U64 Param1;             // Input parameter 1
    U64 Param2;             // Input parameter 2
    U64 Param3;             // Input parameter 3
    U64 Param4;             // Input parameter 4
    U64 Result1;            // Output value 1
    U64 Result2;            // Output value 2
} OMBRA_COMMAND;
```

## Related Documents

- [02-C4-CONTAINERS.md](./02-C4-CONTAINERS.md) - Container architecture
- [04-DATA-ARCHITECTURE.md](./04-DATA-ARCHITECTURE.md) - Database details
