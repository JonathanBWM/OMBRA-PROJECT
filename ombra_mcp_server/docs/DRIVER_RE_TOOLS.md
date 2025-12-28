# Driver Reverse Engineering MCP Tools

Comprehensive toolkit for Windows kernel driver analysis and vulnerability research.

## Overview

The Driver RE tools provide a complete database-backed system for analyzing Windows kernel drivers, tracking IOCTLs, imports, exports, and vulnerabilities. All data is stored in SQLite with full support for cross-referencing and semantic search (when integrated).

## Database Location

```
ombra_mcp_server/src/ombra_mcp/data/driver_re.db
```

Initialize with:
```bash
python3 ombra_mcp_server/scripts/init_driver_re_db.py
```

## Tool Modules

### 1. Driver Tools (`driver_tools`)

Manage driver metadata and analysis lifecycle.

#### `add_driver`
Add a new driver to the database.

**Parameters:**
- `file_path` (str): Path to the .sys file
- `analyzed_name` (Optional[str]): Name for analysis (defaults to filename)
- `tags` (Optional[List[str]]): Tags for categorization
- `notes` (Optional[str]): Analysis notes

**Auto-extracts:**
- MD5, SHA1, SHA256 hashes
- PE metadata (image base, entry point, sections)
- File size and timestamps

**Returns:**
```json
{
  "success": true,
  "driver_id": "uuid",
  "driver_name": "example.sys",
  "sha256": "...",
  "file_size": 123456,
  "message": "Driver added successfully"
}
```

**Example:**
```python
result = await driver_tools.add_driver(
    file_path="/path/to/Ld9BoxSup.sys",
    analyzed_name="Ld9BoxSup",
    tags=["vbox", "byovd"],
    notes="VirtualBox fork driver for BYOVD exploitation"
)
driver_id = result['driver_id']
```

#### `get_driver`
Retrieve driver details by ID, hash, or name.

**Parameters:**
- `driver_id` (Optional[str]): UUID
- `sha256` (Optional[str]): SHA256 hash
- `name` (Optional[str]): Partial name match

**Returns:**
Full driver metadata plus statistics:
- `import_count`, `export_count`, `ioctl_count`
- `vulnerable_ioctl_count`, `section_count`

**Example:**
```python
driver = await driver_tools.get_driver(driver_id="...")
print(f"{driver['original_name']} has {driver['ioctl_count']} IOCTLs")
print(f"Vulnerable: {driver['vulnerable_ioctl_count']}")
```

#### `list_drivers`
List all drivers with filtering.

**Parameters:**
- `tags` (Optional[List[str]]): Filter by tags
- `status` (Optional[str]): pending, in_progress, complete
- `limit` (int): Default 50
- `offset` (int): Pagination offset

**Example:**
```python
byovd_drivers = await driver_tools.list_drivers(tags=["byovd"])
```

#### `update_driver_status`
Update analysis status.

**Parameters:**
- `driver_id` (str)
- `status` (str): pending, in_progress, complete
- `notes` (Optional[str]): Updated notes

**Auto-updates:**
- `analysis_started_at` when status → in_progress
- `analysis_completed_at` when status → complete

**Example:**
```python
await driver_tools.update_driver_status(
    driver_id="...",
    status="complete",
    notes="Full analysis complete - 15 IOCTLs documented, 3 critical vulns"
)
```

#### `delete_driver`
Delete driver and ALL associated data (CASCADE).

**Parameters:**
- `driver_id` (str)
- `confirm` (bool): Must be True

**Deletes:**
- Driver metadata
- Sections
- Imports
- Exports
- IOCTLs
- Cross-references

**Example:**
```python
result = await driver_tools.delete_driver(driver_id="...", confirm=True)
```

---

### 2. IOCTL Tools (`ioctl_tools`)

Manage IOCTL handlers and vulnerability tracking.

#### `add_ioctl`
Add an IOCTL to a driver.

**Parameters:**
- `driver_id` (str): Parent driver UUID
- `name` (str): IOCTL name (e.g., SUP_IOCTL_MSR_PROBER)
- `code` (Optional[int]): Numeric IOCTL code
- `description` (Optional[str])
- `handler_rva` (Optional[int]): Handler function RVA
- `input_struct` (Optional[str]): C struct definition or name
- `output_struct` (Optional[str])
- `min_input_size`, `max_input_size` (Optional[int])
- `min_output_size`, `max_output_size` (Optional[int])
- `requires_admin` (bool)
- `is_vulnerable` (bool)
- `vulnerability_type` (Optional[str]): arb_read, arb_write, code_exec, etc.
- `vulnerability_severity` (Optional[str]): critical, high, medium, low
- `vulnerability_description` (Optional[str])
- `exploitation_notes` (Optional[str])

**Auto-parses IOCTL code:**
Extracts device_type, function_code, method, access from CTL_CODE macro.

**Example:**
```python
await ioctl_tools.add_ioctl(
    driver_id=driver_id,
    name="SUP_IOCTL_MSR_PROBER",
    code=0x00220000 | (1 << 2) | (3 << 14),
    description="Read/write Model Specific Registers",
    handler_rva=0x15200,
    min_input_size=48,
    max_input_size=48,
    requires_admin=True,
    is_vulnerable=True,
    vulnerability_type="msr_access",
    vulnerability_severity="critical",
    vulnerability_description="Allows arbitrary MSR read/write from usermode",
    exploitation_notes="Can read VMX MSRs to detect hypervisor presence"
)
```

#### `get_ioctl`
Retrieve IOCTL details.

**Parameters:**
- `ioctl_id` (Optional[str])
- `driver_id` + `name` (Optional[str])
- `driver_id` + `code` (Optional[int])

**Returns:**
Full IOCTL details including:
- Parsed CTL_CODE components
- Handler xrefs (if handler_rva set)
- Driver name
- CVE IDs (if applicable)

**Example:**
```python
ioctl = await ioctl_tools.get_ioctl(driver_id="...", name="SUP_IOCTL_MSR_PROBER")
print(f"Handler at RVA: 0x{ioctl['handler_rva']:X}")
print(f"Xrefs to handler: {len(ioctl['handler_xrefs'])}")
```

#### `list_ioctls`
List all IOCTLs for a driver.

**Parameters:**
- `driver_id` (str)
- `vulnerable_only` (bool): Filter to vulnerable only
- `category` (Optional[str]): Filter by vulnerability_type

**Example:**
```python
all_ioctls = await ioctl_tools.list_ioctls(driver_id="...")
vuln_ioctls = await ioctl_tools.list_ioctls(driver_id="...", vulnerable_only=True)
```

#### `get_vulnerable_ioctls`
Get all vulnerable IOCTLs (across all drivers or filtered).

**Parameters:**
- `driver_id` (Optional[str]): Filter to specific driver
- `severity` (Optional[str]): critical, high, medium, low
- `vulnerability_type` (Optional[str]): arb_read, arb_write, code_exec, etc.

**Returns:**
List sorted by severity (critical first), then driver name, then function code.

**Example:**
```python
# Get all critical vulnerabilities
critical = await ioctl_tools.get_vulnerable_ioctls(severity="critical")

# Get all arbitrary memory access vulns
memory_vulns = await ioctl_tools.get_vulnerable_ioctls(
    vulnerability_type="arb_read"
)
```

#### `update_ioctl_vulnerability`
Update vulnerability status and details.

**Parameters:**
- `ioctl_id` (str)
- `is_vulnerable` (bool)
- `vulnerability_type` (Optional[str])
- `vulnerability_severity` (Optional[str])
- `vulnerability_description` (Optional[str])
- `exploitation_notes` (Optional[str])
- `cve_ids` (Optional[List[str]])

**Example:**
```python
await ioctl_tools.update_ioctl_vulnerability(
    ioctl_id="...",
    is_vulnerable=True,
    vulnerability_type="arbitrary_read",
    vulnerability_severity="critical",
    cve_ids=["CVE-2023-12345"]
)
```

---

### 3. Import Tools (`import_tools`)

Analyze Windows kernel API imports.

#### `get_imports`
Get imports for a driver.

**Parameters:**
- `driver_id` (str)
- `dll` (Optional[str]): Filter by DLL (e.g., "ntoskrnl.exe")
- `category` (Optional[str]): memory_management, process_thread, io, etc.
- `dangerous_only` (bool): Only dangerous APIs

**Categories:**
- `memory_management`: Physical/virtual memory APIs
- `process_thread`: Process/thread manipulation
- `io`: I/O operations
- `synchronization`: Locks, events, semaphores
- `system_info`: System information query/set
- `registry`: Registry operations
- `object_management`: Object manager APIs
- `callbacks`: Callback registration
- `cpu_hardware`: MSR, CR, CPUID access

**Example:**
```python
all_imports = await import_tools.get_imports(driver_id="...")
memory_apis = await import_tools.get_imports(
    driver_id="...",
    category="memory_management"
)
dangerous = await import_tools.get_imports(
    driver_id="...",
    dangerous_only=True
)
```

#### `get_import_xrefs`
Get all cross-references to an import.

**Parameters:**
- `import_id` (str)

**Returns:**
```json
{
  "import": {...},
  "xref_count": 5,
  "xrefs": [
    {"from_rva": 0x1000, "from_function_id": "...", "xref_type": "call"},
    ...
  ]
}
```

**Example:**
```python
xrefs = await import_tools.get_import_xrefs(import_id="...")
print(f"MmMapIoSpace is called from {xrefs['xref_count']} locations")
```

#### `categorize_import`
Manually categorize an import and mark security relevance.

**Parameters:**
- `import_id` (str)
- `category` (str)
- `subcategory` (Optional[str])
- `is_dangerous` (bool)
- `danger_reason` (Optional[str])
- `usage_notes` (Optional[str])

**Example:**
```python
await import_tools.categorize_import(
    import_id="...",
    category="memory_management",
    subcategory="physical_memory",
    is_dangerous=True,
    danger_reason="Allows arbitrary physical memory mapping"
)
```

#### `find_dangerous_apis`
Comprehensive dangerous API detection.

**Detects:**
- Physical memory access (MmMapIoSpace, MmGetPhysicalAddress, etc.)
- Arbitrary read/write (MmCopyMemory, ZwReadVirtualMemory, etc.)
- Code execution (ZwCreateThread, KeInsertQueueApc, etc.)
- MSR access (__readmsr, __writemsr)
- CR access (__readcr0, __writecr4, etc.)
- System modification (ZwSetSystemInformation, ZwLoadDriver, etc.)
- Callback manipulation (PsSetCreateProcessNotifyRoutine, etc.)
- Interrupt manipulation (KeSetSystemAffinityThread, etc.)

**Returns:**
```json
{
  "driver_id": "...",
  "driver_name": "example.sys",
  "total_imports": 150,
  "total_dangerous": 12,
  "risk_level": "critical",
  "dangerous_by_category": {
    "physical_memory_access": {
      "count": 3,
      "apis": [
        {"function_name": "MmMapIoSpace", "dll_name": "ntoskrnl.exe", ...},
        ...
      ]
    },
    "msr_access": {...},
    ...
  }
}
```

**Risk levels:** minimal, low, medium, high, critical

**Example:**
```python
dangerous = await import_tools.find_dangerous_apis(driver_id="...")

print(f"Risk Level: {dangerous['risk_level']}")
for category, data in dangerous['dangerous_by_category'].items():
    print(f"\n{category}: {data['count']} APIs")
    for api in data['apis']:
        print(f"  - {api['function_name']}")
```

---

### 4. Export Tools (`export_tools`)

Analyze driver export functions.

#### `get_exports`
Get exports for a driver.

**Parameters:**
- `driver_id` (str)
- `prefix` (Optional[str]): ASM, RT, RTR0, RTMp, SUPR0, SUP, Nt, Zw
- `category` (Optional[str])
- `dangerous_only` (bool)

**Known Prefixes:**
- `ASM`: Atomic operations (ASMAtomicIncU32, ASMBitSet, etc.)
- `RT`: Runtime library (RTMemAlloc, RTStrCopy, etc.)
- `RTR0`: Ring-0 runtime (RTR0MemObjAllocPhys, etc.)
- `RTMp`: Multiprocessor (RTMpOnAll, RTMpGetCount, etc.)
- `SUPR0`: Supervisor ring-0 (SUPR0PhysMemRead, SUPR0LdrLoad, etc.)
- `SUP`: Supervisor generic (SUP_IOCTL_*, etc.)

**Example:**
```python
all_exports = await export_tools.get_exports(driver_id="...")
supr0 = await export_tools.get_exports(driver_id="...", prefix="SUPR0")
```

#### `document_export`
Document an export function with signature and analysis.

**Parameters:**
- `export_id` (str)
- `description` (Optional[str])
- `return_type` (Optional[str]): e.g., "NTSTATUS"
- `parameters` (Optional[List[Dict]]): `[{"name": "...", "type": "...", "description": "..."}, ...]`
- `calling_convention` (Optional[str]): fastcall, stdcall, cdecl
- `is_dangerous` (bool)
- `danger_reason` (Optional[str])
- `decompiled_code` (Optional[str])

**Auto-detects:**
- Prefix from function name
- Category based on prefix

**Example:**
```python
await export_tools.document_export(
    export_id="...",
    description="Read physical memory at specified address",
    return_type="NTSTATUS",
    parameters=[
        {"name": "PhysAddr", "type": "PHYSICAL_ADDRESS", "description": "Physical address"},
        {"name": "Buffer", "type": "PVOID", "description": "Output buffer"},
        {"name": "Length", "type": "SIZE_T", "description": "Bytes to read"}
    ],
    calling_convention="fastcall",
    is_dangerous=True,
    danger_reason="Arbitrary physical memory read primitive"
)
```

---

## Database Schema

### Drivers Table
- UUID, hashes (MD5, SHA1, SHA256, imphash, ssdeep)
- PE metadata (image base, entry point, size, sections)
- Version info (file version, company, product, etc.)
- Build info (PDB path, linker version, build path)
- Analysis status (pending, in_progress, complete)
- Tags (JSON array), notes

### IOCTLs Table
- Name, code (hex + parsed components)
- Handler RVA/VA, function reference
- Input/output structures, size constraints
- Admin/session requirements
- Vulnerability status (type, severity, description, exploitation notes)
- CVE IDs (JSON array)

### Imports Table
- DLL, function name, ordinal, hint
- IAT RVA/VA
- Category, subcategory
- Dangerous flag + reason
- Usage count, notes

### Exports Table
- Function name, ordinal, RVA/VA
- Prefix, category
- Signature (return type, parameters, calling convention)
- Dangerous flag + reason
- Decompiled code

### Cross-References Table
- From/to RVA/VA
- From/to function references
- To import/export references
- Xref type (call, jump, data_ref, offset)
- Instruction context

---

## Usage Examples

### Full Analysis Workflow

```python
# 1. Add driver
result = await driver_tools.add_driver(
    file_path="/path/to/driver.sys",
    analyzed_name="TargetDriver",
    tags=["research", "vulnerable"]
)
driver_id = result['driver_id']

# 2. Update status
await driver_tools.update_driver_status(
    driver_id=driver_id,
    status="in_progress"
)

# 3. Add IOCTLs (from RE session)
await ioctl_tools.add_ioctl(
    driver_id=driver_id,
    name="IOCTL_READ_PHYS_MEM",
    code=0x222040,
    handler_rva=0x1500,
    is_vulnerable=True,
    vulnerability_type="arbitrary_read",
    vulnerability_severity="critical"
)

# 4. Find dangerous APIs
dangerous = await import_tools.find_dangerous_apis(driver_id=driver_id)
print(f"Risk: {dangerous['risk_level']}")

# 5. List vulnerable IOCTLs
vulns = await ioctl_tools.get_vulnerable_ioctls(driver_id=driver_id)
print(f"Found {len(vulns)} vulnerable IOCTLs")

# 6. Mark complete
await driver_tools.update_driver_status(
    driver_id=driver_id,
    status="complete",
    notes=f"Analysis complete. {len(vulns)} vulnerabilities documented."
)
```

### Cross-Driver Vulnerability Research

```python
# Find all drivers with physical memory access
drivers = await driver_tools.list_drivers()
for driver in drivers:
    dangerous = await import_tools.find_dangerous_apis(driver['id'])
    if 'physical_memory_access' in dangerous['dangerous_by_category']:
        print(f"{driver['original_name']}: {dangerous['total_dangerous']} dangerous APIs")

# Get all critical vulnerabilities across all drivers
critical_vulns = await ioctl_tools.get_vulnerable_ioctls(severity="critical")
print(f"Total critical vulnerabilities: {len(critical_vulns)}")
for vuln in critical_vulns:
    print(f"  {vuln['driver_name']}: {vuln['name']} ({vuln['vulnerability_type']})")
```

---

## Testing

Run comprehensive test suite:
```bash
python3 ombra_mcp_server/scripts/test_driver_re_tools.py
```

Tests all 16 functions across all 4 modules with mock data.

---

## Notes

- All functions are async (use `await`)
- Database uses SQLite with CASCADE deletes
- SHA256 is unique constraint (prevents duplicate drivers)
- IOCTL code parsing follows Windows CTL_CODE macro format
- Risk assessment is automatic based on dangerous API count
- Tags stored as JSON for flexible querying

---

**Built Dec 27, 2025 for PROJECT-OMBRA**
