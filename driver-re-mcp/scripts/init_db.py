#!/usr/bin/env python3
"""Initialize the Driver RE MCP database"""

import asyncio
import sys
from pathlib import Path

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent / "src"))

from driver_re_mcp.config import settings
from driver_re_mcp.database import init_database, get_db

async def main():
    """Initialize database schema"""
    print(f"Initializing database at: {settings.DATABASE_PATH}")

    # Ensure data directory exists
    db_path = settings.database_path_obj
    db_path.parent.mkdir(parents=True, exist_ok=True)

    # Initialize database manager
    db_manager = init_database(db_path)

    # Initialize schema
    schema_path = Path(__file__).parent.parent / "src" / "driver_re_mcp" / "database" / "schema.sql"

    if not schema_path.exists():
        print(f"ERROR: Schema file not found: {schema_path}")
        sys.exit(1)

    print(f"Loading schema from: {schema_path}")

    try:
        await db_manager.init_db(schema_path)
        print("Database initialized successfully!")
    except Exception as e:
        print(f"Error initializing database: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
    finally:
        await db_manager.close()

if __name__ == "__main__":
    asyncio.run(main())
