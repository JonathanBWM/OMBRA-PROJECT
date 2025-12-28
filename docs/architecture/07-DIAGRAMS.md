# Architecture Diagrams

This document contains all Mermaid diagrams for PROJECT-OMBRA architecture visualization.

## System Overview

```mermaid
graph TB
    subgraph "Development Layer"
        DEV[Developer]
        CLAUDE[Claude Code]
        MCP1[ombra-mcp<br/>152 tools]
        MCP2[driver-re-mcp<br/>59 tools]
    end

    subgraph "Usermode Layer"
        LOADER[loader.exe]
        BYOVD[BYOVD Chain]
        CLIENT[Ombra Client]
    end

    subgraph "Kernel Layer"
        DRIVER[ombra.sys]
        LD9[Ld9BoxSup.sys]
    end

    subgraph "Hypervisor Layer"
        HV[Hypervisor Core]
        EPT[EPT Tables]
        HOOKS[Shadow Hooks]
    end

    DEV --> CLAUDE
    CLAUDE --> MCP1
    CLAUDE --> MCP2
    LOADER --> BYOVD
    BYOVD --> LD9
    LD9 --> HV
    CLIENT --> DRIVER
    DRIVER --> HV
    HV --> EPT
    HV --> HOOKS
```

## VM-Exit Processing Flow

```mermaid
sequenceDiagram
    participant Guest
    participant CPU
    participant ASM as vmexit.asm
    participant Dispatch as exit_dispatch.c
    participant Handler
    participant VMCS

    Guest->>CPU: Triggering instruction
    CPU->>CPU: VM-Exit
    CPU->>ASM: Save guest registers
    ASM->>Dispatch: VmexitDispatch()
    Dispatch->>VMCS: vmread(EXIT_REASON)
    VMCS-->>Dispatch: reason_number
    Dispatch->>Handler: Call handler[reason]
    Handler->>Handler: Process exit
    Handler->>VMCS: vmwrite(fields)
    Handler-->>Dispatch: status
    Dispatch-->>ASM: return
    ASM->>CPU: Restore registers
    CPU->>CPU: VMRESUME
    CPU->>Guest: Continue
```

## VMCALL Protocol

```mermaid
sequenceDiagram
    participant Driver as ombra.sys
    participant CPU
    participant HV as Hypervisor
    participant Handler as vmcall.c

    Driver->>CPU: VMCALL instruction
    Note over Driver: RAX=magic, RCX=cmd, RDX=key
    CPU->>CPU: VM-Exit (reason 18)
    CPU->>HV: exit_dispatch
    HV->>Handler: HandleVmcall()
    Handler->>Handler: Validate magic/key
    alt Valid auth
        Handler->>Handler: Execute command
        Handler-->>HV: status in RAX
    else Invalid auth
        Handler-->>HV: ERROR in RAX
    end
    HV->>CPU: VMRESUME
    CPU->>Driver: Continue (result in RAX)
```

## EPT Split-View Hook

```mermaid
graph TB
    subgraph "Guest View"
        GVA[Guest Virtual Address]
        GPA[Guest Physical Address]
    end

    subgraph "EPT Mapping"
        EPT[EPT Tables]
        EXEC[Execute Page<br/>Hook Code]
        READ[Read Page<br/>Original Code]
    end

    subgraph "Physical Memory"
        HPA1[HPA: Hook Trampoline]
        HPA2[HPA: Original Bytes]
    end

    GVA --> GPA
    GPA --> EPT
    EPT -->|Execute Access| EXEC
    EPT -->|Read Access| READ
    EXEC --> HPA1
    READ --> HPA2

    style EXEC fill:#f96
    style READ fill:#9f6
```

## MCP Tool Architecture

```mermaid
graph LR
    subgraph "Claude Code"
        CLI[mcp-cli]
    end

    subgraph "ombra-mcp"
        SERVER1[server.py]
        SDM[sdm_query.py]
        GEN[code_generator.py]
        STEALTH[stealth.py]
        BRAIN[project_brain.py]
    end

    subgraph "Databases"
        DB1[(intel_sdm.db)]
        DB2[(project_brain.db)]
        DB3[(ChromaDB)]
    end

    CLI -->|stdio| SERVER1
    SERVER1 --> SDM
    SERVER1 --> GEN
    SERVER1 --> STEALTH
    SERVER1 --> BRAIN
    SDM --> DB1
    BRAIN --> DB2
    SDM --> DB3
    BRAIN --> DB3
```

## Database Relationships

```mermaid
erDiagram
    DRIVERS ||--o{ SECTIONS : has
    DRIVERS ||--o{ IMPORTS : has
    DRIVERS ||--o{ EXPORTS : has
    DRIVERS ||--o{ IOCTLS : has
    DRIVERS ||--o{ FUNCTIONS : has
    DRIVERS ||--o{ VULNERABILITIES : has

    FUNCTIONS ||--o{ XREFS : from
    FUNCTIONS ||--o{ XREFS : to

    STRUCTURES ||--o{ STRUCTURE_MEMBERS : contains

    IOCTLS ||--o| FUNCTIONS : handler
    IOCTLS ||--o| STRUCTURES : input
    IOCTLS ||--o| STRUCTURES : output

    VULNERABILITIES ||--o{ ATTACK_CHAINS : uses

    ANALYSIS_SESSIONS ||--o{ ANALYSIS_NOTES : contains
```

## Timing Compensation

```mermaid
graph TB
    subgraph "Without Compensation"
        A1[Guest RDTSC: 1000]
        A2[VM-Exit: +500 cycles]
        A3[Handler: +200 cycles]
        A4[VM-Entry: +300 cycles]
        A5[Guest RDTSC: 2000]
        A6[Delta: 1000 cycles<br/>DETECTED]
    end

    subgraph "With Compensation"
        B1[Guest RDTSC: 1000]
        B2[VM-Exit: +500 cycles]
        B3[Handler: +200 cycles<br/>Record overhead]
        B4[VM-Entry: +300 cycles]
        B5[Guest RDTSC: 2000<br/>Apply -1000 offset]
        B6[Delta: 0 cycles<br/>HIDDEN]
    end

    A1 --> A2 --> A3 --> A4 --> A5 --> A6
    B1 --> B2 --> B3 --> B4 --> B5 --> B6

    style A6 fill:#f66
    style B6 fill:#6f6
```

## BYOVD Exploitation Chain

```mermaid
sequenceDiagram
    participant Loader
    participant LD9 as Ld9BoxSup.sys
    participant TS as ThrottleStop
    participant Kernel
    participant HV as Hypervisor

    Loader->>LD9: Load driver
    LD9->>LD9: SUP_IOCTL_COOKIE
    LD9-->>Loader: Session established

    Loader->>LD9: SUP_IOCTL_LDR_OPEN
    LD9-->>Loader: -618 (validation fail)

    Note over Loader: Need to bypass validation

    Loader->>TS: Load driver
    TS->>Kernel: Physical memory access
    Loader->>TS: Read SYSTEM EPROCESS
    Loader->>TS: Walk page tables
    Loader->>TS: Patch validation flags
    Loader->>TS: Unload

    Loader->>LD9: SUP_IOCTL_LDR_OPEN (retry)
    LD9-->>Loader: Success

    Loader->>LD9: SUP_IOCTL_LDR_LOAD
    Note over LD9: Copy payload, call entry
    LD9->>HV: HvEntry()
    HV->>HV: Initialize VMX
    HV-->>LD9: Success
    LD9-->>Loader: Hypervisor active
```

## Anti-Detection Layers

```mermaid
graph TB
    subgraph "Detection Attempts"
        D1[CPUID Timing]
        D2[Hypervisor Bit]
        D3[PML4E Scan]
        D4[BigPool Scan]
        D5[ETW Trace]
    end

    subgraph "Mitigations"
        M1[TSC Offset]
        M2[Bit Masking]
        M3[EPT Hiding]
        M4[MDL Allocation]
        M5[Buffer Wipe]
    end

    subgraph "Result"
        R[Undetected]
    end

    D1 --> M1
    D2 --> M2
    D3 --> M3
    D4 --> M4
    D5 --> M5

    M1 --> R
    M2 --> R
    M3 --> R
    M4 --> R
    M5 --> R

    style R fill:#6f6
```

## Component Dependency Graph

```mermaid
graph TD
    subgraph "Shared"
        TYPES[types.h]
        VMCS_FIELDS[vmcs_fields.h]
    end

    subgraph "Hypervisor Core"
        VMX[vmx.c]
        VMCS[vmcs.c]
        EPT[ept.c]
        DISPATCH[exit_dispatch.c]
        HANDLERS[handlers/*.c]
    end

    subgraph "Assembly"
        ASM_EXIT[vmexit.asm]
        ASM_INTR[intrinsics.asm]
    end

    subgraph "Driver"
        DRV_DISPATCH[dispatch.c]
        DRV_VMCALL[vmcall.c]
        DRV_MEM[memory_ops.c]
    end

    TYPES --> VMX
    TYPES --> VMCS
    TYPES --> EPT
    VMCS_FIELDS --> VMCS

    VMX --> VMCS
    VMX --> EPT
    VMCS --> ASM_INTR
    DISPATCH --> HANDLERS
    ASM_EXIT --> DISPATCH

    DRV_DISPATCH --> DRV_VMCALL
    DRV_VMCALL --> ASM_INTR
    DRV_MEM --> DRV_VMCALL
```

## Deployment Architecture

```mermaid
graph TB
    subgraph "Development Machine"
        subgraph "Claude Code Environment"
            CLAUDE[Claude Code CLI]
            MCP_OMBRA[ombra-mcp process]
            MCP_DRE[driver-re-mcp process]
        end

        subgraph "Build System"
            VS[Visual Studio 2019+]
            BUILD[build.bat]
        end

        subgraph "Artifacts"
            LIB[hypervisor.lib]
            LOADER_EXE[loader.exe]
        end
    end

    subgraph "Target Machine"
        subgraph "Usermode"
            LOADER_RUN[loader.exe running]
        end

        subgraph "Kernel"
            DRIVER_LOADED[ombra.sys loaded]
        end

        subgraph "VMX Root"
            HV_ACTIVE[Hypervisor active]
        end
    end

    CLAUDE --> MCP_OMBRA
    CLAUDE --> MCP_DRE
    VS --> BUILD
    BUILD --> LIB
    BUILD --> LOADER_EXE
    LOADER_EXE -.-> LOADER_RUN
    LOADER_RUN --> DRIVER_LOADED
    DRIVER_LOADED --> HV_ACTIVE
```

## Usage

These diagrams use [Mermaid](https://mermaid.js.org/) syntax. To render:

1. **VS Code**: Install "Mermaid Markdown Preview" extension
2. **GitHub**: Renders natively in markdown files
3. **Docusaurus**: Use `@docusaurus/theme-mermaid` plugin
4. **Online**: Paste into [Mermaid Live Editor](https://mermaid.live/)

## Related Documents

- [00-OVERVIEW.md](./00-OVERVIEW.md) - Architecture overview
- [03-C4-COMPONENTS.md](./03-C4-COMPONENTS.md) - Component details
- [05-SECURITY-ARCHITECTURE.md](./05-SECURITY-ARCHITECTURE.md) - Security model
