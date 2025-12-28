"""
Function analysis tools for driver reverse engineering.
Manages function metadata, signatures, decompilation, and call graph analysis.
"""

import uuid
from typing import Dict, List, Optional
from datetime import datetime
import sqlite3
import json


async def add_function(
    driver_id: str,
    rva: int,
    name: Optional[str] = None,
    size: Optional[int] = None,
    return_type: Optional[str] = None,
    parameters: Optional[List[Dict]] = None,
    is_dispatch: bool = False,
    dispatch_type: Optional[str] = None,
    decompiled: Optional[str] = None,
    annotations: Optional[Dict] = None
) -> Dict:
    """
    Add or update a function in the database.

    Args:
        driver_id: UUID of the parent driver
        rva: Relative Virtual Address of the function
        name: Function name (from symbols, ghidra, or manual)
        size: Function size in bytes
        return_type: Return type (e.g., 'NTSTATUS', 'void*')
        parameters: List of parameter dicts [{name, type, description}, ...]
        is_dispatch: True if this is an IRP dispatch handler
        dispatch_type: Type if dispatch (e.g., 'IRP_MJ_DEVICE_CONTROL')
        decompiled: Decompiled C code
        annotations: Additional metadata as JSON

    Returns:
        Function details including generated UUID
    """
    from ..database.connection import get_db_connection
    from ..embeddings.provider import generate_embedding

    function_id = str(uuid.uuid4())
    now = datetime.utcnow().isoformat()

    # Generate embedding from function name and decompiled code
    embedding_text = f"{name or 'unnamed'} {decompiled or ''}"
    embedding = await generate_embedding(embedding_text)

    # Get driver's image_base for VA calculation
    conn = await get_db_connection()
    cursor = await conn.execute(
        "SELECT image_base FROM drivers WHERE id = ?",
        (driver_id,)
    )
    row = await cursor.fetchone()
    if not row:
        raise ValueError(f"Driver {driver_id} not found")

    image_base = row[0]
    va = image_base + rva

    # Insert or replace function
    await conn.execute("""
        INSERT OR REPLACE INTO functions (
            id, driver_id, name, rva, va, size, return_type, parameters,
            is_dispatch, dispatch_type, decompiled, annotations,
            created_at, updated_at, embedding
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    """, (
        function_id, driver_id, name, rva, va, size, return_type,
        json.dumps(parameters) if parameters else None,
        is_dispatch, dispatch_type, decompiled,
        json.dumps(annotations) if annotations else None,
        now, now, json.dumps(embedding)
    ))

    await conn.commit()

    return {
        "function_id": function_id,
        "driver_id": driver_id,
        "name": name,
        "rva": rva,
        "va": va,
        "is_dispatch": is_dispatch,
        "dispatch_type": dispatch_type
    }


async def get_function(
    function_id: Optional[str] = None,
    driver_id: Optional[str] = None,
    rva: Optional[int] = None,
    va: Optional[int] = None,
    name: Optional[str] = None
) -> Dict:
    """
    Get function details by ID, driver+RVA, VA, or name.

    Returns complete function information including:
    - Signature (return type, parameters)
    - Decompiled code
    - Xrefs (callers and callees)
    - Related IOCTLs
    - Annotations

    Args:
        function_id: Function UUID
        driver_id: Driver UUID (required if using rva or name)
        rva: Relative Virtual Address
        va: Virtual Address
        name: Function name

    Returns:
        Complete function details with xrefs
    """
    from ..database.connection import get_db_connection

    conn = await get_db_connection()

    # Build query based on provided parameters
    query = "SELECT * FROM functions WHERE "
    params = []

    if function_id:
        query += "id = ?"
        params.append(function_id)
    elif va:
        query += "va = ?"
        params.append(va)
    elif driver_id and rva is not None:
        query += "driver_id = ? AND rva = ?"
        params.extend([driver_id, rva])
    elif driver_id and name:
        query += "driver_id = ? AND name = ?"
        params.extend([driver_id, name])
    else:
        raise ValueError("Must provide function_id, va, (driver_id + rva), or (driver_id + name)")

    cursor = await conn.execute(query, params)
    row = await cursor.fetchone()

    if not row:
        raise ValueError("Function not found")

    # Convert row to dict
    func = dict(row)
    func['parameters'] = json.loads(func['parameters']) if func['parameters'] else []
    func['annotations'] = json.loads(func['annotations']) if func['annotations'] else {}

    # Get callers
    func['callers'] = await get_function_callers(func['id'])

    # Get callees
    func['callees'] = await get_function_callees(func['id'])

    # Get related IOCTLs
    cursor = await conn.execute("""
        SELECT id, name, code, description
        FROM ioctls
        WHERE handler_function_id = ?
    """, (func['id'],))
    func['related_ioctls'] = [dict(r) for r in await cursor.fetchall()]

    return func


async def get_function_callers(function_id: str) -> List[Dict]:
    """
    Get all functions that call this function.

    Args:
        function_id: UUID of the target function

    Returns:
        List of caller functions with xref details
    """
    from ..database.connection import get_db_connection

    conn = await get_db_connection()

    cursor = await conn.execute("""
        SELECT
            f.id, f.name, f.rva, f.va,
            x.from_rva, x.from_va, x.xref_type, x.instruction
        FROM xrefs x
        JOIN functions f ON x.from_function_id = f.id
        WHERE x.to_function_id = ?
        ORDER BY x.from_rva
    """, (function_id,))

    callers = []
    for row in await cursor.fetchall():
        callers.append({
            'function_id': row[0],
            'function_name': row[1],
            'function_rva': row[2],
            'function_va': row[3],
            'xref_from_rva': row[4],
            'xref_from_va': row[5],
            'xref_type': row[6],
            'instruction': row[7]
        })

    return callers


async def get_function_callees(function_id: str) -> List[Dict]:
    """
    Get all functions/imports called by this function.

    Args:
        function_id: UUID of the calling function

    Returns:
        List of callee functions/imports with xref details
    """
    from ..database.connection import get_db_connection

    conn = await get_db_connection()

    # Get function callees
    cursor = await conn.execute("""
        SELECT
            f.id, f.name, f.rva, f.va, 'function' as type,
            x.to_rva, x.to_va, x.xref_type, x.instruction
        FROM xrefs x
        JOIN functions f ON x.to_function_id = f.id
        WHERE x.from_function_id = ?
        ORDER BY x.to_rva
    """, (function_id,))

    callees = [dict(row) for row in await cursor.fetchall()]

    # Get import callees
    cursor = await conn.execute("""
        SELECT
            i.id, i.function_name, NULL as rva, i.iat_va, 'import' as type,
            x.to_rva, x.to_va, x.xref_type, x.instruction
        FROM xrefs x
        JOIN imports i ON x.to_import_id = i.id
        WHERE x.from_function_id = ?
        ORDER BY i.function_name
    """, (function_id,))

    callees.extend([dict(row) for row in await cursor.fetchall()])

    return callees


async def trace_call_path(
    driver_id: str,
    from_function: str,
    to_function: str,
    max_depth: int = 20
) -> List[List[Dict]]:
    """
    Find all call paths between two functions using breadth-first search.
    Critical for tracing how user input reaches dangerous APIs.

    Args:
        driver_id: Driver UUID
        from_function: Starting function (name or RVA as hex string)
        to_function: Target function (name or RVA as hex string)

    Returns:
        List of paths, where each path is a list of function dicts
    """
    from ..database.connection import get_db_connection

    conn = await get_db_connection()

    # Resolve from_function and to_function to IDs
    def parse_function_identifier(func_str):
        # Try parsing as hex RVA
        try:
            if func_str.startswith('0x'):
                return int(func_str, 16), 'rva'
            return None, 'name'
        except ValueError:
            return None, 'name'

    from_rva, from_type = parse_function_identifier(from_function)
    to_rva, to_type = parse_function_identifier(to_function)

    # Get from_function_id
    if from_type == 'rva':
        cursor = await conn.execute(
            "SELECT id FROM functions WHERE driver_id = ? AND rva = ?",
            (driver_id, from_rva)
        )
    else:
        cursor = await conn.execute(
            "SELECT id FROM functions WHERE driver_id = ? AND name = ?",
            (driver_id, from_function)
        )

    row = await cursor.fetchone()
    if not row:
        raise ValueError(f"From function '{from_function}' not found")
    from_function_id = row[0]

    # Get to_function_id
    if to_type == 'rva':
        cursor = await conn.execute(
            "SELECT id FROM functions WHERE driver_id = ? AND rva = ?",
            (driver_id, to_rva)
        )
    else:
        cursor = await conn.execute(
            "SELECT id FROM functions WHERE driver_id = ? AND name = ?",
            (driver_id, to_function)
        )

    row = await cursor.fetchone()
    if not row:
        raise ValueError(f"To function '{to_function}' not found")
    to_function_id = row[0]

    # BFS to find all paths (max_depth passed as parameter)
    paths = []
    queue = [([from_function_id], set([from_function_id]))]

    while queue:
        current_path, visited = queue.pop(0)
        current_func = current_path[-1]

        if len(current_path) > max_depth:
            continue

        if current_func == to_function_id:
            # Found a path - get full function details
            path_details = []
            for func_id in current_path:
                cursor = await conn.execute(
                    "SELECT id, name, rva, va FROM functions WHERE id = ?",
                    (func_id,)
                )
                row = await cursor.fetchone()
                if row:
                    path_details.append(dict(row))
            paths.append(path_details)
            continue

        # Get callees of current function
        cursor = await conn.execute("""
            SELECT DISTINCT to_function_id
            FROM xrefs
            WHERE from_function_id = ? AND to_function_id IS NOT NULL
        """, (current_func,))

        for row in await cursor.fetchall():
            next_func = row[0]
            if next_func not in visited:
                new_visited = visited.copy()
                new_visited.add(next_func)
                queue.append((current_path + [next_func], new_visited))

    return paths


async def find_dispatch_handlers(driver_id: str) -> Dict:
    """
    Find all IRP dispatch handlers in the driver.

    Looks for functions marked as dispatch handlers and categorizes them by type:
    - IRP_MJ_CREATE
    - IRP_MJ_CLOSE
    - IRP_MJ_DEVICE_CONTROL
    - IRP_MJ_INTERNAL_DEVICE_CONTROL
    - IRP_MJ_READ
    - IRP_MJ_WRITE
    - etc.

    Args:
        driver_id: Driver UUID

    Returns:
        Dict mapping dispatch type to function details
    """
    from ..database.connection import get_db_connection

    conn = await get_db_connection()

    cursor = await conn.execute("""
        SELECT id, name, rva, va, dispatch_type, decompiled
        FROM functions
        WHERE driver_id = ? AND is_dispatch = 1
        ORDER BY dispatch_type
    """, (driver_id,))

    handlers = {}
    for row in await cursor.fetchall():
        func = dict(row)
        dispatch_type = func['dispatch_type'] or 'UNKNOWN'

        if dispatch_type not in handlers:
            handlers[dispatch_type] = []

        handlers[dispatch_type].append({
            'function_id': func['id'],
            'name': func['name'],
            'rva': func['rva'],
            'va': func['va'],
            'decompiled': func['decompiled']
        })

    return handlers
