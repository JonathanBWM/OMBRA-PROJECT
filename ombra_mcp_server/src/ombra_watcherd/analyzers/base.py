"""
Base Analyzer - Abstract base class for all analyzers
"""

from abc import ABC, abstractmethod
from pathlib import Path
from typing import List, Dict, Any, Optional, Set

from ..database import ProjectBrainDB


class BaseAnalyzer(ABC):
    """
    Base class for code analyzers.

    Each analyzer should:
    1. Implement should_analyze() to filter relevant files
    2. Implement analyze() to produce findings
    """

    # Extensions this analyzer handles
    EXTENSIONS: Set[str] = set()

    def __init__(self, db: ProjectBrainDB):
        self.db = db

    @property
    @abstractmethod
    def name(self) -> str:
        """Analyzer name for logging and findings."""
        pass

    def should_analyze(self, path: Path) -> bool:
        """
        Check if this analyzer should process the given file.
        Override for more complex logic.
        """
        return path.suffix in self.EXTENSIONS

    @abstractmethod
    def analyze(self, path: Path) -> List[Dict[str, Any]]:
        """
        Analyze a file and return findings.

        Each finding should be a dict with:
        - severity: "critical", "warning", or "info"
        - type: analyzer name
        - check_id: specific check that fired (e.g., "unhandled_exit")
        - message: human-readable description
        - line: (optional) line number
        - suggested_fix: (optional) how to fix it
        """
        pass

    def read_file(self, path: Path) -> Optional[str]:
        """Safely read a file's contents."""
        try:
            return path.read_text(encoding='utf-8', errors='replace')
        except Exception:
            return None

    def read_lines(self, path: Path) -> List[str]:
        """Read file as lines."""
        content = self.read_file(path)
        if content:
            return content.splitlines()
        return []

    def make_finding(
        self,
        severity: str,
        check_id: str,
        message: str,
        line: Optional[int] = None,
        suggested_fix: Optional[str] = None
    ) -> Dict[str, Any]:
        """Helper to create a properly formatted finding."""
        finding = {
            "severity": severity,
            "type": self.name,
            "check_id": check_id,
            "message": message,
        }
        if line is not None:
            finding["line"] = line
        if suggested_fix:
            finding["suggested_fix"] = suggested_fix
        return finding
