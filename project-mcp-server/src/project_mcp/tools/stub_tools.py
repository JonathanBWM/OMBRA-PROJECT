"""
Stub Detection and Tracking Tools

Provides MCP tools for:
- Detecting stubs in source files
- Tracking stub resolution progress
- Calculating stub percentages
- Generating stub reports
"""

import json
from datetime import datetime
from pathlib import Path
from typing import Optional

from ..database import fetch_one, fetch_all, insert, update, delete, count
from ..parser import analyze_file, scan_directory


async def detect_stubs(
    file_path: Optional[str] = None,
    component_id: Optional[int] = None,
    directory: Optional[str] = None,
) -> dict:
    """
    Scan for stubs in source files.

    Modes:
    - file_path: Analyze single file
    - component_id: Analyze all files for component
    - directory: Scan all files in directory

    Returns:
        {
            "stubs_detected": int,
            "files_scanned": int,
            "results": [{"file": str, "stubs": int, "details": [...]}]
        }
    """
    results = []
    total_stubs = 0
    files_scanned = 0

    # Mode 1: Single file
    if file_path:
        analysis = analyze_file(file_path)
        if analysis:
            # Get or create file record
            file_record = await fetch_one(
                "SELECT id FROM files WHERE absolute_path = ?",
                (analysis.path,)
            )

            if not file_record:
                # Create file record
                file_id = await insert("files", {
                    "absolute_path": analysis.path,
                    "filename": Path(analysis.path).name,
                    "extension": Path(analysis.path).suffix,
                    "language": analysis.language,
                    "line_count": analysis.line_count,
                    "content_hash": analysis.content_hash,
                })
            else:
                file_id = file_record["id"]
                # Update file record
                await update(
                    "files",
                    {
                        "line_count": analysis.line_count,
                        "content_hash": analysis.content_hash,
                        "last_indexed": datetime.now().isoformat(),
                    },
                    "id = ?",
                    (file_id,)
                )

            # Store stubs
            stub_details = []
            for stub in analysis.stubs:
                stub_id = await insert("stubs", {
                    "file_id": file_id,
                    "line_number": stub.line_number,
                    "stub_type": stub.stub_type,
                    "stub_text": stub.stub_text,
                    "surrounding_context": stub.context,
                    "severity": _classify_severity(stub.stub_type),
                })

                stub_details.append({
                    "id": stub_id,
                    "line": stub.line_number,
                    "type": stub.stub_type,
                    "text": stub.stub_text[:100],
                })
                total_stubs += 1

            files_scanned = 1
            results.append({
                "file": analysis.path,
                "stubs": len(stub_details),
                "details": stub_details,
            })

    # Mode 2: Component files
    elif component_id:
        files = await fetch_all(
            "SELECT id, absolute_path FROM files WHERE component_id = ?",
            (component_id,)
        )

        for file_rec in files:
            analysis = analyze_file(file_rec["absolute_path"])
            if not analysis:
                continue

            stub_details = []
            for stub in analysis.stubs:
                stub_id = await insert("stubs", {
                    "file_id": file_rec["id"],
                    "line_number": stub.line_number,
                    "stub_type": stub.stub_type,
                    "stub_text": stub.stub_text,
                    "surrounding_context": stub.context,
                    "severity": _classify_severity(stub.stub_type),
                })

                stub_details.append({
                    "id": stub_id,
                    "line": stub.line_number,
                    "type": stub.stub_type,
                    "text": stub.stub_text[:100],
                })
                total_stubs += 1

            files_scanned += 1
            results.append({
                "file": file_rec["absolute_path"],
                "stubs": len(stub_details),
                "details": stub_details,
            })

    # Mode 3: Directory scan
    elif directory:
        analyses = scan_directory(directory, recursive=True)

        for analysis in analyses:
            # Get or create file record
            file_record = await fetch_one(
                "SELECT id FROM files WHERE absolute_path = ?",
                (analysis.path,)
            )

            if not file_record:
                file_id = await insert("files", {
                    "absolute_path": analysis.path,
                    "filename": Path(analysis.path).name,
                    "extension": Path(analysis.path).suffix,
                    "language": analysis.language,
                    "line_count": analysis.line_count,
                    "content_hash": analysis.content_hash,
                })
            else:
                file_id = file_record["id"]

            stub_details = []
            for stub in analysis.stubs:
                stub_id = await insert("stubs", {
                    "file_id": file_id,
                    "line_number": stub.line_number,
                    "stub_type": stub.stub_type,
                    "stub_text": stub.stub_text,
                    "surrounding_context": stub.context,
                    "severity": _classify_severity(stub.stub_type),
                })

                stub_details.append({
                    "id": stub_id,
                    "line": stub.line_number,
                    "type": stub.stub_type,
                    "text": stub.stub_text[:100],
                })
                total_stubs += 1

            files_scanned += 1
            results.append({
                "file": analysis.path,
                "stubs": len(stub_details),
                "details": stub_details,
            })

    return {
        "stubs_detected": total_stubs,
        "files_scanned": files_scanned,
        "results": results,
    }


async def list_stubs(
    unresolved_only: bool = True,
    severity: Optional[str] = None,
    feature_id: Optional[int] = None,
    component_id: Optional[int] = None,
) -> dict:
    """
    List detected stubs with filtering.

    Args:
        unresolved_only: Only show unresolved stubs
        severity: Filter by severity (critical, high, medium, low)
        feature_id: Filter by feature
        component_id: Filter by component

    Returns:
        {
            "total": int,
            "stubs": [
                {
                    "id": int,
                    "file_path": str,
                    "line_number": int,
                    "stub_type": str,
                    "severity": str,
                    "stub_text": str,
                    "function_name": str,
                    "resolved_at": str or None
                }
            ]
        }
    """
    # Build query
    query = """
        SELECT
            s.id, s.line_number, s.stub_type, s.severity,
            s.stub_text, s.resolved_at,
            f.absolute_path AS file_path,
            fn.name AS function_name
        FROM stubs s
        JOIN files f ON s.file_id = f.id
        LEFT JOIN functions fn ON s.function_id = fn.id
        WHERE 1=1
    """
    params = []

    if unresolved_only:
        query += " AND s.resolved_at IS NULL"

    if severity:
        query += " AND s.severity = ?"
        params.append(severity)

    if feature_id:
        query += " AND s.feature_id = ?"
        params.append(feature_id)

    if component_id:
        query += " AND f.component_id = ?"
        params.append(component_id)

    query += " ORDER BY s.severity DESC, f.absolute_path, s.line_number"

    stubs = await fetch_all(query, tuple(params))

    return {
        "total": len(stubs),
        "stubs": stubs,
    }


async def resolve_stub(stub_id: int, resolution_notes: str) -> dict:
    """
    Mark a stub as resolved.

    Args:
        stub_id: Stub ID
        resolution_notes: Notes on how it was resolved

    Returns:
        {"success": bool, "message": str}
    """
    stub = await fetch_one("SELECT id FROM stubs WHERE id = ?", (stub_id,))

    if not stub:
        return {
            "success": False,
            "message": f"Stub {stub_id} not found"
        }

    await update(
        "stubs",
        {
            "resolved_at": datetime.now().isoformat(),
            "resolved_by": "agent",
        },
        "id = ?",
        (stub_id,)
    )

    # Add audit log
    await insert("audit_log", {
        "entity_type": "stub",
        "entity_id": stub_id,
        "action": "resolve",
        "new_value": json.dumps({"resolution_notes": resolution_notes}),
        "performed_by": "agent",
    })

    return {
        "success": True,
        "message": f"Stub {stub_id} marked as resolved",
    }


async def get_stub_percentage(
    feature_id: Optional[int] = None,
    component_id: Optional[int] = None,
    file_path: Optional[str] = None,
) -> dict:
    """
    Calculate stub percentage.

    Args:
        feature_id: Calculate for specific feature
        component_id: Calculate for specific component
        file_path: Calculate for specific file

    Returns:
        {
            "total_functions": int,
            "stub_functions": int,
            "percentage": float,
            "breakdown": {...}
        }
    """
    # Build query based on scope
    if file_path:
        query = """
            SELECT
                COUNT(*) as total,
                SUM(CASE WHEN is_stub = 1 THEN 1 ELSE 0 END) as stubs
            FROM functions fn
            JOIN files f ON fn.file_id = f.id
            WHERE f.absolute_path = ?
        """
        params = (file_path,)
    elif component_id:
        query = """
            SELECT
                COUNT(*) as total,
                SUM(CASE WHEN fn.is_stub = 1 THEN 1 ELSE 0 END) as stubs
            FROM functions fn
            JOIN files f ON fn.file_id = f.id
            WHERE f.component_id = ?
        """
        params = (component_id,)
    elif feature_id:
        query = """
            SELECT
                COUNT(*) as total,
                SUM(CASE WHEN fn.is_stub = 1 THEN 1 ELSE 0 END) as stubs
            FROM functions fn
            JOIN feature_functions ff ON fn.id = ff.function_id
            WHERE ff.feature_id = ?
        """
        params = (feature_id,)
    else:
        # Global stats
        query = """
            SELECT
                COUNT(*) as total,
                SUM(CASE WHEN is_stub = 1 THEN 1 ELSE 0 END) as stubs
            FROM functions
        """
        params = ()

    result = await fetch_one(query, params)

    total = result["total"] if result else 0
    stubs = result["stubs"] if result and result["stubs"] else 0
    percentage = (stubs / total * 100) if total > 0 else 0.0

    # Get breakdown by stub type
    breakdown_query = """
        SELECT
            stub_reason,
            COUNT(*) as count
        FROM functions
        WHERE is_stub = 1
    """
    if component_id:
        breakdown_query += " AND file_id IN (SELECT id FROM files WHERE component_id = ?)"
    elif feature_id:
        breakdown_query += " AND id IN (SELECT function_id FROM feature_functions WHERE feature_id = ?)"

    breakdown_query += " GROUP BY stub_reason"

    if component_id or feature_id:
        breakdown = await fetch_all(breakdown_query, params)
    else:
        breakdown = await fetch_all(breakdown_query)

    breakdown_dict = {row["stub_reason"]: row["count"] for row in breakdown}

    return {
        "total_functions": total,
        "stub_functions": stubs,
        "percentage": round(percentage, 2),
        "breakdown": breakdown_dict,
    }


async def get_stub_summary() -> dict:
    """
    Overall stub statistics.

    Returns:
        {
            "total_stubs": int,
            "unresolved_stubs": int,
            "by_severity": {...},
            "by_type": {...},
            "by_component": [...]
        }
    """
    # Total stats
    total = await count("stubs")
    unresolved = await count("stubs", "resolved_at IS NULL")

    # By severity
    severity_rows = await fetch_all("""
        SELECT severity, COUNT(*) as count
        FROM stubs
        WHERE resolved_at IS NULL
        GROUP BY severity
    """)
    by_severity = {row["severity"]: row["count"] for row in severity_rows}

    # By type
    type_rows = await fetch_all("""
        SELECT stub_type, COUNT(*) as count
        FROM stubs
        WHERE resolved_at IS NULL
        GROUP BY stub_type
        ORDER BY count DESC
    """)
    by_type = {row["stub_type"]: row["count"] for row in type_rows}

    # By component
    component_rows = await fetch_all("""
        SELECT
            c.name AS component,
            COUNT(DISTINCT s.id) as stub_count,
            COUNT(DISTINCT f.id) as file_count
        FROM stubs s
        JOIN files f ON s.file_id = f.id
        LEFT JOIN components c ON f.component_id = c.id
        WHERE s.resolved_at IS NULL
        GROUP BY c.id, c.name
        ORDER BY stub_count DESC
    """)

    by_component = [
        {
            "component": row["component"] or "unassigned",
            "stub_count": row["stub_count"],
            "file_count": row["file_count"],
        }
        for row in component_rows
    ]

    return {
        "total_stubs": total,
        "unresolved_stubs": unresolved,
        "by_severity": by_severity,
        "by_type": by_type,
        "by_component": by_component,
    }


def _classify_severity(stub_type: str) -> str:
    """Classify stub severity based on type."""
    critical = ["not_implemented"]
    high = ["todo", "fixme", "empty_body"]
    medium = ["return_zero", "return_null", "placeholder"]

    if stub_type in critical:
        return "critical"
    elif stub_type in high:
        return "high"
    elif stub_type in medium:
        return "medium"
    else:
        return "low"


# Export tools dictionary
STUB_TOOLS = {
    "detect_stubs": detect_stubs,
    "list_stubs": list_stubs,
    "resolve_stub": resolve_stub,
    "get_stub_percentage": get_stub_percentage,
    "get_stub_summary": get_stub_summary,
}
