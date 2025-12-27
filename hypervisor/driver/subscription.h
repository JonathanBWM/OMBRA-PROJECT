#ifndef OMBRA_SUBSCRIPTION_H
#define OMBRA_SUBSCRIPTION_H

#include "../shared/types.h"

// Subscription state
typedef struct _OMBRA_SUBSCRIPTION {
    bool        Active;
    char        ImageName[64];      // Process name to track
    U64         Cr3;                // CR3 when found
    U64         Pid;                // Process ID
    U64         Eprocess;           // EPROCESS pointer
    U64         Peb;                // PEB address
    U64         ImageBase;          // Main module base
    bool        Dead;               // Process terminated
    bool        DllInjected;        // DLL injection completed
    U64         LastScanTime;       // TSC of last scan
} OMBRA_SUBSCRIPTION, *POMBRA_SUBSCRIPTION;

// Initialize subscription system
void SubscriptionInit(void);

// Subscribe to a process by name
// Returns subscription pointer or NULL on error
POMBRA_SUBSCRIPTION SubscriptionAdd(const char* imageName);

// Remove subscription
bool SubscriptionRemove(const char* imageName);

// Find subscription by name
POMBRA_SUBSCRIPTION SubscriptionFind(const char* imageName);

// Find subscription by CR3
POMBRA_SUBSCRIPTION SubscriptionFindByCr3(U64 cr3);

// Scan for subscribed processes (called periodically)
void SubscriptionScan(void);

// Check if a process matches any subscription
// Called from CR3 watch callback
bool SubscriptionCheckCr3(U64 cr3, U64 eprocess);

#endif
