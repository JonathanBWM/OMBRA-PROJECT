"""Async SQLite connection management for Driver RE MCP."""

import aiosqlite
from contextlib import asynccontextmanager
from pathlib import Path
from typing import AsyncGenerator, Optional
import logging

logger = logging.getLogger(__name__)


class DatabaseManager:
    """Manages async SQLite database connections."""

    def __init__(self, db_path: str | Path):
        self.db_path = Path(db_path)
        self._connection: Optional[aiosqlite.Connection] = None

    async def connect(self) -> aiosqlite.Connection:
        """Establish database connection."""
        if self._connection is None:
            self._connection = await aiosqlite.connect(
                self.db_path,
                isolation_level=None  # Autocommit mode
            )
            # Enable foreign keys
            await self._connection.execute("PRAGMA foreign_keys = ON")
            # Use WAL mode for better concurrency
            await self._connection.execute("PRAGMA journal_mode = WAL")
            logger.info(f"Connected to database: {self.db_path}")
        return self._connection

    async def close(self):
        """Close database connection."""
        if self._connection:
            await self._connection.close()
            self._connection = None
            logger.info("Database connection closed")

    async def init_db(self, schema_path: Optional[Path] = None):
        """Initialize database with schema."""
        if schema_path is None:
            schema_path = Path(__file__).parent / "schema.sql"

        if not schema_path.exists():
            raise FileNotFoundError(f"Schema file not found: {schema_path}")

        logger.info(f"Initializing database with schema: {schema_path}")

        conn = await self.connect()

        # Read schema
        with open(schema_path, 'r') as f:
            schema_sql = f.read()

        # Execute schema using executescript (handles triggers and complex SQL)
        await conn.executescript(schema_sql)
        await conn.commit()
        logger.info("Database initialized successfully")

    @asynccontextmanager
    async def transaction(self) -> AsyncGenerator[aiosqlite.Connection, None]:
        """Context manager for database transactions."""
        conn = await self.connect()
        try:
            await conn.execute("BEGIN")
            yield conn
            await conn.commit()
        except Exception:
            await conn.rollback()
            raise

    async def execute(self, query: str, params: tuple = ()) -> aiosqlite.Cursor:
        """Execute a single query."""
        conn = await self.connect()
        return await conn.execute(query, params)

    async def execute_many(self, query: str, params_list: list[tuple]):
        """Execute query with multiple parameter sets."""
        conn = await self.connect()
        await conn.executemany(query, params_list)
        await conn.commit()

    async def fetch_one(self, query: str, params: tuple = ()) -> Optional[dict]:
        """Fetch single row as dict."""
        conn = await self.connect()
        conn.row_factory = aiosqlite.Row
        async with conn.execute(query, params) as cursor:
            row = await cursor.fetchone()
            return dict(row) if row else None

    async def fetch_all(self, query: str, params: tuple = ()) -> list[dict]:
        """Fetch all rows as list of dicts."""
        conn = await self.connect()
        conn.row_factory = aiosqlite.Row
        async with conn.execute(query, params) as cursor:
            rows = await cursor.fetchall()
            return [dict(row) for row in rows]

    async def insert(self, table: str, data: dict) -> str:
        """Insert a row and return the ID."""
        columns = ', '.join(data.keys())
        placeholders = ', '.join(['?' for _ in data])
        query = f"INSERT INTO {table} ({columns}) VALUES ({placeholders})"

        conn = await self.connect()
        cursor = await conn.execute(query, tuple(data.values()))
        await conn.commit()

        # Return the ID (assumes TEXT PRIMARY KEY)
        return data.get('id') or str(cursor.lastrowid)

    async def update(self, table: str, id_value: str, data: dict):
        """Update a row by ID."""
        set_clause = ', '.join([f"{k} = ?" for k in data.keys()])
        query = f"UPDATE {table} SET {set_clause} WHERE id = ?"

        conn = await self.connect()
        await conn.execute(query, (*data.values(), id_value))
        await conn.commit()

    async def delete(self, table: str, id_value: str):
        """Delete a row by ID."""
        query = f"DELETE FROM {table} WHERE id = ?"
        conn = await self.connect()
        await conn.execute(query, (id_value,))
        await conn.commit()

    async def count(self, table: str, where: Optional[str] = None, params: tuple = ()) -> int:
        """Count rows in table."""
        query = f"SELECT COUNT(*) as count FROM {table}"
        if where:
            query += f" WHERE {where}"

        result = await self.fetch_one(query, params)
        return result['count'] if result else 0

    async def exists(self, table: str, where: str, params: tuple = ()) -> bool:
        """Check if a row exists."""
        count = await self.count(table, where, params)
        return count > 0


# Global database manager instance
_db_manager: Optional[DatabaseManager] = None


def init_database(db_path: str | Path) -> DatabaseManager:
    """Initialize global database manager."""
    global _db_manager
    _db_manager = DatabaseManager(db_path)
    return _db_manager


def get_db() -> DatabaseManager:
    """Get global database manager instance."""
    if _db_manager is None:
        raise RuntimeError(
            "Database not initialized. Call init_database() first."
        )
    return _db_manager


@asynccontextmanager
async def get_db_session() -> AsyncGenerator[aiosqlite.Connection, None]:
    """Get database session (connection) for use in tools."""
    db = get_db()
    conn = await db.connect()
    yield conn


@asynccontextmanager
async def get_db_transaction() -> AsyncGenerator[aiosqlite.Connection, None]:
    """Get database transaction context."""
    db = get_db()
    async with db.transaction() as conn:
        yield conn


async def get_db_connection() -> aiosqlite.Connection:
    """
    Simple helper to get database connection.
    Used by tool functions for backward compatibility.
    """
    db = get_db()
    conn = await db.connect()
    conn.row_factory = aiosqlite.Row  # Return rows as dicts
    return conn
