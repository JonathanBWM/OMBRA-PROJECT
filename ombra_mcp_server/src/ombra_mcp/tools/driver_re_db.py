"""
Driver Reverse Engineering Database Connection

Shared database connection and utilities for driver analysis tools.
"""

import sqlite3
from pathlib import Path
from typing import Optional

DATA_DIR = Path(__file__).parent.parent / "data"
DB_PATH = DATA_DIR / "driver_re.db"


def get_conn():
    """Get database connection with row factory."""
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn


def init_driver_re_db():
    """Initialize the driver RE database with full schema."""
    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()

    # Drivers table
    c.execute("""
        CREATE TABLE IF NOT EXISTS drivers (
            id TEXT PRIMARY KEY,
            original_name TEXT NOT NULL,
            analyzed_name TEXT,
            md5 TEXT NOT NULL,
            sha1 TEXT NOT NULL,
            sha256 TEXT NOT NULL UNIQUE,
            imphash TEXT,
            ssdeep TEXT,
            file_size INTEGER NOT NULL,
            image_base INTEGER NOT NULL,
            entry_point_rva INTEGER NOT NULL,
            size_of_image INTEGER NOT NULL,
            timestamp INTEGER,
            machine INTEGER NOT NULL,
            subsystem INTEGER NOT NULL,
            characteristics INTEGER NOT NULL,
            file_version TEXT,
            product_version TEXT,
            company_name TEXT,
            product_name TEXT,
            file_description TEXT,
            original_filename TEXT,
            internal_name TEXT,
            legal_copyright TEXT,
            build_path TEXT,
            pdb_path TEXT,
            linker_version TEXT,
            analysis_status TEXT DEFAULT 'pending',
            analysis_started_at TEXT,
            analysis_completed_at TEXT,
            notes TEXT,
            tags TEXT,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            updated_at TEXT DEFAULT CURRENT_TIMESTAMP
        )
    """)

    # Sections table
    c.execute("""
        CREATE TABLE IF NOT EXISTS sections (
            id TEXT PRIMARY KEY,
            driver_id TEXT NOT NULL,
            name TEXT NOT NULL,
            virtual_address INTEGER NOT NULL,
            virtual_size INTEGER NOT NULL,
            raw_address INTEGER NOT NULL,
            raw_size INTEGER NOT NULL,
            characteristics INTEGER NOT NULL,
            entropy REAL,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (driver_id) REFERENCES drivers(id) ON DELETE CASCADE
        )
    """)

    # Imports table
    c.execute("""
        CREATE TABLE IF NOT EXISTS imports (
            id TEXT PRIMARY KEY,
            driver_id TEXT NOT NULL,
            dll_name TEXT NOT NULL,
            function_name TEXT,
            ordinal INTEGER,
            hint INTEGER,
            iat_rva INTEGER,
            iat_va INTEGER,
            category TEXT,
            subcategory TEXT,
            is_dangerous INTEGER DEFAULT 0,
            danger_reason TEXT,
            description TEXT,
            msdn_url TEXT,
            usage_count INTEGER DEFAULT 0,
            usage_notes TEXT,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (driver_id) REFERENCES drivers(id) ON DELETE CASCADE
        )
    """)

    # Exports table
    c.execute("""
        CREATE TABLE IF NOT EXISTS exports (
            id TEXT PRIMARY KEY,
            driver_id TEXT NOT NULL,
            function_name TEXT,
            ordinal INTEGER NOT NULL,
            rva INTEGER NOT NULL,
            va INTEGER,
            prefix TEXT,
            category TEXT,
            subcategory TEXT,
            is_dangerous INTEGER DEFAULT 0,
            danger_reason TEXT,
            return_type TEXT,
            parameters TEXT,
            calling_convention TEXT,
            description TEXT,
            decompiled_code TEXT,
            pseudocode TEXT,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (driver_id) REFERENCES drivers(id) ON DELETE CASCADE
        )
    """)

    # IOCTLs table
    c.execute("""
        CREATE TABLE IF NOT EXISTS ioctls (
            id TEXT PRIMARY KEY,
            driver_id TEXT NOT NULL,
            name TEXT NOT NULL,
            code INTEGER,
            code_hex TEXT,
            device_type INTEGER,
            function_code INTEGER,
            method INTEGER,
            access INTEGER,
            handler_rva INTEGER,
            handler_va INTEGER,
            handler_function_id TEXT,
            input_struct_id TEXT,
            output_struct_id TEXT,
            min_input_size INTEGER,
            max_input_size INTEGER,
            min_output_size INTEGER,
            max_output_size INTEGER,
            requires_admin INTEGER,
            requires_session INTEGER DEFAULT 1,
            is_vulnerable INTEGER DEFAULT 0,
            vulnerability_type TEXT,
            vulnerability_severity TEXT,
            vulnerability_description TEXT,
            exploitation_notes TEXT,
            cve_ids TEXT,
            description TEXT,
            purpose TEXT,
            has_fast_path INTEGER DEFAULT 0,
            fast_handler_rva INTEGER,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (driver_id) REFERENCES drivers(id) ON DELETE CASCADE
        )
    """)

    # Cross-references table
    c.execute("""
        CREATE TABLE IF NOT EXISTS xrefs (
            id TEXT PRIMARY KEY,
            driver_id TEXT NOT NULL,
            from_rva INTEGER NOT NULL,
            from_va INTEGER,
            from_function_id TEXT,
            to_rva INTEGER NOT NULL,
            to_va INTEGER,
            to_function_id TEXT,
            to_import_id TEXT,
            to_export_id TEXT,
            xref_type TEXT NOT NULL,
            instruction TEXT,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (driver_id) REFERENCES drivers(id) ON DELETE CASCADE
        )
    """)

    # Create indexes
    c.execute("CREATE INDEX IF NOT EXISTS idx_drivers_sha256 ON drivers(sha256)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_drivers_md5 ON drivers(md5)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_drivers_name ON drivers(original_name)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_sections_driver ON sections(driver_id)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_imports_driver ON imports(driver_id)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_imports_function ON imports(function_name)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_imports_dll ON imports(dll_name)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_imports_category ON imports(category)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_imports_dangerous ON imports(is_dangerous)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_exports_driver ON exports(driver_id)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_exports_function ON exports(function_name)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_exports_prefix ON exports(prefix)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_exports_category ON exports(category)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_exports_dangerous ON exports(is_dangerous)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_ioctls_driver ON ioctls(driver_id)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_ioctls_name ON ioctls(name)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_ioctls_code ON ioctls(code)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_ioctls_vulnerable ON ioctls(is_vulnerable)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_xrefs_driver ON xrefs(driver_id)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_xrefs_from_func ON xrefs(from_function_id)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_xrefs_to_func ON xrefs(to_function_id)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_xrefs_to_import ON xrefs(to_import_id)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_xrefs_from_rva ON xrefs(from_rva)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_xrefs_to_rva ON xrefs(to_rva)")

    conn.commit()
    conn.close()
