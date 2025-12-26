# IDT/EXCEPTION HANDLING & VMCALL INTERFACE - C++ to C + Assembly Port Guide

## Overview

This document covers two critical infrastructure components for porting to UEFI C:

1. **IDT (Interrupt Descriptor Table) Management** - Exception handling in both guest and VMXRoot contexts
2. **VMCALL Interface** - Hypercall dispatch and authentication system

Both systems are foundational - exceptions enable SEH-like error recovery, while VMCALL provides the guest↔hypervisor communication channel.

## File Inventory

### Guest IDT (Kernel Mode)
| File | Purpose | Lines | Key Functions |
|------|---------|-------|---------------|
| `OmbraCoreLib/include/IDT.h` | IDT entry structure, register frame definitions | 132 | `IDTGateDescriptor64::setup()`, `IDT::setup()` |
| `OmbraCoreLib/src/IDT.cpp` | IDT initialization and SEH exception walking | 181 | `seh_handler_ecode()`, `seh_handler()` |
| `OmbraCoreLib/src/IDT-helper.asm` | Minimal exception stubs (non-SEH) | 38 | `__pf_handler`, `__gp_handler`, `__de_handler` |

### VMXRoot IDT (Hypervisor Mode)
| File | Purpose | Lines | Key Functions |
|------|---------|-------|---------------|
| `OmbraCoreLib-v/include/VmIDT.h` | VMXRoot exception handler declarations | 20 | `SetupIDTVm()`, `NmiHandler()` |
| `OmbraCoreLib-v/src/VmIDT.cpp` | SEH exception walking with guest re-injection | 145 | `seh_handler_ecode_vm()`, `seh_handler_vm()`, `NmiHandler()` |
| `OmbraCoreLib-v/src/VmIDT-helper.asm` | NMI-blocking handlers, full register save | 165 | `__nmi_handler_vm`, `generic_interrupt_handler_ecode_vm` |

### AMD VMXRoot IDT (Payload-specific)
| File | Purpose | Lines | Key Functions |
|------|---------|-------|---------------|
| `PayLoad/amd/exception.h` | AMD SVM exception handler interface | 109 | `SaveOrigParams()`, `Initialize()` |
| `PayLoad/amd/exception.cpp` | SEH with Hyper-V fallback recovery | 210 | `seh_handler_ecode_vm()`, `seh_handler_vm()` |
| `PayLoad/amd/IDT.asm` | Generic handlers (similar to OmbraCoreLib-v) | 117 | `__pf_handler_vm`, `__gp_handler_vm` |

### VMCALL Interface
| File | Purpose | Lines | Key Components |
|------|---------|-------|----------------|
| `PayLoad/include/vmcall.h` | Authentication types, key validation | 40 | `vmcall::IsVmcall()`, `vmcall::SetKey()` |
| `PayLoad/core/dispatch.h` | Handler declarations, arch callbacks | 108 | `HandleVmcall()`, `RegisterArchCallbacks()` |
| `PayLoad/core/dispatch.cpp` | Main dispatch loop, command handlers | 803 | `HandleVmcall()`, 14 command handlers |
| `OmbraShared/communication.hpp` | VMCALL types, command codes, data structures | 358 | `VMCALL_TYPE` enum, `COMMAND_DATA` union |

### Legacy VMCALL (OmbraCoreLib-v - reference only)
| File | Purpose | Lines | Notes |
|------|---------|-------|-------|
| `OmbraCoreLib-v/include/Vmcall.h` | Legacy handler map, callback system | 151 | **DEPRECATED** - uses C++ hash map |
| `OmbraCoreLib-v/src/Vmcall.cpp` | Legacy switch-based dispatch | 334 | **DEPRECATED** - superseded by PayLoad/core/dispatch.cpp |

## Architecture Summary

### Guest vs VMXRoot IDT

```
┌──────────────────────────────────────────────────────────┐
│ Guest IDT (Windows Kernel Mode)                          │
├──────────────────────────────────────────────────────────┤
│ - Used by OmbraDriver.sys and kernel code                │
│ - Full 256-entry table                                   │
│ - SEH exception walking (RUNTIME_FUNCTION, SCOPE_TABLE)  │
│ - Simple handlers: just iretq (no recovery)              │
│ - __gp_handler, __pf_handler, __de_handler               │
└──────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────┐
│ VMXRoot IDT (Hypervisor Mode)                            │
├──────────────────────────────────────────────────────────┤
│ - Used by PayLoad.dll (Ring -1)                          │
│ - Full register save/restore                             │
│ - SEH with re-injection to guest on failure              │
│ - NMI-blocking iret emulation (__nmi_handler_vm)         │
│ - Recovery: fall back to original Hyper-V handler        │
└──────────────────────────────────────────────────────────┘
```

### VMCALL Flow

```
┌─────────────────────────────────────────────────────────────────────────┐
│ Usermode / Kernel                                                        │
│   ombra::read_guest_virt(target_cr3, addr, buf, size)                   │
│       ↓                                                                  │
│   CPUID(0x13371337, RCX=VMCALL_READ_VIRT, RDX=&cmd, R8=cr3, R9=key)     │
└─────────────────────────────────────────────────────────────────────────┘
                                    ↓
┌─────────────────────────────────────────────────────────────────────────┐
│ CPU VMExit (VMEXIT_CPUID)                                                │
└─────────────────────────────────────────────────────────────────────────┘
                                    ↓
┌─────────────────────────────────────────────────────────────────────────┐
│ PayLoad/{amd,intel}/*_handler.cpp::vmexit_handler()                     │
│   if (exit_reason == CPUID && cpuid_idx == 0x13371337)                  │
│       ctx = new VmExitContext(rcx, rdx, r8, r9, guest_cr3, arch_data)   │
│       result = core::HandleVmcall(&ctx)                                  │
│       guest_rax = result                                                 │
│       return to guest                                                    │
└─────────────────────────────────────────────────────────────────────────┘
                                    ↓
┌─────────────────────────────────────────────────────────────────────────┐
│ PayLoad/core/dispatch.cpp::HandleVmcall()                                │
│   if (!vmcall::IsVmcall(ctx->auth_key)) return INVALID_GUEST_PARAM      │
│   switch (ctx->GetCommandType())                                        │
│       case VMCALL_READ_VIRT: return HandleReadVirt(ctx)                 │
│       case VMCALL_WRITE_VIRT: return HandleWriteVirt(ctx)               │
│       ... (14 total commands)                                            │
└─────────────────────────────────────────────────────────────────────────┘
                                    ↓
┌─────────────────────────────────────────────────────────────────────────┐
│ PayLoad/core/dispatch.cpp::HandleReadVirt()                              │
│   mm::copy_guest_virt(target_cr3, src, guest_cr3, dst, size)            │
│   return VMX_ROOT_ERROR::SUCCESS                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

## IDT Structure

### IDT Entry Format (64-bit)

```c
// Vol3 Ch 6.14.1 - 64-bit Interrupt Gate Descriptor
typedef union _IDT_GATE_DESCRIPTOR_64
{
    struct
    {
        UINT32 offset_0_15 : 16;      // Bits 0-15 of handler address
        UINT32 cs_selector : 16;      // Code segment selector
        UINT32 ist : 3;               // Interrupt Stack Table index (0 = don't switch)
        UINT32 must_be_zero1 : 5;     // Reserved, must be 0
        UINT32 type : 5;              // Gate type (0xE = Interrupt Gate, 0xF = Trap Gate)
        UINT32 dpl : 2;               // Descriptor Privilege Level
        UINT32 present : 1;           // Present flag
        UINT32 offset_16_31 : 16;     // Bits 16-31 of handler address
        UINT32 offset_32_63 : 32;     // Bits 32-63 of handler address
        UINT32 reserved : 32;         // Reserved, must be 0
    } bits;
    UINT32 values[4];
} IDT_GATE_DESCRIPTOR_64;

// Size: 16 bytes (128 bits)
// Total IDT size: 256 entries * 16 bytes = 4096 bytes
```

### IDT Register (IDTR)

```c
typedef struct _IDT_REGISTER
{
    UINT16 limit;          // Size of IDT in bytes minus 1 (e.g., 4095 for 256 entries)
    UINT64 base_address;   // Linear address of IDT
} IDT_REGISTER;

// Load with: __lidt(&idt_register)
// Read with: __sidt(&idt_register)
```

### Gate Types

| Type | Value | Description | Use Case |
|------|-------|-------------|----------|
| Interrupt Gate | 0xE | Disables interrupts (clears IF) | Most exceptions (#PF, #GP, #DE) |
| Trap Gate | 0xF | Leaves interrupts enabled | Debug exceptions (#DB) |

**Critical**: For VMXRoot handlers, always use Interrupt Gate (0xE) to prevent re-entrant exceptions during exception handling.

## Guest IDT Virtualization

### Initialization (OmbraCoreLib/src/IDT.cpp)

```c
// C++ version
void IDT::setup()
{
    RtlZeroMemory(descriptor, sizeof(descriptor));
    setup(generic_interrupt_handler, generic_interrupt_handler_ecode);
}

void IDT::setup(void(*handler)(), void (*handler_ecode)())
{
    // Exceptions WITHOUT error code
    setup_entry(0, true, handler);    // #DE - Divide Error
    setup_entry(1, true, handler);    // #DB - Debug
    setup_entry(3, true, handler);    // #BP - Breakpoint
    setup_entry(4, true, handler);    // #OF - Overflow
    setup_entry(5, true, handler);    // #BR - BOUND Range Exceeded
    setup_entry(6, true, handler);    // #UD - Invalid Opcode
    setup_entry(7, true, handler);    // #NM - Device Not Available

    // Exceptions WITH error code
    setup_entry(8, true, handler_ecode);   // #DF - Double Fault
    setup_entry(10, true, handler_ecode);  // #TS - Invalid TSS
    setup_entry(11, true, handler_ecode);  // #NP - Segment Not Present
    setup_entry(12, true, handler_ecode);  // #SS - Stack Fault
    setup_entry(13, true, handler_ecode);  // #GP - General Protection
    setup_entry(14, true, handler_ecode);  // #PF - Page Fault
    setup_entry(17, true, handler_ecode);  // #AC - Alignment Check

    // Non-exception entries
    setup_entry(16, true, handler);   // #MF - x87 FPU Error
    setup_entry(18, true, handler);   // #MC - Machine Check
    setup_entry(19, true, handler);   // #XM - SIMD Floating-Point Exception
    setup_entry(20, true, handler);   // #VE - Virtualization Exception

    // Reserved entries (15, 21-31) - mark as not present
    for (int i = 15; i == 15 || (i >= 21 && i <= 31); i++)
        setup_entry(i, false);
}
```

### Pure C Port

```c
// idt.h
typedef struct _IDT_TABLE {
    IDT_GATE_DESCRIPTOR_64 entries[256];
} IDT_TABLE;

// idt.c
void idt_setup_entry(
    IDT_TABLE* idt,
    UINT32 vector,
    void* handler,
    UINT16 cs_selector,
    UINT8 type,
    UINT8 present)
{
    UINT64 handler_addr = (UINT64)handler;

    idt->entries[vector].bits.offset_0_15 = (UINT16)(handler_addr & 0xFFFF);
    idt->entries[vector].bits.offset_16_31 = (UINT16)((handler_addr >> 16) & 0xFFFF);
    idt->entries[vector].bits.offset_32_63 = (UINT32)(handler_addr >> 32);

    idt->entries[vector].bits.cs_selector = cs_selector;
    idt->entries[vector].bits.type = type;
    idt->entries[vector].bits.present = present;
    idt->entries[vector].bits.ist = 0;            // Don't switch stacks
    idt->entries[vector].bits.dpl = 0;            // Ring 0 only
    idt->entries[vector].bits.must_be_zero1 = 0;
    idt->entries[vector].bits.reserved = 0;
}

#define SEGMENT_DESCRIPTOR_TYPE_INTERRUPT_GATE 0xE

void idt_initialize(IDT_TABLE* idt, UINT16 cs_selector)
{
    // Zero entire table
    for (int i = 0; i < 256; i++) {
        idt->entries[i].values[0] = 0;
        idt->entries[i].values[1] = 0;
        idt->entries[i].values[2] = 0;
        idt->entries[i].values[3] = 0;
    }

    // Exceptions without error code
    idt_setup_entry(idt, 0, generic_interrupt_handler, cs_selector,
                    SEGMENT_DESCRIPTOR_TYPE_INTERRUPT_GATE, 1);
    idt_setup_entry(idt, 1, generic_interrupt_handler, cs_selector,
                    SEGMENT_DESCRIPTOR_TYPE_INTERRUPT_GATE, 1);
    // ... (continue for vectors 3-7, 16, 18-20)

    // Exceptions with error code
    idt_setup_entry(idt, 8, generic_interrupt_handler_ecode, cs_selector,
                    SEGMENT_DESCRIPTOR_TYPE_INTERRUPT_GATE, 1);
    idt_setup_entry(idt, 10, generic_interrupt_handler_ecode, cs_selector,
                    SEGMENT_DESCRIPTOR_TYPE_INTERRUPT_GATE, 1);
    // ... (continue for vectors 11-14, 17)
}

void idt_load(IDT_TABLE* idt)
{
    IDT_REGISTER idtr;
    idtr.limit = sizeof(IDT_TABLE) - 1;  // 4095
    idtr.base_address = (UINT64)idt;
    __lidt(&idtr);
}
```

## Exception Injection

### Stack Frame Layout

```
┌─────────────────────────────────────────────────────────────────┐
│ WITHOUT Error Code (e.g., #DE)    │ WITH Error Code (e.g., #PF) │
├────────────────────────────────────┼──────────────────────────────┤
│ +0x28: SS                          │ +0x30: SS                    │
│ +0x20: RSP                         │ +0x28: RSP                   │
│ +0x18: RFLAGS                      │ +0x20: RFLAGS                │
│ +0x10: CS                          │ +0x18: CS                    │
│ +0x08: RIP                         │ +0x10: RIP                   │
│ +0x00: ← Stack pointer             │ +0x08: Error Code            │
│                                    │ +0x00: ← Stack pointer       │
└────────────────────────────────────┴──────────────────────────────┘
```

### Register Frame Structures

```c
// For exceptions WITHOUT error code
typedef struct _IDT_REGS
{
    // Pushed by handler (in reverse order)
    UINT64 r15;
    UINT64 r14;
    UINT64 r13;
    UINT64 r12;
    UINT64 r11;
    UINT64 r10;
    UINT64 r9;
    UINT64 r8;
    UINT64 rbp;
    UINT64 rdi;
    UINT64 rsi;
    UINT64 rdx;
    UINT64 rcx;
    UINT64 rbx;
    UINT64 rax;

    // Pushed by CPU
    UINT64 rip;
    UINT64 cs_selector;
    UINT64 rflags;
    UINT64 rsp;
    UINT64 ss_selector;
} IDT_REGS;

// For exceptions WITH error code
typedef struct _IDT_REGS_ECODE
{
    // Pushed by handler (in reverse order)
    UINT64 r15;
    UINT64 r14;
    UINT64 r13;
    UINT64 r12;
    UINT64 r11;
    UINT64 r10;
    UINT64 r9;
    UINT64 r8;
    UINT64 rbp;
    UINT64 rdi;
    UINT64 rsi;
    UINT64 rdx;
    UINT64 rcx;
    UINT64 rbx;
    UINT64 rax;

    UINT64 error_code;     // Pushed by CPU
    UINT64 rip;            // Pushed by CPU
    UINT64 cs_selector;    // Pushed by CPU
    UINT64 rflags;         // Pushed by CPU
    UINT64 rsp;            // Pushed by CPU
    UINT64 ss_selector;    // Pushed by CPU
} IDT_REGS_ECODE;
```

### #GP (General Protection) - Vector 13

**Error Code Format** (when non-zero):
```
Bits 0-15: Selector Index
Bit 16: External (0=internal, 1=external event)
Bits 17-18: Table (0=GDT, 1=IDT, 2=LDT)
Bits 19-31: Reserved
```

Common causes:
- Null selector reference
- Segment limit exceeded
- Non-present segment
- Stack overflow
- Privilege violation

### #PF (Page Fault) - Vector 14

**Error Code Format**:
```
Bit 0: P (Present) - 0=not present, 1=protection violation
Bit 1: W/R (Write/Read) - 0=read, 1=write
Bit 2: U/S (User/Supervisor) - 0=supervisor, 1=user
Bit 3: RSVD (Reserved) - 1=reserved bit set
Bit 4: I/D (Instruction/Data) - 1=instruction fetch
Bit 5: PK (Protection Key) - 1=protection key violation
Bit 15: SGX - 1=SGX access control violation
```

**Faulting Address**: Read from CR2

```c
void handle_page_fault(IDT_REGS_ECODE* regs)
{
    UINT64 fault_addr = __readcr2();
    UINT32 error_code = (UINT32)regs->error_code;

    BOOLEAN present = error_code & 0x1;
    BOOLEAN write = error_code & 0x2;
    BOOLEAN user = error_code & 0x4;

    if (!present) {
        // Page not present - allocate and map
    }
    else if (write) {
        // Write to read-only page - check permissions
    }
    // ... handle fault ...

    // Modify RIP to retry or skip instruction
    regs->rip += instruction_length;
}
```

### #UD (Undefined Opcode) - Vector 6

No error code. Used to detect CPUID-based VMCALLs.

```c
void handle_undefined_opcode(IDT_REGS* regs)
{
    // Read instruction bytes at RIP
    UINT8* inst = (UINT8*)regs->rip;

    // Check if it's CPUID (0x0F 0xA2)
    if (inst[0] == 0x0F && inst[1] == 0xA2) {
        // Handle as VMCALL if magic value in EAX
        if (regs->rax == 0x13371337) {
            // Dispatch VMCALL
        }
        regs->rip += 2;  // Skip CPUID instruction
    }
}
```

## VMCALL Interface

### Command Code Table

From `OmbraShared/communication.hpp`:

| VMCALL_TYPE | Code | RCX | RDX | R8 | R9 | Purpose |
|-------------|------|-----|-----|----|----|---------|
| **Memory Operations** |
| VMCALL_READ_VIRT | 0x100B | Command code | &COMMAND_DATA | target_cr3 | auth_key | Read virtual memory (cross-process) |
| VMCALL_WRITE_VIRT | 0x100C | Command code | &COMMAND_DATA | target_cr3 | auth_key | Write virtual memory (cross-process) |
| VMCALL_READ_PHY | 0x100D | Command code | &COMMAND_DATA | unused | auth_key | Read physical memory |
| VMCALL_WRITE_PHY | 0x100E | Command code | &COMMAND_DATA | unused | auth_key | Write physical memory |
| VMCALL_VIRT_TO_PHY | 0x1013 | Command code | &COMMAND_DATA | dirbase | auth_key | Translate VA to PA |
| **CR3 Operations** |
| VMCALL_GET_CR3 | 0x1011 | Command code | &COMMAND_DATA | unused | auth_key | Get guest CR3 |
| VMCALL_GET_CR3_ROOT | 0x1016 | Command code | &COMMAND_DATA | unused | auth_key | Get vmxroot CR3 |
| **EPT/NPT Operations** |
| VMCALL_GET_EPT_BASE | 0x1012 | Command code | &COMMAND_DATA | unused | auth_key | Get EPT/NPT base pointer |
| VMCALL_SET_EPT_BASE | 0x1015 | Command code | &COMMAND_DATA | unused | auth_key | Set EPT/NPT base pointer |
| VMCALL_DISABLE_EPT | 0x100F | Command code | &COMMAND_DATA | unused | auth_key | Disable nested paging |
| VMCALL_ENABLE_EPT | 0x1017 | Command code | &COMMAND_DATA | unused | auth_key | Enable nested paging |
| **Storage & Communication** |
| VMCALL_STORAGE_QUERY | 0x1014 | Command code | &COMMAND_DATA | unused | auth_key | Read/write storage slots |
| VMCALL_SET_COMM_KEY | 0x1010 | Command code | unused | new_key | unused | Set authentication key |
| **AMD-Specific** |
| VMCALL_GET_VMCB | 0x1018 | Command code | &COMMAND_DATA | unused | auth_key | Get VMCB physical address |
| **Artifact Elimination (Phase 3)** |
| VMCALL_DISABLE_ETW_TI | 0x1020 | Command code | &COMMAND_DATA | unused | auth_key | Disable ETW Threat Intel provider |
| VMCALL_ENABLE_ETW_TI | 0x1021 | Command code | &COMMAND_DATA | unused | auth_key | Re-enable ETW Threat Intel |
| VMCALL_WIPE_ETW_BUFFERS | 0x1022 | Command code | &COMMAND_DATA | unused | auth_key | Zero matching events in ETW buffers |
| VMCALL_CLEAR_EVENT_LOGS | 0x1023 | Command code | &COMMAND_DATA | unused | auth_key | Clear Windows Event Log entries |

### Register Convention

```
┌──────────────────────────────────────────────────────────────┐
│ Register  │ Input                                            │
├──────────────────────────────────────────────────────────────┤
│ RAX       │ CPUID index (0x13371337 = magic)                │
│ RCX       │ VMCALL_TYPE (command code)                       │
│ RDX       │ &COMMAND_DATA (guest virtual address)            │
│ R8        │ Extra parameter (often target_cr3)              │
│ R9        │ VMEXIT_KEY (authentication key)                  │
├──────────────────────────────────────────────────────────────┤
│ Register  │ Output                                           │
├──────────────────────────────────────────────────────────────┤
│ RAX       │ VMX_ROOT_ERROR (result code)                     │
└──────────────────────────────────────────────────────────────┘
```

### Authentication (VMEXIT_KEY)

From `PayLoad/include/vmcall.h`:

```c
namespace vmcall {
    extern u64 g_comm_key;  // Global authentication key

    // Validate hypercall authentication
    // Key comes in R9 register
    inline bool IsVmcall(u64 key) {
        return key == g_comm_key && g_comm_key != 0;
    }

    // Set the communication key (via VMCALL_SET_COMM_KEY)
    inline void SetKey(u64 key) {
        g_comm_key = key;
    }
}
```

**Key Exchange Flow**:
```
1. Hypervisor loads, g_comm_key = 0 (unauthenticated mode)
2. Loader calls: VMCALL_SET_COMM_KEY with new_key in R8
3. Hypervisor: if (g_comm_key == 0) g_comm_key = new_key
4. All subsequent VMCALLs: validate R9 == g_comm_key
```

**Port to C**:
```c
// vmcall.h
extern UINT64 g_vmcall_comm_key;

BOOLEAN vmcall_is_authenticated(UINT64 key);
void vmcall_set_key(UINT64 key);

// vmcall.c
UINT64 g_vmcall_comm_key = 0;

BOOLEAN vmcall_is_authenticated(UINT64 key) {
    return (key == g_vmcall_comm_key) && (g_vmcall_comm_key != 0);
}

void vmcall_set_key(UINT64 key) {
    g_vmcall_comm_key = key;
}
```

### COMMAND_DATA Union

From `OmbraShared/communication.hpp`:

```c
typedef union _COMMAND_DATA {
    READ_DATA read;              // For VMCALL_READ_VIRT/PHY
    WRITE_DATA write;            // For VMCALL_WRITE_VIRT/PHY
    CR3_DATA cr3;                // For VMCALL_GET_CR3/CR3_ROOT/EPT_BASE
    TRANSLATION_DATA translation;// For VMCALL_VIRT_TO_PHY
    EPT_TRANSLATION_DATA ept;    // Reserved for future EPT table ops
    STORAGE_DATA storage;        // For VMCALL_STORAGE_QUERY
    ETW_DATA etw;                // For VMCALL_DISABLE/ENABLE_ETW_TI
    ETW_WIPE_DATA etw_wipe;      // For VMCALL_WIPE_ETW_BUFFERS
    EVENT_LOG_CLEAR_DATA event_log_clear; // For VMCALL_CLEAR_EVENT_LOGS
    PVOID handler;               // Generic pointer field
    UINT64 pa;                   // Generic physical address field
} COMMAND_DATA;

// Size: Maximum of all member sizes (typically 64-256 bytes)
```

**Key Sub-structures**:

```c
typedef struct _READ_DATA {
    PVOID pOutBuf;    // Guest virtual address for output buffer
    PVOID pTarget;    // Target address to read from
    UINT64 length;    // Number of bytes to read
} READ_DATA;

typedef struct _WRITE_DATA {
    PVOID pInBuf;     // Guest virtual address of input buffer
    PVOID pTarget;    // Target address to write to
    UINT64 length;    // Number of bytes to write
} WRITE_DATA;

typedef struct _STORAGE_DATA {
    UINT64 id;        // VMX_ROOT_STORAGE slot ID (0-127)
    union {
        PVOID pvoid;  // Pointer value
        UINT64 uint64;// Integer value
    };
    BOOLEAN bWrite;   // TRUE=write, FALSE=read
} STORAGE_DATA;

typedef struct _ETW_DATA {
    UINT64 ntoskrnl_base;    // Base of ntoskrnl.exe
    UINT64 offset;           // Offset to EtwThreatIntProvRegHandle
    UINT64 saved_value;      // [OUT] Original ProviderEnableInfo for restore
} ETW_DATA;

typedef struct _ETW_WIPE_DATA {
    UINT64 ntoskrnl_base;
    UINT64 etwp_logger_list_offset;
    UINT64 timestamp_start;   // Wipe events >= this timestamp
    UINT64 timestamp_end;     // Wipe events <= this timestamp (0 = no limit)
    UINT32 events_wiped;      // [OUT] Number wiped
    UINT32 buffers_scanned;   // [OUT] Buffers examined
    char target_driver_name[64];  // Reserved for future use
} ETW_WIPE_DATA;
```

## Dispatch Implementation

### Main Entry Point (PayLoad/core/dispatch.cpp)

```c
VMX_ROOT_ERROR HandleVmcall(ombra::VmExitContext* ctx)
{
    // 1. Validate authentication key
    if (!vmcall::IsVmcall(ctx->auth_key)) {
        return VMX_ROOT_ERROR::INVALID_GUEST_PARAM;
    }

    // 2. Read command data from guest memory if needed
    if (!ctx->cmd_cached && ctx->cmd_guest_va) {
        mm::copy_guest_virt(
            ctx->guest_cr3,
            ctx->cmd_guest_va,
            __readcr3(),
            (u64)&ctx->local_cmd,
            sizeof(COMMAND_DATA)
        );
        ctx->cmd_cached = true;
    }

    // 3. Dispatch based on command type
    VMX_ROOT_ERROR result;

    switch (ctx->GetCommandType()) {
        case VMCALL_GET_CR3:
            result = handlers::HandleGetCr3(ctx);
            break;

        case VMCALL_READ_VIRT:
            result = handlers::HandleReadVirt(ctx);
            break;

        // ... (14 total cases)

        default:
            result = VMX_ROOT_ERROR::INVALID_GUEST_PARAM;
            break;
    }

    return result;
}
```

**Port to C**:

```c
// dispatch.h
typedef enum _VMX_ROOT_ERROR {
    VMX_SUCCESS = 0,
    VMX_PML4E_NOT_PRESENT,
    VMX_PDPTE_NOT_PRESENT,
    VMX_PDE_NOT_PRESENT,
    VMX_PTE_NOT_PRESENT,
    VMX_TRANSLATE_FAILURE,
    VMX_INVALID_GUEST_PARAM,
    VMX_PAGE_FAULT
} VMX_ROOT_ERROR;

typedef struct _VMEXIT_CONTEXT {
    UINT64 command_type;     // RCX
    UINT64 cmd_guest_va;     // RDX
    UINT64 extra_param;      // R8
    UINT64 auth_key;         // R9
    UINT64 guest_cr3;        // Current guest CR3
    void* arch_data;         // Intel VMCS or AMD VMCB
    COMMAND_DATA local_cmd;  // Cached command data
    BOOLEAN cmd_cached;      // Has command been read?
} VMEXIT_CONTEXT;

VMX_ROOT_ERROR vmcall_dispatch(VMEXIT_CONTEXT* ctx);

// dispatch.c
VMX_ROOT_ERROR vmcall_dispatch(VMEXIT_CONTEXT* ctx)
{
    // Validate authentication
    if (!vmcall_is_authenticated(ctx->auth_key)) {
        return VMX_INVALID_GUEST_PARAM;
    }

    // Read command data from guest if needed
    if (!ctx->cmd_cached && ctx->cmd_guest_va) {
        mm_copy_guest_virt(
            ctx->guest_cr3,
            ctx->cmd_guest_va,
            read_cr3(),
            (UINT64)&ctx->local_cmd,
            sizeof(COMMAND_DATA)
        );
        ctx->cmd_cached = TRUE;
    }

    // Dispatch
    switch (ctx->command_type) {
        case VMCALL_GET_CR3:
            return vmcall_handle_get_cr3(ctx);

        case VMCALL_READ_VIRT:
            return vmcall_handle_read_virt(ctx);

        case VMCALL_WRITE_VIRT:
            return vmcall_handle_write_virt(ctx);

        case VMCALL_VIRT_TO_PHY:
            return vmcall_handle_virt_to_phy(ctx);

        case VMCALL_STORAGE_QUERY:
            return vmcall_handle_storage_query(ctx);

        case VMCALL_SET_COMM_KEY:
            vmcall_set_key(ctx->extra_param);  // Key in R8
            return VMX_SUCCESS;

        // ... (continue for all 14 commands)

        default:
            return VMX_INVALID_GUEST_PARAM;
    }
}
```

### Example Handler: VMCALL_READ_VIRT

```c
// C++ version (PayLoad/core/dispatch.cpp)
VMX_ROOT_ERROR HandleReadVirt(ombra::VmExitContext* ctx) {
    auto& cmd = ctx->local_cmd;

    // Use target CR3 from R8, or fall back to NTOSKRNL_CR3 from storage
    u64 target_cr3 = ctx->extra_param;
    if (!target_cr3) {
        target_cr3 = storage::Query(VMX_ROOT_STORAGE::NTOSKRNL_CR3);
    }

    if (!cmd.read.pOutBuf) {
        return VMX_ROOT_ERROR::VMXROOT_TRANSLATE_FAILURE;
    }

    return mm::copy_guest_virt(
        target_cr3,
        (u64)cmd.read.pTarget,
        ctx->guest_cr3,
        (u64)cmd.read.pOutBuf,
        cmd.read.length
    );
}

// C port
VMX_ROOT_ERROR vmcall_handle_read_virt(VMEXIT_CONTEXT* ctx)
{
    READ_DATA* cmd = &ctx->local_cmd.read;

    // Get target CR3
    UINT64 target_cr3 = ctx->extra_param;
    if (target_cr3 == 0) {
        target_cr3 = storage_query(VMX_ROOT_STORAGE_NTOSKRNL_CR3);
    }

    // Validate output buffer
    if (cmd->pOutBuf == NULL) {
        return VMX_TRANSLATE_FAILURE;
    }

    // Copy memory
    return mm_copy_guest_virt(
        target_cr3,
        (UINT64)cmd->pTarget,
        ctx->guest_cr3,
        (UINT64)cmd->pOutBuf,
        cmd->length
    );
}
```

### Storage Slots (VMX_ROOT_STORAGE)

From `OmbraShared/communication.hpp`:

```c
enum VMX_ROOT_STORAGE {
    CALLBACK_ADDRESS = 0,         // Driver callback for kernel requests
    EPT_HANDLER_ADDRESS,          // EPT violation handler
    EPT_OS_INIT_BITMAP,           // OS page bitmap (9 slots)
    EPT_OS_INIT_BITMAP_END = EPT_OS_INIT_BITMAP + 8,
    DRIVER_BASE_PA,               // Physical address of hidden driver
    NTOSKRNL_CR3,                 // System process CR3
    CURRENT_CONTROLLER_PROCESS,   // Reserved
    PAYLOAD_BASE,                 // SUPDrv-loaded payload base
    MAX_STORAGE = 127
};
```

Implementation (PayLoad/include/storage.h):

```c
namespace storage {
    // Per-core storage slots (576 cores max, 128 slots per core)
    inline static u64 vmxroot_storage[576][128] = { 0 };

    inline u64 Query(u32 slot) {
        u32 core = CPU::ApicId();
        return vmxroot_storage[core][slot];
    }

    inline void Set(u32 slot, u64 value) {
        u32 core = CPU::ApicId();
        vmxroot_storage[core][slot] = value;
    }
}

// C port
UINT64 g_vmxroot_storage[576][128];  // Zero-initialized by BSS

UINT64 storage_query(UINT32 slot)
{
    UINT32 core = cpu_get_apic_id();
    if (core >= 576 || slot >= 128) return 0;
    return g_vmxroot_storage[core][slot];
}

void storage_set(UINT32 slot, UINT64 value)
{
    UINT32 core = cpu_get_apic_id();
    if (core >= 576 || slot >= 128) return;
    g_vmxroot_storage[core][slot] = value;
}
```

## VMXRoot Exception Handling

### Differences from Guest Exception Handling

| Aspect | Guest IDT | VMXRoot IDT |
|--------|-----------|-------------|
| Context | Windows kernel mode | Hypervisor Ring -1 |
| Re-injection | No | Yes - inject to guest via VMCS/VMCB |
| Recovery | None (crash on unhandled) | Fall back to original Hyper-V handler |
| NMI handling | Standard iretq | NMI-blocking iret emulation |
| Register preservation | Minimal (compiler handles) | Full save/restore (all GPRs) |

### NMI Windowing (OmbraCoreLib-v/VmIDT-helper.asm)

**Problem**: Standard `iretq` unblocks NMIs, but we're in VMXRoot context where we control interrupt delivery. Re-enabling NMIs prematurely can cause re-entrant NMI storms.

**Solution**: Emulate `iretq` without unblocking NMIs.

```asm
; Macro for NMI-blocking iret
nmiret macro
    ; Emulate the iret instruction in order not to unblock NMIs.
    push rax                    ; Save RAX
    ; Stack layout now:
    ; qword ptr [rsp+8*0]  rax
    ; qword ptr [rsp+8*1]  rip
    ; word ptr [rsp+8*2]   cs
    ; qword ptr [rsp+8*3]  rflags
    ; qword ptr [rsp+8*4]  rsp
    ; qword ptr [rsp+8*5]  ss

    mov rax, rsp                ; Save NMI stack pointer
    lss rsp, [rax+8*4]          ; Restore SS:RSP (switch back to pre-NMI stack)

    ; Now using pre-NMI stack, so use rax for memory addressing
    push qword ptr [rax+18h]    ; Push RFLAGS
    popfq                       ; Restore RFLAGS
    push qword ptr [rax+10h]    ; Push CS
    push qword ptr [rax+8h]     ; Push RIP
    mov rax, qword ptr [rax]    ; Restore RAX
    retfq                       ; Far return (pops CS:RIP)
endm

__nmi_handler_vm PROC
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    sub rsp, 20h
    call NmiHandler
    add rsp, 20h

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    nmiret                      ; Use NMI-blocking return
__nmi_handler_vm ENDP
```

### SEH Exception Walking (OmbraCoreLib-v/VmIDT.cpp)

When an exception occurs in VMXRoot code, we walk the PE exception directory to find a matching __try/__except block.

```c
void seh_handler_ecode_vm(PIDT_REGS_ECODE regs)
{
    // 1. Save error code for later inspection
    vmm::vGuestStates[CPU::GetCPUIndex(true)].lastErrorCode = regs->error_code;

    // 2. Calculate RVA of faulting instruction
    const auto rva = regs->rip - reinterpret_cast<DWORD64>(winternl::pDriverBase);

    // 3. Get PE headers
    const auto nt_headers = reinterpret_cast<IMAGE_NT_HEADERS64*>(
        reinterpret_cast<DWORD64>(winternl::pDriverBase) +
        reinterpret_cast<IMAGE_DOS_HEADER*>(winternl::pDriverBase)->e_lfanew);

    // 4. Get exception directory
    const auto exception =
        &nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];

    const auto functions =
        reinterpret_cast<RUNTIME_FUNCTION*>(
            reinterpret_cast<DWORD64>(winternl::pDriverBase) + exception->VirtualAddress);

    // 5. Walk RUNTIME_FUNCTION table to find matching function
    for (auto idx = 0; idx < exception->Size / sizeof(RUNTIME_FUNCTION); ++idx)
    {
        const auto function = &functions[idx];
        if (!(rva >= function->BeginAddress && rva < function->EndAddress))
            continue;

        // 6. Get UNWIND_INFO for this function
        const auto unwind_info =
            reinterpret_cast<UNWIND_INFO*>(
                reinterpret_cast<DWORD64>(winternl::pDriverBase) + function->UnwindData);

        // 7. Check if function has exception handler
        if (!(unwind_info->Flags & UNW_FLAG_EHANDLER))
            continue;

        // 8. Get SCOPE_TABLE (list of __try blocks)
        const auto scope_table =
            reinterpret_cast<SCOPE_TABLE*>(
                reinterpret_cast<DWORD64>(&unwind_info->UnwindCode[
                    (unwind_info->CountOfCodes + 1) & ~1]) + sizeof(DWORD32));

        // 9. Find matching __try block
        for (DWORD32 entry = 0; entry < scope_table->Count; ++entry)
        {
            const auto scope_record = &scope_table->ScopeRecords[entry];
            if (rva >= scope_record->BeginAddress && rva < scope_record->EndAddress)
            {
                // 10. Jump to __except handler
                regs->rip = reinterpret_cast<DWORD64>(winternl::pDriverBase) +
                            scope_record->JumpTarget;
                return;
            }
        }
    }

    // 11. No handler found - inject exception to guest
    if (CPU::bIntelCPU) {
        VTx::Exceptions::InjectException(EXCEPTION_VECTOR_SIMD_FLOATING_POINT_NUMERIC_ERROR,
                                         regs->error_code);
        __vmx_vmwrite(GUEST_RIP, regs->rip);
        __vmx_vmresume();
    }
    else {
        SVM::InjectEvent(vmm::vGuestStates[CPU::GetCPUIndex(true)].SvmState,
                        SVM::e_Exception,
                        InterruptVector::SimdFloatingPointException,
                        0, false);
        // ... restore guest state and continue
    }
}
```

**Port to C**:

This is complex because it relies on PE structures. For UEFI C, you have two options:

**Option 1**: Disable SEH entirely (handlers just crash)
```c
void seh_handler_ecode_vm(IDT_REGS_ECODE* regs)
{
    // No recovery - halt
    while (1) { __halt(); }
}
```

**Option 2**: Implement minimal PE exception walking
```c
typedef struct _PE_EXCEPTION_INFO {
    UINT64 module_base;
    IMAGE_NT_HEADERS64* nt_headers;
    IMAGE_DATA_DIRECTORY* exception_dir;
    RUNTIME_FUNCTION* runtime_functions;
} PE_EXCEPTION_INFO;

BOOLEAN seh_walk_exception_table(
    PE_EXCEPTION_INFO* pe,
    UINT64 fault_rip,
    UINT64* out_handler_rip)
{
    UINT32 rva = (UINT32)(fault_rip - pe->module_base);
    UINT32 num_functions = pe->exception_dir->Size / sizeof(RUNTIME_FUNCTION);

    for (UINT32 i = 0; i < num_functions; i++) {
        RUNTIME_FUNCTION* func = &pe->runtime_functions[i];

        if (rva < func->BeginAddress || rva >= func->EndAddress)
            continue;

        UNWIND_INFO* unwind = (UNWIND_INFO*)(pe->module_base + func->UnwindData);

        if (!(unwind->Flags & UNW_FLAG_EHANDLER))
            continue;

        // Calculate scope table address
        UINT8 code_count_aligned = (unwind->CountOfCodes + 1) & ~1;
        SCOPE_TABLE* scope = (SCOPE_TABLE*)((UINT64)&unwind->UnwindCode[code_count_aligned] + sizeof(UINT32));

        for (UINT32 j = 0; j < scope->Count; j++) {
            SCOPE_RECORD* record = &scope->ScopeRecord[j];

            if (rva >= record->BeginAddress && rva < record->EndAddress) {
                *out_handler_rip = pe->module_base + record->JumpTarget;
                return TRUE;
            }
        }
    }

    return FALSE;
}

void seh_handler_ecode_vm(IDT_REGS_ECODE* regs)
{
    PE_EXCEPTION_INFO pe;
    pe.module_base = g_payload_base;
    pe.nt_headers = (IMAGE_NT_HEADERS64*)(g_payload_base +
                    ((IMAGE_DOS_HEADER*)g_payload_base)->e_lfanew);
    pe.exception_dir = &pe.nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
    pe.runtime_functions = (RUNTIME_FUNCTION*)(g_payload_base + pe.exception_dir->VirtualAddress);

    UINT64 handler_rip;
    if (seh_walk_exception_table(&pe, regs->rip, &handler_rip)) {
        regs->rip = handler_rip;  // Jump to __except block
        return;
    }

    // No handler - crash
    while (1) { __halt(); }
}
```

### AMD Exception Recovery (PayLoad/amd/exception.cpp)

AMD payload has additional recovery: fall back to original Hyper-V handler.

```c
// Global parameter storage for recovery
struct _HYPERV_EXIT_HANDLER_PARAMS {
    void* unknown;
    void* unknown2;
    amd::pguest_context context;
    Seg::DescriptorTableRegister<Seg::Mode::longMode> idt;
    void* rsp;
};

constexpr u32 MAX_SUPPORTED_CORES = 576;
inline static _HYPERV_EXIT_HANDLER_PARAMS CoreParams[MAX_SUPPORTED_CORES] = { 0 };

void exception::SaveOrigParams(
    void* unknown,
    void* unknown2,
    amd::pguest_context context,
    Seg::DescriptorTableRegister<Seg::Mode::longMode> idt,
    void* rsp)
{
    auto core = CPU::ApicId();
    if (core >= MAX_SUPPORTED_CORES) return;

    CoreParams[core].unknown = unknown;
    CoreParams[core].unknown2 = unknown2;
    CoreParams[core].context = context;
    CoreParams[core].idt = idt;
    CoreParams[core].rsp = rsp;
}

void exception::seh_handler_ecode_vm(PIDT_REGS_ECODE regs)
{
    // ... Try SEH exception walking ...

    // If no handler found, recover by calling original Hyper-V handler
    auto core = CPU::ApicId();
    if (core >= MAX_SUPPORTED_CORES) {
        while (1) { __halt(); }  // Can't recover
    }

    // Restore original parameters
    regs->rcx = (UINT64)CoreParams[core].unknown;
    regs->rdx = (UINT64)CoreParams[core].unknown2;
    regs->r8 = (UINT64)CoreParams[core].context;
    regs->rsp = (UINT64)CoreParams[core].rsp;

    // Jump back to original Hyper-V vmexit_handler
    regs->rip = reinterpret_cast<u64>(&amd::vmexit_handler) - amd::ombra_context.vcpu_run_rva;
    __lidt(&CoreParams[core].idt);
}
```

**Port to C**:

```c
// exception.h
typedef struct _IDT_REGISTER {
    UINT16 limit;
    UINT64 base;
} IDT_REGISTER;

typedef struct _HYPERV_RECOVERY_PARAMS {
    void* unknown;
    void* unknown2;
    void* guest_context;
    IDT_REGISTER idt;
    void* rsp;
} HYPERV_RECOVERY_PARAMS;

void exception_save_recovery_params(
    void* unknown,
    void* unknown2,
    void* context,
    IDT_REGISTER* idt,
    void* rsp);

// exception.c
#define MAX_CORES 576
HYPERV_RECOVERY_PARAMS g_recovery_params[MAX_CORES];

void exception_save_recovery_params(
    void* unknown,
    void* unknown2,
    void* context,
    IDT_REGISTER* idt,
    void* rsp)
{
    UINT32 core = cpu_get_apic_id();
    if (core >= MAX_CORES) return;

    g_recovery_params[core].unknown = unknown;
    g_recovery_params[core].unknown2 = unknown2;
    g_recovery_params[core].guest_context = context;
    g_recovery_params[core].idt = *idt;
    g_recovery_params[core].rsp = rsp;
}

void seh_handler_ecode_vm(IDT_REGS_ECODE* regs)
{
    // Try SEH walking first...

    // Fall back to Hyper-V
    UINT32 core = cpu_get_apic_id();
    if (core >= MAX_CORES) {
        while (1) { __halt(); }
    }

    regs->rcx = (UINT64)g_recovery_params[core].unknown;
    regs->rdx = (UINT64)g_recovery_params[core].unknown2;
    regs->r8 = (UINT64)g_recovery_params[core].guest_context;
    regs->rsp = (UINT64)g_recovery_params[core].rsp;

    // Jump to original handler (calculated from ombra_context)
    regs->rip = g_amd_vmexit_handler_addr - g_vcpu_run_rva;

    // Restore original IDT
    __lidt(&g_recovery_params[core].idt);
}
```

## Assembly Components

### Guest Exception Handlers (OmbraCoreLib/IDT-helper.asm)

**Minimal stubs - no register preservation, no C handler call**:

```asm
.code _text

EXTERN seh_handler : proc
EXTERN seh_handler_ecode : proc

; #DE has no error code...
generic_interrupt_handler PROC
__de_handler proc
    iretq
__de_handler endp
generic_interrupt_handler ENDP

; PF and GP have error code...
generic_interrupt_handler_ecode PROC
__pf_handler proc
__gp_handler proc
    add rsp, 8      ; remove error code on the stack...
    iretq
__gp_handler endp
__pf_handler endp
generic_interrupt_handler_ecode ENDP

; Debug handler - clear TF (Trap Flag)
__db_handler proc
    push rax
    pushfq
    pop rax
    btr rax, 8      ; Clear bit 8 (TF)
    push rax
    popfq
    pop rax
    iretq
__db_handler endp

END
```

### VMXRoot Exception Handlers (OmbraCoreLib-v/VmIDT-helper.asm)

**Full register preservation + SEH handler call**:

```asm
.code _text

EXTERN seh_handler_vm : proc
EXTERN seh_handler_ecode_vm : proc
EXTERN NmiHandler : proc

; Macro for NMI-blocking iret
nmiret macro
    push rax
    mov rax, rsp
    lss rsp, [rax+8*4]          ; Restore SS:RSP
    push qword ptr [rax+18h]    ; RFLAGS
    popfq
    push qword ptr [rax+10h]    ; CS
    push qword ptr [rax+8h]     ; RIP
    mov rax, qword ptr [rax]
    retfq
endm

; NMI handler with blocking iret
__nmi_handler_vm PROC
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    sub rsp, 20h
    call NmiHandler
    add rsp, 20h

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    nmiret
__nmi_handler_vm ENDP

; #DE handler (no error code)
generic_interrupt_handler_vm PROC
__de_handler_vm proc
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rcx, rsp        ; Pass IDT_REGS* as first parameter
    sub rsp, 20h        ; Shadow space for call
    call seh_handler_vm
    add rsp, 20h

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    iretq
__de_handler_vm endp
generic_interrupt_handler_vm ENDP

; #PF and #GP handlers (with error code)
generic_interrupt_handler_ecode_vm PROC
__pf_handler_vm proc
__gp_handler_vm proc
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rcx, rsp                ; Pass IDT_REGS_ECODE* as first parameter
    sub rsp, 20h
    call seh_handler_ecode_vm
    add rsp, 20h

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    add rsp, 8          ; Remove error code

    iretq
__gp_handler_vm endp
__pf_handler_vm endp
generic_interrupt_handler_ecode_vm ENDP

END
```

## C Conversion Notes

### Exception Handler Registration

**C++ Pattern** (OmbraCoreLib-v):
```cpp
void SetupIDTVm(IDT& idt)
{
    idt.setup(generic_interrupt_handler_vm, generic_interrupt_handler_ecode_vm);
}
```

**C Pattern**:
```c
// exception.h
typedef struct _IDT_TABLE {
    IDT_GATE_DESCRIPTOR_64 entries[256];
} IDT_TABLE;

void exception_setup_vmxroot_idt(IDT_TABLE* idt);
void exception_load_vmxroot_idt(IDT_TABLE* idt);

// exception.c
void exception_setup_vmxroot_idt(IDT_TABLE* idt)
{
    UINT16 cs = get_cs_selector();

    // Zero table
    for (int i = 0; i < 256; i++) {
        idt->entries[i].values[0] = 0;
        idt->entries[i].values[1] = 0;
        idt->entries[i].values[2] = 0;
        idt->entries[i].values[3] = 0;
    }

    // Exceptions without error code
    idt_setup_entry(idt, 0, generic_interrupt_handler_vm, cs, 0xE, 1);  // #DE
    idt_setup_entry(idt, 1, generic_interrupt_handler_vm, cs, 0xE, 1);  // #DB
    idt_setup_entry(idt, 2, __nmi_handler_vm, cs, 0xE, 1);              // NMI
    idt_setup_entry(idt, 3, generic_interrupt_handler_vm, cs, 0xE, 1);  // #BP
    // ... continue for all vectors without error code

    // Exceptions with error code
    idt_setup_entry(idt, 8, generic_interrupt_handler_ecode_vm, cs, 0xE, 1);   // #DF
    idt_setup_entry(idt, 10, generic_interrupt_handler_ecode_vm, cs, 0xE, 1);  // #TS
    idt_setup_entry(idt, 11, generic_interrupt_handler_ecode_vm, cs, 0xE, 1);  // #NP
    idt_setup_entry(idt, 12, generic_interrupt_handler_ecode_vm, cs, 0xE, 1);  // #SS
    idt_setup_entry(idt, 13, generic_interrupt_handler_ecode_vm, cs, 0xE, 1);  // #GP
    idt_setup_entry(idt, 14, generic_interrupt_handler_ecode_vm, cs, 0xE, 1);  // #PF
    idt_setup_entry(idt, 17, generic_interrupt_handler_ecode_vm, cs, 0xE, 1);  // #AC
}

void exception_load_vmxroot_idt(IDT_TABLE* idt)
{
    IDT_REGISTER idtr;
    idtr.limit = sizeof(IDT_TABLE) - 1;
    idtr.base = (UINT64)idt;
    __lidt(&idtr);
}
```

### Interrupt Gate Setup

**Key Fields**:
```c
void idt_setup_entry(
    IDT_TABLE* idt,
    UINT32 vector,
    void* handler,
    UINT16 cs_selector,
    UINT8 type,
    UINT8 present)
{
    UINT64 addr = (UINT64)handler;

    // Split handler address across 3 fields
    idt->entries[vector].bits.offset_0_15 = addr & 0xFFFF;
    idt->entries[vector].bits.offset_16_31 = (addr >> 16) & 0xFFFF;
    idt->entries[vector].bits.offset_32_63 = addr >> 32;

    // Gate configuration
    idt->entries[vector].bits.cs_selector = cs_selector;
    idt->entries[vector].bits.type = type;          // 0xE = Interrupt Gate
    idt->entries[vector].bits.present = present;    // 1 = valid
    idt->entries[vector].bits.dpl = 0;              // Ring 0 only
    idt->entries[vector].bits.ist = 0;              // No stack switch
    idt->entries[vector].bits.must_be_zero1 = 0;
    idt->entries[vector].bits.reserved = 0;
}
```

### Stack Frame Handling

**Assembly prologue** (push all registers):
```asm
handler_with_ecode proc
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; At this point, RSP points to r15 (lowest address)
    ; Stack layout matches IDT_REGS_ECODE structure

    mov rcx, rsp        ; Pass struct pointer
    sub rsp, 20h        ; Shadow space
    call seh_handler_ecode_vm
    add rsp, 20h

    pop r15
    pop r14
    ; ... pop all in reverse order
    pop rax
    add rsp, 8          ; Remove error code
    iretq
handler_with_ecode endp
```

**C handler** receives pointer to stack frame:
```c
void seh_handler_ecode_vm(IDT_REGS_ECODE* regs)
{
    // regs->rax, regs->rbx, etc. are all populated
    // regs->error_code contains the error code
    // regs->rip contains faulting instruction pointer

    // Modify RIP to skip instruction or jump to handler
    regs->rip += instruction_length;
}
```

## Testing Checklist

### Guest IDT Tests

- [ ] **Basic Exception Triggering**
  - Divide by zero (#DE)
  - Invalid opcode (#UD)
  - General protection fault (#GP)
  - Page fault (#PF)

- [ ] **Error Code Validation**
  - #PF error code matches CR2
  - #GP error code contains selector
  - Error code correctly removed from stack

- [ ] **Stack Frame Integrity**
  - All registers preserved correctly
  - Return address (RIP) correct
  - Stack pointer (RSP) correct after iretq

### VMXRoot IDT Tests

- [ ] **SEH Exception Walking**
  - Exception in __try block jumps to __except
  - Exception outside __try crashes (expected)
  - Nested __try blocks work correctly

- [ ] **NMI Handling**
  - NMI handler called correctly
  - NMIs remain blocked during handler
  - nmiret correctly returns without unblocking

- [ ] **Register Preservation**
  - All 15 GPRs preserved across exception
  - RFLAGS preserved
  - CS/SS selectors preserved

- [ ] **Guest Re-injection (Intel)**
  - Unhandled exception injected to guest
  - GUEST_RIP set correctly
  - vmresume succeeds

- [ ] **Guest Re-injection (AMD)**
  - EVENTINJ set correctly
  - NextRIP set correctly
  - Guest continues after injection

### VMCALL Tests

- [ ] **Authentication**
  - Unauthenticated call returns INVALID_GUEST_PARAM
  - VMCALL_SET_COMM_KEY sets key correctly
  - Authenticated calls succeed

- [ ] **Memory Operations**
  - VMCALL_READ_VIRT reads correctly (same process)
  - VMCALL_READ_VIRT reads correctly (cross-process with R8=cr3)
  - VMCALL_WRITE_VIRT writes correctly
  - VMCALL_READ_PHY reads physical memory
  - VMCALL_WRITE_PHY writes physical memory

- [ ] **Address Translation**
  - VMCALL_VIRT_TO_PHY returns correct PA
  - Translation fails correctly for unmapped VA
  - Uses R8=cr3 when specified

- [ ] **Storage Slots**
  - VMCALL_STORAGE_QUERY reads correct value
  - VMCALL_STORAGE_QUERY writes value
  - Per-core isolation works (different cores see different values)

- [ ] **EPT/NPT Control**
  - VMCALL_GET_EPT_BASE returns current base
  - VMCALL_SET_EPT_BASE switches table
  - VMCALL_DISABLE_EPT disables nested paging
  - VMCALL_ENABLE_EPT re-enables nested paging

- [ ] **ETW Artifact Elimination**
  - VMCALL_DISABLE_ETW_TI zeros ProviderEnableInfo
  - VMCALL_ENABLE_ETW_TI restores original value
  - VMCALL_WIPE_ETW_BUFFERS zeros matching events
  - Events outside timestamp range untouched

### Error Handling

- [ ] **Invalid Parameters**
  - NULL pointers return VMXROOT_TRANSLATE_FAILURE
  - Out-of-range slot IDs return INVALID_GUEST_PARAM
  - Unmapped guest VAs fail gracefully

- [ ] **Page Table Walk Failures**
  - Returns PML4E_NOT_PRESENT when appropriate
  - Returns PDPTE_NOT_PRESENT when appropriate
  - Returns PDE_NOT_PRESENT when appropriate
  - Returns PTE_NOT_PRESENT when appropriate

---

## Summary

This document covered:

1. **IDT Structure** - 16-byte gate descriptors, 256-entry table
2. **Guest IDT** - Simple handlers, minimal SEH support
3. **VMXRoot IDT** - Full register save, SEH walking, guest re-injection, Hyper-V fallback
4. **NMI Windowing** - Emulated iretq to prevent NMI unblocking
5. **VMCALL Interface** - 14 implemented commands, authentication, dispatch flow
6. **Exception Recovery** - SEH via PE exception directory, AMD fallback to Hyper-V
7. **C Conversion Patterns** - Struct-based IDT, function pointers, manual stack frames

**Critical for UEFI Port**:
- IDT table must be 16-byte aligned
- CS selector must be valid code segment
- NMI handlers need `nmiret` macro
- VMCALL authentication required for all commands except SET_COMM_KEY
- Storage slots are per-core (576 cores x 128 slots)
- SEH requires PE exception directory parsing (optional in UEFI)

**Files to Port**:
1. IDT setup: `OmbraCoreLib/IDT.cpp` → `exception.c`
2. IDT handlers: `OmbraCoreLib/IDT-helper.asm` → `exception_handlers.asm`
3. VMCALL dispatch: `PayLoad/core/dispatch.cpp` → `vmcall_dispatch.c`
4. Storage system: `PayLoad/include/storage.h` → `storage.c`
