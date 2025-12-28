#!/usr/bin/env python3
"""Simple database test."""

import asyncio
import aiosqlite
from pathlib import Path


async def test():
    db_path = Path(__file__).parent / "test_simple.db"
    if db_path.exists():
        db_path.unlink()

    print(f"Creating database at: {db_path}")

    # Connect
    conn = await aiosqlite.connect(db_path)

    # Load schema
    schema_path = Path(__file__).parent / "src/driver_re_mcp/database/schema.sql"
    with open(schema_path, 'r') as f:
        schema = f.read()

    print("Executing schema...")
    await conn.executescript(schema)

    print("Schema loaded successfully!")

    # Test insert
    print("\nInserting test driver...")
    await conn.execute("""
        INSERT INTO drivers (id, original_name, md5, sha1, sha256, file_size, image_base,
                           entry_point_rva, size_of_image, machine, subsystem, characteristics)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    """, ('test-id-123', 'test.sys', 'abc', 'def', 'ghi123', 1234, 0x140000000,
          0x1000, 0x50000, 0x8664, 1, 0x2022))

    await conn.commit()

    # Test query
    print("Querying driver...")
    cursor = await conn.execute("SELECT original_name, sha256 FROM drivers WHERE id = ?", ('test-id-123',))
    row = await cursor.fetchone()
    print(f"Found driver: {row[0]} (SHA256: {row[1]})")

    # Close
    await conn.close()

    print("\n✓ Simple test passed!")


if __name__ == "__main__":
    asyncio.run(test())
