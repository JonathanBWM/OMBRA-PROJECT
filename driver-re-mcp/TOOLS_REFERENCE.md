# Driver RE MCP Tools - Quick Reference

## Function Analysis Tools (6 tools)

### add_function
```python
await add_function(
    driver_id: str,           # Required
    rva: int,                 # Required
    name: Optional[str],
    size: Optional[int],
    return_type: Optional[str],
    parameters: Optional[List[Dict]],
    is_dispatch: bool = False,
    dispatch_type: Optional[str],
    decompiled: Optional[str],
    annotations: Optional[Dict]
) -> Dict
```
**Use:** Add/update function metadata with auto VA calculation and embedding generation

### get_function
```python
await get_function(
    function_id: Optional[str],
    driver_id: Optional[str],
    rva: Optional[int],
    va: Optional[int],
    name: Optional[str]
) -> Dict
```
**Use:** Retrieve function with callers, callees, and related IOCTLs

### get_function_callers
```python
await get_function_callers(function_id: str) -> List[Dict]
```
**Use:** Get all functions that call this function (with xref details)

### get_function_callees
```python
await get_function_callees(function_id: str) -> List[Dict]
```
**Use:** Get all functions/imports called by this function

### trace_call_path
```python
await trace_call_path(
    driver_id: str,
    from_function: str,  # Name or hex RVA (0x1234)
    to_function: str     # Name or hex RVA
) -> List[List[Dict]]
```
**Use:** Find all call paths from A to B (critical for taint analysis)

### find_dispatch_handlers
```python
await find_dispatch_handlers(driver_id: str) -> Dict
```
**Use:** Locate IRP_MJ_* dispatch handlers

---

## Structure Management Tools (4 tools)

### add_structure
```python
await add_structure(
    name: str,
    definition_c: str,     # C struct definition
    driver_id: Optional[str],
    struct_type: Optional[str],
    size: Optional[int],
    description: Optional[str]
) -> Dict
```
**Use:** Parse C struct and store with auto member extraction

**Example:**
```python
struct = await add_structure(
    name="IOCTL_INPUT",
    definition_c="""
    struct IOCTL_INPUT {
        ULONG Magic;
        PVOID UserBuffer;
        ULONG Size;
        UCHAR Reserved[8];
    }
    """,
    struct_type="ioctl_input"
)
```

### get_structure
```python
await get_structure(
    structure_id: Optional[str],
    name: Optional[str],
    driver_id: Optional[str]
) -> Dict
```
**Use:** Retrieve structure with all members

### list_structures
```python
await list_structures(
    driver_id: Optional[str],
    struct_type: Optional[str]
) -> List[Dict]
```
**Use:** List structures with optional filtering

### link_structure_to_ioctl
```python
await link_structure_to_ioctl(
    ioctl_id: str,
    input_struct_id: Optional[str],
    output_struct_id: Optional[str]
) -> Dict
```
**Use:** Associate input/output structures with IOCTL handler

---

## Vulnerability Tools (5 tools)

### add_vulnerability
```python
await add_vulnerability(
    driver_id: str,
    title: str,
    vulnerability_class: str,  # arbitrary_read, arbitrary_write, code_exec, etc.
    severity: str,             # critical, high, medium, low, info
    description: str,
    technical_details: Optional[str],
    affected_ioctl_id: Optional[str],
    affected_function_id: Optional[str],
    exploitation_difficulty: Optional[str],  # trivial, easy, moderate, hard, theoretical
    exploitation_requirements: Optional[str],
    exploitation_steps: Optional[List[Dict]],
    poc_code: Optional[str],
    poc_language: Optional[str],
    cve_id: Optional[str],
    cvss_score: Optional[float],
    mitigations: Optional[str],
    references: Optional[List[Dict]]
) -> Dict
```
**Use:** Document vulnerability finding with complete metadata

**Example:**
```python
vuln = await add_vulnerability(
    driver_id="abc-123",
    title="Arbitrary Physical Memory Read",
    vulnerability_class="arbitrary_read",
    severity="critical",
    description="MmMapIoSpace called with unchecked user input",
    affected_ioctl_id=ioctl_id,
    exploitation_difficulty="trivial",
    exploitation_steps=[
        {"step": 1, "description": "Send IOCTL 0x12345 with target PA"},
        {"step": 2, "description": "Read mapped buffer"}
    ],
    poc_code="HANDLE h = CreateFile(...); DeviceIoControl(...);",
    poc_language="c"
)
```

### get_vulnerability
```python
await get_vulnerability(vuln_id: str) -> Dict
```
**Use:** Retrieve full vulnerability details with affected component info

### list_vulnerabilities
```python
await list_vulnerabilities(
    driver_id: Optional[str],
    severity: Optional[str],
    vulnerability_class: Optional[str],
    status: Optional[str]  # suspected, confirmed, exploited, patched
) -> List[Dict]
```
**Use:** Query vulnerabilities with filtering

### create_attack_chain
```python
await create_attack_chain(
    driver_id: str,
    name: str,
    attack_goal: str,  # privilege_escalation, code_execution, info_leak, etc.
    steps: List[Dict],
    initial_access: str,  # user, admin, system, any
    final_privilege: str,
    description: Optional[str],
    poc_code: Optional[str]
) -> Dict
```
**Use:** Compose multi-step attack scenario

**Example:**
```python
chain = await create_attack_chain(
    driver_id="abc-123",
    name="Kernel Code Execution via Physical Memory R/W",
    attack_goal="code_execution",
    steps=[
        {"order": 1, "vuln_id": vuln1_id, "ioctl_id": ioctl1_id, 
         "description": "Leak kernel base via arbitrary read"},
        {"order": 2, "vuln_id": vuln2_id, "ioctl_id": ioctl2_id,
         "description": "Overwrite function pointer via arbitrary write"},
        {"order": 3, "description": "Trigger overwritten function"}
    ],
    initial_access="user",
    final_privilege="kernel",
    poc_code="// Complete exploit here"
)
```

### get_attack_chains
```python
await get_attack_chains(
    driver_id: str,
    attack_goal: Optional[str]
) -> List[Dict]
```
**Use:** Retrieve attack chains with expanded step details

---

## Common Patterns

### Documenting an IOCTL Handler

```python
# 1. Add the handler function
func = await add_function(
    driver_id=driver_id,
    rva=0x1234,
    name="HandleIoctl_MSR_Access",
    is_dispatch=True,
    dispatch_type="IRP_MJ_DEVICE_CONTROL",
    decompiled="NTSTATUS HandleIoctl_MSR_Access(...) { ... }"
)

# 2. Add input/output structures
input_struct = await add_structure(
    name="MSR_ACCESS_INPUT",
    definition_c="""
    struct MSR_ACCESS_INPUT {
        ULONG MsrIndex;
        ULONG64 Value;
    }
    """,
    driver_id=driver_id,
    struct_type="ioctl_input"
)

# 3. Link to IOCTL (assuming IOCTL already exists)
await link_structure_to_ioctl(
    ioctl_id=ioctl_id,
    input_struct_id=input_struct['structure_id']
)

# 4. Document vulnerability
vuln = await add_vulnerability(
    driver_id=driver_id,
    title="Arbitrary MSR Write via IOCTL",
    vulnerability_class="arbitrary_write",
    severity="critical",
    description="No validation of MSR index",
    affected_function_id=func['function_id'],
    exploitation_difficulty="trivial"
)
```

### Tracing Dangerous API Usage

```python
# Find all paths from IOCTL handler to MmMapIoSpace
paths = await trace_call_path(
    driver_id=driver_id,
    from_function="HandleDeviceControl",
    to_function="MmMapIoSpace"
)

for i, path in enumerate(paths, 1):
    print(f"Path {i}:")
    for func in path:
        print(f"  -> {func['name']} @ 0x{func['rva']:X}")
```

### Finding Dispatch Entry Points

```python
handlers = await find_dispatch_handlers(driver_id=driver_id)

for dispatch_type, funcs in handlers.items():
    print(f"{dispatch_type}:")
    for func in funcs:
        print(f"  - {func['name']} @ 0x{func['rva']:X}")
```

---

## Type Enumerations

### Vulnerability Classes
- `arbitrary_read`
- `arbitrary_write`
- `code_exec`
- `info_leak`
- `dos`
- `privilege_escalation`
- `memory_corruption`
- `race_condition`

### Severity Levels
- `critical`
- `high`
- `medium`
- `low`
- `info`

### Exploitation Difficulty
- `trivial`
- `easy`
- `moderate`
- `hard`
- `theoretical`

### Structure Types
- `ioctl_input`
- `ioctl_output`
- `session`
- `internal`
- `windows` (OS structures)

### Dispatch Types
- `IRP_MJ_CREATE`
- `IRP_MJ_CLOSE`
- `IRP_MJ_DEVICE_CONTROL`
- `IRP_MJ_INTERNAL_DEVICE_CONTROL`
- `IRP_MJ_READ`
- `IRP_MJ_WRITE`
- `IRP_MJ_SYSTEM_CONTROL`

---

## Database Schema Dependencies

**Functions Table:**
- Stores function metadata, decompiled code
- Links to driver via `driver_id`
- Generates embeddings for semantic search

**Structures + Structure_Members:**
- Stores C struct definitions
- Members table has offset, size, type
- Can be driver-specific or shared (driver_id NULL)

**Vulnerabilities:**
- Links to driver, IOCTL, function, export
- Stores CVE, CVSS, exploitation details
- Auto-marks affected IOCTLs

**Attack_Chains:**
- Combines multiple vulnerabilities
- Steps stored as JSON array
- Links back to specific vulns/IOCTLs

**Xrefs:**
- Cross-reference table for call graph
- Used by `trace_call_path`
- Links functions to functions/imports

---

Built for PROJECT-OMBRA driver reverse engineering infrastructure.
