// command_ring.h - Command Ring Infrastructure
// OmbraDriver Phase 3

#ifndef OMBRA_COMMAND_RING_H
#define OMBRA_COMMAND_RING_H

#include "../shared/types.h"

// Ring buffer helper macros
#define RING_COMMANDS_OFFSET    OMBRA_RING_HEADER_SIZE
#define RING_RESPONSES_OFFSET   (OMBRA_RING_HEADER_SIZE + (OMBRA_RING_SIZE * OMBRA_COMMAND_SIZE))

// Get command at slot index
static inline POMBRA_COMMAND CmdRingGetCommand(POMBRA_COMMAND_RING ring, U32 slot) {
    U8* base = (U8*)ring;
    return (POMBRA_COMMAND)(base + RING_COMMANDS_OFFSET + (slot * OMBRA_COMMAND_SIZE));
}

// Get response at slot index
static inline POMBRA_RESPONSE CmdRingGetResponse(POMBRA_COMMAND_RING ring, U32 slot) {
    U8* base = (U8*)ring;
    return (POMBRA_RESPONSE)(base + RING_RESPONSES_OFFSET + (slot * OMBRA_RESPONSE_SIZE));
}

// Initialize command ring header
void CmdRingInit(POMBRA_COMMAND_RING ring, U64 scratchOffset, U64 scratchSize);

// Get number of pending commands
U32 CmdRingPendingCount(POMBRA_COMMAND_RING ring);

// Check if ring is empty
bool CmdRingIsEmpty(POMBRA_COMMAND_RING ring);

// Check if ring is full
bool CmdRingIsFull(POMBRA_COMMAND_RING ring);

// Get next pending command (returns NULL if empty)
POMBRA_COMMAND CmdRingPeekCommand(POMBRA_COMMAND_RING ring, U32* outSlot);

// Mark one command as consumed (advance consumer index)
void CmdRingConsume(POMBRA_COMMAND_RING ring);

// Mark response as ready
void CmdRingCompleteResponse(POMBRA_RESPONSE resp, I32 status);

#endif // OMBRA_COMMAND_RING_H
