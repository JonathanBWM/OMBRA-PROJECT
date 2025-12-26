// hooks.h — EPT Hook Framework
// OmbraHypervisor
//
// Implements invisible hooks using EPT execute-only pages.
// When code is executed from a hooked page, it runs from the hook page.
// When code is read from a hooked page, it reads from the original page.

#ifndef OMBRA_HOOKS_H
#define OMBRA_HOOKS_H

#include "../shared/types.h"

// =============================================================================
// Forward Declarations
// =============================================================================

struct _EPT_STATE;

// =============================================================================
// Constants
// =============================================================================

#define MAX_HOOKS               256
#define HOOK_MAGIC              0x484F4F4B484F4F4BULL  // "HOOKHOOK"

// Hook types
#define HOOK_TYPE_INLINE        0   // Inline hook (JMP to handler)
#define HOOK_TYPE_EPT_EXECUTE   1   // EPT execute-only hook
#define HOOK_TYPE_EPT_READWRITE 2   // EPT read/write intercept

// =============================================================================
// Hook Entry Structure
// =============================================================================

typedef struct _EPT_HOOK {
    U64     Magic;              // HOOK_MAGIC for validation

    // Target identification
    U64     TargetVirtual;      // Virtual address being hooked
    U64     TargetPhysical;     // Physical address of target page
    U32     TargetOffset;       // Offset within page

    // Shadow page
    U64     ShadowPhysical;     // Physical address of shadow page
    void*   ShadowVirtual;      // Virtual address of shadow page

    // Original data
    U8      OriginalBytes[16];  // Original bytes at hook point
    U32     OriginalLength;     // Length of original instruction(s)

    // Hook handler
    void*   HandlerAddress;     // Address of hook handler function
    void*   TrampolineAddress;  // Address of trampoline for calling original

    // Hook state
    bool    Active;             // Is hook currently active?
    bool    Executing;          // Is hook currently being executed?
    U32     HitCount;           // Number of times hook was triggered
    U32     CpuMask;            // Which CPUs have this hook (0 = all)

    // Hook type
    U32     Type;               // HOOK_TYPE_*

    // Linked list (for per-page hook lists)
    struct _EPT_HOOK* Next;
} EPT_HOOK;

// =============================================================================
// Hook Manager State
// =============================================================================

typedef struct _HOOK_MANAGER {
    EPT_HOOK    Hooks[MAX_HOOKS];
    U32         HookCount;
    bool        Initialized;

    // Shadow page pool
    void*       ShadowPool;         // Pool of shadow pages
    U64         ShadowPoolPhysical;
    U32         ShadowPoolSize;     // Number of pages
    U32         ShadowPoolUsed;     // Number used

    // Split page tracking
    U32         SplitPageCount;     // Number of 1GB pages split to 2MB
    U32         SplitPdCount;       // Number of 2MB pages split to 4KB

    // Reference to EPT
    struct _EPT_STATE* Ept;
} HOOK_MANAGER;

// =============================================================================
// Hook Callback Types
// =============================================================================

// Pre-hook callback: Called before original function
// Return false to skip calling original
typedef bool (*HOOK_PRE_CALLBACK)(
    EPT_HOOK* hook,
    void* context
);

// Post-hook callback: Called after original function
typedef void (*HOOK_POST_CALLBACK)(
    EPT_HOOK* hook,
    void* context
);

// =============================================================================
// Hook Manager Functions
// =============================================================================

// Initialize the hook manager
OMBRA_STATUS HookManagerInit(HOOK_MANAGER* mgr, struct _EPT_STATE* ept);

// Shutdown and remove all hooks
void HookManagerShutdown(HOOK_MANAGER* mgr);

// =============================================================================
// Hook Installation Functions
// =============================================================================

// Install an EPT execute-only hook
// - Target page becomes execute-only (no read)
// - Execute: runs from shadow page (with hook)
// - Read: sees original page (unmodified)
OMBRA_STATUS HookInstallEpt(
    HOOK_MANAGER* mgr,
    U64 targetVirtual,
    U64 targetPhysical,
    void* handlerAddress,
    EPT_HOOK** outHook
);

// Install an inline hook (traditional JMP hook)
OMBRA_STATUS HookInstallInline(
    HOOK_MANAGER* mgr,
    U64 targetVirtual,
    U64 targetPhysical,
    void* handlerAddress,
    EPT_HOOK** outHook
);

// Remove a hook
OMBRA_STATUS HookRemove(HOOK_MANAGER* mgr, EPT_HOOK* hook);

// Enable/disable a hook temporarily
OMBRA_STATUS HookEnable(EPT_HOOK* hook);
OMBRA_STATUS HookDisable(EPT_HOOK* hook);

// =============================================================================
// Hook Lookup Functions
// =============================================================================

// Find hook by physical address
EPT_HOOK* HookFindByPhysical(HOOK_MANAGER* mgr, U64 physicalAddress);

// Find hook by virtual address
EPT_HOOK* HookFindByVirtual(HOOK_MANAGER* mgr, U64 virtualAddress);

// =============================================================================
// EPT Violation Handler Support
// =============================================================================

// Handle EPT violation for hooked page
// Returns true if violation was handled by hook system
bool HookHandleEptViolation(
    HOOK_MANAGER* mgr,
    U64 guestPhysical,
    U64 qualification,
    void* guestRegs
);

// =============================================================================
// Shadow Page Management
// =============================================================================

// Allocate a shadow page from the pool
void* HookAllocateShadowPage(HOOK_MANAGER* mgr, U64* outPhysical);

// Free a shadow page back to the pool
void HookFreeShadowPage(HOOK_MANAGER* mgr, void* page);

// =============================================================================
// EPT Page Splitting
// =============================================================================

// Split a 1GB page into 512 x 2MB pages
OMBRA_STATUS EptSplit1GbTo2Mb(struct _EPT_STATE* ept, U64 guestPhysical);

// Split a 2MB page into 512 x 4KB pages
OMBRA_STATUS EptSplit2MbTo4Kb(struct _EPT_STATE* ept, U64 guestPhysical);

#endif // OMBRA_HOOKS_H
