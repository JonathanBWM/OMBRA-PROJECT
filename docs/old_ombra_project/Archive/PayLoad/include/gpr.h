// PayLoad/include/gpr.h
// Guest General Purpose Register abstraction
#pragma once

#include "types.h"

//===----------------------------------------------------------------------===//
// Guest General Purpose Registers (Intel-style layout)
//===----------------------------------------------------------------------===//

struct GuestRegisters {
    u64 rax;
    u64 rcx;
    u64 rdx;
    u64 rbx;
    u64 rsp;
    u64 rbp;
    u64 rsi;
    u64 rdi;
    u64 r8;
    u64 r9;
    u64 r10;
    u64 r11;
    u64 r12;
    u64 r13;
    u64 r14;
    u64 r15;
    u64 rflags;
};

//===----------------------------------------------------------------------===//
// Accessor Macros
//===----------------------------------------------------------------------===//

#define GPR_RAX(ctx) ((ctx)->rax)
#define GPR_RCX(ctx) ((ctx)->rcx)
#define GPR_RDX(ctx) ((ctx)->rdx)
#define GPR_RBX(ctx) ((ctx)->rbx)
#define GPR_RSP(ctx) ((ctx)->rsp)
#define GPR_RBP(ctx) ((ctx)->rbp)
#define GPR_RSI(ctx) ((ctx)->rsi)
#define GPR_RDI(ctx) ((ctx)->rdi)
#define GPR_R8(ctx)  ((ctx)->r8)
#define GPR_R9(ctx)  ((ctx)->r9)
#define GPR_R10(ctx) ((ctx)->r10)
#define GPR_R11(ctx) ((ctx)->r11)
#define GPR_R12(ctx) ((ctx)->r12)
#define GPR_R13(ctx) ((ctx)->r13)
#define GPR_R14(ctx) ((ctx)->r14)
#define GPR_R15(ctx) ((ctx)->r15)
#define GPR_RFLAGS(ctx) ((ctx)->rflags)
