# HYPERVISOR DETECTION VECTORS & MITIGATIONS
## Complete Reference Matrix

---

## DETECTION VECTOR MATRIX

| # | Vector | Severity | BattlEye | EAC | ESEA | Mitigation Complexity |
|---|--------|----------|----------|-----|------|----------------------|
| 1 | RDTSC/CPUID/RDTSC timing | HIGH | ✓ | ✓ | ✓ | HIGH |
| 2 | APERF-based IET divergence | CRITICAL | ? | ? | ✓ | VERY HIGH |
| 3 | vmread at Ring 0 | MEDIUM | ✗ | ✓ | ? | LOW |
| 4 | Hypervisor CPUID bit (0x1 ECX[31]) | MEDIUM | ? | ? | ✓ | LOW |
| 5 | CPUID leaf 0x40000000 | MEDIUM | ? | ? | ✓ | LOW |
| 6 | Trap Flag + exiting instruction | HIGH | ? | ? | ✓ | MEDIUM |
| 7 | XSETBV crash test | MEDIUM | ? | ? | ? | MEDIUM |
| 8 | Invalid MSR access | LOW | ✗ | ✗ | ? | LOW |
| 9 | LBR virtualization | MEDIUM | ? | ? | ✓ | MEDIUM |
| 10 | WoW64 descriptor table | HIGH | ? | ? | ✓ | MEDIUM |
| 11 | CR4.VMXE + VMX instructions | MEDIUM | ? | ✓ | ? | LOW |
| 12 | Synthetic MSR (0x40000000+) | MEDIUM | ? | ? | ? | LOW |
| 13 | INVD/WBINVD cache side-channel | HIGH | ? | ? | ✓ | HIGH |

---

## DETAILED MITIGATION IMPLEMENTATIONS

### 1. XSETBV Exception Handling

XSETBV is an unconditionally exiting instruction. Many hypervisors blindly execute it, causing crashes on invalid input.

```cpp
// Valid XCR0 bit combinations per Intel SDM
BOOLEAN IsValidXcr0(UINT64 Xcr0) {
    // Bit 0 (FP) must ALWAYS be set
    if (!(Xcr0 & (1 << 0))) {
        return FALSE;
    }
    
    // Bit 2 (YMM/AVX) requires Bit 1 (SSE)
    if ((Xcr0 & (1 << 2)) && !(Xcr0 & (1 << 1))) {
        return FALSE;
    }
    
    // Bits 3 and 4 (BNDREGS, BNDCSR) must match
    BOOLEAN HasBndregs = (Xcr0 & (1 << 3)) != 0;
    BOOLEAN HasBndcsr = (Xcr0 & (1 << 4)) != 0;
    if (HasBndregs != HasBndcsr) {
        return FALSE;
    }
    
    // AVX-512: bits 5, 6, 7 require bit 2 (YMM)
    if (Xcr0 & ((1 << 5) | (1 << 6) | (1 << 7))) {
        if (!(Xcr0 & (1 << 2))) {
            return FALSE;
        }
        // All three must be set together
        if ((Xcr0 & ((1 << 5) | (1 << 6) | (1 << 7))) != 
            ((1 << 5) | (1 << 6) | (1 << 7))) {
            return FALSE;
        }
    }
    
    return TRUE;
}

// Get supported XCR0 bits from CPUID
UINT64 GetSupportedXcr0Bits() {
    INT32 Regs[4];
    __cpuidex(Regs, 0xD, 0);
    return ((UINT64)Regs[3] << 32) | Regs[0];
}

VOID HandleXsetbv(PVCPU Vcpu) {
    UINT32 Xcr = (UINT32)Vcpu->Context.Rcx;
    UINT64 Value = ((UINT64)Vcpu->Context.Rdx << 32) | 
                   (Vcpu->Context.Rax & 0xFFFFFFFF);
    
    // Only XCR0 is valid
    if (Xcr != 0) {
        InjectGpFault(Vcpu, 0);
        return;
    }
    
    // Check for unsupported bits
    if (Value & ~GetSupportedXcr0Bits()) {
        InjectGpFault(Vcpu, 0);
        return;
    }
    
    // Validate architectural requirements
    if (!IsValidXcr0(Value)) {
        InjectGpFault(Vcpu, 0);
        return;
    }
    
    // Safe to execute
    _xsetbv(0, Value);
    
    AdvanceGuestRip(Vcpu);
}
```

### 2. Invalid MSR Handling

MSR bitmap only covers 0x0000-0x1FFF and 0xC0000000-0xC0001FFF. Accesses outside these ranges can cause issues.

```cpp
VOID HandleMsrRead(PVCPU Vcpu) {
    UINT32 Msr = (UINT32)Vcpu->Context.Rcx;
    UINT64 Value;
    
    // Check if MSR is in valid ranges
    BOOLEAN InLowRange = (Msr <= 0x1FFF);
    BOOLEAN InHighRange = (Msr >= 0xC0000000 && Msr <= 0xC0001FFF);
    
    if (!InLowRange && !InHighRange) {
        // Check if it's a known valid MSR outside bitmap range
        if (!IsKnownValidMsr(Msr)) {
            InjectGpFault(Vcpu, 0);
            return;
        }
    }
    
    // Handle specific MSRs we want to intercept
    switch (Msr) {
        case IA32_EFER:
            Value = VmcsRead(VMCS_GUEST_IA32_EFER);
            break;
            
        case IA32_APERF:
            Value = HandleAperfRead(Vcpu);
            return;  // Handler advances RIP
            
        case IA32_MPERF:
            Value = HandleMperfRead(Vcpu);
            return;
            
        default:
            // Try to read, inject #GP on failure
            __try {
                Value = __readmsr(Msr);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                InjectGpFault(Vcpu, 0);
                return;
            }
            break;
    }
    
    Vcpu->Context.Rax = Value & 0xFFFFFFFF;
    Vcpu->Context.Rdx = Value >> 32;
    
    AdvanceGuestRip(Vcpu);
}

BOOLEAN IsKnownValidMsr(UINT32 Msr) {
    // List of valid MSRs outside bitmap range
    static const UINT32 ValidMsrs[] = {
        0x000001A0,  // IA32_MISC_ENABLE
        0x000001B0,  // IA32_ENERGY_PERF_BIAS
        0x00000277,  // IA32_PAT
        0x000002FF,  // IA32_MTRR_DEF_TYPE
        0x00000480,  // IA32_VMX_BASIC
        // ... more VMX capability MSRs ...
    };
    
    for (UINT32 i = 0; i < ARRAYSIZE(ValidMsrs); i++) {
        if (ValidMsrs[i] == Msr) return TRUE;
    }
    
    return FALSE;
}
```

### 3. LBR (Last Branch Record) Handling

```cpp
// LBR MSR ranges vary by CPU family
typedef struct _LBR_CONFIG {
    UINT32 FromMsrBase;
    UINT32 ToMsrBase;
    UINT32 TosMsr;
    UINT32 StackSize;
} LBR_CONFIG;

LBR_CONFIG GetLbrConfig() {
    INT32 Regs[4];
    __cpuidex(Regs, 1, 0);
    
    UINT32 Family = ((Regs[0] >> 8) & 0xF) + ((Regs[0] >> 20) & 0xFF);
    UINT32 Model = ((Regs[0] >> 4) & 0xF) | ((Regs[0] >> 12) & 0xF0);
    
    LBR_CONFIG Config = {0};
    
    // Skylake and later
    if (Family == 6 && Model >= 0x4E) {
        Config.FromMsrBase = 0x680;
        Config.ToMsrBase = 0x6C0;
        Config.TosMsr = 0x1C9;
        Config.StackSize = 32;
    }
    // Haswell/Broadwell
    else if (Family == 6 && Model >= 0x3C) {
        Config.FromMsrBase = 0x680;
        Config.ToMsrBase = 0x6C0;
        Config.TosMsr = 0x1C9;
        Config.StackSize = 16;
    }
    // Older
    else {
        Config.FromMsrBase = 0x40;
        Config.ToMsrBase = 0x60;
        Config.TosMsr = 0x1C9;
        Config.StackSize = 4;
    }
    
    return Config;
}

// Save LBR state on VM-exit
VOID SaveLbrState(PVCPU Vcpu) {
    LBR_CONFIG Config = GetLbrConfig();
    
    Vcpu->LbrState.Tos = __readmsr(Config.TosMsr);
    
    for (UINT32 i = 0; i < Config.StackSize; i++) {
        Vcpu->LbrState.From[i] = __readmsr(Config.FromMsrBase + i);
        Vcpu->LbrState.To[i] = __readmsr(Config.ToMsrBase + i);
    }
}

// Restore LBR state on VM-entry
VOID RestoreLbrState(PVCPU Vcpu) {
    LBR_CONFIG Config = GetLbrConfig();
    
    __writemsr(Config.TosMsr, Vcpu->LbrState.Tos);
    
    for (UINT32 i = 0; i < Config.StackSize; i++) {
        __writemsr(Config.FromMsrBase + i, Vcpu->LbrState.From[i]);
        __writemsr(Config.ToMsrBase + i, Vcpu->LbrState.To[i]);
    }
}
```

### 4. WoW64 Descriptor Table Size Fix

```cpp
VOID HandleSgdt(PVCPU Vcpu) {
    // Get instruction info
    UINT64 InstructionInfo = VmcsRead(VMCS_EXIT_INSTRUCTION_INFO);
    UINT64 Qualification = VmcsRead(VMCS_EXIT_QUALIFICATION);
    
    // Calculate destination address
    PVOID DestAddress = CalculateLinearAddress(Vcpu, InstructionInfo, Qualification);
    
    // Read guest CS segment
    SEGMENT_DESCRIPTOR CsDesc;
    ReadGuestSegment(Vcpu, SEGMENT_CS, &CsDesc);
    
    // Get GDTR values
    UINT16 Limit = (UINT16)VmcsRead32(VMCS_GUEST_GDTR_LIMIT);
    UINT64 Base = VmcsRead(VMCS_GUEST_GDTR_BASE);
    
    // Write limit (always 2 bytes)
    SafeWriteGuest(Vcpu, DestAddress, &Limit, sizeof(Limit));
    
    // Write base - SIZE DEPENDS ON MODE
    if (CsDesc.Bits.L) {
        // Long mode: 8 bytes for base
        SafeWriteGuest(Vcpu, (PUCHAR)DestAddress + 2, &Base, 8);
    } else {
        // Compatibility/Protected mode: 4 bytes for base
        UINT32 Base32 = (UINT32)Base;
        SafeWriteGuest(Vcpu, (PUCHAR)DestAddress + 2, &Base32, 4);
    }
    
    AdvanceGuestRip(Vcpu);
}

VOID HandleSidt(PVCPU Vcpu) {
    // Same logic as SGDT but for IDTR
    UINT64 InstructionInfo = VmcsRead(VMCS_EXIT_INSTRUCTION_INFO);
    UINT64 Qualification = VmcsRead(VMCS_EXIT_QUALIFICATION);
    
    PVOID DestAddress = CalculateLinearAddress(Vcpu, InstructionInfo, Qualification);
    
    SEGMENT_DESCRIPTOR CsDesc;
    ReadGuestSegment(Vcpu, SEGMENT_CS, &CsDesc);
    
    UINT16 Limit = (UINT16)VmcsRead32(VMCS_GUEST_IDTR_LIMIT);
    UINT64 Base = VmcsRead(VMCS_GUEST_IDTR_BASE);
    
    SafeWriteGuest(Vcpu, DestAddress, &Limit, sizeof(Limit));
    
    if (CsDesc.Bits.L) {
        SafeWriteGuest(Vcpu, (PUCHAR)DestAddress + 2, &Base, 8);
    } else {
        UINT32 Base32 = (UINT32)Base;
        SafeWriteGuest(Vcpu, (PUCHAR)DestAddress + 2, &Base32, 4);
    }
    
    AdvanceGuestRip(Vcpu);
}
```

### 5. INVD/WBINVD Cache Side-Channel Mitigation

This is a tricky one - INVD flushes caches WITHOUT writing back to memory.

```cpp
VOID HandleInvd(PVCPU Vcpu) {
    // INVD invalidates caches without writeback
    // This is DANGEROUS on real hardware (data loss)
    // Most hypervisors just execute WBINVD instead
    // But that's detectable!
    
    // Proper handling: Use cache line by line invalidation
    // Or: Just execute INVD if safe
    
    // Check if SGX is enabled (INVD causes #GP with SGX)
    UINT64 Ia32FeatureControl = __readmsr(IA32_FEATURE_CONTROL);
    if (Ia32FeatureControl & (1 << 18)) {  // SGX enabled
        InjectGpFault(Vcpu, 0);
        return;
    }
    
    // For non-SGX systems, INVD is safe but destructive
    // Execute it to maintain correct behavior
    __invd();
    
    AdvanceGuestRip(Vcpu);
}

VOID HandleWbinvd(PVCPU Vcpu) {
    // WBINVD writes back and invalidates - safe to execute
    __wbinvd();
    AdvanceGuestRip(Vcpu);
}
```

### 6. VMX Instruction Injection

```cpp
VOID HandleVmxInstruction(PVCPU Vcpu, UINT32 ExitReason) {
    // Check if CR4.VMXE is supposed to be visible to guest
    UINT64 Cr4Shadow = VmcsRead(VMCS_CTRL_CR4_READ_SHADOW);
    UINT64 Cr4Mask = VmcsRead(VMCS_CTRL_CR4_GUEST_HOST_MASK);
    
    // If VMXE bit is masked and shadow shows 0, inject #UD
    if ((Cr4Mask & X86_CR4_VMXE) && !(Cr4Shadow & X86_CR4_VMXE)) {
        InjectUdFault(Vcpu);
        return;
    }
    
    // If VMXE appears enabled but we're hiding, inject #GP
    // (VMX instructions require CPL 0 and VMXE=1)
    UINT32 Cpl = GetGuestCpl(Vcpu);
    if (Cpl != 0) {
        InjectGpFault(Vcpu, 0);
        return;
    }
    
    // Default: inject #UD (we're hiding hypervisor)
    InjectUdFault(Vcpu);
}

VOID InjectUdFault(PVCPU Vcpu) {
    UINT32 InterruptInfo = 0;
    InterruptInfo |= 6;          // Vector #UD
    InterruptInfo |= (3 << 8);   // Hardware exception
    InterruptInfo |= (1 << 31);  // Valid
    
    VmcsWrite32(VMCS_ENTRY_INTERRUPTION_INFO, InterruptInfo);
    // Do NOT advance RIP - faults re-execute
}

VOID InjectGpFault(PVCPU Vcpu, UINT32 ErrorCode) {
    UINT32 InterruptInfo = 0;
    InterruptInfo |= 13;         // Vector #GP
    InterruptInfo |= (3 << 8);   // Hardware exception
    InterruptInfo |= (1 << 11);  // Deliver error code
    InterruptInfo |= (1 << 31);  // Valid
    
    VmcsWrite32(VMCS_ENTRY_INTERRUPTION_INFO, InterruptInfo);
    VmcsWrite32(VMCS_ENTRY_EXCEPTION_ERROR_CODE, ErrorCode);
}
```

---

## COMPLETE VM-EXIT HANDLER TEMPLATE

```cpp
VOID VmExitDispatcher(PVCPU Vcpu) {
    // Capture entry TSC for timing compensation
    UINT64 EntryTsc = __rdtsc();
    Vcpu->Timing.ExitEntryTsc = EntryTsc;
    
    UINT32 ExitReason = VmcsRead32(VMCS_EXIT_REASON) & 0xFFFF;
    BOOLEAN NeedsTimingCompensation = FALSE;
    
    switch (ExitReason) {
        // ===== TIMING-SENSITIVE EXITS =====
        case EXIT_REASON_CPUID:
            HandleCpuid(Vcpu);
            NeedsTimingCompensation = TRUE;
            break;
            
        case EXIT_REASON_RDTSC:
            HandleRdtsc(Vcpu);
            // Don't add compensation - we're returning fake TSC
            break;
            
        case EXIT_REASON_RDTSCP:
            HandleRdtscp(Vcpu);
            break;
            
        case EXIT_REASON_MSR_READ:
            HandleMsrRead(Vcpu);
            NeedsTimingCompensation = TRUE;
            break;
            
        case EXIT_REASON_MSR_WRITE:
            HandleMsrWrite(Vcpu);
            NeedsTimingCompensation = TRUE;
            break;
            
        // ===== EXCEPTION-RELATED EXITS =====
        case EXIT_REASON_XSETBV:
            HandleXsetbv(Vcpu);
            break;
            
        case EXIT_REASON_INVD:
            HandleInvd(Vcpu);
            break;
            
        case EXIT_REASON_WBINVD:
            HandleWbinvd(Vcpu);
            break;
            
        // ===== VMX INSTRUCTION EXITS =====
        case EXIT_REASON_VMCALL:
        case EXIT_REASON_VMCLEAR:
        case EXIT_REASON_VMLAUNCH:
        case EXIT_REASON_VMPTRLD:
        case EXIT_REASON_VMPTRST:
        case EXIT_REASON_VMREAD:
        case EXIT_REASON_VMRESUME:
        case EXIT_REASON_VMWRITE:
        case EXIT_REASON_VMXOFF:
        case EXIT_REASON_VMXON:
        case EXIT_REASON_INVEPT:
        case EXIT_REASON_INVVPID:
            HandleVmxInstruction(Vcpu, ExitReason);
            break;
            
        // ===== DESCRIPTOR TABLE EXITS =====
        case EXIT_REASON_GDTR_IDTR_ACCESS:
            HandleGdtrIdtrAccess(Vcpu);
            break;
            
        case EXIT_REASON_LDTR_TR_ACCESS:
            HandleLdtrTrAccess(Vcpu);
            break;
            
        // ===== EPT EXITS =====
        case EXIT_REASON_EPT_VIOLATION:
            HandleEptViolation(Vcpu);
            NeedsTimingCompensation = TRUE;
            break;
            
        case EXIT_REASON_EPT_MISCONFIGURATION:
            HandleEptMisconfig(Vcpu);
            break;
            
        // ===== CONTROL REGISTER EXITS =====
        case EXIT_REASON_CR_ACCESS:
            HandleCrAccess(Vcpu);
            break;
            
        // ===== INTERRUPT/EXCEPTION EXITS =====
        case EXIT_REASON_EXCEPTION_NMI:
            HandleException(Vcpu);
            break;
            
        case EXIT_REASON_EXTERNAL_INTERRUPT:
            HandleExternalInterrupt(Vcpu);
            break;
            
        // ===== MONITOR TRAP FLAG =====
        case EXIT_REASON_MONITOR_TRAP_FLAG:
            HandleMtf(Vcpu);
            NeedsTimingCompensation = TRUE;
            break;
            
        default:
            // Unhandled exit - log and crash for debugging
            DbgPrint("[VMM] Unhandled exit: %d\n", ExitReason);
            KeBugCheck(0xDEADBEEF);
            break;
    }
    
    // Accumulate timing overhead for compensation
    if (NeedsTimingCompensation) {
        UINT64 ExitTsc = __rdtsc();
        UINT64 Overhead = ExitTsc - EntryTsc;
        
        // Subtract expected native time
        UINT64 NativeTime = GetNativeInstructionTime(ExitReason);
        if (Overhead > NativeTime) {
            InterlockedAdd64(&Vcpu->Timing.AccumulatedOverhead,
                            Overhead - NativeTime);
        }
    }
    
    // Check for pending debug exceptions (Trap Flag)
    CheckPendingDebugExceptions(Vcpu);
    
    // Resume guest
    VmResume();
}

UINT64 GetNativeInstructionTime(UINT32 ExitReason) {
    switch (ExitReason) {
        case EXIT_REASON_CPUID:
            return 150;  // CPUID typically 100-200 cycles
        case EXIT_REASON_MSR_READ:
        case EXIT_REASON_MSR_WRITE:
            return 30;   // MSR access typically 20-50 cycles
        case EXIT_REASON_EPT_VIOLATION:
            return 0;    // Varies wildly
        case EXIT_REASON_MONITOR_TRAP_FLAG:
            return 0;    // Varies
        default:
            return 0;
    }
}
```

---

## TESTING CHECKLIST

```cpp
VOID RunDetectionTests() {
    DbgPrint("[TEST] Starting hypervisor detection tests...\n");
    
    // 1. Timing test
    BOOLEAN TimingPass = TestRdtscTiming();
    DbgPrint("[TEST] RDTSC timing: %s\n", TimingPass ? "PASS" : "FAIL");
    
    // 2. CPUID hypervisor bit
    BOOLEAN CpuidPass = TestCpuidHypervisorBit();
    DbgPrint("[TEST] CPUID hypervisor bit: %s\n", CpuidPass ? "PASS" : "FAIL");
    
    // 3. CPUID 0x40000000
    BOOLEAN Leaf40Pass = TestCpuidLeaf40();
    DbgPrint("[TEST] CPUID leaf 0x40000000: %s\n", Leaf40Pass ? "PASS" : "FAIL");
    
    // 4. Trap flag
    BOOLEAN TrapPass = TestTrapFlag();
    DbgPrint("[TEST] Trap flag handling: %s\n", TrapPass ? "PASS" : "FAIL");
    
    // 5. XSETBV
    BOOLEAN XsetbvPass = TestXsetbv();
    DbgPrint("[TEST] XSETBV handling: %s\n", XsetbvPass ? "PASS" : "FAIL");
    
    // 6. VMX instructions
    BOOLEAN VmxPass = TestVmxInstructions();
    DbgPrint("[TEST] VMX instruction handling: %s\n", VmxPass ? "PASS" : "FAIL");
    
    DbgPrint("[TEST] Complete. Failures should be addressed!\n");
}
```

---

*Reference: Intel SDM Vol. 3C, AMD APM Vol. 2, secret.club research, hvpp, HyperPlatform*
