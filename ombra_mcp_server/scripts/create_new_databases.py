#!/usr/bin/env python3
"""
Create the 3 new databases for MCP refactor:
- anticheat_intel.db
- evasion_techniques.db
- byovd_drivers.db

And populate with initial data from existing hardcoded sources.
"""

import sqlite3
from pathlib import Path
import json

DATA_DIR = Path(__file__).parent.parent / "src" / "ombra_mcp" / "data"


def create_anticheat_intel_db():
    """Create anticheat_intel.db with schema and initial data."""
    db_path = DATA_DIR / "anticheat_intel.db"
    conn = sqlite3.connect(db_path)
    c = conn.cursor()

    # Create schema
    c.executescript("""
        -- Anti-cheat products
        CREATE TABLE IF NOT EXISTS anticheats (
            id INTEGER PRIMARY KEY,
            name TEXT UNIQUE NOT NULL,
            full_name TEXT,
            vendor TEXT,
            kernel_driver TEXT,
            ring0 BOOLEAN DEFAULT TRUE,
            last_updated TEXT,
            notes TEXT
        );

        -- Detection methods used by anti-cheats
        CREATE TABLE IF NOT EXISTS detection_methods (
            id INTEGER PRIMARY KEY,
            anticheat_id INTEGER REFERENCES anticheats(id),
            method_id TEXT NOT NULL,
            category TEXT NOT NULL,
            name TEXT NOT NULL,
            description TEXT NOT NULL,
            technique TEXT,
            threshold_value REAL,
            threshold_unit TEXT,
            check_frequency TEXT,
            severity TEXT DEFAULT 'high',
            first_seen TEXT,
            source TEXT,
            source_url TEXT,
            UNIQUE(anticheat_id, method_id)
        );

        -- Bypass techniques for detection methods
        CREATE TABLE IF NOT EXISTS bypasses (
            id INTEGER PRIMARY KEY,
            detection_method_id INTEGER REFERENCES detection_methods(id),
            technique TEXT NOT NULL,
            description TEXT,
            implementation TEXT,
            difficulty TEXT DEFAULT 'medium',
            effectiveness TEXT DEFAULT 'proven',
            side_effects TEXT,
            requires TEXT,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP
        );

        -- Known signatures anti-cheats scan for
        CREATE TABLE IF NOT EXISTS signatures (
            id INTEGER PRIMARY KEY,
            anticheat_id INTEGER REFERENCES anticheats(id),
            signature_type TEXT NOT NULL,
            pattern TEXT NOT NULL,
            description TEXT,
            location TEXT,
            avoidance TEXT
        );

        -- Timing thresholds
        CREATE TABLE IF NOT EXISTS timing_thresholds (
            id INTEGER PRIMARY KEY,
            anticheat_id INTEGER REFERENCES anticheats(id),
            operation TEXT NOT NULL,
            threshold_cycles INTEGER,
            threshold_ns INTEGER,
            notes TEXT,
            source TEXT
        );

        -- Full-text search
        CREATE VIRTUAL TABLE IF NOT EXISTS detection_methods_fts USING fts5(
            name, description, technique,
            content='detection_methods',
            content_rowid='id'
        );

        -- Triggers to keep FTS in sync
        CREATE TRIGGER IF NOT EXISTS detection_methods_ai AFTER INSERT ON detection_methods BEGIN
            INSERT INTO detection_methods_fts(rowid, name, description, technique)
            VALUES (new.id, new.name, new.description, new.technique);
        END;

        CREATE TRIGGER IF NOT EXISTS detection_methods_ad AFTER DELETE ON detection_methods BEGIN
            INSERT INTO detection_methods_fts(detection_methods_fts, rowid, name, description, technique)
            VALUES ('delete', old.id, old.name, old.description, old.technique);
        END;

        CREATE TRIGGER IF NOT EXISTS detection_methods_au AFTER UPDATE ON detection_methods BEGIN
            INSERT INTO detection_methods_fts(detection_methods_fts, rowid, name, description, technique)
            VALUES ('delete', old.id, old.name, old.description, old.technique);
            INSERT INTO detection_methods_fts(rowid, name, description, technique)
            VALUES (new.id, new.name, new.description, new.technique);
        END;

        -- Indexes
        CREATE INDEX IF NOT EXISTS idx_detection_anticheat ON detection_methods(anticheat_id);
        CREATE INDEX IF NOT EXISTS idx_detection_category ON detection_methods(category);
        CREATE INDEX IF NOT EXISTS idx_bypasses_method ON bypasses(detection_method_id);
    """)

    # Insert initial anti-cheat data
    anticheats = [
        ('EAC', 'Easy Anti-Cheat', 'Epic Games', 'EasyAntiCheat.sys'),
        ('BattlEye', 'BattlEye Anti-Cheat', 'BattlEye GmbH', 'BEDaisy.sys'),
        ('Vanguard', 'Riot Vanguard', 'Riot Games', 'vgk.sys'),
        ('FACEIT', 'FACEIT Anti-Cheat', 'FACEIT', 'faceit.sys'),
        ('ESEA', 'ESEA Anti-Cheat', 'ESL Gaming', 'esea.sys'),
    ]

    for ac in anticheats:
        c.execute("""
            INSERT OR IGNORE INTO anticheats (name, full_name, vendor, kernel_driver)
            VALUES (?, ?, ?, ?)
        """, ac)

    # Insert detection methods
    detection_methods = [
        # EAC methods
        (1, 'EAC-PML4E', 'memory', 'PML4E Kernel Page Scan',
         'Scans all kernel PML4 entries for executable pages without backing legitimate drivers. Enumerates page tables and checks each executable page against loaded driver list.',
         'Enumerates kernel PML4Es, walks page tables to find executable pages, cross-references with PsLoadedModuleList. Orphaned executable memory is flagged.',
         'continuous', 'critical', 'UC forum Dec 2025'),

        (1, 'EAC-BIGPOOL', 'memory', 'BigPoolTable Scan',
         'Queries SystemBigPoolInformation for executable allocations >0xFE0 bytes.',
         'Scans nt!PoolBigPageTable approximately 20 minutes after game launch. Looks for executable pool tags and memory without legitimate drivers.',
         'periodic', 'high', 'UC forum research'),

        (1, 'EAC-TIMING', 'timing', 'CPUID Timing Analysis',
         'Measures CPUID instruction timing to detect hypervisor presence.',
         'Calls CPUID in tight loop, measures TSC delta. Threshold ~500 cycles on bare metal, hypervisor overhead detectable.',
         'on_event', 'high', 'Research'),

        (1, 'EAC-MSR', 'msr', 'VMX MSR Visibility',
         'Checks for VMX-related MSR accessibility that indicates hypervisor.',
         'Attempts to read IA32_VMX_BASIC and other VMX MSRs. On bare metal these fault, on hypervisor they may return data.',
         'on_event', 'medium', 'Research'),

        # BattlEye methods
        (2, 'BE-HEARTBEAT', 'behavioral', 'Kernel Heartbeat Monitor',
         'Monitors kernel integrity via periodic heartbeat checks.',
         'Sends periodic packets to server, server expects specific timing. Delays or timing jitter can indicate tampering.',
         'continuous', 'high', 'Research'),

        (2, 'BE-DRIVER', 'memory', 'Driver Integrity Check',
         'Verifies loaded driver integrity against known hashes.',
         'Hashes loaded driver memory, compares to known-good values. Detects code patching.',
         'periodic', 'high', 'Research'),

        # Vanguard methods
        (3, 'VGK-BOOT', 'behavioral', 'Early Boot Attestation',
         'Loads at boot time before other drivers, establishes trust chain.',
         'Kernel driver loads before other software, records boot state. Can detect pre-boot hypervisor installation.',
         'startup', 'critical', 'Public documentation'),
    ]

    for dm in detection_methods:
        c.execute("""
            INSERT OR IGNORE INTO detection_methods
            (anticheat_id, method_id, category, name, description, technique, check_frequency, severity, source)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, dm)

    # Insert timing thresholds
    timing_thresholds = [
        (1, 'cpuid', 500, 200, 'EAC threshold for CPUID timing', 'Research'),
        (1, 'rdmsr', 1000, 400, 'EAC threshold for MSR read timing', 'Research'),
        (1, 'vmexit', 2000, 800, 'General VM-exit overhead threshold', 'Research'),
        (2, 'cpuid', 600, 240, 'BattlEye CPUID timing threshold', 'Research'),
    ]

    for tt in timing_thresholds:
        c.execute("""
            INSERT OR IGNORE INTO timing_thresholds
            (anticheat_id, operation, threshold_cycles, threshold_ns, notes, source)
            VALUES (?, ?, ?, ?, ?, ?)
        """, tt)

    # Insert bypasses
    bypasses = [
        (1, 'EPT Split-View', 'Present different memory to reads vs executes',
         'Use EPT to show clean memory on reads, execute real code. Scans see legitimate code.',
         'hard', 'proven', 'Requires hypervisor', 'EPT support'),

        (2, 'MDL Allocation', 'Allocate memory via MDL to bypass BigPool',
         'Use MmAllocatePagesForMdl + MmMapLockedPagesSpecifyCache instead of ExAllocatePool',
         'medium', 'proven', 'Does not hide from PML4E scan', 'Kernel access'),

        (3, 'Timing Compensation', 'Offset TSC to hide VM-exit overhead',
         'Track accumulated VM-exit cycles, subtract from guest TSC reads',
         'hard', 'proven', 'Complex to get right for all scenarios', 'Hypervisor'),
    ]

    for bp in bypasses:
        c.execute("""
            INSERT OR IGNORE INTO bypasses
            (detection_method_id, technique, description, implementation, difficulty, effectiveness, side_effects, requires)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        """, bp)

    conn.commit()
    conn.close()
    print(f"Created {db_path}")


def create_evasion_techniques_db():
    """Create evasion_techniques.db with schema and initial data."""
    db_path = DATA_DIR / "evasion_techniques.db"
    conn = sqlite3.connect(db_path)
    c = conn.cursor()

    c.executescript("""
        -- Evasion technique categories
        CREATE TABLE IF NOT EXISTS categories (
            id INTEGER PRIMARY KEY,
            name TEXT UNIQUE NOT NULL,
            description TEXT
        );

        -- Individual techniques
        CREATE TABLE IF NOT EXISTS techniques (
            id INTEGER PRIMARY KEY,
            category_id INTEGER REFERENCES categories(id),
            name TEXT NOT NULL,
            short_name TEXT,
            description TEXT NOT NULL,
            use_case TEXT,
            requirements TEXT,
            limitations TEXT,
            detection_surface TEXT,
            source TEXT,
            source_url TEXT,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP
        );

        -- Step-by-step procedures
        CREATE TABLE IF NOT EXISTS procedure_steps (
            id INTEGER PRIMARY KEY,
            technique_id INTEGER REFERENCES techniques(id),
            step_number INTEGER NOT NULL,
            action TEXT NOT NULL,
            details TEXT,
            code_example TEXT,
            UNIQUE(technique_id, step_number)
        );

        -- Bypass chains (combining multiple techniques)
        CREATE TABLE IF NOT EXISTS bypass_chains (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL,
            description TEXT,
            goal TEXT,
            prerequisites TEXT,
            total_steps INTEGER
        );

        -- Chain to technique mapping
        CREATE TABLE IF NOT EXISTS chain_techniques (
            id INTEGER PRIMARY KEY,
            chain_id INTEGER REFERENCES bypass_chains(id),
            technique_id INTEGER REFERENCES techniques(id),
            step_order INTEGER NOT NULL,
            notes TEXT,
            UNIQUE(chain_id, step_order)
        );

        -- Code examples
        CREATE TABLE IF NOT EXISTS code_examples (
            id INTEGER PRIMARY KEY,
            technique_id INTEGER REFERENCES techniques(id),
            language TEXT DEFAULT 'c',
            title TEXT,
            code TEXT NOT NULL,
            explanation TEXT,
            tested BOOLEAN DEFAULT FALSE,
            works_on TEXT
        );

        -- Cleanup procedures
        CREATE TABLE IF NOT EXISTS cleanup_procedures (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL,
            description TEXT,
            requires_ring0 BOOLEAN DEFAULT TRUE,
            timing TEXT,
            artifacts_cleared TEXT
        );

        CREATE TABLE IF NOT EXISTS cleanup_steps (
            id INTEGER PRIMARY KEY,
            procedure_id INTEGER REFERENCES cleanup_procedures(id),
            step_number INTEGER NOT NULL,
            target TEXT NOT NULL,
            action TEXT NOT NULL,
            code TEXT,
            risk_level TEXT DEFAULT 'medium',
            UNIQUE(procedure_id, step_number)
        );

        -- Full-text search
        CREATE VIRTUAL TABLE IF NOT EXISTS techniques_fts USING fts5(
            name, description, use_case,
            content='techniques',
            content_rowid='id'
        );

        -- Triggers
        CREATE TRIGGER IF NOT EXISTS techniques_ai AFTER INSERT ON techniques BEGIN
            INSERT INTO techniques_fts(rowid, name, description, use_case)
            VALUES (new.id, new.name, new.description, new.use_case);
        END;

        CREATE TRIGGER IF NOT EXISTS techniques_ad AFTER DELETE ON techniques BEGIN
            INSERT INTO techniques_fts(techniques_fts, rowid, name, description, use_case)
            VALUES ('delete', old.id, old.name, old.description, old.use_case);
        END;
    """)

    # Insert categories
    categories = [
        ('memory_allocation', 'Techniques for allocating kernel memory without detection'),
        ('memory_hiding', 'Techniques for hiding allocated memory from scans'),
        ('timing_evasion', 'Techniques for defeating timing-based detection'),
        ('forensic_cleanup', 'Techniques for eliminating forensic artifacts'),
        ('driver_loading', 'Techniques for loading unsigned code into kernel'),
        ('hypervisor_stealth', 'Techniques for hiding hypervisor presence'),
    ]

    for cat in categories:
        c.execute("INSERT OR IGNORE INTO categories (name, description) VALUES (?, ?)", cat)

    # Insert techniques
    techniques = [
        (1, 'MDL-Based Physical Allocation', 'mdl_alloc',
         'Use MmAllocatePagesForMdl + MmMapLockedPagesSpecifyCache to allocate memory without BigPool entry',
         'Allocating driver memory that bypasses BigPoolTable scans',
         'Kernel access',
         'Does NOT protect against PML4E enumeration - memory still visible in page tables',
         'PML4E entries still present', 'UC forum'),

        (2, 'EPT Split-View', 'ept_split',
         'Present different memory contents to reads vs executes using EPT permissions',
         'Hide driver code from memory scanners while allowing execution',
         'Hypervisor with EPT support',
         'Adds complexity, requires careful MTF handling for writes',
         'None when implemented correctly', 'Research'),

        (2, 'PFN Nulling', 'pfn_null',
         'Zero out MDL PFN array entries after mapping to hide physical page ownership',
         'Hide physical memory allocation from PFN-based scans',
         'Kernel access, MDL allocation',
         'Cannot free memory normally - MmFreePagesFromMdl causes BSOD',
         'Memory leak, cannot be freed', 'UC forum'),

        (3, 'TSC Offsetting', 'tsc_offset',
         'Track accumulated VM-exit cycles, subtract from guest TSC reads',
         'Hide VM-exit timing overhead from RDTSC-based detection',
         'Hypervisor control of TSC',
         'Complex per-core tracking, difficult with nested virtualization',
         'Careful implementation leaves no traces', 'Research'),

        (4, 'MmUnloadedDrivers Patch', 'unloaded_patch',
         'Remove entries from MmUnloadedDrivers table after driver unload',
         'Eliminate forensic evidence of driver loading',
         'Kernel access',
         'PatchGuard may detect modifications',
         'No remaining driver name evidence', 'Research'),

        (5, 'BYOVD Kernel Execution', 'byovd',
         'Use vulnerable signed drivers to execute arbitrary kernel code',
         'Load unsigned code into kernel via legitimate signed driver IOCTLs',
         'Vulnerable driver with code exec primitive',
         'Driver may be blocklisted by anti-cheats',
         'Driver loading is visible in logs', 'Public'),
    ]

    for t in techniques:
        c.execute("""
            INSERT OR IGNORE INTO techniques
            (category_id, name, short_name, description, use_case, requirements, limitations, detection_surface, source)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, t)

    # Insert bypass chain
    c.execute("""
        INSERT OR IGNORE INTO bypass_chains (name, description, goal, prerequisites, total_steps)
        VALUES ('Full Stealth Driver Load',
                'Complete chain for loading a driver with maximum evasion',
                'Load and execute kernel code with minimal detectable artifacts',
                'Vulnerable signed driver, hypervisor capability',
                5)
    """)

    # Insert cleanup procedures
    c.execute("""
        INSERT OR IGNORE INTO cleanup_procedures (name, description, requires_ring0, timing, artifacts_cleared)
        VALUES ('Standard Forensic Cleanup',
                'Remove common forensic artifacts after driver loading',
                TRUE, 'post_activation',
                '["MmUnloadedDrivers", "PiDDBCacheTable", "ETW traces"]')
    """)

    cleanup_steps = [
        (1, 1, 'MmUnloadedDrivers', 'Walk linked list, remove entries with target driver name', None, 'medium'),
        (1, 2, 'PiDDBCacheTable', 'Find and remove hash table entries for loaded driver', None, 'high'),
        (1, 3, 'ETW Sessions', 'Disable or filter ETW sessions that log driver events', None, 'medium'),
        (1, 4, 'Pool Tags', 'Avoid distinctive pool tags that can be scanned', None, 'low'),
    ]

    for step in cleanup_steps:
        c.execute("""
            INSERT OR IGNORE INTO cleanup_steps
            (procedure_id, step_number, target, action, code, risk_level)
            VALUES (?, ?, ?, ?, ?, ?)
        """, step)

    conn.commit()
    conn.close()
    print(f"Created {db_path}")


def create_byovd_drivers_db():
    """Create byovd_drivers.db with schema and initial data."""
    db_path = DATA_DIR / "byovd_drivers.db"
    conn = sqlite3.connect(db_path)
    c = conn.cursor()

    c.executescript("""
        -- Vulnerable drivers
        CREATE TABLE IF NOT EXISTS drivers (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL,
            original_name TEXT,
            vendor TEXT,
            signer TEXT,
            signer_thumbprint TEXT,
            version TEXT,
            file_size INTEGER,
            sha256 TEXT,
            md5 TEXT,
            device_path TEXT,
            legitimate_use TEXT,
            discovery_date TEXT,
            cve TEXT,
            public_exploit BOOLEAN DEFAULT FALSE,
            notes TEXT,
            UNIQUE(name, version)
        );

        -- Primitives the driver provides
        CREATE TABLE IF NOT EXISTS primitives (
            id INTEGER PRIMARY KEY,
            driver_id INTEGER REFERENCES drivers(id),
            primitive_type TEXT NOT NULL,
            description TEXT NOT NULL,
            restrictions TEXT,
            requires_auth BOOLEAN DEFAULT FALSE,
            UNIQUE(driver_id, primitive_type)
        );

        -- IOCTL definitions
        CREATE TABLE IF NOT EXISTS ioctls (
            id INTEGER PRIMARY KEY,
            driver_id INTEGER REFERENCES drivers(id),
            name TEXT NOT NULL,
            code TEXT NOT NULL,
            code_decimal INTEGER,
            function_number INTEGER,
            description TEXT,
            input_struct TEXT,
            output_struct TEXT,
            input_size INTEGER,
            output_size INTEGER,
            requires_session BOOLEAN DEFAULT TRUE,
            notes TEXT,
            UNIQUE(driver_id, name)
        );

        -- Magic values for authentication
        CREATE TABLE IF NOT EXISTS magic_values (
            id INTEGER PRIMARY KEY,
            driver_id INTEGER REFERENCES drivers(id),
            value_name TEXT NOT NULL,
            value_hex TEXT,
            value_string TEXT,
            binary_offset TEXT,
            description TEXT,
            UNIQUE(driver_id, value_name)
        );

        -- Blocklist status per anti-cheat
        CREATE TABLE IF NOT EXISTS blocklist_status (
            id INTEGER PRIMARY KEY,
            driver_id INTEGER REFERENCES drivers(id),
            anticheat TEXT NOT NULL,
            blocked BOOLEAN DEFAULT FALSE,
            block_type TEXT,
            first_blocked TEXT,
            last_checked TEXT,
            notes TEXT,
            UNIQUE(driver_id, anticheat)
        );

        -- Known issues / gotchas
        CREATE TABLE IF NOT EXISTS driver_gotchas (
            id INTEGER PRIMARY KEY,
            driver_id INTEGER REFERENCES drivers(id),
            symptom TEXT NOT NULL,
            cause TEXT NOT NULL,
            fix TEXT,
            affected_environments TEXT
        );

        -- Indexes
        CREATE INDEX IF NOT EXISTS idx_drivers_name ON drivers(name);
        CREATE INDEX IF NOT EXISTS idx_ioctls_driver ON ioctls(driver_id);
        CREATE INDEX IF NOT EXISTS idx_blocklist_driver ON blocklist_status(driver_id);
    """)

    # Insert Ld9BoxSup.sys driver data
    c.execute("""
        INSERT OR IGNORE INTO drivers
        (name, original_name, vendor, signer, version, device_path, legitimate_use, public_exploit, notes)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    """, (
        'Ld9BoxSup.sys', 'VBoxDrv.sys', 'LDPlayer',
        'Shanghai Yiyu Network Technology', '6.1.36',
        r'\\.\Ld9BoxSup',
        'LDPlayer Android emulator virtualization support',
        False,
        'Rebranded VirtualBox driver with full kernel execution primitives. Not in public BYOVD databases as of Dec 2025.'
    ))

    # Get driver ID
    c.execute("SELECT id FROM drivers WHERE name = 'Ld9BoxSup.sys'")
    driver_id = c.fetchone()[0]

    # Insert primitives
    primitives = [
        (driver_id, 'code_exec', 'Execute arbitrary code at Ring-0 via LDR_LOAD', None, True),
        (driver_id, 'kernel_alloc', 'Allocate executable kernel memory via LDR_OPEN', None, True),
        (driver_id, 'symbol_resolve', 'Resolve ntoskrnl exports via LDR_GET_SYMBOL', None, True),
        (driver_id, 'phys_alloc', 'Allocate physically contiguous memory via CONT_ALLOC', None, True),
        (driver_id, 'msr_access', 'Read/write MSRs via MSR_PROBER (DISABLED in this build)', 'Disabled in LDPlayer build', True),
    ]

    for p in primitives:
        c.execute("""
            INSERT OR IGNORE INTO primitives
            (driver_id, primitive_type, description, restrictions, requires_auth)
            VALUES (?, ?, ?, ?, ?)
        """, p)

    # Insert IOCTLs
    ioctls = [
        (driver_id, 'SUP_IOCTL_COOKIE', '0x00228204', 2262532, 1, 'Establish authenticated session', 48, 56, True, None),
        (driver_id, 'SUP_IOCTL_QUERY_FUNCS', '0x00228208', 2262536, 2, 'Query supported functions (inserted in LDPlayer build)', None, None, True, 'New IOCTL inserted, shifts all function numbers'),
        (driver_id, 'SUP_IOCTL_LDR_OPEN', '0x0022820C', 2262540, 3, 'Allocate executable kernel memory', 328, 40, True, None),
        (driver_id, 'SUP_IOCTL_LDR_LOAD', '0x00228210', 2262544, 4, 'Load code and call entry point', None, None, True, None),
        (driver_id, 'SUP_IOCTL_LDR_FREE', '0x00228214', 2262548, 5, 'Free loaded module', None, None, True, None),
        (driver_id, 'SUP_IOCTL_LDR_GET_SYMBOL', '0x00228218', 2262552, 6, 'Resolve kernel symbol by name', None, None, True, None),
        (driver_id, 'SUP_IOCTL_MSR_PROBER', '0x00228288', 2262664, 34, 'Read/write MSRs (DISABLED)', None, None, True, 'Returns STATUS_NOT_SUPPORTED'),
    ]

    for ioctl in ioctls:
        c.execute("""
            INSERT OR IGNORE INTO ioctls
            (driver_id, name, code, code_decimal, function_number, description, input_size, output_size, requires_session, notes)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, ioctl)

    # Insert magic values
    magic_values = [
        (driver_id, 'initial_cookie', '0x69726F74', 'tori', '0x8f2d', 'Modified from stock VBox "Bori" - critical handshake value'),
        (driver_id, 'magic_string', None, 'The Magic Word!', '0x36d58', '16-byte auth string required for session'),
        (driver_id, 'header_flags', '0x42000042', None, None, 'Required in all IOCTL input headers'),
        (driver_id, 'driver_version', '0x320000', None, '0x4787', 'Interface version - must match'),
        (driver_id, 'ntoskrnl_flag_offset', '0x4a1a0', None, None, 'Validation flag offset for -618 bypass'),
        (driver_id, 'hal_flag_offset', '0x4a210', None, None, 'HAL validation flag for -618 bypass'),
    ]

    for mv in magic_values:
        c.execute("""
            INSERT OR IGNORE INTO magic_values
            (driver_id, value_name, value_hex, value_string, binary_offset, description)
            VALUES (?, ?, ?, ?, ?, ?)
        """, mv)

    # Insert blocklist status
    blocklist = [
        (driver_id, 'EAC', False, None, None, '2025-12-26', None),
        (driver_id, 'BattlEye', False, None, None, '2025-12-26', None),
        (driver_id, 'Vanguard', False, None, None, '2025-12-26', None),
    ]

    for bl in blocklist:
        c.execute("""
            INSERT OR IGNORE INTO blocklist_status
            (driver_id, anticheat, blocked, block_type, first_blocked, last_checked, notes)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        """, bl)

    # Insert gotchas
    gotchas = [
        (driver_id, 'LDR_OPEN returns -618 (VERR_SUPDRV_KERNEL_TOO_OLD_FOR_VTX)',
         'Kernel module enumeration fails in nested virt because Ld9BoxSup walks PsLoadedModuleList checking ntoskrnl/hal sizes. In nested environments this fails.',
         'Use ThrottleStop.sys to patch validation flags at ntoskrnl+0x4a1a0 and hal+0x4a210 to 1 via physical memory access',
         'nested_virt'),
        (driver_id, 'Cookie handshake fails with VERR_INVALID_PARAMETER',
         'Using stock VirtualBox cookie value (0x69726F42 "Bori") instead of LDPlayer value (0x69726F74 "tori")',
         'Use verified magic values from binary disassembly - see magic_values table',
         'all'),
    ]

    for g in gotchas:
        c.execute("""
            INSERT OR IGNORE INTO driver_gotchas
            (driver_id, symptom, cause, fix, affected_environments)
            VALUES (?, ?, ?, ?, ?)
        """, g)

    conn.commit()
    conn.close()
    print(f"Created {db_path}")


def main():
    print("Creating new MCP databases...")
    create_anticheat_intel_db()
    create_evasion_techniques_db()
    create_byovd_drivers_db()
    print("\nAll databases created successfully!")

    # List files
    print("\nDatabase files:")
    for db in DATA_DIR.glob("*.db"):
        size = db.stat().st_size
        print(f"  {db.name}: {size:,} bytes")


if __name__ == "__main__":
    main()
