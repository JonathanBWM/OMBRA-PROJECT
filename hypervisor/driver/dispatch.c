// dispatch.c - Command Dispatcher and Handlers
// OmbraDriver Phase 3

#include "context.h"
#include "command_ring.h"
#include "subscription.h"
#include "module_lock.h"
#include "shadow.h"
#include "diagnostics.h"
#include "memory_ops.h"
#include "protection.h"
#include "spoof.h"
#include "etw.h"
#include <intrin.h>

// Forward declare handler functions
I32 HandleSubscribe(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleUnsubscribe(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleGetInfo(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleResetInfo(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleEnumProcesses(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleEnumModules(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleHideMemory(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleShadowMemory(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleLockModule(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleUnlockModule(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleReadPhysical(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleWritePhysical(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleReadVirtual(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleWriteVirtual(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleInject(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleInjectHidden(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleSetOverlay(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleGetOverlay(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleSetDefaultOverlay(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleProtectProcess(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleUnprotectProcess(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleBlockImage(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleUnblockImage(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleGetIdentityMap(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleReleaseIdentityMap(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleSpoofDisk(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleSpoofNic(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleSpoofVolume(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleSpoofQuery(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleEtwDisableTi(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleEtwWipeBuffer(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleEtwRestore(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandlePing(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleGetStatus(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleGetScore(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleResetScore(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleConfigScore(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleShutdown(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);

// Main command dispatcher
void DispatchCommand(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    I32 status = OMBRA_STATUS_INVALID_COMMAND;

    switch (cmd->CommandId) {
        // Process Tracking (0x01xx)
        case OMBRA_CMD_SUBSCRIBE:
            status = HandleSubscribe(cmd, resp);
            break;
        case OMBRA_CMD_UNSUBSCRIBE:
            status = HandleUnsubscribe(cmd, resp);
            break;
        case OMBRA_CMD_GET_INFO:
            status = HandleGetInfo(cmd, resp);
            break;
        case OMBRA_CMD_RESET_INFO:
            status = HandleResetInfo(cmd, resp);
            break;
        case OMBRA_CMD_ENUM_PROCESSES:
            status = HandleEnumProcesses(cmd, resp);
            break;
        case OMBRA_CMD_ENUM_MODULES:
            status = HandleEnumModules(cmd, resp);
            break;

        // Memory Operations (0x02xx)
        case OMBRA_CMD_HIDE_MEMORY:
            status = HandleHideMemory(cmd, resp);
            break;
        case OMBRA_CMD_SHADOW_MEMORY:
            status = HandleShadowMemory(cmd, resp);
            break;
        case OMBRA_CMD_LOCK_MODULE:
            status = HandleLockModule(cmd, resp);
            break;
        case OMBRA_CMD_UNLOCK_MODULE:
            status = HandleUnlockModule(cmd, resp);
            break;
        case OMBRA_CMD_READ_PHYSICAL:
            status = HandleReadPhysical(cmd, resp);
            break;
        case OMBRA_CMD_WRITE_PHYSICAL:
            status = HandleWritePhysical(cmd, resp);
            break;
        case OMBRA_CMD_READ_VIRTUAL:
            status = HandleReadVirtual(cmd, resp);
            break;
        case OMBRA_CMD_WRITE_VIRTUAL:
            status = HandleWriteVirtual(cmd, resp);
            break;

        // Injection (0x03xx)
        case OMBRA_CMD_INJECT:
            status = HandleInject(cmd, resp);
            break;
        case OMBRA_CMD_INJECT_HIDDEN:
            status = HandleInjectHidden(cmd, resp);
            break;

        // Window Hiding (0x04xx)
        case OMBRA_CMD_SET_OVERLAY:
            status = HandleSetOverlay(cmd, resp);
            break;
        case OMBRA_CMD_GET_OVERLAY:
            status = HandleGetOverlay(cmd, resp);
            break;
        case OMBRA_CMD_SET_DEFAULT_OVERLAY:
            status = HandleSetDefaultOverlay(cmd, resp);
            break;

        // Protection (0x05xx)
        case OMBRA_CMD_PROTECT_PROCESS:
            status = HandleProtectProcess(cmd, resp);
            break;
        case OMBRA_CMD_UNPROTECT_PROCESS:
            status = HandleUnprotectProcess(cmd, resp);
            break;
        case OMBRA_CMD_BLOCK_IMAGE:
            status = HandleBlockImage(cmd, resp);
            break;
        case OMBRA_CMD_UNBLOCK_IMAGE:
            status = HandleUnblockImage(cmd, resp);
            break;

        // Identity Map (0x06xx)
        case OMBRA_CMD_GET_IDENTITY_MAP:
            status = HandleGetIdentityMap(cmd, resp);
            break;
        case OMBRA_CMD_RELEASE_IDENTITY_MAP:
            status = HandleReleaseIdentityMap(cmd, resp);
            break;

        // Spoofing (0x07xx)
        case OMBRA_CMD_SPOOF_DISK:
            status = HandleSpoofDisk(cmd, resp);
            break;
        case OMBRA_CMD_SPOOF_NIC:
            status = HandleSpoofNic(cmd, resp);
            break;
        case OMBRA_CMD_SPOOF_VOLUME:
            status = HandleSpoofVolume(cmd, resp);
            break;
        case OMBRA_CMD_SPOOF_QUERY:
            status = HandleSpoofQuery(cmd, resp);
            break;

        // ETW (0x08xx)
        case OMBRA_CMD_ETW_DISABLE_TI:
            status = HandleEtwDisableTi(cmd, resp);
            break;
        case OMBRA_CMD_ETW_WIPE_BUFFER:
            status = HandleEtwWipeBuffer(cmd, resp);
            break;
        case OMBRA_CMD_ETW_RESTORE:
            status = HandleEtwRestore(cmd, resp);
            break;

        // Diagnostics (0x09xx)
        case OMBRA_CMD_PING:
            status = HandlePing(cmd, resp);
            break;
        case OMBRA_CMD_GET_STATUS:
            status = HandleGetStatus(cmd, resp);
            break;
        case OMBRA_CMD_GET_SCORE:
            status = HandleGetScore(cmd, resp);
            break;
        case OMBRA_CMD_RESET_SCORE:
            status = HandleResetScore(cmd, resp);
            break;
        case OMBRA_CMD_CONFIG_SCORE:
            status = HandleConfigScore(cmd, resp);
            break;
        case OMBRA_CMD_SHUTDOWN:
            status = HandleShutdown(cmd, resp);
            break;

        default:
            status = OMBRA_STATUS_INVALID_COMMAND;
            break;
    }

    // Complete the response
    CmdRingCompleteResponse(resp, status);
}

// =============================================================================
// Process Tracking Handlers
// =============================================================================

I32 HandleSubscribe(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    // SubscriptionAdd takes only imageName and returns subscription pointer
    POMBRA_SUBSCRIPTION sub = SubscriptionAdd(cmd->Process.ImageName);
    if (!sub) {
        return OMBRA_STATUS_NO_MEMORY;
    }

    // Return the CR3 in the ProcessInfo response
    resp->ProcessInfo.Cr3 = sub->Cr3;
    resp->ProcessInfo.Pid = (U32)sub->Pid;
    resp->ProcessInfo.ImageBase = sub->ImageBase;
    return OMBRA_STATUS_SUCCESS;
}

I32 HandleUnsubscribe(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)resp;
    // SubscriptionRemove takes imageName string, not Cr3
    return SubscriptionRemove(cmd->Process.ImageName) ? OMBRA_STATUS_SUCCESS : OMBRA_STATUS_NOT_FOUND;
}

I32 HandleGetInfo(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    // Use SubscriptionFindByCr3 when looking up by CR3
    POMBRA_SUBSCRIPTION sub = SubscriptionFindByCr3(cmd->Process.Cr3);
    if (!sub) return OMBRA_STATUS_NOT_FOUND;

    // Use correct field names from OMBRA_RESPONSE.ProcessInfo
    resp->ProcessInfo.Cr3 = sub->Cr3;
    resp->ProcessInfo.Pid = (U32)sub->Pid;
    resp->ProcessInfo.ImageBase = sub->ImageBase;  // Not BaseAddress
    resp->ProcessInfo.Peb = sub->Peb;
    resp->ProcessInfo.Eprocess = sub->Eprocess;
    resp->ProcessInfo.Dead = sub->Dead ? 1 : 0;
    resp->ProcessInfo.DllInjected = sub->DllInjected ? 1 : 0;
    return OMBRA_STATUS_SUCCESS;
}

I32 HandleResetInfo(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)resp;
    // Use SubscriptionFindByCr3 when looking up by CR3
    POMBRA_SUBSCRIPTION sub = SubscriptionFindByCr3(cmd->Process.Cr3);
    if (!sub) return OMBRA_STATUS_NOT_FOUND;
    // Reset tracking state (subscription struct doesn't have ModuleCount)
    sub->DllInjected = false;
    return OMBRA_STATUS_SUCCESS;
}

I32 HandleEnumProcesses(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)cmd;
    (void)resp;
    // Would enumerate EPROCESS list - complex, leaving stubbed
    return OMBRA_STATUS_NOT_IMPLEMENTED;
}

I32 HandleEnumModules(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)cmd;
    (void)resp;
    // Would walk PEB LDR - complex, leaving stubbed
    return OMBRA_STATUS_NOT_IMPLEMENTED;
}

// =============================================================================
// Memory Operations Handlers - Forward to implementations
// =============================================================================

// Implemented in shadow.c
// I32 HandleHideMemory(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
// I32 HandleShadowMemory(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);

// Implemented in module_lock.c
// I32 HandleLockModule(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
// I32 HandleUnlockModule(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);

// Implemented in memory_ops.c
// I32 HandleReadPhysical(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
// I32 HandleWritePhysical(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
// I32 HandleReadVirtual(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
// I32 HandleWriteVirtual(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);

// =============================================================================
// Injection Handlers (Stubbed)
// =============================================================================

I32 HandleInject(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)cmd;
    (void)resp;
    return OMBRA_STATUS_NOT_IMPLEMENTED;
}

I32 HandleInjectHidden(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)cmd;
    (void)resp;
    return OMBRA_STATUS_NOT_IMPLEMENTED;
}

// =============================================================================
// Window Hiding Handlers (Stubbed)
// =============================================================================

I32 HandleSetOverlay(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)cmd;
    (void)resp;
    return OMBRA_STATUS_NOT_IMPLEMENTED;
}

I32 HandleGetOverlay(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)cmd;
    (void)resp;
    return OMBRA_STATUS_NOT_IMPLEMENTED;
}

I32 HandleSetDefaultOverlay(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)cmd;
    (void)resp;
    return OMBRA_STATUS_NOT_IMPLEMENTED;
}

// =============================================================================
// Protection Handlers - Implemented in protection.c
// =============================================================================
// I32 HandleProtectProcess(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
// I32 HandleUnprotectProcess(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
// I32 HandleBlockImage(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
// I32 HandleUnblockImage(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);

// =============================================================================
// Identity Map Handlers - Implemented in memory_ops.c
// =============================================================================
// I32 HandleGetIdentityMap(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
// I32 HandleReleaseIdentityMap(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);

// =============================================================================
// Spoofing Handlers - Implemented in spoof.c
// =============================================================================
// I32 HandleSpoofDisk(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
// I32 HandleSpoofNic(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
// I32 HandleSpoofVolume(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
// I32 HandleSpoofQuery(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);

// =============================================================================
// ETW Handlers - Implemented in etw.c
// =============================================================================
// I32 HandleEtwDisableTi(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
// I32 HandleEtwWipeBuffer(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
// I32 HandleEtwRestore(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);

// =============================================================================
// Score Handlers - Implemented in diagnostics.c
// =============================================================================
// I32 HandleGetScore(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
// I32 HandleResetScore(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
// I32 HandleConfigScore(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
// I32 HandlePing(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
// I32 HandleGetStatus(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
// I32 HandleShutdown(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
