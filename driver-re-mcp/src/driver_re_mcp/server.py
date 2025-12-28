"""Driver RE MCP Server - Main entry point"""

import asyncio
import logging
from pathlib import Path
from typing import Any

from mcp.server import Server
from mcp.server.stdio import stdio_server
from mcp.types import Tool, TextContent

from .config import settings
from .database import init_database, get_db
from .tools import driver_tools, ioctl_tools, import_tools, export_tools
from .tools import function_tools, struct_tools, vuln_tools, search_tools
from .tools import xref_tools, ghidra_tools, analysis_tools

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Initialize MCP server
app = Server("driver-re-mcp")

# Tool registry - maps tool names to handler functions
TOOLS = {}

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

# ============================================
# DRIVER MANAGEMENT TOOLS (5)
# ============================================

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

@tool("get_driver", "Get driver by ID or SHA256", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "sha256": {"type": "string"}
    }
})
async def get_driver(**kwargs) -> dict:
    return await driver_tools.get_driver(**kwargs)

@tool("list_drivers", "List all drivers", {
    "type": "object",
    "properties": {
        "status": {"type": "string", "enum": ["pending", "in_progress", "analyzed", "blocked"]},
        "is_vulnerable": {"type": "boolean"},
        "limit": {"type": "integer", "default": 100}
    }
})
async def list_drivers(**kwargs) -> dict:
    return await driver_tools.list_drivers(**kwargs)

@tool("update_driver_status", "Update driver analysis status", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "status": {"type": "string", "enum": ["pending", "in_progress", "analyzed", "blocked"]}
    },
    "required": ["driver_id", "status"]
})
async def update_driver_status(**kwargs) -> dict:
    return await driver_tools.update_driver_status(**kwargs)

@tool("delete_driver", "Delete driver from database", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"}
    },
    "required": ["driver_id"]
})
async def delete_driver(**kwargs) -> dict:
    return await driver_tools.delete_driver(**kwargs)

# ============================================
# SCANNER INTEGRATION TOOLS (5)
# ============================================

@tool("import_scanner_results", "Import scanner JSON batch output with driver metadata", {
    "type": "object",
    "properties": {
        "json_path": {"type": "string", "description": "Path to scanner batch JSON output"}
    },
    "required": ["json_path"]
})
async def import_scanner_results(**kwargs) -> dict:
    return await driver_tools.import_scanner_results(**kwargs)

@tool("get_analysis_queue", "Get priority-sorted analysis queue from scanner imports", {
    "type": "object",
    "properties": {
        "min_priority": {"type": "integer", "default": 1, "description": "Minimum priority (1=highest)"},
        "max_priority": {"type": "integer", "default": 5, "description": "Maximum priority (5=lowest)"}
    }
})
async def get_analysis_queue(**kwargs) -> dict:
    return await driver_tools.get_analysis_queue(**kwargs)

@tool("import_single_driver_analysis", "Import a single MCPAnalysisRequest JSON object", {
    "type": "object",
    "properties": {
        "json_data": {"type": "string", "description": "JSON string of MCPAnalysisRequest"}
    },
    "required": ["json_data"]
})
async def import_single_driver_analysis(**kwargs) -> dict:
    return await driver_tools.import_single_driver_analysis(**kwargs)

@tool("set_driver_tier_info", "Update driver with scanner scoring info", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string", "description": "Driver UUID"},
        "tier": {"type": "string", "enum": ["S", "A", "B", "C", "D"], "description": "Capability tier"},
        "score": {"type": "number", "description": "Final score"},
        "classification": {"type": "string", "description": "Classification string"},
        "synergies": {"type": "string", "description": "Optional synergies description"}
    },
    "required": ["driver_id", "tier", "score", "classification"]
})
async def set_driver_tier_info(**kwargs) -> dict:
    return await driver_tools.set_driver_tier_info(**kwargs)

@tool("get_drivers_by_capability", "List drivers with specific capability", {
    "type": "object",
    "properties": {
        "capability": {
            "type": "string",
            "enum": ["module_loading", "physical_memory", "msr_access", "process_control"],
            "description": "Capability to filter by"
        }
    },
    "required": ["capability"]
})
async def get_drivers_by_capability(**kwargs) -> dict:
    return await driver_tools.get_drivers_by_capability(**kwargs)

# ============================================
# IOCTL ANALYSIS TOOLS (5)
# ============================================

@tool("add_ioctl", "Add IOCTL to driver with CTL_CODE decoding and vulnerability tracking", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string", "description": "UUID of the driver"},
        "name": {"type": "string", "description": "IOCTL name (e.g., IOCTL_READ_MEMORY)"},
        "code": {"type": "integer", "description": "IOCTL code (CTL_CODE macro result)"},
        "handler_rva": {"type": "integer", "description": "RVA of the handler function"},
        "handler_function_id": {"type": "string", "description": "UUID of handler function"},
        "input_struct_id": {"type": "string", "description": "UUID of input structure"},
        "output_struct_id": {"type": "string", "description": "UUID of output structure"},
        "min_input_size": {"type": "integer", "description": "Minimum input buffer size"},
        "max_input_size": {"type": "integer", "description": "Maximum input buffer size"},
        "min_output_size": {"type": "integer", "description": "Minimum output buffer size"},
        "max_output_size": {"type": "integer", "description": "Maximum output buffer size"},
        "requires_admin": {"type": "boolean", "description": "Whether admin rights required"},
        "is_vulnerable": {"type": "boolean", "description": "Whether IOCTL is vulnerable"},
        "vulnerability_type": {"type": "string", "description": "Vulnerability classification"},
        "vulnerability_severity": {"type": "string", "enum": ["critical", "high", "medium", "low"]},
        "vulnerability_description": {"type": "string", "description": "Vulnerability details"},
        "description": {"type": "string", "description": "IOCTL description"}
    },
    "required": ["driver_id", "name", "code"]
})
async def add_ioctl(**kwargs) -> dict:
    return await ioctl_tools.add_ioctl(**kwargs)

@tool("get_ioctl", "Get IOCTL by ID or code", {
    "type": "object",
    "properties": {
        "ioctl_id": {"type": "string"},
        "driver_id": {"type": "string"},
        "code": {"type": "integer"}
    }
})
async def get_ioctl(**kwargs) -> dict:
    return await ioctl_tools.get_ioctl(**kwargs)

@tool("list_ioctls", "List IOCTLs for driver", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"}
    },
    "required": ["driver_id"]
})
async def list_ioctls(**kwargs) -> dict:
    return await ioctl_tools.list_ioctls(**kwargs)

@tool("get_vulnerable_ioctls", "Get vulnerable IOCTLs", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "severity": {"type": "string", "enum": ["critical", "high", "medium", "low"]}
    }
})
async def get_vulnerable_ioctls(**kwargs) -> dict:
    return await ioctl_tools.get_vulnerable_ioctls(**kwargs)

@tool("update_ioctl_vulnerability", "Update IOCTL vulnerability info", {
    "type": "object",
    "properties": {
        "ioctl_id": {"type": "string"},
        "is_vulnerable": {"type": "boolean"},
        "vulnerability_type": {"type": "string"},
        "vulnerability_severity": {"type": "string"},
        "vulnerability_description": {"type": "string"}
    },
    "required": ["ioctl_id"]
})
async def update_ioctl_vulnerability(**kwargs) -> dict:
    return await ioctl_tools.update_ioctl_vulnerability(**kwargs)

# ============================================
# FUNCTION TOOLS (6)
# ============================================

@tool("add_function", "Add function to driver with signature and dispatch handler info", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string", "description": "UUID of the driver"},
        "rva": {"type": "integer", "description": "Relative Virtual Address"},
        "name": {"type": "string", "description": "Function name"},
        "size": {"type": "integer", "description": "Function size in bytes"},
        "return_type": {"type": "string", "description": "Return type (e.g., NTSTATUS, void*)"},
        "parameters": {"type": "array", "items": {"type": "object"}, "description": "Parameters [{name, type, description}, ...]"},
        "is_dispatch": {"type": "boolean", "description": "True if IRP dispatch handler"},
        "dispatch_type": {"type": "string", "description": "IRP type (e.g., IRP_MJ_DEVICE_CONTROL)"},
        "decompiled": {"type": "string", "description": "Decompiled C code"},
        "annotations": {"type": "object", "description": "Additional metadata as JSON"}
    },
    "required": ["driver_id", "rva"]
})
async def add_function(**kwargs) -> dict:
    return await function_tools.add_function(**kwargs)

@tool("get_function", "Get function by ID or RVA", {
    "type": "object",
    "properties": {
        "function_id": {"type": "string"},
        "driver_id": {"type": "string"},
        "rva": {"type": "integer"}
    }
})
async def get_function(**kwargs) -> dict:
    return await function_tools.get_function(**kwargs)

@tool("get_function_callers", "Get functions that call this function", {
    "type": "object",
    "properties": {"function_id": {"type": "string"}},
    "required": ["function_id"]
})
async def get_function_callers(**kwargs) -> dict:
    return await function_tools.get_function_callers(**kwargs)

@tool("get_function_callees", "Get functions called by this function", {
    "type": "object",
    "properties": {"function_id": {"type": "string"}},
    "required": ["function_id"]
})
async def get_function_callees(**kwargs) -> dict:
    return await function_tools.get_function_callees(**kwargs)

@tool("trace_call_path", "Find all call paths between functions (BFS with depth limit)", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string", "description": "UUID of the driver"},
        "from_function": {"type": "string", "description": "Starting function (name or 0x-prefixed RVA)"},
        "to_function": {"type": "string", "description": "Target function (name or 0x-prefixed RVA)"},
        "max_depth": {"type": "integer", "default": 20, "description": "Maximum path depth"}
    },
    "required": ["driver_id", "from_function", "to_function"]
})
async def trace_call_path(**kwargs) -> dict:
    return await function_tools.trace_call_path(**kwargs)

@tool("find_dispatch_handlers", "Find IRP dispatch handlers", {
    "type": "object",
    "properties": {"driver_id": {"type": "string"}},
    "required": ["driver_id"]
})
async def find_dispatch_handlers(**kwargs) -> dict:
    return await function_tools.find_dispatch_handlers(**kwargs)

# ============================================
# STRUCTURE TOOLS (4)
# ============================================

@tool("add_structure", "Add structure definition", {
    "type": "object",
    "properties": {
        "name": {"type": "string"},
        "definition_c": {"type": "string"},
        "driver_id": {"type": "string"},
        "struct_type": {"type": "string"}
    },
    "required": ["name", "definition_c"]
})
async def add_structure(**kwargs) -> dict:
    return await struct_tools.add_structure(**kwargs)

@tool("get_structure", "Get structure by ID or name", {
    "type": "object",
    "properties": {
        "structure_id": {"type": "string"},
        "name": {"type": "string"}
    }
})
async def get_structure(**kwargs) -> dict:
    return await struct_tools.get_structure(**kwargs)

@tool("list_structures", "List structures", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "struct_type": {"type": "string"}
    }
})
async def list_structures(**kwargs) -> dict:
    return await struct_tools.list_structures(**kwargs)

@tool("link_structure_to_ioctl", "Link structure to IOCTL", {
    "type": "object",
    "properties": {
        "ioctl_id": {"type": "string"},
        "input_struct_id": {"type": "string"},
        "output_struct_id": {"type": "string"}
    },
    "required": ["ioctl_id"]
})
async def link_structure_to_ioctl(**kwargs) -> dict:
    return await struct_tools.link_structure_to_ioctl(**kwargs)

# ============================================
# VULNERABILITY TOOLS (5)
# ============================================

@tool("add_vulnerability", "Add vulnerability finding", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "title": {"type": "string"},
        "vulnerability_class": {"type": "string"},
        "severity": {"type": "string"},
        "description": {"type": "string"},
        "cve_id": {"type": "string"}
    },
    "required": ["driver_id", "title", "vulnerability_class", "severity", "description"]
})
async def add_vulnerability(**kwargs) -> dict:
    return await vuln_tools.add_vulnerability(**kwargs)

@tool("get_vulnerability", "Get vulnerability by ID", {
    "type": "object",
    "properties": {"vuln_id": {"type": "string"}},
    "required": ["vuln_id"]
})
async def get_vulnerability(**kwargs) -> dict:
    return await vuln_tools.get_vulnerability(**kwargs)

@tool("list_vulnerabilities", "List vulnerabilities", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "severity": {"type": "string"},
        "vulnerability_class": {"type": "string"}
    }
})
async def list_vulnerabilities(**kwargs) -> dict:
    return await vuln_tools.list_vulnerabilities(**kwargs)

@tool("create_attack_chain", "Create attack chain", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "name": {"type": "string"},
        "attack_goal": {"type": "string"},
        "steps": {"type": "array"},
        "initial_access": {"type": "string"},
        "final_privilege": {"type": "string"}
    },
    "required": ["driver_id", "name", "attack_goal", "steps", "initial_access", "final_privilege"]
})
async def create_attack_chain(**kwargs) -> dict:
    return await vuln_tools.create_attack_chain(**kwargs)

@tool("get_attack_chains", "Get attack chains", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "attack_goal": {"type": "string"}
    }
})
async def get_attack_chains(**kwargs) -> dict:
    return await vuln_tools.get_attack_chains(**kwargs)

# ============================================
# SEARCH TOOLS (6)
# ============================================

@tool("semantic_search", "Semantic search across entities", {
    "type": "object",
    "properties": {
        "query": {"type": "string"},
        "driver_id": {"type": "string"},
        "entity_types": {"type": "array"},
        "limit": {"type": "integer"}
    },
    "required": ["query"]
})
async def semantic_search(**kwargs) -> dict:
    return await search_tools.semantic_search(**kwargs)

@tool("text_search", "Full-text search", {
    "type": "object",
    "properties": {
        "query": {"type": "string"},
        "driver_id": {"type": "string"},
        "limit": {"type": "integer"}
    },
    "required": ["query"]
})
async def text_search(**kwargs) -> dict:
    return await search_tools.text_search(**kwargs)

@tool("search_strings", "Search strings in driver", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "pattern": {"type": "string"},
        "contains": {"type": "string"},
        "category": {"type": "string"}
    },
    "required": ["driver_id"]
})
async def search_strings(**kwargs) -> dict:
    return await search_tools.search_strings(**kwargs)

@tool("find_similar_ioctls", "Find similar IOCTLs", {
    "type": "object",
    "properties": {
        "ioctl_id": {"type": "string"},
        "limit": {"type": "integer"}
    },
    "required": ["ioctl_id"]
})
async def find_similar_ioctls(**kwargs) -> dict:
    return await search_tools.find_similar_ioctls(**kwargs)

@tool("find_similar_vulnerabilities", "Find similar vulnerabilities", {
    "type": "object",
    "properties": {
        "vuln_id": {"type": "string"},
        "limit": {"type": "integer"}
    },
    "required": ["vuln_id"]
})
async def find_similar_vulnerabilities(**kwargs) -> dict:
    return await search_tools.find_similar_vulnerabilities(**kwargs)

@tool("search_by_api_usage", "Search by API usage", {
    "type": "object",
    "properties": {
        "api_name": {"type": "string"},
        "driver_id": {"type": "string"}
    },
    "required": ["api_name"]
})
async def search_by_api_usage(**kwargs) -> dict:
    return await search_tools.search_by_api_usage(**kwargs)

# ============================================
# XREF TOOLS (5)
# ============================================

@tool("add_xref", "Add cross-reference", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "from_rva": {"type": "integer"},
        "to_rva": {"type": "integer"},
        "xref_type": {"type": "string"}
    },
    "required": ["driver_id", "from_rva", "to_rva", "xref_type"]
})
async def add_xref(**kwargs) -> dict:
    return await xref_tools.add_xref(**kwargs)

@tool("get_xrefs_to", "Get xrefs to address/function", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "rva": {"type": "integer"},
        "function_id": {"type": "string"}
    },
    "required": ["driver_id"]
})
async def get_xrefs_to(**kwargs) -> dict:
    return await xref_tools.get_xrefs_to(**kwargs)

@tool("get_xrefs_from", "Get xrefs from address/function", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "rva": {"type": "integer"},
        "function_id": {"type": "string"}
    },
    "required": ["driver_id"]
})
async def get_xrefs_from(**kwargs) -> dict:
    return await xref_tools.get_xrefs_from(**kwargs)

@tool("build_call_graph", "Build call graph from function", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "root_function": {"type": "string"},
        "depth": {"type": "integer"},
        "direction": {"type": "string"}
    },
    "required": ["driver_id"]
})
async def build_call_graph(**kwargs) -> dict:
    return await xref_tools.build_call_graph(**kwargs)

@tool("find_paths_to_api", "Find paths to dangerous API", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "api_name": {"type": "string"},
        "max_depth": {"type": "integer"}
    },
    "required": ["driver_id", "api_name"]
})
async def find_paths_to_api(**kwargs) -> dict:
    return await xref_tools.find_paths_to_api(**kwargs)

# ============================================
# GHIDRA INTEGRATION TOOLS (8)
# ============================================

@tool("ghidra_connect", "Connect to Ghidra MCP", {
    "type": "object",
    "properties": {
        "host": {"type": "string"},
        "port": {"type": "integer"}
    }
})
async def ghidra_connect(**kwargs) -> dict:
    return await ghidra_tools.ghidra_connect(**kwargs)

@tool("ghidra_sync_functions", "Sync functions with Ghidra", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "direction": {"type": "string"}
    },
    "required": ["driver_id"]
})
async def ghidra_sync_functions(**kwargs) -> dict:
    return await ghidra_tools.ghidra_sync_functions(**kwargs)

@tool("ghidra_sync_structures", "Sync structures with Ghidra", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "direction": {"type": "string"}
    },
    "required": ["driver_id"]
})
async def ghidra_sync_structures(**kwargs) -> dict:
    return await ghidra_tools.ghidra_sync_structures(**kwargs)

@tool("ghidra_get_decompilation", "Get decompilation from Ghidra", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "rva": {"type": "integer"},
        "function_name": {"type": "string"}
    },
    "required": ["driver_id"]
})
async def ghidra_get_decompilation(**kwargs) -> dict:
    return await ghidra_tools.ghidra_get_decompilation(**kwargs)

@tool("ghidra_get_xrefs", "Get xrefs from Ghidra", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "address": {"type": "integer"}
    },
    "required": ["driver_id", "address"]
})
async def ghidra_get_xrefs(**kwargs) -> dict:
    return await ghidra_tools.ghidra_get_xrefs(**kwargs)

@tool("ghidra_set_comment", "Set comment in Ghidra", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "address": {"type": "integer"},
        "comment": {"type": "string"},
        "comment_type": {"type": "string"}
    },
    "required": ["driver_id", "address", "comment"]
})
async def ghidra_set_comment(**kwargs) -> dict:
    return await ghidra_tools.ghidra_set_comment(**kwargs)

@tool("ghidra_rename_function", "Rename function in Ghidra", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "address": {"type": "integer"},
        "new_name": {"type": "string"}
    },
    "required": ["driver_id", "address", "new_name"]
})
async def ghidra_rename_function(**kwargs) -> dict:
    return await ghidra_tools.ghidra_rename_function(**kwargs)

@tool("ghidra_export_all", "Export all from Ghidra", {
    "type": "object",
    "properties": {"driver_id": {"type": "string"}},
    "required": ["driver_id"]
})
async def ghidra_export_all(**kwargs) -> dict:
    return await ghidra_tools.ghidra_export_all(**kwargs)

# ============================================
# ANALYSIS SESSION TOOLS (8)
# ============================================

@tool("start_analysis_session", "Start analysis session", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "name": {"type": "string"},
        "ghidra_project_path": {"type": "string"}
    },
    "required": ["driver_id"]
})
async def start_analysis_session(**kwargs) -> dict:
    return await analysis_tools.start_analysis_session(**kwargs)

@tool("add_analysis_note", "Add analysis note", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "content": {"type": "string"},
        "title": {"type": "string"},
        "note_type": {"type": "string"},
        "priority": {"type": "string"}
    },
    "required": ["driver_id", "content"]
})
async def add_analysis_note(**kwargs) -> dict:
    return await analysis_tools.add_analysis_note(**kwargs)

@tool("get_analysis_notes", "Get analysis notes", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "note_type": {"type": "string"},
        "priority": {"type": "string"}
    },
    "required": ["driver_id"]
})
async def get_analysis_notes(**kwargs) -> dict:
    return await analysis_tools.get_analysis_notes(**kwargs)

@tool("generate_analysis_report", "Generate analysis report", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "format": {"type": "string"}
    },
    "required": ["driver_id"]
})
async def generate_analysis_report(**kwargs) -> dict:
    return await analysis_tools.generate_analysis_report(**kwargs)

@tool("convert_address", "Convert between address types", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "address": {"type": "integer"},
        "from_type": {"type": "string"},
        "to_type": {"type": "string"}
    },
    "required": ["driver_id", "address", "from_type", "to_type"]
})
async def convert_address(**kwargs) -> dict:
    return await analysis_tools.convert_address(**kwargs)

@tool("get_api_info", "Get Windows API documentation", {
    "type": "object",
    "properties": {"api_name": {"type": "string"}},
    "required": ["api_name"]
})
async def get_api_info(**kwargs) -> dict:
    return await analysis_tools.get_api_info(**kwargs)

@tool("compare_drivers", "Compare two drivers", {
    "type": "object",
    "properties": {
        "driver_id_1": {"type": "string"},
        "driver_id_2": {"type": "string"}
    },
    "required": ["driver_id_1", "driver_id_2"]
})
async def compare_drivers(**kwargs) -> dict:
    return await analysis_tools.compare_drivers(**kwargs)

@tool("get_statistics", "Get driver/project statistics", {
    "type": "object",
    "properties": {"driver_id": {"type": "string"}}
})
async def get_statistics(**kwargs) -> dict:
    return await analysis_tools.get_statistics(**kwargs)

# ============================================
# IMPORT/EXPORT TOOLS (6)
# ============================================

@tool("get_imports", "Get driver imports", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "dll": {"type": "string"},
        "category": {"type": "string"},
        "dangerous_only": {"type": "boolean"}
    },
    "required": ["driver_id"]
})
async def get_imports(**kwargs) -> dict:
    return await import_tools.get_imports(**kwargs)

@tool("get_import_xrefs", "Get import cross-references", {
    "type": "object",
    "properties": {"import_id": {"type": "string"}},
    "required": ["import_id"]
})
async def get_import_xrefs(**kwargs) -> dict:
    return await import_tools.get_import_xrefs(**kwargs)

@tool("categorize_import", "Categorize an import", {
    "type": "object",
    "properties": {
        "import_id": {"type": "string"},
        "category": {"type": "string"},
        "is_dangerous": {"type": "boolean"},
        "danger_reason": {"type": "string"}
    },
    "required": ["import_id", "category"]
})
async def categorize_import(**kwargs) -> dict:
    return await import_tools.categorize_import(**kwargs)

@tool("find_dangerous_apis", "Find dangerous API usage", {
    "type": "object",
    "properties": {"driver_id": {"type": "string"}},
    "required": ["driver_id"]
})
async def find_dangerous_apis(**kwargs) -> dict:
    return await import_tools.find_dangerous_apis(**kwargs)

@tool("get_exports", "Get driver exports", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "prefix": {"type": "string"},
        "dangerous_only": {"type": "boolean"}
    },
    "required": ["driver_id"]
})
async def get_exports(**kwargs) -> dict:
    return await export_tools.get_exports(**kwargs)

@tool("document_export", "Document an export function", {
    "type": "object",
    "properties": {
        "export_id": {"type": "string"},
        "description": {"type": "string"},
        "return_type": {"type": "string"},
        "parameters": {"type": "string"},
        "calling_convention": {"type": "string"},
        "category": {"type": "string"},
        "is_dangerous": {"type": "boolean"},
        "danger_reason": {"type": "string"},
        "decompiled_code": {"type": "string"}
    },
    "required": ["export_id"]
})
async def document_export(**kwargs) -> dict:
    return await export_tools.document_export(**kwargs)

@tool("analyze_exports", "Analyze all exports for security patterns and VirtualBox BYOVD indicators", {
    "type": "object",
    "properties": {
        "driver_id": {"type": "string"},
        "auto_flag_dangerous": {"type": "boolean", "default": True}
    },
    "required": ["driver_id"]
})
async def analyze_exports(**kwargs) -> dict:
    return await export_tools.analyze_exports(**kwargs)

# ============================================
# MCP SERVER HANDLERS
# ============================================

@app.list_tools()
async def list_tools() -> list[Tool]:
    """List all available tools"""
    tools = []
    for name, tool_info in TOOLS.items():
        tools.append(Tool(
            name=name,
            description=tool_info['description'],
            inputSchema=tool_info['input_schema']
        ))
    return tools

@app.call_tool()
async def call_tool(name: str, arguments: dict) -> list[TextContent]:
    """Execute a tool"""
    if name not in TOOLS:
        raise ValueError(f"Unknown tool: {name}")

    try:
        handler = TOOLS[name]['handler']
        result = await handler(**arguments)

        # Format result as JSON
        import json
        result_text = json.dumps(result, indent=2, default=str)

        return [TextContent(
            type="text",
            text=result_text
        )]
    except Exception as e:
        logger.error(f"Error executing tool {name}: {e}", exc_info=True)
        return [TextContent(
            type="text",
            text=f"Error: {str(e)}"
        )]

# ============================================
# SERVER LIFECYCLE
# ============================================

async def initialize_server():
    """Initialize server components"""
    logger.info("Initializing Driver RE MCP Server...")

    # Initialize database
    db_path = settings.database_path_obj
    db_path.parent.mkdir(parents=True, exist_ok=True)

    db_manager = init_database(db_path)

    # Initialize schema if needed
    schema_path = Path(__file__).parent / "database" / "schema.sql"
    if schema_path.exists():
        try:
            await db_manager.init_db(schema_path)
            logger.info("Database schema initialized")
        except Exception as e:
            logger.warning(f"Schema init skipped (may already exist): {e}")

    logger.info("Server initialized successfully")

async def shutdown_server():
    """Cleanup on shutdown"""
    logger.info("Shutting down Driver RE MCP Server...")
    db = get_db()
    await db.close()
    logger.info("Server shut down")

def main():
    """Main entry point"""
    asyncio.run(run_server())

async def run_server():
    """Run the MCP server"""
    await initialize_server()

    try:
        async with stdio_server() as (read_stream, write_stream):
            await app.run(
                read_stream,
                write_stream,
                app.create_initialization_options()
            )
    finally:
        await shutdown_server()

if __name__ == "__main__":
    main()
