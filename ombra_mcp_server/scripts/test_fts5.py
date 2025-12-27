#!/usr/bin/env python3
"""Test FTS5 functionality and triggers."""

import sqlite3
from pathlib import Path

DB_PATH = Path(__file__).parent.parent / "src" / "ombra_mcp" / "data" / "intel_sdm.db"

def test_triggers():
    """Test that triggers keep FTS tables in sync."""
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()

    print("Testing FTS5 triggers...")

    # Test INSERT trigger
    cursor.execute("""
        INSERT INTO vmcs_fields (name, encoding, width, category, description)
        VALUES ('TEST_FIELD', 9999, '64', 'test', 'This is a test field for FTS5 trigger verification')
    """)

    cursor.execute("""
        SELECT name, description FROM vmcs_fields_fts
        WHERE vmcs_fields_fts MATCH 'trigger verification'
    """)
    result = cursor.fetchone()
    assert result is not None, "INSERT trigger failed"
    assert result[0] == 'TEST_FIELD', f"Expected TEST_FIELD, got {result[0]}"
    print("  ✓ INSERT trigger works")

    # Test UPDATE trigger
    cursor.execute("""
        UPDATE vmcs_fields
        SET description = 'Updated test field description with FTS5'
        WHERE name = 'TEST_FIELD'
    """)

    cursor.execute("""
        SELECT name, description FROM vmcs_fields_fts
        WHERE vmcs_fields_fts MATCH 'Updated test'
    """)
    result = cursor.fetchone()
    assert result is not None, "UPDATE trigger failed"
    print("  ✓ UPDATE trigger works")

    # Test DELETE trigger (cleanup)
    cursor.execute("DELETE FROM vmcs_fields WHERE name = 'TEST_FIELD'")

    cursor.execute("""
        SELECT name FROM vmcs_fields_fts
        WHERE vmcs_fields_fts MATCH 'TEST_FIELD'
    """)
    result = cursor.fetchone()
    assert result is None, "DELETE trigger failed - record still in FTS"
    print("  ✓ DELETE trigger works")

    conn.commit()
    conn.close()

def test_search_quality():
    """Test search quality with various queries."""
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()

    print("\nTesting search quality...")

    # Test case-insensitive search
    cursor.execute("""
        SELECT name FROM vmcs_fields_fts
        WHERE vmcs_fields_fts MATCH 'GUEST'
        LIMIT 3
    """)
    results = cursor.fetchall()
    assert len(results) > 0, "Case-insensitive search failed"
    print(f"  ✓ Case-insensitive search: {len(results)} results for 'GUEST'")

    # Test prefix search
    cursor.execute("""
        SELECT name FROM vmcs_fields_fts
        WHERE vmcs_fields_fts MATCH 'guest*'
        LIMIT 5
    """)
    results = cursor.fetchall()
    print(f"  ✓ Prefix search: {len(results)} results for 'guest*'")

    # Test phrase search
    cursor.execute("""
        SELECT name FROM exit_reasons_fts
        WHERE exit_reasons_fts MATCH '"ept violation"'
    """)
    results = cursor.fetchall()
    print(f"  ✓ Phrase search: {len(results)} results for '\"ept violation\"'")

    # Test NOT operator
    cursor.execute("""
        SELECT name FROM vmcs_fields_fts
        WHERE vmcs_fields_fts MATCH 'guest NOT selector'
        LIMIT 5
    """)
    results = cursor.fetchall()
    print(f"  ✓ NOT operator: {len(results)} results for 'guest NOT selector'")

    # Test ranking (BM25)
    cursor.execute("""
        SELECT name, rank FROM vmcs_fields_fts
        WHERE vmcs_fields_fts MATCH 'control'
        ORDER BY rank
        LIMIT 3
    """)
    results = cursor.fetchall()
    print(f"  ✓ Ranking: Top 3 results for 'control'")
    for name, rank in results:
        print(f"    - {name} (rank: {rank:.4f})")

    conn.close()

def benchmark_search():
    """Compare LIKE vs FTS5 performance."""
    import time

    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()

    print("\nBenchmarking search performance...")

    # LIKE query
    start = time.time()
    cursor.execute("""
        SELECT name, description FROM vmcs_fields
        WHERE name LIKE '%GUEST%' OR description LIKE '%GUEST%'
    """)
    like_results = cursor.fetchall()
    like_time = time.time() - start

    # FTS5 query
    start = time.time()
    cursor.execute("""
        SELECT name, description FROM vmcs_fields_fts
        WHERE vmcs_fields_fts MATCH 'GUEST'
    """)
    fts_results = cursor.fetchall()
    fts_time = time.time() - start

    print(f"  LIKE query:  {like_time*1000:.2f}ms ({len(like_results)} results)")
    print(f"  FTS5 query:  {fts_time*1000:.2f}ms ({len(fts_results)} results)")
    print(f"  Speedup:     {like_time/fts_time:.1f}x faster")

    conn.close()

if __name__ == "__main__":
    try:
        test_triggers()
        test_search_quality()
        benchmark_search()
        print("\n✅ All FTS5 tests passed!")
    except AssertionError as e:
        print(f"\n❌ Test failed: {e}")
    except Exception as e:
        print(f"\n❌ Error: {e}")
