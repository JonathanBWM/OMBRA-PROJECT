// vmcall.c — VMCALL Exit Handler
// OmbraHypervisor

#include "handlers.h"
#include "../vmx.h"
#include "../timing.h"
#include "../nested.h"
#include "../hooks.h"
#include "../ept.h"
#include "../spoof.h"
#include "../../shared/vmcs_fields.h"

// =============================================================================
// VMCALL Interface Protocol
// =============================================================================
//
// To distinguish our VMCALLs from other hypervisor VMCALLs (e.g., Hyper-V),
// we use an obfuscated magic signature to defeat signature scanning.
//
// RAX = Magic signature (deobfuscates to unique value at runtime)
// RCX = Command code (non-sequential hash-based values)
// RDX = Parameter 1 (command-specific)
// R8  = Parameter 2 (command-specific)
// R9  = Parameter 3 (command-specific)
//
// Returns:
// RAX = Status code (0 = success, negative = error)
// RDX = Return value 1 (command-specific)
// R8  = Return value 2 (command-specific)
//
// Note: Magic and command codes defined in types.h with obfuscation

// Status codes
#define VMCALL_STATUS_SUCCESS           0
#define VMCALL_STATUS_INVALID_MAGIC     -1
#define VMCALL_STATUS_INVALID_COMMAND   -2
#define VMCALL_STATUS_INVALID_PARAM     -3
#define VMCALL_STATUS_NOT_IMPLEMENTED   -4
#define VMCALL_STATUS_ACCESS_DENIED     -5

// =============================================================================
// Physical Address Security Validation
// =============================================================================
//
// Blacklisted physical address ranges prevent VMCALL memory access to:
// 1. MMIO regions - reads/writes could trigger hardware state changes
// 2. Firmware regions - modification could brick system or leak secrets
// 3. APIC regions - direct APIC manipulation bypasses OS interrupt handling
//
// Security rationale:
// - Even ring 0 code shouldn't arbitrarily access MMIO (can cause hardware faults)
// - Firmware regions (SMM RAM, UEFI runtime) contain secrets and critical state
// - Cross-page accesses could leak data from adjacent protected pages if permissions differ
//
// This prevents malicious usermode payloads from abusing VMCALL to:
// - Dump SMM memory or UEFI variables
// - Trigger hardware bugs via malformed MMIO accesses
// - Read APIC state to fingerprint interrupt patterns

typedef struct {
    U64 Start;
    U64 End;
    const char* Description;
} PHYS_ADDR_BLACKLIST_ENTRY;

static const PHYS_ADDR_BLACKLIST_ENTRY g_PhysAddrBlacklist[] = {
    // Standard PC MMIO region (conservative range covering most devices)
    // Actual MMIO varies by chipset, but 0xFE000000-0xFFFFFFFF is common
    { 0xFE000000ULL, 0xFFFFFFFFULL, "Standard MMIO" },

    // Legacy VGA framebuffer and option ROM area
    // Writes could corrupt display or trigger legacy emulation bugs
    { 0x000A0000ULL, 0x000FFFFFULL, "Legacy VGA/ROM" },

    // Local APIC default base (can be relocated via IA32_APIC_BASE MSR)
    // Direct APIC access bypasses OS interrupt management
    { 0xFEE00000ULL, 0xFEE00FFFULL, "Local APIC" },

    // I/O APIC default address range
    // Direct I/O APIC manipulation could corrupt interrupt routing tables
    { 0xFEC00000ULL, 0xFEC00FFFULL, "IO APIC" },

    // Note: SMRAM regions not listed here because they're configured via
    // chipset-specific MSRs and vary by platform. For production, query
    // SMRAM range from IA32_SMRR_PHYSBASE/MASK MSRs and add dynamically.
};

#define PHYS_BLACKLIST_COUNT (sizeof(g_PhysAddrBlacklist) / sizeof(g_PhysAddrBlacklist[0]))

// Check if physical address range overlaps any blacklisted region
static bool IsPhysAddrBlacklisted(U64 physAddr, U64 size) {
    U64 endAddr = physAddr + size - 1;

    for (U32 i = 0; i < PHYS_BLACKLIST_COUNT; i++) {
        // Check if [physAddr, endAddr] overlaps with [Start, End]
        // Ranges overlap if: start1 <= end2 AND start2 <= end1
        if (physAddr <= g_PhysAddrBlacklist[i].End &&
            endAddr >= g_PhysAddrBlacklist[i].Start) {
            return true;  // Overlaps blacklisted region
        }
    }
    return false;
}

// Comprehensive validation for physical memory access via VMCALL
// Returns true if access is safe, false otherwise
static bool ValidatePhysAccess(U64 physAddr, U64 size) {
    // Validate size is a valid power-of-2 operand size
    if (size != 1 && size != 2 && size != 4 && size != 8) {
        return false;
    }

    // Check natural alignment for the operand size
    // Unaligned accesses could span cache lines or trigger #AC
    if ((physAddr & (size - 1)) != 0) {
        return false;
    }

    // Check address is within EPT identity-mapped range
    if (physAddr >= EPT_IDENTITY_MAP_SIZE) {
        return false;
    }

    // Check end address doesn't overflow or exceed EPT limit
    // Prevent integer overflow: if physAddr + size wraps, endAddr < physAddr
    U64 endAddr = physAddr + size;
    if (endAddr < physAddr || endAddr > EPT_IDENTITY_MAP_SIZE) {
        return false;
    }

    // Check against MMIO/firmware blacklist
    if (IsPhysAddrBlacklisted(physAddr, size)) {
        return false;
    }

    // Security: Reject accesses that cross 4KB page boundaries
    // Rationale: Adjacent pages may have different EPT permissions
    // (e.g., one page RW, next page execute-only for hook).
    // A cross-page read could leak execute-only code bytes.
    // A cross-page write could corrupt hook trampolines.
    //
    // Alternative: Walk EPT to verify both pages have same permissions,
    // but that's expensive and complex. Simpler to just reject cross-page.
    U64 startPage = physAddr >> 12;
    U64 endPage = (physAddr + size - 1) >> 12;
    if (startPage != endPage) {
        return false;  // Access spans pages - unsafe
    }

    return true;
}

// =============================================================================
// VMCALL Handlers
// =============================================================================

static I64 VmcallPing(GUEST_REGS* regs) {
    // PING - Return signature and version
    // Input: None
    // Output: RDX = version (debug only)
    //         R8  = feature flags (debug only)
    //
    // Security: In release builds, returns minimal response to prevent
    // version-based fingerprinting. Only confirms hypervisor is active.

#ifdef OMBRA_DEBUG
    // Debug builds: Full version and feature information
    regs->Rdx = 0x00010000;  // Version 1.0
    regs->R8 = 0;            // No special features yet
#else
    // Release builds: Minimal response - no version/feature information
    regs->Rdx = 0;
    regs->R8 = 0;
#endif

    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallGetStatus(GUEST_REGS* regs) {
    // GET_STATUS - Return hypervisor status
    // Input: None
    // Output: RDX = number of virtualized CPUs (debug only)
    //         R8  = total VM-exit count (debug only)
    //
    // Security: In release builds, this returns minimal information to prevent
    // anti-cheat fingerprinting via CPU count analysis and VM-exit statistics.

#ifdef OMBRA_DEBUG
    // Debug builds: Full status information for development/testing
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
#else
    // Release builds: Minimal response - just confirms hypervisor is running
    // No detailed information to prevent fingerprinting attacks
    (void)regs;
    return VMCALL_STATUS_SUCCESS;
#endif
}

static I64 VmcallHookInstall(GUEST_REGS* regs) {
    // HOOK_INSTALL - Install an EPT hook
    // Input: RDX = target physical address
    //        R8  = hook function physical address
    //        R9  = flags
    // Output: RDX = hook ID (or 0 on failure)

    VMX_CPU* cpu = VmxGetCurrentCpu();
    if (!cpu || !cpu->Ept) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    U64 targetPhys = regs->Rdx;
    U64 hookPhys = regs->R8;
    U64 flags = regs->R9;
    (void)flags;  // Reserved for future use

    // Validate addresses are within EPT range
    if (targetPhys >= EPT_IDENTITY_MAP_SIZE || hookPhys >= EPT_IDENTITY_MAP_SIZE) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Get hook manager (global state)
    extern HOOK_MANAGER g_HookManager;
    if (!g_HookManager.Initialized) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Install EPT hook
    EPT_HOOK* hook = NULL;
    void* handlerVirtual = (void*)hookPhys;  // Simplified - assumes identity mapping
    U64 targetVirtual = targetPhys;  // Simplified - physical as virtual for kernel addresses

    OMBRA_STATUS status = HookInstallEpt(
        &g_HookManager,
        targetVirtual,
        targetPhys,
        handlerVirtual,
        &hook
    );

    if (OMBRA_FAILED(status)) {
        regs->Rdx = 0;  // Hook ID 0 = failure
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Return hook ID (array index + 1, so 0 means failure)
    U32 hookId = ((U64)hook - (U64)&g_HookManager.Hooks[0]) / sizeof(EPT_HOOK) + 1;
    regs->Rdx = hookId;

    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallHookRemove(GUEST_REGS* regs) {
    // HOOK_REMOVE - Remove an EPT hook
    // Input: RDX = hook ID
    // Output: None

    U64 hookId = regs->Rdx;

    // Get hook manager
    extern HOOK_MANAGER g_HookManager;
    if (!g_HookManager.Initialized) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Validate hook ID (1-based, 0 = invalid)
    if (hookId == 0 || hookId > MAX_HOOKS) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Get hook pointer (ID is 1-based)
    EPT_HOOK* hook = &g_HookManager.Hooks[hookId - 1];

    // Remove the hook
    OMBRA_STATUS status = HookRemove(&g_HookManager, hook);
    if (OMBRA_FAILED(status)) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallReadPhys(GUEST_REGS* regs) {
    // READ_PHYS - Read from physical memory
    // Input: RDX = physical address
    //        R8  = size (1, 2, 4, or 8)
    // Output: RDX = value read

    U64 physAddr = regs->Rdx;
    U64 size = regs->R8;

    // Comprehensive security validation
    // Checks: size, alignment, EPT bounds, MMIO blacklist, cross-page boundary
    if (!ValidatePhysAccess(physAddr, size)) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Since we use identity mapping in EPT, physical address equals virtual
    // in our hypervisor's address space (kernel mode)
    void* virtualAddr = (void*)physAddr;

    U64 value = 0;
    switch (size) {
    case 1:
        value = *(volatile U8*)virtualAddr;
        break;
    case 2:
        value = *(volatile U16*)virtualAddr;
        break;
    case 4:
        value = *(volatile U32*)virtualAddr;
        break;
    case 8:
        value = *(volatile U64*)virtualAddr;
        break;
    }

    regs->Rdx = value;
    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallWritePhys(GUEST_REGS* regs) {
    // WRITE_PHYS - Write to physical memory
    // Input: RDX = physical address
    //        R8  = size (1, 2, 4, or 8)
    //        R9  = value to write
    // Output: None

    U64 physAddr = regs->Rdx;
    U64 size = regs->R8;
    U64 value = regs->R9;

    // Comprehensive security validation
    // Checks: size, alignment, EPT bounds, MMIO blacklist, cross-page boundary
    if (!ValidatePhysAccess(physAddr, size)) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Since we use identity mapping in EPT, physical address equals virtual
    void* virtualAddr = (void*)physAddr;

    switch (size) {
    case 1:
        *(volatile U8*)virtualAddr = (U8)value;
        break;
    case 2:
        *(volatile U16*)virtualAddr = (U16)value;
        break;
    case 4:
        *(volatile U32*)virtualAddr = (U32)value;
        break;
    case 8:
        *(volatile U64*)virtualAddr = value;
        break;
    }

    return VMCALL_STATUS_SUCCESS;
}

// Helper: Walk guest 4-level page tables (IA-32e paging)
U64 WalkGuestPageTables(U64 guestVirtual, U64 guestCr3) {
    // Extract indices from virtual address
    U64 pml4Index = (guestVirtual >> 39) & 0x1FF;
    U64 pdptIndex = (guestVirtual >> 30) & 0x1FF;
    U64 pdIndex   = (guestVirtual >> 21) & 0x1FF;
    U64 ptIndex   = (guestVirtual >> 12) & 0x1FF;
    U64 offset    = guestVirtual & 0xFFF;

    // Get PML4 base (clear lower 12 bits, ignore PCID in CR3)
    U64 pml4Base = guestCr3 & ~0xFFFULL;

    // Read PML4E (each entry is 8 bytes)
    U64 pml4ePhys = pml4Base + (pml4Index * 8);
    if (pml4ePhys >= EPT_IDENTITY_MAP_SIZE) return 0;
    U64 pml4e = *(volatile U64*)pml4ePhys;

    // Check present bit
    if (!(pml4e & 0x1)) return 0;

    // Get PDPT base
    U64 pdptBase = pml4e & 0x000FFFFFFFFFF000ULL;
    U64 pdptePhys = pdptBase + (pdptIndex * 8);
    if (pdptePhys >= EPT_IDENTITY_MAP_SIZE) return 0;
    U64 pdpte = *(volatile U64*)pdptePhys;

    // Check present bit
    if (!(pdpte & 0x1)) return 0;

    // Check for 1GB page (bit 7)
    if (pdpte & 0x80) {
        // 1GB page - bits 51:30 are physical address
        U64 pageBase = pdpte & 0x000FFFFFC0000000ULL;
        U64 pageOffset = guestVirtual & 0x3FFFFFFFULL;
        return pageBase | pageOffset;
    }

    // Get PD base
    U64 pdBase = pdpte & 0x000FFFFFFFFFF000ULL;
    U64 pdePhys = pdBase + (pdIndex * 8);
    if (pdePhys >= EPT_IDENTITY_MAP_SIZE) return 0;
    U64 pde = *(volatile U64*)pdePhys;

    // Check present bit
    if (!(pde & 0x1)) return 0;

    // Check for 2MB page (bit 7)
    if (pde & 0x80) {
        // 2MB page - bits 51:21 are physical address
        U64 pageBase = pde & 0x000FFFFFFFE00000ULL;
        U64 pageOffset = guestVirtual & 0x1FFFFFULL;
        return pageBase | pageOffset;
    }

    // Get PT base
    U64 ptBase = pde & 0x000FFFFFFFFFF000ULL;
    U64 ptePhys = ptBase + (ptIndex * 8);
    if (ptePhys >= EPT_IDENTITY_MAP_SIZE) return 0;
    U64 pte = *(volatile U64*)ptePhys;

    // Check present bit
    if (!(pte & 0x1)) return 0;

    // 4KB page - bits 51:12 are physical address
    U64 pageBase = pte & 0x000FFFFFFFFFF000ULL;
    return pageBase | offset;
}

static I64 VmcallVirtToPhys(GUEST_REGS* regs) {
    // VIRT_TO_PHYS - Convert guest virtual to physical address
    // Input: RDX = guest virtual address
    // Output: RDX = guest physical address

    U64 guestVirtual = regs->Rdx;

    // Read guest CR3 from VMCS
    U64 guestCr3 = VmcsRead(VMCS_GUEST_CR3);

    // Walk guest page tables
    U64 guestPhysical = WalkGuestPageTables(guestVirtual, guestCr3);

    if (guestPhysical == 0) {
        // Translation failed (page not present or invalid)
        return VMCALL_STATUS_INVALID_PARAM;
    }

    regs->Rdx = guestPhysical;
    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallReadVirt(GUEST_REGS* regs) {
    // READ_VIRT - Read from guest virtual address with cross-process support
    // Input: RDX = guest virtual address
    //        R8  = target CR3 (0 = use current guest CR3)
    //        R9  = size (1, 2, 4, or 8)
    // Output: RDX = value read

    U64 guestVa = regs->Rdx;
    U64 targetCr3 = regs->R8;
    U64 size = regs->R9;

    // Validate size
    if (size != 1 && size != 2 && size != 4 && size != 8) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Use current guest CR3 if not specified
    if (targetCr3 == 0) {
        targetCr3 = VmcsRead(VMCS_GUEST_CR3);
    }

    // Mask off PCID bits (lower 12 bits may contain PCID if enabled)
    targetCr3 &= ~0xFFFULL;

    // Translate VA → PA using target process page tables
    U64 physAddr = WalkGuestPageTables(guestVa, targetCr3);
    if (physAddr == 0) {
        return VMCALL_STATUS_INVALID_PARAM;  // Page not present
    }

    // Check if read crosses page boundary
    U64 pageOffset = guestVa & 0xFFF;
    if (pageOffset + size > 0x1000) {
        // Cross-page read: split into two operations
        U64 page1Bytes = 0x1000 - pageOffset;
        U64 page2Bytes = size - page1Bytes;
        U64 value = 0;

        // Read from first page
        if (physAddr >= EPT_IDENTITY_MAP_SIZE) {
            return VMCALL_STATUS_INVALID_PARAM;
        }
        for (U64 i = 0; i < page1Bytes; i++) {
            value |= ((U64)(*(volatile U8*)(physAddr + i))) << (i * 8);
        }

        // Translate second page
        U64 page2Va = (guestVa + page1Bytes) & ~0xFFFULL;  // Align to page
        U64 physAddr2 = WalkGuestPageTables(page2Va, targetCr3);
        if (physAddr2 == 0 || physAddr2 >= EPT_IDENTITY_MAP_SIZE) {
            return VMCALL_STATUS_INVALID_PARAM;
        }

        // Read from second page
        for (U64 i = 0; i < page2Bytes; i++) {
            value |= ((U64)(*(volatile U8*)(physAddr2 + i))) << ((page1Bytes + i) * 8);
        }

        regs->Rdx = value;
        return VMCALL_STATUS_SUCCESS;
    }

    // Single-page read: validate bounds (including size to prevent overflow)
    if (physAddr >= EPT_IDENTITY_MAP_SIZE || physAddr + size > EPT_IDENTITY_MAP_SIZE) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Check alignment
    if ((physAddr & (size - 1)) != 0) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Read from physical memory (identity mapped in EPT)
    void* virtualAddr = (void*)physAddr;
    U64 value = 0;

    switch (size) {
    case 1:
        value = *(volatile U8*)virtualAddr;
        break;
    case 2:
        value = *(volatile U16*)virtualAddr;
        break;
    case 4:
        value = *(volatile U32*)virtualAddr;
        break;
    case 8:
        value = *(volatile U64*)virtualAddr;
        break;
    }

    regs->Rdx = value;
    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallWriteVirt(GUEST_REGS* regs) {
    // WRITE_VIRT - Write to guest virtual address with cross-process support
    // Input: RDX = guest virtual address
    //        R8  = target CR3 (0 = use current guest CR3)
    //        R9  = size (1, 2, 4, or 8)
    //        R10 = value to write
    // Output: None

    U64 guestVa = regs->Rdx;
    U64 targetCr3 = regs->R8;
    U64 size = regs->R9;
    U64 value = regs->R10;

    // Validate size
    if (size != 1 && size != 2 && size != 4 && size != 8) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Use current guest CR3 if not specified
    if (targetCr3 == 0) {
        targetCr3 = VmcsRead(VMCS_GUEST_CR3);
    }

    // Mask off PCID bits
    targetCr3 &= ~0xFFFULL;

    // Translate VA → PA using target process page tables
    U64 physAddr = WalkGuestPageTables(guestVa, targetCr3);
    if (physAddr == 0) {
        return VMCALL_STATUS_INVALID_PARAM;  // Page not present
    }

    // Check if write crosses page boundary
    U64 pageOffset = guestVa & 0xFFF;
    if (pageOffset + size > 0x1000) {
        // Cross-page write: split into two operations
        U64 page1Bytes = 0x1000 - pageOffset;
        U64 page2Bytes = size - page1Bytes;

        // Write to first page
        if (physAddr >= EPT_IDENTITY_MAP_SIZE) {
            return VMCALL_STATUS_INVALID_PARAM;
        }
        for (U64 i = 0; i < page1Bytes; i++) {
            *(volatile U8*)(physAddr + i) = (U8)(value >> (i * 8));
        }

        // Translate second page
        U64 page2Va = (guestVa + page1Bytes) & ~0xFFFULL;  // Align to page
        U64 physAddr2 = WalkGuestPageTables(page2Va, targetCr3);
        if (physAddr2 == 0 || physAddr2 >= EPT_IDENTITY_MAP_SIZE) {
            return VMCALL_STATUS_INVALID_PARAM;
        }

        // Write to second page
        for (U64 i = 0; i < page2Bytes; i++) {
            *(volatile U8*)(physAddr2 + i) = (U8)(value >> ((page1Bytes + i) * 8));
        }

        return VMCALL_STATUS_SUCCESS;
    }

    // Single-page write: validate bounds (including size to prevent overflow)
    if (physAddr >= EPT_IDENTITY_MAP_SIZE || physAddr + size > EPT_IDENTITY_MAP_SIZE) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Check alignment
    if ((physAddr & (size - 1)) != 0) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Write to physical memory (identity mapped in EPT)
    void* virtualAddr = (void*)physAddr;

    switch (size) {
    case 1:
        *(volatile U8*)virtualAddr = (U8)value;
        break;
    case 2:
        *(volatile U16*)virtualAddr = (U16)value;
        break;
    case 4:
        *(volatile U32*)virtualAddr = (U32)value;
        break;
    case 8:
        *(volatile U64*)virtualAddr = value;
        break;
    }

    return VMCALL_STATUS_SUCCESS;
}

// =============================================================================
// Process CR3 Lookup via EPROCESS Walking
// =============================================================================
//
// Walks the kernel's ActiveProcessLinks list starting from PsInitialSystemProcess
// to find the EPROCESS for a given PID, then reads DirectoryTableBase (CR3).
//
// This works because:
// 1. We're in VMX root with full physical memory access
// 2. EPROCESS structures are in non-paged pool (always resident)
// 3. ActiveProcessLinks is a circular doubly-linked list
//

static I64 VmcallGetProcessCr3(GUEST_REGS* regs) {
    // GET_PROCESS_CR3 - Get CR3 for a process by PID
    // Input: RDX = target PID
    //        R8  = PsInitialSystemProcess kernel address (from usermode)
    // Output: RDX = DirectoryTableBase (CR3) or 0 if not found

    U64 targetPid = regs->Rdx;
    U64 systemEprocess = regs->R8;

    // Validate inputs
    if (targetPid == 0 || systemEprocess == 0) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Use current CR3 (kernel CR3) to walk kernel structures
    U64 kernelCr3 = VmcsRead(VMCS_GUEST_CR3) & ~0xFFFULL;

    // Translate PsInitialSystemProcess VA to PA
    U64 systemEprocessPhys = WalkGuestPageTables(systemEprocess, kernelCr3);
    if (systemEprocessPhys == 0 || systemEprocessPhys >= EPT_IDENTITY_MAP_SIZE) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Get the ActiveProcessLinks list head from System process
    U64 listHeadPhys = systemEprocessPhys + EPROCESS_ACTIVEPROCESSLINKS_OFFSET;
    if (listHeadPhys >= EPT_IDENTITY_MAP_SIZE) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Read the first Flink from System's ActiveProcessLinks
    U64 listHeadFlink = *(volatile U64*)listHeadPhys;

    // Walk the process list (up to 4096 processes to prevent infinite loop)
    U64 currentLink = listHeadFlink;
    for (U32 i = 0; i < 4096; i++) {
        // Calculate EPROCESS base from LIST_ENTRY pointer
        // LIST_ENTRY is at offset ActiveProcessLinks within EPROCESS
        U64 currentEprocessVa = currentLink - EPROCESS_ACTIVEPROCESSLINKS_OFFSET;

        // Translate to physical
        U64 currentEprocessPhys = WalkGuestPageTables(currentEprocessVa, kernelCr3);
        if (currentEprocessPhys == 0 || currentEprocessPhys >= EPT_IDENTITY_MAP_SIZE) {
            break;  // Invalid translation, stop walking
        }

        // Read UniqueProcessId
        U64 pidPhys = currentEprocessPhys + EPROCESS_UNIQUEPROCESSID_OFFSET;
        if (pidPhys >= EPT_IDENTITY_MAP_SIZE) break;
        U64 pid = *(volatile U64*)pidPhys;

        // Sanity check: PIDs on Windows are typically < 65536
        // If we read garbage, the list is corrupted - stop walking
        if (pid > 0x100000) {
            break;
        }

        // Check if this is our target
        if (pid == targetPid) {
            // Found it! Read DirectoryTableBase
            U64 dtbPhys = currentEprocessPhys + EPROCESS_DIRECTORYTABLEBASE_OFFSET;
            if (dtbPhys >= EPT_IDENTITY_MAP_SIZE) {
                return VMCALL_STATUS_INVALID_PARAM;
            }
            U64 cr3 = *(volatile U64*)dtbPhys;

            // Mask off any flags in CR3 (PCID, etc)
            regs->Rdx = cr3 & ~0xFFFULL;
            return VMCALL_STATUS_SUCCESS;
        }

        // Move to next process
        U64 linkPhys = currentEprocessPhys + EPROCESS_ACTIVEPROCESSLINKS_OFFSET;
        if (linkPhys >= EPT_IDENTITY_MAP_SIZE) break;
        U64 nextLink = *(volatile U64*)linkPhys;  // Read Flink

        // Check if we've wrapped around
        if (nextLink == listHeadFlink || nextLink == 0) {
            break;
        }

        currentLink = nextLink;
    }

    // Process not found
    regs->Rdx = 0;
    return VMCALL_STATUS_INVALID_PARAM;
}

// =============================================================================
// Nested Virtualization VMCALL Handlers
// =============================================================================

static I64 VmcallNestedGetInfo(GUEST_REGS* regs) {
    // NESTED_GET_INFO - Get nested virtualization status
    // Input: None
    // Output: RDX = nested status flags (debug only)
    //         R8  = L0 hypervisor type (debug only)
    //         R9  = feature flags (debug only)
    //
    // Security: In release builds, limits information disclosure about the
    // host hypervisor environment to prevent fingerprinting attacks.

#ifdef OMBRA_DEBUG
    // Debug builds: Full nested virtualization information
    U64 flags = 0;

    // Build status flags
    if (NestedIsRunningNested()) {
        flags |= 0x01;  // Bit 0: Running nested
    }
    if (NestedIsHyperV()) {
        flags |= 0x02;  // Bit 1: Under Hyper-V
    }

    regs->Rdx = flags;
    regs->R8 = (U64)NestedGetL0Type();
    regs->R9 = NestedGetHyperVFeatures();

    return VMCALL_STATUS_SUCCESS;
#else
    // Release builds: Minimal response
    // Only indicate if nested virtualization is active (boolean)
    regs->Rdx = NestedIsRunningNested() ? 1 : 0;
    regs->R8 = 0;  // No hypervisor type disclosure
    regs->R9 = 0;  // No feature flags disclosure

    return VMCALL_STATUS_SUCCESS;
#endif
}

static I64 VmcallNestedEnable(GUEST_REGS* regs) {
    // NESTED_ENABLE - Enable nested virtualization features
    // Input: RDX = feature mask to enable
    // Output: RDX = currently enabled features

    VMX_CPU* cpu = VmxGetCurrentCpu();
    U64 features = regs->Rdx;

    (void)features;  // Reserved for future feature selection

    // Enable VMCS shadowing if available and requested
    if (cpu && NestedIsRunningNested()) {
        OMBRA_STATUS status = NestedEnableVmcsShadowing(cpu);
        if (status == OMBRA_SUCCESS) {
            regs->Rdx = 0x01;  // VMCS shadowing enabled
            return VMCALL_STATUS_SUCCESS;
        }
    }

    regs->Rdx = 0;
    return VMCALL_STATUS_NOT_IMPLEMENTED;
}

static I64 VmcallNestedDisable(GUEST_REGS* regs) {
    // NESTED_DISABLE - Disable nested virtualization features
    // Input: RDX = feature mask to disable
    // Output: RDX = remaining enabled features

    VMX_CPU* cpu = VmxGetCurrentCpu();

    if (cpu) {
        NestedDisableVmcsShadowing(cpu);
    }

    regs->Rdx = 0;  // All features disabled
    return VMCALL_STATUS_SUCCESS;
}

// =============================================================================
// NMI Blocking for EAC Evasion
// =============================================================================

static I64 VmcallNmiBlockCr3(GUEST_REGS* regs) {
    // NMI_BLOCK_CR3 - Block NMIs for specific CR3 (process)
    // Input: RDX = CR3 to block NMIs for
    // Output: RDX = slot index used

    VMX_CPU* cpu = VmxGetCurrentCpu();
    U64 cr3 = regs->Rdx & ~0xFFFULL;  // Mask PCID bits

    if (!cpu) return VMCALL_STATUS_INVALID_PARAM;

    // Check if already blocked
    for (U32 i = 0; i < cpu->NmiBlockedCount; i++) {
        if (cpu->NmiBlockedCr3[i] == cr3) {
            regs->Rdx = i;
            return VMCALL_STATUS_SUCCESS;
        }
    }

    // Add to blocked list
    if (cpu->NmiBlockedCount >= MAX_NMI_BLOCKED_CR3) {
        return VMCALL_STATUS_INVALID_PARAM;  // List full
    }

    U32 slot = cpu->NmiBlockedCount++;
    cpu->NmiBlockedCr3[slot] = cr3;
    regs->Rdx = slot;

    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallNmiUnblockCr3(GUEST_REGS* regs) {
    // NMI_UNBLOCK_CR3 - Unblock NMIs for specific CR3
    // Input: RDX = CR3 to unblock
    // Output: RDX = queued NMI count that will be delivered

    VMX_CPU* cpu = VmxGetCurrentCpu();
    U64 cr3 = regs->Rdx & ~0xFFFULL;

    if (!cpu) return VMCALL_STATUS_INVALID_PARAM;

    for (U32 i = 0; i < cpu->NmiBlockedCount; i++) {
        if (cpu->NmiBlockedCr3[i] == cr3) {
            // Remove by shifting remaining entries
            for (U32 j = i; j < cpu->NmiBlockedCount - 1; j++) {
                cpu->NmiBlockedCr3[j] = cpu->NmiBlockedCr3[j + 1];
            }
            cpu->NmiBlockedCount--;

            // Return queued count (and clear it)
            regs->Rdx = cpu->NmiQueuedCount;
            cpu->NmiQueuedCount = 0;
            return VMCALL_STATUS_SUCCESS;
        }
    }

    return VMCALL_STATUS_INVALID_PARAM;  // Not found
}

static I64 VmcallNmiEnable(GUEST_REGS* regs) {
    // NMI_ENABLE - Enable NMI blocking feature
    VMX_CPU* cpu = VmxGetCurrentCpu();
    if (!cpu) return VMCALL_STATUS_INVALID_PARAM;

    cpu->NmiBlockingEnabled = true;
    regs->Rdx = 1;
    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallNmiDisable(GUEST_REGS* regs) {
    // NMI_DISABLE - Disable NMI blocking (deliver all queued NMIs)
    VMX_CPU* cpu = VmxGetCurrentCpu();
    if (!cpu) return VMCALL_STATUS_INVALID_PARAM;

    cpu->NmiBlockingEnabled = false;
    cpu->NmiBlockedCount = 0;

    // Report queued count before clearing
    regs->Rdx = cpu->NmiQueuedCount;
    cpu->NmiQueuedCount = 0;

    return VMCALL_STATUS_SUCCESS;
}

// =============================================================================
// SMBIOS Spoofing via EPT Shadow Page
// =============================================================================
//
// Technique from old project: Create a shadow copy of the SMBIOS table,
// randomize serial numbers, then use EPT to redirect reads to the shadow
// while writes still go to the original.
//
// This defeats anti-cheat hardware fingerprinting via SMBIOS queries.

// Simple PRNG for generating random serial characters
static U32 g_SmbiosSeed = 0x12345678;

static U32 SmbiosRand(void) {
    g_SmbiosSeed = g_SmbiosSeed * 1103515245 + 12345;
    return (g_SmbiosSeed >> 16) & 0x7FFF;
}

static void RandomizeSmbiosString(U8* str, U32 len) {
    // Randomize string in-place, preserving alphanumeric pattern
    static const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (U32 i = 0; i < len && str[i] != 0; i++) {
        if ((str[i] >= '0' && str[i] <= '9') ||
            (str[i] >= 'A' && str[i] <= 'Z') ||
            (str[i] >= 'a' && str[i] <= 'z')) {
            str[i] = charset[SmbiosRand() % 36];
        }
    }
}

static I64 VmcallSpoofSmbios(GUEST_REGS* regs) {
    // SPOOF_SMBIOS - Setup SMBIOS shadow page with randomized serials
    // Input: RDX = Physical address of SMBIOS table
    //        R8  = Size of SMBIOS table
    //        R9  = Random seed for PRNG
    // Output: RDX = Shadow page physical address

    VMX_CPU* cpu = VmxGetCurrentCpu();
    U64 smbiosPhys = regs->Rdx;
    U64 smbiosSize = regs->R8;
    U32 seed = (U32)regs->R9;

    if (!cpu || !cpu->Ept) return VMCALL_STATUS_INVALID_PARAM;
    if (smbiosPhys >= EPT_IDENTITY_MAP_SIZE) return VMCALL_STATUS_INVALID_PARAM;
    if (smbiosSize == 0 || smbiosSize > 0x10000) return VMCALL_STATUS_INVALID_PARAM;

    // Set PRNG seed
    if (seed != 0) g_SmbiosSeed = seed;

    // Use hook framework to install EPT shadow
    extern HOOK_MANAGER g_HookManager;
    if (!g_HookManager.Initialized) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Allocate shadow page
    U64 shadowPhys;
    void* shadowVirt = HookAllocateShadowPage(&g_HookManager, &shadowPhys);
    if (!shadowVirt) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Copy original SMBIOS to shadow (identity mapping)
    U8* origVirt = (U8*)smbiosPhys;  // Identity mapped
    U8* shadow = (U8*)shadowVirt;
    for (U64 i = 0; i < smbiosSize && i < 4096; i++) {
        shadow[i] = origVirt[i];
    }

    // Find and randomize SMBIOS strings
    // SMBIOS structure: Header + String table (null-terminated strings)
    // We look for common serial number patterns and randomize them
    U8* ptr = shadow;
    U8* end = shadow + (smbiosSize < 4096 ? smbiosSize : 4096);

    while (ptr < end - 4) {
        // Look for SMBIOS structure headers (type, length, handle)
        U8 type = ptr[0];
        U8 length = ptr[1];

        if (length < 4 || ptr + length >= end) break;

        // After the formatted area, strings are null-terminated
        U8* strArea = ptr + length;
        while (strArea < end - 1) {
            if (*strArea == 0) {
                if (*(strArea + 1) == 0) {
                    // Double null = end of structure
                    ptr = strArea + 2;
                    break;
                }
                strArea++;
            } else {
                // Found a string - randomize if it looks like a serial
                U32 strLen = 0;
                while (strArea + strLen < end && strArea[strLen] != 0) strLen++;

                // Randomize strings > 4 chars (likely serials, not labels)
                if (strLen > 4 && strLen < 64) {
                    RandomizeSmbiosString(strArea, strLen);
                }
                strArea += strLen;
            }
        }

        if (strArea >= end) break;
        (void)type;  // May use for selective spoofing later
    }

    // Setup EPT shadow: reads go to shadow page
    // Use hook framework's inline hook mechanism on the page
    OMBRA_STATUS status = EptSplit2MbTo4Kb(cpu->Ept, smbiosPhys);
    if (OMBRA_FAILED(status)) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    U64* pte = EptGetEntry(cpu->Ept, smbiosPhys);
    if (!pte) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Point to shadow page for all accesses
    EPT_PTE* entry = (EPT_PTE*)pte;
    entry->PagePhysAddr = shadowPhys >> 12;

    EptInvalidate(cpu->Ept, INVEPT_TYPE_SINGLE_CONTEXT);

    regs->Rdx = shadowPhys;
    return VMCALL_STATUS_SUCCESS;
}

// =============================================================================
// Disk/NIC Hardware Spoofing
// =============================================================================
//
// Uses the spoof manager (spoof.c) to intercept and modify hardware
// identification queries. Unlike SMBIOS spoofing which uses EPT shadows,
// this hooks the DeviceIoControl return path to modify responses.

static I64 VmcallSpoofInit(GUEST_REGS* regs) {
    // SPOOF_INIT - Initialize spoof manager
    // Input: RDX = random seed for auto-generated serials/MACs
    // Output: RDX = number of disk slots available
    //         R8  = number of NIC slots available

    U32 seed = (U32)regs->Rdx;

    OMBRA_STATUS status = SpoofManagerInit(&g_SpoofManager);
    if (OMBRA_FAILED(status)) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Set custom seed if provided
    if (seed != 0) {
        g_SpoofManager.RandomSeed = seed;
    }

    regs->Rdx = MAX_SPOOFED_DISKS;
    regs->R8 = MAX_SPOOFED_NICS;

    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallSpoofAddDisk(GUEST_REGS* regs) {
    // SPOOF_ADD_DISK - Add disk serial spoof entry
    // Input: RDX = disk index (0-7)
    //        R8  = pointer to serial string (guest VA, null = auto-generate)
    //        R9  = pointer to model string (guest VA, null = keep original)
    // Output: RDX = assigned slot index

    U32 diskIndex = (U32)regs->Rdx;
    U64 serialVa = regs->R8;
    U64 modelVa = regs->R9;

    char serial[SERIAL_MAX_LEN] = {0};
    char model[SERIAL_MAX_LEN] = {0};

    // Read serial string from guest if provided
    if (serialVa != 0) {
        U64 guestCr3 = VmcsRead(VMCS_GUEST_CR3) & ~0xFFFULL;
        U64 serialPhys = WalkGuestPageTables(serialVa, guestCr3);
        if (serialPhys != 0 && serialPhys < EPT_IDENTITY_MAP_SIZE) {
            // Copy string from guest (max SERIAL_MAX_LEN - 1 chars)
            volatile char* src = (volatile char*)serialPhys;
            for (U32 i = 0; i < SERIAL_MAX_LEN - 1 && src[i] != 0; i++) {
                serial[i] = src[i];
            }
        }
    }

    // Read model string from guest if provided
    if (modelVa != 0) {
        U64 guestCr3 = VmcsRead(VMCS_GUEST_CR3) & ~0xFFFULL;
        U64 modelPhys = WalkGuestPageTables(modelVa, guestCr3);
        if (modelPhys != 0 && modelPhys < EPT_IDENTITY_MAP_SIZE) {
            volatile char* src = (volatile char*)modelPhys;
            for (U32 i = 0; i < SERIAL_MAX_LEN - 1 && src[i] != 0; i++) {
                model[i] = src[i];
            }
        }
    }

    // Add to spoof manager (pass NULL if empty to auto-generate)
    OMBRA_STATUS status = SpoofAddDisk(&g_SpoofManager, diskIndex,
                                       serial[0] ? serial : NULL,
                                       model[0] ? model : NULL);
    if (OMBRA_FAILED(status)) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    regs->Rdx = diskIndex;
    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallSpoofDelDisk(GUEST_REGS* regs) {
    // SPOOF_DEL_DISK - Remove disk serial spoof entry
    // Input: RDX = disk index
    // Output: None

    U32 diskIndex = (U32)regs->Rdx;

    OMBRA_STATUS status = SpoofRemoveDisk(&g_SpoofManager, diskIndex);
    if (OMBRA_FAILED(status)) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallSpoofAddNic(GUEST_REGS* regs) {
    // SPOOF_ADD_NIC - Add NIC MAC spoof entry
    // Input: RDX = NIC index (0-3)
    //        R8  = pointer to 6-byte MAC (guest VA, null = auto-generate)
    // Output: RDX = assigned slot index

    U32 nicIndex = (U32)regs->Rdx;
    U64 macVa = regs->R8;

    U8 mac[MAC_ADDR_LEN] = {0};
    U8* macPtr = NULL;

    // Read MAC from guest if provided
    if (macVa != 0) {
        U64 guestCr3 = VmcsRead(VMCS_GUEST_CR3) & ~0xFFFULL;
        U64 macPhys = WalkGuestPageTables(macVa, guestCr3);
        if (macPhys != 0 && macPhys < EPT_IDENTITY_MAP_SIZE) {
            volatile U8* src = (volatile U8*)macPhys;
            for (U32 i = 0; i < MAC_ADDR_LEN; i++) {
                mac[i] = src[i];
            }
            macPtr = mac;
        }
    }

    // Add to spoof manager (pass NULL to auto-generate)
    OMBRA_STATUS status = SpoofAddNic(&g_SpoofManager, nicIndex, macPtr);
    if (OMBRA_FAILED(status)) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    regs->Rdx = nicIndex;
    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallSpoofDelNic(GUEST_REGS* regs) {
    // SPOOF_DEL_NIC - Remove NIC MAC spoof entry
    // Input: RDX = NIC index
    // Output: None

    U32 nicIndex = (U32)regs->Rdx;

    OMBRA_STATUS status = SpoofRemoveNic(&g_SpoofManager, nicIndex);
    if (OMBRA_FAILED(status)) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallSpoofQuery(GUEST_REGS* regs) {
    // SPOOF_QUERY - Query spoof manager status
    // Input: None
    // Output: RDX = status flags (bit 0 = initialized, debug: bit 1 = disk active, bit 2 = NIC active)
    //         R8  = disk spoof count (debug only)
    //         R9  = NIC spoof count (debug only)
    //
    // Security: In release builds, limits detailed spoof statistics to prevent
    // fingerprinting of the spoofing configuration.

    U64 flags = 0;

    if (g_SpoofManager.Initialized) {
        flags |= 0x01;
    }

#ifdef OMBRA_DEBUG
    // Debug builds: Full spoof manager statistics
    if (g_SpoofManager.DiskSpoofEnabled) {
        flags |= 0x02;
    }
    if (g_SpoofManager.NicSpoofEnabled) {
        flags |= 0x04;
    }

    regs->Rdx = flags;
    regs->R8 = g_SpoofManager.DiskCount;
    regs->R9 = g_SpoofManager.NicCount;
#else
    // Release builds: Minimal response - only initialization status
    regs->Rdx = flags;
    regs->R8 = 0;
    regs->R9 = 0;
#endif

    return VMCALL_STATUS_SUCCESS;
}

// =============================================================================
// Phase 3 Driver VMCALLs
// =============================================================================

// Phase 3 VMCALL command codes (from driver/vmcall.h)
#define VMCALL_WATCH_CR3            0x4A8E71B2ULL
#define VMCALL_UNWATCH_CR3          0x9F3C2D4EULL
#define VMCALL_PIN_PAGE             0x7B1F4C93ULL
#define VMCALL_UNPIN_PAGE           0x2D9A6E17ULL
#define VMCALL_ALLOC_PHYSICAL_PAGE  0xC6E3A941ULL
#define VMCALL_FREE_PHYSICAL_PAGE   0x5F2B8D7CULL
#define VMCALL_COPY_PHYSICAL_PAGE   0x8A4E1CF5ULL
#define VMCALL_SPLIT_EPT_PAGE       0x3D7F9A21ULL
#define VMCALL_UNSPLIT_EPT_PAGE     0xB4C2E68DULL
#define VMCALL_DRIVER_READY         0x6F1A8D39ULL
#define VMCALL_DRIVER_SHUTDOWN      0x1E9C4B7AULL
#define VMCALL_HIDE_MEMORY          0xA3F5E219ULL
#define VMCALL_UNHIDE_MEMORY        0x4D2C7B8EULL

// Watched CR3 list for process tracking
#define MAX_WATCHED_CR3 32
static U64 g_WatchedCr3[MAX_WATCHED_CR3] = {0};
static U32 g_WatchedCr3Count = 0;

static I64 VmcallWatchCr3(GUEST_REGS* regs) {
    U64 cr3 = regs->Rdx & ~0xFFFULL;

    // Check if already watched
    for (U32 i = 0; i < g_WatchedCr3Count; i++) {
        if (g_WatchedCr3[i] == cr3) {
            return VMCALL_STATUS_SUCCESS;  // Already watching
        }
    }

    if (g_WatchedCr3Count >= MAX_WATCHED_CR3) {
        return VMCALL_STATUS_INVALID_PARAM;  // List full
    }

    g_WatchedCr3[g_WatchedCr3Count++] = cr3;
    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallUnwatchCr3(GUEST_REGS* regs) {
    U64 cr3 = regs->Rdx & ~0xFFFULL;

    for (U32 i = 0; i < g_WatchedCr3Count; i++) {
        if (g_WatchedCr3[i] == cr3) {
            // Remove by shifting
            for (U32 j = i; j < g_WatchedCr3Count - 1; j++) {
                g_WatchedCr3[j] = g_WatchedCr3[j + 1];
            }
            g_WatchedCr3Count--;
            return VMCALL_STATUS_SUCCESS;
        }
    }
    return VMCALL_STATUS_INVALID_PARAM;  // Not found
}

// Pinned page list for preventing page eviction/modification
#define MAX_PINNED_PAGES 64
static U64 g_PinnedPages[MAX_PINNED_PAGES] = {0};
static U32 g_PinnedPageCount = 0;

static I64 VmcallPinPage(GUEST_REGS* regs) {
    U64 physAddr = regs->Rdx & ~0xFFFULL;  // Page-align

    if (physAddr >= EPT_IDENTITY_MAP_SIZE) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Check if already pinned
    for (U32 i = 0; i < g_PinnedPageCount; i++) {
        if (g_PinnedPages[i] == physAddr) {
            return VMCALL_STATUS_SUCCESS;  // Already pinned
        }
    }

    if (g_PinnedPageCount >= MAX_PINNED_PAGES) {
        return VMCALL_STATUS_INVALID_PARAM;  // List full
    }

    g_PinnedPages[g_PinnedPageCount++] = physAddr;
    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallUnpinPage(GUEST_REGS* regs) {
    U64 physAddr = regs->Rdx & ~0xFFFULL;  // Page-align

    for (U32 i = 0; i < g_PinnedPageCount; i++) {
        if (g_PinnedPages[i] == physAddr) {
            // Remove by shifting
            for (U32 j = i; j < g_PinnedPageCount - 1; j++) {
                g_PinnedPages[j] = g_PinnedPages[j + 1];
            }
            g_PinnedPageCount--;
            return VMCALL_STATUS_SUCCESS;
        }
    }
    return VMCALL_STATUS_INVALID_PARAM;  // Not found
}

static I64 VmcallAllocPhysicalPage(GUEST_REGS* regs) {
    // Allocate physical page for shadow hooks
    extern HOOK_MANAGER g_HookManager;
    if (!g_HookManager.Initialized) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    U64 physAddr;
    void* virtAddr = HookAllocateShadowPage(&g_HookManager, &physAddr);
    if (!virtAddr) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    regs->Rdx = physAddr;
    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallFreePhysicalPage(GUEST_REGS* regs) {
    U64 physAddr = regs->Rdx;

    extern HOOK_MANAGER g_HookManager;
    if (!g_HookManager.Initialized) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Would need proper tracking to free - for now just mark success
    // Real implementation would use a freelist
    (void)physAddr;
    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallCopyPhysicalPage(GUEST_REGS* regs) {
    U64 srcPhys = regs->Rdx;
    U64 dstPhys = regs->R8;

    if (srcPhys >= EPT_IDENTITY_MAP_SIZE || dstPhys >= EPT_IDENTITY_MAP_SIZE) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Copy 4KB page (identity mapped)
    volatile U8* src = (volatile U8*)srcPhys;
    volatile U8* dst = (volatile U8*)dstPhys;
    for (U64 i = 0; i < 4096; i++) {
        dst[i] = src[i];
    }

    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallSplitEptPage(GUEST_REGS* regs) {
    VMX_CPU* cpu = VmxGetCurrentCpu();
    U64 targetCr3 = regs->Rdx;
    U64 guestPhys = regs->R8;
    U64 cleanPhys = regs->R9;

    if (!cpu || !cpu->Ept) return VMCALL_STATUS_INVALID_PARAM;
    if (guestPhys >= EPT_IDENTITY_MAP_SIZE) return VMCALL_STATUS_INVALID_PARAM;

    (void)targetCr3;  // Per-process EPT would use this
    (void)cleanPhys;  // Used for shadow page read view

    // Split 2MB page to 4KB for fine-grained control
    OMBRA_STATUS status = EptSplit2MbTo4Kb(cpu->Ept, guestPhys);
    if (OMBRA_FAILED(status)) {
        return VMCALL_STATUS_INVALID_PARAM;
    }

    // Get the 4KB PTE for this page
    U64* pte = EptGetEntry(cpu->Ept, guestPhys);
    if (!pte) return VMCALL_STATUS_INVALID_PARAM;

    // Setup execute-only view: execute sees guestPhys, read/write sees cleanPhys
    // For true shadow pages, we'd need per-permission EPT views
    // Simplified: just mark as execute-only for now
    EPT_PTE* entry = (EPT_PTE*)pte;
    entry->Read = 0;
    entry->Write = 0;
    entry->Execute = 1;

    EptInvalidate(cpu->Ept, INVEPT_TYPE_SINGLE_CONTEXT);

    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallUnsplitEptPage(GUEST_REGS* regs) {
    VMX_CPU* cpu = VmxGetCurrentCpu();
    U64 guestPhys = regs->R8;

    if (!cpu || !cpu->Ept) return VMCALL_STATUS_INVALID_PARAM;

    // Restore normal EPT permissions
    U64* pte = EptGetEntry(cpu->Ept, guestPhys);
    if (pte) {
        EPT_PTE* entry = (EPT_PTE*)pte;
        entry->Read = 1;
        entry->Write = 1;
        entry->Execute = 1;
        EptInvalidate(cpu->Ept, INVEPT_TYPE_SINGLE_CONTEXT);
    }

    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallDriverReady(GUEST_REGS* regs) {
    U64 ownerCr3 = regs->Rdx;

    // Register driver's CR3 so we can track it
    g_Ombra.DriverCr3 = ownerCr3;
    g_Ombra.DriverLoaded = true;

    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallDriverShutdown(GUEST_REGS* regs) {
    (void)regs;

    g_Ombra.DriverLoaded = false;
    g_Ombra.DriverCr3 = 0;

    // Clear watched CR3 list
    g_WatchedCr3Count = 0;

    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallHideMemory(GUEST_REGS* regs) {
    VMX_CPU* cpu = VmxGetCurrentCpu();
    U64 addr = regs->Rdx;
    U64 size = regs->R8;

    if (!cpu || !cpu->Ept) return VMCALL_STATUS_INVALID_PARAM;
    if (addr >= EPT_IDENTITY_MAP_SIZE) return VMCALL_STATUS_INVALID_PARAM;

    // Page-align
    U64 pageStart = addr & ~0xFFFULL;
    U64 pageEnd = (addr + size + 0xFFF) & ~0xFFFULL;

    for (U64 pa = pageStart; pa < pageEnd; pa += 0x1000) {
        if (pa >= EPT_IDENTITY_MAP_SIZE) break;

        // Ensure 4KB granularity
        EptSplit2MbTo4Kb(cpu->Ept, pa);

        // Mark as no-access
        U64* pte = EptGetEntry(cpu->Ept, pa);
        if (pte) {
            EPT_PTE* entry = (EPT_PTE*)pte;
            entry->Read = 0;
            entry->Write = 0;
            entry->Execute = 0;
        }
    }

    EptInvalidate(cpu->Ept, INVEPT_TYPE_SINGLE_CONTEXT);
    return VMCALL_STATUS_SUCCESS;
}

static I64 VmcallUnhideMemory(GUEST_REGS* regs) {
    VMX_CPU* cpu = VmxGetCurrentCpu();
    U64 addr = regs->Rdx;
    U64 size = regs->R8;

    if (!cpu || !cpu->Ept) return VMCALL_STATUS_INVALID_PARAM;

    U64 pageStart = addr & ~0xFFFULL;
    U64 pageEnd = (addr + size + 0xFFF) & ~0xFFFULL;

    for (U64 pa = pageStart; pa < pageEnd; pa += 0x1000) {
        if (pa >= EPT_IDENTITY_MAP_SIZE) break;

        U64* pte = EptGetEntry(cpu->Ept, pa);
        if (pte) {
            EPT_PTE* entry = (EPT_PTE*)pte;
            entry->Read = 1;
            entry->Write = 1;
            entry->Execute = 1;
        }
    }

    EptInvalidate(cpu->Ept, INVEPT_TYPE_SINGLE_CONTEXT);
    return VMCALL_STATUS_SUCCESS;
}

// =============================================================================
// Main VMCALL Handler
// =============================================================================

VMEXIT_ACTION HandleVmcall(GUEST_REGS* regs) {
    U64 entryTsc = TimingStart();
    VMX_CPU* cpu = VmxGetCurrentCpu();
    U64 magic = regs->Rax;
    U64 command = regs->Rcx;
    I64 status;

    // Security: Only allow VMCALLs from ring 0 (kernel mode)
    // CPL is stored in bits 1:0 of CS selector (RPL field = CPL for CS)
    U64 guestCs = VmcsRead(VMCS_GUEST_CS_SEL);
    U8 cpl = (U8)(guestCs & 0x3);
    if (cpl != 0) {
        regs->Rax = (U64)VMCALL_STATUS_ACCESS_DENIED;
        if (cpu) TimingEnd(cpu, entryTsc, TIMING_VMCALL_OVERHEAD);
        return VMEXIT_ADVANCE_RIP;
    }

    // Check obfuscated magic signature (deobfuscate at runtime)
    U64 expected_magic = ombra_deobf_magic(OMBRA_VMCALL_MAGIC_OBF);
    if (magic != expected_magic) {
        // Not our VMCALL - could be Hyper-V or other hypervisor
        // Pass through or inject #UD

        // For stealth, pretend VMCALL doesn't exist
        // In reality, we should check if Hyper-V is expected
        regs->Rax = (U64)VMCALL_STATUS_INVALID_MAGIC;
        if (cpu) TimingEnd(cpu, entryTsc, TIMING_VMCALL_OVERHEAD);
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
        if (cpu) TimingEnd(cpu, entryTsc, TIMING_VMCALL_OVERHEAD);
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

    case VMCALL_READ_VIRT:
        status = VmcallReadVirt(regs);
        break;

    case VMCALL_WRITE_VIRT:
        status = VmcallWriteVirt(regs);
        break;

    case VMCALL_GET_PROCESS_CR3:
        status = VmcallGetProcessCr3(regs);
        break;

    case VMCALL_NESTED_GET_INFO:
        status = VmcallNestedGetInfo(regs);
        break;

    case VMCALL_NESTED_ENABLE:
        status = VmcallNestedEnable(regs);
        break;

    case VMCALL_NESTED_DISABLE:
        status = VmcallNestedDisable(regs);
        break;

    case VMCALL_NMI_BLOCK_CR3:
        status = VmcallNmiBlockCr3(regs);
        break;

    case VMCALL_NMI_UNBLOCK_CR3:
        status = VmcallNmiUnblockCr3(regs);
        break;

    case VMCALL_NMI_ENABLE:
        status = VmcallNmiEnable(regs);
        break;

    case VMCALL_NMI_DISABLE:
        status = VmcallNmiDisable(regs);
        break;

    case VMCALL_SPOOF_SMBIOS:
        status = VmcallSpoofSmbios(regs);
        break;

    case VMCALL_SPOOF_INIT:
        status = VmcallSpoofInit(regs);
        break;

    case VMCALL_SPOOF_ADD_DISK:
        status = VmcallSpoofAddDisk(regs);
        break;

    case VMCALL_SPOOF_DEL_DISK:
        status = VmcallSpoofDelDisk(regs);
        break;

    case VMCALL_SPOOF_ADD_NIC:
        status = VmcallSpoofAddNic(regs);
        break;

    case VMCALL_SPOOF_DEL_NIC:
        status = VmcallSpoofDelNic(regs);
        break;

    case VMCALL_SPOOF_QUERY:
        status = VmcallSpoofQuery(regs);
        break;

    // Phase 3 Driver VMCALLs
    case VMCALL_WATCH_CR3:
        status = VmcallWatchCr3(regs);
        break;

    case VMCALL_UNWATCH_CR3:
        status = VmcallUnwatchCr3(regs);
        break;

    case VMCALL_PIN_PAGE:
        status = VmcallPinPage(regs);
        break;

    case VMCALL_UNPIN_PAGE:
        status = VmcallUnpinPage(regs);
        break;

    case VMCALL_ALLOC_PHYSICAL_PAGE:
        status = VmcallAllocPhysicalPage(regs);
        break;

    case VMCALL_FREE_PHYSICAL_PAGE:
        status = VmcallFreePhysicalPage(regs);
        break;

    case VMCALL_COPY_PHYSICAL_PAGE:
        status = VmcallCopyPhysicalPage(regs);
        break;

    case VMCALL_SPLIT_EPT_PAGE:
        status = VmcallSplitEptPage(regs);
        break;

    case VMCALL_UNSPLIT_EPT_PAGE:
        status = VmcallUnsplitEptPage(regs);
        break;

    case VMCALL_DRIVER_READY:
        status = VmcallDriverReady(regs);
        break;

    case VMCALL_DRIVER_SHUTDOWN:
        status = VmcallDriverShutdown(regs);
        break;

    case VMCALL_HIDE_MEMORY:
        status = VmcallHideMemory(regs);
        break;

    case VMCALL_UNHIDE_MEMORY:
        status = VmcallUnhideMemory(regs);
        break;

    default:
        status = VMCALL_STATUS_INVALID_COMMAND;
        break;
    }

    regs->Rax = (U64)status;
    if (cpu) TimingEnd(cpu, entryTsc, TIMING_VMCALL_OVERHEAD);
    return VMEXIT_ADVANCE_RIP;
}
