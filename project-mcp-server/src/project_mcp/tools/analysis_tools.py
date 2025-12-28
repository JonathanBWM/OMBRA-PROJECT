"""
Analysis Tools - Codebase scanning and recommendations

Tools for full/incremental scans, health metrics, and recommendation generation.
"""

from ..database import fetch_one, fetch_all, insert, update, count, execute, get_connection
from ..parser import scan_directory, analyze_file
from datetime import datetime
from pathlib import Path
import json
import os


async def full_codebase_scan(
    project_root: str,
    component_mapping: dict = None
) -> dict:
    """
    Perform a full codebase scan.

    Args:
        project_root: Root directory to scan
        component_mapping: Optional dict mapping paths to component IDs

    Returns:
        Comprehensive scan summary
    """
    # Create analysis run record
    run_id = await insert("analysis_runs", {
        "run_type": "full_scan",
        "status": "running",
        "started_at": datetime.now().isoformat()
    })

    stats = {
        "files_scanned": 0,
        "functions_indexed": 0,
        "stubs_detected": 0,
        "recommendations_generated": 0,
        "errors": []
    }

    try:
        # Scan all source files
        analyses = scan_directory(project_root, recursive=True)

        for analysis in analyses:
            try:
                # Check if file already indexed
                existing = await fetch_one(
                    "SELECT id, content_hash FROM files WHERE absolute_path = ?",
                    (analysis.path,)
                )

                # Get file mtime
                mtime = os.path.getmtime(analysis.path)

                # Determine component ownership
                component_id = None
                if component_mapping:
                    for path_prefix, comp_id in component_mapping.items():
                        if analysis.path.startswith(path_prefix):
                            component_id = comp_id
                            break

                # Get relative path
                relative_path = str(Path(analysis.path).relative_to(project_root))

                if existing:
                    # Update if hash changed
                    if existing["content_hash"] != analysis.content_hash:
                        await update(
                            "files",
                            {
                                "line_count": analysis.line_count,
                                "function_count": len(analysis.functions),
                                "struct_count": len(analysis.structs),
                                "stub_line_count": sum(1 for f in analysis.functions if f.is_stub),
                                "todo_count": analysis.todo_count,
                                "fixme_count": analysis.fixme_count,
                                "content_hash": analysis.content_hash,
                                "last_indexed": datetime.now().isoformat(),
                                "mtime": mtime
                            },
                            "id = ?",
                            (existing["id"],)
                        )
                        file_id = existing["id"]

                        # Delete old functions/structs/stubs for re-indexing
                        await execute("DELETE FROM functions WHERE file_id = ?", (file_id,))
                        await execute("DELETE FROM structs WHERE file_id = ?", (file_id,))
                        await execute("DELETE FROM stubs WHERE file_id = ?", (file_id,))
                else:
                    # Insert new file
                    file_id = await insert("files", {
                        "absolute_path": analysis.path,
                        "relative_path": relative_path,
                        "filename": Path(analysis.path).name,
                        "extension": Path(analysis.path).suffix,
                        "component_id": component_id,
                        "language": analysis.language,
                        "line_count": analysis.line_count,
                        "function_count": len(analysis.functions),
                        "struct_count": len(analysis.structs),
                        "stub_line_count": sum(1 for f in analysis.functions if f.is_stub),
                        "todo_count": analysis.todo_count,
                        "fixme_count": analysis.fixme_count,
                        "content_hash": analysis.content_hash,
                        "last_indexed": datetime.now().isoformat(),
                        "mtime": mtime
                    })

                stats["files_scanned"] += 1

                # Index functions
                for func in analysis.functions:
                    await insert("functions", {
                        "file_id": file_id,
                        "name": func.name,
                        "signature": func.signature,
                        "return_type": func.return_type,
                        "line_start": func.line_start,
                        "line_end": func.line_end,
                        "is_stub": 1 if func.is_stub else 0,
                        "stub_reason": func.stub_reason,
                        "has_doc_comment": 1 if func.doc_comment else 0,
                        "doc_comment": func.doc_comment
                    })
                    stats["functions_indexed"] += 1

                # Index structs
                for struct in analysis.structs:
                    await insert("structs", {
                        "file_id": file_id,
                        "name": struct.name,
                        "definition": struct.definition,
                        "member_count": struct.member_count,
                        "line_start": struct.line_start,
                        "line_end": struct.line_end
                    })

                # Index stubs
                for stub in analysis.stubs:
                    await insert("stubs", {
                        "file_id": file_id,
                        "line_number": stub.line_number,
                        "stub_type": stub.stub_type,
                        "stub_text": stub.stub_text,
                        "surrounding_context": stub.context,
                        "detected_at": datetime.now().isoformat()
                    })
                    stats["stubs_detected"] += 1

            except Exception as e:
                stats["errors"].append(f"{analysis.path}: {str(e)}")

        # Generate recommendations
        rec_count = await _generate_scan_recommendations()
        stats["recommendations_generated"] = rec_count

        # Mark run as complete
        await update(
            "analysis_runs",
            {
                "status": "completed",
                "completed_at": datetime.now().isoformat(),
                "files_scanned": stats["files_scanned"],
                "functions_indexed": stats["functions_indexed"],
                "stubs_detected": stats["stubs_detected"],
                "recommendations_generated": stats["recommendations_generated"],
                "errors": json.dumps(stats["errors"])
            },
            "id = ?",
            (run_id,)
        )

        return {
            "run_id": run_id,
            "status": "completed",
            "stats": stats
        }

    except Exception as e:
        # Mark run as failed
        await update(
            "analysis_runs",
            {
                "status": "failed",
                "completed_at": datetime.now().isoformat(),
                "errors": json.dumps([str(e)])
            },
            "id = ?",
            (run_id,)
        )
        raise


async def incremental_scan(since_timestamp: str = None) -> dict:
    """
    Scan only changed files since last scan.

    Args:
        since_timestamp: ISO timestamp to scan from (default: last indexed time)

    Returns:
        Scan summary
    """
    # Create analysis run
    run_id = await insert("analysis_runs", {
        "run_type": "incremental",
        "status": "running",
        "started_at": datetime.now().isoformat()
    })

    stats = {
        "files_scanned": 0,
        "functions_indexed": 0,
        "stubs_detected": 0,
        "errors": []
    }

    try:
        # Get all indexed files
        files = await fetch_all(
            "SELECT id, absolute_path, mtime, content_hash FROM files"
        )

        for file_record in files:
            file_path = file_record["absolute_path"]

            # Check if file exists
            if not os.path.exists(file_path):
                # File deleted - mark in audit log
                await insert("audit_log", {
                    "entity_type": "file",
                    "entity_id": file_record["id"],
                    "action": "delete",
                    "performed_by": "system",
                    "notes": "File no longer exists"
                })
                continue

            # Check if modified
            current_mtime = os.path.getmtime(file_path)
            if file_record["mtime"] and current_mtime <= file_record["mtime"]:
                continue  # Not modified

            # Re-analyze file
            analysis = analyze_file(file_path)
            if not analysis:
                continue

            # Check if content actually changed
            if analysis.content_hash == file_record["content_hash"]:
                # Just update mtime
                await update("files", {"mtime": current_mtime}, "id = ?", (file_record["id"],))
                continue

            # Update file record
            await update(
                "files",
                {
                    "line_count": analysis.line_count,
                    "function_count": len(analysis.functions),
                    "struct_count": len(analysis.structs),
                    "stub_line_count": sum(1 for f in analysis.functions if f.is_stub),
                    "todo_count": analysis.todo_count,
                    "fixme_count": analysis.fixme_count,
                    "content_hash": analysis.content_hash,
                    "last_indexed": datetime.now().isoformat(),
                    "mtime": current_mtime
                },
                "id = ?",
                (file_record["id"],)
            )

            # Delete and re-index functions/structs/stubs
            await execute("DELETE FROM functions WHERE file_id = ?", (file_record["id"],))
            await execute("DELETE FROM structs WHERE file_id = ?", (file_record["id"],))
            await execute("DELETE FROM stubs WHERE file_id = ?", (file_record["id"],))

            # Index functions
            for func in analysis.functions:
                await insert("functions", {
                    "file_id": file_record["id"],
                    "name": func.name,
                    "signature": func.signature,
                    "return_type": func.return_type,
                    "line_start": func.line_start,
                    "line_end": func.line_end,
                    "is_stub": 1 if func.is_stub else 0,
                    "stub_reason": func.stub_reason,
                    "has_doc_comment": 1 if func.doc_comment else 0,
                    "doc_comment": func.doc_comment
                })
                stats["functions_indexed"] += 1

            # Index structs
            for struct in analysis.structs:
                await insert("structs", {
                    "file_id": file_record["id"],
                    "name": struct.name,
                    "definition": struct.definition,
                    "member_count": struct.member_count,
                    "line_start": struct.line_start,
                    "line_end": struct.line_end
                })

            # Index stubs
            for stub in analysis.stubs:
                await insert("stubs", {
                    "file_id": file_record["id"],
                    "line_number": stub.line_number,
                    "stub_type": stub.stub_type,
                    "stub_text": stub.stub_text,
                    "surrounding_context": stub.context,
                    "detected_at": datetime.now().isoformat()
                })
                stats["stubs_detected"] += 1

            stats["files_scanned"] += 1

        # Complete run
        await update(
            "analysis_runs",
            {
                "status": "completed",
                "completed_at": datetime.now().isoformat(),
                "files_scanned": stats["files_scanned"],
                "functions_indexed": stats["functions_indexed"],
                "stubs_detected": stats["stubs_detected"]
            },
            "id = ?",
            (run_id,)
        )

        return {
            "run_id": run_id,
            "status": "completed",
            "stats": stats
        }

    except Exception as e:
        await update(
            "analysis_runs",
            {"status": "failed", "errors": json.dumps([str(e)])},
            "id = ?",
            (run_id,)
        )
        raise


async def get_codebase_health() -> dict:
    """
    Get overall codebase health metrics.

    Returns:
        Health dashboard data
    """
    # Total counts
    total_files = await count("files")
    total_functions = await count("functions")
    total_stubs = await count("stubs", "resolved_at IS NULL")
    stub_functions = await count("functions", "is_stub = 1")

    # LOC
    loc_result = await fetch_one("SELECT SUM(line_count) as total_loc FROM files")
    total_loc = loc_result["total_loc"] or 0

    # Feature implementation
    total_features = await count("features")
    implemented_features = await count("features", "status IN ('implemented', 'testing', 'verified')")
    in_progress_features = await count("features", "status = 'in_progress'")
    not_started_features = await count("features", "status = 'not_started'")

    # Stub percentage
    if total_functions > 0:
        stub_percentage = (stub_functions / total_functions) * 100
    else:
        stub_percentage = 0

    # TODO/FIXME counts
    todo_result = await fetch_one("SELECT SUM(todo_count) as total FROM files")
    fixme_result = await fetch_one("SELECT SUM(fixme_count) as total FROM files")
    total_todos = todo_result["total"] or 0
    total_fixmes = fixme_result["total"] or 0

    # Open recommendations
    open_recommendations = await count("recommendations", "status = 'open'")
    critical_recommendations = await count(
        "recommendations",
        "status = 'open' AND severity = 'critical'"
    )

    # Feature implementation percentages
    feature_breakdown = {
        "implemented": implemented_features,
        "in_progress": in_progress_features,
        "not_started": not_started_features
    }

    if total_features > 0:
        feature_breakdown["implemented_pct"] = (implemented_features / total_features) * 100
        feature_breakdown["in_progress_pct"] = (in_progress_features / total_features) * 100
        feature_breakdown["not_started_pct"] = (not_started_features / total_features) * 100
    else:
        feature_breakdown["implemented_pct"] = 0
        feature_breakdown["in_progress_pct"] = 0
        feature_breakdown["not_started_pct"] = 0

    return {
        "overview": {
            "total_files": total_files,
            "total_functions": total_functions,
            "total_loc": total_loc,
            "stub_functions": stub_functions,
            "stub_percentage": round(stub_percentage, 2)
        },
        "features": {
            "total": total_features,
            "breakdown": feature_breakdown
        },
        "quality": {
            "open_stubs": total_stubs,
            "todo_count": total_todos,
            "fixme_count": total_fixmes
        },
        "recommendations": {
            "total_open": open_recommendations,
            "critical": critical_recommendations
        }
    }


async def generate_recommendations(
    scope: str = "all",
    scope_id: int = None
) -> list[dict]:
    """
    Generate recommendations for code improvements.

    Args:
        scope: "all", "component", "feature", "file"
        scope_id: ID of the scoped entity

    Returns:
        List of generated recommendations
    """
    recommendations = []

    # Find critical stubbed features
    if scope in ("all", "feature"):
        where_clause = "f.is_stub = 1"
        params = ()

        if scope == "feature" and scope_id:
            where_clause += " AND ff.feature_id = ?"
            params = (scope_id,)

        critical_stubs = await fetch_all(
            f"""
            SELECT
                f.id as function_id,
                f.name as function_name,
                f.stub_reason,
                fi.absolute_path,
                feat.id as feature_id,
                feat.name as feature_name,
                feat.priority
            FROM functions f
            JOIN files fi ON f.file_id = fi.id
            LEFT JOIN feature_functions ff ON f.id = ff.function_id
            LEFT JOIN features feat ON ff.feature_id = feat.id
            WHERE {where_clause} AND feat.priority IN ('P0', 'P1')
            ORDER BY feat.priority
            """,
            params
        )

        for stub in critical_stubs:
            rec_id = await insert("recommendations", {
                "recommendation_type": "stub_resolution",
                "severity": "critical" if stub["priority"] == "P0" else "warning",
                "title": f"Implement critical stub: {stub['function_name']}",
                "description": f"Function {stub['function_name']} is stubbed ({stub['stub_reason']}) but belongs to priority {stub['priority']} feature '{stub['feature_name']}'",
                "rationale": "Critical feature blocked by stub implementation",
                "affected_feature_id": stub["feature_id"],
                "affected_function_ids": json.dumps([stub["function_id"]]),
                "suggested_action": f"Implement full logic for {stub['function_name']}",
                "estimated_effort": "medium",
                "generated_by": "codebase_scan",
                "status": "open"
            })

            rec = await fetch_one("SELECT * FROM recommendations WHERE id = ?", (rec_id,))
            recommendations.append(rec)

    # Find orphan files (not linked to features)
    if scope in ("all", "file"):
        orphan_query = """
            SELECT
                f.id,
                f.absolute_path,
                f.filename,
                f.component_id,
                c.name as component_name
            FROM files f
            LEFT JOIN feature_files ff ON f.id = ff.file_id
            LEFT JOIN components c ON f.component_id = c.id
            WHERE ff.id IS NULL
            AND f.extension IN ('.c', '.cpp', '.h', '.hpp')
            LIMIT 50
        """

        orphans = await fetch_all(orphan_query)

        if orphans:
            rec_id = await insert("recommendations", {
                "recommendation_type": "missing_feature",
                "severity": "suggestion",
                "title": f"Link {len(orphans)} orphan files to features",
                "description": f"Found {len(orphans)} source files not linked to any features",
                "rationale": "Better tracking requires file-to-feature mapping",
                "affected_file_ids": json.dumps([o["id"] for o in orphans]),
                "suggested_action": "Review and link files to appropriate features",
                "estimated_effort": "low",
                "generated_by": "codebase_scan",
                "status": "open"
            })

            rec = await fetch_one("SELECT * FROM recommendations WHERE id = ?", (rec_id,))
            recommendations.append(rec)

    # Find features with no implementing files
    if scope in ("all", "feature"):
        empty_features = await fetch_all("""
            SELECT
                f.id,
                f.name,
                f.priority,
                f.status
            FROM features f
            LEFT JOIN feature_files ff ON f.id = ff.feature_id
            WHERE ff.id IS NULL
            AND f.status NOT IN ('deprecated', 'not_started')
        """)

        for feat in empty_features:
            rec_id = await insert("recommendations", {
                "recommendation_type": "missing_feature",
                "severity": "warning",
                "title": f"No implementation files for feature: {feat['name']}",
                "description": f"Feature '{feat['name']}' (status: {feat['status']}) has no linked implementation files",
                "rationale": "Features should have implementing code or be marked not_started",
                "affected_feature_id": feat["id"],
                "suggested_action": "Link implementation files or update feature status",
                "estimated_effort": "low",
                "generated_by": "codebase_scan",
                "status": "open"
            })

            rec = await fetch_one("SELECT * FROM recommendations WHERE id = ?", (rec_id,))
            recommendations.append(rec)

    return recommendations


async def list_recommendations(
    status: str = "open",
    severity: str = None,
    recommendation_type: str = None
) -> list[dict]:
    """
    List recommendations with filters.

    Args:
        status: Filter by status
        severity: Filter by severity
        recommendation_type: Filter by type

    Returns:
        List of recommendations
    """
    where_clauses = []
    params = []

    if status:
        where_clauses.append("status = ?")
        params.append(status)

    if severity:
        where_clauses.append("severity = ?")
        params.append(severity)

    if recommendation_type:
        where_clauses.append("recommendation_type = ?")
        params.append(recommendation_type)

    where_sql = " AND ".join(where_clauses) if where_clauses else "1=1"

    return await fetch_all(
        f"""
        SELECT
            r.*,
            f.name as affected_feature_name
        FROM recommendations r
        LEFT JOIN features f ON r.affected_feature_id = f.id
        WHERE {where_sql}
        ORDER BY
            CASE severity
                WHEN 'critical' THEN 0
                WHEN 'warning' THEN 1
                WHEN 'suggestion' THEN 2
                ELSE 3
            END,
            generated_at DESC
        """,
        tuple(params)
    )


async def accept_recommendation(
    recommendation_id: int,
    create_task: bool = True
) -> dict:
    """
    Accept a recommendation and optionally create a task.

    Args:
        recommendation_id: ID of recommendation to accept
        create_task: Whether to create a task for this

    Returns:
        Updated recommendation and optional task
    """
    # Get recommendation
    rec = await fetch_one(
        "SELECT * FROM recommendations WHERE id = ?",
        (recommendation_id,)
    )

    if not rec:
        raise ValueError(f"Recommendation {recommendation_id} not found")

    # Update status
    await update(
        "recommendations",
        {
            "status": "accepted",
            "resolved_at": datetime.now().isoformat(),
            "resolution_notes": "Accepted for implementation"
        },
        "id = ?",
        (recommendation_id,)
    )

    task_id = None
    if create_task:
        # Create task from recommendation
        task_id = await insert("tasks", {
            "title": rec["title"],
            "description": rec["description"],
            "task_type": "improvement",
            "priority": "P1" if rec["severity"] == "critical" else "P2",
            "status": "todo",
            "feature_id": rec["affected_feature_id"],
            "component_id": rec["affected_component_id"],
            "affected_files": rec["affected_file_ids"],
            "estimated_hours": 2.0 if rec["estimated_effort"] == "low" else 8.0
        })

    return {
        "recommendation_id": recommendation_id,
        "status": "accepted",
        "task_id": task_id,
        "task_created": create_task
    }


async def _generate_scan_recommendations() -> int:
    """Internal helper to generate recommendations after scan."""
    recs = await generate_recommendations(scope="all")
    return len(recs)


# Export tools
ANALYSIS_TOOLS = {
    "full_codebase_scan": full_codebase_scan,
    "incremental_scan": incremental_scan,
    "get_codebase_health": get_codebase_health,
    "generate_recommendations": generate_recommendations,
    "list_recommendations": list_recommendations,
    "accept_recommendation": accept_recommendation,
}
