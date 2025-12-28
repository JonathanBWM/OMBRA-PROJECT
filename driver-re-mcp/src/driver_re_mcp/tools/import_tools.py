"""
Import analysis tools for driver reverse engineering.
Manages PE imports (IAT), categorization, and dangerous API detection.
"""

import uuid
from typing import Dict, List, Optional
from datetime import datetime

# Dangerous kernel APIs that should be flagged
DANGEROUS_APIS = {
    # Memory operations (physical memory access = exploitation primitive)
    'MmCopyMemory': 'Physical memory read - can bypass process isolation',
    'MmMapIoSpace': 'Maps physical address to virtual - common BYOVD primitive',
    'MmMapIoSpaceEx': 'Extended MmMapIoSpace - same danger',
    'MmMapLockedPages': 'Maps MDL pages - can expose arbitrary physical memory',
    'MmMapLockedPagesSpecifyCache': 'MDL mapping with cache control',
    'MmMapLockedPagesWithReservedMapping': 'MDL mapping with reserved VA',
    'ZwMapViewOfSection': 'Section mapping - process memory manipulation',
    'ZwAllocateVirtualMemory': 'Allocate memory in any process',
    'ZwWriteVirtualMemory': 'Write to process memory',
    'ZwReadVirtualMemory': 'Read from process memory',

    # Process/Thread operations
    'PsLookupProcessByProcessId': 'Get EPROCESS by PID - process targeting',
    'PsLookupThreadByThreadId': 'Get ETHREAD by TID - thread targeting',
    'KeStackAttachProcess': 'Attach to process context',
    'KeAttachProcess': 'Legacy process attach',
    'PsGetCurrentProcess': 'Get current EPROCESS',
    'IoGetCurrentProcess': 'Get current EPROCESS via I/O manager',

    # Registry operations
    'ZwCreateKey': 'Registry key creation - persistence',
    'ZwSetValueKey': 'Registry value write - persistence',
    'ZwDeleteKey': 'Registry deletion - anti-forensics',

    # File operations
    'ZwCreateFile': 'File creation - could be dropper',
    'ZwWriteFile': 'File write - potential payload delivery',
    'ZwDeleteFile': 'File deletion - anti-forensics',

    # Object/Handle operations
    'ObOpenObjectByPointer': 'Get handle from kernel pointer',
    'ZwDuplicateObject': 'Handle duplication - privilege escalation',

    # Callback operations
    'PsSetCreateProcessNotifyRoutine': 'Process callback - can block processes',
    'PsSetLoadImageNotifyRoutine': 'Image load callback - can intercept',
    'CmRegisterCallback': 'Registry callback - registry filtering',

    # MSR operations
    'rdmsr': 'Read MSR - hypervisor detection/manipulation',
    'wrmsr': 'Write MSR - hypervisor manipulation',

    # CR operations
    '__readcr0': 'Read CR0 - write protection bypass',
    '__writecr0': 'Write CR0 - disable write protection',
    '__readcr3': 'Read CR3 - page table base',
    '__readcr4': 'Read CR4 - CPU features',
}

# API categories for classification
API_CATEGORIES = {
    'memory_management': [
        'ExAllocatePool', 'ExAllocatePoolWithTag', 'ExFreePool',
        'MmAllocateContiguousMemory', 'MmFreeContiguousMemory',
        'MmAllocatePagesForMdl', 'MmFreePagesFromMdl',
        'IoAllocateMdl', 'IoFreeMdl', 'MmBuildMdlForNonPagedPool',
        'MmProbeAndLockPages', 'MmUnlockPages',
    ],
    'process_thread': [
        'PsCreateSystemThread', 'PsTerminateSystemThread',
        'PsLookupProcessByProcessId', 'PsLookupThreadByThreadId',
        'KeStackAttachProcess', 'KeUnstackDetachProcess',
        'ZwQueryInformationProcess', 'ZwQueryInformationThread',
    ],
    'io': [
        'IoCreateDevice', 'IoDeleteDevice', 'IoCreateSymbolicLink',
        'IoGetDeviceObjectPointer', 'IoCallDriver', 'IoBuildDeviceIoControlRequest',
        'IoCompleteRequest', 'IoGetCurrentIrpStackLocation',
    ],
    'synchronization': [
        'KeInitializeSpinLock', 'KeAcquireSpinLock', 'KeReleaseSpinLock',
        'KeInitializeMutex', 'KeWaitForSingleObject', 'KeSetEvent',
        'KeInitializeEvent', 'ExAcquireFastMutex', 'ExReleaseFastMutex',
    ],
    'system_info': [
        'ZwQuerySystemInformation', 'MmGetSystemRoutineAddress',
        'RtlGetVersion', 'KeQueryActiveProcessors',
    ],
    'registry': [
        'ZwOpenKey', 'ZwCreateKey', 'ZwQueryValueKey', 'ZwSetValueKey',
        'ZwDeleteKey', 'ZwEnumerateKey', 'ZwEnumerateValueKey',
    ],
    'object_management': [
        'ObReferenceObject', 'ObDereferenceObject', 'ObOpenObjectByPointer',
        'ZwDuplicateObject', 'ObReferenceObjectByHandle',
    ],
    'callbacks': [
        'PsSetCreateProcessNotifyRoutine', 'PsSetCreateProcessNotifyRoutineEx',
        'PsSetLoadImageNotifyRoutine', 'CmRegisterCallback',
        'ObRegisterCallbacks', 'IoRegisterFsRegistrationChange',
    ],
    'cpu_hardware': [
        '__rdtsc', '__cpuid', '__halt', '__invlpg',
        '__readmsr', '__writemsr', '__readcr0', '__writecr0',
    ],
}


async def get_imports(
    driver_id: str,
    dll: Optional[str] = None,
    category: Optional[str] = None,
    dangerous_only: bool = False,
    limit: int = 100,
    offset: int = 0
) -> Dict:
    """
    Get imports for a driver with filtering.

    Retrieves IAT (Import Address Table) entries for a driver, with optional
    filtering by DLL name, category, or dangerous API status.

    Args:
        driver_id: UUID of the driver
        dll: Filter by DLL name (e.g., 'ntoskrnl.exe', 'FLTMGR.SYS')
        category: Filter by API category (memory_management, io, etc.)
        dangerous_only: Only return potentially dangerous APIs
        limit: Maximum results (default 100)
        offset: Pagination offset

    Returns:
        Dict with imports list and counts
    """
    from ..database.connection import get_db_connection

    conn = await get_db_connection()

    # Build query with filters
    query = """
        SELECT
            id, dll_name, function_name, ordinal, hint,
            iat_rva, category, subcategory, is_dangerous,
            danger_reason, description, usage_count
        FROM imports
        WHERE driver_id = ?
    """
    params = [driver_id]

    if dll:
        query += " AND LOWER(dll_name) LIKE LOWER(?)"
        params.append(f"%{dll}%")

    if category:
        query += " AND LOWER(category) = LOWER(?)"
        params.append(category)

    if dangerous_only:
        query += " AND is_dangerous = 1"

    # Get total count first
    count_query = query.replace(
        "SELECT \n            id, dll_name, function_name, ordinal, hint,\n            iat_rva, category, subcategory, is_dangerous,\n            danger_reason, description, usage_count",
        "SELECT COUNT(*)"
    )
    cursor = await conn.execute(count_query, params)
    row = await cursor.fetchone()
    total_count = row[0] if row else 0

    # Add pagination and ordering
    query += " ORDER BY dll_name, function_name LIMIT ? OFFSET ?"
    params.extend([limit, offset])

    cursor = await conn.execute(query, params)
    imports = []
    for row in await cursor.fetchall():
        imports.append({
            'id': row[0],
            'dll_name': row[1],
            'function_name': row[2],
            'ordinal': row[3],
            'hint': row[4],
            'iat_rva': row[5],
            'iat_rva_hex': f"0x{row[5]:x}" if row[5] else None,
            'category': row[6],
            'subcategory': row[7],
            'is_dangerous': bool(row[8]),
            'danger_reason': row[9],
            'description': row[10],
            'usage_count': row[11]
        })

    # Get summary by DLL
    dll_summary = {}
    cursor = await conn.execute("""
        SELECT dll_name, COUNT(*) as count
        FROM imports
        WHERE driver_id = ?
        GROUP BY dll_name
        ORDER BY count DESC
    """, (driver_id,))
    for row in await cursor.fetchall():
        dll_summary[row[0]] = row[1]

    # Get dangerous count
    cursor = await conn.execute("""
        SELECT COUNT(*) FROM imports
        WHERE driver_id = ? AND is_dangerous = 1
    """, (driver_id,))
    row = await cursor.fetchone()
    dangerous_count = row[0] if row else 0

    return {
        'driver_id': driver_id,
        'imports': imports,
        'total_count': total_count,
        'returned_count': len(imports),
        'offset': offset,
        'limit': limit,
        'dll_summary': dll_summary,
        'dangerous_count': dangerous_count,
        'filters': {
            'dll': dll,
            'category': category,
            'dangerous_only': dangerous_only
        }
    }


async def get_import_xrefs(
    driver_id: str,
    import_id: Optional[str] = None,
    function_name: Optional[str] = None
) -> Dict:
    """
    Get cross-references to an imported function.

    Shows all locations in the driver that call a specific import.
    Critical for understanding how dangerous APIs are used.

    Args:
        driver_id: UUID of the driver
        import_id: UUID of the specific import
        function_name: Alternative: search by function name

    Returns:
        Dict with import details and all xrefs
    """
    from ..database.connection import get_db_connection

    conn = await get_db_connection()

    # Get the import
    if import_id:
        cursor = await conn.execute("""
            SELECT * FROM imports WHERE id = ?
        """, (import_id,))
    elif function_name:
        cursor = await conn.execute("""
            SELECT * FROM imports
            WHERE driver_id = ? AND LOWER(function_name) = LOWER(?)
        """, (driver_id, function_name))
    else:
        return {'error': 'Must provide import_id or function_name'}

    row = await cursor.fetchone()
    if not row:
        return {'error': 'Import not found'}

    import_data = dict(row)
    import_id = import_data['id']

    # Get all xrefs to this import
    cursor = await conn.execute("""
        SELECT
            x.id, x.from_rva, x.from_function_id, x.xref_type, x.instruction,
            f.name as caller_name, f.rva as caller_rva
        FROM xrefs x
        LEFT JOIN functions f ON x.from_function_id = f.id
        WHERE x.to_import_id = ?
        ORDER BY x.from_rva
    """, (import_id,))

    xrefs = []
    for row in await cursor.fetchall():
        xrefs.append({
            'xref_id': row[0],
            'from_rva': row[1],
            'from_rva_hex': f"0x{row[1]:x}" if row[1] else None,
            'from_function_id': row[2],
            'xref_type': row[3],
            'instruction': row[4],
            'caller_name': row[5],
            'caller_rva': row[6],
            'caller_rva_hex': f"0x{row[6]:x}" if row[6] else None
        })

    # Update usage count
    await conn.execute("""
        UPDATE imports SET usage_count = ? WHERE id = ?
    """, (len(xrefs), import_id))
    await conn.commit()

    return {
        'import': import_data,
        'xrefs': xrefs,
        'xref_count': len(xrefs),
        'caller_functions': list(set(x['caller_name'] for x in xrefs if x['caller_name']))
    }


async def categorize_import(
    import_id: str,
    category: str,
    subcategory: Optional[str] = None,
    is_dangerous: Optional[bool] = None,
    danger_reason: Optional[str] = None,
    description: Optional[str] = None
) -> Dict:
    """
    Categorize an import with security context.

    Used to classify imports by their purpose and flag dangerous APIs
    that require special attention during analysis.

    Args:
        import_id: UUID of the import
        category: Category (memory_management, io, process_thread, etc.)
        subcategory: Optional subcategory
        is_dangerous: Whether this is a dangerous API
        danger_reason: Why it's dangerous (if applicable)
        description: Custom description/notes

    Returns:
        Updated import details
    """
    from ..database.connection import get_db_connection

    conn = await get_db_connection()

    # Check import exists
    cursor = await conn.execute("SELECT * FROM imports WHERE id = ?", (import_id,))
    row = await cursor.fetchone()
    if not row:
        return {'error': f'Import {import_id} not found'}

    # Build update query
    updates = ['category = ?']
    params = [category]

    if subcategory is not None:
        updates.append('subcategory = ?')
        params.append(subcategory)

    if is_dangerous is not None:
        updates.append('is_dangerous = ?')
        params.append(1 if is_dangerous else 0)

    if danger_reason is not None:
        updates.append('danger_reason = ?')
        params.append(danger_reason)

    if description is not None:
        updates.append('description = ?')
        params.append(description)

    params.append(import_id)

    await conn.execute(f"""
        UPDATE imports SET {', '.join(updates)} WHERE id = ?
    """, params)
    await conn.commit()

    # Return updated import
    cursor = await conn.execute("SELECT * FROM imports WHERE id = ?", (import_id,))
    row = await cursor.fetchone()
    return {
        'success': True,
        'import': dict(row)
    }


async def find_dangerous_apis(
    driver_id: str,
    auto_categorize: bool = True
) -> Dict:
    """
    Scan driver imports for dangerous/suspicious APIs.

    Automatically identifies APIs commonly used in kernel exploits and BYOVD attacks.
    Optionally categorizes them in the database.

    Args:
        driver_id: UUID of the driver to scan
        auto_categorize: If True, automatically mark dangerous APIs in DB

    Returns:
        Dict with dangerous APIs found and risk assessment
    """
    from ..database.connection import get_db_connection

    conn = await get_db_connection()

    # Get all imports for the driver
    cursor = await conn.execute("""
        SELECT id, dll_name, function_name, is_dangerous
        FROM imports
        WHERE driver_id = ?
    """, (driver_id,))

    dangerous_found = []
    categorization_updates = []

    for row in await cursor.fetchall():
        import_id, dll_name, func_name, already_dangerous = row

        # Check against known dangerous APIs
        if func_name in DANGEROUS_APIS:
            dangerous_found.append({
                'import_id': import_id,
                'dll_name': dll_name,
                'function_name': func_name,
                'danger_reason': DANGEROUS_APIS[func_name],
                'already_flagged': bool(already_dangerous)
            })

            if auto_categorize and not already_dangerous:
                categorization_updates.append((
                    1,  # is_dangerous
                    DANGEROUS_APIS[func_name],
                    import_id
                ))

    # Auto-categorize imports
    if auto_categorize:
        # Update dangerous flags
        if categorization_updates:
            await conn.executemany("""
                UPDATE imports SET is_dangerous = ?, danger_reason = ? WHERE id = ?
            """, categorization_updates)

        # Also categorize by API category
        for category, api_list in API_CATEGORIES.items():
            for api_name in api_list:
                await conn.execute("""
                    UPDATE imports
                    SET category = ?
                    WHERE driver_id = ? AND function_name LIKE ?
                    AND (category IS NULL OR category = '')
                """, (category, driver_id, f"{api_name}%"))

        await conn.commit()

    # Calculate risk score
    risk_factors = {
        'physical_memory': len([d for d in dangerous_found if 'physical' in d['danger_reason'].lower()]),
        'process_manipulation': len([d for d in dangerous_found if 'process' in d['danger_reason'].lower()]),
        'persistence': len([d for d in dangerous_found if 'persistence' in d['danger_reason'].lower()]),
        'anti_forensics': len([d for d in dangerous_found if 'forensic' in d['danger_reason'].lower()]),
    }

    risk_score = sum([
        risk_factors['physical_memory'] * 30,  # Highest risk
        risk_factors['process_manipulation'] * 20,
        risk_factors['persistence'] * 15,
        risk_factors['anti_forensics'] * 10,
    ])
    risk_score = min(risk_score, 100)  # Cap at 100

    risk_level = 'low'
    if risk_score >= 70:
        risk_level = 'critical'
    elif risk_score >= 50:
        risk_level = 'high'
    elif risk_score >= 25:
        risk_level = 'medium'

    return {
        'driver_id': driver_id,
        'dangerous_apis': dangerous_found,
        'dangerous_count': len(dangerous_found),
        'newly_categorized': len(categorization_updates),
        'risk_factors': risk_factors,
        'risk_score': risk_score,
        'risk_level': risk_level,
        'auto_categorize': auto_categorize,
        'categories_available': list(API_CATEGORIES.keys())
    }
