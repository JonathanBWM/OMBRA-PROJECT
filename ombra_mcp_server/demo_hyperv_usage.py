#!/usr/bin/env python3
"""
Demo: Real-world usage of Hyper-V enlightenment stealth tools

This demonstrates how to use the new functions when implementing
a CPUID exit handler in a hypervisor.
"""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent / "src"))

from ombra_mcp.tools.stealth import get_hyperv_enlightenment_info, check_cpuid_safety
import json


def demo_cpuid_handler_design():
    """
    Demonstrate designing a CPUID exit handler with proper Hyper-V support.
    """
    print("=" * 80)
    print("DEMO: Designing a Stealth CPUID Exit Handler")
    print("=" * 80)

    # Scenario: We're implementing a CPUID exit handler and need to decide
    # what to do with various CPUID leaves

    test_scenarios = [
        (0x00000001, "Standard features - has hypervisor bit"),
        (0x40000000, "Hypervisor vendor ID - stealth target"),
        (0x40000001, "Hyper-V interface - Windows needs this"),
        (0x40000003, "Hyper-V features - critical for Windows"),
        (0x40000007, "Reserved Hyper-V leaf"),
    ]

    print("\nCPUID Exit Handler Decision Matrix:")
    print("-" * 80)

    for leaf, description in test_scenarios:
        safety = check_cpuid_safety(leaf)

        print(f"\nLeaf {hex(leaf)}: {description}")
        print(f"  Name: {safety['name']}")
        print(f"  Risk Level: {safety['risk_level']}")
        print(f"  Safe to Modify: {safety.get('safe_to_modify_code', 'N/A')}")
        print(f"  Action: {safety['recommended_action']}")

        if safety['warnings']:
            print(f"  Warnings:")
            for warning in safety['warnings']:
                print(f"    - {warning}")


def demo_generate_cpuid_handler():
    """
    Generate skeleton CPUID handler code based on safety analysis.
    """
    print("\n" + "=" * 80)
    print("DEMO: Generated CPUID Handler Code")
    print("=" * 80)

    enlightenments = get_hyperv_enlightenment_info()

    print("""
// Auto-generated CPUID Exit Handler with Hyper-V Enlightenment Support
// Generated based on Hyper-V TLFS and stealth analysis

BOOLEAN HandleCpuidExit(PVCPU vcpu) {
    UINT32 leaf = (UINT32)vcpu->GuestRegisters.Rax;
    UINT32 subleaf = (UINT32)vcpu->GuestRegisters.Rcx;
    UINT32 regs[4] = {0};

    // Execute real CPUID to get baseline values
    __cpuidex((int*)regs, leaf, subleaf);

    switch (leaf) {
    case 0x00000001:
        // Processor Info and Features
        // CRITICAL: Clear hypervisor present bit for stealth
        regs[2] &= ~(1u << 31);  // Clear ECX[31]
        break;

    case 0x40000000:
        // Hypervisor Vendor ID
        // STRATEGY: Hide vendor but preserve max leaf for Windows
""")

    # Get the actual strategy from our data
    strategy = enlightenments.get('safe_evasion_strategy', {})
    print(f"        // {strategy.get('approach', 'Pass through unchanged')}")
    print("""        if (IsRunningNestedUnderHyperV()) {
            // Nested - pass through everything
            // (already in regs from __cpuidex)
        } else {
            // Not nested - hide vendor signature
            regs[1] = 0;  // EBX - zero vendor string
            regs[2] = 0;  // ECX - zero vendor string
            regs[3] = 0;  // EDX - zero vendor string
            // Keep EAX (max leaf) from real CPUID
        }
        break;
""")

    # Generate cases for critical Hyper-V leaves
    critical_leaves = {
        '0x40000001': 'Hypervisor Interface',
        '0x40000002': 'System Identity',
        '0x40000003': 'Feature Identification',
        '0x40000004': 'Implementation Recommendations',
        '0x40000005': 'Hypervisor Limits',
    }

    for leaf_hex, name in critical_leaves.items():
        leaf_int = int(leaf_hex, 16)
        safety = check_cpuid_safety(leaf_int)

        print(f"    case {leaf_hex}:")
        print(f"        // {name}")
        print(f"        // Risk Level: {safety['risk_level']}")
        print(f"        // {safety['recommended_action']}")

        if safety['risk_level'] in ['CRITICAL', 'HIGH']:
            print("        // PASS THROUGH - DO NOT MODIFY")
            print("        // (already in regs from __cpuidex)")
        else:
            print("        // Can be zeroed if not running nested")
            print("        if (!IsRunningNestedUnderHyperV()) {")
            print("            regs[0] = regs[1] = regs[2] = regs[3] = 0;")
            print("        }")
        print("        break;")
        print("")

    print("""    default:
        // All other leaves - pass through unchanged
        break;
    }

    // Write results back to guest registers
    vcpu->GuestRegisters.Rax = regs[0];
    vcpu->GuestRegisters.Rbx = regs[1];
    vcpu->GuestRegisters.Rcx = regs[2];
    vcpu->GuestRegisters.Rdx = regs[3];

    // Advance guest RIP past CPUID instruction
    AdvanceGuestRip(vcpu);

    return TRUE;
}

// Helper: Check if we're running nested under Hyper-V
BOOLEAN IsRunningNestedUnderHyperV(VOID) {
    int regs[4];
    __cpuid(regs, 0x40000000);

    // Check for "Microsoft Hv" signature
    return (regs[1] == 0x7263694D &&  // "Micr"
            regs[2] == 0x666F736F &&  // "osof"
            regs[3] == 0x76482074);   // "t Hv"
}
""")


def demo_msr_documentation():
    """
    Display Hyper-V MSR information.
    """
    print("\n" + "=" * 80)
    print("DEMO: Hyper-V MSR Reference")
    print("=" * 80)

    enlightenments = get_hyperv_enlightenment_info()
    msrs = enlightenments.get('hyper_v_msrs', {})

    print(f"\nTotal Hyper-V MSRs documented: {len(msrs)}\n")

    # Show critical MSRs
    print("Critical MSRs (must be virtualized for Windows):")
    print("-" * 80)

    for msr_name, msr_data in msrs.items():
        if msr_data.get('critical', False):
            print(f"\n{msr_name}")
            print(f"  Address: {msr_data['address']}")
            print(f"  Access: {msr_data['access']}")
            print(f"  Description: {msr_data['description']}")
            if 'note' in msr_data:
                print(f"  Note: {msr_data['note']}")


def demo_risk_summary():
    """
    Generate a risk summary for common CPUID operations.
    """
    print("\n" + "=" * 80)
    print("DEMO: CPUID Modification Risk Summary")
    print("=" * 80)

    leaves_to_check = list(range(0x40000000, 0x4000000B))

    risk_levels = {"CRITICAL": [], "HIGH": [], "MEDIUM": [], "LOW": []}

    for leaf in leaves_to_check:
        safety = check_cpuid_safety(leaf)
        risk = safety['risk_level']
        if risk in risk_levels:
            risk_levels[risk].append((hex(leaf), safety['name']))

    for risk, leaves in risk_levels.items():
        if leaves:
            print(f"\n{risk} Risk Leaves:")
            for leaf_hex, name in leaves:
                print(f"  {leaf_hex}: {name}")


def main():
    """Run all demos."""
    print("Hyper-V Enlightenment Stealth Tools - Usage Demonstration")
    print("=" * 80)
    print("\nThis demo shows how to use the new Hyper-V enlightenment tools")
    print("when implementing a stealth hypervisor that needs to support Windows.\n")

    demo_cpuid_handler_design()
    demo_generate_cpuid_handler()
    demo_msr_documentation()
    demo_risk_summary()

    print("\n" + "=" * 80)
    print("Demo complete!")
    print("=" * 80)
    print("\nKey Takeaways:")
    print("1. NEVER zero CPUID leaves 0x40000001-0x40000005 on Windows guests")
    print("2. Only modify vendor string in 0x40000000 (EBX/ECX/EDX)")
    print("3. Preserve EAX (max leaf) in 0x40000000")
    print("4. Virtualize critical Hyper-V MSRs even if returning dummy values")
    print("5. Detect nested execution and pass through enlightenments if nested")
    print("\nFor full documentation, see:")
    print("  - src/ombra_mcp/data/hyperv_enlightenments.json")
    print("  - Microsoft Hyper-V TLFS documentation")


if __name__ == "__main__":
    main()
