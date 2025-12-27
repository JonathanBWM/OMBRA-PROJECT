"""
OmbraMCP Server Audit - End-to-End Test
"""

import asyncio
import sys
from pathlib import Path

# Add src to path
sys.path.insert(0, str(Path(__file__).parent / "src"))

from ombra_mcp.tools import sdm_query, stealth


async def test_vmx_controls():
    """Test VMX controls database population"""
    print("=" * 60)
    print("TEST 1: VMX Controls Database")
    print("=" * 60)

    pin_controls = await sdm_query.get_vmx_control_bits("pin_based")
    proc_controls = await sdm_query.get_vmx_control_bits("proc_based")
    proc2_controls = await sdm_query.get_vmx_control_bits("proc_based2")

    pin_count = len(pin_controls.get('bits', []))
    proc_count = len(proc_controls.get('bits', []))
    proc2_count = len(proc2_controls.get('bits', []))

    print(f"Pin-based controls: {pin_count} bits")
    print(f"Proc-based controls: {proc_count} bits")
    print(f"Proc-based2 controls: {proc2_count} bits")

    total = pin_count + proc_count + proc2_count
    print(f"Total control bits: {total}")

    assert pin_count > 0, "Pin-based controls empty!"
    assert proc_count > 0, "Proc-based controls empty!"
    assert proc2_count > 0, "Proc-based2 controls empty!"

    print("✓ PASS: VMX controls database populated\n")
    return True


async def test_timing_msrs():
    """Test timing MSRs (APERF/MPERF)"""
    print("=" * 60)
    print("TEST 2: Timing MSRs")
    print("=" * 60)

    aperf = await sdm_query.get_msr_info("APERF")
    mperf = await sdm_query.get_msr_info("MPERF")

    aperf_count = aperf.get('count', 0)
    mperf_count = mperf.get('count', 0)

    print(f"APERF found: {aperf_count > 0}")
    print(f"MPERF found: {mperf_count > 0}")

    if aperf_count > 0:
        aperf_data = aperf.get('msrs', [])[0]
        print(f"  APERF address: {aperf_data.get('address', 'unknown')}")
        print(f"  APERF description: {aperf_data.get('description', 'unknown')[:60]}...")

    if mperf_count > 0:
        mperf_data = mperf.get('msrs', [])[0]
        print(f"  MPERF address: {mperf_data.get('address', 'unknown')}")
        print(f"  MPERF description: {mperf_data.get('description', 'unknown')[:60]}...")

    assert aperf_count > 0, "APERF MSR not found!"
    assert mperf_count > 0, "MPERF MSR not found!"

    print("✓ PASS: Timing MSRs present\n")
    return True


def test_amd_evasion():
    """Test AMD evasion data"""
    print("=" * 60)
    print("TEST 3: AMD Evasion Data")
    print("=" * 60)

    amd = stealth.get_amd_evasion_info()

    has_cpuid = 'cpuid_leaves' in amd
    has_msrs = 'svm_msrs' in amd
    has_checklist = 'evasion_checklist' in amd

    print(f"AMD evasion data loaded: {not 'error' in amd}")
    print(f"  Has CPUID leaves: {has_cpuid}")
    print(f"  Has SVM MSRs: {has_msrs}")
    print(f"  Has evasion checklist: {has_checklist}")

    if has_cpuid:
        cpuid_count = len(amd.get('cpuid_leaves', {}))
        print(f"  CPUID leaves: {cpuid_count}")

    if has_msrs:
        msr_count = len(amd.get('svm_msrs', {}))
        print(f"  SVM MSRs: {msr_count}")

    assert not 'error' in amd, f"AMD data load error: {amd.get('error')}"
    assert has_cpuid, "AMD CPUID leaves missing!"

    print("✓ PASS: AMD evasion data valid\n")
    return True


def test_hyperv_enlightenments():
    """Test Hyper-V enlightenment data"""
    print("=" * 60)
    print("TEST 4: Hyper-V Enlightenments")
    print("=" * 60)

    hv = stealth.get_hyperv_enlightenment_info()

    has_cpuid = 'cpuid_leaves' in hv
    has_msrs = 'msrs' in hv
    has_features = 'features' in hv

    print(f"Hyper-V data loaded: {not 'error' in hv}")
    print(f"  Has CPUID leaves: {has_cpuid}")
    print(f"  Has MSRs: {has_msrs}")
    print(f"  Has features: {has_features}")

    if has_cpuid:
        cpuid_count = len(hv.get('cpuid_leaves', {}))
        print(f"  CPUID leaves: {cpuid_count}")

    if has_msrs:
        msr_count = len(hv.get('msrs', {}))
        print(f"  MSRs: {msr_count}")

    assert not 'error' in hv, f"Hyper-V data load error: {hv.get('error')}"
    assert has_cpuid, "Hyper-V CPUID leaves missing!"

    print("✓ PASS: Hyper-V enlightenment data valid\n")
    return True


def test_cpuid_safety():
    """Test CPUID safety checker"""
    print("=" * 60)
    print("TEST 5: CPUID Safety Checker")
    print("=" * 60)

    # Test Hyper-V leaf (should be risky)
    safety_hv = stealth.check_cpuid_safety(0x40000001)

    # Test standard leaf 1 (should be partial)
    safety_std = stealth.check_cpuid_safety(0x1)

    # Test standard leaf 0 (should be safe)
    safety_max = stealth.check_cpuid_safety(0x0)

    print("CPUID 0x40000001 (Hyper-V):")
    print(f"  Risk level: {safety_hv.get('risk_level')}")
    print(f"  Safe to modify: {safety_hv.get('safe_to_modify_code')}")
    print(f"  Recommended: {safety_hv.get('recommended_action', '')[:60]}...")

    print("\nCPUID 0x1 (Processor Info):")
    print(f"  Risk level: {safety_std.get('risk_level')}")
    print(f"  Safe to modify: {safety_std.get('safe_to_modify')}")
    print(f"  Recommended: {safety_std.get('recommended_action', '')[:60]}...")

    print("\nCPUID 0x0 (Max Leaf):")
    print(f"  Risk level: {safety_max.get('risk_level')}")
    print(f"  Safe to modify: {safety_max.get('safe_to_modify')}")

    assert 'risk_level' in safety_hv, "CPUID safety check failed!"
    assert 'recommended_action' in safety_hv, "Missing recommended action!"

    print("\n✓ PASS: CPUID safety checker working\n")
    return True


async def run_all_tests():
    """Run all audit tests"""
    print("\n" + "=" * 60)
    print("OMBRA MCP SERVER AUDIT")
    print("=" * 60 + "\n")

    try:
        # Async tests
        await test_vmx_controls()
        await test_timing_msrs()

        # Sync tests
        test_amd_evasion()
        test_hyperv_enlightenments()
        test_cpuid_safety()

        print("=" * 60)
        print("✓ ALL AUDITS PASSED")
        print("=" * 60)
        return 0

    except AssertionError as e:
        print(f"\n✗ AUDIT FAILED: {e}")
        return 1
    except Exception as e:
        print(f"\n✗ UNEXPECTED ERROR: {e}")
        import traceback
        traceback.print_exc()
        return 2


if __name__ == "__main__":
    exit_code = asyncio.run(run_all_tests())
    sys.exit(exit_code)
