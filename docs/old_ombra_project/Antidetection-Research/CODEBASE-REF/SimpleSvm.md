# SimpleSvm AMD-V Pattern Extraction

**Source**: `Refs/codebases/SimpleSvm/`
**Purpose**: Basic AMD-V hypervisor reference implementation
**Author**: Satoshi Tanda
**Relevance**: Critical for Ombra's AMD CPU support

---

## 1. SVM ENABLE SEQUENCE

### Feature Detection
**File**: `SimpleSvm.cpp:1638-1698`

- Check vendor is AMD: `CPUID(0)` returns "AuthenticAMD"
  - `SimpleSvm.cpp:1653-1659`
  ```c
  __cpuid(registers, CPUID_MAX_STANDARD_FN_NUMBER_AND_VENDOR_STRING);
  if ((registers[1] != 'htuA') ||
      (registers[3] != 'itne') ||
      (registers[2] != 'DMAc'))
  ```

- Check SVM feature bit: `CPUID(0x80000001).ECX[2]`
  - `SimpleSvm.cpp:1665-1669`
  ```c
  __cpuid(registers, CPUID_PROCESSOR_AND_PROCESSOR_FEATURE_IDENTIFIERS_EX);
  if ((registers[2] & CPUID_FN8000_0001_ECX_SVM) == 0)
  ```

- Check NPT support: `CPUID(0x8000000A).EDX[0]`
  - `SimpleSvm.cpp:1676-1680`
  ```c
  __cpuid(registers, CPUID_SVM_FEATURES);
  if ((registers[3] & CPUID_FN8000_000A_EDX_NP) == 0)
  ```

- Check VM_CR.SVMDIS not set: `MSR 0xc0010114[4]`
  - `SimpleSvm.cpp:1688-1692`
  - `SimpleSvm.hpp:22-25`
  ```c
  vmcr = __readmsr(SVM_MSR_VM_CR);
  if ((vmcr & SVM_VM_CR_SVMDIS) != 0)
  ```

### EFER.SVME Setup
**File**: `SimpleSvm.cpp:1205-1209`

- Enable SVM before VMRUN: set `EFER[12]`
  - `SimpleSvm.cpp:245-247` (constant definitions)
  ```c
  #define IA32_MSR_EFER   0xc0000080
  #define EFER_SVME       (1UL << 12)

  __writemsr(IA32_MSR_EFER, __readmsr(IA32_MSR_EFER) | EFER_SVME);
  ```

### VM_HSAVE_PA Setup
**File**: `SimpleSvm.cpp:1114-1118`

- Allocate host state area: page-aligned 4KB
  - `SimpleSvm.cpp:209` (in VIRTUAL_PROCESSOR_DATA)
  ```c
  DECLSPEC_ALIGN(PAGE_SIZE) UINT8 HostStateArea[PAGE_SIZE];
  ```

- Write physical address to MSR
  - `SimpleSvm.hpp:23` (MSR definition)
  - `SimpleSvm.cpp:1118`
  ```c
  #define SVM_MSR_VM_HSAVE_PA  0xc0010117

  __writemsr(SVM_MSR_VM_HSAVE_PA, hostStateAreaPa.QuadPart);
  ```

---

## 2. VMCB STRUCTURE

### Control Area Layout
**File**: `SimpleSvm.hpp:35-77`

**Critical Fields**:
- `+0x00C`: `InterceptMisc1` - Intercept bitmap 1
  - `SimpleSvm.hpp:30-31` - CPUID intercept bit 18
  - `SimpleSvm.hpp:31` - MSR protection bit 28
- `+0x010`: `InterceptMisc2` - Intercept bitmap 2
  - `SimpleSvm.hpp:32` - VMRUN intercept bit 0
- `+0x048`: `MsrpmBasePa` - MSR permissions map PA
- `+0x058`: `GuestAsid` - ASID for TLB tagging
- `+0x070`: `ExitCode` - VMEXIT reason
- `+0x078`: `ExitInfo1` - Additional exit info
- `+0x080`: `ExitInfo2` - Additional exit info
- `+0x090`: `NpEnable` - NPT enable flag
  - `SimpleSvm.hpp:33` - NPT enable bit 0
- `+0x0B0`: `NCr3` - NPT page table base PA
- `+0x0C8`: `NRip` - Next RIP after instruction

**Usage**: `SimpleSvm.cpp:1021-1054`

### State Save Area Layout
**File**: `SimpleSvm.hpp:82-158`

**Segment Registers** (`+0x000 - +0x0A0`):
- Each segment: Selector + Attributes + Limit + Base
- ES, CS, SS, DS, FS, GS, GDTR, LDTR, IDTR, TR

**Control Registers**:
- `+0x0D0`: `Efer`
- `+0x148`: `Cr4`
- `+0x150`: `Cr3`
- `+0x158`: `Cr0`

**GP Registers**:
- `+0x1F8`: `Rax` (only RAX stored in VMCB)
- `+0x1D8`: `Rsp`
- `+0x178`: `Rip`
- `+0x170`: `Rflags`

**Syscall MSRs** (`+0x200 - +0x238`):
- STAR, LSTAR, CSTAR, SFMASK, KernelGsBase, SYSENTER_*

### Full VMCB
**File**: `SimpleSvm.hpp:163-170`

```c
typedef struct _VMCB
{
    VMCB_CONTROL_AREA ControlArea;      // 0x000-0x3FF (1KB)
    VMCB_STATE_SAVE_AREA StateSaveArea; // 0x400-0x697 (664 bytes)
    UINT8 Reserved1[...];                // Pad to 4KB
} VMCB;
static_assert(sizeof(VMCB) == 0x1000);
```

**Allocation**: `SimpleSvm.cpp:207-208`
```c
DECLSPEC_ALIGN(PAGE_SIZE) VMCB GuestVmcb;
DECLSPEC_ALIGN(PAGE_SIZE) VMCB HostVmcb;
```

---

## 3. VMRUN/VMEXIT FLOW

### VMRUN Execution
**File**: `x64.asm:78-123`

**Pre-VMRUN**:
- `x64.asm:84`: Switch to host stack (`mov rsp, rcx`)
- `x64.asm:98`: Load VMCB PA into RAX
- `x64.asm:99`: Execute `vmload rax` - loads guest FS/GS/TR/LDTR/SYSENTER/etc
- `x64.asm:123`: Execute `vmrun rax` - enter guest

**VMRUN behavior** (documented in `x64.asm:107-121`):
1. Saves host state to VM_HSAVE_PA area
2. Loads guest state from VMCB
3. Sets GIF (enables interrupts)
4. Resumes guest execution

### VMEXIT Handling Entry
**File**: `x64.asm:125-189`

**Post-VMEXIT**:
- `x64.asm:132`: Execute `vmsave rax` - save guest FS/GS/TR/LDTR/etc back to VMCB
- `x64.asm:140`: Allocate trap frame (for Windbg stack traces)
- `x64.asm:147`: Save all GP registers with `PUSHAQ` macro
- `x64.asm:166-167`: Set parameters for C handler:
  ```asm
  mov rdx, rsp                                ; GuestRegisters
  mov rcx, [rsp + 8 * 18 + KTRAP_FRAME_SIZE]  ; VpData
  ```
- `x64.asm:177-183`: Save volatile XMM0-XMM5
- `x64.asm:189`: Call `SvHandleVmExit`

**VMEXIT behavior** (documented in `x64.asm:115-121`):
1. Clears GIF (disables interrupts)
2. Saves guest state to VMCB
3. Loads host state from VM_HSAVE_PA

### C-Level Exit Handler
**File**: `SimpleSvm.cpp:740-872`

**Entry**: `SvHandleVmExit(VpData, GuestRegisters)`

**Sequence**:
1. `SimpleSvm.cpp:758`: Load host state with `__svm_vmload(HostVmcbPa)`
2. `SimpleSvm.cpp:772-776`: Raise IRQL to DISPATCH_LEVEL
3. `SimpleSvm.cpp:782`: Restore guest RAX from VMCB to register context
4. `SimpleSvm.cpp:789-790`: Update trap frame for Windbg
5. `SimpleSvm.cpp:795-810`: Dispatch based on ExitCode:
   - `VMEXIT_CPUID` → `SvHandleCpuid`
   - `VMEXIT_MSR` → `SvHandleMsrAccess`
   - `VMEXIT_VMRUN` → `SvHandleVmrun` (inject #GP)
   - Default → Bugcheck
6. `SimpleSvm.cpp:817-820`: Lower IRQL
7. `SimpleSvm.cpp:867`: Save guest RAX back to VMCB
8. Return to assembly

### Exit Info Extraction
**File**: `SimpleSvm.cpp:610-611`

- MSR write detection:
  ```c
  writeAccess = (VpData->GuestVmcb.ControlArea.ExitInfo1 != 0);
  ```
- ExitInfo1/ExitInfo2 usage varies by exit code (see AMD manual)

### Resume Guest
**File**: `x64.asm:206-215`

- `x64.asm:206`: Test return value (`al`)
- `x64.asm:207`: Restore GP registers with `POPAQ`
- `x64.asm:213`: If `al == 0`, continue loop (`jnz` not taken)
- `x64.asm:214-215`: Restore trap frame, jump back to `SvLV10` (VMRUN)

---

## 4. NPT CONFIGURATION

### NPT Structure
**File**: `SimpleSvm.cpp:170-174`

```c
typedef struct _PML4E_TREE
{
    DECLSPEC_ALIGN(PAGE_SIZE) PDPT_ENTRY_2MB PdptEntries[512];
    DECLSPEC_ALIGN(PAGE_SIZE) PD_ENTRY_2MB PdEntries[512][512];
} PML4E_TREE;
```

**Shared Data**: `SimpleSvm.cpp:176-181`
```c
typedef struct _SHARED_VIRTUAL_PROCESSOR_DATA
{
    PVOID MsrPermissionsMap;
    DECLSPEC_ALIGN(PAGE_SIZE) PML4_ENTRY_2MB Pml4Entries[512];
    DECLSPEC_ALIGN(PAGE_SIZE) PML4E_TREE Pml4eTrees[2];  // For 1TB
}
```

### NPT Page Table Entries
**File**: `SimpleSvm.cpp:43-99`

**PML4/PDPT Entry** (same structure):
- Bit 0: Valid
- Bit 1: Write
- Bit 2: User
- Bit 63: NoExecute
- Bits 12-51: PageFrameNumber (PFN)

**PD Entry (2MB large page)**:
- Bit 7: LargePage (must be 1)
- Other bits same as PML4/PDPT

### NPT Construction
**File**: `SimpleSvm.cpp:1524-1624`

**Build sequence**:
1. Create 2 PML4 entries (covers 1TB total)
   - `SimpleSvm.cpp:1539-1560`: Set PML4 Valid/Write/User bits
2. Each PML4 → 512 PDPT entries
   - `SimpleSvm.cpp:1564-1573`: Set PDPT Valid/Write/User bits
3. Each PDPT → 512 PD entries (2MB large pages)
   - `SimpleSvm.cpp:1607-1620`: Set PD Valid/Write/User/LargePage bits
   - Identity mapping: PA = GPA

**Permission bits** (different from Intel EPT!):
- `SimpleSvm.cpp:1544-1559`: All entries set User=1, Write=1
  - Required because guest accesses are user-level at NPT level
  - Guest page tables still enforce actual permissions

**PAT handling**: `SimpleSvm.cpp:1578-1605`
- NPT does NOT explicitly configure PAT bits
- Relies on MTRR + guest PAT for memory type
- Acceptable tradeoff documented in code comments

### NPT Enable
**File**: `SimpleSvm.cpp:1042-1054`

```c
VpData->GuestVmcb.ControlArea.NpEnable |= SVM_NP_ENABLE_NP_ENABLE;
VpData->GuestVmcb.ControlArea.NCr3 = pml4BasePa.QuadPart;
```

- `NpEnable` bit 0 = enable NPT
- `NCr3` = physical address of NPT PML4 table

### ASID Configuration
**File**: `SimpleSvm.cpp:1032-1039`

```c
VpData->GuestVmcb.ControlArea.GuestAsid = 1;
```

- ASID 0 is reserved/illegal
- ASID 1 used for simplicity (single guest)
- All processors share ASID 1 (single guest across all CPUs)
- Max ASID from `CPUID(0x8000000A).EBX`

---

## 5. INTERCEPT BITMAP

### Intercept Setup
**File**: `SimpleSvm.cpp:1012-1029`

**CPUID intercept**:
```c
VpData->GuestVmcb.ControlArea.InterceptMisc1 |= SVM_INTERCEPT_MISC1_CPUID;
```
- `SimpleSvm.hpp:30`: `#define SVM_INTERCEPT_MISC1_CPUID (1UL << 18)`
- Reason: Hypervisor presence indication + unload interface

**VMRUN intercept**:
```c
VpData->GuestVmcb.ControlArea.InterceptMisc2 |= SVM_INTERCEPT_MISC2_VMRUN;
```
- `SimpleSvm.hpp:32`: `#define SVM_INTERCEPT_MISC2_VMRUN (1UL << 0)`
- Reason: Required by AMD spec to avoid VMEXIT_INVALID

**MSR intercept**:
```c
VpData->GuestVmcb.ControlArea.InterceptMisc1 |= SVM_INTERCEPT_MISC1_MSR_PROT;
VpData->GuestVmcb.ControlArea.MsrpmBasePa = msrpmPa.QuadPart;
```
- `SimpleSvm.hpp:31`: `#define SVM_INTERCEPT_MISC1_MSR_PROT (1UL << 28)`
- Enables MSR permission map filtering

### MSR Permission Map
**File**: `SimpleSvm.cpp:1467-1502`

**Map structure**:
- Size: `2 * PAGE_SIZE` (8KB)
  - `SimpleSvm.hpp:17`: `#define SVM_MSR_PERMISSIONS_MAP_SIZE (PAGE_SIZE * 2)`
- 2 bits per MSR: bit 0 = read intercept, bit 1 = write intercept

**EFER write interception**:
```c
constexpr UINT32 BITS_PER_MSR = 2;
constexpr UINT32 SECOND_MSR_RANGE_BASE = 0xc0000000;
constexpr UINT32 SECOND_MSRPM_OFFSET = 0x800 * CHAR_BIT;

offsetFrom2ndBase = (IA32_MSR_EFER - SECOND_MSR_RANGE_BASE) * BITS_PER_MSR;
offset = SECOND_MSRPM_OFFSET + offsetFrom2ndBase;

RtlSetBits(&bitmapHeader, offset + 1, 1);  // Set write-intercept bit
```

**MSR ranges** (from AMD spec):
- 0x00000000 - 0x00001FFF: offset 0x000
- 0xC0000000 - 0xC0001FFF: offset 0x800
- 0xC0010000 - 0xC0011FFF: offset 0x1000

**Allocation**:
- `SimpleSvm.cpp:1756-1763`: Allocate contiguous physical memory
- Must be contiguous because processor DMAs from it

### I/O Permission Map
**Not implemented in SimpleSvm**

- Would use `IopmBasePa` field at `VMCB+0x040`
- Same bitmap concept as MSRPM
- SimpleSvm does not intercept I/O operations

---

## 6. VMEXIT CODES

**File**: `SimpleSvm.hpp:196-360`

**Common codes**:
- `0x0072`: `VMEXIT_CPUID`
- `0x007C`: `VMEXIT_MSR`
- `0x0080`: `VMEXIT_VMRUN`
- `0x0081`: `VMEXIT_VMMCALL`
- `0x0400`: `VMEXIT_NPF` (nested page fault)
- `-1`: `VMEXIT_INVALID`

**CR access**: `0x0000-0x001F` (CR0-CR15 read/write)
**DR access**: `0x0020-0x003F` (DR0-DR15 read/write)
**Exceptions**: `0x0040-0x005F` (vectors 0-31)

---

## 7. KEY PATTERNS FOR OMBRA

### VMCB Initialization
1. Zero-initialize entire VMCB (4KB)
2. Set intercept bits in ControlArea
3. Configure MSRPM/IOPM base PAs
4. Set GuestAsid to non-zero value
5. Enable NPT and set NCr3
6. Capture current state for StateSaveArea
7. Execute VMSAVE to save FS/GS/SYSENTER/etc

### Host Stack Layout
**File**: `SimpleSvm.cpp:184-212`

```c
union {
    UINT8 HostStackLimit[KERNEL_STACK_SIZE];
    struct {
        UINT8 StackContents[...];
        KTRAP_FRAME TrapFrame;
        UINT64 GuestVmcbPa;     // HostRsp points here
        UINT64 HostVmcbPa;
        PVIRTUAL_PROCESSOR_DATA Self;
        PSHARED_VIRTUAL_PROCESSOR_DATA SharedVpData;
        UINT64 Padding1;
        UINT64 Reserved1;
    } HostStackLayout;
};
```

- HostRsp passed to SvLaunchVm points to GuestVmcbPa
- Allows accessing per-CPU data via stack offset

### EFER.SVME Protection
**File**: `SimpleSvm.cpp:614-635`

- Intercept EFER writes via MSRPM
- Check if EFER.SVME is being cleared
- Inject #GP if cleared (undefined behavior otherwise)

### Segment Attributes
**File**: `SimpleSvm.cpp:896-930`

- Extract attributes from GDT descriptor
- Convert to VMCB attribute format
- Fields: Type, System, DPL, Present, AVL, LongMode, DefaultBit, Granularity

### Event Injection
**File**: `SimpleSvm.hpp:175-192`

```c
typedef struct _EVENTINJ
{
    UINT64 Vector : 8;          // Exception/interrupt vector
    UINT64 Type : 3;            // 3 = exception
    UINT64 ErrorCodeValid : 1;  // 1 if error code present
    UINT64 Valid : 1;           // 1 to inject
    UINT64 ErrorCode : 32;      // Error code value
} EVENTINJ;
```

**Usage**: `SimpleSvm.cpp:456-469` (#GP injection)

---

## 8. SIMPLICITY CHOICES

SimpleSvm intentionally omits:
- I/O intercepts (no IOPM setup)
- Exception intercepts (except via NPT faults)
- Interrupt virtualization (AVIC)
- 1GB huge pages (VMware compatibility)
- Complex ASID management (single guest)
- CR intercepts
- Full MSR emulation (passes through most MSRs)

These are acceptable for Ombra **initially**, but may need extension for:
- Anti-detection (CR4, CPUID more complex)
- Hook stability (need #PF intercepts)
- Performance (need interrupt virtualization?)

---

## 9. CRITICAL DIFFERENCES: AMD vs INTEL

| Aspect | AMD SVM | Intel VMX |
|--------|---------|-----------|
| **Enable MSR** | EFER.SVME (bit 12) | IA32_FEATURE_CONTROL[2] + CR4.VMXE |
| **State structure** | VMCB (single 4KB block) | VMCS (opaque, accessed via VMREAD/VMWRITE) |
| **Enter guest** | VMRUN instruction | VMLAUNCH/VMRESUME |
| **NPT/EPT enable** | VMCB.NpEnable[0] + NCr3 | VMCS Secondary Proc Controls + EPTP |
| **NPT/EPT permissions** | User bit required for all | Execute-only pages supported |
| **ASID/VPID** | VMCB.GuestAsid (required) | VMCS VPID (optional) |
| **Host state save** | VM_HSAVE_PA MSR area | VMCS host-state fields |
| **Segment state** | VMSAVE/VMLOAD instructions | Loaded from VMCS on exit/entry |
| **Next RIP** | VMCB.NRip (auto-saved) | Must manually add instruction length |
| **MSR intercept** | Bitmap (2 bits per MSR) | Bitmap (1 bit per MSR) + MSR load/store lists |
| **GIF** | Cleared on VMEXIT, set on VMRUN | No equivalent (IF used) |

**Ombra implications**:
- Need separate VMCB vs VMCS setup paths
- NPT User bit always set (unlike EPT)
- ASID mandatory on AMD, VPID optional on Intel
- Can use NRip directly on AMD, must calculate on Intel

---

## 10. FILES SUMMARY

| File | Lines | Purpose |
|------|-------|---------|
| **SimpleSvm.hpp** | 361 | SVM structures, constants, VMEXIT codes |
| **SimpleSvm.cpp** | 2009 | All C logic (init, NPT, MSRPM, handlers) |
| **x64.asm** | 246 | VMRUN loop, GP register save/restore |

**Total implementation**: ~2,600 lines for basic AMD-V hypervisor

---

## 11. OMBRA INTEGRATION CHECKLIST

- [ ] Adapt VMCB structure to OmbraPayload
- [ ] Port NPT build logic (identity mapping 1TB)
- [ ] Port MSRPM setup for EFER protection
- [ ] Implement VMRUN/VMEXIT assembly loop
- [ ] Add VMEXIT dispatch table for AMD
- [ ] Unify ASID handling with Intel VPID
- [ ] Test EFER.SVME protection on AMD hardware
- [ ] Verify NPT User bit semantics
- [ ] Compare performance: 2MB vs 1GB pages
- [ ] Ensure VM_HSAVE_PA per-CPU allocation

---

**Extraction complete**: 2025-12-20
**Next reference**: SimpleSvmHook for AMD syscall hooking patterns
