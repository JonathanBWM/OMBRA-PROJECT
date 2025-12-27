// mtf.c — Monitor Trap Flag Handler
// OmbraHypervisor
//
// Handles MTF (Monitor Trap Flag) VM-exits for shadow hook support.
// MTF causes a VM-exit after the guest executes exactly one instruction,
// similar to single-stepping in a debugger.
//
// Shadow hooks use MTF to temporarily switch memory views:
// 1. Page is normally execute-only (shadow page with hook visible)
// 2. On read/write EPT violation, switch to original page (RW allowed, X denied)
// 3. Enable MTF to single-step one instruction
// 4. On MTF exit, restore execute-only shadow view
//
// This makes hooks completely invisible to memory scanners and integrity checks.

#include "handlers.h"
#include "../vmx.h"
#include "../hooks.h"
#include "../debug.h"
#include "../../shared/vmcs_fields.h"

// =============================================================================
// MTF Handler
// =============================================================================

VMEXIT_ACTION HandleMtf(GUEST_REGS* regs) {
    U64 guestRip;
    VMX_CPU* cpu;

    (void)regs;  // Not used for MTF handling

    // Get current guest RIP
    guestRip = VmcsRead(VMCS_GUEST_RIP);

    cpu = VmxGetCurrentCpu();

    TRACE("MTF exit at RIP=0x%llx", guestRip);

    // Call hook manager's MTF handler to restore shadow view
    extern HOOK_MANAGER g_HookManager;
    if (g_HookManager.Initialized) {
        HookHandleMtf(&g_HookManager, guestRip);
    }

    // Continue execution - don't advance RIP
    // The guest will execute the next instruction normally
    return VMEXIT_CONTINUE;
}
