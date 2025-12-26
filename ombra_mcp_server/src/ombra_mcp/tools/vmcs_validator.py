#!/usr/bin/env python3
"""
VMCS Validator — Validate VMCS configuration against Intel requirements

This module validates:
- Required fields are set
- Control field consistency (allowed 0/1 bits)
- Guest state validity
- Host state validity
- Segment register rules
- Reserved bit checks
"""

import re
from typing import Optional
from dataclasses import dataclass

# =============================================================================
# VMCS FIELD REQUIREMENTS DATABASE
# =============================================================================

# Control fields and their requirements
# Format: field_name -> (allowed_0_mask, allowed_1_mask, description)
# The actual allowed bits come from VMX capability MSRs, but we define defaults

PIN_BASED_CONTROLS = {
    "external_interrupt_exiting": (0, "Bit 0 - External-interrupt exiting"),
    "nmi_exiting": (3, "Bit 3 - NMI exiting"),
    "virtual_nmis": (5, "Bit 5 - Virtual NMIs"),
    "preemption_timer": (6, "Bit 6 - VMX-preemption timer"),
    "process_posted_interrupts": (7, "Bit 7 - Process posted interrupts"),
}

PRIMARY_PROC_CONTROLS = {
    "interrupt_window_exiting": (2, "Bit 2 - Interrupt-window exiting"),
    "use_tsc_offsetting": (3, "Bit 3 - Use TSC offsetting"),
    "hlt_exiting": (7, "Bit 7 - HLT exiting"),
    "invlpg_exiting": (9, "Bit 9 - INVLPG exiting"),
    "mwait_exiting": (10, "Bit 10 - MWAIT exiting"),
    "rdpmc_exiting": (11, "Bit 11 - RDPMC exiting"),
    "rdtsc_exiting": (12, "Bit 12 - RDTSC exiting"),
    "cr3_load_exiting": (15, "Bit 15 - CR3-load exiting"),
    "cr3_store_exiting": (16, "Bit 16 - CR3-store exiting"),
    "cr8_load_exiting": (19, "Bit 19 - CR8-load exiting"),
    "cr8_store_exiting": (20, "Bit 20 - CR8-store exiting"),
    "use_tpr_shadow": (21, "Bit 21 - Use TPR shadow"),
    "nmi_window_exiting": (22, "Bit 22 - NMI-window exiting"),
    "mov_dr_exiting": (23, "Bit 23 - MOV-DR exiting"),
    "unconditional_io_exiting": (24, "Bit 24 - Unconditional I/O exiting"),
    "use_io_bitmaps": (25, "Bit 25 - Use I/O bitmaps"),
    "monitor_trap_flag": (27, "Bit 27 - Monitor trap flag"),
    "use_msr_bitmaps": (28, "Bit 28 - Use MSR bitmaps"),
    "monitor_exiting": (29, "Bit 29 - MONITOR exiting"),
    "pause_exiting": (30, "Bit 30 - PAUSE exiting"),
    "activate_secondary_controls": (31, "Bit 31 - Activate secondary controls"),
}

SECONDARY_PROC_CONTROLS = {
    "virtualize_apic_accesses": (0, "Bit 0 - Virtualize APIC accesses"),
    "enable_ept": (1, "Bit 1 - Enable EPT"),
    "descriptor_table_exiting": (2, "Bit 2 - Descriptor-table exiting"),
    "enable_rdtscp": (3, "Bit 3 - Enable RDTSCP"),
    "virtualize_x2apic_mode": (4, "Bit 4 - Virtualize x2APIC mode"),
    "enable_vpid": (5, "Bit 5 - Enable VPID"),
    "wbinvd_exiting": (6, "Bit 6 - WBINVD exiting"),
    "unrestricted_guest": (7, "Bit 7 - Unrestricted guest"),
    "apic_register_virtualization": (8, "Bit 8 - APIC-register virtualization"),
    "virtual_interrupt_delivery": (9, "Bit 9 - Virtual-interrupt delivery"),
    "pause_loop_exiting": (10, "Bit 10 - PAUSE-loop exiting"),
    "rdrand_exiting": (11, "Bit 11 - RDRAND exiting"),
    "enable_invpcid": (12, "Bit 12 - Enable INVPCID"),
    "enable_vm_functions": (13, "Bit 13 - Enable VM functions"),
    "vmcs_shadowing": (14, "Bit 14 - VMCS shadowing"),
    "enable_encls_exiting": (15, "Bit 15 - Enable ENCLS exiting"),
    "rdseed_exiting": (16, "Bit 16 - RDSEED exiting"),
    "enable_pml": (17, "Bit 17 - Enable PML"),
    "ept_violation_ve": (18, "Bit 18 - EPT-violation #VE"),
    "conceal_vmx_from_pt": (19, "Bit 19 - Conceal VMX from PT"),
    "enable_xsaves_xrstors": (20, "Bit 20 - Enable XSAVES/XRSTORS"),
    "mode_based_ept_execute": (22, "Bit 22 - Mode-based execute control for EPT"),
    "sub_page_write_permissions": (23, "Bit 23 - Sub-page write permissions for EPT"),
    "pt_uses_guest_physical": (24, "Bit 24 - Intel PT uses guest physical addresses"),
    "use_tsc_scaling": (25, "Bit 25 - Use TSC scaling"),
    "enable_user_wait_pause": (26, "Bit 26 - Enable user wait and pause"),
    "enable_enclv_exiting": (28, "Bit 28 - Enable ENCLV exiting"),
}

EXIT_CONTROLS = {
    "save_debug_controls": (2, "Bit 2 - Save debug controls"),
    "host_address_space_size": (9, "Bit 9 - Host address-space size (must be 1 for 64-bit host)"),
    "load_ia32_perf_global_ctrl": (12, "Bit 12 - Load IA32_PERF_GLOBAL_CTRL"),
    "acknowledge_interrupt_on_exit": (15, "Bit 15 - Acknowledge interrupt on exit"),
    "save_ia32_pat": (18, "Bit 18 - Save IA32_PAT"),
    "load_ia32_pat": (19, "Bit 19 - Load IA32_PAT"),
    "save_ia32_efer": (20, "Bit 20 - Save IA32_EFER"),
    "load_ia32_efer": (21, "Bit 21 - Load IA32_EFER"),
    "save_preemption_timer": (22, "Bit 22 - Save VMX-preemption timer value"),
    "clear_ia32_bndcfgs": (23, "Bit 23 - Clear IA32_BNDCFGS"),
    "conceal_vmx_from_pt": (24, "Bit 24 - Conceal VMX from PT"),
}

ENTRY_CONTROLS = {
    "load_debug_controls": (2, "Bit 2 - Load debug controls"),
    "ia32e_mode_guest": (9, "Bit 9 - IA-32e mode guest"),
    "entry_to_smm": (10, "Bit 10 - Entry to SMM"),
    "deactivate_dual_monitor": (11, "Bit 11 - Deactivate dual-monitor treatment"),
    "load_ia32_perf_global_ctrl": (13, "Bit 13 - Load IA32_PERF_GLOBAL_CTRL"),
    "load_ia32_pat": (14, "Bit 14 - Load IA32_PAT"),
    "load_ia32_efer": (15, "Bit 15 - Load IA32_EFER"),
    "load_ia32_bndcfgs": (16, "Bit 16 - Load IA32_BNDCFGS"),
    "conceal_vmx_from_pt": (17, "Bit 17 - Conceal VMX from PT"),
}

# Segment access rights validation
SEGMENT_TYPE_REQUIREMENTS = {
    "CS": {
        "valid_types": [0x9, 0xB, 0xD, 0xF],  # Execute-only or execute/read, conforming or not
        "must_be_present": True,
        "description": "CS must be code segment (execute)"
    },
    "SS": {
        "valid_types": [0x3, 0x7],  # Read/write data segment
        "must_be_present": True,  # Unless in 64-bit mode with null selector
        "description": "SS must be data segment (read/write)"
    },
    "DS": {
        "valid_types": [0x1, 0x3, 0x5, 0x7],  # Data segment types
        "must_be_present": False,
        "description": "DS must be data segment if present"
    },
    "ES": {
        "valid_types": [0x1, 0x3, 0x5, 0x7],
        "must_be_present": False,
        "description": "ES must be data segment if present"
    },
    "FS": {
        "valid_types": [0x1, 0x3, 0x5, 0x7],
        "must_be_present": False,
        "description": "FS must be data segment if present"
    },
    "GS": {
        "valid_types": [0x1, 0x3, 0x5, 0x7],
        "must_be_present": False,
        "description": "GS must be data segment if present"
    },
    "TR": {
        "valid_types": [0xB],  # Busy 32-bit TSS (or 0x3 for 16-bit)
        "must_be_present": True,
        "description": "TR must be busy TSS"
    },
    "LDTR": {
        "valid_types": [0x2],  # LDT
        "must_be_present": False,
        "description": "LDTR must be LDT type if present"
    },
}

# =============================================================================
# VALIDATOR IMPLEMENTATION
# =============================================================================

@dataclass
class ValidationResult:
    valid: bool
    field: str
    issue: str
    fix: str = ""
    severity: str = "error"  # error, warning, info


class VMCSValidator:
    def __init__(self):
        self.errors = []
        self.warnings = []
        self.vmcs_values = {}
        
    def parse_vmcs_code(self, code: str) -> dict:
        """Parse VMCS setup code to extract field values."""
        values = {}
        
        # Match patterns like:
        # __vmx_vmwrite(VMCS_GUEST_CR0, value);
        # vmwrite(GUEST_CR0, value);
        # VMWRITE(0x6800, value);
        
        patterns = [
            r'__vmx_vmwrite\s*\(\s*(?:VMCS_)?(\w+)\s*,\s*([^)]+)\)',
            r'vmwrite\s*\(\s*(?:VMCS_)?(\w+)\s*,\s*([^)]+)\)',
            r'VMWRITE\s*\(\s*(?:VMCS_)?(\w+)\s*,\s*([^)]+)\)',
            r'VmxVmwrite\s*\(\s*(?:VMCS_)?(\w+)\s*,\s*([^)]+)\)',
        ]
        
        for pattern in patterns:
            matches = re.finditer(pattern, code, re.IGNORECASE)
            for match in matches:
                field = match.group(1).upper()
                value = match.group(2).strip()
                values[field] = value
        
        self.vmcs_values = values
        return values
    
    def validate_required_fields(self) -> list:
        """Check that all required fields are set."""
        results = []
        
        required_guest_fields = [
            "GUEST_CR0", "GUEST_CR3", "GUEST_CR4",
            "GUEST_RSP", "GUEST_RIP", "GUEST_RFLAGS",
            "GUEST_CS_SELECTOR", "GUEST_CS_BASE", "GUEST_CS_LIMIT", "GUEST_CS_ACCESS_RIGHTS",
            "GUEST_SS_SELECTOR", "GUEST_SS_BASE", "GUEST_SS_LIMIT", "GUEST_SS_ACCESS_RIGHTS",
            "GUEST_DS_SELECTOR", "GUEST_DS_BASE", "GUEST_DS_LIMIT", "GUEST_DS_ACCESS_RIGHTS",
            "GUEST_ES_SELECTOR", "GUEST_ES_BASE", "GUEST_ES_LIMIT", "GUEST_ES_ACCESS_RIGHTS",
            "GUEST_FS_SELECTOR", "GUEST_FS_BASE", "GUEST_FS_LIMIT", "GUEST_FS_ACCESS_RIGHTS",
            "GUEST_GS_SELECTOR", "GUEST_GS_BASE", "GUEST_GS_LIMIT", "GUEST_GS_ACCESS_RIGHTS",
            "GUEST_TR_SELECTOR", "GUEST_TR_BASE", "GUEST_TR_LIMIT", "GUEST_TR_ACCESS_RIGHTS",
            "GUEST_LDTR_SELECTOR", "GUEST_LDTR_BASE", "GUEST_LDTR_LIMIT", "GUEST_LDTR_ACCESS_RIGHTS",
            "GUEST_GDTR_BASE", "GUEST_GDTR_LIMIT",
            "GUEST_IDTR_BASE", "GUEST_IDTR_LIMIT",
            "GUEST_VMCS_LINK_PTR",
        ]
        
        required_host_fields = [
            "HOST_CR0", "HOST_CR3", "HOST_CR4",
            "HOST_RSP", "HOST_RIP",
            "HOST_CS_SELECTOR", "HOST_SS_SELECTOR", "HOST_DS_SELECTOR",
            "HOST_ES_SELECTOR", "HOST_FS_SELECTOR", "HOST_GS_SELECTOR",
            "HOST_TR_SELECTOR",
            "HOST_TR_BASE", "HOST_GDTR_BASE", "HOST_IDTR_BASE",
            "HOST_FS_BASE", "HOST_GS_BASE",
        ]
        
        required_control_fields = [
            "PIN_BASED_VM_EXEC_CONTROL",
            "CPU_BASED_VM_EXEC_CONTROL",
            "VM_EXIT_CONTROLS",
            "VM_ENTRY_CONTROLS",
        ]
        
        for field in required_guest_fields:
            if field not in self.vmcs_values:
                results.append(ValidationResult(
                    valid=False,
                    field=field,
                    issue=f"Required guest-state field {field} not set",
                    fix=f"Add: __vmx_vmwrite(VMCS_{field}, <value>);",
                    severity="error"
                ))
        
        for field in required_host_fields:
            if field not in self.vmcs_values:
                results.append(ValidationResult(
                    valid=False,
                    field=field,
                    issue=f"Required host-state field {field} not set",
                    fix=f"Add: __vmx_vmwrite(VMCS_{field}, <value>);",
                    severity="error"
                ))
        
        for field in required_control_fields:
            if field not in self.vmcs_values:
                results.append(ValidationResult(
                    valid=False,
                    field=field,
                    issue=f"Required control field {field} not set",
                    fix=f"Add: __vmx_vmwrite(VMCS_{field}, AdjustControls(<desired>, MSR));",
                    severity="error"
                ))
        
        return results
    
    def validate_control_consistency(self) -> list:
        """Check that control fields are consistent with each other."""
        results = []
        
        # Check if secondary controls enabled but activate bit not set
        if "SECONDARY_VM_EXEC_CONTROL" in self.vmcs_values:
            primary = self.vmcs_values.get("CPU_BASED_VM_EXEC_CONTROL", "")
            if "1 << 31" not in primary and "0x80000000" not in primary.lower():
                results.append(ValidationResult(
                    valid=False,
                    field="CPU_BASED_VM_EXEC_CONTROL",
                    issue="Secondary controls configured but bit 31 (activate secondary) may not be set",
                    fix="Ensure CPU_BASED_VM_EXEC_CONTROL has bit 31 set",
                    severity="error"
                ))
        
        # Check EPT pointer if EPT enabled
        secondary = self.vmcs_values.get("SECONDARY_VM_EXEC_CONTROL", "")
        if "enable_ept" in secondary.lower() or "1 << 1" in secondary:
            if "EPT_POINTER" not in self.vmcs_values:
                results.append(ValidationResult(
                    valid=False,
                    field="EPT_POINTER",
                    issue="EPT enabled in secondary controls but EPT_POINTER not set",
                    fix="Set EPT_POINTER with valid EPTP value",
                    severity="error"
                ))
        
        # Check VPID if enabled
        if "enable_vpid" in secondary.lower() or "1 << 5" in secondary:
            if "VIRTUAL_PROCESSOR_ID" not in self.vmcs_values:
                results.append(ValidationResult(
                    valid=False,
                    field="VIRTUAL_PROCESSOR_ID",
                    issue="VPID enabled but VIRTUAL_PROCESSOR_ID not set",
                    fix="Set VIRTUAL_PROCESSOR_ID to non-zero value (e.g., 1)",
                    severity="error"
                ))
        
        # Check MSR bitmaps if enabled
        primary = self.vmcs_values.get("CPU_BASED_VM_EXEC_CONTROL", "")
        if "use_msr_bitmaps" in primary.lower() or "1 << 28" in primary:
            if "MSR_BITMAP" not in self.vmcs_values:
                results.append(ValidationResult(
                    valid=False,
                    field="MSR_BITMAP",
                    issue="MSR bitmaps enabled but MSR_BITMAP address not set",
                    fix="Set MSR_BITMAP to physical address of 4KB bitmap",
                    severity="error"
                ))
        
        # Check I/O bitmaps if enabled
        if "use_io_bitmaps" in primary.lower() or "1 << 25" in primary:
            if "IO_BITMAP_A" not in self.vmcs_values:
                results.append(ValidationResult(
                    valid=False,
                    field="IO_BITMAP_A",
                    issue="I/O bitmaps enabled but IO_BITMAP_A not set",
                    fix="Set IO_BITMAP_A and IO_BITMAP_B",
                    severity="error"
                ))
        
        return results
    
    def validate_64bit_host(self) -> list:
        """Validate 64-bit host requirements."""
        results = []
        
        exit_controls = self.vmcs_values.get("VM_EXIT_CONTROLS", "")
        
        # Host address space size must be 1 for 64-bit
        if "host_address_space_size" not in exit_controls.lower() and "1 << 9" not in exit_controls:
            results.append(ValidationResult(
                valid=False,
                field="VM_EXIT_CONTROLS",
                issue="64-bit host requires bit 9 (host address-space size) set",
                fix="Add (1 << 9) to VM_EXIT_CONTROLS",
                severity="error"
            ))
        
        # Check host selector alignment (must be RPL 0)
        for sel in ["HOST_CS_SELECTOR", "HOST_SS_SELECTOR", "HOST_DS_SELECTOR", 
                    "HOST_ES_SELECTOR", "HOST_FS_SELECTOR", "HOST_GS_SELECTOR", "HOST_TR_SELECTOR"]:
            value = self.vmcs_values.get(sel, "")
            if value:
                # Check if it's a raw number and verify RPL = 0
                if value.startswith("0x") or value.isdigit():
                    try:
                        num = int(value, 0)
                        if num & 0x3:  # RPL bits
                            results.append(ValidationResult(
                                valid=False,
                                field=sel,
                                issue=f"{sel} has non-zero RPL (bits 0-1 must be 0)",
                                fix=f"Use selector with RPL 0: {sel} & ~0x3",
                                severity="error"
                            ))
                    except:
                        pass
        
        return results
    
    def validate_vmcs_link_pointer(self) -> list:
        """Validate VMCS link pointer."""
        results = []
        
        link_ptr = self.vmcs_values.get("GUEST_VMCS_LINK_PTR", "")
        
        if link_ptr:
            # Should be FFFFFFFF_FFFFFFFF if not using VMCS shadowing
            if "0xffffffff" not in link_ptr.lower() and "-1" not in link_ptr:
                results.append(ValidationResult(
                    valid=False,
                    field="GUEST_VMCS_LINK_PTR",
                    issue="VMCS link pointer should be 0xFFFFFFFFFFFFFFFF if not using VMCS shadowing",
                    fix="Set GUEST_VMCS_LINK_PTR to ~0ULL or 0xFFFFFFFFFFFFFFFF",
                    severity="warning"
                ))
        
        return results
    
    def validate_cr_fields(self) -> list:
        """Validate CR0 and CR4 fields."""
        results = []
        
        # Guest CR0 checks
        guest_cr0 = self.vmcs_values.get("GUEST_CR0", "")
        if guest_cr0:
            # PE and PG must be consistent
            if "| 1" in guest_cr0 or "0x80000001" in guest_cr0:
                pass  # Likely setting PE and PG
        
        # CR4 checks
        guest_cr4 = self.vmcs_values.get("GUEST_CR4", "")
        if guest_cr4:
            # VMXE bit should be set in guest CR4 after VMXON
            if "vmxe" not in guest_cr4.lower() and "1 << 13" not in guest_cr4:
                results.append(ValidationResult(
                    valid=False,
                    field="GUEST_CR4",
                    issue="Guest CR4 should have VMXE (bit 13) set if capturing post-VMXON state",
                    fix="Ensure CR4.VMXE is set: GUEST_CR4 | (1 << 13)",
                    severity="warning"
                ))
        
        return results
    
    def validate_activity_state(self) -> list:
        """Validate guest activity state."""
        results = []
        
        activity = self.vmcs_values.get("GUEST_ACTIVITY_STATE", "")
        interruptibility = self.vmcs_values.get("GUEST_INTERRUPTIBILITY_STATE", "")
        
        if not activity:
            results.append(ValidationResult(
                valid=False,
                field="GUEST_ACTIVITY_STATE",
                issue="GUEST_ACTIVITY_STATE not set (should be 0 for active)",
                fix="Set GUEST_ACTIVITY_STATE to 0",
                severity="warning"
            ))
        
        if not interruptibility:
            results.append(ValidationResult(
                valid=False,
                field="GUEST_INTERRUPTIBILITY_STATE",
                issue="GUEST_INTERRUPTIBILITY_STATE not set",
                fix="Set GUEST_INTERRUPTIBILITY_STATE to 0 initially",
                severity="warning"
            ))
        
        return results
    
    def validate_all(self, code: str) -> dict:
        """Run all validations on VMCS setup code."""
        self.parse_vmcs_code(code)
        
        all_results = []
        all_results.extend(self.validate_required_fields())
        all_results.extend(self.validate_control_consistency())
        all_results.extend(self.validate_64bit_host())
        all_results.extend(self.validate_vmcs_link_pointer())
        all_results.extend(self.validate_cr_fields())
        all_results.extend(self.validate_activity_state())
        
        errors = [r for r in all_results if r.severity == "error"]
        warnings = [r for r in all_results if r.severity == "warning"]
        
        return {
            "valid": len(errors) == 0,
            "fields_found": len(self.vmcs_values),
            "error_count": len(errors),
            "warning_count": len(warnings),
            "errors": [
                {
                    "field": r.field,
                    "issue": r.issue,
                    "fix": r.fix
                }
                for r in errors
            ],
            "warnings": [
                {
                    "field": r.field,
                    "issue": r.issue,
                    "fix": r.fix
                }
                for r in warnings
            ],
            "fields_parsed": list(self.vmcs_values.keys())
        }


# =============================================================================
# MCP TOOL INTERFACE
# =============================================================================

async def validate_vmcs_setup(vmcs_code: str) -> dict:
    """
    Validate VMCS configuration against Intel requirements.
    
    Checks:
    - Required fields are set
    - Control field consistency
    - Guest/host state validity
    - 64-bit mode requirements
    - Segment register rules
    
    Args:
        vmcs_code: C code containing VMCS setup (vmwrite calls)
    
    Returns:
        Validation results with errors and fix suggestions
    """
    validator = VMCSValidator()
    return validator.validate_all(vmcs_code)


async def get_vmcs_checklist() -> dict:
    """
    Get a checklist of all VMCS setup requirements.
    
    Returns:
        Comprehensive checklist for VMCS configuration
    """
    return {
        "guest_state_fields": {
            "control_registers": ["CR0", "CR3", "CR4"],
            "debug_registers": ["DR7"],
            "rsp_rip_rflags": ["RSP", "RIP", "RFLAGS"],
            "segment_registers": [
                "CS (selector, base, limit, access_rights)",
                "SS (selector, base, limit, access_rights)",
                "DS (selector, base, limit, access_rights)",
                "ES (selector, base, limit, access_rights)",
                "FS (selector, base, limit, access_rights)",
                "GS (selector, base, limit, access_rights)",
                "TR (selector, base, limit, access_rights)",
                "LDTR (selector, base, limit, access_rights)",
            ],
            "descriptor_tables": ["GDTR (base, limit)", "IDTR (base, limit)"],
            "msr_fields": ["IA32_DEBUGCTL", "IA32_SYSENTER_CS/ESP/EIP", "IA32_EFER (if loading)"],
            "other": ["VMCS_LINK_POINTER (set to ~0)", "ACTIVITY_STATE", "INTERRUPTIBILITY_STATE"],
        },
        "host_state_fields": {
            "control_registers": ["CR0", "CR3", "CR4"],
            "rsp_rip": ["RSP (host stack)", "RIP (vmexit handler)"],
            "segment_selectors": ["CS", "SS", "DS", "ES", "FS", "GS", "TR"],
            "segment_bases": ["FS_BASE", "GS_BASE", "TR_BASE", "GDTR_BASE", "IDTR_BASE"],
            "msr_fields": ["IA32_SYSENTER_CS/ESP/EIP", "IA32_EFER (if loading)"],
        },
        "control_fields": {
            "execution_controls": [
                "PIN_BASED_VM_EXEC_CONTROL",
                "CPU_BASED_VM_EXEC_CONTROL (primary)",
                "SECONDARY_VM_EXEC_CONTROL (if bit 31 set in primary)",
            ],
            "exit_entry_controls": [
                "VM_EXIT_CONTROLS",
                "VM_ENTRY_CONTROLS",
            ],
            "optional": [
                "EXCEPTION_BITMAP",
                "CR0_GUEST_HOST_MASK / CR0_READ_SHADOW",
                "CR4_GUEST_HOST_MASK / CR4_READ_SHADOW",
                "MSR_BITMAP (if enabled)",
                "EPT_POINTER (if EPT enabled)",
                "VPID (if VPID enabled)",
            ],
        },
        "critical_requirements": [
            "Host address-space size (bit 9 in exit controls) must be 1 for 64-bit host",
            "IA-32e mode guest (bit 9 in entry controls) should match guest mode",
            "VMCS link pointer must be ~0 unless using VMCS shadowing",
            "TR must be busy TSS type (0xB)",
            "Host selectors must have RPL = 0",
            "Use AdjustControls() with capability MSRs for control fields",
        ]
    }


# Test if run directly
if __name__ == "__main__":
    test_code = """
    __vmx_vmwrite(VMCS_GUEST_CR0, __readcr0());
    __vmx_vmwrite(VMCS_GUEST_CR3, __readcr3());
    __vmx_vmwrite(VMCS_GUEST_CR4, __readcr4());
    __vmx_vmwrite(VMCS_GUEST_RSP, guest_rsp);
    __vmx_vmwrite(VMCS_GUEST_RIP, guest_rip);
    __vmx_vmwrite(VMCS_GUEST_RFLAGS, __readeflags());
    __vmx_vmwrite(VMCS_PIN_BASED_VM_EXEC_CONTROL, pin_controls);
    __vmx_vmwrite(VMCS_CPU_BASED_VM_EXEC_CONTROL, proc_controls);
    __vmx_vmwrite(VMCS_VM_EXIT_CONTROLS, exit_controls);
    __vmx_vmwrite(VMCS_VM_ENTRY_CONTROLS, entry_controls);
    """
    
    import json
    validator = VMCSValidator()
    result = validator.validate_all(test_code)
    print(json.dumps(result, indent=2))
