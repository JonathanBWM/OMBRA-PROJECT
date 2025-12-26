#!/usr/bin/env python3
"""
Intel SDM Database Builder — Comprehensive VMX Reference Database

Builds a SQLite database from Intel SDM Volume 3 (System Programming Guide)
with complete VMX specifications for instant MCP tool queries.

Usage:
    python prepare_intel_sdm.py

The database will be created at: src/ombra_mcp/data/intel_sdm.db
"""

import sqlite3
import json
from pathlib import Path

# Database location
SCRIPT_DIR = Path(__file__).parent
DATA_DIR = SCRIPT_DIR.parent / "src" / "ombra_mcp" / "data"
DB_PATH = DATA_DIR / "intel_sdm.db"


def create_schema(conn):
    """Create all database tables."""
    
    conn.executescript("""
        -- VMCS Fields (Appendix B)
        CREATE TABLE IF NOT EXISTS vmcs_fields (
            id INTEGER PRIMARY KEY,
            name TEXT UNIQUE NOT NULL,
            encoding INTEGER NOT NULL,
            width TEXT NOT NULL,
            category TEXT NOT NULL,
            subcategory TEXT,
            description TEXT,
            sdm_table TEXT
        );
        
        -- Exit Reasons (Appendix C)
        CREATE TABLE IF NOT EXISTS exit_reasons (
            id INTEGER PRIMARY KEY,
            reason_number INTEGER UNIQUE NOT NULL,
            name TEXT NOT NULL,
            has_qualification INTEGER DEFAULT 0,
            qualification_format TEXT,
            description TEXT,
            handling_notes TEXT,
            sdm_section TEXT
        );
        
        -- Exit Qualification Bit Fields
        CREATE TABLE IF NOT EXISTS exit_qualifications (
            id INTEGER PRIMARY KEY,
            exit_reason INTEGER NOT NULL,
            bit_start INTEGER NOT NULL,
            bit_end INTEGER NOT NULL,
            field_name TEXT NOT NULL,
            description TEXT,
            field_values TEXT
        );
        
        -- MSRs
        CREATE TABLE IF NOT EXISTS msrs (
            id INTEGER PRIMARY KEY,
            name TEXT UNIQUE NOT NULL,
            address INTEGER NOT NULL,
            category TEXT,
            description TEXT,
            bit_fields TEXT,
            sdm_section TEXT
        );
        
        -- VMX Control Fields
        CREATE TABLE IF NOT EXISTS vmx_controls (
            id INTEGER PRIMARY KEY,
            control_type TEXT NOT NULL,
            bit_position INTEGER NOT NULL,
            name TEXT NOT NULL,
            description TEXT,
            default_setting TEXT,
            capability_msr TEXT,
            sdm_section TEXT,
            UNIQUE(control_type, bit_position)
        );
        
        -- Exceptions
        CREATE TABLE IF NOT EXISTS exceptions (
            id INTEGER PRIMARY KEY,
            vector INTEGER UNIQUE NOT NULL,
            mnemonic TEXT NOT NULL,
            name TEXT NOT NULL,
            type TEXT NOT NULL,
            error_code INTEGER DEFAULT 0,
            description TEXT,
            source TEXT
        );
        
        -- SDM Excerpts (for full-text search)
        CREATE TABLE IF NOT EXISTS sdm_excerpts (
            id INTEGER PRIMARY KEY,
            chapter TEXT NOT NULL,
            section TEXT NOT NULL,
            title TEXT NOT NULL,
            content TEXT NOT NULL,
            keywords TEXT
        );
        
        -- FTS5 virtual table for full-text search
        CREATE VIRTUAL TABLE IF NOT EXISTS sdm_fts USING fts5(
            chapter, section, title, content, keywords
        );
        
        -- Indexes
        CREATE INDEX IF NOT EXISTS idx_vmcs_encoding ON vmcs_fields(encoding);
        CREATE INDEX IF NOT EXISTS idx_vmcs_category ON vmcs_fields(category);
        CREATE INDEX IF NOT EXISTS idx_exit_reason ON exit_reasons(reason_number);
        CREATE INDEX IF NOT EXISTS idx_msr_address ON msrs(address);
        CREATE INDEX IF NOT EXISTS idx_control_type ON vmx_controls(control_type);
    """)
    conn.commit()


# =============================================================================
# VMCS FIELDS — Intel SDM Appendix B (Complete)
# =============================================================================

VMCS_FIELDS = [
    # 16-bit Control Fields
    ("VIRTUAL_PROCESSOR_ID", 0x0000, "16", "control", "16-bit", "VPID"),
    ("POSTED_INTERRUPT_NOTIFICATION_VECTOR", 0x0002, "16", "control", "16-bit", "Posted-interrupt notification vector"),
    ("EPTP_INDEX", 0x0004, "16", "control", "16-bit", "EPTP index"),
    
    # 16-bit Guest-State Fields
    ("GUEST_ES_SELECTOR", 0x0800, "16", "guest_state", "selector", "Guest ES selector"),
    ("GUEST_CS_SELECTOR", 0x0802, "16", "guest_state", "selector", "Guest CS selector"),
    ("GUEST_SS_SELECTOR", 0x0804, "16", "guest_state", "selector", "Guest SS selector"),
    ("GUEST_DS_SELECTOR", 0x0806, "16", "guest_state", "selector", "Guest DS selector"),
    ("GUEST_FS_SELECTOR", 0x0808, "16", "guest_state", "selector", "Guest FS selector"),
    ("GUEST_GS_SELECTOR", 0x080A, "16", "guest_state", "selector", "Guest GS selector"),
    ("GUEST_LDTR_SELECTOR", 0x080C, "16", "guest_state", "selector", "Guest LDTR selector"),
    ("GUEST_TR_SELECTOR", 0x080E, "16", "guest_state", "selector", "Guest TR selector"),
    ("GUEST_INTERRUPT_STATUS", 0x0810, "16", "guest_state", "interrupt", "Guest interrupt status"),
    ("GUEST_PML_INDEX", 0x0812, "16", "guest_state", "pml", "PML index"),
    
    # 16-bit Host-State Fields
    ("HOST_ES_SELECTOR", 0x0C00, "16", "host_state", "selector", "Host ES selector"),
    ("HOST_CS_SELECTOR", 0x0C02, "16", "host_state", "selector", "Host CS selector"),
    ("HOST_SS_SELECTOR", 0x0C04, "16", "host_state", "selector", "Host SS selector"),
    ("HOST_DS_SELECTOR", 0x0C06, "16", "host_state", "selector", "Host DS selector"),
    ("HOST_FS_SELECTOR", 0x0C08, "16", "host_state", "selector", "Host FS selector"),
    ("HOST_GS_SELECTOR", 0x0C0A, "16", "host_state", "selector", "Host GS selector"),
    ("HOST_TR_SELECTOR", 0x0C0C, "16", "host_state", "selector", "Host TR selector"),
    
    # 64-bit Control Fields
    ("IO_BITMAP_A", 0x2000, "64", "control", "64-bit", "I/O bitmap A address"),
    ("IO_BITMAP_B", 0x2002, "64", "control", "64-bit", "I/O bitmap B address"),
    ("MSR_BITMAP", 0x2004, "64", "control", "64-bit", "MSR bitmap address"),
    ("VM_EXIT_MSR_STORE_ADDR", 0x2006, "64", "control", "64-bit", "VM-exit MSR-store address"),
    ("VM_EXIT_MSR_LOAD_ADDR", 0x2008, "64", "control", "64-bit", "VM-exit MSR-load address"),
    ("VM_ENTRY_MSR_LOAD_ADDR", 0x200A, "64", "control", "64-bit", "VM-entry MSR-load address"),
    ("EXECUTIVE_VMCS_PTR", 0x200C, "64", "control", "64-bit", "Executive-VMCS pointer"),
    ("PML_ADDRESS", 0x200E, "64", "control", "64-bit", "PML address"),
    ("TSC_OFFSET", 0x2010, "64", "control", "64-bit", "TSC offset"),
    ("VIRTUAL_APIC_PAGE_ADDR", 0x2012, "64", "control", "64-bit", "Virtual-APIC address"),
    ("APIC_ACCESS_ADDR", 0x2014, "64", "control", "64-bit", "APIC-access address"),
    ("POSTED_INTERRUPT_DESC_ADDR", 0x2016, "64", "control", "64-bit", "Posted-interrupt descriptor address"),
    ("VM_FUNCTION_CONTROLS", 0x2018, "64", "control", "64-bit", "VM-function controls"),
    ("EPT_POINTER", 0x201A, "64", "control", "64-bit", "EPT pointer (EPTP)"),
    ("EOI_EXIT_BITMAP_0", 0x201C, "64", "control", "64-bit", "EOI-exit bitmap 0"),
    ("EOI_EXIT_BITMAP_1", 0x201E, "64", "control", "64-bit", "EOI-exit bitmap 1"),
    ("EOI_EXIT_BITMAP_2", 0x2020, "64", "control", "64-bit", "EOI-exit bitmap 2"),
    ("EOI_EXIT_BITMAP_3", 0x2022, "64", "control", "64-bit", "EOI-exit bitmap 3"),
    ("EPTP_LIST_ADDRESS", 0x2024, "64", "control", "64-bit", "EPTP-list address"),
    ("VMREAD_BITMAP_ADDRESS", 0x2026, "64", "control", "64-bit", "VMREAD-bitmap address"),
    ("VMWRITE_BITMAP_ADDRESS", 0x2028, "64", "control", "64-bit", "VMWRITE-bitmap address"),
    ("VIRTUALIZATION_EXCEPTION_INFO_ADDR", 0x202A, "64", "control", "64-bit", "Virtualization-exception information address"),
    ("XSS_EXITING_BITMAP", 0x202C, "64", "control", "64-bit", "XSS-exiting bitmap"),
    ("ENCLS_EXITING_BITMAP", 0x202E, "64", "control", "64-bit", "ENCLS-exiting bitmap"),
    ("SUB_PAGE_PERMISSION_TABLE_PTR", 0x2030, "64", "control", "64-bit", "Sub-page-permission-table pointer"),
    ("TSC_MULTIPLIER", 0x2032, "64", "control", "64-bit", "TSC multiplier"),
    ("TERTIARY_PROC_BASED_VM_EXEC_CONTROL", 0x2034, "64", "control", "64-bit", "Tertiary processor-based VM-execution controls"),
    ("ENCLV_EXITING_BITMAP", 0x2036, "64", "control", "64-bit", "ENCLV-exiting bitmap"),
    
    # 64-bit Read-Only Data Fields
    ("GUEST_PHYSICAL_ADDRESS", 0x2400, "64", "exit_info", "64-bit", "Guest-physical address"),
    
    # 64-bit Guest-State Fields
    ("VMCS_LINK_POINTER", 0x2800, "64", "guest_state", "64-bit", "VMCS link pointer"),
    ("GUEST_IA32_DEBUGCTL", 0x2802, "64", "guest_state", "64-bit", "Guest IA32_DEBUGCTL"),
    ("GUEST_IA32_PAT", 0x2804, "64", "guest_state", "64-bit", "Guest IA32_PAT"),
    ("GUEST_IA32_EFER", 0x2806, "64", "guest_state", "64-bit", "Guest IA32_EFER"),
    ("GUEST_IA32_PERF_GLOBAL_CTRL", 0x2808, "64", "guest_state", "64-bit", "Guest IA32_PERF_GLOBAL_CTRL"),
    ("GUEST_PDPTE0", 0x280A, "64", "guest_state", "64-bit", "Guest PDPTE0"),
    ("GUEST_PDPTE1", 0x280C, "64", "guest_state", "64-bit", "Guest PDPTE1"),
    ("GUEST_PDPTE2", 0x280E, "64", "guest_state", "64-bit", "Guest PDPTE2"),
    ("GUEST_PDPTE3", 0x2810, "64", "guest_state", "64-bit", "Guest PDPTE3"),
    ("GUEST_IA32_BNDCFGS", 0x2812, "64", "guest_state", "64-bit", "Guest IA32_BNDCFGS"),
    ("GUEST_IA32_RTIT_CTL", 0x2814, "64", "guest_state", "64-bit", "Guest IA32_RTIT_CTL"),
    ("GUEST_IA32_PKRS", 0x2818, "64", "guest_state", "64-bit", "Guest IA32_PKRS"),
    
    # 64-bit Host-State Fields
    ("HOST_IA32_PAT", 0x2C00, "64", "host_state", "64-bit", "Host IA32_PAT"),
    ("HOST_IA32_EFER", 0x2C02, "64", "host_state", "64-bit", "Host IA32_EFER"),
    ("HOST_IA32_PERF_GLOBAL_CTRL", 0x2C04, "64", "host_state", "64-bit", "Host IA32_PERF_GLOBAL_CTRL"),
    ("HOST_IA32_PKRS", 0x2C06, "64", "host_state", "64-bit", "Host IA32_PKRS"),
    
    # 32-bit Control Fields
    ("PIN_BASED_VM_EXEC_CONTROL", 0x4000, "32", "control", "32-bit", "Pin-based VM-execution controls"),
    ("CPU_BASED_VM_EXEC_CONTROL", 0x4002, "32", "control", "32-bit", "Primary processor-based VM-execution controls"),
    ("EXCEPTION_BITMAP", 0x4004, "32", "control", "32-bit", "Exception bitmap"),
    ("PAGE_FAULT_ERROR_CODE_MASK", 0x4006, "32", "control", "32-bit", "Page-fault error-code mask"),
    ("PAGE_FAULT_ERROR_CODE_MATCH", 0x4008, "32", "control", "32-bit", "Page-fault error-code match"),
    ("CR3_TARGET_COUNT", 0x400A, "32", "control", "32-bit", "CR3-target count"),
    ("VM_EXIT_CONTROLS", 0x400C, "32", "control", "32-bit", "VM-exit controls"),
    ("VM_EXIT_MSR_STORE_COUNT", 0x400E, "32", "control", "32-bit", "VM-exit MSR-store count"),
    ("VM_EXIT_MSR_LOAD_COUNT", 0x4010, "32", "control", "32-bit", "VM-exit MSR-load count"),
    ("VM_ENTRY_CONTROLS", 0x4012, "32", "control", "32-bit", "VM-entry controls"),
    ("VM_ENTRY_MSR_LOAD_COUNT", 0x4014, "32", "control", "32-bit", "VM-entry MSR-load count"),
    ("VM_ENTRY_INTERRUPTION_INFO", 0x4016, "32", "control", "32-bit", "VM-entry interruption-information field"),
    ("VM_ENTRY_EXCEPTION_ERROR_CODE", 0x4018, "32", "control", "32-bit", "VM-entry exception error code"),
    ("VM_ENTRY_INSTRUCTION_LEN", 0x401A, "32", "control", "32-bit", "VM-entry instruction length"),
    ("TPR_THRESHOLD", 0x401C, "32", "control", "32-bit", "TPR threshold"),
    ("SECONDARY_VM_EXEC_CONTROL", 0x401E, "32", "control", "32-bit", "Secondary processor-based VM-execution controls"),
    ("PLE_GAP", 0x4020, "32", "control", "32-bit", "PLE_Gap"),
    ("PLE_WINDOW", 0x4022, "32", "control", "32-bit", "PLE_Window"),
    
    # 32-bit Read-Only Data Fields
    ("VM_INSTRUCTION_ERROR", 0x4400, "32", "exit_info", "32-bit", "VM-instruction error"),
    ("VM_EXIT_REASON", 0x4402, "32", "exit_info", "32-bit", "Exit reason"),
    ("VM_EXIT_INTERRUPTION_INFO", 0x4404, "32", "exit_info", "32-bit", "VM-exit interruption information"),
    ("VM_EXIT_INTERRUPTION_ERROR_CODE", 0x4406, "32", "exit_info", "32-bit", "VM-exit interruption error code"),
    ("IDT_VECTORING_INFO", 0x4408, "32", "exit_info", "32-bit", "IDT-vectoring information field"),
    ("IDT_VECTORING_ERROR_CODE", 0x440A, "32", "exit_info", "32-bit", "IDT-vectoring error code"),
    ("VM_EXIT_INSTRUCTION_LEN", 0x440C, "32", "exit_info", "32-bit", "VM-exit instruction length"),
    ("VM_EXIT_INSTRUCTION_INFO", 0x440E, "32", "exit_info", "32-bit", "VM-exit instruction information"),
    
    # 32-bit Guest-State Fields
    ("GUEST_ES_LIMIT", 0x4800, "32", "guest_state", "limit", "Guest ES limit"),
    ("GUEST_CS_LIMIT", 0x4802, "32", "guest_state", "limit", "Guest CS limit"),
    ("GUEST_SS_LIMIT", 0x4804, "32", "guest_state", "limit", "Guest SS limit"),
    ("GUEST_DS_LIMIT", 0x4806, "32", "guest_state", "limit", "Guest DS limit"),
    ("GUEST_FS_LIMIT", 0x4808, "32", "guest_state", "limit", "Guest FS limit"),
    ("GUEST_GS_LIMIT", 0x480A, "32", "guest_state", "limit", "Guest GS limit"),
    ("GUEST_LDTR_LIMIT", 0x480C, "32", "guest_state", "limit", "Guest LDTR limit"),
    ("GUEST_TR_LIMIT", 0x480E, "32", "guest_state", "limit", "Guest TR limit"),
    ("GUEST_GDTR_LIMIT", 0x4810, "32", "guest_state", "limit", "Guest GDTR limit"),
    ("GUEST_IDTR_LIMIT", 0x4812, "32", "guest_state", "limit", "Guest IDTR limit"),
    ("GUEST_ES_ACCESS_RIGHTS", 0x4814, "32", "guest_state", "access_rights", "Guest ES access rights"),
    ("GUEST_CS_ACCESS_RIGHTS", 0x4816, "32", "guest_state", "access_rights", "Guest CS access rights"),
    ("GUEST_SS_ACCESS_RIGHTS", 0x4818, "32", "guest_state", "access_rights", "Guest SS access rights"),
    ("GUEST_DS_ACCESS_RIGHTS", 0x481A, "32", "guest_state", "access_rights", "Guest DS access rights"),
    ("GUEST_FS_ACCESS_RIGHTS", 0x481C, "32", "guest_state", "access_rights", "Guest FS access rights"),
    ("GUEST_GS_ACCESS_RIGHTS", 0x481E, "32", "guest_state", "access_rights", "Guest GS access rights"),
    ("GUEST_LDTR_ACCESS_RIGHTS", 0x4820, "32", "guest_state", "access_rights", "Guest LDTR access rights"),
    ("GUEST_TR_ACCESS_RIGHTS", 0x4822, "32", "guest_state", "access_rights", "Guest TR access rights"),
    ("GUEST_INTERRUPTIBILITY_STATE", 0x4824, "32", "guest_state", "state", "Guest interruptibility state"),
    ("GUEST_ACTIVITY_STATE", 0x4826, "32", "guest_state", "state", "Guest activity state"),
    ("GUEST_SMBASE", 0x4828, "32", "guest_state", "state", "Guest SMBASE"),
    ("GUEST_IA32_SYSENTER_CS", 0x482A, "32", "guest_state", "sysenter", "Guest IA32_SYSENTER_CS"),
    ("GUEST_VMX_PREEMPTION_TIMER_VALUE", 0x482E, "32", "guest_state", "timer", "VMX-preemption timer value"),
    
    # 32-bit Host-State Field
    ("HOST_IA32_SYSENTER_CS", 0x4C00, "32", "host_state", "sysenter", "Host IA32_SYSENTER_CS"),
    
    # Natural-Width Control Fields
    ("CR0_GUEST_HOST_MASK", 0x6000, "natural", "control", "natural", "CR0 guest/host mask"),
    ("CR4_GUEST_HOST_MASK", 0x6002, "natural", "control", "natural", "CR4 guest/host mask"),
    ("CR0_READ_SHADOW", 0x6004, "natural", "control", "natural", "CR0 read shadow"),
    ("CR4_READ_SHADOW", 0x6006, "natural", "control", "natural", "CR4 read shadow"),
    ("CR3_TARGET_VALUE_0", 0x6008, "natural", "control", "natural", "CR3-target value 0"),
    ("CR3_TARGET_VALUE_1", 0x600A, "natural", "control", "natural", "CR3-target value 1"),
    ("CR3_TARGET_VALUE_2", 0x600C, "natural", "control", "natural", "CR3-target value 2"),
    ("CR3_TARGET_VALUE_3", 0x600E, "natural", "control", "natural", "CR3-target value 3"),
    
    # Natural-Width Read-Only Data Fields
    ("EXIT_QUALIFICATION", 0x6400, "natural", "exit_info", "natural", "Exit qualification"),
    ("IO_RCX", 0x6402, "natural", "exit_info", "natural", "I/O RCX"),
    ("IO_RSI", 0x6404, "natural", "exit_info", "natural", "I/O RSI"),
    ("IO_RDI", 0x6406, "natural", "exit_info", "natural", "I/O RDI"),
    ("IO_RIP", 0x6408, "natural", "exit_info", "natural", "I/O RIP"),
    ("GUEST_LINEAR_ADDRESS", 0x640A, "natural", "exit_info", "natural", "Guest-linear address"),
    
    # Natural-Width Guest-State Fields
    ("GUEST_CR0", 0x6800, "natural", "guest_state", "cr", "Guest CR0"),
    ("GUEST_CR3", 0x6802, "natural", "guest_state", "cr", "Guest CR3"),
    ("GUEST_CR4", 0x6804, "natural", "guest_state", "cr", "Guest CR4"),
    ("GUEST_ES_BASE", 0x6806, "natural", "guest_state", "base", "Guest ES base"),
    ("GUEST_CS_BASE", 0x6808, "natural", "guest_state", "base", "Guest CS base"),
    ("GUEST_SS_BASE", 0x680A, "natural", "guest_state", "base", "Guest SS base"),
    ("GUEST_DS_BASE", 0x680C, "natural", "guest_state", "base", "Guest DS base"),
    ("GUEST_FS_BASE", 0x680E, "natural", "guest_state", "base", "Guest FS base"),
    ("GUEST_GS_BASE", 0x6810, "natural", "guest_state", "base", "Guest GS base"),
    ("GUEST_LDTR_BASE", 0x6812, "natural", "guest_state", "base", "Guest LDTR base"),
    ("GUEST_TR_BASE", 0x6814, "natural", "guest_state", "base", "Guest TR base"),
    ("GUEST_GDTR_BASE", 0x6816, "natural", "guest_state", "base", "Guest GDTR base"),
    ("GUEST_IDTR_BASE", 0x6818, "natural", "guest_state", "base", "Guest IDTR base"),
    ("GUEST_DR7", 0x681A, "natural", "guest_state", "dr", "Guest DR7"),
    ("GUEST_RSP", 0x681C, "natural", "guest_state", "gpr", "Guest RSP"),
    ("GUEST_RIP", 0x681E, "natural", "guest_state", "gpr", "Guest RIP"),
    ("GUEST_RFLAGS", 0x6820, "natural", "guest_state", "flags", "Guest RFLAGS"),
    ("GUEST_PENDING_DEBUG_EXCEPTIONS", 0x6822, "natural", "guest_state", "debug", "Guest pending debug exceptions"),
    ("GUEST_IA32_SYSENTER_ESP", 0x6824, "natural", "guest_state", "sysenter", "Guest IA32_SYSENTER_ESP"),
    ("GUEST_IA32_SYSENTER_EIP", 0x6826, "natural", "guest_state", "sysenter", "Guest IA32_SYSENTER_EIP"),
    ("GUEST_IA32_S_CET", 0x6828, "natural", "guest_state", "cet", "Guest IA32_S_CET"),
    ("GUEST_SSP", 0x682A, "natural", "guest_state", "cet", "Guest SSP"),
    ("GUEST_IA32_INTERRUPT_SSP_TABLE_ADDR", 0x682C, "natural", "guest_state", "cet", "Guest IA32_INTERRUPT_SSP_TABLE_ADDR"),
    
    # Natural-Width Host-State Fields
    ("HOST_CR0", 0x6C00, "natural", "host_state", "cr", "Host CR0"),
    ("HOST_CR3", 0x6C02, "natural", "host_state", "cr", "Host CR3"),
    ("HOST_CR4", 0x6C04, "natural", "host_state", "cr", "Host CR4"),
    ("HOST_FS_BASE", 0x6C06, "natural", "host_state", "base", "Host FS base"),
    ("HOST_GS_BASE", 0x6C08, "natural", "host_state", "base", "Host GS base"),
    ("HOST_TR_BASE", 0x6C0A, "natural", "host_state", "base", "Host TR base"),
    ("HOST_GDTR_BASE", 0x6C0C, "natural", "host_state", "base", "Host GDTR base"),
    ("HOST_IDTR_BASE", 0x6C0E, "natural", "host_state", "base", "Host IDTR base"),
    ("HOST_IA32_SYSENTER_ESP", 0x6C10, "natural", "host_state", "sysenter", "Host IA32_SYSENTER_ESP"),
    ("HOST_IA32_SYSENTER_EIP", 0x6C12, "natural", "host_state", "sysenter", "Host IA32_SYSENTER_EIP"),
    ("HOST_RSP", 0x6C14, "natural", "host_state", "gpr", "Host RSP"),
    ("HOST_RIP", 0x6C16, "natural", "host_state", "gpr", "Host RIP"),
    ("HOST_IA32_S_CET", 0x6C18, "natural", "host_state", "cet", "Host IA32_S_CET"),
    ("HOST_SSP", 0x6C1A, "natural", "host_state", "cet", "Host SSP"),
    ("HOST_IA32_INTERRUPT_SSP_TABLE_ADDR", 0x6C1C, "natural", "host_state", "cet", "Host IA32_INTERRUPT_SSP_TABLE_ADDR"),
]


# =============================================================================
# EXIT REASONS — Intel SDM Appendix C, Table C-1
# =============================================================================

EXIT_REASONS = [
    (0, "EXCEPTION_OR_NMI", 1, "Exception or NMI", "Check interruption info for vector and type"),
    (1, "EXTERNAL_INTERRUPT", 0, "External interrupt", "Acknowledge interrupt if control set"),
    (2, "TRIPLE_FAULT", 0, "Triple fault", "Fatal - shutdown guest"),
    (3, "INIT_SIGNAL", 0, "INIT signal", "Usually blocked in VMX operation"),
    (4, "SIPI", 0, "Startup IPI (SIPI)", "Target AP wakeup"),
    (5, "IO_SMI", 0, "I/O SMI", "SMI triggered by I/O port access"),
    (6, "OTHER_SMI", 0, "Other SMI", "Non-I/O SMI"),
    (7, "INTERRUPT_WINDOW", 0, "Interrupt window", "Interrupts can now be delivered"),
    (8, "NMI_WINDOW", 0, "NMI window", "NMIs can now be delivered"),
    (9, "TASK_SWITCH", 1, "Task switch", "Emulate task switch"),
    (10, "CPUID", 0, "CPUID", "Spoof CPUID results for stealth"),
    (11, "GETSEC", 0, "GETSEC", "SGX instruction"),
    (12, "HLT", 0, "HLT", "Idle - schedule another vcpu"),
    (13, "INVD", 0, "INVD", "Cache invalidate - dangerous"),
    (14, "INVLPG", 1, "INVLPG", "TLB invalidate single page"),
    (15, "RDPMC", 0, "RDPMC", "Read performance counter"),
    (16, "RDTSC", 0, "RDTSC", "Apply TSC offset for stealth"),
    (17, "RSM", 0, "RSM", "Return from SMM"),
    (18, "VMCALL", 0, "VMCALL", "Hypercall - handle or inject #UD"),
    (19, "VMCLEAR", 0, "VMCLEAR", "Nested: clear shadow VMCS"),
    (20, "VMLAUNCH", 0, "VMLAUNCH", "Nested: launch nested guest"),
    (21, "VMPTRLD", 0, "VMPTRLD", "Nested: load VMCS pointer"),
    (22, "VMPTRST", 0, "VMPTRST", "Nested: store VMCS pointer"),
    (23, "VMREAD", 0, "VMREAD", "Nested: read shadow VMCS"),
    (24, "VMRESUME", 0, "VMRESUME", "Nested: resume nested guest"),
    (25, "VMWRITE", 0, "VMWRITE", "Nested: write shadow VMCS"),
    (26, "VMXOFF", 0, "VMXOFF", "Nested: disable nested VMX"),
    (27, "VMXON", 0, "VMXON", "Nested: enable nested VMX"),
    (28, "CR_ACCESS", 1, "Control-register access", "Check qualification for CR#, type"),
    (29, "DR_ACCESS", 1, "MOV DR", "Debug register access"),
    (30, "IO_INSTRUCTION", 1, "I/O instruction", "Port, size, direction in qualification"),
    (31, "RDMSR", 0, "RDMSR", "Check ECX for MSR number"),
    (32, "WRMSR", 0, "WRMSR", "Validate MSR write"),
    (33, "VM_ENTRY_FAILURE_INVALID_GUEST_STATE", 0, "Entry fail: invalid guest state", "Check guest state fields"),
    (34, "VM_ENTRY_FAILURE_MSR_LOADING", 0, "Entry fail: MSR loading", "Check MSR-load area"),
    (36, "MWAIT", 0, "MWAIT", "Monitor wait"),
    (37, "MONITOR_TRAP_FLAG", 0, "Monitor trap flag", "Single-step for debugging"),
    (39, "MONITOR", 0, "MONITOR", "Monitor instruction"),
    (40, "PAUSE", 0, "PAUSE", "Pause loop exiting"),
    (41, "VM_ENTRY_FAILURE_MACHINE_CHECK", 0, "Entry fail: machine check", "MCE during entry"),
    (43, "TPR_BELOW_THRESHOLD", 0, "TPR below threshold", "APIC TPR decreased"),
    (44, "APIC_ACCESS", 1, "APIC access", "Virtualize or emulate"),
    (45, "VIRTUALIZED_EOI", 0, "Virtualized EOI", "EOI virtualization"),
    (46, "GDTR_IDTR_ACCESS", 1, "GDTR/IDTR access", "Descriptor table access"),
    (47, "LDTR_TR_ACCESS", 1, "LDTR/TR access", "Descriptor table access"),
    (48, "EPT_VIOLATION", 1, "EPT violation", "Access type and permissions in qualification"),
    (49, "EPT_MISCONFIGURATION", 0, "EPT misconfiguration", "Invalid EPT entry"),
    (50, "INVEPT", 0, "INVEPT", "Invalidate EPT"),
    (51, "RDTSCP", 0, "RDTSCP", "RDTSC + processor ID"),
    (52, "VMX_PREEMPTION_TIMER_EXPIRED", 0, "Preemption timer", "Timer expired"),
    (53, "INVVPID", 0, "INVVPID", "Invalidate VPID"),
    (54, "WBINVD_WBNOINVD", 0, "WBINVD/WBNOINVD", "Cache writeback"),
    (55, "XSETBV", 0, "XSETBV", "Set extended control register"),
    (56, "APIC_WRITE", 0, "APIC write", "Write to APIC page"),
    (57, "RDRAND", 0, "RDRAND", "Random number"),
    (58, "INVPCID", 0, "INVPCID", "Invalidate PCID"),
    (59, "VMFUNC", 0, "VMFUNC", "VM function"),
    (60, "ENCLS", 0, "ENCLS", "SGX enclave instruction"),
    (61, "RDSEED", 0, "RDSEED", "Seed random number"),
    (62, "PML_FULL", 0, "Page-modification log full", "PML buffer full"),
    (63, "XSAVES", 0, "XSAVES", "Save extended state"),
    (64, "XRSTORS", 0, "XRSTORS", "Restore extended state"),
    (67, "UMWAIT", 0, "UMWAIT", "User-mode wait"),
    (68, "TPAUSE", 0, "TPAUSE", "Timed pause"),
    (74, "BUS_LOCK", 0, "Bus lock", "Split-lock or bus lock"),
    (75, "NOTIFY", 0, "Notify", "Notify VM exit"),
]


# =============================================================================
# MSRs — VMX-Related Model Specific Registers
# =============================================================================

MSRS = [
    # VMX Capability MSRs
    ("IA32_VMX_BASIC", 0x480, "vmx_capability", "Basic VMX capabilities (revision, VMCS size, memory type)"),
    ("IA32_VMX_PINBASED_CTLS", 0x481, "vmx_capability", "Allowed pin-based VM-execution controls"),
    ("IA32_VMX_PROCBASED_CTLS", 0x482, "vmx_capability", "Allowed primary processor-based controls"),
    ("IA32_VMX_EXIT_CTLS", 0x483, "vmx_capability", "Allowed VM-exit controls"),
    ("IA32_VMX_ENTRY_CTLS", 0x484, "vmx_capability", "Allowed VM-entry controls"),
    ("IA32_VMX_MISC", 0x485, "vmx_capability", "Miscellaneous VMX data"),
    ("IA32_VMX_CR0_FIXED0", 0x486, "vmx_capability", "CR0 bits fixed to 0 in VMX operation"),
    ("IA32_VMX_CR0_FIXED1", 0x487, "vmx_capability", "CR0 bits fixed to 1 in VMX operation"),
    ("IA32_VMX_CR4_FIXED0", 0x488, "vmx_capability", "CR4 bits fixed to 0 in VMX operation"),
    ("IA32_VMX_CR4_FIXED1", 0x489, "vmx_capability", "CR4 bits fixed to 1 in VMX operation"),
    ("IA32_VMX_VMCS_ENUM", 0x48A, "vmx_capability", "VMCS field enumeration"),
    ("IA32_VMX_PROCBASED_CTLS2", 0x48B, "vmx_capability", "Allowed secondary processor-based controls"),
    ("IA32_VMX_EPT_VPID_CAP", 0x48C, "vmx_capability", "EPT and VPID capabilities"),
    ("IA32_VMX_TRUE_PINBASED_CTLS", 0x48D, "vmx_capability", "True pin-based controls (if supported)"),
    ("IA32_VMX_TRUE_PROCBASED_CTLS", 0x48E, "vmx_capability", "True processor-based controls"),
    ("IA32_VMX_TRUE_EXIT_CTLS", 0x48F, "vmx_capability", "True VM-exit controls"),
    ("IA32_VMX_TRUE_ENTRY_CTLS", 0x490, "vmx_capability", "True VM-entry controls"),
    ("IA32_VMX_VMFUNC", 0x491, "vmx_capability", "VM functions"),
    ("IA32_VMX_PROCBASED_CTLS3", 0x492, "vmx_capability", "Tertiary processor-based controls"),
    
    # Feature Control
    ("IA32_FEATURE_CONTROL", 0x3A, "feature", "Feature control (lock, VMX enable)"),
    
    # System MSRs
    ("IA32_SYSENTER_CS", 0x174, "system", "SYSENTER CS"),
    ("IA32_SYSENTER_ESP", 0x175, "system", "SYSENTER ESP"),
    ("IA32_SYSENTER_EIP", 0x176, "system", "SYSENTER EIP"),
    ("IA32_DEBUGCTL", 0x1D9, "debug", "Debug control"),
    ("IA32_PAT", 0x277, "memory", "Page Attribute Table"),
    ("IA32_EFER", 0xC0000080, "system", "Extended Feature Enable Register (LME, LMA, NXE)"),
    ("IA32_STAR", 0xC0000081, "system", "SYSCALL target address"),
    ("IA32_LSTAR", 0xC0000082, "system", "SYSCALL 64-bit target"),
    ("IA32_CSTAR", 0xC0000083, "system", "SYSCALL compatibility target"),
    ("IA32_FMASK", 0xC0000084, "system", "SYSCALL EFLAGS mask"),
    ("IA32_FS_BASE", 0xC0000100, "segment", "FS segment base"),
    ("IA32_GS_BASE", 0xC0000101, "segment", "GS segment base"),
    ("IA32_KERNEL_GS_BASE", 0xC0000102, "segment", "Kernel GS base (swapgs)"),
    ("IA32_TSC_AUX", 0xC0000103, "timing", "TSC auxiliary (processor ID for RDTSCP)"),
    
    # APIC
    ("IA32_APIC_BASE", 0x1B, "apic", "APIC base address and enable"),
]


# =============================================================================
# EXCEPTIONS — Interrupt/Exception Vectors
# =============================================================================

EXCEPTIONS = [
    (0, "#DE", "Divide Error", "fault", 0, "DIV/IDIV by zero"),
    (1, "#DB", "Debug", "fault/trap", 0, "Debug breakpoint"),
    (2, "NMI", "Non-Maskable Interrupt", "interrupt", 0, "External NMI"),
    (3, "#BP", "Breakpoint", "trap", 0, "INT3"),
    (4, "#OF", "Overflow", "trap", 0, "INTO with OF=1"),
    (5, "#BR", "BOUND Range Exceeded", "fault", 0, "BOUND instruction"),
    (6, "#UD", "Invalid Opcode", "fault", 0, "Undefined opcode"),
    (7, "#NM", "Device Not Available", "fault", 0, "FPU not available"),
    (8, "#DF", "Double Fault", "abort", 1, "Exception during exception"),
    (10, "#TS", "Invalid TSS", "fault", 1, "Task switch/TSS access"),
    (11, "#NP", "Segment Not Present", "fault", 1, "Loading segment register"),
    (12, "#SS", "Stack-Segment Fault", "fault", 1, "Stack limit violation"),
    (13, "#GP", "General Protection", "fault", 1, "Memory protection violation"),
    (14, "#PF", "Page Fault", "fault", 1, "Page not present/access violation"),
    (16, "#MF", "x87 FPU Error", "fault", 0, "x87 floating-point error"),
    (17, "#AC", "Alignment Check", "fault", 1, "Unaligned access"),
    (18, "#MC", "Machine Check", "abort", 0, "Hardware error"),
    (19, "#XM", "SIMD Floating-Point", "fault", 0, "SIMD exception"),
    (20, "#VE", "Virtualization Exception", "fault", 0, "EPT violation #VE"),
    (21, "#CP", "Control Protection", "fault", 1, "Control flow violation"),
]


def populate_database(conn):
    """Populate all database tables."""
    
    # VMCS Fields
    conn.executemany(
        "INSERT INTO vmcs_fields (name, encoding, width, category, subcategory, description) VALUES (?, ?, ?, ?, ?, ?)",
        VMCS_FIELDS
    )
    print(f"  ✓ {len(VMCS_FIELDS)} VMCS fields")
    
    # Exit Reasons
    conn.executemany(
        "INSERT INTO exit_reasons (reason_number, name, has_qualification, description, handling_notes) VALUES (?, ?, ?, ?, ?)",
        EXIT_REASONS
    )
    print(f"  ✓ {len(EXIT_REASONS)} exit reasons")
    
    # MSRs
    conn.executemany(
        "INSERT INTO msrs (name, address, category, description) VALUES (?, ?, ?, ?)",
        MSRS
    )
    print(f"  ✓ {len(MSRS)} MSRs")
    
    # Exceptions
    conn.executemany(
        "INSERT INTO exceptions (vector, mnemonic, name, type, error_code, description) VALUES (?, ?, ?, ?, ?, ?)",
        EXCEPTIONS
    )
    print(f"  ✓ {len(EXCEPTIONS)} exceptions")
    
    conn.commit()


def generate_header(conn, path: Path):
    """Generate vmcs_fields.h C header from database."""
    
    cursor = conn.execute("SELECT name, encoding, width, category, description FROM vmcs_fields ORDER BY encoding")
    
    lines = [
        "/**",
        " * vmcs_fields.h — VMCS Field Encodings",
        " * Auto-generated from Intel SDM Vol 3, Appendix B",
        " */",
        "",
        "#ifndef VMCS_FIELDS_H",
        "#define VMCS_FIELDS_H",
        "",
    ]
    
    current_cat = None
    for name, encoding, width, category, desc in cursor:
        if category != current_cat:
            lines.append("")
            lines.append(f"// === {category.upper().replace('_', ' ')} ===")
            current_cat = category
        lines.append(f"#define VMCS_{name} 0x{encoding:04X}")
    
    lines.extend(["", "#endif // VMCS_FIELDS_H", ""])
    
    path.write_text("\n".join(lines))
    print(f"  ✓ Generated {path}")


def main():
    """Main entry point."""
    
    print("=" * 60)
    print("Intel SDM Database Builder for OmbraMCP")
    print("=" * 60)
    print()
    
    # Create directories
    DATA_DIR.mkdir(parents=True, exist_ok=True)
    
    # Remove old database
    if DB_PATH.exists():
        DB_PATH.unlink()
    
    # Create and populate database
    conn = sqlite3.connect(DB_PATH)
    
    print("Creating schema...")
    create_schema(conn)
    
    print("Populating database...")
    populate_database(conn)
    
    print("Generating C header...")
    generate_header(conn, DATA_DIR / "vmcs_fields.h")
    
    conn.close()
    
    print()
    print(f"Database: {DB_PATH}")
    print(f"Size: {DB_PATH.stat().st_size / 1024:.1f} KB")
    print()
    print("Done! ✓")


if __name__ == "__main__":
    main()
