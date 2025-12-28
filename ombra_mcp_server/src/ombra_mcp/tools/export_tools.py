"""
Export Analysis Tools

Tools for querying and documenting driver export functions.
"""

import json
from typing import Dict, List, Optional, Any

from .driver_re_db import get_conn


# Known export prefixes and their meanings
EXPORT_PREFIX_INFO = {
    'ASM': {
        'category': 'atomic_operations',
        'description': 'Atomic assembly operations (ASMAtomicXxx, ASMBitXxx)'
    },
    'RT': {
        'category': 'runtime',
        'description': 'Runtime library functions (generic)'
    },
    'RTR0': {
        'category': 'ring0_runtime',
        'description': 'Ring-0 specific runtime functions'
    },
    'RTMp': {
        'category': 'multiprocessor',
        'description': 'Multiprocessor/threading functions'
    },
    'SUPR0': {
        'category': 'supervisor_ring0',
        'description': 'Supervisor ring-0 support functions'
    },
    'SUP': {
        'category': 'supervisor',
        'description': 'Generic supervisor functions'
    },
    'Nt': {
        'category': 'native_api',
        'description': 'Windows Native API exports'
    },
    'Zw': {
        'category': 'native_api',
        'description': 'Windows Native API exports (system call interface)'
    }
}


def _extract_prefix(function_name: str) -> Optional[str]:
    """Extract prefix from function name."""
    if not function_name:
        return None

    for prefix in EXPORT_PREFIX_INFO.keys():
        if function_name.startswith(prefix):
            return prefix

    return None


async def get_exports(
    driver_id: str,
    prefix: Optional[str] = None,
    category: Optional[str] = None,
    dangerous_only: bool = False
) -> List[Dict[str, Any]]:
    """
    Get exports for a driver with filtering.

    Prefixes: ASM, RT, RTR0, RTMp, SUPR0, SUP, Nt, Zw, etc.
    """
    conn = get_conn()
    c = conn.cursor()

    query = "SELECT * FROM exports WHERE driver_id = ?"
    params = [driver_id]

    if prefix:
        query += " AND prefix = ?"
        params.append(prefix)

    if category:
        query += " AND category = ?"
        params.append(category)

    if dangerous_only:
        query += " AND is_dangerous = 1"

    query += " ORDER BY ordinal"

    c.execute(query, params)
    results = []

    for row in c.fetchall():
        result = dict(row)
        # Parse parameters if present
        if result.get('parameters'):
            result['parameters'] = json.loads(result['parameters'])
        results.append(result)

    conn.close()
    return results


async def document_export(
    export_id: str,
    description: Optional[str] = None,
    return_type: Optional[str] = None,
    parameters: Optional[List[Dict]] = None,
    calling_convention: Optional[str] = None,
    is_dangerous: bool = False,
    danger_reason: Optional[str] = None,
    decompiled_code: Optional[str] = None
) -> Dict[str, Any]:
    """
    Document an export function.

    Parameters format: [{"name": "param1", "type": "PVOID", "description": "..."}, ...]
    """
    conn = get_conn()
    c = conn.cursor()

    # Verify export exists
    c.execute("""
        SELECT e.id, e.function_name, e.ordinal, d.original_name as driver_name
        FROM exports e
        JOIN drivers d ON e.driver_id = d.id
        WHERE e.id = ?
    """, (export_id,))

    row = c.fetchone()
    if not row:
        conn.close()
        return {"error": f"Export not found: {export_id}"}

    # Build update query
    update_fields = []
    params_list = []

    if description is not None:
        update_fields.append("description = ?")
        params_list.append(description)

    if return_type is not None:
        update_fields.append("return_type = ?")
        params_list.append(return_type)

    if parameters is not None:
        update_fields.append("parameters = ?")
        params_list.append(json.dumps(parameters))

    if calling_convention is not None:
        update_fields.append("calling_convention = ?")
        params_list.append(calling_convention)

    update_fields.append("is_dangerous = ?")
    params_list.append(1 if is_dangerous else 0)

    if danger_reason is not None:
        update_fields.append("danger_reason = ?")
        params_list.append(danger_reason)

    if decompiled_code is not None:
        update_fields.append("decompiled_code = ?")
        params_list.append(decompiled_code)

    # Auto-detect prefix if not set
    prefix = _extract_prefix(row['function_name'])
    if prefix:
        update_fields.append("prefix = ?")
        params_list.append(prefix)

        # Auto-assign category based on prefix
        if prefix in EXPORT_PREFIX_INFO:
            update_fields.append("category = ?")
            params_list.append(EXPORT_PREFIX_INFO[prefix]['category'])

    params_list.append(export_id)

    if update_fields:
        c.execute(f"UPDATE exports SET {', '.join(update_fields)} WHERE id = ?", params_list)
        conn.commit()

    conn.close()

    return {
        "success": True,
        "export_id": export_id,
        "function_name": row['function_name'],
        "ordinal": row['ordinal'],
        "driver_name": row['driver_name'],
        "prefix": prefix,
        "is_dangerous": is_dangerous,
        "message": f"Export '{row['function_name']}' documented successfully"
    }
