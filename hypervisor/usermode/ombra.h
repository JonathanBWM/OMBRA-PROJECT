// ombra.h — OmbraHypervisor Usermode API
// Cross-process memory access via Ring -1 hypervisor
//
// Usage:
//   #include "ombra.h"
//
//   OmbraInit();
//   float health = OmbraRead<float>(pid, healthAddr);
//   OmbraWrite<int>(pid, ammoAddr, 999);
//   OmbraShutdown();

#ifndef OMBRA_H
#define OMBRA_H

#include <Windows.h>
#include <stdint.h>
#include <stdbool.h>
#include <TlHelp32.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// VMCALL Protocol (must match hypervisor/shared/types.h)
// =============================================================================
//
// SYNC WARNING: VMCALL codes below are duplicated from shared/types.h
// This is intentional - ombra.h is a standalone usermode header.
// If you modify VMCALL codes, update BOTH files!
// Canonical source: hypervisor/shared/types.h
//

// Obfuscated magic value - deobfuscates to "OMBRALL\0" at runtime
#define OMBRA_OBF_KEY           0x9C7A3B5E2D1F4E8AULL
#define OMBRA_VMCALL_MAGIC_OBF  0xD337790C6C53028AULL

static inline uint64_t ombra_get_magic(void) {
    return OMBRA_VMCALL_MAGIC_OBF ^ OMBRA_OBF_KEY;
}

// Command codes (non-sequential for stealth)
#define VMCALL_PING             0x7C9E2A8FULL
#define VMCALL_UNLOAD           0x3F7B8D21ULL
#define VMCALL_GET_STATUS       0x92C4E6F3ULL
#define VMCALL_READ_PHYS        0xE2A849D7ULL
#define VMCALL_WRITE_PHYS       0x6C3DF52BULL
#define VMCALL_VIRT_TO_PHYS     0xA971D8F4ULL
#define VMCALL_READ_VIRT        0xD4F83A19ULL
#define VMCALL_WRITE_VIRT       0x8B2E57C6ULL
#define VMCALL_GET_PROCESS_CR3  0x2F6E9A7CULL

// Hardware spoofing commands
#define VMCALL_SPOOF_SMBIOS     0xB4E21C87ULL
#define VMCALL_SPOOF_INIT       0xC7A3E518ULL
#define VMCALL_SPOOF_ADD_DISK   0x39F8B2D1ULL
#define VMCALL_SPOOF_DEL_DISK   0xE5D4A726ULL
#define VMCALL_SPOOF_ADD_NIC    0x8B61F4C9ULL
#define VMCALL_SPOOF_DEL_NIC    0x4E27D8A3ULL
#define VMCALL_SPOOF_QUERY      0xF1829C5BULL

// Spoofing constants
#define MAX_SPOOFED_DISKS       8
#define MAX_SPOOFED_NICS        4
#define SERIAL_MAX_LEN          64
#define MAC_ADDR_LEN            6

// Status codes
#define OMBRA_STATUS_SUCCESS           0
#define OMBRA_STATUS_INVALID_MAGIC    -1
#define OMBRA_STATUS_INVALID_COMMAND  -2
#define OMBRA_STATUS_INVALID_PARAM    -3
#define OMBRA_STATUS_NOT_IMPLEMENTED  -4
#define OMBRA_STATUS_ACCESS_DENIED    -5

// =============================================================================
// Low-Level VMCALL Interface
// =============================================================================

// Execute VMCALL with full register control
// Returns: RAX (status), outputs via pointers
static inline int64_t OmbraVmcall(
    uint64_t command,
    uint64_t rdx_in,
    uint64_t r8_in,
    uint64_t r9_in,
    uint64_t r10_in,
    uint64_t* rdx_out,
    uint64_t* r8_out
) {
    uint64_t magic = ombra_get_magic();
    int64_t status;
    uint64_t rdx_ret, r8_ret;

    // VMCALL: RAX=magic, RCX=cmd, RDX=p1, R8=p2, R9=p3, R10=p4
    __asm__ __volatile__ (
        "movq %[magic], %%rax\n\t"
        "movq %[cmd], %%rcx\n\t"
        "movq %[rdx], %%rdx\n\t"
        "movq %[r8], %%r8\n\t"
        "movq %[r9], %%r9\n\t"
        "movq %[r10], %%r10\n\t"
        "vmcall\n\t"
        "movq %%rax, %[status]\n\t"
        "movq %%rdx, %[rdx_ret]\n\t"
        "movq %%r8, %[r8_ret]\n\t"
        : [status] "=r" (status),
          [rdx_ret] "=r" (rdx_ret),
          [r8_ret] "=r" (r8_ret)
        : [magic] "r" (magic),
          [cmd] "r" (command),
          [rdx] "r" (rdx_in),
          [r8] "r" (r8_in),
          [r9] "r" (r9_in),
          [r10] "r" (r10_in)
        : "rax", "rcx", "rdx", "r8", "r9", "r10", "memory"
    );

    if (rdx_out) *rdx_out = rdx_ret;
    if (r8_out) *r8_out = r8_ret;
    return status;
}

// =============================================================================
// CR3 Lookup via Hypervisor EPROCESS Walking
// =============================================================================

// Cache for CR3 lookups (simple, not thread-safe)
typedef struct {
    DWORD    pid;
    uint64_t cr3;
    DWORD    tick;  // For cache invalidation
} OMBRA_CR3_CACHE;

static OMBRA_CR3_CACHE g_Cr3Cache[16] = {0};
static int g_Cr3CacheIdx = 0;
static uint64_t g_PsInitialSystemProcess = 0;

// Get PsInitialSystemProcess address via NtQuerySystemInformation
// This is a one-time lookup cached for the session
static inline uint64_t OmbraGetSystemProcess(void) {
    if (g_PsInitialSystemProcess != 0) {
        return g_PsInitialSystemProcess;
    }

    // Method 1: Get from ntoskrnl.exe exports
    // PsInitialSystemProcess is exported, we can get it via driver
    // For now, use a hardcoded approach that works for testing

    // Method 2: Use NtQuerySystemInformation to get System process EPROCESS
    // SystemProcessInformation (class 5) gives us process info including handles
    // But doesn't directly give EPROCESS...

    // Method 3: Read from KPCR/KPRCB (requires ring0)
    // KPRCB.CurrentThread -> ETHREAD.Process -> EPROCESS
    // Then walk to System (PID 4)

    // For usermode-only approach, we need the driver to tell us
    // The loader should cache this during hypervisor init

    // Placeholder: This would be set by the loader during init
    // The loader has kernel access and can resolve PsInitialSystemProcess
    return 0;
}

// Set PsInitialSystemProcess (called by loader after resolving symbol)
static inline void OmbraSetSystemProcess(uint64_t addr) {
    g_PsInitialSystemProcess = addr;
}

// Get DirectoryTableBase (CR3) for a process by PID
// Uses hypervisor to walk EPROCESS list
static inline uint64_t OmbraGetProcessCr3(DWORD pid) {
    // Check cache first (5 second TTL)
    DWORD now = GetTickCount();
    for (int i = 0; i < 16; i++) {
        if (g_Cr3Cache[i].pid == pid && (now - g_Cr3Cache[i].tick) < 5000) {
            return g_Cr3Cache[i].cr3;
        }
    }

    // Need PsInitialSystemProcess to walk the list
    uint64_t systemProcess = OmbraGetSystemProcess();
    if (systemProcess == 0) {
        // Fallback: use current process CR3 (only works for self-reads)
        return 0;
    }

    // Ask hypervisor to walk EPROCESS list and find target PID
    uint64_t cr3 = 0;
    int64_t status = OmbraVmcall(
        VMCALL_GET_PROCESS_CR3,
        (uint64_t)pid,       // RDX = target PID
        systemProcess,       // R8  = PsInitialSystemProcess address
        0,                   // R9  = unused
        0,                   // R10 = unused
        &cr3,                // Output: CR3
        NULL
    );

    if (status != OMBRA_STATUS_SUCCESS || cr3 == 0) {
        return 0;  // Process not found
    }

    // Cache the result
    int idx = g_Cr3CacheIdx;
    g_Cr3Cache[idx].pid = pid;
    g_Cr3Cache[idx].cr3 = cr3;
    g_Cr3Cache[idx].tick = now;
    g_Cr3CacheIdx = (idx + 1) % 16;

    return cr3;
}

// Invalidate CR3 cache (call when process exits or you need fresh data)
static inline void OmbraInvalidateCr3Cache(void) {
    for (int i = 0; i < 16; i++) {
        g_Cr3Cache[i].pid = 0;
        g_Cr3Cache[i].cr3 = 0;
        g_Cr3Cache[i].tick = 0;
    }
}

// =============================================================================
// Virtual Memory Operations
// =============================================================================

// Read from virtual address with target CR3
// cr3 = 0 means current process
static inline bool OmbraReadVirt(uint64_t cr3, uint64_t va, void* out, size_t size) {
    if (size > 8) return false;  // Single operation limit

    uint64_t value = 0;
    int64_t status = OmbraVmcall(
        VMCALL_READ_VIRT,
        va,             // RDX = virtual address
        cr3,            // R8  = target CR3
        size,           // R9  = size
        0,              // R10 = unused
        &value,         // Output in RDX
        NULL
    );

    if (status != OMBRA_STATUS_SUCCESS) {
        return false;
    }

    memcpy(out, &value, size);
    return true;
}

// Write to virtual address with target CR3
static inline bool OmbraWriteVirt(uint64_t cr3, uint64_t va, const void* in, size_t size) {
    if (size > 8) return false;

    uint64_t value = 0;
    memcpy(&value, in, size);

    int64_t status = OmbraVmcall(
        VMCALL_WRITE_VIRT,
        va,             // RDX = virtual address
        cr3,            // R8  = target CR3
        size,           // R9  = size
        value,          // R10 = value to write
        NULL,
        NULL
    );

    return status == OMBRA_STATUS_SUCCESS;
}

// =============================================================================
// Buffer Operations (loops over single-value operations)
// =============================================================================

static inline bool OmbraReadBuffer(uint64_t cr3, uint64_t va, void* buffer, size_t size) {
    uint8_t* dst = (uint8_t*)buffer;
    size_t remaining = size;
    uint64_t addr = va;

    while (remaining > 0) {
        size_t chunk = remaining >= 8 ? 8 :
                       remaining >= 4 ? 4 :
                       remaining >= 2 ? 2 : 1;

        if (!OmbraReadVirt(cr3, addr, dst, chunk)) {
            return false;
        }

        dst += chunk;
        addr += chunk;
        remaining -= chunk;
    }

    return true;
}

static inline bool OmbraWriteBuffer(uint64_t cr3, uint64_t va, const void* buffer, size_t size) {
    const uint8_t* src = (const uint8_t*)buffer;
    size_t remaining = size;
    uint64_t addr = va;

    while (remaining > 0) {
        size_t chunk = remaining >= 8 ? 8 :
                       remaining >= 4 ? 4 :
                       remaining >= 2 ? 2 : 1;

        if (!OmbraWriteVirt(cr3, addr, src, chunk)) {
            return false;
        }

        src += chunk;
        addr += chunk;
        remaining -= chunk;
    }

    return true;
}

// =============================================================================
// High-Level API (PID-based)
// =============================================================================

static inline bool OmbraReadMemory(DWORD pid, void* address, void* buffer, size_t size) {
    uint64_t cr3 = OmbraGetProcessCr3(pid);
    return OmbraReadBuffer(cr3, (uint64_t)address, buffer, size);
}

static inline bool OmbraWriteMemory(DWORD pid, void* address, const void* buffer, size_t size) {
    uint64_t cr3 = OmbraGetProcessCr3(pid);
    return OmbraWriteBuffer(cr3, (uint64_t)address, buffer, size);
}

// =============================================================================
// Hypervisor Status
// =============================================================================

static inline bool OmbraIsActive(void) {
    uint64_t version = 0;
    int64_t status = OmbraVmcall(VMCALL_PING, 0, 0, 0, 0, &version, NULL);
    return status == OMBRA_STATUS_SUCCESS;
}

static inline bool OmbraGetStatus(uint32_t* cpuCount, uint64_t* exitCount) {
    uint64_t count = 0, exits = 0;
    int64_t status = OmbraVmcall(VMCALL_GET_STATUS, 0, 0, 0, 0, &count, &exits);
    if (status != OMBRA_STATUS_SUCCESS) return false;
    if (cpuCount) *cpuCount = (uint32_t)count;
    if (exitCount) *exitCount = exits;
    return true;
}

// =============================================================================
// Initialization / Shutdown
// =============================================================================

static inline bool OmbraInit(void) {
    // Verify hypervisor is active
    if (!OmbraIsActive()) {
        return false;
    }
    return true;
}

// Full initialization with driver context (for cross-process support)
// Call this from the loader after DrvInitialize()
//
// Example:
//   DRV_CONTEXT ctx;
//   DrvInitialize(&ctx, driverPath);
//   OmbraInitWithDriver(&ctx);  // Resolves PsInitialSystemProcess
//
#ifdef DRIVER_INTERFACE_H  // Only available if driver_interface.h included
static inline bool OmbraInitWithDriver(DRV_CONTEXT* ctx) {
    if (!OmbraIsActive()) {
        return false;
    }

    // Resolve PsInitialSystemProcess from ntoskrnl.exe
    void* psInitialSystemProcess = NULL;
    DRV_STATUS status = DrvGetSymbol(ctx, "PsInitialSystemProcess", &psInitialSystemProcess);
    if (status == DRV_SUCCESS && psInitialSystemProcess != NULL) {
        // This is a pointer to the EPROCESS pointer, we need to dereference
        // But we can't do that from usermode - pass the address and let
        // hypervisor dereference it during EPROCESS walk
        // Actually, PsInitialSystemProcess IS the EPROCESS* value, not a pointer to it
        // So we read the value at that address
        uint64_t systemEprocess = 0;
        if (OmbraReadVirt(0, (uint64_t)psInitialSystemProcess, &systemEprocess, 8)) {
            OmbraSetSystemProcess(systemEprocess);
            return true;
        }
    }

    // Fallback: still functional for current-process reads
    return true;
}
#endif

static inline void OmbraShutdown(void) {
    // Clear CR3 cache
    OmbraInvalidateCr3Cache();

    // Request hypervisor unload (optional - uncomment if desired)
    // OmbraVmcall(VMCALL_UNLOAD, 0, 0, 0, 0, NULL, NULL);
}

// =============================================================================
// Hardware Spoofing API
// =============================================================================
//
// Defeats anti-cheat hardware fingerprinting by intercepting disk serial and
// NIC MAC queries via the hypervisor. Call OmbraSpoofInit() first, then add
// spoofs for each device. The hypervisor modifies IOCTL/OID responses.

// Spoof status flags
#define OMBRA_SPOOF_INITIALIZED  0x01
#define OMBRA_SPOOF_DISK_ACTIVE  0x02
#define OMBRA_SPOOF_NIC_ACTIVE   0x04

// Initialize spoof manager
// seed: PRNG seed for auto-generated serials/MACs (0 = use default)
// Returns: true on success
static inline bool OmbraSpoofInit(uint32_t seed) {
    uint64_t diskSlots = 0, nicSlots = 0;
    int64_t status = OmbraVmcall(
        VMCALL_SPOOF_INIT,
        (uint64_t)seed,
        0, 0, 0,
        &diskSlots,
        &nicSlots
    );
    return status == OMBRA_STATUS_SUCCESS;
}

// Add disk serial spoof
// diskIndex: 0-7 (MAX_SPOOFED_DISKS - 1)
// serial: custom serial string, or NULL to auto-generate
// model: custom model string, or NULL to keep original
static inline bool OmbraSpoofAddDisk(uint32_t diskIndex, const char* serial, const char* model) {
    uint64_t slot = 0;
    int64_t status = OmbraVmcall(
        VMCALL_SPOOF_ADD_DISK,
        (uint64_t)diskIndex,
        (uint64_t)serial,
        (uint64_t)model,
        0,
        &slot,
        NULL
    );
    return status == OMBRA_STATUS_SUCCESS;
}

// Remove disk serial spoof
static inline bool OmbraSpoofRemoveDisk(uint32_t diskIndex) {
    int64_t status = OmbraVmcall(
        VMCALL_SPOOF_DEL_DISK,
        (uint64_t)diskIndex,
        0, 0, 0,
        NULL, NULL
    );
    return status == OMBRA_STATUS_SUCCESS;
}

// Add NIC MAC spoof
// nicIndex: 0-3 (MAX_SPOOFED_NICS - 1)
// mac: 6-byte MAC address, or NULL to auto-generate
static inline bool OmbraSpoofAddNic(uint32_t nicIndex, const uint8_t* mac) {
    uint64_t slot = 0;
    int64_t status = OmbraVmcall(
        VMCALL_SPOOF_ADD_NIC,
        (uint64_t)nicIndex,
        (uint64_t)mac,
        0, 0,
        &slot,
        NULL
    );
    return status == OMBRA_STATUS_SUCCESS;
}

// Remove NIC MAC spoof
static inline bool OmbraSpoofRemoveNic(uint32_t nicIndex) {
    int64_t status = OmbraVmcall(
        VMCALL_SPOOF_DEL_NIC,
        (uint64_t)nicIndex,
        0, 0, 0,
        NULL, NULL
    );
    return status == OMBRA_STATUS_SUCCESS;
}

// Query spoof manager status
// flags: OMBRA_SPOOF_* flags (output)
// diskCount: number of active disk spoofs (output, optional)
// nicCount: number of active NIC spoofs (output, optional)
static inline bool OmbraSpoofQuery(uint32_t* flags, uint32_t* diskCount, uint32_t* nicCount) {
    uint64_t f = 0, dc = 0, nc = 0;
    int64_t status = OmbraVmcall(
        VMCALL_SPOOF_QUERY,
        0, 0, 0, 0,
        &f,
        &dc
    );
    // Note: nicCount comes in R9, which we don't capture with current OmbraVmcall
    // For now, query diskCount only. Could extend OmbraVmcall to capture R9 if needed.

    if (status != OMBRA_STATUS_SUCCESS) return false;
    if (flags) *flags = (uint32_t)f;
    if (diskCount) *diskCount = (uint32_t)dc;
    if (nicCount) *nicCount = 0;  // Would need extended vmcall to get this
    return true;
}

// Convenience: spoof all disks with auto-generated serials
static inline bool OmbraSpoofAllDisks(void) {
    for (uint32_t i = 0; i < MAX_SPOOFED_DISKS; i++) {
        if (!OmbraSpoofAddDisk(i, NULL, NULL)) {
            return false;
        }
    }
    return true;
}

// Convenience: spoof all NICs with auto-generated MACs
static inline bool OmbraSpoofAllNics(void) {
    for (uint32_t i = 0; i < MAX_SPOOFED_NICS; i++) {
        if (!OmbraSpoofAddNic(i, NULL)) {
            return false;
        }
    }
    return true;
}

#ifdef __cplusplus
}
#endif

#endif // OMBRA_H
