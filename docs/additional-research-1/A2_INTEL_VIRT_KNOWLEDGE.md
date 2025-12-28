# Intel VT-x Implementation Knowledge Base

This document provides comprehensive knowledge for implementing Intel VT-x virtualization in the OmbraHypervisor project. All information is derived from analysis of HyperPlatform, NoirVisor, SimpleVisor, gbhv, and ksm reference implementations.

## Table of Contents

1. [VMX Operation Prerequisites](#vmx-operation-prerequisites)
2. [VMXON Region Setup](#vmxon-region-setup)
3. [VMCS Field Reference](#vmcs-field-reference)
4. [VMCS Setup Process](#vmcs-setup-process)
5. [VM-Exit Handling](#vm-exit-handling)
6. [Assembly Implementation](#assembly-implementation)
7. [Common Pitfalls](#common-pitfalls)
8. [Recommended Patterns for OmbraHypervisor](#recommended-patterns)

---

## 1. VMX Operation Prerequisites

### 1.1 CPU Feature Detection

Before enabling VMX, verify hardware support via CPUID.

```c
// Reference: HyperPlatform/vm.cpp:172-213 (VmpIsVmxAvailable)

// Step 1: Check CPUID.1:ECX.VMX[bit 5]
int cpu_info[4] = {};
__cpuid(cpu_info, 1);
bool vmx_supported = (cpu_info[2] >> 5) & 1;  // ECX bit 5

// Step 2: Check IA32_VMX_BASIC for memory type
// Write-back (6) is required by modern processors
uint64_t vmx_basic = __readmsr(0x480);  // IA32_VMX_BASIC
uint8_t memory_type = (vmx_basic >> 50) & 0xF;
bool wb_supported = (memory_type == 6);  // Write-back

// Step 3: Check IA32_FEATURE_CONTROL MSR
uint64_t feature_control = __readmsr(0x3A);  // IA32_FEATURE_CONTROL
bool lock_bit = feature_control & 1;
bool vmxon_enabled = (feature_control >> 2) & 1;

// If lock bit is clear, we can set it
if (!lock_bit) {
    feature_control |= 1;         // Lock bit
    feature_control |= (1 << 2);  // Enable VMXON outside SMX
    __writemsr(0x3A, feature_control);
}
```

### 1.2 MSR Definitions

| MSR Name | Index | Purpose |
|----------|-------|---------|
| IA32_VMX_BASIC | 0x480 | VMX capability info, revision ID, VMCS size |
| IA32_VMX_PINBASED_CTLS | 0x481 | Pin-based controls allowed 0/1 settings |
| IA32_VMX_PROCBASED_CTLS | 0x482 | Primary processor-based controls |
| IA32_VMX_EXIT_CTLS | 0x483 | VM-exit controls |
| IA32_VMX_ENTRY_CTLS | 0x484 | VM-entry controls |
| IA32_VMX_MISC | 0x485 | Miscellaneous VMX info |
| IA32_VMX_CR0_FIXED0 | 0x486 | CR0 bits that must be 1 |
| IA32_VMX_CR0_FIXED1 | 0x487 | CR0 bits that must be 0 |
| IA32_VMX_CR4_FIXED0 | 0x488 | CR4 bits that must be 1 |
| IA32_VMX_CR4_FIXED1 | 0x489 | CR4 bits that must be 0 |
| IA32_VMX_PROCBASED_CTLS2 | 0x48B | Secondary processor-based controls |
| IA32_VMX_TRUE_PINBASED_CTLS | 0x48D | True pin-based controls (if supported) |
| IA32_VMX_TRUE_PROCBASED_CTLS | 0x48E | True processor-based controls |
| IA32_VMX_TRUE_EXIT_CTLS | 0x48F | True exit controls |
| IA32_VMX_TRUE_ENTRY_CTLS | 0x490 | True entry controls |
| IA32_FEATURE_CONTROL | 0x3A | VMX enable/lock control |

### 1.3 CR0/CR4 Fixed Bits

```c
// Reference: HyperPlatform/vm.cpp:512-564 (VmpEnterVmxMode)

// CR0 adjustment: Apply fixed bits before VMXON
uint64_t cr0_fixed0 = __readmsr(0x486);  // IA32_VMX_CR0_FIXED0
uint64_t cr0_fixed1 = __readmsr(0x487);  // IA32_VMX_CR0_FIXED1
uint64_t cr0 = __readcr0();
cr0 &= cr0_fixed1;  // Clear bits that must be 0
cr0 |= cr0_fixed0;  // Set bits that must be 1
__writecr0(cr0);

// CR4 adjustment: Must set VMXE bit and apply fixed bits
uint64_t cr4_fixed0 = __readmsr(0x488);  // IA32_VMX_CR4_FIXED0
uint64_t cr4_fixed1 = __readmsr(0x489);  // IA32_VMX_CR4_FIXED1
uint64_t cr4 = __readcr4();
cr4 &= cr4_fixed1;  // Clear bits that must be 0
cr4 |= cr4_fixed0;  // Set bits that must be 1 (includes VMXE)
__writecr4(cr4);
```

---

## 2. VMXON Region Setup

### 2.1 Memory Allocation Requirements

| Requirement | Value | Notes |
|-------------|-------|-------|
| Size | 4096 bytes (1 page) | Read from IA32_VMX_BASIC[44:32] |
| Alignment | 4KB page-aligned | Physical address must be page-aligned |
| Memory Type | Write-back | Required by modern processors |
| Initialization | Zero-filled | Clear before writing revision ID |

### 2.2 VMXON Region Structure

```c
// Reference: HyperPlatform/vm.cpp:393-410

typedef struct _VmControlStructure {
    uint32_t revision_identifier;  // Must match IA32_VMX_BASIC[30:0]
    uint32_t vmx_abort_indicator;  // Set by processor on VMX abort
    uint8_t  data[4088];           // Implementation-specific
} VmControlStructure;

// Allocation
VmControlStructure* vmxon_region = (VmControlStructure*)
    AllocateContiguousMemory(4096);  // 4KB, page-aligned
memset(vmxon_region, 0, 4096);

// Write revision ID from IA32_VMX_BASIC
uint64_t vmx_basic = __readmsr(0x480);
vmxon_region->revision_identifier = (uint32_t)(vmx_basic & 0x7FFFFFFF);
```

### 2.3 VMXON Execution

```c
// Reference: HyperPlatform/vm.cpp:555-564

uint64_t vmxon_phys = GetPhysicalAddress(vmxon_region);

// Execute VMXON instruction
// Returns: 0=success, 1=fail with status, 2=fail without status
int result = __vmx_on(&vmxon_phys);
if (result != 0) {
    // VMXON failed - check IA32_FEATURE_CONTROL and CR4.VMXE
    return false;
}

// After successful VMXON, invalidate TLBs
// Reference: HyperPlatform/vm.cpp:562-563
InveptGlobal();      // Invalidate all EPT-derived translations
InvvpidAllContext(); // Invalidate all VPID-tagged translations
```

---

## 3. VMCS Field Reference

### 3.1 VMCS Field Encoding Format

```
Bits [31:14] - Reserved (must be 0)
Bits [13:12] - Width: 00=16-bit, 01=64-bit, 10=32-bit, 11=natural-width
Bit  [11]    - Reserved
Bits [10:1]  - Index
Bit  [0]     - Access type: 0=full, 1=high 32 bits of 64-bit field
```

### 3.2 Complete VMCS Field Encodings

Reference: NoirVisor/src/vt_core/vt_vmcs.h:20-208

#### 16-Bit Control Fields
| Field | Encoding | Purpose |
|-------|----------|---------|
| Virtual Processor ID | 0x0000 | VPID for TLB tagging |
| Posted Interrupt Notification Vector | 0x0002 | Posted interrupt delivery |
| EPT Pointer Index | 0x0004 | EPTP list index |

#### 16-Bit Guest-State Fields
| Field | Encoding | Purpose |
|-------|----------|---------|
| Guest ES Selector | 0x0800 | ES segment selector |
| Guest CS Selector | 0x0802 | CS segment selector |
| Guest SS Selector | 0x0804 | SS segment selector |
| Guest DS Selector | 0x0806 | DS segment selector |
| Guest FS Selector | 0x0808 | FS segment selector |
| Guest GS Selector | 0x080A | GS segment selector |
| Guest LDTR Selector | 0x080C | LDTR segment selector |
| Guest TR Selector | 0x080E | TR segment selector |
| Guest Interrupt Status | 0x0810 | APIC virtual interrupt status |
| PML Index | 0x0812 | Page modification log index |

#### 16-Bit Host-State Fields
| Field | Encoding | Purpose |
|-------|----------|---------|
| Host ES Selector | 0x0C00 | ES (must have RPL=0, TI=0) |
| Host CS Selector | 0x0C02 | CS (must have RPL=0, TI=0) |
| Host SS Selector | 0x0C04 | SS (must have RPL=0, TI=0) |
| Host DS Selector | 0x0C06 | DS (must have RPL=0, TI=0) |
| Host FS Selector | 0x0C08 | FS (must have RPL=0, TI=0) |
| Host GS Selector | 0x0C0A | GS (must have RPL=0, TI=0) |
| Host TR Selector | 0x0C0C | TR (must have RPL=0, TI=0) |

#### 64-Bit Control Fields
| Field | Encoding | Purpose |
|-------|----------|---------|
| I/O Bitmap A Address | 0x2000 | Physical address of I/O bitmap A |
| I/O Bitmap B Address | 0x2002 | Physical address of I/O bitmap B |
| MSR Bitmap Address | 0x2004 | Physical address of MSR bitmap |
| VM-Exit MSR Store Address | 0x2006 | MSR store list for VM-exit |
| VM-Exit MSR Load Address | 0x2008 | MSR load list for VM-exit |
| VM-Entry MSR Load Address | 0x200A | MSR load list for VM-entry |
| Executive VMCS Pointer | 0x200C | Shadow VMCS pointer |
| PML Address | 0x200E | Page modification log address |
| TSC Offset | 0x2010 | TSC offset for guest |
| Virtual APIC Address | 0x2012 | Virtual APIC page address |
| APIC Access Address | 0x2014 | APIC-access page address |
| Posted Interrupt Descriptor | 0x2016 | Posted interrupt descriptor address |
| VM Function Controls | 0x2018 | VM functions enabled |
| EPT Pointer | 0x201A | Extended page table pointer |
| EOI Exit Bitmap 0-3 | 0x201C-0x2022 | EOI-exit bitmaps |
| EPTP List Address | 0x2024 | EPTP switching list |
| VMREAD Bitmap Address | 0x2026 | VMREAD bitmap (shadow VMCS) |
| VMWRITE Bitmap Address | 0x2028 | VMWRITE bitmap (shadow VMCS) |
| XSS Exiting Bitmap | 0x202C | XSS exit control |
| ENCLS Exiting Bitmap | 0x202E | ENCLS exit control |
| TSC Multiplier | 0x2032 | TSC scaling multiplier |

#### 64-Bit Read-Only Field
| Field | Encoding | Purpose |
|-------|----------|---------|
| Guest Physical Address | 0x2400 | GPA causing EPT violation |

#### 64-Bit Guest-State Fields
| Field | Encoding | Purpose |
|-------|----------|---------|
| VMCS Link Pointer | 0x2800 | Shadow VMCS link (0xFFFFFFFFFFFFFFFF if unused) |
| Guest IA32_DEBUGCTL | 0x2802 | Debug control MSR |
| Guest IA32_PAT | 0x2804 | Page attribute table MSR |
| Guest IA32_EFER | 0x2806 | Extended feature enable MSR |
| Guest IA32_PERF_GLOBAL_CTRL | 0x2808 | Performance monitoring control |
| Guest PDPTE0-3 | 0x280A-0x2810 | PAE page directory pointers |

#### 64-Bit Host-State Fields
| Field | Encoding | Purpose |
|-------|----------|---------|
| Host IA32_PAT | 0x2C00 | Host PAT MSR |
| Host IA32_EFER | 0x2C02 | Host EFER MSR |
| Host IA32_PERF_GLOBAL_CTRL | 0x2C04 | Host performance control |

#### 32-Bit Control Fields
| Field | Encoding | Purpose |
|-------|----------|---------|
| Pin-Based Controls | 0x4000 | Pin-based VM-execution controls |
| Primary Processor-Based Controls | 0x4002 | Primary proc-based controls |
| Exception Bitmap | 0x4004 | Exception interception bitmap |
| Page-Fault Error Code Mask | 0x4006 | PF error code mask |
| Page-Fault Error Code Match | 0x4008 | PF error code match |
| CR3 Target Count | 0x400A | Number of CR3 target values |
| VM-Exit Controls | 0x400C | VM-exit behavior controls |
| VM-Exit MSR Store Count | 0x400E | Number of MSRs to store on exit |
| VM-Exit MSR Load Count | 0x4010 | Number of MSRs to load on exit |
| VM-Entry Controls | 0x4012 | VM-entry behavior controls |
| VM-Entry MSR Load Count | 0x4014 | Number of MSRs to load on entry |
| VM-Entry Interruption Info | 0x4016 | Event injection info |
| VM-Entry Exception Error Code | 0x4018 | Exception error code for injection |
| VM-Entry Instruction Length | 0x401A | Instruction length for injection |
| TPR Threshold | 0x401C | Virtual TPR threshold |
| Secondary Processor-Based Controls | 0x401E | Secondary proc-based controls |
| PLE Gap | 0x4020 | Pause-loop exiting gap |
| PLE Window | 0x4022 | Pause-loop exiting window |

#### 32-Bit Read-Only Fields
| Field | Encoding | Purpose |
|-------|----------|---------|
| VM Instruction Error | 0x4400 | VMX instruction error code |
| VM-Exit Reason | 0x4402 | Exit reason (basic + flags) |
| VM-Exit Interruption Info | 0x4404 | Interruption info on exit |
| VM-Exit Interruption Error Code | 0x4406 | Exception error code |
| IDT-Vectoring Info | 0x4408 | IDT vectoring info |
| IDT-Vectoring Error Code | 0x440A | IDT vectoring error code |
| VM-Exit Instruction Length | 0x440C | Length of exiting instruction |
| VM-Exit Instruction Info | 0x440E | Instruction-specific info |

#### 32-Bit Guest-State Fields
| Field | Encoding | Purpose |
|-------|----------|---------|
| Guest ES Limit | 0x4800 | ES segment limit |
| Guest CS Limit | 0x4802 | CS segment limit |
| Guest SS Limit | 0x4804 | SS segment limit |
| Guest DS Limit | 0x4806 | DS segment limit |
| Guest FS Limit | 0x4808 | FS segment limit |
| Guest GS Limit | 0x480A | GS segment limit |
| Guest LDTR Limit | 0x480C | LDTR limit |
| Guest TR Limit | 0x480E | TR limit |
| Guest GDTR Limit | 0x4810 | GDTR limit |
| Guest IDTR Limit | 0x4812 | IDTR limit |
| Guest ES Access Rights | 0x4814 | ES attributes |
| Guest CS Access Rights | 0x4816 | CS attributes |
| Guest SS Access Rights | 0x4818 | SS attributes |
| Guest DS Access Rights | 0x481A | DS attributes |
| Guest FS Access Rights | 0x481C | FS attributes |
| Guest GS Access Rights | 0x481E | GS attributes |
| Guest LDTR Access Rights | 0x4820 | LDTR attributes |
| Guest TR Access Rights | 0x4822 | TR attributes |
| Guest Interruptibility State | 0x4824 | Interrupt blocking state |
| Guest Activity State | 0x4826 | Activity state (active, HLT, etc.) |
| Guest SMBASE | 0x4828 | SMM base address |
| Guest IA32_SYSENTER_CS | 0x482A | SYSENTER CS MSR |
| VMX Preemption Timer | 0x482E | Preemption timer value |

#### 32-Bit Host-State Field
| Field | Encoding | Purpose |
|-------|----------|---------|
| Host IA32_SYSENTER_CS | 0x4C00 | Host SYSENTER CS |

#### Natural-Width Control Fields
| Field | Encoding | Purpose |
|-------|----------|---------|
| CR0 Guest/Host Mask | 0x6000 | CR0 bits owned by host |
| CR4 Guest/Host Mask | 0x6002 | CR4 bits owned by host |
| CR0 Read Shadow | 0x6004 | CR0 value shown to guest |
| CR4 Read Shadow | 0x6006 | CR4 value shown to guest |
| CR3 Target Value 0-3 | 0x6008-0x600E | CR3 target values |

#### Natural-Width Read-Only Fields
| Field | Encoding | Purpose |
|-------|----------|---------|
| Exit Qualification | 0x6400 | Exit-specific info |
| I/O RCX | 0x6402 | RCX for I/O string operations |
| I/O RSI | 0x6404 | RSI for I/O string operations |
| I/O RDI | 0x6406 | RDI for I/O string operations |
| I/O RIP | 0x6408 | RIP for I/O string operations |
| Guest Linear Address | 0x640A | Linear address for certain exits |

#### Natural-Width Guest-State Fields
| Field | Encoding | Purpose |
|-------|----------|---------|
| Guest CR0 | 0x6800 | Guest CR0 value |
| Guest CR3 | 0x6802 | Guest CR3 value |
| Guest CR4 | 0x6804 | Guest CR4 value |
| Guest ES Base | 0x6806 | ES base address |
| Guest CS Base | 0x6808 | CS base address |
| Guest SS Base | 0x680A | SS base address |
| Guest DS Base | 0x680C | DS base address |
| Guest FS Base | 0x680E | FS base address |
| Guest GS Base | 0x6810 | GS base address |
| Guest LDTR Base | 0x6812 | LDTR base address |
| Guest TR Base | 0x6814 | TR base address |
| Guest GDTR Base | 0x6816 | GDTR base address |
| Guest IDTR Base | 0x6818 | IDTR base address |
| Guest DR7 | 0x681A | Debug register 7 |
| Guest RSP | 0x681C | Guest stack pointer |
| Guest RIP | 0x681E | Guest instruction pointer |
| Guest RFLAGS | 0x6820 | Guest flags register |
| Guest Pending Debug Exceptions | 0x6822 | Pending #DB |
| Guest IA32_SYSENTER_ESP | 0x6824 | SYSENTER ESP MSR |
| Guest IA32_SYSENTER_EIP | 0x6826 | SYSENTER EIP MSR |
| Guest S_CET | 0x6828 | Shadow stack CET |
| Guest SSP | 0x682A | Shadow stack pointer |

#### Natural-Width Host-State Fields
| Field | Encoding | Purpose |
|-------|----------|---------|
| Host CR0 | 0x6C00 | Host CR0 |
| Host CR3 | 0x6C02 | Host CR3 |
| Host CR4 | 0x6C04 | Host CR4 |
| Host FS Base | 0x6C06 | Host FS base |
| Host GS Base | 0x6C08 | Host GS base |
| Host TR Base | 0x6C0A | Host TR base |
| Host GDTR Base | 0x6C0C | Host GDTR base |
| Host IDTR Base | 0x6C0E | Host IDTR base |
| Host IA32_SYSENTER_ESP | 0x6C10 | Host SYSENTER ESP |
| Host IA32_SYSENTER_EIP | 0x6C12 | Host SYSENTER EIP |
| Host RSP | 0x6C14 | Host stack pointer |
| Host RIP | 0x6C16 | VM-exit handler entry point |

---

## 4. VMCS Setup Process

### 4.1 VMCS Initialization Sequence

```c
// Reference: HyperPlatform/vm.cpp:567-587 (VmpInitializeVmcs)

// 1. Allocate VMCS region (same requirements as VMXON region)
VmControlStructure* vmcs_region = AllocateContiguousMemory(4096);
memset(vmcs_region, 0, 4096);

// 2. Write revision ID
uint64_t vmx_basic = __readmsr(0x480);
vmcs_region->revision_identifier = (uint32_t)(vmx_basic & 0x7FFFFFFF);

// 3. Execute VMCLEAR (initializes VMCS, sets launch state to "clear")
uint64_t vmcs_phys = GetPhysicalAddress(vmcs_region);
if (__vmx_vmclear(&vmcs_phys) != 0) {
    return false;
}

// 4. Execute VMPTRLD (loads VMCS as current)
if (__vmx_vmptrld(&vmcs_phys) != 0) {
    return false;
}

// VMCS is now active and ready for VMWRITE operations
```

### 4.2 Control Field Adjustment

VMX controls have mandatory 0 and 1 bits defined by capability MSRs.

```c
// Reference: HyperPlatform/vm.cpp:918-930 (VmpAdjustControlValue)

uint32_t AdjustControlValue(uint32_t msr_index, uint32_t requested) {
    uint64_t msr_value = __readmsr(msr_index);
    uint32_t allowed_0 = (uint32_t)(msr_value & 0xFFFFFFFF);
    uint32_t allowed_1 = (uint32_t)(msr_value >> 32);

    uint32_t result = requested;
    result &= allowed_1;  // Clear bits that must be 0
    result |= allowed_0;  // Set bits that must be 1
    return result;
}

// Usage - check IA32_VMX_BASIC for "true" MSR support
uint64_t vmx_basic = __readmsr(0x480);
bool use_true_msrs = (vmx_basic >> 55) & 1;

// Pin-based controls
uint32_t pin_ctl = AdjustControlValue(
    use_true_msrs ? 0x48D : 0x481,  // IA32_VMX_TRUE_PINBASED_CTLS or PINBASED_CTLS
    requested_pin_controls);

// Primary processor-based controls
uint32_t proc_ctl = AdjustControlValue(
    use_true_msrs ? 0x48E : 0x482,
    requested_proc_controls);

// Secondary processor-based controls
uint32_t proc_ctl2 = AdjustControlValue(0x48B, requested_proc2_controls);

// VM-exit controls
uint32_t exit_ctl = AdjustControlValue(
    use_true_msrs ? 0x48F : 0x483,
    requested_exit_controls);

// VM-entry controls
uint32_t entry_ctl = AdjustControlValue(
    use_true_msrs ? 0x490 : 0x484,
    requested_entry_controls);
```

### 4.3 Complete VMCS Setup Pattern

```c
// Reference: HyperPlatform/vm.cpp:590-820 (VmpSetupVmcs)

void SetupVmcs(
    ProcessorData* proc_data,
    uint64_t guest_rsp,
    uint64_t guest_rip,
    uint64_t vmm_stack_pointer)
{
    // Read current GDT and IDT
    GDTR gdtr;
    IDTR idtr;
    _sgdt(&gdtr);
    __sidt(&idtr);

    // === VM-Execution Control Fields ===

    // Pin-based controls (typically minimal)
    uint32_t pin_ctl_requested = 0;
    // Bit 0: External-interrupt exiting
    // Bit 3: NMI exiting
    // Bit 5: Virtual NMIs
    // Bit 6: VMX-preemption timer
    // Bit 7: Process posted interrupts

    // Primary processor-based controls
    uint32_t proc_ctl_requested = 0;
    proc_ctl_requested |= (1 << 15);  // CR3-load exiting
    proc_ctl_requested |= (1 << 23);  // MOV-DR exiting
    proc_ctl_requested |= (1 << 25);  // Use I/O bitmaps
    proc_ctl_requested |= (1 << 28);  // Use MSR bitmaps
    proc_ctl_requested |= (1 << 31);  // Activate secondary controls

    // Secondary processor-based controls
    uint32_t proc_ctl2_requested = 0;
    proc_ctl2_requested |= (1 << 1);   // Enable EPT
    proc_ctl2_requested |= (1 << 2);   // Descriptor-table exiting
    proc_ctl2_requested |= (1 << 3);   // Enable RDTSCP
    proc_ctl2_requested |= (1 << 5);   // Enable VPID
    proc_ctl2_requested |= (1 << 12);  // Enable INVPCID
    proc_ctl2_requested |= (1 << 20);  // Enable XSAVES/XRSTORS

    // VM-exit controls
    uint32_t exit_ctl_requested = 0;
    exit_ctl_requested |= (1 << 9);   // Host address-space size (64-bit host)
    exit_ctl_requested |= (1 << 15);  // Acknowledge interrupt on exit

    // VM-entry controls
    uint32_t entry_ctl_requested = 0;
    entry_ctl_requested |= (1 << 2);  // Load debug controls
    entry_ctl_requested |= (1 << 9);  // IA-32e mode guest (64-bit guest)

    // Apply adjustments
    uint32_t pin_ctl = AdjustControlValue(MSR_PIN, pin_ctl_requested);
    uint32_t proc_ctl = AdjustControlValue(MSR_PROC, proc_ctl_requested);
    uint32_t proc_ctl2 = AdjustControlValue(0x48B, proc_ctl2_requested);
    uint32_t exit_ctl = AdjustControlValue(MSR_EXIT, exit_ctl_requested);
    uint32_t entry_ctl = AdjustControlValue(MSR_ENTRY, entry_ctl_requested);

    // Write control fields
    __vmx_vmwrite(0x4000, pin_ctl);    // Pin-based controls
    __vmx_vmwrite(0x4002, proc_ctl);   // Primary proc-based controls
    __vmx_vmwrite(0x401E, proc_ctl2);  // Secondary proc-based controls
    __vmx_vmwrite(0x400C, exit_ctl);   // VM-exit controls
    __vmx_vmwrite(0x4012, entry_ctl);  // VM-entry controls

    // Exception bitmap (intercept #BP for example)
    __vmx_vmwrite(0x4004, 0);  // No exception interception

    // === 64-Bit Control Fields ===

    __vmx_vmwrite64(0x2000, io_bitmap_a_phys);  // I/O bitmap A
    __vmx_vmwrite64(0x2002, io_bitmap_b_phys);  // I/O bitmap B
    __vmx_vmwrite64(0x2004, msr_bitmap_phys);   // MSR bitmap
    __vmx_vmwrite64(0x201A, ept_pointer);       // EPT pointer

    // === 16-Bit Control Fields ===

    uint32_t vpid = GetProcessorNumber() + 1;  // VPID must be non-zero
    __vmx_vmwrite(0x0000, vpid);

    // === Guest State ===

    // 16-bit guest selectors
    __vmx_vmwrite(0x0800, __read_es());   // ES selector
    __vmx_vmwrite(0x0802, __read_cs());   // CS selector
    __vmx_vmwrite(0x0804, __read_ss());   // SS selector
    __vmx_vmwrite(0x0806, __read_ds());   // DS selector
    __vmx_vmwrite(0x0808, __read_fs());   // FS selector
    __vmx_vmwrite(0x080A, __read_gs());   // GS selector
    __vmx_vmwrite(0x080C, __sldt());      // LDTR selector
    __vmx_vmwrite(0x080E, __str());       // TR selector

    // 32-bit guest segment limits
    __vmx_vmwrite(0x4800, __segmentlimit(__read_es()));
    __vmx_vmwrite(0x4802, __segmentlimit(__read_cs()));
    __vmx_vmwrite(0x4804, __segmentlimit(__read_ss()));
    __vmx_vmwrite(0x4806, __segmentlimit(__read_ds()));
    __vmx_vmwrite(0x4808, __segmentlimit(__read_fs()));
    __vmx_vmwrite(0x480A, __segmentlimit(__read_gs()));
    __vmx_vmwrite(0x480C, __segmentlimit(__sldt()));
    __vmx_vmwrite(0x480E, __segmentlimit(__str()));
    __vmx_vmwrite(0x4810, gdtr.limit);   // GDTR limit
    __vmx_vmwrite(0x4812, idtr.limit);   // IDTR limit

    // 32-bit guest segment access rights
    __vmx_vmwrite(0x4814, GetSegmentAccessRight(__read_es()));
    __vmx_vmwrite(0x4816, GetSegmentAccessRight(__read_cs()));
    __vmx_vmwrite(0x4818, GetSegmentAccessRight(__read_ss()));
    __vmx_vmwrite(0x481A, GetSegmentAccessRight(__read_ds()));
    __vmx_vmwrite(0x481C, GetSegmentAccessRight(__read_fs()));
    __vmx_vmwrite(0x481E, GetSegmentAccessRight(__read_gs()));
    __vmx_vmwrite(0x4820, GetSegmentAccessRight(__sldt()));
    __vmx_vmwrite(0x4822, GetSegmentAccessRight(__str()));

    // Guest SYSENTER CS
    __vmx_vmwrite(0x482A, __readmsr(0x174));

    // Natural-width guest segment bases (64-bit mode)
    __vmx_vmwrite(0x6806, 0);  // ES base = 0 in 64-bit
    __vmx_vmwrite(0x6808, 0);  // CS base = 0 in 64-bit
    __vmx_vmwrite(0x680A, 0);  // SS base = 0 in 64-bit
    __vmx_vmwrite(0x680C, 0);  // DS base = 0 in 64-bit
    __vmx_vmwrite(0x680E, __readmsr(0xC0000100));  // FS base
    __vmx_vmwrite(0x6810, __readmsr(0xC0000101));  // GS base
    __vmx_vmwrite(0x6812, GetSegmentBase(gdtr.base, __sldt()));  // LDTR base
    __vmx_vmwrite(0x6814, GetSegmentBase(gdtr.base, __str()));   // TR base
    __vmx_vmwrite(0x6816, gdtr.base);  // GDTR base
    __vmx_vmwrite(0x6818, idtr.base);  // IDTR base

    // Guest control registers
    __vmx_vmwrite(0x6800, __readcr0());
    __vmx_vmwrite(0x6802, __readcr3());
    __vmx_vmwrite(0x6804, __readcr4());

    // Guest DR7
    __vmx_vmwrite(0x681A, __readdr(7));

    // Guest RSP, RIP, RFLAGS
    __vmx_vmwrite(0x681C, guest_rsp);
    __vmx_vmwrite(0x681E, guest_rip);
    __vmx_vmwrite(0x6820, __readeflags());

    // Guest SYSENTER ESP/EIP
    __vmx_vmwrite(0x6824, __readmsr(0x175));
    __vmx_vmwrite(0x6826, __readmsr(0x176));

    // 64-bit guest state
    __vmx_vmwrite64(0x2800, 0xFFFFFFFFFFFFFFFF);  // VMCS link pointer
    __vmx_vmwrite64(0x2802, __readmsr(0x1D9));    // IA32_DEBUGCTL

    // Guest activity/interruptibility state
    __vmx_vmwrite(0x4824, 0);  // Interruptibility = none
    __vmx_vmwrite(0x4826, 0);  // Activity = active

    // === Host State ===

    // 16-bit host selectors (must have RPL=0, TI=0)
    __vmx_vmwrite(0x0C00, __read_es() & 0xF8);
    __vmx_vmwrite(0x0C02, __read_cs() & 0xF8);
    __vmx_vmwrite(0x0C04, __read_ss() & 0xF8);
    __vmx_vmwrite(0x0C06, __read_ds() & 0xF8);
    __vmx_vmwrite(0x0C08, __read_fs() & 0xF8);
    __vmx_vmwrite(0x0C0A, __read_gs() & 0xF8);
    __vmx_vmwrite(0x0C0C, __str() & 0xF8);

    // Host SYSENTER CS
    __vmx_vmwrite(0x4C00, __readmsr(0x174));

    // Host control registers
    __vmx_vmwrite(0x6C00, __readcr0());
    __vmx_vmwrite(0x6C02, __readcr3());
    __vmx_vmwrite(0x6C04, __readcr4());

    // Host segment bases
    __vmx_vmwrite(0x6C06, __readmsr(0xC0000100));  // FS base
    __vmx_vmwrite(0x6C08, __readmsr(0xC0000101));  // GS base
    __vmx_vmwrite(0x6C0A, GetSegmentBase(gdtr.base, __str()));  // TR base
    __vmx_vmwrite(0x6C0C, gdtr.base);  // GDTR base
    __vmx_vmwrite(0x6C0E, idtr.base);  // IDTR base

    // Host SYSENTER ESP/EIP
    __vmx_vmwrite(0x6C10, __readmsr(0x175));
    __vmx_vmwrite(0x6C12, __readmsr(0x176));

    // Host RSP and RIP (critical!)
    __vmx_vmwrite(0x6C14, vmm_stack_pointer);
    __vmx_vmwrite(0x6C16, (uint64_t)VmExitHandler);  // VM-exit entry point

    // === CR0/CR4 Masks and Shadows ===

    __vmx_vmwrite(0x6000, 0);  // CR0 guest/host mask
    __vmx_vmwrite(0x6002, 0);  // CR4 guest/host mask
    __vmx_vmwrite(0x6004, __readcr0());  // CR0 read shadow
    __vmx_vmwrite(0x6006, __readcr4());  // CR4 read shadow
}
```

### 4.4 Segment Access Rights Format

```c
// Reference: HyperPlatform/vm.cpp:843-860 (VmpGetSegmentAccessRight)

typedef union _SegmentAccessRight {
    struct {
        uint32_t type : 4;        // Segment type
        uint32_t s : 1;           // Descriptor type (0=system, 1=code/data)
        uint32_t dpl : 2;         // Descriptor privilege level
        uint32_t present : 1;     // Segment present
        uint32_t reserved1 : 4;   // Reserved (cleared)
        uint32_t avl : 1;         // Available for use
        uint32_t l : 1;           // 64-bit code segment
        uint32_t db : 1;          // Default operation size
        uint32_t g : 1;           // Granularity
        uint32_t unusable : 1;    // Segment unusable
        uint32_t reserved2 : 15;  // Reserved (cleared)
    };
    uint32_t all;
} SegmentAccessRight;

uint32_t GetSegmentAccessRight(uint16_t selector) {
    if (selector == 0) {
        SegmentAccessRight ar = {0};
        ar.unusable = 1;
        return ar.all;
    }

    // LAR instruction returns access rights in high 24 bits
    uint32_t lar_result = __lar(selector);
    lar_result >>= 8;  // Shift to get access rights portion

    SegmentAccessRight ar;
    ar.all = lar_result;
    ar.reserved1 = 0;
    ar.reserved2 = 0;
    ar.unusable = 0;
    return ar.all;
}
```

---

## 5. VM-Exit Handling

### 5.1 VM-Exit Reasons

Reference: NoirVisor/src/vt_core/vt_exit.c

| Reason | Value | Description | Handler Required |
|--------|-------|-------------|------------------|
| Exception/NMI | 0 | Exception or NMI | Yes |
| External Interrupt | 1 | External interrupt | Optional |
| Triple Fault | 2 | Triple fault | Yes (fatal) |
| INIT Signal | 3 | INIT signal | Yes |
| SIPI | 4 | Startup IPI | Yes |
| I/O SMI | 5 | I/O SMI | Rare |
| Other SMI | 6 | Other SMI | Rare |
| Interrupt Window | 7 | Interrupt window | Optional |
| NMI Window | 8 | NMI window | Optional |
| Task Switch | 9 | Task switch | Yes |
| CPUID | 10 | CPUID instruction | Yes |
| GETSEC | 11 | GETSEC instruction | Yes |
| HLT | 12 | HLT instruction | Optional |
| INVD | 13 | INVD instruction | Yes |
| INVLPG | 14 | INVLPG instruction | Optional |
| RDPMC | 15 | RDPMC instruction | Optional |
| RDTSC | 16 | RDTSC instruction | Optional |
| RSM | 17 | RSM instruction | Yes |
| VMCALL | 18 | VMCALL instruction | Yes |
| VMCLEAR | 19 | VMCLEAR instruction | Yes (if nested) |
| VMLAUNCH | 20 | VMLAUNCH instruction | Yes (if nested) |
| VMPTRLD | 21 | VMPTRLD instruction | Yes (if nested) |
| VMPTRST | 22 | VMPTRST instruction | Yes (if nested) |
| VMREAD | 23 | VMREAD instruction | Yes (if nested) |
| VMRESUME | 24 | VMRESUME instruction | Yes (if nested) |
| VMWRITE | 25 | VMWRITE instruction | Yes (if nested) |
| VMXOFF | 26 | VMXOFF instruction | Yes |
| VMXON | 27 | VMXON instruction | Yes (if nested) |
| CR Access | 28 | CR access | Yes |
| MOV DR | 29 | MOV to/from DR | Optional |
| I/O Instruction | 30 | I/O instruction | Optional |
| RDMSR | 31 | RDMSR instruction | Yes |
| WRMSR | 32 | WRMSR instruction | Yes |
| VM-Entry Fail (Guest) | 33 | Invalid guest state | Yes (debug) |
| VM-Entry Fail (MSR) | 34 | MSR loading failure | Yes (debug) |
| MWAIT | 36 | MWAIT instruction | Optional |
| Monitor Trap Flag | 37 | MTF | Optional |
| MONITOR | 39 | MONITOR instruction | Optional |
| PAUSE | 40 | PAUSE instruction | Optional |
| VM-Entry Fail (MC) | 41 | Machine-check during entry | Yes (fatal) |
| TPR Below Threshold | 43 | TPR below threshold | Optional |
| APIC Access | 44 | APIC-access | Optional |
| Virtualized EOI | 45 | Virtualized EOI | Optional |
| GDTR/IDTR Access | 46 | Access to GDTR/IDTR | Optional |
| LDTR/TR Access | 47 | Access to LDTR/TR | Optional |
| EPT Violation | 48 | EPT violation | Yes |
| EPT Misconfiguration | 49 | EPT misconfiguration | Yes (fatal) |
| INVEPT | 50 | INVEPT instruction | Yes (if nested) |
| RDTSCP | 51 | RDTSCP instruction | Optional |
| VMX Preemption Timer | 52 | Preemption timer expired | Optional |
| INVVPID | 53 | INVVPID instruction | Yes (if nested) |
| WBINVD | 54 | WBINVD instruction | Optional |
| XSETBV | 55 | XSETBV instruction | Yes |
| APIC Write | 56 | APIC write | Optional |
| RDRAND | 57 | RDRAND instruction | Optional |
| INVPCID | 58 | INVPCID instruction | Optional |
| VMFUNC | 59 | VMFUNC instruction | Optional |
| ENCLS | 60 | ENCLS instruction | Optional |
| RDSEED | 61 | RDSEED instruction | Optional |
| PML Full | 62 | Page-modification log full | Optional |
| XSAVES | 63 | XSAVES instruction | Optional |
| XRSTORS | 64 | XRSTORS instruction | Optional |

### 5.2 VM-Exit Handler Structure

```c
// Reference: HyperPlatform/vmm.cpp:180-330 (VmmVmExitHandler, VmmpHandleVmExit)

// GPR structure for saving/restoring registers
typedef struct _GpRegisters {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rbp;
    uint64_t rsp;  // Placeholder - not actually used
    uint64_t rbx;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rax;
} GpRegisters;

// VM-exit handler context
typedef struct _GuestContext {
    GpRegisters* gp_regs;
    uint64_t flag_reg;
    uint64_t rip;
    uint64_t cr8;
    bool vm_continue;  // false = call VMXOFF
} GuestContext;

// Main exit handler (called from assembly)
bool VmExitHandler(GpRegisters* gpr_state) {
    // Read exit reason
    uint32_t exit_reason = (uint32_t)__vmx_vmread(0x4402) & 0xFFFF;

    // Read guest state
    GuestContext ctx;
    ctx.gp_regs = gpr_state;
    ctx.flag_reg = __vmx_vmread(0x6820);  // RFLAGS
    ctx.rip = __vmx_vmread(0x681E);       // RIP
    ctx.vm_continue = true;

    // Also restore RSP from VMCS
    gpr_state->rsp = __vmx_vmread(0x681C);

    // Dispatch based on exit reason
    switch (exit_reason) {
        case 0:   HandleException(&ctx); break;
        case 2:   HandleTripleFault(&ctx); break;
        case 10:  HandleCpuid(&ctx); break;
        case 13:  HandleInvd(&ctx); break;
        case 18:  HandleVmcall(&ctx); break;
        case 28:  HandleCrAccess(&ctx); break;
        case 31:  HandleRdmsr(&ctx); break;
        case 32:  HandleWrmsr(&ctx); break;
        case 48:  HandleEptViolation(&ctx); break;
        case 49:  HandleEptMisconfig(&ctx); break;
        case 55:  HandleXsetbv(&ctx); break;
        default:  HandleUnexpected(&ctx); break;
    }

    return ctx.vm_continue;
}

// Advance RIP past the exiting instruction
void AdvanceGuestRip(GuestContext* ctx) {
    uint64_t inst_len = __vmx_vmread(0x440C);  // VM-exit instruction length
    __vmx_vmwrite(0x681E, ctx->rip + inst_len);
}
```

### 5.3 CPUID Handler

```c
// Reference: HyperPlatform/vmm.cpp:419-445, NoirVisor/src/vt_core/vt_exit.c:366-379

void HandleCpuid(GuestContext* ctx) {
    int cpu_info[4] = {0};
    uint32_t function = (uint32_t)ctx->gp_regs->rax;
    uint32_t subfunction = (uint32_t)ctx->gp_regs->rcx;

    // Execute real CPUID
    __cpuidex(cpu_info, function, subfunction);

    // Modify specific leaves
    if (function == 1) {
        // Hide VMX capability, set hypervisor present bit
        cpu_info[2] &= ~(1 << 5);   // Clear ECX.VMX
        cpu_info[2] |= (1 << 31);   // Set ECX.HV (hypervisor present)
    }
    else if (function == 0x40000000) {
        // Return hypervisor signature
        cpu_info[0] = 0x40000001;   // Max hypervisor leaf
        cpu_info[1] = 'OmbR';       // Signature
        cpu_info[2] = 'aHyp';
        cpu_info[3] = 'erVi';
    }

    // Store results
    ctx->gp_regs->rax = cpu_info[0];
    ctx->gp_regs->rbx = cpu_info[1];
    ctx->gp_regs->rcx = cpu_info[2];
    ctx->gp_regs->rdx = cpu_info[3];

    AdvanceGuestRip(ctx);
}
```

### 5.4 MSR Handler

```c
// Reference: HyperPlatform/vmm.cpp:499-567, NoirVisor/src/vt_core/vt_exit.c:870-953

void HandleRdmsr(GuestContext* ctx) {
    uint32_t msr_index = (uint32_t)ctx->gp_regs->rcx;
    uint64_t value;

    // Handle specific MSRs via VMCS
    switch (msr_index) {
        case 0x174:  // IA32_SYSENTER_CS
            value = __vmx_vmread(0x482A);
            break;
        case 0x175:  // IA32_SYSENTER_ESP
            value = __vmx_vmread(0x6824);
            break;
        case 0x176:  // IA32_SYSENTER_EIP
            value = __vmx_vmread(0x6826);
            break;
        case 0x1D9:  // IA32_DEBUGCTL
            value = __vmx_vmread64(0x2802);
            break;
        case 0xC0000100:  // IA32_FS_BASE
            value = __vmx_vmread(0x680E);
            break;
        case 0xC0000101:  // IA32_GS_BASE
            value = __vmx_vmread(0x6810);
            break;
        default:
            // Pass through to real MSR
            value = __readmsr(msr_index);
            break;
    }

    ctx->gp_regs->rax = (uint32_t)(value & 0xFFFFFFFF);
    ctx->gp_regs->rdx = (uint32_t)(value >> 32);
    AdvanceGuestRip(ctx);
}

void HandleWrmsr(GuestContext* ctx) {
    uint32_t msr_index = (uint32_t)ctx->gp_regs->rcx;
    uint64_t value = ((uint64_t)ctx->gp_regs->rdx << 32) |
                     ((uint64_t)ctx->gp_regs->rax & 0xFFFFFFFF);

    switch (msr_index) {
        case 0x174:  // IA32_SYSENTER_CS
            __vmx_vmwrite(0x482A, value);
            break;
        case 0x175:  // IA32_SYSENTER_ESP
            __vmx_vmwrite(0x6824, value);
            break;
        case 0x176:  // IA32_SYSENTER_EIP
            __vmx_vmwrite(0x6826, value);
            break;
        case 0xC0000100:  // IA32_FS_BASE
            __vmx_vmwrite(0x680E, value);
            break;
        case 0xC0000101:  // IA32_GS_BASE
            __vmx_vmwrite(0x6810, value);
            break;
        default:
            __writemsr(msr_index, value);
            break;
    }

    AdvanceGuestRip(ctx);
}
```

### 5.5 CR Access Handler

```c
// Reference: NoirVisor/src/vt_core/vt_exit.c:732-866

typedef union _CrAccessQualification {
    struct {
        uint64_t cr_number : 4;       // CR0, CR3, CR4, or CR8
        uint64_t access_type : 2;     // 0=MOV to CR, 1=MOV from CR, 2=CLTS, 3=LMSW
        uint64_t lmsw_operand : 1;    // 0=reg, 1=mem (for LMSW)
        uint64_t reserved1 : 1;
        uint64_t gpr : 4;             // General-purpose register
        uint64_t reserved2 : 4;
        uint64_t lmsw_source : 16;    // Source data for LMSW
        uint64_t reserved3 : 32;
    };
    uint64_t all;
} CrAccessQualification;

void HandleCrAccess(GuestContext* ctx) {
    CrAccessQualification qual;
    qual.all = __vmx_vmread(0x6400);  // Exit qualification

    uint64_t* gpr_array = (uint64_t*)ctx->gp_regs;

    if (qual.access_type == 0) {  // MOV to CR
        uint64_t value = gpr_array[qual.gpr];

        switch (qual.cr_number) {
            case 0:
                __vmx_vmwrite(0x6800, value);  // Guest CR0
                __vmx_vmwrite(0x6004, value);  // CR0 read shadow
                break;
            case 3:
                __vmx_vmwrite(0x6802, value);  // Guest CR3
                // May need to invalidate TLB via INVVPID
                break;
            case 4:
                // Keep VMXE bit set in actual CR4
                __vmx_vmwrite(0x6804, value | (1 << 13));
                __vmx_vmwrite(0x6006, value);  // CR4 read shadow (without VMXE)
                break;
        }
    }
    else if (qual.access_type == 1) {  // MOV from CR
        uint64_t value;
        switch (qual.cr_number) {
            case 0:
                value = __vmx_vmread(0x6004);  // CR0 read shadow
                break;
            case 3:
                value = __vmx_vmread(0x6802);  // Guest CR3
                break;
            case 4:
                value = __vmx_vmread(0x6006);  // CR4 read shadow
                break;
        }
        gpr_array[qual.gpr] = value;
    }

    AdvanceGuestRip(ctx);
}
```

### 5.6 Event Injection

```c
// Reference: HyperPlatform/vmm.cpp:1158-1162

typedef union _VmEntryInterruptionInfo {
    struct {
        uint32_t vector : 8;          // Interrupt/exception vector
        uint32_t type : 3;            // 0=ext int, 2=NMI, 3=hw exception, 4=sw int, 5=priv sw exception, 6=sw exception
        uint32_t deliver_error : 1;   // 1=error code is valid
        uint32_t reserved : 19;
        uint32_t valid : 1;           // Must be 1 to inject
    };
    uint32_t all;
} VmEntryInterruptionInfo;

void InjectException(uint8_t vector, uint8_t type, bool has_error, uint32_t error_code) {
    VmEntryInterruptionInfo info = {0};
    info.vector = vector;
    info.type = type;
    info.deliver_error = has_error ? 1 : 0;
    info.valid = 1;

    __vmx_vmwrite(0x4016, info.all);  // VM-entry interruption-info

    if (has_error) {
        __vmx_vmwrite(0x4018, error_code);  // VM-entry exception error code
    }

    // For software exceptions/interrupts, also set instruction length
    if (type == 4 || type == 5 || type == 6) {
        uint32_t inst_len = (uint32_t)__vmx_vmread(0x440C);
        __vmx_vmwrite(0x401A, inst_len);
    }
}

// Inject #GP(0)
InjectException(13, 3, true, 0);

// Inject #UD
InjectException(6, 3, false, 0);

// Inject #PF
InjectException(14, 3, true, error_code);
__writecr2(fault_address);  // Must also update CR2
```

---

## 6. Assembly Implementation

### 6.1 VM-Exit Entry Point

```asm
; Reference: HyperPlatform/Arch/x64/x64.asm:137-234

; Macro to save all GPRs
PUSHAQ MACRO
    push    rax
    push    rcx
    push    rdx
    push    rbx
    push    -1          ; Placeholder for RSP
    push    rbp
    push    rsi
    push    rdi
    push    r8
    push    r9
    push    r10
    push    r11
    push    r12
    push    r13
    push    r14
    push    r15
ENDM

; Macro to restore all GPRs
POPAQ MACRO
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     r11
    pop     r10
    pop     r9
    pop     r8
    pop     rdi
    pop     rsi
    pop     rbp
    add     rsp, 8      ; Skip RSP placeholder
    pop     rbx
    pop     rdx
    pop     rcx
    pop     rax
ENDM

; VM-exit handler entry point (host RIP points here)
AsmVmmEntryPoint PROC
    ; Save all general-purpose registers
    PUSHAQ

    ; RCX = pointer to saved register state
    mov     rcx, rsp

    ; Save volatile XMM registers (x64 ABI requirement)
    sub     rsp, 68h
    movaps  [rsp + 00h], xmm0
    movaps  [rsp + 10h], xmm1
    movaps  [rsp + 20h], xmm2
    movaps  [rsp + 30h], xmm3
    movaps  [rsp + 40h], xmm4
    movaps  [rsp + 50h], xmm5

    ; Allocate shadow space for called function
    sub     rsp, 20h

    ; Call C handler: bool VmExitHandler(GpRegisters* gpr_state)
    call    VmExitHandler

    ; Restore shadow space
    add     rsp, 20h

    ; Restore XMM registers
    movaps  xmm0, [rsp + 00h]
    movaps  xmm1, [rsp + 10h]
    movaps  xmm2, [rsp + 20h]
    movaps  xmm3, [rsp + 30h]
    movaps  xmm4, [rsp + 40h]
    movaps  xmm5, [rsp + 50h]
    add     rsp, 68h

    ; Check return value
    test    al, al
    jz      vmx_off

    ; Resume guest execution
    POPAQ
    vmresume
    jmp     vmx_error

vmx_off:
    ; Exit VMX operation
    ; RAX = guest RFLAGS
    ; RCX = guest RIP
    ; RDX = guest RSP
    POPAQ
    vmxoff
    jz      vmx_error
    jc      vmx_error

    ; Restore guest state
    push    rax
    popfq               ; Restore RFLAGS
    mov     rsp, rdx    ; Restore RSP
    push    rcx
    ret                 ; Jump to guest RIP

vmx_error:
    ; Handle VMX failure
    int     3

AsmVmmEntryPoint ENDP
```

### 6.2 VMXON/VMLAUNCH Wrapper

```asm
; Reference: HyperPlatform/Arch/x64/x64.asm:97-135

; Initialize VM and start execution
; bool AsmInitializeVm(void (*init_routine)(rsp, rip, ctx), void* context)
AsmInitializeVm PROC
    ; Save all registers (guest state)
    pushfq
    PUSHAQ

    ; RAX = init_routine
    ; R8 = context
    ; RDX = return address (where guest resumes after VMLAUNCH)
    ; RCX = current RSP (guest RSP)
    mov     rax, rcx
    mov     r8, rdx
    mov     rdx, resume_point
    mov     rcx, rsp

    ; Call init_routine(guest_rsp, guest_rip, context)
    sub     rsp, 20h
    call    rax
    add     rsp, 20h

    ; If we get here, VMLAUNCH failed
    POPAQ
    popfq
    xor     rax, rax    ; Return false
    ret

resume_point:
    ; VMLAUNCH succeeded - guest resumes here
    nop                 ; For debugging
    POPAQ
    popfq

    xor     rax, rax
    inc     rax         ; Return true
    ret

AsmInitializeVm ENDP
```

### 6.3 VMX Instructions

```asm
; VMCALL wrapper
; unsigned char AsmVmxCall(uint64_t hypercall_number, void* context)
AsmVmxCall PROC
    vmcall              ; RCX = hypercall number, RDX = context
    jz      error_with_status
    jc      error_without_status
    xor     rax, rax    ; Return 0 (success)
    ret

error_without_status:
    mov     rax, 2      ; Return 2 (error without status)
    ret

error_with_status:
    mov     rax, 1      ; Return 1 (error with status)
    ret
AsmVmxCall ENDP

; INVEPT wrapper
; unsigned char AsmInvept(uint64_t type, void* descriptor)
AsmInvept PROC
    invept  rcx, oword ptr [rdx]
    jz      error_with_status
    jc      error_without_status
    xor     rax, rax
    ret

error_without_status:
    mov     rax, 2
    ret

error_with_status:
    mov     rax, 1
    ret
AsmInvept ENDP

; INVVPID wrapper
AsmInvvpid PROC
    invvpid rcx, oword ptr [rdx]
    jz      error_with_status
    jc      error_without_status
    xor     rax, rax
    ret

error_without_status:
    mov     rax, 2
    ret

error_with_status:
    mov     rax, 1
    ret
AsmInvvpid ENDP
```

---

## 7. Common Pitfalls

### 7.1 VMLAUNCH Failures

| Error Code | Description | Common Cause |
|------------|-------------|--------------|
| 1 | VMCALL in VMX root | Calling VMCALL when already in VMX root mode |
| 2 | VMCLEAR with invalid PA | VMCLEAR physical address not page-aligned |
| 3 | VMCLEAR with VMXON PA | Attempting to VMCLEAR the VMXON region |
| 4 | VMLAUNCH non-clear VMCS | VMCS already launched, use VMRESUME |
| 5 | VMRESUME non-launched | VMCS never launched, use VMLAUNCH |
| 6 | VMRESUME after VMXOFF | No current VMCS after VMXOFF |
| 7 | VM-entry invalid control | Control field has invalid setting |
| 8 | VM-entry invalid host | Host state field invalid |
| 9 | VMPTRLD invalid PA | VMPTRLD physical address issues |
| 10 | VMPTRLD with VMXON | Attempting to load VMXON as VMCS |
| 11 | VMPTRLD incorrect VMCS | VMCS revision ID mismatch |
| 12 | VMREAD/VMWRITE invalid | Invalid VMCS field encoding |
| 13 | VMWRITE to read-only | Writing to read-only VMCS field |
| 26 | VM-entry invalid guest | Guest state field invalid |
| 33 | VM-entry events blocked | Event injection blocked |

### 7.2 Critical Checklist

1. **Before VMXON**:
   - CR0 fixed bits applied
   - CR4.VMXE set, fixed bits applied
   - IA32_FEATURE_CONTROL lock bit set, VMXON enabled
   - VMXON region has correct revision ID

2. **Before VMLAUNCH**:
   - VMCS has correct revision ID
   - VMCLEAR executed on VMCS
   - VMPTRLD executed to make VMCS current
   - All mandatory VMCS fields written
   - Control fields adjusted via capability MSRs
   - Host selectors have RPL=0, TI=0
   - Host RSP/RIP point to valid VMM stack/handler
   - VMCS link pointer = 0xFFFFFFFFFFFFFFFF
   - Guest activity state = 0 (active)

3. **Host State Requirements**:
   - Host CR0 must have PE and PG bits set
   - Host CR4 must have PAE bit set in IA-32e mode
   - Host CS selector cannot be 0
   - Host TR selector cannot be 0
   - Host segment selectors must not have TI=1

4. **Guest State Requirements**:
   - If unrestricted guest is disabled, CR0.PE and CR0.PG consistency
   - Segment limits must be valid for their access rights
   - VMCS link pointer must be 0xFFFFFFFFFFFFFFFF or valid shadow VMCS

### 7.3 Debugging Tips

1. **Read VM Instruction Error**: After failed VM-entry, read VMCS field 0x4400
2. **Check Exit Reason**: Field 0x4402 contains basic reason and VM-entry failure flag
3. **Dump Guest State**: Use NoirVisor-style VMCS dump function to verify all fields
4. **Single-Step**: Use Monitor Trap Flag (MTF) to single-step through guest code

---

## 8. Recommended Patterns for OmbraHypervisor

### 8.1 Per-CPU VMCS Management

```c
typedef struct _VcpuContext {
    void*    vmxon_region;      // 4KB aligned
    uint64_t vmxon_region_pa;
    void*    vmcs_region;       // 4KB aligned
    uint64_t vmcs_region_pa;
    void*    vmm_stack;         // VMM stack for this CPU
    void*    msr_bitmap;        // Shared or per-CPU
    void*    io_bitmap_a;       // Shared
    void*    io_bitmap_b;       // Shared
    void*    ept_pointer;       // EPT root for this CPU
    bool     vmx_enabled;       // VMX operation active
    bool     vmcs_launched;     // VMCS has been launched
} VcpuContext;

// Global array indexed by CPU number
VcpuContext g_vcpu[MAX_PROCESSORS];
```

### 8.2 Initialization Sequence

```c
bool InitializeVmx(uint32_t cpu_index) {
    VcpuContext* vcpu = &g_vcpu[cpu_index];

    // 1. Allocate VMXON and VMCS regions
    vcpu->vmxon_region = AllocatePageAligned(4096);
    vcpu->vmcs_region = AllocatePageAligned(4096);
    vcpu->vmm_stack = AllocatePageAligned(VMM_STACK_SIZE);

    // 2. Apply CR0/CR4 fixed bits
    ApplyCrFixedBits();

    // 3. Write revision ID and execute VMXON
    WriteVmcsRevisionId(vcpu->vmxon_region);
    vcpu->vmxon_region_pa = GetPhysicalAddress(vcpu->vmxon_region);
    if (__vmx_on(&vcpu->vmxon_region_pa) != 0) {
        return false;
    }
    vcpu->vmx_enabled = true;

    // 4. Initialize VMCS
    WriteVmcsRevisionId(vcpu->vmcs_region);
    vcpu->vmcs_region_pa = GetPhysicalAddress(vcpu->vmcs_region);
    if (__vmx_vmclear(&vcpu->vmcs_region_pa) != 0) {
        return false;
    }
    if (__vmx_vmptrld(&vcpu->vmcs_region_pa) != 0) {
        return false;
    }

    // 5. Setup VMCS fields
    SetupVmcsFields(vcpu);

    // 6. Launch VM
    __vmx_vmlaunch();

    // If we reach here, VMLAUNCH failed
    return false;
}
```

### 8.3 Hypercall Interface

```c
// Hypercall numbers for OmbraHypervisor
#define OMBRA_HYPERCALL_PING            0x0001
#define OMBRA_HYPERCALL_MAP_DRIVER      0x0002
#define OMBRA_HYPERCALL_UNMAP_DRIVER    0x0003
#define OMBRA_HYPERCALL_EXIT_VMX        0xFFFF

void HandleVmcall(GuestContext* ctx) {
    uint64_t hypercall_num = ctx->gp_regs->rcx;
    uint64_t param1 = ctx->gp_regs->rdx;
    uint64_t param2 = ctx->gp_regs->r8;

    switch (hypercall_num) {
        case OMBRA_HYPERCALL_PING:
            ctx->gp_regs->rax = 'OMBR';  // Return signature
            break;

        case OMBRA_HYPERCALL_MAP_DRIVER:
            ctx->gp_regs->rax = MapDriverInternal(param1, param2);
            break;

        case OMBRA_HYPERCALL_EXIT_VMX:
            // Prepare for VMXOFF
            ctx->gp_regs->rax = ctx->flag_reg;  // RFLAGS for restore
            ctx->gp_regs->rcx = ctx->rip + GetInstructionLength();  // Return RIP
            ctx->gp_regs->rdx = ctx->gp_regs->rsp;  // Return RSP
            ctx->vm_continue = false;  // Signal to call VMXOFF
            return;  // Don't advance RIP

        default:
            // Inject #UD for unknown hypercalls
            InjectException(6, 3, false, 0);
            return;
    }

    AdvanceGuestRip(ctx);
}
```

### 8.4 EPT Pointer Format

```c
typedef union _EptPointer {
    struct {
        uint64_t memory_type : 3;      // 0=UC, 6=WB
        uint64_t page_walk_length : 3; // 3 = 4-level paging (value is N-1)
        uint64_t enable_ad_bits : 1;   // Enable accessed/dirty bits
        uint64_t reserved1 : 5;
        uint64_t pml4_pfn : 40;        // Physical frame number of PML4
        uint64_t reserved2 : 12;
    };
    uint64_t all;
} EptPointer;

// Setup EPT pointer
EptPointer eptp = {0};
eptp.memory_type = 6;        // Write-back
eptp.page_walk_length = 3;   // 4-level paging
eptp.pml4_pfn = ept_pml4_pa >> 12;

__vmx_vmwrite64(0x201A, eptp.all);
```

---

## Summary

This document provides the essential knowledge for implementing Intel VT-x in OmbraHypervisor:

1. **CPU Detection**: Check CPUID.1:ECX[5] and IA32_FEATURE_CONTROL
2. **VMXON Setup**: Allocate 4KB region, write revision ID, apply CR0/CR4 fixed bits
3. **VMCS Fields**: Complete reference of all encodings with purposes
4. **VMCS Setup**: Sequential field population with proper adjustment
5. **VM-Exit Handling**: Dispatch table with handlers for critical exits
6. **Assembly Code**: Entry point, register save/restore, VMX instructions
7. **Pitfalls**: Common errors and debugging approaches
8. **Patterns**: Recommended structures and sequences for OmbraHypervisor

Key files to reference:
- `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/HyperPlatform/HyperPlatform/vm.cpp` - VMXON/VMCS setup
- `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/HyperPlatform/HyperPlatform/vmm.cpp` - VM-exit handlers
- `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/NoirVisor/src/vt_core/vt_exit.c` - Comprehensive exit handling
- `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/NoirVisor/src/vt_core/vt_vmcs.h` - VMCS field encodings
- `/Users/jonathanmcclintock/Desktop/OmbraHypervisor/Refs/HyperPlatform/HyperPlatform/Arch/x64/x64.asm` - Assembly entry point
