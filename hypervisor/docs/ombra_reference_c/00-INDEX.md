# OmbraHypervisor C Reference Package

> **Pure C + Assembly implementation reference for Ring -1 hypervisor development**
> **NO C++ - All examples are C11 compatible with MSVC**

---

## Package Contents

| Directory | Purpose | Files |
|-----------|---------|-------|
| `intel_vmx/` | Intel VT-x lifecycle, VMCS, VMExit handling | 5 files |
| `ept_npt/` | Extended Page Tables, shadow pages, TLB | 5 files |
| `antidetection/` | Stealth, evasion, anti-cheat bypass | 5 files |
| `driver_ring0/` | Kernel driver patterns (C only) | 4 files |
| `asm_stubs/` | MASM assembly for VMX operations | 3 files |
| `common/` | Shared types, macros, definitions | 3 files |

---

## Key Concepts Covered

### Intel VMX Lifecycle
1. VMX capability detection (CPUID, MSRs)
2. CR0/CR4 fixed bit application
3. VMXON/VMCS region setup
4. VMCS field configuration (all required fields)
5. VMLAUNCH/VMRESUME patterns
6. VMExit dispatch and handling
7. Clean shutdown (VMXOFF)

### EPT/NPT Memory Virtualization
1. 4-level page table construction
2. Identity mapping (GPA == HPA)
3. Large page splitting (2MB → 4KB)
4. Shadow page hooking (execute-only)
5. EPT violation handling
6. TLB management (INVEPT, VPID)
7. Memory type handling (MTRR awareness)

### Anti-Detection & Stealth
1. CPUID leaf masking (hypervisor bit, vendor)
2. MSR virtualization (VMX capability hiding)
3. CR4.VMXE shadow
4. TSC offsetting (timing attack mitigation)
5. Exception injection (#GP, #UD for VMX instructions)
6. Shadow pages for code hiding

### Driver Patterns (Pure C)
1. Manual mapping without loader
2. Hypercall interface via CPUID
3. Kernel structure offsets (EPROCESS, etc.)
4. Trace cleanup patterns

---

## C Language Constraints

All code in this package follows these rules:

```c
// ALLOWED:
typedef struct { } Name;           // C structs
static inline functions            // Inline helpers
#include <stdbool.h>               // bool type
// comments                        // C99 comments
_Static_assert()                   // Compile-time checks

// FORBIDDEN:
class, template, namespace         // C++ keywords
new, delete                        // C++ allocation
references (int& x)                // C++ references
std::anything                      // C++ STL
extern "C"                         // C++ linkage
operator overloading               // C++ operators
```

---

## Quick Start

### 1. Read Common Types First
```
common/types.h       - Base types (U8, U32, U64, etc.)
common/vmcs_defs.h   - VMCS field encodings
common/msr_defs.h    - MSR addresses
```

### 2. Understand VMX Lifecycle
```
intel_vmx/01_vmx_init.c      - Initialization sequence
intel_vmx/02_vmcs_setup.c    - VMCS configuration
intel_vmx/03_vmexit.c        - Exit handling
```

### 3. Implement EPT
```
ept_npt/01_ept_structures.h  - EPT entry formats
ept_npt/02_identity_map.c    - Building the map
ept_npt/03_shadow_hooks.c    - Hook installation
```

### 4. Add Stealth
```
antidetection/01_cpuid_mask.c    - CPUID hiding
antidetection/02_msr_virt.c      - MSR hiding
antidetection/03_timing.c        - TSC compensation
```

---

## Build Requirements

- **Compiler:** MSVC (Visual Studio 2019+)
- **Target:** Windows x64 kernel mode
- **Assembler:** MASM (ml64.exe)
- **Headers:** Windows DDK (for intrinsics)

---

## File Naming Convention

```
XX_name.c    - Implementation file
XX_name.h    - Header file
XX_name.asm  - Assembly file

XX = ordering number (01, 02, 03...)
```
