# Driver RE MCP Server - Developer Context

This CLAUDE.md provides Driver RE MCP-specific context. See root `/CLAUDE.MD` for project-wide conventions and ENI identity.

## Quick Stats

| Metric | Value |
|--------|-------|
| **Server Version** | v1.0.0 |
| **Total Tools** | 59 |
| **Framework** | Python MCP SDK (`mcp>=1.0.0`) |
| **Python** | 3.10+ |
| **Entry Point** | `src/driver_re_mcp/server.py` (862 LOC) |
| **Tool Code** | 5,820 LOC across 11 modules |
| **Pattern** | Decorator-based registration (`@tool()`) |

---

## Directory Structure

```
driver-re-mcp/
├── src/driver_re_mcp/
│   ├── server.py              # MCP server (862 LOC)
│   ├── config.py              # Configuration
│   ├── models.py              # Pydantic models
│   ├── __init__.py
│   ├── tools/                 # Tool modules (5,820 LOC)
│   │   ├── __init__.py        # Exports all 11 modules
│   │   ├── driver_tools.py    # Driver lifecycle (3.6KB)
│   │   ├── ioctl_tools.py     # IOCTL analysis (6KB)
│   │   ├── import_tools.py    # Import/API analysis (16KB)
│   │   ├── export_tools.py    # Export analysis (14KB)
│   │   ├── function_tools.py  # Call graph analysis (12KB)
│   │   ├── struct_tools.py    # Structure management (13KB)
│   │   ├── vuln_tools.py      # Vulnerability tracking (13KB)
│   │   ├── xref_tools.py      # Cross-references (24KB)
│   │   ├── search_tools.py    # Search (35KB - largest)
│   │   ├── ghidra_tools.py    # Ghidra integration (18KB)
│   │   └── analysis_tools.py  # Session/reporting (31KB)
│   ├── database/              # Database layer
│   │   ├── schema.sql         # Full schema (19KB, 16 tables)
│   │   ├── schema.py          # Schema creation (11KB)
│   │   ├── connection.py      # Connection management (6.5KB)
│   │   ├── models.py          # ORM models (11KB)
│   │   └── README.md          # Schema documentation
│   ├── embeddings/            # Vector search
│   ├── ghidra/                # Ghidra bridge
│   ├── parsers/               # PE/binary parsers
│   └── utils/                 # Utilities
├── data/
│   └── driver_re.db           # SQLite database (168KB)
└── pyproject.toml
```

---

## Tool Categories (59 Total)

| Module | Tools | Purpose |
|--------|-------|---------|
| **driver_tools** | 5 | Driver lifecycle: add, get, list, update, delete |
| **ioctl_tools** | 5 | IOCTL analysis, CTL_CODE decoding, vulnerability |
| **import_tools** | 4 | IAT analysis, dangerous API detection (54+ APIs) |
| **export_tools** | 3 | EAT analysis, VirtualBox pattern detection |
| **function_tools** | 6 | Call graphs, dispatch handlers, tracing |
| **struct_tools** | 4 | Structure management, member tracking |
| **vuln_tools** | 5 | Vulnerability tracking, attack chains |
| **xref_tools** | 4 | Cross-reference analysis |
| **search_tools** | 6 | Full-text (FTS5) + semantic (ChromaDB) search |
| **ghidra_tools** | 8 | Ghidra bidirectional sync |
| **analysis_tools** | 9 | Sessions, notes, reporting |

---

## Tool Registration Pattern

This server uses **decorator-based registration** (different from ombra-mcp):

```python
# In tools/driver_tools.py
from ..server import tool  # Import decorator

@tool("add_driver", "Add a new driver for analysis", {
    "type": "object",
    "properties": {
        "name": {"type": "string"},
        "path": {"type": "string"},
        "sha256": {"type": "string"}
    },
    "required": ["name", "path"]
})
async def add_driver(**kwargs) -> dict:
    """Implementation..."""
    return {"driver_id": new_id}
```

The `@tool` decorator populates `TOOLS` dict at import time.
`server.py` converts to MCP `Tool` objects in `list_tools()`.

---

## Database Schema (16 Tables)

```sql
-- Core entities
drivers         -- PE metadata, hashes, version info, status
sections        -- PE sections with entropy analysis
imports         -- IAT entries with danger classification
exports         -- EAT entries with VBox pattern detection
functions       -- Analyzed functions with prototypes

-- Relationships
xrefs           -- Cross-references (caller → callee)
ioctls          -- IOCTL handlers with vulnerabilities
structures      -- Defined structures
structure_members -- Structure fields

-- Security analysis
vulnerabilities -- Discovered vulnerabilities
attack_chains   -- Multi-step exploitation paths

-- Session management
analysis_sessions -- Work sessions
analysis_notes    -- Per-session notes

-- Metadata
strings         -- Interesting strings
globals         -- Global variables
api_categories  -- Import categorization
```

**Indexes**: 65 total (including partial indexes for flags)
**Triggers**: 7 auto-updating timestamps

---

## Dangerous API Detection

`import_tools.py` flags 54+ kernel APIs with security context:

| Category | Examples | Risk Level |
|----------|----------|------------|
| **Physical Memory** | MmCopyMemory, MmMapIoSpace, MmMapLockedPages | CRITICAL |
| **Process/Thread** | PsLookupProcessByProcessId, KeStackAttachProcess | HIGH |
| **Registry** | ZwCreateKey, ZwSetValueKey, ZwDeleteKey | MEDIUM |
| **Object/Handle** | ObOpenObjectByPointer, ZwDuplicateObject | HIGH |
| **MSR/CR** | rdmsr, wrmsr, __readcr0, __writecr0 | CRITICAL |
| **Paging** | MmProbeAndLockPages, MmGetPhysicalAddress | HIGH |

**Risk Scoring**:
```python
risk_score = 0
risk_score += physical_memory * 30
risk_score += process_manipulation * 20
risk_score += persistence * 15
risk_score += anti_forensics * 10
```

---

## VirtualBox Pattern Detection

`export_tools.py` identifies BYOVD candidates via export prefixes:

| Prefix | Source | Risk |
|--------|--------|------|
| SUP, SUPR0, SUPR3 | VirtualBox/LDPlayer | CRITICAL |
| ASM, RT | VBox runtime | HIGH |
| Ke, Mm, Ob, Ps, Zw | Kernel-style | MEDIUM |

**Risk Calculation**:
```python
risk_score = min(dangerous_count * 15, 60)  # Cap at 60
risk_score += 30 if vbox_pattern else 0
risk_score += 10 if total_exports > 50 else 0
# Thresholds: 70+ critical, 50+ high, 25+ medium
```

---

## Key Tools Reference

### Driver Lifecycle
```bash
mcp-cli call driver-re/add_driver '{"name": "test.sys", "path": "/path/to/driver"}'
mcp-cli call driver-re/get_driver '{"driver_id": 1}'
mcp-cli call driver-re/list_drivers '{}'
mcp-cli call driver-re/update_driver_status '{"driver_id": 1, "status": "analyzed"}'
```

### IOCTL Analysis
```bash
mcp-cli call driver-re/add_ioctl '{"driver_id": 1, "code": 2236420, "name": "READ_PHYS"}'
# Decodes CTL_CODE: device_type=0x22, function=0x801, method=0, access=0
mcp-cli call driver-re/get_vulnerable_ioctls '{"driver_id": 1}'
```

### Import/Export Analysis
```bash
mcp-cli call driver-re/find_dangerous_apis '{"driver_id": 1}'
mcp-cli call driver-re/analyze_exports '{"driver_id": 1}'
```

### Function Tracing
```bash
mcp-cli call driver-re/trace_call_path '{"driver_id": 1, "from_func": "DispatchIoControl", "to_func": "MmCopyMemory", "max_depth": 10}'
```

### Ghidra Integration
```bash
mcp-cli call driver-re/ghidra_connect '{"host": "localhost", "port": 8080}'
mcp-cli call driver-re/ghidra_sync_functions '{"driver_id": 1}'
mcp-cli call driver-re/ghidra_get_decompilation '{"function_address": "0x140001000"}'
```

---

## Adding New Tools

### 1. Create in appropriate module

```python
# tools/new_module.py
from ..database.connection import get_connection

@tool("my_new_tool", "Description", {
    "type": "object",
    "properties": {
        "param1": {"type": "string"}
    },
    "required": ["param1"]
})
async def my_new_tool(param1: str) -> dict:
    conn = get_connection()
    # Implementation
    return {"result": data}
```

### 2. Export from __init__.py

```python
# tools/__init__.py
from . import new_module  # Add this line
```

### 3. Verify registration

```bash
python3 -c "from src.driver_re_mcp.server import TOOLS; print('my_new_tool' in TOOLS)"
```

---

## Database Connection

All tools share connection via `database/connection.py`:

```python
from ..database.connection import get_connection

conn = get_connection()  # Returns sqlite3.Row-enabled connection
cursor = conn.execute("SELECT * FROM drivers WHERE id = ?", (driver_id,))
```

---

## Ghidra Integration

The server can sync bidirectionally with Ghidra via HTTP bridge:

```
Ghidra ←→ GhidraMCP (port 8080) ←→ driver-re-mcp
```

**Sync Operations**:
- Import functions from Ghidra
- Push renames back to Ghidra
- Get decompilation on demand
- Sync cross-references

---

## DANGER ZONES

### Critical Files

| File | Risk | Notes |
|------|------|-------|
| `database/schema.sql` | **CRITICAL** | 16 tables, 65 indexes - migration breaks everything |
| `tools/__init__.py` | **HIGH** | Must export all tool modules |
| `server.py` | **HIGH** | Tool registration - typos break discovery |
| `import_tools.py` | **MEDIUM** | DANGEROUS_APIS dict - security classification |

### Common Mistakes

1. **Schema changes**: Always create migration, never modify schema.sql directly
2. **Tool not appearing**: Check decorator syntax AND __init__.py export
3. **Database locks**: Don't hold connections across async boundaries
4. **Ghidra timeout**: Default 30s may not be enough for large functions

---

## Testing

```bash
# Count registered tools
python3 -c "from src.driver_re_mcp.server import TOOLS; print(f'{len(TOOLS)} tools')"

# Verify database schema
sqlite3 data/driver_re.db ".tables"
sqlite3 data/driver_re.db "SELECT COUNT(*) || ' indexes' FROM sqlite_master WHERE type='index'"

# Test specific tool schema
mcp-cli info driver-re/add_ioctl
```

---

## Workflow: Analyzing a New Driver

1. **Add driver**
```bash
mcp-cli call driver-re/add_driver '{"name": "target.sys", "path": "/path/to/target.sys"}'
```

2. **Parse PE (imports/exports)**
```bash
mcp-cli call driver-re/find_dangerous_apis '{"driver_id": 1}'
mcp-cli call driver-re/analyze_exports '{"driver_id": 1}'
```

3. **Sync with Ghidra**
```bash
mcp-cli call driver-re/ghidra_connect '{}'
mcp-cli call driver-re/ghidra_sync_functions '{"driver_id": 1}'
```

4. **Analyze IOCTLs**
```bash
mcp-cli call driver-re/add_ioctl '{"driver_id": 1, "code": 2236420, "name": "VULN_READ"}'
mcp-cli call driver-re/update_ioctl_vulnerability '{"ioctl_id": 1, "is_vulnerable": true, "type": "arbitrary_read"}'
```

5. **Trace exploitation path**
```bash
mcp-cli call driver-re/trace_call_path '{"driver_id": 1, "from_func": "IRP_MJ_DEVICE_CONTROL", "to_func": "MmCopyMemory"}'
```

6. **Document vulnerability**
```bash
mcp-cli call driver-re/add_vulnerability '{"driver_id": 1, "type": "arbitrary_physical_read", "severity": "critical"}'
mcp-cli call driver-re/create_attack_chain '{"driver_id": 1, "name": "BYOVD Chain", "steps": [...]}'
```

---

## Known Issues

1. **Large search_tools.py**: 35KB - consider splitting semantic vs full-text
2. **No ChromaDB pooling**: Each search opens new connection
3. **Ghidra sync blocking**: Large drivers can timeout
4. **No versioning**: Schema changes require manual migration
