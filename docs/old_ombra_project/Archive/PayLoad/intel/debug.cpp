// PayLoad/intel/debug.cpp
// Intel VMX Debug Output Implementation
// Serial port output for debugging in vmxroot context

#include "debug.h"

#ifdef OMBRA_DEBUG

//===----------------------------------------------------------------------===//
// Decimal Number Printing
//===----------------------------------------------------------------------===//

auto dbg::debug_print_decimal(long long number) -> void
{
    if (number < 0)
    {
        __outbyte(DEBUG_PORT_NUM, '-');
        number = -number;
    }

    // Find first non-zero digit and print from there
    for (auto d = 1000000000000000000LL; d != 0; d /= 10)
    {
        if ((number / d) != 0)
        {
            __outbyte(DEBUG_PORT_NUM, alphabet[(number / d) % 10]);
        }
    }
}

//===----------------------------------------------------------------------===//
// Hexadecimal Number Printing
//===----------------------------------------------------------------------===//

auto dbg::debug_print_hex(u64 number, const bool show_zeros) -> void
{
    // Start from most significant nibble
    for (auto d = 0x1000000000000000ULL; d != 0; d /= 0x10)
    {
        if (show_zeros || (number / d) != 0)
        {
            __outbyte(DEBUG_PORT_NUM, alphabet[(number / d) % 0x10]);
        }
    }
}

//===----------------------------------------------------------------------===//
// Printf-style Formatted Output
//===----------------------------------------------------------------------===//

auto dbg::print(const char* format, ...) -> void
{
    va_list args;
    va_start(args, format);

    while (format[0])
    {
        if (format[0] == '%')
        {
            switch (format[1])
            {
            case 'd':
                // 32-bit signed decimal
                debug_print_decimal(va_arg(args, int));
                format += 2;
                continue;

            case 'x':
                // 32-bit hex (no leading zeros)
                debug_print_hex(va_arg(args, u32), false);
                format += 2;
                continue;

            case 'l':
                // Check for 'll' (64-bit) specifier
                if (format[2] == 'l')
                {
                    switch (format[3])
                    {
                    case 'd':
                        // 64-bit signed decimal
                        debug_print_decimal(va_arg(args, u64));
                        format += 4;
                        continue;

                    case 'x':
                        // 64-bit hex (no leading zeros)
                        debug_print_hex(va_arg(args, u64), false);
                        format += 4;
                        continue;
                    }
                }
                break;

            case 'p':
                // Pointer (64-bit hex with leading zeros)
                debug_print_hex(va_arg(args, u64), true);
                format += 2;
                continue;
            }
        }

        // Regular character - output directly
        __outbyte(DEBUG_PORT_NUM, format[0]);
        ++format;
    }

    va_end(args);
}

#endif // OMBRA_DEBUG
