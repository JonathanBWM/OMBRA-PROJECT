#!/usr/bin/env python3
"""
Populate vmx_controls table with all VMX control bits from Intel SDM Chapter 24.
"""

import sqlite3
from pathlib import Path

# Database path
DB_PATH = Path(__file__).parent.parent / "src/ombra_mcp/data/intel_sdm.db"

# VMX Control Bits Data
VMX_CONTROLS = [
    # Pin-Based VM-Execution Controls (MSR 0x481)
    ("pin_based", 0, "External-interrupt exiting",
     "VM exits occur on external interrupts", "flexible", "IA32_VMX_PINBASED_CTLS (0x481)"),
    ("pin_based", 3, "NMI exiting",
     "VM exits occur on non-maskable interrupts (NMIs)", "flexible", "IA32_VMX_PINBASED_CTLS (0x481)"),
    ("pin_based", 5, "Virtual NMIs",
     "NMIs are never blocked in VMX non-root operation", "flexible", "IA32_VMX_PINBASED_CTLS (0x481)"),
    ("pin_based", 6, "Activate VMX-preemption timer",
     "VMX-preemption timer counts down in VMX non-root operation", "flexible", "IA32_VMX_PINBASED_CTLS (0x481)"),
    ("pin_based", 7, "Process posted interrupts",
     "Processor treats interrupts with posted-interrupt notification vector specially", "flexible", "IA32_VMX_PINBASED_CTLS (0x481)"),

    # Primary Processor-Based VM-Execution Controls (MSR 0x482)
    ("proc_based", 2, "Interrupt-window exiting",
     "VM exit when RFLAGS.IF=1 and no blocking by STI or MOV SS", "flexible", "IA32_VMX_PROCBASED_CTLS (0x482)"),
    ("proc_based", 3, "Use TSC offsetting",
     "RDTSC, RDTSCP, and RDMSR reading IA32_TIME_STAMP_COUNTER return value + TSC offset", "flexible", "IA32_VMX_PROCBASED_CTLS (0x482)"),
    ("proc_based", 7, "HLT exiting",
     "VM exit on HLT instruction", "flexible", "IA32_VMX_PROCBASED_CTLS (0x482)"),
    ("proc_based", 9, "INVLPG exiting",
     "VM exit on INVLPG instruction", "flexible", "IA32_VMX_PROCBASED_CTLS (0x482)"),
    ("proc_based", 10, "MWAIT exiting",
     "VM exit on MWAIT instruction", "flexible", "IA32_VMX_PROCBASED_CTLS (0x482)"),
    ("proc_based", 11, "RDPMC exiting",
     "VM exit on RDPMC instruction", "flexible", "IA32_VMX_PROCBASED_CTLS (0x482)"),
    ("proc_based", 12, "RDTSC exiting",
     "VM exit on RDTSC instruction", "flexible", "IA32_VMX_PROCBASED_CTLS (0x482)"),
    ("proc_based", 15, "CR3-load exiting",
     "VM exit on MOV to CR3", "flexible", "IA32_VMX_PROCBASED_CTLS (0x482)"),
    ("proc_based", 16, "CR3-store exiting",
     "VM exit on MOV from CR3", "flexible", "IA32_VMX_PROCBASED_CTLS (0x482)"),
    ("proc_based", 19, "CR8-load exiting",
     "VM exit on MOV to CR8", "flexible", "IA32_VMX_PROCBASED_CTLS (0x482)"),
    ("proc_based", 20, "CR8-store exiting",
     "VM exit on MOV from CR8", "flexible", "IA32_VMX_PROCBASED_CTLS (0x482)"),
    ("proc_based", 21, "Use TPR shadow",
     "Enables TPR virtualization and other APIC-virtualization features", "flexible", "IA32_VMX_PROCBASED_CTLS (0x482)"),
    ("proc_based", 22, "NMI-window exiting",
     "VM exit when no virtual-NMI blocking", "flexible", "IA32_VMX_PROCBASED_CTLS (0x482)"),
    ("proc_based", 23, "MOV-DR exiting",
     "VM exit on MOV to/from debug registers", "flexible", "IA32_VMX_PROCBASED_CTLS (0x482)"),
    ("proc_based", 24, "Unconditional I/O exiting",
     "VM exit on all IN, INS, OUT, OUTS instructions", "flexible", "IA32_VMX_PROCBASED_CTLS (0x482)"),
    ("proc_based", 25, "Use I/O bitmaps",
     "I/O bitmaps control VM exits on I/O instructions", "flexible", "IA32_VMX_PROCBASED_CTLS (0x482)"),
    ("proc_based", 27, "Monitor trap flag",
     "VM exit after each instruction if RFLAGS.TF=1", "flexible", "IA32_VMX_PROCBASED_CTLS (0x482)"),
    ("proc_based", 28, "Use MSR bitmaps",
     "MSR bitmaps control VM exits on RDMSR and WRMSR", "flexible", "IA32_VMX_PROCBASED_CTLS (0x482)"),
    ("proc_based", 29, "MONITOR exiting",
     "VM exit on MONITOR instruction", "flexible", "IA32_VMX_PROCBASED_CTLS (0x482)"),
    ("proc_based", 30, "PAUSE exiting",
     "VM exit on PAUSE instruction", "flexible", "IA32_VMX_PROCBASED_CTLS (0x482)"),
    ("proc_based", 31, "Activate secondary controls",
     "Determines whether secondary processor-based VM-execution controls are used", "flexible", "IA32_VMX_PROCBASED_CTLS (0x482)"),

    # Secondary Processor-Based VM-Execution Controls (MSR 0x48B)
    ("proc_based2", 0, "Virtualize APIC accesses",
     "Virtualizes memory-mapped APIC accesses", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 1, "Enable EPT",
     "Enables Extended Page Tables for guest physical address translation", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 2, "Descriptor-table exiting",
     "VM exit on LGDT, LIDT, LLDT, LTR, SGDT, SIDT, SLDT, STR", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 3, "Enable RDTSCP",
     "Enables RDTSCP instruction in VMX non-root operation", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 4, "Virtualize x2APIC mode",
     "Virtualizes x2APIC MSR accesses", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 5, "Enable VPID",
     "Enables Virtual Processor Identifiers", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 6, "WBINVD exiting",
     "VM exit on WBINVD instruction", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 7, "Unrestricted guest",
     "Guest can run in unpaged protected mode or real mode", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 8, "APIC-register virtualization",
     "Enables virtualization of APIC register accesses via virtual-APIC page", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 9, "Virtual-interrupt delivery",
     "Enables evaluation and delivery of virtual interrupts", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 10, "PAUSE-loop exiting",
     "VM exit if PAUSE executed in loop based on gap and window settings", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 11, "RDRAND exiting",
     "VM exit on RDRAND instruction", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 12, "Enable INVPCID",
     "Enables INVPCID instruction in VMX non-root operation", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 13, "Enable VM functions",
     "Enables VM functions facility (VMFUNC instruction)", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 14, "VMCS shadowing",
     "Enables VMCS shadowing for nested virtualization", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 15, "Enable ENCLS exiting",
     "VM exit on ENCLS instruction (SGX leaf functions)", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 16, "RDSEED exiting",
     "VM exit on RDSEED instruction", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 17, "Enable PML",
     "Enables Page-Modification Logging", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 18, "EPT-violation #VE",
     "EPT violations may cause virtualization exceptions (#VE) instead of VM exits", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 19, "Conceal VMX from PT",
     "Intel Processor Trace does not trace VMX transitions", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 20, "Enable XSAVES/XRSTORS",
     "Enables XSAVES/XRSTORS instructions in VMX non-root operation", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 22, "Mode-based execute control for EPT",
     "EPT execute permissions depend on guest mode (supervisor vs user)", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 23, "Sub-page write permissions for EPT",
     "Enables sub-page (128-byte) write permissions for EPT", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 24, "Intel PT uses guest physical addresses",
     "Intel Processor Trace logs guest physical addresses instead of linear", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 25, "Use TSC scaling",
     "RDTSC, RDTSCP return (value + offset) * multiplier", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 26, "Enable user wait and pause",
     "Enables TPAUSE, UMONITOR, UMWAIT instructions in VMX non-root operation", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),
    ("proc_based2", 28, "Enable ENCLV exiting",
     "VM exit on ENCLV instruction (SGX leaf functions)", "flexible", "IA32_VMX_PROCBASED_CTLS2 (0x48B)"),

    # VM-Exit Controls (MSR 0x483)
    ("exit", 2, "Save debug controls",
     "DR7 and IA32_DEBUGCTL saved on VM exit", "flexible", "IA32_VMX_EXIT_CTLS (0x483)"),
    ("exit", 9, "Host address-space size",
     "Logical processor in 64-bit mode after VM exit (IA-32e mode host)", "1", "IA32_VMX_EXIT_CTLS (0x483)"),
    ("exit", 12, "Load IA32_PERF_GLOBAL_CTRL",
     "IA32_PERF_GLOBAL_CTRL loaded from host-state area on VM exit", "flexible", "IA32_VMX_EXIT_CTLS (0x483)"),
    ("exit", 15, "Acknowledge interrupt on exit",
     "Processor acknowledges external interrupt and saves vector to VM-exit interruption information", "flexible", "IA32_VMX_EXIT_CTLS (0x483)"),
    ("exit", 18, "Save IA32_PAT",
     "IA32_PAT saved on VM exit", "flexible", "IA32_VMX_EXIT_CTLS (0x483)"),
    ("exit", 19, "Load IA32_PAT",
     "IA32_PAT loaded from host-state area on VM exit", "flexible", "IA32_VMX_EXIT_CTLS (0x483)"),
    ("exit", 20, "Save IA32_EFER",
     "IA32_EFER saved on VM exit", "flexible", "IA32_VMX_EXIT_CTLS (0x483)"),
    ("exit", 21, "Load IA32_EFER",
     "IA32_EFER loaded from host-state area on VM exit", "flexible", "IA32_VMX_EXIT_CTLS (0x483)"),
    ("exit", 22, "Save VMX-preemption timer value",
     "VMX-preemption timer value saved on VM exit", "flexible", "IA32_VMX_EXIT_CTLS (0x483)"),
    ("exit", 23, "Clear IA32_BNDCFGS",
     "IA32_BNDCFGS cleared on VM exit", "flexible", "IA32_VMX_EXIT_CTLS (0x483)"),
    ("exit", 24, "Conceal VMX from PT",
     "Intel Processor Trace does not trace packets on VM exit", "flexible", "IA32_VMX_EXIT_CTLS (0x483)"),
    ("exit", 25, "Clear IA32_RTIT_CTL",
     "IA32_RTIT_CTL cleared on VM exit", "flexible", "IA32_VMX_EXIT_CTLS (0x483)"),

    # VM-Entry Controls (MSR 0x484)
    ("entry", 2, "Load debug controls",
     "DR7 and IA32_DEBUGCTL loaded from guest-state area on VM entry", "flexible", "IA32_VMX_ENTRY_CTLS (0x484)"),
    ("entry", 9, "IA-32e mode guest",
     "Guest is in IA-32e mode (64-bit mode) after VM entry", "flexible", "IA32_VMX_ENTRY_CTLS (0x484)"),
    ("entry", 10, "Entry to SMM",
     "VM entry is to System Management Mode (SMM)", "flexible", "IA32_VMX_ENTRY_CTLS (0x484)"),
    ("entry", 11, "Deactivate dual-monitor treatment",
     "VM entry returns from SMM to normal operation", "flexible", "IA32_VMX_ENTRY_CTLS (0x484)"),
    ("entry", 13, "Load IA32_PERF_GLOBAL_CTRL",
     "IA32_PERF_GLOBAL_CTRL loaded from guest-state area on VM entry", "flexible", "IA32_VMX_ENTRY_CTLS (0x484)"),
    ("entry", 14, "Load IA32_PAT",
     "IA32_PAT loaded from guest-state area on VM entry", "flexible", "IA32_VMX_ENTRY_CTLS (0x484)"),
    ("entry", 15, "Load IA32_EFER",
     "IA32_EFER loaded from guest-state area on VM entry", "flexible", "IA32_VMX_ENTRY_CTLS (0x484)"),
    ("entry", 16, "Load IA32_BNDCFGS",
     "IA32_BNDCFGS loaded from guest-state area on VM entry", "flexible", "IA32_VMX_ENTRY_CTLS (0x484)"),
    ("entry", 17, "Conceal VMX from PT",
     "Intel Processor Trace does not trace packets on VM entry", "flexible", "IA32_VMX_ENTRY_CTLS (0x484)"),
    ("entry", 18, "Load IA32_RTIT_CTL",
     "IA32_RTIT_CTL loaded from guest-state area on VM entry", "flexible", "IA32_VMX_ENTRY_CTLS (0x484)"),
]


def populate_vmx_controls():
    """Populate the vmx_controls table with all control bits."""

    if not DB_PATH.exists():
        print(f"ERROR: Database not found at {DB_PATH}")
        return

    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()

    # Clear existing data
    cursor.execute("DELETE FROM vmx_controls")
    print(f"Cleared existing vmx_controls data")

    # Insert all control bits
    cursor.executemany(
        """
        INSERT INTO vmx_controls
        (control_type, bit_position, name, description, default_setting, capability_msr)
        VALUES (?, ?, ?, ?, ?, ?)
        """,
        VMX_CONTROLS
    )

    conn.commit()

    # Verify insertion
    cursor.execute("SELECT COUNT(*) FROM vmx_controls")
    total_count = cursor.fetchone()[0]
    print(f"\nTotal control bits inserted: {total_count}")

    # Count per control type
    print("\nControl bits by type:")
    cursor.execute("""
        SELECT control_type, COUNT(*)
        FROM vmx_controls
        GROUP BY control_type
        ORDER BY control_type
    """)

    for control_type, count in cursor.fetchall():
        print(f"  {control_type}: {count} bits")

    # Show sample entries from each type
    print("\nSample entries per control type:")
    for control_type in ["pin_based", "proc_based", "proc_based2", "exit", "entry"]:
        cursor.execute("""
            SELECT bit_position, name
            FROM vmx_controls
            WHERE control_type = ?
            ORDER BY bit_position
            LIMIT 3
        """, (control_type,))

        print(f"\n  {control_type}:")
        for bit_pos, name in cursor.fetchall():
            print(f"    Bit {bit_pos}: {name}")

    conn.close()
    print(f"\n✓ vmx_controls table populated successfully!")


if __name__ == "__main__":
    populate_vmx_controls()
