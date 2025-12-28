# Core Hypervisor Concepts

## Abstract Reference for AI Agents

This document provides conceptual understanding for implementing
Ring -1 hypervisor functionality. Each concept is platform-agnostic
thinking that maps to specific Intel/AMD implementations.

---

## 1. Privilege Hierarchy

```
Ring 3 (User Mode)
    ↓ syscall/interrupt
Ring 0 (Kernel Mode)  
    ↓ VMExit (hardware trigger)
Ring -1 (Hypervisor / VMX Root)
    ↓ Physical hardware
```

**Key Insight**: Ring -1 is invisible to everything above it.
Ring 0 security software (PatchGuard, anti-cheats) cannot
detect properly implemented Ring -1 operations.

---

## 2. Two-Phase Address Translation

```
Guest Virtual Address (GVA)
    ↓ Guest Page Tables (controlled by guest OS)
Guest Physical Address (GPA)
    ↓ Extended Page Tables (controlled by hypervisor)
Host Physical Address (HPA)
    ↓ Physical RAM
```

**Key Insight**: The hypervisor controls GPA→HPA translation.
This enables:
- Memory hiding (don't map certain GPAs)
- Shadow pages (different HPA for read vs execute)
- Access interception (permission violations trigger VMExit)

---

## 3. VMExit/VMResume Cycle

```
┌──────────────────────────────────────┐
│         Guest Execution              │
│  (Ring 0/Ring 3, thinks it's alone)  │
└─────────────────┬────────────────────┘
                  │ Intercepted operation
                  ↓ (CPUID, MSR access, etc.)
┌──────────────────────────────────────┐
│            VMExit                    │
│  Hardware saves guest state          │
│  Loads host state                    │
│  Jumps to hypervisor handler         │
└─────────────────┬────────────────────┘
                  │
                  ↓
┌──────────────────────────────────────┐
│        Hypervisor Handler            │
│  Read exit reason                    │
│  Modify guest state if needed        │
│  Optionally modify what guest sees   │
└─────────────────┬────────────────────┘
                  │
                  ↓ VMRESUME
┌──────────────────────────────────────┐
│         Guest Execution              │
│  (Continues, unaware of exit)        │
└──────────────────────────────────────┘
```

**Key Insight**: Guest is unaware VMExit occurred. We can
intercept any operation, modify results, and return.

---

## 4. Shadow Page Hook Technique

```
Guest Physical Address 0x1000
        │
        ├─── READ access ────→ Original page (clean code)
        │                      Scanner sees unmodified bytes
        │
        └─── EXECUTE access ─→ Shadow page (hooked code)
                               Our hook runs instead

EPT Configuration:
  R=0, W=0, X=1 (execute-only)
  PFN → shadow page physical address
  
On read: EPT violation → handle → show original
On exec: Allowed → runs shadow page with hook
```

**Key Insight**: Code integrity scanners read memory, they don't
execute it. We show clean bytes on read, run hooks on execute.

---

## 5. CPUID Stealth

```
Guest executes CPUID(leaf=1)
    ↓ VMExit
Hypervisor intercepts
    ↓
Execute real CPUID
    ↓
Modify results:
  - Clear ECX[31] (hypervisor present bit)
  - Clear ECX[5] (VMX capability)
    ↓
Return modified results to guest
    ↓ VMRESUME
Guest receives: "No hypervisor detected"
```

**Key Insight**: We control what software sees when it probes
for virtualization. We can hide or masquerade.

---

## 6. MSR Virtualization

```
Guest executes RDMSR(0x3A)  [IA32_FEATURE_CONTROL]
    ↓ VMExit (MSR in bitmap)
Hypervisor intercepts
    ↓
Real value: VMX enabled bits set
    ↓
Return: VMX enabled bits cleared
    ↓ VMRESUME
Guest sees: "VMX not available"
```

**Key Insight**: Certain MSRs reveal VMX/SVM state. We intercept
reads and return modified values hiding virtualization evidence.

---

## 7. Timing Compensation

```
Problem:
  t1 = RDTSC
  CPUID     ← VMExit takes ~1500 cycles
  t2 = RDTSC
  delta = t2 - t1  ← ~1500+ cycles reveals virtualization

Solution (TSC Offsetting):
  VMExit starts:  record start_tsc
  VMExit ends:    elapsed = rdtsc - start_tsc
                  TSC_OFFSET -= elapsed
                  
  Guest sees: constant time (VMExit overhead hidden)
```

**Key Insight**: VMExits take measurable time. We subtract that
time from what guest sees via hardware TSC offset feature.

---

## 8. Control Register Shadow

```
Guest writes: CR4 = value | VMXE
    ↓ VMExit (CR4 masked)
Hypervisor:
  Actual CR4 = value | VMXE  (VMXE required for VMX)
  CR4_SHADOW = value & ~VMXE (what guest sees)
    ↓ VMRESUME

Guest reads: CR4 → returns CR4_SHADOW (VMXE hidden)
```

**Key Insight**: Hardware shadows let us lie about CR values.
Guest thinks VMXE is clear, but it's actually set.

---

## 9. Exception/Interrupt Injection

```
Scenario: Guest executes VMXON (tries to enter VMX)

We want guest to see: #UD (undefined opcode)
  → "VMX not supported"

Hypervisor:
  Set VMCS_CTRL_VMENTRY_INT_INFO = #UD exception
  Set VMCS_CTRL_VMENTRY_EXCEPTION_CODE = 0
    ↓ VMRESUME
  
Guest receives #UD exception
Guest concludes: VMX instructions don't work
```

**Key Insight**: We can inject any exception into guest. Make
VMX instructions appear to #GP/#UD to hide VMX capability.

---

## 10. Hook Handler Pattern

```c
// Original function:
void TargetFunction() {
    <original code>
}

// After hooking:
void TargetFunction() {
    JMP HookHandler     // 14-byte inline hook
}

void HookHandler() {
    // Pre-hook logic (logging, modification, etc.)
    
    // Call original via trampoline
    result = Trampoline();
    
    // Post-hook logic
    return result;
}

void Trampoline() {
    <original first N bytes>    // Copied from target
    JMP TargetFunction+N        // Jump past our hook
}
```

**Key Insight**: Trampoline preserves original functionality.
We intercept, optionally modify, then call original.

---

## 11. Identity Mapping

```
Identity Map: GPA == HPA everywhere

GPA 0x00000000 → HPA 0x00000000
GPA 0x00001000 → HPA 0x00001000
GPA 0x12345000 → HPA 0x12345000
...

Why: Guest was using physical addresses before we arrived.
     Identity map preserves all existing memory references.
     
Hooks are modifications to this baseline:
  GPA 0x12345000 → HPA 0x99999000 (shadow page)
```

**Key Insight**: Start with identity map (everything works),
then selectively modify for hooks/protection.

---

## 12. Large Page Splitting

```
Before: 2MB page covering 0x200000-0x3FFFFF
        Single EPT PDE entry
        
Need: 4KB granularity for hook at 0x234000

After:  Split into 512 × 4KB pages
        EPT PDE → PT with 512 PTEs
        Each 4KB page individually controllable
        
        PTE[0x34] (page at 0x234000) → shadow page
        All other PTEs → original mappings
```

**Key Insight**: Large pages are efficient but coarse. Split
only when fine-grained control needed (hooks, protection).

---

## 13. Monitor Trap Flag (MTF)

```
Shadow hook flow with integrity check:

1. Guest READS hooked page
   → EPT violation (R=0 on execute-only page)
   
2. Handler: Switch to RW view, enable MTF

3. VMRESUME - guest executes one instruction (the read)

4. MTF trap fires (VMExit after 1 instruction)

5. Handler: Switch back to X-only view, disable MTF

6. VMRESUME - back to normal operation
```

**Key Insight**: MTF provides single-step capability. Essential
for safely handling read accesses to execute-only pages.

---

## 14. TLB Management

```
Problem: CPU caches address translations
         After changing EPT entry, old mapping cached
         
Solution: INVEPT instruction invalidates EPT cache

Types:
  Single-context: Invalidate specific EPTP
  All-contexts: Invalidate all EPTPs (nuclear option)

When required:
  - After modifying any EPT entry
  - After changing page permissions
  - After switching shadow page mappings
```

**Key Insight**: Memory changes require TLB flush. Miss this
and guest uses stale translations → crash or wrong page.

---

## Summary: The Stealth Equation

```
Stealth = 
    Hide CPUID indicators
  + Hide MSR indicators  
  + Hide CR4.VMXE via shadow
  + Compensate timing overhead
  + Shadow pages hide code changes
  + Inject exceptions for VMX instructions
  + Clean all forensic traces
```

Each layer defeats a different detection method. Complete
stealth requires addressing ALL vectors.
