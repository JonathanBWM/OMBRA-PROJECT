"""
Code Generation Tools - Generate hypervisor code from templates
"""

import sqlite3
from pathlib import Path
import re

DATA_DIR = Path(__file__).parent.parent / "data"
DB_PATH = DATA_DIR / "intel_sdm.db"

# Project root detection
HYPERVISOR_ROOT = Path(__file__).parent.parent.parent.parent.parent / "hypervisor" / "hypervisor"


def get_db():
    return sqlite3.connect(DB_PATH)


# =============================================================================
# Project-Aware Implementation Scanner
# =============================================================================

def check_implementation_status(component_type: str, component_name: str = None) -> dict:
    """
    Scan the actual hypervisor codebase to check implementation status.

    Args:
        component_type: Type of component to check
                       ("exit_handler", "vmcs_setup", "ept_setup", "msr_bitmap")
        component_name: Specific component name (e.g., "CPUID" for exit handlers)

    Returns:
        dict with keys:
            - exists: bool - whether component is implemented
            - file_path: str - path to implementation file (if exists)
            - function_name: str - name of the implementing function
            - line_number: int - line where implementation starts
            - is_stub: bool - whether it's just a stub/TODO
            - snippet: str - code snippet showing implementation
    """

    result = {
        "exists": False,
        "file_path": None,
        "function_name": None,
        "line_number": None,
        "is_stub": False,
        "snippet": None
    }

    if component_type == "exit_handler":
        return _check_exit_handler(component_name, result)
    elif component_type == "vmcs_setup":
        return _check_vmcs_setup(result)
    elif component_type == "ept_setup":
        return _check_ept_setup(result)
    elif component_type == "msr_bitmap":
        return _check_msr_bitmap(result)

    return result


def _check_exit_handler(reason_name: str, result: dict) -> dict:
    """Check if an exit handler is implemented."""

    # Map exit reason names to handler function names and files
    handler_map = {
        "CPUID": ("HandleCpuid", "cpuid.c"),
        "RDTSC": ("HandleRdtsc", "rdtsc.c"),
        "RDTSCP": ("HandleRdtscp", "rdtsc.c"),
        "RDMSR": ("HandleRdmsr", "msr.c"),
        "WRMSR": ("HandleWrmsr", "msr.c"),
        "CR_ACCESS": ("HandleCrAccess", "cr_access.c"),
        "EPT_VIOLATION": ("HandleEptViolation", "ept_violation.c"),
        "EPT_MISCONFIG": ("HandleEptMisconfiguration", "ept_misconfig.c"),
        "VMCALL": ("HandleVmcall", "vmcall.c"),
        "EXCEPTION_NMI": ("HandleException", "exception.c"),
        "IO_INSTRUCTION": ("HandleIo", "io.c"),
        "MONITOR": ("HandleMonitor", "power_mgmt.c"),
        "MWAIT": ("HandleMwait", "power_mgmt.c"),
        "PAUSE": ("HandlePause", "power_mgmt.c"),
    }

    # Normalize reason name
    reason_upper = reason_name.upper() if reason_name else ""

    if reason_upper in handler_map:
        func_name, file_name = handler_map[reason_upper]
        file_path = HYPERVISOR_ROOT / "handlers" / file_name

        if file_path.exists():
            content = file_path.read_text()

            # Find function definition
            pattern = rf"VMEXIT_ACTION\s+{func_name}\s*\("
            match = re.search(pattern, content)

            if match:
                result["exists"] = True
                result["file_path"] = str(file_path)
                result["function_name"] = func_name

                # Find line number
                lines = content[:match.start()].split('\n')
                result["line_number"] = len(lines)

                # Check if it's a stub (contains TODO or just returns FALSE)
                func_start = match.start()
                # Find closing brace (simplified - assumes proper formatting)
                brace_count = 0
                func_end = func_start
                for i in range(func_start, len(content)):
                    if content[i] == '{':
                        brace_count += 1
                    elif content[i] == '}':
                        brace_count -= 1
                        if brace_count == 0:
                            func_end = i + 1
                            break

                func_body = content[func_start:func_end]
                result["snippet"] = func_body[:500]  # First 500 chars

                # Check if it's a stub
                if "TODO" in func_body or "not implemented" in func_body.lower():
                    result["is_stub"] = True

                # Also check in exit_dispatch.c to verify it's wired up
                dispatch_file = HYPERVISOR_ROOT / "exit_dispatch.c"
                if dispatch_file.exists():
                    dispatch_content = dispatch_file.read_text()
                    if func_name not in dispatch_content:
                        result["is_stub"] = True  # Exists but not wired up

    return result


def _check_vmcs_setup(result: dict) -> dict:
    """Check VMCS setup implementation."""

    vmcs_file = HYPERVISOR_ROOT / "vmcs.c"
    if vmcs_file.exists():
        content = vmcs_file.read_text()

        # Look for main setup functions
        if "VmcsSetupControls" in content:
            result["exists"] = True
            result["file_path"] = str(vmcs_file)
            result["function_name"] = "VmcsSetupControls"

            # Find line number
            match = re.search(r"void\s+VmcsSetupControls", content)
            if match:
                lines = content[:match.start()].split('\n')
                result["line_number"] = len(lines)

            # Extract snippet
            result["snippet"] = content[match.start():match.start()+500] if match else None

    return result


def _check_ept_setup(result: dict) -> dict:
    """Check EPT setup implementation."""

    ept_file = HYPERVISOR_ROOT / "ept.c"
    if ept_file.exists():
        content = ept_file.read_text()

        # Look for EPT initialization
        if "EptInitialize" in content or "SetupEptIdentityMap" in content:
            result["exists"] = True
            result["file_path"] = str(ept_file)
            result["function_name"] = "EptInitialize"

            # Find line number
            match = re.search(r"OMBRA_STATUS\s+EptInitialize", content)
            if match:
                lines = content[:match.start()].split('\n')
                result["line_number"] = len(lines)
                result["snippet"] = content[match.start():match.start()+500]

    return result


def _check_msr_bitmap(result: dict) -> dict:
    """Check MSR bitmap setup implementation."""

    vmcs_file = HYPERVISOR_ROOT / "vmcs.c"
    if vmcs_file.exists():
        content = vmcs_file.read_text()

        # MSR bitmap is configured in VmcsSetupControls
        if "MsrBitmap" in content and "MSR_IA32_VMX" in content:
            result["exists"] = True
            result["file_path"] = str(vmcs_file)
            result["function_name"] = "VmcsSetupControls (MSR bitmap section)"

            # Find the MSR bitmap section
            match = re.search(r"// MSR Bitmap", content)
            if match:
                lines = content[:match.start()].split('\n')
                result["line_number"] = len(lines)
                result["snippet"] = content[match.start():match.start()+800]

    return result


def _format_existing_implementation(component_type: str, component_name: str,
                                    status: dict, note: str = "") -> str:
    """Format a message about an existing implementation."""

    lines = []
    lines.append("=" * 80)
    lines.append(f"EXISTING IMPLEMENTATION DETECTED: {component_type} - {component_name}")
    lines.append("=" * 80)
    lines.append("")
    lines.append(f"File:     {status['file_path']}")
    lines.append(f"Function: {status['function_name']}")
    lines.append(f"Line:     {status['line_number']}")

    if status.get('is_stub'):
        lines.append("")
        lines.append("WARNING: This appears to be a stub or incomplete implementation.")
        lines.append("         Contains TODO markers or is not wired up in exit dispatcher.")

    lines.append("")
    lines.append("CURRENT IMPLEMENTATION:")
    lines.append("-" * 80)

    if status.get('snippet'):
        lines.append(status['snippet'])
    else:
        lines.append("(No snippet available)")

    lines.append("-" * 80)

    if note:
        lines.append("")
        lines.append(f"NOTE: {note}")

    lines.append("")
    lines.append("=" * 80)

    return "\n".join(lines)


async def generate_vmcs_setup(force: bool = False) -> str:
    """
    Generate complete VMCS initialization code.

    Args:
        force: Generate code even if VMCS setup already exists

    Returns:
        Generated code or existing implementation info
    """

    # Check if VMCS setup already exists
    if not force:
        status = check_implementation_status("vmcs_setup")
        if status["exists"]:
            return _format_existing_implementation(
                "VMCS Setup",
                "VmcsSetupControls",
                status,
                "Use force=True to generate anyway"
            )

    conn = get_db()

    code = []
    code.append("// VMCS Initialization - Auto-generated from Intel SDM")
    code.append("// Adjust control fields using capability MSRs before use")
    code.append("")
    code.append("NTSTATUS InitializeVmcs(PVMCS_DATA VmcsData) {")
    code.append("    NTSTATUS status = STATUS_SUCCESS;")
    code.append("")

    # Group by category
    categories = ["control", "guest_state", "host_state"]

    for cat in categories:
        cursor = conn.execute(
            "SELECT name, encoding, width, description FROM vmcs_fields WHERE category = ? ORDER BY encoding",
            (cat,)
        )

        code.append(f"    // === {cat.upper().replace('_', ' ')} FIELDS ===")
        for name, encoding, width, desc in cursor:
            code.append(f"    // {desc}")
            code.append(f"    __vmx_vmwrite(0x{encoding:04X}, VmcsData->{name});  // {name}")
        code.append("")

    code.append("    return status;")
    code.append("}")

    conn.close()
    return "\n".join(code)


async def generate_exit_handler(reason: int, stealth: bool = True, force: bool = False) -> str:
    """
    Generate exit handler for specific reason.

    Args:
        reason: Exit reason number (0-65)
        stealth: Include stealth/timing compensation code
        force: Generate code even if handler already exists

    Returns:
        Generated code or existing implementation info
    """
    conn = get_db()

    cursor = conn.execute(
        "SELECT name, description, handling_notes, has_qualification FROM exit_reasons WHERE reason_number = ?",
        (reason,)
    )
    row = cursor.fetchone()

    if not row:
        conn.close()
        return f"// Exit reason {reason} not found in database"

    name, desc, notes, has_qual = row

    # Check if handler already exists
    if not force:
        status = check_implementation_status("exit_handler", name)
        if status["exists"]:
            conn.close()
            return _format_existing_implementation(
                "Exit Handler",
                name,
                status,
                f"Use force=True to generate anyway"
            )

    # If we get here, either force=True or handler doesn't exist - generate it

    code = []
    code.append(f"// Exit Handler: {name} (Reason {reason})")
    code.append(f"// {desc}")
    code.append(f"// Notes: {notes}")
    code.append("")
    code.append(f"BOOLEAN Handle{name.title().replace('_', '')}(PGUEST_CONTEXT Context) {{")

    if has_qual:
        code.append("    ULONG64 qualification;")
        code.append("    __vmx_vmread(0x6400, &qualification);  // EXIT_QUALIFICATION")
        code.append("")

    if stealth:
        code.append("    // === STEALTH: Timing compensation ===")
        code.append("    ULONG64 tscBefore = __rdtsc();")
        code.append("")

    # Specific handlers
    if reason == 10:  # CPUID
        code.append("    // Spoof CPUID to hide hypervisor presence")
        code.append("    int regs[4];")
        code.append("    __cpuidex(regs, (int)Context->Rax, (int)Context->Rcx);")
        code.append("")
        code.append("    if (Context->Rax == 1) {")
        code.append("        // Clear hypervisor present bit (ECX.31)")
        code.append("        regs[2] &= ~(1 << 31);")
        code.append("    }")
        code.append("    else if (Context->Rax == 0x40000000) {")
        code.append("        // Return garbage for hypervisor leaf")
        code.append("        regs[0] = regs[1] = regs[2] = regs[3] = 0;")
        code.append("    }")
        code.append("")
        code.append("    Context->Rax = regs[0];")
        code.append("    Context->Rbx = regs[1];")
        code.append("    Context->Rcx = regs[2];")
        code.append("    Context->Rdx = regs[3];")

    elif reason == 16:  # RDTSC
        code.append("    // Apply TSC offset for timing stealth")
        code.append("    ULONG64 tsc = __rdtsc();")
        code.append("    ULONG64 offset;")
        code.append("    __vmx_vmread(0x2010, &offset);  // TSC_OFFSET")
        code.append("    tsc += offset;")
        code.append("")
        code.append("    Context->Rax = (ULONG32)tsc;")
        code.append("    Context->Rdx = (ULONG32)(tsc >> 32);")

    elif reason == 18:  # VMCALL
        code.append("    // Handle hypercall or inject #UD")
        code.append("    if (Context->Rax == HYPERCALL_MAGIC) {")
        code.append("        return HandleHypercall(Context);")
        code.append("    }")
        code.append("    // Inject #UD for unauthorized VMCALL")
        code.append("    InjectException(EXCEPTION_INVALID_OPCODE, FALSE, 0);")
        code.append("    return TRUE;")

    elif reason == 28:  # CR_ACCESS
        code.append("    // Decode CR access from qualification")
        code.append("    ULONG crNumber = qualification & 0xF;")
        code.append("    ULONG accessType = (qualification >> 4) & 0x3;")
        code.append("    ULONG gprIndex = (qualification >> 8) & 0xF;")
        code.append("")
        code.append("    if (accessType == 0) {  // MOV to CR")
        code.append("        ULONG64 value = GetGprByIndex(Context, gprIndex);")
        code.append("        HandleCrWrite(crNumber, value);")
        code.append("    } else {  // MOV from CR")
        code.append("        ULONG64 value = GetCrValue(crNumber);")
        code.append("        SetGprByIndex(Context, gprIndex, value);")
        code.append("    }")

    elif reason == 30:  # IO_INSTRUCTION
        code.append("    // Decode I/O instruction")
        code.append("    USHORT port = (USHORT)(qualification >> 16);")
        code.append("    ULONG size = (qualification & 0x7) + 1;")
        code.append("    BOOLEAN isIn = (qualification >> 3) & 1;")
        code.append("    BOOLEAN isString = (qualification >> 4) & 1;")
        code.append("")
        code.append("    if (isIn) {")
        code.append("        Context->Rax = IoRead(port, size);")
        code.append("    } else {")
        code.append("        IoWrite(port, size, (ULONG)Context->Rax);")
        code.append("    }")

    elif reason == 31:  # RDMSR
        code.append("    ULONG32 msr = (ULONG32)Context->Rcx;")
        code.append("    ULONG64 value = __readmsr(msr);")
        code.append("")
        code.append("    // Shadow VMX MSRs to hide hypervisor")
        code.append("    if (msr >= 0x480 && msr <= 0x492) {")
        code.append("        value = 0;  // Hide VMX capability MSRs")
        code.append("    }")
        code.append("")
        code.append("    Context->Rax = (ULONG32)value;")
        code.append("    Context->Rdx = (ULONG32)(value >> 32);")

    elif reason == 32:  # WRMSR
        code.append("    ULONG32 msr = (ULONG32)Context->Rcx;")
        code.append("    ULONG64 value = (Context->Rdx << 32) | (ULONG32)Context->Rax;")
        code.append("")
        code.append("    // Validate MSR write")
        code.append("    if (!ValidateMsrWrite(msr, value)) {")
        code.append("        InjectException(EXCEPTION_GENERAL_PROTECTION, TRUE, 0);")
        code.append("        return TRUE;")
        code.append("    }")
        code.append("    __writemsr(msr, value);")

    elif reason == 48:  # EPT_VIOLATION
        code.append("    ULONG64 guestPhysical;")
        code.append("    __vmx_vmread(0x2400, &guestPhysical);  // GUEST_PHYSICAL_ADDRESS")
        code.append("")
        code.append("    // Decode violation type")
        code.append("    BOOLEAN read = qualification & 1;")
        code.append("    BOOLEAN write = (qualification >> 1) & 1;")
        code.append("    BOOLEAN execute = (qualification >> 2) & 1;")
        code.append("")
        code.append("    return HandleEptViolation(guestPhysical, read, write, execute);")

    else:
        code.append(f"    // TODO: Implement handler for {name}")
        code.append("    return FALSE;")

    if stealth:
        code.append("")
        code.append("    // === STEALTH: Compensate for exit time ===")
        code.append("    ULONG64 tscAfter = __rdtsc();")
        code.append("    ULONG64 exitTime = tscAfter - tscBefore;")
        code.append("    AdjustTscOffset(exitTime);")

    code.append("")
    code.append("    AdvanceGuestRip();")
    code.append("    return TRUE;")
    code.append("}")

    conn.close()
    return "\n".join(code)


async def generate_ept_setup(memory_gb: int = 512, force: bool = False) -> str:
    """
    Generate EPT identity mapping setup code.

    Args:
        memory_gb: Size of identity map in GB (default 512)
        force: Generate code even if EPT setup already exists

    Returns:
        Generated code or existing implementation info
    """

    # Check if EPT setup already exists
    if not force:
        status = check_implementation_status("ept_setup")
        if status["exists"]:
            return _format_existing_implementation(
                "EPT Setup",
                "EptInitialize",
                status,
                "Use force=True to generate anyway"
            )

    code = []
    code.append(f"// EPT Identity Mapping for {memory_gb}GB physical memory")
    code.append("// Uses 1GB pages for efficiency")
    code.append("")
    code.append("NTSTATUS SetupEptIdentityMap(PEPT_STATE EptState) {")
    code.append("    PHYSICAL_ADDRESS maxPhys;")
    code.append(f"    maxPhys.QuadPart = {memory_gb}ULL * 1024 * 1024 * 1024;")
    code.append("")
    code.append("    // Allocate PML4 (1 page)")
    code.append("    EptState->Pml4 = AllocateContiguousMemory(PAGE_SIZE);")
    code.append("    RtlZeroMemory(EptState->Pml4, PAGE_SIZE);")
    code.append("")
    code.append("    // Allocate PDPT (1 page for 512GB)")
    code.append("    EptState->Pdpt = AllocateContiguousMemory(PAGE_SIZE);")
    code.append("    RtlZeroMemory(EptState->Pdpt, PAGE_SIZE);")
    code.append("")
    code.append("    // Setup PML4E pointing to PDPT")
    code.append("    PEPT_PML4E pml4e = &EptState->Pml4[0];")
    code.append("    pml4e->Read = 1;")
    code.append("    pml4e->Write = 1;")
    code.append("    pml4e->Execute = 1;")
    code.append("    pml4e->PageFrameNumber = MmGetPhysicalAddress(EptState->Pdpt).QuadPart >> 12;")
    code.append("")
    code.append("    // Setup 1GB identity-mapped pages")
    code.append(f"    for (ULONG i = 0; i < {memory_gb}; i++) {{")
    code.append("        PEPT_PDPTE_1GB pdpte = &EptState->Pdpt[i];")
    code.append("        pdpte->Read = 1;")
    code.append("        pdpte->Write = 1;")
    code.append("        pdpte->Execute = 1;")
    code.append("        pdpte->LargePage = 1;  // 1GB page")
    code.append("        pdpte->MemoryType = MTRR_TYPE_WB;  // Write-back")
    code.append("        pdpte->PageFrameNumber = i;  // 1GB aligned")
    code.append("    }")
    code.append("")
    code.append("    // Build EPTP")
    code.append("    EptState->Eptp.MemoryType = MTRR_TYPE_WB;")
    code.append("    EptState->Eptp.PageWalkLength = 3;  // 4-level paging - 1")
    code.append("    EptState->Eptp.EnableAccessAndDirtyFlags = 1;")
    code.append("    EptState->Eptp.PageFrameNumber = MmGetPhysicalAddress(EptState->Pml4).QuadPart >> 12;")
    code.append("")
    code.append("    return STATUS_SUCCESS;")
    code.append("}")

    return "\n".join(code)


def generate_msr_bitmap_setup(intercept_msrs: list = None, force: bool = False) -> str:
    """
    Generate MSR bitmap configuration code.

    Args:
        intercept_msrs: List of MSR addresses to intercept (default: VMX MSRs)
        force: Generate code even if MSR bitmap setup already exists

    Returns:
        Generated code or existing implementation info
    """

    # Check if MSR bitmap already exists
    if not force:
        status = check_implementation_status("msr_bitmap")
        if status["exists"]:
            return _format_existing_implementation(
                "MSR Bitmap Setup",
                "MSR Bitmap Configuration",
                status,
                "Use force=True to generate anyway"
            )

    if intercept_msrs is None:
        intercept_msrs = [0x480, 0x481, 0x482, 0x483, 0x484, 0x485]  # VMX MSRs

    code = []
    code.append("// MSR Bitmap Setup")
    code.append("// Intercepts specific MSRs for virtualization")
    code.append("")
    code.append("NTSTATUS SetupMsrBitmap(PMSR_BITMAP_STATE MsrState) {")
    code.append("    // Allocate 4KB MSR bitmap (4 x 1KB regions)")
    code.append("    MsrState->Bitmap = AllocateContiguousMemory(PAGE_SIZE);")
    code.append("    RtlZeroMemory(MsrState->Bitmap, PAGE_SIZE);")
    code.append("")
    code.append("    // Bitmap layout:")
    code.append("    // 0x000-0x3FF: Read bitmap for MSRs 0x00000000-0x00001FFF")
    code.append("    // 0x400-0x7FF: Read bitmap for MSRs 0xC0000000-0xC0001FFF")
    code.append("    // 0x800-0xBFF: Write bitmap for MSRs 0x00000000-0x00001FFF")
    code.append("    // 0xC00-0xFFF: Write bitmap for MSRs 0xC0000000-0xC0001FFF")
    code.append("")

    for msr in intercept_msrs:
        if msr < 0x2000:
            offset = msr // 8
            bit = msr % 8
            code.append(f"    // Intercept MSR 0x{msr:X}")
            code.append(f"    MsrState->Bitmap[0x{offset:03X}] |= (1 << {bit});  // Read")
            code.append(f"    MsrState->Bitmap[0x{0x800 + offset:03X}] |= (1 << {bit});  // Write")
        elif msr >= 0xC0000000 and msr < 0xC0002000:
            adjusted = msr - 0xC0000000
            offset = adjusted // 8
            bit = adjusted % 8
            code.append(f"    // Intercept MSR 0x{msr:X}")
            code.append(f"    MsrState->Bitmap[0x{0x400 + offset:03X}] |= (1 << {bit});  // Read")
            code.append(f"    MsrState->Bitmap[0x{0xC00 + offset:03X}] |= (1 << {bit});  // Write")

    code.append("")
    code.append("    return STATUS_SUCCESS;")
    code.append("}")

    return "\n".join(code)


# =============================================================================
# Project Status Summary
# =============================================================================

def get_project_implementation_status() -> dict:
    """
    Get comprehensive status of all hypervisor components.

    Returns:
        dict with keys:
            - exit_handlers: dict of handler_name -> status
            - vmcs_setup: status dict
            - ept_setup: status dict
            - msr_bitmap: status dict
            - summary: text summary
    """

    # All possible exit handlers we might implement
    common_handlers = [
        "CPUID", "RDTSC", "RDTSCP", "RDMSR", "WRMSR",
        "CR_ACCESS", "EPT_VIOLATION", "EPT_MISCONFIG",
        "VMCALL", "EXCEPTION_NMI", "IO_INSTRUCTION",
        "MONITOR", "MWAIT", "PAUSE", "HLT",
        "INVLPG", "XSETBV", "INVEPT", "INVVPID",
        "VMXON", "VMXOFF", "VMLAUNCH", "VMRESUME",
    ]

    result = {
        "exit_handlers": {},
        "vmcs_setup": None,
        "ept_setup": None,
        "msr_bitmap": None,
        "summary": []
    }

    # Check all exit handlers
    implemented = []
    stubs = []
    missing = []

    for handler in common_handlers:
        status = check_implementation_status("exit_handler", handler)
        result["exit_handlers"][handler] = status

        if status["exists"]:
            if status.get("is_stub"):
                stubs.append(handler)
            else:
                implemented.append(handler)
        else:
            missing.append(handler)

    # Check infrastructure components
    result["vmcs_setup"] = check_implementation_status("vmcs_setup")
    result["ept_setup"] = check_implementation_status("ept_setup")
    result["msr_bitmap"] = check_implementation_status("msr_bitmap")

    # Build summary
    summary = []
    summary.append("=" * 80)
    summary.append("HYPERVISOR IMPLEMENTATION STATUS")
    summary.append("=" * 80)
    summary.append("")

    # Exit handlers summary
    summary.append(f"Exit Handlers: {len(implemented)} implemented, {len(stubs)} stubs, {len(missing)} missing")
    summary.append("")

    if implemented:
        summary.append("IMPLEMENTED:")
        for name in implemented:
            status = result["exit_handlers"][name]
            file_name = Path(status["file_path"]).name if status["file_path"] else "?"
            summary.append(f"  ✓ {name:20s} ({file_name}:{status.get('line_number', '?')})")
        summary.append("")

    if stubs:
        summary.append("STUBS (need completion):")
        for name in stubs:
            status = result["exit_handlers"][name]
            file_name = Path(status["file_path"]).name if status["file_path"] else "?"
            summary.append(f"  ⚠ {name:20s} ({file_name}:{status.get('line_number', '?')})")
        summary.append("")

    if missing:
        summary.append("NOT IMPLEMENTED:")
        for name in missing:
            summary.append(f"  ✗ {name}")
        summary.append("")

    # Infrastructure components
    summary.append("Infrastructure Components:")
    components = [
        ("VMCS Setup", result["vmcs_setup"]),
        ("EPT Setup", result["ept_setup"]),
        ("MSR Bitmap", result["msr_bitmap"]),
    ]

    for comp_name, status in components:
        if status and status["exists"]:
            file_name = Path(status["file_path"]).name if status["file_path"] else "?"
            summary.append(f"  ✓ {comp_name:20s} ({file_name}:{status.get('line_number', '?')})")
        else:
            summary.append(f"  ✗ {comp_name}")

    summary.append("")
    summary.append("=" * 80)

    result["summary"] = "\n".join(summary)
    return result
