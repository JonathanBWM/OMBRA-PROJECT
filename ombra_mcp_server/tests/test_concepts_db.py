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
