"""
Component management tools for OMBRA Project Management MCP.

Handles top-level component registration, status tracking, and lifecycle.
"""

import json
from typing import Optional, Any
from ..database import fetch_one, fetch_all, insert, update, delete, count


async def register_component(
    name: str,
    component_type: str,
    description: str,
    root_path: str,
    language: str = "c",
    build_system: str = "msbuild"
) -> dict[str, Any]:
    """
    Register a top-level component in the project.

    Args:
        name: Component name (unique)
        component_type: Type - hypervisor, kernel_lib, usermode_api, kernel_driver,
                        vuln_driver, bootloader, payload, common, installer, gui
        description: Component description
        root_path: Base directory path
        language: Primary language - c, cpp, asm, mixed, python
        build_system: Build system - msbuild, cmake, make, custom

    Returns:
        Created component with ID and timestamps
    """
    # Validate component type
    valid_types = [
        "hypervisor", "kernel_lib", "usermode_api", "kernel_driver",
        "vuln_driver", "bootloader", "payload", "common", "installer", "gui"
    ]
    if component_type not in valid_types:
        return {
            "success": False,
            "error": f"Invalid component_type. Must be one of: {', '.join(valid_types)}"
        }

    # Check for duplicates
    existing = await fetch_one(
        "SELECT id, name FROM components WHERE name = ?",
        (name,)
    )
    if existing:
        return {
            "success": False,
            "error": f"Component '{name}' already exists with ID {existing['id']}"
        }

    # Insert component
    try:
        component_id = await insert("components", {
            "name": name,
            "component_type": component_type,
            "description": description,
            "root_path": root_path,
            "language": language,
            "build_system": build_system
        })

        # Fetch and return the created component
        component = await fetch_one(
            "SELECT * FROM components WHERE id = ?",
            (component_id,)
        )

        return {
            "success": True,
            "component": component
        }

    except Exception as e:
        return {
            "success": False,
            "error": f"Failed to register component: {str(e)}"
        }


async def list_components() -> dict[str, Any]:
    """
    List all registered components with status summary.

    Returns:
        List of components with feature counts, module counts, and implementation stats
    """
    components = await fetch_all(
        """
        SELECT
            c.id,
            c.name,
            c.component_type,
            c.description,
            c.root_path,
            c.language,
            c.build_system,
            c.created_at,
            c.updated_at
        FROM components c
        ORDER BY c.name
        """
    )

    # Enrich each component with stats
    enriched = []
    for comp in components:
        # Get feature counts
        feature_count = await count(
            "feature_components",
            "component_id = ?",
            (comp["id"],)
        )

        # Get module count
        module_count = await count(
            "modules",
            "component_id = ?",
            (comp["id"],)
        )

        # Get file count
        file_count = await count(
            "files",
            "component_id = ?",
            (comp["id"],)
        )

        # Get implementation status breakdown
        status_breakdown = await fetch_all(
            """
            SELECT
                status,
                COUNT(*) as count
            FROM implementation_status
            WHERE component_id = ?
            GROUP BY status
            """,
            (comp["id"],)
        )

        # Get stub statistics from files
        stub_stats = await fetch_one(
            """
            SELECT
                COALESCE(SUM(stub_line_count), 0) as total_stub_lines,
                COALESCE(SUM(line_count), 0) as total_lines,
                COALESCE(SUM(todo_count), 0) as total_todos,
                COALESCE(SUM(fixme_count), 0) as total_fixmes
            FROM files
            WHERE component_id = ?
            """,
            (comp["id"],)
        )

        enriched.append({
            **comp,
            "feature_count": feature_count,
            "module_count": module_count,
            "file_count": file_count,
            "status_breakdown": {s["status"]: s["count"] for s in status_breakdown},
            "stub_stats": stub_stats
        })

    return {
        "success": True,
        "components": enriched,
        "total_count": len(enriched)
    }


async def get_component_status(component_id: int) -> dict[str, Any]:
    """
    Get detailed status for a component including all features and implementation state.

    Args:
        component_id: Component ID to query

    Returns:
        Complete component status with features, modules, files, stubs
    """
    # Get base component
    component = await fetch_one(
        "SELECT * FROM components WHERE id = ?",
        (component_id,)
    )

    if not component:
        return {
            "success": False,
            "error": f"Component {component_id} not found"
        }

    # Get modules
    modules = await fetch_all(
        """
        SELECT
            id,
            name,
            description,
            path,
            implementation_status,
            stub_percentage,
            created_at,
            updated_at
        FROM modules
        WHERE component_id = ?
        ORDER BY name
        """,
        (component_id,)
    )

    # Get linked features
    features = await fetch_all(
        """
        SELECT
            f.id,
            f.name,
            f.short_name,
            f.status,
            f.implementation_percentage,
            f.stub_percentage,
            f.priority,
            f.category,
            fc.role
        FROM features f
        JOIN feature_components fc ON f.id = fc.feature_id
        WHERE fc.component_id = ?
        ORDER BY f.priority, f.name
        """,
        (component_id,)
    )

    # Get implementation status details
    impl_status = await fetch_all(
        """
        SELECT
            is.*,
            f.name as feature_name,
            f.short_name as feature_short_name
        FROM implementation_status is
        JOIN features f ON is.feature_id = f.id
        WHERE is.component_id = ?
        """,
        (component_id,)
    )

    # Parse JSON arrays in implementation status
    for impl in impl_status:
        impl["implementing_files"] = json.loads(impl["implementing_files"]) if impl["implementing_files"] else []
        impl["implementing_functions"] = json.loads(impl["implementing_functions"]) if impl["implementing_functions"] else []
        impl["blockers"] = json.loads(impl["blockers"]) if impl["blockers"] else []
        impl["warnings"] = json.loads(impl["warnings"]) if impl["warnings"] else []

    # Get files
    files = await fetch_all(
        """
        SELECT
            id,
            filename,
            relative_path,
            language,
            line_count,
            function_count,
            stub_line_count,
            todo_count,
            fixme_count,
            last_indexed
        FROM files
        WHERE component_id = ?
        ORDER BY relative_path
        """,
        (component_id,)
    )

    # Get stub summary
    stubs = await fetch_all(
        """
        SELECT
            s.stub_type,
            s.severity,
            COUNT(*) as count
        FROM stubs s
        JOIN files f ON s.file_id = f.id
        WHERE f.component_id = ? AND s.resolved_at IS NULL
        GROUP BY s.stub_type, s.severity
        """,
        (component_id,)
    )

    # Get recommendations
    recommendations = await fetch_all(
        """
        SELECT
            id,
            recommendation_type,
            severity,
            title,
            status,
            generated_at
        FROM recommendations
        WHERE affected_component_id = ? AND status = 'open'
        ORDER BY
            CASE severity
                WHEN 'critical' THEN 1
                WHEN 'warning' THEN 2
                WHEN 'suggestion' THEN 3
                ELSE 4
            END,
            generated_at DESC
        LIMIT 10
        """,
        (component_id,)
    )

    return {
        "success": True,
        "component": component,
        "modules": modules,
        "features": features,
        "implementation_status": impl_status,
        "files": files,
        "stubs": stubs,
        "recommendations": recommendations,
        "summary": {
            "module_count": len(modules),
            "feature_count": len(features),
            "file_count": len(files),
            "unresolved_stubs": sum(s["count"] for s in stubs),
            "open_recommendations": len(recommendations)
        }
    }


async def update_component(
    component_id: int,
    **kwargs
) -> dict[str, Any]:
    """
    Update component fields.

    Args:
        component_id: Component ID to update
        **kwargs: Fields to update (name, description, component_type, root_path, language, build_system)

    Returns:
        Updated component record
    """
    # Verify component exists
    component = await fetch_one(
        "SELECT id FROM components WHERE id = ?",
        (component_id,)
    )

    if not component:
        return {
            "success": False,
            "error": f"Component {component_id} not found"
        }

    # Filter valid fields
    valid_fields = {
        "name", "description", "component_type",
        "root_path", "language", "build_system"
    }
    update_data = {k: v for k, v in kwargs.items() if k in valid_fields}

    if not update_data:
        return {
            "success": False,
            "error": "No valid fields provided for update"
        }

    # Check for name conflicts if renaming
    if "name" in update_data:
        existing = await fetch_one(
            "SELECT id FROM components WHERE name = ? AND id != ?",
            (update_data["name"], component_id)
        )
        if existing:
            return {
                "success": False,
                "error": f"Component name '{update_data['name']}' already exists"
            }

    # Update
    try:
        rows_affected = await update(
            "components",
            update_data,
            "id = ?",
            (component_id,)
        )

        # Fetch updated component
        updated = await fetch_one(
            "SELECT * FROM components WHERE id = ?",
            (component_id,)
        )

        return {
            "success": True,
            "component": updated,
            "rows_affected": rows_affected
        }

    except Exception as e:
        return {
            "success": False,
            "error": f"Failed to update component: {str(e)}"
        }


async def delete_component(component_id: int) -> dict[str, Any]:
    """
    Delete a component (cascades to modules, files, etc).

    Args:
        component_id: Component ID to delete

    Returns:
        Deletion confirmation with cascade summary
    """
    # Verify component exists and get details
    component = await fetch_one(
        "SELECT name FROM components WHERE id = ?",
        (component_id,)
    )

    if not component:
        return {
            "success": False,
            "error": f"Component {component_id} not found"
        }

    # Get counts before deletion (for reporting)
    module_count = await count("modules", "component_id = ?", (component_id,))
    file_count = await count("files", "component_id = ?", (component_id,))
    feature_link_count = await count("feature_components", "component_id = ?", (component_id,))

    # Delete (cascades via foreign keys)
    try:
        rows_deleted = await delete(
            "components",
            "id = ?",
            (component_id,)
        )

        return {
            "success": True,
            "deleted_component": component["name"],
            "cascade_summary": {
                "modules_deleted": module_count,
                "files_deleted": file_count,
                "feature_links_deleted": feature_link_count
            }
        }

    except Exception as e:
        return {
            "success": False,
            "error": f"Failed to delete component: {str(e)}"
        }


# Export tools dict
COMPONENT_TOOLS = {
    "register_component": register_component,
    "list_components": list_components,
    "get_component_status": get_component_status,
    "update_component": update_component,
    "delete_component": delete_component,
}
