# Architecture Decision Records (ADRs)

## Overview

This document indexes all Architecture Decision Records for PROJECT-OMBRA. ADRs capture significant architectural decisions, their context, and consequences.

## ADR Template

Each ADR follows this structure:

```markdown
# ADR-NNN: Title

## Status
[Proposed | Accepted | Deprecated | Superseded by ADR-XXX]

## Context
What is the issue motivating this decision?

## Decision
What is the change being proposed/implemented?

## Consequences
What are the positive and negative effects?

## Related
Links to related ADRs, code, or documentation.
```

## Decision Index

### Hypervisor Architecture

| ADR | Title | Status | Date |
|-----|-------|--------|------|
| [ADR-001](./adr/ADR-001-pure-c-hypervisor.md) | Pure C for Hypervisor Core | Accepted | 2024-10 |
| [ADR-002](./adr/ADR-002-per-cpu-structures.md) | Per-CPU VMX Structures | Accepted | 2024-10 |
| [ADR-003](./adr/ADR-003-shared-ept.md) | Shared EPT Across CPUs | Accepted | 2024-10 |
| [ADR-004](./adr/ADR-004-vmcall-magic.md) | VMCALL Magic Authentication | Accepted | 2024-11 |

### Anti-Detection Strategy

| ADR | Title | Status | Date |
|-----|-------|--------|------|
| [ADR-010](./adr/ADR-010-timing-compensation.md) | TSC/APERF/MPERF Timing Compensation | Accepted | 2024-11 |
| [ADR-011](./adr/ADR-011-ept-split-view.md) | EPT Split-View for Shadow Hooks | Accepted | 2024-11 |
| [ADR-012](./adr/ADR-012-cpuid-spoofing.md) | CPUID Hypervisor Bit Masking | Accepted | 2024-11 |

### MCP Architecture

| ADR | Title | Status | Date |
|-----|-------|--------|------|
| [ADR-020](./adr/ADR-020-dual-mcp-servers.md) | Dual MCP Server Architecture | Accepted | 2024-12 |
| [ADR-021](./adr/ADR-021-sqlite-databases.md) | SQLite for Reference Data | Accepted | 2024-12 |
| [ADR-022](./adr/ADR-022-chromadb-vectors.md) | ChromaDB for Semantic Search | Accepted | 2024-12 |
| [ADR-023](./adr/ADR-023-explicit-vs-decorator.md) | Explicit vs Decorator Tool Registration | Accepted | 2024-12 |

### BYOVD Strategy

| ADR | Title | Status | Date |
|-----|-------|--------|------|
| [ADR-030](./adr/ADR-030-two-driver-chain.md) | Two-Driver BYOVD Chain | Accepted | 2024-11 |
| [ADR-031](./adr/ADR-031-ld9boxsup-selection.md) | Ld9BoxSup.sys as Primary Driver | Accepted | 2024-11 |

## Decision Log

### 2024-12

**ADR-023: Explicit vs Decorator Tool Registration**
- **Context**: ombra-mcp uses explicit TOOLS/TOOL_HANDLERS, driver-re-mcp uses @tool decorator
- **Decision**: Keep both patterns - explicit for large server (152 tools), decorator for smaller (59)
- **Consequence**: Different patterns must be documented for contributors

**ADR-022: ChromaDB for Semantic Search**
- **Context**: Need semantic search across 7 document collections
- **Decision**: Use ChromaDB with all-MiniLM-L6-v2 (384-dim ONNX)
- **Consequence**: No PyTorch dependency, but locked to ONNX runtime

### 2024-11

**ADR-011: EPT Split-View for Shadow Hooks**
- **Context**: Need to intercept execution while passing integrity checks
- **Decision**: Maintain separate EPT pages for execute vs read access
- **Consequence**: Increased complexity but effective anti-detection

**ADR-010: Timing Compensation**
- **Context**: VM-exits introduce measurable latency (200-2000 cycles)
- **Decision**: Track exit count, compensate TSC/APERF/MPERF reads
- **Consequence**: Defeats most timing-based detection

### 2024-10

**ADR-001: Pure C for Hypervisor Core**
- **Context**: Kernel code requires maximum control, no runtime dependencies
- **Decision**: Pure C with x64 MASM, no C++ in hypervisor core
- **Consequence**: Manual memory management, but predictable behavior

## Pending Decisions

| Topic | Status | Blocking |
|-------|--------|----------|
| Nested VMX VMCS shadowing | Under Review | L1/L2 guest support |
| Physical memory pool allocator | Proposed | Large allocation stealth |
| Working set query spoofing | Proposed | MDL detection evasion |

## How to Create an ADR

1. Copy the template from `adr/TEMPLATE.md`
2. Assign next available ADR number in series:
   - 001-009: Hypervisor architecture
   - 010-019: Anti-detection
   - 020-029: MCP architecture
   - 030-039: BYOVD strategy
3. Fill in context, decision, consequences
4. Submit PR for review
5. Update this index when accepted

## Related Documents

- [00-OVERVIEW.md](./00-OVERVIEW.md) - Architecture overview
- [03-C4-COMPONENTS.md](./03-C4-COMPONENTS.md) - Component details
