// protection.h - Process Protection
// OmbraDriver Phase 3
//
// Protects processes from external access and blocks image loading

#ifndef OMBRA_PROTECTION_H
#define OMBRA_PROTECTION_H

#include "../shared/types.h"

// Protection method flags
#define PROTECT_HANDLE_ACCESS   0x0001  // Block handle operations
#define PROTECT_MEMORY_ACCESS   0x0002  // Block memory read/write
#define PROTECT_THREAD_ACCESS   0x0004  // Block thread operations
#define PROTECT_TOKEN_ACCESS    0x0008  // Block token access
#define PROTECT_ALL             0xFFFF

// Access mask flags
#define ACCESS_READ             0x0001
#define ACCESS_WRITE            0x0002
#define ACCESS_EXECUTE          0x0004
#define ACCESS_ALL              0xFFFF

// Add process protection
I32 ProtectionAdd(U64 cr3, U32 method, U32 accessMask);

// Remove process protection
I32 ProtectionRemove(U64 cr3);

// Check if access should be blocked
bool ProtectionShouldBlock(U64 targetCr3, U64 accessorCr3, U32 accessType);

// Add image block entry
I32 ImageBlockAdd(U64 cr3, const char* imageName);

// Remove image block entry
I32 ImageBlockRemove(U64 cr3, const char* imageName);

// Check if image should be blocked
bool ImageShouldBlock(U64 cr3, const char* imageName);

// Command handlers
I32 HandleProtectProcess(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleUnprotectProcess(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleBlockImage(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleUnblockImage(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);

#endif // OMBRA_PROTECTION_H
