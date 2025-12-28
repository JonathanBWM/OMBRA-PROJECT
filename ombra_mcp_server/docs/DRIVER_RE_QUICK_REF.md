# Driver RE Tools - Quick Reference

## Initialize Database
```bash
python3 ombra_mcp_server/scripts/init_driver_re_db.py
```

## Import in Python
```python
from ombra_mcp.tools import driver_tools, ioctl_tools, import_tools, export_tools
```

---

## Driver Tools

| Function | Purpose | Required Params |
|----------|---------|-----------------|
| `add_driver()` | Add driver to DB | `file_path` |
| `get_driver()` | Get driver details | `driver_id` OR `sha256` OR `name` |
| `list_drivers()` | List all drivers | - |
| `update_driver_status()` | Update status | `driver_id`, `status` |
| `delete_driver()` | Delete driver | `driver_id`, `confirm=True` |

**Statuses:** `pending`, `in_progress`, `complete`

---

## IOCTL Tools

| Function | Purpose | Required Params |
|----------|---------|-----------------|
| `add_ioctl()` | Add IOCTL | `driver_id`, `name` |
| `get_ioctl()` | Get IOCTL details | `ioctl_id` OR (`driver_id` + `name`/`code`) |
| `list_ioctls()` | List driver IOCTLs | `driver_id` |
| `get_vulnerable_ioctls()` | Get vulnerable IOCTLs | - |
| `update_ioctl_vulnerability()` | Update vuln status | `ioctl_id`, `is_vulnerable` |

**Vulnerability Types:** `arb_read`, `arb_write`, `code_exec`, `msr_access`, `cr_access`
**Severities:** `critical`, `high`, `medium`, `low`

---

## Import Tools

| Function | Purpose | Required Params |
|----------|---------|-----------------|
| `get_imports()` | Get driver imports | `driver_id` |
| `get_import_xrefs()` | Get xrefs to import | `import_id` |
| `categorize_import()` | Categorize import | `import_id`, `category` |
| `find_dangerous_apis()` | Find dangerous APIs | `driver_id` |

**Categories:**
- `memory_management` - Physical/virtual memory
- `process_thread` - Process/thread ops
- `cpu_hardware` - MSR, CR, CPUID
- `system_info` - System query/set
- `callbacks` - Callback registration

**Dangerous API Categories:**
- `physical_memory_access` - MmMapIoSpace, etc.
- `arbitrary_read_write` - MmCopyMemory, etc.
- `code_execution` - ZwCreateThread, etc.
- `msr_access` - __readmsr, __writemsr
- `cr_access` - __readcr*, __writecr*

---

## Export Tools

| Function | Purpose | Required Params |
|----------|---------|-----------------|
| `get_exports()` | Get driver exports | `driver_id` |
| `document_export()` | Document export | `export_id` |

**Prefixes:** `ASM`, `RT`, `RTR0`, `RTMp`, `SUPR0`, `SUP`, `Nt`, `Zw`

---

## Quick Examples

### Add Driver
```python
result = await driver_tools.add_driver(
    file_path="/path/to/driver.sys",
    tags=["byovd", "research"]
)
driver_id = result['driver_id']
```

### Add Vulnerable IOCTL
```python
await ioctl_tools.add_ioctl(
    driver_id=driver_id,
    name="IOCTL_READ_MSR",
    code=0x222040,
    is_vulnerable=True,
    vulnerability_type="msr_access",
    vulnerability_severity="critical"
)
```

### Find Dangerous APIs
```python
dangerous = await import_tools.find_dangerous_apis(driver_id)
print(f"Risk: {dangerous['risk_level']}")  # critical, high, medium, low, minimal
```

### Get All Critical Vulns
```python
critical = await ioctl_tools.get_vulnerable_ioctls(severity="critical")
for ioctl in critical:
    print(f"{ioctl['driver_name']}: {ioctl['name']}")
```

---

## Database Location
```
ombra_mcp_server/src/ombra_mcp/data/driver_re.db
```

## Test Suite
```bash
python3 ombra_mcp_server/scripts/test_driver_re_tools.py
```

---

**16 Functions | 4 Modules | SQLite Backend**
