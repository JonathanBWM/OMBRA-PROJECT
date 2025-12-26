# OmbraMCP Project Brain — Design Document

**Date:** 2025-12-25
**Status:** Approved
**Author:** ENI + LO

---

## Overview

Transform the OmbraMCP server from a passive tool collection into a living, breathing system that actively monitors the PROJECT-OMBRA codebase, identifies issues, tracks progress, and remembers context across sessions.

## Goals

1. **Real-time awareness** — Daemon watches filesystem, catches issues as files change
2. **Hypervisor-specific intelligence** — Understands VMCS, EPT, exit handlers, stealth requirements
3. **Component tracking** — Knows what's implemented, what's partial, what's blocked
4. **Knowledge persistence** — Remembers design decisions, gotchas, session context
5. **Proactive surfacing** — Critical issues interrupt; others queue for session start

## Non-Goals

- Test coverage tracking (not applicable to kernel code)
- Generic linting (compiler handles that)
- Duplicating claude-mem functionality

---

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    PROJECT-OMBRA Directory                  │
│  /hypervisor  /ombra_mcp_server  /byovd  /docs  /tools     │
└─────────────────────────────────────────────────────────────┘
                              │
                              │ filesystem events
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                     ombra-watcherd                          │
│                   (launchd service)                         │
│                                                             │
│  ┌─────────────┐ ┌──────────────┐ ┌───────────────┐        │
│  │ hypervisor  │ │ consistency  │ │   security    │        │
│  │  analyzer   │ │   analyzer   │ │   analyzer    │        │
│  └─────────────┘ └──────────────┘ └───────────────┘        │
│                                                             │
│  ┌─────────────────────────────────────────────────┐       │
│  │            component_tracker                     │       │
│  └─────────────────────────────────────────────────┘       │
└─────────────────────────────────────────────────────────────┘
                              │
                              │ writes
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    project_brain.db                         │
│                                                             │
│  components | findings | suggestions | decisions | gotchas  │
│  exit_handlers | vmcs_usage | sessions                      │
└─────────────────────────────────────────────────────────────┘
                              │
                              │ queries
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    OmbraMCP Server                          │
│                                                             │
│  get_project_status | get_findings | get_suggestions        │
│  add_decision | add_gotcha | get_session_context            │
└─────────────────────────────────────────────────────────────┘
                              │
                              │ session hook
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                      Claude Code                            │
│                                                             │
│  Session starts → critical findings injected                │
│  Session ends → context snapshot saved                      │
└─────────────────────────────────────────────────────────────┘
```

---

## Components

### 1. ombra-watcherd (Daemon)

**Location:** `ombra_mcp_server/src/ombra_watcherd/`

**Technology:**
- Python 3.10+
- `watchdog` library for filesystem monitoring
- SQLite for storage
- `launchd` for macOS service management

**Lifecycle:**
```bash
ombra-watcherd install   # Install launchd plist, start service
ombra-watcherd start     # Start if stopped
ombra-watcherd stop      # Stop daemon
ombra-watcherd status    # Show running state, last scan time
ombra-watcherd uninstall # Remove launchd plist
```

**Behavior:**
- Watches `/Users/jonathanmcclintock/PROJECT-OMBRA` recursively
- Debounces rapid changes (500ms window)
- Ignores: `.git/`, `.venv/`, `__pycache__/`, `*.pyc`, `build/`
- On file change: queue for analysis
- Runs analyzers on queued files
- Writes findings to database
- Logs to `~/Library/Logs/ombra-watcherd.log`

**launchd plist:** `~/Library/LaunchAgents/com.ombra.watcherd.plist`
- RunAtLoad: true
- KeepAlive: true
- Auto-restart on crash

---

### 2. Analyzers

#### hypervisor_analyzer (Priority 1)

The core value. Understands hypervisor semantics.

**Checks:**

| Check | Description | Severity |
|-------|-------------|----------|
| `unhandled_exit` | Exit reason in switch but body is stub/log-only | warning |
| `missing_exit` | Exit reason not in switch at all | info |
| `vmcs_write_no_read` | Field written but never read (dead write) | info |
| `vmcs_read_no_write` | Field read but never initialized | critical |
| `control_bit_mismatch` | Feature used but control bit not set | critical |
| `no_timing_compensation` | Exit handler lacks TSC adjustment | warning |
| `msr_not_in_bitmap` | MSR handler exists but bitmap doesn't intercept | critical |
| `ept_permission_gap` | Hook page still has execute permission | critical |
| `missing_invept` | EPT modified but INVEPT not called | warning |
| `stealth_gap` | CPUID/MSR handler doesn't hide hypervisor | warning |

**Implementation:**
- Parse C files with tree-sitter or regex patterns
- Build VMCS field usage map from vmread/vmwrite calls
- Build exit handler map from switch cases
- Cross-reference against Intel SDM database

#### consistency_analyzer (Priority 2)

**Checks:**

| Check | Description | Severity |
|-------|-------------|----------|
| `signature_mismatch` | Function declaration differs from definition | critical |
| `unused_function` | Function defined but never called | info |
| `unused_include` | Header included but nothing used from it | info |
| `forward_decl_missing` | Function called before declaration | warning |

#### security_analyzer (Priority 3)

**Checks:**

| Check | Description | Severity |
|-------|-------------|----------|
| `signature_exposure` | Known hypervisor strings in code | critical |
| `pool_tag_exposure` | Known pool tags (HvPl, VMXr, etc.) | critical |
| `hardcoded_address` | Magic addresses that could be signatures | warning |
| `debug_left_in` | DbgPrint/KdPrint in release-intended code | warning |

---

### 3. Component Tracker

Maintains semantic understanding of project structure.

**Component Registry:**
```yaml
vmx_core:
  files: [hypervisor/vmx.c, hypervisor/vmx.h]
  status: implemented  # implemented | partial | stub | planned
  depends_on: []

vmcs_setup:
  files: [hypervisor/vmcs.c, hypervisor/vmcs.h]
  status: partial
  missing:
    - secondary_proc_based_controls
    - vm_function_controls
  depends_on: [vmx_core]

exit_dispatch:
  files: [hypervisor/exit_dispatch.c]
  status: partial
  handlers:
    cpuid: {status: implemented, stealth: true, line: 45}
    rdtsc: {status: implemented, stealth: true, line: 89}
    ept_violation: {status: partial, line: 156}
    # ... all 77 tracked
  depends_on: [vmcs_setup]

ept:
  files: [hypervisor/ept.c, hypervisor/ept.h]
  status: partial
  missing:
    - split_2mb_to_4kb
    - invept_single_context
  depends_on: [vmx_core]

hooks:
  files: [hypervisor/hooks.c, hypervisor/hooks.h]
  status: partial
  depends_on: [ept]

stealth:
  files: [hypervisor/handlers/*.c]
  status: implemented
  coverage:
    cpuid_spoofing: true
    msr_hiding: true
    timing_compensation: true
    cr4_shadow: true
```

**Auto-Detection:**
- Scans for function definitions → maps to components
- Checks function bodies for stub patterns (`// TODO`, empty, just returns)
- Tracks `#include` relationships for dependency inference

---

### 4. Database Schema

**Location:** `ombra_mcp_server/src/ombra_mcp/data/project_brain.db`

```sql
-- Component tracking
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

CREATE TABLE vmcs_field_usage (
    encoding INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    is_read BOOLEAN DEFAULT FALSE,
    is_written BOOLEAN DEFAULT FALSE,
    read_locations TEXT,  -- JSON array of file:line
    write_locations TEXT, -- JSON array of file:line
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Findings
CREATE TABLE findings (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    file TEXT NOT NULL,
    line INTEGER,
    severity TEXT NOT NULL,  -- critical, warning, info
    type TEXT NOT NULL,      -- analyzer type
    check_id TEXT NOT NULL,  -- specific check that fired
    message TEXT NOT NULL,
    suggested_fix TEXT,
    dismissed BOOLEAN DEFAULT FALSE,
    dismissed_reason TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE suggestions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    priority TEXT NOT NULL,  -- high, medium, low
    type TEXT NOT NULL,      -- implementation, optimization, security
    component TEXT,
    message TEXT NOT NULL,
    action TEXT,             -- suggested MCP tool call or code change
    acted_on BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Knowledge persistence
CREATE TABLE decisions (
    id TEXT PRIMARY KEY,     -- D001, D002, etc.
    date DATE NOT NULL,
    topic TEXT NOT NULL,
    choice TEXT NOT NULL,
    rationale TEXT,
    alternatives TEXT,       -- JSON array of rejected options
    affects TEXT,            -- JSON array of files/components
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE gotchas (
    id TEXT PRIMARY KEY,     -- G001, G002, etc.
    symptom TEXT NOT NULL,
    cause TEXT NOT NULL,
    fix TEXT NOT NULL,
    files TEXT,              -- JSON array of file:line
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE sessions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    started_at TIMESTAMP NOT NULL,
    ended_at TIMESTAMP,
    working_on TEXT,         -- what we were doing
    context_snapshot TEXT,   -- full context for resume
    files_touched TEXT,      -- JSON array
    decisions_made TEXT,     -- JSON array of decision IDs
    findings_addressed TEXT  -- JSON array of finding IDs
);

-- Indexes
CREATE INDEX idx_findings_severity ON findings(severity) WHERE NOT dismissed;
CREATE INDEX idx_findings_file ON findings(file) WHERE NOT dismissed;
CREATE INDEX idx_suggestions_priority ON suggestions(priority) WHERE NOT acted_on;
CREATE INDEX idx_exit_handlers_status ON exit_handlers(status);
```

---

### 5. MCP Tools

**New tools for project brain:**

| Tool | Description | Parameters |
|------|-------------|------------|
| `get_project_status` | Dashboard: component status, health score, blockers | `verbose: bool` |
| `get_findings` | Query active findings | `severity: str, file: str, type: str, limit: int` |
| `get_suggestions` | What to work on next | `priority: str, limit: int` |
| `dismiss_finding` | Mark as false positive | `finding_id: int, reason: str` |
| `get_component` | Details on specific component | `component_id: str` |
| `get_exit_handler_status` | All exit handlers with status | `status_filter: str` |
| `get_decision` | Look up a design decision | `decision_id: str` |
| `list_decisions` | All decisions, optionally filtered | `topic: str, affects: str` |
| `add_decision` | Record new decision | `topic, choice, rationale, alternatives, affects` |
| `add_gotcha` | Record a solved bug | `symptom, cause, fix, files` |
| `get_gotchas` | Search gotchas | `keyword: str` |
| `get_session_context` | What were we doing last time? | none |
| `save_session_context` | Snapshot current work | `working_on: str, context: str` |
| `refresh_analysis` | Force full rescan | `path: str` (optional, default all) |

---

### 6. Session Hooks

**On Claude Code session start:**

1. Query `findings WHERE severity = 'critical' AND NOT dismissed`
2. If any exist, inject summary message
3. Query last session context, surface "Last time we were working on..."
4. Query top 3 high-priority suggestions

**On Claude Code session end (if detectable):**

1. Save session context snapshot
2. Record files touched
3. Link any decisions made during session

**Implementation:** Claude Code hook in `.claude/hooks/` or plugin integration.

---

## File Structure

```
ombra_mcp_server/
├── src/
│   ├── ombra_mcp/
│   │   ├── server.py           # Add new tools
│   │   ├── tools/
│   │   │   ├── project_brain.py  # New: project brain tools
│   │   │   └── ...
│   │   └── data/
│   │       ├── project_brain.db  # New: brain database
│   │       └── ...
│   │
│   └── ombra_watcherd/          # New: daemon package
│       ├── __init__.py
│       ├── __main__.py          # CLI entry point
│       ├── daemon.py            # Main watcher loop
│       ├── analyzers/
│       │   ├── __init__.py
│       │   ├── base.py          # Analyzer base class
│       │   ├── hypervisor.py    # Hypervisor-specific checks
│       │   ├── consistency.py   # Code consistency checks
│       │   └── security.py      # Security/stealth checks
│       ├── tracker.py           # Component tracker
│       ├── database.py          # DB operations
│       └── launchd.py           # launchd plist management
│
├── scripts/
│   └── ombra-watcherd           # CLI wrapper script
│
└── pyproject.toml               # Add watcherd entry point
```

---

## Implementation Phases

### Phase 1: Foundation
- [ ] Create `ombra_watcherd` package structure
- [ ] Implement daemon with watchdog
- [ ] Set up launchd integration
- [ ] Create database schema
- [ ] Basic MCP tools: `get_findings`, `refresh_analysis`

### Phase 2: Hypervisor Analyzer
- [ ] C file parsing (tree-sitter or regex)
- [ ] VMCS field usage tracking
- [ ] Exit handler detection
- [ ] Control bit cross-reference
- [ ] Implement all hypervisor checks

### Phase 3: Component Tracking
- [ ] Component registry schema
- [ ] Auto-detection from code
- [ ] Dependency graph
- [ ] Status inference (stub/partial/implemented)

### Phase 4: Knowledge Layer
- [ ] Decision recording and retrieval
- [ ] Gotcha database
- [ ] Session context snapshots

### Phase 5: Session Integration
- [ ] Claude Code hooks for session start/end
- [ ] Critical finding injection
- [ ] Context restoration

---

## Success Criteria

1. Daemon runs reliably in background, auto-starts on login
2. Critical hypervisor bugs (VMCS misconfig, missing control bits) caught within seconds of save
3. `get_project_status` gives accurate component completion picture
4. Session context persists — can resume after days/weeks without losing context
5. Zero false positives on critical findings (dismissals rare)

---

## Open Questions

1. **Tree-sitter vs regex for C parsing?** Tree-sitter is more accurate but heavier dependency. Regex is fragile but simple.
2. **How to detect session end?** Claude Code doesn't have an explicit exit hook. May need to infer from inactivity.
3. **Should daemon run analysis in thread pool?** For large codebases, blocking on each file change could lag.

---

## References

- Intel SDM Vol 3C, Chapters 23-29 (VMX specification)
- OmbraMCP existing tools (`server.py`, `tools/`)
- watchdog library: https://python-watchdog.readthedocs.io/
- launchd plist format: `man launchd.plist`
