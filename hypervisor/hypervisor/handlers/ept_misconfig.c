// ept_misconfig.c — EPT Misconfiguration Handler
// OmbraHypervisor

#include "handlers.h"
#include "../vmx.h"
#include "../ept.h"
#include "../debug.h"
#include "../timing.h"
#include "../../shared/vmcs_fields.h"
#include "../../shared/ept_defs.h"

// =============================================================================
// EPT Misconfiguration (Exit Reason 49)
// =============================================================================
//
// An EPT misconfiguration occurs when the processor encounters an EPT entry
// with reserved bits set or an invalid combination of permissions.
//
// Intel SDM Vol 3, Section 28.2.3.1 - EPT Misconfigurations
//
// Common causes:
// 1. Reserved bits set in EPT entry (bits 63:52, bits 2:0 specific patterns)
// 2. Invalid memory type in EPT PTE (bits 5:3)
// 3. Large page with non-zero lower address bits
// 4. Write permission without read permission (W=1, R=0)
// 5. Leaf entry at wrong level (e.g., page table entry with large-page bit)
//
// Unlike EPT violations (which are normal during operation), EPT misconfigurations
// indicate a BUG in the hypervisor's EPT setup. These should NEVER happen in
// production if EPT is configured correctly.
//
// =============================================================================

// Helper: Decode EPT entry permissions
static const char* EptPermStr(U64 entry) {
    static char buf[16];
    buf[0] = (entry & EPT_READ) ? 'R' : '-';
    buf[1] = (entry & EPT_WRITE) ? 'W' : '-';
    buf[2] = (entry & EPT_EXECUTE) ? 'X' : '-';
    buf[3] = '\0';
    return buf;
}

// Helper: Get EPT memory type name
static const char* EptMemTypeStr(U64 entry) {
    if (!(entry & EPT_READ)) return "N/A";  // Not a leaf entry

    U64 memType = (entry >> 3) & 0x7;  // Bits 5:3
    switch (memType) {
    case 0: return "UC";   // Uncacheable
    case 1: return "WC";   // Write Combining
    case 4: return "WT";   // Write Through
    case 5: return "WP";   // Write Protected
    case 6: return "WB";   // Write Back
    default: return "INVALID";
    }
}

// Helper: Walk EPT to find the misconfigured entry
static void DumpEptWalk(U64 guestPhysical) {
    VMX_CPU* cpu = VmxGetCurrentCpu();
    if (!cpu || !cpu->Ept) {
        ERR("  Cannot dump EPT walk: No EPT pointer available");
        return;
    }

    EPT_STATE* ept = cpu->Ept;

    // Extract indices from guest physical address
    U64 pml4Index = (guestPhysical >> 39) & 0x1FF;
    U64 pdptIndex = (guestPhysical >> 30) & 0x1FF;
    U64 offset = guestPhysical & 0x3FFFFFFF;  // Offset within 1GB

    ERR("  EPT Walk for GPA 0x%llx:", guestPhysical);
    ERR("    Indices: PML4[%llu] PDPT[%llu] offset=0x%llx",
        pml4Index, pdptIndex, offset);

    // PML4 Entry
    if (!ept->Pml4 || pml4Index >= EPT_ENTRIES_PER_TABLE) {
        ERR("    PML4 table invalid or index out of range");
        return;
    }

    U64 pml4e = ept->Pml4[pml4Index].Value;
    ERR("    PML4E[%llu] = 0x%llx (%s)", pml4Index, pml4e, EptPermStr(pml4e));

    if (!(pml4e & EPT_READ)) {
        ERR("      ^ NOT PRESENT - This is the misconfiguration!");
        return;
    }

    // PDPT Entry
    if (!ept->Pdpt || pdptIndex >= EPT_ENTRIES_PER_TABLE) {
        ERR("    PDPT table invalid or index out of range");
        return;
    }

    U64 pdpte = ept->Pdpt[pdptIndex].Value;
    ERR("    PDPTE[%llu] = 0x%llx (%s)", pdptIndex, pdpte, EptPermStr(pdpte));

    if (!(pdpte & EPT_READ)) {
        ERR("      ^ NOT PRESENT - This is the misconfiguration!");
        return;
    }

    // Check for 1GB page (our default configuration)
    if (pdpte & EPT_LARGE_PAGE) {
        ERR("      ^ 1GB LARGE PAGE (memtype=%s)", EptMemTypeStr(pdpte));
        ERR("      ^ MISCONFIGURATION: Check reserved bits or invalid memtype!");
        ERR("      ^ Common issues:");
        ERR("        - Write-only page (W=1, R=0) - INVALID");
        ERR("        - Reserved bits 29:12 must be zero for 1GB pages");
        ERR("        - Invalid memory type (bits 5:3)");
        return;
    }

    // If not a 1GB page, it points to PD (not implemented in our simple EPT)
    ERR("    PDPTE points to PD table (not implemented in basic 1GB page setup)");
    ERR("    This indicates EPT was modified or page was split for hooks");
    ERR("    ^ MISCONFIGURATION: Likely in PD/PT entries (not accessible here)");
}

VMEXIT_ACTION HandleEptMisconfiguration(GUEST_REGS* regs, U64 qualification) {
    U64 entryTsc = TimingStart();
    VMX_CPU* cpu = VmxGetCurrentCpu();
    U64 guestPhysical;
    U64 guestRip;

    (void)regs;
    (void)qualification;  // Exit qualification is not used for EPT misconfig

    // Get the guest physical address that caused the misconfiguration
    guestPhysical = VmcsRead(VMCS_EXIT_GUEST_PHYSICAL);
    guestRip = VmcsRead(VMCS_GUEST_RIP);

    // =============================================================================
    // EPT Misconfiguration is FATAL
    // =============================================================================
    //
    // This indicates a serious bug in our EPT setup code. Common issues:
    //
    // 1. Write-only pages (W=1, R=0) - INVALID, must have R=1 if W=1
    // 2. Reserved bits set in EPT entries
    // 3. Invalid memory type (bits 5:3 have reserved encoding)
    // 4. Large page bit set with non-zero lower address bits
    // 5. Corrupt EPT structures
    //
    // Unlike EPT violations, we CANNOT recover from this gracefully.
    // The only fix is to correct the EPT setup code.
    //
    // =============================================================================

    ERR("================================================================================");
    ERR("EPT MISCONFIGURATION - FATAL BUG IN HYPERVISOR EPT SETUP");
    ERR("================================================================================");
    ERR("Guest Physical Address: 0x%llx", guestPhysical);
    ERR("Guest RIP: 0x%llx", guestRip);
    ERR("");

    // Dump the EPT walk to show exactly which entry is misconfigured
    DumpEptWalk(guestPhysical);

    ERR("");
    ERR("Common causes:");
    ERR("  1. Write permission without read (W=1, R=0) - MUST have R=1");
    ERR("  2. Invalid memory type in bits 5:3");
    ERR("  3. Reserved bits set (63:52 or specific combinations)");
    ERR("  4. Large page with non-aligned address");
    ERR("  5. Corrupt EPT table entry");
    ERR("");
    ERR("FIX: Correct the EPT setup code in ept.c");
    ERR("================================================================================");

    // This is unrecoverable - shutdown the VM
    FATAL("EPT misconfiguration at GPA 0x%llx - cannot continue", guestPhysical);

    if (cpu) TimingEnd(cpu, entryTsc, TIMING_MISCONFIG_OVERHEAD);
    return VMEXIT_SHUTDOWN;
}
