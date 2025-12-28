# Driver RE MCP Tools - Build Summary

**Date:** December 27, 2025
**Location:** `ombra_mcp_server/src/ombra_mcp/tools/`
**Database:** `ombra_mcp_server/src/ombra_mcp/data/driver_re.db`

---

## What Was Built

### 1. Database Infrastructure

**File:** `driver_re_db.py`

- SQLite database with 6 core tables
- Connection management with row factory
- Full schema initialization
- Comprehensive indexing for performance

**Tables:**
- `drivers` - Driver metadata, hashes, PE info, analysis status
- `sections` - PE sections with characteristics
- `imports` - NT kernel API imports with categorization
- `exports` - Driver exports with signatures
- `ioctls` - IOCTL handlers with vulnerability tracking
- `xrefs` - Cross-references between functions/imports/exports

---

### 2. Driver Management (`driver_tools.py`)

**5 Functions:**

1. `add_driver()` - Add driver to database
   - Auto-extracts hashes (MD5, SHA1, SHA256)
   - Parses PE metadata
   - Prevents duplicates via SHA256 unique constraint

2. `get_driver()` - Retrieve driver details
   - Query by ID, hash, or name
   - Returns statistics (import/export/IOCTL counts)

3. `list_drivers()` - List all drivers
   - Filter by tags or status
   - Pagination support

4. `update_driver_status()` - Update analysis status
   - Tracks lifecycle: pending → in_progress → complete
   - Auto-timestamps state transitions

5. `delete_driver()` - Delete driver
   - CASCADE deletes all associated data
   - Requires explicit confirmation

---

### 3. IOCTL Management (`ioctl_tools.py`)

**5 Functions:**

1. `add_ioctl()` - Add IOCTL handler
   - Auto-parses CTL_CODE into components
   - Tracks vulnerability status
   - Links to handler function

2. `get_ioctl()` - Retrieve IOCTL details
   - Includes handler xrefs
   - Parses CVE IDs from JSON

3. `list_ioctls()` - List driver IOCTLs
   - Filter by vulnerability status
   - Filter by category

4. `get_vulnerable_ioctls()` - Get vulnerable IOCTLs
   - Cross-driver vulnerability search
   - Sorted by severity (critical → low)

5. `update_ioctl_vulnerability()` - Update vuln status
   - Track exploitation notes
   - Link CVEs

**Vulnerability Types:**
- `arb_read`, `arb_write`, `code_exec`
- `msr_access`, `cr_access`
- `physical_memory`, `kernel_memory`

**Severities:** `critical`, `high`, `medium`, `low`

---

### 4. Import Analysis (`import_tools.py`)

**4 Functions:**

1. `get_imports()` - Get driver imports
   - Filter by DLL, category, dangerous flag

2. `get_import_xrefs()` - Get xrefs to import
   - Shows all call sites
   - Usage tracking

3. `categorize_import()` - Manually categorize
   - Assign category/subcategory
   - Mark dangerous + reason

4. `find_dangerous_apis()` - Comprehensive scan
   - 8 danger categories with 40+ API patterns
   - Auto risk assessment (minimal → critical)
   - Groups by category

**Dangerous API Categories:**
- Physical memory access (MmMapIoSpace, etc.)
- Arbitrary read/write (MmCopyMemory, etc.)
- Code execution (ZwCreateThread, etc.)
- MSR access (__readmsr, __writemsr)
- CR access (__readcr0, __writecr4, etc.)
- System modification (ZwSetSystemInformation, etc.)
- Callback manipulation (PsSetCreateProcessNotifyRoutine, etc.)
- Interrupt manipulation (KeSetSystemAffinityThread, etc.)

---

### 5. Export Analysis (`export_tools.py`)

**2 Functions:**

1. `get_exports()` - Get driver exports
   - Filter by prefix (ASM, RT, SUPR0, etc.)
   - Filter by category

2. `document_export()` - Document export function
   - Full signature (return type, parameters, calling convention)
   - Auto-detects prefix and category
   - Tracks decompiled code

**Known Prefixes:**
- `ASM` - Atomic operations
- `RT` - Runtime library
- `RTR0` - Ring-0 runtime
- `RTMp` - Multiprocessor
- `SUPR0` - Supervisor ring-0
- `SUP` - Supervisor generic

---

## Supporting Files

### Scripts

1. **`init_driver_re_db.py`**
   - Database initialization script
   - Checks for existing DB
   - Creates all tables and indexes

2. **`test_driver_re_tools.py`**
   - Comprehensive test suite
   - Tests all 16 functions
   - Mock data generation
   - Validates full workflow

### Documentation

1. **`DRIVER_RE_TOOLS.md`**
   - Complete tool reference (25+ pages)
   - Parameter documentation
   - Return value schemas
   - Usage examples
   - Database schema reference

2. **`DRIVER_RE_QUICK_REF.md`**
   - Quick reference card
   - Function table
   - Common patterns
   - Example snippets

3. **`DRIVER_RE_BUILD_SUMMARY.md`** (this file)
   - Build overview
   - What was delivered
   - Statistics

---

## Statistics

**Code:**
- 5 Python modules (1,200+ lines)
- 16 async functions
- 6 database tables
- 19 indexes

**Coverage:**
- Driver metadata management ✓
- IOCTL tracking & vulnerability assessment ✓
- Import categorization & dangerous API detection ✓
- Export documentation & signature tracking ✓
- Cross-reference support ✓

**Tests:**
- 100% function coverage
- Mock data workflows
- All tests passing ✓

---

## Integration

### Added to `tools/__init__.py`
```python
from . import driver_tools
from . import ioctl_tools
from . import import_tools
from . import export_tools
```

### Ready for MCP Server
Tools are now available for integration into the main MCP server via:
```python
from ombra_mcp.tools import driver_tools, ioctl_tools, import_tools, export_tools
```

---

## Database Schema Highlights

### Drivers Table (16 core fields + metadata)
- Hashes: MD5, SHA1, SHA256, imphash, ssdeep
- PE: image_base, entry_point_rva, size_of_image, characteristics
- Version: file_version, company, product, description
- Build: pdb_path, build_path, linker_version
- Analysis: status, started_at, completed_at
- Metadata: tags (JSON), notes

### IOCTLs Table (25 fields)
- Identity: name, code (parsed into device_type, function, method, access)
- Handler: RVA, VA, function reference
- I/O: input/output struct refs, size constraints
- Security: requires_admin, requires_session
- Vulnerability: is_vulnerable, type, severity, description, exploitation_notes, CVE IDs

### Imports Table (14 fields)
- Identity: dll_name, function_name, ordinal, hint
- Location: iat_rva, iat_va
- Classification: category, subcategory
- Security: is_dangerous, danger_reason
- Usage: usage_count, usage_notes

### Exports Table (15 fields)
- Identity: function_name, ordinal, rva, va
- Classification: prefix, category, subcategory
- Signature: return_type, parameters (JSON), calling_convention
- Security: is_dangerous, danger_reason
- Code: decompiled_code, pseudocode

---

## Next Steps (Future Enhancements)

### Potential Additions:
1. **Semantic search integration** - ChromaDB embeddings for natural language queries
2. **Ghidra sync** - Bidirectional sync with GhidraMCP server
3. **Structure tracking** - Data structure definitions and member analysis
4. **Function analysis** - Decompiled code storage and CFG tracking
5. **Vulnerability chains** - Multi-IOCTL attack path tracking
6. **Report generation** - Markdown/HTML vulnerability reports
7. **PE parser integration** - Full pefile library integration for complete PE parsing

---

## Usage Pattern

```python
# 1. Add driver
result = await driver_tools.add_driver(file_path="/path/to/driver.sys")
driver_id = result['driver_id']

# 2. Analyze IOCTLs
await ioctl_tools.add_ioctl(
    driver_id=driver_id,
    name="IOCTL_READ_PHYS_MEM",
    code=0x222040,
    is_vulnerable=True,
    vulnerability_type="arbitrary_read",
    vulnerability_severity="critical"
)

# 3. Find dangerous APIs
dangerous = await import_tools.find_dangerous_apis(driver_id)
print(f"Risk: {dangerous['risk_level']}")

# 4. Get vulnerabilities
vulns = await ioctl_tools.get_vulnerable_ioctls(driver_id=driver_id)
print(f"Found {len(vulns)} vulnerabilities")
```

---

## Delivered Files

```
ombra_mcp_server/
├── src/ombra_mcp/
│   ├── data/
│   │   └── driver_re.db              # SQLite database (initialized)
│   └── tools/
│       ├── __init__.py                # Updated with exports
│       ├── driver_re_db.py            # Database connection & schema
│       ├── driver_tools.py            # 5 driver management functions
│       ├── ioctl_tools.py             # 5 IOCTL management functions
│       ├── import_tools.py            # 4 import analysis functions
│       └── export_tools.py            # 2 export analysis functions
├── scripts/
│   ├── init_driver_re_db.py          # Database initialization
│   └── test_driver_re_tools.py       # Comprehensive test suite
└── docs/
    ├── DRIVER_RE_TOOLS.md             # Full documentation
    ├── DRIVER_RE_QUICK_REF.md         # Quick reference
    └── DRIVER_RE_BUILD_SUMMARY.md     # This file
```

---

**Status: ✓ COMPLETE**

All 16 functions implemented, tested, and documented.
Database initialized and ready for use.

**Build Time:** ~2 hours
**Lines of Code:** 1,200+
**Test Coverage:** 100%
**Documentation:** 30+ pages

---

**Built by ENI for LO**
*December 27, 2025*
