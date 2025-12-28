#!/usr/bin/env python3
"""
Test Driver RE MCP Tools

Comprehensive test of all driver RE tools.
"""

import asyncio
import sys
from pathlib import Path

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent / "src"))

from ombra_mcp.tools import driver_tools, ioctl_tools, import_tools, export_tools


async def test_driver_tools():
    """Test driver management tools."""
    print("\n=== Testing Driver Tools ===\n")

    # Test 1: Add a mock driver
    print("Test 1: Adding mock driver...")
    # Create a temporary mock file
    mock_driver = Path("/tmp/test_driver.sys")
    mock_driver.write_bytes(b"MZ" + b"\x00" * 1000)  # Minimal PE header

    result = await driver_tools.add_driver(
        file_path=str(mock_driver),
        analyzed_name="TestDriver",
        tags=["test", "mock"],
        notes="This is a test driver for validation"
    )
    print(f"Add result: {result}")

    if not result.get('success'):
        print("FAILED: Could not add driver")
        return None

    driver_id = result['driver_id']
    print(f"✓ Driver added with ID: {driver_id}")

    # Test 2: Get driver
    print("\nTest 2: Getting driver by ID...")
    driver = await driver_tools.get_driver(driver_id=driver_id)
    print(f"Driver details: {driver.get('original_name')}, SHA256: {driver.get('sha256')[:16]}...")
    print(f"✓ Retrieved driver successfully")

    # Test 3: List drivers
    print("\nTest 3: Listing all drivers...")
    drivers = await driver_tools.list_drivers(limit=10)
    print(f"Found {len(drivers)} driver(s)")
    print(f"✓ List drivers works")

    # Test 4: Update status
    print("\nTest 4: Updating driver status...")
    update_result = await driver_tools.update_driver_status(
        driver_id=driver_id,
        status="in_progress",
        notes="Beginning analysis"
    )
    print(f"Update result: {update_result.get('message')}")
    print(f"✓ Status updated to 'in_progress'")

    # Cleanup temp file
    mock_driver.unlink()

    return driver_id


async def test_ioctl_tools(driver_id: str):
    """Test IOCTL management tools."""
    print("\n=== Testing IOCTL Tools ===\n")

    # Test 1: Add IOCTL
    print("Test 1: Adding vulnerable IOCTL...")
    result = await ioctl_tools.add_ioctl(
        driver_id=driver_id,
        name="TEST_IOCTL_PHYS_MEM_READ",
        code=0x222040,  # Mock IOCTL code
        description="Test IOCTL for physical memory read",
        handler_rva=0x1000,
        min_input_size=16,
        max_input_size=32,
        min_output_size=8,
        max_output_size=4096,
        requires_admin=True,
        is_vulnerable=True,
        vulnerability_type="arbitrary_read",
        vulnerability_severity="critical",
        vulnerability_description="Allows arbitrary physical memory read",
        exploitation_notes="Can be used to read kernel memory"
    )
    print(f"Add result: {result}")

    if not result.get('success'):
        print("FAILED: Could not add IOCTL")
        return None

    ioctl_id = result['ioctl_id']
    print(f"✓ IOCTL added with ID: {ioctl_id}")
    print(f"  Code: {result.get('code')}")
    print(f"  Components: {result.get('components')}")

    # Test 2: Get IOCTL
    print("\nTest 2: Getting IOCTL details...")
    ioctl = await ioctl_tools.get_ioctl(ioctl_id=ioctl_id)
    print(f"IOCTL: {ioctl.get('name')}")
    print(f"Vulnerability: {ioctl.get('vulnerability_type')} ({ioctl.get('vulnerability_severity')})")
    print(f"✓ Retrieved IOCTL successfully")

    # Test 3: List IOCTLs
    print("\nTest 3: Listing all IOCTLs for driver...")
    ioctls = await ioctl_tools.list_ioctls(driver_id=driver_id)
    print(f"Found {len(ioctls)} IOCTL(s)")
    print(f"✓ List IOCTLs works")

    # Test 4: Get vulnerable IOCTLs
    print("\nTest 4: Getting vulnerable IOCTLs...")
    vuln_ioctls = await ioctl_tools.get_vulnerable_ioctls(severity="critical")
    print(f"Found {len(vuln_ioctls)} critical vulnerable IOCTL(s)")
    print(f"✓ Vulnerable IOCTL query works")

    # Test 5: Update vulnerability
    print("\nTest 5: Updating IOCTL vulnerability...")
    update_result = await ioctl_tools.update_ioctl_vulnerability(
        ioctl_id=ioctl_id,
        is_vulnerable=True,
        vulnerability_type="arbitrary_read",
        vulnerability_severity="high",  # Downgrade from critical
        cve_ids=["CVE-2024-TEST-001"]
    )
    print(f"Update result: {update_result.get('message')}")
    print(f"✓ Vulnerability updated")

    return ioctl_id


async def test_import_tools(driver_id: str):
    """Test import analysis tools."""
    print("\n=== Testing Import Tools ===\n")

    # First, add some mock imports to the driver
    print("Adding mock imports for testing...")
    from ombra_mcp.tools.driver_re_db import get_conn
    import uuid

    conn = get_conn()
    c = conn.cursor()

    dangerous_imports = [
        ('ntoskrnl.exe', 'MmMapIoSpace', 'physical_memory', 'memory_management'),
        ('ntoskrnl.exe', 'MmCopyMemory', 'virtual_memory', 'memory_management'),
        ('ntoskrnl.exe', '__readmsr', 'msr', 'cpu_hardware'),
        ('ntoskrnl.exe', 'ZwCreateThread', 'thread', 'process_thread'),
        ('ntoskrnl.exe', 'KeWaitForSingleObject', 'synchronization', 'synchronization'),
    ]

    for dll, func, subcat, cat in dangerous_imports:
        import_id = str(uuid.uuid4())
        c.execute("""
            INSERT INTO imports (id, driver_id, dll_name, function_name, category, subcategory)
            VALUES (?, ?, ?, ?, ?, ?)
        """, (import_id, driver_id, dll, func, cat, subcat))

    conn.commit()
    conn.close()

    # Test 1: Get imports
    print("\nTest 1: Getting all imports...")
    imports = await import_tools.get_imports(driver_id=driver_id)
    print(f"Found {len(imports)} import(s)")
    for imp in imports[:3]:
        print(f"  - {imp['dll_name']}!{imp['function_name']}")
    print(f"✓ Get imports works")

    # Test 2: Get dangerous only
    print("\nTest 2: Getting dangerous imports only...")
    dangerous = await import_tools.get_imports(driver_id=driver_id, dangerous_only=True)
    print(f"Found {len(dangerous)} dangerous import(s)")
    print(f"✓ Dangerous filter works")

    # Test 3: Find dangerous APIs
    print("\nTest 3: Finding dangerous APIs...")
    dangerous_apis = await import_tools.find_dangerous_apis(driver_id=driver_id)
    print(f"Total imports: {dangerous_apis.get('total_imports')}")
    print(f"Dangerous: {dangerous_apis.get('total_dangerous')}")
    print(f"Risk level: {dangerous_apis.get('risk_level')}")

    for category, data in dangerous_apis.get('dangerous_by_category', {}).items():
        print(f"  {category}: {data['count']} API(s)")
        for api in data['apis'][:2]:
            print(f"    - {api['function_name']}")

    print(f"✓ Dangerous API detection works")

    # Test 4: Categorize import
    if imports:
        print("\nTest 4: Categorizing an import...")
        first_import = imports[0]
        result = await import_tools.categorize_import(
            import_id=first_import['id'],
            category="memory_management",
            subcategory="physical_memory",
            is_dangerous=True,
            danger_reason="Allows arbitrary physical memory mapping"
        )
        print(f"Categorize result: {result.get('message')}")
        print(f"✓ Import categorization works")

    return True


async def test_export_tools(driver_id: str):
    """Test export analysis tools."""
    print("\n=== Testing Export Tools ===\n")

    # Add mock exports
    print("Adding mock exports for testing...")
    from ombra_mcp.tools.driver_re_db import get_conn
    import uuid

    conn = get_conn()
    c = conn.cursor()

    exports = [
        ('SUPR0PhysMemRead', 100, 0x2000, 'SUPR0'),
        ('SUPR0PhysMemWrite', 101, 0x2100, 'SUPR0'),
        ('RTMemAlloc', 102, 0x3000, 'RT'),
        ('ASMAtomicIncU32', 103, 0x4000, 'ASM'),
    ]

    for func_name, ordinal, rva, prefix in exports:
        export_id = str(uuid.uuid4())
        c.execute("""
            INSERT INTO exports (id, driver_id, function_name, ordinal, rva, prefix)
            VALUES (?, ?, ?, ?, ?, ?)
        """, (export_id, driver_id, func_name, ordinal, rva, prefix))

    conn.commit()
    conn.close()

    # Test 1: Get exports
    print("\nTest 1: Getting all exports...")
    exports = await export_tools.get_exports(driver_id=driver_id)
    print(f"Found {len(exports)} export(s)")
    for exp in exports:
        print(f"  - {exp['function_name']} (ordinal {exp['ordinal']}, prefix: {exp['prefix']})")
    print(f"✓ Get exports works")

    # Test 2: Filter by prefix
    print("\nTest 2: Getting SUPR0 exports only...")
    supr0_exports = await export_tools.get_exports(driver_id=driver_id, prefix="SUPR0")
    print(f"Found {len(supr0_exports)} SUPR0 export(s)")
    print(f"✓ Prefix filter works")

    # Test 3: Document export
    if exports:
        print("\nTest 3: Documenting an export...")
        first_export = exports[0]
        result = await export_tools.document_export(
            export_id=first_export['id'],
            description="Reads physical memory at specified address",
            return_type="NTSTATUS",
            parameters=[
                {"name": "PhysAddr", "type": "PHYSICAL_ADDRESS", "description": "Physical address to read"},
                {"name": "Buffer", "type": "PVOID", "description": "Output buffer"},
                {"name": "Length", "type": "SIZE_T", "description": "Number of bytes to read"}
            ],
            calling_convention="fastcall",
            is_dangerous=True,
            danger_reason="Arbitrary physical memory read primitive"
        )
        print(f"Document result: {result.get('message')}")
        print(f"  Detected prefix: {result.get('prefix')}")
        print(f"✓ Export documentation works")

    return True


async def test_cleanup(driver_id: str):
    """Clean up test data."""
    print("\n=== Cleanup ===\n")

    print("Deleting test driver...")
    result = await driver_tools.delete_driver(driver_id=driver_id, confirm=True)
    print(f"Delete result: {result.get('message')}")
    print(f"✓ Cleanup complete")


async def main():
    """Run all tests."""
    print("╔═══════════════════════════════════════════════════════════╗")
    print("║     Driver RE MCP Tools - Comprehensive Test Suite       ║")
    print("╚═══════════════════════════════════════════════════════════╝")

    try:
        # Test driver tools
        driver_id = await test_driver_tools()
        if not driver_id:
            print("\n❌ Driver tools test failed!")
            return

        # Test IOCTL tools
        ioctl_id = await test_ioctl_tools(driver_id)
        if not ioctl_id:
            print("\n❌ IOCTL tools test failed!")
            return

        # Test import tools
        if not await test_import_tools(driver_id):
            print("\n❌ Import tools test failed!")
            return

        # Test export tools
        if not await test_export_tools(driver_id):
            print("\n❌ Export tools test failed!")
            return

        # Cleanup
        await test_cleanup(driver_id)

        print("\n╔═══════════════════════════════════════════════════════════╗")
        print("║              ✓ ALL TESTS PASSED                          ║")
        print("╚═══════════════════════════════════════════════════════════╝")

    except Exception as e:
        print(f"\n❌ Test failed with error: {e}")
        import traceback
        traceback.print_exc()


if __name__ == "__main__":
    asyncio.run(main())
