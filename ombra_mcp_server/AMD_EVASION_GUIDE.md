# AMD SVM Evasion Guide

Complete guide to using the AMD SVM evasion tools in the OmbraMCP server.

## Overview

This guide covers AMD-specific anti-cheat evasion for hypervisors using Secure Virtual Machine (SVM) technology. AMD SVM is functionally equivalent to Intel VMX but has different detection vectors.

## Quick Start

```python
from ombra_mcp.tools.stealth import (
    get_amd_evasion_info,
    generate_amd_cpuid_spoofing,
    generate_amd_msr_virtualization
)

# Get all AMD evasion data
amd_data = get_amd_evasion_info("all")

# Get specific categories
cpuid_info = get_amd_evasion_info("cpuid")
msr_info = get_amd_evasion_info("msrs")
checklist = get_amd_evasion_info("checklist")
detection_info = get_amd_evasion_info("detection")
comparison = get_amd_evasion_info("intel_vs_amd")

# Generate code
cpuid_handler = generate_amd_cpuid_spoofing()
msr_handler = generate_amd_msr_virtualization()
```

## Critical Detection Vectors

### 1. CPUID.80000001h:ECX[2] - SVM Bit

**MOST COMMON DETECTION METHOD**

```c
// Detection code (what anti-cheat does):
int regs[4];
__cpuidex(regs, 0x80000001, 0);
if (regs[2] & (1 << 2)) {
    // SVM is supported - might be running in hypervisor
}
```

**Evasion**: Clear bit 2 in ECX when handling CPUID exit for leaf 0x80000001.

### 2. CPUID.8000000Ah - SVM Features

If this leaf returns valid data, SVM hardware is present. Options:

- **OPTION 1 (Safest)**: Return all zeros (EAX=EBX=ECX=EDX=0)
- **OPTION 2**: Pass through EAX/EBX but zero EDX (clears all feature bits)

### 3. EFER.SVME (MSR 0xC0000080, bit 12)

**CRITICAL MSR**

```c
// Detection code:
uint64_t efer = __readmsr(0xC0000080);
if (efer & (1ULL << 12)) {
    // SVM is enabled - hypervisor is active
}
```

**Evasion**: Virtualize EFER reads and clear bit 12.

### 4. VM_CR MSR (0xC0010114)

SVM control register. Non-zero values indicate SVM state.

**Evasion Options**:
- Return 0
- Inject #GP(0) to simulate MSR doesn't exist

### 5. VM_HSAVE_PA MSR (0xC0010117)

Host save area physical address. Non-zero means VMRUN is configured.

**Evasion**: Return 0 in virtualized reads.

### 6. VMMCALL Instruction

AMD equivalent of Intel's VMCALL. Will cause #UD outside guest, #VMEXIT inside guest.

**Evasion**: Handle carefully - may need to inject #UD or emulate.

## Code Generation

### AMD CPUID Spoofing

```c
// Generated code handles:
// - Leaf 0x80000001: Clear SVM bit (ECX[2])
// - Leaf 0x8000000A: Zero all registers (hide SVM features)
// - Leaves 0x40000000-0x40000006: Zero (hide hypervisor presence)

BOOLEAN HandleAmdCpuidExit(PGUEST_CONTEXT Context) {
    int regs[4] = {0};
    int leaf = (int)Context->Rax;
    int subleaf = (int)Context->Rcx;

    __cpuidex(regs, leaf, subleaf);

    switch (leaf) {
    case 0x80000001:
        regs[2] &= ~(1 << 2);  // Clear SVM bit
        break;
    case 0x8000000A:
        regs[0] = regs[1] = regs[2] = regs[3] = 0;  // Hide SVM features
        break;
    // ... more cases
    }

    Context->Rax = regs[0];
    Context->Rbx = regs[1];
    Context->Rcx = regs[2];
    Context->Rdx = regs[3];

    AdvanceGuestRip();
    return TRUE;
}
```

### AMD MSR Virtualization

```c
// Generated code handles:
// - EFER (0xC0000080): Clear SVME bit
// - VM_CR (0xC0010114): Return 0
// - VM_HSAVE_PA (0xC0010117): Return 0
// - TSC_RATIO (0xC0010104): Inject #GP or handle if using TSC compensation
// - IA32_TSC_AUX (0xC0000103): Return shadow value for RDTSCP

BOOLEAN HandleAmdRdmsrExit(PGUEST_CONTEXT Context) {
    ULONG64 value = 0;
    ULONG msr = (ULONG)Context->Rcx;

    switch (msr) {
    case 0xC0000080:  // EFER
        value = __readmsr(msr);
        value &= ~(1ULL << 12);  // Clear SVME
        break;
    case 0xC0010114:  // VM_CR
        value = 0;
        break;
    case 0xC0010117:  // VM_HSAVE_PA
        value = 0;
        break;
    // ... more cases
    }

    Context->Rax = (ULONG)(value & 0xFFFFFFFF);
    Context->Rdx = (ULONG)(value >> 32);

    AdvanceGuestRip();
    return TRUE;
}
```

## Evasion Checklist

Complete this checklist for full AMD SVM stealth:

- [ ] Clear CPUID.80000001h:ECX[2] (SVM bit) when guest queries
- [ ] Clear EFER.SVME (bit 12, MSR 0xC0000080) in virtualized reads
- [ ] Return 0 for CPUID leaf 0x8000000Ah (SVM features) or don't modify EAX/EBX
- [ ] Virtualize VM_CR (0xC0010114) to return 0 or inject #GP(0)
- [ ] Virtualize VM_HSAVE_PA (0xC0010117) to return 0
- [ ] If using TSC_RATIO MSR, ensure compensation is precise
- [ ] Virtualize IA32_TSC_AUX (0xC0000103) for RDTSCP consistency
- [ ] Handle VMMCALL instruction appropriately
- [ ] Consider NPT violations have similar timing signatures to EPT
- [ ] Use AMD's hardware TSC_RATIO for better timing compensation

## Intel vs AMD Differences

### Instructions

| Operation | Intel | AMD | Notes |
|-----------|-------|-----|-------|
| Hypercall | VMCALL | VMMCALL | Different opcodes - anti-cheat may test both |
| VM Entry | VMLAUNCH/VMRESUME | VMRUN | AMD simpler - single instruction |
| VM Exit | Automatic | Automatic | Different exit reason formats |

### Structures

| Feature | Intel | AMD |
|---------|-------|-----|
| Control Structure | VMCS (opaque, VMREAD/VMWRITE) | VMCB (direct memory access) |
| Second-Level Paging | EPT | NPT/RVI |
| Control Complexity | More complex | Simpler |

### Timing Considerations

- **AMD SVM exits** are generally slightly faster than Intel VMX exits
- **NPT violations** have similar timing to EPT violations - same compensation strategy
- **TSC_RATIO MSR** (AMD) provides more precise timing compensation than Intel's TSC_OFFSET

## TSC Compensation on AMD

AMD provides hardware TSC scaling via the TSC_RATIO MSR (0xC0010104), which is superior to Intel's TSC_OFFSET:

```c
// TSC_RATIO format:
// - Bits 31:0:  TSCScale (numerator)
// - Bits 63:32: Reserved (must be 0)
//
// Guest TSC = (Host TSC * TSCScale) >> 32

// Example: Run guest at 80% of host TSC speed
uint64_t tsc_ratio = (uint64_t)(0x80000000ULL * 0.8);
__writemsr(0xC0010104, tsc_ratio);
```

**Advantages over Intel TSC_OFFSET**:
- Compensates for timing drift automatically
- Scales all TSC-based timing proportionally
- No accumulating offset errors

## Data File Reference

All AMD SVM data is stored in:
```
ombra_mcp_server/src/ombra_mcp/data/amd_cpuid.json
```

### Structure

```json
{
  "cpuid_leaves": {
    "0x80000001": { /* AMD Extended Features */ },
    "0x8000000A": { /* SVM Features */ },
    "0x80000008": { /* Address Sizes */ }
  },
  "svm_msrs": {
    "VM_CR": { /* 0xC0010114 */ },
    "VM_HSAVE_PA": { /* 0xC0010117 */ },
    "TSC_RATIO": { /* 0xC0010104 */ },
    "EFER": { /* 0xC0000080 */ },
    "IA32_TSC_AUX": { /* 0xC0000103 */ }
  },
  "evasion_checklist": [ /* 10 items */ ],
  "detection_specific": {
    "timing_considerations": { /* AMD-specific timing */ },
    "common_checks": [ /* Detection methods */ ]
  },
  "intel_vs_amd": {
    "instruction_differences": { /* VMCALL vs VMMCALL, etc */ },
    "structure_differences": { /* VMCS vs VMCB, EPT vs NPT */ }
  }
}
```

## Common Detection Methods

Anti-cheat may use these techniques to detect AMD SVM:

1. **CPUID.80000001h:ECX[2]** - Most common, checks SVM bit
2. **CPUID.8000000Ah** - If exists with valid data, SVM present
3. **RDMSR 0xC0010114 (VM_CR)** - Checking SVME state
4. **RDMSR 0xC0000080 (EFER)** - Checking SVME bit
5. **VMMCALL execution** - Will #UD or #VMEXIT
6. **Memory scanning** - Looking for 'VMCB' strings or SVM driver signatures

## Testing Your Implementation

```python
from ombra_mcp.tools.stealth import get_amd_evasion_info

# Verify your implementation covers all detection vectors
detection = get_amd_evasion_info("detection")
for check in detection["detection_specific"]["common_checks"]:
    print(f"Implemented: {check}")

# Ensure all checklist items are complete
checklist = get_amd_evasion_info("checklist")
for item in checklist["evasion_checklist"]:
    print(f"[ ] {item}")
```

## References

- **AMD64 Architecture Programmer's Manual Volume 2**: System Programming (Chapter 15: Secure Virtual Machine)
- **amd_cpuid.json**: Complete AMD SVM reference data
- **stealth.py**: AMD evasion tool implementations

## Support

For questions or issues with AMD SVM evasion:
1. Check `get_amd_evasion_info("detection")` for common detection methods
2. Review `get_amd_evasion_info("intel_vs_amd")` for architecture differences
3. Use `generate_amd_cpuid_spoofing()` and `generate_amd_msr_virtualization()` as starting points

---

**Created**: 2025-12-26  
**Data Source**: AMD64 Architecture Programmer's Manual  
**MCP Server**: OmbraHypervisor
