// PayLoad/intel/vmx_handler.h
// Intel VMX backend interface
// Authentication uses dynamic key via vmcall::IsVmcall() - set via VMCALL_SET_COMM_KEY
#pragma once

#include "../include/types.h"
#include "../include/context.h"

namespace intel {

//===----------------------------------------------------------------------===//
// Intel VMX Types
//===----------------------------------------------------------------------===//

// Guest context structure (Intel-style register layout)
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

// Original handler function type
using vmexit_handler_t = void (__fastcall*)(pcontext_t* context, void* unknown);

//===----------------------------------------------------------------------===//
// Ombra Context Structure
//===----------------------------------------------------------------------===//

#pragma pack(push, 1)
typedef struct _OMBRA_T
{
    u64 vmexit_handler_rva;
    u64 hyperv_module_base;
    u64 hyperv_module_size;
    u64 payload_base;
    u64 payload_size;
} OMBRA_T, *pOMBRA_T;
#pragma pack(pop)

// Global Ombra context - exported for boot chain
__declspec(dllexport) inline OMBRA_T ombra_context;

//===----------------------------------------------------------------------===//
// VMExit Command Types
// NOTE: Using VMCALL_TYPE enum from communication.hpp for consistency with AMD
// Old vmexit_command_t enum removed - hypercalls now use unified protocol
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Command Data Structure
//===----------------------------------------------------------------------===//

typedef union _command_t
{
    struct _copy_phys
    {
        host_phys_t  phys_addr;
        guest_virt_t buffer;
        u64 size;
    } copy_phys;

    struct _copy_virt
    {
        guest_phys_t dirbase_src;
        guest_virt_t virt_src;
        guest_phys_t dirbase_dest;
        guest_virt_t virt_dest;
        u64 size;
    } copy_virt;

    struct _translate_virt
    {
        guest_virt_t virt_src;
        guest_phys_t phys_addr;
    } translate_virt;

    guest_phys_t dirbase;

} command_t, *pcommand_t;

//===----------------------------------------------------------------------===//
// VMCS Field Definitions
//===----------------------------------------------------------------------===//

// Common VMCS fields we need to read
constexpr u64 VMCS_EXIT_REASON = 0x4402;
constexpr u64 VMCS_GUEST_RIP = 0x681E;
constexpr u64 VMCS_GUEST_CR3 = 0x6802;
constexpr u64 VMCS_VMEXIT_INSTRUCTION_LENGTH = 0x440C;

// VMExit reasons
constexpr u64 VMX_EXIT_REASON_EXECUTE_CPUID = 10;
constexpr u64 VMX_EXIT_REASON_EPT_VIOLATION = 48;

//===----------------------------------------------------------------------===//
// CR3 Structure for PCID handling
//===----------------------------------------------------------------------===//

typedef union _cr3
{
    u64 value;
    struct
    {
        u64 pcid : 12;
        u64 pml4_pfn : 36;
        u64 reserved : 16;
    };
} cr3;

//===----------------------------------------------------------------------===//
// VMExit Helper Functions
//===----------------------------------------------------------------------===//

namespace vmexit {
    // Get command data from guest virtual address
    auto get_command(guest_virt_t command_ptr) -> command_t;
    // Write command data back to guest
    auto set_command(guest_virt_t command_ptr, command_t& command_data) -> void;
}

//===----------------------------------------------------------------------===//
// Intel VMExit Handler
//===----------------------------------------------------------------------===//

// Main entry point - signature must match Hyper-V's hooked handler
// CRITICAL: Intel handler returns void (not pgs_base_struct*)
void vmexit_handler(pcontext_t* context, void* unknown);

// Initialize Intel backend
void Initialize();

} // namespace intel
