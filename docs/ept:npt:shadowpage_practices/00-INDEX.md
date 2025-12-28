# EPT/NPT/Shadow Page Implementation Guide

> **The definitive reference for OmbraPayload memory virtualization**

> **Target**: Ring -1 C++ implementation supporting BOTH Intel and AMD processors

---

## Document Index

| # | Document | Purpose | Priority |
|---|----------|---------|----------|
| 01 | [Second-Level Paging Fundamentals](01-SECOND-LEVEL-PAGING-FUNDAMENTALS.md) | EPT/NPT concepts, entry formats, permission bits | **Read First** |
| 02 | [Page Table Construction](02-PAGE-TABLE-CONSTRUCTION.md) | Building hierarchies, identity mapping, large page splitting | Core |
| 03 | [Shadow Page Hooking](03-SHADOW-PAGE-HOOKING.md) | Invisible hook technique, shadow pages, trampolines | Core |
| 04 | [Violation Handling](04-VIOLATION-HANDLING.md) | EPT violation / NPT fault handlers, MTF, state machine | Core |
| 05 | [TLB Management](05-TLB-MANAGEMENT.md) | INVEPT, INVLPGA, VPID, ASID, cross-CPU sync | Reference |

---

## Quick Reference: Intel vs AMD

| Aspect | Intel EPT | AMD NPT |
|--------|-----------|---------|
| **Root Pointer** | VMCS field 0x201A (EPTP) | VMCB offset 0x0B0 (nCR3) |
| **Enable Bit** | Secondary Controls bit 1 | VMCB offset 0x090 bit 0 |
| **Page Table Format** | EPT-specific (R/W/X bits) | Standard AMD64 PTE format |
| **User Bit Required** | No | **YES - ALWAYS (bit 2 = 1)** |
| **Execute Permission** | Bit 2 (X=1 → execute OK) | Bit 63 NX (NX=0 → execute OK) |
| **Execute-Only Support** | **YES** (R=0, W=0, X=1) | **NO** (not possible) |
| **Violation Exit** | Exit Reason 48 | Exit Code 0x400 |
| **Faulting GPA** | VMCS 0x2400 | VMCB offset 0x080 |
| **Error Code** | VMCS 0x6400 (Exit Qual) | VMCB offset 0x078 |
| **TLB Flush** | INVEPT instruction | VMCB TLB_CONTROL field |
| **Processor ID** | VPID (16-bit) | ASID (32-bit) |
| **Single-Step** | Monitor Trap Flag (MTF) | RFLAGS.TF or emulation |

---

## Implementation Flow Diagram

```
                     ┌─────────────────────────────────────┐
                     │          INITIALIZATION             │
                     └─────────────────────────────────────┘
                                      │
                     ┌────────────────┴────────────────┐
                     ▼                                 ▼
              ┌─────────────┐                   ┌─────────────┐
              │ Intel VT-x  │                   │   AMD SVM   │
              └─────────────┘                   └─────────────┘
                     │                                 │
                     ▼                                 ▼
        ┌────────────────────────┐       ┌────────────────────────┐
        │  Allocate EPT Tables   │       │  Allocate NPT Tables   │
        │  (PML4, PDPT, PD, PT)  │       │  (PML4, PDPT, PD, PT)  │
        │  Doc: 02-PAGE-TABLE    │       │  Doc: 02-PAGE-TABLE    │
        └────────────────────────┘       └────────────────────────┘
                     │                                 │
                     ▼                                 ▼
        ┌────────────────────────┐       ┌────────────────────────┐
        │  Build Identity Map    │       │  Build Identity Map    │
        │  (GPA = HPA, 2MB pages)│       │  (GPA = HPA, 2MB pages)│
        │  Set R/W/X = 1/1/1     │       │  Set P/W/U/NX = 1/1/1/0│
        └────────────────────────┘       └────────────────────────┘
                     │                                 │
                     ▼                                 ▼
        ┌────────────────────────┐       ┌────────────────────────┐
        │  Set EPTP in VMCS      │       │  Set nCR3 in VMCB      │
        │  Enable EPT (Sec. Ctl) │       │  Enable NPT (NP_ENABLE)│
        └────────────────────────┘       └────────────────────────┘
                     │                                 │
                     └────────────────┬────────────────┘
                                      ▼
                     ┌─────────────────────────────────────┐
                     │         HOOK INSTALLATION           │
                     │         Doc: 03-SHADOW-PAGE         │
                     └─────────────────────────────────────┘
                                      │
                     ┌────────────────┴────────────────┐
                     ▼                                 ▼
        ┌────────────────────────┐       ┌────────────────────────┐
        │  Split 2MB → 4KB page  │       │  Split 2MB → 4KB page  │
        │  Doc: 02-PAGE-TABLE    │       │  (User bit = 1 always!)│
        └────────────────────────┘       └────────────────────────┘
                     │                                 │
                     ▼                                 ▼
        ┌────────────────────────┐       ┌────────────────────────┐
        │  Allocate shadow pages │       │  Allocate shadow pages │
        │  - RW shadow (clean)   │       │  - Original copy       │
        │  - Exec shadow (hook)  │       │  - Exec shadow (hook)  │
        └────────────────────────┘       └────────────────────────┘
                     │                                 │
                     ▼                                 ▼
        ┌────────────────────────┐       ┌────────────────────────┐
        │  Configure EPT entry:  │       │  Configure NPT entry:  │
        │  R=0, W=0, X=1         │       │  Mark hooked pages NX  │
        │  PFN → exec shadow     │       │  (State 1: RW- on hook)│
        └────────────────────────┘       └────────────────────────┘
                     │                                 │
                     ▼                                 ▼
        ┌────────────────────────┐       ┌────────────────────────┐
        │  INVEPT (flush TLB)    │       │  Set TLB_CONTROL flush │
        │  Doc: 05-TLB-MGMT      │       │  Doc: 05-TLB-MGMT      │
        └────────────────────────┘       └────────────────────────┘
                     │                                 │
                     └────────────────┬────────────────┘
                                      ▼
                     ┌─────────────────────────────────────┐
                     │           RUNTIME HANDLING          │
                     │         Doc: 04-VIOLATION           │
                     └─────────────────────────────────────┘
                                      │
                     ┌────────────────┴────────────────┐
                     ▼                                 ▼
        ┌────────────────────────┐       ┌────────────────────────┐
        │ EPT Violation Handler  │       │  NPT Fault Handler     │
        │ (Exit Reason 48)       │       │  (Exit Code 0x400)     │
        └────────────────────────┘       └────────────────────────┘
                     │                                 │
                     ▼                                 ▼
        ┌────────────────────────┐       ┌────────────────────────┐
        │ Read/Write violation:  │       │ Execute violation:     │
        │  → Switch to RW page   │       │  → Transition State    │
        │  → Enable MTF          │       │    1→2 or 2→1          │
        │  → Resume              │       │  → Flush TLB           │
        └────────────────────────┘       └────────────────────────┘
                     │                                 │
                     ▼                                 │
        ┌────────────────────────┐                     │
        │ MTF Exit Handler:      │                     │
        │  → Restore exec-only   │                     │
        │  → Disable MTF         │                     │
        │  → Resume              │                     │
        └────────────────────────┘                     │
                     │                                 │
                     └────────────────┬────────────────┘
                                      ▼
                            ┌─────────────────┐
                            │   HOOK ACTIVE   │
                            │   & INVISIBLE   │
                            └─────────────────┘
```

---

## Decision Tree: Which Document Do I Need?

```
START
  │
  ├─ "What is EPT/NPT?"
  │     └─ 01-SECOND-LEVEL-PAGING-FUNDAMENTALS.md
  │
  ├─ "How do I build the page tables?"
  │     └─ 02-PAGE-TABLE-CONSTRUCTION.md
  │
  ├─ "How do I set up identity mapping?"
  │     └─ 02-PAGE-TABLE-CONSTRUCTION.md
  │
  ├─ "How do I split a 2MB page to 4KB?"
  │     └─ 02-PAGE-TABLE-CONSTRUCTION.md
  │
  ├─ "How do shadow page hooks work?"
  │     └─ 03-SHADOW-PAGE-HOOKING.md
  │
  ├─ "How do I install an invisible hook?"
  │     └─ 03-SHADOW-PAGE-HOOKING.md
  │
  ├─ "How do I build a trampoline?"
  │     └─ 03-SHADOW-PAGE-HOOKING.md
  │
  ├─ "How do I handle EPT violations?"
  │     └─ 04-VIOLATION-HANDLING.md
  │
  ├─ "How do I handle NPT faults?"
  │     └─ 04-VIOLATION-HANDLING.md
  │
  ├─ "What is the AMD NPT state machine?"
  │     └─ 04-VIOLATION-HANDLING.md
  │
  ├─ "How does Intel MTF work?"
  │     └─ 04-VIOLATION-HANDLING.md
  │
  ├─ "How do I flush the TLB?"
  │     └─ 05-TLB-MANAGEMENT.md
  │
  ├─ "What is INVEPT/INVLPGA?"
  │     └─ 05-TLB-MANAGEMENT.md
  │
  └─ "How do I synchronize across CPUs?"
        └─ 05-TLB-MANAGEMENT.md
```

---

## Critical Constants Quick Reference

### Intel EPT

```cpp
// VMCS Fields
constexpr u32 VMCS_EPT_POINTER = 0x201A;
constexpr u32 VMCS_GUEST_PHYSICAL_ADDRESS = 0x2400;
constexpr u32 VMCS_EXIT_QUALIFICATION = 0x6400;
constexpr u32 VMCS_EXIT_REASON = 0x4402;

// Exit Reasons
constexpr u32 EXIT_REASON_EPT_VIOLATION = 48;
constexpr u32 EXIT_REASON_EPT_MISCONFIGURATION = 49;
constexpr u32 EXIT_REASON_MTF = 37;

// EPTP Configuration
constexpr u64 EPTP_MEMORY_TYPE_WB = 6;
constexpr u64 EPTP_PAGE_WALK_LENGTH_4 = 3;  // (4 - 1)

// EPT Entry Bits
constexpr u64 EPT_READ = (1 << 0);
constexpr u64 EPT_WRITE = (1 << 1);
constexpr u64 EPT_EXECUTE = (1 << 2);
constexpr u64 EPT_LARGE_PAGE = (1 << 7);
```

### AMD NPT

```cpp
// VMCB Offsets
constexpr u32 VMCB_NCR3 = 0x0B0;
constexpr u32 VMCB_NP_ENABLE = 0x090;
constexpr u32 VMCB_EXITCODE = 0x070;
constexpr u32 VMCB_EXITINFO1 = 0x078;
constexpr u32 VMCB_EXITINFO2 = 0x080;
constexpr u32 VMCB_TLB_CONTROL = 0x058;
constexpr u32 VMCB_GUEST_ASID = 0x068;

// Exit Codes
constexpr u64 VMEXIT_NPF = 0x400;

// NPT Entry Bits
constexpr u64 NPT_PRESENT = (1 << 0);
constexpr u64 NPT_WRITE = (1 << 1);
constexpr u64 NPT_USER = (1 << 2);     // MUST ALWAYS BE SET
constexpr u64 NPT_LARGE_PAGE = (1 << 7);
constexpr u64 NPT_NX = (1ULL << 63);

// TLB Control Values
constexpr u8 TLB_CONTROL_FLUSH_ALL = 1;
constexpr u8 TLB_CONTROL_FLUSH_THIS_ASID = 3;
```

---

## Implementation Checklist

### Phase 1: Core Infrastructure
- [ ] Detect CPU vendor at runtime (CPUID)
- [ ] Allocate page table memory (pre-allocated pool)
- [ ] Build identity map (GPA = HPA)
- [ ] Set EPTP (Intel) or nCR3 (AMD)
- [ ] Enable EPT/NPT in VMCS/VMCB

### Phase 2: Large Page Splitting
- [ ] Implement 2MB → 4KB split function
- [ ] Track split pages for cleanup
- [ ] Handle AMD User bit in new PTEs

### Phase 3: Shadow Page Management
- [ ] Allocate shadow pages (RW + Exec copies)
- [ ] Implement page reuse for same-page hooks
- [ ] Build trampoline generator

### Phase 4: Hook Installation
- [ ] Intel: Configure execute-only EPT entry
- [ ] AMD: Configure NX-based state machine
- [ ] Install breakpoint or jump on exec shadow
- [ ] Invalidate TLB on all CPUs

### Phase 5: Violation Handling
- [ ] Intel: EPT violation handler (exit 48)
- [ ] Intel: MTF handler (exit 37)
- [ ] AMD: NPT fault handler (exit 0x400)
- [ ] AMD: State 1↔2 transitions
- [ ] Breakpoint exception handler

### Phase 6: TLB Management
- [ ] Implement INVEPT wrapper
- [ ] Implement VMCB TLB_CONTROL usage
- [ ] Cross-CPU synchronization (IPI)
- [ ] Batched invalidation for multi-hook install

---

## Common Pitfall Summary

| Pitfall | Intel | AMD |
|---------|-------|-----|
| Forgetting User bit | N/A | **All NPT entries need User=1** |
| Wrong NX logic | EPT bit 2 = Execute OK | NPT bit 63 = No Execute |
| Expecting execute-only | Works (R=0,W=0,X=1) | **Not possible** |
| No TLB flush | INVEPT required | TLB_CONTROL required |
| Single-step mechanism | MTF built-in | Manual (RFLAGS.TF) |

---

## Source Attribution

These documents synthesize knowledge from:

| Project | Vendor | Key Contribution |
|---------|--------|------------------|
| DdiMon | Intel | Shadow hook architecture, MTF handling |
| HyperPlatform | Intel | EPT management, violation handling |
| SimpleSvmHook | AMD | NPT state machine, dual-table design |
| NoirVisor | Both | Multi-vendor abstraction patterns |
| Sputnik | Both | Unified EPT/NPT with shadow EPT |
| HyperHide | Intel | IPI synchronization, breakpoint hooks |

---

## Version History

| Date | Change |
|------|--------|
| 2025-12-23 | Initial synthesis from 33 source documents |

---

*The source files in the parent directory are now archival. These FINAL documents are canon for OmbraPayload implementation.*
