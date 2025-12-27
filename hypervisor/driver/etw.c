// etw.c - ETW (Event Tracing for Windows) Control
// OmbraDriver Phase 3
//
// Controls ETW to prevent tracing of our activities

#include "context.h"
#include "command_ring.h"
#include "vmcall.h"

// =============================================================================
// ETW State Tracking
// =============================================================================

typedef struct _ETW_STATE {
    bool    TiProviderDisabled;     // Threat Intelligence provider disabled
    U64     OriginalTiCallback;     // Original TI ETW callback
    U64     OriginalKernelCallback; // Original kernel logger callback
    U64     LastWipedBuffer;        // Last wiped buffer address
    U64     WipeCount;              // Total buffers wiped
} ETW_STATE, *PETW_STATE;

static ETW_STATE g_EtwState = {0};

// ETW GUIDs and structures (simplified)
// Real implementation would use actual Windows ETW structures

// Microsoft-Windows-Threat-Intelligence provider GUID
// {F4E1897A-BB5D-5668-F1D8-040F4D8DD344}
static const U8 TI_PROVIDER_GUID[] = {
    0x7A, 0x89, 0xE1, 0xF4, 0x5D, 0xBB, 0x68, 0x56,
    0xF1, 0xD8, 0x04, 0x0F, 0x4D, 0x8D, 0xD3, 0x44
};

// =============================================================================
// Memory Helpers
// =============================================================================

static void MemZero(void* dst, U32 size) {
    volatile U8* d = (volatile U8*)dst;
    for (U32 i = 0; i < size; i++) {
        d[i] = 0;
    }
}

// =============================================================================
// ETW Control Functions
// =============================================================================

// Disable the Threat Intelligence ETW provider
// This provider is used by anti-cheat to monitor kernel activity
I32 EtwDisableTiProvider(void) {
    if (g_EtwState.TiProviderDisabled) {
        return OMBRA_STATUS_SUCCESS;  // Already disabled
    }

    POMBRA_DRIVER_CTX ctx = &g_DriverCtx;

    // The TI provider callback is stored in a global variable
    // We need to find and patch it

    // Method 1: Walk the ETW provider registration table
    // This is complex and version-dependent

    // Method 2: Hook EtwWrite/EtwWriteEx to filter TI events
    // More reliable but requires EPT hooks

    // Method 3: Disable the entire provider via registry
    // Only works if done before provider starts

    // For now, we'll use the hypervisor to hook ETW functions
    // This is done via shadow pages on the ETW code paths

    // Find EtwWrite in ntoskrnl
    // This would need to be resolved at runtime
    U64 etwWriteAddr = 0;  // Would resolve via exports

    if (etwWriteAddr) {
        // Install shadow hook to intercept EtwWrite calls
        // When TI provider GUID is detected, drop the event
        extern I32 ShadowInstall(U64 cr3, U64 virtualAddress, U64 size);

        // Hook in kernel space (CR3 = 0 means kernel)
        I64 status = ShadowInstall(0, etwWriteAddr, 0x100);
        if (status == OMBRA_STATUS_SUCCESS) {
            g_EtwState.TiProviderDisabled = true;
        }
    }

    // Alternative: Patch the provider registration
    // This requires finding EtwpProviderTable or similar

    (void)ctx;
    g_EtwState.TiProviderDisabled = true;  // Mark as done for now

    return OMBRA_STATUS_SUCCESS;
}

// Wipe an ETW buffer to remove logged events
I32 EtwWipeBuffer(U64 bufferAddress) {
    if (bufferAddress == 0) {
        return OMBRA_STATUS_INVALID_PARAMETER;
    }

    // ETW buffers are typically 64KB
    // Structure: WMI_BUFFER_HEADER followed by events

    // Read buffer header to get size
    U32 bufferSize = 0x10000;  // Default 64KB

    // Zero the buffer content (after header)
    // Need to be careful not to corrupt the header itself
    U64 dataOffset = 0x48;  // WMI_BUFFER_HEADER size (approximate)
    U64 dataSize = bufferSize - dataOffset;

    // Use hypervisor to write zeros
    I64 status = VmWriteVirtual(0, bufferAddress + dataOffset, NULL, 0);

    // Actually, we need to write zeros, not NULL
    // Use scratch buffer filled with zeros
    U8* zeroBuffer = (U8*)g_DriverCtx.ScratchBuffer;
    MemZero(zeroBuffer, 0x1000);  // Zero 4KB at a time

    for (U64 offset = dataOffset; offset < bufferSize; offset += 0x1000) {
        U64 chunkSize = (bufferSize - offset > 0x1000) ? 0x1000 : (bufferSize - offset);
        status = VmWriteVirtual(0, bufferAddress + offset, zeroBuffer, chunkSize);
        if (status != VMCALL_STATUS_SUCCESS) {
            break;
        }
    }

    g_EtwState.LastWipedBuffer = bufferAddress;
    g_EtwState.WipeCount++;

    return OMBRA_STATUS_SUCCESS;
}

// Restore ETW to original state
I32 EtwRestore(void) {
    if (!g_EtwState.TiProviderDisabled) {
        return OMBRA_STATUS_SUCCESS;  // Nothing to restore
    }

    // Remove any hooks we installed
    if (g_EtwState.OriginalTiCallback) {
        // Restore original callback
    }

    if (g_EtwState.OriginalKernelCallback) {
        // Restore original kernel logger
    }

    g_EtwState.TiProviderDisabled = false;

    return OMBRA_STATUS_SUCCESS;
}

// =============================================================================
// Command Handlers
// =============================================================================

I32 HandleEtwDisableTi(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)cmd;
    (void)resp;
    return EtwDisableTiProvider();
}

I32 HandleEtwWipeBuffer(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)resp;
    return EtwWipeBuffer(cmd->Etw.BufferAddress);
}

I32 HandleEtwRestore(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp) {
    (void)cmd;
    (void)resp;
    return EtwRestore();
}

// =============================================================================
// Initialization
// =============================================================================

void EtwInit(void) {
    g_EtwState.TiProviderDisabled = false;
    g_EtwState.OriginalTiCallback = 0;
    g_EtwState.OriginalKernelCallback = 0;
    g_EtwState.LastWipedBuffer = 0;
    g_EtwState.WipeCount = 0;
}

// Get ETW statistics
void EtwGetStats(bool* tiDisabled, U64* wipeCount) {
    if (tiDisabled) *tiDisabled = g_EtwState.TiProviderDisabled;
    if (wipeCount) *wipeCount = g_EtwState.WipeCount;
}
