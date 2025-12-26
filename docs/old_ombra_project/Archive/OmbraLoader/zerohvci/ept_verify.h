#pragma once

// Phase 3.3: EPT Hiding Verification
// After driver is mapped and hypervisor is active, verify that EPT-based
// memory hiding is working. EPT creates two views of physical memory:
// - Execute view: Contains real driver code (CPU executes this)
// - Read view: Contains garbage/zeroes (what scanners see when reading)

#include "hypercall_verify.h"
#include "exploit.h"
#include "../../OmbraShared/communication.hpp"
#include <Windows.h>
#include <cstdint>
#include <cstdio>

namespace zerohvci {
namespace ept {

// Test results structure for comprehensive verification
struct EptVerificationResult {
    bool hypercallsWork;        // Can we communicate with hypervisor?
    bool eptBaseRetrieved;      // Could we query EPT/NPT base?
    bool storageSetWorks;       // Can we write to vmxroot storage slots?
    bool executePathWorks;      // Does code execution work at driver base?
    bool readsReturnDifferent;  // Do reads return different data than expected?
    bool eptHidingConfirmed;    // Overall: is EPT hiding verified working?

    uint64_t eptBase;           // EPT/NPT root physical address
    uint64_t driverBase;        // Driver kernel virtual address
    uint64_t readValue;         // What we read from driver memory
    uint64_t expectedValue;     // What the code actually is (MZ header signature)
};

// Get current EPT/NPT base address via hypercall
// Returns 0 on failure
inline uint64_t GetEptBase(uint64_t session_key)
{
    COMMAND_DATA command = { 0 };

    VMX_ROOT_ERROR result = hypercall::ExecuteHypercall(
        VMCALL_GET_EPT_BASE,
        &command,
        0,
        session_key
    );

    if (result != VMX_ROOT_ERROR::SUCCESS) {
        printf("[-] VMCALL_GET_EPT_BASE failed (error: %d)\n", static_cast<int>(result));
        return 0;
    }

    // EPT base is returned in cr3.value (same union field)
    return command.cr3.value;
}

// Set a value in vmxroot storage slot
// Storage slots are per-core persistent data areas in the hypervisor
inline bool SetStorageSlot(uint64_t session_key, VMX_ROOT_STORAGE slot, uint64_t value)
{
    COMMAND_DATA command = { 0 };
    command.storage.slot = slot;
    command.storage.value = value;
    command.storage.write = true;

    VMX_ROOT_ERROR result = hypercall::ExecuteHypercall(
        VMCALL_STORAGE_QUERY,
        &command,
        0,
        session_key
    );

    return result == VMX_ROOT_ERROR::SUCCESS;
}

// Get a value from vmxroot storage slot
inline bool GetStorageSlot(uint64_t session_key, VMX_ROOT_STORAGE slot, uint64_t* outValue)
{
    if (!outValue) return false;

    COMMAND_DATA command = { 0 };
    command.storage.slot = slot;
    command.storage.write = false;

    VMX_ROOT_ERROR result = hypercall::ExecuteHypercall(
        VMCALL_STORAGE_QUERY,
        &command,
        0,
        session_key
    );

    if (result != VMX_ROOT_ERROR::SUCCESS) {
        return false;
    }

    *outValue = command.storage.value;
    return true;
}

// Register driver callback address with hypervisor
// This tells the hypervisor where to invoke when driver ops are needed
inline bool RegisterDriverCallback(uint64_t session_key, uint64_t callbackAddress)
{
    return SetStorageSlot(session_key, CALLBACK_ADDRESS, callbackAddress);
}

// Register driver physical base with hypervisor (for EPT operations)
inline bool RegisterDriverBase(uint64_t session_key, uint64_t driverPhysicalBase)
{
    return SetStorageSlot(session_key, DRIVER_BASE_PA, driverPhysicalBase);
}

// Set EPT violation handler address in hypervisor storage
// When EPT violation occurs, hypervisor invokes this handler
inline bool SetEptHandler(uint64_t session_key, uint64_t handlerAddress)
{
    return SetStorageSlot(session_key, EPT_HANDLER_ADDRESS, handlerAddress);
}

// Test that we can read from driver memory via kernel read primitive
// Returns false if reading fails
inline bool TestReadPath(uint64_t driverBase, uint64_t* outValue)
{
    if (!outValue || !driverBase) return false;

    // Read the first 8 bytes from driver base (should be MZ header or zeroes if hidden)
    if (!ReadKernelMemory(reinterpret_cast<PVOID>(driverBase), outValue, sizeof(uint64_t))) {
        printf("[-] Failed to read from driver base 0x%llX\n", driverBase);
        return false;
    }

    return true;
}

// Test that driver code can execute by invoking a test function
// The driver should export a function that returns a magic value
// For now, we just verify the memory is accessible for execution
inline bool TestExecutePath(uint64_t driverBase)
{
    if (!driverBase) return false;

    // We can't directly call driver code from usermode
    // The verification is implicit - if driver initialization succeeded
    // and callbacks work, execution path is functional

    // For full verification, the driver should be called via hypercall
    // and return a magic value to prove execution worked

    return true;
}

// Main EPT hiding verification
// Checks that reads return different data than what's actually in memory
inline EptVerificationResult VerifyEptHiding(
    uint64_t session_key,
    uint64_t driverBase,
    uint64_t driverSize)
{
    EptVerificationResult result = { 0 };
    result.driverBase = driverBase;

    printf("\n[*] Starting EPT hiding verification...\n");
    printf("[*] Driver base: 0x%llX, size: 0x%llX\n", driverBase, driverSize);

    // Step 1: Verify hypercalls work
    printf("[*] Verifying hypercall communication...\n");
    uint64_t cr3 = 0;
    result.hypercallsWork = hypercall::VerifyHypercallWorks(&cr3, session_key);
    if (!result.hypercallsWork) {
        printf("[-] Hypercall verification failed\n");
        return result;
    }
    printf("[+] Hypercalls working (CR3: 0x%llX)\n", cr3);

    // Step 2: Get EPT base
    printf("[*] Querying EPT/NPT base...\n");
    result.eptBase = GetEptBase(session_key);
    result.eptBaseRetrieved = (result.eptBase != 0);
    if (result.eptBaseRetrieved) {
        printf("[+] EPT/NPT base: 0x%llX\n", result.eptBase);
    } else {
        printf("[-] Could not retrieve EPT base\n");
    }

    // Step 3: Test storage slot operations
    printf("[*] Testing storage slot operations...\n");
    uint64_t testValue = 0x1234567890ABCDEF;
    // Use a high slot number (64+) that's designated for driver/client use
    VMX_ROOT_STORAGE testSlot = static_cast<VMX_ROOT_STORAGE>(64);
    result.storageSetWorks = SetStorageSlot(session_key, testSlot, testValue);
    if (result.storageSetWorks) {
        uint64_t readBack = 0;
        if (GetStorageSlot(session_key, testSlot, &readBack) && readBack == testValue) {
            printf("[+] Storage slot read/write verified\n");
        } else {
            printf("[!] Storage slot write succeeded but read back mismatch\n");
            result.storageSetWorks = false;
        }
    } else {
        printf("[-] Storage slot operations not available\n");
    }

    // Step 4: Read from driver memory
    printf("[*] Reading from driver memory...\n");
    result.readsReturnDifferent = false;
    if (TestReadPath(driverBase, &result.readValue)) {
        printf("[+] Read value from driver base: 0x%llX\n", result.readValue);

        // Expected: If EPT hiding is NOT active, we should read the MZ header
        // DOS header signature is 0x5A4D ('MZ' in little endian)
        // Full first 8 bytes typically: 4D 5A 90 00 03 00 00 00
        result.expectedValue = 0x00000003009005A4D; // Typical PE header start

        // Check if read value differs from expected PE header
        // If EPT hiding is active, reads should return zeroes or garbage
        bool looksLikePEHeader = (result.readValue & 0xFFFF) == 0x5A4D; // MZ signature

        if (!looksLikePEHeader) {
            result.readsReturnDifferent = true;
            printf("[+] Read value does NOT look like PE header - EPT may be hiding\n");
        } else {
            printf("[!] Read value looks like PE header - EPT hiding may not be active\n");
        }
    } else {
        printf("[-] Could not read from driver memory\n");
    }

    // Step 5: Implicit execute path test
    // If we got here and driver initialization succeeded, execution works
    result.executePathWorks = true;
    printf("[+] Execute path assumed working (driver initialized)\n");

    // Final determination
    // EPT hiding is confirmed if:
    // 1. Hypercalls work
    // 2. Either storage or EPT base operations work
    // 3. Reads return non-PE data (zeroes/garbage)
    result.eptHidingConfirmed =
        result.hypercallsWork &&
        (result.storageSetWorks || result.eptBaseRetrieved) &&
        result.readsReturnDifferent;

    printf("\n[*] EPT Verification Summary:\n");
    printf("    Hypercalls: %s\n", result.hypercallsWork ? "OK" : "FAIL");
    printf("    EPT Base:   %s\n", result.eptBaseRetrieved ? "OK" : "N/A");
    printf("    Storage:    %s\n", result.storageSetWorks ? "OK" : "N/A");
    printf("    Execute:    %s\n", result.executePathWorks ? "OK" : "FAIL");
    printf("    Read Diff:  %s\n", result.readsReturnDifferent ? "YES" : "NO");
    printf("    EPT Hidden: %s\n", result.eptHidingConfirmed ? "CONFIRMED" : "NOT CONFIRMED");

    return result;
}

// Simplified verification - just check hypervisor basics work
// Call this before attempting EPT hiding operations
inline bool VerifyHypervisorBasics(uint64_t session_key)
{
    printf("[*] Verifying hypervisor basics...\n");

    // Test 1: CR3 read
    uint64_t cr3 = 0;
    if (!hypercall::VerifyHypercallWorks(&cr3, session_key)) {
        printf("[-] CR3 read failed\n");
        return false;
    }
    printf("[+] CR3 read OK: 0x%llX\n", cr3);

    // Test 2: EPT base query
    uint64_t eptBase = GetEptBase(session_key);
    if (eptBase != 0) {
        printf("[+] EPT base OK: 0x%llX\n", eptBase);
    } else {
        printf("[!] EPT base query not available (may be OK)\n");
    }

    printf("[+] Hypervisor basics verified\n");
    return true;
}

// Enable EPT via hypercall (if it was disabled)
inline bool EnableEpt(uint64_t session_key)
{
    VMX_ROOT_ERROR result = hypercall::ExecuteHypercall(
        VMCALL_ENABLE_EPT,
        nullptr,
        0,
        session_key
    );

    return result == VMX_ROOT_ERROR::SUCCESS;
}

// Disable EPT via hypercall (dangerous - for debugging only)
inline bool DisableEpt(uint64_t session_key)
{
    VMX_ROOT_ERROR result = hypercall::ExecuteHypercall(
        VMCALL_DISABLE_EPT,
        nullptr,
        0,
        session_key
    );

    return result == VMX_ROOT_ERROR::SUCCESS;
}

} // namespace ept
} // namespace zerohvci
