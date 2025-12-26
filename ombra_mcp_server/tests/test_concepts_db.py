# tests/test_concepts_db.py
import pytest
from pathlib import Path
import tempfile
from ombra_watcherd.concepts_db import ConceptsDB

def test_create_database():
    with tempfile.TemporaryDirectory() as tmpdir:
        db_path = Path(tmpdir) / "concepts.db"
        db = ConceptsDB(db_path)

        # Should create tables
        assert db_path.exists()

        # Should have concepts table
        tables = db.execute("SELECT name FROM sqlite_master WHERE type='table'").fetchall()
        table_names = [t[0] for t in tables]
        assert "concepts" in table_names
        assert "concept_findings" in table_names
        assert "concept_annotations" in table_names


def test_add_and_get_concept():
    with tempfile.TemporaryDirectory() as tmpdir:
        db = ConceptsDB(Path(tmpdir) / "concepts.db")

        concept = {
            "id": "TSC_OFFSET_COMPENSATION",
            "category": "timing",
            "name": "TSC Offset Compensation",
            "description": "Adjust TSC offset on every VMexit to hide overhead",
            "priority": "critical",
            "required_patterns": ["__rdtsc", "vmwrite.*0x2010"],
            "vmcs_fields": ["0x2010"],
            "exit_reasons": [10, 31],
        }

        db.add_concept(concept)

        retrieved = db.get_concept("TSC_OFFSET_COMPENSATION")
        assert retrieved is not None
        assert retrieved["name"] == "TSC Offset Compensation"
        assert retrieved["category"] == "timing"
        assert "0x2010" in retrieved["vmcs_fields"]


def test_list_concepts_by_category():
    with tempfile.TemporaryDirectory() as tmpdir:
        db = ConceptsDB(Path(tmpdir) / "concepts.db")

        db.add_concept({"id": "C1", "category": "timing", "name": "Concept 1"})
        db.add_concept({"id": "C2", "category": "timing", "name": "Concept 2"})
        db.add_concept({"id": "C3", "category": "vmx", "name": "Concept 3"})

        timing = db.list_concepts(category="timing")
        assert len(timing) == 2

        all_concepts = db.list_concepts()
        assert len(all_concepts) == 3


def test_add_and_query_findings():
    with tempfile.TemporaryDirectory() as tmpdir:
        db = ConceptsDB(Path(tmpdir) / "concepts.db")

        db.add_concept({"id": "C1", "category": "timing", "name": "Concept 1"})

        finding_id = db.add_finding(
            concept_id="C1",
            file_path="vmx.c",
            line_number=45,
            finding_type="partial",
            message="Found TSC read but no offset write",
            severity="warning"
        )

        assert finding_id > 0

        findings = db.get_findings(concept_id="C1")
        assert len(findings) == 1
        assert findings[0]["file_path"] == "vmx.c"
        assert findings[0]["severity"] == "warning"


def test_dismiss_finding():
    with tempfile.TemporaryDirectory() as tmpdir:
        db = ConceptsDB(Path(tmpdir) / "concepts.db")

        db.add_concept({"id": "C1", "category": "timing", "name": "Concept 1"})
        finding_id = db.add_finding(
            concept_id="C1",
            file_path="vmx.c",
            line_number=45,
            finding_type="partial",
            message="Test finding",
            severity="warning"
        )

        db.dismiss_finding(finding_id, "False positive - handled elsewhere")

        findings = db.get_findings(concept_id="C1", include_dismissed=False)
        assert len(findings) == 0

        all_findings = db.get_findings(concept_id="C1", include_dismissed=True)
        assert len(all_findings) == 1
        assert all_findings[0]["dismissed"] == 1


def test_add_annotation_with_verification():
    with tempfile.TemporaryDirectory() as tmpdir:
        db = ConceptsDB(Path(tmpdir) / "concepts.db")

        db.add_concept({"id": "C1", "category": "timing", "name": "Concept 1"})
        db.add_concept({"id": "C2", "category": "timing", "name": "Concept 2"})

        # Verified annotation should set flag
        annotation_id = db.add_annotation(
            concept_id="C1",
            file_path="vmx.c",
            line_number=100,
            annotation_type="verified",
            notes="Manually verified implementation"
        )

        assert annotation_id > 0
        concept = db.get_concept("C1")
        assert concept["verified_by_annotation"] == 1

        # Non-verified annotation should NOT set flag
        db.add_annotation(
            concept_id="C2",
            file_path="handler.c",
            line_number=50,
            annotation_type="note",
            notes="Needs review"
        )
        concept2 = db.get_concept("C2")
        assert concept2["verified_by_annotation"] == 0


def test_get_annotations():
    with tempfile.TemporaryDirectory() as tmpdir:
        db = ConceptsDB(Path(tmpdir) / "concepts.db")

        db.add_concept({"id": "C1", "category": "timing", "name": "Concept 1"})
        db.add_concept({"id": "C2", "category": "vmx", "name": "Concept 2"})

        # Add multiple annotations
        db.add_annotation("C1", "vmx.c", 100, "verified", "Fully implemented")
        db.add_annotation("C1", "exit.c", 200, "note", "Check edge cases")
        db.add_annotation("C2", "vmx.c", 150, "todo", "Needs implementation")

        # Query by concept_id
        c1_annotations = db.get_annotations(concept_id="C1")
        assert len(c1_annotations) == 2
        assert all(a["concept_id"] == "C1" for a in c1_annotations)

        # Query by file_path
        vmx_annotations = db.get_annotations(file_path="vmx.c")
        assert len(vmx_annotations) == 2
        assert all("vmx.c" in a["file_path"] for a in vmx_annotations)

        # Query by annotation_type
        verified = db.get_annotations(annotation_type="verified")
        assert len(verified) == 1
        assert verified[0]["annotation_type"] == "verified"

        # Query all
        all_annotations = db.get_annotations()
        assert len(all_annotations) == 3
