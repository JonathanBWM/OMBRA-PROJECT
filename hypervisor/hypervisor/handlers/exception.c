#include "handlers.h"
VMEXIT_ACTION HandleException(GUEST_REGS* r) {
    (void)r;
    return VMEXIT_CONTINUE;
}
