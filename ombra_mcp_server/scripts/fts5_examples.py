#!/usr/bin/env python3
"""
Examples of FTS5 search queries for the OmbraMCP server.

These queries demonstrate the improved search capabilities
that should be used in MCP tools instead of LIKE queries.
"""

import sqlite3
from pathlib import Path

DB_PATH = Path(__file__).parent.parent / "src" / "ombra_mcp" / "data" / "intel_sdm.db"

def print_section(title):
    """Print a section header."""
    print(f"\n{'='*60}")
    print(f"  {title}")
    print('='*60)

def example_vmcs_search():
    """Example VMCS field searches."""
    print_section("VMCS Field Search Examples")

    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()

    # Example 1: Simple term search
    print("\n1. Find all VMCS fields related to 'guest rip':")
    cursor.execute("""
        SELECT name, category, encoding
        FROM vmcs_fields_fts
        WHERE vmcs_fields_fts MATCH 'guest rip'
    """)
    for row in cursor.fetchall():
        print(f"   {row[0]:30s} {row[1]:15s} 0x{row[2]:04X}")

    # Example 2: Control fields search
    print("\n2. Find processor-based control fields:")
    cursor.execute("""
        SELECT name, category
        FROM vmcs_fields_fts
        WHERE vmcs_fields_fts MATCH 'processor AND control'
        LIMIT 5
    """)
    for row in cursor.fetchall():
        print(f"   {row[0]:40s} {row[1]}")

    # Example 3: Guest state fields
    print("\n3. All guest state segment registers:")
    cursor.execute("""
        SELECT name, encoding
        FROM vmcs_fields_fts
        WHERE vmcs_fields_fts MATCH 'guest AND (selector OR base OR limit)'
        AND category = 'guest_state'
        LIMIT 10
    """)
    for row in cursor.fetchall():
        print(f"   {row[0]:30s} 0x{row[1]:04X}")

    conn.close()

def example_exit_reason_search():
    """Example exit reason searches."""
    print_section("Exit Reason Search Examples")

    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()

    # Example 1: EPT-related exits
    print("\n1. All EPT-related VM-exits:")
    cursor.execute("""
        SELECT name, reason_number, description
        FROM exit_reasons_fts
        WHERE exit_reasons_fts MATCH 'ept'
    """)
    for row in cursor.fetchall():
        print(f"   [{row[1]:2d}] {row[0]:25s} - {row[2]}")

    # Example 2: Instruction-related exits
    print("\n2. Instruction-related VM-exits:")
    cursor.execute("""
        SELECT name, reason_number
        FROM exit_reasons_fts
        WHERE exit_reasons_fts MATCH 'instruction'
        LIMIT 5
    """)
    for row in cursor.fetchall():
        print(f"   [{row[1]:2d}] {row[0]}")

    # Example 3: Search in handling notes
    print("\n3. Exits with 'qualification' in handling notes:")
    cursor.execute("""
        SELECT name, reason_number
        FROM exit_reasons_fts
        WHERE handling_notes MATCH 'qualification'
        LIMIT 5
    """)
    for row in cursor.fetchall():
        print(f"   [{row[1]:2d}] {row[0]}")

    conn.close()

def example_msr_search():
    """Example MSR searches."""
    print_section("MSR Search Examples")

    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()

    # Example 1: VMX capability MSRs
    print("\n1. All VMX capability MSRs:")
    cursor.execute("""
        SELECT name, address, category
        FROM msrs_fts
        WHERE msrs_fts MATCH 'IA32_VMX*'
    """)
    for row in cursor.fetchall():
        print(f"   {row[0]:35s} 0x{row[1]:03X}  [{row[2]}]")

    # Example 2: Feature control MSRs
    print("\n2. Feature and control MSRs:")
    cursor.execute("""
        SELECT name, address, description
        FROM msrs_fts
        WHERE msrs_fts MATCH 'feature OR control'
        LIMIT 5
    """)
    for row in cursor.fetchall():
        print(f"   {row[0]:30s} 0x{row[1]:03X}")
        print(f"      {row[2]}")

    conn.close()

def example_ranked_search():
    """Example of ranked search results."""
    print_section("Ranked Search (BM25 Algorithm)")

    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()

    print("\n1. Best matches for 'VM execution control' (ranked by relevance):")
    cursor.execute("""
        SELECT name, rank
        FROM vmcs_fields_fts
        WHERE vmcs_fields_fts MATCH 'VM execution control'
        ORDER BY rank
        LIMIT 5
    """)
    for i, row in enumerate(cursor.fetchall(), 1):
        print(f"   {i}. {row[0]:40s} (score: {row[1]:.4f})")

    conn.close()

def example_advanced_queries():
    """Example of advanced FTS5 features."""
    print_section("Advanced FTS5 Features")

    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()

    # Example 1: Phrase search
    print("\n1. Exact phrase search '\"VM exit\"':")
    cursor.execute("""
        SELECT name, category
        FROM vmcs_fields_fts
        WHERE vmcs_fields_fts MATCH '"VM exit"'
        LIMIT 3
    """)
    for row in cursor.fetchall():
        print(f"   {row[0]:40s} {row[1]}")

    # Example 2: Prefix wildcard
    print("\n2. Prefix search 'GUEST_CR*':")
    cursor.execute("""
        SELECT name, encoding
        FROM vmcs_fields_fts
        WHERE vmcs_fields_fts MATCH 'GUEST_CR*'
    """)
    for row in cursor.fetchall():
        print(f"   {row[0]:30s} 0x{row[1]:04X}")

    # Example 3: Column-specific search
    print("\n3. Search only in category field:")
    cursor.execute("""
        SELECT name, category
        FROM vmcs_fields_fts
        WHERE category MATCH 'host_state'
        LIMIT 5
    """)
    for row in cursor.fetchall():
        print(f"   {row[0]:30s} {row[1]}")

    # Example 4: Negation
    print("\n4. Guest fields NOT related to selectors:")
    cursor.execute("""
        SELECT name
        FROM vmcs_fields_fts
        WHERE vmcs_fields_fts MATCH 'guest NOT selector'
        LIMIT 5
    """)
    for row in cursor.fetchall():
        print(f"   {row[0]}")

    conn.close()

def main():
    """Run all examples."""
    print("\n" + "="*60)
    print("  FTS5 Full-Text Search Examples for OmbraMCP")
    print("="*60)
    print("\nThese examples show how to use the new FTS5 tables")
    print("instead of slow LIKE queries in MCP tools.")

    example_vmcs_search()
    example_exit_reason_search()
    example_msr_search()
    example_ranked_search()
    example_advanced_queries()

    print("\n" + "="*60)
    print("  Use these patterns in your MCP tool queries!")
    print("="*60 + "\n")

if __name__ == "__main__":
    main()
