/**
 * 03_shadow_hooks.c - Shadow Page Hooking Implementation
 * 
 * Pure C implementation of EPT-based invisible hooks.
 * Reference: Intel SDM Volume 3, Chapter 28
 * 
 * CONCEPTS COVERED:
 * - Shadow page technique (dual memory views)
 * - Execute-only pages (Intel EPT)
 * - Hook installation
 * - Trampoline generation
 * - Hook management
 */

#include "01_ept_structures.h"
#include "../common/types.h"

/*===========================================================================
 * CONCEPT 1: Shadow Page Technique
 *===========================================================================
 * The core idea: Present different physical pages based on access type.
 * 
 * For a hooked function at GPA X:
 * - READ access → Original clean page (scanner sees unmodified code)
 * - EXECUTE access → Shadow page with hook (our code runs)
 * 
 * Intel EPT supports this natively with execute-only pages:
 *   R=0, W=0, X=1 → Only execute allowed
 *   Any read attempt triggers EPT violation
 * 
 * AMD NPT does NOT support execute-only. Requires dual-table switching.
 */

/*===========================================================================
 * Hook Entry Structure
 *===========================================================================*/

#define MAX_HOOKS 64
#define TRAMPOLINE_SIZE 32

typedef struct _SHADOW_HOOK {
    /* Target identification */
    U64     TargetGpa;          /* Guest physical address of hooked page */
    U64     TargetOffset;       /* Offset within page where hook is */
    
    /* Physical pages */
    U64     OriginalHpa;        /* Original host physical address */
    U64     ShadowHpa;          /* Shadow page with hook installed */
    
    /* Hook details */
    U64     HandlerAddress;     /* Where to jump on hook */
    U8      OriginalBytes[16];  /* Original instruction bytes */
    U32     OriginalLength;     /* Length of overwritten instructions */
    
    /* Trampoline for calling original */
    U8      Trampoline[TRAMPOLINE_SIZE];
    U64     TrampolineGpa;      /* GPA of trampoline (execute-allowed) */
    
    /* EPT entry pointer */
    EPT_PTE* Pte;               /* Pointer to the 4KB EPT entry */
    
    /* State */
    bool    Active;
    
} SHADOW_HOOK;

typedef struct _HOOK_MANAGER {
    SHADOW_HOOK Hooks[MAX_HOOKS];
    U32         HookCount;
    EPT_STATE*  Ept;
} HOOK_MANAGER;

/*===========================================================================
 * CONCEPT 2: Hook Installation
 *===========================================================================
 * Steps to install a shadow hook:
 * 1. Split containing large page to 4KB if needed
 * 2. Allocate shadow page
 * 3. Copy original page to shadow
 * 4. Install hook bytes in shadow (jmp to handler)
 * 5. Generate trampoline for calling original
 * 6. Set EPT entry to execute-only, pointing to shadow
 */

/**
 * Create a trampoline that executes original bytes then jumps back
 * 
 * @param trampoline    Buffer for trampoline code
 * @param originalBytes Original instruction bytes
 * @param originalLen   Length of original instructions
 * @param returnAddr    Address to jump after trampoline
 */
static void CreateTrampoline(
    U8* trampoline,
    const U8* originalBytes,
    U32 originalLen,
    U64 returnAddr)
{
    U32 i;
    
    /* Copy original instructions */
    for (i = 0; i < originalLen; i++) {
        trampoline[i] = originalBytes[i];
    }
    
    /* Append: JMP QWORD PTR [RIP+0] ; absolute jump */
    /* FF 25 00 00 00 00 <8-byte address> */
    trampoline[originalLen + 0] = 0xFF;
    trampoline[originalLen + 1] = 0x25;
    trampoline[originalLen + 2] = 0x00;
    trampoline[originalLen + 3] = 0x00;
    trampoline[originalLen + 4] = 0x00;
    trampoline[originalLen + 5] = 0x00;
    
    /* 8-byte absolute address */
    *(U64*)&trampoline[originalLen + 6] = returnAddr;
}

/**
 * Install inline hook in shadow page
 * 
 * @param shadowPage    Virtual address of shadow page
 * @param offset        Offset within page
 * @param handlerAddr   Handler to jump to
 */
static void InstallInlineHook(
    U8* shadowPage,
    U32 offset,
    U64 handlerAddr)
{
    U8* hookLocation = shadowPage + offset;
    
    /* Write: JMP QWORD PTR [RIP+0] */
    hookLocation[0] = 0xFF;     /* JMP opcode */
    hookLocation[1] = 0x25;     /* ModR/M: [RIP+disp32] */
    hookLocation[2] = 0x00;     /* disp32 = 0 */
    hookLocation[3] = 0x00;
    hookLocation[4] = 0x00;
    hookLocation[5] = 0x00;
    
    /* 8-byte absolute address follows immediately */
    *(U64*)(hookLocation + 6) = handlerAddr;
    
    /* Total: 14 bytes for this hook method */
}

/**
 * Alternative: INT3 hook (single byte, needs EPT violation handling)
 */
static void InstallInt3Hook(U8* shadowPage, U32 offset)
{
    shadowPage[offset] = 0xCC;  /* INT3 */
}

/*===========================================================================
 * CONCEPT 3: Main Hook Installation Function
 *===========================================================================*/

/**
 * Install a shadow page hook
 * 
 * @param mgr           Hook manager
 * @param targetGpa     GPA of function to hook
 * @param handlerAddr   Address of hook handler
 * @param originalLen   Number of bytes to overwrite (must be >= 14 for JMP)
 * @param shadowPage    Pre-allocated shadow page (virtual address)
 * @param shadowHpa     Physical address of shadow page
 * @param ptPage        Pre-allocated PT (if large page split needed)
 * @param ptPhysical    Physical address of PT
 * @return Pointer to hook entry, or NULL on failure
 */
SHADOW_HOOK* HookInstall(
    HOOK_MANAGER* mgr,
    U64 targetGpa,
    U64 handlerAddr,
    U32 originalLen,
    void* shadowPage,
    U64 shadowHpa,
    EPT_PTE* ptPage,        /* For splitting large pages */
    U64 ptPhysical)
{
    SHADOW_HOOK* hook;
    EPT_PTE* pte;
    U64 pageBase;
    U32 offset;
    U64 originalHpa;
    U8* originalPage;
    U32 i;
    
    if (!mgr || !mgr->Ept || mgr->HookCount >= MAX_HOOKS) {
        return NULL;
    }
    
    if (originalLen < 14) {
        /* Need at least 14 bytes for JMP QWORD PTR [RIP] */
        return NULL;
    }
    
    /* Calculate page-aligned GPA and offset */
    pageBase = targetGpa & ~PAGE_MASK;
    offset = (U32)(targetGpa & PAGE_MASK);
    
    /* Get or create 4KB PTE for this GPA */
    pte = EptGetPte(mgr->Ept, pageBase);
    
    if (!pte) {
        /* Page is still a large page - need to split */
        /* This requires knowing which PDE to split */
        /* For simplicity, assume caller handles this externally */
        return NULL;
    }
    
    /* Get original physical address */
    originalHpa = (U64)pte->PagePhysAddr << 12;
    
    /* Copy original page to shadow page */
    /* NOTE: In production, use proper physical-to-virtual translation */
    originalPage = (U8*)originalHpa;  /* Simplified - needs mapping */
    for (i = 0; i < PAGE_SIZE; i++) {
        ((U8*)shadowPage)[i] = originalPage[i];
    }
    
    /* Find free hook slot */
    hook = &mgr->Hooks[mgr->HookCount];
    
    /* Save original bytes for trampoline */
    for (i = 0; i < originalLen && i < 16; i++) {
        hook->OriginalBytes[i] = ((U8*)shadowPage)[offset + i];
    }
    hook->OriginalLength = originalLen;
    
    /* Install hook in shadow page */
    InstallInlineHook((U8*)shadowPage, offset, handlerAddr);
    
    /* Create trampoline */
    CreateTrampoline(
        hook->Trampoline,
        hook->OriginalBytes,
        originalLen,
        targetGpa + originalLen   /* Return to instruction after hook */
    );
    
    /* Fill in hook entry */
    hook->TargetGpa = targetGpa;
    hook->TargetOffset = offset;
    hook->OriginalHpa = originalHpa;
    hook->ShadowHpa = shadowHpa;
    hook->HandlerAddress = handlerAddr;
    hook->Pte = pte;
    
    /* Configure EPT for execute-only on shadow page */
    /* R=0, W=0, X=1 pointing to shadow */
    pte->Read = 0;
    pte->Write = 0;
    pte->Execute = 1;
    pte->PagePhysAddr = shadowHpa >> 12;
    
    /* Invalidate TLB for this page */
    EptInvalidate(mgr->Ept->Eptp, INVEPT_TYPE_SINGLE_CONTEXT);
    
    hook->Active = true;
    mgr->HookCount++;
    
    return hook;
}

/*===========================================================================
 * CONCEPT 4: EPT Violation Handling for Hooks
 *===========================================================================
 * When guest READs a hooked execute-only page:
 * 1. EPT violation occurs (read on X-only page)
 * 2. Handler checks if it's a hooked page
 * 3. Temporarily switch to RW view (original page)
 * 4. Enable Monitor Trap Flag for single-step
 * 5. Resume guest
 * 6. MTF trap fires after one instruction
 * 7. Switch back to X-only view
 * 8. Disable MTF, resume normally
 */

typedef enum _HOOK_VIEW {
    HOOK_VIEW_EXECUTE,      /* Showing shadow page (X-only) */
    HOOK_VIEW_READWRITE,    /* Showing original page (RW) */
} HOOK_VIEW;

/**
 * Handle EPT violation on hooked page
 * 
 * @param mgr           Hook manager
 * @param faultingGpa   GPA that caused violation
 * @param wasRead       True if access was read
 * @param wasWrite      True if access was write
 * @param wasExecute    True if access was execute
 * @return true if handled, false if not our hook
 */
bool HookHandleEptViolation(
    HOOK_MANAGER* mgr,
    U64 faultingGpa,
    bool wasRead,
    bool wasWrite,
    bool wasExecute)
{
    U32 i;
    U64 pageBase = faultingGpa & ~PAGE_MASK;
    SHADOW_HOOK* hook = NULL;
    
    /* Find hook for this page */
    for (i = 0; i < mgr->HookCount; i++) {
        if ((mgr->Hooks[i].TargetGpa & ~PAGE_MASK) == pageBase) {
            hook = &mgr->Hooks[i];
            break;
        }
    }
    
    if (!hook || !hook->Active) {
        return false;  /* Not our hook */
    }
    
    if (wasRead || wasWrite) {
        /*
         * Guest tried to READ or WRITE the hooked page.
         * Switch to RW view showing original bytes.
         */
        hook->Pte->Read = 1;
        hook->Pte->Write = 1;
        hook->Pte->Execute = 0;
        hook->Pte->PagePhysAddr = hook->OriginalHpa >> 12;
        
        /* Enable Monitor Trap Flag for single-stepping */
        /* After one instruction, MTF trap will fire */
        /* Then we switch back to X-only view */
        
        /* VmcsWrite(VMCS_CTRL_PROC_BASED, existing | CPU_MONITOR_TRAP); */
        
        EptInvalidate(mgr->Ept->Eptp, INVEPT_TYPE_SINGLE_CONTEXT);
        return true;
    }
    
    /* Execute violation shouldn't happen on X-only pages */
    /* Unless there's a configuration error */
    return false;
}

/**
 * Handle MTF (Monitor Trap Flag) exit
 * Called after single-stepping past the read/write
 * 
 * @param mgr   Hook manager
 * @param gpa   GPA of instruction that just executed
 */
void HookHandleMtf(HOOK_MANAGER* mgr, U64 gpa)
{
    U32 i;
    U64 pageBase = gpa & ~PAGE_MASK;
    SHADOW_HOOK* hook = NULL;
    
    for (i = 0; i < mgr->HookCount; i++) {
        if ((mgr->Hooks[i].TargetGpa & ~PAGE_MASK) == pageBase) {
            hook = &mgr->Hooks[i];
            break;
        }
    }
    
    if (!hook || !hook->Active) {
        return;
    }
    
    /* Switch back to execute-only view */
    hook->Pte->Read = 0;
    hook->Pte->Write = 0;
    hook->Pte->Execute = 1;
    hook->Pte->PagePhysAddr = hook->ShadowHpa >> 12;
    
    /* Disable MTF */
    /* VmcsWrite(VMCS_CTRL_PROC_BASED, existing & ~CPU_MONITOR_TRAP); */
    
    EptInvalidate(mgr->Ept->Eptp, INVEPT_TYPE_SINGLE_CONTEXT);
}

/*===========================================================================
 * CONCEPT 5: Hook Removal
 *===========================================================================*/

/**
 * Remove a shadow hook
 * 
 * @param mgr   Hook manager
 * @param hook  Hook to remove
 */
void HookRemove(HOOK_MANAGER* mgr, SHADOW_HOOK* hook)
{
    if (!hook || !hook->Active) {
        return;
    }
    
    /* Restore EPT to point to original page with full permissions */
    hook->Pte->Read = 1;
    hook->Pte->Write = 1;
    hook->Pte->Execute = 1;
    hook->Pte->PagePhysAddr = hook->OriginalHpa >> 12;
    
    EptInvalidate(mgr->Ept->Eptp, INVEPT_TYPE_SINGLE_CONTEXT);
    
    hook->Active = false;
}
