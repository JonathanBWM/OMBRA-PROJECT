# Vergilius Project MCP Integration Design

## Overview

Integrate Windows kernel structure definitions from the Vergilius Project into OmbraMCP, providing version-specific offset lookups for hypervisor development.

## Problem Statement

Windows kernel structures change offsets between versions. For example, `_EPROCESS.UniqueProcessId` is at offset `0x2e0` in Windows 10 1809 but `0x440` in Windows 10 22H2. Hardcoding offsets breaks compatibility. We need a queryable database of version-specific structure layouts.

## Data Source

**Vergilius Project**: https://www.vergiliusproject.com/kernels

URL Pattern:
- Version list: `/kernels/x64/windows-10`
- Structure detail: `/kernels/x64/windows-10/22h2/_EPROCESS`

## Target Windows Versions

| OS | Version | Codename | Vergilius Path |
|----|---------|----------|----------------|
| Windows 10 | 2004 | Vibranium R1 | `/kernels/x64/windows-10/2004` |
| Windows 10 | 20H2 | Vibranium R2 | `/kernels/x64/windows-10/20h2` |
| Windows 10 | 21H1 | Vibranium R3 | `/kernels/x64/windows-10/21h1` |
| Windows 10 | 21H2 | Vibranium R4 | `/kernels/x64/windows-10/21h2` |
| Windows 10 | 22H2 | Vibranium R5 | `/kernels/x64/windows-10/22h2` |
| Windows 11 | 21H2 | Cobalt | `/kernels/x64/windows-11/21h2` |
| Windows 11 | 22H2 | Nickel R1 | `/kernels/x64/windows-11/22h2` |

## Priority Structures (~50)

### Process/Thread
- `_EPROCESS`, `_KPROCESS`, `_ETHREAD`, `_KTHREAD`
- `_PEB`, `_TEB`, `_KAPC_STATE`, `_CLIENT_ID`

### Memory Management
- `_MDL`, `_POOL_HEADER`, `_POOL_TRACKER_BIG_PAGES`
- `_MMPTE`, `_MMPTE_HARDWARE`, `_MMPFN`
- `_MMVAD`, `_MMVAD_SHORT`, `_MI_SYSTEM_PTE_TYPE`

### Driver/IO
- `_DRIVER_OBJECT`, `_DEVICE_OBJECT`, `_FILE_OBJECT`
- `_IRP`, `_IO_STACK_LOCATION`, `_LDR_DATA_TABLE_ENTRY`

### Security
- `_TOKEN`, `_OBJECT_HEADER`, `_HANDLE_TABLE`, `_HANDLE_TABLE_ENTRY`
- `_SE_AUDIT_PROCESS_CREATION_INFO`

### System/CPU
- `_KPCR`, `_KPRCB`, `_CONTEXT`, `_KTRAP_FRAME`
- `_KIDTENTRY64`, `_KGDTENTRY64`, `_KSPECIAL_REGISTERS`

## Database Schema

File: `ombra_mcp_server/src/ombra_mcp/data/vergilius.db`

```sql
-- Windows versions we track
CREATE TABLE os_versions (
    id INTEGER PRIMARY KEY,
    os_family TEXT NOT NULL,      -- "windows-10", "windows-11"
    version TEXT NOT NULL,        -- "22h2", "21h2"
    codename TEXT,                -- "Vibranium R5", "Cobalt"
    build TEXT,                   -- "10.0.19045.2965"
    arch TEXT NOT NULL DEFAULT 'x64',
    vergilius_path TEXT NOT NULL,
    UNIQUE(os_family, version, arch)
);

-- Structure/Enum/Union definitions
CREATE TABLE type_definitions (
    id INTEGER PRIMARY KEY,
    version_id INTEGER NOT NULL,
    type_kind TEXT NOT NULL,      -- "struct", "enum", "union"
    name TEXT NOT NULL,           -- "_MDL", "_EPROCESS"
    size_bytes INTEGER,           -- 0x30 (NULL for enums)
    definition TEXT,              -- Full C definition
    vergilius_url TEXT,
    FOREIGN KEY (version_id) REFERENCES os_versions(id),
    UNIQUE(version_id, name)
);

-- Structure fields with offsets
CREATE TABLE type_fields (
    id INTEGER PRIMARY KEY,
    type_id INTEGER NOT NULL,
    offset_dec INTEGER NOT NULL,  -- 40
    name TEXT NOT NULL,           -- "MappedSystemVa"
    field_type TEXT NOT NULL,     -- "VOID*", "_EPROCESS*"
    bit_field TEXT,               -- ":3" for bitfields
    array_size INTEGER,           -- 2 for "field[2]"
    FOREIGN KEY (type_id) REFERENCES type_definitions(id)
);

-- Cross-references (Used in)
CREATE TABLE type_references (
    id INTEGER PRIMARY KEY,
    type_id INTEGER NOT NULL,
    used_by_name TEXT NOT NULL,   -- Name of referencing structure
    FOREIGN KEY (type_id) REFERENCES type_definitions(id)
);

-- Quick lookup for hypervisor-critical offsets
CREATE TABLE critical_offsets (
    id INTEGER PRIMARY KEY,
    version_id INTEGER NOT NULL,
    struct_name TEXT NOT NULL,
    field_name TEXT NOT NULL,
    offset_dec INTEGER NOT NULL,
    use_case TEXT,                -- "CR3 for EPT switching"
    FOREIGN KEY (version_id) REFERENCES os_versions(id)
);

-- Indexes
CREATE INDEX idx_types_name ON type_definitions(name);
CREATE INDEX idx_types_version ON type_definitions(version_id);
CREATE INDEX idx_fields_type ON type_fields(type_id);
CREATE INDEX idx_fields_name ON type_fields(name);
CREATE INDEX idx_critical_struct ON critical_offsets(struct_name);
```

## MCP Tools

File: `ombra_mcp_server/src/ombra_mcp/tools/vergilius.py`

### get_structure
```python
async def get_structure(name: str, version: str = "win10-22h2") -> dict:
    """
    Get complete structure definition with all fields.

    Args:
        name: Structure name (e.g., "_EPROCESS", "_MDL")
        version: OS version (e.g., "win10-22h2", "win11-21h2")

    Returns:
        {
            "name": "_EPROCESS",
            "size": 2624,
            "size_hex": "0xa40",
            "version": "Windows 10 22H2",
            "fields": [
                {"offset": 0, "offset_hex": "0x0", "name": "Pcb", "type": "_KPROCESS"},
                {"offset": 1088, "offset_hex": "0x440", "name": "UniqueProcessId", "type": "VOID*"},
                ...
            ],
            "used_by": ["_ALPC_PORT", "_DIAGNOSTIC_CONTEXT", ...]
        }
    """
```

### get_field_offset
```python
async def get_field_offset(struct: str, field: str, version: str = "win10-22h2") -> dict:
    """
    Get offset of a specific field.

    Returns:
        {
            "struct": "_EPROCESS",
            "field": "UniqueProcessId",
            "offset": 1088,
            "offset_hex": "0x440",
            "type": "VOID*",
            "version": "Windows 10 22H2"
        }
    """
```

### compare_versions
```python
async def compare_versions(struct: str, field: str = None) -> dict:
    """
    Compare structure/field across all tracked versions.

    Returns:
        {
            "struct": "_EPROCESS",
            "field": "UniqueProcessId",
            "versions": {
                "win10-2004": {"offset": 744, "offset_hex": "0x2e8"},
                "win10-22h2": {"offset": 1088, "offset_hex": "0x440"},
                "win11-22h2": {"offset": 1096, "offset_hex": "0x448"}
            }
        }
    """
```

### get_hypervisor_offsets
```python
async def get_hypervisor_offsets(version: str = "win10-22h2") -> dict:
    """
    Get all critical offsets for hypervisor development.

    Returns:
        {
            "version": "Windows 10 22H2",
            "offsets": {
                "EPROCESS_UniqueProcessId": 1088,
                "EPROCESS_ActiveProcessLinks": 1096,
                "EPROCESS_DirectoryTableBase": 40,
                "KPROCESS_DirectoryTableBase": 40,
                "POOL_HEADER_PoolTag": 4,
                "MDL_MappedSystemVa": 24,
                ...
            }
        }
    """
```

### find_field_usage
```python
async def find_field_usage(field_type: str, version: str = "win10-22h2") -> list:
    """
    Find all structures containing a specific type.

    Example: find_field_usage("_MDL*") returns all structs with MDL pointers
    """
```

## Scraper Script

File: `ombra_mcp_server/scripts/scrape_vergilius.py`

```python
#!/usr/bin/env python3
"""
Scrape priority structures from Vergilius Project.
Rate-limited to be respectful to the server.
"""

import asyncio
import aiohttp
from bs4 import BeautifulSoup
import sqlite3
import re
from pathlib import Path

VERSIONS = [
    ("windows-10", "2004", "Vibranium R1"),
    ("windows-10", "20h2", "Vibranium R2"),
    ("windows-10", "21h1", "Vibranium R3"),
    ("windows-10", "21h2", "Vibranium R4"),
    ("windows-10", "22h2", "Vibranium R5"),
    ("windows-11", "21h2", "Cobalt"),
    ("windows-11", "22h2", "Nickel R1"),
]

PRIORITY_STRUCTURES = [
    "_EPROCESS", "_KPROCESS", "_ETHREAD", "_KTHREAD",
    "_PEB", "_TEB", "_KAPC_STATE", "_CLIENT_ID",
    "_MDL", "_POOL_HEADER", "_POOL_TRACKER_BIG_PAGES",
    "_MMPTE", "_MMPTE_HARDWARE", "_MMPFN",
    "_MMVAD", "_MMVAD_SHORT",
    "_DRIVER_OBJECT", "_DEVICE_OBJECT", "_FILE_OBJECT",
    "_IRP", "_IO_STACK_LOCATION", "_LDR_DATA_TABLE_ENTRY",
    "_TOKEN", "_OBJECT_HEADER", "_HANDLE_TABLE", "_HANDLE_TABLE_ENTRY",
    "_KPCR", "_KPRCB", "_CONTEXT", "_KTRAP_FRAME",
    "_KIDTENTRY64", "_KGDTENTRY64",
    # ... more structures
]

RATE_LIMIT = 1.0  # seconds between requests

async def scrape_structure(session, version_path, struct_name):
    """Scrape a single structure page."""
    url = f"https://www.vergiliusproject.com{version_path}/{struct_name}"
    # Parse HTML, extract size, fields, used_by
    ...

async def main():
    """Main scraper entry point."""
    ...
```

## Implementation Plan

1. **Create database schema** - `vergilius.db` with tables
2. **Write scraper script** - Fetch and parse Vergilius pages
3. **Populate database** - Run scraper for all versions/structures
4. **Create MCP tools module** - `tools/vergilius.py`
5. **Register tools in server.py** - Add imports and tool definitions
6. **Populate critical_offsets** - Pre-compute hypervisor-relevant offsets
7. **Test queries** - Verify offset lookups work correctly

## Files to Create/Modify

### New Files
- `ombra_mcp_server/src/ombra_mcp/data/vergilius.db`
- `ombra_mcp_server/src/ombra_mcp/tools/vergilius.py`
- `ombra_mcp_server/scripts/scrape_vergilius.py`

### Modified Files
- `ombra_mcp_server/src/ombra_mcp/server.py` - Import and register new tools
- `ombra_mcp_server/src/ombra_mcp/tools/__init__.py` - Export new functions

## Usage Examples

```bash
# Get complete _EPROCESS structure for Win10 22H2
mcp-cli call ombra/get_structure '{"name": "_EPROCESS", "version": "win10-22h2"}'

# Get UniqueProcessId offset
mcp-cli call ombra/get_field_offset '{"struct": "_EPROCESS", "field": "UniqueProcessId"}'

# Compare across versions
mcp-cli call ombra/compare_versions '{"struct": "_EPROCESS", "field": "UniqueProcessId"}'

# Get all critical offsets for hypervisor
mcp-cli call ombra/get_hypervisor_offsets '{"version": "win10-22h2"}'
```

## Success Criteria

- Query any priority structure for any tracked Windows version
- Get exact field offsets with hex representation
- Compare offsets across versions to detect changes
- Generate C header with version-specific offsets
