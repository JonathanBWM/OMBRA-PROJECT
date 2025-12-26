#ifndef OMBRA_HANDLERS_H
#define OMBRA_HANDLERS_H
#include "../exit_dispatch.h"
VMEXIT_ACTION HandleCpuid(GUEST_REGS* r);
VMEXIT_ACTION HandleRdtsc(GUEST_REGS* r);
VMEXIT_ACTION HandleRdtscp(GUEST_REGS* r);
VMEXIT_ACTION HandleRdmsr(GUEST_REGS* r);
VMEXIT_ACTION HandleWrmsr(GUEST_REGS* r);
VMEXIT_ACTION HandleCrAccess(GUEST_REGS* r, U64 q);
VMEXIT_ACTION HandleEptViolation(GUEST_REGS* r, U64 q);
VMEXIT_ACTION HandleVmcall(GUEST_REGS* r);
VMEXIT_ACTION HandleException(GUEST_REGS* r);
VMEXIT_ACTION HandleIo(GUEST_REGS* r, U64 q);
#endif
