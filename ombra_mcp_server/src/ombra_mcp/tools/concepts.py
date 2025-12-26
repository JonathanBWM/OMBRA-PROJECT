"""
Concept Intelligence MCP Tools

Query and analyze hypervisor implementation concepts.
"""

from typing import Optional, List, Dict, Any
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from ombra_watcherd.concepts_db import ConceptsDB


_db: Optional[ConceptsDB] = None


def get_db() -> ConceptsDB:
    global _db
    if _db is None:
        _db = ConceptsDB()
    return _db


async def get_concept(concept_id: str) -> Dict[str, Any]:
    """Get full details on a concept."""
    db = get_db()
    concept = db.get_concept(concept_id)

    if not concept:
        return {"error": f"Concept '{concept_id}' not found"}

    concept["findings"] = db.get_findings(concept_id=concept_id, include_dismissed=False)
    return concept


async def list_concepts(
    category: Optional[str] = None,
    status: Optional[str] = None,
    priority: Optional[str] = None
) -> List[Dict[str, Any]]:
    """List concepts with optional filters."""
    db = get_db()
    return db.list_concepts(category=category, status=status, priority=priority)


async def get_implementation_gaps(category: Optional[str] = None) -> List[Dict[str, Any]]:
    """Get concepts that are not fully implemented."""
    db = get_db()

    gaps = []
    for status in ["not_started", "partial"]:
        concepts = db.list_concepts(category=category, status=status)
        for c in concepts:
            c["findings"] = db.get_findings(concept_id=c["id"], include_dismissed=False)
            gaps.append(c)

    priority_order = {"critical": 0, "high": 1, "medium": 2, "low": 3}
    gaps.sort(key=lambda x: priority_order.get(x.get("priority", "medium"), 2))
    return gaps


async def verify_concept(
    concept_id: str,
    file_path: str,
    line_number: int,
    notes: Optional[str] = None
) -> Dict[str, Any]:
    """Mark a concept as verified at a specific location."""
    db = get_db()

    annotation_id = db.add_annotation(
        concept_id=concept_id,
        file_path=file_path,
        line_number=line_number,
        annotation_type="verified",
        notes=notes
    )

    db.execute(
        "UPDATE concepts SET implementation_status = 'verified', confidence = 1.0 WHERE id = ?",
        (concept_id,)
    )
    db.commit()

    return {
        "success": True,
        "annotation_id": annotation_id,
        "message": f"Concept {concept_id} verified at {file_path}:{line_number}"
    }


async def dismiss_concept_finding(finding_id: int, reason: str) -> Dict[str, Any]:
    """Dismiss a concept finding as false positive."""
    db = get_db()
    db.dismiss_finding(finding_id, reason)
    return {"success": True, "message": f"Finding {finding_id} dismissed"}


async def suggest_next_work() -> Dict[str, Any]:
    """Suggest what to implement next based on priorities and dependencies."""
    db = get_db()

    gaps = []
    for status in ["not_started", "partial"]:
        gaps.extend(db.list_concepts(status=status))

    if not gaps:
        return {"message": "All concepts implemented!", "suggestions": []}

    suggestions = []
    priority_order = {"critical": 0, "high": 1, "medium": 2, "low": 3}
    gaps.sort(key=lambda x: priority_order.get(x.get("priority", "medium"), 2))

    for gap in gaps[:5]:
        deps = gap.get("depends_on") or []
        if isinstance(deps, str):
            import json
            try:
                deps = json.loads(deps)
            except:
                deps = []

        unmet = []
        for dep in deps:
            dep_concept = db.get_concept(dep)
            if dep_concept and dep_concept.get("implementation_status") not in ["complete", "verified"]:
                unmet.append(dep)

        if not unmet:
            suggestions.append({
                "concept_id": gap["id"],
                "name": gap["name"],
                "priority": gap.get("priority", "medium"),
                "category": gap["category"],
                "reason": "Ready to implement (no blocking dependencies)"
            })

    return {"suggestions": suggestions, "total_gaps": len(gaps)}
