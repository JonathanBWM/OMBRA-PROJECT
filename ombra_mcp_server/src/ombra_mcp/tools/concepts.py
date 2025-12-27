"""
Concept Intelligence System - Extract, track, and analyze hypervisor concepts

This module provides tools for:
- Querying concepts from the concept database
- Checking implementation coverage for concepts
- Identifying implementation gaps
- Suggesting next work based on dependencies
"""

from typing import Optional, List, Dict, Any
from pathlib import Path
import sys
import json

# Add watcherd to path for database access
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from ombra_watcherd.database import ProjectBrainDB


# Database location (shares project_brain.db for now)
DEFAULT_DB_PATH = Path(__file__).parent.parent / "data" / "project_brain.db"


def _get_db():
    """Get database instance."""
    return ProjectBrainDB(db_path=DEFAULT_DB_PATH)


# =============================================================================
# CONCEPT QUERY TOOLS
# =============================================================================

async def get_concept(concept_id: str) -> Optional[Dict[str, Any]]:
    """
    Get full details on a specific concept including SDM cross-references.

    Args:
        concept_id: Concept identifier (e.g., "TSC_OFFSET_COMPENSATION")

    Returns:
        Dictionary with concept details, patterns, SDM references, and status
    """
    db = _get_db()

    with db._connection() as conn:
        row = conn.execute("""
            SELECT * FROM concepts WHERE id = ?
        """, (concept_id,)).fetchone()

        if not row:
            return None

        concept = dict(row)

        # Parse JSON fields
        json_fields = [
            'required_patterns', 'optional_patterns', 'anti_patterns',
            'vmcs_fields', 'exit_reasons', 'msrs', 'implementation_files',
            'anti_cheat_relevance', 'depends_on'
        ]

        for field in json_fields:
            if concept.get(field):
                concept[field] = json.loads(concept[field])

        return concept


async def list_concepts(
    category: Optional[str] = None,
    status: Optional[str] = None,
    priority: Optional[str] = None
) -> List[Dict[str, Any]]:
    """
    List concepts with optional filtering.

    Args:
        category: Filter by category (timing, vmx, ept, stealth)
        status: Filter by status (not_started, partial, complete, verified)
        priority: Filter by priority (critical, high, medium, low)

    Returns:
        List of concept dictionaries
    """
    db = _get_db()

    conditions = []
    params = []

    if category:
        conditions.append("category = ?")
        params.append(category)

    if status:
        conditions.append("implementation_status = ?")
        params.append(status)

    if priority:
        conditions.append("priority = ?")
        params.append(priority)

    where = "WHERE " + " AND ".join(conditions) if conditions else ""

    with db._connection() as conn:
        rows = conn.execute(f"""
            SELECT
                id, category, name, description,
                implementation_status, confidence, priority,
                verified_by_annotation
            FROM concepts
            {where}
            ORDER BY
                CASE priority
                    WHEN 'critical' THEN 1
                    WHEN 'high' THEN 2
                    WHEN 'medium' THEN 3
                    ELSE 4
                END,
                category,
                phase_order
        """, params).fetchall()

        return [dict(row) for row in rows]


async def check_concept_coverage(concept_id: str) -> Dict[str, Any]:
    """
    Deep analysis of a concept's implementation with gap report.

    Checks:
    - Which required patterns are present
    - Which are missing
    - Anti-patterns detected
    - Files implementing this concept
    - Confidence score

    Args:
        concept_id: Concept to analyze

    Returns:
        Dictionary with coverage analysis and gaps
    """
    concept = await get_concept(concept_id)

    if not concept:
        return {
            "error": f"Concept {concept_id} not found"
        }

    db = _get_db()

    # Get findings related to this concept
    with db._connection() as conn:
        findings = conn.execute("""
            SELECT * FROM concept_findings
            WHERE concept_id = ? AND NOT dismissed
            ORDER BY severity DESC, detected_at DESC
        """, (concept_id,)).fetchall()

    findings_list = [dict(f) for f in findings]

    # Categorize findings
    missing = [f for f in findings_list if f['finding_type'] == 'missing']
    partial = [f for f in findings_list if f['finding_type'] == 'partial']
    antipatterns = [f for f in findings_list if f['finding_type'] == 'antipattern']
    complete = [f for f in findings_list if f['finding_type'] == 'complete']

    # Calculate coverage score
    required_count = len(concept.get('required_patterns', []))
    found_count = len(complete)

    if required_count > 0:
        coverage = (found_count / required_count) * 100
    else:
        coverage = 100.0 if concept['implementation_status'] == 'complete' else 0.0

    return {
        "concept_id": concept_id,
        "name": concept['name'],
        "status": concept['implementation_status'],
        "confidence": concept['confidence'],
        "verified": concept['verified_by_annotation'],
        "coverage_percent": coverage,
        "required_patterns": concept.get('required_patterns', []),
        "found_patterns": found_count,
        "missing_patterns": len(missing),
        "antipatterns_detected": len(antipatterns),
        "findings": {
            "missing": missing,
            "partial": partial,
            "antipatterns": antipatterns,
            "complete": complete
        },
        "implementation_files": concept.get('implementation_files', []),
        "dependencies": concept.get('depends_on', [])
    }


# =============================================================================
# GAP ANALYSIS TOOLS
# =============================================================================

async def get_implementation_gaps(category: Optional[str] = None) -> List[Dict[str, Any]]:
    """
    Get list of missing or partial concept implementations.

    Sorted by priority to show what needs attention first.

    Args:
        category: Optional filter by category (timing, vmx, ept, stealth)

    Returns:
        List of gaps with details on what's missing
    """
    db = _get_db()

    conditions = ["implementation_status IN ('not_started', 'partial')"]
    params = []

    if category:
        conditions.append("category = ?")
        params.append(category)

    where = "WHERE " + " AND ".join(conditions)

    with db._connection() as conn:
        rows = conn.execute(f"""
            SELECT
                id, category, name, description,
                implementation_status, confidence, priority,
                depends_on, phase_order
            FROM concepts
            {where}
            ORDER BY
                CASE priority
                    WHEN 'critical' THEN 1
                    WHEN 'high' THEN 2
                    WHEN 'medium' THEN 3
                    ELSE 4
                END,
                phase_order
        """, params).fetchall()

        gaps = []
        for row in rows:
            concept = dict(row)

            # Parse depends_on
            if concept.get('depends_on'):
                concept['depends_on'] = json.loads(concept['depends_on'])
            else:
                concept['depends_on'] = []

            # Check if dependencies are satisfied
            all_deps_met = True
            if concept['depends_on']:
                for dep_id in concept['depends_on']:
                    dep_row = conn.execute("""
                        SELECT implementation_status FROM concepts WHERE id = ?
                    """, (dep_id,)).fetchone()

                    if dep_row and dep_row['implementation_status'] not in ['complete', 'verified']:
                        all_deps_met = False
                        break

            concept['dependencies_satisfied'] = all_deps_met
            gaps.append(concept)

        return gaps


async def suggest_next_work() -> Dict[str, Any]:
    """
    Suggest what to implement next based on:
    - Dependencies (don't suggest if deps not met)
    - Priority (critical > high > medium > low)
    - Phase order (sequential work)
    - Current status (prefer partial over not_started for continuity)

    Returns:
        Dictionary with top suggestions and reasoning
    """
    gaps = await get_implementation_gaps()

    # Filter to only items with satisfied dependencies
    ready = [g for g in gaps if g['dependencies_satisfied']]

    if not ready:
        # Check if we're blocked by dependencies
        blocked = [g for g in gaps if not g['dependencies_satisfied']]
        if blocked:
            return {
                "suggestion": "blocked_by_dependencies",
                "message": "All remaining work is blocked by dependencies",
                "blockers": blocked[:5]  # Show top 5 blockers
            }
        else:
            return {
                "suggestion": "complete",
                "message": "All concepts implemented or in progress!"
            }

    # Sort by priority, then phase order
    ready.sort(key=lambda x: (
        {'critical': 0, 'high': 1, 'medium': 2, 'low': 3}.get(x['priority'], 4),
        x['phase_order'] or 999
    ))

    # Prefer partial (continue existing work)
    partial_work = [g for g in ready if g['implementation_status'] == 'partial']
    if partial_work:
        top = partial_work[0]
        return {
            "suggestion": "continue_partial",
            "concept_id": top['id'],
            "name": top['name'],
            "category": top['category'],
            "priority": top['priority'],
            "reason": f"Continue partial implementation in {top['category']}",
            "alternatives": ready[1:4]  # Show next 3 options
        }
    else:
        # Start new work
        top = ready[0]
        return {
            "suggestion": "start_new",
            "concept_id": top['id'],
            "name": top['name'],
            "category": top['category'],
            "priority": top['priority'],
            "reason": f"Start {top['priority']}-priority work in {top['category']}",
            "alternatives": ready[1:4]
        }


# =============================================================================
# ANNOTATION TOOLS
# =============================================================================

async def verify_concept(
    concept_id: str,
    file_path: str,
    line_number: int,
    notes: Optional[str] = None
) -> Dict[str, Any]:
    """
    Mark a concept as verified at a specific location.

    Human verification overrides pattern detection.

    Args:
        concept_id: Concept being verified
        file_path: File where implementation exists
        line_number: Line number of implementation
        notes: Optional notes about the implementation

    Returns:
        Success status
    """
    db = _get_db()

    with db._connection() as conn:
        # Add annotation
        conn.execute("""
            INSERT INTO concept_annotations
            (concept_id, file_path, line_number, annotation_type, notes, created_at)
            VALUES (?, ?, ?, 'verified', ?, CURRENT_TIMESTAMP)
        """, (concept_id, file_path, line_number, notes))

        # Update concept status
        conn.execute("""
            UPDATE concepts SET
                verified_by_annotation = 1,
                confidence = 1.0,
                implementation_status = 'verified'
            WHERE id = ?
        """, (concept_id,))

    return {
        "success": True,
        "concept_id": concept_id,
        "message": f"Concept {concept_id} verified at {file_path}:{line_number}"
    }


async def dismiss_concept_finding(
    finding_id: int,
    reason: Optional[str] = None
) -> Dict[str, Any]:
    """
    Dismiss a concept finding as false positive.

    Args:
        finding_id: ID of the finding to dismiss
        reason: Optional reason for dismissal

    Returns:
        Success status
    """
    db = _get_db()

    with db._connection() as conn:
        cursor = conn.execute("""
            UPDATE concept_findings SET
                dismissed = TRUE,
                dismissed_reason = ?
            WHERE id = ?
        """, (reason, finding_id))

        if cursor.rowcount == 0:
            return {
                "success": False,
                "message": f"Finding {finding_id} not found"
            }

    return {
        "success": True,
        "finding_id": finding_id,
        "message": "Finding dismissed"
    }
