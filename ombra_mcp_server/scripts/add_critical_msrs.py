#!/usr/bin/env python3
"""
Add critical MSRs for hypervisor development and anti-cheat evasion.
Focuses on timing, system, debug, and APIC MSRs.
"""

import sqlite3
import json
from pathlib import Path

DB_PATH = Path(__file__).parent.parent / "src" / "ombra_mcp" / "data" / "intel_sdm.db"

# Critical MSRs to add
CRITICAL_MSRS = [
    # Timing MSRs (CRITICAL for anti-cheat)
    {
        "name": "IA32_TIME_STAMP_COUNTER",
        "address": 0x10,
        "category": "Timing",
        "description": "Time Stamp Counter. Read by RDTSC/RDTSCP instructions. CRITICAL: Most anti-cheats monitor TSC for hypervisor detection via timing inconsistencies.",
        "bit_fields": json.dumps({
            "63:0": "Time Stamp Counter value"
        }),
        "sdm_section": "17.17"
    },
    {
        "name": "IA32_MPERF",
        "address": 0xE7,
        "category": "Timing",
        "description": "Maximum Performance Frequency Clock Count. CRITICAL: ESEA anti-cheat monitors APERF/MPERF ratio to detect hypervisors. Must be compensated alongside TSC.",
        "bit_fields": json.dumps({
            "63:0": "Maximum performance frequency clock count"
        }),
        "sdm_section": "14.2"
    },
    {
        "name": "IA32_APERF",
        "address": 0xE8,
        "category": "Timing",
        "description": "Actual Performance Frequency Clock Count. CRITICAL: ESEA anti-cheat monitors APERF/MPERF ratio to detect hypervisors. Must be compensated alongside TSC.",
        "bit_fields": json.dumps({
            "63:0": "Actual performance frequency clock count"
        }),
        "sdm_section": "14.2"
    },
    {
        "name": "IA32_TSC_AUX",
        "address": 0xC0000103,
        "category": "Timing",
        "description": "TSC Auxiliary register. Used by RDTSCP instruction. Contains processor ID on most systems. Must be properly virtualized.",
        "bit_fields": json.dumps({
            "31:0": "AUX value returned by RDTSCP",
            "63:32": "Reserved"
        }),
        "sdm_section": "17.17.2"
    },
    {
        "name": "IA32_TSC_ADJUST",
        "address": 0x3B,
        "category": "Timing",
        "description": "TSC adjustment value. Allows software to adjust the TSC without causing discontinuities. CRITICAL: Anti-cheats may monitor this for tampering.",
        "bit_fields": json.dumps({
            "63:0": "TSC adjustment (signed integer)"
        }),
        "sdm_section": "17.17.3"
    },

    # Segment Base MSRs
    {
        "name": "IA32_FS_BASE",
        "address": 0xC0000100,
        "category": "Segment",
        "description": "FS segment base address in 64-bit mode. Used for Thread Local Storage (TLS).",
        "bit_fields": json.dumps({
            "63:0": "FS base address"
        }),
        "sdm_section": "3.4.4"
    },
    {
        "name": "IA32_GS_BASE",
        "address": 0xC0000101,
        "category": "Segment",
        "description": "GS segment base address in 64-bit mode. Windows uses this for per-CPU data structures.",
        "bit_fields": json.dumps({
            "63:0": "GS base address"
        }),
        "sdm_section": "3.4.4"
    },
    {
        "name": "IA32_KERNEL_GS_BASE",
        "address": 0xC0000102,
        "category": "Segment",
        "description": "Swap target for GS base. SWAPGS instruction exchanges GS_BASE with this MSR. Used for user/kernel mode transitions.",
        "bit_fields": json.dumps({
            "63:0": "Kernel GS base address"
        }),
        "sdm_section": "3.4.4"
    },

    # System MSRs
    {
        "name": "IA32_EFER",
        "address": 0xC0000080,
        "category": "System",
        "description": "Extended Feature Enable Register. Controls long mode, syscall/sysret, NX bit.",
        "bit_fields": json.dumps({
            "0": "SCE - SYSCALL Enable",
            "7:1": "Reserved",
            "8": "LME - Long Mode Enable",
            "9": "Reserved",
            "10": "LMA - Long Mode Active (read-only)",
            "11": "NXE - No-Execute Enable",
            "12": "SVME - Secure Virtual Machine Enable",
            "13": "LMSLE - Long Mode Segment Limit Enable",
            "14": "FFXSR - Fast FXSAVE/FXRSTOR",
            "15": "TCE - Translation Cache Extension",
            "63:16": "Reserved"
        }),
        "sdm_section": "2.2.1"
    },
    {
        "name": "IA32_STAR",
        "address": 0xC0000081,
        "category": "System",
        "description": "System Call Target Address. Contains segment selectors for SYSCALL/SYSRET.",
        "bit_fields": json.dumps({
            "31:0": "Reserved",
            "47:32": "SYSCALL CS and SS (CS = value, SS = value + 8)",
            "63:48": "SYSRET CS and SS (CS = value + 16, SS = value + 8)"
        }),
        "sdm_section": "5.8.8"
    },
    {
        "name": "IA32_LSTAR",
        "address": 0xC0000082,
        "category": "System",
        "description": "Long Mode SYSCALL Target RIP. Entry point for SYSCALL in 64-bit mode.",
        "bit_fields": json.dumps({
            "63:0": "Target RIP for SYSCALL in 64-bit mode"
        }),
        "sdm_section": "5.8.8"
    },
    {
        "name": "IA32_CSTAR",
        "address": 0xC0000083,
        "category": "System",
        "description": "Compatibility Mode SYSCALL Target RIP. Entry point for SYSCALL in compatibility mode.",
        "bit_fields": json.dumps({
            "63:0": "Target RIP for SYSCALL in compatibility mode"
        }),
        "sdm_section": "5.8.8"
    },
    {
        "name": "IA32_FMASK",
        "address": 0xC0000084,
        "category": "System",
        "description": "SYSCALL Flag Mask. Specifies which RFLAGS bits are cleared on SYSCALL.",
        "bit_fields": json.dumps({
            "31:0": "RFLAGS bits to clear on SYSCALL",
            "63:32": "Reserved"
        }),
        "sdm_section": "5.8.8"
    },
    {
        "name": "IA32_SYSENTER_CS",
        "address": 0x174,
        "category": "System",
        "description": "SYSENTER CS. Code segment selector loaded by SYSENTER (legacy fast syscall).",
        "bit_fields": json.dumps({
            "15:0": "CS selector for SYSENTER",
            "63:16": "Reserved"
        }),
        "sdm_section": "5.8.7"
    },
    {
        "name": "IA32_SYSENTER_ESP",
        "address": 0x175,
        "category": "System",
        "description": "SYSENTER ESP. Stack pointer loaded by SYSENTER.",
        "bit_fields": json.dumps({
            "63:0": "ESP/RSP for SYSENTER"
        }),
        "sdm_section": "5.8.7"
    },
    {
        "name": "IA32_SYSENTER_EIP",
        "address": 0x176,
        "category": "System",
        "description": "SYSENTER EIP. Instruction pointer loaded by SYSENTER.",
        "bit_fields": json.dumps({
            "63:0": "EIP/RIP for SYSENTER"
        }),
        "sdm_section": "5.8.7"
    },

    # Debug MSRs
    {
        "name": "IA32_DEBUGCTL",
        "address": 0x1D9,
        "category": "Debug",
        "description": "Debug Control MSR. Controls debug features including LBR, BTF, and performance monitoring.",
        "bit_fields": json.dumps({
            "0": "LBR - Last Branch Record",
            "1": "BTF - Single-Step on Branches",
            "5:2": "Reserved",
            "6": "TR - Trace Messages Enable",
            "7": "BTS - Branch Trace Store",
            "8": "BTINT - Branch Trace Interrupt",
            "9": "BTS_OFF_OS - BTS off in OS",
            "10": "BTS_OFF_USR - BTS off in user mode",
            "11": "FREEZE_LBRS_ON_PMI",
            "12": "FREEZE_PERFMON_ON_PMI",
            "13": "ENABLE_UNCORE_PMI",
            "14": "FREEZE_WHILE_SMM",
            "15": "RTM_DEBUG",
            "63:16": "Reserved"
        }),
        "sdm_section": "17.4.1"
    },
    {
        "name": "IA32_DS_AREA",
        "address": 0x600,
        "category": "Debug",
        "description": "Debug Store (DS) Area. Points to the DS buffer for BTS and PEBS.",
        "bit_fields": json.dumps({
            "63:0": "Linear address of DS buffer management area"
        }),
        "sdm_section": "18.10.4"
    },
    {
        "name": "IA32_PEBS_ENABLE",
        "address": 0x3F1,
        "category": "Debug",
        "description": "Precise Event-Based Sampling (PEBS) Enable. Controls which performance counters use PEBS.",
        "bit_fields": json.dumps({
            "3:0": "Enable PEBS on PMC0-3",
            "31:4": "Reserved",
            "35:32": "Enable PEBS on fixed counters",
            "63:36": "Reserved"
        }),
        "sdm_section": "18.10.2.2"
    },

    # Power/Thermal MSRs
    {
        "name": "IA32_MISC_ENABLE",
        "address": 0x1A0,
        "category": "Power",
        "description": "Miscellaneous Enable bits. Controls various processor features including Fast Strings, TCC, SpeedStep.",
        "bit_fields": json.dumps({
            "0": "Fast-Strings Enable",
            "2:1": "Reserved",
            "3": "Automatic Thermal Control Circuit Enable",
            "6:4": "Reserved",
            "7": "Performance Monitoring Available",
            "10:8": "Reserved",
            "11": "Branch Trace Storage Unavailable",
            "12": "Precise Event Based Sampling Unavailable",
            "15:13": "Reserved",
            "16": "Enhanced Intel SpeedStep Technology Enable",
            "17": "Reserved",
            "18": "ENABLE MONITOR FSM",
            "21:19": "Reserved",
            "22": "Limit CPUID Maxval",
            "23": "xTPR Message Disable",
            "33:24": "Reserved",
            "34": "XD Bit Disable",
            "63:35": "Reserved"
        }),
        "sdm_section": "Table 2-2"
    },
    {
        "name": "IA32_PERF_CTL",
        "address": 0x199,
        "category": "Power",
        "description": "Performance Control. Controls processor frequency and voltage.",
        "bit_fields": json.dumps({
            "15:0": "Target performance state value",
            "31:16": "Reserved",
            "32": "IDA Disengage",
            "63:33": "Reserved"
        }),
        "sdm_section": "14.3.2"
    },
    {
        "name": "IA32_PERF_STATUS",
        "address": 0x198,
        "category": "Power",
        "description": "Performance Status. Reports current processor frequency and voltage.",
        "bit_fields": json.dumps({
            "15:0": "Current performance state value",
            "63:16": "Reserved"
        }),
        "sdm_section": "14.3.2"
    },
    {
        "name": "IA32_CLOCK_MODULATION",
        "address": 0x19A,
        "category": "Power",
        "description": "Clock Modulation Control. Controls on-demand clock modulation for thermal management.",
        "bit_fields": json.dumps({
            "0": "Extended On-Demand Clock Modulation Duty Cycle",
            "3:1": "On-Demand Clock Modulation Duty Cycle",
            "4": "On-Demand Clock Modulation Enable",
            "63:5": "Reserved"
        }),
        "sdm_section": "14.7.3"
    },
    {
        "name": "IA32_THERM_STATUS",
        "address": 0x19C,
        "category": "Power",
        "description": "Thermal Status Information. Reports thermal monitoring status.",
        "bit_fields": json.dumps({
            "0": "Thermal Status",
            "1": "Thermal Status Log",
            "2": "PROCHOT# or FORCEPR# event",
            "3": "PROCHOT# or FORCEPR# log",
            "4": "Critical Temperature Status",
            "5": "Critical Temperature Status Log",
            "6": "Thermal Threshold #1 Status",
            "7": "Thermal Threshold #1 Log",
            "8": "Thermal Threshold #2 Status",
            "9": "Thermal Threshold #2 Log",
            "10": "Power Limitation Status",
            "11": "Power Limitation Log",
            "15:12": "Reserved",
            "22:16": "Digital Readout",
            "26:23": "Reserved",
            "30:27": "Resolution in Degrees Celsius",
            "31": "Reading Valid",
            "63:32": "Reserved"
        }),
        "sdm_section": "14.7.2"
    },

    # APIC MSRs
    {
        "name": "IA32_APIC_BASE",
        "address": 0x1B,
        "category": "APIC",
        "description": "APIC Base Address. Specifies the base address of the APIC register space.",
        "bit_fields": json.dumps({
            "7:0": "Reserved",
            "8": "BSP flag - Bootstrap Processor",
            "9": "Reserved",
            "10": "Enable x2APIC mode",
            "11": "APIC Global Enable",
            "35:12": "APIC Base address (4KB aligned)",
            "63:36": "Reserved"
        }),
        "sdm_section": "10.4.4"
    },
    {
        "name": "IA32_X2APIC_APICID",
        "address": 0x802,
        "category": "APIC",
        "description": "x2APIC ID register. Read-only register containing the x2APIC ID.",
        "bit_fields": json.dumps({
            "31:0": "x2APIC ID",
            "63:32": "Reserved"
        }),
        "sdm_section": "10.12.1"
    },
    {
        "name": "IA32_X2APIC_VERSION",
        "address": 0x803,
        "category": "APIC",
        "description": "x2APIC Version register. Contains version and max LVT entry information.",
        "bit_fields": json.dumps({
            "7:0": "Version",
            "15:8": "Reserved",
            "23:16": "Max LVT Entry",
            "24": "Suppress EOI-broadcast",
            "31:25": "Reserved",
            "63:32": "Reserved"
        }),
        "sdm_section": "10.12.1"
    },
    {
        "name": "IA32_X2APIC_TPR",
        "address": 0x808,
        "category": "APIC",
        "description": "x2APIC Task Priority Register. Controls interrupt priority threshold.",
        "bit_fields": json.dumps({
            "7:0": "Task Priority",
            "31:8": "Reserved",
            "63:32": "Reserved"
        }),
        "sdm_section": "10.12.1"
    },
    {
        "name": "IA32_X2APIC_EOI",
        "address": 0x80B,
        "category": "APIC",
        "description": "x2APIC End Of Interrupt register. Write-only register to signal EOI.",
        "bit_fields": json.dumps({
            "31:0": "EOI (write 0 to signal EOI)",
            "63:32": "Reserved"
        }),
        "sdm_section": "10.12.1"
    },
    {
        "name": "IA32_X2APIC_SIVR",
        "address": 0x80F,
        "category": "APIC",
        "description": "x2APIC Spurious Interrupt Vector Register. Controls APIC enable and spurious vector.",
        "bit_fields": json.dumps({
            "7:0": "Spurious Vector",
            "8": "APIC Software Enable",
            "9": "Focus Processor Checking",
            "11:10": "Reserved",
            "12": "Suppress EOI-broadcast",
            "31:13": "Reserved",
            "63:32": "Reserved"
        }),
        "sdm_section": "10.12.1"
    },

    # CR/PAT MSRs
    {
        "name": "IA32_PAT",
        "address": 0x277,
        "category": "Memory",
        "description": "Page Attribute Table. Defines memory types for PAT page table entries.",
        "bit_fields": json.dumps({
            "7:0": "PA0 - Memory type for PAT entry 0",
            "15:8": "PA1 - Memory type for PAT entry 1",
            "23:16": "PA2 - Memory type for PAT entry 2",
            "31:24": "PA3 - Memory type for PAT entry 3",
            "39:32": "PA4 - Memory type for PAT entry 4",
            "47:40": "PA5 - Memory type for PAT entry 5",
            "55:48": "PA6 - Memory type for PAT entry 6",
            "63:56": "PA7 - Memory type for PAT entry 7"
        }),
        "sdm_section": "11.12.3"
    },
]

def add_critical_msrs():
    """Add all critical MSRs to the database."""
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()

    print(f"Database path: {DB_PATH}")
    print(f"Adding {len(CRITICAL_MSRS)} critical MSRs...\n")

    added = 0
    skipped = 0

    for msr in CRITICAL_MSRS:
        try:
            cursor.execute("""
                INSERT OR IGNORE INTO msrs (name, address, category, description, bit_fields, sdm_section)
                VALUES (?, ?, ?, ?, ?, ?)
            """, (
                msr["name"],
                msr["address"],
                msr["category"],
                msr["description"],
                msr["bit_fields"],
                msr["sdm_section"]
            ))

            if cursor.rowcount > 0:
                added += 1
                print(f"✓ Added {msr['name']} (0x{msr['address']:X}) - {msr['category']}")
            else:
                skipped += 1
                print(f"- Skipped {msr['name']} (already exists)")

        except sqlite3.Error as e:
            print(f"✗ Error adding {msr['name']}: {e}")

    conn.commit()

    # Verify total count
    cursor.execute("SELECT COUNT(*) FROM msrs")
    total = cursor.fetchone()[0]

    print(f"\n{'='*60}")
    print(f"Summary:")
    print(f"  Added: {added}")
    print(f"  Skipped: {skipped}")
    print(f"  Total MSRs in database: {total}")
    print(f"{'='*60}\n")

    # Show timing-related MSRs
    print("Timing-related MSRs (CRITICAL for anti-cheat evasion):")
    print(f"{'='*60}")
    cursor.execute("""
        SELECT name, address, description
        FROM msrs
        WHERE category = 'Timing' OR name LIKE '%TSC%' OR name LIKE '%TIME%'
        ORDER BY address
    """)

    for row in cursor.fetchall():
        name, address, desc = row
        # Truncate description for display
        desc_short = desc[:80] + "..." if len(desc) > 80 else desc
        print(f"  {name:30s} (0x{address:08X})")
        print(f"    {desc_short}\n")

    conn.close()
    print(f"{'='*60}")
    print("Done! Database updated successfully.")

if __name__ == "__main__":
    add_critical_msrs()
