// obfuscate.h - Compile-time string obfuscation
// OmbraHypervisor
//
// Simple XOR-based obfuscation to defeat signature scanners.
// Strings are encrypted at compile time and decrypted at runtime.

#ifndef OMBRA_OBFUSCATE_H
#define OMBRA_OBFUSCATE_H

#include <stdint.h>
#include <string.h>

// =============================================================================
// Configuration
// =============================================================================

// XOR key - change this to generate different encrypted values
#define OBF_KEY 0x5A

// Build mode: 1 = obfuscated, 0 = plaintext (for debugging)
#ifndef OBF_ENABLED
#define OBF_ENABLED 1
#endif

// =============================================================================
// Runtime Decryption
// =============================================================================

// Decrypt a string in-place (modifies the buffer)
static inline void obf_decrypt(char* buf, size_t len) {
#if OBF_ENABLED
    for (size_t i = 0; i < len; i++) {
        buf[i] ^= OBF_KEY;
    }
#else
    (void)buf; (void)len;
#endif
}

// Decrypt a wide string in-place
static inline void obf_decrypt_w(wchar_t* buf, size_t len) {
#if OBF_ENABLED
    for (size_t i = 0; i < len; i++) {
        buf[i] ^= OBF_KEY;
    }
#else
    (void)buf; (void)len;
#endif
}

// Decrypt to a static buffer (thread-unsafe but convenient for printf)
// Returns pointer to internal static buffer - use immediately
static inline const char* obf_dec(const char* encrypted, size_t len) {
    static char buf[512];
    if (len >= sizeof(buf)) len = sizeof(buf) - 1;
    memcpy(buf, encrypted, len);
    buf[len] = '\0';
    obf_decrypt(buf, len);
    return buf;
}

// Wide string version
static inline const wchar_t* obf_dec_w(const wchar_t* encrypted, size_t len) {
    static wchar_t buf[256];
    if (len >= (sizeof(buf)/sizeof(wchar_t))) len = (sizeof(buf)/sizeof(wchar_t)) - 1;
    memcpy(buf, encrypted, len * sizeof(wchar_t));
    buf[len] = L'\0';
    obf_decrypt_w(buf, len);
    return buf;
}

// =============================================================================
// Pre-encrypted Strings
// =============================================================================
// Generated with: python3 -c "print(''.join(f'\\x{ord(c)^0x5A:02x}' for c in 'STRING'))"

#if OBF_ENABLED

// "hypervisor" (10 chars) ^ 0x5A
#define OBF_HYPERVISOR      "\x32\x2f\x2a\x38\x2c\x2e\x39\x2d\x35\x2c"
#define OBF_HYPERVISOR_LEN  10

// "hypervisor.bin" (14 chars) ^ 0x5A
#define OBF_HV_BIN          "\x32\x2f\x2a\x38\x2c\x2e\x39\x2d\x35\x2c\x74\x38\x39\x3e"
#define OBF_HV_BIN_LEN      14

// "OmbraHypervisor" (15 chars) ^ 0x5A
#define OBF_OMBRA_HV        "\x15\x37\x38\x2c\x3b\x12\x2f\x2a\x38\x2c\x2e\x39\x2d\x35\x2c"
#define OBF_OMBRA_HV_LEN    15

// "OmbraHypervisor Loader" (22 chars) ^ 0x5A
#define OBF_LOADER_TITLE    "\x15\x37\x38\x2c\x3b\x12\x2f\x2a\x38\x2c\x2e\x39\x2d\x35\x2c\x7a\x16\x35\x3b\x3c\x38\x2c"
#define OBF_LOADER_TITLE_LEN 22

// "OmbraHV" (7 chars) ^ 0x5A
#define OBF_OMBRAHV         "\x15\x37\x38\x2c\x3b\x12\x18"
#define OBF_OMBRAHV_LEN     7

// "OMBR" (4 chars) ^ 0x5A -> 0x15, 0x17, 0x18, 0x08
// As uint32: 'O'^0x5A=0x15, 'M'^0x5A=0x17, 'B'^0x5A=0x18, 'R'^0x5A=0x08
#define OBF_MAGIC_OMBR      0x0818170F  // XOR with 0x5A5A5A5A to get "OMBR"

// "VMX" (3 chars) ^ 0x5A
#define OBF_VMX             "\x0C\x17\x02"
#define OBF_VMX_LEN         3

// "VMCS" (4 chars) ^ 0x5A
#define OBF_VMCS            "\x0C\x17\x19\x09"
#define OBF_VMCS_LEN        4

// "EPT" (3 chars) ^ 0x5A
#define OBF_EPT             "\x1F\x0A\x0E"
#define OBF_EPT_LEN         3

// Decryption macros for common strings
#define DEC_HYPERVISOR()    obf_dec(OBF_HYPERVISOR, OBF_HYPERVISOR_LEN)
#define DEC_HV_BIN()        obf_dec(OBF_HV_BIN, OBF_HV_BIN_LEN)
#define DEC_LOADER_TITLE()  obf_dec(OBF_LOADER_TITLE, OBF_LOADER_TITLE_LEN)
#define DEC_OMBRAHV()       obf_dec(OBF_OMBRAHV, OBF_OMBRAHV_LEN)

// =============================================================================
// Wide String Obfuscation (for Windows API calls)
// =============================================================================
// Generated with: python3 -c "print(''.join(f'\\x{ord(c)^0x5A:02x}' for c in 'STRING'))"

// "VirtualBox Support Driver" (25 chars) ^ 0x5A - service display name
#define OBF_VBOX_DRIVER_W       L"\x0C\x33\x28\x2E\x2F\x3B\x36\x18\x35\x22\x7A\x09\x2F\x2A\x2A\x35\x28\x2E\x7A\x1E\x28\x33\x2C\x3F\x28"
#define OBF_VBOX_DRIVER_W_LEN   25

// "Ld9BoxSup" (9 chars) ^ 0x5A - service name
#define OBF_LD9BOXSUP_W         L"\x16\x3E\x63\x18\x35\x22\x09\x2F\x2A"
#define OBF_LD9BOXSUP_W_LEN     9

// "VBoxDrv" (7 chars) ^ 0x5A - fallback service name
#define OBF_VBOXDRV_W           L"\x0C\x18\x35\x22\x1E\x28\x2C"
#define OBF_VBOXDRV_W_LEN       7

// "\\\\.\\Ld9BoxSup" (13 chars) ^ 0x5A - device path
#define OBF_LD9BOX_DEVICE_W     L"\x66\x66\x74\x66\x16\x3E\x63\x18\x35\x22\x09\x2F\x2A"
#define OBF_LD9BOX_DEVICE_W_LEN 13

// "\\\\.\\VBoxDrv" (11 chars) ^ 0x5A - VBox device path
#define OBF_VBOX_DEVICE_W       L"\x66\x66\x74\x66\x0C\x18\x35\x22\x1E\x28\x2C"
#define OBF_VBOX_DEVICE_W_LEN   11

// Wide decryption macros
#define DEC_VBOX_DRIVER_W()     obf_dec_w(OBF_VBOX_DRIVER_W, OBF_VBOX_DRIVER_W_LEN)
#define DEC_LD9BOXSUP_W()       obf_dec_w(OBF_LD9BOXSUP_W, OBF_LD9BOXSUP_W_LEN)
#define DEC_VBOXDRV_W()         obf_dec_w(OBF_VBOXDRV_W, OBF_VBOXDRV_W_LEN)
#define DEC_LD9BOX_DEVICE_W()   obf_dec_w(OBF_LD9BOX_DEVICE_W, OBF_LD9BOX_DEVICE_W_LEN)
#define DEC_VBOX_DEVICE_W()     obf_dec_w(OBF_VBOX_DEVICE_W, OBF_VBOX_DEVICE_W_LEN)

#else
// Plaintext for debugging
#define DEC_HYPERVISOR()    "hypervisor"
#define DEC_HV_BIN()        "hypervisor.bin"
#define DEC_LOADER_TITLE()  "OmbraHypervisor Loader"
#define DEC_OMBRAHV()       "OmbraHV"
#define DEC_VBOX_DRIVER_W() L"VirtualBox Support Driver"
#define DEC_LD9BOXSUP_W()   L"Ld9BoxSup"
#define DEC_VBOXDRV_W()     L"VBoxDrv"
#define DEC_LD9BOX_DEVICE_W() L"\\\\.\\Ld9BoxSup"
#define DEC_VBOX_DEVICE_W() L"\\\\.\\VBoxDrv"
#endif

// =============================================================================
// Debug String Control
// =============================================================================

// Compile out debug strings in release builds
#ifdef NDEBUG
    #define DBG_STR(s) ""
    #define DBG_PRINTF(...) ((void)0)
#else
    #define DBG_STR(s) s
    #define DBG_PRINTF(...) printf(__VA_ARGS__)
#endif

// =============================================================================
// Magic Value Obfuscation
// =============================================================================

// Original: 0x4F4D4252 ("OMBR")
// Obfuscated by XOR with 0xDEADBEEF
#define OBF_MARKER_MAGIC_KEY    0xDEADBEEF
#define OBF_MARKER_MAGIC_ENC    0x91E0FCBD  // 0x4F4D4252 ^ 0xDEADBEEF

static inline uint32_t get_marker_magic(void) {
    return OBF_MARKER_MAGIC_ENC ^ OBF_MARKER_MAGIC_KEY;
}

#endif // OMBRA_OBFUSCATE_H
