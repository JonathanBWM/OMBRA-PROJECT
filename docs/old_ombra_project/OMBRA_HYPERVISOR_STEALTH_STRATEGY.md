# OMBRA HYPERVISOR STEALTH STRATEGY
## Comprehensive Intel VT-x Detection Bypass for BattlEye

---

## EXECUTIVE SUMMARY

**The Problem:** Intel CPUs running Ombra hypervisor get detected by BattlEye while AMD CPUs don't.

**Root Cause:** CPUID is an *unconditionally exiting* instruction on Intel VT-x. There is NO way to disable CPUID VM-exits. Every CPUID execution causes a VM-exit, creating a massive timing anomaly (~1000-5000 cycles vs ~50-200 on bare metal).

**The Solution:** A multi-layered approach combining:
1. TSC emulation to hide VM-exit timing
2. Proper exception handling (Trap Flag, #DB)
3. APERF/MPERF counter compensation
4. Hyper-V hijacking to hide within legitimate hypervisor
5. Minimal hook architecture (memhv-style)

---

## SECTION 1: UNDERSTANDING THE DETECTION VECTORS

### 1.1 BattlEye's Current Detection Methods

From secret.club research and our analysis:

```cpp
// BattlEye's timing attack (kernel mode)
void battleye::take_time() {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    
    // Calibrate cycles for 1000ms
    const auto timestamp_calibrator = __rdtsc();
    Sleep(1000);
    const auto timestamp_calibration = __rdtsc() - timestamp_calibrator;
    
    // Time CPUID 0x6694 times
    auto total_time = 0;
    for (std::size_t count = 0; count < 0x6694; count++) {
        const auto timestamp_pre = __rdtsc();
        std::uint32_t cpuid_data[4] = {};
        __cpuid(cpuid_data, 0);
        total_time += __rdtsc() - timestamp_pre;
    }
    
    // Report normalized timing
    battleye::report_table[0x1A8] = 10000000 * total_time / timestamp_calibration / 0x65;
}
```

**Detection Threshold:** ~750 cycles. Real hardware: 50-200 cycles. Hypervisor: 1000-5000+ cycles.

### 1.2 EasyAntiCheat Detection Methods

```cpp
// EAC performs vmread on driver init
// If vmread succeeds at Ring 0 → hypervisor detected
__try {
    ULONG64 value;
    __vmx_vmread(VMCS_GUEST_RIP, &value);
    // If we get here without #UD, hypervisor is present
    detected = TRUE;
} __except(EXCEPTION_EXECUTE_HANDLER) {
    detected = FALSE;
}
```

**Fix:** Inject #UD on vmread/vmwrite/vmx instructions.

### 1.3 Advanced Detection: IET Divergence (APERF-based)

This is the HARDEST to bypass:

```asm
cli
xor     r8d, r8d
mov     ecx, IA32_APERF_MSR       ; 0xE8
rdmsr
shl     rdx, 20h
or      rax, rdx
mov     r9, rax                    ; Save APERF before
xor     eax, eax
cpuid                              ; Unconditionally exits
mov     ecx, IA32_APERF_MSR
rdmsr
shl     rdx, 20h
or      rax, rdx
sub     rax, r9                    ; Delta APERF
; Compare against FYL2XP1 IET
; If CPUID takes LONGER than FYL2XP1, system is virtualized
```

**Why this is deadly:** APERF only increments when CPU is in C0 state doing actual work. Can't easily fake it like TSC.

### 1.4 Trap Flag Detection

```asm
pushfq
or dword ptr [rsp], 0100h    ; Set TF
mov eax, 0FFFFFFFFh
popfq
cpuid                         ; CPUID with TF set
nop
ret
```

**Expected behavior:** #DB exception on CPUID instruction itself.
**Broken hypervisor:** #DB on NOP (instruction after CPUID).

Most open-source hypervisors get this wrong. hvpp handles it correctly.

---

## SECTION 2: BYPASS STRATEGIES

### Strategy A: TSC Emulation (Primary)

The goal: Make CPUID appear to take native execution time by manipulating what RDTSC returns.

```cpp
// Conceptual implementation
struct VcpuTimingState {
    UINT64 emulated_tsc;           // Our fake TSC value
    UINT64 last_real_tsc;          // Real TSC at last sync
    UINT64 accumulated_exit_time;  // Total VM-exit overhead to hide
    BOOLEAN in_timing_critical;    // Are we between RDTSC calls?
};

VOID HandleRdtsc(PVCPU vcpu) {
    UINT64 real_tsc = __rdtsc();
    
    if (vcpu->timing.in_timing_critical) {
        // We're in a timing check, return emulated value
        UINT64 elapsed = real_tsc - vcpu->timing.last_real_tsc;
        UINT64 hidden_overhead = vcpu->timing.accumulated_exit_time;
        
        // Subtract the VM-exit overhead from perceived time
        vcpu->context.rax = (vcpu->timing.emulated_tsc + elapsed - hidden_overhead) & 0xFFFFFFFF;
        vcpu->context.rdx = (vcpu->timing.emulated_tsc + elapsed - hidden_overhead) >> 32;
    } else {
        // Normal case, just return real TSC with offset
        vcpu->context.rax = real_tsc & 0xFFFFFFFF;
        vcpu->context.rdx = real_tsc >> 32;
    }
    
    vcpu->timing.last_real_tsc = real_tsc;
}

VOID HandleCpuid(PVCPU vcpu) {
    UINT64 entry_tsc = __rdtsc();
    
    // Execute actual CPUID
    INT32 regs[4];
    __cpuidex(regs, vcpu->context.rax, vcpu->context.rcx);
    
    // Modify hypervisor bit if needed
    if (vcpu->context.rax == 1) {
        regs[2] &= ~(1 << 31);  // Clear hypervisor present bit
    }
    
    // Handle vendor string for 0x40000000
    if (vcpu->context.rax == 0x40000000) {
        // Return same as highest standard leaf
        __cpuidex(regs, 0, 0);
        __cpuidex(regs, regs[0], 0);
    }
    
    vcpu->context.rax = regs[0];
    vcpu->context.rbx = regs[1];
    vcpu->context.rcx = regs[2];
    vcpu->context.rdx = regs[3];
    
    UINT64 exit_tsc = __rdtsc();
    UINT64 exit_overhead = exit_tsc - entry_tsc;
    
    // Accumulate overhead to subtract from next RDTSC
    vcpu->timing.accumulated_exit_time += exit_overhead;
    
    // Subtract native CPUID time (keep ~100-200 cycles visible)
    vcpu->timing.accumulated_exit_time -= 150;  // Tunable constant
}
```

**Key Insight from secret.club:**
> "Trace from the first rdtsc instruction to the second, add an average cycle count to the emulated counter. No TSC offsetting, or other feature is utilized. You will also need to determine an average number of cycles the VM transitions take and subtract that from the emulated counter. The typical average is between 1.2k - 2k cycles on modern processors."

### Strategy B: VMFUNC + #VE (No VM-Exit EPT Handling)

Use Intel's VMFUNC for EPT switching WITHOUT VM-exits:

```cpp
// Setup dual EPT tables
EPT_POINTER CleanEPT;    // Shows original code
EPT_POINTER HookedEPT;   // Shows hooked code

// VMFUNC allows guest to switch EPT without VM-exit!
// Costs ~130 cycles vs ~700 for VMCALL

// In hooked function prologue:
__asm {
    mov eax, 0          ; EPTP switching function
    mov ecx, 1          ; Index of hooked EPT
    vmfunc              ; Switch to hooked view - NO VM EXIT!
    ; ... execute hook logic ...
    mov eax, 0
    mov ecx, 0          ; Back to clean EPT
    vmfunc
}
```

**Requirements:**
- Intel Haswell or newer (VMFUNC support)
- Intel Broadwell or newer for #VE (Virtualization Exception)

With #VE, EPT violations become guest exceptions instead of VM-exits:
```cpp
// EPT violation triggers #VE (interrupt 20) in guest
// Guest handles it at near-native speed
// No VM-exit timing anomaly!
```

### Strategy C: Hyper-V Hijacking (Current Ombra Approach - Enhanced)

Since Windows already runs under Hyper-V, we HIJACK it rather than create a new hypervisor:

```cpp
// From Voyager analysis:
// hvix64.exe contains VMexit handler
// Signature scan for 0F 78 (vmread) to find handler

// Patch Hyper-V's VMexit handler to call our code
VOID HijackHyperV() {
    PVOID hvix64_base = GetHvix64Base();
    
    // Find VMexit handler via pattern
    // 0F 78 is vmread instruction
    PVOID vmexit_handler = PatternScan(hvix64_base, 
        "\x0F\x78", "xx");
    
    // Install inline hook
    // Now OUR code runs in Hyper-V's context
    // Detection sees "normal" Hyper-V, not custom hypervisor
}
```

**Advantage:** BattlEye sees legitimate Hyper-V signatures. The timing anomalies are attributed to Windows' own virtualization.

**Enhancement Needed:** Still must handle CPUID timing within hijacked handler.

### Strategy D: memhv-Style Pure Memory Access

The safest approach - NO hooks at all:

```cpp
// memhv architecture:
// - NO CPUID interception
// - NO RDTSC interception  
// - NO code hooks
// Pure physical memory R/W via VMCALL hypercall

VOID MemhvRead(UINT64 PhysicalAddress, PVOID Buffer, SIZE_T Size) {
    // VMCALL with custom hypercall number
    // Hypervisor reads physical memory, returns to guest
}

VOID MemhvWrite(UINT64 PhysicalAddress, PVOID Buffer, SIZE_T Size) {
    // Same but write
}

// Features implemented via memory manipulation:
// - ESP: Read player positions from game memory
// - Aimbot: Write view angles directly
// - No recoil: Patch recoil values
// - Chams: Not possible (need rendering hooks)

// For rendering: External overlay on second PC or same-PC overlay
```

**memhv README states:**
> "Common timing attacks are ineffective against this hypervisor, as it does not exit on CPUID or similar instructions typically used in such attacks."

---

## SECTION 3: COMPREHENSIVE DETECTION MITIGATION CHECKLIST

### 3.1 Timing Attack Mitigations

| Attack | Mitigation | Implementation |
|--------|------------|----------------|
| RDTSC/CPUID/RDTSC | TSC emulation | Subtract VM-exit overhead from RDTSC return |
| APERF-based IET | Trap IA32_APERF MSR | Apply same compensation to APERF reads |
| Cross-core timing | Sync TSC across cores | Use TSC_OFFSET VMCS field per-core |
| Calibration attack | Dynamic offset | Adjust based on calibration detection |

### 3.2 Exception Handling Mitigations

| Attack | Mitigation | Implementation |
|--------|------------|----------------|
| Trap Flag + CPUID | Inject #DB correctly | Check pending debug exceptions, inject after CPUID emulation |
| XSETBV crash | Validate XCR0 | Check all XSETBV conditions, inject #GP if invalid |
| Invalid MSR | Inject #GP | Any MSR outside 0000-1FFF, C0000000-C0001FFF → #GP |
| vmread/vmwrite | Inject #UD | VMX instructions in guest must fault |

### 3.3 CPUID Leaf Mitigations

| Leaf | Expected | Fix |
|------|----------|-----|
| 0x1 ECX[31] | 0 (no hypervisor) | Clear hypervisor bit |
| 0x40000000 | Same as highest standard leaf | Return CPUID(max_leaf) response |
| 0x40000001+ | #GP or same as invalid | Return invalid leaf response |

### 3.4 Control Register Mitigations

| Issue | Mitigation |
|-------|------------|
| CR4.VMXE visible | Set CR4 shadow with VMXE=0 |
| VMX instructions work | Inject #UD for all VMX instructions |
| CR4 read inconsistency | Ensure shadow matches expected |

### 3.5 Descriptor Table Mitigations (WoW64)

```cpp
// When handling SGDT/SIDT in compatibility mode:
VmcsReadGuestGdtEntry(X86_REG_CS, &SegmentDesc);

if (SegmentDesc.Bits.L) {
    // Long mode: write 10 bytes
    WriteQword(&Descriptor->Base, VmcsRead(VMCS_GUEST_GDTR_BASE));
} else {
    // Compatibility mode: write only 6 bytes!
    WriteDword(&Descriptor->Base, (UINT32)VmcsRead(VMCS_GUEST_GDTR_BASE));
}
```

---

## SECTION 4: RECOMMENDED OMBRA ARCHITECTURE

### Phase 1: Immediate Fixes

1. **Fix Trap Flag Handling**
   - Check EFLAGS.TF before advancing RIP on VM-exit
   - Inject #DB after emulating exiting instruction if TF was set

2. **Fix CPUID Responses**
   - Clear hypervisor bit in leaf 0x1
   - Return consistent data for leaf 0x40000000

3. **Inject #UD for VMX Instructions**
   - vmread, vmwrite, vmlaunch, vmresume, etc.

### Phase 2: TSC Emulation

1. **Track VM-Exit Overhead**
   ```cpp
   UINT64 exit_start = __rdtsc();
   // ... handle VM-exit ...
   UINT64 exit_end = __rdtsc();
   vcpu->accumulated_overhead += (exit_end - exit_start);
   ```

2. **Compensate RDTSC Returns**
   ```cpp
   UINT64 real_tsc = __rdtsc();
   UINT64 fake_tsc = real_tsc - vcpu->accumulated_overhead;
   // Add back native instruction time (~150 cycles for CPUID)
   fake_tsc += 150;
   ```

3. **Handle RDTSCP Similarly**
   - Same compensation, also handle IA32_TSC_AUX

### Phase 3: APERF Compensation

1. **Intercept IA32_APERF MSR**
   - Set bit in MSR bitmap

2. **Apply Same Compensation Logic**
   - APERF only counts C0 cycles
   - Subtract VM-exit time from APERF returns

### Phase 4: Advanced Stealth (Optional)

1. **VMFUNC Implementation**
   - Use EPT switching for hooks without VM-exits
   - Requires Haswell+ CPU

2. **#VE (Virtualization Exception)**
   - Handle EPT violations as guest exceptions
   - Requires Broadwell+ CPU

3. **Hyper-V Integration**
   - Hide within legitimate Hyper-V
   - Attribute timing anomalies to Windows

---

## SECTION 5: ALTERNATIVE APPROACHES

### 5.1 DMA-Based (Hardware)

Completely bypasses hypervisor detection by using hardware:
- FPGA PCIe card (LeetDMA, Screamer, etc.)
- Reads game memory from second PC
- No software on gaming PC to detect

**Downside:** Expensive hardware, Riot/Vanguard now scans PCIe devices

### 5.2 External Rendering Only

If hypervisor detection can't be solved:
- Use memhv-style pure memory R/W
- No game function hooks
- External overlay for ESP
- Aimbot via memory writes (no rendering)

### 5.3 AMD-Only Support

Since AMD doesn't get detected:
- Ship AMD-only version
- Different hook architecture (NPT vs EPT)
- Accept Intel users are at higher risk

---

## SECTION 6: TESTING METHODOLOGY

### 6.1 Detection Testing Tools

1. **Pafish** - Basic VM detection tests
2. **Al-Khaser** - Comprehensive anti-analysis checks
3. **Custom RDTSC/CPUID timer** - BattlEye simulation

### 6.2 Timing Verification

```cpp
// Test timing attack resilience
UINT64 total = 0;
for (int i = 0; i < 10000; i++) {
    UINT64 start = __rdtsc();
    __cpuid(regs, 0);
    UINT64 end = __rdtsc();
    total += (end - start);
}
UINT64 average = total / 10000;

// Should be < 500 cycles to pass BattlEye
printf("Average CPUID cycles: %llu\n", average);
```

### 6.3 Exception Verification

```cpp
// Test trap flag handling
BOOLEAN TrapFlagTest() {
    __try {
        __asm {
            pushfq
            or dword ptr [rsp], 0x100
            popfq
            cpuid
            nop
        }
    } __except(GetExceptionCode() == EXCEPTION_SINGLE_STEP ? 
               EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Check if we're on CPUID or NOP
        // Should be on CPUID for correct implementation
    }
}
```

---

## CONCLUSION

The Intel detection issue stems from fundamental VT-x architecture: CPUID unconditionally exits. The solution requires:

1. **Immediate:** Fix exception handling, CPUID responses, inject #UD for VMX
2. **Short-term:** Implement TSC emulation to hide VM-exit overhead
3. **Medium-term:** Add APERF compensation for IET divergence attacks
4. **Long-term:** Consider VMFUNC/#VE for truly exit-less hooks

The Hyper-V hijacking approach gives us cover ("it's just Windows Hyper-V") but still requires timing compensation. The memhv approach of no hooks + pure memory access is the safest but limits features.

**Recommended Priority:**
1. TSC emulation (defeats current BattlEye check)
2. Trap flag fix (defeats common detection)
3. APERF compensation (defeats advanced detection)
4. VMFUNC implementation (future-proofs against new detection)

---

*Research compiled from: secret.club, tandasat projects, memhv, Intel SDM, AMD APM, public hypervisor projects*
