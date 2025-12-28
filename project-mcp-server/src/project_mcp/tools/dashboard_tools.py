"""
Dashboard Tools - Aggregate data for dashboard consumption

Tools for getting comprehensive dashboard data with minimal round trips.
"""

from ..database import fetch_one, fetch_all, count
import json


async def get_dashboard_overview() -> dict:
    """
    Get all data needed for main dashboard in a single call.

    Returns:
        Complete dashboard data
    """
    # Component summary
    components = await fetch_all("""
        SELECT
            c.id,
            c.name,
            c.component_type,
            c.language,
            COUNT(DISTINCT m.id) as module_count,
            COUNT(DISTINCT f.id) as file_count,
            SUM(f.line_count) as total_loc
        FROM components c
        LEFT JOIN modules m ON c.id = m.component_id
        LEFT JOIN files f ON c.id = f.component_id
        GROUP BY c.id
        ORDER BY c.name
    """)

    # Feature breakdown by status
    feature_stats = await fetch_all("""
        SELECT
            status,
            priority,
            COUNT(*) as count
        FROM features
        GROUP BY status, priority
        ORDER BY
            CASE priority
                WHEN 'P0' THEN 0
                WHEN 'P1' THEN 1
                WHEN 'P2' THEN 2
                WHEN 'P3' THEN 3
                ELSE 4
            END,
            status
    """)

    # Aggregate feature totals
    total_features = await count("features")
    implemented_features = await count("features", "status IN ('implemented', 'testing', 'verified')")
    in_progress_features = await count("features", "status = 'in_progress'")
    not_started_features = await count("features", "status = 'not_started'")

    # Stub metrics
    total_functions = await count("functions")
    stub_functions = await count("functions", "is_stub = 1")
    critical_stubs = await count("""
        stubs s
        JOIN functions f ON s.function_id = f.id
        JOIN feature_functions ff ON f.id = ff.function_id
        JOIN features feat ON ff.feature_id = feat.id
    """, "s.resolved_at IS NULL AND feat.priority IN ('P0', 'P1')")

    stub_percentage = (stub_functions / total_functions * 100) if total_functions > 0 else 0

    # Recent activity from audit log
    recent_activity = await fetch_all("""
        SELECT
            entity_type,
            entity_id,
            action,
            performed_by,
            performed_at,
            notes
        FROM audit_log
        ORDER BY performed_at DESC
        LIMIT 10
    """)

    # Enrich activity with entity names
    for activity in recent_activity:
        if activity["entity_type"] == "feature":
            feat = await fetch_one(
                "SELECT name FROM features WHERE id = ?",
                (activity["entity_id"],)
            )
            activity["entity_name"] = feat["name"] if feat else "Unknown"
        elif activity["entity_type"] == "task":
            task = await fetch_one(
                "SELECT title FROM tasks WHERE id = ?",
                (activity["entity_id"],)
            )
            activity["entity_name"] = task["title"] if task else "Unknown"
        elif activity["entity_type"] == "file":
            file_rec = await fetch_one(
                "SELECT filename FROM files WHERE id = ?",
                (activity["entity_id"],)
            )
            activity["entity_name"] = file_rec["filename"] if file_rec else "Unknown"

    # Open P0/P1 tasks
    priority_tasks = await fetch_all("""
        SELECT
            t.id,
            t.title,
            t.priority,
            t.status,
            t.task_type,
            t.created_at,
            t.due_date,
            f.name as feature_name,
            c.name as component_name
        FROM tasks t
        LEFT JOIN features f ON t.feature_id = f.id
        LEFT JOIN components c ON t.component_id = c.id
        WHERE t.priority IN ('P0', 'P1')
        AND t.status NOT IN ('done', 'cancelled')
        ORDER BY
            CASE t.priority
                WHEN 'P0' THEN 0
                WHEN 'P1' THEN 1
                ELSE 2
            END,
            t.created_at ASC
    """)

    # Open recommendations
    open_recommendations = await fetch_all("""
        SELECT
            r.id,
            r.title,
            r.severity,
            r.recommendation_type,
            r.generated_at,
            f.name as feature_name
        FROM recommendations r
        LEFT JOIN features f ON r.affected_feature_id = f.id
        WHERE r.status = 'open'
        ORDER BY
            CASE r.severity
                WHEN 'critical' THEN 0
                WHEN 'warning' THEN 1
                WHEN 'suggestion' THEN 2
                ELSE 3
            END,
            r.generated_at DESC
        LIMIT 20
    """)

    # Overall stats
    total_files = await count("files")
    total_loc_result = await fetch_one("SELECT SUM(line_count) as total FROM files")
    total_loc = total_loc_result["total"] or 0

    total_todos = await fetch_one("SELECT SUM(todo_count) as total FROM files")
    total_fixmes = await fetch_one("SELECT SUM(fixme_count) as total FROM files")

    return {
        "overview": {
            "total_features": total_features,
            "implemented_features": implemented_features,
            "in_progress_features": in_progress_features,
            "not_started_features": not_started_features,
            "total_files": total_files,
            "total_loc": total_loc,
            "total_functions": total_functions,
            "stub_functions": stub_functions,
            "stub_percentage": round(stub_percentage, 2),
            "critical_stubs": critical_stubs,
            "total_todos": total_todos["total"] or 0,
            "total_fixmes": total_fixmes["total"] or 0
        },
        "components": components,
        "feature_breakdown": feature_stats,
        "recent_activity": recent_activity,
        "priority_tasks": priority_tasks,
        "open_recommendations": open_recommendations
    }


async def get_component_tree() -> list[dict]:
    """
    Get component hierarchy with modules and features for tree view.

    Returns:
        Nested component tree structure
    """
    # Get all components
    components = await fetch_all("""
        SELECT
            id,
            name,
            description,
            component_type,
            root_path,
            language
        FROM components
        ORDER BY name
    """)

    tree = []

    for component in components:
        # Get modules for this component
        modules = await fetch_all("""
            SELECT
                id,
                name,
                description,
                implementation_status,
                stub_percentage
            FROM modules
            WHERE component_id = ?
            ORDER BY name
        """, (component["id"],))

        # Get features linked to this component
        features = await fetch_all("""
            SELECT
                f.id,
                f.name,
                f.short_name,
                f.priority,
                f.status,
                f.implementation_percentage,
                fc.role
            FROM features f
            JOIN feature_components fc ON f.id = fc.feature_id
            WHERE fc.component_id = ?
            ORDER BY
                CASE f.priority
                    WHEN 'P0' THEN 0
                    WHEN 'P1' THEN 1
                    WHEN 'P2' THEN 2
                    WHEN 'P3' THEN 3
                    ELSE 4
                END,
                f.name
        """, (component["id"],))

        # Get file count
        file_count = await count("files", "component_id = ?", (component["id"],))

        tree.append({
            "id": component["id"],
            "name": component["name"],
            "description": component["description"],
            "type": component["component_type"],
            "root_path": component["root_path"],
            "language": component["language"],
            "file_count": file_count,
            "modules": modules,
            "features": features
        })

    return tree


async def get_feature_matrix() -> dict:
    """
    Get feature-component matrix showing which features are in which components.

    Returns:
        Matrix data for heatmap/table visualization
    """
    # Get all components
    components = await fetch_all(
        "SELECT id, name FROM components ORDER BY name"
    )

    # Get all features
    features = await fetch_all("""
        SELECT
            id,
            name,
            short_name,
            priority,
            status
        FROM features
        ORDER BY
            CASE priority
                WHEN 'P0' THEN 0
                WHEN 'P1' THEN 1
                WHEN 'P2' THEN 2
                WHEN 'P3' THEN 3
                ELSE 4
            END,
            name
    """)

    # Build matrix
    matrix = []

    for feature in features:
        row = {
            "feature_id": feature["id"],
            "feature_name": feature["name"],
            "short_name": feature["short_name"],
            "priority": feature["priority"],
            "status": feature["status"],
            "components": {}
        }

        # Get component involvement
        component_links = await fetch_all("""
            SELECT
                fc.component_id,
                fc.role,
                ist.status as impl_status,
                ist.percentage as impl_percentage
            FROM feature_components fc
            LEFT JOIN implementation_status ist
                ON fc.feature_id = ist.feature_id
                AND fc.component_id = ist.component_id
            WHERE fc.feature_id = ?
        """, (feature["id"],))

        for link in component_links:
            row["components"][link["component_id"]] = {
                "role": link["role"],
                "status": link["impl_status"],
                "percentage": link["impl_percentage"]
            }

        matrix.append(row)

    return {
        "components": components,
        "features": features,
        "matrix": matrix
    }


async def get_activity_feed(limit: int = 20) -> list[dict]:
    """
    Get recent activity feed from audit log.

    Args:
        limit: Number of entries to return

    Returns:
        Activity feed entries
    """
    activities = await fetch_all("""
        SELECT
            entity_type,
            entity_id,
            action,
            old_value,
            new_value,
            performed_by,
            performed_at,
            notes
        FROM audit_log
        ORDER BY performed_at DESC
        LIMIT ?
    """, (limit,))

    # Enrich with entity details
    for activity in activities:
        entity_type = activity["entity_type"]
        entity_id = activity["entity_id"]

        # Get entity name based on type
        if entity_type == "feature":
            entity = await fetch_one(
                "SELECT name, priority FROM features WHERE id = ?",
                (entity_id,)
            )
            if entity:
                activity["entity_name"] = entity["name"]
                activity["entity_priority"] = entity["priority"]

        elif entity_type == "task":
            entity = await fetch_one(
                "SELECT title, priority FROM tasks WHERE id = ?",
                (entity_id,)
            )
            if entity:
                activity["entity_name"] = entity["title"]
                activity["entity_priority"] = entity["priority"]

        elif entity_type == "file":
            entity = await fetch_one(
                "SELECT filename, relative_path FROM files WHERE id = ?",
                (entity_id,)
            )
            if entity:
                activity["entity_name"] = entity["filename"]
                activity["entity_path"] = entity["relative_path"]

        elif entity_type == "component":
            entity = await fetch_one(
                "SELECT name FROM components WHERE id = ?",
                (entity_id,)
            )
            if entity:
                activity["entity_name"] = entity["name"]

        # Parse JSON values if present
        if activity["old_value"]:
            try:
                activity["old_value"] = json.loads(activity["old_value"])
            except:
                pass

        if activity["new_value"]:
            try:
                activity["new_value"] = json.loads(activity["new_value"])
            except:
                pass

    return activities


async def get_priority_items() -> dict:
    """
    Get high-priority items requiring attention.

    Returns:
        Priority tasks, stubs, and blockers
    """
    # P0/P1 tasks
    priority_tasks = await fetch_all("""
        SELECT
            t.id,
            t.title,
            t.priority,
            t.status,
            t.task_type,
            t.created_at,
            t.due_date,
            t.estimated_hours,
            f.name as feature_name,
            c.name as component_name
        FROM tasks t
        LEFT JOIN features f ON t.feature_id = f.id
        LEFT JOIN components c ON t.component_id = c.id
        WHERE t.priority IN ('P0', 'P1')
        AND t.status NOT IN ('done', 'cancelled')
        ORDER BY
            CASE t.priority
                WHEN 'P0' THEN 0
                WHEN 'P1' THEN 1
            END,
            t.created_at ASC
    """)

    # Critical stubs (in P0/P1 features)
    critical_stubs = await fetch_all("""
        SELECT
            s.id,
            s.stub_type,
            s.line_number,
            func.name as function_name,
            func.signature,
            f.filename,
            f.relative_path,
            feat.name as feature_name,
            feat.priority
        FROM stubs s
        JOIN functions func ON s.function_id = func.id
        JOIN files f ON s.file_id = f.id
        LEFT JOIN feature_functions ff ON func.id = ff.function_id
        LEFT JOIN features feat ON ff.feature_id = feat.id
        WHERE s.resolved_at IS NULL
        AND feat.priority IN ('P0', 'P1')
        ORDER BY
            CASE feat.priority
                WHEN 'P0' THEN 0
                WHEN 'P1' THEN 1
            END,
            feat.name
    """)

    # Blocked tasks
    blocked_tasks = await fetch_all("""
        SELECT
            t.id,
            t.title,
            t.priority,
            t.status,
            t.blocked_by_tasks,
            f.name as feature_name
        FROM tasks t
        LEFT JOIN features f ON t.feature_id = f.id
        WHERE t.status = 'blocked'
        AND t.priority IN ('P0', 'P1', 'P2')
        ORDER BY
            CASE t.priority
                WHEN 'P0' THEN 0
                WHEN 'P1' THEN 1
                WHEN 'P2' THEN 2
            END
    """)

    # Enrich blocked tasks with blocker details
    for task in blocked_tasks:
        if task["blocked_by_tasks"]:
            blocker_ids = json.loads(task["blocked_by_tasks"])
            blockers = await fetch_all(
                f"""
                SELECT id, title, status
                FROM tasks
                WHERE id IN ({','.join('?' * len(blocker_ids))})
                """,
                tuple(blocker_ids)
            )
            task["blockers"] = blockers

    # Features without implementation
    unimplemented_features = await fetch_all("""
        SELECT
            f.id,
            f.name,
            f.priority,
            f.status
        FROM features f
        LEFT JOIN feature_files ff ON f.id = ff.feature_id
        WHERE ff.id IS NULL
        AND f.status = 'in_progress'
        AND f.priority IN ('P0', 'P1')
        ORDER BY
            CASE f.priority
                WHEN 'P0' THEN 0
                WHEN 'P1' THEN 1
            END
    """)

    # Critical recommendations
    critical_recommendations = await fetch_all("""
        SELECT
            id,
            title,
            description,
            severity,
            recommendation_type,
            generated_at
        FROM recommendations
        WHERE status = 'open'
        AND severity = 'critical'
        ORDER BY generated_at DESC
    """)

    return {
        "priority_tasks": priority_tasks,
        "critical_stubs": critical_stubs,
        "blocked_tasks": blocked_tasks,
        "unimplemented_features": unimplemented_features,
        "critical_recommendations": critical_recommendations,
        "summary": {
            "p0_p1_tasks": len(priority_tasks),
            "critical_stubs": len(critical_stubs),
            "blocked_items": len(blocked_tasks),
            "unimplemented_features": len(unimplemented_features),
            "critical_recommendations": len(critical_recommendations)
        }
    }


# Export tools
DASHBOARD_TOOLS = {
    "get_dashboard_overview": get_dashboard_overview,
    "get_component_tree": get_component_tree,
    "get_feature_matrix": get_feature_matrix,
    "get_activity_feed": get_activity_feed,
    "get_priority_items": get_priority_items,
}
