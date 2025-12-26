#include "handlers.h"
VMEXIT_ACTION HandleIo(GUEST_REGS* r, U64 q) {
    (void)r; (void)q;
    return VMEXIT_ADVANCE_RIP;
}
