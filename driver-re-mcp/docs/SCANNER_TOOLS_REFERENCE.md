# Scanner Integration Tools - Quick Reference

Five new MCP tools for ombra-driver-scanner integration.

## Tools

### 1. `import_scanner_results`
**Purpose:** Batch import scanner JSON output
**Input:** Path to MCPBatchExport JSON file
**Output:** Import count, skip count, errors
**Status:** Sets `analysis_status = 'scanner_imported'`

```bash
mcp-cli call driver-re-mcp/import_scanner_results '{"json_path": "/path/to/batch.json"}'
```

---

### 2. `get_analysis_queue`
**Purpose:** Get priority-sorted analysis queue
**Input:** Optional min/max priority (1-5)
**Output:** Array of drivers sorted by priority/score
**Filter:** Only returns `scanner_imported` status drivers

```bash
mcp-cli call driver-re-mcp/get_analysis_queue '{"min_priority": 1, "max_priority": 2}'
```

---

### 3. `import_single_driver_analysis`
**Purpose:** Import single MCPAnalysisRequest
**Input:** Full MCPAnalysisRequest JSON string
**Output:** driver_id, tier, score, priority
**Tags:** Adds tier, family, era, cap_* tags

```bash
mcp-cli call driver-re-mcp/import_single_driver_analysis '{"json_data": "{...}"}'
```

---

### 4. `set_driver_tier_info`
**Purpose:** Update driver tier/score/classification
**Input:** driver_id, tier (S/A/B/C/D), score, classification
**Output:** Success confirmation
**Update:** Replaces tier tag and notes fields

```bash
mcp-cli call driver-re-mcp/set_driver_tier_info '{
  "driver_id": "uuid",
  "tier": "S",
  "score": 95.0,
  "classification": "Hypervisor-Ready Module Loader"
}'
```

---

### 5. `get_drivers_by_capability`
**Purpose:** Filter drivers by capability
**Input:** Capability name (module_loading, physical_memory, msr_access, process_control)
**Output:** Array of drivers with that capability, sorted by score
**Search:** Looks for `cap_{capability}` tag

```bash
mcp-cli call driver-re-mcp/get_drivers_by_capability '{"capability": "module_loading"}'
```

---

## Tag Schema

| Tag | Example | Meaning |
|-----|---------|---------|
| `tier_{tier}` | `tier_S` | Capability tier |
| `family_{family}` | `family_VirtualBox` | Driver family |
| `era_{era}` | `era_modern` | Age classification |
| `cap_{capability}` | `cap_module_loading` | Detected capability |

---

## Notes Format

```
Tier: S | Score: 95.0 | Priority: 1 | Classification: X | Family: Y | Era: Z | Dangerous APIs: [...] | IOCTLs detected: N
```

Parse with: `notes.split(' | ')`

---

## Workflow

```
Scanner Output → import_scanner_results → SQLite DB
                                              ↓
                                   get_analysis_queue
                                              ↓
                                   (analyze drivers)
                                              ↓
                                   set_driver_tier_info
```

---

## Files Modified

- `/Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp/src/driver_re_mcp/tools/driver_tools.py` - Added 5 functions
- `/Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp/src/driver_re_mcp/server.py` - Registered 5 tools

## Examples

- `/Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp/examples/scanner_batch_example.json`
- `/Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp/examples/scanner_single_example.json`

## Full Documentation

See `/Users/jonathanmcclintock/PROJECT-OMBRA/driver-re-mcp/docs/scanner_integration.md`
