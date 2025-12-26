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

    result = {
        "health_score": _calculate_health_score(health),
        "findings": health["findings"],
        "total_findings": health["total_findings"],
        "critical_count": health["critical_count"],
        "components": health["components"],
        "exit_handlers": health["exit_handlers"],
    }

    if verbose:
        result["critical_findings"] = db.get_critical_findings()
        result["pending_suggestions"] = db.get_suggestions(limit=5)
        result["components_list"] = db.get_all_components()

    return result


def _calculate_health_score(health: Dict) -> str:
    """Calculate a simple health score."""
    critical = health.get("critical_count", 0)
    warnings = health.get("findings", {}).get("warning", 0)

    if critical > 0:
        return "critical"
    elif warnings > 5:
        return "needs_attention"
    elif warnings > 0:
        return "good"
    else:
        return "excellent"


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
    Get pending suggestions for what to work on next.

    Args:
        priority: Filter by priority (high, medium, low)
        limit: Maximum suggestions to return
    """
    db = get_db()
    return db.get_suggestions(priority=priority, limit=limit)


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
    Force a full rescan of the codebase.

    Args:
        path: Optional specific path to scan (default: entire project)
    """
    from ombra_watcherd.daemon import OmbraWatcherd

    watch_path = Path(path) if path else Path("/Users/jonathanmcclintock/PROJECT-OMBRA")
    daemon = OmbraWatcherd(watch_path=watch_path)

    if path:
        result = daemon.scan_path(Path(path))
    else:
        result = daemon.scan_all()

    return result


async def get_daemon_status() -> Dict[str, Any]:
    """
    Get status of the watcher daemon.

    Returns running state, last scan time, and finding counts.
    """
    from ombra_watcherd.launchd import LaunchdManager

    manager = LaunchdManager()
    return manager.status()
