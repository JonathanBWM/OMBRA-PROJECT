#!/usr/bin/env python3
"""
Migrate gotchas from project_brain.db to appropriate specialized databases.

Categories:
- anticheat_intel.db: Detection methods and bypasses
- evasion_techniques.db: Evasion techniques and cleanup procedures
- byovd_drivers.db: Driver-specific gotchas
- project_brain.db: Keep general project-specific gotchas
"""

import sqlite3
from pathlib import Path

DATA_DIR = Path(__file__).parent.parent / "src" / "ombra_mcp" / "data"

# Migration mapping: gotcha_id -> (target_db, category/type, extra_data)
MIGRATION_MAP = {
    # Anti-cheat intel - these become detection methods or bypasses
    "G001": ("anticheat", "bypass", {
        "anticheat": "EAC",
        "technique": "MmCopyMemory Physical Read",
        "description": "MmCopyMemory and MmMapIoSpace are PatchGuard protected - EAC cannot hook them",
        "for_detection": "memory_scanning",
    }),
    "G002": ("anticheat", "detection", {
        "anticheat": "EAC",
        "method_id": "ALLOC_HEURISTICS",
        "name": "Page Fault Allocation Heuristics",
        "description": "EAC uses KiPageFault heuristics via kdTrap hook at IDT 0x14",
        "technique": "Monitors allocation timing and patterns, not specific APIs",
        "category": "behavioral",
        "severity": "high",
    }),
    "G003": ("anticheat", "detection", {
        "anticheat": "ALL",  # Multiple ACs
        "method_id": "RWX_DETECTION",
        "name": "RWX Page Permission Detection",
        "description": "Anti-cheats flag RWX (Read-Write-Execute) memory as suspicious",
        "technique": "Scan for pages with combined RWX permissions",
        "category": "memory",
        "severity": "critical",
    }),
    "G004": ("anticheat", "detection", {
        "anticheat": "EAC",
        "method_id": "ETW_MMCOPY",
        "name": "ETW MmCopyMemory Virtual Tracing",
        "description": "MmCopyMemory for virtual memory copies is logged to ETW trace providers",
        "technique": "ETW consumer monitors memory copy operations",
        "category": "memory",
        "severity": "medium",
    }),
    "G005": ("anticheat", "detection", {
        "anticheat": "ALL",
        "method_id": "DRV_PTR_INTEGRITY",
        "name": "Driver Data Pointer Integrity",
        "description": "Anti-cheats verify driver internal structures haven't been tampered with",
        "technique": "Scan for hooks on driver data pointers",
        "category": "integrity",
        "severity": "high",
    }),
    "G007": ("anticheat", "detection", {
        "anticheat": "KDSTINKER",
        "method_id": "IAT_HOOK_DUMP",
        "name": "IAT Hook Driver Dump",
        "description": "KDStinker hooks driver IAT entry for IoCreateDevice to steal DRIVER_OBJECT and dump driver",
        "technique": "Hook FF 15 opcode IAT calls, intercept IoCreateDevice, dump via DRIVER_OBJECT",
        "category": "driver_scanning",
        "severity": "critical",
    }),
    "G011": ("anticheat", "detection", {
        "anticheat": "ALL",
        "method_id": "CERT_BLACKLIST",
        "name": "Certificate Blacklist",
        "description": "Anti-cheats maintain blacklists of leaked/revoked signing certificates",
        "technique": "Certificate thumbprint matching against known bad list",
        "category": "signature",
        "severity": "critical",
    }),
    "G015": ("anticheat", "detection", {
        "anticheat": "EAC",
        "method_id": "PML4E_SCAN",
        "name": "Kernel PML4E Executable Scan",
        "description": "EAC scans ALL kernel PML4Es for executable pages without backing legitimate drivers",
        "technique": "Walk page tables, find executable kernel pages, verify driver backing. Dumps metadata, sends to server. Continuous scan, not periodic.",
        "category": "memory",
        "severity": "critical",
    }),

    # BYOVD driver gotchas
    "G016": ("byovd", "gotcha", {
        "driver": "ld9boxsup",
        "symptom": "COOKIE handshake fails with invalid parameter error",
        "cause": "Vendor modified VirtualBox values: cookie=0x69726F74, magic='The Magic Word!', flags=0x42000042",
    }),
    "G017": ("byovd", "gotcha", {
        "driver": "ld9boxsup",
        "symptom": "LDR_OPEN returns -618 (VERR_LDR_GENERAL_FAILURE)",
        "cause": "Driver checks global flags at 0x4a1a0 and 0x4a210 set during DriverEntry. Fails in nested virtualization.",
    }),

    # Evasion techniques
    "G010": ("evasion", "technique", {
        "category": "Driver Hiding",
        "name": "Driverless Communication",
        "short_name": "driverless_comm",
        "description": "Avoid creating driver objects entirely to prevent artifact scanning",
        "use_case": "Alternative communication with mapped drivers without IOCTL",
        "limitations": "More complex implementation",
    }),
    "G013": ("evasion", "technique", {
        "category": "Memory Hiding",
        "name": "BigPoolTable Avoidance",
        "short_name": "bigpool_avoid",
        "description": "Use MDL allocation instead of ExAllocatePool to avoid BigPoolTable tracking",
        "use_case": "Hide large kernel allocations from pool scanning",
        "limitations": "MDL allocation has different semantics",
    }),
    "G014": ("evasion", "technique", {
        "category": "Memory Hiding",
        "name": "PFN Array Nulling",
        "short_name": "pfn_null",
        "description": "Null PFN entries in MDL to hide physical memory allocation from PFN database scans",
        "use_case": "Hide physical page allocations after MDL mapping",
        "requirements": "Must be done from Ring 0 after mapping",
    }),
    "G018": ("evasion", "cleanup", {
        "name": "Kernel Forensic Artifact Cleanup",
        "description": "Clear MmUnloadedDrivers, PiDDBCacheTable, ETW buffers, prefetch files",
        "requires_ring0": True,
        "timing": "post_activation",
        "artifacts_cleared": ["MmUnloadedDrivers", "PiDDBCacheTable", "ETW circular buffers", "Prefetch files"],
    }),

    # Keep in project_brain (project-specific, not generalizable)
    "G006": ("keep", None, {}),  # KTHREAD offsets - project specific
    "G008": ("keep", None, {}),  # Hypervisor superiority - project philosophy
    "G009": ("keep", None, {}),  # Alternative BYOVD vectors - project notes
    "G012": ("keep", None, {}),  # MDL BSOD debugging - project specific
}


def migrate():
    # Read all gotchas from project_brain
    brain_conn = sqlite3.connect(DATA_DIR / "project_brain.db")
    brain_conn.row_factory = sqlite3.Row
    brain_c = brain_conn.cursor()

    brain_c.execute("SELECT * FROM gotchas")
    gotchas = {row['id']: dict(row) for row in brain_c.fetchall()}

    # Connect to target databases
    anticheat_conn = sqlite3.connect(DATA_DIR / "anticheat_intel.db")
    anticheat_conn.row_factory = sqlite3.Row
    ac_c = anticheat_conn.cursor()

    evasion_conn = sqlite3.connect(DATA_DIR / "evasion_techniques.db")
    evasion_conn.row_factory = sqlite3.Row
    ev_c = evasion_conn.cursor()

    byovd_conn = sqlite3.connect(DATA_DIR / "byovd_drivers.db")
    byovd_conn.row_factory = sqlite3.Row
    by_c = byovd_conn.cursor()

    migrated = {"anticheat": 0, "evasion": 0, "byovd": 0, "kept": 0}

    for gotcha_id, gotcha in gotchas.items():
        if gotcha_id not in MIGRATION_MAP:
            print(f"  SKIP: {gotcha_id} - no mapping defined")
            continue

        target_db, item_type, extra = MIGRATION_MAP[gotcha_id]

        if target_db == "keep":
            print(f"  KEEP: {gotcha_id} in project_brain")
            migrated["kept"] += 1
            continue

        if target_db == "anticheat":
            if item_type == "detection":
                # Get or create anticheat
                anticheat_name = extra.get("anticheat", "UNKNOWN")
                ac_c.execute("SELECT id FROM anticheats WHERE name = ?", (anticheat_name,))
                row = ac_c.fetchone()
                if not row:
                    ac_c.execute("INSERT INTO anticheats (name) VALUES (?)", (anticheat_name,))
                    ac_id = ac_c.lastrowid
                else:
                    ac_id = row['id']

                # Insert detection method
                ac_c.execute("""
                    INSERT OR REPLACE INTO detection_methods
                    (anticheat_id, method_id, name, description, technique, category, severity)
                    VALUES (?, ?, ?, ?, ?, ?, ?)
                """, (
                    ac_id,
                    extra.get("method_id"),
                    extra.get("name"),
                    extra.get("description"),
                    extra.get("technique"),
                    extra.get("category"),
                    extra.get("severity"),
                ))
                print(f"  MIGRATE: {gotcha_id} -> anticheat_intel.detection_methods ({extra.get('name')})")

            elif item_type == "bypass":
                # Find detection method to link to (or create standalone)
                ac_c.execute("""
                    INSERT INTO bypasses (detection_method_id, technique, description, implementation)
                    VALUES (
                        (SELECT MIN(id) FROM detection_methods WHERE anticheat_id =
                            (SELECT id FROM anticheats WHERE name = ?)),
                        ?, ?, ?
                    )
                """, (
                    extra.get("anticheat"),
                    extra.get("technique"),
                    extra.get("description"),
                    gotcha.get("fix"),
                ))
                print(f"  MIGRATE: {gotcha_id} -> anticheat_intel.bypasses ({extra.get('technique')})")

            migrated["anticheat"] += 1

        elif target_db == "byovd":
            # Get driver ID
            driver_name = extra.get("driver")
            by_c.execute("SELECT id FROM drivers WHERE LOWER(name) = LOWER(?)", (driver_name,))
            row = by_c.fetchone()
            if row:
                by_c.execute("""
                    INSERT INTO driver_gotchas (driver_id, symptom, cause, fix)
                    VALUES (?, ?, ?, ?)
                """, (row['id'], extra.get("symptom"), extra.get("cause"), gotcha.get("fix")))
                print(f"  MIGRATE: {gotcha_id} -> byovd_drivers.driver_gotchas ({driver_name})")
                migrated["byovd"] += 1
            else:
                print(f"  SKIP: {gotcha_id} - driver '{driver_name}' not found in byovd_drivers.db")

        elif target_db == "evasion":
            if item_type == "technique":
                # Get or create category
                cat_name = extra.get("category", "Uncategorized")
                ev_c.execute("SELECT id FROM categories WHERE name = ?", (cat_name,))
                row = ev_c.fetchone()
                if not row:
                    ev_c.execute("INSERT INTO categories (name) VALUES (?)", (cat_name,))
                    cat_id = ev_c.lastrowid
                else:
                    cat_id = row['id']

                ev_c.execute("""
                    INSERT OR REPLACE INTO techniques
                    (category_id, name, short_name, description, use_case, requirements, limitations)
                    VALUES (?, ?, ?, ?, ?, ?, ?)
                """, (
                    cat_id,
                    extra.get("name"),
                    extra.get("short_name"),
                    extra.get("description"),
                    extra.get("use_case"),
                    extra.get("requirements"),
                    extra.get("limitations"),
                ))
                print(f"  MIGRATE: {gotcha_id} -> evasion_techniques.techniques ({extra.get('name')})")

            elif item_type == "cleanup":
                import json
                ev_c.execute("""
                    INSERT INTO cleanup_procedures
                    (name, description, requires_ring0, timing, artifacts_cleared)
                    VALUES (?, ?, ?, ?, ?)
                """, (
                    extra.get("name"),
                    extra.get("description"),
                    extra.get("requires_ring0", False),
                    extra.get("timing"),
                    json.dumps(extra.get("artifacts_cleared", [])),
                ))
                print(f"  MIGRATE: {gotcha_id} -> evasion_techniques.cleanup_procedures ({extra.get('name')})")

            migrated["evasion"] += 1

    # Commit all
    anticheat_conn.commit()
    evasion_conn.commit()
    byovd_conn.commit()

    # Close connections
    brain_conn.close()
    anticheat_conn.close()
    evasion_conn.close()
    byovd_conn.close()

    return migrated


if __name__ == "__main__":
    print("Migrating gotchas from project_brain.db to specialized databases...")
    print()
    result = migrate()
    print()
    print(f"Migration complete:")
    print(f"  anticheat_intel.db: {result['anticheat']} items")
    print(f"  evasion_techniques.db: {result['evasion']} items")
    print(f"  byovd_drivers.db: {result['byovd']} items")
    print(f"  Kept in project_brain.db: {result['kept']} items")
