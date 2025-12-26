"""
End-to-end integration tests for Concept Intelligence System.
"""

import pytest
import asyncio
from pathlib import Path
import tempfile

from ombra_watcherd.concepts_db import ConceptsDB
from ombra_watcherd.analyzers.concept import ConceptAnalyzer
from ombra_mcp.tools.concepts import (
    get_concept,
    list_concepts,
    get_implementation_gaps,
    suggest_next_work,
)


@pytest.fixture
def populated_db():
    """Create a database with test concepts."""
    with tempfile.TemporaryDirectory() as tmpdir:
        db_path = Path(tmpdir) / "concepts.db"
        db = ConceptsDB(db_path)

        # Add test concepts
        db.add_concept({
            "id": "TSC_OFFSET_COMPENSATION",
            "category": "timing",
            "name": "TSC Offset Compensation",
            "description": "Adjust TSC offset on every VMexit",
            "required_patterns": [r"__rdtsc\(\)", r"0x2010"],
            "priority": "critical",
            "implementation_status": "not_started",
        })

        db.add_concept({
            "id": "CPUID_HV_BIT",
            "category": "stealth",
            "name": "Hide Hypervisor Bit",
            "description": "Clear CPUID.1.ECX[31]",
            "required_patterns": [r"ecx.*31", r"cpuid"],
            "priority": "high",
            "implementation_status": "partial",
            "depends_on": [],
        })

        db.add_concept({
            "id": "EPT_VIOLATION",
            "category": "ept",
            "name": "EPT Violation Handler",
            "description": "Handle exit reason 48",
            "required_patterns": [r"exit_reason.*48", r"ept"],
            "priority": "medium",
            "implementation_status": "complete",
        })

        yield db_path


@pytest.mark.asyncio
async def test_full_workflow(populated_db, monkeypatch):
    """Test complete workflow from concept query to gap analysis."""
    # Patch the database path
    import ombra_mcp.tools.concepts as concepts_module
    concepts_module._db = ConceptsDB(populated_db)

    # 1. List all concepts
    all_concepts = await list_concepts()
    assert len(all_concepts) == 3

    # 2. Filter by category
    timing = await list_concepts(category="timing")
    assert len(timing) == 1
    assert timing[0]["id"] == "TSC_OFFSET_COMPENSATION"

    # 3. Get implementation gaps
    gaps = await get_implementation_gaps()
    assert len(gaps) == 2  # not_started and partial

    # 4. Get suggestions
    suggestions = await suggest_next_work()
    assert len(suggestions["suggestions"]) > 0
    # Should suggest TSC_OFFSET first (critical priority, no deps)
    assert suggestions["suggestions"][0]["priority"] == "critical"


@pytest.mark.asyncio
async def test_analyzer_integration(populated_db):
    """Test that analyzer correctly detects implementations."""
    analyzer = ConceptAnalyzer(populated_db)

    code = """
    void handle_vmexit() {
        uint64_t tsc = __rdtsc();
        // Process exit
        VmcsWrite(0x2010, new_offset);
    }
    """

    findings = analyzer.analyze(Path("test.c"), code)

    # Should find complete TSC implementation
    tsc_findings = [f for f in findings if "TSC" in f["concept_id"]]
    assert len(tsc_findings) == 1
    assert tsc_findings[0]["finding_type"] == "complete"
