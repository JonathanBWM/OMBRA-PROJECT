# OMBRA V3 IMPLEMENTATION ROADMAP
## Timing Bypass Integration & Detection Evasion

---

## CURRENT STATE (From Audit)

**Working:**
- Intel VMX VMexit handler hijacking
- Bootchain hooking (OmbraBoot → winload → hvloader → hv.exe)
- Payload injection into hv.exe .rsrc section
- Basic CPUID backdoor commands

**Broken/Incomplete:**
- AMD SVM support (stubs only)
- No timing bypass (BattlEye detects on Intel)
- No EPT execute-only split view
- No TSC compensation
- No APERF/MPERF handling

---

## PHASE 1: TIMING BYPASS (CRITICAL)

### 1.1 TSC Offset Compensation

The primary detection vector is CPUID timing. Must subtract VM-exit overhead from TSC.

**Location:** Patch into hvix64.exe VMexit handler (before calling original)

```cpp
// Add to OmbraPayload/Entry.cpp or create Timing.cpp

struct TIMING_STATE {
    UINT64 AccumulatedOverhead;  // Total overhead tracked
    UINT64 LastExitTsc;          // Entry TSC of current exit
    UINT64 CalibrationOffset;    // Per-system calibration
};

// Per-vCPU timing state
__declspec(align(64)) TIMING_STATE g_TimingState[64];

void ApplyTscCompensation(UINT32 CpuIndex, UINT64 ExitReason) {
    TIMING_STATE* ts = &g_TimingState[CpuIndex];
    
    // Only compensate timing-sensitive exits
    if (ExitReason != EXIT_REASON_CPUID && 
        ExitReason != EXIT_REASON_MSR_READ &&
        ExitReason != EXIT_REASON_MSR_WRITE) {
        return;
    }
    
    // Read current TSC offset from VMCS
    UINT64 CurrentOffset;
    __vmx_vmread(VMCS_TSC_OFFSET, &CurrentOffset);
    
    // Get expected overhead for this exit type
    UINT64 ExpectedOverhead = GetExpectedOverhead(ExitReason);
    UINT64 NativeTime = GetNativeInstructionTime(ExitReason);
    
    // Compensation = Overhead - NativeTime
    UINT64 Compensation = ExpectedOverhead > NativeTime ? 
                          ExpectedOverhead - NativeTime : 0;
    
    // Subtract from TSC offset
    __vmx_vmwrite(VMCS_TSC_OFFSET, CurrentOffset - Compensation);
    
    // Track for APERF compensation
    ts->AccumulatedOverhead += Compensation;
}

UINT64 GetExpectedOverhead(UINT64 ExitReason) {
    switch (ExitReason) {
        case EXIT_REASON_CPUID: return 1200;  // Intel typical
        case EXIT_REASON_MSR_READ: return 800;
        case EXIT_REASON_MSR_WRITE: return 800;
        case EXIT_REASON_EPT_VIOLATION: return 1500;
        default: return 500;
    }
}

UINT64 GetNativeInstructionTime(UINT64 ExitReason) {
    switch (ExitReason) {
        case EXIT_REASON_CPUID: return 150;  // Native ~100-200
        case EXIT_REASON_MSR_READ: return 30;
        case EXIT_REASON_MSR_WRITE: return 30;
        default: return 0;
    }
}
```

### 1.2 Entry/Exit TSC Tracking

Modify the VMexit entry point to capture TSC:

```asm
; EntryAsm.asm - Add timing capture

_OmbraEntry PROC
    ; Capture entry TSC FIRST
    rdtsc
    shl rdx, 32
    or rax, rdx
    mov r12, rax            ; r12 = entry TSC (preserved)
    
    ; Save original registers (existing code)
    push rbx
    push rbp
    push rsi
    push rdi
    push r13
    push r14
    push r15
    
    ; Store entry TSC for compensation
    mov rcx, gs:[188h]      ; Get current VCPU index
    lea rax, g_TimingState
    imul rcx, 64            ; sizeof(TIMING_STATE)
    add rax, rcx
    mov [rax + 8], r12      ; Store LastExitTsc
    
    ; ... existing handler code ...
    
    ; Before returning, apply compensation
    call ApplyTscCompensation
    
    ; Restore and continue to original handler
    pop r15
    pop r14
    pop r13
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    jmp OriginalHandler
_OmbraEntry ENDP
```

### 1.3 APERF/MPERF MSR Interception

APERF-based detection is harder to bypass. Must intercept MSR reads:

```cpp
// Add to OmbraPayload/Entry.cpp

void HandleMsrRead(PGUEST_STATE GuestState) {
    UINT32 Msr = (UINT32)GuestState->Rcx;
    UINT64 Value;
    
    switch (Msr) {
        case IA32_APERF:  // 0xE8
        case IA32_MPERF:  // 0xE7
            // Read actual value
            Value = __readmsr(Msr);
            
            // Subtract accumulated VM-exit time
            UINT32 CpuIndex = GetCurrentCpuIndex();
            TIMING_STATE* ts = &g_TimingState[CpuIndex];
            
            if (Value > ts->AccumulatedOverhead) {
                Value -= ts->AccumulatedOverhead;
            }
            
            GuestState->Rax = Value & 0xFFFFFFFF;
            GuestState->Rdx = Value >> 32;
            break;
            
        default:
            // Pass through to real MSR read
            Value = __readmsr(Msr);
            GuestState->Rax = Value & 0xFFFFFFFF;
            GuestState->Rdx = Value >> 32;
            break;
    }
    
    AdvanceGuestRip();
}

// Enable MSR bitmap interception for APERF/MPERF
void EnableAperfMperfInterception() {
    // Find Hyper-V's MSR bitmap
    UINT64 MsrBitmapAddr;
    __vmx_vmread(VMCS_MSR_BITMAP, &MsrBitmapAddr);
    
    PUCHAR MsrBitmap = (PUCHAR)PhysToVirt(MsrBitmapAddr);
    
    // Enable read interception for 0xE7 (MPERF) and 0xE8 (APERF)
    // Low MSR range: bits 0-0x1FFF in first 1KB
    MsrBitmap[0xE7 / 8] |= (1 << (0xE7 % 8));
    MsrBitmap[0xE8 / 8] |= (1 << (0xE8 % 8));
}
```

---

## PHASE 2: AMD SVM COMPLETION

### 2.1 CPU Vendor Detection in Payload

```cpp
// Add to OmbraPayload/Entry.cpp

enum CPU_VENDOR {
    VENDOR_UNKNOWN = 0,
    VENDOR_INTEL = 1,
    VENDOR_AMD = 2
};

CPU_VENDOR DetectCpuVendor() {
    int regs[4];
    __cpuid(regs, 0);
    
    // "GenuineIntel"
    if (regs[1] == 0x756E6547 && regs[3] == 0x49656E69 && regs[2] == 0x6C65746E)
        return VENDOR_INTEL;
    
    // "AuthenticAMD"
    if (regs[1] == 0x68747541 && regs[3] == 0x69746E65 && regs[2] == 0x444D4163)
        return VENDOR_AMD;
    
    return VENDOR_UNKNOWN;
}

// Main dispatch based on vendor
void HyperVBackdoor(void* GuestState, void* Arg2) {
    static CPU_VENDOR s_Vendor = VENDOR_UNKNOWN;
    
    if (s_Vendor == VENDOR_UNKNOWN) {
        s_Vendor = DetectCpuVendor();
    }
    
    if (s_Vendor == VENDOR_INTEL) {
        HyperVBackdoor_Intel((PGUEST_STATE)GuestState, Arg2);
    } else if (s_Vendor == VENDOR_AMD) {
        HyperVBackdoor_AMD((PGUEST_STATE)GuestState, Arg2);
    }
}
```

### 2.2 AMD VMCB Handling

```cpp
// OmbraPayload/Virt/Amd/Vmcb.h

#pragma pack(push, 1)
typedef struct _VMCB_CONTROL_AREA {
    UINT16 InterceptCrRead;      // +0x000
    UINT16 InterceptCrWrite;     // +0x002
    UINT16 InterceptDrRead;      // +0x004
    UINT16 InterceptDrWrite;     // +0x006
    UINT32 InterceptException;   // +0x008
    UINT32 InterceptMisc1;       // +0x00C
    UINT32 InterceptMisc2;       // +0x010
    UINT8  Reserved1[0x03C - 0x014];
    UINT16 PauseFilterThreshold; // +0x03C
    UINT16 PauseFilterCount;     // +0x03E
    UINT64 IopmBasePa;           // +0x040
    UINT64 MsrpmBasePa;          // +0x048
    UINT64 TscOffset;            // +0x050  <-- KEY for timing!
    UINT32 GuestAsid;            // +0x058
    UINT32 TlbControl;           // +0x05C
    UINT64 VIntr;                // +0x060
    UINT64 InterruptShadow;      // +0x068
    UINT64 ExitCode;             // +0x070  <-- Exit reason
    UINT64 ExitInfo1;            // +0x078
    UINT64 ExitInfo2;            // +0x080
    UINT64 ExitIntInfo;          // +0x088
    UINT64 NpEnable;             // +0x090
    UINT64 AvicApicBar;          // +0x098
    UINT64 GhcbPa;               // +0x0A0
    UINT64 EventInj;             // +0x0A8
    UINT64 NCr3;                 // +0x0B0  <-- NPT CR3
    UINT64 LbrVirtualizationEnable; // +0x0B8
    UINT64 VmcbClean;            // +0x0C0
    UINT64 NextRip;              // +0x0C8
    UINT8  NumberOfBytesFetched; // +0x0D0
    UINT8  GuestInstructionBytes[15]; // +0x0D1
    // ... more fields
} VMCB_CONTROL_AREA;

typedef struct _VMCB_STATE_SAVE_AREA {
    // Segment registers at various offsets
    UINT64 Rax;                  // +0x5F8
    // ... 
} VMCB_STATE_SAVE_AREA;

typedef struct _VMCB {
    VMCB_CONTROL_AREA Control;
    UINT8 Reserved[0x400 - sizeof(VMCB_CONTROL_AREA)];
    VMCB_STATE_SAVE_AREA State;
} VMCB, *PVMCB;
#pragma pack(pop)

// AMD SVM exit codes
#define SVM_EXIT_CPUID          0x72
#define SVM_EXIT_MSR            0x7C
#define SVM_EXIT_NPF            0x400  // Nested Page Fault
```

### 2.3 AMD VMexit Handler

```cpp
// OmbraPayload/Virt/Amd/SvmHandler.cpp

void HyperVBackdoor_AMD(void* GuestState, void* Arg2) {
    // For AMD, GuestState points to VMCB or saved state
    // This depends on how Hyper-V hvax64.exe structures things
    
    // Get VMCB (may need to find via pattern scan or context)
    PVMCB Vmcb = GetCurrentVmcb();
    if (!Vmcb) {
        // Fall through to original handler
        goto CallOriginal;
    }
    
    UINT64 ExitCode = Vmcb->Control.ExitCode;
    
    // Check for our backdoor
    if (ExitCode == SVM_EXIT_CPUID) {
        // AMD stores guest registers differently
        PGUEST_STATE_AMD GuestRegs = GetAmdGuestRegs(GuestState);
        
        if (GuestRegs->R10 == OMBRA_MAGIC) {
            HandleBackdoorCommand_AMD(Vmcb, GuestRegs);
            
            // Apply TSC compensation for AMD
            ApplyTscCompensation_AMD(Vmcb);
            
            // Skip original handler
            return;
        }
    }
    
    // Apply timing compensation even for non-backdoor exits
    ApplyTscCompensation_AMD(Vmcb);
    
CallOriginal:
    CallOriginalHandler(GuestState, Arg2);
}

void ApplyTscCompensation_AMD(PVMCB Vmcb) {
    // AMD uses TscOffset field in VMCB control area
    UINT64 CurrentOffset = Vmcb->Control.TscOffset;
    
    // Get expected overhead (AMD typically higher: 1400-1800 cycles)
    UINT64 Compensation = 1600 - 150;  // Overhead - native CPUID time
    
    // Write compensated offset
    Vmcb->Control.TscOffset = CurrentOffset - Compensation;
}
```

---

## PHASE 3: DETECTION EVASION

### 3.1 VMX Instruction Injection (#UD)

EasyAntiCheat tests vmread at Ring 0. Must inject #UD:

```cpp
void HandleVmxInstruction(PGUEST_STATE GuestState, UINT32 ExitReason) {
    // All VMX instructions should inject #UD
    // This makes it appear no hypervisor is present
    
    UINT32 InterruptInfo = 0;
    InterruptInfo |= 6;          // Vector 6 = #UD
    InterruptInfo |= (3 << 8);   // Hardware exception
    InterruptInfo |= (1 << 31);  // Valid
    
    __vmx_vmwrite(VMCS_ENTRY_INTERRUPTION_INFO, InterruptInfo);
    
    // Don't advance RIP - fault re-executes instruction
}

// Add to main vmexit dispatcher
switch (ExitReason) {
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
        HandleVmxInstruction(GuestState, ExitReason);
        return;
}
```

### 3.2 Trap Flag Handling

Fix #DB exception delivery timing:

```cpp
void HandleCpuidWithTrapFlag(PGUEST_STATE GuestState) {
    // Check if EFLAGS.TF was set
    UINT64 Rflags;
    __vmx_vmread(VMCS_GUEST_RFLAGS, &Rflags);
    
    if (Rflags & 0x100) {  // TF bit
        // Must inject #DB AFTER advancing RIP
        // Not before (which would be wrong)
        
        // First, emulate CPUID
        int regs[4];
        __cpuidex(regs, (int)GuestState->Rax, (int)GuestState->Rcx);
        GuestState->Rax = regs[0];
        GuestState->Rbx = regs[1];
        GuestState->Rcx = regs[2];
        GuestState->Rdx = regs[3];
        
        // Advance RIP
        UINT64 InsnLen;
        __vmx_vmread(VMCS_EXIT_INSTRUCTION_LENGTH, &InsnLen);
        UINT64 Rip;
        __vmx_vmread(VMCS_GUEST_RIP, &Rip);
        __vmx_vmwrite(VMCS_GUEST_RIP, Rip + InsnLen);
        
        // Clear TF before injection
        Rflags &= ~0x100;
        __vmx_vmwrite(VMCS_GUEST_RFLAGS, Rflags);
        
        // NOW inject #DB (after RIP advance)
        UINT32 InterruptInfo = 0;
        InterruptInfo |= 1;          // Vector 1 = #DB
        InterruptInfo |= (3 << 8);   // Hardware exception
        InterruptInfo |= (1 << 31);  // Valid
        
        __vmx_vmwrite(VMCS_ENTRY_INTERRUPTION_INFO, InterruptInfo);
        
        // Set pending debug exceptions
        __vmx_vmwrite(VMCS_GUEST_PENDING_DEBUG_EXCEPTIONS, 0x4000);
    }
}
```

### 3.3 Hide Hypervisor CPUID Bit

```cpp
void HandleCpuid(PGUEST_STATE GuestState) {
    UINT32 Leaf = (UINT32)GuestState->Rax;
    UINT32 Subleaf = (UINT32)GuestState->Rcx;
    
    int regs[4];
    __cpuidex(regs, Leaf, Subleaf);
    
    // Check for backdoor
    if (GuestState->R10 == OMBRA_MAGIC) {
        HandleBackdoorCommand(GuestState);
        return;
    }
    
    // Hide hypervisor presence
    if (Leaf == 1) {
        // Clear hypervisor present bit (ECX bit 31)
        // WARNING: This breaks Windows Hyper-V awareness!
        // Only do this if you want to hide from detection
        // regs[2] &= ~(1 << 31);
        
        // Alternative: Leave it set (Hyper-V is expected)
    }
    
    if (Leaf == 0x40000000) {
        // This leaf reveals hypervisor vendor
        // Return "Microsoft Hv" to appear as normal Hyper-V
        regs[0] = 0x40000006;  // Max leaf
        regs[1] = 0x7263694D;  // "Micr"
        regs[2] = 0x666F736F;  // "osof"
        regs[3] = 0x76482074;  // "t Hv"
    }
    
    GuestState->Rax = regs[0];
    GuestState->Rbx = regs[1];
    GuestState->Rcx = regs[2];
    GuestState->Rdx = regs[3];
    
    AdvanceGuestRip();
}
```

---

## PHASE 4: VMFUNC/VE FUTURE PROOFING

### 4.1 Check Hardware Support

```cpp
BOOLEAN CanUseVmfunc() {
    // Check VMFUNC support
    int regs[4];
    __cpuidex(regs, 7, 0);
    BOOLEAN VmfuncSupported = (regs[2] & (1 << 13)) != 0;
    
    // Check execute-only EPT support
    UINT64 EptVpidCap = __readmsr(IA32_VMX_EPT_VPID_CAP);
    BOOLEAN ExecuteOnlySupported = (EptVpidCap & 1) != 0;
    
    return VmfuncSupported && ExecuteOnlySupported;
}

BOOLEAN CanUseVe() {
    // Check #VE support (requires Broadwell+)
    int regs[4];
    __cpuidex(regs, 7, 0);
    return (regs[2] & (1 << 10)) != 0;
}
```

### 4.2 Enable VMFUNC in VMCS

```cpp
void EnableVmfunc() {
    if (!CanUseVmfunc()) return;
    
    // Read secondary controls
    UINT64 SecondaryControls;
    __vmx_vmread(VMCS_SECONDARY_PROCESSOR_BASED_VM_EXEC_CONTROLS, &SecondaryControls);
    
    // Enable VMFUNC (bit 13)
    SecondaryControls |= (1 << 13);
    __vmx_vmwrite(VMCS_SECONDARY_PROCESSOR_BASED_VM_EXEC_CONTROLS, SecondaryControls);
    
    // Enable EPTP switching (function 0)
    __vmx_vmwrite(VMCS_VMFUNC_CONTROLS, 1);
    
    // Setup EPTP list (must allocate and populate)
    // ... see VMFUNC_VE_IMPLEMENTATION.md for details
}
```

---

## PHASE 5: TESTING & VALIDATION

### 5.1 Timing Test (pafish)

```cpp
// Test code to run in guest
BOOLEAN TestCpuidTiming() {
    const int ITERATIONS = 10000;
    UINT64 TotalCycles = 0;
    
    for (int i = 0; i < ITERATIONS; i++) {
        UINT64 Start = __rdtsc();
        
        int regs[4];
        __cpuid(regs, 0);
        
        UINT64 End = __rdtsc();
        TotalCycles += (End - Start);
    }
    
    UINT64 AvgCycles = TotalCycles / ITERATIONS;
    
    // Native: ~100-200 cycles
    // VM with compensation: ~150-250 cycles
    // VM without compensation: ~1000-5000 cycles
    
    // BattlEye threshold: ~750 cycles
    return AvgCycles < 750;
}
```

### 5.2 Self-Test Commands

Add diagnostic commands to backdoor:

```cpp
case CMD_SELF_TEST:
    // Run internal tests
    Result.TimingTestPass = TestCpuidTiming();
    Result.VmfuncAvailable = CanUseVmfunc();
    Result.VeAvailable = CanUseVe();
    Result.ExecuteOnlyAvailable = IsExecuteOnlySupported();
    Result.CpuVendor = DetectCpuVendor();
    break;

case CMD_GET_TIMING_STATE:
    // Return timing compensation state
    UINT32 CpuIndex = GetCurrentCpuIndex();
    Result.AccumulatedOverhead = g_TimingState[CpuIndex].AccumulatedOverhead;
    Result.CurrentTscOffset = VmcsRead(VMCS_TSC_OFFSET);
    break;
```

---

## IMPLEMENTATION PRIORITY

### High Priority (Detection Bypass)
1. ✅ TSC offset compensation in VMexit handler
2. ✅ APERF/MPERF MSR interception  
3. ✅ VMX instruction #UD injection
4. ✅ Trap flag proper handling

### Medium Priority (Completeness)
5. ⬜ AMD SVM full implementation
6. ⬜ NPT hook support
7. ⬜ Cross-core TSC synchronization

### Low Priority (Future)
8. ⬜ VMFUNC/VE zero-exit hooks
9. ⬜ EPT split-view memory hiding
10. ⬜ Boot integrity spoofing

---

## FILES TO MODIFY

```
OmbraPayload/
├── Entry.cpp           # Add vendor dispatch, timing hooks
├── EntryAsm.asm        # Add TSC capture at entry
├── Timing.cpp          # NEW: TSC compensation logic
├── Timing.h            # NEW: Timing state structures
├── Virt/
│   ├── Intel/
│   │   └── VmxHandler.cpp  # Intel-specific handler
│   └── Amd/
│       ├── Vmcb.h          # NEW: VMCB structures
│       └── SvmHandler.cpp  # NEW: AMD handler
└── Memory/
    └── Paging/
        └── Npt.cpp     # Complete NPT implementation
```

---

*Reference: 12-OMBRA-AUDIT.md, Intel SDM, AMD APM, secret.club*
