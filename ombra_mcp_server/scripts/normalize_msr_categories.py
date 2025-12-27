#!/usr/bin/env python3
"""
Normalize MSR category capitalization and verify critical anti-cheat MSRs.
"""

import sqlite3
from pathlib import Path

DB_PATH = Path(__file__).parent.parent / "src" / "ombra_mcp" / "data" / "intel_sdm.db"

def normalize_categories():
    """Normalize category names to proper capitalization."""
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()

    # Disable FTS triggers temporarily
    cursor.execute("PRAGMA writable_schema = ON;")

    updates = [
        ("system", "System"),
        ("timing", "Timing"),
        ("memory", "Memory"),
        ("debug", "Debug"),
        ("apic", "APIC"),
        ("feature", "Feature"),
        ("segment", "Segment"),
        ("vmx_capability", "VMX_Capability"),
    ]

    for old, new in updates:
        cursor.execute("UPDATE msrs SET category = ? WHERE category = ?", (new, old))
        if cursor.rowcount > 0:
            print(f"Updated {cursor.rowcount} MSRs from '{old}' to '{new}'")

    conn.commit()

    # Re-enable FTS
    cursor.execute("PRAGMA writable_schema = OFF;")

    # Rebuild FTS index
    print("\nRebuilding FTS index...")
    cursor.execute("INSERT INTO msrs_fts(msrs_fts) VALUES('rebuild');")
    conn.commit()

    # Show updated distribution
    print("\nMSR distribution by category:")
    print("="*60)
    cursor.execute("""
        SELECT category, COUNT(*) as count
        FROM msrs
        GROUP BY category
        ORDER BY count DESC
    """)

    for row in cursor.fetchall():
        category, count = row
        print(f"  {category:20s} {count:3d}")

    total = sum(r[1] for r in cursor.fetchall())
    cursor.execute("SELECT COUNT(*) FROM msrs")
    total = cursor.fetchone()[0]
    print(f"  {'='*20} {'='*3}")
    print(f"  {'TOTAL':20s} {total:3d}")

    conn.close()

def verify_critical_msrs():
    """Verify all critical anti-cheat MSRs are present."""
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()

    critical_msrs = [
        # Timing (CRITICAL for anti-cheat)
        ("IA32_TIME_STAMP_COUNTER", 0x10),
        ("IA32_TSC_ADJUST", 0x3B),
        ("IA32_MPERF", 0xE7),
        ("IA32_APERF", 0xE8),
        ("IA32_TSC_AUX", 0xC0000103),
    ]

    print("\n" + "="*60)
    print("Verifying critical anti-cheat MSRs:")
    print("="*60)

    all_present = True
    for name, addr in critical_msrs:
        cursor.execute("SELECT name, address, category, description FROM msrs WHERE name = ?", (name,))
        row = cursor.fetchone()

        if row:
            msr_name, msr_addr, category, desc = row
            # Truncate description
            desc_short = desc[:60] + "..." if len(desc) > 60 else desc
            print(f"✓ {name:30s} 0x{addr:08X} [{category}]")
            if "CRITICAL" in desc or "ESEA" in desc:
                print(f"  → {desc_short}")
        else:
            print(f"✗ MISSING: {name} (0x{addr:08X})")
            all_present = False

    if all_present:
        print("\n✓ All critical anti-cheat MSRs are present!")
    else:
        print("\n✗ Some critical MSRs are missing!")

    # Show all timing-related MSRs
    print("\n" + "="*60)
    print("All Timing MSRs in database:")
    print("="*60)
    cursor.execute("""
        SELECT name, address, description
        FROM msrs
        WHERE category = 'Timing'
        ORDER BY address
    """)

    for row in cursor.fetchall():
        name, address, desc = row
        print(f"\n{name} (0x{address:08X}):")
        # Word wrap description
        words = desc.split()
        line = "  "
        for word in words:
            if len(line) + len(word) + 1 > 78:
                print(line)
                line = "  " + word
            else:
                line += " " + word if line != "  " else word
        if line != "  ":
            print(line)

    conn.close()

if __name__ == "__main__":
    normalize_categories()
    verify_critical_msrs()
