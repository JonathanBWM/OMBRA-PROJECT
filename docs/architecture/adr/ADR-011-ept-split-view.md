# ADR-011: EPT Split-View for Shadow Hooks

## Status

Accepted

## Date

2024-11-20

## Context

Shadow hooks allow intercepting code execution while evading integrity checks. Anti-cheat systems commonly:

1. **Read code pages** to verify integrity (hash comparison)
2. **Execute code** normally, expecting original behavior

Problem: Traditional hooks modify code in place, which fails both checks.

Detection methods used:
- CRC/hash validation of code pages
- Memory scanning for known hook patterns (JMP, CALL gadgets)
- Exception-based detection (invalid instruction at hook site)

Options considered:
- **Inline hooks**: Replace first bytes with JMP - detected by integrity checks
- **Hardware breakpoints**: Limited slots (4), DR register monitoring
- **EPT hooks**: Separate execute and read views - more complex but undetectable

## Decision

Implement **EPT split-view hooks** using Intel EPT (Extended Page Tables):

1. Maintain two physical pages for each hooked virtual page:
   - **Execute page**: Contains hook code (trampoline to handler)
   - **Read page**: Contains original code (passes integrity checks)

2. Configure EPT permissions:
   - Execute page: EPT_EXECUTE only (no read/write)
   - Read page: EPT_READ | EPT_WRITE (no execute)

3. Handle EPT violations:
   - Read/write access → Map to read page
   - Execute access → Map to execute page

## Consequences

### Positive

- **Invisible to memory scans**: Reading code shows original bytes
- **Invisible to execution**: Hook triggers only on execute
- **CPU-enforced**: EPT is hardware-level, cannot be bypassed from guest
- **Flexible targeting**: Can hook any code page

### Negative

- **Requires execute-only EPT support**: Need CPU feature (most modern CPUs)
- **Complex page management**: Must track dual mappings per hook
- **TLB considerations**: INVEPT required on mapping changes
- **Performance overhead**: EPT violations add latency on first access

### Neutral

- **Memory overhead**: 2x physical pages for each hooked page (8KB per 4KB hook)

## Implementation Notes

### Hook Installation

```c
STATUS HookInstall(GVA target_va, void* handler) {
    // 1. Allocate execute page
    HPA exec_page = AllocatePhysicalPage();

    // 2. Copy original code
    memcpy(exec_page, original_page, PAGE_SIZE);

    // 3. Patch execute page with trampoline
    PatchTrampoline(exec_page, target_offset, handler);

    // 4. Configure EPT split-view
    EptSetSplitView(target_va, exec_page, original_page);

    // 5. Invalidate TLB
    InveptSingleContext(eptp);

    return OMBRA_SUCCESS;
}
```

### EPT Configuration

```c
void EptSetSplitView(GVA va, HPA exec_hpa, HPA read_hpa) {
    EPT_PTE* pte = EptGetPte(va);

    // Store both HPAs in extended tracking structure
    SPLIT_VIEW* view = AllocateSplitView();
    view->ExecuteHpa = exec_hpa;
    view->ReadHpa = read_hpa;
    view->OriginalPte = *pte;

    // Set execute-only initially
    pte->PageFrameNumber = exec_hpa >> 12;
    pte->ReadAccess = 0;
    pte->WriteAccess = 0;
    pte->ExecuteAccess = 1;  // Execute-only
}
```

### EPT Violation Handler

```c
void HandleEptViolation(GUEST_CONTEXT* ctx) {
    U64 qualification = VmRead(VMCS_EXIT_QUALIFICATION);
    GPA fault_gpa = VmRead(VMCS_GUEST_PHYSICAL_ADDRESS);

    SPLIT_VIEW* view = FindSplitView(fault_gpa);
    if (!view) {
        // Normal violation, handle as usual
        return;
    }

    if (qualification & EPT_VIOLATION_READ) {
        // Read access - switch to read page
        EptSetReadPage(fault_gpa, view->ReadHpa);
    } else if (qualification & EPT_VIOLATION_EXECUTE) {
        // Execute access - switch to execute page
        EptSetExecutePage(fault_gpa, view->ExecuteHpa);
    }

    InveptSingleContext(eptp);
}
```

## CPU Requirements

- **Execute-only pages**: IA32_VMX_EPT_VPID_CAP bit 0
- Supported on most CPUs since Haswell (2013)
- Fallback: Use MTF (Monitor Trap Flag) single-stepping (slower)

## Related

- [ADR-003](./ADR-003-shared-ept.md) - Shared EPT design
- [ADR-010](./ADR-010-timing-compensation.md) - Timing for EPT violations
- `hypervisor/hypervisor/hooks.c` - Hook implementation
- `hypervisor/hypervisor/handlers/ept_violation.c` - Violation handler
