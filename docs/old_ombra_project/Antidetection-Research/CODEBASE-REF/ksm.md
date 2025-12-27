# KSM Stealth & Anti-Detection Pattern Extraction

**Source**: `Refs/codebases/ksm/`
**Focus**: Hypervisor stealth, anti-detection, CPUID masking, MSR filtering, TSC manipulation
**Relevance**: Critical for Ombra's anti-cheat evasion

---

## 1. STEALTH/HIDING MECHANISMS

### 1.1 VMX Feature Hiding via CPUID

**Source**: `exit.c:452-474`

```c
static bool vcpu_handle_cpuid(struct vcpu *vcpu)
{
    int cpuid[4];
    int func = ksm_read_reg32(vcpu, STACK_REG_AX);
    int subf = ksm_read_reg32(vcpu, STACK_REG_CX);
    __cpuidex(cpuid, func, subf);

#ifndef NESTED_VMX
    if (func == 1)
        cpuid[2] &= ~(1 << (X86_FEATURE_VMX & 31));  // MASK VMX BIT
#endif

    ksm_write_reg32(vcpu, STACK_REG_AX, cpuid[0]);
    ksm_write_reg32(vcpu, STACK_REG_BX, cpuid[1]);
    ksm_write_reg32(vcpu, STACK_REG_CX, cpuid[2]);  // Modified ECX
    ksm_write_reg32(vcpu, STACK_REG_DX, cpuid[3]);
    vcpu_advance_rip(vcpu);
    return true;
}
```

**Analysis**:
- Intercepts CPUID leaf 1 (basic processor info)
- **Clears bit 5 in ECX (X86_FEATURE_VMX)** to hide VMX capability
- This prevents detection via `CPUID.01H:ECX.VMX[bit 5]`
- Pattern: Execute real CPUID, modify result, return to guest

**Ombra Application**:
- Ombra MUST intercept CPUID leaf 1 and clear VMX bit
- Also mask HYPERVISOR bit: `cpuid[2] &= ~(1 << (X86_FEATURE_HYPERVISOR & 31))`
- Consider hiding other hypervisor leaves: 0x40000000-0x400000FF

---

### 1.2 MSR Filtering - Hypervisor MSR Access Control

**Source**: `ksm.c:74-87`

```c
static inline void init_msr_bitmap(struct ksm *k)
{
    // Read bitmap (causes VM-exit on RDMSR)
    unsigned long *read_lo = (unsigned long *)k->msr_bitmap;
    set_bit(MSR_IA32_FEATURE_CONTROL, read_lo);

#ifdef NESTED_VMX
    // Intercept ALL VMX capability MSRs
    for (u32 msr = MSR_IA32_VMX_BASIC; msr <= MSR_IA32_VMX_VMFUNC; ++msr)
        set_bit(msr, read_lo);
#endif

#ifdef NESTED_VMX
    // Write bitmap (causes VM-exit on WRMSR)
    unsigned long *write_lo = (unsigned long *)((char *)k->msr_bitmap + 2048);
    set_bit(MSR_IA32_FEATURE_CONTROL, write_lo);
#endif
}
```

**MSR Bitmap Structure**:
- Byte 0-1023: RDMSR bitmap (MSRs 0x00000000-0x00001FFF)
- Byte 1024-2047: RDMSR bitmap (MSRs 0xC0000000-0xC0001FFF)
- Byte 2048-3071: WRMSR bitmap (MSRs 0x00000000-0x00001FFF)
- Byte 3072-4095: WRMSR bitmap (MSRs 0xC0000000-0xC0001FFF)

**MSRs Intercepted**:
- `MSR_IA32_FEATURE_CONTROL` (0x3A)
- `MSR_IA32_VMX_BASIC` through `MSR_IA32_VMX_VMFUNC` (0x480-0x491)

**Source**: `exit.c:1906-1961` (RDMSR handler)

```c
static bool vcpu_handle_rdmsr(struct vcpu *vcpu)
{
    u32 msr = ksm_read_reg32(vcpu, STACK_REG_CX);
    u64 val = 0;

    switch (msr) {
    case MSR_IA32_FEATURE_CONTROL:
#ifdef NESTED_VMX
        val = vcpu->nested_vcpu.feat_ctl;  // Synthetic value
#else
        // HIDE VMXON BITS
        val = __readmsr(msr) & ~(FEATURE_CONTROL_VMXON_ENABLED_OUTSIDE_SMX |
                                 FEATURE_CONTROL_VMXON_ENABLED_INSIDE_SMX);
#endif
        break;
    default:
        // Intercept VMX capability MSRs (0x480-0x491)
        if (msr >= MSR_IA32_VMX_BASIC && msr <= MSR_IA32_VMX_VMFUNC) {
#ifndef NESTED_VMX
            vcpu_inject_hardirq(vcpu, X86_TRAP_GP, 0);  // Inject #GP
#else
            val = __readmsr(msr);
            // FILTER CAPABILITIES
            switch (msr) {
            case MSR_IA32_VMX_PROCBASED_CTLS:
            case MSR_IA32_VMX_TRUE_PROCBASED_CTLS:
                val &= ~((u64)nested_unsupported_primary << 32);
                break;
            case MSR_IA32_VMX_PROCBASED_CTLS2:
                val &= ~((u64)nested_unsupported_secondary << 32);
                break;
            }
#endif
        }
        break;
    }

    ksm_write_reg32(vcpu, STACK_REG_AX, (u32)val);
    ksm_write_reg32(vcpu, STACK_REG_DX, val >> 32);
    vcpu_advance_rip(vcpu);
    return true;
}
```

**Analysis**:
- When non-nested: Inject #GP on VMX MSR reads (appear unsupported)
- When non-nested: Clear VMXON enable bits from IA32_FEATURE_CONTROL
- This makes VMX appear disabled even if CPU supports it

**Ombra Application**:
- Set MSR bitmap to intercept 0x480-0x491 (VMX MSRs)
- Inject #GP(0) on reads to fake "VMX not supported"
- Also intercept MSR_IA32_FEATURE_CONTROL and clear bits 1-2

---

## 2. ANTI-DETECTION PATTERNS

### 2.1 Hypervisor Bit Masking

**Source**: `x86.h:288`

```c
#define X86_FEATURE_HYPERVISOR  ( 4*32+31) /* Running on a hypervisor */
```

**CPUID Leaf 1 ECX[31]**: Set when guest runs under hypervisor

**Pattern** (inferred from VMX bit hiding):
```c
if (func == 1)
    cpuid[2] &= ~(1 << 31);  // Clear HYPERVISOR bit
```

**Ombra Application**:
- MUST clear bit 31 in CPUID.01H:ECX
- Also hide hypervisor leaves 0x40000000-0x400000FF entirely
- Return zeros or inject #GP for 0x40000000 queries

---

### 2.2 VMX Instruction Emulation (Fake #UD)

**Source**: `exit.c:1361`, `exit.c:1570` (nested VMX checks)

```c
// VMXON region validation
bool match = *(u32 *)nested->vmcs == (u32)__readmsr(MSR_IA32_VMX_BASIC);
```

**Pattern**: When non-nested, VMX instructions cause #UD (invalid opcode)
- KSM injects #GP instead on MSR reads
- Implies VMX instructions should remain disabled (cause #UD naturally)

**Ombra Application**:
- Do NOT enable VMXON/VMXOFF/VMPTRLD in secondary controls
- Let CPU inject #UD naturally on VMX instructions
- This makes VMX appear genuinely unsupported

---

## 3. TSC MANIPULATION

### 3.1 RDTSC Interception

**Source**: `exit.c:507-518`

```c
static bool vcpu_handle_rdtsc(struct vcpu *vcpu)
{
    u64 tsc = __rdtsc();  // Read actual TSC
    ksm_write_reg32(vcpu, STACK_REG_AX, (u32)tsc);
    ksm_write_reg32(vcpu, STACK_REG_DX, tsc >> 32);
    vcpu_advance_rip(vcpu);
    return true;
}
```

**Control Bit**: `CPU_BASED_RDTSC_EXITING` (vmx.h:41)

```c
#define CPU_BASED_RDTSC_EXITING  0x00001000
```

**Analysis**:
- KSM intercepts RDTSC but returns ACTUAL value (no offsetting)
- This is a hook point for timing normalization
- **Timing attack vulnerability**: VM-exit overhead adds latency

**Ombra Enhancement Pattern**:
```c
static u64 tsc_offset = 0;  // Per-VCPU

static bool vcpu_handle_rdtsc(struct vcpu *vcpu)
{
    u64 tsc = __rdtsc() + tsc_offset;  // Apply offset
    // ... write to RAX:RDX
}
```

---

### 3.2 RDTSCP Interception

**Source**: `exit.c:2245-2259`

```c
static bool vcpu_handle_rdtscp(struct vcpu *vcpu)
{
    u32 tsc_aux;
    u64 tsc = __rdtscp((unsigned int *)&tsc_aux);

    ksm_write_reg32(vcpu, STACK_REG_AX, (u32)tsc);
    ksm_write_reg32(vcpu, STACK_REG_DX, tsc >> 32);
    ksm_write_reg32(vcpu, STACK_REG_CX, tsc_aux);  // TSC_AUX in ECX
    vcpu_advance_rip(vcpu);
    return true;
}
```

**Control Bit**: `SECONDARY_EXEC_RDTSCP` (vmx.h:60)

```c
#define SECONDARY_EXEC_RDTSCP  0x00000008
```

**Analysis**:
- Also returns actual TSC + TSC_AUX
- TSC_AUX typically contains CPU ID (for NUMA awareness)

**Ombra Pattern**:
- Apply same offset as RDTSC
- Consider spoofing TSC_AUX to consistent value

---

### 3.3 TSC Offsetting via VMCS

**VMCS Fields** (not directly shown in KSM, but standard VMX):
- `TSC_OFFSET` (0x2010): Add to TSC on every read
- `TSC_MULTIPLIER` (0x2032): Scale TSC (if supported)

**Ombra Pattern**:
```c
// During VCPU setup
vmcs_write64(TSC_OFFSET, calculate_tsc_offset());

// No interception needed - hardware applies offset automatically
// Only intercept if dynamic adjustment required
```

**When to Intercept vs. Hardware Offset**:
- Hardware offset: Zero overhead, static offset
- Interception: Adds latency, but allows dynamic adjustment

---

## 4. CPUID SPOOFING

### 4.1 Current Implementation

**Source**: `exit.c:452-474` (already shown)

**Leaves Intercepted**:
- Leaf 1: VMX bit clearing

**Leaves NOT Intercepted** (passes through):
- Leaf 0: Vendor string
- Leaf 0x40000000: Hypervisor identification
- Leaf 0x80000000+: Extended features

---

### 4.2 Recommended Spoofing Strategy

**Critical Leaves to Intercept**:

| Leaf | Sub-Leaf | Register | Bits | Purpose |
|------|----------|----------|------|---------|
| 0x01 | 0 | ECX | [5] | VMX support (MUST clear) |
| 0x01 | 0 | ECX | [31] | Hypervisor present (MUST clear) |
| 0x40000000 | 0 | ALL | ALL | Hypervisor CPUID leaf (inject #GP or return 0) |
| 0x40000001-0x400000FF | ALL | ALL | ALL | Hypervisor info leaves (inject #GP) |

**Ombra Pattern**:
```c
static bool vcpu_handle_cpuid(struct vcpu *vcpu)
{
    int cpuid[4];
    int func = ksm_read_reg32(vcpu, STACK_REG_AX);
    int subf = ksm_read_reg32(vcpu, STACK_REG_CX);

    // Hypervisor CPUID space - fake unsupported
    if (func >= 0x40000000 && func <= 0x400000FF) {
        cpuid[0] = cpuid[1] = cpuid[2] = cpuid[3] = 0;
        goto write_regs;
    }

    __cpuidex(cpuid, func, subf);

    // Hide virtualization features
    if (func == 1) {
        cpuid[2] &= ~(1 << 5);   // Clear VMX bit
        cpuid[2] &= ~(1 << 31);  // Clear HYPERVISOR bit
    }

write_regs:
    ksm_write_reg32(vcpu, STACK_REG_AX, cpuid[0]);
    ksm_write_reg32(vcpu, STACK_REG_BX, cpuid[1]);
    ksm_write_reg32(vcpu, STACK_REG_CX, cpuid[2]);
    ksm_write_reg32(vcpu, STACK_REG_DX, cpuid[3]);
    vcpu_advance_rip(vcpu);
    return true;
}
```

---

### 4.3 Vendor String Spoofing (Optional)

**Pattern** (not in KSM, but common technique):
```c
if (func == 0) {
    // Keep actual max leaf in EAX
    // Spoof vendor string in EBX:EDX:ECX
    // "GenuineIntel" = 0x756E6547:0x49656E69:0x6C65746E
    cpuid[1] = 0x756E6547;  // "uneG"
    cpuid[2] = 0x6C65746E;  // "letn"
    cpuid[3] = 0x49656E69;  // "Ieni"
}
```

**Risk**: May conflict with actual CPU vendor. Better to pass through.

---

## 5. MSR FILTERING

### 5.1 MSR Bitmap Configuration

**Source**: `ksm.h:513`

```c
__align(PAGE_SIZE) u8 msr_bitmap[PAGE_SIZE];
```

**Initialization**: `ksm.c:74-87` (shown above)

**VMCS Setup**: `vcpu.c:705`

```c
err |= vmcs_write64(MSR_BITMAP, __pa(k->msr_bitmap));
```

---

### 5.2 Which MSRs to Filter

**KSM Filtered MSRs**:
- `MSR_IA32_FEATURE_CONTROL` (0x3A)
- `MSR_IA32_VMX_BASIC` (0x480)
- `MSR_IA32_VMX_PINBASED_CTLS` (0x481)
- `MSR_IA32_VMX_PROCBASED_CTLS` (0x482)
- `MSR_IA32_VMX_EXIT_CTLS` (0x483)
- `MSR_IA32_VMX_ENTRY_CTLS` (0x484)
- ... through `MSR_IA32_VMX_VMFUNC` (0x491)

**Ombra Additional MSRs to Filter**:
- `MSR_IA32_APICBASE` (0x1B): May leak virtualization via x2APIC mode
- `MSR_IA32_TSC_ADJUST` (0x3B): Can detect VM-exit overhead
- `MSR_IA32_DEBUGCTLMSR` (0x1D9): LBR may leak hypervisor addresses

---

### 5.3 Synthetic MSR Responses

**Pattern**: Return fake values instead of real hardware state

**Source**: `exit.c:1917-1923`

```c
case MSR_IA32_FEATURE_CONTROL:
    val = __readmsr(msr) & ~(FEATURE_CONTROL_VMXON_ENABLED_OUTSIDE_SMX |
                             FEATURE_CONTROL_VMXON_ENABLED_INSIDE_SMX);
    break;
```

**Ombra Pattern for VMX MSRs**:
```c
if (msr >= MSR_IA32_VMX_BASIC && msr <= MSR_IA32_VMX_VMFUNC) {
    // Inject #GP - make VMX appear unsupported
    vcpu_inject_hardirq(vcpu, X86_TRAP_GP, 0);
    return true;  // Don't advance RIP - exception delivery does that
}
```

---

## 6. TIMING ATTACK MITIGATION

### 6.1 Current KSM Approach

**Weakness**: KSM does NOT mitigate timing attacks
- RDTSC/RDTSCP return actual values
- VM-exit overhead (~1000-5000 cycles) is observable
- Pattern: Measure time around intercepted instructions

**Attack Pattern**:
```c
uint64_t start = __rdtsc();
__cpuid(1, 0);  // Intercepted by hypervisor
uint64_t end = __rdtsc();
if ((end - start) > 1000) {
    // Hypervisor detected!
}
```

---

### 6.2 Mitigation Strategies

**Strategy 1: TSC Offset Compensation**
```c
static u64 compensate_tsc_overhead(u64 actual_tsc, u64 overhead_cycles)
{
    // Subtract estimated VM-exit overhead
    return actual_tsc - overhead_cycles;
}

static bool vcpu_handle_rdtsc(struct vcpu *vcpu)
{
    u64 entry_tsc = __rdtsc();
    // ... VM-exit handling ...
    u64 exit_tsc = __rdtsc();
    u64 overhead = exit_tsc - entry_tsc;

    u64 compensated = exit_tsc - overhead;
    ksm_write_reg32(vcpu, STACK_REG_AX, (u32)compensated);
    ksm_write_reg32(vcpu, STACK_REG_DX, compensated >> 32);
    vcpu_advance_rip(vcpu);
    return true;
}
```

**Strategy 2: Hardware TSC Offsetting (Preferred)**
```c
// During VCPU init
vmcs_write64(TSC_OFFSET, 0);  // Start at zero

// Disable RDTSC exiting - let hardware handle
vm_cpuctl &= ~CPU_BASED_RDTSC_EXITING;
```

**Trade-off**:
- Hardware offsetting: No overhead, but static offset
- Interception: Dynamic compensation, but adds measurable latency

**Recommendation for Ombra**:
- Use hardware TSC offset by default
- Only intercept if dynamic TSC manipulation needed
- If intercepting, measure and compensate overhead aggressively

---

### 6.3 CPUID Timing Normalization

**Pattern**: Measure CPUID execution time, add padding if too fast

```c
static bool vcpu_handle_cpuid(struct vcpu *vcpu)
{
    u64 start_tsc = __rdtsc();

    int cpuid[4];
    int func = ksm_read_reg32(vcpu, STACK_REG_AX);
    int subf = ksm_read_reg32(vcpu, STACK_REG_CX);
    __cpuidex(cpuid, func, subf);

    // Apply stealth masks
    if (func == 1) {
        cpuid[2] &= ~((1 << 5) | (1 << 31));
    }

    u64 end_tsc = __rdtsc();
    u64 overhead = end_tsc - start_tsc;

    // If overhead > threshold, add delay
    if (overhead > 500) {
        // Busy wait to normalize timing
        u64 target = end_tsc + overhead;
        while (__rdtsc() < target) {
            _mm_pause();
        }
    }

    // Write results
    ksm_write_reg32(vcpu, STACK_REG_AX, cpuid[0]);
    // ...
    vcpu_advance_rip(vcpu);
    return true;
}
```

**Risk**: Adds more overhead, may make timing more suspicious

---

## 7. IMPLEMENTATION CHECKLIST FOR OMBRA

### 7.1 CPUID Hiding

- [ ] Intercept leaf 0x01, clear ECX[5] (VMX bit)
- [ ] Intercept leaf 0x01, clear ECX[31] (HYPERVISOR bit)
- [ ] Intercept leaf 0x40000000-0x400000FF, return zeros or #GP
- [ ] Pass through all other leaves unmodified

### 7.2 MSR Filtering

- [ ] Set MSR bitmap to intercept 0x480-0x491 (VMX MSRs)
- [ ] Inject #GP(0) on RDMSR of VMX capability MSRs
- [ ] Intercept MSR_IA32_FEATURE_CONTROL (0x3A), clear VMX enable bits
- [ ] Consider intercepting MSR_IA32_TSC_ADJUST for timing mitigation

### 7.3 TSC Manipulation

- [ ] Decide: Hardware TSC offset vs. RDTSC interception
- [ ] If intercepting: Measure and compensate VM-exit overhead
- [ ] Handle both RDTSC and RDTSCP consistently
- [ ] Test timing attack resistance with known exploits

### 7.4 General Stealth

- [ ] Disable VMXON/VMXOFF in secondary controls (let #UD occur naturally)
- [ ] Test with pafish, al-khaser, and custom VM detection tools
- [ ] Profile VM-exit overhead on target CPU (measure baseline)
- [ ] Consider EPT-based hooks for stealthier interception

---

## 8. CRITICAL PATTERNS FOR OMBRA

### 8.1 Dual-CPU Considerations

**KSM**: Intel-only, no AMD handling shown in these files

**Ombra Requirement**: Single binary, dual CPU support

**Pattern for Dual CPU**:
```c
if (cpu_vendor == CPU_INTEL) {
    // KSM-style CPUID/MSR handling
} else if (cpu_vendor == CPU_AMD) {
    // SVM equivalent - see SimpleSvm extraction
}
```

### 8.2 Boot-Time vs. Runtime Injection

**KSM**: Runtime injection (kernel driver)

**Ombra**: UEFI boot-time injection

**Implication**: Ombra has earlier control, can hide more aggressively
- Hide from boot-time VM checks
- No need for driver-level stealth
- Can intercept hypervisor setup itself (if Hyper-V detection bypass needed)

---

## 9. FILES TO MODIFY IN OMBRA

### OmbraPayload

- `OmbraPayload/Vmx/VmExit.cpp` - Add CPUID/RDTSC/MSR handlers
- `OmbraPayload/Vmx/Vmcs.cpp` - Configure MSR bitmap
- `OmbraPayload/Vmx/Vmx.cpp` - Initialize TSC offset

### OmbraBoot

- `OmbraBoot/Hooks/HvLoader.cpp` - Inject before Hyper-V initialization
- Consider: Hook hvloader.efi's VMX setup to inject Ombra first

---

## 10. RISKS & MITIGATIONS

| Risk | Mitigation |
|------|------------|
| **Timing attacks detect VM-exit overhead** | Use hardware TSC offset; disable RDTSC interception unless needed |
| **CPUID leaf 0x40000000 probing** | Return zeros or inject #GP for all 0x4000xxxx leaves |
| **MSR access patterns different from bare metal** | Inject #GP consistently for all VMX MSRs |
| **LBR (Last Branch Record) leaks hypervisor RIP** | Disable LBR or filter MSR_IA32_DEBUGCTLMSR |
| **Performance counters detect anomalies** | Consider intercepting PMC MSRs (0xC1-0xCF) |
| **Nested VM detection (VMX inside Ombra)** | If supporting nested: carefully filter VMX capabilities |

---

## 11. COMPARISON WITH OTHER HYPERVISORS

### KSM vs. HyperPlatform

- **KSM**: Minimal interception, basic CPUID masking
- **HyperPlatform**: More aggressive EPT-based hooks
- **Ombra**: Should combine both - KSM-style CPUID + HyperPlatform EPT

### KSM vs. VirtualBox Detection Bypass

- **KSM**: Hides VMX feature via CPUID bit clearing
- **VBoxHardenedLoader**: Hooks CPUID string, vendor info, timing
- **Ombra**: Adopt VBox techniques for vendor string spoofing

---

## SUMMARY

KSM provides **foundational stealth patterns** via:
1. CPUID bit masking (VMX feature hiding)
2. MSR bitmap filtering (VMX capability MSR injection of #GP)
3. Basic RDTSC/RDTSCP interception hooks

**Missing from KSM** (Ombra must add):
- Hypervisor bit (CPUID.01H:ECX[31]) clearing
- Hypervisor CPUID leaf (0x40000000) suppression
- Timing attack mitigation (TSC overhead compensation)
- LBR/PMC filtering

**Key Takeaway**: KSM shows WHERE to intercept, but Ombra needs MORE AGGRESSIVE masking for anti-cheat evasion.

---

**Extraction Complete**: 2025-12-20
**Next Steps**: Extract VBoxHardenedLoader for additional timing/vendor spoofing patterns
