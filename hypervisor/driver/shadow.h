// shadow.h - EPT Shadow Page Management
// OmbraDriver Phase 3
//
// Provides shadow hook management for execute-only memory views

#ifndef OMBRA_SHADOW_H
#define OMBRA_SHADOW_H

#include "../shared/types.h"

// Maximum shadow hooks per driver instance
#define MAX_SHADOW_HOOKS 64

// Install shadow hook on memory region
// Creates execute-only view: execute sees hooked page, read/write sees clean page
I32 ShadowInstall(U64 cr3, U64 virtualAddress, U64 size);

// Remove shadow hook from memory region
I32 ShadowRemove(U64 cr3, U64 virtualAddress, U64 size);

// Remove all shadow hooks for a process
void ShadowRemoveAll(U64 cr3);

// Get count of active shadow hooks
U32 ShadowGetCount(void);

// Command handlers
I32 HandleHideMemory(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleShadowMemory(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);

#endif // OMBRA_SHADOW_H
