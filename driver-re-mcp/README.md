# Driver RE MCP

MCP server for Windows kernel driver reverse engineering.

## Features

- 52 specialized tools for driver analysis
- **NEW:** Scanner integration for automated driver intake and prioritization
- SQLite database for structured data storage
- ChromaDB for semantic search across analysis notes, vulnerabilities, and IOCTLs
- Ghidra integration via GhidraMCP bridge
- Vulnerability tracking and attack chain construction
- Import/export analysis with dangerous API detection
- Function call graph construction and path tracing
- Priority-based analysis queuing from ombra-driver-scanner

## Installation

```bash
cd driver-re-mcp
pip install -e .
```

## Database Initialization

```bash
python scripts/init_db.py
python scripts/seed_categories.py
```

## Usage

### Via MCP Client

Add to your `.mcp.json`:

```json
{
  "mcpServers": {
    "driver-re": {
      "command": "driver-re-mcp"
    }
  }
}
```

### Standalone

```bash
driver-re-mcp
```

## Tool Categories

### Driver Management (5 tools)
- add_driver, get_driver, list_drivers, update_driver_status, delete_driver

### Scanner Integration (5 tools) **NEW**
- import_scanner_results, get_analysis_queue, import_single_driver_analysis
- set_driver_tier_info, get_drivers_by_capability

### IOCTL Analysis (5 tools)
- add_ioctl, get_ioctl, list_ioctls, get_vulnerable_ioctls, update_ioctl_vulnerability

### Import/Export (6 tools)
- get_imports, get_import_xrefs, categorize_import, find_dangerous_apis
- get_exports, document_export

### Functions (6 tools)
- add_function, get_function, get_function_callers, get_function_callees
- trace_call_path, find_dispatch_handlers

### Structures (4 tools)
- add_structure, get_structure, list_structures, link_structure_to_ioctl

### Vulnerabilities (5 tools)
- add_vulnerability, get_vulnerability, list_vulnerabilities
- create_attack_chain, get_attack_chains

### Search (6 tools)
- semantic_search, text_search, search_strings
- find_similar_ioctls, find_similar_vulnerabilities, search_by_api_usage

### Cross-References (5 tools)
- add_xref, get_xrefs_to, get_xrefs_from, build_call_graph, find_paths_to_api

### Ghidra Integration (8 tools)
- ghidra_connect, ghidra_sync_functions, ghidra_sync_structures
- ghidra_get_decompilation, ghidra_get_xrefs, ghidra_set_comment
- ghidra_rename_function, ghidra_export_all

### Analysis Session (8 tools)
- start_analysis_session, add_analysis_note, get_analysis_notes
- generate_analysis_report, convert_address, get_api_info
- compare_drivers, get_statistics

## Scanner Integration

The MCP server integrates with [ombra-driver-scanner](../ombra-driver-scanner) for automated driver analysis workflows.

### Quick Start

```bash
# 1. Run scanner to generate batch JSON
cd /path/to/ombra-driver-scanner
./scanner --batch /drivers/*.sys --output /tmp/scan_results.json

# 2. Import results into MCP
mcp-cli call driver-re-mcp/import_scanner_results '{
  "json_path": "/tmp/scan_results.json"
}'

# 3. Get priority queue for analysis
mcp-cli call driver-re-mcp/get_analysis_queue '{
  "min_priority": 1,
  "max_priority": 2
}'

# 4. Find all drivers with specific capabilities
mcp-cli call driver-re-mcp/get_drivers_by_capability '{
  "capability": "module_loading"
}'
```

See [docs/scanner_integration.md](docs/scanner_integration.md) for complete documentation.

## Database Schema

See `src/driver_re_mcp/database/schema.sql` for complete schema.

## License

Private - PROJECT-OMBRA
