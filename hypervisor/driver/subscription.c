#include "subscription.h"
#include "context.h"
#include "vmcall.h"

// Static subscription storage
static OMBRA_SUBSCRIPTION g_Subscriptions[OMBRA_MAX_SUBSCRIPTIONS] = {0};

// String comparison (no CRT)
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

void SubscriptionInit(void) {
    for (U32 i = 0; i < OMBRA_MAX_SUBSCRIPTIONS; i++) {
        g_Subscriptions[i].Active = false;
    }
    g_DriverCtx.SubscriptionCount = 0;
}

POMBRA_SUBSCRIPTION SubscriptionAdd(const char* imageName) {
    if (!imageName) return 0;

    // Check if already exists
    POMBRA_SUBSCRIPTION existing = SubscriptionFind(imageName);
    if (existing) return existing;

    // Find free slot
    for (U32 i = 0; i < OMBRA_MAX_SUBSCRIPTIONS; i++) {
        if (!g_Subscriptions[i].Active) {
            POMBRA_SUBSCRIPTION sub = &g_Subscriptions[i];

            // Clear and initialize
            volatile U8* p = (volatile U8*)sub;
            for (U32 j = 0; j < sizeof(OMBRA_SUBSCRIPTION); j++) p[j] = 0;

            StrCopy(sub->ImageName, imageName, sizeof(sub->ImageName));
            sub->Active = true;

            // Store in context
            g_DriverCtx.Subscriptions[g_DriverCtx.SubscriptionCount++] = sub;

            // Do immediate scan to find process
            SubscriptionScan();

            return sub;
        }
    }

    return 0;  // No free slots
}

bool SubscriptionRemove(const char* imageName) {
    POMBRA_SUBSCRIPTION sub = SubscriptionFind(imageName);
    if (!sub) return false;

    // If we were watching CR3, stop
    if (sub->Cr3) {
        VmUnwatchCr3(sub->Cr3);
    }

    sub->Active = false;

    // Remove from context array
    for (U32 i = 0; i < g_DriverCtx.SubscriptionCount; i++) {
        if (g_DriverCtx.Subscriptions[i] == sub) {
            // Shift remaining
            for (U32 j = i; j < g_DriverCtx.SubscriptionCount - 1; j++) {
                g_DriverCtx.Subscriptions[j] = g_DriverCtx.Subscriptions[j + 1];
            }
            g_DriverCtx.SubscriptionCount--;
            break;
        }
    }

    return true;
}

POMBRA_SUBSCRIPTION SubscriptionFind(const char* imageName) {
    for (U32 i = 0; i < OMBRA_MAX_SUBSCRIPTIONS; i++) {
        if (g_Subscriptions[i].Active &&
            StrEqualNoCase(g_Subscriptions[i].ImageName, imageName)) {
            return &g_Subscriptions[i];
        }
    }
    return 0;
}

POMBRA_SUBSCRIPTION SubscriptionFindByCr3(U64 cr3) {
    for (U32 i = 0; i < OMBRA_MAX_SUBSCRIPTIONS; i++) {
        if (g_Subscriptions[i].Active && g_Subscriptions[i].Cr3 == cr3) {
            return &g_Subscriptions[i];
        }
    }
    return 0;
}

// Walk EPROCESS list to find processes
void SubscriptionScan(void) {
    POMBRA_DRIVER_CTX ctx = &g_DriverCtx;

    // Get current process as starting point
    typedef void* (*PsGetCurrentProcessFn)(void);
    PsGetCurrentProcessFn getCurrentProcess = (PsGetCurrentProcessFn)ctx->Imports.PsGetCurrentProcess;
    if (!getCurrentProcess) return;

    void* currentProcess = getCurrentProcess();
    if (!currentProcess) return;

    // Walk ActiveProcessLinks
    U64 eprocess = (U64)currentProcess;
    U64 startEprocess = eprocess;
    U64 linksOffset = ctx->Offsets.ActiveProcessLinks;
    U64 imageNameOffset = ctx->Offsets.ImageFileName;
    U64 cr3Offset = ctx->Offsets.DirectoryTableBase;
    U64 pidOffset = ctx->Offsets.UniqueProcessId;
    U64 pebOffset = ctx->Offsets.Peb;

    do {
        // Read image name (15 chars max in EPROCESS)
        char imageName[16] = {0};
        volatile U8* namePtr = (volatile U8*)(eprocess + imageNameOffset);
        for (int i = 0; i < 15; i++) {
            imageName[i] = namePtr[i];
        }

        // Check against subscriptions
        for (U32 i = 0; i < OMBRA_MAX_SUBSCRIPTIONS; i++) {
            POMBRA_SUBSCRIPTION sub = &g_Subscriptions[i];
            if (!sub->Active || sub->Cr3) continue;  // Skip if already found

            if (StrEqualNoCase(sub->ImageName, imageName)) {
                // Found it!
                sub->Eprocess = eprocess;
                sub->Cr3 = *(U64*)(eprocess + cr3Offset);
                sub->Pid = *(U64*)(eprocess + pidOffset);
                sub->Peb = *(U64*)(eprocess + pebOffset);

                // Register CR3 with hypervisor for tracking
                VmWatchCr3(sub->Cr3);
            }
        }

        // Move to next process
        U64 flink = *(U64*)(eprocess + linksOffset);
        eprocess = flink - linksOffset;

    } while (eprocess != startEprocess && eprocess != 0);
}

bool SubscriptionCheckCr3(U64 cr3, U64 eprocess) {
    POMBRA_SUBSCRIPTION sub = SubscriptionFindByCr3(cr3);
    if (sub) {
        // Update EPROCESS if changed (shouldn't happen normally)
        if (sub->Eprocess != eprocess) {
            sub->Eprocess = eprocess;
        }
        return true;
    }
    return false;
}
