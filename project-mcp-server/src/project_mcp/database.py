"""
Database module for Project Management MCP.

Provides:
- Async SQLite connection management
- Schema initialization
- Common query helpers
"""

import aiosqlite
import sqlite3
import os
from pathlib import Path
from typing import Optional, Any
from contextlib import asynccontextmanager

# Database path
DATA_DIR = Path(__file__).parent / "data"
DB_PATH = DATA_DIR / "project_mgmt.db"
SCHEMA_PATH = DATA_DIR / "schema.sql"

# Connection pool (simple implementation)
_connection: Optional[aiosqlite.Connection] = None


async def get_connection() -> aiosqlite.Connection:
    """Get or create the database connection."""
    global _connection
    if _connection is None:
        # Ensure data directory exists
        DATA_DIR.mkdir(parents=True, exist_ok=True)

        # Connect with row factory for dict-like access
        _connection = await aiosqlite.connect(str(DB_PATH))
        _connection.row_factory = aiosqlite.Row

        # Enable foreign keys
        await _connection.execute("PRAGMA foreign_keys = ON")

        # Initialize schema if needed
        await _init_schema(_connection)

    return _connection


async def _init_schema(conn: aiosqlite.Connection) -> None:
    """Initialize the database schema if tables don't exist."""
    # Check if tables exist
    cursor = await conn.execute(
        "SELECT name FROM sqlite_master WHERE type='table' AND name='components'"
    )
    if await cursor.fetchone() is not None:
        return  # Schema already exists

    # Read and execute schema
    if SCHEMA_PATH.exists():
        schema_sql = SCHEMA_PATH.read_text()
        # Split by semicolons and execute each statement
        # (aiosqlite.executescript doesn't work with async)
        statements = [s.strip() for s in schema_sql.split(';') if s.strip()]
        for statement in statements:
            try:
                await conn.execute(statement)
            except Exception as e:
                # Ignore errors for CREATE IF NOT EXISTS
                if "already exists" not in str(e).lower():
                    print(f"Schema error: {e}")
        await conn.commit()


async def close_connection() -> None:
    """Close the database connection."""
    global _connection
    if _connection is not None:
        await _connection.close()
        _connection = None


@asynccontextmanager
async def transaction():
    """Context manager for database transactions."""
    conn = await get_connection()
    try:
        yield conn
        await conn.commit()
    except Exception:
        await conn.rollback()
        raise


async def execute(query: str, params: tuple = ()) -> aiosqlite.Cursor:
    """Execute a query and return the cursor."""
    conn = await get_connection()
    return await conn.execute(query, params)


async def execute_many(query: str, params_list: list) -> None:
    """Execute a query with multiple parameter sets."""
    conn = await get_connection()
    await conn.executemany(query, params_list)
    await conn.commit()


async def fetch_one(query: str, params: tuple = ()) -> Optional[dict]:
    """Fetch a single row as a dictionary."""
    conn = await get_connection()
    cursor = await conn.execute(query, params)
    row = await cursor.fetchone()
    if row is None:
        return None
    return dict(row)


async def fetch_all(query: str, params: tuple = ()) -> list[dict]:
    """Fetch all rows as dictionaries."""
    conn = await get_connection()
    cursor = await conn.execute(query, params)
    rows = await cursor.fetchall()
    return [dict(row) for row in rows]


async def insert(table: str, data: dict) -> int:
    """Insert a row and return the last row ID."""
    columns = ", ".join(data.keys())
    placeholders = ", ".join("?" * len(data))
    query = f"INSERT INTO {table} ({columns}) VALUES ({placeholders})"

    conn = await get_connection()
    cursor = await conn.execute(query, tuple(data.values()))
    await conn.commit()
    return cursor.lastrowid


async def update(table: str, data: dict, where: str, where_params: tuple) -> int:
    """Update rows and return the number of affected rows."""
    set_clause = ", ".join(f"{k} = ?" for k in data.keys())
    query = f"UPDATE {table} SET {set_clause} WHERE {where}"

    conn = await get_connection()
    cursor = await conn.execute(query, tuple(data.values()) + where_params)
    await conn.commit()
    return cursor.rowcount


async def delete(table: str, where: str, where_params: tuple) -> int:
    """Delete rows and return the number of affected rows."""
    query = f"DELETE FROM {table} WHERE {where}"

    conn = await get_connection()
    cursor = await conn.execute(query, where_params)
    await conn.commit()
    return cursor.rowcount


async def count(table: str, where: str = "1=1", where_params: tuple = ()) -> int:
    """Count rows in a table."""
    query = f"SELECT COUNT(*) as cnt FROM {table} WHERE {where}"
    result = await fetch_one(query, where_params)
    return result["cnt"] if result else 0


async def search_fts(table_fts: str, query: str, limit: int = 20) -> list[dict]:
    """Search using full-text search."""
    # Escape special characters in FTS query
    safe_query = query.replace('"', '""')
    fts_query = f'SELECT rowid, * FROM {table_fts} WHERE {table_fts} MATCH ? LIMIT ?'
    return await fetch_all(fts_query, (safe_query, limit))


def sync_get_connection() -> sqlite3.Connection:
    """Get a synchronous connection for use in non-async contexts."""
    DATA_DIR.mkdir(parents=True, exist_ok=True)
    conn = sqlite3.connect(str(DB_PATH))
    conn.row_factory = sqlite3.Row
    conn.execute("PRAGMA foreign_keys = ON")
    return conn


def sync_init_db() -> None:
    """Initialize the database synchronously (for startup)."""
    conn = sync_get_connection()

    # Check if tables exist
    cursor = conn.execute(
        "SELECT name FROM sqlite_master WHERE type='table' AND name='components'"
    )
    if cursor.fetchone() is not None:
        conn.close()
        return  # Schema already exists

    # Read and execute schema
    if SCHEMA_PATH.exists():
        schema_sql = SCHEMA_PATH.read_text()
        conn.executescript(schema_sql)
        conn.commit()

    conn.close()
