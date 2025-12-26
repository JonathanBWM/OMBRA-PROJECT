"""
Security Analyzer - Checks for signature exposure and security issues

Detects:
- Known hypervisor signatures
- Hardcoded magic values that could be detected
- Debug output left in code
- Pool tags that could be scanned
"""

import re
from pathlib import Path
from typing import List, Dict, Any, Set

from .base import BaseAnalyzer


# Known hypervisor signatures to avoid
KNOWN_SIGNATURES = [
    # Public hypervisor names
    r"HyperPlatform",
    r"SimpleSvm",
    r"hvpp",
    r"gbhv",
    r"Voyager",
    r"BluePill",
    r"VirtualBox",
    r"VMware",
    r"QEMU",
    r"KVM",

    # Common pool tags
    r"HvPl",
    r"SSvm",
    r"VMXr",
    r"VmHv",
    r"OMBR",  # Our own if not randomized

    # Detection-prone strings
    r"hypervisor",
    r"vmx.root",
    r"vmxroot",
]

# Debug patterns that should be removed for release
DEBUG_PATTERNS = [
    r"DbgPrint\s*\(",
    r"KdPrint\s*\(",
    r"KdPrintEx\s*\(",
    r"OutputDebugString",
    r"printf\s*\(",
    r"__debugbreak\s*\(",
    r"DebugBreak\s*\(",
    r"#\s*pragma\s+message",
]

# Hardcoded values that might be signatures
MAGIC_VALUES = [
    (r"0x(?:DEAD|BEEF|CAFE|BABE|1337|F00D)", "Common magic value - easily detected"),
    (r"'VMXR'|'HVSR'|'ROOT'", "ASCII pool tag - could be scanned"),
]


class SecurityAnalyzer(BaseAnalyzer):
    """
    Analyzes code for security and signature exposure issues.
    """

    EXTENSIONS = {".c", ".h", ".cpp", ".hpp", ".asm", ".s"}

    @property
    def name(self) -> str:
        return "security"

    def analyze(self, path: Path) -> List[Dict[str, Any]]:
        """Run security checks on a file."""
        findings = []
        content = self.read_file(path)
        lines = self.read_lines(path)

        if not content:
            return findings

        findings.extend(self._check_signatures(path, content, lines))
        findings.extend(self._check_debug_output(path, content, lines))
        findings.extend(self._check_magic_values(path, content, lines))

        return findings

    def _check_signatures(
        self,
        path: Path,
        content: str,
        lines: List[str]
    ) -> List[Dict[str, Any]]:
        """Check for known hypervisor signatures."""
        findings = []

        for i, line in enumerate(lines, 1):
            # Skip comments
            stripped = line.strip()
            if stripped.startswith("//") or stripped.startswith("*"):
                continue

            for sig in KNOWN_SIGNATURES:
                if re.search(sig, line, re.IGNORECASE):
                    # Don't flag if it's in a comment or string comparison
                    if "strcmp" in line or "compare" in line.lower():
                        continue

                    findings.append(self.make_finding(
                        severity="warning",
                        check_id="signature_exposure",
                        message=f"Potential signature exposure: '{sig}' pattern found",
                        line=i,
                        suggested_fix="Remove or obfuscate this string to avoid detection"
                    ))

        return findings

    def _check_debug_output(
        self,
        path: Path,
        content: str,
        lines: List[str]
    ) -> List[Dict[str, Any]]:
        """Check for debug output that should be removed."""
        findings = []

        for i, line in enumerate(lines, 1):
            # Skip if in DEBUG block
            for pattern in DEBUG_PATTERNS:
                if re.search(pattern, line):
                    # Check if it's conditionally compiled
                    context_start = max(0, i - 10)
                    context = "\n".join(lines[context_start:i])

                    if "#ifdef DEBUG" in context or "#if DBG" in context:
                        continue

                    findings.append(self.make_finding(
                        severity="info",
                        check_id="debug_in_release",
                        message=f"Debug output found - ensure removed in release",
                        line=i,
                        suggested_fix="Wrap in #ifdef DEBUG or remove"
                    ))

        return findings

    def _check_magic_values(
        self,
        path: Path,
        content: str,
        lines: List[str]
    ) -> List[Dict[str, Any]]:
        """Check for easily-detected magic values."""
        findings = []

        for i, line in enumerate(lines, 1):
            for pattern, description in MAGIC_VALUES:
                if re.search(pattern, line, re.IGNORECASE):
                    findings.append(self.make_finding(
                        severity="info",
                        check_id="magic_value",
                        message=description,
                        line=i,
                        suggested_fix="Use randomized or less obvious values"
                    ))

        return findings
