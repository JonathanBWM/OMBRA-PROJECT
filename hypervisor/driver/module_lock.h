// module_lock.h - Module IAT Protection
// OmbraDriver Phase 3
//
// Locks module Import Address Tables to prevent tampering

#ifndef OMBRA_MODULE_LOCK_H
#define OMBRA_MODULE_LOCK_H

#include "../shared/types.h"

// Maximum locked modules per process
#define MAX_LOCKED_MODULES 16

// Locked module tracking
typedef struct _LOCKED_MODULE {
    bool    Active;
    U64     Cr3;                // Process CR3
    char    ModuleName[64];     // Module name
    U64     ImageBase;          // Module base address
    U64     IatAddress;         // IAT address
    U64     IatSize;            // IAT size
    U64     IatHash;            // Hash of original IAT content
} LOCKED_MODULE, *PLOCKED_MODULE;

// Initialize module lock system
void ModuleLockInit(void);

// Lock a module's IAT
I32 ModuleLockAdd(U64 cr3, const char* moduleName);

// Unlock a module
I32 ModuleLockRemove(U64 cr3, const char* moduleName);

// Verify all locked modules (check for tampering)
// Returns number of modules with detected tampering
U32 ModuleLockVerifyAll(void);

// Get locked module count
U32 ModuleLockGetCount(void);

// Remove all locks for a process (called on process termination)
void ModuleLockRemoveAll(U64 cr3);

// Command handlers
I32 HandleLockModule(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);
I32 HandleUnlockModule(POMBRA_COMMAND cmd, POMBRA_RESPONSE resp);

#endif // OMBRA_MODULE_LOCK_H
