#pragma once
// OmbraSELib Standalone IDT Implementation for VMXRoot Context
// No WDK dependencies

#include "types.h"
#include "cpu.h"

// Forward declarations for interrupt handlers (defined in PayLoad assembly)
extern "C" void generic_interrupt_handler_vm();
extern "C" void generic_interrupt_handler_ecode_vm();

// RFLAGS structure
union RFLAGS {
    u64 Flags;
    struct {
        u64 CF : 1;
        u64 Reserved1 : 1;
        u64 PF : 1;
        u64 Reserved2 : 1;
        u64 AF : 1;
        u64 Reserved3 : 1;
        u64 ZF : 1;
        u64 SF : 1;
        u64 TF : 1;
        u64 IF : 1;
        u64 DF : 1;
        u64 OF : 1;
        u64 IOPL : 2;
        u64 NT : 1;
        u64 Reserved4 : 1;
        u64 RF : 1;
        u64 VM : 1;
        u64 AC : 1;
        u64 VIF : 1;
        u64 VIP : 1;
        u64 ID : 1;
        u64 Reserved5 : 42;
    };
};

// IDT register context when error code is pushed
typedef struct _IDT_REGS_ECODE {
    u64 r15;
    u64 r14;
    u64 r13;
    u64 r12;
    u64 r11;
    u64 r10;
    u64 r9;
    u64 r8;
    u64 rbp;
    u64 rdi;
    u64 rsi;
    u64 rdx;
    u64 rcx;
    u64 rbx;
    u64 rax;

    u64 error_code;
    u64 rip;
    u64 cs_selector;
    RFLAGS rflags;
    u64 rsp;
    u64 ss_selector;
} IDT_REGS_ECODE, *PIDT_REGS_ECODE;

// IDT register context without error code
typedef struct _IDT_REGS {
    u64 r15;
    u64 r14;
    u64 r13;
    u64 r12;
    u64 r11;
    u64 r10;
    u64 r9;
    u64 r8;
    u64 rbp;
    u64 rdi;
    u64 rsi;
    u64 rdx;
    u64 rcx;
    u64 rbx;
    u64 rax;

    u64 rip;
    u64 cs_selector;
    RFLAGS rflags;
    u64 rsp;
    u64 ss_selector;
} IDT_REGS, *PIDT_REGS;

// IDT Gate Descriptor for 64-bit mode (Vol3 Ch 6.14.1)
union IDTGateDescriptor64 {
    struct {
        u32 offset0_15 : 16;
        u32 cs_selector : 16;
        u32 ist : 3;
        u32 must_be_zero1 : 5;
        u32 type : 5;
        u32 dpl : 2;
        u32 present : 1;
        u32 offset16_31 : 16;
        u32 offset32_63 : 32;
        u32 reserved : 32;
    } bits;
    u32 values[4] = {};

    constexpr IDTGateDescriptor64() = default;

    __forceinline void setup(void* handler, bool present, u32 selector, u32 type) {
        u64 addr = reinterpret_cast<u64>(handler);
        bits.offset0_15 = addr & 0xFFFF;
        bits.offset16_31 = (addr >> 16) & 0xFFFF;
        bits.offset32_63 = (addr >> 32) & 0xFFFFFFFF;
        bits.cs_selector = selector;
        bits.type = type;
        bits.present = present ? 1 : 0;
        bits.dpl = 0;
        bits.ist = 0;
        bits.must_be_zero1 = 0;
        bits.reserved = 0;
    }

    __forceinline void setup(void* handler) {
        setup(handler, true, CPU::GetCs(), SEGMENT_DESCRIPTOR_TYPE_INTERRUPT_GATE);
    }

    __forceinline u64 getAddress() {
        return static_cast<u64>(bits.offset0_15) |
               (static_cast<u64>(bits.offset16_31) << 16) |
               (static_cast<u64>(bits.offset32_63) << 32);
    }
};

// IDT class for vmxroot exception handling
class IDT {
public:
    IDTGateDescriptor64 descriptor[256];

    __forceinline void setup_entry(size_t i, bool present, void* handler = nullptr) {
        descriptor[i].setup(
            handler,
            present,
            CPU::GetCs(),
            SEGMENT_DESCRIPTOR_TYPE_INTERRUPT_GATE
        );
    }

    __forceinline void setup_entry(size_t i, void* handler = nullptr) {
        descriptor[i].setup(handler);
    }

    __forceinline void* get_address() { return &descriptor; }
    __forceinline size_t get_limit() const { return sizeof(descriptor) | 7; }

    // Setup IDT with custom handlers for exceptions with and without error codes
    __forceinline void setup(void(*handler)(), void(*handler_ecode)()) {
        // Exceptions that push error codes: 8, 10-14, 17, 21, 29, 30
        constexpr bool has_error_code[32] = {
            false, false, false, false, false, false, false, false,  // 0-7
            true,  false, true,  true,  true,  true,  true,  false,  // 8-15
            false, true,  false, false, false, true,  false, false,  // 16-23
            false, false, false, false, false, true,  true,  false   // 24-31
        };

        for (size_t i = 0; i < 256; i++) {
            if (i < 32 && has_error_code[i]) {
                setup_entry(i, true, reinterpret_cast<void*>(handler_ecode));
            } else {
                setup_entry(i, true, reinterpret_cast<void*>(handler));
            }
        }
    }
};
