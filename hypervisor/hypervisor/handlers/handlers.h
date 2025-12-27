#ifndef OMBRA_HANDLERS_H
#define OMBRA_HANDLERS_H
#include "../exit_dispatch.h"

// Basic instruction handlers
VMEXIT_ACTION HandleCpuid(GUEST_REGS* r);
VMEXIT_ACTION HandleRdtsc(GUEST_REGS* r);
VMEXIT_ACTION HandleRdtscp(GUEST_REGS* r);
VMEXIT_ACTION HandleRdmsr(GUEST_REGS* r);
VMEXIT_ACTION HandleWrmsr(GUEST_REGS* r);

// Control register and I/O handlers
VMEXIT_ACTION HandleCrAccess(GUEST_REGS* r, U64 q);
VMEXIT_ACTION HandleIo(GUEST_REGS* r, U64 q);

// EPT handlers
VMEXIT_ACTION HandleEptViolation(GUEST_REGS* r, U64 q);
VMEXIT_ACTION HandleEptMisconfiguration(GUEST_REGS* r, U64 q);

// Power management handlers
VMEXIT_ACTION HandleMonitor(GUEST_REGS* r);
VMEXIT_ACTION HandleMwait(GUEST_REGS* r);
VMEXIT_ACTION HandlePause(GUEST_REGS* r);

// Exception and hypercall handlers
VMEXIT_ACTION HandleVmcall(GUEST_REGS* r);
VMEXIT_ACTION HandleException(GUEST_REGS* r);

// MTF handler for shadow hooks
VMEXIT_ACTION HandleMtf(GUEST_REGS* r);

// Helper functions
U64 WalkGuestPageTables(U64 guestVirtual, U64 guestCr3);

#endif
