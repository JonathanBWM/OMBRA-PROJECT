"""
Stealth Tools - Hypervisor detection evasion and analysis
"""

import json
from pathlib import Path

DATA_DIR = Path(__file__).parent.parent / "data"


DETECTION_VECTORS = {
    "timing_based": {
        "rdtsc_delta": {
            "description": "CPUID timing measurement using RDTSC",
            "technique": "Measure cycles for CPUID execution, VM-exits add ~1000+ cycles",
            "threshold": 500,
            "mitigation": "TSC offsetting, RDTSC exit handling with timing compensation"
        },
        "rdtscp_delta": {
            "description": "RDTSCP timing with processor ID",
            "technique": "Same as RDTSC but includes TSC_AUX check",
            "threshold": 500,
            "mitigation": "Virtualize IA32_TSC_AUX MSR, offset compensation"
        },
        "vmexit_timing": {
            "description": "Measure any instruction that causes VM-exit",
            "technique": "CPUID, RDMSR on VMX MSRs, port I/O timing",
            "threshold": 200,
            "mitigation": "Reduce exit frequency, inline handlers, timing compensation"
        }
    },
    "cpuid_based": {
        "hypervisor_bit": {
            "description": "CPUID.1:ECX[31] - Hypervisor Present Bit",
            "technique": "Check if hypervisor bit is set",
            "mitigation": "Clear ECX[31] in CPUID exit handler"
        },
        "hypervisor_leaf": {
            "description": "CPUID leaf 0x40000000 - Hypervisor Vendor",
            "technique": "Query hypervisor identification string",
            "mitigation": "Return zeros or invalid data for leaves 0x40000000-0x4FFFFFFF"
        },
        "max_leaf": {
            "description": "CPUID.0:EAX - Maximum supported leaf",
            "technique": "Compare max leaf to expected hardware value",
            "mitigation": "Return genuine hardware max leaf value"
        }
    },
    "msr_based": {
        "vmx_msrs": {
            "description": "Read VMX capability MSRs (0x480-0x492)",
            "technique": "Attempt RDMSR on VMX MSRs, check for #GP or values",
            "mitigation": "Intercept and return 0 or inject #GP"
        },
        "feature_control": {
            "description": "IA32_FEATURE_CONTROL (0x3A)",
            "technique": "Check VMX enable bit",
            "mitigation": "Shadow MSR with VMX bits cleared"
        },
        "tsc_aux": {
            "description": "IA32_TSC_AUX inconsistency",
            "technique": "Compare TSC_AUX across operations",
            "mitigation": "Properly virtualize TSC_AUX"
        }
    },
    "memory_based": {
        "signature_scan": {
            "description": "Scan for known hypervisor signatures in memory",
            "technique": "Search for strings like 'HyperPlatform', 'SimpleSvm', etc.",
            "mitigation": "Encrypt strings, use hashes, avoid static signatures"
        },
        "pool_tags": {
            "description": "Search for hypervisor pool tags",
            "technique": "Scan nonpaged pool for tags like 'HvPl', 'VMXr'",
            "mitigation": "Use generic tags, randomize allocations"
        },
        "ept_detection": {
            "description": "Detect EPT by timing memory accesses",
            "technique": "Measure TLB miss timing differences",
            "mitigation": "Minimize EPT violations, use large pages"
        }
    },
    "behavioral": {
        "exception_timing": {
            "description": "Measure exception delivery timing",
            "technique": "Trigger #UD or #GP and measure delivery time",
            "mitigation": "Use hardware injection, minimize emulation"
        },
        "instruction_count": {
            "description": "PMC-based instruction counting",
            "technique": "Count instructions across sensitive operations",
            "mitigation": "Virtualize PMCs, adjust counts"
        },
        "nested_vmx": {
            "description": "Attempt VMXON to detect nested virtualization",
            "technique": "Execute VMXON and check behavior",
            "mitigation": "Inject #GP or implement proper nesting"
        }
    }
}


def get_detection_vectors() -> dict:
    """Return all known hypervisor detection techniques with mitigations."""
    return DETECTION_VECTORS


def audit_stealth(code: str) -> dict:
    """Analyze code for detection risks."""
    findings = []

    # Check for signature patterns
    signatures = [
        ("HyperPlatform", "Known hypervisor signature string", "critical"),
        ("SimpleSvm", "Known hypervisor signature string", "critical"),
        ("hvpp", "Known hypervisor signature string", "critical"),
        ("VMXON", "VMX instruction string", "warning"),
        ("VMCS", "VMCS reference string", "warning"),
        ("EPT violation", "EPT-related string", "warning"),
        ("HvPl", "Known pool tag", "critical"),
        ("VMXr", "Known pool tag", "critical"),
        ("EPTP", "EPT pointer reference", "info"),
    ]

    for sig, desc, severity in signatures:
        if sig in code:
            findings.append({
                "type": "signature",
                "pattern": sig,
                "description": desc,
                "severity": severity,
                "remediation": f"Remove or obfuscate '{sig}'"
            })

    # Check for timing vulnerabilities
    timing_patterns = [
        ("__rdtsc", "Direct RDTSC without compensation", "warning"),
        ("QueryPerformanceCounter", "High-res timer without virtualization", "info"),
    ]

    for pattern, desc, severity in timing_patterns:
        if pattern in code and "compensation" not in code.lower():
            findings.append({
                "type": "timing",
                "pattern": pattern,
                "description": desc,
                "severity": severity,
                "remediation": "Add timing compensation after VM-exits"
            })

    # Check for information leaks
    leak_patterns = [
        ("DbgPrint", "Debug output in release build", "warning"),
        ("KdPrint", "Debug output in release build", "warning"),
        ("printf", "Console output may leak info", "info"),
    ]

    for pattern, desc, severity in leak_patterns:
        if pattern in code:
            findings.append({
                "type": "info_leak",
                "pattern": pattern,
                "description": desc,
                "severity": severity,
                "remediation": "Conditionally compile debug output"
            })

    return {
        "findings": findings,
        "critical_count": len([f for f in findings if f["severity"] == "critical"]),
        "warning_count": len([f for f in findings if f["severity"] == "warning"]),
        "info_count": len([f for f in findings if f["severity"] == "info"])
    }


def generate_timing_compensation(exit_cycles: int = 1000) -> str:
    """Generate timing compensation code for exit handler."""
    code = []
    code.append(f"// Timing Compensation for ~{exit_cycles} cycle VM-exit overhead")
    code.append("")
    code.append("typedef struct _TIMING_STATE {")
    code.append("    ULONG64 TscOffset;")
    code.append("    ULONG64 ExitCount;")
    code.append("    ULONG64 TotalExitCycles;")
    code.append("    ULONG64 AverageExitCycles;")
    code.append("} TIMING_STATE, *PTIMING_STATE;")
    code.append("")
    code.append("VOID UpdateTimingCompensation(PTIMING_STATE State, ULONG64 ExitCycles) {")
    code.append("    State->ExitCount++;")
    code.append("    State->TotalExitCycles += ExitCycles;")
    code.append("    State->AverageExitCycles = State->TotalExitCycles / State->ExitCount;")
    code.append("")
    code.append("    // Adjust TSC offset to hide exit time")
    code.append("    State->TscOffset -= ExitCycles;")
    code.append("    __vmx_vmwrite(0x2010, State->TscOffset);  // TSC_OFFSET")
    code.append("}")
    code.append("")
    code.append("// Usage in exit handler:")
    code.append("// ULONG64 tscBefore = __rdtsc();")
    code.append("// ... handle exit ...")
    code.append("// ULONG64 tscAfter = __rdtsc();")
    code.append("// UpdateTimingCompensation(&g_TimingState, tscAfter - tscBefore);")

    return "\n".join(code)


def generate_cpuid_spoofing() -> str:
    """Generate CPUID spoofing code for stealth."""
    code = []
    code.append("// CPUID Spoofing for Hypervisor Stealth")
    code.append("")
    code.append("BOOLEAN HandleCpuidExit(PGUEST_CONTEXT Context) {")
    code.append("    int regs[4] = {0};")
    code.append("    int leaf = (int)Context->Rax;")
    code.append("    int subleaf = (int)Context->Rcx;")
    code.append("")
    code.append("    // Execute real CPUID")
    code.append("    __cpuidex(regs, leaf, subleaf);")
    code.append("")
    code.append("    switch (leaf) {")
    code.append("    case 0:")
    code.append("        // Return genuine max leaf")
    code.append("        break;")
    code.append("")
    code.append("    case 1:")
    code.append("        // Clear hypervisor present bit (ECX.31)")
    code.append("        regs[2] &= ~(1 << 31);")
    code.append("        break;")
    code.append("")
    code.append("    case 0x40000000:")
    code.append("    case 0x40000001:")
    code.append("    case 0x40000002:")
    code.append("    case 0x40000003:")
    code.append("    case 0x40000004:")
    code.append("    case 0x40000005:")
    code.append("    case 0x40000006:")
    code.append("        // Hypervisor leaves - return invalid/zero")
    code.append("        regs[0] = regs[1] = regs[2] = regs[3] = 0;")
    code.append("        break;")
    code.append("    }")
    code.append("")
    code.append("    Context->Rax = regs[0];")
    code.append("    Context->Rbx = regs[1];")
    code.append("    Context->Rcx = regs[2];")
    code.append("    Context->Rdx = regs[3];")
    code.append("")
    code.append("    AdvanceGuestRip();")
    code.append("    return TRUE;")
    code.append("}")

    return "\n".join(code)
