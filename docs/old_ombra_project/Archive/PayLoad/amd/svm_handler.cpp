// PayLoad/amd/svm_handler.cpp
// AMD SVM VMExit handler - Full production implementation
// Handles CPUID-based hypercalls, NPF, and CR write interception
//
// This backend uses the unified dispatch layer (core/dispatch.cpp) for
// all VMCALL command handling. Architecture-specific operations are
// provided via registered callbacks.

#include "svm_handler.h"
#include "mm.h"
#include "debug.h"
#include "exception.h"
#include "../include/storage.h"
#include "../include/timing.h"
#include "../core/dispatch.h"
#include "../core/cpuid_spoof.h"

#include <communication.hpp>
#include <OmbraSELib/vmxroot/bitmap.h>
#include <OmbraSELib/vmxroot/cpu.h>
#include <OmbraSELib/vmxroot/pe.h>
#include <Arch/Segmentation.h>

namespace amd {

//===----------------------------------------------------------------------===//
// Global State
//===----------------------------------------------------------------------===//

static bool bGlobalSetupDone = false;           // Global one-time init (shared across CPUs)
static bool bCpuSetupDone[storage::MAX_CPUS];   // Per-CPU init tracking
static bool bCpuidVmcallCalled = false;

//===----------------------------------------------------------------------===//
// AMD Architecture Callbacks for Dispatch Layer
//===----------------------------------------------------------------------===//

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
    vmcb->ControlArea.TlbControl.layout.TlbControl = 0x1;
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
    return core::mm::translate((host_virt_t)vmcb);
}

static void AmdFlushEptTlb(void* arch_data) {
    auto vmcb = reinterpret_cast<Svm::Vmcb*>(arch_data);
    vmcb->ControlArea.TlbControl.layout.TlbControl = 0x1;
}

static void AmdSetEptInitBit(void* /* arch_data */, u32 core_id, bool value) {
    bitmap::SetBit(&storage::GetStorageArray()[EPT_OS_INIT_BITMAP], core_id, value);
}

static bool AmdGetEptInitBit(void* /* arch_data */, u32 core_id) {
    return bitmap::GetBit(&storage::GetStorageArray()[EPT_OS_INIT_BITMAP], core_id);
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

//===----------------------------------------------------------------------===//
// CR Exit Register Access
//===----------------------------------------------------------------------===//

static UINT64 GetGPRNumberForCrExit(Svm::Vmcb* vmcb)
{
    return (vmcb->ControlArea.ExitInfo1 & 15);
}

static UINT64* GetRegisterForCrExit(Svm::Vmcb* vmcb, pguest_context context)
{
    switch (GetGPRNumberForCrExit(vmcb))
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

//===----------------------------------------------------------------------===//
// CR Write Handlers
//===----------------------------------------------------------------------===//

static bool HandleCr4Write(Svm::Vmcb* vmcb, pguest_context context)
{
    UINT64* reg = GetRegisterForCrExit(vmcb, context);
    CR4 cr4;
    cr4.Flags = *reg;

    // GP fault checks:
    // - Cannot change CR4.PCIDE from 0 to 1 while CR3[11:0] != 0
    // - Cannot write 1 to reserved bits
    // - Cannot clear PAE while in IA-32e mode
    if (cr4.PcidEnable == 1)
    {
        CR3 cr3;
        cr3.Flags = vmcb->Cr3();
        if (cr3.Flags & 0xFFF)
        {
            return false;
        }
    }

    if (cr4.Reserved1 || cr4.Reserved2 || cr4.Reserved3 || cr4.Reserved4)
    {
        return false;
    }

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
    {
        return false;
    }

    // Disable CET if enabled (compatibility)
    if (cr4.CETEnabled == 1)
    {
        cr4.CETEnabled = false;
        vmcb->StateSaveArea.Cr4 = cr4.Flags;
    }

    vmcb->StateSaveArea.Cr0 = cr0.Flags;
    return true;
}

//===----------------------------------------------------------------------===//
// CPUID Handler - Hypercall Dispatch via Unified Layer
//===----------------------------------------------------------------------===//

static bool HandleCpuid(Svm::Vmcb* vmcb, pguest_context context)
{
    // Get CPUID leaf/subleaf from VMCB RAX and guest context RCX
    u32 leaf = static_cast<u32>(vmcb->Rax());
    u32 subleaf = static_cast<u32>(context->rcx);

    // Check if this is our magic hypercall leaf
    if (leaf == cpuid_spoof::HYPERCALL_MAGIC_LEAF)
    {
        // Build VmExitContext from AMD-specific state
        ombra::VmExitContext ctx;
        ctx.Reset();

        // Populate guest state
        ctx.guest_cr3 = vmcb->Cr3();
        ctx.guest_rip = vmcb->StateSaveArea.Rip;
        ctx.p_rax = &vmcb->Rax();

        // Populate hypercall parameters from guest registers
        ctx.cmd_type = context->rcx;      // RCX = VMCALL_TYPE
        ctx.cmd_guest_va = context->rdx;  // RDX = &COMMAND_DATA
        ctx.extra_param = context->r8;    // R8 = target CR3 or extra param
        ctx.auth_key = context->r9;       // R9 = authentication key

        // Store arch-specific data for callbacks
        ctx.arch_data = vmcb;
        ctx.arch_data2 = context;

        // Dispatch to unified handler
        VMX_ROOT_ERROR result = core::HandleVmcall(&ctx);

        // Check if this was a valid authenticated vmcall
        if (result != VMX_ROOT_ERROR::INVALID_GUEST_PARAM || vmcall::IsVmcall(ctx.auth_key)) {
            // Valid hypercall - set return value and mark that we've received a vmcall
            vmcb->Rax() = static_cast<u64>(result);
            bCpuidVmcallCalled = true;
            return true;
        }
        // Invalid auth on magic leaf - fall through to spoofing
    }

    // Handle ALL CPUID leaves with spoofing
    // Execute real CPUID in VMXRoot (native execution, no VMExit)
    // Then apply antidetection modifications
    u32 eax, ebx, ecx, edx;
    cpuid_spoof::ExecuteAndSpoof(leaf, subleaf, &eax, &ebx, &ecx, &edx);

    // Return spoofed values via VMCB/context
    vmcb->Rax() = static_cast<u64>(eax);
    context->rbx = static_cast<u64>(ebx);
    context->rcx = static_cast<u64>(ecx);
    context->rdx = static_cast<u64>(edx);

    return true;  // We handle ALL CPUID - don't chain to original Hyper-V handler
}

//===----------------------------------------------------------------------===//
// Root Setup - One-time initialization
//===----------------------------------------------------------------------===//

static void RootSetup()
{
    u32 cpu_id = storage::GetCurrentCpuId();

    // Per-CPU initialization - runs once per CPU on first VMExit
    if (cpu_id < storage::MAX_CPUS && !bCpuSetupDone[cpu_id])
    {
        bCpuSetupDone[cpu_id] = true;

        // Initialize this CPU's storage slots (Phase 4 per-CPU storage)
        storage::Initialize();

        // Initialize timing compensation for this CPU
        timing::Initialize();
    }

    // Global one-time initialization - runs once for the entire system
    if (!bGlobalSetupDone)
    {
        bGlobalSetupDone = true;

        // Initialize global storage slots (shared across CPUs)
        storage::InitializeGlobal();

        // Initialize exception handling
        exception::Initialize();

        // Find our PE base for exception handling
        ombra_context.record_base = (u64)pe::FindPE();

        // Initialize identity mapping
        core::mm::init();

        // Initialize CPU utilities
        CPU::Init();

        // Initialize CPUID spoofing for antidetection
        cpuid_spoof::Initialize();

        // Register AMD-specific callbacks for dispatch layer
        RegisterAmdCallbacks();
    }
}

//===----------------------------------------------------------------------===//
// Initialization (public)
//===----------------------------------------------------------------------===//

void Initialize()
{
    RootSetup();
}

//===----------------------------------------------------------------------===//
// Main VMExit Handler
//===----------------------------------------------------------------------===//

auto vmexit_handler(void* unknown, void* unknown2, pguest_context context) -> pgs_base_struct
{
    //=========================================================================
    // TIMING: Capture entry TSC immediately (must be first instruction)
    // Critical for defeating BattlEye's RDTSC timing checks
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
        // Nested Page Fault - call registered EPT handler if available
        u64 eptHandler = storage::Query(EPT_HANDLER_ADDRESS);
        u64* storageArr = storage::GetStorageArray();
        if (eptHandler && bitmap::GetBit(&storageArr[EPT_OS_INIT_BITMAP], CPU::ApicId()))
        {
            auto pEptHandler = (fnEptHandler)eptHandler;
            pEptHandler(vmcb->ControlArea.ExitInfo2);
            bHandledExit = true;
        }
        break;
    }

    //=========================================================================
    // Handle MSR reads - Virtualize APERF/MPERF for Anti-Timing Detection
    // ESEA uses these MSRs to detect hypervisor overhead via IET analysis
    //=========================================================================
    case Svm::SvmExitCode::VMEXIT_MSR:
    {
        bool is_write = vmcb->ControlArea.ExitInfo1 & 1;
        u32 msr_id = static_cast<u32>(context->rcx);

        // Only handle reads, and only for APERF/MPERF
        if (!is_write && (msr_id == 0xE7 || msr_id == 0xE8)) {
            // IA32_MPERF (0xE7) or IA32_APERF (0xE8)
            // Return compensated value that hides hypervisor overhead
            u64 fake_value = timing::ReadMsrVirtualized(msr_id);
            vmcb->Rax() = fake_value & 0xFFFFFFFF;
            context->rdx = fake_value >> 32;
            bHandledExit = true;
            bIncRip = true;
        }
        // If not APERF/MPERF or is a write, fall through to original handler
        break;
    }

    //=========================================================================
    // Handle SVM Instructions - Inject #UD for Anti-Detection
    // EAC probes vmrun/vmmcall from Ring 0 to detect hypervisors
    // On bare metal (SVM disabled), these cause #UD - we must emulate this
    //=========================================================================
    case Svm::SvmExitCode::VMEXIT_VMRUN:
    case Svm::SvmExitCode::VMEXIT_VMMCALL:  // Guest VMMCALL (not our hypercall)
    case Svm::SvmExitCode::VMEXIT_VMLOAD:
    case Svm::SvmExitCode::VMEXIT_VMSAVE:
    case Svm::SvmExitCode::VMEXIT_CLGI:
    case Svm::SvmExitCode::VMEXIT_STGI:
    case Svm::SvmExitCode::VMEXIT_SKINIT:
    {
        // Inject #UD exception to simulate bare-metal behavior
        // Use EventInj union format from Svm.h:500-511
        Svm::EventInj inj = {};
        inj.layout.Vector = 6;            // #UD exception
        inj.layout.Type = 3;              // Exception (fault/trap)
        inj.layout.ErrorCodeValid = 0;    // #UD has no error code
        inj.layout.Valid = 1;             // Injection is valid
        vmcb->ControlArea.EventInjection = inj.raw;

        bHandledExit = true;
        bIncRip = false;  // Exception uses faulting RIP
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

    // Advance RIP if needed
    if (bIncRip)
    {
        vmcb->StateSaveArea.Rip = vmcb->ControlArea.NextRip;
    }

    //=========================================================================
    // TIMING: Adjust TSC offset before VMRUN
    // Subtracts VMExit overhead to hide from guest timing measurements
    //=========================================================================
    timing::OnExitComplete(vmcb);  // AMD passes VMCB pointer for TSC offset

    // Restore original IDT and return success
    __lidt(&origIdt);
    return reinterpret_cast<pgs_base_struct>(__readgsqword(0));
}

} // namespace amd
