"""
OmbraMCP Tools Package

Advanced analysis and simulation tools for hypervisor development.
"""

from .binary_scanner import (
    scan_binary_signatures,
    scan_source_for_signatures,
)

from .vmcs_validator import (
    validate_vmcs_setup,
    get_vmcs_checklist,
)

from .anticheat_intel import (
    get_anticheat_intel,
    get_timing_requirements,
    get_bypass_for_detection,
    check_evasion_coverage,
)

from .timing_simulator import (
    simulate_handler_timing,
    get_timing_best_practices,
    compare_handler_implementations,
)

from .sdm_query import (
    ask_sdm,
    vmcs_field_complete,
    exit_reason_complete,
    get_msr_info,
    get_exception_info,
    list_vmcs_by_category,
    get_vmx_control_bits,
)

from .code_generator import (
    generate_vmcs_setup,
    generate_exit_handler,
    generate_ept_setup,
    generate_msr_bitmap_setup,
)

from .stealth import (
    get_detection_vectors,
    audit_stealth,
    generate_timing_compensation,
    generate_cpuid_spoofing,
)

from .byovd import (
    ld9boxsup_ioctl_guide,
    generate_driver_wrapper,
    generate_hypervisor_loader,
)

from .driver_mapper import (
    get_pe_parsing_guide,
    get_import_resolution_guide,
    get_relocation_guide,
    get_memory_allocation_guide,
    get_cleanup_guide,
    get_syscall_hook_guide,
    validate_driver_binary,
    generate_mapping_checklist,
)

from .hvci_bypass import (
    get_zerohvci_architecture,
    get_hypercall_protocol,
    get_hyperv_hijack_concepts,
    get_phase_implementation_guide,
    get_detection_mitigation,
    get_build_integration_guide,
    get_amd_vs_intel_considerations,
)

from .project_brain import (
    get_project_status,
    get_findings,
    dismiss_finding,
    get_suggestions,
    get_priority_work,
    get_component,
    get_exit_handler_status,
    add_decision,
    get_decision,
    list_decisions,
    add_gotcha,
    search_gotchas,
    get_session_context,
    save_session_context,
    refresh_analysis,
    get_daemon_status,
    seed_components,
)

from .vergilius import (
    get_structure,
    get_field_offset,
    compare_versions,
    get_hypervisor_offsets,
    find_field_usage,
    list_structures,
    list_versions,
    generate_offsets_header,
)

# NEW Dec 2025: Refactored database modules
from . import anticheat_db
from . import evasion_db
from . import byovd_db
from . import semantic_search
from . import mslearn_db

# Driver RE tools (Dec 27, 2025)
from . import driver_tools
from . import ioctl_tools
from . import import_tools
from . import export_tools

__all__ = [
    # Binary Scanner
    "scan_binary_signatures",
    "scan_source_for_signatures",
    # VMCS Validator
    "validate_vmcs_setup",
    "get_vmcs_checklist",
    # Anti-Cheat Intel
    "get_anticheat_intel",
    "get_timing_requirements",
    "get_bypass_for_detection",
    "check_evasion_coverage",
    # Timing Simulator
    "simulate_handler_timing",
    "get_timing_best_practices",
    "compare_handler_implementations",
    # SDM Query
    "ask_sdm",
    "vmcs_field_complete",
    "exit_reason_complete",
    "get_msr_info",
    "get_exception_info",
    "list_vmcs_by_category",
    "get_vmx_control_bits",
    # Code Generator
    "generate_vmcs_setup",
    "generate_exit_handler",
    "generate_ept_setup",
    "generate_msr_bitmap_setup",
    # Stealth
    "get_detection_vectors",
    "audit_stealth",
    "generate_timing_compensation",
    "generate_cpuid_spoofing",
    # BYOVD
    "ld9boxsup_ioctl_guide",
    "generate_driver_wrapper",
    "generate_hypervisor_loader",
    # Driver Mapper
    "get_pe_parsing_guide",
    "get_import_resolution_guide",
    "get_relocation_guide",
    "get_memory_allocation_guide",
    "get_cleanup_guide",
    "get_syscall_hook_guide",
    "validate_driver_binary",
    "generate_mapping_checklist",
    # HVCI Bypass
    "get_zerohvci_architecture",
    "get_hypercall_protocol",
    "get_hyperv_hijack_concepts",
    "get_phase_implementation_guide",
    "get_detection_mitigation",
    "get_build_integration_guide",
    "get_amd_vs_intel_considerations",
    # Project Brain
    "get_project_status",
    "get_findings",
    "dismiss_finding",
    "get_suggestions",
    "get_priority_work",
    "get_component",
    "get_exit_handler_status",
    "add_decision",
    "get_decision",
    "list_decisions",
    "add_gotcha",
    "search_gotchas",
    "get_session_context",
    "save_session_context",
    "refresh_analysis",
    "get_daemon_status",
    "seed_components",
    # Vergilius - Windows Kernel Structures
    "get_structure",
    "get_field_offset",
    "compare_versions",
    "get_hypervisor_offsets",
    "find_field_usage",
    "list_structures",
    "list_versions",
    "generate_offsets_header",
    # NEW Dec 2025: Database modules
    "anticheat_db",
    "evasion_db",
    "byovd_db",
    "semantic_search",
    "mslearn_db",
    # Driver RE tools (Dec 27, 2025)
    "driver_tools",
    "ioctl_tools",
    "import_tools",
    "export_tools",
]
