// exception.c — Exception/NMI Exit Handler
// OmbraHypervisor

#include "handlers.h"
#include "../debug.h"
#include "../timing.h"
#include "../vmx.h"
#include "../../shared/vmcs_fields.h"
#include <intrin.h>

// =============================================================================
// x86 Exception Vectors
// =============================================================================

#define EXCEPTION_DIVIDE_ERROR          0   // #DE
#define EXCEPTION_DEBUG                 1   // #DB
#define EXCEPTION_NMI                   2   // NMI (not an exception)
#define EXCEPTION_BREAKPOINT            3   // #BP
#define EXCEPTION_OVERFLOW              4   // #OF
#define EXCEPTION_BOUND_RANGE           5   // #BR
#define EXCEPTION_INVALID_OPCODE        6   // #UD
#define EXCEPTION_DEVICE_NOT_AVAILABLE  7   // #NM
#define EXCEPTION_DOUBLE_FAULT          8   // #DF
#define EXCEPTION_INVALID_TSS           10  // #TS
#define EXCEPTION_SEGMENT_NOT_PRESENT   11  // #NP
#define EXCEPTION_STACK_FAULT           12  // #SS
#define EXCEPTION_GENERAL_PROTECTION    13  // #GP
#define EXCEPTION_PAGE_FAULT            14  // #PF
#define EXCEPTION_FPU_ERROR             16  // #MF
#define EXCEPTION_ALIGNMENT_CHECK       17  // #AC
#define EXCEPTION_MACHINE_CHECK         18  // #MC
#define EXCEPTION_SIMD_ERROR            19  // #XM
#define EXCEPTION_VIRTUALIZATION        20  // #VE

// =============================================================================
// Interrupt/Exception Type (bits 10:8 of VM-exit/entry interrupt info)
// =============================================================================

#define INTERRUPT_TYPE_EXTERNAL         0   // External interrupt
#define INTERRUPT_TYPE_NMI              2   // NMI
#define INTERRUPT_TYPE_HARDWARE_EXCEPTION 3 // Hardware exception
#define INTERRUPT_TYPE_SOFTWARE_INT     4   // Software interrupt (INT n)
#define INTERRUPT_TYPE_PRIV_SW_EXCEPTION 5  // Privileged software exception (INT1)
#define INTERRUPT_TYPE_SOFTWARE_EXCEPTION 6 // Software exception (INT3, INTO)
#define INTERRUPT_TYPE_OTHER            7   // Other event

// =============================================================================
// VM-Exit Interrupt Information Field Format (Intel SDM Vol 3, 24.9.2)
// =============================================================================
// Bits 7:0   - Vector
// Bits 10:8  - Interrupt type
// Bit 11     - Error code valid
// Bit 12     - NMI unblocking due to IRET
// Bits 30:13 - Reserved
// Bit 31     - Valid

#define INT_INFO_VECTOR_MASK            0xFF
#define INT_INFO_TYPE_SHIFT             8
#define INT_INFO_TYPE_MASK              0x700
#define INT_INFO_ERROR_CODE_VALID       (1 << 11)
#define INT_INFO_NMI_UNBLOCK_IRET       (1 << 12)
#define INT_INFO_VALID                  (1UL << 31)

// =============================================================================
// Exception Handler
// =============================================================================
//
// Handles exceptions and NMIs that occur in the guest. Most exceptions should
// be re-injected to the guest's own exception handlers. NMIs require special
// handling to maintain proper blocking state.

VMEXIT_ACTION HandleException(GUEST_REGS* r) {
    U32 intInfo;
    U32 errorCode;
    U32 vector;
    U32 type;
    bool hasErrorCode;
    VMX_CPU* cpu;
    U64 entryTsc;
    U64 guestRip;

    (void)r;  // Guest registers not typically needed for exception handling

    // TIMING COMPENSATION: Capture TSC at handler entry
    // Exception exits can reveal hypervisor presence through timing
    entryTsc = TimingStart();

    // Read the VM-exit interruption information
    intInfo = (U32)VmcsRead(VMCS_EXIT_INT_INFO);

    // Validate the interruption information field
    if (!(intInfo & INT_INFO_VALID)) {
        // Invalid interruption info - should never happen
        ERR("Exception exit with invalid interruption info: 0x%x", intInfo);
        return VMEXIT_SHUTDOWN;
    }

    // Extract exception details
    vector = intInfo & INT_INFO_VECTOR_MASK;
    type = (intInfo & INT_INFO_TYPE_MASK) >> INT_INFO_TYPE_SHIFT;
    hasErrorCode = !!(intInfo & INT_INFO_ERROR_CODE_VALID);

    // Read error code if present
    errorCode = 0;
    if (hasErrorCode) {
        errorCode = (U32)VmcsRead(VMCS_EXIT_INT_ERROR_CODE);
    }

    // Get current CPU state for logging
    cpu = VmxGetCurrentCpu();
    guestRip = cpu ? cpu->GuestRip : VmcsRead(VMCS_GUEST_RIP);

    // Log the exception for debugging
    if (hasErrorCode) {
        TRACE("Exception exit: vector=%u type=%u error=0x%x RIP=0x%llx",
              vector, type, errorCode, guestRip);
    } else {
        TRACE("Exception exit: vector=%u type=%u RIP=0x%llx",
              vector, type, guestRip);
    }

    // =========================================================================
    // Exception-Specific Handling
    // =========================================================================

    switch (vector) {
    case EXCEPTION_NMI:
        // NMI (Non-Maskable Interrupt)
        // EAC uses NMIs for hypervisor detection - block for specific CR3s
        if (cpu && cpu->NmiBlockingEnabled) {
            U64 guestCr3 = VmcsRead(VMCS_GUEST_CR3) & ~0xFFFULL;  // Mask PCID bits
            for (U32 i = 0; i < cpu->NmiBlockedCount; i++) {
                if (cpu->NmiBlockedCr3[i] == guestCr3) {
                    // Block this NMI - queue it for later delivery
                    cpu->NmiQueuedCount++;
                    TRACE("NMI blocked for CR3=0x%llx (queued=%u)", guestCr3, cpu->NmiQueuedCount);
                    // Don't re-inject - we'll deliver later when process exits
                    if (cpu) TimingEnd(cpu, entryTsc, TIMING_GENERIC_OVERHEAD);
                    return VMEXIT_CONTINUE;
                }
            }
        }
        TRACE("NMI delivered to guest at RIP=0x%llx", guestRip);
        break;

    case EXCEPTION_DEBUG:
        // Debug exception (#DB)
        // Could be from hardware breakpoints, single-step, etc.
        // Re-inject to guest debugger
        TRACE("Debug exception (#DB) at RIP=0x%llx", guestRip);
        break;

    case EXCEPTION_BREAKPOINT:
        // Breakpoint (#BP from INT3)
        // Re-inject to guest debugger
        TRACE("Breakpoint (#BP) at RIP=0x%llx", guestRip);
        break;

    case EXCEPTION_INVALID_OPCODE:
        // Invalid opcode (#UD)
        // Could be from executing VMX instructions we're hiding
        // or genuinely invalid instructions. Re-inject to guest.
        WARN("Invalid opcode (#UD) at RIP=0x%llx - may be VMX instruction", guestRip);
        break;

    case EXCEPTION_GENERAL_PROTECTION:
        // General protection fault (#GP)
        // Could be from various causes - access violations, segment violations, etc.
        // Re-inject to guest with error code
        TRACE("General protection fault (#GP) error=0x%x at RIP=0x%llx",
              errorCode, guestRip);
        break;

    case EXCEPTION_PAGE_FAULT:
        // Page fault (#PF)
        // NOTE: Most page faults should be handled via EPT violations, not here.
        // This path is for page faults that occur due to guest paging issues,
        // not EPT issues. Re-inject with error code and faulting address.
        {
            U64 cr2 = VmcsRead(VMCS_EXIT_GUEST_LINEAR);
            TRACE("Page fault (#PF) error=0x%x addr=0x%llx RIP=0x%llx",
                  errorCode, cr2, guestRip);
            // Guest CR2 will be set automatically by hardware during injection
        }
        break;

    case EXCEPTION_DOUBLE_FAULT:
        // Double fault (#DF) - two exceptions collided
        // This is serious - the guest is in trouble
        WARN("Double fault (#DF) at RIP=0x%llx", guestRip);
        break;

    case EXCEPTION_MACHINE_CHECK:
        // Machine check exception (#MC) - hardware error
        // This is critical - could indicate real hardware failure
        ERR("Machine check exception (#MC) at RIP=0x%llx", guestRip);
        break;

    default:
        // Other exceptions - just re-inject
        TRACE("Exception vector=%u at RIP=0x%llx", vector, guestRip);
        break;
    }

    // =========================================================================
    // Re-inject Exception to Guest
    // =========================================================================
    //
    // VM-entry interruption-information field format (Intel SDM Vol 3, 24.8.3):
    // Bits 7:0   = Vector
    // Bits 10:8  = Interrupt type
    // Bit 11     = Deliver error code
    // Bits 30:12 = Reserved (0)
    // Bit 31     = Valid

    VmcsWrite(VMCS_CTRL_VMENTRY_INT_INFO, intInfo);

    // Set error code if exception delivers one
    if (hasErrorCode) {
        VmcsWrite(VMCS_CTRL_VMENTRY_EXCEPTION_CODE, errorCode);
    }

    // Set instruction length for software exceptions
    // Hardware exceptions don't need instruction length
    if (type == INTERRUPT_TYPE_SOFTWARE_EXCEPTION ||
        type == INTERRUPT_TYPE_SOFTWARE_INT ||
        type == INTERRUPT_TYPE_PRIV_SW_EXCEPTION) {
        U32 instrLen = (U32)VmcsRead(VMCS_EXIT_INSTR_LEN);
        VmcsWrite(VMCS_CTRL_VMENTRY_INSTR_LEN, instrLen);
    }

    // TIMING COMPENSATION: Apply compensation before re-entry
    // Exception handling should be fast to avoid detection
    cpu = VmxGetCurrentCpu();
    if (cpu) {
        TimingEnd(cpu, entryTsc, TIMING_GENERIC_OVERHEAD);
    }

    // Return without advancing RIP - the exception injection will handle
    // instruction pointer updates according to the exception type
    return VMEXIT_CONTINUE;
}
