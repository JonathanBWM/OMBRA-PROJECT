# Project-Aware Code Generator Enhancement

## Summary

Enhanced the OmbraMCP code generators to be **project-aware** by scanning the actual hypervisor codebase before generating code. This prevents duplicate generation and provides detailed information about existing implementations.

## Location

`/Users/jonathanmcclintock/PROJECT-OMBRA/ombra_mcp_server/src/ombra_mcp/tools/code_generator.py`

## Key Changes

### 1. Implementation Scanner (`check_implementation_status`)

New function that scans the hypervisor codebase to check if a component is already implemented:

```python
def check_implementation_status(component_type: str, component_name: str = None) -> dict
```

**Supported component types:**
- `exit_handler` - VM-exit handlers (e.g., CPUID, RDTSC, VMCALL)
- `vmcs_setup` - VMCS initialization code
- `ept_setup` - EPT setup code
- `msr_bitmap` - MSR bitmap configuration

**Returns detailed status:**
- `exists` - Whether component is implemented
- `file_path` - Path to implementation file
- `function_name` - Name of implementing function
- `line_number` - Line where implementation starts
- `is_stub` - Whether it's just a stub/TODO
- `snippet` - Code snippet showing implementation

**Detection logic:**
- Maps exit reasons to handler functions and file locations
- Uses regex to find function definitions
- Checks for TODO markers to detect stubs
- Verifies handlers are wired up in `exit_dispatch.c`

### 2. Enhanced Generator Functions

All code generator functions now:

1. **Check for existing implementation first** (unless `force=True`)
2. **Return detailed info if exists** instead of blindly generating
3. **Support force parameter** to override and generate anyway

**Modified functions:**
- `generate_exit_handler(reason, stealth=True, force=False)`
- `generate_vmcs_setup(force=False)`
- `generate_ept_setup(memory_gb=512, force=False)`
- `generate_msr_bitmap_setup(intercept_msrs=None, force=False)`

### 3. Project Status Summary (`get_project_implementation_status`)

New function that scans ALL hypervisor components and returns comprehensive status:

```python
def get_project_implementation_status() -> dict
```

**Returns:**
- Exit handler status for all common handlers
- Infrastructure component status (VMCS, EPT, MSR bitmap)
- Formatted text summary with:
  - Implemented handlers (with file:line references)
  - Stub handlers (need completion)
  - Missing handlers
  - Infrastructure component status

## Usage Examples

### Example 1: Generate Exit Handler (Already Exists)

```python
from ombra_mcp.tools.code_generator import generate_exit_handler

# Try to generate CPUID handler
code = generate_exit_handler(10)  # Exit reason 10 = CPUID
```

**Output:**
```
================================================================================
EXISTING IMPLEMENTATION DETECTED: Exit Handler - CPUID
================================================================================

File:     /path/to/hypervisor/handlers/cpuid.c
Function: HandleCpuid
Line:     22

CURRENT IMPLEMENTATION:
--------------------------------------------------------------------------------
VMEXIT_ACTION HandleCpuid(GUEST_REGS* r) {
    int info[4];
    U32 leaf = (U32)r->Rax;
    ...
}
--------------------------------------------------------------------------------

NOTE: Use force=True to generate anyway

================================================================================
```

### Example 2: Force Generate

```python
# Force generate even if exists
code = generate_exit_handler(10, force=True)
```

**Output:**
```c
// Exit Handler: CPUID (Reason 10)
// CPUID
// Notes: Spoof CPUID results for stealth

BOOLEAN HandleCpuid(PGUEST_CONTEXT Context) {
    // === STEALTH: Timing compensation ===
    ULONG64 tscBefore = __rdtsc();

    // Spoof CPUID to hide hypervisor presence
    int regs[4];
    __cpuidex(regs, (int)Context->Rax, (int)Context->Rcx);
    ...
}
```

### Example 3: Check Project Status

```python
from ombra_mcp.tools.code_generator import get_project_implementation_status

status = get_project_implementation_status()
print(status["summary"])
```

**Output:**
```
================================================================================
HYPERVISOR IMPLEMENTATION STATUS
================================================================================

Exit Handlers: 13 implemented, 1 stubs, 9 missing

IMPLEMENTED:
  ✓ CPUID                (cpuid.c:22)
  ✓ RDTSC                (rdtsc.c:22)
  ✓ RDTSCP               (rdtsc.c:84)
  ✓ RDMSR                (msr.c:64)
  ✓ WRMSR                (msr.c:131)
  ✓ CR_ACCESS            (cr_access.c:91)
  ✓ EPT_VIOLATION        (ept_violation.c:35)
  ✓ EPT_MISCONFIG        (ept_misconfig.c:123)
  ✓ VMCALL               (vmcall.c:416)
  ✓ EXCEPTION_NMI        (exception.c:72)
  ✓ MONITOR              (power_mgmt.c:27)
  ✓ MWAIT                (power_mgmt.c:67)
  ✓ PAUSE                (power_mgmt.c:136)

STUBS (need completion):
  ⚠ IO_INSTRUCTION       (io.c:43)

NOT IMPLEMENTED:
  ✗ HLT
  ✗ INVLPG
  ✗ XSETBV
  ✗ INVEPT
  ✗ INVVPID
  ✗ VMXON
  ✗ VMXOFF
  ✗ VMLAUNCH
  ✗ VMRESUME

Infrastructure Components:
  ✓ VMCS Setup           (vmcs.c:66)
  ✓ EPT Setup            (ept.c:63)
  ✓ MSR Bitmap           (vmcs.c:218)

================================================================================
```

## Implementation Details

### Handler Detection Map

```python
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
```

### Path Resolution

```python
HYPERVISOR_ROOT = Path(__file__).parent.parent.parent.parent.parent / "hypervisor" / "hypervisor"
```

Auto-detects hypervisor root from MCP server location.

### Stub Detection

A handler is marked as a stub if:
1. Contains "TODO" in function body
2. Contains "not implemented" (case-insensitive) in function body
3. Function exists but is NOT called in `exit_dispatch.c`

## Benefits

### 1. Prevents Duplication
No more generating handlers that already exist. Generator checks first and shows what's already there.

### 2. Shows Project State
Instantly see what's implemented, what's stubbed, and what's missing.

### 3. Provides Context
When handler exists, shows exact file, line, and code snippet - makes it easy to navigate to existing implementation.

### 4. Detects Stubs
Identifies incomplete implementations that need work, distinguishing them from fully implemented handlers.

### 5. Force Override
Can still generate code when needed via `force=True` parameter.

## Testing

Created test scripts in `/Users/jonathanmcclintock/PROJECT-OMBRA/ombra_mcp_server/`:

- `test_code_generator.py` - Full test suite for scanner and generators
- `test_status_summary.py` - Quick project status check

Run tests:
```bash
cd /Users/jonathanmcclintock/PROJECT-OMBRA/ombra_mcp_server
python3 test_code_generator.py
python3 test_status_summary.py
```

## Current Project Status (as of enhancement)

- **13 handlers fully implemented**
- **1 handler is a stub** (IO_INSTRUCTION)
- **9 handlers not implemented** (HLT, INVLPG, XSETBV, INVEPT, INVVPID, VMX instructions)
- **All infrastructure components implemented** (VMCS, EPT, MSR bitmap)

## Future Enhancements

1. **Auto-detection of exit_dispatch.c registration**
   - Scan for case statements to verify handlers are wired up
   - Suggest adding missing case entries

2. **Diff generation**
   - Compare generated code vs existing implementation
   - Show what's different

3. **Completion suggestions**
   - For stub handlers, suggest specific TODO items
   - Reference Intel SDM sections for unimplemented handlers

4. **Cross-reference detection**
   - Find all places a handler is called/referenced
   - Build dependency graph

5. **Performance tracking**
   - Track which handlers are called most frequently
   - Suggest optimization targets

## Notes for LO

The code generators now understand what's already built. When you ask for a handler:

1. **If it exists** - You get file:line info and a snippet. No duplicate generation.
2. **If it's a stub** - You get a warning that it needs completion.
3. **If it's missing** - You get fresh generated code.
4. **If you force** - You get generated code regardless.

Use `get_project_implementation_status()` anytime to see exactly what state the hypervisor is in.

---

Built by ENI for PROJECT-OMBRA
Enhancement Date: 2025-12-26
