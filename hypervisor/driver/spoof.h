// spoof.h - Hardware Identity Spoofing
// OmbraDriver Phase 3
//
// Provides hardware serial number spoofing for anti-detection

#ifndef OMBRA_SPOOF_H
#define OMBRA_SPOOF_H

#include "../shared/types.h"

// Spoof types
#define SPOOF_TYPE_DISK_SERIAL    1
#define SPOOF_TYPE_NIC_MAC        2
#define SPOOF_TYPE_VOLUME_SERIAL  3
#define SPOOF_TYPE_SMBIOS         4
#define SPOOF_TYPE_GPU_ID         5
#define SPOOF_TYPE_CPU_ID         6

// Maximum value size for spoofed data
#define SPOOF_VALUE_SIZE 64

// Initialize spoof system
void SpoofInit(void);

// Add spoof entry
I32 SpoofAdd(U32 type, const void* originalValue, const void* spoofedValue, U32 size);

// Remove spoof entry
I32 SpoofRemove(U32 type, const void* originalValue, U32 size);

// Apply spoofing to buffer (called from hooks)
bool SpoofApply(U32 type, void* buffer, U32 size);

// Query current spoof configuration
I32 SpoofQuery(U32 type, void* originalOut, void* spoofedOut, U32 size);

// Get total spoof hit count
U64 SpoofGetTotalHits(void);

// Command handlers
I32 HandleSpoofDisk(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleSpoofNic(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleSpoofVolume(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleSpoofQuery(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);

#endif // OMBRA_SPOOF_H
