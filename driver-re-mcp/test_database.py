#!/usr/bin/env python3
"""Quick test script to verify database layer functionality."""

import asyncio
from pathlib import Path
import sys

# Add src to path
sys.path.insert(0, str(Path(__file__).parent / "src"))

from driver_re_mcp.database import (
    init_database,
    get_db,
    Driver,
    IOCTL,
    Import,
    Function,
)


async def test_database():
    """Test database initialization and basic operations."""

    # Initialize database
    db_path = Path(__file__).parent / "test_driver_re.db"
    if db_path.exists():
        db_path.unlink()

    print(f"Creating database at: {db_path}")
    db_manager = init_database(db_path)

    # Initialize schema
    print("Initializing schema...")
    await db_manager.init_db()

    # Test inserting a driver
    print("\nTesting driver insert...")
    driver = Driver(
        original_name="Ld9BoxSup.sys",
        analyzed_name="Ld9BoxSup",
        md5="abc123",
        sha1="def456",
        sha256="ghi789xyz",
        file_size=123456,
        image_base=0x140000000,
        entry_point_rva=0x1000,
        size_of_image=0x50000,
        machine=0x8664,
        subsystem=1,
        characteristics=0x2022,
        company_name="LDPlayer",
        product_name="Ld9BoxSup",
        tags='["vbox", "byovd"]'
    )

    driver_id = await db_manager.insert("drivers", driver.model_dump(exclude_none=True))
    print(f"Inserted driver with ID: {driver_id}")

    # Test fetching the driver
    print("\nTesting driver fetch...")
    fetched = await db_manager.fetch_one(
        "SELECT * FROM drivers WHERE id = ?",
        (driver_id,)
    )
    print(f"Fetched driver: {fetched['original_name']} (SHA256: {fetched['sha256']})")

    # Test inserting an IOCTL
    print("\nTesting IOCTL insert...")
    ioctl = IOCTL(
        driver_id=driver_id,
        name="SUP_IOCTL_COOKIE",
        code=0xc0106900,
        code_hex="0xC0106900",
        device_type=0xC010,
        function_code=0x900,
        method=0,
        access=0,
        description="Establish session with driver",
        requires_admin=False,
        is_vulnerable=False
    )

    ioctl_id = await db_manager.insert("ioctls", ioctl.model_dump(exclude_none=True))
    print(f"Inserted IOCTL with ID: {ioctl_id}")

    # Test inserting an import
    print("\nTesting import insert...")
    import_entry = Import(
        driver_id=driver_id,
        dll_name="ntoskrnl.exe",
        function_name="MmMapIoSpace",
        category="memory_management",
        subcategory="physical_memory",
        is_dangerous=True,
        danger_reason="Allows mapping arbitrary physical memory",
        description="Maps physical memory into kernel address space"
    )

    import_id = await db_manager.insert("imports", import_entry.model_dump(exclude_none=True))
    print(f"Inserted import with ID: {import_id}")

    # Test fetching dangerous imports
    print("\nTesting dangerous imports query...")
    dangerous = await db_manager.fetch_all(
        "SELECT * FROM imports WHERE driver_id = ? AND is_dangerous = 1",
        (driver_id,)
    )
    print(f"Found {len(dangerous)} dangerous imports:")
    for imp in dangerous:
        print(f"  - {imp['function_name']}: {imp['danger_reason']}")

    # Test counting
    print("\nTesting count operations...")
    driver_count = await db_manager.count("drivers")
    ioctl_count = await db_manager.count("ioctls", "driver_id = ?", (driver_id,))
    import_count = await db_manager.count("imports", "driver_id = ?", (driver_id,))

    print(f"Total drivers: {driver_count}")
    print(f"IOCTLs for this driver: {ioctl_count}")
    print(f"Imports for this driver: {import_count}")

    # Test transaction
    print("\nTesting transaction...")
    async with db_manager.transaction() as conn:
        await conn.execute(
            "INSERT INTO strings (id, driver_id, value, rva, length, encoding) VALUES (?, ?, ?, ?, ?, ?)",
            (Function().id, driver_id, "Test String", 0x1234, 11, "ascii")
        )
        print("Transaction committed successfully")

    # Verify string was inserted
    string_count = await db_manager.count("strings", "driver_id = ?", (driver_id,))
    print(f"Strings for this driver: {string_count}")

    # Close connection
    await db_manager.close()

    print("\n✓ All tests passed!")
    print(f"\nTest database created at: {db_path}")
    print("You can inspect it with: sqlite3 test_driver_re.db")


if __name__ == "__main__":
    asyncio.run(test_database())
