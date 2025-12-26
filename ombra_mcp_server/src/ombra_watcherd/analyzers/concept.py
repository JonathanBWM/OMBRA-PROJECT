"""
Concept Analyzer - Pattern-based concept implementation detection.

Analyzes source files against concept patterns to identify:
- Complete implementations (all required patterns found)
- Partial implementations (some patterns found, others missing)
- Anti-patterns (wrong implementation approaches)
"""

import re
from pathlib import Path
from typing import List, Dict, Any, Optional
import json

from .base import BaseAnalyzer
from ..concepts_db import ConceptsDB


class ConceptAnalyzer(BaseAnalyzer):
    """Analyzer that checks code against concept patterns."""

    EXTENSIONS = {".c", ".h", ".cpp", ".hpp", ".asm", ".S"}

    def __init__(self, db_path: Optional[Path] = None):
        """
        Initialize ConceptAnalyzer.

        Unlike other analyzers, this works with ConceptsDB instead of ProjectBrainDB.
        """
        # Don't call super().__init__ since we don't use ProjectBrainDB
        self.concepts_db = ConceptsDB(db_path)
        self._concepts_cache = None

    @property
    def name(self) -> str:
        return "concept"

    @property
    def concepts(self) -> List[Dict[str, Any]]:
        """Lazy-load concepts from database."""
        if self._concepts_cache is None:
            self._concepts_cache = self.concepts_db.list_concepts()
        return self._concepts_cache

    def should_analyze(self, path: Path) -> bool:
        """Check if this analyzer should process the given file."""
        return path.suffix in self.EXTENSIONS

    def analyze(self, path: Path, content: Optional[str] = None) -> List[Dict[str, Any]]:
        """
        Analyze file against all concept patterns.

        Args:
            path: File path being analyzed
            content: File content (if None, will be read from path)

        Returns:
            List of findings (complete, partial, or antipattern detections)
        """
        if content is None:
            content = self.read_file(path)
            if content is None:
                return []

        findings = []

        for concept in self.concepts:
            finding = self._check_concept(concept, path, content)
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
        # Extract and normalize pattern lists
        required = self._normalize_patterns(concept.get("required_patterns"))
        anti = self._normalize_patterns(concept.get("anti_patterns"))

        # Check required patterns
        matched = []
        missing = []

        for pattern in required:
            try:
                if re.search(pattern, content, re.IGNORECASE | re.MULTILINE):
                    matched.append(pattern)
                else:
                    missing.append(pattern)
            except re.error:
                # Invalid regex - skip this pattern
                continue

        # Check anti-patterns
        anti_matched = []
        for pattern in anti:
            try:
                if re.search(pattern, content, re.IGNORECASE | re.MULTILINE):
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

    def _normalize_patterns(self, patterns: Any) -> List[str]:
        """
        Normalize pattern field from database to list of strings.

        Handles:
        - None/empty
        - JSON-encoded list
        - Single string
        - Already a list
        """
        if not patterns:
            return []

        # If it's a string, try to parse as JSON first
        if isinstance(patterns, str):
            try:
                parsed = json.loads(patterns)
                if isinstance(parsed, list):
                    return parsed
                else:
                    return [patterns]
            except (json.JSONDecodeError, TypeError):
                # Not JSON, treat as single pattern
                return [patterns]

        # Already a list
        if isinstance(patterns, list):
            return patterns

        # Unknown type
        return []
