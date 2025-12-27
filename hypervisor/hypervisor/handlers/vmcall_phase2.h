// vmcall_phase2.h — Phase 2 VMCALL Handlers (Driver Mapper)
// OmbraHypervisor

#ifndef VMCALL_PHASE2_H
#define VMCALL_PHASE2_H

#include "../../shared/types.h"

// Initialize Phase 2 pool tracking with pre-allocated memory region
void Phase2_InitPool(U64 poolBase, U64 poolSize);

// Main dispatch for Phase 2 VMCALLs (0x3000-0x3FFF range)
// Returns OMBRA_ERROR_INVALID_OPERATION if vmcallId not in Phase 2 range
OMBRA_STATUS Phase2_HandleVmcall(U64 vmcallId, void* inputBuf, void* outputBuf);

#endif // VMCALL_PHASE2_H
