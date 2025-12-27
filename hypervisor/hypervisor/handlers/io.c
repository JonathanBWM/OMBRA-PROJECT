// io.c — I/O Instruction Exit Handler
// OmbraHypervisor

#include "handlers.h"
#include "../debug.h"
#include "../timing.h"
#include "../vmx.h"
#include "../ept.h"
#include "../../shared/vmcs_fields.h"
#include <intrin.h>

// =============================================================================
// I/O Exit Qualification Bit Layout (Intel SDM Vol 3, Table 27-5)
// =============================================================================
// Bits 2:0   - Size of access (0=1-byte, 1=2-byte, 3=4-byte)
// Bit 3      - Direction (0=OUT, 1=IN)
// Bit 4      - String instruction (INS/OUTS)
// Bit 5      - REP prefix
// Bit 6      - Operand encoding (0=DX, 1=immediate)
// Bits 15:7  - Reserved
// Bits 31:16 - Port number

#define IO_SIZE_MASK        0x7
#define IO_SIZE_1_BYTE      0
#define IO_SIZE_2_BYTE      1
#define IO_SIZE_4_BYTE      3

#define IO_DIRECTION_BIT    (1 << 3)
#define IO_DIRECTION_OUT    0
#define IO_DIRECTION_IN     IO_DIRECTION_BIT

#define IO_STRING_BIT       (1 << 4)
#define IO_REP_BIT          (1 << 5)
#define IO_IMM_BIT          (1 << 6)

#define IO_PORT_SHIFT       16
#define IO_PORT_MASK        0xFFFF

// =============================================================================
// I/O Instruction Handler
// =============================================================================

VMEXIT_ACTION HandleIo(GUEST_REGS* r, U64 q) {
    U32 size = (U32)(q & IO_SIZE_MASK);
    bool isIn = (q & IO_DIRECTION_BIT) != 0;
    bool isString = (q & IO_STRING_BIT) != 0;
    bool hasRep = (q & IO_REP_BIT) != 0;
    U16 port = (U16)((q >> IO_PORT_SHIFT) & IO_PORT_MASK);
    VMX_CPU* cpu;
    U64 entryTsc;

    // TIMING COMPENSATION: Capture TSC at handler entry
    // Some anti-cheat may profile I/O instruction timing
    entryTsc = TimingStart();

    // Handle string I/O instructions (INS/OUTS)
    if (isString) {
        // String I/O uses RSI (OUT) or RDI (IN), with count in RCX if REP prefix
        // VMCS provides these values in EXIT_IO fields for string operations
        U64 rsi = VmcsRead(VMCS_EXIT_IO_RSI);
        U64 rdi = VmcsRead(VMCS_EXIT_IO_RDI);
        U64 rcx = VmcsRead(VMCS_EXIT_IO_RCX);
        U64 rflags = VmcsRead(VMCS_GUEST_RFLAGS);
        U64 guestCr3 = VmcsRead(VMCS_GUEST_CR3);
        bool directionUp = (rflags & (1 << 10)) == 0;  // DF flag: 0=up, 1=down
        U64 count = hasRep ? rcx : 1;
        U64 stride;

        // Calculate stride based on operand size
        switch (size) {
        case IO_SIZE_1_BYTE: stride = 1; break;
        case IO_SIZE_2_BYTE: stride = 2; break;
        case IO_SIZE_4_BYTE: stride = 4; break;
        default: stride = 1; break;
        }

        if (isIn) {
            // INS: port -> [RDI], increment/decrement RDI, decrement RCX
            // RDI is a guest linear address - must translate GVA->GPA before access
            for (U64 i = 0; i < count; i++) {
                U32 value = 0;

                // Read from port
                switch (size) {
                case IO_SIZE_1_BYTE:
                    value = __inbyte(port);
                    break;
                case IO_SIZE_2_BYTE:
                    value = __inword(port);
                    break;
                case IO_SIZE_4_BYTE:
                    value = __indword(port);
                    break;
                }

                // Translate guest virtual to physical
                // Handle page boundary crossing
                for (U64 byteIdx = 0; byteIdx < stride; byteIdx++) {
                    U64 gva = rdi + byteIdx;
                    U64 gpa = WalkGuestPageTables(gva, guestCr3);

                    if (gpa == 0 || gpa >= EPT_IDENTITY_MAP_SIZE) {
                        // Translation failed or out of range - inject #PF
                        // For now, skip this iteration (page fault injection TODO)
                        goto skip_ins_iteration;
                    }

                    // Write byte to guest physical memory
                    U8* physPtr = (U8*)gpa;
                    *physPtr = (U8)(value >> (byteIdx * 8));
                }

skip_ins_iteration:
                rdi = directionUp ? (rdi + stride) : (rdi - stride);

                if (hasRep && rcx > 0) {
                    rcx--;
                }
            }

            // Update guest state
            r->Rdi = rdi;
            if (hasRep) {
                r->Rcx = rcx;
            }
        } else {
            // OUTS: [RSI] -> port, increment/decrement RSI, decrement RCX
            for (U64 i = 0; i < count; i++) {
                U32 value = 0;

                // Translate guest virtual to physical and read value
                // Handle page boundary crossing
                for (U64 byteIdx = 0; byteIdx < stride; byteIdx++) {
                    U64 gva = rsi + byteIdx;
                    U64 gpa = WalkGuestPageTables(gva, guestCr3);

                    if (gpa == 0 || gpa >= EPT_IDENTITY_MAP_SIZE) {
                        // Translation failed or out of range - inject #PF
                        // For now, skip this iteration (page fault injection TODO)
                        goto skip_outs_iteration;
                    }

                    // Read byte from guest physical memory
                    U8* physPtr = (U8*)gpa;
                    value |= ((U32)(*physPtr)) << (byteIdx * 8);
                }

                // Write to port
                switch (size) {
                case IO_SIZE_1_BYTE:
                    __outbyte(port, (U8)value);
                    break;
                case IO_SIZE_2_BYTE:
                    __outword(port, (U16)value);
                    break;
                case IO_SIZE_4_BYTE:
                    __outdword(port, value);
                    break;
                }

skip_outs_iteration:
                rsi = directionUp ? (rsi + stride) : (rsi - stride);

                if (hasRep && rcx > 0) {
                    rcx--;
                }
            }

            // Update guest state
            r->Rsi = rsi;
            if (hasRep) {
                r->Rcx = rcx;
            }
        }
    } else {
        // Regular I/O instructions (IN/OUT)
        if (isIn) {
            // IN: port -> AL/AX/EAX (stored in RAX)
            U32 value = 0;

            switch (size) {
            case IO_SIZE_1_BYTE:
                value = __inbyte(port);
                r->Rax = (r->Rax & ~0xFFULL) | (value & 0xFF);
                break;
            case IO_SIZE_2_BYTE:
                value = __inword(port);
                r->Rax = (r->Rax & ~0xFFFFULL) | (value & 0xFFFF);
                break;
            case IO_SIZE_4_BYTE:
                value = __indword(port);
                r->Rax = (r->Rax & ~0xFFFFFFFFULL) | value;
                break;
            }
        } else {
            // OUT: AL/AX/EAX -> port (from RAX)
            switch (size) {
            case IO_SIZE_1_BYTE:
                __outbyte(port, (U8)(r->Rax & 0xFF));
                break;
            case IO_SIZE_2_BYTE:
                __outword(port, (U16)(r->Rax & 0xFFFF));
                break;
            case IO_SIZE_4_BYTE:
                __outdword(port, (U32)(r->Rax & 0xFFFFFFFF));
                break;
            }
        }
    }

    // TIMING COMPENSATION: Apply compensation before returning to guest
    cpu = VmxGetCurrentCpu();
    if (cpu) {
        TimingEnd(cpu, entryTsc, TIMING_IO_OVERHEAD);
    }

    return VMEXIT_ADVANCE_RIP;
}
