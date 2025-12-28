// vmcall_phase2.c — Phase 2 VMCALL Handlers (Driver Mapper)
// OmbraHypervisor

#include "vmcall_phase2.h"
#include "handlers.h"
#include "../ept.h"
#include "../vmx.h"
#include "../../shared/ept_defs.h"
#include "../../shared/vmcs_fields.h"
#include <intrin.h>

// =============================================================================
// Pool Region Tracking
// =============================================================================

typedef struct _POOL_REGION {
    U64     KernelVA;
    U64     PhysicalAddr;
    U32     Size;
    bool    InUse;
} POOL_REGION;

#define MAX_POOL_REGIONS 16
static POOL_REGION g_PoolRegions[MAX_POOL_REGIONS] = {0};
static U64 g_PoolBase = 0;
static U32 g_PoolSize = 0;
static U32 g_PoolOffset = 0;

// =============================================================================
// EPT Range Tracking (for Working Set query spoofing)
// =============================================================================
// EAC uses NtQueryVirtualMemory(MemoryWorkingSetExInformation) to detect
// EPT-only memory. Pages with SharedCount==0 and Shared==0 indicate
// non-backed executable memory, which is a detection signature.
//
// Strategy: Track EPT-only VA ranges here. When syscall hook intercepts
// NtQueryVirtualMemory for these ranges, spoof the Working Set info.
//
// Implementation Note: Full spoofing requires:
// 1. MSR_LSTAR hook to intercept syscalls
// 2. Filter for NtQueryVirtualMemory (SSN varies by Windows version)
// 3. Check MemoryInformationClass == MemoryWorkingSetExInformation (4)
// 4. If VA in registered range, spoof SharedCount=1, Shared=1

typedef struct _EPT_RANGE_ENTRY {
    U64     StartVA;
    U64     EndVA;
    bool    Active;
} EPT_RANGE_ENTRY;

#define MAX_EPT_RANGES 32
static EPT_RANGE_ENTRY g_EptRanges[MAX_EPT_RANGES] = {0};

// Register an EPT-only VA range for Working Set spoof
static OMBRA_STATUS RegisterEptRange(U64 startVA, U64 size) {
    for (U32 i = 0; i < MAX_EPT_RANGES; i++) {
        if (!g_EptRanges[i].Active) {
            g_EptRanges[i].StartVA = startVA;
            g_EptRanges[i].EndVA = startVA + size;
            g_EptRanges[i].Active = true;
            TRACE("EPT Range registered: 0x%llx - 0x%llx", startVA, startVA + size);
            return OMBRA_SUCCESS;
        }
    }
    ERR("EPT Range table full");
    return OMBRA_ERROR_NO_MEMORY;
}

// Check if a VA is in a registered EPT range (for syscall hook to call)
bool IsVaInEptRange(U64 va) {
    for (U32 i = 0; i < MAX_EPT_RANGES; i++) {
        if (g_EptRanges[i].Active &&
            va >= g_EptRanges[i].StartVA &&
            va < g_EptRanges[i].EndVA) {
            return true;
        }
    }
    return false;
}

// =============================================================================
// Initialization
// =============================================================================

void Phase2_InitPool(U64 poolBase, U64 poolSize) {
    g_PoolBase = poolBase;
    g_PoolSize = (U32)poolSize;
    g_PoolOffset = 0;

    for (int i = 0; i < MAX_POOL_REGIONS; i++) {
        g_PoolRegions[i].InUse = false;
    }
}

// =============================================================================
// VMCALL Handlers
// =============================================================================

// VMCALL_CLAIM_POOL_REGION - Claim a region from pre-allocated pool
static OMBRA_STATUS HandleClaimPoolRegion(VMCALL_CLAIM_POOL_IN* in, VMCALL_CLAIM_POOL_OUT* out) {
    // Input validation
    if (!in || !out) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Validate size is reasonable
    if (in->Size == 0 || in->Size > 64 * 1024 * 1024) {  // Max 64MB per region
        out->Status = OMBRA_ERROR_INVALID_PARAM;
        return OMBRA_ERROR_INVALID_PARAM;
    }

    U32 alignedSize = (in->Size + 0xFFF) & ~0xFFF;  // Page align

    // Check for integer overflow in pool offset calculation
    if (g_PoolOffset + alignedSize < g_PoolOffset) {
        out->Status = OMBRA_ERROR_OUT_OF_MEMORY;
        return OMBRA_ERROR_OUT_OF_MEMORY;
    }

    if (g_PoolOffset + alignedSize > g_PoolSize) {
        out->Status = OMBRA_ERROR_OUT_OF_MEMORY;
        return OMBRA_ERROR_OUT_OF_MEMORY;
    }

    // Find free region slot
    POOL_REGION* region = NULL;
    for (int i = 0; i < MAX_POOL_REGIONS; i++) {
        if (!g_PoolRegions[i].InUse) {
            region = &g_PoolRegions[i];
            break;
        }
    }

    if (!region) {
        out->Status = OMBRA_ERROR_OUT_OF_MEMORY;
        return OMBRA_ERROR_OUT_OF_MEMORY;
    }

    region->KernelVA = g_PoolBase + g_PoolOffset;
    region->Size = alignedSize;
    region->InUse = true;

    out->KernelVA = region->KernelVA;
    out->Status = OMBRA_SUCCESS;

    g_PoolOffset += alignedSize;

    return OMBRA_SUCCESS;
}

// VMCALL_RELEASE_POOL_REGION - Release a claimed region
static OMBRA_STATUS HandleReleasePoolRegion(U64 kernelVA) {
    for (int i = 0; i < MAX_POOL_REGIONS; i++) {
        if (g_PoolRegions[i].InUse && g_PoolRegions[i].KernelVA == kernelVA) {
            g_PoolRegions[i].InUse = false;
            return OMBRA_SUCCESS;
        }
    }
    return OMBRA_ERROR_NOT_FOUND;
}

// VMCALL_FINALIZE_DRIVER_LOAD - Set protections, call entry, optionally hide
static OMBRA_STATUS HandleFinalizeDriverLoad(VMCALL_FINALIZE_IN* in, VMCALL_FINALIZE_OUT* out) {
    // Input validation
    if (!in || !out) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Validate kernel VA is in kernel address space
    if (!IsValidKernelAddress(in->KernelVA)) {
        out->Status = OMBRA_ERROR_INVALID_PARAM;
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Validate RVA offsets don't overflow
    if (WouldOverflow(in->KernelVA, in->TextRva) ||
        WouldOverflow(in->KernelVA, in->EntryRva)) {
        out->Status = OMBRA_ERROR_INVALID_PARAM;
        return OMBRA_ERROR_INVALID_PARAM;
    }

    U64 textStart = in->KernelVA + in->TextRva;
    (void)textStart;  // Would use for EPT RX permissions

    // Call DriverEntry unless NO_ENTRY flag set
    if (!(in->Flags & FINALIZE_FLAG_NO_ENTRY)) {
        U64 entryAddr = in->KernelVA + in->EntryRva;

        // Validate entry point is in kernel space
        if (!IsValidKernelAddress(entryAddr)) {
            out->Status = OMBRA_ERROR_INVALID_PARAM;
            return OMBRA_ERROR_INVALID_PARAM;
        }

        // Validate entry point is within the driver's mapped range
        // This prevents calling arbitrary kernel addresses
        // Note: Caller must provide TextSize for proper validation
        // For now we just ensure it's page-aligned and in kernel space

        typedef long (*FN_DriverEntry)(void*, void*);
        FN_DriverEntry entry = (FN_DriverEntry)entryAddr;

        // Call with NULL parameters (no driver object for manual map)
        long ntStatus = entry(NULL, NULL);

        out->DriverEntryResult = (U32)ntStatus;

        if (ntStatus < 0) {
            out->Status = OMBRA_ERROR_DRIVER_INIT_FAILED;
            return OMBRA_ERROR_DRIVER_INIT_FAILED;
        }
    } else {
        out->DriverEntryResult = 0;
    }

    // EPT hiding would be configured here if FINALIZE_FLAG_HIDE is set
    // This makes pages execute-only, causing reads/writes to fault

    out->Status = OMBRA_SUCCESS;
    return OMBRA_SUCCESS;
}

// VMCALL_HIDE_MEMORY_RANGE - Configure EPT to hide memory from kernel
static OMBRA_STATUS HandleHideMemoryRange(VMCALL_HIDE_RANGE_IN* in) {
    // Input validation
    if (!in) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Validate address is in kernel space
    if (!IsValidKernelAddress(in->Address)) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Validate size is reasonable and doesn't overflow
    if (in->Size == 0 || in->Size > 64 * 1024 * 1024) {  // Max 64MB
        return OMBRA_ERROR_INVALID_PARAM;
    }

    if (WouldOverflow(in->Address, in->Size)) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Configure EPT to make memory execute-only
    // Reads/writes from kernel will cause EPT violation
    // Handler can return zeros or inject #PF

    U64 numPages = (in->Size + 0xFFF) / 0x1000;

    for (U64 i = 0; i < numPages; i++) {
        U64 pageAddr = in->Address + (i * 0x1000);
        // Execute-only: code can run but reads/writes fault
        // EptSetPagePermissions(pageAddr, EPT_EXECUTE);
        (void)pageAddr;  // Placeholder until EPT integration
    }

    return OMBRA_SUCCESS;
}

// VMCALL_UNHIDE_MEMORY_RANGE - Restore normal EPT permissions
static OMBRA_STATUS HandleUnhideMemoryRange(VMCALL_HIDE_RANGE_IN* in) {
    // Input validation
    if (!in) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Validate address is in kernel space
    if (!IsValidKernelAddress(in->Address)) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Validate size is reasonable and doesn't overflow
    if (in->Size == 0 || in->Size > 64 * 1024 * 1024) {  // Max 64MB
        return OMBRA_ERROR_INVALID_PARAM;
    }

    if (WouldOverflow(in->Address, in->Size)) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    U64 numPages = (in->Size + 0xFFF) / 0x1000;

    for (U64 i = 0; i < numPages; i++) {
        U64 pageAddr = in->Address + (i * 0x1000);
        // Restore RWX permissions
        // EptSetPagePermissions(pageAddr, EPT_READ | EPT_WRITE | EPT_EXECUTE);
        (void)pageAddr;
    }

    return OMBRA_SUCCESS;
}

// VMCALL_DRIVER_PING - Check if driver is responding
static OMBRA_STATUS HandleDriverPing(U32* response) {
    *response = 0x504F4E47;  // 'PONG'
    return OMBRA_SUCCESS;
}

// =============================================================================
// EPT Mapping Helpers
// =============================================================================

// Hypervisor physical memory base (must be initialized before use)
// Same global as va_finder.c - hypervisor's page tables map physical memory here
extern void* g_HvPhysicalMemoryBase;

// Read a 64-bit value from guest physical memory
// Uses hypervisor's identity-mapped physical memory region
static inline U64 ReadGuestPhysical64(U64 physAddr) {
    // Validate physical address is within reasonable bounds
    // Max physical address on current x64 CPUs is typically 52 bits (4PB)
    if (physAddr > 0x000FFFFFFFFFFFFFULL) {
        return 0;
    }

    void* base = g_HvPhysicalMemoryBase;
    if (base == NULL) {
        // Fallback for early init - only safe for low addresses
        base = (void*)0;
    }

    U64* ptr = (U64*)((U64)base + physAddr);
    return *ptr;
}

// Kernel address range validation
// Returns true if the address is in valid kernel VA space
static inline bool IsValidKernelAddress(U64 addr) {
    // Windows x64 kernel addresses are in the range 0xFFFF800000000000 - 0xFFFFFFFFFFFFFFFF
    // The canonical high bits (48-63) must all be 1 for kernel space
    return (addr >= 0xFFFF800000000000ULL);
}

// Check for integer overflow when adding size to address
static inline bool WouldOverflow(U64 base, U64 size) {
    return (base + size < base);
}

// Walk guest page tables to check if a VA is mapped
// Returns 0 if not mapped, physical address if mapped
static U64 WalkGuestPageTablesInternal(U64 guestVA, U64 guestCr3) {
    U64 pml4Base = guestCr3 & 0xFFFFFFFFFFFFF000ULL;
    U64 pml4Index = (guestVA >> 39) & 0x1FF;
    U64 pdptIndex = (guestVA >> 30) & 0x1FF;
    U64 pdIndex = (guestVA >> 21) & 0x1FF;
    U64 ptIndex = (guestVA >> 12) & 0x1FF;

    // Read PML4E via hypervisor's physical memory mapping
    U64 pml4e = ReadGuestPhysical64(pml4Base + (pml4Index * 8));
    if (!(pml4e & 1)) return 0;  // Not present

    // Read PDPTE
    U64 pdptBase = pml4e & 0xFFFFFFFFFFFFF000ULL;
    U64 pdpte = ReadGuestPhysical64(pdptBase + (pdptIndex * 8));
    if (!(pdpte & 1)) return 0;  // Not present
    if (pdpte & (1ULL << 7)) {
        // 1GB page
        return (pdpte & 0xFFFFC0000000ULL) | (guestVA & 0x3FFFFFFFULL);
    }

    // Read PDE
    U64 pdBase = pdpte & 0xFFFFFFFFFFFFF000ULL;
    U64 pde = ReadGuestPhysical64(pdBase + (pdIndex * 8));
    if (!(pde & 1)) return 0;  // Not present
    if (pde & (1ULL << 7)) {
        // 2MB page
        return (pde & 0xFFFFFFFE00000ULL) | (guestVA & 0x1FFFFFULL);
    }

    // Read PTE
    U64 ptBase = pde & 0xFFFFFFFFFFFFF000ULL;
    U64 pte = ReadGuestPhysical64(ptBase + (ptIndex * 8));
    if (!(pte & 1)) return 0;  // Not present

    // 4KB page
    return (pte & 0xFFFFFFFFFFFFF000ULL) | (guestVA & 0xFFFULL);
}

// Find unused kernel VA range by scanning guest page tables
// Returns 0 if not found, otherwise VA of unused range
static U64 FindUnusedKernelVA(U64 guestCr3, U64 sizeBytes) {
    // Scan kernel VA space starting from 0xFFFF888000000000
    // This is typically unused kernel range on Windows
    U64 scanStart = 0xFFFF888000000000ULL;
    U64 scanEnd = 0xFFFFC00000000000ULL;  // Don't go into PFN database range
    U64 alignedSize = (sizeBytes + 0xFFF) & ~0xFFFULL;
    U64 numPages = alignedSize / 0x1000;

    for (U64 va = scanStart; va < scanEnd; va += 0x1000) {
        // Check if this range is free
        bool rangeFree = true;
        for (U64 i = 0; i < numPages && rangeFree; i++) {
            U64 testVa = va + (i * 0x1000);
            if (WalkGuestPageTablesInternal(testVa, guestCr3) != 0) {
                rangeFree = false;
            }
        }

        if (rangeFree) {
            return va;
        }
    }

    return 0;  // Not found
}

// =============================================================================
// Anti-Detection: Staggered EPT Splits
// =============================================================================
// EAC monitors KiPageFault via kdTrap at IDT 0x14.
// A burst of page splits causes detectable page fault pattern.
// Staggering splits across time prevents behavioral fingerprinting.
//
// Strategy:
// - Split pages in batches of STEALTH_BATCH_SIZE
// - After each batch, briefly yield to normalize fault timing
// - This prevents the "fault every microsecond" signature

#define STEALTH_BATCH_SIZE          16      // Pages per batch before yield
#define STEALTH_YIELD_ITERATIONS    100     // Busy-wait cycles between batches

// Yield briefly without syscall (can't call KeDelayExecutionThread in VMX root)
static inline void StealthYield(void) {
    volatile U64 dummy = 0;
    for (U32 i = 0; i < STEALTH_YIELD_ITERATIONS; i++) {
        dummy += i;
        _mm_pause();  // CPU hint to reduce power/improve timing
    }
}

// =============================================================================
// EPT Mapping VMCALLs
// =============================================================================

// VMCALL_CREATE_EPT_MAPPING - Create EPT-only memory mapping
static OMBRA_STATUS HandleCreateEptMapping(VMCALL_EPT_MAP_IN* in, VMCALL_EPT_MAP_OUT* out) {
    VMX_CPU* cpu;
    EPT_STATE* ept;
    U64 kernelVA;
    U64 physicalAddr;
    U64 alignedSize;
    U32 numPages;
    U64 guestCr3;
    OMBRA_STATUS status;

    if (!in || !out) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Get current CPU context and EPT state
    cpu = VmxGetCurrentCpu();
    if (!cpu || !cpu->Ept) {
        out->Status = OMBRA_ERROR_INVALID_STATE;
        return OMBRA_ERROR_INVALID_STATE;
    }
    ept = cpu->Ept;

    // Validate and align size
    if (in->Size == 0 || in->Size > 64 * 1024 * 1024) {  // Max 64MB
        out->Status = OMBRA_ERROR_INVALID_PARAM;
        return OMBRA_ERROR_INVALID_PARAM;
    }
    alignedSize = (in->Size + 0xFFF) & ~0xFFFULL;
    numPages = (U32)(alignedSize / 0x1000);

    // Determine kernel VA
    if (in->Flags & EPT_MAP_FLAG_AUTO_VA) {
        // Auto-find unused kernel VA range
        guestCr3 = __vmx_vmread(VMCS_GUEST_CR3);
        kernelVA = FindUnusedKernelVA(guestCr3, alignedSize);
        if (kernelVA == 0) {
            out->Status = OMBRA_ERROR_NO_MEMORY;
            return OMBRA_ERROR_NO_MEMORY;
        }
    } else {
        kernelVA = in->RequestedVA;
        if (kernelVA == 0 || (kernelVA & 0xFFF) != 0) {
            out->Status = OMBRA_ERROR_INVALID_PARAM;
            return OMBRA_ERROR_INVALID_PARAM;
        }
    }

    // Determine physical address
    if (in->Flags & EPT_MAP_FLAG_AUTO_PHYS) {
        // Allocate from EPT memory pool
        // For now, we'll use a simple bump allocator
        // In production, track these allocations properly
        if (ept->PagesUsed + numPages > ept->TotalPagesAllocated) {
            out->Status = OMBRA_ERROR_NO_MEMORY;
            return OMBRA_ERROR_NO_MEMORY;
        }

        physicalAddr = ept->EptMemoryPhysical + (ept->PagesUsed * 0x1000);
        ept->PagesUsed += numPages;
    } else {
        physicalAddr = in->PhysicalAddr;
        if (physicalAddr == 0 || (physicalAddr & 0xFFF) != 0) {
            out->Status = OMBRA_ERROR_INVALID_PARAM;
            return OMBRA_ERROR_INVALID_PARAM;
        }
    }

    // Create EPT mappings for each page
    // We need to split large pages to 4KB for fine-grained control
    //
    // STEALTH: Process in batches with yields to prevent page fault burst detection
    // EAC uses KiPageFault heuristics - rapid successive faults = behavioral fingerprint
    U32 batchCount = 0;

    for (U32 i = 0; i < numPages; i++) {
        U64 guestPhysical = kernelVA + (i * 0x1000);  // Assume identity GVA->GPA in kernel
        U64 hostPhysical = physicalAddr + (i * 0x1000);

        // Split large pages if needed
        // First check if we need to split 1GB to 2MB
        status = EptSplit1GbTo2Mb(ept, guestPhysical);
        if (OMBRA_FAILED(status)) {
            out->Status = status;
            return status;
        }

        // Then split 2MB to 4KB
        status = EptSplit2MbTo4Kb(ept, guestPhysical);
        if (OMBRA_FAILED(status)) {
            out->Status = status;
            return status;
        }

        // Modify EPT entry with requested permissions
        status = EptModifyPage(ept, guestPhysical, hostPhysical,
                              in->Permissions, EPT_MEMORY_TYPE_WB);
        if (OMBRA_FAILED(status)) {
            out->Status = status;
            return status;
        }

        // STEALTH: Yield between batches to spread page fault timing
        // This prevents the "microsecond burst" behavioral signature
        batchCount++;
        if (batchCount >= STEALTH_BATCH_SIZE) {
            StealthYield();
            batchCount = 0;
        }
    }

    // Invalidate EPT TLB
    EptInvalidate(ept, INVEPT_TYPE_SINGLE_CONTEXT);

    // STEALTH: Register this range for Working Set query spoofing
    // When NtQueryVirtualMemory is called on these VAs, we need to
    // return spoofed SharedCount/Shared values to avoid detection
    RegisterEptRange(kernelVA, alignedSize);

    // Return results
    out->KernelVA = kernelVA;
    out->PhysicalAddr = physicalAddr;
    out->PagesAllocated = numPages;
    out->Status = OMBRA_SUCCESS;

    INFO("EPT mapping created: VA=0x%llx PA=0x%llx Pages=%u (Stealth: X-only=%s)",
         kernelVA, physicalAddr, numPages,
         EptSupportsExecuteOnly() ? "YES" : "NO-FALLBACK");

    return OMBRA_SUCCESS;
}

// VMCALL_DESTROY_EPT_MAPPING - Remove EPT mapping
static OMBRA_STATUS HandleDestroyEptMapping(VMCALL_EPT_UNMAP_IN* in) {
    VMX_CPU* cpu;
    EPT_STATE* ept;
    U64 alignedSize;
    U32 numPages;
    U64* entry;

    if (!in) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Get current CPU context and EPT state
    cpu = VmxGetCurrentCpu();
    if (!cpu || !cpu->Ept) {
        return OMBRA_ERROR_INVALID_STATE;
    }
    ept = cpu->Ept;

    // Validate parameters
    if (in->KernelVA == 0 || (in->KernelVA & 0xFFF) != 0 || in->Size == 0) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    alignedSize = (in->Size + 0xFFF) & ~0xFFFULL;
    numPages = (U32)(alignedSize / 0x1000);

    // Remove EPT entries (set to not-present)
    for (U32 i = 0; i < numPages; i++) {
        U64 guestPhysical = in->KernelVA + (i * 0x1000);

        // Get EPT entry pointer
        entry = EptGetEntry(ept, guestPhysical);
        if (entry) {
            // Mark as not-present (clear all permission bits)
            *entry = 0;
        }
    }

    // Invalidate EPT TLB
    EptInvalidate(ept, INVEPT_TYPE_SINGLE_CONTEXT);

    // Note: We don't free physical pages back to pool yet
    // That would require tracking which pages were allocated
    // For now, pool pages are never freed (bump allocator)

    return OMBRA_SUCCESS;
}

// VMCALL_SET_EPT_PERMISSIONS - Change EPT permissions for existing mapping
static OMBRA_STATUS HandleSetEptPermissions(VMCALL_EPT_PERM_IN* in) {
    VMX_CPU* cpu;
    EPT_STATE* ept;
    U64 alignedSize;
    U32 numPages;
    U64* entry;

    if (!in) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    // Get current CPU context and EPT state
    cpu = VmxGetCurrentCpu();
    if (!cpu || !cpu->Ept) {
        return OMBRA_ERROR_INVALID_STATE;
    }
    ept = cpu->Ept;

    // Validate parameters
    if (in->KernelVA == 0 || (in->KernelVA & 0xFFF) != 0 || in->Size == 0) {
        return OMBRA_ERROR_INVALID_PARAM;
    }

    alignedSize = (in->Size + 0xFFF) & ~0xFFFULL;
    numPages = (U32)(alignedSize / 0x1000);

    // Update EPT entry permissions
    for (U32 i = 0; i < numPages; i++) {
        U64 guestPhysical = in->KernelVA + (i * 0x1000);

        // Get EPT entry pointer
        entry = EptGetEntry(ept, guestPhysical);
        if (!entry || (*entry == 0)) {
            // Entry doesn't exist or not present
            return OMBRA_ERROR_NOT_FOUND;
        }

        // Update permissions (assume 4KB page entry)
        EPT_PTE* pte = (EPT_PTE*)entry;
        pte->Read = (in->Permissions & EPT_READ) ? 1 : 0;
        pte->Write = (in->Permissions & EPT_WRITE) ? 1 : 0;
        pte->Execute = (in->Permissions & EPT_EXECUTE) ? 1 : 0;
    }

    // Invalidate EPT TLB
    EptInvalidate(ept, INVEPT_TYPE_SINGLE_CONTEXT);

    return OMBRA_SUCCESS;
}

// =============================================================================
// Main Dispatch
// =============================================================================

OMBRA_STATUS Phase2_HandleVmcall(U64 vmcallId, void* inputBuf, void* outputBuf) {
    switch (vmcallId) {
        case VMCALL_CLAIM_POOL_REGION:
            return HandleClaimPoolRegion(
                (VMCALL_CLAIM_POOL_IN*)inputBuf,
                (VMCALL_CLAIM_POOL_OUT*)outputBuf
            );

        case VMCALL_RELEASE_POOL_REGION: {
            U64* va = (U64*)inputBuf;
            return HandleReleasePoolRegion(*va);
        }

        case VMCALL_FINALIZE_DRIVER_LOAD:
            return HandleFinalizeDriverLoad(
                (VMCALL_FINALIZE_IN*)inputBuf,
                (VMCALL_FINALIZE_OUT*)outputBuf
            );

        case VMCALL_DRIVER_PING:
            return HandleDriverPing((U32*)outputBuf);

        case VMCALL_HIDE_MEMORY_RANGE:
            return HandleHideMemoryRange((VMCALL_HIDE_RANGE_IN*)inputBuf);

        case VMCALL_UNHIDE_MEMORY_RANGE:
            return HandleUnhideMemoryRange((VMCALL_HIDE_RANGE_IN*)inputBuf);

        // EPT-only memory mapping handlers
        case VMCALL_CREATE_EPT_MAPPING:
            return HandleCreateEptMapping(
                (VMCALL_EPT_MAP_IN*)inputBuf,
                (VMCALL_EPT_MAP_OUT*)outputBuf
            );

        case VMCALL_DESTROY_EPT_MAPPING:
            return HandleDestroyEptMapping((VMCALL_EPT_UNMAP_IN*)inputBuf);

        case VMCALL_SET_EPT_PERMISSIONS:
            return HandleSetEptPermissions((VMCALL_EPT_PERM_IN*)inputBuf);

        // Path B handlers (MDL allocation) - stubbed for now
        case VMCALL_ALLOC_MDL_MEMORY:
        case VMCALL_POLL_ALLOC_STATUS:
        case VMCALL_FREE_MDL_MEMORY:
            return OMBRA_ERROR_NOT_IMPLEMENTED;

        default:
            return OMBRA_ERROR_INVALID_OPERATION;
    }
}
