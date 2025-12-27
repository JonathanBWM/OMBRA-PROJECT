// memory_ops.h - Physical and Virtual Memory Operations
// OmbraDriver Phase 3
//
// Provides memory read/write primitives via hypervisor VMCALLs

#ifndef OMBRA_MEMORY_OPS_H
#define OMBRA_MEMORY_OPS_H

#include "../shared/types.h"

// Physical memory command handlers
I32 HandleReadPhysical(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleWritePhysical(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);

// Virtual memory command handlers
I32 HandleReadVirtual(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleWriteVirtual(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);

// Identity map command handlers
I32 HandleGetIdentityMap(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleReleaseIdentityMap(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);

#endif // OMBRA_MEMORY_OPS_H
