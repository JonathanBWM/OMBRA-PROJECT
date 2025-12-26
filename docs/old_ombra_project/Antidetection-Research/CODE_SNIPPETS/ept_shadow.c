/**
 * EPT Shadow Page Patterns
 * Extracted from: hvpp, DdiMon, CheatDriver, NoirVisor
 *
 * These patterns demonstrate Execute-Only page shadowing for
 * stealthy inline hooks that evade integrity checks.
 */

#include <stdint.h>

/* EPT Entry Permissions */
#define EPT_READ        (1 << 0)
#define EPT_WRITE       (1 << 1)
#define EPT_EXECUTE     (1 << 2)
#define EPT_MEMORY_TYPE_MASK    (7 << 3)
#define EPT_MEMORY_TYPE_WB      (6 << 3)    /* Write-back */
#define EPT_LARGE_PAGE  (1 << 7)

/* EPT Table Levels */
#define EPT_PML4_INDEX(addr)    (((addr) >> 39) & 0x1FF)
#define EPT_PDPT_INDEX(addr)    (((addr) >> 30) & 0x1FF)
#define EPT_PD_INDEX(addr)      (((addr) >> 21) & 0x1FF)
#define EPT_PT_INDEX(addr)      (((addr) >> 12) & 0x1FF)
#define PAGE_SIZE_4KB           0x1000
#define PAGE_SIZE_2MB           0x200000

/**
 * Pattern 1: EPT Entry Structure (from hvpp)
 *
 * Source: hvpp/ept.h
 */
typedef union {
    uint64_t flags;
    struct {
        uint64_t read_access : 1;
        uint64_t write_access : 1;
        uint64_t execute_access : 1;
        uint64_t memory_type : 3;
        uint64_t ignore_pat : 1;
        uint64_t large_page : 1;
        uint64_t accessed : 1;
        uint64_t dirty : 1;
        uint64_t execute_for_user : 1;
        uint64_t reserved1 : 1;
        uint64_t page_frame_number : 36;
        uint64_t reserved2 : 15;
        uint64_t suppress_ve : 1;
    };
} ept_entry_t;

/**
 * Pattern 2: Shadow Page Structure (from DdiMon)
 *
 * Source: DdiMon/ShadowHook.cpp
 *
 * Two pages per hook:
 * - exec_page: Contains hooked code (execute-only)
 * - data_page: Contains original code (read/write, no execute)
 */
typedef struct {
    uint64_t target_pa;         /* Physical address of original page */
    uint64_t exec_page_pa;      /* Physical address of execute page (hooked) */
    uint64_t data_page_pa;      /* Physical address of data page (clean) */
    void* exec_page_va;         /* Virtual address of execute page */
    void* data_page_va;         /* Virtual address of data page */
    int active;                 /* Hook active flag */
} shadow_page_t;

/**
 * Pattern 3: Execute-Only Page Setup (from hvpp)
 *
 * Source: hvpp/ept.cpp
 *
 * Set page to execute-only to hide hooks from read access.
 */
static void setup_execute_only(ept_entry_t* entry, uint64_t target_pa,
                               uint64_t hooked_pa)
{
    /*
     * Point EPT entry to hooked page with execute-only permissions.
     * Reads/writes will trigger EPT violation.
     */
    entry->flags = 0;
    entry->read_access = 0;     /* No read */
    entry->write_access = 0;    /* No write */
    entry->execute_access = 1;  /* Execute allowed */
    entry->memory_type = 6;     /* Write-back */
    entry->page_frame_number = hooked_pa >> 12;
}

/**
 * Pattern 4: EPT Violation Handler (from CheatDriver)
 *
 * Source: CheatDriver/EPTHook.cpp
 *
 * Handle EPT violations to swap between hooked and clean pages.
 */
typedef enum {
    EPT_VIOLATION_READ  = (1 << 0),
    EPT_VIOLATION_WRITE = (1 << 1),
    EPT_VIOLATION_EXEC  = (1 << 2)
} ept_violation_type_t;

static int handle_ept_violation(shadow_page_t* shadow, uint64_t guest_pa,
                                ept_violation_type_t type, ept_entry_t* entry)
{
    if (type & (EPT_VIOLATION_READ | EPT_VIOLATION_WRITE)) {
        /*
         * Guest tried to read/write the hooked page.
         * Swap to clean data page and enable MTF for single-step.
         */
        entry->page_frame_number = shadow->data_page_pa >> 12;
        entry->read_access = 1;
        entry->write_access = 1;
        entry->execute_access = 0;

        /* Enable Monitor Trap Flag for single-step */
        enable_mtf();

        return 0;  /* Let guest continue with single instruction */
    }

    if (type & EPT_VIOLATION_EXEC) {
        /*
         * Guest tried to execute. This shouldn't happen if page
         * is already execute-only, but handle it anyway.
         */
        entry->page_frame_number = shadow->exec_page_pa >> 12;
        entry->read_access = 0;
        entry->write_access = 0;
        entry->execute_access = 1;

        return 0;
    }

    return -1;  /* Unknown violation */
}

/**
 * Pattern 5: MTF (Monitor Trap Flag) Handler (from DdiMon)
 *
 * Source: DdiMon/ShadowHook.cpp
 *
 * After single instruction, restore execute-only mapping.
 */
static int handle_mtf_exit(shadow_page_t* shadow, ept_entry_t* entry)
{
    /*
     * Single instruction completed.
     * Restore execute-only mapping to hide hooks again.
     */
    entry->page_frame_number = shadow->exec_page_pa >> 12;
    entry->read_access = 0;
    entry->write_access = 0;
    entry->execute_access = 1;

    /* Disable MTF */
    disable_mtf();

    /* Invalidate EPT cache for this page */
    invept_single_context(shadow->target_pa);

    return 0;
}

/**
 * Pattern 6: Page Split (2MB -> 4KB) (from hvpp)
 *
 * Source: hvpp/ept.cpp
 *
 * Split large page for fine-grained hook control.
 */
typedef struct {
    ept_entry_t entries[512];
} ept_page_table_t;

static ept_page_table_t* split_2mb_to_4kb(ept_entry_t* large_entry,
                                          uint64_t base_pa)
{
    /* Allocate new page table */
    ept_page_table_t* pt = allocate_page_table();
    if (!pt) return NULL;

    /* Get original large page PFN and permissions */
    uint64_t orig_pfn = large_entry->page_frame_number;
    int orig_perms = large_entry->flags & 0x7;

    /* Fill 512 4KB entries covering the same 2MB region */
    for (int i = 0; i < 512; i++) {
        uint64_t page_pa = (orig_pfn << 21) + (i * PAGE_SIZE_4KB);
        pt->entries[i].flags = 0;
        pt->entries[i].read_access = (orig_perms & EPT_READ) ? 1 : 0;
        pt->entries[i].write_access = (orig_perms & EPT_WRITE) ? 1 : 0;
        pt->entries[i].execute_access = (orig_perms & EPT_EXECUTE) ? 1 : 0;
        pt->entries[i].memory_type = 6;  /* WB */
        pt->entries[i].page_frame_number = page_pa >> 12;
    }

    /* Update parent entry to point to new PT */
    large_entry->large_page = 0;
    large_entry->page_frame_number = (uint64_t)pt >> 12;  /* Physical address */
    large_entry->read_access = 1;
    large_entry->write_access = 1;
    large_entry->execute_access = 1;

    return pt;
}

/**
 * Pattern 7: Hook Installation (from CheatDriver)
 *
 * Source: CheatDriver/EPTHook.cpp
 *
 * Install stealthy inline hook using EPT shadowing.
 */
typedef struct {
    uint8_t original_bytes[16];
    uint8_t hook_bytes[16];
    void* target_address;
    void* handler_address;
} inline_hook_t;

static int install_ept_hook(shadow_page_t* shadow, inline_hook_t* hook,
                            ept_entry_t* entry)
{
    /* Copy original page to both exec and data pages */
    memcpy(shadow->exec_page_va, get_page_va(shadow->target_pa), PAGE_SIZE_4KB);
    memcpy(shadow->data_page_va, get_page_va(shadow->target_pa), PAGE_SIZE_4KB);

    /* Install hook in exec page only */
    void* hook_target = (uint8_t*)shadow->exec_page_va +
                        ((uint64_t)hook->target_address & 0xFFF);
    memcpy(hook_target, hook->hook_bytes, sizeof(hook->hook_bytes));

    /* Set up execute-only mapping */
    setup_execute_only(entry, shadow->target_pa, shadow->exec_page_pa);

    shadow->active = 1;

    return 0;
}

/**
 * Pattern 8: MTRR Memory Type Lookup (from HyperPlatform)
 *
 * Source: HyperPlatform/Ept.cpp
 *
 * Get correct memory type for EPT entry based on MTRR.
 */
#define MTRR_TYPE_UC    0   /* Uncacheable */
#define MTRR_TYPE_WC    1   /* Write-combining */
#define MTRR_TYPE_WT    4   /* Write-through */
#define MTRR_TYPE_WP    5   /* Write-protected */
#define MTRR_TYPE_WB    6   /* Write-back */

static uint8_t get_memory_type_for_pa(uint64_t physical_address)
{
    /*
     * Simplified version - in practice, check MTRR registers.
     * Most memory is WB, low memory may be UC.
     */
    if (physical_address < 0x100000) {
        return MTRR_TYPE_UC;  /* First 1MB often UC */
    }
    return MTRR_TYPE_WB;      /* Most RAM is WB */
}

/**
 * Pattern 9: Complete Shadow Hook Manager
 *
 * Combined pattern for managing multiple shadow hooks.
 */
#define MAX_SHADOW_HOOKS    64

typedef struct {
    shadow_page_t hooks[MAX_SHADOW_HOOKS];
    int hook_count;
    void* ept_pml4;
} shadow_hook_manager_t;

static int add_shadow_hook(shadow_hook_manager_t* mgr, uint64_t target_pa,
                           void* handler)
{
    if (mgr->hook_count >= MAX_SHADOW_HOOKS) {
        return -1;  /* Too many hooks */
    }

    shadow_page_t* shadow = &mgr->hooks[mgr->hook_count];

    /* Allocate exec and data pages */
    shadow->exec_page_va = allocate_page();
    shadow->data_page_va = allocate_page();
    shadow->exec_page_pa = get_physical_address(shadow->exec_page_va);
    shadow->data_page_pa = get_physical_address(shadow->data_page_va);
    shadow->target_pa = target_pa;

    /* Find EPT entry for target page */
    ept_entry_t* entry = get_ept_entry(mgr->ept_pml4, target_pa);
    if (!entry) {
        return -1;
    }

    /* If large page, split it first */
    if (entry->large_page) {
        split_2mb_to_4kb(entry, target_pa & ~(PAGE_SIZE_2MB - 1));
        entry = get_ept_entry(mgr->ept_pml4, target_pa);
    }

    /* Install the hook */
    inline_hook_t hook = {
        .target_address = pa_to_va(target_pa),
        .handler_address = handler
    };

    /* Build hook bytes (jmp to handler) */
    build_hook_bytes(&hook);

    install_ept_hook(shadow, &hook, entry);

    mgr->hook_count++;
    return 0;
}

/*
 * Usage Notes:
 *
 * 1. EPT Shadow Hooking Flow:
 *    a. Allocate two pages per hook (exec + data)
 *    b. Copy original page content to both
 *    c. Install inline hook in exec page only
 *    d. Set EPT to execute-only pointing to exec page
 *    e. On read/write violation: swap to data page, enable MTF
 *    f. On MTF exit: restore execute-only mapping
 *
 * 2. Key Points:
 *    - Execute-only hides hooks from integrity checks
 *    - MTF single-stepping allows proper read/write handling
 *    - Split large pages for fine-grained control
 *    - Proper memory typing (MTRR) prevents cache issues
 *
 * 3. Testing:
 *    - Verify integrity checks see clean pages
 *    - Verify execution uses hooked pages
 *    - Check for BSODs from EPT violations
 *
 * Critical: EPT execute-only requires Intel EPT support with MBEC.
 */
