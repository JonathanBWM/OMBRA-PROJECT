#!/usr/bin/env python3
"""
Seed Project Brain Components Database

Populates the components and exit_handlers tables with accurate data based on
the actual hypervisor codebase analysis.
"""

import sys
from pathlib import Path

# Add parent to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent / "src"))

from ombra_watcherd.database import ProjectBrainDB


def seed_components(db: ProjectBrainDB):
    """Seed the components table with actual hypervisor components."""

    print("Seeding components table...")

    # =============================================================================
    # Core VMX Components
    # =============================================================================

    db.upsert_component(
        id_="vmx_core",
        name="VMX Core Operations",
        category="core",
        status="implemented",
        files=[
            "hypervisor/hypervisor/vmx.c",
            "hypervisor/hypervisor/vmx.h",
            "hypervisor/hypervisor/entry.c",
        ],
        missing=None,
        depends_on=None
    )

    db.upsert_component(
        id_="vmcs_setup",
        name="VMCS Configuration",
        category="core",
        status="implemented",
        files=[
            "hypervisor/hypervisor/vmcs.c",
            "hypervisor/hypervisor/vmcs.h",
            "hypervisor/shared/vmcs_fields.h",
        ],
        missing=None,
        depends_on=["vmx_core"]
    )

    db.upsert_component(
        id_="exit_dispatch",
        name="VM-Exit Dispatcher",
        category="core",
        status="implemented",
        files=[
            "hypervisor/hypervisor/exit_dispatch.c",
            "hypervisor/hypervisor/exit_dispatch.h",
            "hypervisor/shared/exit_reasons.h",
        ],
        missing=None,
        depends_on=["vmcs_setup"]
    )

    # =============================================================================
    # Memory Management
    # =============================================================================

    db.upsert_component(
        id_="ept",
        name="Extended Page Tables",
        category="memory",
        status="partial",
        files=[
            "hypervisor/hypervisor/ept.c",
            "hypervisor/hypervisor/ept.h",
            "hypervisor/shared/ept_defs.h",
        ],
        missing=[
            "2MB page splitting not fully tested",
            "4KB granular mapping in production",
            "INVEPT optimization",
        ],
        depends_on=["vmx_core"]
    )

    db.upsert_component(
        id_="hooks",
        name="EPT Hook Framework",
        category="memory",
        status="partial",
        files=[
            "hypervisor/hypervisor/hooks.c",
            "hypervisor/hypervisor/hooks.h",
        ],
        missing=[
            "Execute-only EPT not fully implemented (needs MTF)",
            "Shadow page pool management simplified",
            "Single-step restoration for read/write violations",
        ],
        depends_on=["ept"]
    )

    # =============================================================================
    # Exit Handlers
    # =============================================================================

    db.upsert_component(
        id_="handler_cpuid",
        name="CPUID Handler",
        category="handlers",
        status="implemented",
        files=[
            "hypervisor/hypervisor/handlers/cpuid.c",
        ],
        missing=None,
        depends_on=["exit_dispatch"]
    )

    db.upsert_component(
        id_="handler_rdtsc",
        name="RDTSC/RDTSCP Handlers",
        category="handlers",
        status="implemented",
        files=[
            "hypervisor/hypervisor/handlers/rdtsc.c",
        ],
        missing=None,
        depends_on=["exit_dispatch", "timing"]
    )

    db.upsert_component(
        id_="handler_msr",
        name="MSR Read/Write Handlers",
        category="handlers",
        status="implemented",
        files=[
            "hypervisor/hypervisor/handlers/msr.c",
        ],
        missing=None,
        depends_on=["exit_dispatch"]
    )

    db.upsert_component(
        id_="handler_cr_access",
        name="CR Access Handler",
        category="handlers",
        status="implemented",
        files=[
            "hypervisor/hypervisor/handlers/cr_access.c",
        ],
        missing=None,
        depends_on=["exit_dispatch"]
    )

    db.upsert_component(
        id_="handler_ept_violation",
        name="EPT Violation Handler",
        category="handlers",
        status="implemented",
        files=[
            "hypervisor/hypervisor/handlers/ept_violation.c",
        ],
        missing=None,
        depends_on=["exit_dispatch", "hooks"]
    )

    db.upsert_component(
        id_="handler_vmcall",
        name="VMCALL Handler",
        category="handlers",
        status="implemented",
        files=[
            "hypervisor/hypervisor/handlers/vmcall.c",
        ],
        missing=None,
        depends_on=["exit_dispatch"]
    )

    db.upsert_component(
        id_="handler_exception",
        name="Exception/NMI Handler",
        category="handlers",
        status="implemented",
        files=[
            "hypervisor/hypervisor/handlers/exception.c",
        ],
        missing=None,
        depends_on=["exit_dispatch"]
    )

    db.upsert_component(
        id_="handler_io",
        name="I/O Instruction Handler",
        category="handlers",
        status="implemented",
        files=[
            "hypervisor/hypervisor/handlers/io.c",
        ],
        missing=None,
        depends_on=["exit_dispatch"]
    )

    # =============================================================================
    # Stealth & Timing
    # =============================================================================

    db.upsert_component(
        id_="stealth",
        name="Stealth Features",
        category="stealth",
        status="implemented",
        files=[
            "hypervisor/hypervisor/handlers/cpuid.c",
            "hypervisor/hypervisor/handlers/msr.c",
            "hypervisor/hypervisor/handlers/rdtsc.c",
        ],
        missing=[
            "Advanced MSR hiding (only basic VMX MSRs hidden)",
            "CPUID timing normalization",
            "CR4 shadow not yet implemented",
        ],
        depends_on=["handler_cpuid", "handler_msr", "handler_rdtsc"]
    )

    db.upsert_component(
        id_="timing",
        name="Timing Compensation",
        category="stealth",
        status="stub",
        files=[
            "hypervisor/hypervisor/timing.c",
            "hypervisor/hypervisor/timing.h",
        ],
        missing=[
            "Calibration not implemented (uses fixed overhead)",
            "Per-handler timing profiles",
            "Adaptive compensation",
        ],
        depends_on=None
    )

    # =============================================================================
    # Usermode Loader
    # =============================================================================

    db.upsert_component(
        id_="usermode_loader",
        name="Ring-3 Loader",
        category="tools",
        status="implemented",
        files=[
            "hypervisor/usermode/main.c",
            "hypervisor/usermode/driver_interface.c",
            "hypervisor/usermode/driver_interface.h",
            "hypervisor/usermode/payload_loader.c",
            "hypervisor/usermode/payload_loader.h",
            "hypervisor/usermode/debug_reader.c",
            "hypervisor/usermode/debug_reader.h",
        ],
        missing=None,
        depends_on=["byovd_loader"]
    )

    # =============================================================================
    # BYOVD Exploits
    # =============================================================================

    db.upsert_component(
        id_="byovd_loader",
        name="BYOVD Driver Loader",
        category="tools",
        status="implemented",
        files=[
            "hypervisor/usermode/byovd/deployer.c",
            "hypervisor/usermode/byovd/deployer.h",
        ],
        missing=None,
        depends_on=["byovd_supdrv", "byovd_throttlestop"]
    )

    db.upsert_component(
        id_="byovd_supdrv",
        name="Ld9BoxSup.sys Exploit",
        category="tools",
        status="implemented",
        files=[
            "hypervisor/usermode/byovd/supdrv.c",
            "hypervisor/usermode/byovd/supdrv.h",
            "hypervisor/usermode/byovd/supdrv_types.h",
        ],
        missing=None,
        depends_on=None
    )

    db.upsert_component(
        id_="byovd_throttlestop",
        name="ThrottleStop Exploit",
        category="tools",
        status="implemented",
        files=[
            "hypervisor/usermode/byovd/throttlestop.c",
            "hypervisor/usermode/byovd/throttlestop.h",
        ],
        missing=None,
        depends_on=None
    )

    db.upsert_component(
        id_="byovd_crypto",
        name="Driver Crypto/Deobfuscation",
        category="tools",
        status="implemented",
        files=[
            "hypervisor/usermode/byovd/crypto.c",
            "hypervisor/usermode/byovd/crypto.h",
        ],
        missing=None,
        depends_on=None
    )

    db.upsert_component(
        id_="byovd_nt_helpers",
        name="NT Undocumented Helpers",
        category="tools",
        status="implemented",
        files=[
            "hypervisor/usermode/byovd/nt_helpers.c",
            "hypervisor/usermode/byovd/nt_defs.h",
        ],
        missing=None,
        depends_on=None
    )

    # =============================================================================
    # Debug/Diagnostics
    # =============================================================================

    db.upsert_component(
        id_="debug",
        name="Debug Logging",
        category="tools",
        status="implemented",
        files=[
            "hypervisor/hypervisor/debug.c",
            "hypervisor/hypervisor/debug.h",
        ],
        missing=None,
        depends_on=None
    )

    print(f"✓ Seeded {len(db.get_all_components())} components")


def seed_exit_handlers(db: ProjectBrainDB):
    """Seed the exit_handlers table with accurate implementation status."""

    print("\nSeeding exit_handlers table...")

    # Exit handlers with full implementation + stealth
    implemented_with_stealth = [
        (10, "EXIT_REASON_CPUID", "handlers/cpuid.c", 20, True, False),
        (16, "EXIT_REASON_RDTSC", "handlers/rdtsc.c", 26, True, True),
        (51, "EXIT_REASON_RDTSCP", "handlers/rdtsc.c", 70, True, True),
        (31, "EXIT_REASON_RDMSR", "handlers/msr.c", None, True, False),
        (32, "EXIT_REASON_WRMSR", "handlers/msr.c", None, True, False),
    ]

    # Exit handlers with implementation but no stealth features
    implemented_no_stealth = [
        (28, "EXIT_REASON_CR_ACCESS", "handlers/cr_access.c", None, False, False),
        (48, "EXIT_REASON_EPT_VIOLATION", "handlers/ept_violation.c", None, False, False),
        (18, "EXIT_REASON_VMCALL", "handlers/vmcall.c", None, False, False),
        (0, "EXIT_REASON_EXCEPTION_NMI", "handlers/exception.c", None, False, False),
        (30, "EXIT_REASON_IO_INSTRUCTION", "handlers/io.c", None, False, False),
    ]

    # Exit handlers that are pass-through in exit_dispatch.c
    passthrough_handlers = [
        (2, "EXIT_REASON_TRIPLE_FAULT", "exit_dispatch.c", 89, False, False),
        (12, "EXIT_REASON_HLT", "exit_dispatch.c", 96, False, False),
        (14, "EXIT_REASON_INVLPG", "exit_dispatch.c", 102, False, False),
        (55, "EXIT_REASON_XSETBV", "exit_dispatch.c", 137, False, False),
    ]

    # VMX instruction handlers (inject #UD)
    vmx_instruction_handlers = [
        (15, "EXIT_REASON_VMLAUNCH", "exit_dispatch.c", 107, True, False),
        (24, "EXIT_REASON_VMRESUME", "exit_dispatch.c", 108, True, False),
        (23, "EXIT_REASON_VMREAD", "exit_dispatch.c", 109, True, False),
        (24, "EXIT_REASON_VMWRITE", "exit_dispatch.c", 110, True, False),
        (27, "EXIT_REASON_VMXON", "exit_dispatch.c", 111, True, False),
        (26, "EXIT_REASON_VMXOFF", "exit_dispatch.c", 112, True, False),
        (25, "EXIT_REASON_VMCLEAR", "exit_dispatch.c", 113, True, False),
        (21, "EXIT_REASON_VMPTRLD", "exit_dispatch.c", 114, True, False),
        (22, "EXIT_REASON_VMPTRST", "exit_dispatch.c", 115, True, False),
        (50, "EXIT_REASON_INVEPT", "exit_dispatch.c", 116, True, False),
        (53, "EXIT_REASON_INVVPID", "exit_dispatch.c", 117, True, False),
    ]

    # Missing handlers (return to default in switch)
    missing_handlers = [
        (1, "EXIT_REASON_EXT_INTERRUPT"),
        (3, "EXIT_REASON_INIT_SIGNAL"),
        (4, "EXIT_REASON_SIPI"),
        (5, "EXIT_REASON_IO_SMI"),
        (6, "EXIT_REASON_OTHER_SMI"),
        (7, "EXIT_REASON_INT_WINDOW"),
        (8, "EXIT_REASON_NMI_WINDOW"),
        (9, "EXIT_REASON_TASK_SWITCH"),
        (11, "EXIT_REASON_GETSEC"),
        (13, "EXIT_REASON_INVD"),
        (17, "EXIT_REASON_VMCLEAR"),
        (19, "EXIT_REASON_VMLAUNCH"),
        (20, "EXIT_REASON_VMPTRLD"),
        (29, "EXIT_REASON_MOV_DR"),
        (33, "EXIT_REASON_INVALID_GUEST_STATE"),
        (34, "EXIT_REASON_MSR_LOADING"),
        (36, "EXIT_REASON_MWAIT"),
        (37, "EXIT_REASON_MONITOR_TRAP_FLAG"),
        (39, "EXIT_REASON_MONITOR"),
        (40, "EXIT_REASON_PAUSE"),
        (41, "EXIT_REASON_MCE_DURING_VMENTRY"),
        (43, "EXIT_REASON_TPR_BELOW_THRESHOLD"),
        (44, "EXIT_REASON_APIC_ACCESS"),
        (45, "EXIT_REASON_EOI_INDUCED"),
        (46, "EXIT_REASON_GDTR_IDTR_ACCESS"),
        (47, "EXIT_REASON_LDTR_TR_ACCESS"),
        (49, "EXIT_REASON_EPT_MISCONFIG"),
        (52, "EXIT_REASON_INVEPT"),
        (54, "EXIT_REASON_RDRAND"),
        (56, "EXIT_REASON_INVPCID"),
        (57, "EXIT_REASON_VMFUNC"),
        (58, "EXIT_REASON_ENCLS"),
        (59, "EXIT_REASON_RDSEED"),
        (60, "EXIT_REASON_PML_FULL"),
        (61, "EXIT_REASON_XSAVES"),
        (62, "EXIT_REASON_XRSTORS"),
        (63, "EXIT_REASON_PCONFIG"),
        (64, "EXIT_REASON_SPP"),
        (65, "EXIT_REASON_UMWAIT"),
        (66, "EXIT_REASON_TPAUSE"),
        (67, "EXIT_REASON_LOADIWKEY"),
    ]

    # Insert all handlers
    for reason, name, file, line, stealth, timing in implemented_with_stealth:
        db.upsert_exit_handler(
            reason=reason,
            name=name,
            status="implemented",
            file=f"hypervisor/hypervisor/{file}",
            line=line,
            has_stealth=stealth,
            has_timing=timing,
            notes="Fully implemented with stealth features"
        )

    for reason, name, file, line, stealth, timing in implemented_no_stealth:
        db.upsert_exit_handler(
            reason=reason,
            name=name,
            status="implemented",
            file=f"hypervisor/hypervisor/{file}",
            line=line,
            has_stealth=stealth,
            has_timing=timing,
            notes="Implemented but no stealth features"
        )

    for reason, name, file, line, stealth, timing in passthrough_handlers:
        db.upsert_exit_handler(
            reason=reason,
            name=name,
            status="stub",
            file=f"hypervisor/hypervisor/{file}",
            line=line,
            has_stealth=stealth,
            has_timing=timing,
            notes="Pass-through in exit_dispatch (advance RIP only)"
        )

    for reason, name, file, line, stealth, timing in vmx_instruction_handlers:
        db.upsert_exit_handler(
            reason=reason,
            name=name,
            status="implemented",
            file=f"hypervisor/hypervisor/{file}",
            line=line,
            has_stealth=stealth,
            has_timing=timing,
            notes="VMX instruction - injects #UD to guest (stealth)"
        )

    for reason, name in missing_handlers:
        db.upsert_exit_handler(
            reason=reason,
            name=name,
            status="missing",
            file=None,
            line=None,
            has_stealth=False,
            has_timing=False,
            notes="Not handled, falls to default case"
        )

    handlers = db.get_exit_handlers()
    print(f"✓ Seeded {len(handlers)} exit handlers")

    # Print summary
    by_status = {}
    for h in handlers:
        status = h['status']
        by_status[status] = by_status.get(status, 0) + 1

    print(f"  - Implemented: {by_status.get('implemented', 0)}")
    print(f"  - Stub: {by_status.get('stub', 0)}")
    print(f"  - Missing: {by_status.get('missing', 0)}")

    stealth_count = sum(1 for h in handlers if h['has_stealth'])
    timing_count = sum(1 for h in handlers if h['has_timing'])
    print(f"  - With stealth: {stealth_count}")
    print(f"  - With timing: {timing_count}")


def main():
    """Main entry point."""
    print("=" * 80)
    print("PROJECT-OMBRA Component Database Seeding")
    print("=" * 80)

    # Initialize database
    db = ProjectBrainDB()

    # Seed components
    seed_components(db)

    # Seed exit handlers
    seed_exit_handlers(db)

    print("\n" + "=" * 80)
    print("✓ Database seeding complete!")
    print("=" * 80)

    # Show project status
    health = db.get_project_health()
    print("\nProject Health Summary:")
    print(f"  Components: {health.get('components', {})}")
    print(f"  Exit Handlers: {health.get('exit_handlers', {})}")


if __name__ == "__main__":
    main()
