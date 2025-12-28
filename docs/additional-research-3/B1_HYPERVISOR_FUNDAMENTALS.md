# B1 - Hypervisor Fundamentals: Writing a Type-1 Hypervisor

This document provides comprehensive technical guidance for implementing a hypervisor that hijacks Windows Hyper-V. It covers Intel VT-x and AMD SVM fundamentals, unified architecture patterns, memory management with EPT/NPT, and practical code examples from the Sputnik, NoirVisor, SimpleSvm, and DmaBackdoorHv reference implementations.

---

## Table of Contents

1. [Architecture Overview](#1-architecture-overview)
2. [Intel VT-x Fundamentals](#2-intel-vt-x-fundamentals)
3. [AMD SVM Fundamentals](#3-amd-svm-fundamentals)
4. [Unified Architecture Design](#4-unified-architecture-design)
5. [Memory Management (EPT/NPT)](#5-memory-management-eptnpt)
6. [VM-Exit Handling](#6-vm-exit-handling)
7. [Practical Code Patterns from References](#7-practical-code-patterns-from-references)
8. [Hypercall Protocol](#8-hypercall-protocol)
9. [Guest Memory Access](#9-guest-memory-access)
10. [Signature-Based Hyper-V Hooking](#10-signature-based-hyper-v-hooking)

---

## 1. Architecture Overview

### 1.1 Hypervisor Types

**Type-1 (Bare-Metal)**: Runs directly on hardware, manages guest OSes.
**Type-2 (Hosted)**: Runs on top of a host OS.

OmbraHypervisor operates as a **parasitic Type-1 hypervisor** that injects into an existing hypervisor (Hyper-V) rather than replacing it entirely.

### 1.2 Execution Hierarchy

```
                    +-----------------------+
                    |   Guest Application   |
                    +-----------------------+
                              |
                    +-----------------------+
                    |   Guest OS (Windows)  |
                    +-----------------------+
                              |
                              | VM-Exit (CPUID, MSR, etc.)
                              v
+-----------------------------------------------------------------------+
|                   OmbraPayload (Injected)                              |
|  - Intercepts specific VM-exits before Hyper-V                         |
|  - Handles hypercalls (VMCALL/VMMCALL)                                 |
|  - Memory read/write operations                                        |
+-----------------------------------------------------------------------+
                              |
                              | Pass-through for unhandled exits
                              v
+-----------------------------------------------------------------------+
|                   Hyper-V (hvix64.exe / hvax64.exe)                    |
|  - Original VM-exit handler                                            |
|  - EPT/NPT management                                                  |
|  - VTL separation                                                      |
+-----------------------------------------------------------------------+
                              |
+-----------------------------------------------------------------------+
|                         Hardware (CPU)                                 |
|  - Intel VT-x or AMD-V                                                 |
|  - EPT/NPT hardware support                                            |
+-----------------------------------------------------------------------+
```

### 1.3 Key Concepts

| Concept | Intel Term | AMD Term | Description |
|---------|------------|----------|-------------|
| Control Structure | VMCS | VMCB | Per-vCPU state and control |
| Extended Page Tables | EPT | NPT | Second-level address translation |
| Enter Guest | VMLAUNCH/VMRESUME | VMRUN | Start guest execution |
| Exit Guest | VM-exit | #VMEXIT | Return to hypervisor |
| Hypercall | VMCALL | VMMCALL | Guest-to-hypervisor call |

---

## 2. Intel VT-x Fundamentals

### 2.1 VMX Operation Prerequisites

Before enabling VMX operations, verify hardware support and configure control registers.

**CPUID Detection:**
```cpp
// Reference: A2_INTEL_VIRT_KNOWLEDGE.md

// Step 1: Check CPUID.1:ECX.VMX[bit 5]
int cpu_info[4] = {};
__cpuid(cpu_info, 1);
bool vmx_supported = (cpu_info[2] >> 5) & 1;  // ECX bit 5

// Step 2: Check IA32_VMX_BASIC for memory type
uint64_t vmx_basic = __readmsr(0x480);  // IA32_VMX_BASIC
uint8_t memory_type = (vmx_basic >> 50) & 0xF;
bool wb_supported = (memory_type == 6);  // Write-back required

// Step 3: Check IA32_FEATURE_CONTROL MSR
uint64_t feature_control = __readmsr(0x3A);  // IA32_FEATURE_CONTROL
bool lock_bit = feature_control & 1;
bool vmxon_enabled = (feature_control >> 2) & 1;

// If lock bit is clear, we can configure it
if (!lock_bit) {
    feature_control |= 1;         // Lock bit
    feature_control |= (1 << 2);  // Enable VMXON outside SMX
    __writemsr(0x3A, feature_control);
}
```

**CR0/CR4 Fixed Bits:**
```cpp
// Apply required fixed bits before VMXON
uint64_t cr0_fixed0 = __readmsr(0x486);  // IA32_VMX_CR0_FIXED0
uint64_t cr0_fixed1 = __readmsr(0x487);  // IA32_VMX_CR0_FIXED1
uint64_t cr0 = __readcr0();
cr0 &= cr0_fixed1;  // Clear bits that must be 0
cr0 |= cr0_fixed0;  // Set bits that must be 1
__writecr0(cr0);

uint64_t cr4_fixed0 = __readmsr(0x488);  // IA32_VMX_CR4_FIXED0
uint64_t cr4_fixed1 = __readmsr(0x489);  // IA32_VMX_CR4_FIXED1
uint64_t cr4 = __readcr4();
cr4 &= cr4_fixed1;
cr4 |= cr4_fixed0;  // Includes VMXE bit
__writecr4(cr4);
```

### 2.2 VMXON Region Setup

The VMXON region is a 4KB page-aligned memory block that enables VMX operation.

```cpp
typedef struct _VmControlStructure {
    uint32_t revision_identifier;  // Must match IA32_VMX_BASIC[30:0]
    uint32_t vmx_abort_indicator;  // Set by processor on VMX abort
    uint8_t  data[4088];           // Implementation-specific
} VmControlStructure;

// Allocation (4KB, page-aligned, write-back cacheable)
VmControlStructure* vmxon_region = AllocateContiguousMemory(4096);
memset(vmxon_region, 0, 4096);

// Write revision ID from IA32_VMX_BASIC
uint64_t vmx_basic = __readmsr(0x480);
vmxon_region->revision_identifier = (uint32_t)(vmx_basic & 0x7FFFFFFF);

// Execute VMXON
uint64_t vmxon_phys = GetPhysicalAddress(vmxon_region);
int result = __vmx_on(&vmxon_phys);
if (result != 0) {
    // VMXON failed - check prerequisites
    return false;
}
```

### 2.3 VMCS Setup

The Virtual Machine Control Structure contains guest state, host state, and control fields.

**VMCS Initialization Sequence:**
```cpp
// 1. Allocate VMCS region (same requirements as VMXON)
VmControlStructure* vmcs_region = AllocateContiguousMemory(4096);
memset(vmcs_region, 0, 4096);

// 2. Write revision ID
uint64_t vmx_basic = __readmsr(0x480);
vmcs_region->revision_identifier = (uint32_t)(vmx_basic & 0x7FFFFFFF);

// 3. VMCLEAR - Initialize VMCS and set launch state to "clear"
uint64_t vmcs_phys = GetPhysicalAddress(vmcs_region);
if (__vmx_vmclear(&vmcs_phys) != 0) {
    return false;
}

// 4. VMPTRLD - Load VMCS as current
if (__vmx_vmptrld(&vmcs_phys) != 0) {
    return false;
}

// VMCS is now active and ready for VMWRITE
```

**Control Field Adjustment:**

VMX controls have mandatory 0 and 1 bits. Always adjust requested values against capability MSRs.

```cpp
uint32_t AdjustControlValue(uint32_t msr_index, uint32_t requested) {
    uint64_t msr_value = __readmsr(msr_index);
    uint32_t allowed_0 = (uint32_t)(msr_value & 0xFFFFFFFF);
    uint32_t allowed_1 = (uint32_t)(msr_value >> 32);

    uint32_t result = requested;
    result &= allowed_1;  // Clear bits that must be 0
    result |= allowed_0;  // Set bits that must be 1
    return result;
}

// Check for "true" MSR support
uint64_t vmx_basic = __readmsr(0x480);
bool use_true_msrs = (vmx_basic >> 55) & 1;

// Pin-based controls
uint32_t pin_ctl = AdjustControlValue(
    use_true_msrs ? 0x48D : 0x481,
    requested_pin_controls);

// Primary processor-based controls
uint32_t proc_ctl = AdjustControlValue(
    use_true_msrs ? 0x48E : 0x482,
    requested_proc_controls);

// Secondary processor-based controls
uint32_t proc_ctl2 = AdjustControlValue(0x48B, requested_proc2_controls);
```

**Essential VMCS Fields:**

```cpp
void SetupVmcs(uint64_t guest_rsp, uint64_t guest_rip, uint64_t vmm_stack) {
    // === Control Fields ===

    // Pin-based controls
    __vmx_vmwrite(0x4000, AdjustControlValue(MSR_PIN, 0));

    // Primary processor-based controls
    uint32_t proc_ctl = 0;
    proc_ctl |= (1 << 28);  // Use MSR bitmaps
    proc_ctl |= (1 << 31);  // Activate secondary controls
    __vmx_vmwrite(0x4002, AdjustControlValue(MSR_PROC, proc_ctl));

    // Secondary processor-based controls
    uint32_t proc_ctl2 = 0;
    proc_ctl2 |= (1 << 1);  // Enable EPT
    proc_ctl2 |= (1 << 5);  // Enable VPID
    __vmx_vmwrite(0x401E, AdjustControlValue(0x48B, proc_ctl2));

    // VM-exit controls
    uint32_t exit_ctl = 0;
    exit_ctl |= (1 << 9);   // Host address-space size (64-bit)
    exit_ctl |= (1 << 15);  // Acknowledge interrupt on exit
    __vmx_vmwrite(0x400C, AdjustControlValue(MSR_EXIT, exit_ctl));

    // VM-entry controls
    uint32_t entry_ctl = 0;
    entry_ctl |= (1 << 9);  // IA-32e mode guest (64-bit)
    __vmx_vmwrite(0x4012, AdjustControlValue(MSR_ENTRY, entry_ctl));

    // EPT pointer
    __vmx_vmwrite(0x201A, ept_pointer);

    // VPID (must be non-zero)
    __vmx_vmwrite(0x0000, GetProcessorNumber() + 1);

    // === Guest State ===

    // Segment selectors
    __vmx_vmwrite(0x0800, __read_es());   // ES
    __vmx_vmwrite(0x0802, __read_cs());   // CS
    __vmx_vmwrite(0x0804, __read_ss());   // SS
    __vmx_vmwrite(0x0806, __read_ds());   // DS
    __vmx_vmwrite(0x0808, __read_fs());   // FS
    __vmx_vmwrite(0x080A, __read_gs());   // GS
    __vmx_vmwrite(0x080E, __str());       // TR

    // Control registers
    __vmx_vmwrite(0x6800, __readcr0());   // Guest CR0
    __vmx_vmwrite(0x6802, __readcr3());   // Guest CR3
    __vmx_vmwrite(0x6804, __readcr4());   // Guest CR4

    // Guest RSP, RIP, RFLAGS
    __vmx_vmwrite(0x681C, guest_rsp);
    __vmx_vmwrite(0x681E, guest_rip);
    __vmx_vmwrite(0x6820, __readeflags());

    // VMCS link pointer (must be 0xFFFFFFFFFFFFFFFF if not using shadow VMCS)
    __vmx_vmwrite(0x2800, 0xFFFFFFFFFFFFFFFF);

    // === Host State ===

    // Host selectors (must have RPL=0, TI=0)
    __vmx_vmwrite(0x0C00, __read_es() & 0xF8);
    __vmx_vmwrite(0x0C02, __read_cs() & 0xF8);
    __vmx_vmwrite(0x0C04, __read_ss() & 0xF8);
    __vmx_vmwrite(0x0C0C, __str() & 0xF8);

    // Host control registers
    __vmx_vmwrite(0x6C00, __readcr0());
    __vmx_vmwrite(0x6C02, __readcr3());
    __vmx_vmwrite(0x6C04, __readcr4());

    // Host RSP and RIP (critical!)
    __vmx_vmwrite(0x6C14, vmm_stack);
    __vmx_vmwrite(0x6C16, (uint64_t)VmExitHandler);
}
```

### 2.4 VM Entry/Exit Cycle

```
              VMLAUNCH/VMRESUME
                    |
                    v
    +----------------------------------+
    |         Guest Execution          |
    |   (Ring 0/3, normal operation)   |
    +----------------------------------+
                    |
          Intercept/Event occurs
                    |
                    v
    +----------------------------------+
    |           VM-Exit                |
    |  - Save guest state to VMCS      |
    |  - Load host state from VMCS     |
    |  - Set exit reason and info      |
    +----------------------------------+
                    |
                    v
    +----------------------------------+
    |      VM-Exit Handler (Host)      |
    |  - Read exit reason              |
    |  - Handle exit                   |
    |  - Modify guest state if needed  |
    |  - Advance RIP if needed         |
    +----------------------------------+
                    |
          +--------+--------+
          |                 |
     VMRESUME           VMXOFF
          |                 |
          v                 v
    Back to Guest      Exit VMX
```

### 2.5 Key VMCS Field Encodings

| Field | Encoding | Type | Description |
|-------|----------|------|-------------|
| VPID | 0x0000 | 16-bit Control | Virtual Processor ID |
| Guest ES Selector | 0x0800 | 16-bit Guest | ES segment |
| Guest CR3 | 0x6802 | Natural Guest | Guest page table |
| Guest RIP | 0x681E | Natural Guest | Instruction pointer |
| Guest RSP | 0x681C | Natural Guest | Stack pointer |
| Host RIP | 0x6C16 | Natural Host | VM-exit handler |
| Host RSP | 0x6C14 | Natural Host | VMM stack |
| EPT Pointer | 0x201A | 64-bit Control | EPT root |
| Exit Reason | 0x4402 | 32-bit Read-Only | Why we exited |
| Exit Qualification | 0x6400 | Natural Read-Only | Additional info |
| VM-Exit Instruction Length | 0x440C | 32-bit Read-Only | For advancing RIP |

---

## 3. AMD SVM Fundamentals

### 3.1 SVM Support Detection

```cpp
// Reference: A3_AMD_VIRT_KNOWLEDGE.md

BOOLEAN IsSvmSupported(VOID) {
    int registers[4];
    ULONG64 vmcr;

    // Check for "AuthenticAMD" vendor string
    __cpuid(registers, 0x00000000);
    if ((registers[1] != 'htuA') ||  // "Auth"
        (registers[3] != 'itne') ||  // "enti"
        (registers[2] != 'DMAc'))    // "cAMD"
    {
        return FALSE;
    }

    // Check SVM support bit (CPUID.8000_0001.ECX[2])
    __cpuid(registers, 0x80000001);
    if ((registers[2] & (1UL << 2)) == 0) {
        return FALSE;
    }

    // Check NPT support (CPUID.8000_000A.EDX[0])
    __cpuid(registers, 0x8000000A);
    if ((registers[3] & (1UL << 0)) == 0) {
        return FALSE;
    }

    // Check if SVM is disabled in VM_CR MSR
    vmcr = __readmsr(0xC0010114);  // SVM_MSR_VM_CR
    if ((vmcr & (1UL << 4)) != 0)  // SVMDIS bit
    {
        return FALSE;  // SVM disabled by BIOS
    }

    return TRUE;
}
```

### 3.2 Enabling SVM

```cpp
void EnableSvm(void) {
    // Set EFER.SVME (bit 12)
    uint64_t efer = __readmsr(0xC0000080);  // IA32_EFER
    efer |= (1UL << 12);  // SVME bit
    __writemsr(0xC0000080, efer);

    // Configure VM_CR
    uint64_t vmcr = __readmsr(0xC0010114);  // VM_CR
    vmcr |= (1UL << 1);  // DIS_A20M - Block A20 masking
    vmcr |= (1UL << 0);  // R_INIT - Redirect INIT to #SX
    __writemsr(0xC0010114, vmcr);

    // Set VM_HSAVE_PA (host state save area)
    uint64_t hsave_pa = GetPhysicalAddress(host_save_area);
    __writemsr(0xC0010117, hsave_pa);  // VM_HSAVE_PA
}
```

### 3.3 VMCB Structure

The Virtual Machine Control Block is a 4KB structure divided into two areas:

**Control Area (0x000-0x3FF):**
```cpp
typedef struct _VMCB_CONTROL_AREA {
    UINT16 InterceptCrRead;             // +0x000 - CR0-15 read intercepts
    UINT16 InterceptCrWrite;            // +0x002 - CR0-15 write intercepts
    UINT16 InterceptDrRead;             // +0x004 - DR0-15 read intercepts
    UINT16 InterceptDrWrite;            // +0x006 - DR0-15 write intercepts
    UINT32 InterceptException;          // +0x008 - Exception bitmap
    UINT32 InterceptMisc1;              // +0x00C - Intercept vector 1
    UINT32 InterceptMisc2;              // +0x010 - Intercept vector 2
    UINT8  Reserved1[0x03c - 0x014];
    UINT16 PauseFilterThreshold;        // +0x03C
    UINT16 PauseFilterCount;            // +0x03E
    UINT64 IopmBasePa;                  // +0x040 - I/O Permission Map
    UINT64 MsrpmBasePa;                 // +0x048 - MSR Permission Map
    UINT64 TscOffset;                   // +0x050 - TSC offset
    UINT32 GuestAsid;                   // +0x058 - Guest ASID (must be non-zero)
    UINT32 TlbControl;                  // +0x05C - TLB flush control
    UINT64 VIntr;                       // +0x060 - Virtual interrupt
    UINT64 InterruptShadow;             // +0x068
    UINT64 ExitCode;                    // +0x070 - Exit reason
    UINT64 ExitInfo1;                   // +0x078 - Exit info
    UINT64 ExitInfo2;                   // +0x080 - Exit info
    UINT64 ExitIntInfo;                 // +0x088
    UINT64 NpEnable;                    // +0x090 - NPT enable (bit 0)
    UINT64 AvicApicBar;                 // +0x098
    UINT64 GuestGhcb;                   // +0x0A0 - GHCB PA (SEV-ES)
    UINT64 EventInj;                    // +0x0A8 - Event injection
    UINT64 NCr3;                        // +0x0B0 - NPT CR3
    UINT64 LbrVirtualizationEnable;     // +0x0B8
    UINT64 VmcbClean;                   // +0x0C0 - Clean bits
    UINT64 NRip;                        // +0x0C8 - Next RIP (decode assist)
    UINT8  NumOfBytesFetched;           // +0x0D0
    UINT8  GuestInstructionBytes[15];   // +0x0D1 - Fetched instruction
    UINT8  Reserved4[0x400 - 0x0E0];
} VMCB_CONTROL_AREA;
```

**State Save Area (0x400-0xFFF):**
```cpp
typedef struct _VMCB_STATE_SAVE_AREA {
    // Segment Registers (ES, CS, SS, DS, FS, GS at 0x10 intervals)
    UINT16 EsSelector;       // +0x400
    UINT16 EsAttrib;         // +0x402
    UINT32 EsLimit;          // +0x404
    UINT64 EsBase;           // +0x408
    // ... (CS at +0x410, SS at +0x420, etc.)

    // Descriptor Tables
    UINT64 GdtrBase;         // +0x468
    UINT32 GdtrLimit;        // +0x464
    UINT64 IdtrBase;         // +0x488
    UINT32 IdtrLimit;        // +0x484

    // Control Registers
    UINT64 Efer;             // +0x4D0
    UINT64 Cr4;              // +0x548
    UINT64 Cr3;              // +0x550
    UINT64 Cr0;              // +0x558
    UINT64 Dr7;              // +0x560
    UINT64 Dr6;              // +0x568
    UINT64 Rflags;           // +0x570
    UINT64 Rip;              // +0x578
    UINT64 Rsp;              // +0x5D8
    UINT64 Rax;              // +0x5F8 - ONLY RAX is auto-saved!

    // System Call MSRs
    UINT64 Star;             // +0x600
    UINT64 LStar;            // +0x608
    UINT64 CStar;            // +0x610
    UINT64 SfMask;           // +0x618
    UINT64 KernelGsBase;     // +0x620
    UINT64 SysenterCs;       // +0x628
    UINT64 SysenterEsp;      // +0x630
    UINT64 SysenterEip;      // +0x638
    UINT64 Cr2;              // +0x640
    UINT64 GPat;             // +0x668
    UINT64 DbgCtl;           // +0x670
} VMCB_STATE_SAVE_AREA;
```

### 3.4 VMRUN/VMEXIT Flow

```
              __svm_vmload(guestVmcbPa)
                    |
                    | (Load additional guest state)
                    v
                  VMRUN
                    |
                    v
    +----------------------------------+
    |         Guest Execution          |
    |  - Host state saved to HSAVE     |
    |  - Guest state loaded from VMCB  |
    |  - GIF set to 1                  |
    +----------------------------------+
                    |
          Intercept/Event occurs
                    |
                    v
    +----------------------------------+
    |          #VMEXIT                 |
    |  - GIF cleared to 0              |
    |  - Guest state saved to VMCB     |
    |    (partial - only RAX + some)   |
    |  - Host state restored           |
    |  - Exit info in VMCB             |
    +----------------------------------+
                    |
                    v
              __svm_vmsave(guestVmcbPa)
                    |
                    | (Save additional guest state)
                    v
    +----------------------------------+
    |      #VMEXIT Handler (Host)      |
    |  - __svm_vmload(hostVmcbPa)      |
    |  - Read ExitCode                 |
    |  - Handle exit                   |
    |  - Advance RIP if needed         |
    +----------------------------------+
                    |
          +--------+--------+
          |                 |
       VMRUN            Disable SVM
          |                 |
          v                 v
    Back to Guest      Exit Virtualization
```

**Assembly Loop (SimpleSvm pattern):**
```asm
SvLaunchVm:
    mov     rax, [rsp]              ; RAX = GuestVmcbPa
    vmload  rax                     ; Load guest state not restored by VMRUN

SvLoop:
    vmrun   rax                     ; Enter guest (sets GIF)

    ; #VMEXIT occurred (GIF now 0)
    vmsave  rax                     ; Save guest state not saved by #VMEXIT

    ; Push all GPRs (only RAX is saved to VMCB)
    PUSHAQ

    ; Call C handler
    mov     rdx, rsp                ; RDX = GuestRegisters
    mov     rcx, [rsp + VpDataOff]  ; RCX = VpData
    call    SvHandleVmExit

    ; Restore GPRs
    POPAQ

    ; Check if we should continue
    test    al, al
    jz      SvExit
    jmp     SvLoop

SvExit:
    ; Disable SVM and return to guest
    stgi                            ; Enable interrupts
    ; Clear EFER.SVME and restore guest state
```

### 3.5 Key Intercepts

**Intercept Vector 1 (offset 0x00C):**
```cpp
#define SVM_INTERCEPT_MISC1_CPUID    (1 << 18)  // CPUID instruction
#define SVM_INTERCEPT_MISC1_MSR_PROT (1 << 28)  // MSR access (use MSRPM)
#define SVM_INTERCEPT_MISC1_INVD     (1 << 22)  // INVD instruction
```

**Intercept Vector 2 (offset 0x010):**
```cpp
#define SVM_INTERCEPT_MISC2_VMRUN    (1 << 0)   // VMRUN - MANDATORY!
#define SVM_INTERCEPT_MISC2_VMMCALL  (1 << 1)   // VMMCALL (hypercall)
```

### 3.6 Critical AMD Quirks

1. **VMRUN Intercept is Mandatory**: Unlike Intel, AMD requires VMRUN to always be intercepted.

2. **RAX Special Handling**: Only RAX is saved/restored via VMCB. All other GPRs must be manually saved.

3. **GIF Management**: Global Interrupt Flag controls interrupt delivery. VMRUN sets it, #VMEXIT clears it.

4. **NPT User Bit**: All NPT entries MUST have User bit set (nested translations are user-mode).

5. **ASID Must Be Non-Zero**: Guest ASID of 0 is reserved and causes failure.

6. **VMSAVE/VMLOAD Required**: These instructions handle state not automatically managed:
   - FS, GS, TR, LDTR (including hidden portions)
   - KernelGsBase
   - STAR, LSTAR, CSTAR, SFMASK
   - SYSENTER_CS, SYSENTER_ESP, SYSENTER_EIP

---

## 4. Unified Architecture Design

### 4.1 Interface Abstractions

To support both Intel and AMD with minimal code duplication, define abstract interfaces.

**IVmControlStructure:**
```cpp
// Abstracts VMCS (Intel) and VMCB (AMD)
class IVmControlStructure {
public:
    virtual ~IVmControlStructure() = default;

    // Guest state access
    virtual uint64_t GetGuestRip() = 0;
    virtual void SetGuestRip(uint64_t rip) = 0;
    virtual uint64_t GetGuestRsp() = 0;
    virtual void SetGuestRsp(uint64_t rsp) = 0;
    virtual uint64_t GetGuestCr3() = 0;
    virtual void SetGuestCr3(uint64_t cr3) = 0;

    // Exit information
    virtual uint64_t GetExitReason() = 0;
    virtual uint64_t GetExitQualification() = 0;
    virtual uint64_t GetExitInstructionLength() = 0;

    // EPT/NPT root
    virtual uint64_t GetEptPointer() = 0;
    virtual void SetEptPointer(uint64_t eptp) = 0;
};
```

**Intel Implementation:**
```cpp
class IntelVmcs : public IVmControlStructure {
public:
    uint64_t GetGuestRip() override {
        size_t value;
        __vmx_vmread(0x681E, &value);
        return value;
    }

    void SetGuestRip(uint64_t rip) override {
        __vmx_vmwrite(0x681E, rip);
    }

    uint64_t GetExitReason() override {
        size_t value;
        __vmx_vmread(0x4402, &value);
        return value & 0xFFFF;  // Basic reason only
    }

    uint64_t GetExitInstructionLength() override {
        size_t value;
        __vmx_vmread(0x440C, &value);
        return value;
    }

    uint64_t GetEptPointer() override {
        size_t value;
        __vmx_vmread(0x201A, &value);
        return value;
    }
    // ... etc
};
```

**AMD Implementation:**
```cpp
class AmdVmcb : public IVmControlStructure {
private:
    VMCB* vmcb;

public:
    uint64_t GetGuestRip() override {
        return vmcb->StateSaveArea.Rip;
    }

    void SetGuestRip(uint64_t rip) override {
        vmcb->StateSaveArea.Rip = rip;
    }

    uint64_t GetExitReason() override {
        return vmcb->ControlArea.ExitCode;
    }

    uint64_t GetExitInstructionLength() override {
        // AMD provides NextRip directly (if decode assist is enabled)
        return vmcb->ControlArea.NRip - vmcb->StateSaveArea.Rip;
    }

    uint64_t GetEptPointer() override {
        return vmcb->ControlArea.NCr3;
    }
    // ... etc
};
```

### 4.2 Vendor Dispatch Table

```cpp
typedef enum _CPU_VENDOR {
    CPU_VENDOR_UNKNOWN = 0,
    CPU_VENDOR_INTEL,
    CPU_VENDOR_AMD
} CPU_VENDOR;

typedef struct _VENDOR_DISPATCH {
    CPU_VENDOR Vendor;

    // VM Control Structure factory
    IVmControlStructure* (*CreateVmcs)(void);

    // Page table factory
    IPageTableManager* (*CreatePageTables)(void);

    // Hypercall instruction
    void (*ExecuteHypercall)(uint64_t code, void* data);

    // Exit reason translation
    uint32_t (*TranslateExitReason)(uint64_t vendor_reason);

} VENDOR_DISPATCH;

// Global dispatch table (set at boot time)
extern VENDOR_DISPATCH* g_VendorDispatch;

// Runtime CPU detection
CPU_VENDOR DetectCpuVendor(void) {
    int regs[4];
    __cpuid(regs, 0);

    // "GenuineIntel"
    if (regs[1] == 0x756E6547 &&
        regs[3] == 0x49656E69 &&
        regs[2] == 0x6C65746E) {
        return CPU_VENDOR_INTEL;
    }

    // "AuthenticAMD"
    if (regs[1] == 0x68747541 &&
        regs[3] == 0x69746E65 &&
        regs[2] == 0x444D4163) {
        return CPU_VENDOR_AMD;
    }

    return CPU_VENDOR_UNKNOWN;
}
```

### 4.3 Unified Exit Reason Mapping

```cpp
typedef enum _UNIFIED_EXIT_REASON {
    EXIT_REASON_CPUID = 0,
    EXIT_REASON_MSR_READ,
    EXIT_REASON_MSR_WRITE,
    EXIT_REASON_CR_ACCESS,
    EXIT_REASON_HYPERCALL,      // VMCALL or VMMCALL
    EXIT_REASON_EPT_VIOLATION,  // Intel EPT or AMD NPF
    EXIT_REASON_EXCEPTION,
    EXIT_REASON_UNKNOWN
} UNIFIED_EXIT_REASON;

// Intel exit reason translation
UNIFIED_EXIT_REASON IntelTranslateExitReason(uint64_t reason) {
    switch (reason & 0xFFFF) {
        case 10:  return EXIT_REASON_CPUID;
        case 31:  return EXIT_REASON_MSR_READ;
        case 32:  return EXIT_REASON_MSR_WRITE;
        case 28:  return EXIT_REASON_CR_ACCESS;
        case 18:  return EXIT_REASON_HYPERCALL;
        case 48:  return EXIT_REASON_EPT_VIOLATION;
        case 0:   return EXIT_REASON_EXCEPTION;
        default:  return EXIT_REASON_UNKNOWN;
    }
}

// AMD exit reason translation
UNIFIED_EXIT_REASON AmdTranslateExitReason(uint64_t code) {
    if (code == 0x72) return EXIT_REASON_CPUID;
    if (code == 0x7C) {
        // Check ExitInfo1 bit 0 for read vs write
        // Return MSR_READ or MSR_WRITE
    }
    if (code >= 0x10 && code <= 0x1F) return EXIT_REASON_CR_ACCESS;
    if (code == 0x81) return EXIT_REASON_HYPERCALL;
    if (code == 0x400) return EXIT_REASON_EPT_VIOLATION;
    if (code >= 0x40 && code <= 0x5F) return EXIT_REASON_EXCEPTION;
    return EXIT_REASON_UNKNOWN;
}
```

---

## 5. Memory Management (EPT/NPT)

### 5.1 EPT Structure (Intel)

Extended Page Tables provide second-level address translation (GPA to HPA).

**EPT Pointer Format:**
```cpp
typedef union _EPT_POINTER {
    struct {
        uint64_t memory_type : 3;      // 0=UC, 6=WB
        uint64_t page_walk_length : 3; // 3 = 4-level (N-1)
        uint64_t enable_ad_bits : 1;   // Accessed/dirty tracking
        uint64_t reserved1 : 5;
        uint64_t pml4_pfn : 40;        // PML4 physical frame
        uint64_t reserved2 : 12;
    };
    uint64_t all;
} EPT_POINTER;

// Setup EPT pointer
EPT_POINTER eptp = {0};
eptp.memory_type = 6;        // Write-back
eptp.page_walk_length = 3;   // 4-level
eptp.pml4_pfn = ept_pml4_pa >> 12;

__vmx_vmwrite(0x201A, eptp.all);
```

**EPT Page Table Entry (4KB page):**
```cpp
typedef union _EPT_PTE {
    struct {
        uint64_t read : 1;
        uint64_t write : 1;
        uint64_t execute : 1;
        uint64_t memory_type : 3;
        uint64_t ignore_pat : 1;
        uint64_t reserved1 : 1;
        uint64_t accessed : 1;
        uint64_t dirty : 1;
        uint64_t execute_user : 1;
        uint64_t reserved2 : 1;
        uint64_t pfn : 40;
        uint64_t reserved3 : 11;
        uint64_t suppress_ve : 1;
    };
    uint64_t all;
} EPT_PTE;
```

### 5.2 NPT Structure (AMD)

Nested Page Tables use standard AMD64 page table format with slight differences.

```cpp
typedef union _NPT_PDE_2MB {
    struct {
        uint64_t present : 1;
        uint64_t write : 1;
        uint64_t user : 1;       // MUST be 1 for NPT
        uint64_t pwt : 1;
        uint64_t pcd : 1;
        uint64_t accessed : 1;
        uint64_t dirty : 1;
        uint64_t large_page : 1; // Must be 1 for 2MB page
        uint64_t global : 1;
        uint64_t reserved1 : 3;
        uint64_t pat : 1;
        uint64_t reserved2 : 8;
        uint64_t pfn : 31;       // 2MB-aligned physical frame
        uint64_t reserved3 : 11;
        uint64_t no_execute : 1;
    };
    uint64_t all;
} NPT_PDE_2MB;
```

### 5.3 Identity Mapping Setup

Identity mapping means GPA == HPA for all physical memory. This is the simplest configuration.

**Intel EPT Identity Map:**
```cpp
void BuildEptIdentityMap(EPT_PML4E* pml4) {
    // Each PML4E covers 512GB
    // Each PDPTE covers 1GB
    // Each PDE covers 2MB (using large pages)

    for (int pml4_idx = 0; pml4_idx < 4; pml4_idx++) {
        EPT_PDPTE* pdpt = AllocatePageAligned(PAGE_SIZE);
        pml4[pml4_idx].all = 0;
        pml4[pml4_idx].read = 1;
        pml4[pml4_idx].write = 1;
        pml4[pml4_idx].execute = 1;
        pml4[pml4_idx].pfn = GetPhysicalAddress(pdpt) >> 12;

        for (int pdpt_idx = 0; pdpt_idx < 512; pdpt_idx++) {
            EPT_PDE_2MB* pd = AllocatePageAligned(PAGE_SIZE);
            pdpt[pdpt_idx].all = 0;
            pdpt[pdpt_idx].read = 1;
            pdpt[pdpt_idx].write = 1;
            pdpt[pdpt_idx].execute = 1;
            pdpt[pdpt_idx].pfn = GetPhysicalAddress(pd) >> 12;

            for (int pd_idx = 0; pd_idx < 512; pd_idx++) {
                // Calculate physical address for this 2MB region
                uint64_t phys = ((uint64_t)pml4_idx << 39) |
                               ((uint64_t)pdpt_idx << 30) |
                               ((uint64_t)pd_idx << 21);

                pd[pd_idx].all = 0;
                pd[pd_idx].read = 1;
                pd[pd_idx].write = 1;
                pd[pd_idx].execute = 1;
                pd[pd_idx].large_page = 1;
                pd[pd_idx].memory_type = 6;  // Write-back
                pd[pd_idx].pfn = phys >> 21;
            }
        }
    }
}
```

**AMD NPT Identity Map:**
```cpp
// Reference: SimpleSvm/SimpleSvm.cpp:1524-1624

void BuildNptIdentityMap(NPT_PML4E* pml4) {
    for (int pml4_idx = 0; pml4_idx < 2; pml4_idx++) {
        NPT_PDPTE* pdpt = AllocatePageAligned(PAGE_SIZE);

        // IMPORTANT: User bit MUST be set for NPT!
        pml4[pml4_idx].present = 1;
        pml4[pml4_idx].write = 1;
        pml4[pml4_idx].user = 1;  // Required!
        pml4[pml4_idx].pfn = GetPhysicalAddress(pdpt) >> 12;

        for (int pdpt_idx = 0; pdpt_idx < 512; pdpt_idx++) {
            NPT_PDE_2MB* pd = AllocatePageAligned(PAGE_SIZE);

            pdpt[pdpt_idx].present = 1;
            pdpt[pdpt_idx].write = 1;
            pdpt[pdpt_idx].user = 1;  // Required!
            pdpt[pdpt_idx].pfn = GetPhysicalAddress(pd) >> 12;

            for (int pd_idx = 0; pd_idx < 512; pd_idx++) {
                uint64_t phys_2mb = ((pml4_idx * 512) + pdpt_idx) * 512 + pd_idx;

                pd[pd_idx].present = 1;
                pd[pd_idx].write = 1;
                pd[pd_idx].user = 1;  // Required!
                pd[pd_idx].large_page = 1;
                pd[pd_idx].pfn = phys_2mb;
            }
        }
    }
}
```

### 5.4 EPT/NPT Violation Handling

When a guest accesses memory without proper permissions, an EPT violation (Intel) or NPF (AMD) occurs.

**Intel EPT Violation Exit Qualification:**
```cpp
typedef union _EPT_VIOLATION_QUALIFICATION {
    struct {
        uint64_t read_access : 1;
        uint64_t write_access : 1;
        uint64_t execute_access : 1;
        uint64_t ept_readable : 1;
        uint64_t ept_writeable : 1;
        uint64_t ept_executable : 1;
        uint64_t ept_executable_user : 1;
        uint64_t guest_linear_valid : 1;
        uint64_t caused_by_translation : 1;
        uint64_t user_mode : 1;
        uint64_t rw_page : 1;
        uint64_t xd_page : 1;
        uint64_t nmi_unblocking : 1;
        uint64_t reserved : 51;
    };
    uint64_t all;
} EPT_VIOLATION_QUALIFICATION;
```

**AMD NPF Error Code:**
```cpp
typedef union _NPT_FAULT_CODE {
    struct {
        uint64_t present : 1;        // Page was present
        uint64_t write : 1;          // Write access
        uint64_t user : 1;           // User access
        uint64_t reserved_bit : 1;   // Reserved bit violation
        uint64_t execute : 1;        // Execute access
        uint64_t reserved1 : 27;
        uint64_t npf_addr : 1;       // NPF in final address
        uint64_t npf_table : 1;      // NPF in page table
        uint64_t reserved2 : 30;
    };
    uint64_t all;
} NPT_FAULT_CODE;
```

---

## 6. VM-Exit Handling

### 6.1 Common Exit Reasons

| Exit Type | Intel Code | AMD Code | Description |
|-----------|------------|----------|-------------|
| CPUID | 10 | 0x72 | CPUID instruction executed |
| RDMSR | 31 | 0x7C (info1=0) | Read MSR |
| WRMSR | 32 | 0x7C (info1=1) | Write MSR |
| CR Access | 28 | 0x00-0x1F | CR0/CR3/CR4 access |
| Hypercall | 18 | 0x81 | VMCALL/VMMCALL |
| EPT Violation | 48 | 0x400 | EPT/NPT violation |
| Triple Fault | 2 | 0x7F | Triple fault |

### 6.2 Unified Exit Handler Pattern

```cpp
// Reference: Sputnik PayLoad Intel/AMD patterns

bool HandleVmExit(IVmControlStructure* vmcs, GuestRegisters* regs) {
    UNIFIED_EXIT_REASON reason = g_VendorDispatch->TranslateExitReason(
        vmcs->GetExitReason());

    bool handled = false;
    bool advance_rip = true;

    switch (reason) {
        case EXIT_REASON_CPUID:
            handled = HandleCpuid(vmcs, regs);
            break;

        case EXIT_REASON_HYPERCALL:
            handled = HandleHypercall(vmcs, regs);
            break;

        case EXIT_REASON_MSR_READ:
            handled = HandleMsrRead(vmcs, regs);
            break;

        case EXIT_REASON_MSR_WRITE:
            handled = HandleMsrWrite(vmcs, regs);
            break;

        case EXIT_REASON_EPT_VIOLATION:
            handled = HandleEptViolation(vmcs, regs);
            advance_rip = false;  // Don't advance RIP for page faults
            break;

        default:
            handled = false;
            break;
    }

    if (handled && advance_rip) {
        // Advance RIP past the instruction
        uint64_t rip = vmcs->GetGuestRip();
        uint64_t len = vmcs->GetExitInstructionLength();
        vmcs->SetGuestRip(rip + len);
    }

    return handled;
}
```

### 6.3 CPUID Handler (Hypercall Detection)

The hypervisor uses CPUID with a magic value in a specific register to implement hypercalls.

**Intel Pattern (from Sputnik PayLoad Intel):**
```cpp
// Reference: /Refs/Sputnik/PayLoad (Intel)/vmexit_handler.cpp

void vmexit_handler(pcontext_t* context, void* unknown) {
    pcontext_t guest_registers = *context;

    size_t vmexit_reason;
    __vmx_vmread(VMCS_EXIT_REASON, &vmexit_reason);

    if (vmexit_reason == VMX_EXIT_REASON_EXECUTE_CPUID) {
        // Check for magic key in RCX
        if (guest_registers->rcx == VMEXIT_KEY) {
            switch ((vmexit_command_t)guest_registers->rdx) {
            case vmexit_command_t::get_dirbase:
                {
                    u64 guest_dirbase;
                    __vmx_vmread(VMCS_GUEST_CR3, &guest_dirbase);
                    guest_dirbase = cr3{ guest_dirbase }.pml4_pfn << 12;
                    // Return via command structure
                }
                break;
            case vmexit_command_t::read_guest_phys:
                // Handle physical memory read
                break;
            case vmexit_command_t::write_guest_phys:
                // Handle physical memory write
                break;
            // ... more commands
            }

            // Advance RIP
            size_t rip, exec_len;
            __vmx_vmread(VMCS_GUEST_RIP, &rip);
            __vmx_vmread(VMCS_VMEXIT_INSTRUCTION_LENGTH, &exec_len);
            __vmx_vmwrite(VMCS_GUEST_RIP, rip + exec_len);
            return;
        }
    }

    // Call original handler for unhandled exits
    reinterpret_cast<vmexit_handler_t>(
        reinterpret_cast<u64>(&vmexit_handler) -
            sputnik_context.vmexit_handler_rva)(context, unknown);
}
```

**AMD Pattern (from Sputnik PayLoad AMD):**
```cpp
// Reference: /Refs/Sputnik/PayLoad (AMD)/vmexit_handler.cpp

bool HandleCpuid(svm::Vmcb* vmcb, svm::pguest_context context) {
    // Check for magic key
    if (!vmcall::IsVmcall(context->r9))
        return false;

    switch (context->rcx) {
    case VMCALL_GET_CR3:
        {
            COMMAND_DATA cmd = { 0 };
            cmd.cr3.value = vmcb->Cr3();
            vmcb->Rax() = mm::copy_guest_virt(
                __readcr3(), (u64)&cmd,
                vmcb->Cr3(), (u64)context->rdx,
                sizeof(cmd));
        }
        break;

    case VMCALL_READ_PHY:
        {
            auto cmd = GetCommand(vmcb, context->rdx);
            vmcb->Rax() = mm::read_guest_phys(
                vmcb->Cr3(),
                (u64)cmd.read.pTarget,
                (u64)cmd.read.pOutBuf,
                cmd.read.length);
        }
        break;

    case VMCALL_WRITE_PHY:
        // Handle physical write
        break;

    case VMCALL_READ_VIRT:
        // Handle virtual read
        break;

    case VMCALL_WRITE_VIRT:
        // Handle virtual write
        break;
    }

    return true;
}

auto vmexit_handler(..., svm::pguest_context context) -> svm::pgs_base_struct {
    const auto vmcb = svm::get_vmcb();
    bool bHandledExit = false;

    switch (vmcb->ControlArea.ExitCode) {
    case svm::SvmExitCode::VMEXIT_CPUID:
        bHandledExit = HandleCpuid(vmcb, context);
        break;

    case svm::SvmExitCode::VMEXIT_NPF:
        // Handle nested page fault
        break;
    }

    if (!bHandledExit) {
        // Call original Hyper-V handler
        return reinterpret_cast<svm::vcpu_run_t>(
            reinterpret_cast<u64>(&vmexit_handler) -
            svm::sputnik_context.vcpu_run_rva)(unknown, unknown2, context);
    }

    // Advance RIP using NextRip (decode assist)
    vmcb->StateSaveArea.Rip = vmcb->ControlArea.NextRip;

    return reinterpret_cast<svm::pgs_base_struct>(__readgsqword(0));
}
```

### 6.4 MSR Handler

```cpp
void HandleRdmsr(IVmControlStructure* vmcs, GuestRegisters* regs) {
    uint32_t msr_index = (uint32_t)regs->rcx;
    uint64_t value;

    // Handle MSRs that need virtualization
    switch (msr_index) {
        case 0x174:  // IA32_SYSENTER_CS
            value = vmcs->GetGuestSysenterCs();
            break;
        case 0x175:  // IA32_SYSENTER_ESP
            value = vmcs->GetGuestSysenterEsp();
            break;
        case 0x176:  // IA32_SYSENTER_EIP
            value = vmcs->GetGuestSysenterEip();
            break;
        case 0xC0000100:  // IA32_FS_BASE
            value = vmcs->GetGuestFsBase();
            break;
        case 0xC0000101:  // IA32_GS_BASE
            value = vmcs->GetGuestGsBase();
            break;
        default:
            // Pass through to real hardware
            value = __readmsr(msr_index);
            break;
    }

    regs->rax = (uint32_t)(value & 0xFFFFFFFF);
    regs->rdx = (uint32_t)(value >> 32);
}

void HandleWrmsr(IVmControlStructure* vmcs, GuestRegisters* regs) {
    uint32_t msr_index = (uint32_t)regs->rcx;
    uint64_t value = ((uint64_t)regs->rdx << 32) | (regs->rax & 0xFFFFFFFF);

    switch (msr_index) {
        case 0x174:  // IA32_SYSENTER_CS
            vmcs->SetGuestSysenterCs(value);
            break;
        case 0xC0000100:  // IA32_FS_BASE
            vmcs->SetGuestFsBase(value);
            break;
        case 0xC0000101:  // IA32_GS_BASE
            vmcs->SetGuestGsBase(value);
            break;
        case 0xC0000080:  // IA32_EFER
            // Be careful - don't let guest clear SVME/VMXE
            value |= GetRequiredEferBits();
            vmcs->SetGuestEfer(value);
            break;
        default:
            __writemsr(msr_index, value);
            break;
    }
}
```

---

## 7. Practical Code Patterns from References

### 7.1 Guest Virtual Address Translation

When the hypervisor needs to read/write guest memory, it must walk the guest page tables.

**From Sputnik PayLoad Intel (mm.cpp):**
```cpp
// Reference: /Refs/Sputnik/PayLoad (Intel)/mm.cpp

auto mm::translate_guest_virtual(guest_phys_t dirbase, guest_virt_t guest_virt,
                                 map_type_t map_type) -> u64
{
    virt_addr_t virt_addr{ guest_virt };

    // Map guest PML4
    const auto pml4 = reinterpret_cast<pml4e*>(
        map_guest_phys(dirbase, map_type));

    if (!pml4[virt_addr.pml4_index].present)
        return {};

    // Map PDPT
    const auto pdpt = reinterpret_cast<pdpte*>(
        map_guest_phys(pml4[virt_addr.pml4_index].pfn << 12, map_type));

    if (!pdpt[virt_addr.pdpt_index].present)
        return {};

    // Handle 1GB large page
    if (pdpt[virt_addr.pdpt_index].large_page)
        return (pdpt[virt_addr.pdpt_index].pfn << 12) + virt_addr.offset_1gb;

    // Map PD
    const auto pd = reinterpret_cast<pde*>(
        map_guest_phys(pdpt[virt_addr.pdpt_index].pfn << 12, map_type));

    if (!pd[virt_addr.pd_index].present)
        return {};

    // Handle 2MB large page
    if (pd[virt_addr.pd_index].large_page)
        return (pd[virt_addr.pd_index].pfn << 12) + virt_addr.offset_2mb;

    // Map PT
    const auto pt = reinterpret_cast<pte*>(
        map_guest_phys(pd[virt_addr.pd_index].pfn << 12, map_type));

    if (!pt[virt_addr.pt_index].present)
        return {};

    return (pt[virt_addr.pt_index].pfn << 12) + virt_addr.offset_4kb;
}
```

### 7.2 EPT Translation (Guest Physical to Host Physical)

**From Sputnik PayLoad Intel (mm.cpp):**
```cpp
auto mm::translate_guest_physical(guest_phys_t phys_addr, map_type_t map_type) -> u64
{
    ept_pointer eptp;
    phys_addr_t guest_phys{ phys_addr };
    __vmx_vmread(VMCS_CTRL_EPT_POINTER, (size_t*)&eptp);

    // Walk EPT structure
    const auto epml4 = reinterpret_cast<ept_pml4e*>(
        map_page(eptp.page_frame_number << 12, map_type));

    const auto epdpt_large = reinterpret_cast<ept_pdpte_1gb*>(
        map_page(epml4[guest_phys.pml4_index].page_frame_number << 12, map_type));

    // Handle 1GB EPT page
    if (epdpt_large[guest_phys.pdpt_index].large_page)
        return (epdpt_large[guest_phys.pdpt_index].page_frame_number
            * 0x1000 * 0x200 * 0x200) + EPT_LARGE_PDPTE_OFFSET(phys_addr);

    const auto epdpt = reinterpret_cast<ept_pdpte*>(epdpt_large);

    const auto epd_large = reinterpret_cast<epde_2mb*>(
        map_page(epdpt[guest_phys.pdpt_index].page_frame_number << 12, map_type));

    // Handle 2MB EPT page
    if (epd_large[guest_phys.pd_index].large_page)
        return (epd_large[guest_phys.pd_index].page_frame_number
            * 0x1000 * 0x200) + EPT_LARGE_PDE_OFFSET(phys_addr);

    const auto epd = reinterpret_cast<ept_pde*>(epd_large);

    const auto ept = reinterpret_cast<ept_pte*>(
        map_page(epd[guest_phys.pd_index].page_frame_number << 12, map_type));

    return ept[guest_phys.pt_index].page_frame_number << 12;
}
```

### 7.3 Cross-Address-Space Memory Copy

**From Sputnik PayLoad Intel (mm.cpp):**
```cpp
auto mm::copy_guest_virt(guest_phys_t dirbase_src, guest_virt_t virt_src,
    guest_virt_t dirbase_dest, guest_virt_t virt_dest, u64 size) -> vmxroot_error_t
{
    while (size) {
        // Calculate how much we can copy without crossing page boundaries
        auto dest_size = PAGE_4KB - virt_addr_t{ virt_dest }.offset_4kb;
        if (size < dest_size)
            dest_size = size;

        auto src_size = PAGE_4KB - virt_addr_t{ virt_src }.offset_4kb;
        if (size < src_size)
            src_size = size;

        // Map source page
        const auto mapped_src = reinterpret_cast<void*>(
            map_guest_virt(dirbase_src, virt_src, map_type_t::map_src));

        if (!mapped_src)
            return vmxroot_error_t::invalid_guest_virtual;

        // Map destination page
        const auto mapped_dest = reinterpret_cast<void*>(
            map_guest_virt(dirbase_dest, virt_dest, map_type_t::map_dest));

        if (!mapped_dest)
            return vmxroot_error_t::invalid_guest_virtual;

        // Copy the data
        auto current_size = min(dest_size, src_size);
        memcpy(mapped_dest, mapped_src, current_size);

        virt_src += current_size;
        virt_dest += current_size;
        size -= current_size;
    }

    return vmxroot_error_t::error_success;
}
```

---

## 8. Hypercall Protocol

### 8.1 Command Types (from SKLib)

```cpp
// Reference: SKLib/SKLib-v/include/Vmcall.h

enum VMCALL_TYPE {
    VMCALL_TEST = 0x1,
    VMCALL_VMXOFF,
    VMCALL_READ_VIRT,       // Read virtual memory
    VMCALL_WRITE_VIRT,      // Write virtual memory
    VMCALL_READ_PHY,        // Read physical memory
    VMCALL_WRITE_PHY,       // Write physical memory
    VMCALL_DISABLE_EPT,
    VMCALL_SET_COMM_KEY,
    VMCALL_GET_CR3,
    VMCALL_GET_EPT_BASE,
    VMCALL_VIRT_TO_PHY,
    VMCALL_STORAGE_QUERY    // 128-entry u64 storage array
};
```

### 8.2 Command Data Structure

```cpp
typedef struct _COMMAND_DATA {
    union {
        struct {
            uint64_t value;
        } cr3;

        struct {
            void* pTarget;
            void* pOutBuf;
            uint64_t length;
        } read;

        struct {
            void* pTarget;
            void* pInBuf;
            uint64_t length;
        } write;

        struct {
            void* va;
            uint64_t pa;
        } translation;

        struct {
            uint32_t id;
            bool bWrite;
            uint64_t uint64;
        } storage;

        uint64_t pa;
    };
} COMMAND_DATA;
```

### 8.3 User-Mode Hypercall Interface

```cpp
// Invoke hypercall from user-mode or kernel-mode guest
uint64_t OmbraHypercall(uint64_t command, void* data) {
    COMMAND_DATA cmd;
    // Fill command data based on command type

    uint64_t result;

    // Use CPUID with magic value as hypercall mechanism
    // This triggers a VM-exit that our handler intercepts

    // Intel: CPUID with specific leaf
    // AMD: CPUID or VMMCALL

    __asm {
        mov     rcx, OMBRA_HYPERCALL_KEY  // Magic key
        mov     rdx, command              // Command code
        mov     r8,  &cmd                 // Command data pointer
        cpuid                             // Trigger VM-exit
        mov     result, rax               // Result in RAX
    }

    return result;
}
```

---

## 9. Guest Memory Access

### 9.1 Identity Mapping for Host Memory Access

In the payload context (running inside Hyper-V), we need to access physical memory. The identity mapping technique maps physical memory 1:1 into virtual address space.

**AMD Identity Mapping (from Sputnik PayLoad AMD):**
```cpp
// Reference: /Refs/Sputnik/PayLoad (AMD)/mm.cpp

#define PTI_SHIFT  12L
#define PDI_SHIFT  21L
#define PPI_SHIFT  30L
#define PXI_SHIFT  39L

constexpr u64 mapped_host_phys_pml = 360;  // Reserved PML4 index

// Virtual address base for identity-mapped region
char* pIdentity = (char*)((mapped_host_phys_pml << PXI_SHIFT) | 0xffff000000000000);

auto mm::init() -> u64 {
    auto mapping = &identity_map;

    // Setup PML4 entry pointing to our PDPT
    hyperv_pml4[mapped_host_phys_pml].value = 0;
    hyperv_pml4[mapped_host_phys_pml].present = true;
    hyperv_pml4[mapped_host_phys_pml].writeable = true;
    hyperv_pml4[mapped_host_phys_pml].user_supervisor = true;
    hyperv_pml4[mapped_host_phys_pml].pfn = translate((UINT64)&mapping->pdpt[0]) / PAGE_SIZE;

    // Build PDPT entries (512 entries, each covering 1GB)
    for (UINT64 EntryIndex = 0; EntryIndex < 512; EntryIndex++) {
        mapping->pdpt[EntryIndex].Flags = 0;
        mapping->pdpt[EntryIndex].Present = true;
        mapping->pdpt[EntryIndex].Write = true;
        mapping->pdpt[EntryIndex].Supervisor = true;
        mapping->pdpt[EntryIndex].PageFrameNumber =
            translate((UINT64)&mapping->pdt[EntryIndex][0]) / PAGE_SIZE;
    }

    // Build PD entries (512x512 entries, each a 2MB large page)
    for (UINT64 EntryGroupIndex = 0; EntryGroupIndex < 512; EntryGroupIndex++) {
        for (UINT64 EntryIndex = 0; EntryIndex < 512; EntryIndex++) {
            mapping->pdt[EntryGroupIndex][EntryIndex].Flags = 0;
            mapping->pdt[EntryGroupIndex][EntryIndex].Present = true;
            mapping->pdt[EntryGroupIndex][EntryIndex].Write = true;
            mapping->pdt[EntryGroupIndex][EntryIndex].LargePage = true;
            mapping->pdt[EntryGroupIndex][EntryIndex].Supervisor = true;
            // Identity map: virtual 2MB page N maps to physical 2MB page N
            mapping->pdt[EntryGroupIndex][EntryIndex].PageFrameNumber =
                (EntryGroupIndex * 512) + EntryIndex;
        }
    }

    return VMX_ROOT_ERROR::SUCCESS;
}

// Map physical address to virtual (simple addition due to identity mapping)
auto mm::map_page(host_phys_t phys_addr, map_type_t map_type) -> u64 {
    return pIdentityAsU64 + phys_addr;
}
```

---

## 10. Signature-Based Hyper-V Hooking

### 10.1 VM Exit Handler Signatures

The DmaBackdoorHv project maintains signatures for locating Hyper-V's VM exit handler across different Windows versions.

**From DmaBackdoorHv/src/HyperV.c:**
```cpp
// Reference: /Refs/s6_pcie_microblaze/python/payloads/DmaBackdoorHv/src/HyperV.c

/*
    VM exit handler signature pattern (Windows 10 22H2 - build 19045):

    .text:FFFFF8000023E313      mov     [rsp+arg_20], rcx
    .text:FFFFF8000023E318      mov     rcx, [rsp+arg_18]
    .text:FFFFF8000023E31D      mov     rcx, [rcx]
    .text:FFFFF8000023E320      mov     [rcx], rax
    .text:FFFFF8000023E323      mov     [rcx+10h], rdx
    ...
    .text:FFFFF8000023E353      mov     [rcx+78h], r15
    .text:FFFFF8000023E357      mov     rax, [rsp+arg_20]
    .text:FFFFF8000023E35C      mov     [rcx+8], rax
    .text:FFFFF8000023E360      lea     rax, [rcx+70h]
    ...
    .text:FFFFF8000023E3DF      call    sub_FFFFF800002118D0   ; <-- Hook target
*/

// Signature matching code
VOID *HyperVHook(VOID *Image) {
    // Find .text section
    EFI_IMAGE_SECTION_HEADER *pSection = /* find .text */;

    while (Size < pSection->Misc.VirtualSize - PAGE_SIZE) {
        UINT8 *Func = RVATOVA(Image, pSection->VirtualAddress + Size);
        UINTN Version = 0, HookLen = 0;

        // Windows 10 22H2 (build 19045) - call at offset 0xCC
        if (*(Func + 0x00) == 0x48 && *(Func + 0x01) == 0x89 &&
            *(Func + 0x02) == 0x4c && *(Func + 0x03) == 0x24 &&
            *(Func + 0x0d) == 0x48 && *(Func + 0x0e) == 0x89 &&
            *(Func + 0x0f) == 0x01 &&
            *(Func + 0x10) == 0x48 && *(Func + 0x11) == 0x89 &&
            *(Func + 0x12) == 0x51 && *(Func + 0x13) == 0x10 &&
            *(Func + 0x40) == 0x4c && *(Func + 0x41) == 0x89 &&
            *(Func + 0x42) == 0x79 && *(Func + 0x43) == 0x78 &&
            *(Func + 0xcc) == 0xe8)  // CALL instruction
        {
            Func = (UINT8 *)JUMP32_ADDR(Func + 0xcc);
            HookLen = 5;
            Version = 19045;
        }
        // Windows 11 22H2 (build 22621) - call at offset 0x10B
        else if (*(Func + 0x00) == 0x48 && *(Func + 0x01) == 0x89 &&
                 *(Func + 0x02) == 0x4c && *(Func + 0x03) == 0x24 &&
                 *(Func + 0x0d) == 0x48 && *(Func + 0x0e) == 0x89 &&
                 *(Func + 0x0f) == 0x01 &&
                 *(Func + 0x10) == 0x48 && *(Func + 0x11) == 0x89 &&
                 *(Func + 0x12) == 0x51 && *(Func + 0x13) == 0x10 &&
                 *(Func + 0x40) == 0x4c && *(Func + 0x41) == 0x89 &&
                 *(Func + 0x42) == 0x79 && *(Func + 0x43) == 0x78 &&
                 *(Func + 0x10b) == 0xe8)
        {
            Func = (UINT8 *)JUMP32_ADDR(Func + 0x10b);
            HookLen = 5;
            Version = 22621;
        }
        // ... additional versions ...

        Size += 1;
    }
}
```

### 10.2 Hook Installation Pattern

```cpp
// Install hook by patching the CALL target
void InstallVmExitHook(UINT8* target_func, UINT8* hook_func, UINTN hook_len) {
    // Save original bytes
    memcpy(saved_bytes, target_func, hook_len);

    // Write JMP to our hook
    *target_func = 0xE9;  // JMP rel32
    *(UINT32*)(target_func + 1) = (UINT32)(hook_func - target_func - 5);
}

// Hook function structure
void OurVmExitHook(void* arg1, void* arg2, void* arg3, void* arg4) {
    // arg1: Guest state pointer (or pointer to pointer for newer versions)
    // arg2: Pointer to backdoor section (we use this for our data)

    // Read exit reason
    UINT64 ExitReason = 0;
    __vmx_vmread(VM_EXIT_REASON, &ExitReason);

    // Check for our hypercall signature
    if (ExitReason == VM_EXIT_CPUID &&
        Context->R10 == HVBD_VM_EXIT_MAGIC) {
        // Handle our commands
        HandleBackdoorCommand(Context);
        return;
    }

    // Not our command - call original handler
    CallOriginalHandler(arg1, arg2, arg3, arg4);
}
```

### 10.3 Guest State Structure

Different Windows versions have different guest state layouts.

```cpp
// Guest state saved by hvix64.sys VM exit handler
typedef struct _VM_GUEST_STATE {
    UINT64 Rax;   // +0x00
    UINT64 Rcx;   // +0x08
    UINT64 Rdx;   // +0x10
    UINT64 Rbx;   // +0x18
    UINT64 Rsp;   // +0x20 (placeholder in some versions)
    UINT64 Rbp;   // +0x28
    UINT64 Rsi;   // +0x30
    UINT64 Rdi;   // +0x38
    UINT64 R8;    // +0x40
    UINT64 R9;    // +0x48
    UINT64 R10;   // +0x50
    UINT64 R11;   // +0x58
    UINT64 R12;   // +0x60
    UINT64 R13;   // +0x68
    UINT64 R14;   // +0x70
    UINT64 R15;   // +0x78
} VM_GUEST_STATE;

// Version detection
if (BackdoorData->Version >= 17763) {
    // 1-st argument is address of guest state POINTER (dereference once)
    Context = *(VM_GUEST_STATE**)arg1;
} else {
    // 1-st argument is guest state pointer directly
    Context = (VM_GUEST_STATE*)arg1;
}
```

---

## Summary

This document covered the fundamental concepts needed to implement a hypervisor:

1. **Intel VT-x**: VMXON, VMCS setup, control field adjustment, VM entry/exit cycle
2. **AMD SVM**: EFER.SVME, VMCB structure, VMRUN/VMEXIT flow, VMSAVE/VMLOAD
3. **Unified Architecture**: Interface abstractions, vendor dispatch, exit reason mapping
4. **Memory Management**: EPT/NPT structures, identity mapping, violation handling
5. **VM-Exit Handling**: Exit reason dispatch, CPUID/MSR/CR handlers
6. **Practical Patterns**: Guest memory translation, cross-address-space copy
7. **Hypercall Protocol**: Command types, data structures, invocation
8. **Hyper-V Hooking**: Signature-based detection, hook installation

### Key Reference Files

| Purpose | Intel | AMD |
|---------|-------|-----|
| VM Exit Handler | `/Refs/Sputnik/PayLoad (Intel)/vmexit_handler.cpp` | `/Refs/Sputnik/PayLoad (AMD)/vmexit_handler.cpp` |
| Memory Management | `/Refs/Sputnik/PayLoad (Intel)/mm.cpp` | `/Refs/Sputnik/PayLoad (AMD)/mm.cpp` |
| Hyper-V Signatures | `/Refs/s6_pcie_microblaze/python/payloads/DmaBackdoorHv/src/HyperV.c` | - |
| Training Docs | `/OmbraHypervisor/Docs/Training/A2_INTEL_VIRT_KNOWLEDGE.md` | `/OmbraHypervisor/Docs/Training/A3_AMD_VIRT_KNOWLEDGE.md` |

### Critical Implementation Checklist

**Intel VT-x:**
- [ ] Check CPUID.1:ECX[5] for VMX support
- [ ] Configure IA32_FEATURE_CONTROL MSR
- [ ] Apply CR0/CR4 fixed bits before VMXON
- [ ] Write revision ID to VMXON/VMCS regions
- [ ] Adjust control fields using capability MSRs
- [ ] Set VMCS link pointer to 0xFFFFFFFFFFFFFFFF
- [ ] Ensure Host RSP/RIP point to valid VMM stack/handler
- [ ] Use VMLAUNCH first, then VMRESUME

**AMD SVM:**
- [ ] Check CPUID.8000_0001:ECX[2] for SVM support
- [ ] Check VM_CR.SVMDIS is clear
- [ ] Set EFER.SVME
- [ ] Configure VM_HSAVE_PA
- [ ] Set ASID to non-zero value
- [ ] Intercept VMRUN (mandatory)
- [ ] Use VMSAVE/VMLOAD for additional state
- [ ] Set User bit on all NPT entries
- [ ] Save all GPRs (only RAX goes to VMCB)
