"""
Import Analysis Tools

Tools for querying and categorizing Windows kernel API imports.
"""

import json
from typing import Dict, List, Optional, Any

from .driver_re_db import get_conn


# Dangerous API patterns by category
DANGEROUS_API_PATTERNS = {
    'physical_memory_access': [
        'MmMapIoSpace', 'MmMapIoSpaceEx', 'MmMapLockedPages',
        'MmMapLockedPagesSpecifyCache', 'MmMapLockedPagesWithReservedMapping',
        'MmGetPhysicalAddress', 'MmGetVirtualForPhysical'
    ],
    'arbitrary_read_write': [
        'MmCopyMemory', 'MmCopyVirtualMemory', 'ZwReadVirtualMemory',
        'ZwWriteVirtualMemory', 'KeStackAttachProcess'
    ],
    'code_execution': [
        'ZwCreateThread', 'PsCreateSystemThread', 'KeInitializeApc',
        'KeInsertQueueApc', 'ZwQueueApcThread'
    ],
    'msr_access': [
        '__readmsr', '__writemsr', '__readpmc'
    ],
    'cr_access': [
        '__readcr0', '__readcr3', '__readcr4', '__readcr8',
        '__writecr0', '__writecr3', '__writecr4', '__writecr8'
    ],
    'system_modification': [
        'ZwSetSystemInformation', 'NtSetSystemInformation',
        'ZwLoadDriver', 'ZwUnloadDriver'
    ],
    'callback_manipulation': [
        'PsSetCreateProcessNotifyRoutine', 'PsSetCreateThreadNotifyRoutine',
        'PsSetLoadImageNotifyRoutine', 'CmRegisterCallback',
        'ObRegisterCallbacks', 'PsRemoveCreateThreadNotifyRoutine',
        'PsRemoveLoadImageNotifyRoutine'
    ],
    'interrupt_manipulation': [
        'KeSetSystemAffinityThread', 'KeRevertToUserAffinityThread',
        'KfRaiseIrql', 'KeLowerIrql', 'KeRaiseIrqlToDpcLevel'
    ]
}


async def get_imports(
    driver_id: str,
    dll: Optional[str] = None,
    category: Optional[str] = None,
    dangerous_only: bool = False
) -> List[Dict[str, Any]]:
    """
    Get imports for a driver with filtering.

    Categories: memory_management, process_thread, io, synchronization,
                system_info, registry, object_management, callbacks, cpu_hardware
    """
    conn = get_conn()
    c = conn.cursor()

    query = "SELECT * FROM imports WHERE driver_id = ?"
    params = [driver_id]

    if dll:
        query += " AND LOWER(dll_name) = LOWER(?)"
        params.append(dll)

    if category:
        query += " AND category = ?"
        params.append(category)

    if dangerous_only:
        query += " AND is_dangerous = 1"

    query += " ORDER BY dll_name, function_name"

    c.execute(query, params)
    results = [dict(row) for row in c.fetchall()]

    conn.close()
    return results


async def get_import_xrefs(import_id: str) -> List[Dict[str, Any]]:
    """
    Get all cross-references to an import.
    Returns list of calling functions with addresses.
    """
    conn = get_conn()
    c = conn.cursor()

    # Get import details
    c.execute("""
        SELECT i.*, d.original_name as driver_name
        FROM imports i
        JOIN drivers d ON i.driver_id = d.id
        WHERE i.id = ?
    """, (import_id,))

    import_info = c.fetchone()
    if not import_info:
        conn.close()
        return {"error": f"Import not found: {import_id}"}

    import_data = dict(import_info)

    # Get all xrefs
    c.execute("""
        SELECT * FROM xrefs
        WHERE to_import_id = ?
        ORDER BY from_rva
    """, (import_id,))

    xrefs = [dict(row) for row in c.fetchall()]

    conn.close()

    return {
        "import": import_data,
        "xref_count": len(xrefs),
        "xrefs": xrefs
    }


async def categorize_import(
    import_id: str,
    category: str,
    subcategory: Optional[str] = None,
    is_dangerous: bool = False,
    danger_reason: Optional[str] = None,
    usage_notes: Optional[str] = None
) -> Dict[str, Any]:
    """
    Categorize an import and mark security relevance.
    """
    conn = get_conn()
    c = conn.cursor()

    # Verify import exists
    c.execute("""
        SELECT i.id, i.function_name, d.original_name as driver_name
        FROM imports i
        JOIN drivers d ON i.driver_id = d.id
        WHERE i.id = ?
    """, (import_id,))

    row = c.fetchone()
    if not row:
        conn.close()
        return {"error": f"Import not found: {import_id}"}

    # Update categorization
    update_fields = ["category = ?"]
    params = [category]

    if subcategory is not None:
        update_fields.append("subcategory = ?")
        params.append(subcategory)

    update_fields.append("is_dangerous = ?")
    params.append(1 if is_dangerous else 0)

    if danger_reason is not None:
        update_fields.append("danger_reason = ?")
        params.append(danger_reason)

    if usage_notes is not None:
        update_fields.append("usage_notes = ?")
        params.append(usage_notes)

    params.append(import_id)

    c.execute(f"UPDATE imports SET {', '.join(update_fields)} WHERE id = ?", params)
    conn.commit()
    conn.close()

    return {
        "success": True,
        "import_id": import_id,
        "function_name": row['function_name'],
        "driver_name": row['driver_name'],
        "category": category,
        "subcategory": subcategory,
        "is_dangerous": is_dangerous,
        "message": f"Import '{row['function_name']}' categorized successfully"
    }


async def find_dangerous_apis(driver_id: str) -> Dict[str, Any]:
    """
    Find all dangerous APIs (imports) for a driver.

    Returns grouped by danger category:
    - physical_memory_access
    - arbitrary_read_write
    - code_execution
    - msr_access
    - cr_access
    - system_modification
    - callback_manipulation
    - interrupt_manipulation
    """
    conn = get_conn()
    c = conn.cursor()

    # Get driver info
    c.execute("SELECT id, original_name FROM drivers WHERE id = ?", (driver_id,))
    driver = c.fetchone()
    if not driver:
        conn.close()
        return {"error": f"Driver not found: {driver_id}"}

    # Get all imports
    c.execute("SELECT * FROM imports WHERE driver_id = ?", (driver_id,))
    all_imports = [dict(row) for row in c.fetchall()]

    # Categorize by danger type
    dangerous_by_category = {}
    total_dangerous = 0

    for category, api_list in DANGEROUS_API_PATTERNS.items():
        matching = []
        for imp in all_imports:
            if imp['function_name'] in api_list:
                matching.append({
                    'id': imp['id'],
                    'function_name': imp['function_name'],
                    'dll_name': imp['dll_name'],
                    'iat_rva': imp['iat_rva'],
                    'usage_notes': imp['usage_notes']
                })
                total_dangerous += 1

        if matching:
            dangerous_by_category[category] = {
                'count': len(matching),
                'apis': matching
            }

    # Also get manually marked dangerous imports not in patterns
    other_dangerous = []
    for imp in all_imports:
        if imp['is_dangerous'] and imp['function_name'] not in [
            api for apis in DANGEROUS_API_PATTERNS.values() for api in apis
        ]:
            other_dangerous.append({
                'id': imp['id'],
                'function_name': imp['function_name'],
                'dll_name': imp['dll_name'],
                'danger_reason': imp['danger_reason'],
                'usage_notes': imp['usage_notes']
            })
            total_dangerous += 1

    if other_dangerous:
        dangerous_by_category['other'] = {
            'count': len(other_dangerous),
            'apis': other_dangerous
        }

    conn.close()

    return {
        "driver_id": driver_id,
        "driver_name": driver['original_name'],
        "total_imports": len(all_imports),
        "total_dangerous": total_dangerous,
        "dangerous_by_category": dangerous_by_category,
        "risk_level": _assess_risk_level(total_dangerous, len(all_imports))
    }


def _assess_risk_level(dangerous_count: int, total_count: int) -> str:
    """Assess overall risk level based on dangerous API count."""
    if total_count == 0:
        return "unknown"

    ratio = dangerous_count / total_count

    if dangerous_count >= 10 or ratio > 0.3:
        return "critical"
    elif dangerous_count >= 5 or ratio > 0.15:
        return "high"
    elif dangerous_count >= 2 or ratio > 0.05:
        return "medium"
    elif dangerous_count >= 1:
        return "low"
    else:
        return "minimal"
