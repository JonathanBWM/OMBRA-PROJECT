# OmbraMCP — Hypervisor Development MCP Server

## Overview

A specialized MCP server that gives Claude Code instant access to:
1. **Intel SDM** — Searchable, chunked, with exact VMCS/MSR/Exit specifications
2. **AMD APM** — For future AMD SVM support
3. **Code Generation** — Pre-built templates and skeleton generators
4. **Build System** — Compile, test, debug integration
5. **Detection Testing** — Run timing checks against the hypervisor
6. **Project Memory** — Track implementation state across sessions

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                         OmbraMCP Server                              │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐               │
│  │   Intel SDM  │  │   AMD APM    │  │  Code Gen    │               │
│  │    Module    │  │   Module     │  │   Module     │               │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘               │
│         │                 │                 │                        │
│  ┌──────┴─────────────────┴─────────────────┴───────┐               │
│  │                 Tool Router                       │               │
│  └──────┬─────────────────┬─────────────────┬───────┘               │
│         │                 │                 │                        │
│  ┌──────┴───────┐  ┌──────┴───────┐  ┌──────┴───────┐               │
│  │    Build     │  │   Testing    │  │   Memory     │               │
│  │    Module    │  │   Module     │  │   Module     │               │
│  └──────────────┘  └──────────────┘  └──────────────┘               │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Module 1: Intel SDM Reference

### Data Preparation (One-Time)

```python
# scripts/prepare_intel_sdm.py

import fitz  # PyMuPDF
import json
import sqlite3
from sentence_transformers import SentenceTransformer

def extract_vmcs_fields(pdf_path):
    """Extract all VMCS field tables from Intel SDM Vol 3C"""
    
    vmcs_fields = []
    
    # Parse tables from Chapter 24 (VMCS)
    # Extract: field name, encoding, width, description
    
    return vmcs_fields

def extract_exit_reasons(pdf_path):
    """Extract all VM-exit reasons and their qualifications"""
    
    exits = []
    
    # Parse Appendix C (VM-Exit Reasons)
    # Extract: reason number, name, qualification bits, handling requirements
    
    return exits

def extract_msr_definitions(pdf_path):
    """Extract VMX-related MSR definitions"""
    
    msrs = []
    
    # Parse Chapter 23 (VMX MSRs)
    # Extract: MSR number, name, bit fields, description
    
    return msrs

def chunk_and_embed(pdf_path, output_db):
    """Chunk entire SDM and create embeddings for semantic search"""
    
    model = SentenceTransformer('all-MiniLM-L6-v2')
    
    doc = fitz.open(pdf_path)
    chunks = []
    
    for page_num, page in enumerate(doc):
        text = page.get_text()
        
        # Chunk by paragraphs/sections
        sections = split_into_sections(text)
        
        for section in sections:
            embedding = model.encode(section)
            chunks.append({
                'page': page_num,
                'text': section,
                'embedding': embedding.tolist()
            })
    
    # Store in SQLite with vector extension
    conn = sqlite3.connect(output_db)
    # ... store chunks and embeddings
    
    return len(chunks)
```

### Database Schema

```sql
-- intel_sdm.db

CREATE TABLE vmcs_fields (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    encoding INTEGER NOT NULL,
    width TEXT NOT NULL,           -- "16", "32", "64", "natural"
    category TEXT NOT NULL,        -- "guest_state", "host_state", "control", "exit_info"
    description TEXT,
    bit_fields TEXT,               -- JSON of bit field definitions
    sdm_section TEXT               -- Reference to SDM section
);

CREATE TABLE exit_reasons (
    id INTEGER PRIMARY KEY,
    reason_number INTEGER NOT NULL,
    name TEXT NOT NULL,
    has_qualification BOOLEAN,
    qualification_format TEXT,     -- JSON describing qual bits
    handling_notes TEXT,
    sdm_section TEXT
);

CREATE TABLE msrs (
    id INTEGER PRIMARY KEY,
    msr_number INTEGER NOT NULL,
    name TEXT NOT NULL,
    bit_fields TEXT,               -- JSON of bit definitions
    read_behavior TEXT,
    write_behavior TEXT,
    vmx_related BOOLEAN,
    sdm_section TEXT
);

CREATE TABLE sdm_chunks (
    id INTEGER PRIMARY KEY,
    chapter INTEGER,
    section TEXT,
    page INTEGER,
    content TEXT,
    embedding BLOB                 -- Vector embedding for semantic search
);

CREATE VIRTUAL TABLE sdm_fts USING fts5(content, chapter, section);
```

### MCP Tools

```python
# Module 1 Tools

@tool
def vmcs_field(field_name: str) -> dict:
    """
    Get complete VMCS field specification.
    
    Args:
        field_name: Field name (e.g., "GUEST_RIP", "VM_EXIT_REASON")
    
    Returns:
        {
            "name": "GUEST_RIP",
            "encoding": "0x681E",
            "width": "natural",
            "category": "guest_state",
            "description": "Guest RIP register value",
            "c_define": "#define VMCS_GUEST_RIP 0x681E"
        }
    """
    pass

@tool
def vmcs_fields_by_category(category: str) -> list:
    """
    Get all VMCS fields in a category.
    
    Args:
        category: One of "guest_state", "host_state", "control", "exit_info", "all"
    
    Returns:
        List of field specifications
    """
    pass

@tool
def exit_reason(reason: int) -> dict:
    """
    Get complete exit reason specification.
    
    Args:
        reason: Exit reason number (0-65)
    
    Returns:
        {
            "number": 10,
            "name": "CPUID",
            "qualification": null,
            "handling": "Execute CPUID with guest RAX/RCX, return results, advance RIP by 2",
            "sdm_reference": "Vol 3C, Section 25.1.3"
        }
    """
    pass

@tool
def exit_qualification_format(reason: int) -> dict:
    """
    Get exit qualification bit layout for specific exit reason.
    
    Args:
        reason: Exit reason number
    
    Returns:
        Bit field definitions for EXIT_QUALIFICATION
    """
    pass

@tool
def msr_info(msr: str) -> dict:
    """
    Get MSR specification by name or number.
    
    Args:
        msr: MSR name ("IA32_VMX_BASIC") or hex number ("0x480")
    
    Returns:
        Full MSR specification including bit fields
    """
    pass

@tool
def search_sdm(query: str, max_results: int = 3) -> list:
    """
    Semantic search across Intel SDM.
    
    Args:
        query: Natural language query
        max_results: Maximum chunks to return
    
    Returns:
        Relevant SDM excerpts with page references
    """
    pass

@tool
def sdm_section(section: str) -> str:
    """
    Get specific SDM section content.
    
    Args:
        section: Section reference (e.g., "24.6.1", "Appendix C")
    
    Returns:
        Full section text
    """
    pass
```

---

## Module 2: Code Generation

### Pre-Built Templates

```python
@tool
def generate_vmcs_header() -> str:
    """
    Generate complete vmcs_fields.h with all field encodings.
    
    Returns:
        C header file content with all VMCS field #defines
    """
    
    fields = db.query("SELECT * FROM vmcs_fields ORDER BY encoding")
    
    output = """// vmcs_fields.h — Auto-generated from Intel SDM
// DO NOT EDIT MANUALLY

#ifndef VMCS_FIELDS_H
#define VMCS_FIELDS_H

// Guest-State Fields
"""
    
    for field in fields:
        output += f"#define VMCS_{field.name} 0x{field.encoding:04X}\n"
    
    output += "\n#endif // VMCS_FIELDS_H\n"
    return output

@tool
def generate_exit_handler_skeleton() -> str:
    """
    Generate exit_dispatch.c skeleton with all exit reasons.
    
    Returns:
        C source file with switch statement covering all exits
    """
    pass

@tool
def generate_msr_header() -> str:
    """
    Generate msr_defs.h with all relevant MSR numbers.
    
    Returns:
        C header file content
    """
    pass

@tool
def generate_handler_template(exit_reason: int) -> str:
    """
    Generate handler function template for specific exit reason.
    
    Args:
        exit_reason: Exit reason number
    
    Returns:
        C function template with comments explaining handling requirements
    """
    
    exit = db.query("SELECT * FROM exit_reasons WHERE reason_number = ?", exit_reason)
    
    template = f"""// Handler for {exit.name} (Exit Reason {exit.reason_number})
// SDM Reference: {exit.sdm_section}

void handle_{exit.name.lower()}(VCPU* vcpu) {{
    // Exit qualification format:
    // {exit.qualification_format}
    
    uint64_t qualification = __vmx_vmread(VMCS_EXIT_QUALIFICATION);
    
    // TODO: Implement handling
    // {exit.handling_notes}
    
    // Advance RIP if needed
    vcpu->guest_rip += __vmx_vmread(VMCS_EXIT_INSTRUCTION_LENGTH);
}}
"""
    return template

@tool
def generate_ept_structures() -> str:
    """
    Generate EPT structure definitions header.
    
    Returns:
        C header with EPT_PML4E, EPT_PDPTE, EPT_PDE, EPT_PTE structures
    """
    pass

@tool
def generate_asm_stub(stub_type: str) -> str:
    """
    Generate assembly stub.
    
    Args:
        stub_type: "vmexit", "vmenter", "intrinsics", "segment"
    
    Returns:
        MASM x64 assembly code
    """
    pass
```

---

## Module 3: Build System Integration

```python
@tool
def compile_hypervisor(target: str = "debug") -> dict:
    """
    Compile the hypervisor payload.
    
    Args:
        target: "debug" or "release"
    
    Returns:
        {
            "success": bool,
            "output_path": str,
            "errors": list,
            "warnings": list,
            "size_bytes": int
        }
    """
    pass

@tool
def compile_file(file_path: str) -> dict:
    """
    Compile single source file to check for errors.
    
    Args:
        file_path: Path to .c or .asm file
    
    Returns:
        Compilation result with errors/warnings
    """
    pass

@tool
def disassemble(binary_path: str, function: str = None) -> str:
    """
    Disassemble compiled binary.
    
    Args:
        binary_path: Path to .sys or .obj file
        function: Optional function name to disassemble
    
    Returns:
        Disassembly listing
    """
    pass

@tool
def check_signatures(binary_path: str) -> list:
    """
    Scan binary for known hypervisor signatures.
    
    Args:
        binary_path: Path to compiled binary
    
    Returns:
        List of detected signatures that need to be changed
    """
    
    # Check against known patterns from:
    # - HyperPlatform
    # - SimpleSvm
    # - Voyager
    # - gbhv
    # etc.
    pass
```

---

## Module 4: Testing & Detection

```python
@tool
def run_timing_test(test_type: str = "cpuid") -> dict:
    """
    Run timing-based detection test against loaded hypervisor.
    
    Args:
        test_type: "cpuid", "rdtsc", "vmcall", "all"
    
    Returns:
        {
            "test": "cpuid",
            "cycles_measured": 87,
            "threshold": 150,
            "passed": True
        }
    """
    pass

@tool
def run_cpuid_check() -> dict:
    """
    Check all CPUID leaves for hypervisor artifacts.
    
    Returns:
        {
            "leaf_1_ecx_bit31": False,  # Should be False (hidden)
            "leaf_40000000": "zeros",    # Should be zeros
            "vmx_bit_hidden": True,
            "passed": True
        }
    """
    pass

@tool
def check_msr_exposure() -> dict:
    """
    Test if VMX MSRs are properly hidden.
    
    Returns:
        List of MSRs and their exposure status
    """
    pass

@tool  
def run_detection_suite() -> dict:
    """
    Run full anti-detection test suite.
    
    Returns:
        Comprehensive detection test results
    """
    pass
```

---

## Module 5: Project Memory

```python
@tool
def get_implementation_status() -> dict:
    """
    Get current implementation status of all components.
    
    Returns:
        {
            "phase": 2,
            "completed": ["shared/vmcs_fields.h", "shared/msr_defs.h", ...],
            "in_progress": ["hypervisor/vmcs.c"],
            "remaining": ["hypervisor/handlers/cpuid.c", ...],
            "blockers": [],
            "notes": "VMCS guest state fields done, working on host state"
        }
    """
    pass

@tool
def update_status(file: str, status: str, notes: str = None) -> None:
    """
    Update implementation status for a file.
    
    Args:
        file: File path
        status: "completed", "in_progress", "blocked", "not_started"
        notes: Optional notes
    """
    pass

@tool
def add_implementation_note(component: str, note: str) -> None:
    """
    Add a note about implementation decisions or issues.
    
    Args:
        component: Component name
        note: Note content
    """
    pass

@tool
def get_component_notes(component: str) -> list:
    """
    Get all notes for a component.
    
    Args:
        component: Component name or "all"
    
    Returns:
        List of notes with timestamps
    """
    pass

@tool
def get_known_issues() -> list:
    """
    Get list of known issues and blockers.
    
    Returns:
        List of issues with status
    """
    pass
```

---

## Module 6: Vulnerable Driver Interface

```python
@tool
def ld9boxsup_ioctl_info(ioctl_name: str) -> dict:
    """
    Get Ld9BoxSup.sys IOCTL specification.
    
    Args:
        ioctl_name: IOCTL name (e.g., "SUP_IOCTL_CONT_ALLOC")
    
    Returns:
        {
            "name": "SUP_IOCTL_CONT_ALLOC",
            "code": "0x22A00C",
            "input_struct": "SUPCONTALLOC_IN",
            "output_struct": "SUPCONTALLOC_OUT",
            "description": "Allocate contiguous physical memory",
            "usage_example": "..."
        }
    """
    pass

@tool
def generate_ioctl_wrapper(ioctl_name: str) -> str:
    """
    Generate C wrapper function for IOCTL.
    
    Args:
        ioctl_name: IOCTL name
    
    Returns:
        C function implementation
    """
    pass

@tool
def list_available_ioctls() -> list:
    """
    List all available Ld9BoxSup.sys IOCTLs.
    
    Returns:
        List of IOCTL names with brief descriptions
    """
    pass
```

---

## Implementation

### Directory Structure

```
ombra-mcp-server/
├── pyproject.toml
├── README.md
├── src/
│   └── ombra_mcp/
│       ├── __init__.py
│       ├── server.py              # Main MCP server
│       ├── modules/
│       │   ├── __init__.py
│       │   ├── intel_sdm.py       # Intel SDM tools
│       │   ├── amd_apm.py         # AMD APM tools (future)
│       │   ├── codegen.py         # Code generation tools
│       │   ├── build.py           # Build system tools
│       │   ├── testing.py         # Detection testing tools
│       │   ├── memory.py          # Project memory tools
│       │   └── driver.py          # Vulnerable driver tools
│       ├── data/
│       │   ├── intel_sdm.db       # Prepared SDM database
│       │   ├── signatures.json    # Known hypervisor signatures
│       │   └── templates/         # Code templates
│       └── utils/
│           ├── pdf_parser.py
│           ├── embeddings.py
│           └── compiler.py
├── scripts/
│   ├── prepare_intel_sdm.py       # One-time SDM preparation
│   ├── prepare_amd_apm.py
│   └── update_signatures.py
└── tests/
    └── ...
```

### server.py

```python
#!/usr/bin/env python3
"""OmbraMCP — Hypervisor Development MCP Server"""

import asyncio
from mcp.server import Server
from mcp.server.stdio import stdio_server
from mcp.types import Tool, TextContent

from .modules import intel_sdm, codegen, build, testing, memory, driver

app = Server("ombra-mcp")

# Register all tools from modules
MODULES = [intel_sdm, codegen, build, testing, memory, driver]

def register_tools():
    for module in MODULES:
        for tool_func in module.get_tools():
            app.register_tool(tool_func)

@app.list_tools()
async def list_tools() -> list[Tool]:
    return [
        # Intel SDM Tools
        Tool(name="vmcs_field", description="Get VMCS field specification", inputSchema={...}),
        Tool(name="vmcs_fields_by_category", description="Get all VMCS fields in category", inputSchema={...}),
        Tool(name="exit_reason", description="Get exit reason specification", inputSchema={...}),
        Tool(name="exit_qualification_format", description="Get exit qualification bits", inputSchema={...}),
        Tool(name="msr_info", description="Get MSR specification", inputSchema={...}),
        Tool(name="search_sdm", description="Semantic search Intel SDM", inputSchema={...}),
        Tool(name="sdm_section", description="Get specific SDM section", inputSchema={...}),
        
        # Code Generation Tools
        Tool(name="generate_vmcs_header", description="Generate vmcs_fields.h", inputSchema={...}),
        Tool(name="generate_exit_handler_skeleton", description="Generate exit handler skeleton", inputSchema={...}),
        Tool(name="generate_msr_header", description="Generate msr_defs.h", inputSchema={...}),
        Tool(name="generate_handler_template", description="Generate handler for exit reason", inputSchema={...}),
        Tool(name="generate_ept_structures", description="Generate EPT structure definitions", inputSchema={...}),
        Tool(name="generate_asm_stub", description="Generate assembly stub", inputSchema={...}),
        
        # Build Tools
        Tool(name="compile_hypervisor", description="Compile the hypervisor", inputSchema={...}),
        Tool(name="compile_file", description="Compile single file", inputSchema={...}),
        Tool(name="disassemble", description="Disassemble binary", inputSchema={...}),
        Tool(name="check_signatures", description="Check for known signatures", inputSchema={...}),
        
        # Testing Tools
        Tool(name="run_timing_test", description="Run timing detection test", inputSchema={...}),
        Tool(name="run_cpuid_check", description="Check CPUID leaves", inputSchema={...}),
        Tool(name="check_msr_exposure", description="Check MSR exposure", inputSchema={...}),
        Tool(name="run_detection_suite", description="Run full detection suite", inputSchema={...}),
        
        # Memory Tools
        Tool(name="get_implementation_status", description="Get project status", inputSchema={...}),
        Tool(name="update_status", description="Update file status", inputSchema={...}),
        Tool(name="add_implementation_note", description="Add implementation note", inputSchema={...}),
        Tool(name="get_known_issues", description="Get known issues", inputSchema={...}),
        
        # Driver Tools
        Tool(name="ld9boxsup_ioctl_info", description="Get IOCTL specification", inputSchema={...}),
        Tool(name="generate_ioctl_wrapper", description="Generate IOCTL wrapper", inputSchema={...}),
        Tool(name="list_available_ioctls", description="List available IOCTLs", inputSchema={...}),
    ]

@app.call_tool()
async def call_tool(name: str, arguments: dict) -> list[TextContent]:
    # Route to appropriate module
    for module in MODULES:
        if hasattr(module, name):
            result = await getattr(module, name)(**arguments)
            return [TextContent(type="text", text=str(result))]
    
    raise ValueError(f"Unknown tool: {name}")

async def main():
    register_tools()
    async with stdio_server() as (read_stream, write_stream):
        await app.run(read_stream, write_stream)

if __name__ == "__main__":
    asyncio.run(main())
```

---

## Setup Instructions

### 1. Prepare Intel SDM Database

```bash
# Download Intel SDM Vol 3 (all parts combined)
# Place at data/intel_sdm_vol3.pdf

cd ombra-mcp-server
python scripts/prepare_intel_sdm.py data/intel_sdm_vol3.pdf

# This creates:
# - data/intel_sdm.db (SQLite with all extracted data + embeddings)
# - data/vmcs_fields.json
# - data/exit_reasons.json
# - data/msrs.json
```

### 2. Install MCP Server

```bash
cd ombra-mcp-server
pip install -e .

# Or with uv
uv pip install -e .
```

### 3. Configure Claude Desktop

```json
// ~/Library/Application Support/Claude/claude_desktop_config.json
// or %APPDATA%\Claude\claude_desktop_config.json

{
  "mcpServers": {
    "ombra": {
      "command": "python",
      "args": ["-m", "ombra_mcp.server"],
      "cwd": "/path/to/ombra-mcp-server"
    }
  }
}
```

### 4. Configure Claude Code

```json
// .claude/settings.json in project root

{
  "mcp": {
    "servers": {
      "ombra": {
        "command": "python",
        "args": ["-m", "ombra_mcp.server"],
        "cwd": "/path/to/ombra-mcp-server"
      }
    }
  }
}
```

---

## Usage Examples

### Claude Code Session

```
User: Generate the VMCS fields header

Claude: I'll use the ombra MCP server to generate this.

[Calls: generate_vmcs_header()]

Here's the complete vmcs_fields.h:
[... generated header ...]

User: What's the exit qualification format for EPT violations?

Claude: Let me look that up.

[Calls: exit_qualification_format(48)]

EPT violation (exit reason 48) qualification bits:
- Bit 0: Read access
- Bit 1: Write access  
- Bit 2: Instruction fetch
- Bit 3: Guest physical address readable
[...]

User: Generate the handler template for CPUID

Claude: [Calls: generate_handler_template(10)]

Here's the CPUID handler template with timing evasion:
[... generated code ...]

User: Compile and check for signatures

Claude: [Calls: compile_hypervisor("debug")]
[Calls: check_signatures("build/hypervisor.sys")]

Compilation successful. Signature check found 2 issues:
1. VMCS setup pattern at offset 0x1234 matches HyperPlatform
2. Exit handler switch structure similar to SimpleSvm

Let me refactor those sections...
```

---

## Power Features

### 1. Contextual Code Generation

The MCP server doesn't just return data — it generates **ready-to-use code** with:
- Correct encodings pulled from SDM
- Comments explaining the "why"
- Detection avoidance patterns built-in

### 2. Signature Database

Maintains patterns from:
- All public hypervisor projects
- Known anti-cheat signatures
- Common code generation patterns to avoid

### 3. Incremental Compilation

Compile individual files to catch errors early without full rebuild.

### 4. Project Memory

Tracks implementation state across sessions so Claude Code can:
- Resume where it left off
- Remember design decisions
- Track blockers and issues

### 5. Integrated Testing

Run detection tests directly from Claude Code to verify stealth.
