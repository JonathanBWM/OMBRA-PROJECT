"""
Export analysis tools for driver reverse engineering.
Manages PE exports (EAT) and exported function analysis.
"""

import uuid
from typing import Dict, List, Optional
from datetime import datetime


# Known export prefixes for vulnerable drivers
KNOWN_PREFIXES = {
    'ASM': 'Assembly/low-level operations',
    'RT': 'Runtime functions',
    'SUP': 'Support/helper functions (VirtualBox pattern)',
    'SUPR0': 'Ring-0 support functions (VirtualBox pattern)',
    'SUPR3': 'Ring-3 support functions (VirtualBox pattern)',
    'Drv': 'Driver operations',
    'Io': 'I/O operations',
    'Ke': 'Kernel executive functions',
    'Mm': 'Memory manager functions',
    'Ob': 'Object manager functions',
    'Ps': 'Process/thread functions',
    'Zw': 'Native API wrappers',
}

# Dangerous export patterns to flag
DANGEROUS_PATTERNS = {
    'MmCopy': 'Physical memory copy - exploitation primitive',
    'MmMap': 'Memory mapping - potential code execution',
    'ReadPhys': 'Physical memory read - info disclosure',
    'WritePhys': 'Physical memory write - code execution',
    'MapPhys': 'Physical address mapping',
    'IoSpace': 'I/O space mapping - hardware access',
    'MSR': 'MSR access - CPU control',
    'CR0': 'CR0 access - write protection bypass',
    'CR3': 'CR3 access - page table manipulation',
    'CR4': 'CR4 access - CPU feature control',
    'AllocExec': 'Executable allocation - code injection',
    'RunCode': 'Code execution primitive',
    'CallKernel': 'Kernel function invocation',
    'Shellcode': 'Shellcode execution',
    'Execute': 'Code execution',
    'Invoke': 'Function invocation',
}


async def get_exports(
    driver_id: str,
    prefix: Optional[str] = None,
    category: Optional[str] = None,
    dangerous_only: bool = False,
    limit: int = 100,
    offset: int = 0
) -> Dict:
    """
    Get exports for a driver with filtering.

    Retrieves EAT (Export Address Table) entries for a driver, with optional
    filtering by prefix, category, or dangerous status.

    Args:
        driver_id: UUID of the driver
        prefix: Filter by function prefix (e.g., 'SUP', 'SUPR0', 'ASM')
        category: Filter by category
        dangerous_only: Only return potentially dangerous exports
        limit: Maximum results (default 100)
        offset: Pagination offset

    Returns:
        Dict with exports list and analysis
    """
    from ..database.connection import get_db_connection

    conn = await get_db_connection()

    # Build query with filters
    query = """
        SELECT
            id, function_name, ordinal, rva,
            prefix, category, is_dangerous, danger_reason,
            return_type, parameters, calling_convention,
            description, decompiled_code
        FROM exports
        WHERE driver_id = ?
    """
    params = [driver_id]

    if prefix:
        query += " AND LOWER(prefix) = LOWER(?)"
        params.append(prefix)

    if category:
        query += " AND LOWER(category) = LOWER(?)"
        params.append(category)

    if dangerous_only:
        query += " AND is_dangerous = 1"

    # Get total count first
    count_query = query.replace(
        "SELECT\n            id, function_name, ordinal, rva,\n            prefix, category, is_dangerous, danger_reason,\n            return_type, parameters, calling_convention,\n            description, decompiled_code",
        "SELECT COUNT(*)"
    )
    cursor = await conn.execute(count_query, params)
    row = await cursor.fetchone()
    total_count = row[0] if row else 0

    # Add pagination and ordering
    query += " ORDER BY ordinal LIMIT ? OFFSET ?"
    params.extend([limit, offset])

    cursor = await conn.execute(query, params)
    exports = []
    for row in await cursor.fetchall():
        exports.append({
            'id': row[0],
            'function_name': row[1],
            'ordinal': row[2],
            'rva': row[3],
            'rva_hex': f"0x{row[3]:x}" if row[3] else None,
            'prefix': row[4],
            'category': row[5],
            'is_dangerous': bool(row[6]),
            'danger_reason': row[7],
            'return_type': row[8],
            'parameters': row[9],
            'calling_convention': row[10],
            'description': row[11],
            'has_decompiled': bool(row[12])
        })

    # Get summary by prefix
    prefix_summary = {}
    cursor = await conn.execute("""
        SELECT prefix, COUNT(*) as count
        FROM exports
        WHERE driver_id = ? AND prefix IS NOT NULL
        GROUP BY prefix
        ORDER BY count DESC
    """, (driver_id,))
    for row in await cursor.fetchall():
        prefix_summary[row[0] or 'NONE'] = row[1]

    # Get dangerous count
    cursor = await conn.execute("""
        SELECT COUNT(*) FROM exports
        WHERE driver_id = ? AND is_dangerous = 1
    """, (driver_id,))
    row = await cursor.fetchone()
    dangerous_count = row[0] if row else 0

    # Auto-detect prefixes if not already set
    cursor = await conn.execute("""
        SELECT id, function_name FROM exports
        WHERE driver_id = ? AND prefix IS NULL
    """, (driver_id,))

    prefix_updates = []
    for row in await cursor.fetchall():
        export_id, func_name = row
        if func_name:
            for prefix_key in KNOWN_PREFIXES.keys():
                if func_name.startswith(prefix_key):
                    prefix_updates.append((prefix_key, export_id))
                    break

    if prefix_updates:
        await conn.executemany("""
            UPDATE exports SET prefix = ? WHERE id = ?
        """, prefix_updates)
        await conn.commit()

    return {
        'driver_id': driver_id,
        'exports': exports,
        'total_count': total_count,
        'returned_count': len(exports),
        'offset': offset,
        'limit': limit,
        'prefix_summary': prefix_summary,
        'dangerous_count': dangerous_count,
        'prefixes_updated': len(prefix_updates),
        'filters': {
            'prefix': prefix,
            'category': category,
            'dangerous_only': dangerous_only
        },
        'known_prefixes': KNOWN_PREFIXES
    }


async def document_export(
    export_id: str,
    description: Optional[str] = None,
    category: Optional[str] = None,
    return_type: Optional[str] = None,
    parameters: Optional[str] = None,
    calling_convention: Optional[str] = None,
    is_dangerous: Optional[bool] = None,
    danger_reason: Optional[str] = None,
    decompiled_code: Optional[str] = None
) -> Dict:
    """
    Document an exported function with reverse engineering findings.

    Used to add analysis results including function signature, decompiled code,
    and security assessment for exported functions.

    Args:
        export_id: UUID of the export
        description: Human-readable description of functionality
        category: Category (memory, process, io, etc.)
        return_type: Return type (e.g., 'NTSTATUS', 'PVOID')
        parameters: JSON string of parameters
        calling_convention: Calling convention (fastcall, stdcall, etc.)
        is_dangerous: Whether this export is dangerous
        danger_reason: Why it's dangerous
        decompiled_code: Decompiled C code from Ghidra

    Returns:
        Updated export details
    """
    from ..database.connection import get_db_connection

    conn = await get_db_connection()

    # Check export exists
    cursor = await conn.execute("SELECT * FROM exports WHERE id = ?", (export_id,))
    row = await cursor.fetchone()
    if not row:
        return {'error': f'Export {export_id} not found'}

    current_data = dict(row)

    # Build update query
    updates = []
    params = []

    if description is not None:
        updates.append('description = ?')
        params.append(description)

    if category is not None:
        updates.append('category = ?')
        params.append(category)

    if return_type is not None:
        updates.append('return_type = ?')
        params.append(return_type)

    if parameters is not None:
        updates.append('parameters = ?')
        params.append(parameters)

    if calling_convention is not None:
        updates.append('calling_convention = ?')
        params.append(calling_convention)

    if is_dangerous is not None:
        updates.append('is_dangerous = ?')
        params.append(1 if is_dangerous else 0)

    if danger_reason is not None:
        updates.append('danger_reason = ?')
        params.append(danger_reason)

    if decompiled_code is not None:
        updates.append('decompiled_code = ?')
        params.append(decompiled_code)

    if not updates:
        return {
            'success': True,
            'export': current_data,
            'message': 'No updates provided'
        }

    params.append(export_id)

    await conn.execute(f"""
        UPDATE exports SET {', '.join(updates)} WHERE id = ?
    """, params)
    await conn.commit()

    # Return updated export
    cursor = await conn.execute("SELECT * FROM exports WHERE id = ?", (export_id,))
    row = await cursor.fetchone()
    updated = dict(row)

    # Auto-detect dangerous patterns if not already flagged
    if not updated.get('is_dangerous') and updated.get('function_name'):
        for pattern, reason in DANGEROUS_PATTERNS.items():
            if pattern.lower() in updated['function_name'].lower():
                await conn.execute("""
                    UPDATE exports SET is_dangerous = 1, danger_reason = ? WHERE id = ?
                """, (reason, export_id))
                await conn.commit()
                updated['is_dangerous'] = True
                updated['danger_reason'] = reason
                updated['auto_flagged'] = True
                break

    return {
        'success': True,
        'export': updated
    }


async def analyze_exports(
    driver_id: str,
    auto_flag_dangerous: bool = True
) -> Dict:
    """
    Analyze all exports for security-relevant patterns.

    Scans all exports for dangerous patterns, categorizes by prefix,
    and generates a security assessment.

    Args:
        driver_id: UUID of the driver
        auto_flag_dangerous: If True, automatically flag dangerous exports

    Returns:
        Analysis results with security assessment
    """
    from ..database.connection import get_db_connection

    conn = await get_db_connection()

    # Get all exports
    cursor = await conn.execute("""
        SELECT id, function_name, ordinal, rva, prefix, is_dangerous
        FROM exports
        WHERE driver_id = ?
        ORDER BY ordinal
    """, (driver_id,))

    exports = await cursor.fetchall()

    # Analyze each export
    dangerous_found = []
    prefix_updates = []
    danger_updates = []

    for row in exports:
        export_id, func_name, ordinal, rva, current_prefix, already_dangerous = row

        # Auto-detect prefix if missing
        if not current_prefix and func_name:
            for prefix_key in KNOWN_PREFIXES.keys():
                if func_name.startswith(prefix_key):
                    prefix_updates.append((prefix_key, export_id))
                    break

        # Check for dangerous patterns
        if func_name and not already_dangerous:
            for pattern, reason in DANGEROUS_PATTERNS.items():
                if pattern.lower() in func_name.lower():
                    dangerous_found.append({
                        'export_id': export_id,
                        'function_name': func_name,
                        'ordinal': ordinal,
                        'rva': rva,
                        'rva_hex': f"0x{rva:x}" if rva else None,
                        'pattern_matched': pattern,
                        'danger_reason': reason
                    })
                    if auto_flag_dangerous:
                        danger_updates.append((reason, export_id))
                    break

    # Apply updates
    if prefix_updates:
        await conn.executemany("""
            UPDATE exports SET prefix = ? WHERE id = ?
        """, prefix_updates)

    if danger_updates:
        await conn.executemany("""
            UPDATE exports SET is_dangerous = 1, danger_reason = ? WHERE id = ?
        """, danger_updates)

    if prefix_updates or danger_updates:
        await conn.commit()

    # Get prefix distribution
    cursor = await conn.execute("""
        SELECT prefix, COUNT(*) as count
        FROM exports
        WHERE driver_id = ? AND prefix IS NOT NULL
        GROUP BY prefix
        ORDER BY count DESC
    """, (driver_id,))
    prefix_distribution = {row[0]: row[1] for row in await cursor.fetchall()}

    # Generate assessment
    total_exports = len(exports)
    dangerous_count = len(dangerous_found)

    # Check for VirtualBox patterns (BYOVD indicator)
    is_vbox_pattern = any(p.startswith('SUP') for p in prefix_distribution.keys())

    # Calculate risk score
    risk_score = 0
    if dangerous_count > 0:
        risk_score += min(dangerous_count * 15, 60)
    if is_vbox_pattern:
        risk_score += 30  # VirtualBox pattern is high-risk
    if total_exports > 50:
        risk_score += 10  # Large attack surface

    risk_score = min(risk_score, 100)

    risk_level = 'low'
    if risk_score >= 70:
        risk_level = 'critical'
    elif risk_score >= 50:
        risk_level = 'high'
    elif risk_score >= 25:
        risk_level = 'medium'

    return {
        'driver_id': driver_id,
        'total_exports': total_exports,
        'dangerous_exports': dangerous_found,
        'dangerous_count': dangerous_count,
        'prefixes_updated': len(prefix_updates),
        'danger_flags_added': len(danger_updates),
        'prefix_distribution': prefix_distribution,
        'is_vbox_pattern': is_vbox_pattern,
        'risk_score': risk_score,
        'risk_level': risk_level,
        'assessment': {
            'vbox_warning': 'VirtualBox-style exports detected (SUPR0/SUP pattern) - common BYOVD target' if is_vbox_pattern else None,
            'attack_surface': 'Large' if total_exports > 50 else ('Medium' if total_exports > 20 else 'Small'),
            'dangerous_patterns': list(set(d['pattern_matched'] for d in dangerous_found))
        }
    }
