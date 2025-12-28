"""
Feature management tools for OMBRA Project Management MCP.

THE CRITICAL TOOLS: "Is X implemented?" and "Where is X?"
"""

import json
from typing import Optional, Any
from ..database import fetch_one, fetch_all, insert, update, delete, count, search_fts


async def create_feature(
    name: str,
    description: str,
    category: str,
    priority: str = "P2",
    components: Optional[list[int]] = None,
    estimated_hours: Optional[int] = None,
    blocked_by: Optional[list[int]] = None,
    specification: Optional[str] = None
) -> dict[str, Any]:
    """
    Create a new feature.

    Args:
        name: Feature name (unique)
        description: Detailed description
        category: Category (detection_evasion, memory_virtualization, process_management, etc)
        priority: P0, P1, P2, P3
        components: List of component IDs this feature spans
        estimated_hours: Effort estimate
        blocked_by: List of feature IDs blocking this
        specification: Link to spec doc

    Returns:
        Created feature with ID and timestamps
    """
    # Validate priority
    if priority not in ["P0", "P1", "P2", "P3"]:
        return {
            "success": False,
            "error": "Priority must be P0, P1, P2, or P3"
        }

    # Create short_name from name
    short_name = name.lower().replace(" ", "_").replace("-", "_")

    # Prepare data
    feature_data = {
        "name": name,
        "short_name": short_name,
        "description": description,
        "category": category,
        "priority": priority,
        "specification_doc": specification,
        "estimated_hours": estimated_hours
    }

    # Handle blocked_by as JSON
    if blocked_by:
        feature_data["blocked_by"] = json.dumps(blocked_by)

    try:
        # Insert feature
        feature_id = await insert("features", feature_data)

        # Link to components if provided
        if components:
            for component_id in components:
                # Verify component exists
                comp = await fetch_one(
                    "SELECT id FROM components WHERE id = ?",
                    (component_id,)
                )
                if comp:
                    await insert("feature_components", {
                        "feature_id": feature_id,
                        "component_id": component_id,
                        "role": "primary" if component_id == components[0] else "supporting"
                    })

        # Fetch and return created feature
        feature = await fetch_one(
            "SELECT * FROM features WHERE id = ?",
            (feature_id,)
        )

        # Parse JSON fields
        if feature["blocked_by"]:
            feature["blocked_by"] = json.loads(feature["blocked_by"])
        if feature["blocks"]:
            feature["blocks"] = json.loads(feature["blocks"])

        # Get linked components
        linked_components = await fetch_all(
            """
            SELECT c.id, c.name, fc.role
            FROM components c
            JOIN feature_components fc ON c.id = fc.component_id
            WHERE fc.feature_id = ?
            """,
            (feature_id,)
        )

        return {
            "success": True,
            "feature": feature,
            "linked_components": linked_components
        }

    except Exception as e:
        return {
            "success": False,
            "error": f"Failed to create feature: {str(e)}"
        }


async def update_feature_status(
    feature_id: int,
    status: str,
    implementation_percentage: Optional[int] = None,
    stub_percentage: Optional[int] = None,
    notes: Optional[str] = None
) -> dict[str, Any]:
    """
    Update feature status and implementation metrics.

    Args:
        feature_id: Feature ID to update
        status: New status (not_started, planned, in_progress, implemented, testing, verified, deprecated)
        implementation_percentage: 0-100
        stub_percentage: 0-100 (how much is stubbed vs real)
        notes: Update notes

    Returns:
        Updated feature record
    """
    # Validate status
    valid_statuses = [
        "not_started", "planned", "in_progress", "implemented",
        "testing", "verified", "deprecated"
    ]
    if status not in valid_statuses:
        return {
            "success": False,
            "error": f"Status must be one of: {', '.join(valid_statuses)}"
        }

    # Verify feature exists
    feature = await fetch_one(
        "SELECT * FROM features WHERE id = ?",
        (feature_id,)
    )

    if not feature:
        return {
            "success": False,
            "error": f"Feature {feature_id} not found"
        }

    # Build update data
    update_data = {"status": status}

    if implementation_percentage is not None:
        if not 0 <= implementation_percentage <= 100:
            return {
                "success": False,
                "error": "implementation_percentage must be 0-100"
            }
        update_data["implementation_percentage"] = implementation_percentage

    if stub_percentage is not None:
        if not 0 <= stub_percentage <= 100:
            return {
                "success": False,
                "error": "stub_percentage must be 0-100"
            }
        update_data["stub_percentage"] = stub_percentage

    # Set timestamps based on status
    if status == "in_progress" and not feature["started_at"]:
        update_data["started_at"] = None  # Trigger will set to now

    if status in ["implemented", "verified"] and not feature["completed_at"]:
        update_data["completed_at"] = None  # Trigger will set to now

    try:
        # Update feature
        await update(
            "features",
            update_data,
            "id = ?",
            (feature_id,)
        )

        # Add note to design_notes if provided
        if notes:
            current_notes = feature["design_notes"] or ""
            from datetime import datetime
            timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            new_notes = f"{current_notes}\n\n[{timestamp}] {notes}".strip()
            await update(
                "features",
                {"design_notes": new_notes},
                "id = ?",
                (feature_id,)
            )

        # Fetch updated feature
        updated = await fetch_one(
            "SELECT * FROM features WHERE id = ?",
            (feature_id,)
        )

        # Parse JSON fields
        if updated["blocked_by"]:
            updated["blocked_by"] = json.loads(updated["blocked_by"])
        if updated["blocks"]:
            updated["blocks"] = json.loads(updated["blocks"])

        return {
            "success": True,
            "feature": updated,
            "previous_status": feature["status"]
        }

    except Exception as e:
        return {
            "success": False,
            "error": f"Failed to update feature status: {str(e)}"
        }


async def get_feature(feature_id: int) -> dict[str, Any]:
    """
    Get complete feature information including linked files, functions, stubs, tasks.

    Args:
        feature_id: Feature ID to query

    Returns:
        Complete feature data with all linkages
    """
    # Get base feature
    feature = await fetch_one(
        "SELECT * FROM features WHERE id = ?",
        (feature_id,)
    )

    if not feature:
        return {
            "success": False,
            "error": f"Feature {feature_id} not found"
        }

    # Parse JSON arrays
    if feature["blocked_by"]:
        feature["blocked_by"] = json.loads(feature["blocked_by"])
    if feature["blocks"]:
        feature["blocks"] = json.loads(feature["blocks"])

    # Get linked components
    components = await fetch_all(
        """
        SELECT c.*, fc.role
        FROM components c
        JOIN feature_components fc ON c.id = fc.component_id
        WHERE fc.feature_id = ?
        """,
        (feature_id,)
    )

    # Get linked files
    files = await fetch_all(
        """
        SELECT
            f.*,
            ff.role,
            ff.relevant_functions,
            ff.line_ranges,
            ff.confidence,
            ff.manually_linked
        FROM files f
        JOIN feature_files ff ON f.id = ff.file_id
        WHERE ff.feature_id = ?
        """,
        (feature_id,)
    )

    # Parse JSON in files
    for file in files:
        file["relevant_functions"] = json.loads(file["relevant_functions"]) if file["relevant_functions"] else []
        file["line_ranges"] = json.loads(file["line_ranges"]) if file["line_ranges"] else []

    # Get linked functions
    functions = await fetch_all(
        """
        SELECT
            func.*,
            ff.role,
            ff.is_stub,
            f.filename,
            f.relative_path
        FROM functions func
        JOIN feature_functions ff ON func.id = ff.function_id
        JOIN files f ON func.file_id = f.id
        WHERE ff.feature_id = ?
        """,
        (feature_id,)
    )

    # Get stubs for this feature
    stubs = await fetch_all(
        """
        SELECT
            s.*,
            f.filename,
            f.relative_path
        FROM stubs s
        JOIN files f ON s.file_id = f.id
        WHERE s.feature_id = ? AND s.resolved_at IS NULL
        ORDER BY
            CASE s.severity
                WHEN 'critical' THEN 1
                WHEN 'high' THEN 2
                WHEN 'medium' THEN 3
                ELSE 4
            END
        """,
        (feature_id,)
    )

    # Get tasks
    tasks = await fetch_all(
        """
        SELECT
            id,
            title,
            status,
            priority,
            task_type,
            assigned_to,
            created_at,
            due_date
        FROM tasks
        WHERE feature_id = ?
        ORDER BY
            CASE priority
                WHEN 'P0' THEN 1
                WHEN 'P1' THEN 2
                WHEN 'P2' THEN 3
                WHEN 'P3' THEN 4
                ELSE 5
            END,
            created_at
        """,
        (feature_id,)
    )

    # Get implementation status per component
    impl_status = await fetch_all(
        """
        SELECT
            is.*,
            c.name as component_name
        FROM implementation_status is
        JOIN components c ON is.component_id = c.id
        WHERE is.feature_id = ?
        """,
        (feature_id,)
    )

    # Parse JSON in implementation status
    for impl in impl_status:
        impl["implementing_files"] = json.loads(impl["implementing_files"]) if impl["implementing_files"] else []
        impl["implementing_functions"] = json.loads(impl["implementing_functions"]) if impl["implementing_functions"] else []
        impl["blockers"] = json.loads(impl["blockers"]) if impl["blockers"] else []
        impl["warnings"] = json.loads(impl["warnings"]) if impl["warnings"] else []

    return {
        "success": True,
        "feature": feature,
        "components": components,
        "files": files,
        "functions": functions,
        "stubs": stubs,
        "tasks": tasks,
        "implementation_status": impl_status,
        "summary": {
            "component_count": len(components),
            "file_count": len(files),
            "function_count": len(functions),
            "unresolved_stubs": len(stubs),
            "task_count": len(tasks),
            "tasks_open": sum(1 for t in tasks if t["status"] not in ["done", "cancelled"])
        }
    }


async def search_features(
    query: str,
    status: Optional[str] = None,
    category: Optional[str] = None,
    component_id: Optional[int] = None,
    include_stubs: bool = True
) -> dict[str, Any]:
    """
    Semantic search for features with optional filters.

    Args:
        query: Search query
        status: Filter by status
        category: Filter by category
        component_id: Filter by component
        include_stubs: Include stub statistics

    Returns:
        Matching features with relevance scores
    """
    # Use FTS for fast text search
    fts_results = await search_fts("features_fts", query, limit=50)

    if not fts_results:
        return {
            "success": True,
            "features": [],
            "total_count": 0
        }

    # Get full feature details
    feature_ids = [r["rowid"] for r in fts_results]
    placeholders = ",".join("?" * len(feature_ids))

    # Build WHERE clause
    where_parts = [f"f.id IN ({placeholders})"]
    params = list(feature_ids)

    if status:
        where_parts.append("f.status = ?")
        params.append(status)

    if category:
        where_parts.append("f.category = ?")
        params.append(category)

    if component_id:
        where_parts.append(
            "EXISTS (SELECT 1 FROM feature_components fc WHERE fc.feature_id = f.id AND fc.component_id = ?)"
        )
        params.append(component_id)

    where_clause = " AND ".join(where_parts)

    # Fetch features
    features = await fetch_all(
        f"""
        SELECT f.*
        FROM features f
        WHERE {where_clause}
        ORDER BY f.priority, f.name
        """,
        tuple(params)
    )

    # Parse JSON and enrich
    enriched = []
    for feat in features:
        # Parse JSON
        if feat["blocked_by"]:
            feat["blocked_by"] = json.loads(feat["blocked_by"])
        if feat["blocks"]:
            feat["blocks"] = json.loads(feat["blocks"])

        # Get stub count if requested
        if include_stubs:
            stub_count = await count(
                "stubs",
                "feature_id = ? AND resolved_at IS NULL",
                (feat["id"],)
            )
            feat["unresolved_stubs"] = stub_count

        # Get component names
        components = await fetch_all(
            """
            SELECT c.name, fc.role
            FROM components c
            JOIN feature_components fc ON c.id = fc.component_id
            WHERE fc.feature_id = ?
            """,
            (feat["id"],)
        )
        feat["components"] = components

        enriched.append(feat)

    return {
        "success": True,
        "features": enriched,
        "total_count": len(enriched),
        "query": query
    }


async def is_feature_implemented(
    feature_name: Optional[str] = None,
    feature_id: Optional[int] = None
) -> dict[str, Any]:
    """
    THE CRITICAL QUERY: Is this feature implemented?

    Args:
        feature_name: Feature name to check (exact or fuzzy)
        feature_id: Feature ID to check

    Returns:
        Implementation status with evidence and stub counts
    """
    if not feature_name and not feature_id:
        return {
            "success": False,
            "error": "Must provide either feature_name or feature_id"
        }

    # Find feature
    if feature_id:
        feature = await fetch_one(
            "SELECT * FROM features WHERE id = ?",
            (feature_id,)
        )
    else:
        # Try exact match first
        feature = await fetch_one(
            "SELECT * FROM features WHERE name = ? OR short_name = ?",
            (feature_name, feature_name)
        )

        # Fuzzy search if no exact match
        if not feature:
            fts_results = await search_fts("features_fts", feature_name, limit=1)
            if fts_results:
                feature = await fetch_one(
                    "SELECT * FROM features WHERE id = ?",
                    (fts_results[0]["rowid"],)
                )

    if not feature:
        return {
            "success": False,
            "error": f"Feature not found: {feature_name or feature_id}"
        }

    # Parse JSON
    if feature["blocked_by"]:
        feature["blocked_by"] = json.loads(feature["blocked_by"])

    # Get implementation evidence
    file_count = await count(
        "feature_files",
        "feature_id = ?",
        (feature["id"],)
    )

    function_count = await count(
        "feature_functions",
        "feature_id = ?",
        (feature["id"],)
    )

    stub_count = await count(
        "stubs",
        "feature_id = ? AND resolved_at IS NULL",
        (feature["id"],)
    )

    # Get stub functions
    stub_functions = await fetch_all(
        """
        SELECT
            func.name,
            func.signature,
            f.filename,
            func.stub_reason
        FROM functions func
        JOIN feature_functions ff ON func.id = ff.function_id
        JOIN files f ON func.file_id = f.id
        WHERE ff.feature_id = ? AND func.is_stub = 1
        """,
        (feature["id"],)
    )

    # Determine implementation verdict
    is_implemented = (
        feature["status"] in ["implemented", "testing", "verified"] and
        feature["implementation_percentage"] >= 80 and
        stub_count < 5
    )

    is_partial = (
        feature["status"] == "in_progress" and
        feature["implementation_percentage"] >= 30 and
        file_count > 0
    )

    # Build verdict
    if is_implemented:
        verdict = "IMPLEMENTED"
        confidence = min(feature["implementation_percentage"], 100 - feature["stub_percentage"])
    elif is_partial:
        verdict = "PARTIAL"
        confidence = feature["implementation_percentage"]
    else:
        verdict = "NOT_IMPLEMENTED"
        confidence = 100 - feature["implementation_percentage"]

    return {
        "success": True,
        "feature": {
            "id": feature["id"],
            "name": feature["name"],
            "status": feature["status"]
        },
        "verdict": verdict,
        "confidence": confidence,
        "evidence": {
            "implementation_percentage": feature["implementation_percentage"],
            "stub_percentage": feature["stub_percentage"],
            "file_count": file_count,
            "function_count": function_count,
            "unresolved_stubs": stub_count,
            "stub_functions": stub_functions
        },
        "blockers": feature["blocked_by"] if feature["blocked_by"] else []
    }


async def get_feature_files(feature_id: int) -> dict[str, Any]:
    """
    Get all files implementing a feature.

    Args:
        feature_id: Feature ID

    Returns:
        List of files with roles and line ranges
    """
    feature = await fetch_one(
        "SELECT name FROM features WHERE id = ?",
        (feature_id,)
    )

    if not feature:
        return {
            "success": False,
            "error": f"Feature {feature_id} not found"
        }

    files = await fetch_all(
        """
        SELECT
            f.id,
            f.filename,
            f.relative_path,
            f.absolute_path,
            f.language,
            f.line_count,
            ff.role,
            ff.relevant_functions,
            ff.line_ranges,
            ff.confidence,
            ff.manually_linked
        FROM files f
        JOIN feature_files ff ON f.id = ff.file_id
        WHERE ff.feature_id = ?
        ORDER BY
            CASE ff.role
                WHEN 'implementation' THEN 1
                WHEN 'header' THEN 2
                WHEN 'test' THEN 3
                ELSE 4
            END,
            f.relative_path
        """,
        (feature_id,)
    )

    # Parse JSON
    for file in files:
        file["relevant_functions"] = json.loads(file["relevant_functions"]) if file["relevant_functions"] else []
        file["line_ranges"] = json.loads(file["line_ranges"]) if file["line_ranges"] else []

    return {
        "success": True,
        "feature_name": feature["name"],
        "files": files,
        "total_count": len(files)
    }


async def link_file_to_feature(
    feature_id: int,
    file_path: str,
    role: str = "implementation",
    functions: Optional[list[str]] = None,
    line_ranges: Optional[list[str]] = None,
    confidence: float = 1.0
) -> dict[str, Any]:
    """
    Link a file to a feature (manual or automated).

    Args:
        feature_id: Feature ID
        file_path: Absolute path to file
        role: File role (implementation, header, test, example, documentation)
        functions: List of function names in this file for this feature
        line_ranges: List of line ranges (e.g., ["100-150", "200-250"])
        confidence: Confidence score 0-1

    Returns:
        Created link record
    """
    # Verify feature exists
    feature = await fetch_one(
        "SELECT id FROM features WHERE id = ?",
        (feature_id,)
    )

    if not feature:
        return {
            "success": False,
            "error": f"Feature {feature_id} not found"
        }

    # Find or create file
    file = await fetch_one(
        "SELECT id FROM files WHERE absolute_path = ?",
        (file_path,)
    )

    if not file:
        # Auto-create file record
        from pathlib import Path
        p = Path(file_path)
        file_id = await insert("files", {
            "absolute_path": file_path,
            "filename": p.name,
            "extension": p.suffix[1:] if p.suffix else None,
            "language": p.suffix[1:] if p.suffix in [".c", ".cpp", ".h", ".hpp", ".asm"] else "unknown"
        })
    else:
        file_id = file["id"]

    # Check for existing link
    existing = await fetch_one(
        "SELECT id FROM feature_files WHERE feature_id = ? AND file_id = ?",
        (feature_id, file_id)
    )

    if existing:
        return {
            "success": False,
            "error": "File already linked to this feature",
            "existing_link_id": existing["id"]
        }

    # Create link
    try:
        link_data = {
            "feature_id": feature_id,
            "file_id": file_id,
            "role": role,
            "confidence": confidence,
            "manually_linked": 1
        }

        if functions:
            link_data["relevant_functions"] = json.dumps(functions)

        if line_ranges:
            link_data["line_ranges"] = json.dumps(line_ranges)

        link_id = await insert("feature_files", link_data)

        # Fetch created link
        link = await fetch_one(
            """
            SELECT
                ff.*,
                f.filename,
                f.relative_path
            FROM feature_files ff
            JOIN files f ON ff.file_id = f.id
            WHERE ff.id = ?
            """,
            (link_id,)
        )

        # Parse JSON
        link["relevant_functions"] = json.loads(link["relevant_functions"]) if link["relevant_functions"] else []
        link["line_ranges"] = json.loads(link["line_ranges"]) if link["line_ranges"] else []

        return {
            "success": True,
            "link": link
        }

    except Exception as e:
        return {
            "success": False,
            "error": f"Failed to link file: {str(e)}"
        }


async def where_is_feature(feature_name: str) -> dict[str, Any]:
    """
    THE AGENT'S BEST FRIEND: Where is X, what does it do, is it implemented?

    Args:
        feature_name: Feature name to locate

    Returns:
        Complete feature location, description, status, and file locations
    """
    # Find feature (fuzzy)
    fts_results = await search_fts("features_fts", feature_name, limit=1)

    if not fts_results:
        return {
            "success": False,
            "error": f"Feature not found: {feature_name}",
            "suggestion": "Try searching with a broader query"
        }

    feature_id = fts_results[0]["rowid"]

    # Get complete feature info
    feature = await fetch_one(
        "SELECT * FROM features WHERE id = ?",
        (feature_id,)
    )

    # Parse JSON
    if feature["blocked_by"]:
        feature["blocked_by"] = json.loads(feature["blocked_by"])

    # Get files
    files = await fetch_all(
        """
        SELECT
            f.absolute_path,
            f.relative_path,
            f.filename,
            ff.role,
            ff.relevant_functions,
            ff.line_ranges
        FROM files f
        JOIN feature_files ff ON f.id = ff.file_id
        WHERE ff.feature_id = ?
        ORDER BY
            CASE ff.role
                WHEN 'implementation' THEN 1
                WHEN 'header' THEN 2
                ELSE 3
            END
        """,
        (feature_id,)
    )

    # Parse JSON in files
    for file in files:
        file["relevant_functions"] = json.loads(file["relevant_functions"]) if file["relevant_functions"] else []
        file["line_ranges"] = json.loads(file["line_ranges"]) if file["line_ranges"] else []

    # Get components
    components = await fetch_all(
        """
        SELECT c.name, c.root_path, fc.role
        FROM components c
        JOIN feature_components fc ON c.id = fc.component_id
        WHERE fc.feature_id = ?
        """,
        (feature_id,)
    )

    # Get key functions
    functions = await fetch_all(
        """
        SELECT
            func.name,
            func.signature,
            func.is_stub,
            f.relative_path,
            func.line_start
        FROM functions func
        JOIN feature_functions ff ON func.id = ff.function_id
        JOIN files f ON func.file_id = f.id
        WHERE ff.feature_id = ?
        ORDER BY ff.role
        LIMIT 10
        """,
        (feature_id,)
    )

    return {
        "success": True,
        "feature": {
            "name": feature["name"],
            "description": feature["description"],
            "status": feature["status"],
            "implementation_percentage": feature["implementation_percentage"],
            "stub_percentage": feature["stub_percentage"],
            "category": feature["category"],
            "priority": feature["priority"]
        },
        "location": {
            "components": components,
            "files": files,
            "key_functions": functions
        },
        "quick_answer": f"{feature['name']} is {feature['status']} ({feature['implementation_percentage']}% complete) in {len(files)} files across {len(components)} components"
    }


# Export tools dict
FEATURE_TOOLS = {
    "create_feature": create_feature,
    "update_feature_status": update_feature_status,
    "get_feature": get_feature,
    "search_features": search_features,
    "is_feature_implemented": is_feature_implemented,
    "get_feature_files": get_feature_files,
    "link_file_to_feature": link_file_to_feature,
    "where_is_feature": where_is_feature,
}
