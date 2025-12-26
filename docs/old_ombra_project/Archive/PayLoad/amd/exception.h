// PayLoad/amd/exception.h
// AMD SVM Exception Handling for VMXRoot Context
// Provides SEH-like exception handling for hypervisor code
#pragma once

// Include types.h FIRST - it brings in Windows.h which defines PE structures.
// OmbraSELib headers check for _WINNT_ to avoid redefinition.
#include "../include/types.h"

#include <OmbraSELib/IDT.h>
#include <Arch/Segmentation.h>
#include <OmbraSELib/pe.h>
#include <OmbraSELib/vmxroot/cpu.h>

#include "svm_handler.h"

//===----------------------------------------------------------------------===//
// PE Exception Directory Constants and Structures
//===----------------------------------------------------------------------===//

#ifndef IMAGE_DIRECTORY_ENTRY_EXCEPTION
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION 3
#endif

#ifndef UNW_FLAG_EHANDLER
#define UNW_FLAG_EHANDLER 0x1
#endif

typedef struct _RUNTIME_FUNCTION {
    ULONG BeginAddress;
    ULONG EndAddress;
    ULONG UnwindData;
} RUNTIME_FUNCTION, *PRUNTIME_FUNCTION;

typedef union _UNWIND_CODE {
    UINT8 CodeOffset;
    UINT8 UnwindOp : 4;
    UINT8 OpInfo : 4;
    UINT16 FrameOffset;
} UNWIND_CODE;

typedef struct _UNWIND_INFO {
    UINT8 Version : 3;
    UINT8 Flags : 5;
    UINT8 SizeOfProlog;
    UINT8 CountOfCodes;
    UINT8 FrameRegister : 4;
    UINT8 FrameOffset : 4;
    UNWIND_CODE UnwindCode[1];

    union {
        UINT32 ExceptionHandler;
        UINT32 FunctionEntry;
    };

    UINT32 ExceptionData[1];
} UNWIND_INFO;

typedef struct _SCOPE_RECORD {
    ULONG BeginAddress;
    ULONG EndAddress;
    ULONG HandlerAddress;
    ULONG JumpTarget;
} SCOPE_RECORD, *PSCOPE_RECORD;

typedef struct _SCOPE_TABLE {
    ULONG Count;
    SCOPE_RECORD ScopeRecord[1];
} SCOPE_TABLE, *PSCOPE_TABLE;

namespace exception {

//===----------------------------------------------------------------------===//
// Host IDT and Register
//===----------------------------------------------------------------------===//

extern IDT HostIdt;
extern Seg::DescriptorTableRegister<Seg::Mode::longMode> IdtReg;

//===----------------------------------------------------------------------===//
// Exception Handlers
//===----------------------------------------------------------------------===//

// SEH-style handlers for exceptions with and without error codes
extern "C" void seh_handler_ecode_vm(PIDT_REGS_ECODE regs);
extern "C" void seh_handler_vm(PIDT_REGS regs);

//===----------------------------------------------------------------------===//
// Parameter Saving for Recovery
//===----------------------------------------------------------------------===//

// When an unhandled exception occurs, use these params to call the original Hyper-V handler
void SaveOrigParams(
    void* unknown,
    void* unknown2,
    amd::pguest_context context,
    Seg::DescriptorTableRegister<Seg::Mode::longMode> idt,
    void* rsp
);

//===----------------------------------------------------------------------===//
// Initialization
//===----------------------------------------------------------------------===//

// Set up the host IDT with our exception handlers
void Initialize();

} // namespace exception
