// PayLoad/intel/vmx_handler.cpp
// Intel VMX VMExit handler - Full production implementation
// Handles CPUID-based hypercalls with dynamic key authentication
//
// This backend uses the unified dispatch layer (core/dispatch.cpp) for
// all VMCALL command handling. Architecture-specific operations are
// provided via registered callbacks.

#include "vmx_handler.h"
#include "mm.h"
#include "debug.h"
#include "../include/vmcall.h"
#include "../include/storage.h"
#include "../include/timing.h"
#include "../core/dispatch.h"
#include "../core/cpuid_spoof.h"

#include <communication.hpp>
#include <Arch/Vmx.h>
#include <intrin.h>

namespace intel {

//===----------------------------------------------------------------------===//
// Global State
//===----------------------------------------------------------------------===//

static bool bGlobalSetupDone = false;           // Global one-time init (shared across CPUs)
static bool bCpuSetupDone[storage::MAX_CPUS];   // Per-CPU init tracking
static bool bCpuidVmcallCalled = false;

//===----------------------------------------------------------------------===//
// EPT Handler Function Type
//===----------------------------------------------------------------------===//

using fnEptHandler = void(*)(u64 fault_pa);

//===----------------------------------------------------------------------===//
// VMCS Fields for EPT Control
//===----------------------------------------------------------------------===//

#ifndef VMCS_CTRL_EPT_POINTER
#define VMCS_CTRL_EPT_POINTER 0x0000201A
#endif

#ifndef VMCS_EXIT_QUALIFICATION
#define VMCS_EXIT_QUALIFICATION 0x6400
#endif

#ifndef VMCS_GUEST_PHYSICAL_ADDRESS
#define VMCS_GUEST_PHYSICAL_ADDRESS 0x2400
#endif

#ifndef VMCS_CTRL_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS
#define VMCS_CTRL_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS 0x0000401E
#endif

// Bit 1 in Secondary Processor-Based VM-Execution Controls enables EPT
#define SECONDARY_EXEC_ENABLE_EPT (1ULL << 1)

//===----------------------------------------------------------------------===//
// CPU Identification (for per-core storage bitmap)
//===----------------------------------------------------------------------===//

namespace CPU {
    inline u32 ApicId() {
        int cpuInfo[4];
        __cpuid(cpuInfo, 1);
        return (cpuInfo[1] >> 24) & 0xFF;
    }
}

//===----------------------------------------------------------------------===//
// Bitmap Helpers (matching AMD pattern)
//===----------------------------------------------------------------------===//

namespace bitmap {
    // EPT_OS_INIT_BITMAP spans 9 slots (slots 2-10), supporting 576 cores max
    constexpr u32 MAX_BITMAP_SLOTS = 9;
    constexpr u32 MAX_SUPPORTED_CORES = MAX_BITMAP_SLOTS * 64;  // 576 cores

    inline void SetBit(u64* bitmap, u32 bit, bool value) {
        u32 index = bit / 64;
        // Bounds check to prevent buffer overflow on high core count systems
        if (index >= MAX_BITMAP_SLOTS) {
            return;  // Silently ignore - core ID exceeds supported range
        }
        u64 mask = 1ULL << (bit % 64);
        u64* slot = bitmap + index;
        if (value) *slot |= mask;
        else *slot &= ~mask;
    }

    inline bool GetBit(u64* bitmap, u32 bit) {
        u32 index = bit / 64;
        // Bounds check to prevent buffer overflow on high core count systems
        if (index >= MAX_BITMAP_SLOTS) {
            return false;  // Core ID exceeds supported range, treat as uninitialized
        }
        u64 mask = 1ULL << (bit % 64);
        return (bitmap[index] & mask) != 0;
    }
}

//===----------------------------------------------------------------------===//
// Intel Architecture Callbacks for Dispatch Layer
//===----------------------------------------------------------------------===//

static u64 IntelGetGuestCr3(void* /* arch_data */) {
    u64 guest_dirbase;
    __vmx_vmread(VMCS_GUEST_CR3, &guest_dirbase);
    return cr3{ guest_dirbase }.pml4_pfn << 12;
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

    // Note: EPT TLB invalidation happens automatically on VMRESUME
    // when EPTP changes. Explicit INVEPT not needed here.

    // Mark this core as having custom EPT
    u64* storage_arr = storage::GetStorageArray();
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

static u64 IntelGetVmcb(void* /* arch_data */) {
    // VMCB is AMD-only concept, return 0 on Intel
    return 0;
}

static void IntelFlushEptTlb(void* /* arch_data */) {
    // EPT TLB invalidation is automatic on EPTP change
    // For explicit flush, would need INVEPT instruction
}

static void IntelSetEptInitBit(void* /* arch_data */, u32 core_id, bool value) {
    bitmap::SetBit(&storage::GetStorageArray()[EPT_OS_INIT_BITMAP], core_id, value);
}

static bool IntelGetEptInitBit(void* /* arch_data */, u32 core_id) {
    return bitmap::GetBit(&storage::GetStorageArray()[EPT_OS_INIT_BITMAP], core_id);
}

static void RegisterIntelCallbacks() {
    ombra::ArchCallbacks callbacks = {};
    callbacks.GetGuestCr3 = IntelGetGuestCr3;
    callbacks.GetHostCr3 = IntelGetHostCr3;
    callbacks.GetEptBase = IntelGetEptBase;
    callbacks.SetEptBase = IntelSetEptBase;
    callbacks.EnableEpt = IntelEnableEpt;
    callbacks.DisableEpt = IntelDisableEpt;
    callbacks.GetVmcb = IntelGetVmcb;
    callbacks.FlushEptTlb = IntelFlushEptTlb;
    callbacks.SetEptInitBit = IntelSetEptInitBit;
    callbacks.GetEptInitBit = IntelGetEptInitBit;

    core::RegisterArchCallbacks(callbacks);
}

//===----------------------------------------------------------------------===//
// VMExit Command Helpers
//===----------------------------------------------------------------------===//

namespace vmexit {

auto get_command(guest_virt_t command_ptr) -> command_t
{
    u64 guest_dirbase;
    __vmx_vmread(VMCS_GUEST_CR3, &guest_dirbase);

    // CR3 can contain PCID bits, extract just the PML4 PFN
    guest_dirbase = cr3{ guest_dirbase }.pml4_pfn << 12;

    const auto command_page = core::mm::map_guest_virt(guest_dirbase, command_ptr);
    return *reinterpret_cast<command_t*>(command_page);
}

auto set_command(guest_virt_t command_ptr, command_t& command_data) -> void
{
    u64 guest_dirbase;
    __vmx_vmread(VMCS_GUEST_CR3, &guest_dirbase);

    // CR3 can contain PCID bits, extract just the PML4 PFN
    guest_dirbase = cr3{ guest_dirbase }.pml4_pfn << 12;

    const auto command_page = core::mm::map_guest_virt(guest_dirbase, command_ptr);
    *reinterpret_cast<command_t*>(command_page) = command_data;
}

// Get guest CR3 (extracted, no PCID bits)
auto get_guest_cr3() -> u64
{
    u64 guest_dirbase;
    __vmx_vmread(VMCS_GUEST_CR3, &guest_dirbase);
    return cr3{ guest_dirbase }.pml4_pfn << 12;
}

} // namespace vmexit

//===----------------------------------------------------------------------===//
// Get Command Data (matches AMD GetCommand pattern)
//===----------------------------------------------------------------------===//

static COMMAND_DATA GetCommand(u64 pCmd)
{
    COMMAND_DATA cmd = { 0 };
    u64 guest_cr3 = vmexit::get_guest_cr3();
    core::mm::copy_guest_virt(guest_cr3, pCmd, __readcr3(), (u64)&cmd, sizeof(cmd));
    return cmd;
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

        // Initialize identity mapping for memory operations
        core::mm::init();

        // Initialize CPUID spoofing for antidetection
        cpuid_spoof::Initialize();

        // Register Intel-specific callbacks for dispatch layer
        RegisterIntelCallbacks();
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
// CPUID Handler - Hypercall Dispatch via Unified Layer
//===----------------------------------------------------------------------===//

static bool HandleCpuid(pcontext_t guest_registers)
{
    // Get CPUID leaf/subleaf from guest registers
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
        ctx.extra_param = guest_registers->r8;    // R8 = target CR3 or extra param
        ctx.auth_key = guest_registers->r9;       // R9 = authentication key

        // Store arch-specific data for callbacks (Intel doesn't need VMCB pointer)
        ctx.arch_data = nullptr;
        ctx.arch_data2 = guest_registers;

        // Dispatch to unified handler
        VMX_ROOT_ERROR result = core::HandleVmcall(&ctx);

        // Check if this was a valid authenticated vmcall
        if (result != VMX_ROOT_ERROR::INVALID_GUEST_PARAM || vmcall::IsVmcall(ctx.auth_key)) {
            // Valid hypercall - set return value and mark that we've received a vmcall
            guest_registers->rax = static_cast<u64>(result);
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

    // Return spoofed values to guest
    guest_registers->rax = static_cast<u64>(eax);
    guest_registers->rbx = static_cast<u64>(ebx);
    guest_registers->rcx = static_cast<u64>(ecx);
    guest_registers->rdx = static_cast<u64>(edx);

    return true;  // We handle ALL CPUID - don't chain to original Hyper-V handler
}

//===----------------------------------------------------------------------===//
// Main VMExit Handler
//===----------------------------------------------------------------------===//

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

    //===----------------------------------------------------------------------===//
    // Handle RDMSR - Virtualize APERF/MPERF for Anti-Timing Detection
    // ESEA uses these MSRs to detect hypervisor overhead via IET analysis
    //===----------------------------------------------------------------------===//
    if (vmexit_reason == static_cast<size_t>(Vmx::VmxExitReason::EXIT_REASON_RDMSR))
    {
        u32 msr_id = static_cast<u32>(guest_registers->rcx);

        // Check for performance monitoring MSRs
        if (msr_id == 0xE7 || msr_id == 0xE8) {
            // IA32_MPERF (0xE7) or IA32_APERF (0xE8)
            // Return compensated value that hides hypervisor overhead
            u64 fake_value = timing::ReadMsrVirtualized(msr_id);
            guest_registers->rax = fake_value & 0xFFFFFFFF;
            guest_registers->rdx = fake_value >> 32;
            bHandledExit = true;
            bIncRip = true;
        }
        // If not APERF/MPERF, fall through to original handler
    }

    //===----------------------------------------------------------------------===//
    // Handle VMX Instructions - Inject #UD for Anti-Detection
    // EAC probes vmread/vmwrite from Ring 0 to detect hypervisors
    // On bare metal (VT-x disabled), these cause #UD - we must emulate this
    //===----------------------------------------------------------------------===//
    if (vmexit_reason == static_cast<size_t>(Vmx::VmxExitReason::EXIT_REASON_VMREAD) ||
        vmexit_reason == static_cast<size_t>(Vmx::VmxExitReason::EXIT_REASON_VMWRITE) ||
        vmexit_reason == static_cast<size_t>(Vmx::VmxExitReason::EXIT_REASON_VMCALL) ||
        vmexit_reason == static_cast<size_t>(Vmx::VmxExitReason::EXIT_REASON_VMXON) ||
        vmexit_reason == static_cast<size_t>(Vmx::VmxExitReason::EXIT_REASON_VMXOFF) ||
        vmexit_reason == static_cast<size_t>(Vmx::VmxExitReason::EXIT_REASON_VMPTRLD) ||
        vmexit_reason == static_cast<size_t>(Vmx::VmxExitReason::EXIT_REASON_VMPTRST) ||
        vmexit_reason == static_cast<size_t>(Vmx::VmxExitReason::EXIT_REASON_VMCLEAR) ||
        vmexit_reason == static_cast<size_t>(Vmx::VmxExitReason::EXIT_REASON_VMLAUNCH) ||
        vmexit_reason == static_cast<size_t>(Vmx::VmxExitReason::EXIT_REASON_VMRESUME))
    {
        // Inject #UD (exception vector 6) to simulate bare-metal behavior
        // This defeats EAC's vmread probing technique
        Vmx::VmentryInterruptionInformation info = {};
        info.fields.VectorOfInterruptOrException = 6;  // #UD
        info.fields.InterruptionType = static_cast<unsigned int>(Vmx::InterruptionType::HardwareException);
        info.fields.DeliverErrorCode = 0;  // #UD has no error code
        info.fields.Valid = 1;

        __vmx_vmwrite(static_cast<size_t>(Vmx::VmcsFieldEncoding::VMCS_FIELD_VMENTRY_INTERRUPTION_INFORMATION_FIELD), info.val);

        // Do NOT advance RIP - exception handler expects RIP at faulting instruction
        bHandledExit = true;
        bIncRip = false;
    }

    //===----------------------------------------------------------------------===//
    // Handle EPT Violations
    //===----------------------------------------------------------------------===//
    if (vmexit_reason == VMX_EXIT_REASON_EPT_VIOLATION)
    {
        u64* storage_arr = storage::GetStorageArray();
        u64 ept_handler_addr = storage::Query(EPT_HANDLER_ADDRESS);

        if (ept_handler_addr && bitmap::GetBit(&storage_arr[EPT_OS_INIT_BITMAP], CPU::ApicId()))
        {
            // Get faulting guest physical address
            size_t fault_pa;
            __vmx_vmread(VMCS_GUEST_PHYSICAL_ADDRESS, &fault_pa);

            // Call registered EPT handler
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

        //=====================================================================
        // TIMING: Adjust TSC offset before VMResume
        // Subtracts VMExit overhead to hide from guest timing measurements
        //=====================================================================
        timing::OnExitComplete(nullptr);  // Intel uses VMCS, not arch_data

        return;
    }

    // For unhandled exits, call the original Hyper-V handler
    // Note: Don't adjust timing here - original handler will do its own return
    reinterpret_cast<vmexit_handler_t>(
        reinterpret_cast<u64>(&vmexit_handler) -
        ombra_context.vmexit_handler_rva)(context, unknown);
}

} // namespace intel
