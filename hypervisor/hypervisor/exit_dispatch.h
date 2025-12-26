#ifndef OMBRA_EXIT_DISPATCH_H
#define OMBRA_EXIT_DISPATCH_H
#include "../shared/types.h"

typedef struct {
    U64 Rax, Rcx, Rdx, Rbx, Rsp, Rbp, Rsi, Rdi;
    U64 R8, R9, R10, R11, R12, R13, R14, R15;
} GUEST_REGS;

typedef enum { VMEXIT_CONTINUE, VMEXIT_ADVANCE_RIP, VMEXIT_SHUTDOWN } VMEXIT_ACTION;

VMEXIT_ACTION VmexitDispatch(GUEST_REGS* regs);
#endif
