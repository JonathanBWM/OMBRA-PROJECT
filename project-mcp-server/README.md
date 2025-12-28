# Project Management MCP Server

The external brain for agent intelligence. Provides comprehensive project management, codebase analysis, and driver scanning integration for the OMBRA hypervisor development project.

## Overview

This MCP server provides 45+ tools organized into these categories:

- **File & Function Indexing** - Track code structure and locate implementations
- **Feature-to-File Linkage** - Map features to their implementation files
- **Stub Detection** - Identify incomplete implementations requiring work
- **Task & Epic Management** - Track development work items and milestones
- **Impact Analysis** - Understand change ripple effects across the codebase
- **Codebase Health Metrics** - Monitor code quality and completeness
- **Dashboard Data** - Aggregate metrics for visualization
- **Driver Scanner Integration** - Connect to ombra-driver-scanner for analysis

## Installation

### Prerequisites

- Python 3.10 or higher
- Virtual environment (recommended)

### Setup

```bash
cd /Users/jonathanmcclintock/PROJECT-OMBRA/project-mcp-server

# Create and activate virtual environment
python3 -m venv .venv
source .venv/bin/activate  # On Windows: .venv\Scripts\activate

# Install the package
pip install -e .

# Verify installation
project-mcp --version
```

### Optional Dashboard Installation

For the web-based dashboard:

```bash
pip install -e ".[dashboard]"
project-dashboard
```

## MCP Configuration

Add to your `.mcp.json` file:

```json
{
  "mcpServers": {
    "project-mcp": {
      "command": "/Users/jonathanmcclintock/PROJECT-OMBRA/project-mcp-server/.venv/bin/python",
      "args": [
        "-m",
        "project_mcp.server"
      ],
      "env": {
        "PYTHONPATH": "/Users/jonathanmcclintock/PROJECT-OMBRA/project-mcp-server/src",
        "PROJECT_ROOT": "/Users/jonathanmcclintock/PROJECT-OMBRA",
        "PROJECT_DB": "/Users/jonathanmcclintock/PROJECT-OMBRA/project-mcp-server/src/project_mcp/data/project_mgmt.db",
        "DRIVER_SCANNER_ROOT": "/Users/jonathanmcclintock/Desktop/Projects/Ombra/ombra-driver-scanner",
        "DRIVER_ANALYSIS_OUTPUT": "/Users/jonathanmcclintock/PROJECT-OMBRA/driver-analysis-results"
      }
    }
  }
}
```

### Environment Variables

| Variable | Purpose | Default |
|----------|---------|---------|
| `PROJECT_ROOT` | Root directory of OMBRA project | Required |
| `PROJECT_DB` | SQLite database file path | `src/project_mcp/data/project_mgmt.db` |
| `DRIVER_SCANNER_ROOT` | Path to ombra-driver-scanner project | Optional |
| `DRIVER_ANALYSIS_OUTPUT` | Directory for driver analysis results | Optional |
| `PYTHONPATH` | Python module search path | Required for import resolution |

## Available Tools

### Component Management (5 tools)

- `register_component` - Register a new system component
- `get_component` - Retrieve component details
- `list_components` - List all registered components
- `update_component_status` - Update component implementation status
- `get_component_health` - Get health metrics for a component

### Feature Management (8 tools)

- `register_feature` - Register a new feature
- `get_feature` - Get feature details
- `list_features` - List features with optional filtering
- `update_feature_status` - Update feature implementation status
- `link_feature_to_file` - Link feature to implementation file
- `get_feature_files` - Get all files implementing a feature
- `get_file_features` - Get all features implemented in a file
- `get_feature_dependencies` - Analyze feature dependencies

### File Indexing (5 tools)

- `index_file` - Index a file with metadata
- `search_files` - Full-text search across indexed files
- `get_file_info` - Get metadata for a specific file
- `update_file_metadata` - Update file tags and metadata
- `list_files_by_tag` - Find files with specific tags

### Function Indexing (7 tools)

- `index_function` - Index a function with signature and location
- `search_functions` - Full-text search for functions
- `find_function_by_name` - Locate function by exact name
- `get_function_callers` - Find functions that call this function
- `get_function_callees` - Find functions called by this function
- `update_function_tags` - Tag functions for categorization
- `list_functions_by_file` - Get all functions in a file

### Stub Detection (5 tools)

- `mark_as_stub` - Mark function/feature as stub (incomplete)
- `unmark_stub` - Remove stub marking
- `list_stubs` - Find all stubs with filtering options
- `get_stub_metrics` - Get statistics on stub distribution
- `find_critical_stubs` - Identify high-priority incomplete work

### Task Management (9 tools)

- `create_task` - Create new task with priority and assignee
- `get_task` - Retrieve task details
- `update_task_status` - Update task progress
- `assign_task` - Assign task to developer
- `list_tasks` - List tasks with filtering
- `create_epic` - Create epic (collection of tasks)
- `link_task_to_epic` - Associate task with epic
- `get_epic_progress` - Get epic completion metrics
- `get_developer_workload` - Analyze developer task load

### Analysis & Recommendations (6 tools)

- `analyze_codebase` - Comprehensive codebase health analysis
- `get_recommendations` - Get prioritized work recommendations
- `analyze_feature_coverage` - Check feature implementation coverage
- `detect_code_smells` - Identify potential code quality issues
- `analyze_dependencies` - Map component dependencies
- `suggest_refactoring` - Get refactoring suggestions

### Impact Analysis (4 tools)

- `analyze_change_impact` - Predict effects of modifying a file
- `get_dependent_features` - Find features depending on a component
- `calculate_blast_radius` - Estimate change propagation scope
- `find_affected_tests` - Identify tests affected by changes

### Dashboard & Metrics (5 tools)

- `get_dashboard_summary` - Overall project health snapshot
- `get_velocity_metrics` - Development velocity over time
- `get_quality_metrics` - Code quality indicators
- `get_completion_forecast` - Estimate completion timelines
- `export_metrics_csv` - Export metrics for external analysis

## Integration with Other MCP Servers

The OMBRA project uses three MCP servers working together:

### 1. ombra (Hypervisor Development MCP)

**Purpose:** Intel SDM reference, VMCS field lookups, exit handlers, MSR information

**Entry Point:** `ombra-mcp` command

**Tools:** 27 hypervisor-specific development tools including:
- VMCS field encoding lookups
- VM-exit reason explanations
- MSR information and bitmaps
- Anti-cheat evasion techniques
- Timing simulation and analysis
- Binary signature scanning

**Database:** `intel_sdm.db` with 167 VMCS fields, 66 exit reasons, 35 MSRs

### 2. project-mcp (This Server)

**Purpose:** Project management, codebase indexing, task tracking, driver scanner integration

**Entry Point:** `project-mcp` command

**Tools:** 45+ project management and analysis tools

**Database:** `project_mgmt.db` with SQLite FTS5 for full-text search

### 3. driver-re (Driver Reverse Engineering MCP)

**Purpose:** Windows kernel driver analysis, IOCTL extraction, vulnerability detection, Microsoft Learn integration

**Entry Point:** `driver-re-mcp` command

**Tools:** 80+ driver analysis tools including:
- Driver database management (add, get, list, update status)
- IOCTL enumeration and vulnerability analysis
- Import/export analysis with cross-references
- Dangerous API detection (process manipulation, memory access)
- Semantic search across driver knowledge
- Microsoft Learn documentation integration
- PE file parsing and analysis

**Databases:**
- `driver_re.db` - Driver metadata, IOCTLs, imports/exports
- `mslearn_cache` - Cached Microsoft documentation
- `chroma` - Vector embeddings for semantic search

### Workflow Integration Example

1. **Use `ombra`** to look up VMCS field encodings for your hypervisor implementation
2. **Use `project-mcp`** to track which files implement those features and mark stubs
3. **Use `driver-re`** to analyze kernel drivers you're mapping or studying for primitives
4. **Use `project-mcp`** to create tasks for implementing missing features
5. **Use `ombra`** to generate timing-safe implementations that evade detection
6. **Use `driver-re`** to validate your driver against known signatures and blocklists

## Database Schema

The server uses SQLite with FTS5 (Full-Text Search) for fast queries.

### Core Tables

- `components` - System components (EPT, VMCS, VMX operations, etc.)
- `features` - Individual features within components
- `files` - Indexed source files with metadata
- `functions` - Function definitions with signatures
- `feature_files` - Many-to-many feature-file relationships
- `stubs` - Incomplete implementations requiring work
- `tasks` - Development work items
- `epics` - Collections of related tasks

### FTS5 Tables (Full-Text Search)

- `files_fts` - Fast search across file content
- `functions_fts` - Fast search across function code

## Usage Examples

### Indexing Your Codebase

```python
# Index a new file
await index_file(
    path="/path/to/vmx_operations.c",
    language="C",
    component="VMX",
    tags=["vmx", "cpu", "low-level"]
)

# Index a function
await index_function(
    name="vmx_vmlaunch",
    file_path="/path/to/vmx_operations.c",
    signature="int vmx_vmlaunch(struct vcpu *vcpu)",
    start_line=245,
    end_line=312,
    tags=["vmx", "critical"]
)
```

### Feature Tracking

```python
# Register a feature
await register_feature(
    name="EPT Violation Handler",
    component="EPT",
    status="in_progress",
    priority="high"
)

# Link feature to implementation files
await link_feature_to_file(
    feature_name="EPT Violation Handler",
    file_path="/path/to/ept_handler.c",
    role="primary"
)
```

### Stub Detection

```python
# Mark incomplete work
await mark_as_stub(
    type="function",
    identifier="handle_cpuid_exit",
    reason="Needs full CPUID leaf emulation",
    priority="high"
)

# Find all high-priority stubs
stubs = await list_stubs(priority="high")
```

### Task Management

```python
# Create an epic for a major feature
await create_epic(
    name="Full EPT Implementation",
    description="Complete EPT setup, violation handling, and optimization",
    target_date="2025-02-15"
)

# Create tasks under the epic
await create_task(
    title="Implement EPT page table walker",
    epic_name="Full EPT Implementation",
    priority="high",
    assignee="LO"
)
```

### Impact Analysis

```python
# Analyze what changing a file affects
impact = await analyze_change_impact(
    file_path="/path/to/vmcs.c"
)
# Returns: affected features, dependent components, related tests
```

## Driver Scanner Integration

The `DRIVER_SCANNER_ROOT` environment variable connects this MCP server to the ombra-driver-scanner project for automated driver analysis.

### Scanner Capabilities

- Binary signature detection for anti-cheat software
- Source code pattern matching for vulnerabilities
- VMCS configuration validation
- Anti-cheat evasion technique identification
- Timing requirement analysis

### Using Scanner Results

Scanner results stored in `DRIVER_ANALYSIS_OUTPUT` can be:
- Indexed as files for searchability
- Linked to features they inform
- Tracked as tasks for remediation
- Referenced in impact analysis

## Performance Considerations

### Database Optimization

- FTS5 provides fast full-text search (< 50ms for most queries)
- Indexes on commonly queried fields (component, status, priority)
- VACUUM database periodically to reclaim space
- Use parameterized queries to prevent SQL injection

### Caching Strategy

- Function call graphs cached in memory
- File metadata cached with 5-minute TTL
- Dashboard metrics cached with 1-minute TTL
- Full-text search results not cached (queries are fast)

### Scalability

Current design handles:
- 10,000+ files
- 50,000+ functions
- 1,000+ features
- 5,000+ tasks

For larger codebases, consider:
- Partitioning database by component
- Using PostgreSQL instead of SQLite
- Implementing a caching layer (Redis)
- Batch indexing operations

## Troubleshooting

### Database Locked Errors

```bash
# Kill any hanging connections
pkill -f project-mcp

# Rebuild database if corrupted
rm src/project_mcp/data/project_mgmt.db
python -c "from project_mcp.database import sync_init_db; sync_init_db()"
```

### Import Resolution Issues

```bash
# Verify PYTHONPATH is set correctly
echo $PYTHONPATH

# Should include: /Users/jonathanmcclintock/PROJECT-OMBRA/project-mcp-server/src

# Reinstall if needed
pip install -e . --force-reinstall
```

### MCP Connection Problems

```bash
# Test the server directly
echo '{"jsonrpc":"2.0","method":"tools/list","id":1}' | project-mcp

# Check logs
tail -f ~/.mcp/logs/project-mcp.log
```

## Development

### Running Tests

```bash
pip install -e ".[dev]"
pytest tests/
```

### Adding New Tools

1. Create tool function in appropriate module under `src/project_mcp/tools/`
2. Add to `ALL_TOOLS` list in `src/project_mcp/tools/__init__.py`
3. Update this README with tool documentation
4. Add tests in `tests/test_tools.py`

### Database Migrations

```bash
# Create migration script
python scripts/create_migration.py "add_new_table"

# Apply migrations
python scripts/apply_migrations.py
```

## License

Private - OMBRA Project

## Support

For issues or questions:
- Check troubleshooting section above
- Review MCP server logs in `~/.mcp/logs/`
- Verify environment variables are set correctly
- Ensure all three MCP servers are installed and configured

## Related Documentation

- [ombra_mcp_server README](../ombra_mcp_server/README.md) - Hypervisor development tools
- [driver-re-mcp README](../driver-re-mcp/README.md) - Driver reverse engineering tools
- [MCP Protocol Specification](https://modelcontextprotocol.io/) - Official MCP documentation
