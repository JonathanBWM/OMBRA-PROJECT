#!/usr/bin/env python3
"""Quick test to verify server functionality"""

import asyncio
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent / "src"))

from driver_re_mcp.config import settings
from driver_re_mcp.database import init_database, get_db
from driver_re_mcp.tools import driver_tools

async def main():
    """Test basic operations"""
    print("Testing Driver RE MCP Server...")

    # Initialize database
    db_manager = init_database(settings.database_path_obj)
    await db_manager.connect()

    try:
        # Test 1: Add a driver
        print("\nTest 1: Adding a test driver...")
        result = await driver_tools.add_driver(
            original_name="test_driver.sys",
            md5="d41d8cd98f00b204e9800998ecf8427e",
            sha1="da39a3ee5e6b4b0d3255bfef95601890afd80709",
            sha256="e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
            file_size=1024,
            image_base=0x140000000,
            entry_point_rva=0x1000,
            size_of_image=0x10000
        )
        print(f"Result: {result}")
        assert result['success'], "Failed to add driver"
        driver_id = result['driver_id']

        # Test 2: Get the driver
        print("\nTest 2: Retrieving driver...")
        result = await driver_tools.get_driver(driver_id=driver_id)
        print(f"Result: {result}")
        assert result['success'], "Failed to get driver"
        assert result['driver']['original_name'] == "test_driver.sys"

        # Test 3: List drivers
        print("\nTest 3: Listing drivers...")
        result = await driver_tools.list_drivers()
        print(f"Result: Found {result['count']} driver(s)")
        assert result['success'], "Failed to list drivers"
        assert result['count'] >= 1

        # Test 4: Update driver status
        print("\nTest 4: Updating driver status...")
        result = await driver_tools.update_driver_status(
            driver_id=driver_id,
            status="in_progress"
        )
        print(f"Result: {result}")
        assert result['success'], "Failed to update status"

        # Test 5: Verify status update
        print("\nTest 5: Verifying status update...")
        result = await driver_tools.get_driver(driver_id=driver_id)
        assert result['driver']['analysis_status'] == "in_progress"

        print("\nAll tests passed!")

    except Exception as e:
        print(f"\nTest failed: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
    finally:
        await db_manager.close()

if __name__ == "__main__":
    asyncio.run(main())
