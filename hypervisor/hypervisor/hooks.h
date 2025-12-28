// hooks.h — EPT Hook Framework
// OmbraHypervisor
//
// Implements invisible hooks using EPT execute-only pages.
// When code is executed from a hooked page, it runs from the hook page.
// When code is read from a hooked page, it reads from the original page.
//
// =============================================================================
// USAGE REQUIREMENT: Identity Mapped Physical Memory
// =============================================================================
//
// This framework REQUIRES that the hypervisor has identity-mapped all guest
// physical memory in its own virtual address space (HVA == HPA).
//
// Why: The hook framework needs to read/write guest physical pages from the
// hypervisor's execution context. Since the hypervisor uses its own CR3
// (not the guest's), it needs HPA->HVA translation. Current implementation
// assumes identity mapping for simplicity.
//
// The HookManagerInit() function validates this requirement at runtime.
// If identity mapping is not configured, initialization will fail with
// OMBRA_ERROR_INVALID_STATE.
//
// =============================================================================

#ifndef OMBRA_HOOKS_H
#define OMBRA_HOOKS_H

#include "../shared/types.h"
#include "../shared/ept_defs.h"
#include <intrin.h>

// =============================================================================
// Forward Declarations
// =============================================================================

struct _EPT_STATE;

// =============================================================================
// Spinlock for Thread Safety
// =============================================================================

typedef volatile long SPINLOCK;

static inline void SpinLockInit(SPINLOCK* lock) {
    *lock = 0;
}

static inline void SpinLockAcquire(SPINLOCK* lock) {
    while (_InterlockedCompareExchange(lock, 1, 0) != 0) {
        _mm_pause();  // Reduce CPU contention
    }
}

static inline void SpinLockRelease(SPINLOCK* lock) {
    _InterlockedExchange(lock, 0);
}

// =============================================================================
// Constants
// =============================================================================

#define MAX_HOOKS               256
#define MAX_SHADOW_HOOKS        64
#define TRAMPOLINE_SIZE         32
// HOOK_MAGIC stored obfuscated in binary, deobfuscated at use sites
// Original: "HOOKHOOK" = 0x484F4F4B484F4F4B
// Use hook_get_magic() to get runtime value

// Hook types
#define HOOK_TYPE_INLINE        0   // Inline hook (JMP to handler)
#define HOOK_TYPE_EPT_EXECUTE   1   // EPT execute-only hook
#define HOOK_TYPE_EPT_READWRITE 2   // EPT read/write intercept
#define HOOK_TYPE_SHADOW        3   // Shadow page hook (dual view)

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
// Shadow Hook Structure (Dual Memory View)
// =============================================================================
//
// Shadow hooks present different physical pages based on access type:
// - READ/WRITE access → Original clean page (scanners see unmodified code)
// - EXECUTE access → Shadow page with hook installed (our code runs)
//
// Uses execute-only EPT pages (R=0, W=0, X=1) and MTF for view switching.

typedef struct _SHADOW_HOOK {
    U64     Magic;              // HOOK_MAGIC for validation

    // Target identification
    U64     TargetGpa;          // Guest physical address of hooked page
    U32     TargetOffset;       // Offset within page where hook is installed

    // Physical pages
    U64     OriginalHpa;        // Original host physical address
    U64     ShadowHpa;          // Shadow page with hook installed

    // Hook details
    U64     HandlerAddress;     // Where to jump on hook execution
    U8      OriginalBytes[16];  // Original instruction bytes (saved)
    U32     OriginalLength;     // Length of overwritten instructions

    // Trampoline for calling original code
    U8      Trampoline[TRAMPOLINE_SIZE];
    U64     TrampolineGpa;      // GPA of trampoline (if allocated)

    // EPT entry pointer
    EPT_PTE* Pte;               // Pointer to the 4KB EPT entry

    // State
    bool    Active;             // Is hook currently active?
    bool    InRwView;           // Currently showing RW view (not execute-only)?
    U32     HitCount;           // Number of times hook was triggered
} SHADOW_HOOK;

// =============================================================================
// Hook Manager State
// =============================================================================

typedef struct _HOOK_MANAGER {
    SPINLOCK    Lock;               // Protect hook operations from race conditions
    EPT_HOOK    Hooks[MAX_HOOKS];
    U32         HookCount;
    bool        Initialized;

    // Shadow hooks
    SHADOW_HOOK ShadowHooks[MAX_SHADOW_HOOKS];
    U32         ShadowHookCount;

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
OMBRA_STATUS HookEnable(HOOK_MANAGER* mgr, EPT_HOOK* hook);
OMBRA_STATUS HookDisable(HOOK_MANAGER* mgr, EPT_HOOK* hook);

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
// Shadow Hook Functions
// =============================================================================

// Install a shadow page hook
// The shadow page must be pre-allocated and populated by the caller
OMBRA_STATUS HookInstallShadow(
    HOOK_MANAGER* mgr,
    U64 targetGpa,
    U64 handlerAddress,
    U32 originalLength,
    void* shadowPage,
    U64 shadowHpa,
    SHADOW_HOOK** outHook
);

// Remove a shadow hook
OMBRA_STATUS HookRemoveShadow(HOOK_MANAGER* mgr, SHADOW_HOOK* hook);

// Handle EPT violation for shadow hooks
// Returns true if violation was handled by shadow hook system
bool HookHandleEptViolationShadow(
    HOOK_MANAGER* mgr,
    U64 faultingGpa,
    bool wasRead,
    bool wasWrite,
    bool wasExecute
);

// Handle MTF (Monitor Trap Flag) exit for shadow hooks
// Called after single-stepping past a read/write to restore execute-only view
void HookHandleMtf(HOOK_MANAGER* mgr, U64 guestRip);

// Find shadow hook by GPA
SHADOW_HOOK* HookFindShadowByGpa(HOOK_MANAGER* mgr, U64 gpa);

// =============================================================================
// NOTE: EPT Page Splitting
// =============================================================================
//
// EptSplit1GbTo2Mb() and EptSplit2MbTo4Kb() are declared in ept.h
// (not here) to maintain a single source of truth. Include ept.h to use them.
//
// =============================================================================

#endif // OMBRA_HOOKS_H
