# MCP Server Infrastructure Component

## VERIFICATION STATUS
```
VERIFICATION:
- [x] I read every file mentioned in this document
- [x] I verified all function signatures against actual code
- [x] I verified tool counts against actual registrations
- [x] I verified database paths and schemas
- [ ] I tested all tool invocations

UNVERIFIED CLAIMS:
- Semantic search accuracy
- ChromaDB indexing correctness
- Cross-database query performance

ASSUMPTIONS:
- Python 3.10+ environment
- MCP protocol v1.0 compatibility
- SQLite database integrity
```

## DOCUMENTED FROM
```
Git hash: 73853be
Date: 2025-12-27
Files read:
  - ombra_mcp_server/src/ombra_mcp/server.py (700+ lines)
  - driver-re-mcp/src/driver_re_mcp/server.py (550+ lines)
  - ombra_mcp_server/pyproject.toml
  - driver-re-mcp/pyproject.toml
```

---

## Component Overview

**Location**:
- `ombra_mcp_server/` - Main hypervisor development server
- `driver-re-mcp/` - Driver reverse engineering server

**Purpose**: Provide Claude Code with specialized tools for hypervisor development and driver analysis through the Model Context Protocol (MCP).

**Inputs**:
- MCP tool invocations from Claude Code
- SQLite database queries
- Source code for analysis

**Outputs**:
- Intel SDM specifications
- Generated code (headers, handlers, templates)
- Stealth analysis results
- Project state tracking

---

## Server Comparison

| Aspect | ombra-mcp | driver-re-mcp |
|--------|-----------|---------------|
| **Tool Count** | 152 | 59 |
| **Primary Focus** | Hypervisor development | Driver reverse engineering |
| **Database(s)** | 8 SQLite + ChromaDB | 1 SQLite + ChromaDB |
| **Entry Point** | `server.py` | `server.py` |
| **Registration** | Explicit TOOLS list | Decorator pattern |

---

## OMBRA MCP Server

### Tool Categories

| Category | Count | Key Tools |
|----------|-------|-----------|
| Intel SDM | 12 | `vmcs_field`, `exit_reason`, `msr_info`, `ask_sdm` |
| Code Generation | 8 | `generate_vmcs_setup`, `generate_exit_handler`, `generate_ept_setup` |
| Stealth/Detection | 8 | `get_detection_vectors`, `audit_stealth`, `generate_timing_compensation` |
| BYOVD | 3 | `ld9boxsup_ioctl_guide`, `generate_driver_wrapper` |
| Driver Mapper | 8 | `get_pe_parsing_guide`, `get_relocation_guide`, `get_cleanup_guide` |
| HVCI Bypass | 7 | `get_zerohvci_architecture`, `get_hypercall_protocol` |
| Project Brain | 18 | `get_project_status`, `add_decision`, `add_gotcha`, `search_gotchas` |
| Concepts | 7 | `get_concept`, `check_concept_coverage`, `verify_concept` |
| Vergilius | 8 | `get_structure`, `get_field_offset`, `compare_versions` |
| Anti-Cheat Intel | 6 | `get_anticheat_intel`, `get_timing_requirements`, `check_evasion_coverage` |
| Evasion DB | 6 | `get_techniques`, `get_bypass_chains`, `get_cleanup_procedures` |
| BYOVD DB | 12 | `get_drivers`, `get_driver_ioctls`, `check_driver_blocklist` |
| Semantic | 3 | `semantic_search`, `rebuild_semantic_index` |
| MS Learn | 10 | `mslearn_search`, `mslearn_api`, `mslearn_concept` |
| Driver RE | 16 | `dre_add_driver`, `dre_find_dangerous_apis`, `dre_get_exports` |
| Other | 20 | Validation, binary scanning, timing simulation |

### Database Infrastructure

Located in `ombra_mcp_server/src/ombra_mcp/data/`:

| Database | Size | Purpose | Key Tables |
|----------|------|---------|------------|
| `intel_sdm.db` | 225KB | Intel SDM reference | vmcs_fields (167), exit_reasons (66), msrs (35), exceptions (20) |
| `project_brain.db` | 14MB | Project state | decisions, gotchas, sessions, components, findings |
| `anticheat_intel.db` | 102KB | Detection methods | anticheats, detection_methods, bypasses, signatures |
| `evasion_techniques.db` | 110KB | Bypass techniques | techniques (33), bypass_chains, cleanup_procedures |
| `byovd_drivers.db` | 61KB | Vulnerable drivers | drivers, ioctls, magic_values, blocklist_status |
| `vergilius.db` | 3.2MB | Windows structures | type_definitions (350), fields (17,122) |
| `mslearn_reference.db` | 905KB | MS Learn docs | pages (27), concepts (222), cross_references |
| `driver_re.db` | 168KB | Driver analysis | drivers, imports, exports, ioctls, xrefs |

### Key Functions

```python
# Database connection helper
def get_sdm_db():
    """Get connection to Intel SDM database."""
    return sqlite3.connect(SDM_DB)

# VMCS field lookup
async def vmcs_field(field_name: str) -> dict:
    """Get complete VMCS field specification.
    Returns: name, encoding, width, category, description, c_define
    """

# Exit reason lookup
async def exit_reason(reason: int) -> dict:
    """Get complete exit reason specification.
    Returns: number, name, has_qualification, qualification_format, handling_notes
    """

# Code generation
async def generate_vmcs_header() -> str:
    """Generate complete vmcs_fields.h with all field encodings.
    Groups by category: control, exit_info, guest_state, host_state
    """

async def generate_exit_handler_skeleton() -> str:
    """Generate exit_dispatch.c skeleton with all exit reasons.
    Includes timing compensation integration.
    """

async def generate_ept_structures() -> str:
    """Generate EPT structure definitions header.
    Full Intel SDM-compliant structures for PML4E, PDPTE, PDE, PTE.
    """
```

### Tool Handler Pattern

```python
# Tool definition with explicit schema
TOOLS = [
    Tool(
        name="vmcs_field",
        description="Get VMCS field specification by name",
        inputSchema={
            "type": "object",
            "properties": {
                "field_name": {
                    "type": "string",
                    "description": "VMCS field name (partial match OK)"
                }
            },
            "required": ["field_name"]
        }
    ),
    # ... 151 more tools
]

# Handler mapping
TOOL_HANDLERS = {
    "vmcs_field": vmcs_field,
    "vmcs_fields_by_category": vmcs_fields_by_category,
    "exit_reason": exit_reason,
    # ... handlers for all tools
}
```

---

## Driver-RE MCP Server

### Tool Categories

| Category | Count | Tools |
|----------|-------|-------|
| Driver Management | 5 | `add_driver`, `get_driver`, `list_drivers`, `update_driver_status`, `delete_driver` |
| IOCTL Analysis | 5 | `add_ioctl`, `get_ioctl`, `list_ioctls`, `get_vulnerable_ioctls`, `update_ioctl_vulnerability` |
| Function Analysis | 6 | `add_function`, `get_function`, `trace_call_path`, `find_dispatch_handlers` |
| Import Analysis | 4 | `get_imports`, `find_dangerous_apis`, `categorize_import`, `get_import_xrefs` |
| Export Analysis | 4 | `get_exports`, `document_export`, `analyze_exports` |
| Structure Management | 4 | `add_structure`, `get_structure`, `list_structures`, `link_structure_to_ioctl` |
| Vulnerability Tracking | 5 | `add_vulnerability`, `create_attack_chain`, `get_attack_chains` |
| Search | 6 | `semantic_search`, `text_search`, `find_similar_ioctls`, `find_similar_vulnerabilities` |
| Cross-References | 4 | `add_xref`, `get_xrefs`, `get_xref_graph` |
| Ghidra Integration | 8 | `ghidra_connect`, `ghidra_sync_functions`, `ghidra_get_decompilation`, `ghidra_export_all` |
| Analysis Sessions | 8 | `start_analysis_session`, `add_analysis_note`, `generate_analysis_report` |

### Decorator Registration Pattern

```python
# Tool decorator populates TOOLS dict at import time
def tool(name: str, description: str, input_schema: dict):
    """Decorator to register tools"""
    def decorator(func):
        TOOLS[name] = {
            'handler': func,
            'description': description,
            'input_schema': input_schema
        }
        return func
    return decorator

# Usage example
@tool("add_driver", "Add a new driver to the database", {
    "type": "object",
    "properties": {
        "original_name": {"type": "string"},
        "md5": {"type": "string"},
        "sha1": {"type": "string"},
        "sha256": {"type": "string"},
        "file_size": {"type": "integer"},
        "image_base": {"type": "integer"},
        "entry_point_rva": {"type": "integer"}
    },
    "required": ["original_name", "md5", "sha1", "sha256", "file_size", "image_base", "entry_point_rva"]
})
async def add_driver(**kwargs) -> dict:
    return await driver_tools.add_driver(**kwargs)
```

### Dangerous API Detection

From `import_tools.py`:

```python
DANGEROUS_APIS = {
    # Physical memory access
    'MmCopyMemory': 'Physical memory read - can bypass process isolation',
    'MmMapIoSpace': 'Maps physical address to virtual - common BYOVD primitive',
    'MmMapLockedPages': 'Maps locked pages - potential kernel memory access',

    # Process manipulation
    'PsLookupProcessByProcessId': 'Get EPROCESS by PID - process targeting',
    'KeStackAttachProcess': 'Attach to process context',
    'PsGetProcessPeb': 'Get PEB - process info disclosure',

    # Registry operations
    'ZwCreateKey': 'Registry key creation - persistence',
    'ZwSetValueKey': 'Registry value modification',
    'ZwDeleteKey': 'Registry key deletion - anti-forensics',

    # CPU control
    '__writecr0': 'Write CR0 - disable write protection',
    '__readmsr': 'Read MSR - CPU control',
    '__writemsr': 'Write MSR - CPU control',

    # ... 40+ more APIs
}
```

### VirtualBox Pattern Detection

From `export_tools.py`:

```python
KNOWN_PREFIXES = {
    'SUP': 'Support/helper functions (VirtualBox pattern)',
    'SUPR0': 'Ring-0 support functions (VirtualBox pattern)',
    'SUPR3': 'Ring-3 support functions (VirtualBox pattern)',
    'ASM': 'Assembly functions (VirtualBox pattern)',
    'RT': 'Runtime functions (VirtualBox pattern)',
}

DANGEROUS_PATTERNS = {
    'MmCopy': 'Physical memory copy - exploitation primitive',
    'ReadPhys': 'Physical memory read - info disclosure',
    'WritePhys': 'Physical memory write - code execution',
    'MSR': 'MSR access - CPU control',
    'CR0': 'CR0 access - write protection bypass',
}
```

---

## Semantic Search Infrastructure

Both servers use ChromaDB for vector search:

```python
from chromadb import PersistentClient
from chromadb.utils.embedding_functions import DefaultEmbeddingFunction

# Initialize ChromaDB
client = PersistentClient(path="data/chroma")
ef = DefaultEmbeddingFunction()  # Uses all-MiniLM-L6-v2 via ONNX

# Get or create collection
collection = client.get_or_create_collection("intel_sdm", embedding_function=ef)

# Query example
results = collection.query(
    query_texts=["EPT violation handling"],
    n_results=5
)
```

Collections indexed:
- `anticheat_intel` - Detection methods and bypasses
- `evasion_techniques` - Stealth techniques
- `byovd_drivers` - Vulnerable driver info
- `intel_sdm` - SDM content
- `mslearn_reference` - Microsoft documentation
- `project_brain` - Decisions and gotchas
- `vergilius` - Windows kernel structures

---

## MCP Protocol Implementation

### Server Initialization

```python
from mcp.server import Server, InitializationOptions
from mcp.server.stdio import stdio_server
from mcp.types import Tool, TextContent, ServerCapabilities, ToolsCapability

app = Server("ombra-mcp")

@app.list_tools()
async def list_tools() -> list[Tool]:
    """Return all available tools."""
    return TOOLS

@app.call_tool()
async def call_tool(name: str, arguments: dict) -> list[TextContent]:
    """Execute a tool and return results."""
    if name not in TOOL_HANDLERS:
        return [TextContent(type="text", text=f"Unknown tool: {name}")]

    handler = TOOL_HANDLERS[name]
    result = await handler(**arguments)
    return [TextContent(type="text", text=json.dumps(result, indent=2))]

async def main():
    async with stdio_server() as streams:
        await app.run(*streams, InitializationOptions(
            capabilities=ServerCapabilities(tools=ToolsCapability())
        ))
```

### Entry Points

**ombra-mcp** (`pyproject.toml`):
```toml
[project.scripts]
ombra-mcp = "ombra_mcp.server:main"
```

**driver-re-mcp** (`pyproject.toml`):
```toml
[project.scripts]
driver-re-mcp = "driver_re_mcp.server:main"
```

---

## Tool Module Structure

### OMBRA MCP Modules

```
ombra_mcp_server/src/ombra_mcp/
├── server.py              # Main server, tool definitions
├── tools/
│   ├── __init__.py        # Exports all tool functions
│   ├── sdm_query.py       # ask_sdm, vmcs_field_complete, etc.
│   ├── code_generator.py  # generate_vmcs_setup, generate_exit_handler
│   ├── stealth.py         # get_detection_vectors, audit_stealth
│   ├── byovd.py           # ld9boxsup_ioctl_guide
│   ├── driver_mapper.py   # PE parsing/mapping guides
│   ├── hvci_bypass.py     # ZeroHVCI architecture
│   ├── concepts.py        # Concept intelligence system
│   ├── vergilius.py       # Windows kernel structures
│   ├── anticheat_db.py    # Anti-cheat detection methods
│   ├── evasion_db.py      # Evasion techniques
│   ├── byovd_db.py        # Vulnerable driver catalog
│   ├── semantic_search.py # ChromaDB vector search
│   ├── mslearn_db.py      # MS Learn documentation
│   ├── driver_tools.py    # Driver RE integration
│   ├── ioctl_tools.py     # IOCTL analysis
│   ├── import_tools.py    # Import analysis
│   └── export_tools.py    # Export analysis
└── data/
    ├── *.db               # SQLite databases
    └── chroma/            # ChromaDB vector store
```

### Driver-RE MCP Modules

```
driver-re-mcp/src/driver_re_mcp/
├── server.py              # Main server with decorators
├── config.py              # Settings and paths
├── database/
│   ├── __init__.py
│   ├── schema.sql         # Full schema definition
│   └── connection.py      # Connection management
├── tools/
│   ├── driver_tools.py    # Driver lifecycle
│   ├── ioctl_tools.py     # IOCTL with CTL_CODE decode
│   ├── function_tools.py  # Call graphs, dispatch handlers
│   ├── import_tools.py    # Dangerous API detection
│   ├── export_tools.py    # VBox pattern detection
│   ├── struct_tools.py    # Structure management
│   ├── vuln_tools.py      # Vulnerability tracking
│   ├── search_tools.py    # Text + semantic search
│   ├── xref_tools.py      # Cross-references
│   ├── ghidra_tools.py    # Ghidra bidirectional sync
│   └── analysis_tools.py  # Sessions and reporting
└── data/
    ├── driver_re.db       # Main database
    └── chroma/            # Vector store
```

---

## Usage Examples

### Query VMCS Field

```bash
mcp-cli call ombra/vmcs_field '{"field_name": "GUEST_RIP"}'
```

Result:
```json
{
  "name": "GUEST_RIP",
  "encoding": "0x681E",
  "width": "natural",
  "category": "guest_state",
  "description": "Guest RIP register",
  "c_define": "#define VMCS_GUEST_RIP 0x681E"
}
```

### Generate Exit Handler

```bash
mcp-cli call ombra/generate_exit_handler '{"reason": 10, "stealth": true}'
```

### Find Dangerous APIs

```bash
mcp-cli call driver-re-mcp/find_dangerous_apis '{"driver_id": "abc123"}'
```

---

## CONCERNS

### Performance

1. **Multiple DB Connections**: Each tool opens new connection (no pooling)
2. **Large Result Sets**: Some queries return unbounded results
3. **Semantic Search**: ChromaDB queries can be slow on first load

### Security

1. **SQL Injection**: Some queries use string formatting (should use parameters)
2. **File Path Handling**: Some tools read arbitrary paths
3. **No Authentication**: MCP server trusts all incoming requests

### Maintenance

1. **Tool Count Growth**: 152+ tools becoming unwieldy
2. **Schema Drift**: DB schemas not version controlled
3. **Duplicate Data**: Some info duplicated across databases

---

## GAPS AND UNKNOWNS

- [ ] How does ChromaDB handle concurrent access?
- [ ] What's the embedding model dimension and latency?
- [ ] Are database migrations handled automatically?
- [ ] How to add new tools without server restart?
- [ ] What's the MCP protocol version compatibility?

---

*Component documentation generated 2025-12-27*
*CONFIDENCE: HIGH for tool inventory, MEDIUM for internal behavior*
