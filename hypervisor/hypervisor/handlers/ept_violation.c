// ept_violation.c — EPT Violation Handler
// OmbraHypervisor

#include "handlers.h"
#include "../vmx.h"
#include "../ept.h"
#include "../hooks.h"
#include "../debug.h"
#include "../../shared/vmcs_fields.h"
#include "../../shared/ept_defs.h"

// =============================================================================
// EPT Violation Exit Qualification (Intel SDM Vol 3, Table 27-7)
// =============================================================================
//
// Bit 0  - Data read caused violation
// Bit 1  - Data write caused violation
// Bit 2  - Instruction fetch caused violation
// Bit 3  - EPT entry read permission (1 if readable)
// Bit 4  - EPT entry write permission (1 if writable)
// Bit 5  - EPT entry execute permission (1 if executable)
// Bit 6  - EPT entry user-execute permission (if mode-based EPT)
// Bit 7  - Guest-physical address valid
// Bit 8  - Linear address valid (if guest paging is on)
// Bit 9  - Guest-linear-address access (vs physical access)
// Bit 10 - NMI unblocking (caused by IRET during NMI)
// Bit 11 - Shadow stack access
// Bit 12 - Supervisor shadow stack access

// =============================================================================
// EPT Violation Handler
// =============================================================================

VMEXIT_ACTION HandleEptViolation(GUEST_REGS* regs, U64 qualification) {
    U64 guestPhysical;
    U64 guestLinear;
    bool wasRead;
    bool wasWrite;
    bool wasExecute;
    bool isReadable;
    bool isWritable;
    bool isExecutable;
    VMX_CPU* cpu;

    (void)regs;  // May be used in hook handling

    // Get the guest physical address that caused the violation
    guestPhysical = VmcsRead(VMCS_EXIT_GUEST_PHYSICAL);
    guestLinear = VmcsRead(VMCS_EXIT_GUEST_LINEAR);

    // Parse exit qualification
    wasRead = (qualification & EPT_VIOLATION_READ) != 0;
    wasWrite = (qualification & EPT_VIOLATION_WRITE) != 0;
    wasExecute = (qualification & EPT_VIOLATION_EXECUTE) != 0;
    isReadable = (qualification & EPT_VIOLATION_READABLE) != 0;
    isWritable = (qualification & EPT_VIOLATION_WRITABLE) != 0;
    isExecutable = (qualification & EPT_VIOLATION_EXECUTABLE) != 0;

    cpu = VmxGetCurrentCpu();

    // =========================================================================
    // Hook Handler Check
    // =========================================================================
    //
    // When we have EPT hooks installed:
    // 1. Execute-only pages cause read/write violations
    // 2. We switch to a shadow page that allows the operation
    // 3. Single-step and switch back for execute-only hook

    // Get hook manager from global state (would need proper initialization)
    // For now, this is a placeholder - actual integration requires passing
    // the hook manager through the CPU structure
    //
    // if (cpu && cpu->Ept) {
    //     extern HOOK_MANAGER g_HookManager;
    //     if (HookHandleEptViolation(&g_HookManager, guestPhysical, qualification, regs)) {
    //         return VMEXIT_CONTINUE;  // Hook handled it
    //     }
    // }

    // =========================================================================
    // MMIO Region Check
    // =========================================================================
    //
    // Some physical addresses are MMIO regions that need special handling:
    // - APIC (0xFEE00000)
    // - HPET (0xFED00000)
    // - PCI config space (0xE0000000+ region)
    //
    // Our 512GB identity map covers all of these with RWX, so we shouldn't
    // get violations for normal MMIO access. If we do, it's unexpected.

    // =========================================================================
    // Unexpected Violation Handling
    // =========================================================================
    //
    // If we get here with our identity map and no hooks, something is wrong.
    // This could happen if:
    // 1. Guest accesses memory above 512GB (not mapped)
    // 2. EPT misconfiguration
    // 3. Bug in our code
    //
    // For debugging, log the violation details.

    (void)guestLinear;
    (void)wasRead;
    (void)wasWrite;
    (void)wasExecute;
    (void)isReadable;
    (void)isWritable;
    (void)isExecutable;
    (void)cpu;

    // Check if this is beyond our 512GB identity map
    if (guestPhysical >= EPT_IDENTITY_MAP_SIZE) {
        // Guest tried to access unmapped memory
        ERR("EPT violation: access beyond 512GB map (GPA=0x%llx)", guestPhysical);
        // This is a guest bug - inject #PF or return error
        // For now, just advance RIP (will likely cause guest crash)
        return VMEXIT_ADVANCE_RIP;
    }

    // For a properly configured identity map, we shouldn't get EPT violations
    // on normal accesses. If we do, it indicates a problem or a hook.
    WARN("Unexpected EPT violation: GPA=0x%llx GLA=0x%llx Qual=0x%llx (R=%d W=%d X=%d)",
         guestPhysical, guestLinear, qualification,
         wasRead, wasWrite, wasExecute);

    // Re-execute the instruction
    // (The identity map should allow it next time)
    return VMEXIT_CONTINUE;
}
