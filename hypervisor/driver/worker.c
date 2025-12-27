// worker.c - Command Ring Worker Thread
// OmbraDriver Phase 3

#include "context.h"
#include "command_ring.h"
#include <intrin.h>

// Forward declare dispatch function
void DispatchCommand(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);

// Worker thread entry point
// This thread polls the command ring and dispatches commands to handlers
// Runs in PASSIVE_LEVEL, sleeps when ring is empty
void OmbraWorkerThread(void* context) {
    POMBRA_DRIVER_CTX ctx = (POMBRA_DRIVER_CTX)context;

    // Delay function from imports
    typedef long (*KeDelayExecutionThreadFn)(char, char, void*);
    KeDelayExecutionThreadFn delayFn = (KeDelayExecutionThreadFn)ctx->Imports.KeDelayExecutionThread;

    // 1ms delay in 100ns units (negative = relative time)
    I64 delay = -10000LL;

    while (!ctx->ShutdownRequested) {
        U32 slot = 0;
        POMBRA_COMMAND cmd = CmdRingPeekCommand(ctx->Ring, &slot);

        if (cmd) {
            // Get corresponding response slot
            POMBRA_RESPONSE resp = CmdRingGetResponse(ctx->Ring, slot);

            // Clear response
            resp->Ready = 0;
            resp->Status = 0;
            resp->SequenceId = cmd->SequenceId;

            // Memory barrier before dispatch
            _mm_mfence();

            // Dispatch command
            DispatchCommand(cmd, resp);

            // Consume command (advance consumer index)
            CmdRingConsume(ctx->Ring);

            // Update stats
            ctx->CommandsProcessed++;
        } else {
            // Ring empty - sleep briefly
            if (delayFn) {
                delayFn(0, 0, &delay);
            } else {
                // Fallback: pause instructions (busy wait)
                for (int i = 0; i < 100; i++) {
                    _mm_pause();
                }
            }
        }
    }

    // Exit thread
    typedef long (*PsTerminateSystemThreadFn)(long);
    PsTerminateSystemThreadFn terminateFn = (PsTerminateSystemThreadFn)ctx->Imports.PsTerminateSystemThread;
    if (terminateFn) {
        terminateFn(0);  // STATUS_SUCCESS
    }
}
