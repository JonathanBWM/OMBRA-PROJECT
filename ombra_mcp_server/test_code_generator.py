#!/usr/bin/env python3
"""
Test script for project-aware code generator
"""

import sys
from pathlib import Path

# Add src to path
sys.path.insert(0, str(Path(__file__).parent / "src"))

from ombra_mcp.tools.code_generator import (
    check_implementation_status,
    generate_exit_handler,
    generate_vmcs_setup,
    generate_ept_setup,
    generate_msr_bitmap_setup
)

def test_check_implementation():
    """Test the implementation checker."""
    print("=" * 80)
    print("Testing Implementation Checker")
    print("=" * 80)
    print()

    # Test existing handlers
    tests = [
        ("exit_handler", "CPUID"),
        ("exit_handler", "RDTSC"),
        ("exit_handler", "VMCALL"),
        ("exit_handler", "IO_INSTRUCTION"),  # Should exist
        ("exit_handler", "XSETBV"),  # Should NOT exist (not implemented)
        ("vmcs_setup", None),
        ("ept_setup", None),
        ("msr_bitmap", None),
    ]

    for component_type, component_name in tests:
        status = check_implementation_status(component_type, component_name)
        name_str = f" ({component_name})" if component_name else ""
        print(f"{component_type}{name_str}:")
        print(f"  Exists: {status['exists']}")
        if status['exists']:
            print(f"  File: {status['file_path']}")
            print(f"  Function: {status['function_name']}")
            print(f"  Line: {status['line_number']}")
            print(f"  Is Stub: {status['is_stub']}")
        print()


def test_generator_with_existing():
    """Test generators detecting existing implementations."""
    print("=" * 80)
    print("Testing Code Generators (Existing Implementation Detection)")
    print("=" * 80)
    print()

    # Test 1: Generate CPUID handler (should detect existing)
    print("Test 1: generate_exit_handler(10)  # CPUID - EXISTS")
    print("-" * 80)
    result = generate_exit_handler(10, stealth=True, force=False)
    print(result[:500])  # First 500 chars
    print()

    # Test 2: Generate RDTSC handler (should detect existing)
    print("Test 2: generate_exit_handler(16)  # RDTSC - EXISTS")
    print("-" * 80)
    result = generate_exit_handler(16, stealth=True, force=False)
    print(result[:500])
    print()

    # Test 3: Generate VMCS setup (should detect existing)
    print("Test 3: generate_vmcs_setup()  # Should detect existing")
    print("-" * 80)
    result = generate_vmcs_setup(force=False)
    print(result[:500])
    print()


def test_generator_with_force():
    """Test generators with force=True."""
    print("=" * 80)
    print("Testing Code Generators (Force Mode)")
    print("=" * 80)
    print()

    # Test: Force generate CPUID handler
    print("Test: generate_exit_handler(10, force=True)  # Force generate")
    print("-" * 80)
    result = generate_exit_handler(10, stealth=True, force=True)
    print(result[:800])  # Show generated code
    print()


if __name__ == "__main__":
    print("\n" + "=" * 80)
    print("PROJECT-AWARE CODE GENERATOR TEST SUITE")
    print("=" * 80)
    print()

    try:
        test_check_implementation()
        test_generator_with_existing()
        test_generator_with_force()

        print("=" * 80)
        print("ALL TESTS COMPLETED")
        print("=" * 80)

    except Exception as e:
        print(f"\nERROR: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
