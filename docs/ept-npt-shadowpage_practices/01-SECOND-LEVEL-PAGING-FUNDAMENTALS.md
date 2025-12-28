# Second-Level Paging Fundamentals: EPT and NPT

> **Purpose**: Complete understanding of Intel EPT and AMD NPT - the hardware-assisted second level of address translation that the hypervisor controls.

> **Implementation target**: OmbraPayload (Ring -1, C++ with restrictions, BOTH Intel & AMD)

---

## Overview

Second-level paging (also called nested paging or two-dimensional paging) is a hardware-assisted memory virtualization technology that allows a hypervisor to control guest physical-to-host physical address translation without trapping every memory access. Before this technology existed, hypervisors had to use shadow paging, which caused a VM-exit on every page table modification - devastating for performance.

Intel calls their implementation **Extended Page Tables (EPT)**. AMD calls theirs **Nested Page Tables (NPT)**. Despite different names, both serve the same fundamental purpose: translating Guest Physical Addresses (GPA) to Host Physical Addresses (HPA) with hardware acceleration.

The guest OS manages its own page tables (CR3-based, translating GVA→GPA), completely unaware that another translation layer exists beneath it. The hypervisor manages EPT/NPT (translating GPA→HPA), providing memory isolation, protection, and the foundation for advanced techniques like shadow page hooking.

---

## Conceptual Model

### Address Translation Flow

```
Guest Virtual Address (GVA)
        │
        ▼ [Guest CR3 Page Tables]
Guest Physical Address (GPA)
        │
        ▼ [EPT/NPT Page Tables]
Host Physical Address (HPA)
        │
        ▼ [Physical RAM]
```

### Why Second-Level Paging Exists

Without EPT/NPT, the hypervisor must:
1. Trap every guest page table modification (CR3 writes, INVLPG, etc.)
2. Maintain "shadow" page tables that combine GVA→HPA
3. Synchronize shadow tables with guest tables on every change

This shadow paging approach causes **thousands of VM-exits per second** on active workloads.

With EPT/NPT:
1. Guest manages its own page tables normally (no trapping)
2. Hardware performs two-level walks transparently
3. VM-exits only occur on actual permission violations
4. Performance approaches native execution

---

## Intel EPT Implementation

### EPTP (EPT Pointer)

The EPT Pointer is stored in the VMCS and points to the root of the EPT hierarchy.

**VMCS Field**: `EPT_POINTER` (encoding 0x201A for full 64-bit)

```cpp
union EptPointer {
    struct {
        u64 memory_type : 3;        // Bits 0-2: Memory type (0=UC, 6=WB)
        u64 page_walk_length : 3;   // Bits 3-5: Page walk length - 1 (must be 3)
        u64 enable_ad_bits : 1;     // Bit 6: Enable accessed/dirty flags
        u64 enable_sss : 1;         // Bit 7: Supervisor shadow stack
        u64 reserved : 4;           // Bits 8-11: Reserved (0)
        u64 pml4_pfn : 52;          // Bits 12-63: Physical address >> 12
    };
    u64 value;
};
```

**Standard Configuration**:
```cpp
EptPointer eptp = {};
eptp.memory_type = 6;           // Write-Back for performance
eptp.page_walk_length = 3;      // 4-level paging (3 means walk length of 4)
eptp.enable_ad_bits = 0;        // Optional - enable if tracking access patterns
eptp.pml4_pfn = pml4_physical >> 12;
```

### EPT 4-Level Structure

```
EPTP → PML4 (512 entries) → PDPT (512 entries) → PD (512 entries) → PT (512 entries)
       ↓ each covers 512GB   ↓ each covers 1GB    ↓ each covers 2MB   ↓ each covers 4KB
```

**Index Extraction from GPA**:
```cpp
constexpr u64 EPT_PML4_INDEX(u64 gpa) { return (gpa >> 39) & 0x1FF; }
constexpr u64 EPT_PDPT_INDEX(u64 gpa) { return (gpa >> 30) & 0x1FF; }
constexpr u64 EPT_PD_INDEX(u64 gpa)   { return (gpa >> 21) & 0x1FF; }
constexpr u64 EPT_PT_INDEX(u64 gpa)   { return (gpa >> 12) & 0x1FF; }
constexpr u64 EPT_PAGE_OFFSET(u64 gpa){ return gpa & 0xFFF; }
```

### EPT Entry Format (All Levels)

```cpp
union EptEntry {
    struct {
        u64 read : 1;              // Bit 0: Read access
        u64 write : 1;             // Bit 1: Write access
        u64 execute : 1;           // Bit 2: Execute access
        u64 memory_type : 3;       // Bits 3-5: Memory type (for leaf entries)
        u64 ignore_pat : 1;        // Bit 6: Ignore PAT (for leaf entries)
        u64 large_page : 1;        // Bit 7: Large page (1GB in PDPT, 2MB in PD)
        u64 accessed : 1;          // Bit 8: Accessed flag (if enabled)
        u64 dirty : 1;             // Bit 9: Dirty flag (if enabled, leaf only)
        u64 execute_user : 1;      // Bit 10: User-mode execute (MBEC)
        u64 reserved1 : 1;         // Bit 11: Reserved
        u64 pfn : 40;              // Bits 12-51: Physical Frame Number
        u64 reserved2 : 11;        // Bits 52-62: Reserved
        u64 suppress_ve : 1;       // Bit 63: Suppress #VE (virtualization exception)
    };
    u64 value;
};
```

**Memory Type Values** (bits 3-5):
| Value | Type | Description |
|-------|------|-------------|
| 0 | UC | Uncacheable |
| 1 | WC | Write Combining |
| 4 | WT | Write Through |
| 5 | WP | Write Protected |
| 6 | WB | Write Back (default for RAM) |

### EPT Permission Combinations

| R | W | X | Meaning | Use Case |
|---|---|---|---------|----------|
| 0 | 0 | 0 | No access | Unmapped/MMIO trap |
| 1 | 0 | 0 | Read-only | Copy-on-write |
| 1 | 1 | 0 | Read-write | Data pages |
| 1 | 1 | 1 | Full access | Normal code+data |
| 0 | 0 | 1 | **Execute-only** | **Shadow page hooks** |
| 1 | 0 | 1 | Read-execute | Code pages |

**Execute-Only is KEY**: Intel EPT uniquely supports execute-only pages (R=0, W=0, X=1). This is the foundation of invisible hooks - execution works but reads trigger violations, allowing clean bytes to be returned.

### EPT Large Page Support

**2MB Large Pages** (PD level):
- Set `large_page = 1` in PD entry
- PFN points directly to 2MB-aligned physical address
- No PT level needed

**1GB Large Pages** (PDPT level, if supported):
- Set `large_page = 1` in PDPT entry
- PFN points directly to 1GB-aligned physical address
- No PD or PT levels needed
- Check `IA32_VMX_EPT_VPID_CAP` bit 17 for support

### Required EPT Capabilities

Before using EPT, check `IA32_VMX_EPT_VPID_CAP` (MSR 0x48C):

| Bit | Capability |
|-----|------------|
| 0 | Execute-only pages supported |
| 6 | 4-level page walk supported |
| 8 | UC memory type allowed |
| 14 | WB memory type allowed |
| 16 | 2MB large pages supported |
| 17 | 1GB large pages supported |
| 20 | INVEPT instruction supported |
| 21 | Accessed/Dirty flags supported |
| 25 | Single-context INVEPT supported |
| 26 | All-context INVEPT supported |

---

## AMD NPT Implementation

### NCR3 (Nested CR3)

The Nested CR3 is stored directly in the VMCB and points to the root of the NPT hierarchy.

**VMCB Offset**: 0x0B0

```cpp
// NCR3 is stored as a raw physical address in VMCB
u64* ncr3_ptr = (u64*)((u8*)vmcb + 0x0B0);
*ncr3_ptr = npt_pml4_physical;
```

**NPT Enable Bit**: VMCB offset 0x090 (NP_ENABLE), bit 0

```cpp
// Enable nested paging
u64* np_enable = (u64*)((u8*)vmcb + 0x090);
*np_enable |= 1;
```

### NPT 4-Level Structure

NPT uses the **standard AMD64 page table format** - the same format as regular CR3-based paging.

```
nCR3 → PML4 (512 entries) → PDPT (512 entries) → PD (512 entries) → PT (512 entries)
```

**Index Extraction** (identical to EPT):
```cpp
constexpr u64 NPT_PML4_INDEX(u64 gpa) { return (gpa >> 39) & 0x1FF; }
constexpr u64 NPT_PDPT_INDEX(u64 gpa) { return (gpa >> 30) & 0x1FF; }
constexpr u64 NPT_PD_INDEX(u64 gpa)   { return (gpa >> 21) & 0x1FF; }
constexpr u64 NPT_PT_INDEX(u64 gpa)   { return (gpa >> 12) & 0x1FF; }
```

### NPT Entry Format (Standard AMD64 PTE)

```cpp
union NptEntry {
    struct {
        u64 present : 1;           // Bit 0: Present (= Read access)
        u64 write : 1;             // Bit 1: Write access
        u64 user : 1;              // Bit 2: User access *** MUST BE 1 ***
        u64 pwt : 1;               // Bit 3: Page Write Through
        u64 pcd : 1;               // Bit 4: Page Cache Disable
        u64 accessed : 1;          // Bit 5: Accessed
        u64 dirty : 1;             // Bit 6: Dirty (leaf only)
        u64 large_page : 1;        // Bit 7: Large page (PS bit)
        u64 global : 1;            // Bit 8: Global (ignored in NPT)
        u64 available : 3;         // Bits 9-11: Available for software
        u64 pfn : 40;              // Bits 12-51: Physical Frame Number
        u64 reserved : 11;         // Bits 52-62: Reserved
        u64 nx : 1;                // Bit 63: No Execute
    };
    u64 value;
};
```

### CRITICAL: The User Bit Requirement

**THIS IS THE #1 AMD NPT PITFALL**

```cpp
// EVERY NPT entry at EVERY level MUST have User=1
npt_entry.user = 1;  // MANDATORY - or guest will immediately fault
```

**Why?**
- AMD hardware treats ALL nested page walks as "user mode" accesses at the nested level
- This is regardless of the guest's actual CPL (privilege level)
- If User=0, the hardware sees a supervisor-only page accessed in user mode → fault
- The guest will crash immediately on first memory access

**Intel EPT does NOT have this requirement** - there's no User bit concept in EPT entries.

### NPT Permission Combinations

| Present | Write | NX | Meaning |
|---------|-------|----|---------|
| 0 | - | - | Not present (fault) |
| 1 | 0 | 0 | Read-execute |
| 1 | 0 | 1 | Read-only |
| 1 | 1 | 0 | Read-write-execute |
| 1 | 1 | 1 | Read-write (no execute) |

**NO EXECUTE-ONLY**: AMD NPT cannot express execute-only pages. Present=0 means no access at all. This fundamental limitation requires different shadow hook strategies on AMD.

### NPT Large Page Support

**2MB Large Pages** (PD level):
- Set `large_page = 1` (PS bit) in PD entry
- Same as standard AMD64 large pages

**1GB Large Pages** (PDPT level):
- Set `large_page = 1` in PDPT entry
- Requires CPU support (CPUID check)

---

## Unified Patterns

### Pattern: Vendor-Abstracted Page Entry

**Abstraction**: Create a common interface that works on both vendors.

```cpp
namespace Ombra::Memory {

enum class Vendor { Intel, Amd };

// Unified permissions - translate to vendor-specific
struct PagePermissions {
    bool read;
    bool write;
    bool execute;
};

// Vendor-abstracted entry manipulation
template<Vendor V>
class PageEntry {
public:
    void SetPermissions(PagePermissions perms);
    void SetPhysicalAddress(u64 hpa);
    void SetLargePage(bool large);
    u64 GetRaw() const;

private:
    u64 m_value;
};

// Intel specialization
template<>
void PageEntry<Vendor::Intel>::SetPermissions(PagePermissions perms) {
    EptEntry* e = reinterpret_cast<EptEntry*>(&m_value);
    e->read = perms.read;
    e->write = perms.write;
    e->execute = perms.execute;
    // Intel has no User bit requirement
}

// AMD specialization
template<>
void PageEntry<Vendor::Amd>::SetPermissions(PagePermissions perms) {
    NptEntry* e = reinterpret_cast<NptEntry*>(&m_value);
    e->present = perms.read;  // Present implies read
    e->write = perms.write;
    e->user = 1;              // ALWAYS SET
    e->nx = !perms.execute;   // Inverted logic
}

} // namespace Ombra::Memory
```

### Pattern: Runtime Vendor Dispatch

```cpp
namespace Ombra {

enum CpuVendor : u8 {
    VendorIntel = 1,
    VendorAmd = 2
};

// Detected once at init, used everywhere
inline CpuVendor g_vendor;

inline void DetectVendor() {
    char vendor[13] = {};
    int regs[4];
    __cpuid(regs, 0);
    *reinterpret_cast<int*>(&vendor[0]) = regs[1];  // EBX
    *reinterpret_cast<int*>(&vendor[4]) = regs[3];  // EDX
    *reinterpret_cast<int*>(&vendor[8]) = regs[2];  // ECX

    if (strcmp(vendor, "GenuineIntel") == 0)
        g_vendor = VendorIntel;
    else if (strcmp(vendor, "AuthenticAMD") == 0)
        g_vendor = VendorAmd;
}

// Runtime dispatch for page table operations
inline void SetPagePermissions(void* entry, PagePermissions perms) {
    if (g_vendor == VendorIntel) {
        EptEntry* e = static_cast<EptEntry*>(entry);
        e->read = perms.read;
        e->write = perms.write;
        e->execute = perms.execute;
    } else {
        NptEntry* e = static_cast<NptEntry*>(entry);
        e->present = perms.read;
        e->write = perms.write;
        e->user = 1;
        e->nx = !perms.execute;
    }
}

} // namespace Ombra
```

---

## Critical Values & Constants

### Intel EPT

| Name | Value | Purpose |
|------|-------|---------|
| `EPTP_MEMORY_TYPE_WB` | 6 | Write-Back memory type for EPTP |
| `EPTP_PAGE_WALK_LENGTH` | 3 | 4-level paging (length - 1) |
| `EPT_VIOLATION_EXIT` | 48 | VM-exit reason for EPT violation |
| `MSR_IA32_VMX_EPT_VPID_CAP` | 0x48C | EPT/VPID capabilities MSR |
| `VMCS_EPT_POINTER` | 0x201A | VMCS field encoding for EPTP |
| `VMCS_GUEST_PHYSICAL_ADDRESS` | 0x2400 | Faulting GPA on violation |
| `VMCS_EXIT_QUALIFICATION` | 0x6400 | Violation details |

### AMD NPT

| Name | Value | Purpose |
|------|-------|---------|
| `VMCB_NCR3_OFFSET` | 0x0B0 | VMCB offset for nested CR3 |
| `VMCB_NP_ENABLE_OFFSET` | 0x090 | VMCB offset for NPT enable |
| `VMEXIT_NPF` | 0x400 | VM-exit code for NPT fault |
| `VMCB_EXITINFO1_OFFSET` | 0x078 | VMCB offset for error code |
| `VMCB_EXITINFO2_OFFSET` | 0x080 | VMCB offset for faulting GPA |
| `NPT_USER_BIT` | (1 << 2) | User bit - MUST BE SET |
| `NPT_NX_BIT` | (1ULL << 63) | No-Execute bit |

---

## Common Pitfalls

### 1. Forgetting NPT User Bit

**What goes wrong**: Guest immediately triple-faults on first memory access.
- **Intel**: Not applicable - EPT has no User bit concept
- **AMD**: EVERY entry, EVERY level must have User=1. No exceptions. The hardware interprets nested walks as user-mode regardless of guest CPL.

### 2. Wrong NX Bit Logic

**What goes wrong**: Pages that should be executable aren't, or vice versa.
- **Intel**: EPT bit 2 is Execute (1 = can execute)
- **AMD**: NPT bit 63 is No-Execute (1 = CANNOT execute) - inverted logic!

### 3. Assuming Execute-Only Works on AMD

**What goes wrong**: Shadow hook strategy fails completely on AMD.
- **Intel**: R=0, W=0, X=1 is valid - reads/writes fault, execution works
- **AMD**: Present=0 means no access. Cannot express "execute but not read."

### 4. Not Checking EPT Capabilities

**What goes wrong**: Using unsupported features causes undefined behavior.
- **Intel**: Must check `IA32_VMX_EPT_VPID_CAP` before using execute-only, large pages, INVEPT types
- **AMD**: NPT capabilities are more standardized but still check CPUID

### 5. Memory Type Mismatch

**What goes wrong**: Performance degradation or instability.
- **Intel**: EPT entry memory type should match MTRR for the region
- **AMD**: Uses PAT/PCD/PWT bits - same as regular paging

---

## Cross-References

- **02-PAGE-TABLE-CONSTRUCTION.md**: Building the actual EPT/NPT hierarchies
- **03-SHADOW-PAGE-HOOKING.md**: Using EPT/NPT permissions for invisible hooks
- **04-VIOLATION-HANDLING.md**: Handling EPT violations and NPT faults
- **05-TLB-MANAGEMENT.md**: INVEPT, INVLPGA, and TLB coherency

---

## Quick Reference Table

| Aspect | Intel EPT | AMD NPT |
|--------|-----------|---------|
| Root Pointer Location | VMCS field 0x201A | VMCB offset 0x0B0 |
| Enable Bit | Secondary Controls bit 1 | VMCB offset 0x090 bit 0 |
| Page Table Format | EPT-specific (R/W/X bits) | Standard AMD64 PTE |
| User Bit Required | No | **YES - ALWAYS** |
| Execute Permission | Bit 2 (X=1 means execute) | Bit 63 (NX=0 means execute) |
| Execute-Only Support | **YES** (R=0,W=0,X=1) | **NO** |
| Violation Exit Code | Reason 48 | Code 0x400 |
| Faulting GPA Source | VMCS 0x2400 | VMCB offset 0x080 |
| Error Code Source | VMCS 0x6400 (Exit Qual) | VMCB offset 0x078 |
| Memory Type Control | Bits 3-5 in entry | PAT/PCD/PWT bits |
| TLB Flush | INVEPT instruction | VMCB TLB_CONTROL field |

---

*This document synthesizes EPT and NPT knowledge from DdiMon, HyperPlatform, SimpleSvmHook, NoirVisor, and Sputnik reference implementations.*
