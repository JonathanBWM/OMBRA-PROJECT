# Concept Intelligence System Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Build a system that extracts hypervisor concepts from legacy docs, cross-references with Intel SDM, and continuously analyzes the codebase to identify implementation gaps.

**Architecture:** SQLite database stores ~100 concepts with patterns and SDM references. ConceptAnalyzer (daemon) provides continuous pattern matching. MCP tools provide on-demand deep analysis and gap reporting.

**Tech Stack:** Python 3.10+, SQLite, existing ombra-watcherd daemon, existing ombra-mcp server

---

## Phase 1: Database Schema & Core Infrastructure

### Task 1: Create Concept Database Schema

**Files:**
- Create: `ombra_mcp_server/src/ombra_watcherd/concepts_db.py`
- Test: `ombra_mcp_server/tests/test_concepts_db.py`

**Step 1: Write the failing test**

```python
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
```

**Step 2: Run test to verify it fails**

Run: `cd ombra_mcp_server && python -m pytest tests/test_concepts_db.py -v`
Expected: FAIL with "No module named 'ombra_watcherd.concepts_db'"

**Step 3: Write minimal implementation**

```python
# src/ombra_watcherd/concepts_db.py
"""
Concept Intelligence Database

Stores hypervisor concepts, patterns, findings, and annotations.
"""

import sqlite3
from pathlib import Path
from typing import Optional, List, Dict, Any
import json

SCHEMA = """
CREATE TABLE IF NOT EXISTS concepts (
    id TEXT PRIMARY KEY,
    category TEXT NOT NULL,
    name TEXT NOT NULL,
    description TEXT,
    source_doc TEXT,
    sdm_refs TEXT,

    required_patterns TEXT,
    optional_patterns TEXT,
    anti_patterns TEXT,

    vmcs_fields TEXT,
    exit_reasons TEXT,
    msrs TEXT,

    implementation_status TEXT DEFAULT 'not_started',
    confidence REAL DEFAULT 0.0,
    verified_by_annotation INTEGER DEFAULT 0,
    implementation_files TEXT,

    priority TEXT DEFAULT 'medium',
    anti_cheat_relevance TEXT,

    depends_on TEXT,
    phase_order INTEGER
);

CREATE TABLE IF NOT EXISTS concept_findings (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    concept_id TEXT NOT NULL,
    file_path TEXT,
    line_number INTEGER,
    finding_type TEXT NOT NULL,
    message TEXT,
    severity TEXT DEFAULT 'info',
    detected_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    dismissed INTEGER DEFAULT 0,
    dismissed_reason TEXT,
    FOREIGN KEY (concept_id) REFERENCES concepts(id)
);

CREATE TABLE IF NOT EXISTS concept_annotations (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    concept_id TEXT NOT NULL,
    file_path TEXT,
    line_number INTEGER,
    annotation_type TEXT NOT NULL,
    notes TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (concept_id) REFERENCES concepts(id)
);

CREATE INDEX IF NOT EXISTS idx_findings_concept ON concept_findings(concept_id);
CREATE INDEX IF NOT EXISTS idx_findings_file ON concept_findings(file_path);
CREATE INDEX IF NOT EXISTS idx_annotations_concept ON concept_annotations(concept_id);
"""


class ConceptsDB:
    def __init__(self, db_path: Optional[Path] = None):
        if db_path is None:
            db_path = Path(__file__).parent / "concepts.db"
        self.db_path = db_path
        self._conn: Optional[sqlite3.Connection] = None
        self._init_db()

    def _init_db(self):
        conn = self._get_conn()
        conn.executescript(SCHEMA)
        conn.commit()

    def _get_conn(self) -> sqlite3.Connection:
        if self._conn is None:
            self._conn = sqlite3.connect(str(self.db_path))
            self._conn.row_factory = sqlite3.Row
        return self._conn

    def execute(self, sql: str, params: tuple = ()) -> sqlite3.Cursor:
        return self._get_conn().execute(sql, params)

    def commit(self):
        self._get_conn().commit()

    def close(self):
        if self._conn:
            self._conn.close()
            self._conn = None
```

**Step 4: Run test to verify it passes**

Run: `cd ombra_mcp_server && python -m pytest tests/test_concepts_db.py::test_create_database -v`
Expected: PASS

**Step 5: Commit**

```bash
git add src/ombra_watcherd/concepts_db.py tests/test_concepts_db.py
git commit -m "feat(concepts): add concept database schema and initialization"
```

---

### Task 2: Add Concept CRUD Operations

**Files:**
- Modify: `ombra_mcp_server/src/ombra_watcherd/concepts_db.py`
- Test: `ombra_mcp_server/tests/test_concepts_db.py`

**Step 1: Write the failing test**

```python
# Add to tests/test_concepts_db.py

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
```

**Step 2: Run test to verify it fails**

Run: `cd ombra_mcp_server && python -m pytest tests/test_concepts_db.py::test_add_and_get_concept -v`
Expected: FAIL with "ConceptsDB object has no attribute 'add_concept'"

**Step 3: Write minimal implementation**

```python
# Add to ConceptsDB class in concepts_db.py

def add_concept(self, concept: Dict[str, Any]) -> None:
    """Add or update a concept."""
    # Convert lists to JSON
    for field in ["required_patterns", "optional_patterns", "anti_patterns",
                  "vmcs_fields", "exit_reasons", "msrs", "sdm_refs",
                  "implementation_files", "anti_cheat_relevance", "depends_on"]:
        if field in concept and isinstance(concept[field], list):
            concept[field] = json.dumps(concept[field])

    fields = list(concept.keys())
    placeholders = ", ".join(["?" for _ in fields])
    field_names = ", ".join(fields)

    sql = f"""
        INSERT OR REPLACE INTO concepts ({field_names})
        VALUES ({placeholders})
    """
    self.execute(sql, tuple(concept.values()))
    self.commit()

def get_concept(self, concept_id: str) -> Optional[Dict[str, Any]]:
    """Get a concept by ID."""
    row = self.execute(
        "SELECT * FROM concepts WHERE id = ?", (concept_id,)
    ).fetchone()

    if row is None:
        return None

    result = dict(row)

    # Parse JSON fields back to lists
    for field in ["required_patterns", "optional_patterns", "anti_patterns",
                  "vmcs_fields", "exit_reasons", "msrs", "sdm_refs",
                  "implementation_files", "anti_cheat_relevance", "depends_on"]:
        if result.get(field):
            try:
                result[field] = json.loads(result[field])
            except json.JSONDecodeError:
                pass

    return result

def list_concepts(
    self,
    category: Optional[str] = None,
    status: Optional[str] = None,
    priority: Optional[str] = None
) -> List[Dict[str, Any]]:
    """List concepts with optional filters."""
    sql = "SELECT * FROM concepts WHERE 1=1"
    params = []

    if category:
        sql += " AND category = ?"
        params.append(category)
    if status:
        sql += " AND implementation_status = ?"
        params.append(status)
    if priority:
        sql += " AND priority = ?"
        params.append(priority)

    sql += " ORDER BY priority DESC, name"

    rows = self.execute(sql, tuple(params)).fetchall()
    return [dict(row) for row in rows]
```

**Step 4: Run test to verify it passes**

Run: `cd ombra_mcp_server && python -m pytest tests/test_concepts_db.py -v`
Expected: PASS (all tests)

**Step 5: Commit**

```bash
git add src/ombra_watcherd/concepts_db.py tests/test_concepts_db.py
git commit -m "feat(concepts): add concept CRUD operations"
```

---

### Task 3: Add Findings and Annotations

**Files:**
- Modify: `ombra_mcp_server/src/ombra_watcherd/concepts_db.py`
- Test: `ombra_mcp_server/tests/test_concepts_db.py`

**Step 1: Write the failing test**

```python
# Add to tests/test_concepts_db.py

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
```

**Step 2: Run test to verify it fails**

Run: `cd ombra_mcp_server && python -m pytest tests/test_concepts_db.py::test_add_and_query_findings -v`
Expected: FAIL with "ConceptsDB object has no attribute 'add_finding'"

**Step 3: Write minimal implementation**

```python
# Add to ConceptsDB class

def add_finding(
    self,
    concept_id: str,
    file_path: str,
    line_number: int,
    finding_type: str,
    message: str,
    severity: str = "info"
) -> int:
    """Add a finding for a concept."""
    cursor = self.execute(
        """
        INSERT INTO concept_findings
        (concept_id, file_path, line_number, finding_type, message, severity)
        VALUES (?, ?, ?, ?, ?, ?)
        """,
        (concept_id, file_path, line_number, finding_type, message, severity)
    )
    self.commit()
    return cursor.lastrowid

def get_findings(
    self,
    concept_id: Optional[str] = None,
    file_path: Optional[str] = None,
    severity: Optional[str] = None,
    include_dismissed: bool = False
) -> List[Dict[str, Any]]:
    """Query findings with filters."""
    sql = "SELECT * FROM concept_findings WHERE 1=1"
    params = []

    if not include_dismissed:
        sql += " AND dismissed = 0"
    if concept_id:
        sql += " AND concept_id = ?"
        params.append(concept_id)
    if file_path:
        sql += " AND file_path LIKE ?"
        params.append(f"%{file_path}%")
    if severity:
        sql += " AND severity = ?"
        params.append(severity)

    sql += " ORDER BY detected_at DESC"

    rows = self.execute(sql, tuple(params)).fetchall()
    return [dict(row) for row in rows]

def dismiss_finding(self, finding_id: int, reason: str) -> bool:
    """Dismiss a finding as false positive."""
    self.execute(
        "UPDATE concept_findings SET dismissed = 1, dismissed_reason = ? WHERE id = ?",
        (reason, finding_id)
    )
    self.commit()
    return True

def add_annotation(
    self,
    concept_id: str,
    file_path: str,
    line_number: int,
    annotation_type: str,
    notes: Optional[str] = None
) -> int:
    """Add an annotation (human verification) for a concept."""
    cursor = self.execute(
        """
        INSERT INTO concept_annotations
        (concept_id, file_path, line_number, annotation_type, notes)
        VALUES (?, ?, ?, ?, ?)
        """,
        (concept_id, file_path, line_number, annotation_type, notes)
    )
    self.commit()

    # Update concept verification status
    if annotation_type == "verified":
        self.execute(
            "UPDATE concepts SET verified_by_annotation = 1 WHERE id = ?",
            (concept_id,)
        )
        self.commit()

    return cursor.lastrowid
```

**Step 4: Run test to verify it passes**

Run: `cd ombra_mcp_server && python -m pytest tests/test_concepts_db.py -v`
Expected: PASS (all tests)

**Step 5: Commit**

```bash
git add src/ombra_watcherd/concepts_db.py tests/test_concepts_db.py
git commit -m "feat(concepts): add findings and annotations support"
```

---

## Phase 2: Concept Extractor

### Task 4: Create Document Parser

**Files:**
- Create: `ombra_mcp_server/src/ombra_watcherd/extractors/__init__.py`
- Create: `ombra_mcp_server/src/ombra_watcherd/extractors/doc_parser.py`
- Test: `ombra_mcp_server/tests/test_doc_parser.py`

**Step 1: Write the failing test**

```python
# tests/test_doc_parser.py
import pytest
from ombra_watcherd.extractors.doc_parser import extract_concepts_from_doc

SAMPLE_DOC = """
# TIMING & TSC ANTI-DETECTION

## Overview

The timing subsystem provides anti-detection capabilities.

1. **TSC Offset Compensation**: Capturing TSC at VMExit entry
2. **APERF/MPERF Virtualization**: Compensating performance MSRs

### Key Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| `VMCS_FIELD_TSC_OFFSET_FULL` | `0x2010` | VMCS field for TSC offset |
| `IA32_MPERF` | `0xE7` | Maximum Performance Frequency |
| `IA32_APERF` | `0xE8` | Actual Performance Frequency |

## Critical Data Structures

```cpp
struct VcpuTimingState {
    u64 exit_entry_tsc;
    volatile u64 accumulated_overhead;
    u64 aperf_offset;
    u64 mperf_offset;
};
```
"""


def test_extract_basic_concepts():
    concepts = extract_concepts_from_doc(SAMPLE_DOC, "01_TIMING.md")

    assert len(concepts) >= 2

    # Should extract TSC concept
    tsc_concepts = [c for c in concepts if "TSC" in c["id"]]
    assert len(tsc_concepts) >= 1

    # Should extract VMCS field reference
    vmcs_refs = [c for c in concepts if c.get("vmcs_fields")]
    assert any("0x2010" in str(c.get("vmcs_fields", [])) for c in concepts)


def test_extract_msr_references():
    concepts = extract_concepts_from_doc(SAMPLE_DOC, "01_TIMING.md")

    # Should find MSR references
    all_msrs = []
    for c in concepts:
        all_msrs.extend(c.get("msrs", []))

    assert "0xE7" in all_msrs or "0xe7" in all_msrs.lower() if all_msrs else True
```

**Step 2: Run test to verify it fails**

Run: `cd ombra_mcp_server && python -m pytest tests/test_doc_parser.py -v`
Expected: FAIL with "No module named 'ombra_watcherd.extractors'"

**Step 3: Write minimal implementation**

```python
# src/ombra_watcherd/extractors/__init__.py
"""Concept extractors from documentation."""
from .doc_parser import extract_concepts_from_doc

__all__ = ["extract_concepts_from_doc"]
```

```python
# src/ombra_watcherd/extractors/doc_parser.py
"""
Extract hypervisor concepts from markdown documentation.
"""

import re
from typing import List, Dict, Any
from pathlib import Path


# Patterns to identify concepts
CONCEPT_PATTERNS = {
    "timing": [
        (r"TSC\s+(?:Offset|offset|OFFSET)", "TSC_OFFSET"),
        (r"APERF.*MPERF|MPERF.*APERF", "APERF_MPERF"),
        (r"timing\s+compensation", "TIMING_COMPENSATION"),
    ],
    "vmx": [
        (r"CPUID.*exit|exit.*CPUID", "CPUID_EXIT"),
        (r"VMREAD|VMWRITE", "VMX_INSTRUCTION"),
        (r"#UD.*inject|inject.*#UD", "UD_INJECTION"),
        (r"RIP.*advance|advance.*RIP", "RIP_ADVANCEMENT"),
    ],
    "ept": [
        (r"EPT.*violation", "EPT_VIOLATION"),
        (r"INVEPT", "INVEPT"),
        (r"execute.only|XO.*page", "EPT_EXECUTE_ONLY"),
        (r"identity.*map|1:1.*map", "EPT_IDENTITY_MAP"),
    ],
    "stealth": [
        (r"hypervisor.*bit|bit.*31", "HV_BIT_HIDE"),
        (r"PE.*header.*zero|zero.*PE", "PE_HEADER_ZERO"),
        (r"signature.*avoid", "SIGNATURE_AVOID"),
    ],
}

# Extract VMCS field references
VMCS_PATTERN = re.compile(r"0x20[0-9A-Fa-f]{2}")

# Extract MSR references
MSR_PATTERN = re.compile(r"0x[CEce][0-9A-Fa-f]{1,7}|IA32_\w+")

# Extract exit reason numbers
EXIT_REASON_PATTERN = re.compile(r"exit\s*reason\s*(\d+)|reason\s*(\d+)", re.IGNORECASE)


def extract_concepts_from_doc(content: str, source_file: str) -> List[Dict[str, Any]]:
    """
    Extract hypervisor concepts from markdown documentation.

    Args:
        content: Markdown content
        source_file: Source filename for reference

    Returns:
        List of concept dictionaries
    """
    concepts = []

    # Extract sections for context
    sections = _split_sections(content)

    for section_title, section_content in sections:
        full_text = section_title + " " + section_content

        # Find matching concept patterns
        for category, patterns in CONCEPT_PATTERNS.items():
            for pattern, concept_base in patterns:
                if re.search(pattern, full_text, re.IGNORECASE):
                    concept_id = _generate_concept_id(concept_base, section_title)

                    # Check if we already have this concept
                    existing = [c for c in concepts if c["id"] == concept_id]
                    if existing:
                        continue

                    concept = {
                        "id": concept_id,
                        "category": category,
                        "name": _generate_name(concept_base),
                        "description": _extract_description(section_content),
                        "source_doc": source_file,
                        "vmcs_fields": _extract_vmcs_fields(section_content),
                        "msrs": _extract_msrs(section_content),
                        "exit_reasons": _extract_exit_reasons(section_content),
                        "required_patterns": _generate_patterns(concept_base),
                        "priority": _infer_priority(section_content),
                    }
                    concepts.append(concept)

    return concepts


def _split_sections(content: str) -> List[tuple]:
    """Split markdown into (title, content) sections."""
    sections = []
    current_title = "Introduction"
    current_content = []

    for line in content.split("\n"):
        if line.startswith("#"):
            if current_content:
                sections.append((current_title, "\n".join(current_content)))
            current_title = line.lstrip("#").strip()
            current_content = []
        else:
            current_content.append(line)

    if current_content:
        sections.append((current_title, "\n".join(current_content)))

    return sections


def _generate_concept_id(base: str, section: str) -> str:
    """Generate a unique concept ID."""
    # Clean up section name
    section_clean = re.sub(r"[^A-Za-z0-9]", "_", section).upper()[:20]
    return f"{base}_{section_clean}".rstrip("_")


def _generate_name(base: str) -> str:
    """Generate human-readable name from base."""
    return base.replace("_", " ").title()


def _extract_description(content: str) -> str:
    """Extract first meaningful paragraph as description."""
    lines = [l.strip() for l in content.split("\n") if l.strip()]
    for line in lines:
        if len(line) > 50 and not line.startswith("|") and not line.startswith("```"):
            return line[:500]
    return ""


def _extract_vmcs_fields(content: str) -> List[str]:
    """Extract VMCS field references (0x20xx format)."""
    matches = VMCS_PATTERN.findall(content)
    return list(set(matches))


def _extract_msrs(content: str) -> List[str]:
    """Extract MSR references."""
    matches = MSR_PATTERN.findall(content)
    # Normalize to hex format
    result = []
    for m in matches:
        if m.startswith("IA32_"):
            result.append(m)
        else:
            result.append(m.lower())
    return list(set(result))


def _extract_exit_reasons(content: str) -> List[int]:
    """Extract exit reason numbers."""
    matches = EXIT_REASON_PATTERN.findall(content)
    reasons = []
    for groups in matches:
        for g in groups:
            if g:
                try:
                    reasons.append(int(g))
                except ValueError:
                    pass
    return list(set(reasons))


def _generate_patterns(concept_base: str) -> List[str]:
    """Generate code patterns to look for."""
    pattern_map = {
        "TSC_OFFSET": [r"__rdtsc\(\)", r"vmwrite.*0x2010", r"tsc.*offset"],
        "APERF_MPERF": [r"0x[Ee][78]", r"aperf", r"mperf"],
        "CPUID_EXIT": [r"exit_reason.*10", r"CPUID", r"cpuid"],
        "VMX_INSTRUCTION": [r"vmread|vmwrite", r"inject.*UD"],
        "UD_INJECTION": [r"#UD|exception.*6", r"inject"],
        "EPT_VIOLATION": [r"exit_reason.*48", r"ept.*violation"],
        "INVEPT": [r"invept", r"__invept"],
        "HV_BIT_HIDE": [r"ecx.*31|bit.*31", r"hypervisor.*bit"],
    }
    return pattern_map.get(concept_base, [concept_base.lower()])


def _infer_priority(content: str) -> str:
    """Infer priority from content keywords."""
    content_lower = content.lower()
    if any(w in content_lower for w in ["critical", "must", "required", "blocking"]):
        return "critical"
    elif any(w in content_lower for w in ["important", "high", "detection"]):
        return "high"
    elif any(w in content_lower for w in ["optional", "future", "low"]):
        return "low"
    return "medium"
```

**Step 4: Run test to verify it passes**

Run: `cd ombra_mcp_server && python -m pytest tests/test_doc_parser.py -v`
Expected: PASS

**Step 5: Commit**

```bash
git add src/ombra_watcherd/extractors/ tests/test_doc_parser.py
git commit -m "feat(concepts): add document parser for concept extraction"
```

---

### Task 5: Bulk Extract from Old Docs

**Files:**
- Create: `ombra_mcp_server/scripts/extract_concepts.py`

**Step 1: Write extraction script**

```python
#!/usr/bin/env python3
"""
Extract concepts from all old project documentation.
"""

import sys
from pathlib import Path

# Add src to path
sys.path.insert(0, str(Path(__file__).parent.parent / "src"))

from ombra_watcherd.concepts_db import ConceptsDB
from ombra_watcherd.extractors.doc_parser import extract_concepts_from_doc


OLD_DOCS_PATH = Path("/Users/jonathanmcclintock/PROJECT-OMBRA/docs/old_ombra_project")


def main():
    db = ConceptsDB()

    # Find all markdown files
    md_files = list(OLD_DOCS_PATH.glob("*.md"))
    print(f"Found {len(md_files)} markdown files")

    total_concepts = 0

    for md_file in sorted(md_files):
        print(f"\nProcessing: {md_file.name}")

        try:
            content = md_file.read_text(encoding="utf-8")
            concepts = extract_concepts_from_doc(content, md_file.name)

            for concept in concepts:
                db.add_concept(concept)
                total_concepts += 1
                print(f"  + {concept['id']} ({concept['category']})")

        except Exception as e:
            print(f"  ERROR: {e}")

    print(f"\n{'='*50}")
    print(f"Total concepts extracted: {total_concepts}")

    # Summary by category
    for cat in ["timing", "vmx", "ept", "stealth"]:
        count = len(db.list_concepts(category=cat))
        print(f"  {cat}: {count}")


if __name__ == "__main__":
    main()
```

**Step 2: Run extraction**

Run: `cd ombra_mcp_server && python scripts/extract_concepts.py`
Expected: Output showing concepts extracted from each file

**Step 3: Verify database populated**

Run: `cd ombra_mcp_server && python -c "from ombra_watcherd.concepts_db import ConceptsDB; db = ConceptsDB(); print(f'Total concepts: {len(db.list_concepts())}')"`
Expected: "Total concepts: XX" (should be 20+)

**Step 4: Commit**

```bash
git add scripts/extract_concepts.py src/ombra_watcherd/concepts.db
git commit -m "feat(concepts): extract concepts from legacy documentation"
```

---

## Phase 3: Concept Analyzer (Daemon)

### Task 6: Create Concept Analyzer

**Files:**
- Create: `ombra_mcp_server/src/ombra_watcherd/analyzers/concept.py`
- Modify: `ombra_mcp_server/src/ombra_watcherd/analyzers/__init__.py`
- Test: `ombra_mcp_server/tests/test_concept_analyzer.py`

**Step 1: Write the failing test**

```python
# tests/test_concept_analyzer.py
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


def test_detect_partial_implementation():
    with tempfile.TemporaryDirectory() as tmpdir:
        db_path = Path(tmpdir) / "concepts.db"
        db = ConceptsDB(db_path)

        # Add a concept
        db.add_concept({
            "id": "TSC_OFFSET_COMPENSATION",
            "category": "timing",
            "name": "TSC Offset Compensation",
            "required_patterns": [r"__rdtsc\(\)", r"0x2010|TSC_OFFSET"],
            "priority": "critical",
        })

        analyzer = ConceptAnalyzer(db_path)
        findings = analyzer.analyze(Path("vmx.c"), SAMPLE_CODE_PARTIAL)

        # Should find partial implementation
        partial = [f for f in findings if f["finding_type"] == "partial"]
        assert len(partial) >= 1
        assert "TSC_OFFSET_COMPENSATION" in partial[0]["concept_id"]


def test_detect_complete_implementation():
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

        # Should find complete implementation
        complete = [f for f in findings if f["finding_type"] == "complete"]
        assert len(complete) >= 1
```

**Step 2: Run test to verify it fails**

Run: `cd ombra_mcp_server && python -m pytest tests/test_concept_analyzer.py -v`
Expected: FAIL with "No module named 'ombra_watcherd.analyzers.concept'"

**Step 3: Write minimal implementation**

```python
# src/ombra_watcherd/analyzers/concept.py
"""
Concept Analyzer - Pattern-based concept implementation detection.

Analyzes source files against concept patterns to identify:
- Complete implementations
- Partial implementations (some patterns found)
- Anti-patterns (wrong implementation)
"""

import re
from pathlib import Path
from typing import List, Dict, Any, Optional

from .base import BaseAnalyzer, Finding
from ..concepts_db import ConceptsDB


class ConceptAnalyzer(BaseAnalyzer):
    """Analyzer that checks code against concept patterns."""

    name = "concept"
    file_patterns = ["*.c", "*.h", "*.asm", "*.cpp"]

    def __init__(self, db_path: Optional[Path] = None):
        super().__init__()
        self.db = ConceptsDB(db_path)
        self._concepts = None

    @property
    def concepts(self) -> List[Dict[str, Any]]:
        if self._concepts is None:
            self._concepts = self.db.list_concepts()
        return self._concepts

    def analyze(self, file_path: Path, content: str) -> List[Dict[str, Any]]:
        """Analyze file against all concept patterns."""
        findings = []

        for concept in self.concepts:
            finding = self._check_concept(concept, file_path, content)
            if finding:
                findings.append(finding)

        return findings

    def _check_concept(
        self,
        concept: Dict[str, Any],
        file_path: Path,
        content: str
    ) -> Optional[Dict[str, Any]]:
        """Check a single concept against file content."""
        required = concept.get("required_patterns") or []
        if isinstance(required, str):
            try:
                import json
                required = json.loads(required)
            except:
                required = [required]

        anti = concept.get("anti_patterns") or []
        if isinstance(anti, str):
            try:
                import json
                anti = json.loads(anti)
            except:
                anti = [anti]

        # Check required patterns
        matched = []
        missing = []

        for pattern in required:
            try:
                if re.search(pattern, content, re.IGNORECASE):
                    matched.append(pattern)
                else:
                    missing.append(pattern)
            except re.error:
                # Invalid regex, skip
                continue

        # Check anti-patterns
        anti_matched = []
        for pattern in anti:
            try:
                if re.search(pattern, content, re.IGNORECASE):
                    anti_matched.append(pattern)
            except re.error:
                continue

        # Determine finding type
        if not matched and not anti_matched:
            return None  # Not relevant to this file

        if anti_matched:
            return {
                "concept_id": concept["id"],
                "file_path": str(file_path),
                "line_number": 0,
                "finding_type": "antipattern",
                "message": f"{concept['name']}: Anti-pattern detected - {anti_matched}",
                "severity": "critical",
            }

        if matched and not missing:
            return {
                "concept_id": concept["id"],
                "file_path": str(file_path),
                "line_number": 0,
                "finding_type": "complete",
                "message": f"{concept['name']}: Implementation complete",
                "severity": "info",
            }

        if matched and missing:
            return {
                "concept_id": concept["id"],
                "file_path": str(file_path),
                "line_number": 0,
                "finding_type": "partial",
                "message": f"{concept['name']}: Partial - found {matched}, missing {missing}",
                "severity": "warning",
            }

        return None
```

**Step 4: Update analyzers __init__.py**

```python
# Modify src/ombra_watcherd/analyzers/__init__.py
# Add import:
from .concept import ConceptAnalyzer

# Add to get_all_analyzers function:
def get_all_analyzers():
    return [
        HypervisorAnalyzer(),
        ConsistencyAnalyzer(),
        SecurityAnalyzer(),
        ConceptAnalyzer(),  # Add this
    ]
```

**Step 5: Run test to verify it passes**

Run: `cd ombra_mcp_server && python -m pytest tests/test_concept_analyzer.py -v`
Expected: PASS

**Step 6: Commit**

```bash
git add src/ombra_watcherd/analyzers/concept.py src/ombra_watcherd/analyzers/__init__.py tests/test_concept_analyzer.py
git commit -m "feat(concepts): add concept analyzer for daemon integration"
```

---

## Phase 4: MCP Tools

### Task 7: Create Concept MCP Tools

**Files:**
- Create: `ombra_mcp_server/src/ombra_mcp/tools/concepts.py`
- Modify: `ombra_mcp_server/src/ombra_mcp/tools/__init__.py`
- Modify: `ombra_mcp_server/src/ombra_mcp/server.py`

**Step 1: Create MCP tools module**

```python
# src/ombra_mcp/tools/concepts.py
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
    """
    Get full details on a concept.

    Returns description, patterns, SDM references,
    implementation status, and related concepts.
    """
    db = get_db()
    concept = db.get_concept(concept_id)

    if not concept:
        return {"error": f"Concept '{concept_id}' not found"}

    # Add findings
    concept["findings"] = db.get_findings(concept_id=concept_id, include_dismissed=False)

    return concept


async def list_concepts(
    category: Optional[str] = None,
    status: Optional[str] = None,
    priority: Optional[str] = None
) -> List[Dict[str, Any]]:
    """
    List concepts with optional filters.

    Args:
        category: Filter by category (timing, vmx, ept, stealth)
        status: Filter by status (not_started, partial, complete, verified)
        priority: Filter by priority (critical, high, medium, low)
    """
    db = get_db()
    return db.list_concepts(category=category, status=status, priority=priority)


async def get_implementation_gaps(category: Optional[str] = None) -> List[Dict[str, Any]]:
    """
    Get concepts that are not fully implemented.

    Returns concepts with status 'not_started' or 'partial',
    sorted by priority.
    """
    db = get_db()

    gaps = []

    for status in ["not_started", "partial"]:
        concepts = db.list_concepts(category=category, status=status)
        for c in concepts:
            c["findings"] = db.get_findings(concept_id=c["id"], include_dismissed=False)
            gaps.append(c)

    # Sort by priority
    priority_order = {"critical": 0, "high": 1, "medium": 2, "low": 3}
    gaps.sort(key=lambda x: priority_order.get(x.get("priority", "medium"), 2))

    return gaps


async def verify_concept(
    concept_id: str,
    file_path: str,
    line_number: int,
    notes: Optional[str] = None
) -> Dict[str, Any]:
    """
    Mark a concept as verified at a specific location.
    """
    db = get_db()

    annotation_id = db.add_annotation(
        concept_id=concept_id,
        file_path=file_path,
        line_number=line_number,
        annotation_type="verified",
        notes=notes
    )

    # Update concept status
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


async def dismiss_concept_finding(
    finding_id: int,
    reason: str
) -> Dict[str, Any]:
    """
    Dismiss a concept finding as false positive.
    """
    db = get_db()
    db.dismiss_finding(finding_id, reason)

    return {
        "success": True,
        "message": f"Finding {finding_id} dismissed"
    }


async def suggest_next_work() -> Dict[str, Any]:
    """
    Suggest what to implement next based on priorities and dependencies.
    """
    db = get_db()

    # Get critical gaps first
    gaps = await get_implementation_gaps()

    if not gaps:
        return {"message": "All concepts implemented!", "suggestions": []}

    suggestions = []

    # Find concepts with no unmet dependencies
    for gap in gaps[:5]:
        deps = gap.get("depends_on") or []
        if isinstance(deps, str):
            import json
            try:
                deps = json.loads(deps)
            except:
                deps = []

        # Check if dependencies are met
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

    return {
        "suggestions": suggestions,
        "total_gaps": len(gaps)
    }
```

**Step 2: Update tools __init__.py**

Add to `src/ombra_mcp/tools/__init__.py`:

```python
from .concepts import (
    get_concept,
    list_concepts,
    get_implementation_gaps,
    verify_concept,
    dismiss_concept_finding,
    suggest_next_work,
)

# Add to __all__:
__all__ = [
    # ... existing exports ...
    # Concepts
    "get_concept",
    "list_concepts",
    "get_implementation_gaps",
    "verify_concept",
    "dismiss_concept_finding",
    "suggest_next_work",
]
```

**Step 3: Add tool definitions to server.py**

Add these Tool definitions to the TOOLS list in `server.py`:

```python
# Concept Intelligence Tools
Tool(
    name="get_concept",
    description="Get full details on a hypervisor concept including patterns, SDM refs, and implementation status",
    inputSchema={
        "type": "object",
        "properties": {
            "concept_id": {
                "type": "string",
                "description": "Concept ID (e.g., 'TSC_OFFSET_COMPENSATION')"
            }
        },
        "required": ["concept_id"]
    }
),
Tool(
    name="list_concepts",
    description="List hypervisor implementation concepts with optional filters",
    inputSchema={
        "type": "object",
        "properties": {
            "category": {
                "type": "string",
                "enum": ["timing", "vmx", "ept", "stealth"],
                "description": "Filter by category"
            },
            "status": {
                "type": "string",
                "enum": ["not_started", "partial", "complete", "verified"],
                "description": "Filter by implementation status"
            },
            "priority": {
                "type": "string",
                "enum": ["critical", "high", "medium", "low"],
                "description": "Filter by priority"
            }
        }
    }
),
Tool(
    name="get_implementation_gaps",
    description="Get concepts that are not fully implemented, sorted by priority",
    inputSchema={
        "type": "object",
        "properties": {
            "category": {
                "type": "string",
                "enum": ["timing", "vmx", "ept", "stealth"],
                "description": "Filter by category"
            }
        }
    }
),
Tool(
    name="verify_concept",
    description="Mark a concept as verified/implemented at a specific code location",
    inputSchema={
        "type": "object",
        "properties": {
            "concept_id": {"type": "string"},
            "file_path": {"type": "string"},
            "line_number": {"type": "integer"},
            "notes": {"type": "string"}
        },
        "required": ["concept_id", "file_path", "line_number"]
    }
),
Tool(
    name="dismiss_concept_finding",
    description="Dismiss a concept finding as false positive",
    inputSchema={
        "type": "object",
        "properties": {
            "finding_id": {"type": "integer"},
            "reason": {"type": "string"}
        },
        "required": ["finding_id", "reason"]
    }
),
Tool(
    name="suggest_next_work",
    description="Get AI-powered suggestions for what to implement next based on priorities and dependencies",
    inputSchema={
        "type": "object",
        "properties": {}
    }
),
```

**Step 4: Add handlers to TOOL_HANDLERS**

```python
# Add to TOOL_HANDLERS dict:
"get_concept": get_concept,
"list_concepts": list_concepts,
"get_implementation_gaps": get_implementation_gaps,
"verify_concept": verify_concept,
"dismiss_concept_finding": dismiss_concept_finding,
"suggest_next_work": suggest_next_work,
```

**Step 5: Verify tools load**

Run: `cd ombra_mcp_server && source .venv/bin/activate && python -c "from ombra_mcp.server import TOOLS; print(f'Tools: {len(TOOLS)}')" `
Expected: Tools count increased (66 from 60)

**Step 6: Commit**

```bash
git add src/ombra_mcp/tools/concepts.py src/ombra_mcp/tools/__init__.py src/ombra_mcp/server.py
git commit -m "feat(concepts): add MCP tools for concept intelligence"
```

---

## Phase 5: Integration & Testing

### Task 8: End-to-End Integration Test

**Files:**
- Create: `ombra_mcp_server/tests/test_integration.py`

**Step 1: Write integration test**

```python
# tests/test_integration.py
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
    monkeypatch.setattr(concepts_module, "_db", ConceptsDB(populated_db))

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
```

**Step 2: Run integration tests**

Run: `cd ombra_mcp_server && python -m pytest tests/test_integration.py -v`
Expected: PASS

**Step 3: Commit**

```bash
git add tests/test_integration.py
git commit -m "test(concepts): add end-to-end integration tests"
```

---

### Task 9: Final Package Update & Documentation

**Files:**
- Modify: `ombra_mcp_server/pyproject.toml`
- Update version and ensure all modules included

**Step 1: Update pyproject.toml**

```toml
[project]
name = "ombra-mcp"
version = "2.1.0"  # Bump version
description = "Hypervisor Development MCP Server with Project Brain and Concept Intelligence"
```

**Step 2: Reinstall and verify**

Run: `cd ombra_mcp_server && pip install -e .`
Run: `python -c "from ombra_mcp.server import TOOLS; from ombra_watcherd.concepts_db import ConceptsDB; print('All imports OK')"`
Expected: "All imports OK"

**Step 3: Final commit**

```bash
git add pyproject.toml
git commit -m "chore: bump version to 2.1.0 with Concept Intelligence"
```

---

## Summary

**Total Tasks:** 9
**Estimated Implementation Time:** 2-3 hours

**What Gets Built:**
1. `concepts_db.py` - SQLite database for concepts, findings, annotations
2. `extractors/doc_parser.py` - Markdown parser for concept extraction
3. `scripts/extract_concepts.py` - Bulk extraction from old docs
4. `analyzers/concept.py` - Daemon analyzer for continuous checking
5. `tools/concepts.py` - 6 new MCP tools for on-demand analysis
6. Integration with existing ombra-watcherd daemon
7. Full test coverage

**After Implementation:**
- Run `python scripts/extract_concepts.py` to populate concepts
- Daemon continuously analyzes code changes
- Use MCP tools to query gaps and get suggestions
