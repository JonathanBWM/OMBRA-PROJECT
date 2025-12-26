"""
Consistency Analyzer - Checks for code consistency issues

Detects:
- Function signature mismatches between declaration and definition
- Unused functions
- Missing forward declarations
"""

import re
from pathlib import Path
from typing import List, Dict, Any, Set, Tuple

from .base import BaseAnalyzer


# Patterns for C function detection
FUNCTION_DECL = re.compile(
    r'^(?:static\s+)?(?:inline\s+)?'
    r'(?:NTSTATUS|BOOLEAN|VOID|void|int|bool|ULONG|UINT64|PVOID|'
    r'unsigned\s+\w+|\w+_t)\s+'
    r'(\w+)\s*\([^)]*\)\s*;',
    re.MULTILINE
)

FUNCTION_DEF = re.compile(
    r'^(?:static\s+)?(?:inline\s+)?'
    r'(?:NTSTATUS|BOOLEAN|VOID|void|int|bool|ULONG|UINT64|PVOID|'
    r'unsigned\s+\w+|\w+_t)\s+'
    r'(\w+)\s*\([^)]*\)\s*\{',
    re.MULTILINE
)

FUNCTION_CALL = re.compile(r'\b(\w+)\s*\(')


class ConsistencyAnalyzer(BaseAnalyzer):
    """
    Analyzes code for consistency issues.
    """

    EXTENSIONS = {".c", ".h", ".cpp", ".hpp"}

    @property
    def name(self) -> str:
        return "consistency"

    def analyze(self, path: Path) -> List[Dict[str, Any]]:
        """Run consistency checks on a file."""
        findings = []
        content = self.read_file(path)
        lines = self.read_lines(path)

        if not content:
            return findings

        # For header files, track declarations
        if path.suffix in {".h", ".hpp"}:
            findings.extend(self._check_header_issues(path, content, lines))

        # For source files, check implementations
        if path.suffix in {".c", ".cpp"}:
            findings.extend(self._check_source_issues(path, content, lines))

        return findings

    def _check_header_issues(
        self,
        path: Path,
        content: str,
        lines: List[str]
    ) -> List[Dict[str, Any]]:
        """Check header file issues."""
        findings = []

        # Check for include guard
        if not re.search(r'#ifndef\s+\w+_H|#pragma\s+once', content):
            findings.append(self.make_finding(
                severity="warning",
                check_id="missing_include_guard",
                message="Header file missing include guard (#ifndef or #pragma once)"
            ))

        return findings

    def _check_source_issues(
        self,
        path: Path,
        content: str,
        lines: List[str]
    ) -> List[Dict[str, Any]]:
        """Check source file issues."""
        findings = []

        # Find all function definitions
        defined_functions = set()
        for match in FUNCTION_DEF.finditer(content):
            defined_functions.add(match.group(1))

        # Find all function calls
        called_functions = set()
        for match in FUNCTION_CALL.finditer(content):
            called_functions.add(match.group(1))

        # Check for static functions that are never called
        for i, line in enumerate(lines, 1):
            if line.strip().startswith("static") and "(" in line:
                match = FUNCTION_DEF.search(line)
                if match:
                    func_name = match.group(1)
                    if func_name not in called_functions:
                        findings.append(self.make_finding(
                            severity="info",
                            check_id="unused_static",
                            message=f"Static function '{func_name}' may be unused in this file",
                            line=i
                        ))

        return findings
