// vmcall.c — VMCALL Exit Handler
// OmbraHypervisor

#include "handlers.h"
#include "../vmx.h"
#include "../../shared/vmcs_fields.h"

// =============================================================================
// VMCALL Interface Protocol
// =============================================================================
//
// To distinguish our VMCALLs from other hypervisor VMCALLs (e.g., Hyper-V),
// we use a magic prefix:
//
// RAX = Magic signature (OMBRA_VMCALL_MAGIC)
// RCX = Command code
// RDX = Parameter 1 (command-specific)
// R8  = Parameter 2 (command-specific)
// R9  = Parameter 3 (command-specific)
//
// Returns:
// RAX = Status code (0 = success, negative = error)
// RDX = Return value 1 (command-specific)
// R8  = Return value 2 (command-specific)

#define OMBRA_VMCALL_MAGIC      0x4F4D4252414C4C00ULL  // "OMBRALL\0"

// Command codes
#define VMCALL_PING             0x0001
#define VMCALL_UNLOAD           0x0002
#define VMCALL_GET_STATUS       0x0003
#define VMCALL_HOOK_INSTALL     0x0010
#define VMCALL_HOOK_REMOVE      0x0011
#define VMCALL_HOOK_LIST        0x0012
#define VMCALL_READ_PHYS        0x0020
#define VMCALL_WRITE_PHYS       0x0021
#define VMCALL_VIRT_TO_PHYS     0x0022

// Status codes
#define VMCALL_STATUS_SUCCESS           0
#define VMCALL_STATUS_INVALID_MAGIC     -1
#define VMCALL_STATUS_INVALID_COMMAND   -2
#define VMCALL_STATUS_INVALID_PARAM     -3
#define VMCALL_STATUS_NOT_IMPLEMENTED   -4
#define VMCALL_STATUS_ACCESS_DENIED     -5

// =============================================================================
// VMCALL Handlers
// =============================================================================

static I64 VmcallPing(GUEST_REGS* regs) {
    // PING - Return signature and version
    // Input: None
    // Output: RDX = version (0x00010000 = 1.0)
    //         R8  = feature flags

    regs->Rdx = 0x00010000;  // Version 1.0
    regs->R8 = 0;            // No special features yet

    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallGetStatus(GUEST_REGS* regs) {
    // GET_STATUS - Return hypervisor status
    // Input: None
    // Output: RDX = number of virtualized CPUs
    //         R8  = total VM-exit count (this CPU)

    VMX_CPU* cpu = VmxGetCurrentCpu();

    // Count virtualized CPUs
    U32 count = 0;
    for (U32 i = 0; i < MAX_CPUS; i++) {
        if (g_Ombra.Cpus[i] && g_Ombra.Cpus[i]->Virtualized) {
            count++;
        }
    }

    regs->Rdx = count;
    regs->R8 = cpu ? cpu->VmexitCount : 0;

    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallHookInstall(GUEST_REGS* regs) {
    // HOOK_INSTALL - Install an EPT hook
    // Input: RDX = target physical address
    //        R8  = hook function physical address
    //        R9  = flags
    // Output: RDX = hook ID (or 0 on failure)

    (void)regs;
    // TODO: Implement when EPT hook framework is ready
    return VMCALL_STATUS_NOT_IMPLEMENTED;
}

static I64 VmcallHookRemove(GUEST_REGS* regs) {
    // HOOK_REMOVE - Remove an EPT hook
    // Input: RDX = hook ID
    // Output: None

    (void)regs;
    // TODO: Implement when EPT hook framework is ready
    return VMCALL_STATUS_NOT_IMPLEMENTED;
}

static I64 VmcallReadPhys(GUEST_REGS* regs) {
    // READ_PHYS - Read from physical memory
    // Input: RDX = physical address
    //        R8  = size (1, 2, 4, or 8)
    // Output: RDX = value read

    U64 physAddr = regs->Rdx;
    U64 size = regs->R8;

    // For now, we need a way to map physical addresses
    // This is a stub - real implementation needs physical-to-virtual mapping
    (void)physAddr;
    (void)size;

    return VMCALL_STATUS_NOT_IMPLEMENTED;
}

static I64 VmcallWritePhys(GUEST_REGS* regs) {
    // WRITE_PHYS - Write to physical memory
    // Input: RDX = physical address
    //        R8  = size (1, 2, 4, or 8)
    //        R9  = value to write
    // Output: None

    (void)regs;
    // TODO: Implement with proper physical memory access
    return VMCALL_STATUS_NOT_IMPLEMENTED;
}

static I64 VmcallVirtToPhys(GUEST_REGS* regs) {
    // VIRT_TO_PHYS - Convert guest virtual to physical address
    // Input: RDX = guest virtual address
    // Output: RDX = guest physical address

    (void)regs;
    // TODO: Walk guest page tables
    return VMCALL_STATUS_NOT_IMPLEMENTED;
}

// =============================================================================
// Main VMCALL Handler
// =============================================================================

VMEXIT_ACTION HandleVmcall(GUEST_REGS* regs) {
    U64 magic = regs->Rax;
    U64 command = regs->Rcx;
    I64 status;

    // Check magic signature
    if (magic != OMBRA_VMCALL_MAGIC) {
        // Not our VMCALL - could be Hyper-V or other hypervisor
        // Pass through or inject #UD

        // For stealth, pretend VMCALL doesn't exist
        // In reality, we should check if Hyper-V is expected
        regs->Rax = (U64)VMCALL_STATUS_INVALID_MAGIC;
        return VMEXIT_ADVANCE_RIP;
    }

    // Dispatch command
    switch (command) {
    case VMCALL_PING:
        status = VmcallPing(regs);
        break;

    case VMCALL_UNLOAD:
        // Unload requested - signal shutdown
        regs->Rax = VMCALL_STATUS_SUCCESS;
        return VMEXIT_SHUTDOWN;

    case VMCALL_GET_STATUS:
        status = VmcallGetStatus(regs);
        break;

    case VMCALL_HOOK_INSTALL:
        status = VmcallHookInstall(regs);
        break;

    case VMCALL_HOOK_REMOVE:
        status = VmcallHookRemove(regs);
        break;

    case VMCALL_READ_PHYS:
        status = VmcallReadPhys(regs);
        break;

    case VMCALL_WRITE_PHYS:
        status = VmcallWritePhys(regs);
        break;

    case VMCALL_VIRT_TO_PHYS:
        status = VmcallVirtToPhys(regs);
        break;

    default:
        status = VMCALL_STATUS_INVALID_COMMAND;
        break;
    }

    regs->Rax = (U64)status;
    return VMEXIT_ADVANCE_RIP;
}
