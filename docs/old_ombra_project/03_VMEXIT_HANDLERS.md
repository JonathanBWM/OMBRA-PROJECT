# VMX/SVM VMEXIT HANDLERS - C++ to C + Assembly Port Guide

## Overview

This document provides a comprehensive analysis of the VMExit handler architecture in the Ombra Hypervisor, covering both Intel VMX and AMD SVM implementations. The handlers intercept hardware virtualization exits and dispatch them to appropriate processing logic.

**Architecture**: The current implementation uses a unified dispatch layer (`PayLoad/core/dispatch.cpp`) shared between Intel and AMD backends. Architecture-specific operations are provided via callback function pointers registered during initialization.

**Key Components**:
- Intel VMX handler: `PayLoad/intel/vmx_handler.cpp`
- AMD SVM handler: `PayLoad/amd/svm_handler.cpp`
- Unified dispatch: `PayLoad/core/dispatch.cpp`
- Assembly stubs: `OmbraCoreLib/OmbraCoreLib-v/src/VTx-helper.asm` (Intel), `SVM-support.asm` (AMD)
- Legacy handlers: `OmbraCoreLib/OmbraCoreLib-v/src/VMExitHandlers.cpp` (kernel-mode only)

---

## File Inventory

### Current PayLoad Implementation (Ring -1 VMXRoot Context)

| File | Lines | Purpose |
|------|-------|---------|
| `PayLoad/intel/vmx_handler.h` | 153 | Intel VMX backend interface |
| `PayLoad/intel/vmx_handler.cpp` | 480 | Intel VMX VMExit handler implementation |
| `PayLoad/amd/svm_handler.h` | 143 | AMD SVM backend interface |
| `PayLoad/amd/svm_handler.cpp` | 460 | AMD SVM VMExit handler implementation |
| `PayLoad/core/dispatch.h` | 108 | Unified hypercall dispatch interface |
| `PayLoad/core/dispatch.cpp` | 803 | Unified hypercall command handlers |
| `OmbraCoreLib/.../VTx-helper.asm` | 318 | Intel VMX assembly stubs |
| `OmbraCoreLib/.../SVM-support.asm` | 309 | AMD SVM assembly stubs |

### Legacy OmbraCoreLib Implementation (Kernel Driver Context)

| File | Lines | Purpose |
|------|-------|---------|
| `OmbraCoreLib/.../include/Vmexit.h` | 38 | Dynamic handler registration API |
| `OmbraCoreLib/.../src/Vmexit.cpp` | 88 | Handler table management |
| `OmbraCoreLib/.../include/VMExitHandlers.h` | 20 | Handler function declarations |
| `OmbraCoreLib/.../src/VMExitHandlers.cpp` | 597 | Handler implementations (CPUID, CR, MSR, etc.) |

**Note**: The legacy kernel-mode handlers are NOT used by the PayLoad. They represent an earlier driver-based approach where the hypervisor ran in kernel mode with full Windows kernel APIs available.

---

## Architecture Summary

### Execution Flow (Intel VMX)

```
1. CPU executes VMLAUNCH/VMRESUME (VmxSaveAndLaunch in VTx-helper.asm)
2. Guest executes instruction causing VMExit (e.g., CPUID, CR access, EPT violation)
3. CPU transfers control to VMExit handler entry point
4. VmExitWrapper (VTx-helper.asm:45-168):
   - Allocates stack frame with KTRAP_FRAME
   - Saves all GPRs (RAX-R15) + XMM0-XMM15
   - Calls VmExitHandler (C++ function)
5. intel::vmexit_handler (vmx_handler.cpp:352):
   - Captures TSC for timing compensation
   - Reads VMCS_EXIT_REASON
   - Dispatches based on reason code
6. Handler returns, VmExitWrapper restores state
7. VMRESUME returns to guest
```

### Execution Flow (AMD SVM)

```
1. CPU executes VMRUN (svm_enter_guest in SVM-support.asm)
2. Guest executes instruction causing #VMEXIT (e.g., CPUID, NPF, CR write)
3. CPU transfers control to VMEXIT handler entry point
4. amd::vmexit_handler (svm_handler.cpp:320):
   - Captures TSC for timing compensation
   - Gets VMCB pointer via GS segment (get_vmcb())
   - Reads VMCB->ControlArea.ExitCode
   - Dispatches via switch statement
5. Handler returns pgs_base_struct* (GS base address)
6. SVM assembly stub (svm_enter_guest) restores host state
7. VMRUN returns to guest
```

### Key Architectural Differences

| Aspect | Intel VMX | AMD SVM |
|--------|-----------|---------|
| **Control Structure** | VMCS (Virtual Machine Control Structure) | VMCB (Virtual Machine Control Block) |
| **Handler Signature** | `void vmexit_handler(pcontext_t* context, void* unknown)` | `pgs_base_struct vmexit_handler(void*, void*, pguest_context)` |
| **Exit Code Location** | VMCS field 0x4402 (VMCS_EXIT_REASON) | `vmcb->ControlArea.ExitCode` |
| **Guest State Access** | `__vmx_vmread(VMCS_GUEST_CR3, &cr3)` | `vmcb->Cr3()`, `vmcb->StateSaveArea.Rip` |
| **RIP Advancement** | Manual: read `VMCS_VMEXIT_INSTRUCTION_LENGTH`, add to RIP | Automatic: `vmcb->ControlArea.NextRip` |
| **Nested Paging** | EPT (Extended Page Tables) | NPT (Nested Page Tables) |
| **Paging Base** | EPTP pointer (VMCS 0x201A) | `vmcb->NestedPageTableCr3()` |
| **TLB Invalidation** | INVEPT instruction | `vmcb->ControlArea.TlbControl = 0x1` |
| **Exception Injection** | VMCS field VMENTRY_INTERRUPTION_INFORMATION_FIELD | `vmcb->ControlArea.EventInjection` |

---

## Intel VMX Handler

### VMExit Reason Table

| Exit Reason | Code | Handler Function | Handled? |
|-------------|------|------------------|----------|
| **EXECUTE_CPUID** | 10 | `HandleCpuid()` | ✅ YES |
| **RDMSR** | 31 | Inline in main handler | ✅ YES (APERF/MPERF only) |
| **EPT_VIOLATION** | 48 | Calls registered EPT handler | ✅ YES |
| **VMREAD** | 23 | Injects #UD | ✅ YES (anti-detection) |
| **VMWRITE** | 24 | Injects #UD | ✅ YES (anti-detection) |
| **VMCALL** | 18 | Injects #UD | ✅ YES (anti-detection) |
| **VMXON** | 27 | Injects #UD | ✅ YES (anti-detection) |
| **VMXOFF** | 26 | Injects #UD | ✅ YES (anti-detection) |
| **VMPTRLD** | 21 | Injects #UD | ✅ YES (anti-detection) |
| **VMPTRST** | 22 | Injects #UD | ✅ YES (anti-detection) |
| **VMCLEAR** | 20 | Injects #UD | ✅ YES (anti-detection) |
| **VMLAUNCH** | 19 | Injects #UD | ✅ YES (anti-detection) |
| **VMRESUME** | 25 | Injects #UD | ✅ YES (anti-detection) |
| All others | N/A | Chains to original Hyper-V handler | ❌ NO |

### Key VMCS Fields

```c
// Exit information
#define VMCS_EXIT_REASON                              0x4402
#define VMCS_VMEXIT_INSTRUCTION_LENGTH                0x440C
#define VMCS_EXIT_QUALIFICATION                       0x6400

// Guest state
#define VMCS_GUEST_RIP                                0x681E
#define VMCS_GUEST_RSP                                0x681C
#define VMCS_GUEST_CR3                                0x6802
#define VMCS_GUEST_CR0                                0x6800
#define VMCS_GUEST_CR4                                0x6804
#define VMCS_GUEST_RFLAGS                             0x6820

// EPT control
#define VMCS_CTRL_EPT_POINTER                         0x201A
#define VMCS_GUEST_PHYSICAL_ADDRESS                   0x2400
#define VMCS_CTRL_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS  0x401E

// Exception injection
#define VMCS_FIELD_VMENTRY_INTERRUPTION_INFORMATION_FIELD  (see Vmx.h)

// Shadow registers
#define VMCS_CTRL_CR0_READ_SHADOW                     0x6004
#define VMCS_CTRL_CR4_READ_SHADOW                     0x6006
```

### Handler Functions

#### Main Entry Point

```cpp
// vmx_handler.cpp:352
void vmexit_handler(pcontext_t* context, void* unknown)
{
    //=========================================================================
    // TIMING: Capture entry TSC immediately (must be first instruction)
    // Critical for defeating BattlEye's RDTSC timing checks
    //=========================================================================
    timing::OnExitEntry();

    // Perform one-time setup on first exit
    RootSetup();

    pcontext_t guest_registers = *context;

    // Read VMExit reason from VMCS
    size_t vmexit_reason;
    __vmx_vmread(VMCS_EXIT_REASON, &vmexit_reason);

    bool bHandledExit = false;
    bool bIncRip = false;

    // Handle CPUID exits - our hypercall mechanism
    if (vmexit_reason == VMX_EXIT_REASON_EXECUTE_CPUID)
    {
        bHandledExit = HandleCpuid(guest_registers);
        bIncRip = bHandledExit;
    }

    // Handle RDMSR - Virtualize APERF/MPERF for Anti-Timing Detection
    if (vmexit_reason == static_cast<size_t>(Vmx::VmxExitReason::EXIT_REASON_RDMSR))
    {
        u32 msr_id = static_cast<u32>(guest_registers->rcx);

        // Check for performance monitoring MSRs
        if (msr_id == 0xE7 || msr_id == 0xE8) {
            // IA32_MPERF (0xE7) or IA32_APERF (0xE8)
            u64 fake_value = timing::ReadMsrVirtualized(msr_id);
            guest_registers->rax = fake_value & 0xFFFFFFFF;
            guest_registers->rdx = fake_value >> 32;
            bHandledExit = true;
            bIncRip = true;
        }
    }

    // Handle VMX Instructions - Inject #UD for Anti-Detection
    if (vmexit_reason == EXIT_REASON_VMREAD ||
        vmexit_reason == EXIT_REASON_VMWRITE ||
        vmexit_reason == EXIT_REASON_VMCALL ||
        vmexit_reason == EXIT_REASON_VMXON ||
        vmexit_reason == EXIT_REASON_VMXOFF ||
        /* ... other VMX instructions ... */)
    {
        // Inject #UD (exception vector 6)
        Vmx::VmentryInterruptionInformation info = {};
        info.fields.VectorOfInterruptOrException = 6;  // #UD
        info.fields.InterruptionType = Vmx::InterruptionType::HardwareException;
        info.fields.DeliverErrorCode = 0;
        info.fields.Valid = 1;

        __vmx_vmwrite(VMCS_FIELD_VMENTRY_INTERRUPTION_INFORMATION_FIELD, info.val);
        bHandledExit = true;
        bIncRip = false;  // Exception uses faulting RIP
    }

    // Handle EPT Violations
    if (vmexit_reason == VMX_EXIT_REASON_EPT_VIOLATION)
    {
        u64 ept_handler_addr = storage::Query(EPT_HANDLER_ADDRESS);
        if (ept_handler_addr && bitmap::GetBit(&storage_arr[EPT_OS_INIT_BITMAP], CPU::ApicId()))
        {
            size_t fault_pa;
            __vmx_vmread(VMCS_GUEST_PHYSICAL_ADDRESS, &fault_pa);

            auto pEptHandler = (fnEptHandler)ept_handler_addr;
            pEptHandler(fault_pa);
            bHandledExit = true;
        }
    }

    // If we handled the exit, advance RIP and return
    if (bHandledExit)
    {
        if (bIncRip)
        {
            size_t rip, exec_len;
            __vmx_vmread(VMCS_GUEST_RIP, &rip);
            __vmx_vmread(VMCS_VMEXIT_INSTRUCTION_LENGTH, &exec_len);
            __vmx_vmwrite(VMCS_GUEST_RIP, rip + exec_len);
        }

        // TIMING: Adjust TSC offset before VMResume
        timing::OnExitComplete(nullptr);
        return;
    }

    // For unhandled exits, call the original Hyper-V handler
    reinterpret_cast<vmexit_handler_t>(
        reinterpret_cast<u64>(&vmexit_handler) -
        ombra_context.vmexit_handler_rva)(context, unknown);
}
```

#### CPUID Handler (Hypercall Dispatch)

```cpp
// vmx_handler.cpp:292
static bool HandleCpuid(pcontext_t guest_registers)
{
    u32 leaf = static_cast<u32>(guest_registers->rax);
    u32 subleaf = static_cast<u32>(guest_registers->rcx);

    // Check if this is our magic hypercall leaf
    if (leaf == cpuid_spoof::HYPERCALL_MAGIC_LEAF)
    {
        // Build VmExitContext from Intel-specific state
        ombra::VmExitContext ctx;
        ctx.Reset();

        // Populate guest state
        ctx.guest_cr3 = vmexit::get_guest_cr3();
        __vmx_vmread(VMCS_GUEST_RIP, (size_t*)&ctx.guest_rip);
        ctx.p_rax = &guest_registers->rax;

        // Populate hypercall parameters from guest registers
        ctx.cmd_type = guest_registers->rcx;      // RCX = VMCALL_TYPE
        ctx.cmd_guest_va = guest_registers->rdx;  // RDX = &COMMAND_DATA
        ctx.extra_param = guest_registers->r8;    // R8 = target CR3
        ctx.auth_key = guest_registers->r9;       // R9 = authentication key

        // Store arch-specific data for callbacks
        ctx.arch_data = nullptr;  // Intel doesn't need VMCB
        ctx.arch_data2 = guest_registers;

        // Dispatch to unified handler
        VMX_ROOT_ERROR result = core::HandleVmcall(&ctx);

        // Check if valid authenticated vmcall
        if (result != VMX_ROOT_ERROR::INVALID_GUEST_PARAM || vmcall::IsVmcall(ctx.auth_key)) {
            guest_registers->rax = static_cast<u64>(result);
            bCpuidVmcallCalled = true;
            return true;
        }
    }

    // Handle ALL CPUID leaves with spoofing
    u32 eax, ebx, ecx, edx;
    cpuid_spoof::ExecuteAndSpoof(leaf, subleaf, &eax, &ebx, &ecx, &edx);

    // Return spoofed values to guest
    guest_registers->rax = static_cast<u64>(eax);
    guest_registers->rbx = static_cast<u64>(ebx);
    guest_registers->rcx = static_cast<u64>(ecx);
    guest_registers->rdx = static_cast<u64>(edx);

    return true;  // We handle ALL CPUID
}
```

#### Intel Architecture Callbacks

```cpp
// vmx_handler.cpp:109-186
static u64 IntelGetGuestCr3(void* /* arch_data */) {
    u64 guest_dirbase;
    __vmx_vmread(VMCS_GUEST_CR3, &guest_dirbase);
    return cr3{ guest_dirbase }.pml4_pfn << 12;  // Strip PCID bits
}

static u64 IntelGetHostCr3(void* /* arch_data */) {
    return __readcr3();
}

static u64 IntelGetEptBase(void* /* arch_data */) {
    size_t eptp;
    __vmx_vmread(VMCS_CTRL_EPT_POINTER, &eptp);
    return eptp;
}

static VMX_ROOT_ERROR IntelSetEptBase(void* /* arch_data */, u64 new_base) {
    __vmx_vmwrite(VMCS_CTRL_EPT_POINTER, new_base);

    // Mark this core as having custom EPT
    bitmap::SetBit(&storage_arr[EPT_OS_INIT_BITMAP], CPU::ApicId(), true);

    return VMX_ROOT_ERROR::SUCCESS;
}

static VMX_ROOT_ERROR IntelEnableEpt(void* /* arch_data */) {
    size_t secondary_controls;
    __vmx_vmread(VMCS_CTRL_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, &secondary_controls);
    secondary_controls |= SECONDARY_EXEC_ENABLE_EPT;
    __vmx_vmwrite(VMCS_CTRL_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, secondary_controls);
    return VMX_ROOT_ERROR::SUCCESS;
}

static VMX_ROOT_ERROR IntelDisableEpt(void* /* arch_data */) {
    size_t secondary_controls;
    __vmx_vmread(VMCS_CTRL_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, &secondary_controls);
    secondary_controls &= ~SECONDARY_EXEC_ENABLE_EPT;
    __vmx_vmwrite(VMCS_CTRL_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, secondary_controls);
    return VMX_ROOT_ERROR::SUCCESS;
}

static void RegisterIntelCallbacks() {
    ombra::ArchCallbacks callbacks = {};
    callbacks.GetGuestCr3 = IntelGetGuestCr3;
    callbacks.GetHostCr3 = IntelGetHostCr3;
    callbacks.GetEptBase = IntelGetEptBase;
    callbacks.SetEptBase = IntelSetEptBase;
    callbacks.EnableEpt = IntelEnableEpt;
    callbacks.DisableEpt = IntelDisableEpt;
    callbacks.GetVmcb = IntelGetVmcb;  // Returns 0 (AMD-only concept)
    callbacks.FlushEptTlb = IntelFlushEptTlb;
    callbacks.SetEptInitBit = IntelSetEptInitBit;
    callbacks.GetEptInitBit = IntelGetEptInitBit;

    core::RegisterArchCallbacks(callbacks);
}
```

---

## AMD SVM Handler

### VMExit Code Table

| Exit Code | Svm::SvmExitCode | Handler Function | Handled? |
|-----------|------------------|------------------|----------|
| **VMEXIT_CPUID** | 0x72 | `HandleCpuid()` | ✅ YES |
| **VMEXIT_NPF** | 0x400 | Calls registered EPT handler | ✅ YES |
| **VMEXIT_MSR** | 0x7C | Inline in main handler | ✅ YES (APERF/MPERF only) |
| **VMEXIT_VMRUN** | 0x80 | Injects #UD | ✅ YES (anti-detection) |
| **VMEXIT_VMMCALL** | 0x81 | Injects #UD | ✅ YES (anti-detection) |
| **VMEXIT_VMLOAD** | 0x82 | Injects #UD | ✅ YES (anti-detection) |
| **VMEXIT_VMSAVE** | 0x83 | Injects #UD | ✅ YES (anti-detection) |
| **VMEXIT_CLGI** | 0x86 | Injects #UD | ✅ YES (anti-detection) |
| **VMEXIT_STGI** | 0x87 | Injects #UD | ✅ YES (anti-detection) |
| **VMEXIT_SKINIT** | 0x88 | Injects #UD | ✅ YES (anti-detection) |
| All others | N/A | Chains to original Hyper-V handler | ❌ NO |

### Key VMCB Structures

```cpp
// VMCB pointer access (GS-based)
__forceinline auto get_vmcb() -> Svm::Vmcb*
{
    return *reinterpret_cast<Svm::Vmcb**>(
        *reinterpret_cast<u64*>(
            *reinterpret_cast<u64*>(
                __readgsqword(0) + ombra_context.vmcb_base)
            + ombra_context.vmcb_link) + ombra_context.vmcb_off);
}

// Key VMCB fields:
vmcb->ControlArea.ExitCode           // Exit reason code
vmcb->ControlArea.ExitInfo1          // Exit-specific info (e.g., CR number)
vmcb->ControlArea.ExitInfo2          // Exit-specific info (e.g., fault PA)
vmcb->ControlArea.NextRip            // RIP after current instruction
vmcb->ControlArea.TlbControl         // TLB invalidation control
vmcb->ControlArea.NpEnable           // Nested paging enable bit
vmcb->ControlArea.EventInjection     // Exception injection field
vmcb->ControlArea.InterceptCr        // CR access intercept bitmap

vmcb->StateSaveArea.Rip              // Guest RIP
vmcb->StateSaveArea.Rsp              // Guest RSP
vmcb->StateSaveArea.Cr0              // Guest CR0
vmcb->StateSaveArea.Cr3              // Guest CR3
vmcb->StateSaveArea.Cr4              // Guest CR4

vmcb->Rax()                          // Guest RAX (convenience accessor)
vmcb->Cr3()                          // Guest CR3 (convenience accessor)
vmcb->Rsp()                          // Guest RSP (convenience accessor)
vmcb->NestedPageTableCr3()           // NPT base pointer
```

### Handler Functions

#### Main Entry Point

```cpp
// svm_handler.cpp:320
auto vmexit_handler(void* unknown, void* unknown2, pguest_context context) -> pgs_base_struct
{
    //=========================================================================
    // TIMING: Capture entry TSC immediately
    //=========================================================================
    timing::OnExitEntry();

    // Perform one-time setup on first exit
    RootSetup();

    // Save original IDT and install our exception handlers
    Seg::DescriptorTableRegister<Seg::Mode::longMode> origIdt = { 0 };
    __sidt(&origIdt);

    exception::SaveOrigParams(unknown, unknown2, context, origIdt, _AddressOfReturnAddress());
    __lidt(&exception::IdtReg);

    // Get VMCB for this VCPU
    const auto vmcb = get_vmcb();

    bool bIncRip = false;
    bool bHandledExit = false;

    // Clear TLB control (default: do nothing)
    vmcb->ControlArea.TlbControl.layout.TlbControl = 0;

    // Once we've received a vmcall, disable CR intercepts for performance
    if (bCpuidVmcallCalled)
    {
        vmcb->ControlArea.InterceptCr.rw.write.layout.WriteCr4 = false;
        vmcb->ControlArea.InterceptCr.rw.write.layout.WriteCr0 = false;
        vmcb->ControlArea.InterceptCr0WritesOther = false;
    }

    // Handle VMExit based on exit code
    switch (vmcb->ControlArea.ExitCode)
    {
    case Svm::SvmExitCode::VMEXIT_CPUID:
    {
        bHandledExit = HandleCpuid(vmcb, context);
        bIncRip = true;
        break;
    }

    case Svm::SvmExitCode::VMEXIT_NPF:
    {
        // Nested Page Fault
        u64 eptHandler = storage::Query(EPT_HANDLER_ADDRESS);
        if (eptHandler && bitmap::GetBit(&storageArr[EPT_OS_INIT_BITMAP], CPU::ApicId()))
        {
            auto pEptHandler = (fnEptHandler)eptHandler;
            pEptHandler(vmcb->ControlArea.ExitInfo2);  // Fault PA
            bHandledExit = true;
        }
        break;
    }

    case Svm::SvmExitCode::VMEXIT_MSR:
    {
        bool is_write = vmcb->ControlArea.ExitInfo1 & 1;
        u32 msr_id = static_cast<u32>(context->rcx);

        if (!is_write && (msr_id == 0xE7 || msr_id == 0xE8)) {
            u64 fake_value = timing::ReadMsrVirtualized(msr_id);
            vmcb->Rax() = fake_value & 0xFFFFFFFF;
            context->rdx = fake_value >> 32;
            bHandledExit = true;
            bIncRip = true;
        }
        break;
    }

    // Handle SVM Instructions - Inject #UD
    case Svm::SvmExitCode::VMEXIT_VMRUN:
    case Svm::SvmExitCode::VMEXIT_VMMCALL:
    case Svm::SvmExitCode::VMEXIT_VMLOAD:
    case Svm::SvmExitCode::VMEXIT_VMSAVE:
    case Svm::SvmExitCode::VMEXIT_CLGI:
    case Svm::SvmExitCode::VMEXIT_STGI:
    case Svm::SvmExitCode::VMEXIT_SKINIT:
    {
        // Inject #UD exception
        Svm::EventInj inj = {};
        inj.layout.Vector = 6;            // #UD
        inj.layout.Type = 3;              // Exception
        inj.layout.ErrorCodeValid = 0;
        inj.layout.Valid = 1;
        vmcb->ControlArea.EventInjection = inj.raw;

        bHandledExit = true;
        bIncRip = false;
        break;
    }

    default:
        break;
    }

    // If we didn't handle the exit, pass to original Hyper-V handler
    if (!bHandledExit)
    {
        __lidt(&origIdt);
        return reinterpret_cast<vcpu_run_t>(
            reinterpret_cast<u64>(&vmexit_handler) -
            ombra_context.vcpu_run_rva)(unknown, unknown2, context);
    }

    // Advance RIP if needed (AMD provides NextRip automatically)
    if (bIncRip)
    {
        vmcb->StateSaveArea.Rip = vmcb->ControlArea.NextRip;
    }

    // TIMING: Adjust TSC offset before VMRUN
    timing::OnExitComplete(vmcb);

    // Restore original IDT and return success
    __lidt(&origIdt);
    return reinterpret_cast<pgs_base_struct>(__readgsqword(0));
}
```

#### CR Write Handlers (AMD-Specific)

```cpp
// svm_handler.cpp:110-140
static UINT64* GetRegisterForCrExit(Svm::Vmcb* vmcb, pguest_context context)
{
    // ExitInfo1[3:0] contains GPR number
    UINT64 gpr_num = vmcb->ControlArea.ExitInfo1 & 15;

    switch (gpr_num)
    {
    case 0:  return &vmcb->Rax();
    case 1:  return &context->rcx;
    case 2:  return &context->rdx;
    case 3:  return &context->rbx;
    case 4:  return &vmcb->Rsp();
    case 5:  return &context->rbp;
    case 6:  return &context->rsi;
    case 7:  return &context->rdi;
    case 8:  return &context->r8;
    case 9:  return &context->r9;
    case 10: return &context->r10;
    case 11: return &context->r11;
    case 12: return &context->r12;
    case 13: return &context->r13;
    case 14: return &context->r14;
    case 15: return &context->r15;
    default: return 0;
    }
}

static bool HandleCr4Write(Svm::Vmcb* vmcb, pguest_context context)
{
    UINT64* reg = GetRegisterForCrExit(vmcb, context);
    CR4 cr4;
    cr4.Flags = *reg;

    // GP fault checks
    if (cr4.PcidEnable == 1)
    {
        CR3 cr3;
        cr3.Flags = vmcb->Cr3();
        if (cr3.Flags & 0xFFF)  // CR3[11:0] must be 0 when enabling PCID
            return false;
    }

    if (cr4.Reserved1 || cr4.Reserved2 || cr4.Reserved3 || cr4.Reserved4)
        return false;

    vmcb->StateSaveArea.Cr4 = cr4.Flags;
    return true;
}

static bool HandleCr0Write(Svm::Vmcb* vmcb, pguest_context context)
{
    UINT64* reg = GetRegisterForCrExit(vmcb, context);
    CR0 cr0;
    cr0.Flags = *reg;
    CR4 cr4;
    cr4.Flags = vmcb->StateSaveArea.Cr4;

    if (cr0.Reserved1 || cr0.Reserved2 || cr0.Reserved3 || cr0.Reserved4)
        return false;

    // Disable CET if enabled (compatibility)
    if (cr4.CETEnabled == 1)
    {
        cr4.CETEnabled = false;
        vmcb->StateSaveArea.Cr4 = cr4.Flags;
    }

    vmcb->StateSaveArea.Cr0 = cr0.Flags;
    return true;
}
```

#### AMD Architecture Callbacks

```cpp
// svm_handler.cpp:38-107
static u64 AmdGetGuestCr3(void* arch_data) {
    auto vmcb = reinterpret_cast<Svm::Vmcb*>(arch_data);
    return vmcb->Cr3();
}

static u64 AmdGetHostCr3(void* /* arch_data */) {
    return __readcr3();
}

static u64 AmdGetEptBase(void* arch_data) {
    auto vmcb = reinterpret_cast<Svm::Vmcb*>(arch_data);
    return vmcb->NestedPageTableCr3();
}

static VMX_ROOT_ERROR AmdSetEptBase(void* arch_data, u64 new_base) {
    auto vmcb = reinterpret_cast<Svm::Vmcb*>(arch_data);

    vmcb->NestedPageTableCr3() = new_base;
    vmcb->ControlArea.TlbControl.layout.TlbControl = 0x1;  // Invalidate TLB
    vmcb->ControlArea.GMETEnable = false;
    bitmap::SetBit(&storage::GetStorageArray()[EPT_OS_INIT_BITMAP], CPU::ApicId(), true);

    return VMX_ROOT_ERROR::SUCCESS;
}

static VMX_ROOT_ERROR AmdEnableEpt(void* arch_data) {
    auto vmcb = reinterpret_cast<Svm::Vmcb*>(arch_data);
    vmcb->ControlArea.NpEnable = true;
    return VMX_ROOT_ERROR::SUCCESS;
}

static VMX_ROOT_ERROR AmdDisableEpt(void* arch_data) {
    auto vmcb = reinterpret_cast<Svm::Vmcb*>(arch_data);
    vmcb->ControlArea.NpEnable = false;
    return VMX_ROOT_ERROR::SUCCESS;
}

static u64 AmdGetVmcb(void* arch_data) {
    auto vmcb = reinterpret_cast<Svm::Vmcb*>(arch_data);
    return core::mm::translate((host_virt_t)vmcb);  // Virtual to physical
}

static void AmdFlushEptTlb(void* arch_data) {
    auto vmcb = reinterpret_cast<Svm::Vmcb*>(arch_data);
    vmcb->ControlArea.TlbControl.layout.TlbControl = 0x1;
}

static void RegisterAmdCallbacks() {
    ombra::ArchCallbacks callbacks = {};
    callbacks.GetGuestCr3 = AmdGetGuestCr3;
    callbacks.GetHostCr3 = AmdGetHostCr3;
    callbacks.GetEptBase = AmdGetEptBase;
    callbacks.SetEptBase = AmdSetEptBase;
    callbacks.EnableEpt = AmdEnableEpt;
    callbacks.DisableEpt = AmdDisableEpt;
    callbacks.GetVmcb = AmdGetVmcb;
    callbacks.FlushEptTlb = AmdFlushEptTlb;
    callbacks.SetEptInitBit = AmdSetEptInitBit;
    callbacks.GetEptInitBit = AmdGetEptInitBit;

    core::RegisterArchCallbacks(callbacks);
}
```

---

## Shared Dispatch Logic

### VmExitContext Structure

```cpp
// PayLoad/include/context.h
namespace ombra {

struct VmExitContext {
    // Guest state
    u64 guest_cr3;
    u64 guest_rip;
    u64* p_rax;  // Pointer to guest RAX (for return value)

    // Hypercall parameters (from guest registers)
    u64 cmd_type;        // RCX = VMCALL_TYPE enum
    u64 cmd_guest_va;    // RDX = &COMMAND_DATA in guest memory
    u64 extra_param;     // R8 = target CR3 or extra parameter
    u64 auth_key;        // R9 = authentication key

    // Local command data cache
    COMMAND_DATA local_cmd;
    bool cmd_cached;

    // Architecture-specific pointers
    void* arch_data;     // Intel: nullptr, AMD: Vmcb*
    void* arch_data2;    // Intel: pcontext_t, AMD: pguest_context

    void Reset() {
        guest_cr3 = 0;
        guest_rip = 0;
        p_rax = nullptr;
        cmd_type = 0;
        cmd_guest_va = 0;
        extra_param = 0;
        auth_key = 0;
        cmd_cached = false;
        arch_data = nullptr;
        arch_data2 = nullptr;
        memset(&local_cmd, 0, sizeof(local_cmd));
    }

    VMCALL_TYPE GetCommandType() const {
        return static_cast<VMCALL_TYPE>(cmd_type);
    }
};

} // namespace ombra
```

### Architecture Callbacks

```cpp
// PayLoad/include/context.h
namespace ombra {

struct ArchCallbacks {
    u64 (*GetGuestCr3)(void* arch_data);
    u64 (*GetHostCr3)(void* arch_data);
    u64 (*GetEptBase)(void* arch_data);
    VMX_ROOT_ERROR (*SetEptBase)(void* arch_data, u64 new_base);
    VMX_ROOT_ERROR (*EnableEpt)(void* arch_data);
    VMX_ROOT_ERROR (*DisableEpt)(void* arch_data);
    u64 (*GetVmcb)(void* arch_data);
    void (*FlushEptTlb)(void* arch_data);
    void (*SetEptInitBit)(void* arch_data, u32 core_id, bool value);
    bool (*GetEptInitBit)(void* arch_data, u32 core_id);
};

extern ArchCallbacks g_arch_callbacks;

} // namespace ombra
```

### Main Dispatch Switch

```cpp
// PayLoad/core/dispatch.cpp:45-145
VMX_ROOT_ERROR HandleVmcall(ombra::VmExitContext* ctx) {
    // Validate authentication key
    if (!vmcall::IsVmcall(ctx->auth_key)) {
        return VMX_ROOT_ERROR::INVALID_GUEST_PARAM;
    }

    // Read command data from guest memory if needed
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

    // Dispatch based on command type
    VMX_ROOT_ERROR result;

    switch (ctx->GetCommandType()) {
        case VMCALL_GET_CR3:
            result = handlers::HandleGetCr3(ctx);
            break;

        case VMCALL_GET_CR3_ROOT:
            result = handlers::HandleGetCr3Root(ctx);
            break;

        case VMCALL_READ_PHY:
            result = handlers::HandleReadPhys(ctx);
            break;

        case VMCALL_WRITE_PHY:
            result = handlers::HandleWritePhys(ctx);
            break;

        case VMCALL_READ_VIRT:
            result = handlers::HandleReadVirt(ctx);
            break;

        case VMCALL_WRITE_VIRT:
            result = handlers::HandleWriteVirt(ctx);
            break;

        case VMCALL_VIRT_TO_PHY:
            result = handlers::HandleVirtToPhys(ctx);
            break;

        case VMCALL_STORAGE_QUERY:
            result = handlers::HandleStorageQuery(ctx);
            break;

        case VMCALL_GET_EPT_BASE:
            result = handlers::HandleGetEptBase(ctx);
            break;

        case VMCALL_SET_EPT_BASE:
            result = handlers::HandleSetEptBase(ctx);
            break;

        case VMCALL_ENABLE_EPT:
            result = handlers::HandleEnableEpt(ctx);
            break;

        case VMCALL_DISABLE_EPT:
            result = handlers::HandleDisableEpt(ctx);
            break;

        case VMCALL_SET_COMM_KEY:
            result = handlers::HandleSetCommKey(ctx);
            break;

        case VMCALL_GET_VMCB:
            result = handlers::HandleGetVmcb(ctx);
            break;

        case VMCALL_DISABLE_ETW_TI:
            result = handlers::HandleDisableEtwTi(ctx);
            break;

        case VMCALL_ENABLE_ETW_TI:
            result = handlers::HandleEnableEtwTi(ctx);
            break;

        case VMCALL_WIPE_ETW_BUFFERS:
            result = handlers::HandleWipeEtwBuffers(ctx);
            break;

        case VMCALL_CLEAR_EVENT_LOGS:
            result = handlers::HandleClearEventLogs(ctx);
            break;

        default:
            result = VMX_ROOT_ERROR::INVALID_GUEST_PARAM;
            break;
    }

    return result;
}
```

### Example Command Handler

```cpp
// PayLoad/core/dispatch.cpp:213-233
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
```

---

## Assembly Components

### Intel VMX Assembly Stubs (VTx-helper.asm)

```asm
;x64 ABI is described here https://learn.microsoft.com/en-us/cpp/build/x64-calling-convention

EXTERN VmExitHandler:PROC
EXTERN VmResumeExec:PROC
EXTERN SaveContext:PROC
EXTERN RestoreContext:PROC
EXTERN VmxLaunch:PROC

.CONST

KTRAP_FRAME_SIZE            EQU     190h
MACHINE_FRAME_SIZE          EQU     28h

.code _text

; void VmxSaveAndLaunch(REGS& context);
VmxSaveAndLaunch PROC
	mov rax, _end
	push rax
	call SaveContext

	vmlaunch

	pop rax ; in case vmlaunch fails we restore the right return address
_end:
	ret
VmxSaveAndLaunch ENDP

; void VmxRestore(REGS& context);
VmxRestore PROC
	xor eax, eax
	mov al, byte ptr [rdx]
	cmp eax, 1
	je _skip_vmxoff
	vmxoff
	mov qword ptr [rdx], 0

_skip_vmxoff:
	call RestoreContext ;This is a non returning call

	int 3
VmxRestore ENDP

; void VmExitWrapper();
VmExitWrapper PROC FRAME
    ; Windbg stack trace support via .FRAME and .PUSHFRAME
    .PUSHFRAME
    sub rsp, KTRAP_FRAME_SIZE - MACHINE_FRAME_SIZE
    .ALLOCSTACK KTRAP_FRAME_SIZE - MACHINE_FRAME_SIZE + 108h

	push 0		; align stack
	push 0
	and rsp, -16

	push 0
	push 0 ;for extended control register

	; Save XMM registers (0x160 bytes)
	sub rsp, 160h
    movaps xmmword ptr [rsp +  0h], xmm0
    movaps xmmword ptr [rsp + 10h], xmm1
    movaps xmmword ptr [rsp + 20h], xmm2
    movaps xmmword ptr [rsp + 30h], xmm3
    movaps xmmword ptr [rsp + 40h], xmm4
    movaps xmmword ptr [rsp + 50h], xmm5
	movaps xmmword ptr [rsp + 60h], xmm6
    movaps xmmword ptr [rsp + 70h], xmm7
    movaps xmmword ptr [rsp + 80h], xmm8
    movaps xmmword ptr [rsp + 90h], xmm9
    movaps xmmword ptr [rsp + 0a0h], xmm10
    movaps xmmword ptr [rsp + 0b0h], xmm11
	movaps xmmword ptr [rsp + 0c0h], xmm12
    movaps xmmword ptr [rsp + 0d0h], xmm13
	movaps xmmword ptr [rsp + 0e0h], xmm14
    movaps xmmword ptr [rsp + 0f0h], xmm15

	; Save general purpose registers
	pushfq
	push 0		; we do not need RIP here
	push r15
	push r14
	push r13
	push r12
	push r11
	push r10
	push r9
	push r8
	push rdi
	push rsi
	push rbp
	push rbp
	push rbx
	push rdx
	push rcx
	push rax

	mov rcx, rsp  ; RCX = pointer to saved context

	sub rsp, 20h		; make space on the stack
	.ENDPROLOG

	call VmExitHandler  ; Call C++ handler

	add rsp, 20h		; restore rsp

	; Restore general purpose registers
	pop rax
	pop rcx
	pop rdx
	pop rbx
	pop rbp
	pop rbp
	pop rsi
	pop rdi
	pop r8
	pop r9
	pop r10
	pop r11
	pop r12
	pop r13
	pop r14
	pop r15
	popfq				; just to move rsp
	popfq

	; Restore XMM registers
	movaps xmm0,  xmmword ptr [rsp +  0h]
    movaps xmm1,  xmmword ptr [rsp + 10h]
    movaps xmm2,  xmmword ptr [rsp + 20h]
    movaps xmm3,  xmmword ptr [rsp + 30h]
    movaps xmm4,  xmmword ptr [rsp + 40h]
    movaps xmm5,  xmmword ptr [rsp + 50h]
	movaps xmm6, xmmword ptr [rsp + 60h]
    movaps xmm7, xmmword ptr [rsp + 70h]
    movaps xmm8, xmmword ptr [rsp + 80h]
    movaps xmm9, xmmword ptr [rsp + 90h]
    movaps xmm10, xmmword ptr [rsp + 0a0h]
    movaps xmm11, xmmword ptr [rsp + 0b0h]
	movaps xmm12, xmmword ptr [rsp + 0c0h]
    movaps xmm13, xmmword ptr [rsp + 0d0h]
	movaps xmm14, xmmword ptr [rsp + 0e0h]
    movaps xmm15, xmmword ptr [rsp + 0f0h]
	add rsp, 60h

	jmp VmResumeExec
VmExitWrapper ENDP

; void VmRestore(PREGS pContext);
VmRestore PROC
	; Build IRETQ frame from VMCS guest state
	lea rbp, [rsp + 38h]

	; push SS
	mov rdx, 0804h; VMCS_GUEST_SS_SELECTOR
	vmread rax, rdx
	mov [rbp - 00h], rax

	; push RSP
	mov rdx, 681Ch; VMCS_GUEST_RSP
	vmread rax, rdx
	mov [rbp - 08h], rax

	; push RFLAGS
	mov rdx, 6820h; VMCS_GUEST_RFLAGS
	vmread rax, rdx
	mov [rbp - 10h], rax

	; push CS
	mov rdx, 0802h; VMCS_GUEST_CS_SELECTOR
	vmread rax, rdx
	mov [rbp - 18h], rax

	; push RIP
	mov rdx, 681Eh; VMCS_GUEST_RIP
	vmread rax, rdx
	mov [rbp - 20h], rax

	; store cr0 in rax
	mov rax, 6004h ; VMCS_CTRL_CR0_READ_SHADOW
	vmread rax, rax

	; store cr4 in rdx
	mov rdx, 6006h ; VMCS_CTRL_CR4_READ_SHADOW
	vmread rdx, rdx

	; execute vmxoff before we restore cr0 and cr4
	vmxoff

	; restore cr0 and cr4
	mov cr0, rax
	mov cr4, rdx
	vmxoff

	pop rbp
	pop rdx
	pop rax

	iretq
VmRestore ENDP

AsmVmxSaveState PROC
	pushfq	; save r/eflag

	push rax
	push rcx
	push rdx
	push rbx
	push rbp
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15

	sub rsp, 0100h
	; It a x64 FastCall function so the first parameter should go to rcx

	mov rcx, rsp

	call VmxLaunch

	jmp AsmVmxRestoreState

AsmVmxSaveState ENDP

AsmVmxRestoreState PROC
	add rsp, 0100h

	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rbp
	pop rbx
	pop rdx
	pop rcx
	pop rax

	popfq	; restore r/eflags

	xor eax, eax ; return STATUS_SUCCESS
	ret

AsmVmxRestoreState ENDP

END
```

**Key Details**:
- **Stack Frame Layout**: KTRAP_FRAME (0x190 bytes) includes all GPRs, XMM registers, and alignment padding
- **Context Pointer**: RSP is passed to VmExitHandler as first argument (RCX)
- **Register Save Order**: RAX, RCX, RDX, RBX, RBP (2x), RSI, RDI, R8-R15, RFLAGS, RIP, XMM0-15
- **IRETQ Frame**: VmRestore builds a complete interrupt return frame from VMCS fields
- **CR0/CR4 Shadow**: Uses READ_SHADOW fields to restore guest-visible values before VMXOFF

### AMD SVM Assembly Stubs (SVM-support.asm)

```asm
.code _text

EXTERN memcpy :PROC
public new_stack
new_stack PROC
	sub rcx, 1024
	mov rdx, rsp
	sub rdx, 1024
	mov r8, 2048
	call memcpy
	add rax, 1024
	mov rsp, rax
	ret
new_stack ENDP

;System V AMD64 argument order: RDI, RSI, RDX
;MSVC64 argument order: RCX, RDX, R8

;extern "C" int svm_enter_guest(ULONG64 vmcb, struct vcpu_gueststate* state, Seg::DescriptorTableRegister<Seg::Mode::longMode>* gdt);
public svm_enter_guest
svm_enter_guest PROC
	mov		r9, rcx ; vmcb PA (FIRST ARGUMENT)
	pushfq

	push	r8	; gdt pointer (THIRD ARGUMENT)
	mov		r10, rdx  ; guest state pointer (SECOND ARGUMENT)

	; Save (possibly) lazy-switched selectors
	str		ax
	push	ax
	mov		ax, es
	push	ax
	mov		ax, ds
	push	ax
	mov		ax, ss
	push	ax

	; Save FS/GS base MSRs
	mov		rcx, 0c0000100h ;MSR_FSBASE
	rdmsr
	push	rax
	push	rdx
	push	fs
	mov		rcx, 0c0000101h ;MSR_GSBASE
	rdmsr
	push	rax
	push	rdx
	push	gs
	mov		rcx, 0c0000102h ;MSR_KERNELGSBASE
	rdmsr
	push	rax
	push	rdx

	; Save various MSRs
	mov		rcx, 0c0000081h ;MSR_STAR
	rdmsr
	push	rax
	push	rdx

	mov		rcx, 0c0000082h ;MSR_LSTAR
	rdmsr
	push	rax
	push	rdx

	mov		rcx, 0c0000083h ;MSR_CSTAR
	rdmsr
	push	rax
	push	rdx

	mov		rcx, 0c0000084h ;MSR_SFMASK
	rdmsr
	push	rax
	push	rdx

	; Preserve callee-preserved registers as per AMD64 ABI
	push	r15
	push	r14
	push	r13
	push	r12
	push	rbp
	push	rbx
	push	rdi
	push	rsi

	push	r10		; Guest Regs Pointer

	; Restore guest registers
	mov		rax, r9  ; rax = vmcb pa
	mov		r8,  [r10 + 0A0h]
	mov		dr0, r8
	mov		r8,  [r10 + 0A8h]
	mov		dr1, r8
	mov		r8,  [r10 + 0B0h]
	mov		dr2, r8
	mov		r8,  [r10 + 0B8h]
	mov		dr3, r8
	; %dr6 is saved in the VMCB
	mov		r8,  [r10 + 078h]
	mov		cr2, r8
	mov		r15, [r10 + 070h]
	mov		r14, [r10 + 068h]
	mov		r13, [r10 + 060h]
	mov		r12, [r10 + 058h]
	mov		r11, [r10 + 050h]
	mov		rsi, [r10]
	mov		r9,  [r10 + 040h]
	mov		r8,  [r10 + 038h]
	mov		rbp, [r10 + 030h]
	mov		rdi, [r10 + 028h]
	mov		rdx, [r10 + 020h]
	mov		rcx, [r10 + 018h]
	mov		rbx, [r10 + 010h]
	; %rax at 0x08(%rsi) is not needed in SVM
	movups xmm0,  xmmword ptr [r10 + 0d0h +  0h]
    movups xmm1,  xmmword ptr [r10 + 0d0h + 10h]
    movups xmm2,  xmmword ptr [r10 + 0d0h + 20h]
    movups xmm3,  xmmword ptr [r10 + 0d0h + 30h]
    movups xmm4,  xmmword ptr [r10 + 0d0h + 40h]
    movups xmm5,  xmmword ptr [r10 + 0d0h + 50h]

	mov		r10, [r10 + 048h]

	vmload	rax   ; Load guest state from VMCB
	vmrun	rax   ; Enter guest - VM exits return here
	vmsave	rax   ; Save guest state to VMCB

	; Preserve guest registers not saved in VMCB
	push	r10
	push	rdi
	mov		rdi, [rsp+010h] ; Gets the R10 we pushed earlier (guest state pointer)
	mov		r10, [rsp+08h]  ; Gets the guest R10
	mov		[rdi + 048h], r10
	pop		rdi
	pop		r10 ; discard

	pop		r10  ; r10 = guest state pointer
	; %rax at 0x08(%rsi) is not needed in SVM
	mov		[r10], rsi
	mov		[r10 + 010h], rbx
	mov		[r10 + 018h], rcx
	mov		[r10 + 020h], rdx
	mov		[r10 + 028h], rdi
	mov		[r10 + 030h], rbp
	mov		[r10 + 038h], r8
	mov		[r10 + 040h], r9
	mov		[r10 + 050h], r11
	mov		[r10 + 058h], r12
	mov		[r10 + 060h], r13
	mov		[r10 + 068h], r14
	mov		[r10 + 070h], r15

    movups xmmword ptr [r10 + 0d0h +  0h], xmm0
    movups xmmword ptr [r10 + 0d0h + 10h], xmm1
    movups xmmword ptr [r10 + 0d0h + 20h], xmm2
    movups xmmword ptr [r10 + 0d0h + 30h], xmm3
    movups xmmword ptr [r10 + 0d0h + 40h], xmm4
    movups xmmword ptr [r10 + 0d0h + 50h], xmm5
	mov		rax, cr2
	mov		[r10 + 078h], rax
	mov		rax, dr0
	mov		[r10 + 0a0h], rax
	mov		rax, dr1
	mov		[r10 + 0a8h], rax
	mov		rax, dr2
	mov		[r10 + 0b0h], rax
	mov		rax, dr3
	mov		[r10 + 0b8h], rax

	; %dr6 is saved in the VMCB

	; %rdi = 0 means we took an exit
	xor		r11, r11
restore_host_svm:
	pop		rsi
	pop		rdi
	pop		rbx
	pop		rbp
	pop		r12
	pop		r13
	pop		r14
	pop		r15

	; Restore saved MSRs
	pop		rdx
	pop		rax
	mov		rcx, 0c0000084h ;MSR_SFMASK
	wrmsr

	pop		rdx
	pop		rax
	mov		rcx, 0c0000083h ;MSR_CSTAR
	wrmsr

	pop		rdx
	pop		rax
	mov		rcx, 0c0000082h ;MSR_LSTAR
	wrmsr

	pop		rdx
	pop		rax
	mov		rcx, 0c0000081h ;MSR_STAR
	wrmsr

	; popw %gs will reset gsbase to 0, so preserve it first
	cli
	pop		rdx
	pop		rax
	mov		rcx, 0c0000102h ;MSR_KERNELGSBASE
	wrmsr

	pop		gs
	pop		rdx
	pop		rax
	mov		rcx, 0c0000101h ;MSR_GSBASE
	wrmsr

	pop		fs
	pop		rdx
	pop		rax
	mov		rcx, 0c0000100h ;MSR_FSBASE
	wrmsr

	pop		ax
	mov		ss, ax
	pop		ax
	mov		ds, ax
	pop		ax
	mov		es, ax

	xor		rax, rax
	lldt	ax		; Host LDT is always 0

	pop		ax		; ax = saved TR

	pop		rdx
	add		rdx, 2
	mov		rdx, [rdx]

	; rdx = GDTR base addr
	and byte ptr[rdx + rax + 5], 0F9h  ; Clear busy bit in TSS descriptor

	ltr		ax

	popfq

	mov		rax, r11
	ret
	lfence

svm_enter_guest ENDP

END
```

**Key Details**:
- **MSR Save/Restore**: Preserves STAR, LSTAR, CSTAR, SFMASK, FS/GS base MSRs
- **Guest State Pointer**: R10 holds `vcpu_gueststate*` throughout
- **VMCB Physical Address**: R9/RAX holds VMCB PA for VMLOAD/VMRUN/VMSAVE
- **Debug Registers**: DR0-DR3, CR2 explicitly saved/restored
- **XMM Registers**: Only XMM0-5 preserved (Windows kernel ABI)
- **TSS Descriptor**: Clears busy bit before LTR to avoid #GP
- **Return Value**: R11 (cleared to 0 on normal exit)

---

## Legacy Kernel-Mode Handlers (OmbraCoreLib)

### Dynamic Handler Registration API

```cpp
// OmbraCoreLib/.../include/Vmexit.h
typedef union _ONEXIT_DATA {
    SVM::SVMState* svm;
    PREGS intel;

    _ONEXIT_DATA(SVM::SVMState* pState) : svm(pState) {}
    _ONEXIT_DATA(PREGS pRegs) : intel(pRegs) {}
} ONEXIT_DATA, * PONEXIT_DATA;

typedef bool (*fnVmexitHandler)(ONEXIT_DATA data);

typedef struct _VMEXIT_DATA {
    ULONG64 exitCode;
    fnVmexitHandler handler;
} VMEXIT_DATA, *PVMEXIT_DATA;

namespace vmexit {
    void Init();
    bool OnVmexit(ULONG64 vmexitCode, ONEXIT_DATA data);

    void InsertHandler(ULONG64 vmexitCode, fnVmexitHandler handler);
    void RemoveHandler(ULONG64 vmexitCode);
    fnVmexitHandler FindHandler(ULONG64 vmexitCode);
}
```

**Usage Pattern**:
```cpp
// Kernel driver initialization
vmexit::Init();
vmexit::InsertHandler(VMX_EXIT_REASON_EXECUTE_CPUID, VTx::VMExitHandlers::HandleCPUID);
vmexit::InsertHandler(VMX_EXIT_REASON_CR_ACCESS, VTx::VMExitHandlers::HandleCR);
vmexit::InsertHandler(VMX_EXIT_REASON_RDMSR, VTx::VMExitHandlers::HandleRDMSR);
vmexit::InsertHandler(VMX_EXIT_REASON_WRMSR, VTx::VMExitHandlers::HandleWRMSR);

// In VMExit handler
if (vmexit::OnVmexit(vmexit_reason, ONEXIT_DATA(guest_regs))) {
    // Handler returned true - handled
} else {
    // Handler returned false - chain to next
}
```

### Handler Table Management

```cpp
// OmbraCoreLib/.../src/Vmexit.cpp
bool bVmexitHandlerInit = false;
vector<VMEXIT_DATA>* vVmexitHandlers = 0;

void vmexit::Init()
{
    if (bVmexitHandlerInit)
        return;

    if (!vVmexitHandlers) {
        vVmexitHandlers = (vector<VMEXIT_DATA>*)cpp::kMalloc(sizeof(*vVmexitHandlers), PAGE_READWRITE);
        RtlZeroMemory(vVmexitHandlers, sizeof(*vVmexitHandlers));
        vVmexitHandlers->Init();
        vVmexitHandlers->reserve(64);
        vVmexitHandlers->DisableLock();  // No locking needed (single core)
    }

    bVmexitHandlerInit = true;
}

bool vmexit::OnVmexit(ULONG64 vmexitCode, ONEXIT_DATA data)
{
    if (!vVmexitHandlers)
        return false;

    for (auto& exitData : *vVmexitHandlers) {
        if (exitData.exitCode == vmexitCode) {
            if(exitData.handler(data))
                return true;
        }
    }
    return false;
}

void vmexit::InsertHandler(ULONG64 vmexitCode, fnVmexitHandler handler)
{
    if (!bVmexitHandlerInit || !handler)
        return;

    // Check if handler already exists - update it
    for (auto& exitHandler : *vVmexitHandlers) {
        if (exitHandler.exitCode == vmexitCode) {
            exitHandler.handler = handler;
            return;
        }
    }

    // Add new handler
    VMEXIT_DATA exitData = { 0 };
    exitData.exitCode = vmexitCode;
    exitData.handler = handler;

    vVmexitHandlers->Append(exitData);
}
```

### Legacy CPUID Handler

```cpp
// OmbraCoreLib/.../src/VMExitHandlers.cpp (lines 78-140)
bool VTx::VMExitHandlers::HandleCPUID(PREGS pContext)
{
    INT32 CpuInfo[4] = { 0 };

    // Check for vmcall key validation (pre-authentication)
    if (vmcall::ValidateCommunicationKey(0) &&  // Key not set yet
        pContext->rax == 'Hypr' && pContext->rcx == 'Chck') {
        pContext->rax = 'Yass';  // "Yes" response
        return false;
    }
    // Check for authenticated vmcall via CPUID
    else if (vmcall::ValidateCommunicationKey(pContext->rax)
        && vmcall::IsVmcall(pContext->r9)) {
        // Use CPUID as a VMCALL substitute for usermode modules
        pContext->rax = vmcall::HandleVmcall(pContext->rcx, pContext->rdx, pContext->r8, pContext->r9);
        return false;
    }

    // Otherwise, issue the CPUID to the logical processor
    __cpuidex(CpuInfo, (INT32)pContext->rax, (INT32)pContext->rcx);

    if (pContext->rax == PROC_FEATURES_CPUID)
    {
        // Unset the Hypervisor Present-bit in RCX
        CpuInfo[2] |= HYPERV_HYPERVISOR_PRESENT_BIT;
        CpuInfo[2] &= VMM_CPUIDECX_MASK;  // Mask out VMX, SMX, etc.
    }
    else if (pContext->rax == (unsigned long)Cpuid::Generic::GenericLeaf::ExtendedFeatureInformation)
    {
        // CpuInfo[2] &= VMM_ECPUIDECX_MASK;  // Commented out - don't mask AMD features
    }
    else if (pContext->rax == HYPERV_CPUID_INTERFACE)
    {
        // Return our interface identifier
        // CpuInfo[0] = CPU::chInterfaceID;  // Commented out
    }
    else if (pContext->rax == PROC_MANU_CPUID) {
#ifdef PROCESSOR_MANUFACTURER
        int* values = (int*)PROCESSOR_MANUFACTURER;
        pContext->rbx = values[0];
        pContext->rcx = values[1];
        pContext->rdx = values[2];
#endif
    }

    // Copy the values from the logical processor registers into the VP GPRs
    pContext->rax = CpuInfo[0];
    pContext->rbx = CpuInfo[1];
    pContext->rcx = CpuInfo[2];
    pContext->rdx = CpuInfo[3];

    return FALSE;  // Indicates we don't have to turn off VMX
}
```

**CPUID Masking Constants**:
```cpp
#define VMM_CPUIDECX_MASK ~(CPUIDECX_EST | CPUIDECX_TM2 | CPUIDECX_MWAIT | \
        CPUIDECX_PDCM | CPUIDECX_VMX | CPUIDECX_DTES64 | \
        CPUIDECX_DSCPL | CPUIDECX_SMX | CPUIDECX_CNXTID | \
        CPUIDECX_SDBG | CPUIDECX_XTPR | CPUIDECX_PCID | \
        CPUIDECX_DCA | CPUIDECX_X2APIC | CPUIDECX_DEADLINE)

#define VMM_ECPUIDECX_MASK ~(CPUIDECX_SVM | CPUIDECX_MWAITX)
```

### Legacy CR Access Handler

```cpp
// OmbraCoreLib/.../src/VMExitHandlers.cpp (lines 142-305)
bool VTx::VMExitHandlers::HandleCR(PREGS pContext)
{
    DWORD64 mask = 0;
    size_t ExitQualification = 0;
    size_t guestRIP;

    __vmx_vmread(GUEST_RIP, &guestRIP);
    __vmx_vmread(EXIT_QUALIFICATION, &ExitQualification);

    PMOV_CR_QUALIFICATION data = (PMOV_CR_QUALIFICATION)&ExitQualification;

    PULONG64 RegPtr = (PULONG64)&pContext->rax + data->Fields.Register;

    // Because its RSP and as we didn't save RSP correctly (because of pushes)
    // so we have to make it points to the GUEST_RSP
    if (data->Fields.Register == 4)
    {
        size_t RSP = 0;
        __vmx_vmread(GUEST_RSP, &RSP);
        *RegPtr = RSP;
    }

    switch (data->Fields.AccessType)
    {
    case TYPE_MOV_TO_CR:
    {
        switch (data->Fields.ControlRegister)
        {
        case 0:  // CR0 write
        {
            CR0 cr0;
            cr0.Flags = *RegPtr;
            if (cr0.Reserved1 || cr0.Reserved2 || cr0.Reserved3 || cr0.Reserved4)
                return true;  // Inject #GP
            __vmx_vmwrite(GUEST_CR0, *RegPtr);
            __vmx_vmwrite(CR0_READ_SHADOW, *RegPtr);

            // CET handling
            if (!CPU::bCETSupported)
                break;

            if (!cr0.WriteProtect) {
                CR4 cr4 = { 0 };
                __vmx_vmread(GUEST_CR4, &cr4.Flags);
                if (!cr4.CETEnabled) {
                    break;
                }
                vmm::vGuestStates[CPU::GetCPUIndex(true)].bCETNeedsEnabling = true;
                cr4.CETEnabled = false;
                __vmx_vmwrite(GUEST_CR4, cr4.Flags);
                __vmx_vmwrite(CR4_READ_SHADOW, cr4.Flags);
            }
            else {
                if (!vmm::vGuestStates[CPU::GetCPUIndex(true)].bCETNeedsEnabling) {
                    break;
                }
                CR4 cr4 = { 0 };
                __vmx_vmread(GUEST_CR4, &cr4.Flags);
                cr4.CETEnabled = true;
                __vmx_vmwrite(GUEST_CR4, cr4.Flags);
                __vmx_vmwrite(CR4_READ_SHADOW, cr4.Flags);
                vmm::vGuestStates[CPU::GetCPUIndex(true)].bCETNeedsEnabling = false;
            }
            break;
        }
        case 3:  // CR3 write
        {
            vmm::UpdateLastValidTsc();

            DWORD dwCore = CPU::GetCPUIndex(true);
            PVM_STATE pState = &vmm::vGuestStates[dwCore];
            CR3 cr3 = { 0 };
            cr3.Flags = *RegPtr;
            if (cr3.Reserved1 || cr3.Reserved2 || cr3.Reserved3)
                return true;  // Inject #GP

            // NMI blocking logic (commented out in source)
            pState->lastExitedCr3 = *RegPtr;
            __vmx_vmwrite(GUEST_CR3, *RegPtr);

            // Invalidate VPID TLB
            INVVPID_DESCRIPTOR desc = { 0 };
            desc.Vpid = dwCore + 1;
            CPU::InvalidateVPID(3, &desc);
            break;
        }
        case 4:  // CR4 write
        {
            CR4 cr4;
            cr4.Flags = *RegPtr;

            // Check against fixed bits
            mask = __readmsr(IA32_VMX_CR4_FIXED1);
            if (cr4.Flags & mask) {
                return true;  // Inject #GP
            }
            mask = __readmsr(IA32_VMX_CR4_FIXED0);
            if ((cr4.Flags & mask) != mask) {
                return true;  // Inject #GP
            }

            // Always set VMXE bit in actual CR4
            __vmx_vmwrite(GUEST_CR4, (*RegPtr | CR4_VMXE));
            __vmx_vmwrite(CR4_READ_SHADOW, *RegPtr);
            break;
        }
        default:
            break;
        }
    }
    break;

    case TYPE_MOV_FROM_CR:
    {
        switch (data->Fields.ControlRegister)
        {
        case 0:
            __vmx_vmread(GUEST_CR0, RegPtr);
            break;
        case 3:
            __vmx_vmread(GUEST_CR3, RegPtr);
            break;
        case 4:
            __vmx_vmread(GUEST_CR4, RegPtr);
            *RegPtr |= ~(CR4_VMXE);  // Hide VMXE bit from guest
            break;
        default:
            break;
        }
    }
    break;

    default:
        break;
    }

    return false;
}
```

### Legacy MSR Handlers

```cpp
// OmbraCoreLib/.../src/VMExitHandlers.cpp (lines 408-461)
bool VTx::VMExitHandlers::HandleRDMSR(PREGS pContext)
{
    MSR msr = { 0 };

    __try {
        msr.Content = __readmsr(pContext->rcx);
        if (pContext->rcx == IA32_FEATURE_CONTROL)
        {
            msr.Content |= IA32_FEATURE_CONTROL_LOCK_BIT_FLAG;
            msr.Content &= ~(IA32_FEATURE_CONTROL_ENABLE_VMX_INSIDE_SMX_FLAG);
            msr.Content &= ~(IA32_FEATURE_CONTROL_ENABLE_VMX_OUTSIDE_SMX_FLAG);
        }
        pContext->rax = msr.Low;
        pContext->rdx = msr.High;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        VTx::Exceptions::InjectException(
            EXCEPTION_VECTOR::EXCEPTION_VECTOR_GENERAL_PROTECTION_FAULT,
            vmm::vGuestStates[CPU::GetCPUIndex(true)].lastErrorCode
        );
        return true;
    }

    return false;
}

bool VTx::VMExitHandlers::HandleWRMSR(PREGS pContext)
{
    MSR msr = { 0 };

    msr.Low = (ULONG)pContext->rax;
    msr.High = (ULONG)pContext->rdx;
    if (pContext->rcx == IA32_FEATURE_CONTROL)
    {
        VTx::Exceptions::InjectException(
            EXCEPTION_VECTOR::EXCEPTION_VECTOR_GENERAL_PROTECTION_FAULT
        );
        return true;
    }
    __try {
        __writemsr(pContext->rcx, msr.Content);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        VTx::Exceptions::InjectException(
            EXCEPTION_VECTOR::EXCEPTION_VECTOR_GENERAL_PROTECTION_FAULT,
            vmm::vGuestStates[CPU::GetCPUIndex(true)].lastErrorCode
        );
        return true;
    }

    vmm::UpdateLastValidTsc();

    return false;
}
```

**Note**: The legacy kernel-mode handlers are NOT used by the PayLoad implementation. They represent a different architecture where the hypervisor ran in kernel mode with full Windows kernel APIs (ExAllocatePool, RtlZeroMemory, etc.). The PayLoad runs in VMXRoot context with NO kernel services available.

---

## C Conversion Notes

### Eliminating C++ Abstractions

#### 1. Replace Classes with Structs + Function Tables

**C++ (Current)**:
```cpp
namespace intel {
    void vmexit_handler(pcontext_t* context, void* unknown);
    static bool HandleCpuid(pcontext_t guest_registers);
    static u64 IntelGetGuestCr3(void* arch_data);
}
```

**C (Proposed)**:
```c
// vmx_handler.h
typedef struct vmx_handler_ops {
    void (*vmexit_handler)(pcontext_t* context, void* unknown);
    bool (*handle_cpuid)(pcontext_t guest_registers);
    u64 (*get_guest_cr3)(void* arch_data);
} vmx_handler_ops_t;

extern vmx_handler_ops_t intel_vmx_ops;

// vmx_handler.c
static bool intel_handle_cpuid(pcontext_t guest_registers);
static u64 intel_get_guest_cr3(void* arch_data);

vmx_handler_ops_t intel_vmx_ops = {
    .vmexit_handler = intel_vmexit_handler,
    .handle_cpuid = intel_handle_cpuid,
    .get_guest_cr3 = intel_get_guest_cr3
};
```

#### 2. Replace Templates/Generics with Macros

**C++ (Current)**:
```cpp
template<typename T>
auto map_guest_virt(u64 dirbase, T virt_addr) -> T {
    const auto virt_base = reinterpret_cast<u64>(virt_addr);
    const auto mapped_page = core::mm::map_guest_virt(dirbase, virt_base);
    return reinterpret_cast<T>(mapped_page);
}
```

**C (Proposed)**:
```c
#define MAP_GUEST_VIRT(dirbase, virt_addr, type) \
    ((type)(map_guest_virt_internal((dirbase), (u64)(virt_addr))))

static inline void* map_guest_virt_internal(u64 dirbase, u64 virt_base) {
    return (void*)core_mm_map_guest_virt(dirbase, virt_base);
}
```

#### 3. Replace Inline Functions with Macros

**C++ (Current)**:
```cpp
__forceinline auto get_vmcb() -> Svm::Vmcb*
{
    return *reinterpret_cast<Svm::Vmcb**>(
        *reinterpret_cast<u64*>(
            *reinterpret_cast<u64*>(
                __readgsqword(0) + ombra_context.vmcb_base)
            + ombra_context.vmcb_link) + ombra_context.vmcb_off);
}
```

**C (Proposed)**:
```c
#define GET_VMCB() \
    (*(Svm_Vmcb**)(*(u64*)(*(u64*)(__readgsqword(0) + ombra_context.vmcb_base) + \
    ombra_context.vmcb_link) + ombra_context.vmcb_off))
```

#### 4. Replace Auto with Explicit Types

**C++ (Current)**:
```cpp
const auto vmcb = get_vmcb();
auto result = core::HandleVmcall(&ctx);
```

**C (Proposed)**:
```c
Svm_Vmcb* vmcb = GET_VMCB();
VMX_ROOT_ERROR result = core_handle_vmcall(&ctx);
```

### Function Pointer Tables for Dispatch

#### Main VMExit Dispatch (Intel)

**C++ (Current)**:
```cpp
void vmexit_handler(pcontext_t* context, void* unknown)
{
    size_t vmexit_reason;
    __vmx_vmread(VMCS_EXIT_REASON, &vmexit_reason);

    if (vmexit_reason == VMX_EXIT_REASON_EXECUTE_CPUID)
    {
        bHandledExit = HandleCpuid(guest_registers);
        bIncRip = bHandledExit;
    }
    else if (vmexit_reason == EXIT_REASON_RDMSR)
    {
        // ... inline handler
    }
    // ... many more if/else branches
}
```

**C (Proposed)**:
```c
typedef bool (*vmx_exit_handler_fn)(pcontext_t* context, bool* inc_rip);

typedef struct vmx_exit_dispatch_entry {
    size_t exit_reason;
    vmx_exit_handler_fn handler;
} vmx_exit_dispatch_entry_t;

static bool intel_handle_cpuid_exit(pcontext_t* context, bool* inc_rip);
static bool intel_handle_rdmsr_exit(pcontext_t* context, bool* inc_rip);
static bool intel_handle_ept_violation_exit(pcontext_t* context, bool* inc_rip);
static bool intel_handle_vmx_instruction_exit(pcontext_t* context, bool* inc_rip);

static vmx_exit_dispatch_entry_t intel_vmx_dispatch_table[] = {
    { VMX_EXIT_REASON_EXECUTE_CPUID, intel_handle_cpuid_exit },
    { EXIT_REASON_RDMSR, intel_handle_rdmsr_exit },
    { VMX_EXIT_REASON_EPT_VIOLATION, intel_handle_ept_violation_exit },
    { EXIT_REASON_VMREAD, intel_handle_vmx_instruction_exit },
    { EXIT_REASON_VMWRITE, intel_handle_vmx_instruction_exit },
    /* ... */
    { 0, NULL }  // sentinel
};

void intel_vmexit_handler(pcontext_t* context, void* unknown)
{
    timing_on_exit_entry();

    root_setup();

    pcontext_t guest_registers = *context;
    size_t vmexit_reason;
    __vmx_vmread(VMCS_EXIT_REASON, &vmexit_reason);

    bool handled = false;
    bool inc_rip = false;

    // Dispatch via table lookup
    for (vmx_exit_dispatch_entry_t* entry = intel_vmx_dispatch_table;
         entry->handler != NULL;
         entry++)
    {
        if (entry->exit_reason == vmexit_reason)
        {
            handled = entry->handler(guest_registers, &inc_rip);
            break;
        }
    }

    if (handled)
    {
        if (inc_rip)
        {
            size_t rip, exec_len;
            __vmx_vmread(VMCS_GUEST_RIP, &rip);
            __vmx_vmread(VMCS_VMEXIT_INSTRUCTION_LENGTH, &exec_len);
            __vmx_vmwrite(VMCS_GUEST_RIP, rip + exec_len);
        }

        timing_on_exit_complete(NULL);
        return;
    }

    // Chain to original handler
    vmexit_handler_t orig_handler = (vmexit_handler_t)(
        (u64)&intel_vmexit_handler - ombra_context.vmexit_handler_rva);
    orig_handler(context, unknown);
}
```

#### Main VMExit Dispatch (AMD)

**C++ (Current)**:
```cpp
auto vmexit_handler(void* unknown, void* unknown2, pguest_context context) -> pgs_base_struct
{
    const auto vmcb = get_vmcb();

    switch (vmcb->ControlArea.ExitCode)
    {
    case Svm::SvmExitCode::VMEXIT_CPUID:
        bHandledExit = HandleCpuid(vmcb, context);
        bIncRip = true;
        break;
    case Svm::SvmExitCode::VMEXIT_NPF:
        // ... handler
        break;
    // ... many more cases
    }
}
```

**C (Proposed)**:
```c
typedef bool (*svm_exit_handler_fn)(Svm_Vmcb* vmcb, pguest_context context, bool* inc_rip);

typedef struct svm_exit_dispatch_entry {
    u64 exit_code;
    svm_exit_handler_fn handler;
} svm_exit_dispatch_entry_t;

static bool amd_handle_cpuid_exit(Svm_Vmcb* vmcb, pguest_context context, bool* inc_rip);
static bool amd_handle_npf_exit(Svm_Vmcb* vmcb, pguest_context context, bool* inc_rip);
static bool amd_handle_msr_exit(Svm_Vmcb* vmcb, pguest_context context, bool* inc_rip);
static bool amd_handle_svm_instruction_exit(Svm_Vmcb* vmcb, pguest_context context, bool* inc_rip);

static svm_exit_dispatch_entry_t amd_svm_dispatch_table[] = {
    { VMEXIT_CPUID, amd_handle_cpuid_exit },
    { VMEXIT_NPF, amd_handle_npf_exit },
    { VMEXIT_MSR, amd_handle_msr_exit },
    { VMEXIT_VMRUN, amd_handle_svm_instruction_exit },
    { VMEXIT_VMMCALL, amd_handle_svm_instruction_exit },
    { VMEXIT_VMLOAD, amd_handle_svm_instruction_exit },
    { VMEXIT_VMSAVE, amd_handle_svm_instruction_exit },
    { VMEXIT_CLGI, amd_handle_svm_instruction_exit },
    { VMEXIT_STGI, amd_handle_svm_instruction_exit },
    { VMEXIT_SKINIT, amd_handle_svm_instruction_exit },
    { 0, NULL }  // sentinel
};

pgs_base_struct amd_vmexit_handler(void* unknown, void* unknown2, pguest_context context)
{
    timing_on_exit_entry();

    root_setup();

    Seg_DescriptorTableRegister orig_idt;
    __sidt(&orig_idt);

    exception_save_orig_params(unknown, unknown2, context, &orig_idt, _AddressOfReturnAddress());
    __lidt(&exception_idt_reg);

    Svm_Vmcb* vmcb = GET_VMCB();

    bool inc_rip = false;
    bool handled = false;

    vmcb->ControlArea.TlbControl.layout.TlbControl = 0;

    if (g_cpuid_vmcall_called)
    {
        vmcb->ControlArea.InterceptCr.rw.write.layout.WriteCr4 = false;
        vmcb->ControlArea.InterceptCr.rw.write.layout.WriteCr0 = false;
        vmcb->ControlArea.InterceptCr0WritesOther = false;
    }

    // Dispatch via table lookup
    for (svm_exit_dispatch_entry_t* entry = amd_svm_dispatch_table;
         entry->handler != NULL;
         entry++)
    {
        if (entry->exit_code == vmcb->ControlArea.ExitCode)
        {
            handled = entry->handler(vmcb, context, &inc_rip);
            break;
        }
    }

    if (!handled)
    {
        __lidt(&orig_idt);
        vcpu_run_t orig_handler = (vcpu_run_t)(
            (u64)&amd_vmexit_handler - ombra_context.vcpu_run_rva);
        return orig_handler(unknown, unknown2, context);
    }

    if (inc_rip)
    {
        vmcb->StateSaveArea.Rip = vmcb->ControlArea.NextRip;
    }

    timing_on_exit_complete(vmcb);

    __lidt(&orig_idt);
    return (pgs_base_struct)__readgsqword(0);
}
```

### Register Context Handling in Pure C

#### Intel Context Structure

**C++ (Current)**:
```cpp
typedef struct _context_t
{
    u64 rax;
    u64 rcx;
    u64 rdx;
    u64 rbx;
    u64 rsp;
    u64 rbp;
    u64 rsi;
    u64 rdi;
    u64 r8;
    u64 r9;
    u64 r10;
    u64 r11;
    u64 r12;
    u64 r13;
    u64 r14;
    u64 r15;
    u128 xmm0;
    u128 xmm1;
    u128 xmm2;
    u128 xmm3;
    u128 xmm4;
    u128 xmm5;
} context_t, *pcontext_t;
```

**C (Same Structure)**:
```c
typedef struct context {
    u64 rax;
    u64 rcx;
    u64 rdx;
    u64 rbx;
    u64 rsp;
    u64 rbp;
    u64 rsi;
    u64 rdi;
    u64 r8;
    u64 r9;
    u64 r10;
    u64 r11;
    u64 r12;
    u64 r13;
    u64 r14;
    u64 r15;
    u128 xmm0;
    u128 xmm1;
    u128 xmm2;
    u128 xmm3;
    u128 xmm4;
    u128 xmm5;
} context_t;

typedef context_t* pcontext_t;
```

#### AMD Guest Context Structure

**C++ (Current)**:
```cpp
typedef struct __declspec(align(16)) _guest_context
{
    u8  gap0[8];
    u64 rcx;
    u64 rdx;
    u64 rbx;
    u8  gap20[8];
    u64 rbp;
    u64 rsi;
    u64 rdi;
    u64 r8;
    u64 r9;
    u64 r10;
    u64 r11;
    u64 r12;
    u64 r13;
    u64 r14;
    u64 r15;
    u128 xmm0;
    u128 xmm1;
    u128 xmm2;
    u128 xmm3;
    u128 xmm4;
    u128 xmm5;
    u128 xmm6;
    u128 xmm7;
    u128 xmm8;
    u128 xmm9;
    u128 xmm10;
    u128 xmm11;
    u128 xmm12;
    u128 xmm13;
    u128 xmm14;
    u128 xmm15;
    u8  gap180[8];
    u64 vmcb_physical_address;
} guest_context, *pguest_context;
```

**C (Use __attribute__ for alignment)**:
```c
typedef struct __attribute__((aligned(16))) guest_context {
    u8  gap0[8];
    u64 rcx;
    u64 rdx;
    u64 rbx;
    u8  gap20[8];
    u64 rbp;
    u64 rsi;
    u64 rdi;
    u64 r8;
    u64 r9;
    u64 r10;
    u64 r11;
    u64 r12;
    u64 r13;
    u64 r14;
    u64 r15;
    u128 xmm0;
    u128 xmm1;
    u128 xmm2;
    u128 xmm3;
    u128 xmm4;
    u128 xmm5;
    u128 xmm6;
    u128 xmm7;
    u128 xmm8;
    u128 xmm9;
    u128 xmm10;
    u128 xmm11;
    u128 xmm12;
    u128 xmm13;
    u128 xmm14;
    u128 xmm15;
    u8  gap180[8];
    u64 vmcb_physical_address;
} guest_context_t;

typedef guest_context_t* pguest_context;
```

#### Register Access Helpers

**C Implementation**:
```c
// Get register pointer by GPR number (AMD CR exit qualification)
static inline u64* get_register_for_cr_exit(Svm_Vmcb* vmcb, pguest_context context)
{
    u64 gpr_num = vmcb->ControlArea.ExitInfo1 & 15;

    switch (gpr_num)
    {
    case 0:  return &vmcb->Rax;
    case 1:  return &context->rcx;
    case 2:  return &context->rdx;
    case 3:  return &context->rbx;
    case 4:  return &vmcb->Rsp;
    case 5:  return &context->rbp;
    case 6:  return &context->rsi;
    case 7:  return &context->rdi;
    case 8:  return &context->r8;
    case 9:  return &context->r9;
    case 10: return &context->r10;
    case 11: return &context->r11;
    case 12: return &context->r12;
    case 13: return &context->r13;
    case 14: return &context->r14;
    case 15: return &context->r15;
    default: return NULL;
    }
}

// Macro for getting register by index (Intel MOV CR qualification)
#define GET_REGISTER_PTR(context, reg_num) \
    ((u64*)((u8*)(context) + offsetof(context_t, rax) + (reg_num) * sizeof(u64)))
```

### Intrinsic Function Availability

All Intel intrinsics (`__vmx_vmread`, `__vmx_vmwrite`, `__vmx_vmlaunch`, etc.) are available in C via `<intrin.h>`. The assembly stubs are already pure assembly.

**No changes needed** for:
- `__vmx_vmread(field, &value)`
- `__vmx_vmwrite(field, value)`
- `__readcr3()`
- `__readgsqword(offset)`
- `__readmsr(msr_id)`
- `__writemsr(msr_id, value)`
- `__cpuid(cpuInfo, leaf)`
- `__cpuidex(cpuInfo, leaf, subleaf)`
- `__sidt(idt_reg)`
- `__lidt(idt_reg)`

### Memory Operations in VMXRoot Context

The PayLoad environment has **NO** standard library functions available. All memory operations must be implemented manually:

**C Implementation**:
```c
// vmxroot_mem.h
static inline void vmxroot_memcpy(void* dst, const void* src, size_t n)
{
    u8* d = (u8*)dst;
    const u8* s = (const u8*)src;
    while (n--) *d++ = *s++;
}

static inline void vmxroot_memset(void* dst, int value, size_t n)
{
    u8* d = (u8*)dst;
    u8 val = (u8)value;
    while (n--) *d++ = val;
}

static inline void vmxroot_memzero(void* dst, size_t n)
{
    vmxroot_memset(dst, 0, n);
}

static inline int vmxroot_memcmp(const void* s1, const void* s2, size_t n)
{
    const u8* p1 = (const u8*)s1;
    const u8* p2 = (const u8*)s2;
    while (n--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++;
        p2++;
    }
    return 0;
}

// String operations
static inline size_t vmxroot_strlen(const char* str)
{
    const char* s = str;
    while (*s) s++;
    return s - str;
}

static inline void vmxroot_strcpy(char* dst, const char* src)
{
    while ((*dst++ = *src++));
}

static inline int vmxroot_strcmp(const char* s1, const char* s2)
{
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const u8*)s1 - *(const u8*)s2;
}
```

---

## Testing Checklist

### Pre-Conversion Validation

- [ ] Document all VMExit reasons currently handled
- [ ] Document all hypercall commands currently implemented
- [ ] Capture baseline performance metrics (RDTSC overhead, VMExit latency)
- [ ] Identify all C++-specific features used (templates, auto, namespaces, classes)
- [ ] List all intrinsics and ensure C equivalents exist
- [ ] Verify assembly stubs work with both MSVC and GCC

### Conversion Process

- [ ] Port Intel VMX handler to C with function pointer dispatch table
- [ ] Port AMD SVM handler to C with function pointer dispatch table
- [ ] Replace all C++ classes with C structs + function pointers
- [ ] Replace all templates with macros or explicit types
- [ ] Replace all `auto` with explicit types
- [ ] Remove all namespaces - use prefix naming convention
- [ ] Replace all `inline` functions with `static inline` or macros
- [ ] Implement VMXRoot-safe memory operations (no libc)
- [ ] Port timing compensation logic to C
- [ ] Port CPUID spoofing logic to C
- [ ] Port storage slot management to C

### Post-Conversion Testing

#### Functional Tests

- [ ] **CPUID Hypercall**: Verify magic leaf 0x13371337 still triggers vmcall dispatch
- [ ] **Authentication**: Verify VMCALL_SET_COMM_KEY sets key correctly
- [ ] **Memory Read/Write**: Test VMCALL_READ_VIRT, VMCALL_WRITE_VIRT, VMCALL_READ_PHY, VMCALL_WRITE_PHY
- [ ] **Address Translation**: Test VMCALL_VIRT_TO_PHY across different CR3s
- [ ] **EPT/NPT Control**: Test VMCALL_GET_EPT_BASE, VMCALL_SET_EPT_BASE, VMCALL_ENABLE_EPT, VMCALL_DISABLE_EPT
- [ ] **Storage Slots**: Test VMCALL_STORAGE_QUERY read/write
- [ ] **CR3 Queries**: Test VMCALL_GET_CR3, VMCALL_GET_CR3_ROOT
- [ ] **VMCB Query** (AMD): Test VMCALL_GET_VMCB returns correct physical address

#### Anti-Detection Tests

- [ ] **Hypervisor Bit**: Verify CPUID leaf 1 ECX bit 31 is cleared
- [ ] **VMX Instructions**: Verify VMREAD/VMWRITE from Ring 0 inject #UD
- [ ] **SVM Instructions**: Verify VMRUN/VMMCALL from Ring 0 inject #UD
- [ ] **APERF/MPERF**: Verify virtualized MSR reads return compensated values
- [ ] **TSC Offsetting**: Verify RDTSC timing shows no hypervisor overhead

#### EPT Violation Tests

- [ ] Register EPT handler via storage slot
- [ ] Trigger EPT violation by accessing shadow page
- [ ] Verify handler is called with correct fault PA
- [ ] Verify EPT_OS_INIT_BITMAP is set for current core

#### Performance Tests

- [ ] **VMExit Latency**: Measure cycles from CPUID to guest resume
- [ ] **VMCALL Latency**: Measure round-trip time for simple hypercall
- [ ] **Memory Copy**: Benchmark VMCALL_READ_VIRT vs. direct memory access
- [ ] **Dispatch Overhead**: Compare C function pointer table vs. C++ switch/if-else

#### Multi-Core Tests

- [ ] Verify per-CPU storage initialization
- [ ] Verify EPT_OS_INIT_BITMAP is per-core
- [ ] Verify timing compensation is per-core
- [ ] Test simultaneous VMExits on multiple cores

#### Edge Cases

- [ ] **Invalid Hypercall**: Test unrecognized VMCALL_TYPE returns INVALID_GUEST_PARAM
- [ ] **Invalid Auth Key**: Test invalid R9 key returns INVALID_GUEST_PARAM
- [ ] **NULL Pointers**: Test NULL cmd_guest_va returns error
- [ ] **Cross-Process Memory**: Test reading from different CR3
- [ ] **Large Transfers**: Test >PAGE_SIZE memory copy

#### Chain to Original Handler

- [ ] **Unhandled VMExit**: Verify unhandled exit reasons chain to original Hyper-V handler
- [ ] **Return Value**: Verify original handler return is propagated correctly
- [ ] **RIP Advancement**: Verify original handler advances RIP when appropriate

### Debugging Verification

- [ ] Verify Windbg can display stack traces through FRAME annotations
- [ ] Verify `.PUSHFRAME` and `.ALLOCSTACK` unwind data is correct
- [ ] Test breakpoint placement in C handlers
- [ ] Test live register inspection during VMExit

### Stress Testing

- [ ] Run 1,000,000 hypercalls in tight loop
- [ ] Trigger 10,000 CPUID exits per second
- [ ] Trigger 1,000 EPT violations per second
- [ ] Run for 24 hours under load
- [ ] Monitor for memory corruption (corrupt stack, heap, VMCS, VMCB)

---

## Summary

This document provides a complete blueprint for porting the Ombra VMExit handlers from C++ to pure C + Assembly. Key takeaways:

1. **Function Pointer Dispatch Tables**: Replace C++ namespaces and switch statements with explicit C function pointer arrays
2. **Eliminate Templates**: Use macros or explicit types for all generic code
3. **Replace Auto**: Use explicit types everywhere
4. **VMXRoot-Safe Memory**: NO libc - implement custom memcpy, memset, strlen, etc.
5. **Preserve Assembly Stubs**: VTx-helper.asm and SVM-support.asm work as-is
6. **Test Exhaustively**: Verify every hypercall, every VMExit reason, every edge case

The conversion should be **mechanical and incremental**:
- Port one handler at a time
- Test after each conversion
- Keep C++ and C implementations side-by-side during transition
- Use `#ifdef PURE_C_BUILD` to toggle between implementations

**Estimated Effort**: 40-60 hours for complete conversion and testing.
