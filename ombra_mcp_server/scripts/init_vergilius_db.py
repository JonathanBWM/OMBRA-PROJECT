#!/usr/bin/env python3
"""
Initialize the Vergilius database with schema and version data.
"""

import sqlite3
from pathlib import Path

DB_PATH = Path(__file__).parent.parent / "src" / "ombra_mcp" / "data" / "vergilius.db"

SCHEMA = """
-- Windows versions we track
CREATE TABLE IF NOT EXISTS os_versions (
    id INTEGER PRIMARY KEY,
    os_family TEXT NOT NULL,
    version TEXT NOT NULL,
    codename TEXT,
    build TEXT,
    arch TEXT NOT NULL DEFAULT 'x64',
    vergilius_path TEXT NOT NULL,
    short_name TEXT NOT NULL,
    UNIQUE(os_family, version, arch)
);

-- Structure/Enum/Union definitions
CREATE TABLE IF NOT EXISTS type_definitions (
    id INTEGER PRIMARY KEY,
    version_id INTEGER NOT NULL,
    type_kind TEXT NOT NULL,
    name TEXT NOT NULL,
    size_bytes INTEGER,
    definition TEXT,
    vergilius_url TEXT,
    scraped_at TEXT,
    FOREIGN KEY (version_id) REFERENCES os_versions(id),
    UNIQUE(version_id, name)
);

-- Structure fields with offsets
CREATE TABLE IF NOT EXISTS type_fields (
    id INTEGER PRIMARY KEY,
    type_id INTEGER NOT NULL,
    offset_dec INTEGER NOT NULL,
    name TEXT NOT NULL,
    field_type TEXT NOT NULL,
    bit_field TEXT,
    array_size INTEGER,
    FOREIGN KEY (type_id) REFERENCES type_definitions(id)
);

-- Cross-references (Used in)
CREATE TABLE IF NOT EXISTS type_references (
    id INTEGER PRIMARY KEY,
    type_id INTEGER NOT NULL,
    used_by_name TEXT NOT NULL,
    FOREIGN KEY (type_id) REFERENCES type_definitions(id)
);

-- Quick lookup for hypervisor-critical offsets
CREATE TABLE IF NOT EXISTS critical_offsets (
    id INTEGER PRIMARY KEY,
    version_id INTEGER NOT NULL,
    struct_name TEXT NOT NULL,
    field_name TEXT NOT NULL,
    offset_dec INTEGER NOT NULL,
    offset_hex TEXT NOT NULL,
    use_case TEXT,
    FOREIGN KEY (version_id) REFERENCES os_versions(id),
    UNIQUE(version_id, struct_name, field_name)
);

-- Indexes for performance
CREATE INDEX IF NOT EXISTS idx_types_name ON type_definitions(name);
CREATE INDEX IF NOT EXISTS idx_types_version ON type_definitions(version_id);
CREATE INDEX IF NOT EXISTS idx_fields_type ON type_fields(type_id);
CREATE INDEX IF NOT EXISTS idx_fields_name ON type_fields(name);
CREATE INDEX IF NOT EXISTS idx_critical_struct ON critical_offsets(struct_name);
CREATE INDEX IF NOT EXISTS idx_refs_type ON type_references(type_id);
"""

VERSIONS = [
    ("windows-10", "2004", "Vibranium R1", "10.0.19041.264", "/kernels/x64/windows-10/2004", "win10-2004"),
    ("windows-10", "20h2", "Vibranium R2", "10.0.19042.508", "/kernels/x64/windows-10/20h2", "win10-20h2"),
    ("windows-10", "21h1", "Vibranium R3", "10.0.19043.928", "/kernels/x64/windows-10/21h1", "win10-21h1"),
    ("windows-10", "21h2", "Vibranium R4", "10.0.19044.1288", "/kernels/x64/windows-10/21h2", "win10-21h2"),
    ("windows-10", "22h2", "Vibranium R5", "10.0.19045.2965", "/kernels/x64/windows-10/22h2", "win10-22h2"),
    ("windows-11", "21h2", "Cobalt", "10.0.22000.194", "/kernels/x64/windows-11/21h2", "win11-21h2"),
    ("windows-11", "22h2", "Nickel R1", "10.0.22621.382", "/kernels/x64/windows-11/22h2", "win11-22h2"),
]


def init_database():
    """Initialize database with schema and seed data."""
    print(f"[*] Creating database at {DB_PATH}")

    conn = sqlite3.connect(DB_PATH)
    cur = conn.cursor()

    # Create schema
    cur.executescript(SCHEMA)
    print("[+] Schema created")

    # Seed versions
    for os_family, version, codename, build, path, short_name in VERSIONS:
        cur.execute("""
            INSERT OR IGNORE INTO os_versions
            (os_family, version, codename, build, arch, vergilius_path, short_name)
            VALUES (?, ?, ?, ?, 'x64', ?, ?)
        """, (os_family, version, codename, build, path, short_name))

    conn.commit()

    # Verify
    cur.execute("SELECT short_name, codename FROM os_versions ORDER BY id")
    versions = cur.fetchall()
    print(f"[+] Seeded {len(versions)} Windows versions:")
    for short_name, codename in versions:
        print(f"    - {short_name} ({codename})")

    conn.close()
    print("[+] Database initialized successfully")


if __name__ == "__main__":
    init_database()
