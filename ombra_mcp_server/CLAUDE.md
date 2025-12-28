# Ombra MCP Server - Developer Context

This CLAUDE.md provides MCP server-specific context. See root `/CLAUDE.MD` for project-wide conventions, ENI identity, and full tool reference.

## Quick Stats

| Metric | Value |
|--------|-------|
| **Server Version** | v2.2.0 |
| **Total Tools** | 152 |
| **Framework** | Python MCP SDK (`mcp>=1.0.0`) |
| **Python** | 3.10+ |
| **Entry Point** | `src/ombra_mcp/server.py` (2,337 LOC) |
| **Build** | Hatchling (`pip install -e .`) |

---

## Directory Structure

```
ombra_mcp_server/
├── src/ombra_mcp/
│   ├── server.py              # Main MCP server (2,337 LOC)
│   ├── __init__.py
│   ├── tools/                 # Tool modules (25 files)
│   │   ├── __init__.py        # Exports all tool modules
│   │   ├── sdm_query.py       # Intel SDM queries (~16KB)
│   │   ├── code_generator.py  # Code generation (~27KB)
│   │   ├── vmcs_validator.py  # VMCS validation (~25KB)
│   │   ├── stealth.py         # Stealth auditing (~24KB)
│   │   ├── timing_simulator.py # Timing simulation (~23KB)
│   │   ├── anticheat_intel.py # Anti-cheat intel (~22KB)
│   │   ├── binary_scanner.py  # Binary scanning (~18KB)
│   │   ├── driver_mapper.py   # PE mapping guides (~21KB)
│   │   ├── hvci_bypass.py     # HVCI bypass (~17KB)
│   │   ├── project_brain.py   # Project state (~24KB)
│   │   ├── concepts.py        # Concept verification (~13KB)
│   │   ├── semantic_search.py # ChromaDB search (~25KB)
│   │   ├── vergilius.py       # Kernel structures (~12KB)
│   │   ├── mslearn_db.py      # MS Learn docs (~16KB)
│   │   ├── byovd.py           # BYOVD exploitation (~10KB)
│   │   ├── byovd_db.py        # BYOVD database (~13KB)
│   │   ├── anticheat_db.py    # Anti-cheat DB (~10KB)
│   │   ├── evasion_db.py      # Evasion techniques (~12KB)
│   │   ├── driver_re_db.py    # Driver RE shared conn (~8KB)
│   │   ├── driver_tools.py    # Driver analysis (~9KB)
│   │   ├── ioctl_tools.py     # IOCTL analysis (~9KB)
│   │   ├── import_tools.py    # Import/API analysis (~8KB)
│   │   └── export_tools.py    # Export analysis (~5KB)
│   └── data/                  # Databases & static data
│       ├── intel_sdm.db       # Intel SDM reference (225KB)
│       ├── project_brain.db   # Project state (14MB)
│       ├── vergilius.db       # Kernel structures (3.2MB)
│       ├── mslearn_reference.db # MS Learn docs (905KB)
│       ├── anticheat_intel.db # Anti-cheat methods (118KB)
│       ├── evasion_techniques.db # Bypass chains (118KB)
│       ├── byovd_drivers.db   # Vulnerable drivers (61KB)
│       ├── driver_re.db       # Driver RE analysis (168KB)
│       ├── chroma/            # ChromaDB vector store
│       ├── ld9boxsup_ioctls.json # BYOVD IOCTLs
│       ├── detection_vectors.json # Detection methods
│       ├── signatures.json    # Binary signatures
│       └── vmcs_fields.h      # Generated VMCS header
├── scripts/                   # Data ingestion scripts
│   ├── scrape_vergilius.py    # Windows struct scraper
│   ├── ingest_mslearn.py      # MS Learn ingestion
│   ├── init_*_db.py           # Database initializers
│   └── migrate_gotchas.py     # Migration utilities
├── docs/                      # Server documentation
└── pyproject.toml             # Package configuration
```

---

## Tool Categories (152 Total)

| Prefix | Count | Purpose | Key Module |
|--------|-------|---------|------------|
| `get_*` | 58 | Data retrieval | sdm_query.py, project_brain.py |
| `generate_*` | 16 | Code generation | code_generator.py |
| `dre_*` | 16 | Driver RE integration | driver_tools.py, ioctl_tools.py |
| `add_*` | 10 | Data insertion | project_brain.py |
| `mslearn_*` | 10 | MS Learn docs | mslearn_db.py |
| `validate_*` | 6 | Validation | vmcs_validator.py |
| `audit_*` | 4 | Security auditing | stealth.py |
| `search_*` | 6 | Search tools | semantic_search.py |
| Other | 26 | Misc utilities | various |

---

## Database Schema Reference

### intel_sdm.db (Intel SDM Reference)

| Table | Rows | Key Fields |
|-------|------|------------|
| vmcs_fields | 167 | encoding, name, category, width, description |
| exit_reasons | 66 | reason_number, name, qualification_format |
| msrs | 35 | address, name, description, vmx_related |
| exceptions | 20 | vector, name, has_error_code |

### project_brain.db (Project State - 14MB)

| Table | Purpose |
|-------|---------|
| decisions | Architecture/implementation decisions |
| gotchas | Bugs, pitfalls, lessons learned |
| sessions | Context for each work session |
| components | Implementation status tracking |
| findings | Code audit findings |
| concepts | VMX concepts to implement |

### vergilius.db (Windows Kernel Structures)

| Table | Rows | Purpose |
|-------|------|---------|
| type_definitions | 350 | EPROCESS, KTHREAD, etc. |
| fields | 17,122 | Structure member offsets |
| critical_offsets | 211 | Important offsets by version |
| windows_versions | 15+ | NT 6.0 through 11.0 |

### driver_re.db (Driver Analysis)

| Table | Purpose |
|-------|---------|
| drivers | PE metadata, hashes, status |
| imports | IAT entries with danger classification |
| exports | EAT entries with VBox pattern detection |
| ioctls | IOCTL handlers with vulnerabilities |
| xrefs | Cross-references between functions |
| sections | PE sections with entropy |

---

## Semantic Search (ChromaDB)

```
data/chroma/
├── chroma.sqlite3             # 8.5MB
└── (7 collections)
```

| Collection | Documents | Purpose |
|------------|-----------|---------|
| intel_sdm | ~200 | SDM content chunks |
| anticheat_intel | ~100 | Anti-cheat methods |
| evasion_techniques | ~50 | Bypass techniques |
| byovd_drivers | ~30 | Vulnerable drivers |
| mslearn_reference | ~400 | MS Learn pages |
| project_brain | ~300 | Decisions/gotchas |
| vergilius | ~170 | Kernel structures |

**Embedding Model**: all-MiniLM-L6-v2 (384 dimensions, ONNX runtime)

---

## Adding New Tools

### 1. Create Handler Function

In appropriate `tools/xxx.py`:
```python
async def my_new_tool(param1: str, param2: int = 10) -> dict:
    """Tool description for documentation."""
    # Implementation
    return {"result": data}
```

### 2. Register in server.py

Add to `TOOLS` list:
```python
Tool(
    name="my_new_tool",
    description="Description shown in MCP discovery",
    inputSchema={
        "type": "object",
        "properties": {
            "param1": {"type": "string", "description": "First param"},
            "param2": {"type": "integer", "default": 10}
        },
        "required": ["param1"]
    }
)
```

Add to `TOOL_HANDLERS` dict:
```python
TOOL_HANDLERS = {
    ...
    "my_new_tool": my_module.my_new_tool,
}
```

### 3. Export from tools/__init__.py

```python
from . import my_module
```

---

## DANGER ZONES

### Critical Files

| File | Risk | Notes |
|------|------|-------|
| `server.py` | **CRITICAL** | 152 tool registrations - typos break discovery |
| `tools/__init__.py` | **HIGH** | Must export all tool modules |
| `intel_sdm.db` | **HIGH** | Read-only - regenerate with scripts if needed |
| `project_brain.db` | **MEDIUM** | 14MB of project history - back up before changes |

### Common Mistakes

1. **Tool not appearing**: Check TOOLS list AND TOOL_HANDLERS dict
2. **Schema mismatch**: inputSchema must match function signature exactly
3. **Import errors**: Module must be exported from `tools/__init__.py`
4. **Database locks**: ChromaDB doesn't support concurrent writes

---

## Running the Server

### Development
```bash
cd ombra_mcp_server
pip install -e .
python -m ombra_mcp.server
```

### Via MCP Client
Configured in `.mcp.json`:
```json
{
  "mcpServers": {
    "ombra": {
      "command": "python",
      "args": ["-m", "ombra_mcp.server"],
      "cwd": "/path/to/ombra_mcp_server"
    }
  }
}
```

---

## Testing Tools

```bash
# List all tools
mcp-cli tools ombra

# Get tool schema
mcp-cli info ombra/<tool_name>

# Call a tool
mcp-cli call ombra/vmcs_field_complete '{"field_name": "GUEST_RIP"}'

# Count registered tools
python3 -c "from src.ombra_mcp import server; print(len(server.TOOLS), len(server.TOOL_HANDLERS))"
```

---

## Key Tool Modules

### sdm_query.py
Intel SDM queries - VMCS fields, exit reasons, MSRs, exceptions.
```python
# Tools: vmcs_field_complete, exit_reason_complete, get_msr_info, ask_sdm
```

### code_generator.py
Generates hypervisor code from templates.
```python
# Tools: generate_vmcs_setup, generate_exit_handler, generate_ept_setup, generate_asm_stub
```

### stealth.py
Stealth auditing and detection analysis.
```python
# Tools: audit_stealth, get_detection_vectors, generate_timing_compensation, generate_cpuid_spoofing
```

### project_brain.py
Persistent project state and memory.
```python
# Tools: add_decision, add_gotcha, get_findings, get_project_status, save_session_context
```

### semantic_search.py
Vector search across all databases.
```python
# Tools: semantic_search, rebuild_semantic_index, get_semantic_index_stats
```

---

## Database Connections

Each tool module manages its own SQLite connection:
```python
import sqlite3
from pathlib import Path

DATA_DIR = Path(__file__).parent.parent / "data"

def get_conn():
    conn = sqlite3.connect(DATA_DIR / "database.db")
    conn.row_factory = sqlite3.Row
    return conn
```

**Note**: No connection pooling - each call opens fresh connection. Consider adding pooling for high-volume usage.

---

## ChromaDB Usage

```python
from chromadb import PersistentClient
from chromadb.utils.embedding_functions import DefaultEmbeddingFunction

DATA_DIR = Path(__file__).parent.parent / "data"

client = PersistentClient(path=str(DATA_DIR / "chroma"))
ef = DefaultEmbeddingFunction()  # all-MiniLM-L6-v2 via ONNX

collection = client.get_or_create_collection("intel_sdm", embedding_function=ef)
results = collection.query(query_texts=["EPT violation handling"], n_results=5)
```

---

## Data Ingestion Scripts

Located in `scripts/`:

| Script | Purpose |
|--------|---------|
| `init_vergilius_db.py` | Create Vergilius schema |
| `scrape_vergilius.py` | Scrape Windows structures |
| `init_mslearn_db.py` | Create MS Learn schema |
| `ingest_mslearn.py` | Ingest MS Learn pages |
| `create_new_databases.py` | Create all specialized DBs |
| `migrate_gotchas.py` | Migrate data between DBs |

Run order for fresh setup:
```bash
cd ombra_mcp_server
python scripts/init_vergilius_db.py
python scripts/scrape_vergilius.py
python scripts/init_mslearn_db.py
python scripts/ingest_mslearn.py
```

---

## When to Use OmbraMCP (MANDATORY CHECKLIST)

**ALWAYS use OmbraMCP when:**
- Writing or modifying VMCS setup code → `vmcs_field_complete`, `validate_vmcs_setup`
- Implementing exit handlers → `exit_reason_complete`, `generate_exit_handler`
- Working on EPT/memory virtualization → `generate_ept_setup`, `get_ept_structures`
- Adding MSR virtualization → `get_msr_info`, `generate_msr_bitmap_setup`
- Implementing stealth features → `get_detection_vectors`, `audit_stealth`, `generate_timing_compensation`
- Debugging VMX issues → `decode_vmx_error`, `decode_exit_qualification`
- Auditing code for anti-cheat detection → `get_anticheat_intel`, `check_evasion_coverage`
- Using BYOVD/Ld9BoxSup.sys → `ld9boxsup_ioctl_guide`, `generate_driver_wrapper`
- Manual driver mapping → `get_pe_parsing_guide`, `generate_mapping_checklist`
- HVCI bypass work → `get_zerohvci_architecture`, `get_hypercall_protocol`

---

## MCP Tool Categories (Complete Reference)

| Category | Key Tools | Use For |
|----------|-----------|---------|
| **Intel SDM** | `ask_sdm`, `vmcs_field_complete`, `exit_reason_complete`, `get_msr_info`, `get_exception_info`, `get_vmx_control_bits` | Any VMX specification questions |
| **Code Generation** | `generate_vmcs_setup`, `generate_exit_handler`, `generate_ept_setup`, `generate_asm_stub`, `generate_msr_bitmap_setup` | Creating hypervisor components |
| **Validation** | `validate_vmcs_setup`, `get_vmcs_checklist`, `validate_driver_binary`, `generate_mapping_checklist` | Pre-flight checks before testing |
| **Stealth** | `get_detection_vectors`, `audit_stealth`, `generate_cpuid_spoofing`, `generate_timing_compensation` | Anti-detection work |
| **Anti-Cheat Intel** | `get_anticheat_intel`, `get_timing_requirements`, `check_evasion_coverage`, `get_timing_thresholds_db`, `get_signatures` | Evasion strategy |
| **BYOVD** | `ld9boxsup_ioctl_guide`, `generate_driver_wrapper`, `generate_hypervisor_loader` | Vulnerable driver exploitation |
| **Driver Mapper** | `get_pe_parsing_guide`, `get_relocation_guide`, `get_cleanup_guide`, `get_import_resolution_guide`, `get_memory_allocation_guide` | Manual PE mapping |
| **HVCI Bypass** | `get_zerohvci_architecture`, `get_hypercall_protocol`, `get_phase_implementation_guide`, `get_hyperv_hijack_concepts` | Runtime Hyper-V hijacking |
| **Project Brain** | `get_project_status`, `get_findings`, `add_decision`, `add_gotcha`, `get_suggestions`, `get_priority_work` | Project state tracking |
| **Driver RE** | `dre_add_driver`, `dre_find_dangerous_apis`, `dre_get_exports`, `dre_document_export`, `dre_get_vulnerable_ioctls` | Driver analysis (Dec 2025) |
| **Vergilius** | `get_structure`, `get_field_offset`, `compare_versions`, `generate_offsets_header`, `get_hypervisor_offsets` | Windows kernel structures |
| **MS Learn** | `mslearn_search`, `mslearn_api`, `mslearn_concept`, `mslearn_cross_reference`, `mslearn_annotate_concept` | Microsoft documentation |
| **Semantic** | `semantic_search`, `rebuild_semantic_index`, `get_semantic_index_stats` | Cross-database vector search |

---

## Paranoia-First MCP Philosophy (CRITICAL)

**CRITICAL**: The MCP exists to WARN about detection vectors, not to validate assumptions. Never dismiss a potential detection risk without querying the MCP first.

### The Paranoia Protocol

1. **Never assume safety** - If you think something "should be fine," query the MCP to verify
2. **Query before implementation** - Not after. Ask `get_detection_vectors`, `search_detections`, `get_anticheat_intel` BEFORE writing code
3. **Search semantically** - Use `semantic_search` with creative queries: "EAC memory scan", "page fault timing", "BigPool detection", "PML4E enumeration"
4. **Layer your queries** - One tool may miss what another catches. Query multiple: `get_detection_vectors` + `search_detections` + `get_anticheat_intel`
5. **Document gaps** - If MCP doesn't have intel on a vector, add it with `add_detection_method` or `add_gotcha`

### Example: Memory Allocation Paranoia

```bash
# WRONG (dismissive)
"A 10MB payload will map correctly without detection"

# RIGHT (paranoid)
# 1. Query for size-based detection
mcp-cli call ombra/search_detections '{"query": "allocation size threshold"}'
mcp-cli call ombra/semantic_search '{"query": "BigPool large allocation detection"}'

# 2. Query for behavioral detection
mcp-cli call ombra/get_detection_vectors '{"category": "memory"}'
mcp-cli call ombra/search_detections '{"query": "page fault pattern timing"}'

# 3. Query for specific anti-cheat methods
mcp-cli call ombra/get_anticheat_intel '{"anticheat": "EAC"}'
mcp-cli call ombra/get_detection_method_detail '{"method_id": "pml4e_scan"}'

# 4. Only then implement with mitigations
```

### Red Flags (These Thoughts Mean STOP and Query)

| Thought | Required Action |
|---------|-----------------|
| "This should be fine" | Query `get_detection_vectors` |
| "Anti-cheats don't check this" | Query `get_anticheat_intel` |
| "It's just kernel memory" | Query `semantic_search` for "kernel memory detection" |
| "The size doesn't matter" | Query `search_detections` for "size threshold" |
| "EPT handles this" | Query `get_detection_vectors` for EPT-specific vectors |
| "Timing isn't an issue" | Query `get_timing_thresholds` |

### Known Critical Detection Vectors (Query These)

| Vector | Risk | Query |
|--------|------|-------|
| PML4E Scan | **CRITICAL/CONTINUOUS** | `search_detections '{"query": "PML4E"}'` |
| BigPool Scan | HIGH (~20min periodic) | `search_detections '{"query": "BigPool"}'` |
| Working Set Query | HIGH | `search_detections '{"query": "SharedCount working set"}'` |
| KiPageFault Heuristics | HIGH/CONTINUOUS | `search_detections '{"query": "page fault timing"}'` |
| PFN Database Differential | HIGH | `search_detections '{"query": "PFN differential"}'` |
| CPUID Timing | MEDIUM | `get_timing_requirements '{"context": "CPUID"}'` |
| TSC Delta | MEDIUM | `get_timing_thresholds '{"anticheat": "EAC"}'` |

### Implementation Checklist (Before Writing Stealth Code)

1. [ ] Queried `get_detection_vectors` for relevant category
2. [ ] Searched semantically for edge case detection methods
3. [ ] Checked `get_anticheat_intel` for EAC/BE/Vanguard specifics
4. [ ] Verified CPU capability requirements (execute-only EPT, etc.)
5. [ ] Checked timing thresholds for any timed operations
6. [ ] Documented mitigations with `add_decision` or `add_gotcha`

**The Rule**: If the MCP has intelligence on a detection vector, use it. If it doesn't, add the intelligence after you research it. Never fly blind.

---

## Mandatory Checks (Pre-Implementation & Pre-Commit)

### Before implementing ANY hypervisor feature:
1. **Query the SDM** - Use `ask_sdm` or specific field/exit tools to get authoritative specs
2. **Generate skeleton** - Use code generation tools as starting points
3. **Validate config** - Run `validate_vmcs_setup` before testing
4. **Audit stealth** - Run `audit_stealth` on new code to catch signatures
5. **Check anti-cheat coverage** - Use `check_evasion_coverage` to identify gaps

### Before committing hypervisor code:
1. Run `audit_stealth` on all modified files
2. Check `get_anticheat_intel` if adding new exit handlers
3. Document decisions with `add_decision`
4. Track solved bugs with `add_gotcha`

---

## Usage Patterns

### Basic Tool Call (After Schema Check)
```bash
# ALWAYS check schema first
mcp-cli info ombra/vmcs_field_complete

# Then call the tool
mcp-cli call ombra/vmcs_field_complete '{"field_name": "GUEST_RIP"}'
```

### Stealth Audit Workflow
```bash
# 1. Get all detection vectors for category
mcp-cli call ombra/get_detection_vectors '{"category": "memory"}'

# 2. Audit specific code file
mcp-cli call ombra/audit_stealth '{"code": "$(cat hypervisor/ept.c)"}'

# 3. Check anti-cheat coverage
mcp-cli call ombra/check_evasion_coverage '{"component": "EPT"}'

# 4. Get timing requirements
mcp-cli call ombra/get_timing_requirements '{"context": "EPT violation handler"}'
```

### VMCS Development Workflow
```bash
# 1. Query field details
mcp-cli call ombra/vmcs_field_complete '{"field_name": "GUEST_CR3"}'

# 2. Generate setup code skeleton
mcp-cli call ombra/generate_vmcs_setup '{"stealth": true}'

# 3. Validate before testing
mcp-cli call ombra/validate_vmcs_setup '{"vmcs_config": {...}}'

# 4. Get VMX control bits
mcp-cli call ombra/get_vmx_control_bits '{"control": "PRIMARY_PROC_BASED"}'
```

### Exit Handler Development
```bash
# 1. Get complete exit reason details
mcp-cli call ombra/exit_reason_complete '{"reason": 10}'

# 2. Generate handler skeleton
mcp-cli call ombra/generate_exit_handler '{"reason": 10, "stealth": true}'

# 3. Check anti-cheat intel for this exit
mcp-cli call ombra/get_anticheat_intel '{"anticheat": "EAC"}'

# 4. Simulate timing overhead
mcp-cli call ombra/simulate_handler_timing '{"handler_type": "CPUID", "instructions": 50}'
```

---

## Database Reference (Pre-Populated Data)

The MCP server has pre-populated databases with verified data:

| Database | Records | Purpose |
|----------|---------|---------|
| **intel_sdm.db** | 167 VMCS fields, 66 exit reasons, 35 MSRs, 20 exceptions | Intel SDM reference |
| **project_brain.db** | Project state, findings, decisions, gotchas | Persistent memory |
| **anticheat_intel.db** | Detection methods, bypasses, timing thresholds | Anti-cheat intel |
| **evasion_techniques.db** | 33 techniques, bypass chains, cleanup procedures | Stealth methods |
| **byovd_drivers.db** | Vulnerable drivers, IOCTLs, blocklist status | BYOVD exploitation |
| **vergilius.db** | 350 structures, 17,122 fields, 211 critical offsets | Kernel structures |
| **mslearn_reference.db** | 27 pages, 222 concepts, 1,196 cross-references | MS Learn docs |
| **driver_re.db** | Drivers, imports, exports, IOCTLs, xrefs | Driver analysis |

---

## Don't Guess - Query

If you're unsure about:
- VMCS field encoding → `vmcs_field_complete`
- Exit qualification bits → `exit_reason_complete`
- MSR address/behavior → `get_msr_info`
- Exception vector details → `get_exception_info`
- Control bit meanings → `get_vmx_control_bits`
- Detection thresholds → `get_timing_requirements`
- Windows structure offsets → `get_structure`, `get_field_offset`
- Dangerous kernel APIs → `dre_find_dangerous_apis`

**Never hardcode VMX constants without verification from OmbraMCP.**

---

## Known Issues

1. **No authentication**: Anyone can call tools - add auth for production
2. **No rate limiting**: Could be DoS'd with expensive queries
3. **Large project_brain.db**: 14MB and growing - consider archival strategy
4. **ChromaDB version sensitivity**: Pin version in pyproject.toml
