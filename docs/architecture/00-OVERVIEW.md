# PROJECT-OMBRA Architecture Documentation

## Overview

PROJECT-OMBRA is a sophisticated Intel VMX-based hypervisor platform designed for advanced memory virtualization, anti-detection, and driver reverse engineering on Windows x64 systems.

**Total Codebase**: ~46,095 LOC
- Hypervisor/Driver: ~34,780 LOC (C + x64 MASM)
- MCP Server Layer: ~11,315 LOC (Python)

## System Layers

```
┌─────────────────────────────────────────────────────────────────┐
│                   Claude Code Interface                         │
│                (MCP Protocol via stdio)                         │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                     MCP Server Layer                            │
│  ┌─────────────────────────┐  ┌─────────────────────────────┐  │
│  │    ombra-mcp (152)      │  │    driver-re-mcp (59)       │  │
│  │  • Intel SDM reference  │  │  • Driver analysis          │  │
│  │  • Code generation      │  │  • IOCTL vulnerability      │  │
│  │  • Stealth auditing     │  │  • Import/export analysis   │  │
│  │  • Anti-cheat intel     │  │  • Ghidra integration       │  │
│  └─────────────────────────┘  └─────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                    Usermode Layer (Ring 3)                      │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────────────┐    │
│  │ loader.exe   │ │ PE Mapper    │ │ BYOVD Exploitation   │    │
│  │              │ │              │ │ • supdrv.c           │    │
│  │ hv_loader.c  │ │ pe_parser.c  │ │ • throttlestop.c     │    │
│  │ ombra_client │ │ pe_mapper.c  │ │ • deployer.c         │    │
│  └──────────────┘ └──────────────┘ └──────────────────────┘    │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼ DeviceIoControl (IOCTL)
┌─────────────────────────────────────────────────────────────────┐
│                    Driver Layer (Ring 0)                        │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────────────┐    │
│  │ dispatch.c   │ │ memory_ops.c │ │ Anti-Forensics       │    │
│  │ 30+ commands │ │              │ │ • etw.c              │    │
│  │              │ │ protection.c │ │ • module_lock.c      │    │
│  │ vmcall.c     │ │ shadow.c     │ │ • spoof.c            │    │
│  └──────────────┘ └──────────────┘ └──────────────────────┘    │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼ VMCALL (Ring -1 transition)
┌─────────────────────────────────────────────────────────────────┐
│                 Hypervisor Core (VMX Root)                      │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────────────┐    │
│  │ VMX Engine   │ │ Memory Virt  │ │ Anti-Detection       │    │
│  │ • vmx.c      │ │ • ept.c      │ │ • timing.c           │    │
│  │ • vmcs.c     │ │ • hooks.c    │ │ • spoof.c            │    │
│  │ • entry.c    │ │ • nested.c   │ │ • handlers/cpuid.c   │    │
│  └──────────────┘ └──────────────┘ └──────────────────────┘    │
│                                                                 │
│  Exit Handlers: cpuid, rdtsc, msr, cr_access, ept_violation,   │
│                 vmcall, io, mtf, exception, power_mgmt...       │
└─────────────────────────────────────────────────────────────────┘
```

## Document Index

| Document | Purpose |
|----------|---------|
| [01-C4-CONTEXT.md](./01-C4-CONTEXT.md) | System context diagram and external relationships |
| [02-C4-CONTAINERS.md](./02-C4-CONTAINERS.md) | Container architecture and communication |
| [03-C4-COMPONENTS.md](./03-C4-COMPONENTS.md) | Component-level architecture details |
| [04-DATA-ARCHITECTURE.md](./04-DATA-ARCHITECTURE.md) | Database schemas and data flows |
| [05-SECURITY-ARCHITECTURE.md](./05-SECURITY-ARCHITECTURE.md) | Security model and threat analysis |
| [06-ADR-INDEX.md](./06-ADR-INDEX.md) | Architecture Decision Records index |
| [adr/](./adr/) | Individual ADR documents |

## Key Technologies

| Layer | Technology | Purpose |
|-------|------------|---------|
| MCP | Python 3.10+, MCP SDK | Development intelligence |
| Vector Search | ChromaDB, all-MiniLM-L6-v2 | Semantic code search |
| Database | SQLite (8 databases) | Reference data, project state |
| Hypervisor | Pure C, x64 MASM | VMX virtualization |
| Driver | C (MSVC) | Kernel-mode interface |
| Build | MSVC cl.exe, ml64.exe | Compilation |

## Repository Structure

```
PROJECT-OMBRA/
├── hypervisor/
│   ├── hypervisor/      # VMX core (vmx.c, vmcs.c, ept.c, handlers/)
│   ├── driver/          # Kernel driver (dispatch.c, vmcall.c)
│   ├── usermode/        # Loader, BYOVD, GUI
│   ├── shared/          # Common types (types.h, vmcs_fields.h)
│   └── build/           # Build scripts
├── ombra_mcp_server/    # Main MCP server (152 tools)
├── driver-re-mcp/       # Driver RE MCP server (59 tools)
├── docs/
│   ├── architecture/    # This documentation
│   ├── intel_hypervisor.pdf
│   └── ...
└── CLAUDE.md            # ENI instructions
```

## Quick Reference

### Build Commands

```bash
# Hypervisor (Windows)
cd hypervisor/build && build.bat

# MCP Server
cd ombra_mcp_server && pip install -e . && ombra-mcp
```

### MCP Tool Discovery

```bash
mcp-cli tools ombra          # 152 tools
mcp-cli tools driver-re      # 59 tools
mcp-cli info ombra/<tool>    # Schema inspection
```

### Critical Files

| File | Purpose |
|------|---------|
| `hypervisor/hypervisor/entry.c` | Hypervisor bootstrap entry |
| `hypervisor/hypervisor/exit_dispatch.c` | VM-exit dispatcher |
| `hypervisor/driver/dispatch.c` | Driver command dispatch |
| `ombra_mcp_server/src/ombra_mcp/server.py` | MCP server entry |
| `hypervisor/shared/types.h` | Shared type definitions |
