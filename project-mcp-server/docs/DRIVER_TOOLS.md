# Driver Analysis Tools Documentation

Integration between ombra-driver-scanner and project-mcp-server for driver analysis workflow.

## Overview

The driver analysis tools consume JSON output from `ombra-driver-scanner` and provide:
- Driver capability tracking
- Analysis queue management
- Automation script generation for `driver-re-mcp`
- Progress tracking and status updates

## Database Schema

The `drivers` table stores comprehensive driver analysis data:

```sql
CREATE TABLE drivers (
    id INTEGER PRIMARY KEY,

    -- Identification
    driver_path TEXT NOT NULL,
    driver_name TEXT NOT NULL,
    hash_sha256 TEXT UNIQUE NOT NULL,
    hash_md5 TEXT,

    -- Scanner Results
    capability_tier TEXT,      -- S, A, B, C, D
    final_score REAL,
    classification TEXT,       -- EXCELLENT, GOOD, FAIR, POOR

    -- Score Breakdown
    capability_score INTEGER,
    accessibility_score INTEGER,
    obscurity_score INTEGER,
    stealth_score INTEGER,

    -- Capability Flags
    has_physical_memory INTEGER,
    has_msr_access INTEGER,
    has_process_control INTEGER,
    has_module_loading INTEGER,
    has_cr_access INTEGER,
    has_port_io INTEGER,

    -- Analysis
    driver_family TEXT,
    dangerous_apis TEXT,        -- JSON array
    detected_ioctls TEXT,       -- JSON array

    -- Blocklists
    on_loldrivers INTEGER,
    on_msdbx INTEGER,

    -- Status
    analysis_status TEXT,       -- pending, in_progress, analyzed, failed
    priority INTEGER,           -- 1=highest, 5=lowest

    -- Timestamps
    scanned_at TEXT,
    analysis_started_at TEXT,
    analysis_completed_at TEXT
);
```

## Tools

### 1. import_driver_analysis

Import scanner JSON output into the database.

**Input:**
```json
{
    "json_path": "/path/to/scanner/output.json"
}
```

**Output:**
```json
{
    "success": true,
    "drivers_imported": 5,
    "driver_ids": [1, 2, 3, 4, 5],
    "errors": []
}
```

**Behavior:**
- Parses scanner JSON format from `ombra-driver-scanner`
- Extracts driver metadata, capabilities, and scores
- Creates or updates driver records based on SHA256 hash
- Logs import in audit trail
- Handles multiple drivers from batch scans

**Usage Example:**
```bash
mcp-cli call project-mcp/import_driver_analysis '{
    "json_path": "/tmp/scanner_output/ranked_candidates.json"
}'
```

### 2. get_driver_analysis

Get full driver analysis details by name or hash.

**Input:**
```json
{
    "driver_name_or_hash": "Ld9BoxSup.sys"
}
```

**Output:**
```json
{
    "driver": {
        "id": 1,
        "driver_name": "Ld9BoxSup.sys",
        "capability_tier": "A",
        "final_score": 84.67,
        "has_module_loading": true,
        "has_physical_memory": true,
        "dangerous_apis": ["MmMapIoSpace", "MmUnmapIoSpace", ...],
        "detected_ioctls": ["0xCD0100BA", "0xB821CD01", ...],
        "driver_family": "VirtualBox"
    },
    "analysis_tasks": [
        "load_driver",
        "analyze_imports",
        "trace_ldr_load_handler",
        "find_module_loading_ioctls",
        ...
    ],
    "related_drivers": [
        {"id": 2, "driver_name": "VBoxDrv.sys", "capability_tier": "S"}
    ],
    "mcp_script": "#!/usr/bin/env python3\n..."
}
```

**Behavior:**
- Searches by driver name or SHA256 hash
- Returns full driver record with parsed JSON fields
- Generates capability-specific analysis tasks
- Finds related drivers in same family
- Produces complete automation script for driver-re-mcp

### 3. list_drivers_by_tier

List drivers filtered by capability tier and score.

**Input:**
```json
{
    "tier": "S",
    "min_score": 85.0
}
```

**Output:**
```json
{
    "total": 3,
    "tier": "S",
    "drivers": [
        {
            "id": 1,
            "driver_name": "capcom.sys",
            "capability_tier": "S",
            "final_score": 95.2,
            "driver_family": "Capcom",
            "analysis_status": "pending",
            "priority": 1
        },
        ...
    ]
}
```

**Behavior:**
- Filters by tier (S, A, B, C, D) or "all"
- Applies minimum score threshold
- Sorts by score descending, then priority
- Parses JSON arrays (dangerous_apis, detected_ioctls)

**Tier Definitions:**
- **S (90-100)**: Elite drivers with exceptional capabilities
- **A (80-89)**: Excellent drivers with strong exploitation potential
- **B (70-79)**: Good drivers with useful capabilities
- **C (60-69)**: Fair drivers with limited use
- **D (<60)**: Poor drivers, minimal exploitation value

### 4. get_mcp_analysis_queue

Get priority-sorted analysis queue for MCP consumption.

**Input:**
```json
{}
```

**Output:**
```json
{
    "queue_size": 12,
    "tier_breakdown": {
        "S": 2,
        "A": 5,
        "B": 3,
        "C": 2,
        "D": 0
    },
    "queue": [
        {
            "rank": 1,
            "driver_id": 1,
            "driver_name": "capcom.sys",
            "capability_tier": "S",
            "final_score": 95.2,
            "priority": 1,
            "driver_family": "Capcom",
            "has_module_loading": true,
            "has_physical_memory": true,
            "analysis_status": "pending"
        },
        ...
    ]
}
```

**Behavior:**
- Returns only pending/in_progress drivers
- Sorts by priority (tier-based), then score
- Calculates tier distribution
- Adds rank to each driver
- Ideal for batch processing workflows

### 5. update_driver_status

Update driver analysis status and track progress.

**Input:**
```json
{
    "driver_id": 1,
    "status": "analyzed",
    "notes": "Completed Ghidra analysis, found 3 critical vulnerabilities"
}
```

**Output:**
```json
{
    "success": true,
    "driver_id": 1,
    "old_status": "in_progress",
    "new_status": "analyzed"
}
```

**Behavior:**
- Updates analysis_status field
- Sets timestamps (analysis_started_at, analysis_completed_at)
- Logs status change in audit trail
- Validates driver exists before update

**Status Values:**
- `pending`: Not yet analyzed
- `in_progress`: Currently being analyzed
- `analyzed`: Analysis complete
- `failed`: Analysis failed (errors encountered)

### 6. generate_analysis_script

Generate Python automation script for driver-re-mcp analysis.

**Input:**
```json
{
    "driver_id": 1
}
```

**Output:**
```json
{
    "success": true,
    "driver_name": "Ld9BoxSup.sys",
    "script": "#!/usr/bin/env python3\n\"\"\"Auto-generated MCP analysis script...",
    "script_path": "analyze_Ld9BoxSup.sys.py"
}
```

**Behavior:**
- Generates complete Python script using driver-re-mcp SDK
- Includes all dangerous APIs for call path tracing
- Decodes and registers detected IOCTLs
- Documents vulnerabilities based on capabilities
- Creates attack chains
- Generates final exploitation report

**Generated Script Features:**
- Async/await pattern for driver-re-mcp
- Loads driver into analysis framework
- Syncs with Ghidra project
- Traces call paths to dangerous APIs
- Registers IOCTLs for dispatch analysis
- Documents vulnerabilities (module loading, physical memory, MSR access)
- Creates BYOVD attack chain
- Generates markdown report

## Workflow Integration

### Complete Analysis Pipeline

```bash
# 1. Scan drivers with ombra-driver-scanner
cd /path/to/ombra-driver-scanner
./ombra-scanner /path/to/drivers --output scan_results.json

# 2. Import results into project-mcp
mcp-cli call project-mcp/import_driver_analysis '{
    "json_path": "/path/to/scan_results.json"
}'

# 3. Get analysis queue
mcp-cli call project-mcp/get_mcp_analysis_queue '{}'

# 4. Generate script for top priority driver
mcp-cli call project-mcp/generate_analysis_script '{
    "driver_id": 1
}' > analyze_driver.py

# 5. Run automation script with driver-re-mcp
chmod +x analyze_driver.py
./analyze_driver.py

# 6. Update status after analysis
mcp-cli call project-mcp/update_driver_status '{
    "driver_id": 1,
    "status": "analyzed",
    "notes": "Analysis complete"
}'
```

### Batch Processing

```bash
# Get all Tier S drivers
mcp-cli call project-mcp/list_drivers_by_tier '{
    "tier": "S",
    "min_score": 0
}'

# Generate scripts for entire queue
mcp-cli call project-mcp/get_mcp_analysis_queue '{}' | \
    jq -r '.queue[].driver_id' | \
    while read id; do
        mcp-cli call project-mcp/generate_analysis_script "{\"driver_id\": $id}" > "analyze_driver_$id.py"
    done
```

## Analysis Task Generation

The tools automatically generate capability-specific analysis tasks:

**Module Loading Capabilities:**
- trace_ldr_load_handler
- find_module_loading_ioctls
- analyze_symbol_resolution_path

**Physical Memory Capabilities:**
- trace_physical_memory_apis
- find_phys_read_write_ioctls
- analyze_mmmapiospace_usage

**MSR Access Capabilities:**
- trace_msr_apis
- find_msr_read_write_ioctls
- check_msr_validation

**Process Control Capabilities:**
- trace_process_apis
- analyze_callback_registration

**IOCTL Analysis:**
- decode_all_ioctls
- trace_dispatch_handler
- map_ioctl_to_capability

**Family-Specific Tasks:**
- VirtualBox/LDPlayer: analyze_vbox_ldr_protocol, find_validation_checks
- Document bypass requirements

## Priority Calculation

Priority is automatically calculated from capability tier:

| Tier | Priority | Score Range | Description |
|------|----------|-------------|-------------|
| S    | 1        | 90-100      | Highest priority - elite drivers |
| A    | 2        | 80-89       | High priority - excellent drivers |
| B    | 3        | 70-79       | Medium priority - good drivers |
| C    | 4        | 60-69       | Low priority - fair drivers |
| D    | 5        | <60         | Lowest priority - poor drivers |

## JSON Field Handling

The tools properly parse and serialize JSON fields:

**dangerous_apis** (array of strings):
```json
["MmMapIoSpace", "MmUnmapIoSpace", "ZwQuerySystemInformation"]
```

**detected_ioctls** (array of hex strings):
```json
["0xCD0100BA", "0xB821CD01", "0x9ED59149"]
```

## Audit Logging

All operations are logged in the audit_log table:

- Driver imports: entity_type="driver_import"
- Driver creation: entity_type="driver", action="create"
- Driver updates: entity_type="driver", action="update"
- Status changes: entity_type="driver", action="status_change"

## Error Handling

All tools return structured error responses:

```json
{
    "success": false,
    "error": "Driver not found: invalid_hash"
}
```

Import operations collect errors but continue processing:

```json
{
    "success": false,
    "drivers_imported": 3,
    "driver_ids": [1, 2, 3],
    "errors": [
        "Failed to import driver4.sys: missing hash field"
    ]
}
```

## Future Enhancements

Potential additions to driver analysis tools:

1. **Cross-reference with driver-re-mcp results**
   - Link scanner findings to detailed reverse engineering
   - Track analysis completeness

2. **Driver comparison tools**
   - Compare capabilities across driver families
   - Identify unique capabilities

3. **Automated reporting**
   - Generate capability matrices
   - Export analysis summaries

4. **Integration with Ghidra**
   - Auto-create Ghidra projects
   - Sync analysis annotations

5. **Vulnerability tracking**
   - Link to CVE database
   - Track exploitation attempts

## See Also

- [ombra-driver-scanner MCP Bridge](../../ombra-driver-scanner/docs/mcp_bridge.hpp)
- [Scanner JSON Format](../../ombra-driver-scanner/docs/output_format.md)
- driver-re-mcp documentation
