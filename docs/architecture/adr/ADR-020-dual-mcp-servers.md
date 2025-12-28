# ADR-020: Dual MCP Server Architecture

## Status

Accepted

## Date

2024-12-15

## Context

PROJECT-OMBRA development requires two distinct capability domains:

1. **Hypervisor Development**: Intel SDM reference, code generation, VMCS validation, stealth auditing
2. **Driver Reverse Engineering**: PE analysis, IOCTL discovery, vulnerability tracking, Ghidra integration

Design considerations:
- Single large server vs. multiple focused servers
- Tool organization and discoverability
- Database isolation vs. sharing
- Independent deployment and testing

Options considered:
- **Monolithic server**: All 211 tools in one server - harder to maintain
- **Microservices**: Many small servers - operational complexity
- **Dual server**: Two focused servers - balanced approach

## Decision

Implement **two specialized MCP servers**:

1. **ombra-mcp** (152 tools)
   - Focus: Hypervisor development intelligence
   - Databases: intel_sdm.db, project_brain.db, vergilius.db, anticheat_intel.db, evasion_techniques.db, byovd_drivers.db
   - Registration: Explicit TOOLS/TOOL_HANDLERS pattern

2. **driver-re-mcp** (59 tools)
   - Focus: Driver reverse engineering
   - Database: driver_re.db (16 tables)
   - Registration: @tool() decorator pattern

Shared resources:
- driver_re.db accessed by both servers
- ChromaDB vector store shared
- Common Python dependencies

## Consequences

### Positive

- **Focused development**: Each server has clear purpose
- **Independent testing**: Can test driver RE without hypervisor tools
- **Parallel development**: Two developers can work on different servers
- **Modular deployment**: Can update one server without affecting other
- **Clear tool organization**: 152 vs 59 tools, easy to find

### Negative

- **Two processes**: Must run both for full capability
- **Shared database coordination**: driver_re.db needs careful sync
- **Duplicate patterns**: Some code patterns duplicated

### Neutral

- **Different registration patterns**: Both patterns documented, team learns both

## Implementation Notes

### Claude Configuration (.mcp.json)

```json
{
  "mcpServers": {
    "ombra": {
      "command": "python",
      "args": ["-m", "ombra_mcp.server"],
      "cwd": "/path/to/ombra_mcp_server"
    },
    "driver-re": {
      "command": "python",
      "args": ["-m", "driver_re_mcp.server"],
      "cwd": "/path/to/driver-re-mcp"
    }
  }
}
```

### Tool Namespacing

```bash
# ombra-mcp tools
mcp-cli call ombra/vmcs_field_complete '{"field_name": "GUEST_RIP"}'
mcp-cli call ombra/generate_exit_handler '{"reason": 10}'

# driver-re-mcp tools
mcp-cli call driver-re/add_driver '{"original_name": "test.sys", ...}'
mcp-cli call driver-re/find_dangerous_apis '{"driver_id": "..."}'
```

### Shared Database Access

Both servers import driver_re_db.py for shared connection:

```python
# ombra_mcp_server/src/ombra_mcp/tools/driver_re_db.py
def get_driver_re_conn():
    return sqlite3.connect(DRIVER_RE_DB_PATH)

# Used by ombra-mcp's dre_* tools
from .driver_re_db import get_driver_re_conn

# driver-re-mcp has its own connection.py with same pattern
```

## Tool Distribution

| Category | ombra-mcp | driver-re-mcp |
|----------|-----------|---------------|
| Intel SDM | 14 | 0 |
| Code Generation | 6 | 0 |
| Validation | 2 | 0 |
| Stealth | 4 | 0 |
| Anti-Cheat Intel | 13 | 0 |
| BYOVD | 16 | 0 |
| Project Brain | 28 | 0 |
| Vergilius | 8 | 0 |
| MS Learn | 10 | 0 |
| Semantic Search | 3 | 4 |
| Driver Lifecycle | 4 | 5 |
| IOCTL Analysis | 4 | 5 |
| Import/Export | 8 | 7 |
| Functions | 0 | 6 |
| Vulnerability | 0 | 5 |
| Ghidra | 0 | 8 |
| Analysis | 0 | 8 |
| **Total** | **152** | **59** |

## Related

- [ADR-021](./ADR-021-sqlite-databases.md) - Database architecture
- [ADR-023](./ADR-023-explicit-vs-decorator.md) - Registration patterns
- `ombra_mcp_server/src/ombra_mcp/server.py` - ombra-mcp entry
- `driver-re-mcp/src/driver_re_mcp/server.py` - driver-re-mcp entry
