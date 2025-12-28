# ADR-001: Pure C for Hypervisor Core

## Status

Accepted

## Date

2024-10-15

## Context

The hypervisor core runs at VMX root level (Ring -1), intercepting all guest operations. This environment has unique constraints:

1. **No runtime dependencies**: Standard C++ runtime (MSVCRT) cannot be used in kernel-mode without significant issues
2. **Predictable memory layout**: C++ features like vtables, exceptions, and RTTI add unpredictable memory structures
3. **Maximum control**: VMX operations require precise control over register state, memory, and timing
4. **Minimal code size**: Smaller code = smaller attack surface and faster loading

Options considered:
- **C++**: Rich abstractions but runtime complexity
- **Pure C**: Manual but predictable
- **Rust**: Memory safety but new toolchain complexity
- **Assembly only**: Maximum control but unmaintainable

## Decision

Use **Pure C** for the hypervisor core with x64 MASM for intrinsic operations.

Specific constraints:
- No C++ in `hypervisor/hypervisor/` directory
- MASM assembly in `hypervisor/hypervisor/asm/` for VMX intrinsics
- Custom memory allocation (no malloc/free)
- No floating-point operations in hypervisor context

## Consequences

### Positive

- **Predictable behavior**: No hidden constructors, destructors, or exceptions
- **Minimal dependencies**: Only Windows kernel APIs used
- **Easy debugging**: Direct correlation between source and binary
- **Small footprint**: hypervisor.lib is compact (~50KB)
- **Portable patterns**: C code can be audited against Intel SDM pseudocode

### Negative

- **Manual memory management**: All allocations tracked manually
- **No RAII**: Resources must be explicitly cleaned up
- **Verbose code**: More boilerplate than C++ equivalent
- **No templates**: Common patterns must be duplicated

### Neutral

- **Team familiarity**: Requires C expertise, but avoids C++ complexity debates

## Implementation Notes

Key patterns used instead of C++ features:

```c
// Instead of constructors: explicit init functions
STATUS VmxCpuInitialize(VMX_CPU* cpu);
void VmxCpuDestroy(VMX_CPU* cpu);

// Instead of classes: structs with function pointers
typedef struct _EXIT_HANDLER {
    U32 Reason;
    PEXIT_HANDLER_FN Handler;
} EXIT_HANDLER;

// Instead of exceptions: status codes
typedef enum _OMBRA_STATUS {
    OMBRA_SUCCESS = 0,
    OMBRA_ERROR_INVALID_PARAM = 1,
    OMBRA_ERROR_VMX_FAILED = 2,
    // ...
} OMBRA_STATUS;
```

## Related

- [ADR-002](./ADR-002-per-cpu-structures.md) - Per-CPU structure design
- `hypervisor/hypervisor/vmx.c` - Main VMX implementation
- `hypervisor/hypervisor/asm/intrinsics.asm` - VMX assembly intrinsics
