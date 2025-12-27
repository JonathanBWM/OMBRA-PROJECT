// ombra_client.c - Usermode Client API Implementation
// OmbraHypervisor Phase 3

#include "ombra_client.h"
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#define MEMORY_BARRIER() MemoryBarrier()
#else
#include <unistd.h>
#define SLEEP_MS(ms) usleep((ms) * 1000)
#define MEMORY_BARRIER() __sync_synchronize()
#endif

// Global client context
OMBRA_CLIENT_CTX g_OmbraClient = {0};

// =============================================================================
// Internal Helpers
// =============================================================================

static inline POMBRA_COMMAND GetCommand(U32 slot) {
    U8* base = (U8*)g_OmbraClient.Ring;
    return (POMBRA_COMMAND)(base + OMBRA_RING_HEADER_SIZE + (slot * OMBRA_COMMAND_SIZE));
}

static inline POMBRA_RESPONSE GetResponse(U32 slot) {
    U8* base = (U8*)g_OmbraClient.Ring;
    U64 responseOffset = OMBRA_RING_HEADER_SIZE + (OMBRA_RING_SIZE * OMBRA_COMMAND_SIZE);
    return (POMBRA_RESPONSE)(base + responseOffset + (slot * OMBRA_RESPONSE_SIZE));
}

static U64 GetNextSequenceId(void) {
    // Simple atomic increment
#ifdef _WIN32
    return (U64)InterlockedIncrement64((volatile LONG64*)&g_OmbraClient.NextSequenceId);
#else
    return __sync_add_and_fetch(&g_OmbraClient.NextSequenceId, 1);
#endif
}

static void CopyString(char* dest, const char* src, U32 maxLen) {
    U32 i = 0;
    while (src[i] && i < maxLen - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

// =============================================================================
// Initialization / Shutdown
// =============================================================================

OMBRA_CLIENT_STATUS OmbraClientInit(void* sharedMemory, U64 size) {
    if (g_OmbraClient.Initialized) {
        return OMBRA_CLIENT_ALREADY_INITIALIZED;
    }

    if (!sharedMemory || size < OMBRA_RING_HEADER_SIZE) {
        return OMBRA_CLIENT_INVALID_PARAM;
    }

    // Validate ring magic
    POMBRA_COMMAND_RING ring = (POMBRA_COMMAND_RING)sharedMemory;
    if (ring->Magic != OMBRA_RING_MAGIC) {
        return OMBRA_CLIENT_INTERNAL_ERROR;  // Ring not initialized by driver
    }

    g_OmbraClient.SharedMemory = sharedMemory;
    g_OmbraClient.SharedMemorySize = size;
    g_OmbraClient.Ring = ring;

    // Calculate scratch buffer location
    U64 ringDataSize = OMBRA_RING_HEADER_SIZE +
                       (OMBRA_RING_SIZE * OMBRA_COMMAND_SIZE) +
                       (OMBRA_RING_SIZE * OMBRA_RESPONSE_SIZE);

    if (ring->ScratchBufferOffset && ring->ScratchBufferSize) {
        g_OmbraClient.ScratchBuffer = (U8*)sharedMemory + ring->ScratchBufferOffset;
        g_OmbraClient.ScratchSize = ring->ScratchBufferSize;
    } else if (size > ringDataSize) {
        // Use remaining space as scratch
        g_OmbraClient.ScratchBuffer = (U8*)sharedMemory + ringDataSize;
        g_OmbraClient.ScratchSize = size - ringDataSize;
    } else {
        g_OmbraClient.ScratchBuffer = NULL;
        g_OmbraClient.ScratchSize = 0;
    }

    g_OmbraClient.NextSequenceId = 1;
    g_OmbraClient.Initialized = true;

    return OMBRA_CLIENT_SUCCESS;
}

void OmbraClientShutdown(void) {
    if (!g_OmbraClient.Initialized) {
        return;
    }

    // Send shutdown command
    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};
    cmd.CommandId = OMBRA_CMD_SHUTDOWN;
    OmbraExecuteCommand(&cmd, 1000, &resp);

    // Clear context
    memset(&g_OmbraClient, 0, sizeof(g_OmbraClient));
}

bool OmbraClientIsReady(void) {
    return g_OmbraClient.Initialized;
}

// =============================================================================
// Low-Level Ring Operations
// =============================================================================

U64 OmbraSubmitCommand(POMBRA_COMMAND cmd) {
    if (!g_OmbraClient.Initialized || !cmd) {
        return 0;
    }

    POMBRA_COMMAND_RING ring = g_OmbraClient.Ring;

    // Check if ring is full
    U32 producer = ring->ProducerIndex;
    MEMORY_BARRIER();
    U32 consumer = ring->ConsumerIndex;

    if ((producer - consumer) >= ring->RingSize) {
        return 0;  // Ring full
    }

    // Assign sequence ID
    U64 seqId = GetNextSequenceId();
    cmd->SequenceId = seqId;

    // Get slot and copy command
    U32 slot = producer % ring->RingSize;
    POMBRA_COMMAND ringCmd = GetCommand(slot);

    // Clear the response slot first
    POMBRA_RESPONSE resp = GetResponse(slot);
    resp->Ready = 0;
    resp->Status = 0;
    resp->SequenceId = seqId;
    MEMORY_BARRIER();

    // Copy command to ring
    memcpy((void*)ringCmd, cmd, sizeof(OMBRA_COMMAND));
    MEMORY_BARRIER();

    // Advance producer index (makes command visible to driver)
    ring->ProducerIndex = producer + 1;
    MEMORY_BARRIER();

    return seqId;
}

OMBRA_CLIENT_STATUS OmbraWaitResponse(U64 sequenceId, U32 timeoutMs, POMBRA_RESPONSE response) {
    if (!g_OmbraClient.Initialized) {
        return OMBRA_CLIENT_NOT_INITIALIZED;
    }

    if (!response) {
        return OMBRA_CLIENT_INVALID_PARAM;
    }

    // Calculate which slot to poll
    // The slot is determined by the sequence ID's corresponding producer index
    U32 startTime = 0;
#ifdef _WIN32
    startTime = GetTickCount();
#endif

    // Poll all slots looking for matching sequence ID
    while (1) {
        for (U32 slot = 0; slot < OMBRA_RING_SIZE; slot++) {
            POMBRA_RESPONSE resp = GetResponse(slot);
            MEMORY_BARRIER();

            if (resp->SequenceId == sequenceId && resp->Ready) {
                memcpy(response, (void*)resp, sizeof(OMBRA_RESPONSE));
                return OMBRA_CLIENT_SUCCESS;
            }
        }

        // Check timeout
        if (timeoutMs > 0) {
#ifdef _WIN32
            U32 elapsed = GetTickCount() - startTime;
            if (elapsed >= timeoutMs) {
                return OMBRA_CLIENT_TIMEOUT;
            }
#endif
        }

        // Brief sleep to avoid spinning
        SLEEP_MS(1);
    }
}

OMBRA_CLIENT_STATUS OmbraExecuteCommand(POMBRA_COMMAND cmd, U32 timeoutMs, POMBRA_RESPONSE response) {
    U64 seqId = OmbraSubmitCommand(cmd);
    if (seqId == 0) {
        return OMBRA_CLIENT_RING_FULL;
    }

    return OmbraWaitResponse(seqId, timeoutMs, response);
}

// =============================================================================
// Process Tracking API
// =============================================================================

OMBRA_CLIENT_STATUS OmbraSubscribe(const char* imageName, U64* outCr3) {
    if (!imageName) {
        return OMBRA_CLIENT_INVALID_PARAM;
    }

    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_SUBSCRIBE;
    CopyString(cmd.Process.ImageName, imageName, sizeof(cmd.Process.ImageName));

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    if (resp.Status != OMBRA_STATUS_SUCCESS) {
        return OMBRA_CLIENT_DRIVER_ERROR;
    }

    if (outCr3) {
        *outCr3 = resp.ProcessInfo.Cr3;
    }

    return OMBRA_CLIENT_SUCCESS;
}

OMBRA_CLIENT_STATUS OmbraUnsubscribe(const char* imageName) {
    if (!imageName) {
        return OMBRA_CLIENT_INVALID_PARAM;
    }

    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_UNSUBSCRIBE;
    CopyString(cmd.Process.ImageName, imageName, sizeof(cmd.Process.ImageName));

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    return (resp.Status == OMBRA_STATUS_SUCCESS) ? OMBRA_CLIENT_SUCCESS : OMBRA_CLIENT_DRIVER_ERROR;
}

OMBRA_CLIENT_STATUS OmbraGetProcessInfo(const char* imageName, OMBRA_PROCESS_INFO* outInfo) {
    if (!imageName || !outInfo) {
        return OMBRA_CLIENT_INVALID_PARAM;
    }

    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_GET_INFO;
    CopyString(cmd.Process.ImageName, imageName, sizeof(cmd.Process.ImageName));

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    if (resp.Status != OMBRA_STATUS_SUCCESS) {
        return OMBRA_CLIENT_DRIVER_ERROR;
    }

    outInfo->Cr3 = resp.ProcessInfo.Cr3;
    outInfo->Peb = resp.ProcessInfo.Peb;
    outInfo->Eprocess = resp.ProcessInfo.Eprocess;
    outInfo->ImageBase = resp.ProcessInfo.ImageBase;
    outInfo->Pid = resp.ProcessInfo.Pid;
    outInfo->Dead = resp.ProcessInfo.Dead ? true : false;
    outInfo->DllInjected = resp.ProcessInfo.DllInjected ? true : false;

    return OMBRA_CLIENT_SUCCESS;
}

// =============================================================================
// Memory Operations API
// =============================================================================

OMBRA_CLIENT_STATUS OmbraHideMemory(U64 cr3, U64 address, U64 size) {
    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_HIDE_MEMORY;
    cmd.Memory.Cr3 = cr3;
    cmd.Memory.Address = address;
    cmd.Memory.Size = size;

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    return (resp.Status == OMBRA_STATUS_SUCCESS) ? OMBRA_CLIENT_SUCCESS : OMBRA_CLIENT_DRIVER_ERROR;
}

OMBRA_CLIENT_STATUS OmbraShadowMemory(U64 cr3, U64 address, U64 size) {
    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_SHADOW_MEMORY;
    cmd.Memory.Cr3 = cr3;
    cmd.Memory.Address = address;
    cmd.Memory.Size = size;

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    return (resp.Status == OMBRA_STATUS_SUCCESS) ? OMBRA_CLIENT_SUCCESS : OMBRA_CLIENT_DRIVER_ERROR;
}

OMBRA_CLIENT_STATUS OmbraLockModule(U64 cr3, const char* moduleName) {
    if (!moduleName) {
        return OMBRA_CLIENT_INVALID_PARAM;
    }

    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_LOCK_MODULE;
    cmd.Memory.Cr3 = cr3;
    CopyString(cmd.Memory.ModuleName, moduleName, sizeof(cmd.Memory.ModuleName));

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    return (resp.Status == OMBRA_STATUS_SUCCESS) ? OMBRA_CLIENT_SUCCESS : OMBRA_CLIENT_DRIVER_ERROR;
}

OMBRA_CLIENT_STATUS OmbraUnlockModule(U64 cr3, const char* moduleName) {
    if (!moduleName) {
        return OMBRA_CLIENT_INVALID_PARAM;
    }

    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_UNLOCK_MODULE;
    cmd.Memory.Cr3 = cr3;
    CopyString(cmd.Memory.ModuleName, moduleName, sizeof(cmd.Memory.ModuleName));

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    return (resp.Status == OMBRA_STATUS_SUCCESS) ? OMBRA_CLIENT_SUCCESS : OMBRA_CLIENT_DRIVER_ERROR;
}

OMBRA_CLIENT_STATUS OmbraReadPhysical(U64 physicalAddress, void* buffer, U64 size) {
    if (!buffer || size == 0) {
        return OMBRA_CLIENT_INVALID_PARAM;
    }

    // For large reads, use scratch buffer
    if (size > 8) {
        U64 scratchOffset = 0;
        if (OmbraCopyToScratch(NULL, size, &scratchOffset) != OMBRA_CLIENT_SUCCESS) {
            return OMBRA_CLIENT_BUFFER_TOO_SMALL;
        }

        OMBRA_COMMAND cmd = {0};
        OMBRA_RESPONSE resp = {0};

        cmd.CommandId = OMBRA_CMD_READ_PHYSICAL;
        cmd.PhysicalMem.PhysicalAddress = physicalAddress;
        cmd.PhysicalMem.Size = size;
        cmd.PhysicalMem.ScratchOffset = scratchOffset;

        OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
        if (status != OMBRA_CLIENT_SUCCESS) {
            OmbraReleaseScratchBuffer(scratchOffset, size);
            return status;
        }

        if (resp.Status != OMBRA_STATUS_SUCCESS) {
            OmbraReleaseScratchBuffer(scratchOffset, size);
            return OMBRA_CLIENT_DRIVER_ERROR;
        }

        OmbraCopyFromScratch(scratchOffset, buffer, size);
        OmbraReleaseScratchBuffer(scratchOffset, size);
        return OMBRA_CLIENT_SUCCESS;
    }

    // Small read - result comes back in response
    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_READ_PHYSICAL;
    cmd.PhysicalMem.PhysicalAddress = physicalAddress;
    cmd.PhysicalMem.Size = size;

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    if (resp.Status != OMBRA_STATUS_SUCCESS) {
        return OMBRA_CLIENT_DRIVER_ERROR;
    }

    memcpy(buffer, resp.Raw, size);
    return OMBRA_CLIENT_SUCCESS;
}

OMBRA_CLIENT_STATUS OmbraWritePhysical(U64 physicalAddress, const void* buffer, U64 size) {
    if (!buffer || size == 0) {
        return OMBRA_CLIENT_INVALID_PARAM;
    }

    // For large writes, use scratch buffer
    if (size > 8) {
        U64 scratchOffset = 0;
        OMBRA_CLIENT_STATUS scratchStatus = OmbraCopyToScratch(buffer, size, &scratchOffset);
        if (scratchStatus != OMBRA_CLIENT_SUCCESS) {
            return scratchStatus;
        }

        OMBRA_COMMAND cmd = {0};
        OMBRA_RESPONSE resp = {0};

        cmd.CommandId = OMBRA_CMD_WRITE_PHYSICAL;
        cmd.PhysicalMem.PhysicalAddress = physicalAddress;
        cmd.PhysicalMem.Size = size;
        cmd.PhysicalMem.ScratchOffset = scratchOffset;

        OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
        OmbraReleaseScratchBuffer(scratchOffset, size);

        if (status != OMBRA_CLIENT_SUCCESS) {
            return status;
        }

        return (resp.Status == OMBRA_STATUS_SUCCESS) ? OMBRA_CLIENT_SUCCESS : OMBRA_CLIENT_DRIVER_ERROR;
    }

    // Small write - data fits in command
    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_WRITE_PHYSICAL;
    cmd.PhysicalMem.PhysicalAddress = physicalAddress;
    cmd.PhysicalMem.Size = size;
    memcpy(cmd.Raw, buffer, size);

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    return (resp.Status == OMBRA_STATUS_SUCCESS) ? OMBRA_CLIENT_SUCCESS : OMBRA_CLIENT_DRIVER_ERROR;
}

OMBRA_CLIENT_STATUS OmbraReadVirtual(U64 cr3, U64 virtualAddress, void* buffer, U64 size) {
    if (!buffer || size == 0) {
        return OMBRA_CLIENT_INVALID_PARAM;
    }

    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_READ_VIRTUAL;
    cmd.VirtualMem.Cr3 = cr3;
    cmd.VirtualMem.VirtualAddress = virtualAddress;
    cmd.VirtualMem.Size = size;

    // Use scratch buffer for large reads
    if (size > sizeof(resp.Raw)) {
        U64 scratchOffset = 0;
        if (OmbraCopyToScratch(NULL, size, &scratchOffset) != OMBRA_CLIENT_SUCCESS) {
            return OMBRA_CLIENT_BUFFER_TOO_SMALL;
        }
        cmd.VirtualMem.ScratchOffset = scratchOffset;

        OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
        if (status != OMBRA_CLIENT_SUCCESS) {
            OmbraReleaseScratchBuffer(scratchOffset, size);
            return status;
        }

        if (resp.Status != OMBRA_STATUS_SUCCESS) {
            OmbraReleaseScratchBuffer(scratchOffset, size);
            return OMBRA_CLIENT_DRIVER_ERROR;
        }

        OmbraCopyFromScratch(scratchOffset, buffer, size);
        OmbraReleaseScratchBuffer(scratchOffset, size);
        return OMBRA_CLIENT_SUCCESS;
    }

    // Small read
    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    if (resp.Status != OMBRA_STATUS_SUCCESS) {
        return OMBRA_CLIENT_DRIVER_ERROR;
    }

    memcpy(buffer, resp.Raw, size);
    return OMBRA_CLIENT_SUCCESS;
}

OMBRA_CLIENT_STATUS OmbraWriteVirtual(U64 cr3, U64 virtualAddress, const void* buffer, U64 size) {
    if (!buffer || size == 0) {
        return OMBRA_CLIENT_INVALID_PARAM;
    }

    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_WRITE_VIRTUAL;
    cmd.VirtualMem.Cr3 = cr3;
    cmd.VirtualMem.VirtualAddress = virtualAddress;
    cmd.VirtualMem.Size = size;

    // Use scratch buffer for large writes
    if (size > sizeof(cmd.Raw) - 32) {  // Leave room for fixed fields
        U64 scratchOffset = 0;
        OMBRA_CLIENT_STATUS scratchStatus = OmbraCopyToScratch(buffer, size, &scratchOffset);
        if (scratchStatus != OMBRA_CLIENT_SUCCESS) {
            return scratchStatus;
        }
        cmd.VirtualMem.ScratchOffset = scratchOffset;

        OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
        OmbraReleaseScratchBuffer(scratchOffset, size);

        if (status != OMBRA_CLIENT_SUCCESS) {
            return status;
        }

        return (resp.Status == OMBRA_STATUS_SUCCESS) ? OMBRA_CLIENT_SUCCESS : OMBRA_CLIENT_DRIVER_ERROR;
    }

    // Small write - embed in command
    // Copy data after the fixed VirtualMem fields
    // The Raw union overlaps, so we copy to an offset
    memcpy(&cmd.Raw[32], buffer, size);

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    return (resp.Status == OMBRA_STATUS_SUCCESS) ? OMBRA_CLIENT_SUCCESS : OMBRA_CLIENT_DRIVER_ERROR;
}

// =============================================================================
// Protection API
// =============================================================================

OMBRA_CLIENT_STATUS OmbraProtectProcess(U64 cr3, U32 method, U32 accessMask) {
    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_PROTECT_PROCESS;
    cmd.Protection.Cr3 = cr3;
    cmd.Protection.Method = method;
    cmd.Protection.AccessMask = accessMask;

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    return (resp.Status == OMBRA_STATUS_SUCCESS) ? OMBRA_CLIENT_SUCCESS : OMBRA_CLIENT_DRIVER_ERROR;
}

OMBRA_CLIENT_STATUS OmbraUnprotectProcess(U64 cr3) {
    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_UNPROTECT_PROCESS;
    cmd.Protection.Cr3 = cr3;

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    return (resp.Status == OMBRA_STATUS_SUCCESS) ? OMBRA_CLIENT_SUCCESS : OMBRA_CLIENT_DRIVER_ERROR;
}

OMBRA_CLIENT_STATUS OmbraBlockImage(U64 cr3, const char* imageName) {
    if (!imageName) {
        return OMBRA_CLIENT_INVALID_PARAM;
    }

    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_BLOCK_IMAGE;
    cmd.Memory.Cr3 = cr3;
    CopyString(cmd.Memory.ModuleName, imageName, sizeof(cmd.Memory.ModuleName));

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    return (resp.Status == OMBRA_STATUS_SUCCESS) ? OMBRA_CLIENT_SUCCESS : OMBRA_CLIENT_DRIVER_ERROR;
}

// =============================================================================
// Injection API
// =============================================================================

OMBRA_CLIENT_STATUS OmbraInjectDll(U64 pid, const char* dllPath, bool hidden) {
    if (!dllPath) {
        return OMBRA_CLIENT_INVALID_PARAM;
    }

    // Read DLL file and copy to scratch
    // This is a simplified version - real implementation would read file
    (void)pid;
    (void)dllPath;
    (void)hidden;

    return OMBRA_CLIENT_INTERNAL_ERROR;  // TODO: Implement
}

// =============================================================================
// Identity Map API
// =============================================================================

OMBRA_CLIENT_STATUS OmbraGetIdentityMap(U64 size, U64* outBase) {
    if (!outBase) {
        return OMBRA_CLIENT_INVALID_PARAM;
    }

    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_GET_IDENTITY_MAP;
    cmd.Memory.Size = size;

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    if (resp.Status != OMBRA_STATUS_SUCCESS) {
        return OMBRA_CLIENT_DRIVER_ERROR;
    }

    *outBase = resp.IdentityMap.IdentityBase;
    return OMBRA_CLIENT_SUCCESS;
}

OMBRA_CLIENT_STATUS OmbraReleaseIdentityMap(U64 base) {
    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_RELEASE_IDENTITY_MAP;
    cmd.Memory.Address = base;

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    return (resp.Status == OMBRA_STATUS_SUCCESS) ? OMBRA_CLIENT_SUCCESS : OMBRA_CLIENT_DRIVER_ERROR;
}

// =============================================================================
// Spoofing API
// =============================================================================

OMBRA_CLIENT_STATUS OmbraSetSpoof(OMBRA_SPOOF_TYPE spoofType, const void* original, const void* spoofed, U32 size) {
    if (size > 64) {
        return OMBRA_CLIENT_INVALID_PARAM;
    }

    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_SPOOF_DISK + (spoofType - 1);  // Adjust command based on type
    cmd.Spoof.SpoofType = spoofType;

    if (original) {
        memcpy(cmd.Spoof.OriginalValue, original, size);
    }
    if (spoofed) {
        memcpy(cmd.Spoof.SpoofedValue, spoofed, size);
    }

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    return (resp.Status == OMBRA_STATUS_SUCCESS) ? OMBRA_CLIENT_SUCCESS : OMBRA_CLIENT_DRIVER_ERROR;
}

OMBRA_CLIENT_STATUS OmbraQuerySpoof(OMBRA_SPOOF_TYPE spoofType, void* original, void* spoofed, U32 size) {
    if (size > 64) {
        return OMBRA_CLIENT_INVALID_PARAM;
    }

    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_SPOOF_QUERY;
    cmd.Spoof.SpoofType = spoofType;

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    if (resp.Status != OMBRA_STATUS_SUCCESS) {
        return OMBRA_CLIENT_DRIVER_ERROR;
    }

    // Copy results from response
    if (original) {
        memcpy(original, resp.Raw, size);
    }
    if (spoofed) {
        memcpy(spoofed, resp.Raw + 64, size);
    }

    return OMBRA_CLIENT_SUCCESS;
}

// =============================================================================
// ETW Control API
// =============================================================================

OMBRA_CLIENT_STATUS OmbraDisableTiEtw(void) {
    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_ETW_DISABLE_TI;

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    return (resp.Status == OMBRA_STATUS_SUCCESS) ? OMBRA_CLIENT_SUCCESS : OMBRA_CLIENT_DRIVER_ERROR;
}

OMBRA_CLIENT_STATUS OmbraWipeEtwBuffer(U64 bufferAddress) {
    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_ETW_WIPE_BUFFER;
    cmd.Etw.BufferAddress = bufferAddress;

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    return (resp.Status == OMBRA_STATUS_SUCCESS) ? OMBRA_CLIENT_SUCCESS : OMBRA_CLIENT_DRIVER_ERROR;
}

OMBRA_CLIENT_STATUS OmbraRestoreEtw(void) {
    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_ETW_RESTORE;

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    return (resp.Status == OMBRA_STATUS_SUCCESS) ? OMBRA_CLIENT_SUCCESS : OMBRA_CLIENT_DRIVER_ERROR;
}

// =============================================================================
// Diagnostics API
// =============================================================================

OMBRA_CLIENT_STATUS OmbraPing(void) {
    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_PING;

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 1000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    // Check for expected response
    if (resp.Status == OMBRA_STATUS_SUCCESS && resp.Ping.Response == 0x474E4F50) {  // "PONG"
        return OMBRA_CLIENT_SUCCESS;
    }

    return OMBRA_CLIENT_DRIVER_ERROR;
}

OMBRA_CLIENT_STATUS OmbraGetStatus(OMBRA_DRIVER_STATUS* status) {
    if (!status) {
        return OMBRA_CLIENT_INVALID_PARAM;
    }

    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_GET_STATUS;

    OMBRA_CLIENT_STATUS clientStatus = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (clientStatus != OMBRA_CLIENT_SUCCESS) {
        return clientStatus;
    }

    if (resp.Status != OMBRA_STATUS_SUCCESS) {
        return OMBRA_CLIENT_DRIVER_ERROR;
    }

    status->DriverVersion = resp.StatusInfo.DriverVersion;
    status->Uptime = resp.StatusInfo.Uptime;
    status->CommandsProcessed = resp.StatusInfo.CommandsProcessed;
    status->VmexitCount = resp.StatusInfo.VmexitCount;
    status->ActiveCpus = resp.StatusInfo.ActiveCpus;
    status->HooksInstalled = resp.StatusInfo.HooksInstalled;

    return OMBRA_CLIENT_SUCCESS;
}

OMBRA_CLIENT_STATUS OmbraGetScore(U32* score) {
    if (!score) {
        return OMBRA_CLIENT_INVALID_PARAM;
    }

    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_GET_SCORE;

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    if (resp.Status != OMBRA_STATUS_SUCCESS) {
        return OMBRA_CLIENT_DRIVER_ERROR;
    }

    *score = *(U32*)resp.Raw;
    return OMBRA_CLIENT_SUCCESS;
}

OMBRA_CLIENT_STATUS OmbraResetScore(void) {
    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_RESET_SCORE;

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    return (resp.Status == OMBRA_STATUS_SUCCESS) ? OMBRA_CLIENT_SUCCESS : OMBRA_CLIENT_DRIVER_ERROR;
}

OMBRA_CLIENT_STATUS OmbraConfigScore(U32 operation, U32 eventMask, U32 threshold) {
    OMBRA_COMMAND cmd = {0};
    OMBRA_RESPONSE resp = {0};

    cmd.CommandId = OMBRA_CMD_CONFIG_SCORE;
    cmd.Score.Operation = operation;
    cmd.Score.EventMask = eventMask;
    cmd.Score.Threshold = threshold;

    OMBRA_CLIENT_STATUS status = OmbraExecuteCommand(&cmd, 5000, &resp);
    if (status != OMBRA_CLIENT_SUCCESS) {
        return status;
    }

    return (resp.Status == OMBRA_STATUS_SUCCESS) ? OMBRA_CLIENT_SUCCESS : OMBRA_CLIENT_DRIVER_ERROR;
}

// =============================================================================
// Scratch Buffer Helpers
// =============================================================================

// Simple bump allocator for scratch buffer
static volatile U64 g_ScratchOffset = 0;

void* OmbraGetScratchBuffer(U64 size, U64* outOffset) {
    if (!g_OmbraClient.Initialized || !g_OmbraClient.ScratchBuffer) {
        return NULL;
    }

    // Align to 8 bytes
    size = (size + 7) & ~7ULL;

    // Check if fits
    U64 currentOffset = g_ScratchOffset;
    if (currentOffset + size > g_OmbraClient.ScratchSize) {
        return NULL;
    }

    // Simple bump allocation (not thread-safe - would need atomics for real use)
    g_ScratchOffset = currentOffset + size;

    if (outOffset) {
        *outOffset = currentOffset;
    }

    return (U8*)g_OmbraClient.ScratchBuffer + currentOffset;
}

void OmbraReleaseScratchBuffer(U64 offset, U64 size) {
    (void)offset;
    (void)size;
    // Simple allocator doesn't support free
    // Real implementation would use a proper allocator
}

OMBRA_CLIENT_STATUS OmbraCopyToScratch(const void* data, U64 size, U64* outOffset) {
    U64 offset = 0;
    void* scratch = OmbraGetScratchBuffer(size, &offset);
    if (!scratch) {
        return OMBRA_CLIENT_BUFFER_TOO_SMALL;
    }

    if (data) {
        memcpy(scratch, data, size);
    }

    if (outOffset) {
        *outOffset = offset;
    }

    return OMBRA_CLIENT_SUCCESS;
}

OMBRA_CLIENT_STATUS OmbraCopyFromScratch(U64 offset, void* buffer, U64 size) {
    if (!g_OmbraClient.Initialized || !g_OmbraClient.ScratchBuffer) {
        return OMBRA_CLIENT_NOT_INITIALIZED;
    }

    if (offset + size > g_OmbraClient.ScratchSize) {
        return OMBRA_CLIENT_INVALID_PARAM;
    }

    if (buffer) {
        memcpy(buffer, (U8*)g_OmbraClient.ScratchBuffer + offset, size);
    }

    return OMBRA_CLIENT_SUCCESS;
}
