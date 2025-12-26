// PayLoad/intel/debug.h
// Intel VMX Debug Output via Serial Port
// Provides printf-style debugging for vmxroot context
//
// SECURITY NOTE: Debug output is disabled in release builds.
// Define OMBRA_DEBUG to enable debug output.
#pragma once

#include <stdarg.h>
#include <stddef.h>
#include "../include/types.h"

// COM2 serial port for debug output
#define DEBUG_PORT_NUM 0x2F8

namespace dbg {

//===----------------------------------------------------------------------===//
// Hex alphabet for number formatting
//===----------------------------------------------------------------------===//

constexpr char alphabet[] = "0123456789ABCDEF";

//===----------------------------------------------------------------------===//
// Debug Print Functions
//===----------------------------------------------------------------------===//

#ifdef OMBRA_DEBUG

// Print a decimal number (supports negative values)
auto debug_print_decimal(long long number) -> void;

// Print a hexadecimal number
// show_zeros: if true, prints leading zeros (useful for addresses)
auto debug_print_hex(u64 number, const bool show_zeros) -> void;

// Printf-style formatted output
// Supported format specifiers:
//   %d  - 32-bit signed decimal
//   %x  - 32-bit hex (no leading zeros)
//   %lld - 64-bit signed decimal
//   %llx - 64-bit hex (no leading zeros)
//   %p  - pointer (64-bit hex with leading zeros)
auto print(const char* format, ...) -> void;

#else

// No-op stubs when debug is disabled
inline auto debug_print_decimal(long long) -> void {}
inline auto debug_print_hex(u64, const bool) -> void {}
inline auto print(const char*, ...) -> void {}

#endif // OMBRA_DEBUG

} // namespace dbg
