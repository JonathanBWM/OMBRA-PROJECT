// command_ring.c - Command Ring Infrastructure
// OmbraDriver Phase 3

#include "command_ring.h"

// Memory barrier for proper ordering on x64
#ifdef _MSC_VER
#include <intrin.h>
#define MemoryBarrier() _mm_mfence()
#else
#define MemoryBarrier() __sync_synchronize()
#endif

// =============================================================================
// Ring Initialization
// =============================================================================

void CmdRingInit(POMBRA_COMMAND_RING ring, U64 scratchOffset, U64 scratchSize) {
    // Clear header
    volatile U8* p = (volatile U8*)ring;
    for (U32 i = 0; i < OMBRA_RING_HEADER_SIZE; i++) {
        p[i] = 0;
    }

    // Set configuration
    ring->RingSize = OMBRA_RING_SIZE;
    ring->CommandSize = OMBRA_COMMAND_SIZE;
    ring->ResponseSize = OMBRA_RESPONSE_SIZE;
    ring->Magic = OMBRA_RING_MAGIC;
    ring->ScratchBufferOffset = scratchOffset;
    ring->ScratchBufferSize = scratchSize;

    // Initialize indices
    ring->ProducerIndex = 0;
    ring->ConsumerIndex = 0;

    MemoryBarrier();
}

// =============================================================================
// Ring Status Queries
// =============================================================================

U32 CmdRingPendingCount(POMBRA_COMMAND_RING ring) {
    U32 producer = ring->ProducerIndex;
    MemoryBarrier();
    U32 consumer = ring->ConsumerIndex;

    return producer - consumer;
}

bool CmdRingIsEmpty(POMBRA_COMMAND_RING ring) {
    return CmdRingPendingCount(ring) == 0;
}

bool CmdRingIsFull(POMBRA_COMMAND_RING ring) {
    return CmdRingPendingCount(ring) >= ring->RingSize;
}

// =============================================================================
// Command Access
// =============================================================================

POMBRA_COMMAND CmdRingPeekCommand(POMBRA_COMMAND_RING ring, U32* outSlot) {
    U32 producer = ring->ProducerIndex;
    MemoryBarrier();
    U32 consumer = ring->ConsumerIndex;

    if (consumer == producer) {
        return NULL;  // Ring is empty
    }

    U32 slot = consumer % ring->RingSize;
    if (outSlot) {
        *outSlot = slot;
    }

    return CmdRingGetCommand(ring, slot);
}

void CmdRingConsume(POMBRA_COMMAND_RING ring) {
    MemoryBarrier();
    ring->ConsumerIndex++;
    MemoryBarrier();
}

// =============================================================================
// Response Completion
// =============================================================================

void CmdRingCompleteResponse(POMBRA_RESPONSE resp, I32 status) {
    resp->Status = status;
    MemoryBarrier();
    resp->Ready = 1;  // Must be last
    MemoryBarrier();
}
