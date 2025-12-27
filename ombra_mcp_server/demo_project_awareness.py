#!/usr/bin/env python3
"""
Comprehensive demonstration of project-aware code generators
"""

import sys
from pathlib import Path

# Add src to path
sys.path.insert(0, str(Path(__file__).parent / "src"))

from ombra_mcp.tools.code_generator import (
    check_implementation_status,
    generate_exit_handler,
    get_project_implementation_status
)

def demo_1_check_existing_handler():
    """Demo: Check status of an existing handler"""
    print("\n" + "=" * 80)
    print("DEMO 1: Check Status of Existing Handler (CPUID)")
    print("=" * 80)

    status = check_implementation_status("exit_handler", "CPUID")

    print(f"\nExists: {status['exists']}")
    print(f"File: {status['file_path']}")
    print(f"Function: {status['function_name']}")
    print(f"Line: {status['line_number']}")
    print(f"Is Stub: {status['is_stub']}")
    print("\nCode Snippet:")
    print("-" * 80)
    print(status['snippet'][:400])
    print("-" * 80)


def demo_2_detect_stub():
    """Demo: Detect a stub implementation"""
    print("\n" + "=" * 80)
    print("DEMO 2: Detect Stub Implementation (IO_INSTRUCTION)")
    print("=" * 80)

    status = check_implementation_status("exit_handler", "IO_INSTRUCTION")

    print(f"\nExists: {status['exists']}")
    print(f"Is Stub: {status['is_stub']}")

    if status['is_stub']:
        print("\nWARNING: This handler exists but is incomplete!")
        print("It needs implementation work before being production-ready.")


def demo_3_detect_missing():
    """Demo: Detect missing handler"""
    print("\n" + "=" * 80)
    print("DEMO 3: Detect Missing Handler (XSETBV)")
    print("=" * 80)

    status = check_implementation_status("exit_handler", "XSETBV")

    print(f"\nExists: {status['exists']}")

    if not status['exists']:
        print("This handler has NOT been implemented yet.")
        print("Use generate_exit_handler() to create it.")


def demo_4_generator_prevents_duplication():
    """Demo: Generator prevents duplicate code"""
    print("\n" + "=" * 80)
    print("DEMO 4: Generator Detects Existing Implementation")
    print("=" * 80)
    print("\nCalling: generate_exit_handler(10)  # CPUID")
    print("-" * 80)

    result = generate_exit_handler(10, stealth=True, force=False)

    # Show first part of result
    lines = result.split('\n')
    for line in lines[:25]:  # First 25 lines
        print(line)
    print("\n... (truncated)")


def demo_5_force_generation():
    """Demo: Force generation even when exists"""
    print("\n" + "=" * 80)
    print("DEMO 5: Force Generation (Ignore Existing)")
    print("=" * 80)
    print("\nCalling: generate_exit_handler(10, force=True)")
    print("-" * 80)

    result = generate_exit_handler(10, stealth=True, force=True)

    # Show first part of generated code
    lines = result.split('\n')
    for line in lines[:30]:  # First 30 lines
        print(line)
    print("\n... (truncated)")


def demo_6_generate_missing():
    """Demo: Generate code for missing handler"""
    print("\n" + "=" * 80)
    print("DEMO 6: Generate Code for Missing Handler (HLT)")
    print("=" * 80)
    print("\nCalling: generate_exit_handler(12)  # HLT - not implemented")
    print("-" * 80)

    result = generate_exit_handler(12, stealth=True, force=False)

    # Show generated code
    lines = result.split('\n')
    for line in lines[:25]:
        print(line)
    print("\n... (truncated)")


def demo_7_project_status():
    """Demo: Complete project status"""
    print("\n" + "=" * 80)
    print("DEMO 7: Complete Project Implementation Status")
    print("=" * 80)

    status = get_project_implementation_status()
    print(status["summary"])


def demo_8_practical_workflow():
    """Demo: Practical development workflow"""
    print("\n" + "=" * 80)
    print("DEMO 8: Practical Development Workflow")
    print("=" * 80)

    print("\nScenario: You want to implement the HLT handler")
    print("-" * 80)

    # Step 1: Check current status
    print("\n1. Check if HLT is already implemented...")
    status = check_implementation_status("exit_handler", "HLT")

    if status['exists']:
        print(f"   → Already exists in {status['file_path']}:{status['line_number']}")
        if status['is_stub']:
            print("   → But it's a stub, needs completion")
    else:
        print("   → Not implemented yet, need to create it")

    # Step 2: Check project status for context
    print("\n2. Check overall project status...")
    project_status = get_project_implementation_status()
    handlers = project_status["exit_handlers"]
    implemented = sum(1 for h, s in handlers.items() if s['exists'] and not s.get('is_stub'))
    stubs = sum(1 for h, s in handlers.items() if s.get('is_stub'))
    missing = sum(1 for h, s in handlers.items() if not s['exists'])
    print(f"   → {implemented} handlers fully implemented")
    print(f"   → {stubs} handlers are stubs")
    print(f"   → {missing} handlers missing")

    # Step 3: Generate if needed
    if not status['exists']:
        print("\n3. Generating HLT handler code...")
        code = generate_exit_handler(12, stealth=True)  # HLT = reason 12
        print("   → Code generated successfully!")
        print("\n   First few lines:")
        for line in code.split('\n')[:10]:
            print(f"   {line}")

    print("\n4. Next steps:")
    print("   → Copy generated code to handlers/hlt.c")
    print("   → Add HandleHlt to handlers/handlers.h")
    print("   → Wire up in exit_dispatch.c")
    print("   → Test with VM-exit tracing")


if __name__ == "__main__":
    print("\n" + "=" * 80)
    print("PROJECT-AWARE CODE GENERATOR - COMPREHENSIVE DEMO")
    print("=" * 80)

    try:
        demo_1_check_existing_handler()
        demo_2_detect_stub()
        demo_3_detect_missing()
        demo_4_generator_prevents_duplication()
        demo_5_force_generation()
        demo_6_generate_missing()
        demo_7_project_status()
        demo_8_practical_workflow()

        print("\n" + "=" * 80)
        print("ALL DEMOS COMPLETED SUCCESSFULLY")
        print("=" * 80)
        print("\nKey Takeaways:")
        print("  1. Generators check existing code before generating")
        print("  2. Stubs are detected and flagged")
        print("  3. force=True allows override when needed")
        print("  4. Project status gives instant overview")
        print("  5. Development workflow is streamlined")
        print("=" * 80 + "\n")

    except Exception as e:
        print(f"\nERROR: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
