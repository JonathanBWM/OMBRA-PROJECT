#!/usr/bin/env python3
"""
Initialize the Driver RE Database

Creates the driver_re.db SQLite database with full schema.
"""

import sys
from pathlib import Path

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent / "src"))

from ombra_mcp.tools.driver_re_db import init_driver_re_db, DB_PATH


def main():
    print(f"Initializing driver RE database at: {DB_PATH}")

    if DB_PATH.exists():
        print(f"WARNING: Database already exists at {DB_PATH}")
        response = input("Overwrite? (yes/no): ")
        if response.lower() != 'yes':
            print("Aborted.")
            return

        DB_PATH.unlink()
        print("Deleted existing database.")

    # Create database
    init_driver_re_db()

    print(f"\nDatabase created successfully!")
    print(f"Location: {DB_PATH}")
    print(f"Size: {DB_PATH.stat().st_size} bytes")

    print("\nTables created:")
    print("  - drivers")
    print("  - sections")
    print("  - imports")
    print("  - exports")
    print("  - ioctls")
    print("  - xrefs")

    print("\nDatabase is ready for driver analysis.")


if __name__ == "__main__":
    main()
