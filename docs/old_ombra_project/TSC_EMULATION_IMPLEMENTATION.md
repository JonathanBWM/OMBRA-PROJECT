# TSC EMULATION IMPLEMENTATION GUIDE
## Defeating RDTSC/CPUID/RDTSC Timing Attacks

---

## Overview

This document provides concrete implementation patterns for TSC (Time Stamp Counter) emulation to defeat timing-based hypervisor detection used by BattlEye and other anti-cheats.

---

## 1. THE PROBLEM

When BattlEye runs its timing check:

```
RDTSC           ; t1 = current timestamp
CPUID           ; <-- This causes VM-exit, takes 1000-5000 cycles
RDTSC           ; t2 = current timestamp  
; delta = t2 - t1
; If delta > ~750 cycles, hypervisor detected
```

On bare metal: CPUID takes ~100-200 cycles
In hypervisor: CPUID causes VM-exit, takes 1000-5000 cycles

**We need to make (t2 - t1) appear to be ~100-200 cycles even though VM-exit took much longer.**

---

## 2. VMCS TSC OFFSET FIELD

Intel provides a TSC_OFFSET field in VMCS:

```cpp
// VMCS field for TSC offsetting
#define VMCS_TSC_OFFSET  0x00002010

// When guest reads TSC:
// returned_value = actual_TSC + TSC_OFFSET

// Enable TSC offsetting in primary proc-based controls
UINT32 controls = VmcsRead32(VMCS_PRIMARY_PROC_BASED_CONTROLS);
controls |= (1 << 3);  // Use TSC offsetting
VmcsWrite32(VMCS_PRIMARY_PROC_BASED_CONTROLS, controls);
```

**Problem:** TSC_OFFSET is static. We need DYNAMIC adjustment based on VM-exit overhead.

---

## 3. DYNAMIC TSC COMPENSATION

### 3.1 Per-VCPU Timing State

```cpp
typedef struct _VCPU_TIMING_STATE {
    // Accumulated VM-exit overhead to hide
    volatile UINT64 AccumulatedOverhead;
    
    // Last real TSC value we saw
    UINT64 LastRealTsc;
    
    // Are we currently in a timing-sensitive section?
    BOOLEAN InTimingCritical;
    
    // TSC at start of current VM-exit
    UINT64 ExitEntryTsc;
    
    // Lock for multi-processor safety
    KSPIN_LOCK Lock;
    
} VCPU_TIMING_STATE, *PVCPU_TIMING_STATE;

typedef struct _VCPU {
    // ... other fields ...
    VCPU_TIMING_STATE Timing;
} VCPU, *PVCPU;
```

### 3.2 VM-Exit Entry Point

```cpp
VOID VmExitHandler(PVCPU Vcpu) {
    // IMMEDIATELY capture entry TSC
    Vcpu->Timing.ExitEntryTsc = __rdtsc();
    
    // Get exit reason
    UINT32 ExitReason = VmcsRead32(VMCS_EXIT_REASON) & 0xFFFF;
    
    // Dispatch to specific handler
    switch (ExitReason) {
        case EXIT_REASON_CPUID:
            HandleCpuid(Vcpu);
            break;
        case EXIT_REASON_RDTSC:
            HandleRdtsc(Vcpu);
            break;
        case EXIT_REASON_RDTSCP:
            HandleRdtscp(Vcpu);
            break;
        case EXIT_REASON_MSR_READ:
            HandleMsrRead(Vcpu);
            break;
        // ... other handlers ...
    }
    
    // Calculate and accumulate overhead BEFORE resuming
    UINT64 ExitEndTsc = __rdtsc();
    UINT64 ExitOverhead = ExitEndTsc - Vcpu->Timing.ExitEntryTsc;
    
    // Only accumulate if this was an instruction that should be "fast"
    if (ExitReason == EXIT_REASON_CPUID || 
        ExitReason == EXIT_REASON_MSR_READ) {
        
        // Subtract expected native execution time
        // CPUID: ~150 cycles, MSR read: ~30 cycles
        UINT64 NativeTime = (ExitReason == EXIT_REASON_CPUID) ? 150 : 30;
        
        if (ExitOverhead > NativeTime) {
            InterlockedAdd64(&Vcpu->Timing.AccumulatedOverhead, 
                            ExitOverhead - NativeTime);
        }
    }
    
    // Resume guest
    VmResume();
}
```

### 3.3 RDTSC Handler

```cpp
VOID HandleRdtsc(PVCPU Vcpu) {
    UINT64 RealTsc = __rdtsc();
    UINT64 FakeTsc;
    
    // Subtract accumulated overhead
    UINT64 Overhead = Vcpu->Timing.AccumulatedOverhead;
    
    if (RealTsc > Overhead) {
        FakeTsc = RealTsc - Overhead;
    } else {
        // Prevent underflow - just use real TSC
        FakeTsc = RealTsc;
    }
    
    // Split into EDX:EAX
    Vcpu->Context.Rax = FakeTsc & 0xFFFFFFFF;
    Vcpu->Context.Rdx = FakeTsc >> 32;
    
    // Advance RIP
    VmcsWrite(VMCS_GUEST_RIP, VmcsRead(VMCS_GUEST_RIP) + 
              VmcsRead32(VMCS_EXIT_INSTRUCTION_LENGTH));
}
```

### 3.4 CPUID Handler

```cpp
VOID HandleCpuid(PVCPU Vcpu) {
    INT32 Regs[4] = {0};
    UINT32 Leaf = (UINT32)Vcpu->Context.Rax;
    UINT32 SubLeaf = (UINT32)Vcpu->Context.Rcx;
    
    // Execute real CPUID
    __cpuidex(Regs, Leaf, SubLeaf);
    
    // ===== CRITICAL MODIFICATIONS =====
    
    // Leaf 0x1: Clear hypervisor present bit
    if (Leaf == 1) {
        Regs[2] &= ~(1 << 31);  // ECX bit 31 = hypervisor present
    }
    
    // Leaf 0x40000000: Return same as highest standard leaf
    if (Leaf == 0x40000000) {
        INT32 MaxLeaf[4];
        __cpuidex(MaxLeaf, 0, 0);
        __cpuidex(Regs, MaxLeaf[0], 0);
    }
    
    // Leaf 0x40000001+: Return as if invalid
    if (Leaf > 0x40000000 && Leaf <= 0x400000FF) {
        INT32 MaxLeaf[4];
        __cpuidex(MaxLeaf, 0, 0);
        __cpuidex(Regs, MaxLeaf[0], 0);
    }
    
    // Set return values
    Vcpu->Context.Rax = Regs[0];
    Vcpu->Context.Rbx = Regs[1];
    Vcpu->Context.Rcx = Regs[2];
    Vcpu->Context.Rdx = Regs[3];
    
    // Advance RIP
    VmcsWrite(VMCS_GUEST_RIP, VmcsRead(VMCS_GUEST_RIP) + 
              VmcsRead32(VMCS_EXIT_INSTRUCTION_LENGTH));
}
```

---

## 4. APERF/MPERF COMPENSATION

APERF (Actual Performance) counter is harder to spoof because it only counts cycles when CPU is actually executing in C0 state.

### 4.1 MSR Bitmap Setup

```cpp
VOID SetupMsrBitmap(PVCPU Vcpu) {
    PUCHAR MsrBitmap = Vcpu->MsrBitmap;
    
    // IA32_APERF = 0xE8
    // IA32_MPERF = 0xE7
    
    // Set read bits for APERF and MPERF
    // Low MSRs (0x0 - 0x1FFF): bytes 0-1023 for read, 2048-3071 for write
    
    UINT32 ApertOffset = 0xE8 / 8;
    UINT32 ApertBit = 0xE8 % 8;
    MsrBitmap[ApertOffset] |= (1 << ApertBit);  // Read intercept
    
    UINT32 MperfOffset = 0xE7 / 8;
    UINT32 MperfBit = 0xE7 % 8;
    MsrBitmap[MperfOffset] |= (1 << MperfBit);  // Read intercept
}
```

### 4.2 APERF Handler

```cpp
VOID HandleAperfRead(PVCPU Vcpu) {
    // Read real APERF
    UINT64 RealAperf = __readmsr(IA32_APERF);
    
    // Apply same compensation as TSC
    // APERF counts actual cycles, so overhead should be subtracted
    UINT64 Overhead = Vcpu->Timing.AccumulatedOverhead;
    UINT64 FakeAperf;
    
    if (RealAperf > Overhead) {
        FakeAperf = RealAperf - Overhead;
    } else {
        FakeAperf = RealAperf;
    }
    
    Vcpu->Context.Rax = FakeAperf & 0xFFFFFFFF;
    Vcpu->Context.Rdx = FakeAperf >> 32;
    
    // Advance RIP
    AdvanceGuestRip(Vcpu);
}
```

---

## 5. TRAP FLAG HANDLING

Critical bug in most hypervisors: #DB exception delivered on wrong instruction.

### 5.1 Correct Implementation

```cpp
VOID HandleCpuidWithTrapFlag(PVCPU Vcpu) {
    // Check if Trap Flag was set when CPUID executed
    UINT64 Rflags = VmcsRead(VMCS_GUEST_RFLAGS);
    BOOLEAN TrapFlagWasSet = (Rflags & 0x100) != 0;
    
    // Execute CPUID emulation as normal
    HandleCpuid(Vcpu);
    
    // If TF was set, we need to inject #DB
    if (TrapFlagWasSet) {
        // Clear TF in guest RFLAGS
        VmcsWrite(VMCS_GUEST_RFLAGS, Rflags & ~0x100);
        
        // Inject debug exception
        UINT32 InterruptInfo = 0;
        InterruptInfo |= 1;                    // Vector = 1 (#DB)
        InterruptInfo |= (3 << 8);             // Type = Hardware exception
        InterruptInfo |= (1 << 31);            // Valid
        
        VmcsWrite32(VMCS_ENTRY_INTERRUPTION_INFO, InterruptInfo);
        
        // Set DR6 to indicate single-step
        UINT64 Dr6 = __readdr(6);
        Dr6 |= (1 << 14);  // BS (single-step) bit
        __writedr(6, Dr6);
    }
}
```

### 5.2 Pending Debug Exceptions

```cpp
// Also handle pending debug exceptions from VMCS
VOID CheckPendingDebugExceptions(PVCPU Vcpu) {
    UINT64 PendingDebug = VmcsRead(VMCS_GUEST_PENDING_DEBUG_EXCEPTIONS);
    
    if (PendingDebug & (1 << 14)) {  // BS bit
        // Single-step pending, inject #DB
        InjectDebugException(Vcpu);
        
        // Clear pending
        VmcsWrite(VMCS_GUEST_PENDING_DEBUG_EXCEPTIONS, 0);
    }
}
```

---

## 6. VMX INSTRUCTION HANDLING

All VMX instructions must fault in guest if CR4.VMXE is supposed to be 0.

```cpp
VOID HandleVmxInstruction(PVCPU Vcpu, UINT32 ExitReason) {
    // Always inject #UD for VMX instructions
    // This matches behavior of real hardware with VMXE=0
    
    UINT32 InterruptInfo = 0;
    InterruptInfo |= 6;                    // Vector = 6 (#UD)
    InterruptInfo |= (3 << 8);             // Type = Hardware exception
    InterruptInfo |= (1 << 31);            // Valid
    
    VmcsWrite32(VMCS_ENTRY_INTERRUPTION_INFO, InterruptInfo);
    
    // Do NOT advance RIP - exception is fault-like
}
```

---

## 7. CROSS-CORE SYNCHRONIZATION

TSC must be consistent across cores or timing attacks can detect differences.

```cpp
typedef struct _GLOBAL_TIMING_STATE {
    volatile UINT64 GlobalTscOffset;
    KSPIN_LOCK Lock;
} GLOBAL_TIMING_STATE;

GLOBAL_TIMING_STATE g_Timing;

VOID SynchronizeTscOffset(PVCPU Vcpu) {
    KIRQL OldIrql;
    KeAcquireSpinLock(&g_Timing.Lock, &OldIrql);
    
    // Find maximum accumulated overhead across all VCPUs
    UINT64 MaxOverhead = 0;
    for (UINT32 i = 0; i < g_VcpuCount; i++) {
        if (g_Vcpus[i].Timing.AccumulatedOverhead > MaxOverhead) {
            MaxOverhead = g_Vcpus[i].Timing.AccumulatedOverhead;
        }
    }
    
    // Set global offset
    g_Timing.GlobalTscOffset = MaxOverhead;
    
    // Update all VCPUs to use same offset
    for (UINT32 i = 0; i < g_VcpuCount; i++) {
        g_Vcpus[i].Timing.AccumulatedOverhead = MaxOverhead;
    }
    
    KeReleaseSpinLock(&g_Timing.Lock, OldIrql);
}
```

---

## 8. TUNING CONSTANTS

These values may need adjustment based on specific CPU:

```cpp
// Average native CPUID execution time (cycles)
#define NATIVE_CPUID_CYCLES         150

// Average native MSR read time (cycles)
#define NATIVE_MSR_READ_CYCLES      30

// Minimum VM-exit overhead (cycles)
#define MIN_VMEXIT_OVERHEAD         1200

// Maximum VM-exit overhead (cycles)
#define MAX_VMEXIT_OVERHEAD         5000

// BattlEye detection threshold (cycles)
#define BATTLEYE_THRESHOLD          750

// Target CPUID visible time (cycles)
// Should be < BATTLEYE_THRESHOLD but realistic
#define TARGET_CPUID_CYCLES         200
```

---

## 9. TESTING

```cpp
BOOLEAN TestTimingBypass() {
    UINT64 TotalCycles = 0;
    INT32 Regs[4];
    
    for (int i = 0; i < 10000; i++) {
        UINT64 Start = __rdtsc();
        __cpuidex(Regs, 0, 0);
        UINT64 End = __rdtsc();
        TotalCycles += (End - Start);
    }
    
    UINT64 Average = TotalCycles / 10000;
    
    DbgPrint("[TSC Test] Average CPUID cycles: %llu\n", Average);
    DbgPrint("[TSC Test] BattlEye threshold: 750\n");
    DbgPrint("[TSC Test] Result: %s\n", 
             Average < 750 ? "PASS" : "FAIL");
    
    return Average < 750;
}
```

---

## 10. KNOWN ISSUES & EDGE CASES

### 10.1 TSC Invariant
Modern CPUs have invariant TSC that runs at constant rate. Ensure compensation doesn't break time-dependent code.

### 10.2 SMI (System Management Interrupt)
SMIs can cause additional timing anomalies. Consider masking around critical sections.

### 10.3 NMI (Non-Maskable Interrupt)
NMIs during timing-critical code can add extra cycles. Handle gracefully.

### 10.4 Nested Virtualization
If running under another hypervisor, timing is already affected. Detection may be unavoidable.

---

*Implementation based on secret.club research, Intel SDM, and analysis of public hypervisor projects.*
