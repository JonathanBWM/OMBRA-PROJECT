# Scanner Integration Guide

The driver-re-mcp server now integrates with the ombra-driver-scanner to automate driver analysis workflows. This guide covers the new scanner intake tools and their usage patterns.

## Overview

The scanner integration enables:
- Batch import of scanner analysis results
- Priority-based analysis queuing
- Capability-based driver filtering
- Automatic tier and scoring metadata
- Seamless workflow from scanner to deep analysis

## New Tools

### 1. import_scanner_results

Import scanner JSON batch output, creating driver entries with all metadata.

**Schema:**
```json
{
  "json_path": "string (required)"
}
```

**Input Format:**
The scanner outputs `MCPBatchExport` JSON with this structure:
```json
{
  "scan_metadata": {
    "timestamp": "ISO-8601",
    "total_scanned": 127,
    "tier_s_count": 3,
    "tier_a_count": 8
  },
  "analysis_queue": [
    {
      "name": "driver.sys",
      "tier": "S|A|B|C|D",
      "score": 95.0,
      "priority": 1,
      "path": "/path/to/driver",
      "hash": "sha256",
      "hash_md5": "md5",
      "hash_sha1": "sha1",
      "family": "VirtualBox",
      "era": "modern|legacy|ancient"
    }
  ]
}
```

**Example Usage:**
```bash
mcp-cli call driver-re-mcp/import_scanner_results '{
  "json_path": "/path/to/scanner_output.json"
}'
```

**Response:**
```json
{
  "success": true,
  "imported": 11,
  "skipped": 0,
  "errors": [],
  "message": "Imported 11 drivers, skipped 0 duplicates"
}
```

**Behavior:**
- Creates driver entries with `analysis_status = 'scanner_imported'`
- Skips duplicates based on SHA256 hash
- Stores tier, score, priority in notes field
- Tags drivers with: `tier_{tier}`, `family_{family}`, `era_{era}`
- Returns count of imported/skipped drivers plus any errors

---

### 2. get_analysis_queue

Get priority-sorted queue for MCP analysis.

**Schema:**
```json
{
  "min_priority": 1,  // optional, default 1 (highest)
  "max_priority": 5   // optional, default 5 (lowest)
}
```

**Example Usage:**
```bash
# Get only highest priority (Tier S) drivers
mcp-cli call driver-re-mcp/get_analysis_queue '{
  "min_priority": 1,
  "max_priority": 1
}'

# Get all scanner imports
mcp-cli call driver-re-mcp/get_analysis_queue '{}'
```

**Response:**
```json
{
  "success": true,
  "count": 3,
  "queue": [
    {
      "driver_id": "uuid",
      "name": "ld9boxdrv.sys",
      "sha256": "hash",
      "tier": "S",
      "score": 95.0,
      "priority": 1,
      "tags": ["tier_S", "family_LDPlayer/Ld9Box", "era_modern"],
      "imported_at": "2025-12-28T10:30:00Z"
    }
  ]
}
```

**Behavior:**
- Filters drivers with `analysis_status = 'scanner_imported'`
- Parses priority/tier/score from notes field
- Sorts by priority ascending, then score descending
- Returns ready-to-analyze queue

**Use Case:**
Fetch the next batch of high-priority drivers for manual or automated analysis.

---

### 3. import_single_driver_analysis

Import a single MCPAnalysisRequest JSON object.

**Schema:**
```json
{
  "json_data": "string (required) - JSON string of MCPAnalysisRequest"
}
```

**Input Format:**
Full `MCPAnalysisRequest` structure from scanner:
```json
{
  "driver": {
    "path": "/path",
    "name": "driver.sys",
    "hash_sha256": "sha256",
    "hash_md5": "md5",
    "hash_sha1": "sha1"
  },
  "scanner_results": {
    "capability_tier": "S",
    "final_score": 95.0,
    "classification": "Hypervisor-Ready Module Loader",
    "driver_family": "LDPlayer/Ld9Box",
    "priority": 1
  },
  "capabilities": {
    "module_loading": true,
    "physical_memory": true,
    "msr_access": true,
    "process_control": false
  },
  "dangerous_apis": [
    "ZwLoadDriver",
    "MmMapIoSpace"
  ],
  "detected_ioctls": [
    "0x222004"
  ],
  "age_analysis": {
    "era": "modern",
    "age_score": 95,
    "warning": "High-value target"
  }
}
```

**Example Usage:**
```bash
mcp-cli call driver-re-mcp/import_single_driver_analysis - <<'EOF'
{
  "json_data": "{\"driver\": {...}, \"scanner_results\": {...}}"
}
EOF
```

**Response:**
```json
{
  "success": true,
  "driver_id": "uuid",
  "message": "Driver ld9boxdrv.sys imported successfully",
  "tier": "S",
  "score": 95.0,
  "priority": 1
}
```

**Behavior:**
- Creates driver entry with full scanner metadata
- Tags with tier, family, era, and capabilities (`cap_module_loading`, etc.)
- Stores dangerous APIs and IOCTL count in notes
- Rejects duplicates based on SHA256
- Returns driver_id for immediate follow-up analysis

**Use Case:**
Import a single high-value driver with full scanner context for immediate deep analysis.

---

### 4. set_driver_tier_info

Update driver with scanner scoring info.

**Schema:**
```json
{
  "driver_id": "string (required)",
  "tier": "S|A|B|C|D (required)",
  "score": "number (required)",
  "classification": "string (required)",
  "synergies": "string (optional)"
}
```

**Example Usage:**
```bash
mcp-cli call driver-re-mcp/set_driver_tier_info '{
  "driver_id": "uuid",
  "tier": "S",
  "score": 95.0,
  "classification": "Hypervisor-Ready Module Loader",
  "synergies": "Module loading + Physical memory + MSR access"
}'
```

**Response:**
```json
{
  "success": true,
  "message": "Driver tier info updated: S (95.0)"
}
```

**Behavior:**
- Updates existing driver's tier/score/classification
- Replaces tier tag while preserving other tags
- Updates notes while preserving non-tier fields
- Useful for re-scoring after manual analysis

**Use Case:**
Update a driver's tier after manual review reveals additional capabilities or after rescoring algorithm changes.

---

### 5. get_drivers_by_capability

List drivers with specific capability.

**Schema:**
```json
{
  "capability": "module_loading|physical_memory|msr_access|process_control (required)"
}
```

**Example Usage:**
```bash
# Find all drivers with module loading capability
mcp-cli call driver-re-mcp/get_drivers_by_capability '{
  "capability": "module_loading"
}'
```

**Response:**
```json
{
  "success": true,
  "capability": "module_loading",
  "count": 5,
  "drivers": [
    {
      "driver_id": "uuid",
      "name": "ld9boxdrv.sys",
      "sha256": "hash",
      "tier": "S",
      "score": 95.0,
      "tags": ["tier_S", "cap_module_loading", "cap_physical_memory"],
      "status": "scanner_imported",
      "imported_at": "2025-12-28T10:30:00Z"
    }
  ]
}
```

**Behavior:**
- Searches for `cap_{capability}` tag in driver tags
- Parses tier/score from notes
- Sorts by score descending
- Returns all matching drivers regardless of analysis status

**Use Case:**
Find all drivers with a specific capability for comparative analysis or exploit chain building.

---

## Workflow Examples

### Batch Import and Analysis

```bash
# 1. Import scanner batch results
mcp-cli call driver-re-mcp/import_scanner_results '{
  "json_path": "/scanner/output/batch_2025-12-28.json"
}'

# 2. Get high-priority analysis queue (Tier S only)
mcp-cli call driver-re-mcp/get_analysis_queue '{
  "min_priority": 1,
  "max_priority": 1
}'

# 3. For each driver in queue, start analysis
# (driver_id from queue response)
mcp-cli call driver-re-mcp/update_driver_status '{
  "driver_id": "uuid",
  "status": "in_progress"
}'

# 4. After analysis, update tier if needed
mcp-cli call driver-re-mcp/set_driver_tier_info '{
  "driver_id": "uuid",
  "tier": "S",
  "score": 98.0,
  "classification": "Confirmed BYOVD with Module Loading",
  "synergies": "Module loading + MSR access + Zero validation"
}'
```

### Capability-Based Filtering

```bash
# Find all drivers with physical memory access
mcp-cli call driver-re-mcp/get_drivers_by_capability '{
  "capability": "physical_memory"
}'

# Find all drivers with MSR access
mcp-cli call driver-re-mcp/get_drivers_by_capability '{
  "capability": "msr_access"
}'

# Cross-reference for drivers with BOTH capabilities
# (parse results and find intersection)
```

### Single High-Value Target Import

```bash
# Import specific driver with full context
mcp-cli call driver-re-mcp/import_single_driver_analysis - <<'EOF'
{
  "json_data": "{ ... full MCPAnalysisRequest JSON ... }"
}
EOF

# Immediately get driver for analysis
mcp-cli call driver-re-mcp/get_driver '{
  "sha256": "driver_sha256_hash"
}'
```

---

## Data Flow Architecture

```
┌─────────────────────────────┐
│  ombra-driver-scanner       │
│  (C++ Batch Scanner)        │
└──────────┬──────────────────┘
           │ Outputs
           │ MCPBatchExport JSON
           ▼
┌─────────────────────────────┐
│  import_scanner_results     │
│  (Batch Import Tool)        │
└──────────┬──────────────────┘
           │ Creates
           │ driver entries
           ▼
┌─────────────────────────────┐
│  SQLite Database            │
│  (drivers table)            │
│  - analysis_status:         │
│    'scanner_imported'       │
│  - tags: tier, family, caps │
│  - notes: tier/score/apis   │
└──────────┬──────────────────┘
           │
           ├─► get_analysis_queue
           │   (Priority-sorted queue)
           │
           ├─► get_drivers_by_capability
           │   (Capability filtering)
           │
           └─► set_driver_tier_info
               (Update after analysis)
```

---

## Tag Schema

Tags added during scanner import:

| Tag Pattern | Example | Meaning |
|-------------|---------|---------|
| `tier_{tier}` | `tier_S` | Capability tier (S/A/B/C/D) |
| `family_{family}` | `family_VirtualBox` | Driver family |
| `era_{era}` | `era_modern` | Age classification |
| `cap_{capability}` | `cap_module_loading` | Specific capability detected |

**Querying by Tag:**
```sql
SELECT * FROM drivers WHERE tags LIKE '%tier_S%';
SELECT * FROM drivers WHERE tags LIKE '%cap_module_loading%';
```

---

## Notes Field Format

Notes field stores structured data separated by ` | `:

```
Tier: S | Score: 95.0 | Priority: 1 | Classification: Hypervisor-Ready Module Loader | Family: LDPlayer/Ld9Box | Era: modern | Warning: High-value target | Dangerous APIs: ZwLoadDriver, MmMapIoSpace, __readmsr | IOCTLs detected: 4
```

**Parsing:**
```python
notes = driver['notes']
parts = notes.split(' | ')
tier = parts[0].split(': ')[1]  # "S"
score = float(parts[1].split(': ')[1])  # 95.0
```

---

## Database Schema Extensions

The scanner integration uses existing `drivers` table fields:

- `analysis_status`: Set to `'scanner_imported'` for scanner imports
- `tags`: JSON array with tier, family, era, capability tags
- `notes`: Structured text with tier/score/classification/APIs
- `sha256`: Used for duplicate detection

No schema changes required - fully backward compatible.

---

## Error Handling

All tools return structured error responses:

```json
{
  "success": false,
  "error": "Driver already exists",
  "driver_id": "existing-uuid"
}
```

**Common Errors:**

| Error | Cause | Solution |
|-------|-------|----------|
| `File not found` | Invalid json_path | Check file path |
| `Invalid JSON` | Malformed JSON | Validate JSON syntax |
| `Missing analysis_queue` | Wrong JSON format | Use MCPBatchExport format |
| `Driver already exists` | Duplicate SHA256 | Use get_driver to retrieve existing |
| `Driver not found` | Invalid driver_id | Verify UUID |

---

## Integration with Existing Tools

Scanner imports create standard driver entries compatible with all existing tools:

```bash
# After scanner import, use existing tools
mcp-cli call driver-re-mcp/add_ioctl '{
  "driver_id": "uuid",
  "name": "IOCTL_LOAD_MODULE",
  "code": 2236420
}'

mcp-cli call driver-re-mcp/add_vulnerability '{
  "driver_id": "uuid",
  "title": "Arbitrary Module Loading",
  "vulnerability_class": "code_execution",
  "severity": "critical"
}'
```

Scanner imports are first-class driver entries - no special handling required.

---

## Performance Considerations

**Batch Import:**
- Processes drivers sequentially
- SHA256 lookup per driver (indexed)
- Typical: 100 drivers/sec on SSD

**Queue Retrieval:**
- Single query with status filter
- Notes parsing in Python (fast)
- Typical: <10ms for 1000 drivers

**Capability Search:**
- Tag LIKE query (not indexed on pattern)
- Consider full-text index for large databases
- Typical: <50ms for 10k drivers

---

## Future Enhancements

Planned additions:
- Auto-import IOCTLs from `detected_ioctls` field
- Auto-create import entries from `dangerous_apis` field
- Auto-generate analysis tasks from `suggested_tasks` field
- Webhook support for scanner completion notifications
- Real-time queue monitoring dashboard

---

## Examples Directory

See `/examples/` for sample JSON files:
- `scanner_batch_example.json` - MCPBatchExport format
- `scanner_single_example.json` - MCPAnalysisRequest format

Test with these examples to validate integration.
