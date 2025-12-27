#!/usr/bin/env python3
"""
Seed Initial Concepts - Extract concepts from old project documentation

Extracts 30+ core hypervisor concepts from legacy docs and populates the concepts database.
"""

import sys
import json
from pathlib import Path

# Add parent to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent / "src"))

from ombra_watcherd.database import ProjectBrainDB


# Initial concept seed data - extracted from old documentation
INITIAL_CONCEPTS = [
    # ==== TIMING CATEGORY ====
    {
        "id": "TSC_OFFSET_COMPENSATION",
        "category": "timing",
        "name": "TSC Offset Compensation",
        "description": "Capture TSC at VMExit entry and adjust TSC offset before VMResume to hide VMExit processing overhead from guest",
        "source_doc": "01_TIMING_ANTI_DETECTION.md",
        "sdm_refs": json.dumps(["Vol 3C 24.6.5", "Vol 3C Appendix B"]),
        "required_patterns": json.dumps([
            r"timing.*OnExitEntry",
            r"__rdtsc\(\)",
            r"VMCS.*TSC_OFFSET",
            r"timing.*OnExitComplete"
        ]),
        "optional_patterns": json.dumps([
            r"exit_entry_tsc",
            r"vmcb.*TscOffset"
        ]),
        "anti_patterns": json.dumps([
            r"VMResume.*before.*timing",  # VMResume before timing adjustment
            r"OnExitEntry.*\n.*\n.*__rdtsc"  # Too much code before TSC capture
        ]),
        "vmcs_fields": json.dumps(["0x2010"]),  # TSC_OFFSET
        "exit_reasons": json.dumps([]),  # Applies to all exits
        "msrs": json.dumps([]),
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "critical",
        "anti_cheat_relevance": json.dumps(["battleye", "eac", "vanguard"]),
        "depends_on": json.dumps([]),
        "phase_order": 1
    },
    {
        "id": "APERF_MPERF_VIRTUALIZATION",
        "category": "timing",
        "name": "APERF/MPERF Performance MSR Virtualization",
        "description": "Shadow APERF/MPERF MSRs (0xE7/0xE8) to hide hypervisor overhead from IET detection",
        "source_doc": "01_TIMING_ANTI_DETECTION.md",
        "sdm_refs": json.dumps(["Vol 4 Table 2-2"]),
        "required_patterns": json.dumps([
            r"IA32_MPERF|0xE7",
            r"IA32_APERF|0xE8",
            r"aperf_offset",
            r"mperf_offset"
        ]),
        "optional_patterns": json.dumps([
            r"timing.*ReadMsrVirtualized"
        ]),
        "anti_patterns": json.dumps([
            r"__readmsr.*0xE[78].*return"  # Direct passthrough without compensation
        ]),
        "vmcs_fields": json.dumps([]),
        "exit_reasons": json.dumps([31]),  # RDMSR
        "msrs": json.dumps(["0xE7", "0xE8"]),
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "critical",
        "anti_cheat_relevance": json.dumps(["esea"]),
        "depends_on": json.dumps(["TSC_OFFSET_COMPENSATION"]),
        "phase_order": 2
    },
    {
        "id": "TIMING_STATE_PER_VCPU",
        "category": "timing",
        "name": "Per-vCPU Timing State",
        "description": "Cache-line aligned per-CPU timing state to prevent false sharing between cores",
        "source_doc": "01_TIMING_ANTI_DETECTION.md",
        "sdm_refs": json.dumps([]),
        "required_patterns": json.dumps([
            r"VCPU_TIMING_STATE.*\[.*\]",
            r"__declspec\(align\(64\)\)|__attribute__\(\(aligned\(64\)\)\)",
            r"GetApicId|timing_get_apic_id"
        ]),
        "optional_patterns": json.dumps([
            r"g_timing_states"
        ]),
        "anti_patterns": json.dumps([
            r"static.*timing_state[^s]"  # Single global state (not array)
        ]),
        "vmcs_fields": json.dumps([]),
        "exit_reasons": json.dumps([]),
        "msrs": json.dumps([]),
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "high",
        "anti_cheat_relevance": json.dumps([]),
        "depends_on": json.dumps([]),
        "phase_order": 1
    },
    {
        "id": "TSC_CAPTURE_AT_ENTRY",
        "category": "timing",
        "name": "TSC Capture as First Instruction",
        "description": "__rdtsc() must be the first instruction in VMExit handler for accurate overhead measurement",
        "source_doc": "01_TIMING_ANTI_DETECTION.md",
        "sdm_refs": json.dumps(["Vol 2B RDTSC"]),
        "required_patterns": json.dumps([
            r"vmexit_handler.*\n.*timing.*OnExitEntry"
        ]),
        "optional_patterns": json.dumps([]),
        "anti_patterns": json.dumps([
            r"vmexit_handler.*\n.*[^timing].*\n.*timing.*OnExitEntry"  # Code before timing capture
        ]),
        "vmcs_fields": json.dumps([]),
        "exit_reasons": json.dumps([]),
        "msrs": json.dumps([]),
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "critical",
        "anti_cheat_relevance": json.dumps(["battleye", "eac", "vanguard"]),
        "depends_on": json.dumps(["TSC_OFFSET_COMPENSATION"]),
        "phase_order": 1
    },

    # ==== VMX CATEGORY ====
    {
        "id": "CPUID_EXIT_HANDLING",
        "category": "vmx",
        "name": "CPUID VMExit Handling",
        "description": "Intercept all CPUID instructions (exit reason 10) and spoof results",
        "source_doc": "02_CPUID_SPOOFING.md",
        "sdm_refs": json.dumps(["Vol 3C 25.1.2"]),
        "required_patterns": json.dumps([
            r"case.*10:|EXIT_REASON_CPUID",
            r"HandleCpuid|handle_cpuid",
            r"cpuid_spoof"
        ]),
        "optional_patterns": json.dumps([
            r"__cpuidex"
        ]),
        "anti_patterns": json.dumps([]),
        "vmcs_fields": json.dumps([]),
        "exit_reasons": json.dumps([10]),
        "msrs": json.dumps([]),
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "critical",
        "anti_cheat_relevance": json.dumps(["battleye", "eac", "vanguard"]),
        "depends_on": json.dumps([]),
        "phase_order": 1
    },
    {
        "id": "CPUID_HYPERVISOR_BIT",
        "category": "stealth",
        "name": "CPUID Hypervisor Bit Hiding",
        "description": "Clear CPUID.1.ECX[31] to hide hypervisor presence",
        "source_doc": "02_CPUID_SPOOFING.md",
        "sdm_refs": json.dumps(["Vol 2A CPUID"]),
        "required_patterns": json.dumps([
            r"ecx.*&.*~.*CPUID_HV_PRESENT",
            r"ecx.*&.*~.*\(1.*<<.*31\)"
        ]),
        "optional_patterns": json.dumps([
            r"hide_hypervisor"
        ]),
        "anti_patterns": json.dumps([
            r"CPUID.*0x1.*return.*ecx"  # Direct return without masking
        ]),
        "vmcs_fields": json.dumps([]),
        "exit_reasons": json.dumps([10]),
        "msrs": json.dumps([]),
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "critical",
        "anti_cheat_relevance": json.dumps(["battleye", "eac", "vanguard"]),
        "depends_on": json.dumps(["CPUID_EXIT_HANDLING"]),
        "phase_order": 2
    },
    {
        "id": "CPUID_VMX_HIDING",
        "category": "stealth",
        "name": "VMX Capability Bit Hiding",
        "description": "Clear CPUID.1.ECX[5] (Intel VMX) to hide virtualization capability",
        "source_doc": "02_CPUID_SPOOFING.md",
        "sdm_refs": json.dumps(["Vol 2A CPUID"]),
        "required_patterns": json.dumps([
            r"ecx.*&.*~.*CPUID_VMX|ecx.*&.*~.*\(1.*<<.*5\)"
        ]),
        "optional_patterns": json.dumps([
            r"hide_vmx"
        ]),
        "anti_patterns": json.dumps([]),
        "vmcs_fields": json.dumps([]),
        "exit_reasons": json.dumps([10]),
        "msrs": json.dumps([]),
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "high",
        "anti_cheat_relevance": json.dumps(["eac"]),
        "depends_on": json.dumps(["CPUID_EXIT_HANDLING"]),
        "phase_order": 2
    },
    {
        "id": "CPUID_VENDOR_STRING_SPOOFING",
        "category": "stealth",
        "name": "Hypervisor Vendor String Spoofing",
        "description": "Zero vendor string in CPUID.0x40000000 while preserving max leaf",
        "source_doc": "02_CPUID_SPOOFING.md",
        "sdm_refs": json.dumps(["Hyper-V TLFS 2.4"]),
        "required_patterns": json.dumps([
            r"0x40000000",
            r"ebx.*=.*0.*ecx.*=.*0.*edx.*=.*0"
        ]),
        "optional_patterns": json.dumps([
            r"spoof_hv_vendor"
        ]),
        "anti_patterns": json.dumps([
            r"0x40000000.*eax.*=.*0"  # Zeroing max leaf breaks Windows
        ]),
        "vmcs_fields": json.dumps([]),
        "exit_reasons": json.dumps([10]),
        "msrs": json.dumps([]),
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "high",
        "anti_cheat_relevance": json.dumps(["battleye", "eac"]),
        "depends_on": json.dumps(["CPUID_EXIT_HANDLING"]),
        "phase_order": 2
    },
    {
        "id": "RIP_ADVANCEMENT",
        "category": "vmx",
        "name": "Guest RIP Advancement",
        "description": "Advance guest RIP by instruction length after emulating instructions",
        "source_doc": "03_VMEXIT_HANDLERS.md",
        "sdm_refs": json.dumps(["Vol 3C 27.2.4"]),
        "required_patterns": json.dumps([
            r"VMCS.*INSTRUCTION_LENGTH|VM_EXIT_INSTRUCTION_LEN",
            r"VMCS.*GUEST_RIP",
            r"guest_rip.*\+.*instruction_len"
        ]),
        "optional_patterns": json.dumps([
            r"AdvanceGuestRip|advance_guest_rip"
        ]),
        "anti_patterns": json.dumps([
            r"return.*without.*RIP"  # Returning without advancing RIP
        ]),
        "vmcs_fields": json.dumps(["0x681C", "0x440C"]),  # RIP, INSTRUCTION_LENGTH
        "exit_reasons": json.dumps([10, 30, 31]),  # CPUID, IO, RDMSR/WRMSR
        "msrs": json.dumps([]),
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "critical",
        "anti_cheat_relevance": json.dumps([]),
        "depends_on": json.dumps([]),
        "phase_order": 1
    },
    {
        "id": "MSR_BITMAP_HANDLER_MATCH",
        "category": "vmx",
        "name": "MSR Bitmap Handler Matching",
        "description": "Every MSR in bitmap must have a handler case in RDMSR/WRMSR exit",
        "source_doc": "03_VMEXIT_HANDLERS.md",
        "sdm_refs": json.dumps(["Vol 3C 24.6.9"]),
        "required_patterns": json.dumps([
            r"msr_bitmap",
            r"case.*IA32_|case.*0x"
        ]),
        "optional_patterns": json.dumps([]),
        "anti_patterns": json.dumps([
            r"msr_bitmap.*=.*1.*\n(?!.*case.*0x)"  # Bitmap set but no handler
        ]),
        "vmcs_fields": json.dumps(["0x2004"]),  # MSR_BITMAP
        "exit_reasons": json.dumps([31, 32]),  # RDMSR, WRMSR
        "msrs": json.dumps([]),
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "high",
        "anti_cheat_relevance": json.dumps([]),
        "depends_on": json.dumps([]),
        "phase_order": 2
    },
    {
        "id": "VMREAD_UD_INJECTION",
        "category": "stealth",
        "name": "VMREAD #UD Injection",
        "description": "Inject #UD exception for VMX instruction execution to hide VMX capability",
        "source_doc": "DETECTION_VECTORS_AND_MITIGATIONS.md",
        "sdm_refs": json.dumps(["Vol 3C 30.3"]),
        "required_patterns": json.dumps([
            r"InjectUdFault|inject.*ud",
            r"VMREAD|VMWRITE|VMCLEAR"
        ]),
        "optional_patterns": json.dumps([]),
        "anti_patterns": json.dumps([]),
        "vmcs_fields": json.dumps([]),
        "exit_reasons": json.dumps([18]),  # Invalid opcode
        "msrs": json.dumps([]),
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "medium",
        "anti_cheat_relevance": json.dumps(["eac"]),
        "depends_on": json.dumps([]),
        "phase_order": 3
    },

    # ==== EPT CATEGORY ====
    {
        "id": "EPT_IDENTITY_MAPPING",
        "category": "ept",
        "name": "EPT Identity Mapping",
        "description": "Identity map all physical memory in EPT (guest physical = host physical)",
        "source_doc": "04_EPT_NPT_MEMORY.md",
        "sdm_refs": json.dumps(["Vol 3C 28.2"]),
        "required_patterns": json.dumps([
            r"EPT.*PML4|ept_pml4",
            r"page_frame_number.*=.*pfn",
            r"identity.*map"
        ]),
        "optional_patterns": json.dumps([
            r"CreatePageTable|create_page_table"
        ]),
        "anti_patterns": json.dumps([]),
        "vmcs_fields": json.dumps(["0x201A"]),  # EPT_POINTER
        "exit_reasons": json.dumps([]),
        "msrs": json.dumps([]),
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "critical",
        "anti_cheat_relevance": json.dumps([]),
        "depends_on": json.dumps([]),
        "phase_order": 1
    },
    {
        "id": "EPT_HOOK_EXECUTE_ONLY",
        "category": "ept",
        "name": "EPT Execute-Only Page Hooks",
        "description": "Use execute-only pages for hooks (read/write disabled, execute enabled)",
        "source_doc": "04_EPT_NPT_MEMORY.md",
        "sdm_refs": json.dumps(["Vol 3C 28.2.2"]),
        "required_patterns": json.dumps([
            r"execute_access.*=.*1",
            r"read_access.*=.*0",
            r"write_access.*=.*0"
        ]),
        "optional_patterns": json.dumps([
            r"hook.*page"
        ]),
        "anti_patterns": json.dumps([]),
        "vmcs_fields": json.dumps(["0x201A"]),  # EPT_POINTER
        "exit_reasons": json.dumps([48]),  # EPT_VIOLATION
        "msrs": json.dumps([]),
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "high",
        "anti_cheat_relevance": json.dumps(["battleye", "eac"]),
        "depends_on": json.dumps(["EPT_IDENTITY_MAPPING", "EPT_VIOLATION_HANDLING"]),
        "phase_order": 3
    },
    {
        "id": "EPT_VIOLATION_HANDLING",
        "category": "ept",
        "name": "EPT Violation Exit Handling",
        "description": "Handle EPT violations (exit reason 48) and parse qualification bits",
        "source_doc": "04_EPT_NPT_MEMORY.md",
        "sdm_refs": json.dumps(["Vol 3C 27.2.1"]),
        "required_patterns": json.dumps([
            r"case.*48:|EXIT_REASON_EPT_VIOLATION",
            r"EXIT_QUALIFICATION",
            r"HandleEptViolation|handle_ept_violation"
        ]),
        "optional_patterns": json.dumps([]),
        "anti_patterns": json.dumps([]),
        "vmcs_fields": json.dumps(["0x6400"]),  # EXIT_QUALIFICATION
        "exit_reasons": json.dumps([48]),
        "msrs": json.dumps([]),
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "critical",
        "anti_cheat_relevance": json.dumps([]),
        "depends_on": json.dumps(["EPT_IDENTITY_MAPPING"]),
        "phase_order": 2
    },
    {
        "id": "INVEPT_AFTER_MODIFY",
        "category": "ept",
        "name": "INVEPT After EPT Modification",
        "description": "Call INVEPT instruction after modifying EPT entries to flush TLB",
        "source_doc": "04_EPT_NPT_MEMORY.md",
        "sdm_refs": json.dumps(["Vol 3C 30.3"]),
        "required_patterns": json.dumps([
            r"INVEPT|__invept",
            r"ept.*modify|SetPTE|split.*page"
        ]),
        "optional_patterns": json.dumps([]),
        "anti_patterns": json.dumps([
            r"ept.*=.*new_entry[^I]*\n[^I]*\n[^I]*return"  # EPT modify without INVEPT
        ]),
        "vmcs_fields": json.dumps([]),
        "exit_reasons": json.dumps([]),
        "msrs": json.dumps([]),
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "critical",
        "anti_cheat_relevance": json.dumps([]),
        "depends_on": json.dumps(["EPT_IDENTITY_MAPPING"]),
        "phase_order": 2
    },
    {
        "id": "EPT_LARGE_PAGE_OPTIMIZATION",
        "category": "ept",
        "name": "EPT Large Page Optimization",
        "description": "Use 2MB/1GB pages where possible to reduce page table walks",
        "source_doc": "04_EPT_NPT_MEMORY.md",
        "sdm_refs": json.dumps(["Vol 3C 28.2.2"]),
        "required_patterns": json.dumps([
            r"large_page.*=.*1|SIZE_2_MB"
        ]),
        "optional_patterns": json.dumps([
            r"PDE_2MB|PDPTE_1GB"
        ]),
        "anti_patterns": json.dumps([]),
        "vmcs_fields": json.dumps([]),
        "exit_reasons": json.dumps([]),
        "msrs": json.dumps([]),
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "medium",
        "anti_cheat_relevance": json.dumps([]),
        "depends_on": json.dumps(["EPT_IDENTITY_MAPPING"]),
        "phase_order": 2
    },

    # ==== STEALTH CATEGORY ====
    {
        "id": "PE_HEADER_ELIMINATION",
        "category": "stealth",
        "name": "PE Header Elimination",
        "description": "Zero PE headers in memory after driver load to prevent signature scanning",
        "source_doc": "DETECTION_VECTORS_AND_MITIGATIONS.md",
        "sdm_refs": json.dumps([]),
        "required_patterns": json.dumps([
            r"memset.*pe_header.*0|RtlZeroMemory.*dos_header",
            r"IMAGE_DOS_HEADER"
        ]),
        "optional_patterns": json.dumps([]),
        "anti_patterns": json.dumps([]),
        "vmcs_fields": json.dumps([]),
        "exit_reasons": json.dumps([]),
        "msrs": json.dumps([]),
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "high",
        "anti_cheat_relevance": json.dumps(["battleye", "eac"]),
        "depends_on": json.dumps([]),
        "phase_order": 3
    },
    {
        "id": "XSETBV_VALIDATION",
        "category": "vmx",
        "name": "XSETBV Exception Validation",
        "description": "Validate XCR0 value before executing XSETBV to prevent crash detection",
        "source_doc": "DETECTION_VECTORS_AND_MITIGATIONS.md",
        "sdm_refs": json.dumps(["Vol 2B XSETBV"]),
        "required_patterns": json.dumps([
            r"IsValidXcr0|validate.*xcr0",
            r"InjectGpFault|inject.*gp",
            r"XSETBV"
        ]),
        "optional_patterns": json.dumps([]),
        "anti_patterns": json.dumps([
            r"_xsetbv.*\n(?!.*IsValid)"  # Direct XSETBV without validation
        ]),
        "vmcs_fields": json.dumps([]),
        "exit_reasons": json.dumps([55]),  # XSETBV
        "msrs": json.dumps([]),
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "medium",
        "anti_cheat_relevance": json.dumps(["eac"]),
        "depends_on": json.dumps([]),
        "phase_order": 3
    },
    {
        "id": "LBR_VIRTUALIZATION",
        "category": "stealth",
        "name": "Last Branch Record Virtualization",
        "description": "Save/restore LBR MSRs to hide hypervisor control flow",
        "source_doc": "DETECTION_VECTORS_AND_MITIGATIONS.md",
        "sdm_refs": json.dumps(["Vol 3B 17.4.8"]),
        "required_patterns": json.dumps([
            r"MSR_LBR_TOS|0x1C9",
            r"SaveLbrState|save_lbr",
            r"RestoreLbrState|restore_lbr"
        ]),
        "optional_patterns": json.dumps([]),
        "anti_patterns": json.dumps([]),
        "vmcs_fields": json.dumps([]),
        "exit_reasons": json.dumps([]),
        "msrs": json.dumps(["0x1C9", "0x680", "0x6C0"]),
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "medium",
        "anti_cheat_relevance": json.dumps(["esea"]),
        "depends_on": json.dumps([]),
        "phase_order": 4
    },
    {
        "id": "SGDT_SIDT_WOW64_FIX",
        "category": "stealth",
        "name": "SGDT/SIDT WoW64 Descriptor Size Fix",
        "description": "Return correct descriptor table size based on guest mode (4 bytes in 32-bit, 8 bytes in 64-bit)",
        "source_doc": "DETECTION_VECTORS_AND_MITIGATIONS.md",
        "sdm_refs": json.dumps(["Vol 2B SGDT", "Vol 2B SIDT"]),
        "required_patterns": json.dumps([
            r"HandleSgdt|handle_sgdt",
            r"CsDesc\.Bits\.L|cs.*long_mode",
            r"Base32|base.*4.*bytes"
        ]),
        "optional_patterns": json.dumps([]),
        "anti_patterns": json.dumps([
            r"sgdt.*8.*always"  # Always writing 8 bytes
        ]),
        "vmcs_fields": json.dumps(["0x6816", "0x6818"]),  # GDTR, IDTR
        "exit_reasons": json.dumps([27, 28]),  # GDTR/IDTR access
        "msrs": json.dumps([]),
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "high",
        "anti_cheat_relevance": json.dumps(["vanguard"]),
        "depends_on": json.dumps([]),
        "phase_order": 3
    },

    # ==== ADDITIONAL CONCEPTS ====
    {
        "id": "VMCALL_AUTHENTICATION",
        "category": "vmx",
        "name": "VMCALL Authentication",
        "description": "Validate authentication key for VMCALL hypercalls using magic CPUID leaf",
        "source_doc": "02_CPUID_SPOOFING.md",
        "sdm_refs": json.dumps(["Vol 3C 30.3"]),
        "required_patterns": json.dumps([
            r"HYPERCALL_MAGIC_LEAF|0x13371337",
            r"auth_key",
            r"IsVmcall|vmcall_is_valid"
        ]),
        "optional_patterns": json.dumps([]),
        "anti_patterns": json.dumps([]),
        "vmcs_fields": json.dumps([]),
        "exit_reasons": json.dumps([10, 18]),  # CPUID, VMCALL
        "msrs": json.dumps([]),
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "high",
        "anti_cheat_relevance": json.dumps([]),
        "depends_on": json.dumps(["CPUID_EXIT_HANDLING"]),
        "phase_order": 2
    },
    {
        "id": "MTRR_CACHE_TYPE_MAPPING",
        "category": "ept",
        "name": "MTRR Cache Type Mapping",
        "description": "Read CPU MTRR registers and apply correct cache types to EPT entries",
        "source_doc": "04_EPT_NPT_MEMORY.md",
        "sdm_refs": json.dumps(["Vol 3A 11.11"]),
        "required_patterns": json.dumps([
            r"MSR_IA32_MTRR|IA32_MTRR",
            r"BuildMtrrMap|build_mtrr",
            r"MEMORY_TYPE_WRITE_BACK|MEMORY_TYPE_UNCACHEABLE"
        ]),
        "optional_patterns": json.dumps([]),
        "anti_patterns": json.dumps([]),
        "vmcs_fields": json.dumps([]),
        "exit_reasons": json.dumps([]),
        "msrs": json.dumps(["0xFE", "0x200", "0x201"]),
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "medium",
        "anti_cheat_relevance": json.dumps([]),
        "depends_on": json.dumps(["EPT_IDENTITY_MAPPING"]),
        "phase_order": 2
    },
    {
        "id": "EPT_PAGE_SPLITTING",
        "category": "ept",
        "name": "EPT Large Page Splitting",
        "description": "Split 2MB pages into 512x4KB pages for fine-grained hooks",
        "source_doc": "04_EPT_NPT_MEMORY.md",
        "sdm_refs": json.dumps(["Vol 3C 28.2"]),
        "required_patterns": json.dumps([
            r"SplitLargePage|split.*page",
            r"EPT_PTE|ept_pte",
            r"512.*4KB|512.*PAGE_SIZE"
        ]),
        "optional_patterns": json.dumps([]),
        "anti_patterns": json.dumps([]),
        "vmcs_fields": json.dumps([]),
        "exit_reasons": json.dumps([]),
        "msrs": json.dumps([]),
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "high",
        "anti_cheat_relevance": json.dumps([]),
        "depends_on": json.dumps(["EPT_IDENTITY_MAPPING"]),
        "phase_order": 2
    },
    {
        "id": "VMCS_VALIDATION_PREFLIGHT",
        "category": "vmx",
        "name": "VMCS Validation Pre-Flight",
        "description": "Validate all VMCS fields before VMLAUNCH to prevent failures",
        "source_doc": "Intel SDM Vol 3C 26.1",
        "sdm_refs": json.dumps(["Vol 3C 26.1", "Vol 3C 26.2"]),
        "required_patterns": json.dumps([
            r"VMCS.*validate|validate.*vmcs",
            r"__vmx_vmread",
            r"control.*capability"
        ]),
        "optional_patterns": json.dumps([]),
        "anti_patterns": json.dumps([]),
        "vmcs_fields": json.dumps([]),
        "exit_reasons": json.dumps([]),
        "msrs": json.dumps(["0x480", "0x481", "0x482"]),  # VMX capability MSRs
        "implementation_status": "not_started",
        "confidence": 0.0,
        "priority": "high",
        "anti_cheat_relevance": json.dumps([]),
        "depends_on": json.dumps([]),
        "phase_order": 1
    },
]


def seed_concepts():
    """Seed initial concepts into database."""
    db = ProjectBrainDB()

    print(f"Seeding {len(INITIAL_CONCEPTS)} concepts...")

    with db._connection() as conn:
        for concept in INITIAL_CONCEPTS:
            # Check if already exists
            existing = conn.execute(
                "SELECT id FROM concepts WHERE id = ?",
                (concept['id'],)
            ).fetchone()

            if existing:
                print(f"  ⊗ {concept['id']} (already exists)")
                continue

            # Insert concept
            conn.execute("""
                INSERT INTO concepts (
                    id, category, name, description, source_doc, sdm_refs,
                    required_patterns, optional_patterns, anti_patterns,
                    vmcs_fields, exit_reasons, msrs,
                    implementation_status, confidence, verified_by_annotation,
                    implementation_files, priority, anti_cheat_relevance,
                    depends_on, phase_order
                ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """, (
                concept['id'],
                concept['category'],
                concept['name'],
                concept['description'],
                concept['source_doc'],
                concept['sdm_refs'],
                concept['required_patterns'],
                concept.get('optional_patterns'),
                concept.get('anti_patterns'),
                concept.get('vmcs_fields'),
                concept.get('exit_reasons'),
                concept.get('msrs'),
                concept['implementation_status'],
                concept['confidence'],
                False,  # verified_by_annotation
                None,   # implementation_files
                concept['priority'],
                concept.get('anti_cheat_relevance'),
                concept.get('depends_on'),
                concept.get('phase_order')
            ))

            print(f"  ✓ {concept['id']}")

    # Summary
    with db._connection() as conn:
        total = conn.execute("SELECT COUNT(*) FROM concepts").fetchone()[0]
        by_category = conn.execute("""
            SELECT category, COUNT(*) as count
            FROM concepts
            GROUP BY category
            ORDER BY count DESC
        """).fetchall()

    print(f"\nSeeded {total} total concepts:")
    for row in by_category:
        print(f"  {row['category']}: {row['count']}")


if __name__ == "__main__":
    seed_concepts()
