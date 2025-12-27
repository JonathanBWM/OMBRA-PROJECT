// shadow.c - EPT Shadow Page Requests
// OmbraDriver Phase 3
//
// Handles EPT page splitting and shadow hook installation via hypervisor VMCALLs

#include "context.h"
#include "command_ring.h"
#include "vmcall.h"

// =============================================================================
// Shadow Hook Tracking
// =============================================================================

#define MAX_SHADOW_HOOKS 64

typedef struct _SHADOW_HOOK {
    bool    Active;
    U64     Cr3;            // Target process CR3
    U64     GuestPhysical;  // Original guest physical address
    U64     CleanPhysical;  // Clean copy physical address
    U64     VirtualAddress; // Original virtual address (for lookup)
    U64     Size;           // Size of hooked region
} SHADOW_HOOK, *PSHADOW_HOOK;

static SHADOW_HOOK g_ShadowHooks[MAX_SHADOW_HOOKS] = {0};

// =============================================================================
// Internal Helpers
// =============================================================================

static PSHADOW_HOOK FindFreeSlot(void) {
    for (U32 i = 0; i < MAX_SHADOW_HOOKS; i++) {
        if (!g_ShadowHooks[i].Active) {
            return &g_ShadowHooks[i];
        }
    }
    return 0;
}

static PSHADOW_HOOK FindHookByVa(U64 cr3, U64 va) {
    for (U32 i = 0; i < MAX_SHADOW_HOOKS; i++) {
        if (g_ShadowHooks[i].Active &&
            g_ShadowHooks[i].Cr3 == cr3 &&
            va >= g_ShadowHooks[i].VirtualAddress &&
            va < g_ShadowHooks[i].VirtualAddress + g_ShadowHooks[i].Size) {
            return &g_ShadowHooks[i];
        }
    }
    return 0;
}

static PSHADOW_HOOK FindHookByGpa(U64 cr3, U64 gpa) {
    for (U32 i = 0; i < MAX_SHADOW_HOOKS; i++) {
        if (g_ShadowHooks[i].Active &&
            g_ShadowHooks[i].Cr3 == cr3 &&
            g_ShadowHooks[i].GuestPhysical == (gpa & ~0xFFFULL)) {
            return &g_ShadowHooks[i];
        }
    }
    return 0;
}

// =============================================================================
// Public API
// =============================================================================

// Install shadow hook on memory region
// This creates an execute-only view where:
// - Execute access sees the modified (hooked) page
// - Read/Write access sees the original (clean) page
I32 ShadowInstall(U64 cr3, U64 virtualAddress, U64 size) {
    if (size == 0) return OMBRA_STATUS_INVALID_PARAMETER;

    // Page-align
    U64 pageStart = virtualAddress & ~0xFFFULL;
    U64 pageEnd = (virtualAddress + size + 0xFFF) & ~0xFFFULL;

    for (U64 va = pageStart; va < pageEnd; va += 0x1000) {
        // Check if already hooked
        if (FindHookByVa(cr3, va)) {
            continue;  // Already have a hook here
        }

        PSHADOW_HOOK hook = FindFreeSlot();
        if (!hook) {
            return OMBRA_STATUS_LIMIT_EXCEEDED;
        }

        // Translate VA to physical
        U64 guestPhysical = 0;
        I64 status = VmVirtToPhys(cr3, va, &guestPhysical);
        if (status != VMCALL_STATUS_SUCCESS) {
            continue;  // Page not mapped, skip
        }

        // Allocate clean page copy
        U64 cleanPhysical = 0;
        status = VmAllocPhysicalPage(&cleanPhysical);
        if (status != VMCALL_STATUS_SUCCESS) {
            return OMBRA_STATUS_OUT_OF_MEMORY;
        }

        // Copy original page content to clean copy
        status = VmCopyPhysicalPage(guestPhysical, cleanPhysical);
        if (status != VMCALL_STATUS_SUCCESS) {
            VmFreePhysicalPage(cleanPhysical);
            return OMBRA_STATUS_VMCALL_FAILED;
        }

        // Request EPT split from hypervisor
        // This creates per-permission views:
        // - Execute: points to guestPhysical (where we can install hooks)
        // - Read/Write: points to cleanPhysical (original content)
        status = VmSplitEptPage(cr3, guestPhysical, cleanPhysical);
        if (status != VMCALL_STATUS_SUCCESS) {
            VmFreePhysicalPage(cleanPhysical);
            return OMBRA_STATUS_VMCALL_FAILED;
        }

        // Record hook
        hook->Active = true;
        hook->Cr3 = cr3;
        hook->GuestPhysical = guestPhysical & ~0xFFFULL;
        hook->CleanPhysical = cleanPhysical;
        hook->VirtualAddress = va;
        hook->Size = 0x1000;
    }

    return OMBRA_STATUS_SUCCESS;
}

// Remove shadow hook
I32 ShadowRemove(U64 cr3, U64 virtualAddress, U64 size) {
    U64 pageStart = virtualAddress & ~0xFFFULL;
    U64 pageEnd = (virtualAddress + size + 0xFFF) & ~0xFFFULL;

    for (U64 va = pageStart; va < pageEnd; va += 0x1000) {
        PSHADOW_HOOK hook = FindHookByVa(cr3, va);
        if (!hook) continue;

        // Unsplit EPT page
        VmUnsplitEptPage(cr3, hook->GuestPhysical);

        // Free clean copy
        VmFreePhysicalPage(hook->CleanPhysical);

        // Clear hook record
        hook->Active = false;
    }

    return OMBRA_STATUS_SUCCESS;
}

// Remove all hooks for a process
void ShadowRemoveAll(U64 cr3) {
    for (U32 i = 0; i < MAX_SHADOW_HOOKS; i++) {
        if (g_ShadowHooks[i].Active && g_ShadowHooks[i].Cr3 == cr3) {
            VmUnsplitEptPage(cr3, g_ShadowHooks[i].GuestPhysical);
            VmFreePhysicalPage(g_ShadowHooks[i].CleanPhysical);
            g_ShadowHooks[i].Active = false;
        }
    }
}

// Get hook count
U32 ShadowGetCount(void) {
    U32 count = 0;
    for (U32 i = 0; i < MAX_SHADOW_HOOKS; i++) {
        if (g_ShadowHooks[i].Active) count++;
    }
    return count;
}

// =============================================================================
// Command Handlers
// =============================================================================

I32 HandleShadowMemory(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)resp;
    return ShadowInstall(cmd->Memory.Cr3, cmd->Memory.Address, cmd->Memory.Size);
}

I32 HandleHideMemory(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)resp;

    // Hide memory creates a NO-ACCESS view
    // For now, we use shadow hooks but don't expose the hooked version
    U64 cr3 = cmd->Memory.Cr3;
    U64 addr = cmd->Memory.Address;
    U64 size = cmd->Memory.Size;

    // Request hypervisor to hide memory
    I64 status = VmHideMemory(addr, size);
    if (status != VMCALL_STATUS_SUCCESS) {
        return OMBRA_STATUS_VMCALL_FAILED;
    }

    (void)cr3;  // CR3-specific hiding would require per-process EPT views

    return OMBRA_STATUS_SUCCESS;
}
