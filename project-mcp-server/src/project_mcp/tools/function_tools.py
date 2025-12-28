"""
Function analysis and management tools.

Provides capabilities for:
- Finding and retrieving function definitions
- Searching functions by name and content
- Linking functions to features
- Analyzing function call relationships
"""

import json
from typing import Optional

from ..database import fetch_one, fetch_all, insert, update, delete, count, search_fts


async def find_function(
    function_name: str,
    exact_match: bool = False
) -> dict:
    """
    Find where a function is defined.

    Searches the function index for matches and returns file location,
    line numbers, stub status, and linked features.

    Args:
        function_name: Name of the function to find
        exact_match: If True, match exact name only. If False, allow partial matches.

    Returns:
        List of matching functions with file and feature information.
    """
    if exact_match:
        # Exact name match
        functions = await fetch_all(
            """
            SELECT
                fn.id, fn.name, fn.signature, fn.return_type,
                fn.line_start, fn.line_end,
                fn.is_stub, fn.stub_reason,
                fn.has_doc_comment, fn.doc_comment,
                f.absolute_path, f.filename, f.component_id,
                c.name as component_name
            FROM functions fn
            JOIN files f ON fn.file_id = f.id
            LEFT JOIN components c ON f.component_id = c.id
            WHERE fn.name = ?
            ORDER BY f.filename, fn.line_start
            """,
            (function_name,)
        )
    else:
        # Partial match using LIKE
        functions = await fetch_all(
            """
            SELECT
                fn.id, fn.name, fn.signature, fn.return_type,
                fn.line_start, fn.line_end,
                fn.is_stub, fn.stub_reason,
                fn.has_doc_comment, fn.doc_comment,
                f.absolute_path, f.filename, f.component_id,
                c.name as component_name
            FROM functions fn
            JOIN files f ON fn.file_id = f.id
            LEFT JOIN components c ON f.component_id = c.id
            WHERE fn.name LIKE ?
            ORDER BY f.filename, fn.line_start
            LIMIT 50
            """,
            (f"%{function_name}%",)
        )

    # For each function, get linked features
    results = []

    for func in functions:
        # Get linked features
        features = await fetch_all(
            """
            SELECT
                f.id, f.name, f.status,
                ff.role
            FROM features f
            JOIN feature_functions ff ON f.id = ff.feature_id
            WHERE ff.function_id = ?
            """,
            (func['id'],)
        )

        func_data = dict(func)
        func_data['linked_features'] = features
        func_data['feature_count'] = len(features)

        results.append(func_data)

    return {
        "success": True,
        "count": len(results),
        "exact_match": exact_match,
        "functions": results
    }


async def get_function(function_id: int) -> dict:
    """
    Get complete information about a function.

    Returns function details including file location, stub status,
    documentation, linked features, and call relationships.

    Args:
        function_id: Database ID of the function

    Returns:
        Complete function information.
    """
    # Get function record
    func = await fetch_one(
        """
        SELECT
            fn.*,
            f.absolute_path, f.filename, f.component_id,
            c.name as component_name
        FROM functions fn
        JOIN files f ON fn.file_id = f.id
        LEFT JOIN components c ON f.component_id = c.id
        WHERE fn.id = ?
        """,
        (function_id,)
    )

    if not func:
        return {
            "success": False,
            "error": f"Function not found: {function_id}"
        }

    # Get linked features
    features = await fetch_all(
        """
        SELECT
            f.id, f.name, f.status, f.priority,
            ff.role
        FROM features f
        JOIN feature_functions ff ON f.id = ff.feature_id
        WHERE ff.function_id = ?
        """,
        (function_id,)
    )

    # Parse calls/called_by if they exist
    calls = []
    called_by = []

    if func['calls']:
        try:
            calls = json.loads(func['calls'])
        except:
            pass

    if func['called_by']:
        try:
            called_by = json.loads(func['called_by'])
        except:
            pass

    # Get stub info if this is a stub
    stub_info = None
    if func['is_stub']:
        stub_info = await fetch_one(
            """
            SELECT id, stub_type, stub_text, severity, resolved_at
            FROM stubs
            WHERE function_id = ?
            ORDER BY detected_at DESC
            LIMIT 1
            """,
            (function_id,)
        )

    return {
        "success": True,
        "function": dict(func),
        "features": features,
        "feature_count": len(features),
        "calls": calls,
        "calls_count": len(calls),
        "called_by": called_by,
        "called_by_count": len(called_by),
        "stub_info": stub_info,
    }


async def list_functions(
    file_id: Optional[int] = None,
    is_stub: Optional[bool] = None,
    limit: int = 100
) -> dict:
    """
    List functions with optional filters.

    Args:
        file_id: Filter by file
        is_stub: Filter by stub status (True = stubs only, False = implemented only)
        limit: Maximum results to return

    Returns:
        List of functions matching the filters.
    """
    where_parts = ["1=1"]
    params = []

    if file_id is not None:
        where_parts.append("fn.file_id = ?")
        params.append(file_id)

    if is_stub is not None:
        where_parts.append("fn.is_stub = ?")
        params.append(1 if is_stub else 0)

    where_clause = " AND ".join(where_parts)

    functions = await fetch_all(
        f"""
        SELECT
            fn.id, fn.name, fn.signature, fn.return_type,
            fn.line_start, fn.line_end,
            fn.is_stub, fn.stub_reason,
            f.filename, f.absolute_path, f.component_id
        FROM functions fn
        JOIN files f ON fn.file_id = f.id
        WHERE {where_clause}
        ORDER BY f.filename, fn.line_start
        LIMIT ?
        """,
        tuple(params + [limit])
    )

    # Get total count
    total = await count("functions fn", where_clause, tuple(params))

    return {
        "success": True,
        "total": total,
        "returned": len(functions),
        "functions": functions
    }


async def search_functions(
    query: str,
    is_stub: Optional[bool] = None
) -> dict:
    """
    Full-text search on function names, signatures, and documentation.

    Args:
        query: Search query
        is_stub: Optional filter by stub status

    Returns:
        List of matching functions.
    """
    # Use FTS for search
    fts_results = await search_fts("functions_fts", query, limit=50)
    function_ids = [r['rowid'] for r in fts_results]

    if not function_ids:
        return {
            "success": True,
            "count": 0,
            "functions": []
        }

    # Build additional filters
    where_parts = [f"fn.id IN ({','.join('?' * len(function_ids))})"]
    params = function_ids

    if is_stub is not None:
        where_parts.append("fn.is_stub = ?")
        params.append(1 if is_stub else 0)

    where_clause = " AND ".join(where_parts)

    functions = await fetch_all(
        f"""
        SELECT
            fn.id, fn.name, fn.signature, fn.return_type,
            fn.line_start, fn.line_end,
            fn.is_stub, fn.stub_reason,
            fn.has_doc_comment,
            f.filename, f.absolute_path, f.component_id
        FROM functions fn
        JOIN files f ON fn.file_id = f.id
        WHERE {where_clause}
        ORDER BY f.filename, fn.line_start
        """,
        tuple(params)
    )

    return {
        "success": True,
        "count": len(functions),
        "functions": functions
    }


async def link_function_to_feature(
    function_id: int,
    feature_id: int,
    role: str = "implementation"
) -> dict:
    """
    Link a function to a feature.

    Creates a relationship between a function and a feature,
    marking the function's role in implementing that feature.

    Args:
        function_id: Database ID of the function
        feature_id: Database ID of the feature
        role: Role of this function (entry_point, helper, callback, handler)

    Returns:
        Confirmation of the link creation.
    """
    # Verify function exists
    func = await fetch_one("SELECT id, name, is_stub FROM functions WHERE id = ?", (function_id,))
    if not func:
        return {
            "success": False,
            "error": f"Function not found: {function_id}"
        }

    # Verify feature exists
    feature = await fetch_one("SELECT id, name FROM features WHERE id = ?", (feature_id,))
    if not feature:
        return {
            "success": False,
            "error": f"Feature not found: {feature_id}"
        }

    # Check if link already exists
    existing = await fetch_one(
        "SELECT id FROM feature_functions WHERE feature_id = ? AND function_id = ?",
        (feature_id, function_id)
    )

    if existing:
        # Update role
        await update(
            "feature_functions",
            {"role": role, "is_stub": func['is_stub']},
            "id = ?",
            (existing['id'],)
        )
        link_id = existing['id']
        action = "updated"
    else:
        # Create new link
        link_id = await insert(
            "feature_functions",
            {
                "feature_id": feature_id,
                "function_id": function_id,
                "role": role,
                "is_stub": func['is_stub'],
            }
        )
        action = "created"

    return {
        "success": True,
        "action": action,
        "link_id": link_id,
        "function_name": func['name'],
        "feature_name": feature['name'],
        "role": role,
        "is_stub": bool(func['is_stub']),
    }


async def get_function_callers(function_name: str) -> dict:
    """
    Find what functions call the specified function.

    Searches the called_by field in the function index.

    Args:
        function_name: Name of the function to find callers for

    Returns:
        List of functions that call this function.
    """
    # Find the target function
    target = await fetch_one(
        "SELECT id, name, called_by FROM functions WHERE name = ?",
        (function_name,)
    )

    if not target:
        return {
            "success": False,
            "error": f"Function not found: {function_name}"
        }

    # Parse called_by JSON
    caller_names = []
    if target['called_by']:
        try:
            caller_names = json.loads(target['called_by'])
        except:
            pass

    if not caller_names:
        return {
            "success": True,
            "function": function_name,
            "callers": [],
            "count": 0
        }

    # Find caller functions
    # Build IN clause for names
    placeholders = ','.join('?' * len(caller_names))
    callers = await fetch_all(
        f"""
        SELECT
            fn.id, fn.name, fn.signature,
            fn.line_start, fn.line_end,
            f.filename, f.absolute_path
        FROM functions fn
        JOIN files f ON fn.file_id = f.id
        WHERE fn.name IN ({placeholders})
        ORDER BY f.filename, fn.line_start
        """,
        tuple(caller_names)
    )

    return {
        "success": True,
        "function": function_name,
        "callers": callers,
        "count": len(callers),
    }


async def get_function_callees(function_name: str) -> dict:
    """
    Find what functions are called by the specified function.

    Searches the calls field in the function index.

    Args:
        function_name: Name of the function to find callees for

    Returns:
        List of functions called by this function.
    """
    # Find the target function
    target = await fetch_one(
        "SELECT id, name, calls FROM functions WHERE name = ?",
        (function_name,)
    )

    if not target:
        return {
            "success": False,
            "error": f"Function not found: {function_name}"
        }

    # Parse calls JSON
    callee_names = []
    if target['calls']:
        try:
            callee_names = json.loads(target['calls'])
        except:
            pass

    if not callee_names:
        return {
            "success": True,
            "function": function_name,
            "callees": [],
            "count": 0
        }

    # Find callee functions
    placeholders = ','.join('?' * len(callee_names))
    callees = await fetch_all(
        f"""
        SELECT
            fn.id, fn.name, fn.signature,
            fn.line_start, fn.line_end,
            fn.is_stub, fn.stub_reason,
            f.filename, f.absolute_path
        FROM functions fn
        JOIN files f ON fn.file_id = f.id
        WHERE fn.name IN ({placeholders})
        ORDER BY f.filename, fn.line_start
        """,
        tuple(callee_names)
    )

    return {
        "success": True,
        "function": function_name,
        "callees": callees,
        "count": len(callees),
        "stub_count": sum(1 for c in callees if c['is_stub']),
    }


# Export tools
FUNCTION_TOOLS = {
    "find_function": find_function,
    "get_function": get_function,
    "list_functions": list_functions,
    "search_functions": search_functions,
    "link_function_to_feature": link_function_to_feature,
    "get_function_callers": get_function_callers,
    "get_function_callees": get_function_callees,
}
