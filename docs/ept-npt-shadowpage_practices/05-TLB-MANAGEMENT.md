# TLB Management

> **Purpose**: TLB invalidation, VPID/ASID usage, and ensuring EPT/NPT page table changes take effect correctly.

> **Implementation target**: OmbraPayload (Ring -1, C++ with restrictions, BOTH Intel & AMD)

---

## Overview

The Translation Lookaside Buffer (TLB) caches address translations to avoid page table walks on every memory access. This creates a critical challenge for hypervisors: after modifying EPT/NPT entries, the CPU may continue using stale cached translations until explicitly invalidated.

Failure to properly manage TLB state leads to:
- Wrong physical pages accessed
- Incorrect permissions enforced
- Hooks not activating
- Random crashes from stale translations

This document covers TLB invalidation mechanisms for both Intel (INVEPT, INVVPID) and AMD (TLB_CONTROL, INVLPGA), plus cross-CPU synchronization patterns.

---

## Why TLB Matters

### The Caching Problem

```
WITHOUT TLB INVALIDATION:

Time 0: GPA 0x1000 → HPA 0x2000 (EPT entry)
        TLB caches: GPA 0x1000 → HPA 0x2000

Time 1: Hypervisor modifies EPT: GPA 0x1000 → HPA 0x3000
        TLB STILL HAS: GPA 0x1000 → HPA 0x2000  ← STALE!

Time 2: Guest accesses GPA 0x1000
        CPU uses cached translation → accesses HPA 0x2000
        WRONG PAGE ACCESSED!
```

### What Gets Cached

The TLB caches multiple types of translations:
- Guest virtual → Guest physical (guest page tables)
- Guest physical → Host physical (EPT/NPT)
- Combined translations (GVA → HPA)

### When Invalidation is Required

1. After modifying any EPT/NPT entry
2. After changing page permissions (R/W/X)
3. After switching page backing (shadow pages)
4. After installing or removing hooks
5. After splitting large pages
6. When synchronizing across CPUs

---

## Intel INVEPT

### Instruction Overview

`INVEPT` invalidates EPT-based translations in the TLB.

```cpp
// INVEPT descriptor
struct InveptDescriptor {
    u64 eptp;      // EPT pointer value
    u64 reserved;  // Must be 0
};

// INVEPT types
enum InveptType : u64 {
    InveptSingleContext = 1,  // Invalidate specific EPTP
    InveptAllContexts = 2     // Invalidate all EPTPs
};
```

### Inline Assembly Implementation

```cpp
enum class VmxError : u8 {
    Success = 0,
    FailWithStatus = 1,
    FailNoStatus = 2
};

inline VmxError Invept(InveptType type, const InveptDescriptor* desc) {
    u8 error;
    asm volatile(
        "invept %[desc], %[type]\n\t"
        "setna %[error]\n\t"
        : [error] "=r" (error)
        : [type] "r" ((u64)type),
          [desc] "m" (*desc)
        : "cc", "memory"
    );
    return error ? VmxError::FailWithStatus : VmxError::Success;
}
```

### Single-Context Invalidation

Invalidates TLB entries for a specific EPTP. Faster than all-contexts.

```cpp
void InvalidateEptSingleContext(u64 eptp_value) {
    InveptDescriptor desc = {};
    desc.eptp = eptp_value;
    desc.reserved = 0;

    VmxError result = Invept(InveptSingleContext, &desc);
    if (result != VmxError::Success) {
        // Handle error - should not happen in normal operation
    }
}
```

### All-Contexts Invalidation

Invalidates TLB entries for all EPTPs. Simpler but slower.

```cpp
void InvalidateEptAllContexts() {
    InveptDescriptor desc = {};
    desc.eptp = 0;  // Ignored for all-contexts
    desc.reserved = 0;

    Invept(InveptAllContexts, &desc);
}
```

### Checking INVEPT Support

```cpp
bool IsInveptSupported() {
    u64 ept_vpid_cap = __readmsr(MSR_IA32_VMX_EPT_VPID_CAP);

    bool invept_supported = (ept_vpid_cap >> 20) & 1;
    bool single_context = (ept_vpid_cap >> 25) & 1;
    bool all_contexts = (ept_vpid_cap >> 26) & 1;

    return invept_supported && (single_context || all_contexts);
}
```

---

## Intel VPID (Virtual Processor ID)

### Purpose

VPID allows TLB entries to be tagged with a processor ID, preventing unnecessary flushes on VM-exit/entry. Without VPID, the CPU flushes guest TLB entries on every transition.

### VPID Assignment

Each virtual CPU gets a unique VPID:

```cpp
void SetupVpid(VcpuContext* ctx, u16 vpid) {
    // VPID must be non-zero
    if (vpid == 0) vpid = 1;

    // Write to VMCS
    VmcsWrite16(VMCS_VIRTUAL_PROCESSOR_ID, vpid);

    // Enable VPID in secondary processor-based controls
    u64 proc_ctls2 = VmcsRead64(VMCS_SECONDARY_PROCESSOR_BASED_VM_EXEC_CONTROLS);
    proc_ctls2 |= (1 << 5);  // Enable VPID
    VmcsWrite64(VMCS_SECONDARY_PROCESSOR_BASED_VM_EXEC_CONTROLS, proc_ctls2);
}
```

### INVVPID Instruction

```cpp
struct InvvpidDescriptor {
    u16 vpid;
    u16 reserved1;
    u32 reserved2;
    u64 linear_address;
};

enum InvvpidType : u64 {
    InvvpidIndividualAddress = 0,        // Single address + VPID
    InvvpidSingleContext = 1,            // All addresses for VPID
    InvvpidAllContexts = 2,              // All VPIDs (not including 0)
    InvvpidSingleContextRetainingGlobal = 3  // All except global pages
};

inline VmxError Invvpid(InvvpidType type, const InvvpidDescriptor* desc) {
    u8 error;
    asm volatile(
        "invvpid %[desc], %[type]\n\t"
        "setna %[error]\n\t"
        : [error] "=r" (error)
        : [type] "r" ((u64)type),
          [desc] "m" (*desc)
        : "cc", "memory"
    );
    return error ? VmxError::FailWithStatus : VmxError::Success;
}
```

### When to Use INVVPID vs INVEPT

| Situation | Use |
|-----------|-----|
| Modified EPT entry | INVEPT |
| Modified guest page table | INVVPID |
| Changed EPTP | INVEPT |
| Need to flush specific linear address | INVVPID individual |
| VM-exit handler modified guest memory | INVVPID |

---

## AMD TLB Management

### VMCB TLB Control Field

AMD uses the VMCB TLB_CONTROL field (offset 0x058) to request TLB flushes on VMRUN.

```cpp
enum TlbControl : u8 {
    TlbControlDoNothing = 0,         // No flush
    TlbControlFlushAll = 1,          // Flush all TLB entries
    TlbControlReserved = 2,          // Reserved
    TlbControlFlushThisAsid = 3,     // Flush entries for this ASID
    TlbControlFlushNonGlobal = 7     // Flush non-global entries
};

void RequestTlbFlush(VMCB* vmcb, TlbControl type) {
    vmcb->control.tlb_control = type;
    // Will be applied on next VMRUN
}
```

### ASID (Address Space Identifier)

Similar to Intel's VPID, ASID tags TLB entries to avoid unnecessary flushes.

```cpp
void SetupAsid(VMCB* vmcb, u32 asid) {
    // ASID must be non-zero for nested paging
    if (asid == 0) asid = 1;

    vmcb->control.guest_asid = asid;
}

// Get max ASID from CPUID
u32 GetMaxAsid() {
    u32 eax, ebx, ecx, edx;
    __cpuid(0x8000000A, eax, ebx, ecx, edx);
    return ebx;  // Maximum ASID value
}
```

### INVLPGA Instruction

Invalidates a single page for a specific ASID.

```cpp
inline void Invlpga(u64 virtual_address, u32 asid) {
    asm volatile(
        "invlpga %[addr], %[asid]"
        :
        : [addr] "a" (virtual_address),
          [asid] "c" (asid)
        : "memory"
    );
}
```

**Note**: INVLPGA uses the virtual address and ASID, NOT the physical address. It's primarily for guest page table invalidation, not NPT.

### AMD NPT TLB Flush Pattern

For NPT modifications, use VMCB TLB_CONTROL:

```cpp
void InvalidateNpt(VMCB* vmcb) {
    // Request flush on next VMRUN
    vmcb->control.tlb_control = TlbControlFlushAll;
}

void InvalidateNptForAsid(VMCB* vmcb) {
    // Only flush entries for current ASID
    vmcb->control.tlb_control = TlbControlFlushThisAsid;
}
```

### VMCB Clean Bits

AMD also uses "clean bits" to indicate which VMCB fields haven't changed, allowing the processor to skip reloading them.

```cpp
// VMCB offset 0x0C0 - VMCB Clean Bits
enum VmcbCleanBits : u32 {
    CleanIntercepts = (1 << 0),
    CleanIopm = (1 << 1),
    CleanAsid = (1 << 2),
    CleanTpr = (1 << 3),
    CleanNp = (1 << 4),      // NPT - set to 0 if nCR3 changed
    CleanCrX = (1 << 5),
    CleanDrX = (1 << 6),
    CleanDt = (1 << 7),
    CleanSeg = (1 << 8),
    CleanCr2 = (1 << 9),
    CleanLbr = (1 << 10),
    CleanAvic = (1 << 11),
    CleanCet = (1 << 12)
};

void MarkNptDirty(VMCB* vmcb) {
    // Clear the NP clean bit - forces reload of nCR3
    vmcb->control.vmcb_clean &= ~CleanNp;
}
```

---

## Cross-CPU Synchronization

### The Multi-Core Problem

Hook installation must be atomic across all CPUs:

```
Without synchronization:
  CPU 0: Modifies EPT entry
  CPU 1: Still using stale TLB entry
  CPU 1: Accesses wrong page → undefined behavior

With synchronization:
  CPU 0: Modifies EPT entry
  CPU 0: Sends IPI to CPU 1
  CPU 1: Receives IPI, flushes TLB
  CPU 1: Now sees correct page
```

### IPI-Based Synchronization

**Using KeIpiGenericCall** (Windows):

```cpp
ULONG_PTR TlbFlushCallback(ULONG_PTR context) {
    // Each CPU executes this
    if (g_vendor == VendorIntel) {
        u64 eptp = VmcsRead64(VMCS_EPT_POINTER);
        InvalidateEptSingleContext(eptp);
    } else {
        // AMD: Mark for flush on next VMRUN
        // (Can't directly modify VMCB from IPI context)
        g_per_cpu_flush_pending[KeGetCurrentProcessorNumber()] = true;
    }
    return 0;
}

void BroadcastTlbFlush() {
    KeIpiGenericCall(TlbFlushCallback, 0);
}
```

**Using KeGenericCallDpc**:

```cpp
void TlbFlushDpc(KDPC* Dpc, PVOID Context, PVOID Arg1, PVOID Arg2) {
    // Execute flush
    if (g_vendor == VendorIntel) {
        InvalidateEptAllContexts();
    } else {
        // AMD: Set per-CPU flag
        g_per_cpu_flush_pending[KeGetCurrentProcessorNumber()] = true;
    }

    // Synchronize with other CPUs
    KeSignalCallDpcSynchronize(Arg2);
    KeSignalCallDpcDone(Arg1);
}

void BroadcastTlbFlushDpc() {
    KeGenericCallDpc(TlbFlushDpc, nullptr);
}
```

### Hypervisor-Level Synchronization

Within VMX root mode, use hypercalls to synchronize:

```cpp
void BroadcastFromVmxRoot(void (*function)(void*), void* context) {
    // For each CPU except current
    u32 current_cpu = GetCurrentCpu();

    for (u32 cpu = 0; cpu < g_cpu_count; cpu++) {
        if (cpu == current_cpu) continue;

        // Send NMI or use inter-processor interrupt
        // Target CPU's NMI handler calls function
        SendNmi(cpu, function, context);
    }

    // Execute locally
    function(context);

    // Wait for all CPUs to complete
    WaitForAllCpus();
}
```

---

## Unified Patterns

### Pattern: Vendor-Abstracted TLB Flush

```cpp
namespace Ombra::Tlb {

void InvalidatePage(void* state, u64 gpa) {
    if (g_vendor == VendorIntel) {
        u64 eptp = GetCurrentEptp();
        InveptDescriptor desc = { eptp, 0 };
        Invept(InveptSingleContext, &desc);
    } else {
        VMCB* vmcb = GetCurrentVmcb();
        vmcb->control.tlb_control = TlbControlFlushThisAsid;
    }
}

void InvalidateAll() {
    if (g_vendor == VendorIntel) {
        InveptDescriptor desc = { 0, 0 };
        Invept(InveptAllContexts, &desc);
    } else {
        VMCB* vmcb = GetCurrentVmcb();
        vmcb->control.tlb_control = TlbControlFlushAll;
    }
}

void BroadcastInvalidate() {
    // Execute on all CPUs
    for (u32 cpu = 0; cpu < g_cpu_count; cpu++) {
        ExecuteOnCpu(cpu, []() {
            InvalidateAll();
        });
    }
}

} // namespace Ombra::Tlb
```

### Pattern: Batched Invalidation

```cpp
class TlbBatch {
    bool m_dirty = false;
    u64 m_eptp = 0;

public:
    void MarkDirty() {
        m_dirty = true;
    }

    void Commit() {
        if (!m_dirty) return;

        if (g_vendor == VendorIntel) {
            InvalidateEptSingleContext(m_eptp);
        } else {
            GetCurrentVmcb()->control.tlb_control = TlbControlFlushThisAsid;
        }

        m_dirty = false;
    }
};

// Usage:
void InstallMultipleHooks(HookEntry* hooks, u32 count) {
    TlbBatch batch;

    for (u32 i = 0; i < count; i++) {
        ModifyEptEntry(&hooks[i]);
        batch.MarkDirty();
    }

    // Single flush at end instead of per-hook
    batch.Commit();
}
```

---

## Critical Values & Constants

### Intel MSRs and VMCS Fields

| Name | Value | Description |
|------|-------|-------------|
| `MSR_IA32_VMX_EPT_VPID_CAP` | 0x48C | EPT/VPID capabilities |
| `VMCS_VIRTUAL_PROCESSOR_ID` | 0x0000 | VPID field |
| `VMCS_EPT_POINTER` | 0x201A | EPTP field |

### AMD VMCB Offsets

| Offset | Name | Description |
|--------|------|-------------|
| 0x058 | TLB_CONTROL | TLB flush request |
| 0x068 | GUEST_ASID | Address Space ID |
| 0x0B0 | N_CR3 | Nested page table root |
| 0x0C0 | VMCB_CLEAN | Clean bits |

### TLB Control Values (AMD)

| Value | Name | Description |
|-------|------|-------------|
| 0 | DoNothing | No TLB flush |
| 1 | FlushAll | Flush entire TLB |
| 3 | FlushThisAsid | Flush entries for current ASID |
| 7 | FlushNonGlobal | Flush non-global entries |

---

## Common Pitfalls

### 1. Forgetting to Flush After EPT/NPT Changes

**What goes wrong**: Stale translations cause wrong pages to be accessed.
- **Both**: ALWAYS invalidate TLB after modifying any page table entry.

### 2. Using Wrong Invalidation Scope

**What goes wrong**: Either too narrow (some caches not flushed) or too broad (performance impact).
- **Intel**: Use single-context INVEPT when possible.
- **AMD**: Use FlushThisAsid instead of FlushAll when possible.

### 3. Not Synchronizing Across CPUs

**What goes wrong**: Other CPUs continue using stale TLB entries.
- **Both**: For hook installation, must IPI or otherwise synchronize all CPUs.

### 4. AMD: Relying on INVLPGA for NPT

**What goes wrong**: INVLPGA is for guest page tables, not NPT.
- **AMD**: Use VMCB TLB_CONTROL for NPT invalidation.

### 5. VPID/ASID = 0

**What goes wrong**: Zero VPID/ASID has special meaning.
- **Intel**: VPID 0 means "no VPID" - entries not tagged.
- **AMD**: ASID 0 typically means host - don't use for guests.

---

## Performance Considerations

### Minimizing TLB Invalidation Overhead

1. **Batch modifications**: Make multiple EPT/NPT changes, then single flush.
2. **Use narrow scope**: Single-context over all-contexts when possible.
3. **VPID/ASID**: Properly tag entries to avoid unnecessary flushes.
4. **Track dirty state**: Only flush if actually modified.

### Benchmark Data (Approximate)

| Operation | Cycles |
|-----------|--------|
| INVEPT single-context | ~1000-5000 |
| INVEPT all-contexts | ~5000-20000 |
| VMCB TLB flush | Included in VMRUN |
| INVLPGA | ~100-500 |

---

## Cross-References

- **01-SECOND-LEVEL-PAGING-FUNDAMENTALS.md**: EPT/NPT concepts
- **02-PAGE-TABLE-CONSTRUCTION.md**: Building page tables
- **03-SHADOW-PAGE-HOOKING.md**: Hook installation requiring TLB flush
- **04-VIOLATION-HANDLING.md**: Violation handlers that modify EPT/NPT

---

*This document synthesizes TLB management patterns from DdiMon, SimpleSvmHook, NoirVisor, Sputnik, and HyperPlatform reference implementations.*
