#!/usr/bin/env python3
"""
Migration script to add FTS5 full-text search to intel_sdm.db

This script adds FTS5 virtual tables for fast text search on:
- vmcs_fields (name + description)
- exit_reasons (name + description + handling_notes)
- msrs (name + description)

Safe to run multiple times (idempotent).
"""

import sqlite3
import sys
from pathlib import Path

# Database path
DB_PATH = Path(__file__).parent.parent / "src" / "ombra_mcp" / "data" / "intel_sdm.db"

def create_fts5_tables(conn: sqlite3.Connection) -> None:
    """Create FTS5 virtual tables for full-text search."""
    cursor = conn.cursor()

    print("Creating FTS5 virtual tables...")

    # Check if vmcs_fields_fts exists
    cursor.execute("""
        SELECT name FROM sqlite_master
        WHERE type='table' AND name='vmcs_fields_fts'
    """)
    if cursor.fetchone():
        print("  - vmcs_fields_fts already exists, dropping...")
        cursor.execute("DROP TABLE vmcs_fields_fts")

    # Create vmcs_fields FTS5 table
    cursor.execute("""
        CREATE VIRTUAL TABLE vmcs_fields_fts USING fts5(
            name,
            description,
            category,
            subcategory,
            encoding UNINDEXED,
            width UNINDEXED,
            content='vmcs_fields',
            content_rowid='id'
        )
    """)
    print("  ✓ Created vmcs_fields_fts")

    # Check if exit_reasons_fts exists
    cursor.execute("""
        SELECT name FROM sqlite_master
        WHERE type='table' AND name='exit_reasons_fts'
    """)
    if cursor.fetchone():
        print("  - exit_reasons_fts already exists, dropping...")
        cursor.execute("DROP TABLE exit_reasons_fts")

    # Create exit_reasons FTS5 table
    cursor.execute("""
        CREATE VIRTUAL TABLE exit_reasons_fts USING fts5(
            name,
            description,
            handling_notes,
            qualification_format,
            reason_number UNINDEXED,
            content='exit_reasons',
            content_rowid='id'
        )
    """)
    print("  ✓ Created exit_reasons_fts")

    # Check if msrs_fts exists
    cursor.execute("""
        SELECT name FROM sqlite_master
        WHERE type='table' AND name='msrs_fts'
    """)
    if cursor.fetchone():
        print("  - msrs_fts already exists, dropping...")
        cursor.execute("DROP TABLE msrs_fts")

    # Create msrs FTS5 table
    cursor.execute("""
        CREATE VIRTUAL TABLE msrs_fts USING fts5(
            name,
            description,
            category,
            bit_fields,
            address UNINDEXED,
            content='msrs',
            content_rowid='id'
        )
    """)
    print("  ✓ Created msrs_fts")

    conn.commit()

def create_triggers(conn: sqlite3.Connection) -> None:
    """Create triggers to keep FTS5 tables in sync with base tables."""
    cursor = conn.cursor()

    print("\nCreating triggers for automatic FTS5 sync...")

    # Drop existing triggers if they exist
    for table in ['vmcs_fields', 'exit_reasons', 'msrs']:
        for action in ['insert', 'delete', 'update']:
            trigger_name = f"{table}_{action}_fts"
            cursor.execute(f"DROP TRIGGER IF EXISTS {trigger_name}")

    # vmcs_fields triggers
    cursor.execute("""
        CREATE TRIGGER vmcs_fields_insert_fts AFTER INSERT ON vmcs_fields BEGIN
            INSERT INTO vmcs_fields_fts(rowid, name, description, category, subcategory, encoding, width)
            VALUES (new.id, new.name, new.description, new.category, new.subcategory, new.encoding, new.width);
        END
    """)

    cursor.execute("""
        CREATE TRIGGER vmcs_fields_delete_fts AFTER DELETE ON vmcs_fields BEGIN
            INSERT INTO vmcs_fields_fts(vmcs_fields_fts, rowid, name, description, category, subcategory, encoding, width)
            VALUES ('delete', old.id, old.name, old.description, old.category, old.subcategory, old.encoding, old.width);
        END
    """)

    cursor.execute("""
        CREATE TRIGGER vmcs_fields_update_fts AFTER UPDATE ON vmcs_fields BEGIN
            INSERT INTO vmcs_fields_fts(vmcs_fields_fts, rowid, name, description, category, subcategory, encoding, width)
            VALUES ('delete', old.id, old.name, old.description, old.category, old.subcategory, old.encoding, old.width);
            INSERT INTO vmcs_fields_fts(rowid, name, description, category, subcategory, encoding, width)
            VALUES (new.id, new.name, new.description, new.category, new.subcategory, new.encoding, new.width);
        END
    """)
    print("  ✓ Created vmcs_fields triggers")

    # exit_reasons triggers
    cursor.execute("""
        CREATE TRIGGER exit_reasons_insert_fts AFTER INSERT ON exit_reasons BEGIN
            INSERT INTO exit_reasons_fts(rowid, name, description, handling_notes, qualification_format, reason_number)
            VALUES (new.id, new.name, new.description, new.handling_notes, new.qualification_format, new.reason_number);
        END
    """)

    cursor.execute("""
        CREATE TRIGGER exit_reasons_delete_fts AFTER DELETE ON exit_reasons BEGIN
            INSERT INTO exit_reasons_fts(exit_reasons_fts, rowid, name, description, handling_notes, qualification_format, reason_number)
            VALUES ('delete', old.id, old.name, old.description, old.handling_notes, old.qualification_format, old.reason_number);
        END
    """)

    cursor.execute("""
        CREATE TRIGGER exit_reasons_update_fts AFTER UPDATE ON exit_reasons BEGIN
            INSERT INTO exit_reasons_fts(exit_reasons_fts, rowid, name, description, handling_notes, qualification_format, reason_number)
            VALUES ('delete', old.id, old.name, old.description, old.handling_notes, old.qualification_format, old.reason_number);
            INSERT INTO exit_reasons_fts(rowid, name, description, handling_notes, qualification_format, reason_number)
            VALUES (new.id, new.name, new.description, new.handling_notes, new.qualification_format, new.reason_number);
        END
    """)
    print("  ✓ Created exit_reasons triggers")

    # msrs triggers
    cursor.execute("""
        CREATE TRIGGER msrs_insert_fts AFTER INSERT ON msrs BEGIN
            INSERT INTO msrs_fts(rowid, name, description, category, bit_fields, address)
            VALUES (new.id, new.name, new.description, new.category, new.bit_fields, new.address);
        END
    """)

    cursor.execute("""
        CREATE TRIGGER msrs_delete_fts AFTER DELETE ON msrs BEGIN
            INSERT INTO msrs_fts(msrs_fts, rowid, name, description, category, bit_fields, address)
            VALUES ('delete', old.id, old.name, old.description, old.category, old.bit_fields, old.address);
        END
    """)

    cursor.execute("""
        CREATE TRIGGER msrs_update_fts AFTER UPDATE ON msrs BEGIN
            INSERT INTO msrs_fts(msrs_fts, rowid, name, description, category, bit_fields, address)
            VALUES ('delete', old.id, old.name, old.description, old.category, old.bit_fields, old.address);
            INSERT INTO msrs_fts(rowid, name, description, category, bit_fields, address)
            VALUES (new.id, new.name, new.description, new.category, new.bit_fields, new.address);
        END
    """)
    print("  ✓ Created msrs triggers")

    conn.commit()

def populate_fts_tables(conn: sqlite3.Connection) -> None:
    """Populate FTS5 tables with existing data."""
    cursor = conn.cursor()

    print("\nPopulating FTS5 tables with existing data...")

    # Populate vmcs_fields_fts
    cursor.execute("""
        INSERT INTO vmcs_fields_fts(rowid, name, description, category, subcategory, encoding, width)
        SELECT id, name, description, category, subcategory, encoding, width
        FROM vmcs_fields
    """)
    vmcs_count = cursor.rowcount
    print(f"  ✓ Populated vmcs_fields_fts with {vmcs_count} rows")

    # Populate exit_reasons_fts
    cursor.execute("""
        INSERT INTO exit_reasons_fts(rowid, name, description, handling_notes, qualification_format, reason_number)
        SELECT id, name, description, handling_notes, qualification_format, reason_number
        FROM exit_reasons
    """)
    exit_count = cursor.rowcount
    print(f"  ✓ Populated exit_reasons_fts with {exit_count} rows")

    # Populate msrs_fts
    cursor.execute("""
        INSERT INTO msrs_fts(rowid, name, description, category, bit_fields, address)
        SELECT id, name, description, category, bit_fields, address
        FROM msrs
    """)
    msr_count = cursor.rowcount
    print(f"  ✓ Populated msrs_fts with {msr_count} rows")

    conn.commit()

def verify_fts_search(conn: sqlite3.Connection) -> None:
    """Run test queries to verify FTS5 is working."""
    cursor = conn.cursor()

    print("\nVerifying FTS5 search functionality...")

    # Test vmcs_fields_fts
    cursor.execute("""
        SELECT name, category FROM vmcs_fields_fts
        WHERE vmcs_fields_fts MATCH 'guest rip'
    """)
    results = cursor.fetchall()
    print(f"  ✓ VMCS search for 'guest rip': {len(results)} results")
    if results:
        print(f"    Example: {results[0]}")

    # Test exit_reasons_fts
    cursor.execute("""
        SELECT name, reason_number FROM exit_reasons_fts
        WHERE exit_reasons_fts MATCH 'cpuid'
    """)
    results = cursor.fetchall()
    print(f"  ✓ Exit reasons search for 'cpuid': {len(results)} results")
    if results:
        print(f"    Example: {results[0]}")

    # Test msrs_fts
    cursor.execute("""
        SELECT name, category FROM msrs_fts
        WHERE msrs_fts MATCH 'IA32_VMX'
    """)
    results = cursor.fetchall()
    print(f"  ✓ MSR search for 'IA32_VMX': {len(results)} results")
    if results:
        print(f"    Example: {results[0]}")

    # Test phrase search
    cursor.execute("""
        SELECT name FROM vmcs_fields_fts
        WHERE vmcs_fields_fts MATCH '"control fields"'
    """)
    results = cursor.fetchall()
    print(f"  ✓ Phrase search for '\"control fields\"': {len(results)} results")

    # Test boolean operators
    cursor.execute("""
        SELECT name FROM exit_reasons_fts
        WHERE exit_reasons_fts MATCH 'ept OR violation'
    """)
    results = cursor.fetchall()
    print(f"  ✓ Boolean search for 'ept OR violation': {len(results)} results")

def main():
    """Run the migration."""
    if not DB_PATH.exists():
        print(f"Error: Database not found at {DB_PATH}")
        sys.exit(1)

    print(f"Connecting to database: {DB_PATH}")
    conn = sqlite3.connect(DB_PATH)

    try:
        create_fts5_tables(conn)
        create_triggers(conn)
        populate_fts_tables(conn)
        verify_fts_search(conn)

        print("\n✅ Migration complete! FTS5 full-text search is now enabled.")
        print("\nExample queries:")
        print("  SELECT * FROM vmcs_fields_fts WHERE vmcs_fields_fts MATCH 'guest state';")
        print("  SELECT * FROM exit_reasons_fts WHERE exit_reasons_fts MATCH 'ept violation';")
        print("  SELECT * FROM msrs_fts WHERE msrs_fts MATCH 'IA32_VMX_BASIC';")

    except Exception as e:
        print(f"\n❌ Error during migration: {e}")
        conn.rollback()
        sys.exit(1)
    finally:
        conn.close()

if __name__ == "__main__":
    main()
