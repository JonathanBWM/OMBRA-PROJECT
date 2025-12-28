# vmx.c Implementation Details

## VERIFICATION STATUS
```
VERIFICATION:
- [x] I read the entire file
- [x] I verified all function signatures
- [x] I traced all code paths
- [x] I verified data structure usage
- [ ] I tested execution

UNVERIFIED CLAIMS:
- VMLAUNCH success behavior
- APIC ID mapping correctness
- Nested detection on non-Hyper-V platforms

ASSUMPTIONS:
- Intel VT-x CPU with VMX support
- Running at CPL=0 (kernel mode)
- Pre-allocated memory from loader
```

## DOCUMENTED FROM
```
File: hypervisor/hypervisor/vmx.c
Git hash: 73853be
Lines: 419
Date: 2025-12-27
```

---

## File Purpose

VMX lifecycle management: checking support, enabling VMX, per-CPU initialization, VMLAUNCH, and CPU identification.

---

## Global State

```c
OMBRA_STATE g_Ombra = {0};
```

Single global state structure holding all per-CPU VMX_CPU pointers.

---

## Function-by-Function Analysis

### VmxCheckSupport (lines 25-52)

**Purpose**: Verify CPU supports VMX and it's not locked out.

**Algorithm**:
1. Execute CPUID.1, check ECX bit 5 (VMX feature flag)
2. If ECX bit 31 set, log hypervisor present (don't fail - nested scenario)
3. Read IA32_FEATURE_CONTROL MSR
4. If locked AND VMX disabled → fail

**Return Values**:
- `OMBRA_SUCCESS` - VMX available
- `OMBRA_ERROR_NOT_SUPPORTED` - CPU doesn't support VMX
- `OMBRA_ERROR_VMX_DISABLED` - VMX locked out in BIOS

**VERIFIED**: Logic matches Intel SDM requirements for VMX entry.

---

### AdjustControls (lines 65-77)

**Purpose**: Adjust VMX control bits per capability MSR.

**Algorithm**:
```c
U32 AdjustControls(U32 requested, U32 msr) {
    U64 cap = __readmsr(msr);
    U32 allowed0 = (U32)cap;        // Bits that CAN be 0
    U32 allowed1 = (U32)(cap >> 32); // Bits that CAN be 1

    requested |= allowed0;  // Set bits that MUST be 1
    requested &= allowed1;  // Clear bits that MUST be 0

    return requested;
}
```

**Key Insight**: The capability MSR format is counter-intuitive:
- Low 32 bits = bits that are allowed to be 0 (so the inverse = bits that MUST be 1)
- High 32 bits = bits that are allowed to be 1

Actually per Intel SDM:
- allowed0 = bits that are reserved to 1 (must be set)
- allowed1 = bits that are reserved to 0 (must be clear, so we AND with this)

Wait, let me re-verify...

**CORRECTION**: After checking Intel SDM Vol 3C Appendix A.3.1:
- Bits 31:0 report the allowed 0-settings (if bit is clear, control MUST be 0)
- Bits 63:32 report the allowed 1-settings (if bit is set, control CAN be 1)

So the code correctly:
1. ORs with allowed0 to set any bits that MUST be 1
2. ANDs with allowed1 to clear any bits that MUST be 0

**VERIFIED**: Correct per Intel SDM.

---

### GetSegmentBase (lines 109-122)

**Purpose**: Extract segment base address from GDT entry.

**Algorithm**:
1. Get GDT entry via selector
2. Combine BaseLow (16 bits), BaseMiddle (8 bits), BaseHigh (8 bits)
3. For system segments (TSS, LDT), add 32-bit BaseUpper

**Selector Format** (bits):
- 15:3 = Index into GDT
- 2 = Table Indicator (0=GDT, 1=LDT)
- 1:0 = RPL (privilege level)

**VERIFIED**: Correctly handles 64-bit system segment extensions.

---

### GetSegmentAccessRights (lines 138-158)

**Purpose**: Build VMCS-format access rights from GDT descriptor.

**Algorithm**:
```c
U32 ar = desc->Access;                              // Bits 7:0
ar |= ((U32)(desc->LimitHighAndFlags & 0xF0) << 8); // Bits 15:12
ar &= 0xF0FF;                                        // Clear undefined bits
```

**Special Case**: Null selector (0) returns `SEG_ACCESS_UNUSABLE`.

**VMCS Access Rights Format**:
- Bits 3:0 = Segment type
- Bit 4 = S (descriptor type)
- Bits 6:5 = DPL
- Bit 7 = P (present)
- Bits 11:8 = Reserved
- Bit 12 = AVL (available)
- Bit 13 = L (64-bit code)
- Bit 14 = D/B (default operation size)
- Bit 15 = G (granularity)
- Bit 16 = Unusable

**VERIFIED**: Format matches Intel SDM Vol 3C Section 24.4.1.

---

### VmxEnable (lines 171-203)

**Purpose**: Enable VMX operation on current CPU.

**Sequence**:
1. Set CR4.VMXE (bit 13)
2. Adjust CR0 per VMX_CR0_FIXED0/FIXED1 MSRs
3. Adjust CR4 per VMX_CR4_FIXED0/FIXED1 MSRs
4. Write VMCS revision ID to VMXON region
5. Execute VMXON

**Error Handling**: If VMXON fails, restore CR4 and return error.

**VERIFIED**: Follows Intel SDM entry requirements.

---

### VmxInitializeCpu (lines 263-365)

**Purpose**: Complete per-CPU VMX initialization.

**Sequence**:
1. Get static storage for VMX_CPU (uses static array, NOT dynamic allocation)
2. Store APIC ID (not index) for correct CPU lookup
3. Copy memory pointers from params (VMXON, VMCS, host stack, MSR bitmap)
4. Initialize timing state (TscOffset, counters)
5. Capture IA32_TSC_AUX for MSR virtualization
6. Initialize timing compensation system
7. On CPU 0 only: detect nested hypervisor
8. Check VMX support
9. Enable VMX
10. Initialize VMCS
11. Launch VM

**Static Storage Pattern**:
```c
static VMX_CPU cpuStorage[MAX_CPUS];
cpu = &cpuStorage[params->CpuId];
```

**CONCERN**: Static array limits to MAX_CPUS processors. No dynamic allocation for >MAX_CPUS systems.

**VERIFIED**: Initialization sequence correct, but static storage is a limitation.

---

### GetCurrentApicId (lines 241-257)

**Purpose**: Get current processor's APIC ID correctly for x2APIC and legacy.

**Algorithm**:
```c
// Check CPUID.1:ECX bit 21 for x2APIC support
if (info[2] & (1 << 21)) {
    // x2APIC: Use CPUID leaf 0xB for full 32-bit ID
    __cpuidex(info, 0xB, 0);
    return (U32)info[3];  // EDX
} else {
    // Legacy: Use 8-bit ID from CPUID.1:EBX[31:24]
    return (info[1] >> 24) & 0xFF;
}
```

**Why This Matters**: x2APIC supports >256 CPUs with 32-bit IDs. Legacy APIC only 8 bits.

**VERIFIED**: Correct x2APIC detection per Intel SDM.

---

### VmxGetCurrentCpu (lines 400-418)

**Purpose**: Find VMX_CPU structure for current processor.

**Algorithm**:
1. Get APIC ID via GetCurrentApicId()
2. Linear search g_Ombra.Cpus[] for matching CpuId
3. Fallback: try direct index if APIC ID < MAX_CPUS

**Performance Note**: Linear search on every VM-exit is suboptimal. Could use per-CPU storage via FS/GS.

**CONCERN**: Returns NULL if no match found - callers should check!

---

### VmxLaunchCpu (lines 371-394)

**Purpose**: Execute VMLAUNCH to start virtualization.

**Algorithm**:
```c
U64 result = AsmVmxLaunch();  // Assembly captures state and launches
if (result != 0) {
    U64 errorCode = VmcsRead(VMCS_EXIT_INSTRUCTION_ERROR);
    return OMBRA_ERROR_VMLAUNCH_FAILED;
}
return OMBRA_SUCCESS;  // Now running as guest
```

**The Trick**: AsmVmxLaunch must:
1. Capture current RSP/RIP
2. Write them to VMCS guest state
3. Execute VMLAUNCH
4. On success, return 0 (as guest)
5. On failure, return non-zero (still host)

**VERIFIED**: Error handling reads VMCS_EXIT_INSTRUCTION_ERROR correctly.

---

## Data Flow

```
HV_PER_CPU_PARAMS (from loader)
        │
        v
VmxInitializeCpu()
        │
        ├── VMX_CPU structure initialized
        ├── g_Ombra.Cpus[i] = cpu
        │
        v
VmxEnable()
        │
        ├── CR4.VMXE set
        ├── CR0/CR4 adjusted
        ├── VMXON executed
        │
        v
VmcsInitialize()
        │
        ├── VMCS configured
        │
        v
VmxLaunchCpu()
        │
        ├── VMLAUNCH
        │
        v
Running as guest (VM-exits handled)
```

---

## MSR Usage

| MSR | Purpose |
|-----|---------|
| `MSR_IA32_FEATURE_CONTROL` | Check VMX lock/enable |
| `MSR_IA32_VMX_BASIC` | Get VMCS revision ID |
| `MSR_IA32_VMX_CR0_FIXED0/1` | CR0 requirements |
| `MSR_IA32_VMX_CR4_FIXED0/1` | CR4 requirements |
| `MSR_IA32_TSC_AUX` | Capture processor ID for RDTSCP |

---

## Intrinsics Used

| Intrinsic | Purpose |
|-----------|---------|
| `__cpuid` | Feature detection |
| `__cpuidex` | Extended topology (x2APIC) |
| `__readmsr` | Read MSRs |
| `__readcr0/4` | Read control registers |
| `__writecr0/4` | Write control registers |
| `__vmx_on` | Execute VMXON |
| `__vmx_off` | Execute VMXOFF |
| `__vmx_vmread` | Read VMCS field |
| `__vmx_vmwrite` | Write VMCS field |

---

## Error Codes

| Error | Meaning |
|-------|---------|
| `OMBRA_ERROR_NOT_SUPPORTED` | CPU lacks VMX |
| `OMBRA_ERROR_VMX_DISABLED` | VMX locked in BIOS |
| `OMBRA_ERROR_VMXON_FAILED` | VMXON instruction failed |
| `OMBRA_ERROR_VMLAUNCH_FAILED` | VMLAUNCH instruction failed |

---

## CONCERNS

1. **Static CPU Storage**: Fixed MAX_CPUS array limits scalability
2. **Linear CPU Lookup**: O(n) search on every VM-exit
3. **No NUMA Awareness**: All CPUs use same allocation pattern
4. **Error Code Unused**: VmxLaunchCpu reads but discards error code

---

## GAPS AND UNKNOWNS

- [ ] What is MAX_CPUS defined as?
- [ ] How does AsmVmxLaunch handle the state capture?
- [ ] Is there VMRESUME support (for subsequent entries)?
- [ ] How are CPU hotplug events handled?

---

*Implementation documentation generated 2025-12-27*
*CONFIDENCE: HIGH for logic, MEDIUM for runtime behavior*
