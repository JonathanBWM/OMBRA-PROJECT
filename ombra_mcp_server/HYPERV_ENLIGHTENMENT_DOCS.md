# Hyper-V Enlightenment Documentation

## Overview

This documentation was added to prevent a critical mistake: zeroing Hyper-V CPUID enlightenment leaves, which causes Windows VMs to experience severe performance degradation or crashes.

## Files Added

### 1. `/src/ombra_mcp/data/hyperv_enlightenments.json`

Complete reference database containing:

- **11 CPUID Leaves (0x40000000 - 0x4000000A)**
  - Full bit-field documentation
  - Safety ratings (CRITICAL/HIGH/MEDIUM/LOW)
  - Modification warnings
  - Recommended evasion strategies

- **22 Hyper-V MSRs**
  - Address and access type
  - Criticality ratings
  - Usage descriptions

- **Safe Evasion Strategy**
  - How to hide hypervisor presence without breaking Windows
  - Nested execution detection

- **Windows Symptoms Reference**
  - What happens when you break enlightenments
  - BSODs, timing issues, performance problems

- **Detection Risk Analysis**
  - What anti-cheat can detect
  - Mitigation strategies

## Functions Added to `stealth.py`

### `get_hyperv_enlightenment_info() -> dict`

Loads and returns the complete Hyper-V enlightenment database.

**Usage:**
```python
from ombra_mcp.tools.stealth import get_hyperv_enlightenment_info

data = get_hyperv_enlightenment_info()
print(f"Overview: {data['overview']}")
print(f"CPUID leaves: {len(data['cpuid_leaves'])}")
print(f"MSRs: {len(data['hyper_v_msrs'])}")
```

**Returns:**
```python
{
    "overview": "Windows Hyper-V enlightenments are...",
    "cpuid_leaves": {...},
    "hyper_v_msrs": {...},
    "safe_evasion_strategy": {...},
    "dangerous_modifications": [...],
    "windows_symptoms": {...},
    "detection_risks": {...},
    "implementation_notes": {...}
}
```

### `check_cpuid_safety(leaf: int) -> dict`

Analyzes a CPUID leaf for modification safety.

**Usage:**
```python
from ombra_mcp.tools.stealth import check_cpuid_safety

# Check if safe to modify Hyper-V interface leaf
result = check_cpuid_safety(0x40000001)

print(f"Leaf: {result['leaf']}")
print(f"Name: {result['name']}")
print(f"Safe to modify: {result['safe_to_modify']}")  # Full description
print(f"Safe code: {result['safe_to_modify_code']}")  # Just keyword
print(f"Risk level: {result['risk_level']}")
print(f"Action: {result['recommended_action']}")
```

**Returns:**
```python
{
    "leaf": "0x40000001",
    "name": "Hypervisor Interface Identification",
    "safe_to_modify": "NO - Critical for Windows compatibility",
    "safe_to_modify_code": "NO",
    "risk_level": "CRITICAL",
    "description": "Interface signature - 'Hv#1' for Hyper-V compatible",
    "warnings": ["DO NOT MODIFY - Windows uses this to enable enlightenments"],
    "recommended_action": "Pass through unchanged - DO NOT MODIFY",
    "is_hyperv_leaf": True
}
```

**Risk Levels:**
- `CRITICAL` - Will cause crashes/BSOD if modified
- `HIGH` - May cause crashes or severe issues
- `MEDIUM` - Performance degradation but functional
- `LOW` - Safe to modify

**Safety Codes:**
- `NO` - Do not modify (maps to CRITICAL risk)
- `PARTIAL` - Some fields can be modified (maps to MEDIUM risk)
- `RISKY` - Modify only if necessary (maps to HIGH risk)
- `YES` - Safe to modify (maps to LOW risk)
- `UNKNOWN` - Unknown/undocumented

## Key Findings

### Critical Leaves (NEVER MODIFY):

| Leaf | Name | Why Critical |
|------|------|--------------|
| 0x40000001 | Hypervisor Interface | Windows checks 'Hv#1' signature to enable enlightenments |
| 0x40000002 | System Identity | Version compatibility checking |
| 0x40000003 | Feature Identification | Enables SynIC, timers, hypercalls - crashes if disabled |
| 0x40000005 | Hypervisor Limits | Resource allocation - can cause exhaustion/crashes |

### Partial Safe (0x40000000):

**Hypervisor Vendor ID**
- `EAX`: Max leaf value - **MUST PRESERVE** (Windows won't query other leaves if zero)
- `EBX/ECX/EDX`: Vendor string - **CAN ZERO** for stealth

### Critical MSRs:

| MSR | Address | Purpose |
|-----|---------|---------|
| HV_X64_MSR_GUEST_OS_ID | 0x40000000 | Windows writes this on boot |
| HV_X64_MSR_HYPERCALL | 0x40000001 | Hypercall page - TLB flush, etc. |
| HV_X64_MSR_REFERENCE_TSC | 0x40000021 | QueryPerformanceCounter source |
| HV_X64_MSR_TSC_FREQUENCY | 0x40000022 | TSC calibration |

## Safe Evasion Strategy

### If NOT running nested under Hyper-V:

```c
case 0x40000000:
    __cpuidex(regs, leaf, subleaf);
    // Zero vendor string only
    regs[1] = 0;  // EBX
    regs[2] = 0;  // ECX
    regs[3] = 0;  // EDX
    // KEEP regs[0] (EAX) - max leaf value
    break;

case 0x40000001 ... 0x40000005:
    // PASS THROUGH - execute real CPUID
    __cpuidex(regs, leaf, subleaf);
    break;
```

### If running nested under Hyper-V:

```c
// Detect nested execution
BOOLEAN IsNestedUnderHyperV(void) {
    int regs[4];
    __cpuid(regs, 0x40000000);
    return (regs[1] == 0x7263694D &&  // "Micr"
            regs[2] == 0x666F736F &&  // "osof"
            regs[3] == 0x76482074);   // "t Hv"
}

// Pass through ALL enlightenments when nested
if (IsNestedUnderHyperV()) {
    __cpuidex(regs, leaf, subleaf);
    // Don't modify anything
}
```

## Windows Symptoms if Broken

### Missing Enlightenments (0x40000003 zeroed):
- Extreme slowness (10-100x slower)
- High CPU usage when idle
- BSOD: `HYPERVISOR_ERROR (0x00020001)`

### Broken Timing (0x40000021/0x40000022):
- `QueryPerformanceCounter()` returns incorrect values
- `Sleep()` functions broken
- Multimedia timers fail
- Game/application stuttering

### Broken Hypercalls (0x40000001):
- TLB flush takes milliseconds instead of microseconds
- Context switch overhead 100x normal
- BSOD under high load

## Testing

Two test scripts are provided:

### `test_hyperv_stealth.py`
Unit tests for the new functions:
```bash
python3 test_hyperv_stealth.py
```

Tests:
1. Loading enlightenment data
2. CPUID safety checker
3. Critical leaf detection
4. Partial safe leaf handling

### `demo_hyperv_usage.py`
Real-world usage demonstration:
```bash
python3 demo_hyperv_usage.py
```

Demonstrates:
1. CPUID handler design process
2. Generated handler code
3. MSR virtualization requirements
4. Risk summary

## Integration with Hypervisor Code

When implementing CPUID exit handler in `/Users/jonathanmcclintock/PROJECT-OMBRA/hypervisor/hypervisor/handlers/cpuid.c`:

1. Import safety checker logic
2. Check each leaf before modification
3. Follow recommended actions
4. Detect nested execution
5. Pass through enlightenments when nested

## References

- **Microsoft Hyper-V TLFS**: https://learn.microsoft.com/en-us/virtualization/hyper-v-on-windows/tlfs/tlfs
  - Chapter 2: Hypervisor CPUID Interface
  - Chapter 3: Hypercall Interface
  - Chapter 4: Virtual Processor Registers

- **Data File**: `/src/ombra_mcp/data/hyperv_enlightenments.json`
- **Code**: `/src/ombra_mcp/tools/stealth.py`

## Quick Reference

**What to zero for stealth:**
- CPUID 1 ECX[31] (hypervisor present bit)
- CPUID 0x40000000 EBX/ECX/EDX (vendor string)

**What to NEVER touch:**
- CPUID 0x40000000 EAX (max leaf)
- CPUID 0x40000001-0x40000005 (all fields)
- HV_X64_MSR_* addresses 0x40000000-0x40000023

**Minimal viable Windows support:**
- Virtualize HV_X64_MSR_GUEST_OS_ID (storage only)
- Virtualize HV_X64_MSR_HYPERCALL (storage only)
- Virtualize HV_X64_MSR_VP_INDEX (return CPU index)
- Virtualize HV_X64_MSR_TIME_REF_COUNT (return TSC / 10)
- Virtualize HV_X64_MSR_REFERENCE_TSC (point to TSC reference page)
- Virtualize HV_X64_MSR_TSC_FREQUENCY (return real TSC freq)
- Virtualize HV_X64_MSR_APIC_FREQUENCY (return APIC freq)

---

**Last Updated**: 2025-12-26
**Author**: ENI (OmbraMCP Enhancement)
