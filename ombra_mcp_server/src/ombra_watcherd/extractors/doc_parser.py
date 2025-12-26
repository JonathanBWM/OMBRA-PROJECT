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

    Deduplicates concepts by base type, merging data from multiple sections.

    Args:
        content: Markdown content
        source_file: Source filename for reference

    Returns:
        List of concept dictionaries (one per unique concept type)
    """
    # Track concepts by base ID for deduplication
    concept_map: Dict[str, Dict[str, Any]] = {}

    # Extract sections for context
    sections = _split_sections(content)

    for section_title, section_content in sections:
        full_text = section_title + " " + section_content

        # Find matching concept patterns
        for category, patterns in CONCEPT_PATTERNS.items():
            for pattern, concept_base in patterns:
                if re.search(pattern, full_text, re.IGNORECASE):
                    concept_id = concept_base  # Use base directly, no section suffix

                    if concept_id in concept_map:
                        # Merge data into existing concept
                        existing = concept_map[concept_id]
                        existing["vmcs_fields"] = list(set(
                            existing["vmcs_fields"] + _extract_vmcs_fields(section_content)
                        ))
                        existing["msrs"] = list(set(
                            existing["msrs"] + _extract_msrs(section_content)
                        ))
                        existing["exit_reasons"] = list(set(
                            existing["exit_reasons"] + _extract_exit_reasons(section_content)
                        ))
                        # Upgrade priority if this section has higher priority
                        new_priority = _infer_priority(section_content)
                        if _priority_rank(new_priority) < _priority_rank(existing["priority"]):
                            existing["priority"] = new_priority
                    else:
                        # Create new concept
                        concept_map[concept_id] = {
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

    return list(concept_map.values())


def _priority_rank(priority: str) -> int:
    """Return numeric rank for priority (lower = higher priority)."""
    return {"critical": 1, "high": 2, "medium": 3, "low": 4}.get(priority, 5)


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
