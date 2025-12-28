# OMBRA MCP Quick Reference

Fast lookup for common MCP server operations.

## Server Entry Points

```bash
# ombra - Hypervisor Development Tools
/Users/jonathanmcclintock/PROJECT-OMBRA/ombra_mcp_server/.venv/bin/python -m ombra_mcp.server

# project-mcp - Project Management Tools
/Users/jonathanmcclintock/PROJECT-OMBRA/project-mcp-server/.venv/bin/python -m project_mcp.server

# driver-re - Driver Reverse Engineering Tools
/Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp/.venv/bin/python -m driver_re_mcp.server
```

## Most Common Commands

### ombra - Intel/AMD Hypervisor Reference

| Task | Tool | Example Input |
|------|------|---------------|
| Look up VMCS field | `vmcs_field` | `{"field_name": "GUEST_RIP"}` |
| Get exit reason | `exit_reason` | `{"reason": "CPUID"}` |
| Find MSR details | `msr_info` | `{"msr_name": "IA32_TSC_DEADLINE"}` |
| Search Intel SDM | `search_sdm` | `{"query": "EPT violations"}` |
| Generate VMCS header | `generate_vmcs_header` | `{"fields": ["GUEST_RIP", "GUEST_RSP"]}` |
| Get anti-cheat info | `get_anticheat_intel` | `{"anticheat": "EasyAntiCheat"}` |
| Check timing | `simulate_handler_timing` | `{"handler": "cpuid", "duration_ns": 450}` |
| Scan binary | `scan_binary_signatures` | `{"binary_path": "/path/to/file"}` |

### project-mcp - Codebase Management

| Task | Tool | Example Input |
|------|------|---------------|
| Index file | `index_file` | `{"path": "src/vmx.c", "component": "VMX"}` |
| Search files | `search_files` | `{"query": "ept violation"}` |
| Index function | `index_function` | `{"name": "vmx_vmlaunch", "file_path": "src/vmx.c"}` |
| Find function | `find_function_by_name` | `{"name": "handle_cpuid_exit"}` |
| Register feature | `register_feature` | `{"name": "EPT Handler", "component": "EPT"}` |
| Mark stub | `mark_as_stub` | `{"type": "function", "identifier": "handle_io"}` |
| List stubs | `list_stubs` | `{"priority": "high"}` |
| Create task | `create_task` | `{"title": "Implement CPUID", "priority": "high"}` |
| Create epic | `create_epic` | `{"name": "Exit Handlers", "description": "..."}` |
| Dashboard | `get_dashboard_summary` | `{}` |
| Analyze impact | `analyze_change_impact` | `{"file_path": "src/vmcs.c"}` |
| Get recommendations | `get_recommendations` | `{}` |

### driver-re - Driver Analysis

| Task | Tool | Example Input |
|------|------|---------------|
| Add driver | `dre_add_driver` | `{"name": "vuln.sys", "path": "/samples/vuln.sys"}` |
| List drivers | `dre_list_drivers` | `{"status": "analyzed"}` |
| Get IOCTLs | `dre_list_ioctls` | `{"driver_id": 1}` |
| Find vulnerabilities | `dre_get_vulnerable_ioctls` | `{"driver_id": 1}` |
| Get imports | `dre_get_imports` | `{"driver_id": 1}` |
| Find dangerous APIs | `dre_find_dangerous_apis` | `{"driver_id": 1}` |
| Check blocklist | `check_driver_blocklist` | `{"driver_name": "vuln.sys"}` |
| Search drivers | `search_drivers` | `{"query": "memory write"}` |
| Find primitive | `find_drivers_with_primitive` | `{"primitive": "arbitrary_write"}` |
| MS Learn API | `mslearn_api` | `{"api_name": "MmMapIoSpace"}` |
| Semantic search | `semantic_search` | `{"query": "allocate MDL in kernel"}` |

## Configuration Paths

```
Main Config:     /Users/jonathanmcclintock/PROJECT-OMBRA/.mcp.json

ombra Database:  /Users/jonathanmcclintock/PROJECT-OMBRA/ombra_mcp_server/src/ombra_mcp/data/intel_sdm.db

project Database: /Users/jonathanmcclintock/PROJECT-OMBRA/project-mcp-server/src/project_mcp/data/project_mgmt.db

driver-re DB:    /Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp/src/driver_re_mcp/data/driver_re.db
driver-re Cache: /Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp/src/driver_re_mcp/data/mslearn_cache
driver-re Chroma: /Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp/src/driver_re_mcp/data/chroma
```

## Debugging Commands

### Test Server Connectivity

```bash
# Test ombra
echo '{"jsonrpc":"2.0","method":"tools/list","id":1}' | \
  /Users/jonathanmcclintock/PROJECT-OMBRA/ombra_mcp_server/.venv/bin/python -m ombra_mcp.server

# Test project-mcp
echo '{"jsonrpc":"2.0","method":"tools/list","id":1}' | \
  /Users/jonathanmcclintock/PROJECT-OMBRA/project-mcp-server/.venv/bin/python -m project_mcp.server

# Test driver-re
echo '{"jsonrpc":"2.0","method":"tools/list","id":1}' | \
  /Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp/.venv/bin/python -m driver_re_mcp.server
```

### Check Database Health

```bash
# ombra - Should return 167
sqlite3 /Users/jonathanmcclintock/PROJECT-OMBRA/ombra_mcp_server/src/ombra_mcp/data/intel_sdm.db \
  "SELECT COUNT(*) FROM vmcs_fields;"

# project-mcp - Should list tables
sqlite3 /Users/jonathanmcclintock/PROJECT-OMBRA/project-mcp-server/src/project_mcp/data/project_mgmt.db \
  "SELECT name FROM sqlite_master WHERE type='table';"

# driver-re - Should list tables
sqlite3 /Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp/src/driver_re_mcp/data/driver_re.db \
  "SELECT name FROM sqlite_master WHERE type='table';"
```

### Rebuild Databases

```bash
# ombra - Rebuild Intel SDM database
cd /Users/jonathanmcclintock/PROJECT-OMBRA/ombra_mcp_server
source .venv/bin/activate
python scripts/prepare_intel_sdm.py

# project-mcp - Auto-rebuilds on next run
rm /Users/jonathanmcclintock/PROJECT-OMBRA/project-mcp-server/src/project_mcp/data/project_mgmt.db

# driver-re - Auto-rebuilds on next run
rm /Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp/src/driver_re_mcp/data/driver_re.db
```

### Kill Hung Servers

```bash
# Kill all MCP servers
pkill -f "ombra_mcp\|project_mcp\|driver_re_mcp"

# Kill specific server
pkill -f "ombra_mcp.server"
pkill -f "project_mcp.server"
pkill -f "driver_re_mcp.server"
```

### View Logs

```bash
# MCP server logs (if enabled)
tail -f ~/.mcp/logs/ombra.log
tail -f ~/.mcp/logs/project-mcp.log
tail -f ~/.mcp/logs/driver-re.log
```

## Common Workflows

### 1. Start New Feature Implementation

```
1. project-mcp/register_feature
   {"name": "CPUID Exit Handler", "component": "Exit Handlers", "priority": "high"}

2. ombra/exit_reason
   {"reason": "CPUID"}

3. ombra/generate_exit_handler_skeleton
   {"exit_reason": 10}

4. project-mcp/index_file
   {"path": "src/exit_handlers/cpuid.c", "component": "Exit Handlers"}

5. project-mcp/create_task
   {"title": "Implement CPUID handler", "epic": "Exit Handlers"}
```

### 2. Analyze Driver for Primitives

```
1. driver-re/dre_add_driver
   {"name": "driver.sys", "path": "/samples/driver.sys"}

2. driver-re/dre_get_imports
   {"driver_id": 1}

3. driver-re/dre_find_dangerous_apis
   {"driver_id": 1}

4. driver-re/dre_list_ioctls
   {"driver_id": 1}

5. driver-re/check_driver_blocklist
   {"driver_name": "driver.sys"}
```

### 3. Check Project Health

```
1. project-mcp/get_dashboard_summary
   {}

2. project-mcp/list_stubs
   {"priority": "high"}

3. project-mcp/get_recommendations
   {}

4. project-mcp/analyze_codebase
   {}
```

## Environment Variables

| Variable | Server | Purpose |
|----------|--------|---------|
| `PYTHONPATH` | All | Module import path (points to `src/`) |
| `OMBRA_PROJECT_ROOT` | ombra | Project root directory |
| `INTEL_SDM_DB` | ombra | Intel SDM database path |
| `PROJECT_ROOT` | project-mcp | Project root directory |
| `PROJECT_DB` | project-mcp | Project database path |
| `DRIVER_SCANNER_ROOT` | project-mcp | Scanner integration path |
| `DRIVER_RE_DB` | driver-re | Driver analysis database |
| `MSLEARN_CACHE` | driver-re | Microsoft Learn cache dir |
| `CHROMA_DB_DIR` | driver-re | Vector database directory |

## Quick Setup

```bash
# Clone/navigate to project
cd /Users/jonathanmcclintock/PROJECT-OMBRA

# Install all servers
for server in ombra_mcp_server project-mcp-server driver-re-mcp; do
  cd $server
  python3 -m venv .venv
  source .venv/bin/activate
  pip install -e .
  cd ..
done

# Build Intel SDM database
cd ombra_mcp_server
source .venv/bin/activate
python scripts/prepare_intel_sdm.py
cd ..

# Copy config to Claude
cp .mcp.json ~/Library/Application\ Support/Claude/
# Or for CLI: cp .mcp.json ~/.config/claude-code/

# Restart Claude Desktop or CLI
```

## Performance Tips

- **Search operations:** Use full-text search (FTS5) - very fast (< 50ms)
- **Large result sets:** Request filtered results instead of `list_all`
- **Repeated queries:** Results cached where appropriate
- **Database locks:** Close connections promptly, avoid long-running queries
- **ChromaDB:** Rebuild index periodically for optimal search performance

## Support

For detailed documentation see:
- `/Users/jonathanmcclintock/PROJECT-OMBRA/MCP-INTEGRATION-GUIDE.md`
- Individual server READMEs in each server directory
