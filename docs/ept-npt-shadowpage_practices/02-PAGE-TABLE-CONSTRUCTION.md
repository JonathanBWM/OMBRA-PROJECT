# Page Table Construction

> **Purpose**: Building and managing EPT/NPT page tables - allocation, identity mapping, large page splitting, and dynamic modification.

> **Implementation target**: OmbraPayload (Ring -1, C++ with restrictions, BOTH Intel & AMD)

---

## Overview

Before the hypervisor can manage guest memory, it must construct the EPT (Intel) or NPT (AMD) page table hierarchy. This involves allocating properly-aligned memory for each level, establishing identity mappings so guest physical addresses equal host physical addresses initially, and preparing infrastructure for dynamic modifications like shadow page hooks.

The key challenge in Ring -1 is that we have no runtime allocator. All page table memory must be pre-allocated before entering VMX/SVM operation. This document covers allocation strategies, identity mapping construction, and large page splitting for fine-grained hooks.

---

## Identity Mapping

### What Identity Mapping Means

With identity mapping, every Guest Physical Address (GPA) maps to the same Host Physical Address (HPA):

```
GPA 0x0000_0000 → HPA 0x0000_0000
GPA 0x1000_0000 → HPA 0x1000_0000
GPA 0xFFFF_FFFF → HPA 0xFFFF_FFFF
```

The guest sees the same physical memory layout as the host. This is the baseline configuration - hooks and protections are applied as modifications to this identity map.

### Why Start With Identity Mapping

1. **Guest continues to work**: The guest OS was already using physical addresses correctly. Identity mapping preserves this.
2. **Simplicity**: No address translation needed - GPA == HPA everywhere initially.
3. **Incremental protection**: Add hooks/restrictions as overlay on working baseline.
4. **Debugging**: Problems are easier to isolate when the default is "pass-through."

### Physical Memory Range Discovery

Before building page tables, discover what physical memory exists:

**Windows Kernel Method** (during driver init, before hypervisor launch):
```cpp
// Query physical memory ranges via MmGetPhysicalMemoryRanges()
PPHYSICAL_MEMORY_RANGE ranges = MmGetPhysicalMemoryRanges();
for (PPHYSICAL_MEMORY_RANGE range = ranges; range->NumberOfBytes.QuadPart; range++) {
    u64 start = range->BaseAddress.QuadPart;
    u64 size = range->NumberOfBytes.QuadPart;
    // Record range for identity mapping
}
ExFreePool(ranges);
```

**Conservative Approach** (map everything up to max physical address):
```cpp
// Get max physical address from CPUID
u32 eax, ebx, ecx, edx;
__cpuid(regs, 0x80000008);
u8 max_phys_bits = eax & 0xFF;  // Usually 36, 40, 46, or 52
u64 max_phys_addr = 1ULL << max_phys_bits;
```

---

## Page Table Allocation

### Ring -1 Allocation Challenge

In Ring -1 (VMX root / SVM host), we have:
- No `malloc()` or `ExAllocatePool()`
- No page fault handling
- No virtual memory management

**Solution**: Pre-allocate all page table memory in Ring 0 before hypervisor entry.

### Allocation Requirements

| Level | Entry Count | Size | Alignment | Quantity Needed |
|-------|-------------|------|-----------|-----------------|
| PML4 | 512 | 4 KB | 4 KB | 1 per CPU (or 1 global) |
| PDPT | 512 | 4 KB | 4 KB | 512 (for full 256 TB) |
| PD | 512 | 4 KB | 4 KB | 262,144 (if all 4KB pages) |
| PT | 512 | 4 KB | 4 KB | Dynamic - for split pages |

**Practical Reality**: Most memory uses 2MB large pages. Only split when needed (hooks).

### Pre-Allocation Strategy

**Minimum Allocation** (2MB pages everywhere):
```cpp
struct EptState {
    alignas(PAGE_SIZE) EptPml4e pml4[512];
    alignas(PAGE_SIZE) EptPdpte pdpt[512];
    alignas(PAGE_SIZE) EptPde pd[512][512];  // 512 PDPTs × 512 PDEs = 1GB each
    // No PT level - using 2MB large pages
};
```

**With Dynamic Split Pool**:
```cpp
struct EptState {
    // Static hierarchy
    alignas(PAGE_SIZE) EptPml4e pml4[512];
    alignas(PAGE_SIZE) EptPdpte pdpt[512];
    alignas(PAGE_SIZE) EptPde pd[512][512];

    // Pre-allocated PT pool for splitting
    static constexpr u32 MAX_SPLITS = 128;
    alignas(PAGE_SIZE) EptPte pt_pool[MAX_SPLITS][512];
    u32 next_pt_index;

    EptPte* AllocatePt() {
        if (next_pt_index >= MAX_SPLITS) return nullptr;
        return pt_pool[next_pt_index++];
    }
};
```

### Sputnik Pattern: Pre-Allocated Dynamic Splits

```cpp
// From Sputnik EPT.cpp - pre-allocate split structures
struct VMM_EPT_DYNAMIC_SPLIT {
    alignas(PAGE_SIZE) EptPte PML1[512];  // The PT
    LIST_ENTRY DynamicSplitList;          // For tracking
    u64 PhysicalAddress;                  // Which 2MB page was split
};

// Pool of splits allocated during init
VMM_EPT_DYNAMIC_SPLIT* g_SplitPool;
u32 g_SplitPoolSize;
u32 g_NextSplitIndex;
```

---

## Building the Hierarchy

### Intel EPT Construction

```cpp
namespace Ombra::Ept {

void BuildIdentityMap(EptState* state, u64 max_physical) {
    // Clear all tables
    memset(state, 0, sizeof(EptState));

    // Template entry: RWX, Write-Back
    EptEntry template_entry = {};
    template_entry.read = 1;
    template_entry.write = 1;
    template_entry.execute = 1;
    template_entry.memory_type = 6;  // WB

    // Build PML4 → PDPT links
    for (u32 pml4i = 0; pml4i < 512; pml4i++) {
        u64 pdpt_pa = GetPhysicalAddress(&state->pdpt[pml4i][0]);
        state->pml4[pml4i].value = template_entry.value;
        state->pml4[pml4i].pfn = pdpt_pa >> 12;
    }

    // Build PDPT → PD links
    for (u32 pml4i = 0; pml4i < 512; pml4i++) {
        for (u32 pdpti = 0; pdpti < 512; pdpti++) {
            u64 pd_pa = GetPhysicalAddress(&state->pd[pml4i][pdpti][0]);
            state->pdpt[pml4i][pdpti].value = template_entry.value;
            state->pdpt[pml4i][pdpti].pfn = pd_pa >> 12;
        }
    }

    // Build PD entries as 2MB large pages
    for (u32 pml4i = 0; pml4i < 512; pml4i++) {
        for (u32 pdpti = 0; pdpti < 512; pdpti++) {
            for (u32 pdi = 0; pdi < 512; pdi++) {
                u64 gpa = ((u64)pml4i << 39) | ((u64)pdpti << 30) | ((u64)pdi << 21);
                if (gpa >= max_physical) {
                    state->pd[pml4i][pdpti][pdi].value = 0;  // Not present
                    continue;
                }

                EptPde* pde = &state->pd[pml4i][pdpti][pdi];
                pde->read = 1;
                pde->write = 1;
                pde->execute = 1;
                pde->large_page = 1;  // 2MB
                pde->memory_type = 6; // WB
                pde->pfn = gpa >> 12; // Identity: GPA == HPA
            }
        }
    }
}

} // namespace Ombra::Ept
```

### AMD NPT Construction

```cpp
namespace Ombra::Npt {

void BuildIdentityMap(NptState* state, u64 max_physical) {
    memset(state, 0, sizeof(NptState));

    // Template: Present, Write, User, No NX
    NptEntry template_entry = {};
    template_entry.present = 1;
    template_entry.write = 1;
    template_entry.user = 1;    // CRITICAL: Must be 1
    template_entry.nx = 0;      // Allow execute

    // Build PML4 → PDPT links
    for (u32 pml4i = 0; pml4i < 512; pml4i++) {
        u64 pdpt_pa = GetPhysicalAddress(&state->pdpt[pml4i][0]);
        state->pml4[pml4i].value = template_entry.value;
        state->pml4[pml4i].pfn = pdpt_pa >> 12;
    }

    // Build PDPT → PD links
    for (u32 pml4i = 0; pml4i < 512; pml4i++) {
        for (u32 pdpti = 0; pdpti < 512; pdpti++) {
            u64 pd_pa = GetPhysicalAddress(&state->pd[pml4i][pdpti][0]);
            state->pdpt[pml4i][pdpti].value = template_entry.value;
            state->pdpt[pml4i][pdpti].pfn = pd_pa >> 12;
        }
    }

    // Build PD entries as 2MB large pages
    for (u32 pml4i = 0; pml4i < 512; pml4i++) {
        for (u32 pdpti = 0; pdpti < 512; pdpti++) {
            for (u32 pdi = 0; pdi < 512; pdi++) {
                u64 gpa = ((u64)pml4i << 39) | ((u64)pdpti << 30) | ((u64)pdi << 21);
                if (gpa >= max_physical) {
                    state->pd[pml4i][pdpti][pdi].value = 0;
                    continue;
                }

                NptPde* pde = &state->pd[pml4i][pdpti][pdi];
                pde->present = 1;
                pde->write = 1;
                pde->user = 1;        // CRITICAL
                pde->large_page = 1;  // 2MB
                pde->nx = 0;          // Allow execute
                pde->pfn = gpa >> 12; // Identity
            }
        }
    }
}

} // namespace Ombra::Npt
```

---

## Large Page Splitting

### Why Splitting is Required

EPT/NPT hooks need 4KB granularity. A 2MB large page applies the same permissions to the entire 2MB region. To hook a single function, we must split the 2MB page into 512 × 4KB pages.

```
Before Split:                     After Split:
┌─────────────────────┐          ┌────┬────┬────┬─...─┬────┐
│   2MB Large Page    │          │4KB │4KB │4KB │     │4KB │
│  Single PDE entry   │    →     │ 0  │ 1  │ 2  │     │511 │
│  One permission set │          │    │    │    │     │    │
└─────────────────────┘          └────┴────┴────┴─...─┴────┘
                                  512 PTE entries
                                  Individual permissions
```

### Split Implementation

**Intel EPT Split**:
```cpp
namespace Ombra::Ept {

bool SplitLargePage(EptState* state, u64 gpa) {
    // Get PD entry for this GPA
    u32 pml4i = EPT_PML4_INDEX(gpa);
    u32 pdpti = EPT_PDPT_INDEX(gpa);
    u32 pdi = EPT_PD_INDEX(gpa);

    EptPde* pde = &state->pd[pml4i][pdpti][pdi];

    // Already split?
    if (!pde->large_page) return true;

    // Allocate PT from pool
    EptPte* pt = state->AllocatePt();
    if (!pt) return false;

    // Get the 2MB base physical address
    u64 base_pa = (u64)pde->pfn << 12;

    // Create 512 PTEs, each mapping 4KB
    EptPte template_pte = {};
    template_pte.read = pde->read;
    template_pte.write = pde->write;
    template_pte.execute = pde->execute;
    template_pte.memory_type = pde->memory_type;

    for (u32 pti = 0; pti < 512; pti++) {
        pt[pti].value = template_pte.value;
        pt[pti].pfn = (base_pa + (pti * PAGE_SIZE)) >> 12;
    }

    // Convert PDE from large page to pointer
    u64 pt_pa = GetPhysicalAddress(pt);
    pde->large_page = 0;
    pde->pfn = pt_pa >> 12;
    // Keep R/W/X the same - they're now interpreted for PT access

    return true;
}

} // namespace Ombra::Ept
```

**AMD NPT Split** (nearly identical, with User bit):
```cpp
namespace Ombra::Npt {

bool SplitLargePage(NptState* state, u64 gpa) {
    u32 pml4i = NPT_PML4_INDEX(gpa);
    u32 pdpti = NPT_PDPT_INDEX(gpa);
    u32 pdi = NPT_PD_INDEX(gpa);

    NptPde* pde = &state->pd[pml4i][pdpti][pdi];

    if (!pde->large_page) return true;

    NptPte* pt = state->AllocatePt();
    if (!pt) return false;

    u64 base_pa = (u64)pde->pfn << 12;

    NptPte template_pte = {};
    template_pte.present = pde->present;
    template_pte.write = pde->write;
    template_pte.user = 1;      // CRITICAL: Always set
    template_pte.nx = pde->nx;

    for (u32 pti = 0; pti < 512; pti++) {
        pt[pti].value = template_pte.value;
        pt[pti].pfn = (base_pa + (pti * PAGE_SIZE)) >> 12;
    }

    u64 pt_pa = GetPhysicalAddress(pt);
    pde->large_page = 0;
    pde->pfn = pt_pa >> 12;

    return true;
}

} // namespace Ombra::Npt
```

### Tracking Split Pages

Track which pages have been split for cleanup and to avoid redundant splits:

```cpp
struct SplitDescriptor {
    u64 gpa_base;       // 2MB-aligned GPA that was split
    void* pt_virtual;   // Virtual address of allocated PT
    u64 pt_physical;    // Physical address of PT
};

struct EptState {
    // ... page table arrays ...

    static constexpr u32 MAX_SPLITS = 128;
    SplitDescriptor splits[MAX_SPLITS];
    u32 split_count;

    bool IsSplit(u64 gpa) {
        u64 base = gpa & ~(SIZE_2MB - 1);
        for (u32 i = 0; i < split_count; i++) {
            if (splits[i].gpa_base == base) return true;
        }
        return false;
    }
};
```

---

## Entry Manipulation

### Getting a PTE for a GPA

```cpp
namespace Ombra::Ept {

EptPte* GetPte(EptState* state, u64 gpa) {
    u32 pml4i = EPT_PML4_INDEX(gpa);
    u32 pdpti = EPT_PDPT_INDEX(gpa);
    u32 pdi = EPT_PD_INDEX(gpa);
    u32 pti = EPT_PT_INDEX(gpa);

    // Verify PML4 entry exists
    if (!state->pml4[pml4i].read) return nullptr;

    // Verify PDPT entry exists
    EptPdpte* pdpte = &state->pdpt[pml4i][pdpti];
    if (!pdpte->read) return nullptr;

    // If PDPT is 1GB page, can't get 4KB PTE
    if (pdpte->large_page) return nullptr;

    // Verify PD entry exists
    EptPde* pde = &state->pd[pml4i][pdpti][pdi];
    if (!pde->read) return nullptr;

    // If PD is 2MB page, can't get 4KB PTE
    if (pde->large_page) return nullptr;

    // Get PT from PDE
    u64 pt_pa = (u64)pde->pfn << 12;
    EptPte* pt = PhysicalToVirtual<EptPte*>(pt_pa);

    return &pt[pti];
}

} // namespace Ombra::Ept
```

### Modifying Permissions

```cpp
namespace Ombra::Ept {

void SetPagePermissions(EptState* state, u64 gpa, bool r, bool w, bool x) {
    // Ensure page is split to 4KB
    if (!SplitLargePage(state, gpa)) {
        // Handle error
        return;
    }

    EptPte* pte = GetPte(state, gpa);
    if (!pte) return;

    pte->read = r;
    pte->write = w;
    pte->execute = x;

    // Must invalidate TLB after modification
    // See 05-TLB-MANAGEMENT.md
}

void SetPagePhysical(EptState* state, u64 gpa, u64 new_hpa) {
    EptPte* pte = GetPte(state, gpa);
    if (!pte) return;

    pte->pfn = new_hpa >> 12;

    // Invalidate TLB
}

} // namespace Ombra::Ept
```

---

## Unified Patterns

### Pattern: Vendor-Agnostic Page Table Walker

```cpp
namespace Ombra::Memory {

template<typename State, typename Pte>
class PageTableWalker {
public:
    PageTableWalker(State* state) : m_state(state) {}

    Pte* GetPte(u64 gpa) {
        // Split if needed
        if (IsLargePage(gpa)) {
            if (!Split(gpa)) return nullptr;
        }
        return GetPteInternal(gpa);
    }

    bool SetPermissions(u64 gpa, bool r, bool w, bool x) {
        Pte* pte = GetPte(gpa);
        if (!pte) return false;
        ApplyPermissions(pte, r, w, x);
        return true;
    }

protected:
    virtual bool IsLargePage(u64 gpa) = 0;
    virtual bool Split(u64 gpa) = 0;
    virtual Pte* GetPteInternal(u64 gpa) = 0;
    virtual void ApplyPermissions(Pte* pte, bool r, bool w, bool x) = 0;

    State* m_state;
};

// Intel specialization
class EptWalker : public PageTableWalker<EptState, EptPte> {
protected:
    void ApplyPermissions(EptPte* pte, bool r, bool w, bool x) override {
        pte->read = r;
        pte->write = w;
        pte->execute = x;
    }
};

// AMD specialization
class NptWalker : public PageTableWalker<NptState, NptPte> {
protected:
    void ApplyPermissions(NptPte* pte, bool r, bool w, bool x) override {
        pte->present = r;
        pte->write = w;
        pte->user = 1;      // ALWAYS
        pte->nx = !x;       // Inverted
    }
};

} // namespace Ombra::Memory
```

### Pattern: Runtime Dispatch

```cpp
namespace Ombra {

inline bool SetPagePermissions(void* state, u64 gpa, bool r, bool w, bool x) {
    if (g_vendor == VendorIntel) {
        return Ept::SetPagePermissions(static_cast<EptState*>(state), gpa, r, w, x);
    } else {
        return Npt::SetPagePermissions(static_cast<NptState*>(state), gpa, r, w, x);
    }
}

inline bool SplitLargePage(void* state, u64 gpa) {
    if (g_vendor == VendorIntel) {
        return Ept::SplitLargePage(static_cast<EptState*>(state), gpa);
    } else {
        return Npt::SplitLargePage(static_cast<NptState*>(state), gpa);
    }
}

} // namespace Ombra
```

---

## Critical Values & Constants

### Size Constants

| Name | Value | Description |
|------|-------|-------------|
| `PAGE_SIZE` | 0x1000 (4096) | 4KB page size |
| `SIZE_2MB` | 0x200000 | 2MB large page size |
| `SIZE_1GB` | 0x40000000 | 1GB huge page size |
| `PTE_COUNT` | 512 | Entries per page table |

### Index Masks

| Name | Value | Description |
|------|-------|-------------|
| `PML4_SHIFT` | 39 | Shift for PML4 index |
| `PDPT_SHIFT` | 30 | Shift for PDPT index |
| `PD_SHIFT` | 21 | Shift for PD index |
| `PT_SHIFT` | 12 | Shift for PT index |
| `INDEX_MASK` | 0x1FF (511) | 9-bit index mask |

---

## Common Pitfalls

### 1. Not Pre-Allocating Enough PT Entries

**What goes wrong**: Hook installation fails because no PT slots available.
- **Intel**: Pre-allocate based on expected hook count
- **AMD**: Same - dual NPT approach doubles memory needs

### 2. Forgetting User Bit During NPT Split

**What goes wrong**: Guest faults immediately after split.
- **Intel**: Not applicable
- **AMD**: Every new PTE must have User=1, even when copying from parent PDE

### 3. Not Invalidating TLB After Modification

**What goes wrong**: Stale translations cause incorrect behavior.
- **Intel**: Must call INVEPT after any EPT change
- **AMD**: Must set TLB_CONTROL in VMCB or use INVLPGA

### 4. Splitting Already-Split Pages

**What goes wrong**: Memory corruption if PT pointer overwritten.
- **Both**: Check `large_page` bit before attempting split

### 5. Identity Map Beyond Physical Memory

**What goes wrong**: Creating entries for non-existent memory wastes space and can cause issues.
- **Both**: Query physical memory ranges and only map what exists

---

## Cross-References

- **01-SECOND-LEVEL-PAGING-FUNDAMENTALS.md**: EPT/NPT entry formats and concepts
- **03-SHADOW-PAGE-HOOKING.md**: Using these page tables for invisible hooks
- **05-TLB-MANAGEMENT.md**: Critical - must invalidate after modifications

---

*This document synthesizes page table construction patterns from Sputnik, NoirVisor, DdiMon, and HyperPlatform reference implementations.*
