// protection.c - Process Protection Implementation
// OmbraDriver Phase 3
//
// Protects processes from external access and blocks image loading

#include "context.h"
#include "command_ring.h"
#include "vmcall.h"

// =============================================================================
// Protection Tracking
// =============================================================================

#define MAX_PROTECTED_PROCESSES 16
#define MAX_BLOCKED_IMAGES 32

typedef struct _PROTECTED_PROCESS {
    bool    Active;
    U64     Cr3;
    U32     Method;         // Protection method flags
    U32     AccessMask;     // Which access types to block
} PROTECTED_PROCESS, *PPROTECTED_PROCESS;

typedef struct _BLOCKED_IMAGE {
    bool    Active;
    U64     Cr3;            // Target process (0 = global)
    char    ImageName[64];  // Image name to block
} BLOCKED_IMAGE, *PBLOCKED_IMAGE;

// Protection method flags
#define PROTECT_HANDLE_ACCESS   0x0001  // Block handle operations
#define PROTECT_MEMORY_ACCESS   0x0002  // Block memory read/write
#define PROTECT_THREAD_ACCESS   0x0004  // Block thread operations
#define PROTECT_TOKEN_ACCESS    0x0008  // Block token access
#define PROTECT_ALL             0xFFFF

// Access mask flags
#define ACCESS_READ             0x0001
#define ACCESS_WRITE            0x0002
#define ACCESS_EXECUTE          0x0004
#define ACCESS_ALL              0xFFFF

static PROTECTED_PROCESS g_ProtectedProcesses[MAX_PROTECTED_PROCESSES] = {0};
static BLOCKED_IMAGE g_BlockedImages[MAX_BLOCKED_IMAGES] = {0};

// =============================================================================
// String Helpers
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
// Process Protection
// =============================================================================

static PPROTECTED_PROCESS FindProtectedProcess(U64 cr3) {
    for (U32 i = 0; i < MAX_PROTECTED_PROCESSES; i++) {
        if (g_ProtectedProcesses[i].Active && g_ProtectedProcesses[i].Cr3 == cr3) {
            return &g_ProtectedProcesses[i];
        }
    }
    return 0;
}

I32 ProtectionAdd(U64 cr3, U32 method, U32 accessMask) {
    // Check if already protected
    PPROTECTED_PROCESS existing = FindProtectedProcess(cr3);
    if (existing) {
        // Update protection
        existing->Method = method;
        existing->AccessMask = accessMask;
        return OMBRA_STATUS_SUCCESS;
    }

    // Find free slot
    for (U32 i = 0; i < MAX_PROTECTED_PROCESSES; i++) {
        if (!g_ProtectedProcesses[i].Active) {
            g_ProtectedProcesses[i].Active = true;
            g_ProtectedProcesses[i].Cr3 = cr3;
            g_ProtectedProcesses[i].Method = method;
            g_ProtectedProcesses[i].AccessMask = accessMask;

            // Notify hypervisor to hook handle operations for this CR3
            // This would intercept NtOpenProcess, NtReadVirtualMemory, etc.
            // Implementation via EPT hooks on kernel functions

            return OMBRA_STATUS_SUCCESS;
        }
    }

    return OMBRA_STATUS_LIMIT_EXCEEDED;
}

I32 ProtectionRemove(U64 cr3) {
    PPROTECTED_PROCESS prot = FindProtectedProcess(cr3);
    if (!prot) {
        return OMBRA_STATUS_NOT_FOUND;
    }

    prot->Active = false;
    return OMBRA_STATUS_SUCCESS;
}

// Check if access should be blocked
bool ProtectionShouldBlock(U64 targetCr3, U64 accessorCr3, U32 accessType) {
    PPROTECTED_PROCESS prot = FindProtectedProcess(targetCr3);
    if (!prot) return false;

    // Allow self-access
    if (targetCr3 == accessorCr3) return false;

    // Check if this access type is blocked
    return (prot->AccessMask & accessType) != 0;
}

// =============================================================================
// Image Blocking
// =============================================================================

static PBLOCKED_IMAGE FindBlockedImage(U64 cr3, const char* imageName) {
    for (U32 i = 0; i < MAX_BLOCKED_IMAGES; i++) {
        if (g_BlockedImages[i].Active &&
            (g_BlockedImages[i].Cr3 == 0 || g_BlockedImages[i].Cr3 == cr3) &&
            StrEqualNoCase(g_BlockedImages[i].ImageName, imageName)) {
            return &g_BlockedImages[i];
        }
    }
    return 0;
}

I32 ImageBlockAdd(U64 cr3, const char* imageName) {
    if (!imageName) return OMBRA_STATUS_INVALID_PARAMETER;

    // Check if already blocked
    if (FindBlockedImage(cr3, imageName)) {
        return OMBRA_STATUS_ALREADY_EXISTS;
    }

    // Find free slot
    for (U32 i = 0; i < MAX_BLOCKED_IMAGES; i++) {
        if (!g_BlockedImages[i].Active) {
            g_BlockedImages[i].Active = true;
            g_BlockedImages[i].Cr3 = cr3;
            StrCopy(g_BlockedImages[i].ImageName, imageName, sizeof(g_BlockedImages[i].ImageName));

            // Would need to hook NtMapViewOfSection or similar
            // to intercept DLL loading and check against block list

            return OMBRA_STATUS_SUCCESS;
        }
    }

    return OMBRA_STATUS_LIMIT_EXCEEDED;
}

I32 ImageBlockRemove(U64 cr3, const char* imageName) {
    PBLOCKED_IMAGE block = FindBlockedImage(cr3, imageName);
    if (!block) {
        return OMBRA_STATUS_NOT_FOUND;
    }

    block->Active = false;
    return OMBRA_STATUS_SUCCESS;
}

// Check if image should be blocked
bool ImageShouldBlock(U64 cr3, const char* imageName) {
    return FindBlockedImage(cr3, imageName) != 0;
}

// =============================================================================
// Command Handlers
// =============================================================================

I32 HandleProtectProcess(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)resp;
    return ProtectionAdd(cmd->Protection.Cr3, cmd->Protection.Method, cmd->Protection.AccessMask);
}

I32 HandleUnprotectProcess(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)resp;
    return ProtectionRemove(cmd->Protection.Cr3);
}

I32 HandleBlockImage(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)resp;
    return ImageBlockAdd(cmd->Memory.Cr3, cmd->Memory.ModuleName);
}

I32 HandleUnblockImage(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)resp;
    return ImageBlockRemove(cmd->Memory.Cr3, cmd->Memory.ModuleName);
}
