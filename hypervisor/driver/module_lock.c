// module_lock.c - Module IAT Protection Implementation
// OmbraDriver Phase 3

#include "module_lock.h"
#include "context.h"
#include "vmcall.h"

// =============================================================================
// Static Storage
// =============================================================================

static LOCKED_MODULE g_LockedModules[MAX_LOCKED_MODULES] = {0};

// =============================================================================
// String Helpers (no CRT)
// =============================================================================

static bool StrEqualNoCase(const char* a, const char* b) {
    while (*a && *b) {
        char ca = *a, cb = *b;
        if (ca >= 'A' && ca <= 'Z') ca += 32;
        if (cb >= 'A' && cb <= 'Z') cb += 32;
        if (ca != cb) return false;
        a++; b++;
    }
    return *a == *b;
}

static void StrCopy(char* dst, const char* src, U32 maxLen) {
    U32 i = 0;
    while (src[i] && i < maxLen - 1) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

// =============================================================================
// Hash Function (FNV-1a)
// =============================================================================

static U64 HashMemory(const void* data, U64 size) {
    const U8* p = (const U8*)data;
    U64 hash = 0xcbf29ce484222325ULL;  // FNV offset basis

    for (U64 i = 0; i < size; i++) {
        hash ^= p[i];
        hash *= 0x100000001b3ULL;  // FNV prime
    }

    return hash;
}

// =============================================================================
// Internal Helpers
// =============================================================================

static PLOCKED_MODULE FindFreeSlot(void) {
    for (U32 i = 0; i < MAX_LOCKED_MODULES; i++) {
        if (!g_LockedModules[i].Active) {
            return &g_LockedModules[i];
        }
    }
    return 0;
}

static PLOCKED_MODULE FindModule(U64 cr3, const char* moduleName) {
    for (U32 i = 0; i < MAX_LOCKED_MODULES; i++) {
        if (g_LockedModules[i].Active &&
            g_LockedModules[i].Cr3 == cr3 &&
            StrEqualNoCase(g_LockedModules[i].ModuleName, moduleName)) {
            return &g_LockedModules[i];
        }
    }
    return 0;
}

// Find module base address in PEB LDR
static bool FindModuleInProcess(U64 cr3, const char* moduleName,
                                U64* outBase, U64* outIatAddr, U64* outIatSize) {
    POMBRA_DRIVER_CTX ctx = &g_DriverCtx;

    // Would walk PEB->Ldr->InLoadOrderModuleList
    // For each module, compare name and extract IAT info from PE headers

    // Simplified: just use provided base if we can find it
    // Real implementation would:
    // 1. Get PEB from current subscription
    // 2. Walk Ldr->InLoadOrderModuleList
    // 3. Find matching DllBase
    // 4. Parse PE headers to find IAT

    (void)cr3;
    (void)moduleName;
    (void)ctx;
    *outBase = 0;
    *outIatAddr = 0;
    *outIatSize = 0;

    return false;  // Stub - would implement PEB walking
}

// =============================================================================
// Public API
// =============================================================================

void ModuleLockInit(void) {
    for (U32 i = 0; i < MAX_LOCKED_MODULES; i++) {
        g_LockedModules[i].Active = false;
    }
}

I32 ModuleLockAdd(U64 cr3, const char* moduleName) {
    if (!moduleName) return OMBRA_STATUS_INVALID_PARAMETER;

    // Check if already locked
    if (FindModule(cr3, moduleName)) {
        return OMBRA_STATUS_ALREADY_EXISTS;
    }

    // Find free slot
    PLOCKED_MODULE lock = FindFreeSlot();
    if (!lock) {
        return OMBRA_STATUS_LIMIT_EXCEEDED;
    }

    // Find module in process
    U64 imageBase = 0, iatAddr = 0, iatSize = 0;
    if (!FindModuleInProcess(cr3, moduleName, &imageBase, &iatAddr, &iatSize)) {
        // Module not found - might not be loaded yet
        // Store anyway and will resolve later
        lock->ImageBase = 0;
        lock->IatAddress = 0;
        lock->IatSize = 0;
        lock->IatHash = 0;
    } else {
        lock->ImageBase = imageBase;
        lock->IatAddress = iatAddr;
        lock->IatSize = iatSize;

        // Read IAT and compute hash
        if (iatSize > 0 && iatSize < 0x10000) {
            U8 iatBuffer[4096];  // Read up to 4KB
            U64 toRead = (iatSize > sizeof(iatBuffer)) ? sizeof(iatBuffer) : iatSize;

            I64 status = VmReadVirtual(cr3, iatAddr, iatBuffer, toRead);
            if (status == VMCALL_STATUS_SUCCESS) {
                lock->IatHash = HashMemory(iatBuffer, toRead);
            }
        }

        // Install shadow hook on IAT pages for tamper detection
        extern I32 ShadowInstall(U64 cr3, U64 virtualAddress, U64 size);
        ShadowInstall(cr3, iatAddr, iatSize);
    }

    // Store module info
    lock->Active = true;
    lock->Cr3 = cr3;
    StrCopy(lock->ModuleName, moduleName, sizeof(lock->ModuleName));

    return OMBRA_STATUS_SUCCESS;
}

I32 ModuleLockRemove(U64 cr3, const char* moduleName) {
    PLOCKED_MODULE lock = FindModule(cr3, moduleName);
    if (!lock) {
        return OMBRA_STATUS_NOT_FOUND;
    }

    // Remove shadow hook if installed
    if (lock->IatAddress && lock->IatSize) {
        extern I32 ShadowRemove(U64 cr3, U64 virtualAddress, U64 size);
        ShadowRemove(cr3, lock->IatAddress, lock->IatSize);
    }

    lock->Active = false;
    return OMBRA_STATUS_SUCCESS;
}

U32 ModuleLockVerifyAll(void) {
    U32 tamperedCount = 0;

    for (U32 i = 0; i < MAX_LOCKED_MODULES; i++) {
        PLOCKED_MODULE lock = &g_LockedModules[i];
        if (!lock->Active || lock->IatAddress == 0) continue;

        // Read current IAT
        U8 iatBuffer[4096];
        U64 toRead = (lock->IatSize > sizeof(iatBuffer)) ? sizeof(iatBuffer) : lock->IatSize;

        I64 status = VmReadVirtual(lock->Cr3, lock->IatAddress, iatBuffer, toRead);
        if (status != VMCALL_STATUS_SUCCESS) continue;

        // Compare hash
        U64 currentHash = HashMemory(iatBuffer, toRead);
        if (currentHash != lock->IatHash) {
            tamperedCount++;

            // Record detection event
            extern void DiagnosticsRecordEvent(U32 event);
            DiagnosticsRecordEvent(0x0200);  // DET_INTEGRITY_CHECK
        }
    }

    return tamperedCount;
}

U32 ModuleLockGetCount(void) {
    U32 count = 0;
    for (U32 i = 0; i < MAX_LOCKED_MODULES; i++) {
        if (g_LockedModules[i].Active) count++;
    }
    return count;
}

void ModuleLockRemoveAll(U64 cr3) {
    for (U32 i = 0; i < MAX_LOCKED_MODULES; i++) {
        if (g_LockedModules[i].Active && g_LockedModules[i].Cr3 == cr3) {
            if (g_LockedModules[i].IatAddress && g_LockedModules[i].IatSize) {
                extern I32 ShadowRemove(U64 cr3, U64 virtualAddress, U64 size);
                ShadowRemove(cr3, g_LockedModules[i].IatAddress, g_LockedModules[i].IatSize);
            }
            g_LockedModules[i].Active = false;
        }
    }
}

// =============================================================================
// Command Handlers
// =============================================================================

I32 HandleLockModule(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)resp;
    return ModuleLockAdd(cmd->Memory.Cr3, cmd->Memory.ModuleName);
}

I32 HandleUnlockModule(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)resp;
    return ModuleLockRemove(cmd->Memory.Cr3, cmd->Memory.ModuleName);
}
