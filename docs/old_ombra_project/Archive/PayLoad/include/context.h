// PayLoad/include/context.h
// VmExitContext - The core abstraction layer between backends and dispatch
#pragma once

#include "types.h"
#include "vmcall.h"

namespace ombra {

//===----------------------------------------------------------------------===//
// Architecture-Specific Callbacks
//===----------------------------------------------------------------------===//
//
// These function pointers allow the dispatch layer to call arch-specific
// operations without knowing whether we're on Intel VMX or AMD SVM.
// Each backend registers its implementations during initialization.
//

struct ArchCallbacks {
    // Get guest CR3 (must mask PCID bits if present)
    u64 (*GetGuestCr3)(void* arch_data);

    // Get host/root CR3
    u64 (*GetHostCr3)(void* arch_data);

    // Get EPT/NPT base pointer
    u64 (*GetEptBase)(void* arch_data);

    // Set EPT/NPT base pointer
    VMX_ROOT_ERROR (*SetEptBase)(void* arch_data, u64 new_base);

    // Enable EPT/NPT
    VMX_ROOT_ERROR (*EnableEpt)(void* arch_data);

    // Disable EPT/NPT
    VMX_ROOT_ERROR (*DisableEpt)(void* arch_data);

    // Get VMCB physical address (AMD only, returns 0 on Intel)
    u64 (*GetVmcb)(void* arch_data);

    // Flush TLB for EPT/NPT changes
    void (*FlushEptTlb)(void* arch_data);

    // Set bitmap bit for EPT initialization tracking
    void (*SetEptInitBit)(void* arch_data, u32 core_id, bool value);

    // Get bitmap bit for EPT initialization tracking
    bool (*GetEptInitBit)(void* arch_data, u32 core_id);
};

// Global callback table - set by first backend to initialize
extern ArchCallbacks g_arch_callbacks;

//===----------------------------------------------------------------------===//
// VmExitContext - The Core Abstraction
//===----------------------------------------------------------------------===//
//
// This structure bridges architecture-specific VMExit handling (Intel VMX / AMD SVM)
// with the shared command dispatch logic in core/dispatch.cpp.
//
// The backend (intel/vmx_handler.cpp or amd/svm_handler.cpp) populates this
// structure, then calls core::HandleVmcall(). The dispatch code uses only
// this abstraction - it never touches VMCS or VMCB directly.
//

struct VmExitContext {
    //=== Guest State (populated by backend) ===

    // Guest CR3 value (physical address of PML4)
    // Backend must mask off PCID bits if present
    u64 guest_cr3;

    // Guest RIP at time of VMExit
    u64 guest_rip;

    // Pointer to guest RAX for setting return value
    // On AMD: &vmcb->Rax()
    // On Intel: &guest_context->rax (from VMCS_GUEST_RAX)
    u64* p_rax;

    //=== Hypercall Parameters (from guest registers) ===

    // RCX = VMCALL_TYPE command code
    u64 cmd_type;

    // RDX = pointer to COMMAND_DATA in guest virtual memory
    // Will be read via mm::copy_guest_virt to local_cmd
    u64 cmd_guest_va;

    // R8 = extra parameter (typically target CR3 for cross-process ops)
    u64 extra_param;

    // R9 = authentication key
    u64 auth_key;

    //=== Backend-Specific Data ===

    // Opaque pointer to arch-specific context
    // AMD: svm::Vmcb*
    // Intel: pcontext_t* (guest context from VMExit)
    // Core dispatch code MUST NOT dereference this
    void* arch_data;

    // Second opaque pointer for additional arch-specific data
    // AMD: svm::pguest_context
    // Intel: unused (nullptr)
    void* arch_data2;

    //=== Cached Command Data ===

    // Local copy of COMMAND_DATA read from guest memory
    COMMAND_DATA local_cmd;

    // Whether local_cmd has been populated
    bool cmd_cached;

    //=== Helper Methods ===

    // Set return value in guest RAX
    void SetResult(VMX_ROOT_ERROR result) {
        if (p_rax) {
            *p_rax = static_cast<u64>(result);
        }
    }

    // Get command type as enum
    VMCALL_TYPE GetCommandType() const {
        return static_cast<VMCALL_TYPE>(cmd_type);
    }

    // Initialize context to known state
    void Reset() {
        guest_cr3 = 0;
        guest_rip = 0;
        p_rax = nullptr;
        cmd_type = 0;
        cmd_guest_va = 0;
        extra_param = 0;
        auth_key = 0;
        arch_data = nullptr;
        arch_data2 = nullptr;
        cmd_cached = false;
    }
};

} // namespace ombra
