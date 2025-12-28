"""
Cross-reference tools for Driver RE MCP server.

Provides functionality for managing and querying cross-references between
functions, imports, exports, and data. Includes call graph building and
data flow analysis.
"""

import sqlite3
from typing import Dict, List, Optional, Set, Tuple
from collections import deque


class XrefTools:
    """Cross-reference analysis tools."""

    def __init__(self, db_path: str):
        """
        Initialize xref tools.

        Args:
            db_path: Path to SQLite database
        """
        self.db_path = db_path

    def _get_db_connection(self):
        """Get SQLite database connection."""
        conn = sqlite3.connect(self.db_path)
        conn.row_factory = sqlite3.Row
        return conn


async def add_xref(
    driver_id: str,
    from_rva: int,
    to_rva: int,
    xref_type: str,
    instruction: Optional[str] = None,
    db_path: str = "./data/driver_re.db"
) -> Dict:
    """
    Add a cross-reference to the database.

    Args:
        driver_id: Driver UUID
        from_rva: Source address (RVA)
        to_rva: Destination address (RVA)
        xref_type: Type of reference (call, jump, data_ref, offset)
        instruction: Optional instruction at reference site
        db_path: Path to SQLite database

    Returns:
        Created xref record with ID

    Xref Types:
        - call: Function call (CALL instruction)
        - jump: Control flow jump (JMP, conditional jumps)
        - data_ref: Data reference (MOV, LEA, etc.)
        - offset: Offset calculation (for structure member access)
    """
    tools = XrefTools(db_path)
    conn = tools._get_db_connection()
    cursor = conn.cursor()

    try:
        # Get driver image base for VA calculation
        cursor.execute("SELECT image_base FROM drivers WHERE id = ?", (driver_id,))
        row = cursor.fetchone()
        if not row:
            raise ValueError(f"Driver not found: {driver_id}")

        image_base = row["image_base"]
        from_va = image_base + from_rva
        to_va = image_base + to_rva

        # Try to find function IDs for from/to addresses
        cursor.execute(
            "SELECT id FROM functions WHERE driver_id = ? AND rva = ?",
            (driver_id, from_rva)
        )
        from_func = cursor.fetchone()
        from_function_id = from_func["id"] if from_func else None

        cursor.execute(
            "SELECT id FROM functions WHERE driver_id = ? AND rva = ?",
            (driver_id, to_rva)
        )
        to_func = cursor.fetchone()
        to_function_id = to_func["id"] if to_func else None

        # Check if this is a reference to an import
        cursor.execute(
            "SELECT id FROM imports WHERE driver_id = ? AND iat_rva = ?",
            (driver_id, to_rva)
        )
        import_row = cursor.fetchone()
        to_import_id = import_row["id"] if import_row else None

        # Check if this is a reference to an export
        cursor.execute(
            "SELECT id FROM exports WHERE driver_id = ? AND rva = ?",
            (driver_id, to_rva)
        )
        export_row = cursor.fetchone()
        to_export_id = export_row["id"] if export_row else None

        # Insert xref
        cursor.execute("""
            INSERT INTO xrefs (
                driver_id, from_rva, from_va, from_function_id,
                to_rva, to_va, to_function_id, to_import_id, to_export_id,
                xref_type, instruction
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, (
            driver_id, from_rva, from_va, from_function_id,
            to_rva, to_va, to_function_id, to_import_id, to_export_id,
            xref_type, instruction
        ))

        xref_id = cursor.lastrowid
        conn.commit()

        return {
            "id": xref_id,
            "driver_id": driver_id,
            "from_rva": from_rva,
            "from_va": from_va,
            "to_rva": to_rva,
            "to_va": to_va,
            "xref_type": xref_type,
            "instruction": instruction
        }

    finally:
        conn.close()


async def get_xrefs_to(
    driver_id: str,
    rva: Optional[int] = None,
    function_id: Optional[str] = None,
    import_id: Optional[str] = None,
    db_path: str = "./data/driver_re.db"
) -> List[Dict]:
    """
    Get all cross-references TO an address, function, or import.

    Shows all locations that reference the target (callers, data references).

    Args:
        driver_id: Driver UUID
        rva: Optional RVA to find references to
        function_id: Optional function UUID to find callers of
        import_id: Optional import UUID to find callers of
        db_path: Path to SQLite database

    Returns:
        List of xrefs with source function details

    Use cases:
        - Find all callers of a function
        - Find all references to an import API
        - Identify data access patterns
    """
    tools = XrefTools(db_path)
    conn = tools._get_db_connection()
    cursor = conn.cursor()

    try:
        # Build query based on what we're searching for
        if function_id:
            where_clause = "x.to_function_id = ?"
            param = function_id
        elif import_id:
            where_clause = "x.to_import_id = ?"
            param = import_id
        elif rva is not None:
            where_clause = "x.to_rva = ? AND x.driver_id = ?"
            param = (rva, driver_id)
        else:
            raise ValueError("Must specify either rva, function_id, or import_id")

        # Query xrefs with source function details
        query = f"""
            SELECT
                x.id,
                x.driver_id,
                x.from_rva,
                x.from_va,
                x.to_rva,
                x.to_va,
                x.xref_type,
                x.instruction,
                f.id as from_func_id,
                f.name as from_func_name,
                f.rva as from_func_rva
            FROM xrefs x
            LEFT JOIN functions f ON x.from_function_id = f.id
            WHERE {where_clause}
            ORDER BY x.from_rva
        """

        if isinstance(param, tuple):
            cursor.execute(query, param)
        else:
            cursor.execute(query, (param,))

        rows = cursor.fetchall()

        results = []
        for row in rows:
            results.append({
                "xref_id": row["id"],
                "driver_id": row["driver_id"],
                "from_rva": row["from_rva"],
                "from_va": row["from_va"],
                "to_rva": row["to_rva"],
                "to_va": row["to_va"],
                "xref_type": row["xref_type"],
                "instruction": row["instruction"],
                "from_function": {
                    "id": row["from_func_id"],
                    "name": row["from_func_name"],
                    "rva": row["from_func_rva"]
                } if row["from_func_id"] else None
            })

        return results

    finally:
        conn.close()


async def get_xrefs_from(
    driver_id: str,
    rva: Optional[int] = None,
    function_id: Optional[str] = None,
    db_path: str = "./data/driver_re.db"
) -> List[Dict]:
    """
    Get all cross-references FROM an address or function.

    Shows all locations that this address/function references (callees, data).

    Args:
        driver_id: Driver UUID
        rva: Optional RVA to find references from
        function_id: Optional function UUID to find callees of
        db_path: Path to SQLite database

    Returns:
        List of xrefs with destination details

    Use cases:
        - Find all functions called by a function
        - Find all APIs used by a function
        - Trace data dependencies
    """
    tools = XrefTools(db_path)
    conn = tools._get_db_connection()
    cursor = conn.cursor()

    try:
        # Build query
        if function_id:
            where_clause = "x.from_function_id = ?"
            param = function_id
        elif rva is not None:
            where_clause = "x.from_rva = ? AND x.driver_id = ?"
            param = (rva, driver_id)
        else:
            raise ValueError("Must specify either rva or function_id")

        # Query xrefs with destination details
        query = f"""
            SELECT
                x.id,
                x.driver_id,
                x.from_rva,
                x.from_va,
                x.to_rva,
                x.to_va,
                x.xref_type,
                x.instruction,
                f.id as to_func_id,
                f.name as to_func_name,
                i.id as to_import_id,
                i.function_name as to_import_name,
                i.dll_name as to_import_dll,
                e.id as to_export_id,
                e.function_name as to_export_name
            FROM xrefs x
            LEFT JOIN functions f ON x.to_function_id = f.id
            LEFT JOIN imports i ON x.to_import_id = i.id
            LEFT JOIN exports e ON x.to_export_id = e.id
            WHERE {where_clause}
            ORDER BY x.to_rva
        """

        if isinstance(param, tuple):
            cursor.execute(query, param)
        else:
            cursor.execute(query, (param,))

        rows = cursor.fetchall()

        results = []
        for row in rows:
            # Build destination info
            to_info = None
            if row["to_func_id"]:
                to_info = {
                    "type": "function",
                    "id": row["to_func_id"],
                    "name": row["to_func_name"]
                }
            elif row["to_import_id"]:
                to_info = {
                    "type": "import",
                    "id": row["to_import_id"],
                    "name": row["to_import_name"],
                    "dll": row["to_import_dll"]
                }
            elif row["to_export_id"]:
                to_info = {
                    "type": "export",
                    "id": row["to_export_id"],
                    "name": row["to_export_name"]
                }

            results.append({
                "xref_id": row["id"],
                "driver_id": row["driver_id"],
                "from_rva": row["from_rva"],
                "from_va": row["from_va"],
                "to_rva": row["to_rva"],
                "to_va": row["to_va"],
                "xref_type": row["xref_type"],
                "instruction": row["instruction"],
                "to": to_info
            })

        return results

    finally:
        conn.close()


async def build_call_graph(
    driver_id: str,
    root_function: Optional[str] = None,
    depth: int = 5,
    direction: str = "callees",
    db_path: str = "./data/driver_re.db"
) -> Dict:
    """
    Build a call graph starting from a function.

    Returns nodes and edges for visualization (compatible with graphviz, D3, etc.).

    Args:
        driver_id: Driver UUID
        root_function: Optional function name or RVA to start from
                      (None = all functions)
        depth: Maximum depth to traverse (default 5)
        direction: Graph direction (callees, callers, both)
        db_path: Path to SQLite database

    Returns:
        Dictionary with:
            - nodes: List of function nodes
            - edges: List of call edges
            - stats: Graph statistics

    Directions:
        - callees: Show what this function calls (downward)
        - callers: Show what calls this function (upward)
        - both: Bidirectional graph

    Use cases:
        - Visualize function call relationships
        - Understand control flow
        - Find execution paths
    """
    tools = XrefTools(db_path)
    conn = tools._get_db_connection()
    cursor = conn.cursor()

    try:
        # Find root function(s)
        root_functions = []

        if root_function:
            # Try as RVA first
            try:
                rva = int(root_function, 16) if isinstance(root_function, str) and root_function.startswith("0x") else int(root_function)
                cursor.execute(
                    "SELECT id, name, rva FROM functions WHERE driver_id = ? AND rva = ?",
                    (driver_id, rva)
                )
            except ValueError:
                # Try as function name
                cursor.execute(
                    "SELECT id, name, rva FROM functions WHERE driver_id = ? AND name = ?",
                    (driver_id, root_function)
                )

            row = cursor.fetchone()
            if row:
                root_functions.append({
                    "id": row["id"],
                    "name": row["name"],
                    "rva": row["rva"]
                })
        else:
            # Get all functions as roots
            cursor.execute(
                "SELECT id, name, rva FROM functions WHERE driver_id = ?",
                (driver_id,)
            )
            for row in cursor.fetchall():
                root_functions.append({
                    "id": row["id"],
                    "name": row["name"],
                    "rva": row["rva"]
                })

        if not root_functions:
            return {"nodes": [], "edges": [], "stats": {}}

        # Build graph using BFS
        nodes = {}
        edges = []
        visited = set()

        def add_node(func_id, name, rva, level):
            if func_id not in nodes:
                nodes[func_id] = {
                    "id": func_id,
                    "name": name or f"sub_{rva:X}",
                    "rva": rva,
                    "level": level
                }

        def traverse_callees(func_id, current_depth):
            if current_depth >= depth or func_id in visited:
                return
            visited.add(func_id)

            # Get xrefs from this function
            cursor.execute("""
                SELECT
                    x.to_function_id,
                    x.to_import_id,
                    f.name as func_name,
                    f.rva as func_rva,
                    i.function_name as import_name,
                    i.dll_name as import_dll
                FROM xrefs x
                LEFT JOIN functions f ON x.to_function_id = f.id
                LEFT JOIN imports i ON x.to_import_id = i.id
                WHERE x.from_function_id = ? AND x.xref_type = 'call'
            """, (func_id,))

            for row in cursor.fetchall():
                if row["to_function_id"]:
                    target_id = row["to_function_id"]
                    add_node(target_id, row["func_name"], row["func_rva"], current_depth + 1)
                    edges.append({
                        "from": func_id,
                        "to": target_id,
                        "type": "call"
                    })
                    traverse_callees(target_id, current_depth + 1)
                elif row["to_import_id"]:
                    import_id = row["to_import_id"]
                    import_label = f"{row['import_dll']}!{row['import_name']}"
                    add_node(import_id, import_label, 0, current_depth + 1)
                    edges.append({
                        "from": func_id,
                        "to": import_id,
                        "type": "import_call"
                    })

        def traverse_callers(func_id, current_depth):
            if current_depth >= depth or func_id in visited:
                return
            visited.add(func_id)

            # Get xrefs to this function
            cursor.execute("""
                SELECT
                    x.from_function_id,
                    f.name as func_name,
                    f.rva as func_rva
                FROM xrefs x
                JOIN functions f ON x.from_function_id = f.id
                WHERE x.to_function_id = ? AND x.xref_type = 'call'
            """, (func_id,))

            for row in cursor.fetchall():
                caller_id = row["from_function_id"]
                add_node(caller_id, row["func_name"], row["func_rva"], current_depth + 1)
                edges.append({
                    "from": caller_id,
                    "to": func_id,
                    "type": "call"
                })
                traverse_callers(caller_id, current_depth + 1)

        # Traverse from each root
        for root in root_functions:
            add_node(root["id"], root["name"], root["rva"], 0)

            if direction == "callees":
                traverse_callees(root["id"], 0)
            elif direction == "callers":
                traverse_callers(root["id"], 0)
            elif direction == "both":
                traverse_callees(root["id"], 0)
                visited.clear()
                traverse_callers(root["id"], 0)

        return {
            "nodes": list(nodes.values()),
            "edges": edges,
            "stats": {
                "node_count": len(nodes),
                "edge_count": len(edges),
                "max_depth": depth
            }
        }

    finally:
        conn.close()


async def find_paths_to_api(
    driver_id: str,
    api_name: str,
    from_entry_points: bool = True,
    max_depth: int = 10,
    db_path: str = "./data/driver_re.db"
) -> List[List[Dict]]:
    """
    Find all call paths that reach a specific Windows kernel API.

    CRITICAL for vulnerability analysis - shows how user-controlled input
    can reach dangerous APIs like MmMapIoSpace, ZwSetSystemInformation, etc.

    Args:
        driver_id: Driver UUID
        api_name: API function name (e.g., "MmMapIoSpace")
        from_entry_points: Start from driver entry/dispatch handlers only
        max_depth: Maximum path length (default 10)
        db_path: Path to SQLite database

    Returns:
        List of call paths, where each path is a list of function/import nodes

    Each path node contains:
        - type: "function" or "import"
        - id: Entity ID
        - name: Function/import name
        - rva: Function RVA (if applicable)

    Use cases:
        - Trace user input to MmMapIoSpace (arbitrary physical memory access)
        - Find paths to ZwSetSystemInformation (system manipulation)
        - Discover MSR/CR access chains
        - Identify code execution vectors
    """
    tools = XrefTools(db_path)
    conn = tools._get_db_connection()
    cursor = conn.cursor()

    try:
        # Find the target import
        cursor.execute(
            "SELECT id FROM imports WHERE driver_id = ? AND function_name = ?",
            (driver_id, api_name)
        )
        import_row = cursor.fetchone()

        if not import_row:
            return []

        target_import_id = import_row["id"]

        # Find all xrefs to this import (direct callers)
        cursor.execute("""
            SELECT
                x.from_function_id,
                f.name as func_name,
                f.rva as func_rva
            FROM xrefs x
            JOIN functions f ON x.from_function_id = f.id
            WHERE x.to_import_id = ? AND x.xref_type = 'call'
        """, (target_import_id,))

        direct_callers = []
        for row in cursor.fetchall():
            direct_callers.append({
                "id": row["from_function_id"],
                "name": row["func_name"],
                "rva": row["func_rva"]
            })

        if not direct_callers:
            return []

        # Find entry points
        entry_points = []
        if from_entry_points:
            # Get DriverEntry
            cursor.execute(
                "SELECT id, name, rva FROM functions WHERE driver_id = ? AND is_entry_point = 1",
                (driver_id,)
            )
            for row in cursor.fetchall():
                entry_points.append({
                    "id": row["id"],
                    "name": row["name"],
                    "rva": row["rva"]
                })

            # Get dispatch handlers
            cursor.execute(
                "SELECT id, name, rva FROM functions WHERE driver_id = ? AND is_dispatch = 1",
                (driver_id,)
            )
            for row in cursor.fetchall():
                entry_points.append({
                    "id": row["id"],
                    "name": row["name"],
                    "rva": row["rva"]
                })

            # Get IOCTL handler functions
            cursor.execute("""
                SELECT DISTINCT f.id, f.name, f.rva
                FROM ioctls io
                JOIN functions f ON io.handler_function_id = f.id
                WHERE io.driver_id = ?
            """, (driver_id,))
            for row in cursor.fetchall():
                entry_points.append({
                    "id": row["id"],
                    "name": row["name"],
                    "rva": row["rva"]
                })
        else:
            # Use all functions as potential entry points
            cursor.execute(
                "SELECT id, name, rva FROM functions WHERE driver_id = ?",
                (driver_id,)
            )
            for row in cursor.fetchall():
                entry_points.append({
                    "id": row["id"],
                    "name": row["name"],
                    "rva": row["rva"]
                })

        # Find paths using BFS from each direct caller backwards to entry points
        all_paths = []

        for caller in direct_callers:
            # BFS to find paths from entry points to this caller
            for entry in entry_points:
                paths = _find_paths_between(
                    conn, entry["id"], caller["id"], max_depth
                )

                # Append the target API to each path
                for path in paths:
                    path.append({
                        "type": "import",
                        "id": target_import_id,
                        "name": api_name,
                        "rva": None
                    })
                    all_paths.append(path)

        return all_paths

    finally:
        conn.close()


def _find_paths_between(
    conn: sqlite3.Connection,
    start_func_id: str,
    end_func_id: str,
    max_depth: int
) -> List[List[Dict]]:
    """
    Find all paths between two functions using BFS.

    Returns list of paths, where each path is a list of function nodes.
    """
    cursor = conn.cursor()

    # BFS to find all paths
    queue = deque([(start_func_id, [start_func_id])])
    paths = []

    while queue:
        current_id, path = queue.popleft()

        if len(path) > max_depth:
            continue

        if current_id == end_func_id:
            # Found a complete path - convert IDs to full nodes
            full_path = []
            for func_id in path:
                cursor.execute(
                    "SELECT id, name, rva FROM functions WHERE id = ?",
                    (func_id,)
                )
                row = cursor.fetchone()
                if row:
                    full_path.append({
                        "type": "function",
                        "id": row["id"],
                        "name": row["name"],
                        "rva": row["rva"]
                    })
            paths.append(full_path)
            continue

        # Get callees
        cursor.execute(
            "SELECT to_function_id FROM xrefs WHERE from_function_id = ? AND xref_type = 'call' AND to_function_id IS NOT NULL",
            (current_id,)
        )

        for row in cursor.fetchall():
            callee_id = row["to_function_id"]
            if callee_id not in path:  # Avoid cycles
                queue.append((callee_id, path + [callee_id]))

    return paths
