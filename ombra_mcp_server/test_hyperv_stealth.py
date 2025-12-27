#!/usr/bin/env python3
"""
Test script for Hyper-V enlightenment stealth functions
"""

import sys
from pathlib import Path

# Add src to path for imports
sys.path.insert(0, str(Path(__file__).parent / "src"))

from ombra_mcp.tools.stealth import get_hyperv_enlightenment_info, check_cpuid_safety
import json


def test_get_hyperv_enlightenment_info():
    """Test loading Hyper-V enlightenment documentation."""
    print("=" * 80)
    print("TEST: get_hyperv_enlightenment_info()")
    print("=" * 80)

    data = get_hyperv_enlightenment_info()

    if "error" in data:
        print(f"ERROR: {data['error']}")
        return False

    print(f"\nLoaded enlightenment data successfully")
    print(f"CPUID leaves documented: {len(data.get('cpuid_leaves', {}))}")
    print(f"MSRs documented: {len(data.get('hyper_v_msrs', {}))}")

    # Print overview
    print(f"\nOverview:\n{data.get('overview', 'N/A')}")

    # Print a sample leaf
    print(f"\n--- Sample CPUID Leaf (0x40000000) ---")
    leaf_0x40000000 = data['cpuid_leaves'].get('0x40000000', {})
    print(json.dumps(leaf_0x40000000, indent=2))

    # Print safe evasion strategy
    print(f"\n--- Safe Evasion Strategy ---")
    print(json.dumps(data.get('safe_evasion_strategy', {}), indent=2))

    return True


def test_check_cpuid_safety():
    """Test CPUID safety checker with various leaves."""
    print("\n" + "=" * 80)
    print("TEST: check_cpuid_safety()")
    print("=" * 80)

    test_leaves = [
        0x00000000,  # Max standard function
        0x00000001,  # Processor info (has hypervisor bit)
        0x40000000,  # Hypervisor vendor (SAFE TO MODIFY PARTIALLY)
        0x40000001,  # Hypervisor interface (CRITICAL - DO NOT MODIFY)
        0x40000003,  # Hypervisor features (CRITICAL)
        0x40000004,  # Implementation recommendations (RISKY)
        0x40000005,  # Hypervisor limits (CRITICAL)
        0x40000007,  # Reserved/unused
        0x80000000,  # Extended max function
        0x12345678,  # Unknown leaf
    ]

    for leaf in test_leaves:
        print(f"\n--- CPUID Leaf: {hex(leaf)} ---")
        result = check_cpuid_safety(leaf)

        if "error" in result:
            print(f"ERROR: {result['error']}")
            continue

        print(f"Name: {result['name']}")
        print(f"Safe to modify: {result['safe_to_modify']}")
        print(f"Risk level: {result['risk_level']}")
        print(f"Description: {result['description']}")
        if result['warnings']:
            print(f"Warnings:")
            for warning in result['warnings']:
                print(f"  - {warning}")
        print(f"Recommended action: {result['recommended_action']}")
        print(f"Is Hyper-V leaf: {result['is_hyperv_leaf']}")

    return True


def test_critical_leaf_detection():
    """Test that critical leaves are properly flagged."""
    print("\n" + "=" * 80)
    print("TEST: Critical Leaf Detection")
    print("=" * 80)

    critical_leaves = [0x40000001, 0x40000003, 0x40000005]

    all_critical = True
    for leaf in critical_leaves:
        result = check_cpuid_safety(leaf)
        is_critical = result['risk_level'] == 'CRITICAL'
        status = "PASS" if is_critical else "FAIL"
        safe_code = result.get('safe_to_modify_code', result.get('safe_to_modify', 'UNKNOWN'))
        print(f"{status}: {hex(leaf)} - Risk: {result['risk_level']}, Safe code: {safe_code}")
        if not is_critical:
            all_critical = False

    return all_critical


def test_partial_safe_leaf():
    """Test that 0x40000000 is correctly marked as PARTIAL."""
    print("\n" + "=" * 80)
    print("TEST: Partial Safe Leaf (0x40000000)")
    print("=" * 80)

    result = check_cpuid_safety(0x40000000)

    safe_code = result.get('safe_to_modify_code', 'UNKNOWN')
    is_partial = safe_code == 'PARTIAL'
    is_medium_risk = result['risk_level'] == 'MEDIUM'
    correct_action = 'EBX/ECX/EDX' in result['recommended_action'] and 'EAX' in result['recommended_action']

    print(f"Safe to modify: {result.get('safe_to_modify', 'N/A')}")
    print(f"Safe to modify code: {safe_code} (expected: PARTIAL)")
    print(f"Risk level: {result['risk_level']} (expected: MEDIUM)")
    print(f"Recommended action: {result['recommended_action']}")

    passed = is_partial and is_medium_risk and correct_action
    print(f"\nResult: {'PASS' if passed else 'FAIL'}")

    return passed


def main():
    """Run all tests."""
    print("Testing Hyper-V Enlightenment Stealth Functions")
    print("=" * 80)

    tests = [
        ("Load Enlightenment Data", test_get_hyperv_enlightenment_info),
        ("Check CPUID Safety", test_check_cpuid_safety),
        ("Critical Leaf Detection", test_critical_leaf_detection),
        ("Partial Safe Leaf", test_partial_safe_leaf),
    ]

    results = []
    for name, test_func in tests:
        try:
            passed = test_func()
            results.append((name, passed))
        except Exception as e:
            print(f"\nEXCEPTION in {name}: {e}")
            import traceback
            traceback.print_exc()
            results.append((name, False))

    # Summary
    print("\n" + "=" * 80)
    print("TEST SUMMARY")
    print("=" * 80)

    for name, passed in results:
        status = "PASS" if passed else "FAIL"
        print(f"{status}: {name}")

    total = len(results)
    passed = sum(1 for _, p in results if p)
    print(f"\nTotal: {passed}/{total} tests passed")

    return passed == total


if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
