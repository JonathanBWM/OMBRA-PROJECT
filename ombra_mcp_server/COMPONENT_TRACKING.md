# Component Tracking Database - Setup Guide

## Overview

The Project Brain component tracking system maintains a live database of all hypervisor components, their implementation status, dependencies, and exit handler coverage.

## Database Location

`ombra_mcp_server/src/ombra_mcp/data/project_brain.db`

## Seeding the Database

The component tracking database has been seeded with accurate data based on analysis of the actual hypervisor codebase.

### Initial Seed

```bash
cd ombra_mcp_server
python3 scripts/seed_components.py
```

### Reseed After Changes

Run the seed script again after:
- Major refactoring that changes component structure
- Adding new components or handlers
- Renaming or reorganizing files

### Via MCP Tool (after server restart)

Once the MCP server recognizes the new tool:

```bash
mcp-cli call ombra/seed_components '{}'
```

## Current Status

**Last seeded:** 2025-12-26

### Components Summary

| Category | Count | Status |
|----------|-------|--------|
| Core VMX | 3 | Implemented |
| Memory | 2 | 1 implemented, 1 partial |
| Handlers | 8 | All implemented |
| Stealth | 2 | 1 implemented, 1 stub |
| Tools | 6 | All implemented |
| **Total** | **22** | **19 implemented, 2 partial, 1 stub** |

### Exit Handlers Summary

| Status | Count | Notes |
|--------|-------|-------|
| Implemented | 20 | Full handlers with logic |
| Stub | 4 | Pass-through (HLT, INVLPG, XSETBV, etc) |
| Missing | 41 | Fall to default case |
| **Total** | **65** | Covers all Intel SDM exit reasons |

### Stealth Coverage

- **15 handlers** have stealth features
  - CPUID: Hides VMX and hypervisor presence
  - RDTSC/RDTSCP: Timing compensation
  - RDMSR/WRMSR: MSR hiding
  - VMX instructions: Inject #UD to prevent nested VMX

- **2 handlers** have timing compensation
  - RDTSC
  - RDTSCP

## Component Details

### Core Components

1. **vmx_core** - VMX Operations (implemented)
   - Files: `vmx.c`, `vmx.h`, `entry.c`
   - No dependencies

2. **vmcs_setup** - VMCS Configuration (implemented)
   - Files: `vmcs.c`, `vmcs.h`, `vmcs_fields.h`
   - Depends on: vmx_core

3. **exit_dispatch** - VM-Exit Dispatcher (implemented)
   - Files: `exit_dispatch.c`, `exit_dispatch.h`, `exit_reasons.h`
   - Depends on: vmcs_setup

### Memory Management

4. **ept** - Extended Page Tables (partial)
   - Files: `ept.c`, `ept.h`, `ept_defs.h`
   - Missing: 2MB splitting not fully tested, 4KB production mapping, INVEPT optimization
   - Depends on: vmx_core

5. **hooks** - EPT Hook Framework (partial)
   - Files: `hooks.c`, `hooks.h`
   - Missing: Execute-only EPT (needs MTF), shadow page pool, single-step restoration
   - Depends on: ept

### Exit Handlers

6. **handler_cpuid** - CPUID Handler (implemented, stealth)
7. **handler_rdtsc** - RDTSC/RDTSCP Handlers (implemented, stealth, timing)
8. **handler_msr** - MSR Handlers (implemented, stealth)
9. **handler_cr_access** - CR Access Handler (implemented)
10. **handler_ept_violation** - EPT Violation Handler (implemented)
11. **handler_vmcall** - VMCALL Handler (implemented)
12. **handler_exception** - Exception/NMI Handler (implemented)
13. **handler_io** - I/O Instruction Handler (implemented)

### Stealth & Timing

14. **stealth** - Stealth Features (implemented)
    - Files: CPUID, MSR, RDTSC handlers
    - Missing: Advanced MSR hiding, CPUID timing normalization, CR4 shadow
    - Depends on: handler_cpuid, handler_msr, handler_rdtsc

15. **timing** - Timing Compensation (stub)
    - Files: `timing.c`, `timing.h`
    - Missing: Calibration (uses fixed overhead), per-handler profiles, adaptive compensation

### Usermode Loader

16. **usermode_loader** - Ring-3 Loader (implemented)
    - Files: `main.c`, `driver_interface.*`, `payload_loader.*`, `debug_reader.*`
    - Depends on: byovd_loader

### BYOVD Exploits

17. **byovd_loader** - BYOVD Driver Loader (implemented)
18. **byovd_supdrv** - Ld9BoxSup.sys Exploit (implemented)
19. **byovd_throttlestop** - ThrottleStop Exploit (implemented)
20. **byovd_crypto** - Driver Crypto/Deobfuscation (implemented)
21. **byovd_nt_helpers** - NT Undocumented Helpers (implemented)

### Debug/Diagnostics

22. **debug** - Debug Logging (implemented)

## Using Component Tracking

### Query Project Status

```bash
mcp-cli call ombra/get_project_status '{}'
```

Returns health score, findings count, component summary, and exit handler summary.

### Get Component Details

```bash
mcp-cli call ombra/get_component '{"component_id": "stealth"}'
```

Returns files, status, missing features, and dependencies.

### Query Exit Handlers

```bash
# All implemented handlers
mcp-cli call ombra/get_exit_handler_status '{"status": "implemented"}'

# Missing handlers
mcp-cli call ombra/get_exit_handler_status '{"status": "missing"}'

# All handlers
mcp-cli call ombra/get_exit_handler_status '{}'
```

## Maintenance

The seed script (`scripts/seed_components.py`) analyzes the actual codebase to determine:

1. **Component status:**
   - `implemented`: Fully working code
   - `partial`: Working but missing features
   - `stub`: Placeholder or minimal implementation
   - `planned`: Not yet implemented

2. **Exit handler status:**
   - `implemented`: Has dedicated handler function
   - `stub`: Pass-through in dispatcher (advance RIP only)
   - `missing`: Falls to default case

3. **Stealth flags:**
   - `has_stealth`: Handler includes anti-detection measures
   - `has_timing`: Handler includes timing compensation

The script reads actual source files to make these determinations accurately.

## Schema

### Components Table

```sql
CREATE TABLE components (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    category TEXT,  -- core, handlers, memory, stealth, tools
    status TEXT,    -- implemented, partial, stub, planned
    files TEXT,     -- JSON array
    missing TEXT,   -- JSON array of missing features
    depends_on TEXT, -- JSON array of component IDs
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

### Exit Handlers Table

```sql
CREATE TABLE exit_handlers (
    reason INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    status TEXT,    -- implemented, partial, stub, missing
    has_stealth BOOLEAN DEFAULT FALSE,
    has_timing BOOLEAN DEFAULT FALSE,
    file TEXT,
    line INTEGER,
    notes TEXT,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

## Next Steps

With the component database seeded, the Project Brain can now:

1. Track implementation progress
2. Identify missing handlers that should be implemented
3. Monitor stealth coverage
4. Generate suggestions for what to work on next
5. Detect when components have unresolved dependencies

The watcherd daemon will eventually scan code changes and update component status automatically.
