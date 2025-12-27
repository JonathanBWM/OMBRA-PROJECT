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


def get_hyperv_enlightenment_info() -> dict:
    """
    Load and return complete Hyper-V enlightenment documentation.

    Returns:
        dict: Complete documentation of CPUID leaves, MSRs, and evasion strategies
    """
    enlightenment_file = DATA_DIR / "hyperv_enlightenments.json"

    try:
        with open(enlightenment_file, 'r') as f:
            return json.load(f)
    except FileNotFoundError:
        return {
            "error": "Hyper-V enlightenments data file not found",
            "path": str(enlightenment_file)
        }
    except json.JSONDecodeError as e:
        return {
            "error": f"Failed to parse Hyper-V enlightenments JSON: {e}",
            "path": str(enlightenment_file)
        }


def check_cpuid_safety(leaf: int) -> dict:
    """
    Check if a CPUID leaf is safe to modify and return risk analysis.

    Args:
        leaf: CPUID leaf number (e.g., 0x40000000)

    Returns:
        dict: Safety analysis including:
            - safe_to_modify: YES/NO/PARTIAL/RISKY/UNKNOWN
            - risk_level: CRITICAL/HIGH/MEDIUM/LOW
            - description: What this leaf does
            - warnings: What breaks if modified
            - recommended_action: How to handle this leaf safely
    """
    enlightenments = get_hyperv_enlightenment_info()

    if "error" in enlightenments:
        return {
            "leaf": hex(leaf),
            "safe_to_modify": "UNKNOWN",
            "risk_level": "UNKNOWN",
            "error": enlightenments["error"]
        }

    # Convert integer leaf to hex string for lookup
    leaf_hex = f"0x{leaf:08X}"

    # Check if this is a Hyper-V enlightenment leaf
    if leaf_hex in enlightenments.get("cpuid_leaves", {}):
        leaf_data = enlightenments["cpuid_leaves"][leaf_hex]

        safe_status_raw = leaf_data.get("safe_to_modify", "UNKNOWN")
        warning = leaf_data.get("warning", "")

        # Extract just the status keyword (before any hyphen/description)
        safe_status = safe_status_raw.split(" - ")[0].split(":")[0].strip()

        # Determine risk level based on safe_to_modify field
        risk_map = {
            "NO": "CRITICAL",
            "PARTIAL": "MEDIUM",
            "RISKY": "HIGH",
            "YES": "LOW",
            "PLATFORM_DEPENDENT": "MEDIUM",
            "UNKNOWN": "HIGH"
        }
        risk_level = risk_map.get(safe_status, "HIGH")

        # Build recommended action
        if leaf == 0x40000000:
            recommended_action = "Zero EBX/ECX/EDX (vendor string) but preserve EAX (max leaf value)"
        elif safe_status == "NO":
            recommended_action = "Pass through unchanged - DO NOT MODIFY"
        elif safe_status == "RISKY":
            recommended_action = "Pass through if possible, modify only if absolutely necessary"
        elif safe_status == "YES":
            recommended_action = "Safe to modify or zero"
        else:
            recommended_action = "Pass through to be safe"

        return {
            "leaf": leaf_hex,
            "name": leaf_data.get("name", "Unknown"),
            "safe_to_modify": safe_status_raw,  # Return full description
            "safe_to_modify_code": safe_status,  # Return just the keyword
            "risk_level": risk_level,
            "description": leaf_data.get("eax", leaf_data.get("description", "")),
            "warnings": [warning] if warning else [],
            "recommended_action": recommended_action,
            "is_hyperv_leaf": True
        }

    # Check standard x86 CPUID leaves
    if leaf == 0x0:
        return {
            "leaf": hex(leaf),
            "name": "Maximum Standard Function",
            "safe_to_modify": "YES",
            "risk_level": "LOW",
            "description": "Returns maximum supported standard CPUID leaf",
            "warnings": ["Ensure EAX is >= actual max leaf or software may not detect CPU features"],
            "recommended_action": "Return genuine hardware value",
            "is_hyperv_leaf": False
        }

    if leaf == 0x1:
        return {
            "leaf": hex(leaf),
            "name": "Processor Info and Feature Bits",
            "safe_to_modify": "PARTIAL",
            "risk_level": "MEDIUM",
            "description": "Processor signature and feature flags",
            "warnings": [
                "ECX[31] is hypervisor present bit - MUST clear for stealth",
                "Modifying other bits may break software expecting specific features"
            ],
            "recommended_action": "Clear ECX[31] only, pass through all other values",
            "is_hyperv_leaf": False
        }

    if 0x40000000 <= leaf <= 0x400000FF:
        return {
            "leaf": hex(leaf),
            "name": "Hypervisor Information Leaf",
            "safe_to_modify": "RISKY",
            "risk_level": "HIGH",
            "description": "Reserved for hypervisor use - may be Hyper-V, VMware, KVM, or other",
            "warnings": ["Unknown hypervisor leaf - may be critical for guest OS"],
            "recommended_action": "Research this specific leaf or pass through unchanged",
            "is_hyperv_leaf": True  # Potentially
        }

    # Standard x86 feature leaves
    if leaf in [0x2, 0x4, 0x5, 0x6, 0x7, 0xA, 0xB, 0xD, 0xF, 0x10]:
        return {
            "leaf": hex(leaf),
            "name": "Standard CPU Feature/Info Leaf",
            "safe_to_modify": "RISKY",
            "risk_level": "MEDIUM",
            "description": "Standard x86 CPUID leaf for CPU features/cache/topology",
            "warnings": ["Modifying may break OS or application expectations"],
            "recommended_action": "Pass through unchanged unless you know what you're doing",
            "is_hyperv_leaf": False
        }

    # Extended function leaves
    if leaf >= 0x80000000:
        return {
            "leaf": hex(leaf),
            "name": "Extended Function Leaf",
            "safe_to_modify": "RISKY",
            "risk_level": "MEDIUM",
            "description": "Extended CPUID function (processor name, advanced features, etc.)",
            "warnings": ["May contain processor branding or advanced feature flags"],
            "recommended_action": "Pass through unchanged",
            "is_hyperv_leaf": False
        }

    # Unknown leaf
    return {
        "leaf": hex(leaf),
        "name": "Unknown CPUID Leaf",
        "safe_to_modify": "UNKNOWN",
        "risk_level": "HIGH",
        "description": "Unknown CPUID leaf - behavior depends on CPU vendor and model",
        "warnings": ["May be reserved, vendor-specific, or invalid"],
        "recommended_action": "Execute real CPUID and pass through result",
        "is_hyperv_leaf": False
    }


def get_amd_evasion_info(query_type: str = "all") -> dict:
    """
    Load and return AMD-specific evasion information.

    Args:
        query_type: Type of information to retrieve
            - "all": Complete AMD evasion data
            - "cpuid": CPUID leaf information
            - "msrs": SVM MSR information
            - "checklist": Evasion checklist
            - "detection": Detection-specific info
            - "intel_vs_amd": Architecture comparison

    Returns:
        Dictionary with requested AMD evasion information
    """
    amd_data_path = DATA_DIR / "amd_cpuid.json"

    try:
        with open(amd_data_path, 'r') as f:
            amd_data = json.load(f)
    except FileNotFoundError:
        return {
            "error": "AMD CPUID data file not found",
            "path": str(amd_data_path)
        }
    except json.JSONDecodeError as e:
        return {
            "error": "Failed to parse AMD CPUID data",
            "details": str(e)
        }

    if query_type == "all":
        return amd_data
    elif query_type == "cpuid":
        return {
            "cpuid_leaves": amd_data.get("cpuid_leaves", {}),
            "note": "Use these to identify which CPUID leaves need spoofing on AMD systems"
        }
    elif query_type == "msrs":
        return {
            "svm_msrs": amd_data.get("svm_msrs", {}),
            "note": "Virtualize these MSRs to hide AMD SVM presence"
        }
    elif query_type == "checklist":
        return {
            "evasion_checklist": amd_data.get("evasion_checklist", []),
            "note": "Complete this checklist to ensure AMD SVM stealth"
        }
    elif query_type == "detection":
        return {
            "detection_specific": amd_data.get("detection_specific", {}),
            "note": "Common AMD-specific detection techniques and considerations"
        }
    elif query_type == "intel_vs_amd":
        return {
            "intel_vs_amd": amd_data.get("intel_vs_amd", {}),
            "note": "Key architectural differences that impact evasion strategy"
        }
    else:
        return {
            "error": f"Unknown query_type: {query_type}",
            "valid_types": ["all", "cpuid", "msrs", "checklist", "detection", "intel_vs_amd"]
        }


def generate_amd_cpuid_spoofing() -> str:
    """Generate AMD-specific CPUID spoofing code for SVM stealth."""
    code = []
    code.append("// AMD SVM CPUID Spoofing for Anti-Cheat Evasion")
    code.append("")
    code.append("BOOLEAN HandleAmdCpuidExit(PGUEST_CONTEXT Context) {")
    code.append("    int regs[4] = {0};")
    code.append("    int leaf = (int)Context->Rax;")
    code.append("    int subleaf = (int)Context->Rcx;")
    code.append("")
    code.append("    // Execute real CPUID")
    code.append("    __cpuidex(regs, leaf, subleaf);")
    code.append("")
    code.append("    switch (leaf) {")
    code.append("    case 0:")
    code.append("        // Return genuine max standard leaf")
    code.append("        break;")
    code.append("")
    code.append("    case 1:")
    code.append("        // Clear hypervisor present bit (ECX.31)")
    code.append("        regs[2] &= ~(1 << 31);")
    code.append("        break;")
    code.append("")
    code.append("    case 0x80000000:")
    code.append("        // Return genuine max extended leaf")
    code.append("        // Ensure 0x8000000A is within range but we'll handle it specially")
    code.append("        break;")
    code.append("")
    code.append("    case 0x80000001:")
    code.append("        // AMD Extended Features - CRITICAL FOR SVM DETECTION")
    code.append("        // Clear ECX[2] (SVM bit) to hide AMD-V support")
    code.append("        regs[2] &= ~(1 << 2);")
    code.append("        break;")
    code.append("")
    code.append("    case 0x8000000A:")
    code.append("        // SVM Feature Identification")
    code.append("        // OPTION 1: Return all zeros (safest)")
    code.append("        regs[0] = regs[1] = regs[2] = regs[3] = 0;")
    code.append("        ")
    code.append("        // OPTION 2: Return valid data but clear critical bits")
    code.append("        // Uncomment if anti-cheat expects this leaf to exist:")
    code.append("        // regs[3] &= ~0x1FFFF;  // Clear all SVM feature bits")
    code.append("        break;")
    code.append("")
    code.append("    case 0x40000000:")
    code.append("    case 0x40000001:")
    code.append("    case 0x40000002:")
    code.append("    case 0x40000003:")
    code.append("    case 0x40000004:")
    code.append("    case 0x40000005:")
    code.append("    case 0x40000006:")
    code.append("        // Hypervisor information leaves - return invalid/zero")
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


def generate_amd_msr_virtualization() -> str:
    """Generate AMD SVM MSR virtualization code."""
    code = []
    code.append("// AMD SVM MSR Virtualization")
    code.append("")
    code.append("BOOLEAN HandleAmdRdmsrExit(PGUEST_CONTEXT Context) {")
    code.append("    ULONG64 value = 0;")
    code.append("    ULONG msr = (ULONG)Context->Rcx;")
    code.append("")
    code.append("    switch (msr) {")
    code.append("    case 0xC0000080:  // EFER")
    code.append("        // Read real EFER but clear SVME bit (bit 12)")
    code.append("        value = __readmsr(msr);")
    code.append("        value &= ~(1ULL << 12);  // Clear SVM Enable")
    code.append("        break;")
    code.append("")
    code.append("    case 0xC0010114:  // VM_CR")
    code.append("        // Return 0 to hide SVM state")
    code.append("        value = 0;")
    code.append("        // Alternative: Inject #GP(0)")
    code.append("        // InjectException(Context, EXCEPTION_GP, 0);")
    code.append("        // return TRUE;")
    code.append("        break;")
    code.append("")
    code.append("    case 0xC0010117:  // VM_HSAVE_PA")
    code.append("        // Return 0 (no host save area configured)")
    code.append("        value = 0;")
    code.append("        break;")
    code.append("")
    code.append("    case 0xC0010104:  // TSC_RATIO")
    code.append("        // If using TSC ratio for compensation, return normalized value")
    code.append("        // Otherwise inject #GP")
    code.append("        InjectException(Context, EXCEPTION_GP, 0);")
    code.append("        return TRUE;")
    code.append("")
    code.append("    case 0xC0000103:  // IA32_TSC_AUX")
    code.append("        // Return consistent value (used by RDTSCP)")
    code.append("        value = g_ShadowTscAux;")
    code.append("        break;")
    code.append("")
    code.append("    default:")
    code.append("        // Pass through to real hardware")
    code.append("        value = __readmsr(msr);")
    code.append("        break;")
    code.append("    }")
    code.append("")
    code.append("    Context->Rax = (ULONG)(value & 0xFFFFFFFF);")
    code.append("    Context->Rdx = (ULONG)(value >> 32);")
    code.append("")
    code.append("    AdvanceGuestRip();")
    code.append("    return TRUE;")
    code.append("}")

    return "\n".join(code)
