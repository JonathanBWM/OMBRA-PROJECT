"""
Project Brain MCP Tools - Query and manage project intelligence

Tools for:
- Project health and status
- Findings management
- Decision recording
- Gotcha tracking
- Session context
"""

from typing import Optional, List, Dict, Any
from pathlib import Path
import sys

# Add watcherd to path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from ombra_watcherd.database import ProjectBrainDB


# Singleton database instance
_db: Optional[ProjectBrainDB] = None


def get_db() -> ProjectBrainDB:
    """Get or create database instance."""
    global _db
    if _db is None:
        _db = ProjectBrainDB()
    return _db


# =============================================================================
# PROJECT STATUS
# =============================================================================

async def get_project_status(verbose: bool = False) -> Dict[str, Any]:
    """
    Get overall project health dashboard.

    Returns component status, active findings, and blockers.
    """
    db = get_db()
    health = db.get_project_health()

    # Get breakdown by finding type
    findings_by_type = _get_findings_breakdown(db)

    # Identify blockers (critical findings)
    blockers = []
    if health["critical_count"] > 0:
        critical = db.get_critical_findings()
        # Group by check_id to show unique blocker types
        blocker_groups = {}
        for finding in critical:
            check_id = finding["check_id"]
            if check_id not in blocker_groups:
                blocker_groups[check_id] = {
                    "check": check_id,
                    "message": finding["message"],
                    "count": 0,
                    "files": []
                }
            blocker_groups[check_id]["count"] += 1
            file_ref = f"{finding['file']}:{finding['line']}" if finding['line'] else finding['file']
            if file_ref not in blocker_groups[check_id]["files"]:
                blocker_groups[check_id]["files"].append(file_ref)
        blockers = list(blocker_groups.values())

    result = {
        "health_score": _calculate_health_score(health),
        "health_message": _get_health_message(health, findings_by_type),
        "findings": health["findings"],
        "findings_by_type": findings_by_type,
        "total_findings": health["total_findings"],
        "critical_count": health["critical_count"],
        "blockers": blockers,
        "components": health["components"],
        "exit_handlers": health["exit_handlers"],
    }

    if verbose:
        result["critical_findings"] = db.get_critical_findings()
        result["top_suggestions"] = await get_suggestions(limit=5)
        result["components_list"] = db.get_all_components()

    return result


def _calculate_health_score(health: Dict) -> str:
    """Calculate health score based on severity and count of findings."""
    critical = health.get("critical_count", 0)
    warnings = health.get("findings", {}).get("warning", 0)

    if critical > 15:
        return "CRITICAL - MULTIPLE BLOCKERS"
    elif critical > 0:
        return "BLOCKED"
    elif warnings > 500:
        return "POOR"
    elif warnings > 100:
        return "NEEDS ATTENTION"
    elif warnings > 10:
        return "GOOD"
    else:
        return "EXCELLENT"


def _get_health_message(health: Dict, breakdown: Dict) -> str:
    """Generate actionable health message."""
    critical = health.get("critical_count", 0)
    warnings = health.get("findings", {}).get("warning", 0)

    if critical > 0:
        return f"{critical} signature exposures in kernel code blocking deployment - MUST FIX BEFORE TESTING"
    elif warnings > 500:
        return f"{warnings} warnings ({breakdown.get('security', {}).get('warning', 0)} security, {breakdown.get('consistency', {}).get('warning', 0)} consistency) - review needed"
    elif warnings > 100:
        return f"{warnings} warnings to review - most are likely false positives in info files"
    elif warnings > 0:
        return f"{warnings} warnings - low priority cleanup items"
    else:
        return "No active findings - codebase is clean"


def _get_findings_breakdown(db: ProjectBrainDB) -> Dict[str, Dict[str, int]]:
    """Get findings breakdown by type and severity."""
    import sqlite3
    from contextlib import contextmanager

    @contextmanager
    def conn():
        connection = sqlite3.connect(db.db_path)
        connection.row_factory = sqlite3.Row
        try:
            yield connection
        finally:
            connection.close()

    with conn() as connection:
        rows = connection.execute("""
            SELECT type, severity, COUNT(*) as count
            FROM findings WHERE NOT dismissed
            GROUP BY type, severity
        """).fetchall()

    breakdown = {}
    for row in rows:
        type_ = row['type']
        severity = row['severity']
        count = row['count']
        if type_ not in breakdown:
            breakdown[type_] = {}
        breakdown[type_][severity] = count

    return breakdown


# =============================================================================
# FINDINGS
# =============================================================================

async def get_findings(
    severity: Optional[str] = None,
    file: Optional[str] = None,
    type_: Optional[str] = None,
    limit: int = 50
) -> List[Dict[str, Any]]:
    """
    Query active findings.

    Args:
        severity: Filter by severity (critical, warning, info)
        file: Filter by file path (partial match)
        type_: Filter by analyzer type (hypervisor, consistency, security)
        limit: Maximum findings to return
    """
    db = get_db()
    return db.get_findings(
        severity=severity,
        file=file,
        type_=type_,
        limit=limit
    )


async def dismiss_finding(finding_id: int, reason: Optional[str] = None) -> Dict[str, Any]:
    """
    Dismiss a finding as false positive.

    Args:
        finding_id: ID of the finding to dismiss
        reason: Optional reason for dismissal
    """
    db = get_db()
    success = db.dismiss_finding(finding_id, reason)
    return {
        "success": success,
        "finding_id": finding_id,
        "message": "Finding dismissed" if success else "Finding not found"
    }


# =============================================================================
# SUGGESTIONS
# =============================================================================

async def get_suggestions(
    priority: Optional[str] = None,
    limit: int = 10
) -> List[Dict[str, Any]]:
    """
    Get actionable suggestions for what to work on next.

    Intelligently analyzes findings and generates prioritized,
    actionable suggestions with file locations and specific fixes.

    Args:
        priority: Filter by priority (critical, high, medium, low)
        limit: Maximum suggestions to return
    """
    db = get_db()
    suggestions = []

    # Generate suggestions from findings
    import sqlite3
    from contextlib import contextmanager

    @contextmanager
    def conn():
        connection = sqlite3.connect(db.db_path)
        connection.row_factory = sqlite3.Row
        try:
            yield connection
        finally:
            connection.close()

    with conn() as connection:
        # 1. CRITICAL: Signature exposures (grouped by pattern)
        critical_sigs = connection.execute("""
            SELECT check_id, message, suggested_fix,
                   COUNT(*) as count,
                   GROUP_CONCAT(file || ':' || COALESCE(line, '?')) as locations
            FROM findings
            WHERE dismissed = 0 AND severity = 'critical' AND check_id = 'signature_exposure'
            GROUP BY check_id, message
            ORDER BY count DESC
        """).fetchall()

        for sig in critical_sigs:
            locations = sig['locations'].split(',')[:5]  # First 5 locations
            remaining = sig['count'] - len(locations)
            loc_str = '\n  - '.join(locations)
            if remaining > 0:
                loc_str += f"\n  - ...and {remaining} more"

            suggestions.append({
                "priority": "CRITICAL",
                "type": "security",
                "title": "Remove runtime signatures from kernel code",
                "message": f"{sig['message']} (found in {sig['count']} locations)",
                "action": f"{sig['suggested_fix']}\n\nLocations:\n  - {loc_str}",
                "urgency": "BLOCKING - Cannot deploy until fixed",
                "estimated_effort": f"{sig['count']} string removals/obfuscations"
            })

        # 2. HIGH: Consistency issues in core hypervisor files
        consistency_issues = connection.execute("""
            SELECT check_id, message, file,
                   COUNT(*) as count,
                   GROUP_CONCAT(DISTINCT line) as lines
            FROM findings
            WHERE dismissed = 0
              AND severity = 'warning'
              AND type = 'consistency'
              AND file LIKE '%/hypervisor/hypervisor/%'
              AND file NOT LIKE '%/.worktrees/%'
            GROUP BY check_id, message, file
            ORDER BY count DESC
            LIMIT 5
        """).fetchall()

        for issue in consistency_issues:
            suggestions.append({
                "priority": "HIGH",
                "type": "consistency",
                "title": f"Fix consistency issue in {issue['file'].split('/')[-1]}",
                "message": issue['message'],
                "action": f"Review {issue['file']} at lines: {issue['lines']}",
                "file": issue['file'],
                "urgency": "Important for code quality",
                "estimated_effort": "5-10 minutes per file"
            })

        # 3. MEDIUM: Undefined references in hypervisor code
        undefined_refs = connection.execute("""
            SELECT message, file, line, suggested_fix
            FROM findings
            WHERE dismissed = 0
              AND severity = 'warning'
              AND check_id LIKE '%undefined%'
              AND file LIKE '%/hypervisor/%'
              AND file NOT LIKE '%/.worktrees/%'
            ORDER BY file
            LIMIT 10
        """).fetchall()

        if undefined_refs:
            # Group by file
            by_file = {}
            for ref in undefined_refs:
                f = ref['file']
                if f not in by_file:
                    by_file[f] = []
                by_file[f].append(f"Line {ref['line']}: {ref['message']}")

            for file, issues in list(by_file.items())[:3]:
                suggestions.append({
                    "priority": "MEDIUM",
                    "type": "consistency",
                    "title": f"Resolve undefined references in {file.split('/')[-1]}",
                    "message": f"{len(issues)} undefined reference(s)",
                    "action": f"Check {file}:\n  - " + '\n  - '.join(issues[:5]),
                    "file": file,
                    "urgency": "May indicate incomplete implementation",
                    "estimated_effort": "10-30 minutes"
                })

        # 4. LOW: Info-level findings (usually false positives in docs/tests)
        info_count = connection.execute("""
            SELECT COUNT(*) as count FROM findings
            WHERE dismissed = 0 AND severity = 'info'
        """).fetchone()['count']

        if info_count > 1000:
            suggestions.append({
                "priority": "LOW",
                "type": "cleanup",
                "title": "Review and dismiss false positive findings",
                "message": f"{info_count} info-level findings (likely in docs/tests)",
                "action": "Run: SELECT file, COUNT(*) FROM findings WHERE severity='info' GROUP BY file ORDER BY COUNT(*) DESC to see distribution, then dismiss irrelevant ones",
                "urgency": "Low priority cleanup",
                "estimated_effort": "30-60 minutes to filter"
            })

    # Filter by priority if requested
    if priority:
        suggestions = [s for s in suggestions if s['priority'].lower() == priority.lower()]

    # Sort by priority
    priority_order = {"CRITICAL": 0, "HIGH": 1, "MEDIUM": 2, "LOW": 3}
    suggestions.sort(key=lambda s: priority_order.get(s['priority'], 999))

    return suggestions[:limit]


async def get_priority_work() -> Dict[str, Any]:
    """
    Get the single most important thing to work on right now.

    Returns a focused, actionable task with specific file locations
    and clear success criteria.
    """
    db = get_db()
    health = db.get_project_health()

    # Check for critical blockers first
    if health["critical_count"] > 0:
        critical = db.get_critical_findings()

        # Group signature exposures by file
        by_file = {}
        for finding in critical:
            if finding["check_id"] == "signature_exposure":
                file = finding["file"]
                # Skip worktree duplicates
                if "/.worktrees/" in file:
                    continue
                if file not in by_file:
                    by_file[file] = {
                        "file": file,
                        "lines": [],
                        "pattern": None
                    }
                by_file[file]["lines"].append(finding["line"])
                # Extract pattern from message
                if "'" in finding["message"]:
                    pattern = finding["message"].split("'")[1]
                    by_file[file]["pattern"] = pattern

        if by_file:
            # Pick the file with most exposures
            worst_file = max(by_file.values(), key=lambda x: len(x["lines"]))

            return {
                "priority": "CRITICAL",
                "task": f"Remove '{worst_file['pattern']}' signatures from {worst_file['file'].split('/')[-1]}",
                "file": worst_file['file'],
                "lines": sorted(worst_file['lines']),
                "action": f"Replace all instances of '{worst_file['pattern']}' with obfuscated alternatives or remove entirely",
                "success_criteria": [
                    f"No literal '{worst_file['pattern']}' strings in {worst_file['file'].split('/')[-1]}",
                    "Signature scan shows 0 critical findings",
                    "Code still compiles and runs"
                ],
                "why_this_matters": "Anti-cheat memory scanners will flag literal 'hypervisor' strings in kernel memory. This is a deployment blocker.",
                "estimated_time": f"{len(worst_file['lines'])} * 2 minutes = {len(worst_file['lines']) * 2} minutes total",
                "next_after_this": f"Repeat for remaining {len(by_file) - 1} files with signature exposures"
            }

    # No critical findings - look for next most important work
    suggestions = await get_suggestions(limit=1)
    if suggestions:
        top = suggestions[0]
        return {
            "priority": top["priority"],
            "task": top["title"],
            "action": top["action"],
            "file": top.get("file"),
            "why_this_matters": top["urgency"],
            "estimated_time": top["estimated_effort"]
        }

    # Nothing urgent - return project status
    components = db.get_all_components()
    incomplete = [c for c in components if c.get("status") != "complete"]

    if incomplete:
        return {
            "priority": "NORMAL",
            "task": "Continue development on incomplete components",
            "action": f"Focus on: {', '.join(c['name'] for c in incomplete[:3])}",
            "why_this_matters": "Core hypervisor functionality needs implementation",
            "estimated_time": "Varies by component"
        }

    return {
        "priority": "NONE",
        "task": "No critical work items",
        "action": "Project is in good shape. Consider adding new features or running tests.",
        "why_this_matters": "All critical issues resolved"
    }


# =============================================================================
# COMPONENTS
# =============================================================================

async def get_component(component_id: str) -> Optional[Dict[str, Any]]:
    """
    Get details on a specific component.

    Args:
        component_id: Component identifier (e.g., "vmcs_setup", "ept")
    """
    db = get_db()
    return db.get_component(component_id)


async def get_exit_handler_status(status: Optional[str] = None) -> List[Dict[str, Any]]:
    """
    Get status of all exit handlers.

    Args:
        status: Filter by status (implemented, partial, stub, missing)
    """
    db = get_db()
    return db.get_exit_handlers(status=status)


# =============================================================================
# DECISIONS
# =============================================================================

async def add_decision(
    topic: str,
    choice: str,
    rationale: Optional[str] = None,
    alternatives: Optional[List[str]] = None,
    affects: Optional[List[str]] = None
) -> Dict[str, Any]:
    """
    Record a design decision.

    Args:
        topic: What the decision is about
        choice: What was decided
        rationale: Why this choice was made
        alternatives: Other options that were considered
        affects: Files or components affected by this decision
    """
    db = get_db()
    decision_id = db.add_decision(
        topic=topic,
        choice=choice,
        rationale=rationale,
        alternatives=alternatives,
        affects=affects
    )
    return {
        "decision_id": decision_id,
        "topic": topic,
        "choice": choice,
        "message": f"Decision {decision_id} recorded"
    }


async def get_decision(decision_id: str) -> Optional[Dict[str, Any]]:
    """
    Look up a design decision.

    Args:
        decision_id: Decision ID (e.g., "D001")
    """
    db = get_db()
    return db.get_decision(decision_id)


async def list_decisions(
    topic: Optional[str] = None,
    affects: Optional[str] = None
) -> List[Dict[str, Any]]:
    """
    List all decisions, optionally filtered.

    Args:
        topic: Filter by topic (partial match)
        affects: Filter by affected file/component (partial match)
    """
    db = get_db()
    return db.list_decisions(topic=topic, affects=affects)


# =============================================================================
# GOTCHAS
# =============================================================================

async def add_gotcha(
    symptom: str,
    cause: str,
    fix: str,
    files: Optional[List[str]] = None
) -> Dict[str, Any]:
    """
    Record a solved bug for future reference.

    Args:
        symptom: What the bug looked like
        cause: What actually caused it
        fix: How it was fixed
        files: Affected files (e.g., ["vmcs.c:45", "ept.c:123"])
    """
    db = get_db()
    gotcha_id = db.add_gotcha(
        symptom=symptom,
        cause=cause,
        fix=fix,
        files=files
    )
    return {
        "gotcha_id": gotcha_id,
        "symptom": symptom,
        "message": f"Gotcha {gotcha_id} recorded"
    }


async def search_gotchas(keyword: str) -> List[Dict[str, Any]]:
    """
    Search gotchas by keyword.

    Args:
        keyword: Search term (matches symptom, cause, or fix)
    """
    db = get_db()
    return db.search_gotchas(keyword)


# =============================================================================
# SESSION CONTEXT
# =============================================================================

async def get_session_context() -> Optional[Dict[str, Any]]:
    """
    Get context from the last session.

    Returns what we were working on last time, useful for resuming work.
    """
    db = get_db()
    return db.get_last_session()


async def save_session_context(
    working_on: str,
    context: Optional[str] = None,
    files_touched: Optional[List[str]] = None
) -> Dict[str, Any]:
    """
    Save current session context for later resumption.

    Args:
        working_on: Brief description of current work
        context: Detailed context/notes
        files_touched: List of files modified this session
    """
    db = get_db()

    # Start or get current session
    session_id = db.start_session()
    db.update_session(
        session_id=session_id,
        working_on=working_on,
        context_snapshot=context,
        files_touched=files_touched
    )
    db.end_session(session_id)

    return {
        "session_id": session_id,
        "working_on": working_on,
        "message": "Session context saved"
    }


# =============================================================================
# DAEMON CONTROL
# =============================================================================

async def refresh_analysis(path: Optional[str] = None) -> Dict[str, Any]:
    """
    Force a rescan of the codebase.

    Args:
        path: Optional specific path to scan (default: entire project)

    Note: For full rescans, this runs asynchronously and returns immediately.
    Check daemon status later for results.
    """
    import asyncio
    from concurrent.futures import ThreadPoolExecutor
    from ombra_watcherd.daemon import OmbraWatcherd

    watch_path = Path(path) if path else Path(__file__).parent.parent.parent.parent.parent
    daemon = OmbraWatcherd(watch_path=watch_path)

    def do_scan():
        if path:
            return daemon.scan_path(Path(path))
        else:
            return daemon.scan_all()

    # For single file/path scans, run inline (fast)
    if path:
        loop = asyncio.get_event_loop()
        with ThreadPoolExecutor(max_workers=1) as executor:
            result = await loop.run_in_executor(executor, do_scan)
        return result

    # For full scans, run in background and return immediately
    # This prevents blocking the MCP server for minutes
    loop = asyncio.get_event_loop()
    executor = ThreadPoolExecutor(max_workers=1)
    loop.run_in_executor(executor, do_scan)

    return {
        "status": "scan_started",
        "message": "Full scan started in background. Check daemon status for progress.",
        "path": str(watch_path)
    }


async def get_daemon_status() -> Dict[str, Any]:
    """
    Get status of the watcher daemon.

    Returns running state, last scan time, and finding counts.
    """
    from ombra_watcherd.launchd import LaunchdManager

    manager = LaunchdManager()
    return manager.status()


async def seed_components() -> Dict[str, Any]:
    """
    Seed the components and exit_handlers tables with current hypervisor status.

    This runs the seed_components.py script to populate the database with
    accurate component tracking based on actual codebase analysis.

    Use this when:
    - First setting up the Project Brain
    - After major refactoring that changes component structure
    - To reset component tracking to current state
    """
    import subprocess
    from pathlib import Path

    script_path = Path(__file__).parent.parent.parent.parent / "scripts" / "seed_components.py"

    if not script_path.exists():
        return {
            "success": False,
            "error": f"Seed script not found at {script_path}"
        }

    try:
        result = subprocess.run(
            ["python3", str(script_path)],
            capture_output=True,
            text=True,
            timeout=30
        )

        return {
            "success": result.returncode == 0,
            "returncode": result.returncode,
            "stdout": result.stdout,
            "stderr": result.stderr
        }
    except subprocess.TimeoutExpired:
        return {
            "success": False,
            "error": "Seed script timed out after 30 seconds"
        }
    except Exception as e:
        return {
            "success": False,
            "error": str(e)
        }
