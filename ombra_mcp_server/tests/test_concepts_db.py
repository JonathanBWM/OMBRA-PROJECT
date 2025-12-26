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
