# OMBRA MCP Integration Guide

Complete guide to the three-server MCP architecture powering the OMBRA hypervisor development project.

## Architecture Overview

The OMBRA project uses three specialized MCP servers that work together:

```
┌─────────────────────────────────────────────────────────────┐
│                    Claude Desktop/CLI                        │
└─────────────────────────────────────────────────────────────┘
                             │
                             │ MCP Protocol (stdio)
                             │
        ┌────────────────────┼────────────────────┐
        │                    │                    │
        ▼                    ▼                    ▼
┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│   ombra      │    │ project-mcp  │    │  driver-re   │
│              │    │              │    │              │
│ Intel SDM    │    │ Codebase     │    │ Driver       │
│ VMCS/MSR     │    │ Management   │    │ Analysis     │
│ Exit Handlers│    │ Task Track   │    │ IOCTL Enum   │
│ Anti-Cheat   │    │ File Index   │    │ MS Learn     │
│              │    │              │    │              │
│ 27 tools     │    │ 45+ tools    │    │ 80+ tools    │
└──────────────┘    └──────────────┘    └──────────────┘
       │                    │                    │
       ▼                    ▼                    ▼
intel_sdm.db      project_mgmt.db        driver_re.db
                                         mslearn_cache
                                         chroma_db
```

## Configuration File

Location: `/Users/jonathanmcclintock/PROJECT-OMBRA/.mcp.json`

```json
{
  "mcpServers": {
    "ombra": {
      "command": "/Users/jonathanmcclintock/PROJECT-OMBRA/ombra_mcp_server/.venv/bin/python",
      "args": ["-m", "ombra_mcp.server"],
      "env": {
        "PYTHONPATH": "/Users/jonathanmcclintock/PROJECT-OMBRA/ombra_mcp_server/src",
        "OMBRA_PROJECT_ROOT": "/Users/jonathanmcclintock/PROJECT-OMBRA",
        "OMBRA_DATA_DIR": "/Users/jonathanmcclintock/PROJECT-OMBRA/ombra_mcp_server/src/ombra_mcp/data",
        "INTEL_SDM_DB": "/Users/jonathanmcclintock/PROJECT-OMBRA/ombra_mcp_server/src/ombra_mcp/data/intel_sdm.db"
      }
    },
    "project-mcp": {
      "command": "/Users/jonathanmcclintock/PROJECT-OMBRA/project-mcp-server/.venv/bin/python",
      "args": ["-m", "project_mcp.server"],
      "env": {
        "PYTHONPATH": "/Users/jonathanmcclintock/PROJECT-OMBRA/project-mcp-server/src",
        "PROJECT_ROOT": "/Users/jonathanmcclintock/PROJECT-OMBRA",
        "PROJECT_DB": "/Users/jonathanmcclintock/PROJECT-OMBRA/project-mcp-server/src/project_mcp/data/project_mgmt.db",
        "DRIVER_SCANNER_ROOT": "/Users/jonathanmcclintock/Desktop/Projects/Ombra/ombra-driver-scanner",
        "DRIVER_ANALYSIS_OUTPUT": "/Users/jonathanmcclintock/PROJECT-OMBRA/driver-analysis-results"
      }
    },
    "driver-re": {
      "command": "/Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp/.venv/bin/python",
      "args": ["-m", "driver_re_mcp.server"],
      "env": {
        "PYTHONPATH": "/Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp/src",
        "DRIVER_RE_DB": "/Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp/src/driver_re_mcp/data/driver_re.db",
        "MSLEARN_CACHE": "/Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp/src/driver_re_mcp/data/mslearn_cache",
        "DRIVER_SAMPLES_DIR": "/Users/jonathanmcclintock/PROJECT-OMBRA/driver-samples",
        "CHROMA_DB_DIR": "/Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp/src/driver_re_mcp/data/chroma"
      }
    }
  }
}
```

## Server 1: ombra (Hypervisor Development MCP)

**Package:** `ombra-mcp`
**Entry Point:** `ombra-mcp` command
**Source:** `/Users/jonathanmcclintock/PROJECT-OMBRA/ombra_mcp_server`

### Purpose

Intel SDM reference implementation providing immediate access to hypervisor development details without manual lookup.

### Core Capabilities

- **VMCS Field Reference** - 167 field encodings with access types and descriptions
- **VM-Exit Handlers** - 66 exit reasons with qualification formats and handler templates
- **MSR Information** - 35 model-specific registers with bitmaps and access semantics
- **Exception Vectors** - 20 exception types with error code formats
- **Anti-Cheat Evasion** - Timing analysis, signature detection, bypass techniques
- **Code Generation** - VMCS headers, exit handler skeletons, ASM stubs

### Key Tools

```
vmcs_field                  - Get VMCS field encoding and details
vmcs_fields_by_category     - List fields by category (Guest, Host, Control, etc.)
exit_reason                 - Get exit reason details and qualification format
exit_qualification_format   - Parse exit qualification bits
msr_info                    - Get MSR details and bitmap position
search_sdm                  - Full-text search across Intel SDM content
generate_vmcs_header        - Generate C header with VMCS field defines
generate_exit_handler_skeleton - Generate exit handler template
generate_msr_header         - Generate MSR bitmap helper code
get_anticheat_intel         - Get detection methods for specific anti-cheat
get_timing_requirements     - Get acceptable timing ranges for operations
simulate_handler_timing     - Validate handler timing against detection
scan_binary_signatures      - Scan binary for anti-cheat signatures
validate_vmcs_setup         - Check VMCS configuration for issues
```

### Database

`intel_sdm.db` - SQLite database with:
- 167 VMCS fields (Guest State, Host State, VM-Execution Controls, VM-Exit Controls, VM-Entry Controls)
- 66 VM-exit reasons with qualification bit layouts
- 35 MSRs with read/write semantics
- 20 exception vectors with error code formats

### Installation

```bash
cd /Users/jonathanmcclintock/PROJECT-OMBRA/ombra_mcp_server
python3 -m venv .venv
source .venv/bin/activate
pip install -e .
python scripts/prepare_intel_sdm.py  # Build database
```

### Use Cases

- Look up VMCS field encoding before writing initialization code
- Generate exit handler skeleton for new exit reason
- Check MSR bitmap setup for performance-critical MSRs
- Validate VMCS configuration before VMLAUNCH
- Analyze timing requirements for anti-cheat evasion
- Scan binaries for known anti-cheat signatures

---

## Server 2: project-mcp (Project Management MCP)

**Package:** `project-mcp`
**Entry Point:** `project-mcp` command
**Source:** `/Users/jonathanmcclintock/PROJECT-OMBRA/project-mcp-server`

### Purpose

External brain for project intelligence - tracks implementation status, manages tasks, analyzes codebase health.

### Core Capabilities

- **File & Function Indexing** - Full-text searchable code index
- **Feature Tracking** - Map features to implementation files
- **Stub Detection** - Identify incomplete implementations
- **Task Management** - Epics, tasks, assignments, progress tracking
- **Impact Analysis** - Understand change propagation
- **Codebase Metrics** - Health, velocity, quality indicators
- **Driver Scanner Integration** - Link to ombra-driver-scanner results

### Key Tools

```
# File & Function Indexing
index_file                  - Index file with metadata and tags
search_files                - Full-text search across indexed files
index_function              - Index function with signature and location
search_functions            - Full-text search for functions
find_function_by_name       - Locate function by exact name

# Feature Management
register_feature            - Register new feature with component
link_feature_to_file        - Link feature to implementation file
get_feature_files           - Get all files implementing a feature
update_feature_status       - Update feature implementation status
get_feature_dependencies    - Analyze feature dependencies

# Stub Detection
mark_as_stub                - Mark function/feature as incomplete
list_stubs                  - Find all stubs with filtering
find_critical_stubs         - Identify high-priority incomplete work
get_stub_metrics            - Statistics on stub distribution

# Task Management
create_task                 - Create task with priority and assignee
create_epic                 - Create epic (collection of tasks)
link_task_to_epic           - Associate task with epic
get_epic_progress           - Get epic completion metrics
get_developer_workload      - Analyze developer task load

# Impact Analysis
analyze_change_impact       - Predict effects of modifying a file
calculate_blast_radius      - Estimate change propagation scope
get_dependent_features      - Find features depending on component

# Codebase Analysis
analyze_codebase            - Comprehensive health analysis
get_recommendations         - Prioritized work recommendations
detect_code_smells          - Identify code quality issues
suggest_refactoring         - Get refactoring suggestions

# Dashboard & Metrics
get_dashboard_summary       - Overall project health snapshot
get_velocity_metrics        - Development velocity over time
get_quality_metrics         - Code quality indicators
get_completion_forecast     - Estimate completion timelines
```

### Database

`project_mgmt.db` - SQLite with FTS5 full-text search:
- `components` - System components (EPT, VMCS, VMX, etc.)
- `features` - Individual features within components
- `files` - Indexed source files with metadata
- `functions` - Function definitions with signatures
- `feature_files` - Many-to-many feature-file relationships
- `stubs` - Incomplete implementations requiring work
- `tasks` - Development work items
- `epics` - Collections of related tasks
- `files_fts` - Full-text search index for files
- `functions_fts` - Full-text search index for functions

### Installation

```bash
cd /Users/jonathanmcclintock/PROJECT-OMBRA/project-mcp-server
python3 -m venv .venv
source .venv/bin/activate
pip install -e .
# Database auto-initializes on first run
```

### Use Cases

- Index new source file after creating it
- Search for all functions related to EPT handling
- Mark CPUID emulation as stub needing work
- Create epic for "Full Anti-Cheat Evasion" with subtasks
- Analyze impact of changing VMCS initialization code
- Get dashboard showing project completion percentage
- Track which files implement "VM-Exit Handling" feature

---

## Server 3: driver-re (Driver Reverse Engineering MCP)

**Package:** `driver-re-mcp`
**Entry Point:** `driver-re-mcp` command
**Source:** `/Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp`

### Purpose

Windows kernel driver analysis and reverse engineering with Microsoft Learn integration.

### Core Capabilities

- **Driver Database** - Catalog analyzed drivers with metadata
- **IOCTL Enumeration** - Extract and categorize IOCTL handlers
- **Import/Export Analysis** - Track API usage and cross-references
- **Vulnerability Detection** - Identify dangerous patterns and primitives
- **Semantic Search** - Vector embeddings for conceptual queries
- **Microsoft Learn Integration** - Cached API documentation and examples
- **PE File Parsing** - Binary structure analysis

### Key Tools

```
# Driver Database Management
dre_add_driver              - Add driver to database with metadata
dre_get_driver              - Retrieve driver details
dre_list_drivers            - List drivers with filtering
dre_update_driver_status    - Update analysis status
dre_delete_driver           - Remove driver from database

# IOCTL Analysis
dre_add_ioctl               - Add IOCTL handler to database
dre_get_ioctl               - Get IOCTL details by code
dre_list_ioctls             - List IOCTLs for a driver
dre_get_vulnerable_ioctls   - Find IOCTLs with vulnerabilities
dre_update_ioctl_vulnerability - Mark IOCTL as vulnerable

# Import/Export Analysis
dre_get_imports             - Get imported functions for driver
dre_get_import_xrefs        - Find cross-references to import
dre_categorize_import       - Categorize import by functionality
dre_find_dangerous_apis     - Identify process/memory manipulation APIs
dre_get_exports             - Get exported functions
dre_document_export         - Add documentation for export

# Microsoft Learn Integration
mslearn_search              - Search Microsoft documentation
mslearn_api                 - Get API reference documentation
mslearn_concept             - Get conceptual documentation
mslearn_page                - Retrieve specific documentation page
mslearn_examples            - Find code examples for API
mslearn_topics              - List available documentation topics

# Semantic Search
semantic_search             - Vector similarity search across knowledge
rebuild_semantic_index      - Rebuild embeddings for search
get_semantic_index_stats    - Check index health and coverage

# Analysis & Detection
get_drivers                 - List drivers by capability
get_driver_primitives       - Get exploitation primitives for driver
get_driver_ioctls           - Enumerate IOCTL codes
check_driver_blocklist      - Check if driver is blocked by anti-cheat
get_driver_gotchas          - Get known issues and quirks
search_drivers              - Search drivers by name or capability
find_drivers_with_primitive - Find drivers providing specific primitive
```

### Databases

**driver_re.db** - SQLite with:
- `drivers` - Driver metadata (name, version, hash, status)
- `ioctls` - IOCTL handlers with codes and parameters
- `imports` - Imported functions with categories
- `exports` - Exported functions with documentation
- `import_xrefs` - Cross-references for imports
- `vulnerabilities` - Known vulnerabilities and exploits
- `driver_capabilities` - Exploitation primitives by driver

**mslearn_cache/** - Cached Microsoft documentation:
- API reference pages
- Conceptual guides
- Code examples
- WDM/WDF documentation

**chroma/** - ChromaDB vector database:
- Document embeddings for semantic search
- API documentation vectors
- Driver analysis notes vectors

### Installation

```bash
cd /Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp
python3 -m venv .venv
source .venv/bin/activate
pip install -e .
# Databases auto-initialize on first use
```

### Use Cases

- Add vulnerable driver to database for analysis
- Extract all IOCTL codes from a driver binary
- Find drivers providing arbitrary memory read/write
- Check if driver is on anti-cheat blocklist
- Search Microsoft docs for ZwMapViewOfSection usage
- Semantic search for "how to allocate MDL in kernel"
- Find cross-references to NtQuerySystemInformation import
- Identify dangerous APIs (PsLookupProcessByProcessId, etc.)

---

## Workflow Integration Examples

### Example 1: Implementing New Exit Handler

**Goal:** Implement CPUID exit handler with anti-cheat evasion

```
1. ombra/exit_reason
   Input: "CPUID"
   Output: Exit reason 10, qualification format, register state

2. ombra/generate_exit_handler_skeleton
   Input: exit_reason=10
   Output: C function skeleton with register access

3. project-mcp/register_feature
   Input: name="CPUID Exit Handler", component="Exit Handlers"
   Output: Feature registered in database

4. ombra/get_anticheat_intel
   Input: anticheat="EasyAntiCheat"
   Output: CPUID spoofing requirements, timing constraints

5. ombra/get_timing_requirements
   Input: operation="cpuid_emulation"
   Output: Max 500ns to avoid detection

6. project-mcp/index_file
   Input: path="src/vmx/exit_handlers/cpuid.c"
   Output: File indexed and linked to feature

7. project-mcp/create_task
   Input: title="Test CPUID handler against EAC", epic="Exit Handlers"
   Output: Task created with priority high
```

### Example 2: Analyzing Vulnerable Driver

**Goal:** Find driver with memory write primitive for hypervisor mapping

```
1. driver-re/dre_add_driver
   Input: name="vulnerable.sys", path="/samples/vulnerable.sys"
   Output: Driver added to database

2. driver-re/dre_get_imports
   Input: driver_id=1
   Output: List of imported kernel APIs

3. driver-re/dre_find_dangerous_apis
   Input: driver_id=1
   Output: Found MmMapIoSpace, ZwMapViewOfSection

4. driver-re/dre_list_ioctls
   Input: driver_id=1
   Output: 15 IOCTL codes found

5. driver-re/dre_get_vulnerable_ioctls
   Input: driver_id=1
   Output: IOCTL 0x22E004 has arbitrary write

6. driver-re/check_driver_blocklist
   Input: driver_name="vulnerable.sys"
   Output: Not on EAC/BE blocklists

7. project-mcp/create_task
   Input: title="Integrate vulnerable.sys for mapping"
   Output: Task created

8. driver-re/mslearn_api
   Input: api_name="MmMapIoSpace"
   Output: Usage guide, parameters, security notes
```

### Example 3: Validating VMCS Setup

**Goal:** Ensure VMCS configuration passes anti-cheat detection

```
1. ombra/validate_vmcs_setup
   Input: vmcs_config (JSON of current settings)
   Output: 3 issues found - MSR bitmap misconfigured

2. ombra/get_vmcs_checklist
   Output: 47-point checklist with status

3. ombra/vmcs_fields_by_category
   Input: category="VM-Execution Controls"
   Output: All execution control fields

4. ombra/msr_info
   Input: msr_name="IA32_TSC_DEADLINE"
   Output: MSR 0x6E0, bitmap offset, access semantics

5. project-mcp/mark_as_stub
   Input: type="feature", identifier="MSR Bitmap Setup"
   Output: Marked as incomplete

6. project-mcp/create_task
   Input: title="Fix MSR bitmap for TSC deadline timer"
   Output: Task created with high priority

7. ombra/generate_msr_header
   Input: msr_list=["IA32_TSC_DEADLINE", "IA32_APIC_BASE"]
   Output: C header with bitmap helper functions
```

### Example 4: Codebase Health Check

**Goal:** Assess project completion and prioritize work

```
1. project-mcp/get_dashboard_summary
   Output: 62% complete, 147 tasks, 23 stubs, velocity declining

2. project-mcp/list_stubs
   Input: priority="high"
   Output: 12 critical stubs (EPT violation, CPUID, I/O ports)

3. project-mcp/get_recommendations
   Output: Top 5 features to implement based on dependencies

4. project-mcp/analyze_codebase
   Output: 15 code smells, 8 circular dependencies, test coverage 34%

5. project-mcp/get_completion_forecast
   Output: Estimated 6 weeks at current velocity

6. project-mcp/create_epic
   Input: name="Critical Path to MVP"
   Output: Epic created

7. project-mcp/find_critical_stubs
   Output: CPUID emulation blocks 4 features, prioritize first
```

---

## Environment Variables Reference

### ombra Server

| Variable | Purpose | Required |
|----------|---------|----------|
| `PYTHONPATH` | Python import path | Yes |
| `OMBRA_PROJECT_ROOT` | Project root directory | Yes |
| `OMBRA_DATA_DIR` | Data directory for databases | Yes |
| `INTEL_SDM_DB` | Intel SDM SQLite database | Yes |

### project-mcp Server

| Variable | Purpose | Required |
|----------|---------|----------|
| `PYTHONPATH` | Python import path | Yes |
| `PROJECT_ROOT` | Project root directory | Yes |
| `PROJECT_DB` | Project management SQLite database | Yes |
| `DRIVER_SCANNER_ROOT` | ombra-driver-scanner project path | No |
| `DRIVER_ANALYSIS_OUTPUT` | Driver analysis results directory | No |

### driver-re Server

| Variable | Purpose | Required |
|----------|---------|----------|
| `PYTHONPATH` | Python import path | Yes |
| `DRIVER_RE_DB` | Driver analysis SQLite database | Yes |
| `MSLEARN_CACHE` | Microsoft Learn cache directory | Yes |
| `DRIVER_SAMPLES_DIR` | Directory for driver samples | No |
| `CHROMA_DB_DIR` | ChromaDB vector database directory | Yes |

---

## Installation Checklist

### 1. Install All Servers

```bash
# ombra server
cd /Users/jonathanmcclintock/PROJECT-OMBRA/ombra_mcp_server
python3 -m venv .venv
source .venv/bin/activate
pip install -e .
python scripts/prepare_intel_sdm.py

# project-mcp server
cd /Users/jonathanmcclintock/PROJECT-OMBRA/project-mcp-server
python3 -m venv .venv
source .venv/bin/activate
pip install -e .

# driver-re server
cd /Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp
python3 -m venv .venv
source .venv/bin/activate
pip install -e .
```

### 2. Verify Entry Points

```bash
# Should all execute without errors
/Users/jonathanmcclintock/PROJECT-OMBRA/ombra_mcp_server/.venv/bin/python -m ombra_mcp.server --version
/Users/jonathanmcclintock/PROJECT-OMBRA/project-mcp-server/.venv/bin/python -m project_mcp.server --version
/Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp/.venv/bin/python -m driver_re_mcp.server --version
```

### 3. Configure MCP Client

Copy `.mcp.json` to your Claude Desktop/CLI configuration directory:

```bash
# For Claude Desktop (macOS)
cp /Users/jonathanmcclintock/PROJECT-OMBRA/.mcp.json ~/Library/Application\ Support/Claude/

# For Claude Code CLI
cp /Users/jonathanmcclintock/PROJECT-OMBRA/.mcp.json ~/.config/claude-code/
```

### 4. Test Connections

In Claude, try these test commands:

```
ombra test:          "Look up VMCS field GUEST_RIP"
project-mcp test:    "List all registered components"
driver-re test:      "List all drivers in database"
```

### 5. Verify Databases

```bash
# Check databases exist and have data
sqlite3 /Users/jonathanmcclintock/PROJECT-OMBRA/ombra_mcp_server/src/ombra_mcp/data/intel_sdm.db "SELECT COUNT(*) FROM vmcs_fields;"
# Should return 167

sqlite3 /Users/jonathanmcclintock/PROJECT-OMBRA/project-mcp-server/src/project_mcp/data/project_mgmt.db "SELECT name FROM sqlite_master WHERE type='table';"
# Should list tables

sqlite3 /Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp/src/driver_re_mcp/data/driver_re.db "SELECT name FROM sqlite_master WHERE type='table';"
# Should list tables
```

---

## Troubleshooting

### Server Won't Start

**Symptom:** MCP server fails to initialize

**Checks:**
1. Verify virtual environment is activated
2. Check PYTHONPATH points to `src/` directory
3. Ensure database files exist
4. Check permissions on data directories
5. Review logs in `~/.mcp/logs/[server-name].log`

**Fix:**
```bash
cd /Users/jonathanmcclintock/PROJECT-OMBRA/[server-name]
source .venv/bin/activate
pip install -e . --force-reinstall
```

### Database Locked

**Symptom:** "database is locked" errors

**Fix:**
```bash
# Kill any hanging processes
pkill -f "ombra_mcp\|project_mcp\|driver_re_mcp"

# If corruption suspected, rebuild database
cd /Users/jonathanmcclintock/PROJECT-OMBRA/[server-name]
source .venv/bin/activate
# For ombra:
python scripts/prepare_intel_sdm.py
# For others, delete and let auto-recreate:
rm src/*/data/*.db
```

### Import Errors

**Symptom:** `ModuleNotFoundError: No module named 'ombra_mcp'`

**Fix:**
```bash
# Check PYTHONPATH in .mcp.json matches actual paths
# Should be: /path/to/server/src

# Verify package installed in development mode
cd /Users/jonathanmcclintock/PROJECT-OMBRA/[server-name]
source .venv/bin/activate
pip list | grep -E "ombra-mcp|project-mcp|driver-re-mcp"
# Should show packages installed from local paths
```

### No Tools Available

**Symptom:** Claude reports no MCP tools available

**Checks:**
1. Verify `.mcp.json` is in correct location
2. Check file permissions (should be readable)
3. Validate JSON syntax (use `jq . < .mcp.json`)
4. Restart Claude Desktop/CLI after config changes
5. Check server logs for startup errors

**Fix:**
```bash
# Validate configuration
jq . < /Users/jonathanmcclintock/PROJECT-OMBRA/.mcp.json

# Test server directly
echo '{"jsonrpc":"2.0","method":"tools/list","id":1}' | \
  /Users/jonathanmcclintock/PROJECT-OMBRA/ombra_mcp_server/.venv/bin/python -m ombra_mcp.server
```

---

## Performance Optimization

### Database Optimization

```bash
# Vacuum databases to reclaim space
sqlite3 [database].db "VACUUM;"

# Analyze query plans
sqlite3 [database].db "EXPLAIN QUERY PLAN SELECT ..."

# Add indexes for common queries
sqlite3 [database].db "CREATE INDEX idx_component ON features(component);"
```

### ChromaDB Tuning (driver-re only)

```bash
# Rebuild semantic index with optimal settings
cd /Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp
source .venv/bin/activate
python scripts/rebuild_chroma_index.py --batch-size 100
```

### Caching Strategy

- **ombra:** Intel SDM data cached in memory on startup
- **project-mcp:** File metadata cached with 5-minute TTL
- **driver-re:** Microsoft Learn responses cached to disk indefinitely

---

## Security Considerations

### Environment Variables

Never commit `.mcp.json` with sensitive paths to public repositories. Use environment variable substitution if needed.

### Database Permissions

Ensure database files are only readable by your user:

```bash
chmod 600 /Users/jonathanmcclintock/PROJECT-OMBRA/*/src/*/data/*.db
```

### Driver Analysis

When analyzing third-party drivers:
- Scan in isolated VM/sandbox
- Don't execute untrusted driver code
- Validate all file paths before parsing
- Sanitize inputs to SQL queries (parameterized queries used throughout)

---

## Future Enhancements

### Planned Features

**ombra:**
- AMD-V (SVM) instruction support alongside Intel VT-x
- Nested virtualization detection techniques
- Hardware breakpoint evasion strategies

**project-mcp:**
- Real-time file watching for automatic indexing
- Git integration for change tracking
- Code complexity metrics (cyclomatic complexity)
- Dependency graph visualization endpoint

**driver-re:**
- IDA Pro script generation for IOCTL analysis
- Ghidra integration for binary analysis
- Automated vulnerability scanning with PoC generation
- Binary diffing for patch analysis

### Integration Improvements

- Cross-server queries (e.g., "find drivers using MSR from ombra knowledge")
- Unified dashboard showing data from all three servers
- Shared vector embeddings across project-mcp and driver-re
- Real-time collaboration features for multi-developer teams

---

## Related Documentation

- [ombra_mcp_server README](ombra_mcp_server/README.md)
- [project-mcp-server README](project-mcp-server/README.md)
- [driver-re-mcp README](driver-re-mcp/README.md)
- [MCP Protocol Specification](https://modelcontextprotocol.io/)
- [Intel SDM Documentation](https://www.intel.com/sdm)
- [Microsoft Kernel-Mode Documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/)
