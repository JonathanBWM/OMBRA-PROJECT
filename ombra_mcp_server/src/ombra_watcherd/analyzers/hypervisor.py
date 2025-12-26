"""
Hypervisor Analyzer - The core value: understands VMX/VMCS/EPT semantics

Checks for:
- Unhandled exit reasons
- VMCS field usage errors
- Missing stealth measures
- EPT configuration issues
- Control bit mismatches
"""

import re
from pathlib import Path
from typing import List, Dict, Any, Set, Optional, Tuple

from .base import BaseAnalyzer


# All Intel VMX exit reasons (0-69 as of SDM 2024)
EXIT_REASONS = {
    0: "EXCEPTION_NMI",
    1: "EXTERNAL_INTERRUPT",
    2: "TRIPLE_FAULT",
    3: "INIT",
    4: "SIPI",
    5: "IO_SMI",
    6: "OTHER_SMI",
    7: "INTERRUPT_WINDOW",
    8: "NMI_WINDOW",
    9: "TASK_SWITCH",
    10: "CPUID",
    11: "GETSEC",
    12: "HLT",
    13: "INVD",
    14: "INVLPG",
    15: "RDPMC",
    16: "RDTSC",
    17: "RSM",
    18: "VMCALL",
    19: "VMCLEAR",
    20: "VMLAUNCH",
    21: "VMPTRLD",
    22: "VMPTRST",
    23: "VMREAD",
    24: "VMRESUME",
    25: "VMWRITE",
    26: "VMXOFF",
    27: "VMXON",
    28: "CR_ACCESS",
    29: "MOV_DR",
    30: "IO_INSTRUCTION",
    31: "RDMSR",
    32: "WRMSR",
    33: "ENTRY_FAIL_GUEST_STATE",
    34: "ENTRY_FAIL_MSR_LOAD",
    36: "MWAIT",
    37: "MONITOR_TRAP_FLAG",
    39: "MONITOR",
    40: "PAUSE",
    41: "ENTRY_FAIL_MACHINE_CHECK",
    43: "TPR_BELOW_THRESHOLD",
    44: "APIC_ACCESS",
    45: "VIRTUALIZED_EOI",
    46: "GDTR_IDTR_ACCESS",
    47: "LDTR_TR_ACCESS",
    48: "EPT_VIOLATION",
    49: "EPT_MISCONFIG",
    50: "INVEPT",
    51: "RDTSCP",
    52: "PREEMPTION_TIMER",
    53: "INVVPID",
    54: "WBINVD",
    55: "XSETBV",
    56: "APIC_WRITE",
    57: "RDRAND",
    58: "INVPCID",
    59: "VMFUNC",
    60: "ENCLS",
    61: "RDSEED",
    62: "PML_FULL",
    63: "XSAVES",
    64: "XRSTORS",
    66: "SPP_RELATED",
    67: "UMWAIT",
    68: "TPAUSE",
    74: "BUS_LOCK",
    75: "NOTIFY",
}

# Exit reasons that REQUIRE stealth handling
STEALTH_CRITICAL_EXITS = {
    10,  # CPUID - must hide hypervisor presence
    16,  # RDTSC - timing attacks
    31,  # RDMSR - VMX MSRs
    32,  # WRMSR - VMX MSRs
    51,  # RDTSCP - timing attacks
}

# VMX MSRs that must be hidden
VMX_MSRS = {
    0x480: "IA32_VMX_BASIC",
    0x481: "IA32_VMX_PINBASED_CTLS",
    0x482: "IA32_VMX_PROCBASED_CTLS",
    0x483: "IA32_VMX_EXIT_CTLS",
    0x484: "IA32_VMX_ENTRY_CTLS",
    0x485: "IA32_VMX_MISC",
    0x486: "IA32_VMX_CR0_FIXED0",
    0x487: "IA32_VMX_CR0_FIXED1",
    0x488: "IA32_VMX_CR4_FIXED0",
    0x489: "IA32_VMX_CR4_FIXED1",
    0x48A: "IA32_VMX_VMCS_ENUM",
    0x48B: "IA32_VMX_PROCBASED_CTLS2",
    0x48C: "IA32_VMX_EPT_VPID_CAP",
    0x48D: "IA32_VMX_TRUE_PINBASED_CTLS",
    0x48E: "IA32_VMX_TRUE_PROCBASED_CTLS",
    0x48F: "IA32_VMX_TRUE_EXIT_CTLS",
    0x490: "IA32_VMX_TRUE_ENTRY_CTLS",
    0x491: "IA32_VMX_VMFUNC",
}

# Patterns for detecting various constructs
PATTERNS = {
    "vmread": re.compile(r'__vmx_vmread\s*\(\s*([^,)]+)'),
    "vmwrite": re.compile(r'__vmx_vmwrite\s*\(\s*([^,)]+)'),
    "exit_switch": re.compile(r'switch\s*\(\s*(?:exit_reason|reason|exit_code)'),
    "exit_case": re.compile(r'case\s+(\d+)\s*:|case\s+EXIT_REASON_(\w+)'),
    "tsc_offset": re.compile(r'tsc_offset|TSC_OFFSET|timing.*offset', re.IGNORECASE),
    "cpuid_leaf": re.compile(r'leaf\s*==\s*0x40|leaf\s*>=\s*0x40000000'),
    "hypervisor_bit": re.compile(r'ECX.*&.*\(1\s*<<\s*31\)|bit\s*31|hypervisor.*bit', re.IGNORECASE),
    "inject_gp": re.compile(r'inject.*(?:gp|exception)|EXCEPTION_GP|#GP', re.IGNORECASE),
    "msr_check": re.compile(r'msr\s*>=\s*0x480|msr\s*<=\s*0x491|VMX.*MSR', re.IGNORECASE),
    "invept": re.compile(r'__invept|INVEPT|invept_'),
    "stub_body": re.compile(r'^\s*(?://\s*TODO|return;?\s*$|break;?\s*$|\{\s*\})'),
    "ept_rwx": re.compile(r'\.read\s*=.*\.write\s*=.*\.execute\s*=|RWX|read.*write.*execute', re.IGNORECASE),
}


class HypervisorAnalyzer(BaseAnalyzer):
    """
    Analyzes hypervisor code for correctness and stealth issues.
    """

    EXTENSIONS = {".c", ".h", ".cpp", ".hpp"}

    @property
    def name(self) -> str:
        return "hypervisor"

    def should_analyze(self, path: Path) -> bool:
        """Only analyze files in hypervisor directory or with VMX-related names."""
        if path.suffix not in self.EXTENSIONS:
            return False

        path_str = str(path).lower()
        relevant_dirs = ["hypervisor", "vmx", "vmcs", "ept", "handler"]
        relevant_names = ["vmx", "vmcs", "ept", "exit", "vmexit", "handler", "cpuid", "msr"]

        return (
            any(d in path_str for d in relevant_dirs) or
            any(n in path.stem.lower() for n in relevant_names)
        )

    def analyze(self, path: Path) -> List[Dict[str, Any]]:
        """Run all hypervisor checks on a file."""
        findings = []
        content = self.read_file(path)
        lines = self.read_lines(path)

        if not content:
            return findings

        # Run each check
        findings.extend(self._check_exit_handlers(path, content, lines))
        findings.extend(self._check_vmcs_usage(path, content, lines))
        findings.extend(self._check_stealth_cpuid(path, content, lines))
        findings.extend(self._check_stealth_msr(path, content, lines))
        findings.extend(self._check_stealth_timing(path, content, lines))
        findings.extend(self._check_ept_issues(path, content, lines))
        findings.extend(self._check_stub_handlers(path, content, lines))

        return findings

    def _check_exit_handlers(
        self,
        path: Path,
        content: str,
        lines: List[str]
    ) -> List[Dict[str, Any]]:
        """Check for exit handler coverage and quality."""
        findings = []

        # Look for exit dispatch switch statement
        if not PATTERNS["exit_switch"].search(content):
            return findings

        # Find all handled exit reasons
        handled = set()
        for match in PATTERNS["exit_case"].finditer(content):
            if match.group(1):
                handled.add(int(match.group(1)))
            elif match.group(2):
                # Named constant - try to map it
                name = match.group(2)
                for reason, rname in EXIT_REASONS.items():
                    if name in rname or rname in name:
                        handled.add(reason)
                        break

        # Check for missing critical handlers
        for reason, name in EXIT_REASONS.items():
            if reason not in handled and reason in STEALTH_CRITICAL_EXITS:
                findings.append(self.make_finding(
                    severity="warning",
                    check_id="missing_stealth_exit",
                    message=f"Stealth-critical exit reason {reason} ({name}) not explicitly handled",
                    suggested_fix=f"Add handler for exit reason {reason} with stealth measures"
                ))

        # Track handled exits in database
        for reason in handled:
            name = EXIT_REASONS.get(reason, f"UNKNOWN_{reason}")
            self.db.upsert_exit_handler(
                reason=reason,
                name=name,
                status="handled",
                file=str(path)
            )

        return findings

    def _check_vmcs_usage(
        self,
        path: Path,
        content: str,
        lines: List[str]
    ) -> List[Dict[str, Any]]:
        """Track VMCS field reads/writes and check for mismatches."""
        findings = []

        # Find vmread calls
        for i, line in enumerate(lines, 1):
            for match in PATTERNS["vmread"].finditer(line):
                field = match.group(1).strip()
                self.db.update_vmcs_usage(
                    encoding=field,
                    name=field,
                    is_read=True,
                    location=f"{path}:{i}"
                )

            for match in PATTERNS["vmwrite"].finditer(line):
                field = match.group(1).strip()
                self.db.update_vmcs_usage(
                    encoding=field,
                    name=field,
                    is_written=True,
                    location=f"{path}:{i}"
                )

        return findings

    def _check_stealth_cpuid(
        self,
        path: Path,
        content: str,
        lines: List[str]
    ) -> List[Dict[str, Any]]:
        """Check CPUID handler for proper stealth."""
        findings = []

        # Only check files that look like CPUID handlers
        if "cpuid" not in path.stem.lower() and "cpuid" not in content.lower():
            return findings

        # Check for hypervisor bit handling
        if not PATTERNS["hypervisor_bit"].search(content):
            findings.append(self.make_finding(
                severity="warning",
                check_id="cpuid_no_hypervisor_bit",
                message="CPUID handler may not clear hypervisor present bit (ECX[31])",
                suggested_fix="Clear bit 31 of ECX when leaf=1 to hide hypervisor"
            ))

        # Check for hypervisor leaves handling
        if not PATTERNS["cpuid_leaf"].search(content):
            findings.append(self.make_finding(
                severity="warning",
                check_id="cpuid_no_hv_leaves",
                message="CPUID handler may not handle hypervisor leaves (0x40000000+)",
                suggested_fix="Return zeros for all leaves >= 0x40000000"
            ))

        return findings

    def _check_stealth_msr(
        self,
        path: Path,
        content: str,
        lines: List[str]
    ) -> List[Dict[str, Any]]:
        """Check MSR handler for proper stealth."""
        findings = []

        # Only check files that look like MSR handlers
        if "msr" not in path.stem.lower() and "rdmsr" not in content.lower():
            return findings

        # Check for VMX MSR handling
        if not PATTERNS["msr_check"].search(content):
            findings.append(self.make_finding(
                severity="warning",
                check_id="msr_no_vmx_check",
                message="MSR handler may not check for VMX MSRs (0x480-0x491)",
                suggested_fix="Inject #GP for reads of VMX MSRs"
            ))

        # Check for #GP injection
        if PATTERNS["msr_check"].search(content) and not PATTERNS["inject_gp"].search(content):
            findings.append(self.make_finding(
                severity="warning",
                check_id="msr_no_gp_inject",
                message="MSR handler checks VMX MSRs but may not inject #GP",
                suggested_fix="Inject #GP(0) when guest reads VMX MSRs"
            ))

        return findings

    def _check_stealth_timing(
        self,
        path: Path,
        content: str,
        lines: List[str]
    ) -> List[Dict[str, Any]]:
        """Check for timing compensation in handlers."""
        findings = []

        # Check RDTSC handler
        if "rdtsc" in path.stem.lower() or "rdtsc" in content.lower():
            if not PATTERNS["tsc_offset"].search(content):
                findings.append(self.make_finding(
                    severity="warning",
                    check_id="rdtsc_no_offset",
                    message="RDTSC handler may not compensate for VM-exit timing",
                    suggested_fix="Subtract accumulated exit overhead from TSC value"
                ))

        # Check CPUID handler for timing
        if "cpuid" in path.stem.lower():
            if not PATTERNS["tsc_offset"].search(content):
                findings.append(self.make_finding(
                    severity="info",
                    check_id="cpuid_no_timing",
                    message="CPUID handler doesn't track timing overhead",
                    suggested_fix="Track CPUID exit cycles for TSC compensation"
                ))

        return findings

    def _check_ept_issues(
        self,
        path: Path,
        content: str,
        lines: List[str]
    ) -> List[Dict[str, Any]]:
        """Check EPT code for common issues."""
        findings = []

        if "ept" not in path.stem.lower() and "ept" not in content.lower():
            return findings

        # Check for INVEPT usage after EPT modifications
        if "ept" in content.lower() and "write" in content.lower():
            if not PATTERNS["invept"].search(content):
                findings.append(self.make_finding(
                    severity="warning",
                    check_id="ept_no_invept",
                    message="EPT modifications detected but no INVEPT call found",
                    suggested_fix="Call INVEPT after modifying EPT structures"
                ))

        # Check for RWX pages (should be avoided for hooks)
        for i, line in enumerate(lines, 1):
            if PATTERNS["ept_rwx"].search(line):
                findings.append(self.make_finding(
                    severity="info",
                    check_id="ept_rwx_page",
                    message="EPT page with RWX permissions - consider execute-only for hooks",
                    line=i
                ))

        return findings

    def _check_stub_handlers(
        self,
        path: Path,
        content: str,
        lines: List[str]
    ) -> List[Dict[str, Any]]:
        """Detect stub/incomplete handler implementations."""
        findings = []

        # Look for case statements followed by minimal code
        in_case = False
        case_line = 0
        case_reason = ""
        brace_depth = 0

        for i, line in enumerate(lines, 1):
            # Track case statements
            case_match = PATTERNS["exit_case"].search(line)
            if case_match:
                in_case = True
                case_line = i
                case_reason = case_match.group(1) or case_match.group(2)
                brace_depth = 0
                continue

            if in_case:
                # Track braces
                brace_depth += line.count('{') - line.count('}')

                # Check for stub patterns
                if PATTERNS["stub_body"].search(line):
                    if case_reason:
                        findings.append(self.make_finding(
                            severity="info",
                            check_id="stub_handler",
                            message=f"Exit handler for reason {case_reason} appears to be a stub",
                            line=case_line,
                            suggested_fix=f"Implement handler for exit reason {case_reason}"
                        ))
                    in_case = False

                # End of case
                if "break" in line or "return" in line or brace_depth < 0:
                    in_case = False

        return findings
