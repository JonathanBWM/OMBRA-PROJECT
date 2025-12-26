"""
Code Generation Tools - Generate hypervisor code from templates
"""

import sqlite3
from pathlib import Path

DATA_DIR = Path(__file__).parent.parent / "data"
DB_PATH = DATA_DIR / "intel_sdm.db"


def get_db():
    return sqlite3.connect(DB_PATH)


def generate_vmcs_setup() -> str:
    """Generate complete VMCS initialization code."""
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


def generate_exit_handler(reason: int, stealth: bool = True) -> str:
    """Generate exit handler for specific reason."""
    conn = get_db()

    cursor = conn.execute(
        "SELECT name, description, handling_notes, has_qualification FROM exit_reasons WHERE reason_number = ?",
        (reason,)
    )
    row = cursor.fetchone()

    if not row:
        conn.close()
        return f"// Exit reason {reason} not found"

    name, desc, notes, has_qual = row

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


def generate_ept_setup(memory_gb: int = 512) -> str:
    """Generate EPT identity mapping setup code."""
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


def generate_msr_bitmap_setup(intercept_msrs: list = None) -> str:
    """Generate MSR bitmap configuration code."""
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
