// PayLoad/amd/exception.cpp
// AMD SVM Exception Handling Implementation
// Provides SEH-like exception recovery in vmxroot context

#include "exception.h"

// Forward declaration of vmexit_handler for recovery (matches svm_handler.h)
namespace amd {
auto vmexit_handler(void* unknown, void* unknown2, pguest_context context) -> pgs_base_struct;
}

//===----------------------------------------------------------------------===//
// Global IDT and Register
//===----------------------------------------------------------------------===//

IDT exception::HostIdt = { 0 };
Seg::DescriptorTableRegister<Seg::Mode::longMode> exception::IdtReg = { 0 };

//===----------------------------------------------------------------------===//
// Per-Core Parameter Storage for Recovery
//===----------------------------------------------------------------------===//

struct _HYPERV_EXIT_HANDLER_PARAMS {
    void* unknown;
    void* unknown2;
    amd::pguest_context context;
    Seg::DescriptorTableRegister<Seg::Mode::longMode> idt;
    void* rsp;
};

// Match the 576 core limit from EPT bitmap (9 slots x 64 bits)
// This supports modern high-core-count systems (AMD EPYC, Intel Xeon)
constexpr u32 MAX_SUPPORTED_CORES = 576;
inline static _HYPERV_EXIT_HANDLER_PARAMS CoreParams[MAX_SUPPORTED_CORES] = { 0 };

//===----------------------------------------------------------------------===//
// Parameter Saving
//===----------------------------------------------------------------------===//

void exception::SaveOrigParams(
    void* unknown,
    void* unknown2,
    amd::pguest_context context,
    Seg::DescriptorTableRegister<Seg::Mode::longMode> idt,
    void* rsp)
{
    auto core = CPU::ApicId();

    // Bounds check to prevent buffer overflow on high core count systems
    if (core >= MAX_SUPPORTED_CORES) {
        return;  // Core ID exceeds supported range, cannot save params
    }

    CoreParams[core].unknown = unknown;
    CoreParams[core].unknown2 = unknown2;
    CoreParams[core].context = context;
    CoreParams[core].idt = idt;
    CoreParams[core].rsp = rsp;
}

//===----------------------------------------------------------------------===//
// Exception Handler with Error Code
//===----------------------------------------------------------------------===//

void exception::seh_handler_ecode_vm(PIDT_REGS_ECODE regs)
{
    auto rva = regs->rip - amd::ombra_context.record_base;
    auto nt_headers = reinterpret_cast<IMAGE_NT_HEADERS64*>(
        amd::ombra_context.record_base +
        reinterpret_cast<IMAGE_DOS_HEADER*>(amd::ombra_context.record_base)->e_lfanew);

    auto exception =
        &nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];

    auto functions =
        reinterpret_cast<RUNTIME_FUNCTION*>(
            amd::ombra_context.record_base + exception->VirtualAddress);

    // Walk exception directory to find matching handler
    for (auto idx = 0; idx < exception->Size / sizeof(RUNTIME_FUNCTION); ++idx)
    {
        auto function = &functions[idx];
        if (!(rva >= function->BeginAddress && rva < function->EndAddress))
            continue;

        auto unwind_info =
            reinterpret_cast<UNWIND_INFO*>(
                amd::ombra_context.record_base + function->UnwindData);

        if (!(unwind_info->Flags & UNW_FLAG_EHANDLER))
            continue;

        auto scope_table =
            reinterpret_cast<SCOPE_TABLE*>(
                reinterpret_cast<UINT64>(&unwind_info->UnwindCode[
                    (unwind_info->CountOfCodes + 1) & ~1]) + sizeof(UINT32));

        // Find matching scope record
        for (UINT32 entry = 0; entry < scope_table->Count; ++entry)
        {
            auto scope_record = &scope_table->ScopeRecord[entry];
            if (rva >= scope_record->BeginAddress && rva < scope_record->EndAddress)
            {
                // Jump to exception handler (__except block)
                regs->rip = amd::ombra_context.record_base + scope_record->JumpTarget;
                return;
            }
        }
    }

    // No handler found - recover by calling original Hyper-V handler
    auto core = CPU::ApicId();

    // Bounds check to prevent buffer overflow on high core count systems
    if (core >= MAX_SUPPORTED_CORES) {
        // Cannot recover - halt execution
        while (1) { __halt(); }
    }

    regs->rcx = (UINT64)CoreParams[core].unknown;
    regs->rdx = (UINT64)CoreParams[core].unknown2;
    regs->r8 = (UINT64)CoreParams[core].context;
    regs->rsp = (UINT64)CoreParams[core].rsp;

    // Jump back to original handler
    regs->rip = reinterpret_cast<u64>(&amd::vmexit_handler) - amd::ombra_context.vcpu_run_rva;
    __lidt(&CoreParams[core].idt);
}

//===----------------------------------------------------------------------===//
// Exception Handler without Error Code
//===----------------------------------------------------------------------===//

void exception::seh_handler_vm(PIDT_REGS regs)
{
    auto rva = regs->rip - amd::ombra_context.record_base;
    auto nt_headers = reinterpret_cast<IMAGE_NT_HEADERS64*>(
        amd::ombra_context.record_base +
        reinterpret_cast<IMAGE_DOS_HEADER*>(amd::ombra_context.record_base)->e_lfanew);

    auto exception =
        &nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];

    auto functions =
        reinterpret_cast<RUNTIME_FUNCTION*>(
            amd::ombra_context.record_base + exception->VirtualAddress);

    // Walk exception directory to find matching handler
    for (auto idx = 0; idx < exception->Size / sizeof(RUNTIME_FUNCTION); ++idx)
    {
        auto function = &functions[idx];
        if (!(rva >= function->BeginAddress && rva < function->EndAddress))
            continue;

        auto unwind_info =
            reinterpret_cast<UNWIND_INFO*>(
                amd::ombra_context.record_base + function->UnwindData);

        if (!(unwind_info->Flags & UNW_FLAG_EHANDLER))
            continue;

        auto scope_table =
            reinterpret_cast<SCOPE_TABLE*>(
                reinterpret_cast<UINT64>(&unwind_info->UnwindCode[
                    (unwind_info->CountOfCodes + 1) & ~1]) + sizeof(UINT32));

        // Find matching scope record
        for (UINT32 entry = 0; entry < scope_table->Count; ++entry)
        {
            auto scope_record = &scope_table->ScopeRecord[entry];
            if (rva >= scope_record->BeginAddress && rva < scope_record->EndAddress)
            {
                // Jump to exception handler (__except block)
                regs->rip = amd::ombra_context.record_base + scope_record->JumpTarget;
                return;
            }
        }
    }

    // No handler found - recover by calling original Hyper-V handler
    auto core = CPU::ApicId();

    // Bounds check to prevent buffer overflow on high core count systems
    if (core >= MAX_SUPPORTED_CORES) {
        // Cannot recover - halt execution
        while (1) { __halt(); }
    }

    regs->rcx = (UINT64)CoreParams[core].unknown;
    regs->rdx = (UINT64)CoreParams[core].unknown2;
    regs->r8 = (UINT64)CoreParams[core].context;
    regs->rsp = (UINT64)CoreParams[core].rsp;

    // Jump back to original handler
    regs->rip = reinterpret_cast<u64>(&amd::vmexit_handler) - amd::ombra_context.vcpu_run_rva;
    __lidt(&CoreParams[core].idt);
}

//===----------------------------------------------------------------------===//
// Initialization
//===----------------------------------------------------------------------===//

void exception::Initialize()
{
    // Set up host IDT with our exception handlers
    HostIdt.setup(generic_interrupt_handler_vm, generic_interrupt_handler_ecode_vm);
    IdtReg.BaseAddress = (uintptr_t)HostIdt.get_address();
    IdtReg.Limit = HostIdt.get_limit();
}
