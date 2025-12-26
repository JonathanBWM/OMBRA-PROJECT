"""
Tests for ConceptAnalyzer - pattern-based concept detection.
"""

import pytest
from pathlib import Path
import tempfile
from ombra_watcherd.analyzers.concept import ConceptAnalyzer
from ombra_watcherd.concepts_db import ConceptsDB


SAMPLE_CODE_PARTIAL = """
// vmx.c - VM Exit Handler
void vmexit_handler(void) {
    // Capture TSC at entry
    uint64_t entry_tsc = __rdtsc();

    // Handle exit...

    // TODO: Adjust TSC offset before resume
}
"""

SAMPLE_CODE_COMPLETE = """
// vmx.c - VM Exit Handler
void vmexit_handler(void) {
    uint64_t entry_tsc = __rdtsc();

    // Handle exit...

    uint64_t exit_tsc = __rdtsc();
    uint64_t overhead = exit_tsc - entry_tsc;

    // Compensate TSC offset
    uint64_t current_offset = VmcsRead(0x2010);
    VmcsWrite(0x2010, current_offset - overhead);
}
"""

SAMPLE_CODE_ANTIPATTERN = """
// vmx.c - VM Exit Handler with anti-pattern
void vmexit_handler(void) {
    uint64_t entry_tsc = __rdtsc();

    // Handle exit...

    // BAD: Hardcoded offset instead of measuring overhead
    VmcsWrite(0x2010, 12345);
}
"""


def test_detect_partial_implementation():
    """Should detect partial concept implementations."""
    with tempfile.TemporaryDirectory() as tmpdir:
        db_path = Path(tmpdir) / "concepts.db"
        db = ConceptsDB(db_path)

        # Add a concept with required patterns
        db.add_concept({
            "id": "TSC_OFFSET_COMPENSATION",
            "category": "timing",
            "name": "TSC Offset Compensation",
            "required_patterns": [r"__rdtsc\(\)", r"0x2010|TSC_OFFSET"],
            "priority": "critical",
        })

        analyzer = ConceptAnalyzer(db_path)
        findings = analyzer.analyze(Path("vmx.c"), SAMPLE_CODE_PARTIAL)

        # Should find partial implementation (has __rdtsc but missing offset write)
        partial = [f for f in findings if f["finding_type"] == "partial"]
        assert len(partial) >= 1
        assert "TSC_OFFSET_COMPENSATION" in partial[0]["concept_id"]
        assert partial[0]["severity"] == "warning"


def test_detect_complete_implementation():
    """Should detect complete concept implementations."""
    with tempfile.TemporaryDirectory() as tmpdir:
        db_path = Path(tmpdir) / "concepts.db"
        db = ConceptsDB(db_path)

        db.add_concept({
            "id": "TSC_OFFSET_COMPENSATION",
            "category": "timing",
            "name": "TSC Offset Compensation",
            "required_patterns": [r"__rdtsc\(\)", r"0x2010"],
            "priority": "critical",
        })

        analyzer = ConceptAnalyzer(db_path)
        findings = analyzer.analyze(Path("vmx.c"), SAMPLE_CODE_COMPLETE)

        # Should find complete implementation (has all required patterns)
        complete = [f for f in findings if f["finding_type"] == "complete"]
        assert len(complete) >= 1
        assert "TSC_OFFSET_COMPENSATION" in complete[0]["concept_id"]
        assert complete[0]["severity"] == "info"


def test_detect_antipattern():
    """Should detect anti-patterns in implementations."""
    with tempfile.TemporaryDirectory() as tmpdir:
        db_path = Path(tmpdir) / "concepts.db"
        db = ConceptsDB(db_path)

        db.add_concept({
            "id": "TSC_OFFSET_COMPENSATION",
            "category": "timing",
            "name": "TSC Offset Compensation",
            "required_patterns": [r"__rdtsc\(\)", r"0x2010"],
            "anti_patterns": [r"VmcsWrite\(0x2010,\s*\d+\)"],  # Hardcoded value
            "priority": "critical",
        })

        analyzer = ConceptAnalyzer(db_path)
        findings = analyzer.analyze(Path("vmx.c"), SAMPLE_CODE_ANTIPATTERN)

        # Should detect anti-pattern
        anti = [f for f in findings if f["finding_type"] == "antipattern"]
        assert len(anti) >= 1
        assert "TSC_OFFSET_COMPENSATION" in anti[0]["concept_id"]
        assert anti[0]["severity"] == "critical"


def test_ignore_irrelevant_files():
    """Should return empty findings for files without relevant patterns."""
    with tempfile.TemporaryDirectory() as tmpdir:
        db_path = Path(tmpdir) / "concepts.db"
        db = ConceptsDB(db_path)

        db.add_concept({
            "id": "TSC_OFFSET_COMPENSATION",
            "category": "timing",
            "name": "TSC Offset Compensation",
            "required_patterns": [r"__rdtsc\(\)", r"0x2010"],
            "priority": "critical",
        })

        analyzer = ConceptAnalyzer(db_path)

        # Code with no relevant patterns
        unrelated_code = """
        void some_other_function(void) {
            printf("Hello world\\n");
        }
        """

        findings = analyzer.analyze(Path("other.c"), unrelated_code)
        assert len(findings) == 0


def test_should_analyze_file_patterns():
    """Should only analyze files matching configured patterns."""
    with tempfile.TemporaryDirectory() as tmpdir:
        db_path = Path(tmpdir) / "concepts.db"
        analyzer = ConceptAnalyzer(db_path)

        # Should analyze C/C++ files
        assert analyzer.should_analyze(Path("vmx.c"))
        assert analyzer.should_analyze(Path("ept.cpp"))
        assert analyzer.should_analyze(Path("vmcs.h"))
        assert analyzer.should_analyze(Path("entry.asm"))

        # Should not analyze other files
        assert not analyzer.should_analyze(Path("README.md"))
        assert not analyzer.should_analyze(Path("config.json"))
        assert not analyzer.should_analyze(Path("script.py"))


def test_multiple_concepts():
    """Should detect multiple concepts in a single file."""
    with tempfile.TemporaryDirectory() as tmpdir:
        db_path = Path(tmpdir) / "concepts.db"
        db = ConceptsDB(db_path)

        # Add multiple concepts
        db.add_concept({
            "id": "TSC_OFFSET_COMPENSATION",
            "category": "timing",
            "name": "TSC Offset Compensation",
            "required_patterns": [r"__rdtsc\(\)", r"0x2010"],
            "priority": "critical",
        })

        db.add_concept({
            "id": "EPT_VIOLATION_HANDLING",
            "category": "memory",
            "name": "EPT Violation Handling",
            "required_patterns": [r"EPT_VIOLATION", r"GPA"],
            "priority": "high",
        })

        code = """
        void vmexit_handler(void) {
            if (exit_reason == EPT_VIOLATION) {
                uint64_t GPA = VmcsRead(GUEST_PHYSICAL_ADDR);
                // Handle EPT violation
            }

            uint64_t tsc = __rdtsc();
            VmcsWrite(0x2010, tsc);
        }
        """

        analyzer = ConceptAnalyzer(db_path)
        findings = analyzer.analyze(Path("vmx.c"), code)

        # Should find both concepts
        concept_ids = {f["concept_id"] for f in findings}
        assert "TSC_OFFSET_COMPENSATION" in concept_ids
        assert "EPT_VIOLATION_HANDLING" in concept_ids
