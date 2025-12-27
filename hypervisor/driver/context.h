#ifndef OMBRA_DRIVER_CONTEXT_H
#define OMBRA_DRIVER_CONTEXT_H

#include "../shared/types.h"
#include "vmcall.h"

// Maximum tracked subscriptions
#define OMBRA_MAX_SUBSCRIPTIONS 32

// Forward declarations
typedef struct _OMBRA_SUBSCRIPTION OMBRA_SUBSCRIPTION;

// Driver global context
typedef struct _OMBRA_DRIVER_CTX {
    // Initialization state
    bool                    Initialized;
    U64                     OwnerCr3;           // Loader's CR3

    // Command ring
    POMBRA_COMMAND_RING     Ring;
    void*                   ScratchBuffer;
    U64                     ScratchSize;

    // VMCALL credentials
    U64                     VmcallMagic;
    U64                     VmcallKey;

    // Kernel imports
    OMBRA_KERNEL_IMPORTS    Imports;

    // EPROCESS offsets
    EPROCESS_OFFSETS_EX     Offsets;

    // Worker thread
    void*                   WorkerThread;
    volatile bool           ShutdownRequested;

    // Subscriptions
    U32                     SubscriptionCount;
    OMBRA_SUBSCRIPTION*     Subscriptions[OMBRA_MAX_SUBSCRIPTIONS];

    // Statistics
    volatile U64            CommandsProcessed;
    volatile U64            VmexitCount;
    U64                     StartTime;

} OMBRA_DRIVER_CTX, *POMBRA_DRIVER_CTX;

// Global context
extern OMBRA_DRIVER_CTX g_DriverCtx;

#endif
