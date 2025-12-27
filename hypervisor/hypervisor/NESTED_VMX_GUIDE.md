# Nested VMX Implementation Guide

## Overview

The OmbraHypervisor now supports nested virtualization for the ZeroHVCI (Hyper-V hijacking) scenario. This infrastructure enables the hypervisor to:

1. **Detect** if running under another hypervisor (L0)
2. **Coexist** with Hyper-V without detection
3. **Handle** L1/L2 exit scenarios correctly
4. **Shadow** VMCS for nested guests (optional)

## Architecture

```
┌─────────────────────────────────────────┐
│  Windows Guest (L2)                     │
│  - Sees "bare metal"                    │
│  - CPUID hypervisor bit hidden          │
└─────────────────────────────────────────┘
            ↓ VM-Exits
┌─────────────────────────────────────────┐
│  OmbraHypervisor (L1)                  │
│  - Filters exits                        │
│  - Handles most exits locally           │
│  - Forwards some to L0                  │
└─────────────────────────────────────────┘
            ↓ Selective Forwarding
┌─────────────────────────────────────────┐
│  Hyper-V (L0)                           │
│  - Hijacked via ZeroHVCI                │
│  - Exit handler patched                 │
│  - Still manages physical hardware      │
└─────────────────────────────────────────┘
```

## Detection

### At Initialization

The hypervisor detects nested execution during `VmxInitializeCpu()`:

```c
// In vmx.c - automatically called during init
if (params->CpuId == 0) {
    NESTED_STATE nestedState;
    status = NestedDetectHypervisor(&nestedState);
    if (OMBRA_SUCCESS(status)) {
        if (nestedState.IsNested) {
            INFO("Running nested under %s", NestedGetHypervisorName(nestedState.L0Type));
            if (nestedState.IsHyperV) {
                INFO("Hyper-V coexistence mode enabled (ZeroHVCI scenario)");
            }
        }
    }
}
```

### Detection Methods

**CPUID.1.ECX[31]** - Hypervisor Present Bit
- Set to 1 if running under a hypervisor
- Checked first in `NestedDetectHypervisor()`

**CPUID.0x40000000** - Hypervisor Vendor ID
- EBX:ECX:EDX contains 12-character vendor string
- "Microsoft Hv" = Hyper-V
- "VMwareVMware" = VMware
- "KVMKVMKVM" = KVM

**Hyper-V Specifics (CPUID.0x40000001-0x40000006)**
- Interface signature
- Version information
- Feature identification
- Hypercall MSR availability

## Key Data Structures

### NESTED_STATE

```c
typedef struct _NESTED_STATE {
    // Detection results
    bool            IsNested;           // Running under L0
    HYPERVISOR_TYPE L0Type;             // Type of L0
    char            L0Vendor[13];       // Vendor string

    // Hyper-V specific
    bool            IsHyperV;
    U64             HyperVInterface;
    U64             HyperVVersion;
    U64             HyperVFeatures;
    U64             HypercallPage;      // Hypercall page PA

    // VMCS shadowing
    bool            VmcsShadowingAvailable;
    void*           VmreadBitmap;       // 4KB bitmap
    U64             VmreadBitmapPhys;
    void*           VmwriteBitmap;      // 4KB bitmap
    U64             VmwriteBitmapPhys;

    // Shadow VMCS
    void*           ShadowVmcs;         // Shadow VMCS region
    U64             ShadowVmcsPhys;
    bool            ShadowActive;

    // Statistics
    U64             L0ExitCount;        // Exits to L0
    U64             L1ExitCount;        // Exits handled at L1
} NESTED_STATE;
```

### VMX_CPU Extension

```c
typedef struct _VMX_CPU {
    // ... existing fields ...

    // Nested virtualization state (shared, pointer to global)
    struct _NESTED_STATE* Nested;

    // ... rest of fields ...
} VMX_CPU;
```

## Exit Handling in Nested Scenario

### Determining Exit Ownership

```c
bool NestedShouldForwardToL0(U32 exitReason, U64 qualification) {
    // If not nested, we handle everything
    if (!g_NestedState.IsNested) {
        return false;
    }

    switch (exitReason) {
        // VMX instructions - forward to L0
        case 18:  // VMCALL
        case 19:  // VMCLEAR
        case 20:  // VMLAUNCH
        case 21:  // VMPTRLD
        case 22:  // VMPTRST
        case 23:  // VMREAD
        case 24:  // VMRESUME
        case 25:  // VMWRITE
        case 26:  // VMXOFF
        case 27:  // VMXON
            return true;

        // Everything else - handle at L1
        default:
            return false;
    }
}
```

### In Exit Handler

```c
void VmexitHandler(void) {
    VMX_CPU* cpu = VmxGetCurrentCpu();
    U32 exitReason = (U32)VmcsRead(VMCS_EXIT_REASON) & 0xFFFF;
    U64 qualification = VmcsRead(VMCS_EXIT_QUALIFICATION);

    // Check if we should forward to L0
    if (NestedShouldForwardToL0(exitReason, qualification)) {
        NestedHandleVmxInstruction(cpu, exitReason);
        // This advances RIP and returns to guest
        // OR triggers a real exit to L0 (implementation dependent)
        return;
    }

    // Handle exit at L1
    switch (exitReason) {
        case 10:  // CPUID
            HandleCpuid(cpu);
            break;
        case 30:  // RDTSC
            HandleRdtsc(cpu);
            break;
        // ... other handlers ...
    }
}
```

## VMCS Shadowing

### When to Use

VMCS shadowing is useful when:
- Running nested guests (L2)
- L2 needs to read/write VMCS fields
- Want to avoid exits on VMREAD/VMWRITE

### Setup

```c
OMBRA_STATUS SetupVmcsShadowing(VMX_CPU* cpu, NESTED_STATE* state) {
    // Check hardware support
    if (!state->VmcsShadowingAvailable) {
        return OMBRA_ERROR_NOT_SUPPORTED;
    }

    // Initialize bitmaps
    NestedInitVmcsShadowing(cpu, state);

    // Create shadow VMCS
    NestedCreateShadowVmcs(state);

    // Enable shadowing
    NestedEnableVmcsShadowing(cpu);

    return OMBRA_SUCCESS;
}
```

### Bitmap Configuration

Bitmaps control which fields cause exits:
- **Bit 0** = cause exit on VMREAD/VMWRITE
- **Bit 1** = use shadow (no exit)

```c
// Example: Force exit on GUEST_RIP reads/writes
U32 guestRipEncoding = VMCS_GUEST_RIP;  // 0x681E
U32 byteOffset = guestRipEncoding / 8;
U32 bitOffset = guestRipEncoding % 8;

state->VmreadBitmap[byteOffset] |= (1 << bitOffset);   // Exit on VMREAD
state->VmwriteBitmap[byteOffset] |= (1 << bitOffset);  // Exit on VMWRITE
```

## Hyper-V Coexistence (ZeroHVCI)

### Scenario

In ZeroHVCI:
1. Hyper-V (L0) is already running
2. We hijack its VMExit handler at runtime
3. Our code executes in Hyper-V's exit path
4. We must maintain stealth

### Requirements

**Don't interfere with L0:**
- Let Hyper-V handle external interrupts
- Don't modify L0's VMCS directly
- Maintain timing expectations

**Hide from L2:**
- Filter CPUID to hide hypervisor bit
- Intercept VMX capability MSRs
- Compensate TSC timing

**Maintain L0 stealth:**
- Don't allocate suspicious memory
- Don't register obvious callbacks
- Keep exit overhead minimal

### API Usage

```c
// After detection shows Hyper-V
if (NestedIsHyperV()) {
    // Setup coexistence mode
    NestedSetupHyperVCoexistence(&nestedState);

    // Optionally hook hypercalls
    NestedHookHyperVHypercall(&nestedState);

    // Get feature bits
    U64 features = NestedGetHyperVFeatures();
    INFO("Hyper-V features: 0x%llX", features);
}
```

## Query Functions

### Runtime Queries

```c
// Is nested?
if (NestedIsRunningNested()) {
    // Adjust behavior for nested scenario
}

// Get L0 type
HYPERVISOR_TYPE l0 = NestedGetL0Type();
switch (l0) {
    case HV_TYPE_HYPERV:
        // Hyper-V specific adjustments
        break;
    case HV_TYPE_VMWARE:
        // VMware specific adjustments
        break;
    // ...
}

// Hyper-V specific
if (NestedIsHyperV()) {
    U64 features = NestedGetHyperVFeatures();
    // Check specific feature bits
}
```

### Vendor String

```c
const char* NestedGetHypervisorName(HYPERVISOR_TYPE type);

// Example
const char* name = NestedGetHypervisorName(HV_TYPE_HYPERV);
// Returns: "Microsoft Hyper-V"
```

## Integration with Exit Handlers

### CPUID Handler

```c
void HandleCpuid(VMX_CPU* cpu) {
    U32 leaf = (U32)VmcsRead(VMCS_GUEST_RAX);
    U32 subleaf = (U32)VmcsRead(VMCS_GUEST_RCX);

    // If nested, we need to filter some leaves
    if (NestedIsRunningNested()) {
        // Leaf 1 - Hide hypervisor bit from L2
        if (leaf == 1) {
            U32 ecx = /* ... get actual value ... */;
            ecx &= ~CPUID_FEAT_ECX_HV;  // Clear bit 31
            VmcsWrite(VMCS_GUEST_RCX, ecx);
            // Advance RIP and return
            return;
        }

        // Leaf 0x40000000 - Don't expose hypervisor vendor
        if (leaf == 0x40000000) {
            // Return zeros or fake vendor
            VmcsWrite(VMCS_GUEST_RAX, 0);
            VmcsWrite(VMCS_GUEST_RBX, 0);
            VmcsWrite(VMCS_GUEST_RCX, 0);
            VmcsWrite(VMCS_GUEST_RDX, 0);
            return;
        }
    }

    // Normal CPUID handling
    // ...
}
```

### MSR Handler

```c
void HandleRdmsr(VMX_CPU* cpu) {
    U32 msr = (U32)VmcsRead(VMCS_GUEST_RCX);

    // VMX capability MSRs - hide from guest
    if (msr >= 0x480 && msr <= 0x491) {
        if (NestedIsRunningNested()) {
            // Return 0 or fake values
            VmcsWrite(VMCS_GUEST_RAX, 0);
            VmcsWrite(VMCS_GUEST_RDX, 0);
            // Advance RIP
            return;
        }
    }

    // Hyper-V MSRs
    if (msr >= 0x40000000 && msr <= 0x40000010) {
        if (NestedIsHyperV()) {
            // Either pass through or synthesize
            // Depends on whether we want to expose Hyper-V to L2
        }
    }

    // Normal MSR handling
    // ...
}
```

## Best Practices

### 1. Always Check Nested State

```c
// Don't assume bare metal
if (NestedIsRunningNested()) {
    // Adjust behavior
}
```

### 2. Forward Carefully

Only forward exits that L0 truly must handle:
- VMX instructions (if L2 tries to use VMX)
- Hardware interrupts (if L0 manages APIC)
- Some exceptions (depends on L0's expectations)

### 3. Maintain Timing

```c
// Before forwarding to L0
U64 tscBefore = __rdtsc();

// Let L0 handle exit
// ...

// After return from L0
U64 tscAfter = __rdtsc();
U64 overhead = tscAfter - tscBefore;

// Compensate in TSC offset
cpu->TscOffset -= overhead;
VmcsWrite(VMCS_CTRL_TSC_OFFSET, cpu->TscOffset);
```

### 4. Log Nested Events

```c
if (NestedIsRunningNested()) {
    TRACE("Nested exit: reason=%u, L0=%s",
          exitReason,
          NestedGetHypervisorName(NestedGetL0Type()));
}
```

## Limitations

### Current Implementation

- VMCS shadowing setup but not fully integrated
- Shadow VMCS sync is stubbed
- VMX instruction emulation is minimal (just advances RIP)
- No full L2 guest support (yet)

### ZeroHVCI Specific

- Assumes Hyper-V is already running
- Assumes exit handler is already hijacked by loader
- No runtime patch/unpatch of Hyper-V
- No Hyper-V hypercall emulation (just detection)

## Future Enhancements

1. **Full L2 Support**
   - Complete VMCS shadowing
   - VMX instruction emulation
   - Nested EPT (EPT within EPT)

2. **Hyper-V Hypercall Handling**
   - Intercept hypercalls from L2
   - Emulate or forward to L0
   - Maintain Hyper-V enlightenments

3. **AMD SVM Support**
   - Detect AMD virtualization
   - Handle VMCB instead of VMCS
   - Nested paging (NPT)

4. **Cross-Hypervisor**
   - Support VMware, KVM, Xen
   - Detect paravirt interfaces
   - Handle vendor-specific features

## Files

- **nested.h** - API and structures
- **nested.c** - Implementation
- **vmx.c** - Integration with VMX init
- **vmx.h** - VMX_CPU extension

## References

- Intel SDM Volume 3, Chapter 24 (VMCS)
- Intel SDM Volume 3, Section 24.10 (VMCS Shadowing)
- Intel SDM Volume 3, Chapter 25 (VMX Non-Root Operation)
- ZeroHVCI documentation in `ombra_mcp_server/src/ombra_mcp/tools/hvci_bypass.py`
