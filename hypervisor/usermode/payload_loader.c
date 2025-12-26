// payload_loader.c — Hypervisor Payload Loader Implementation
// OmbraHypervisor

#include "payload_loader.h"
#include <stdio.h>
#include <string.h>

// =============================================================================
// CPU Enumeration
// =============================================================================

static uint32_t GetCpuCount(void) {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwNumberOfProcessors;
}

// =============================================================================
// Per-CPU Allocation
// =============================================================================

bool HvAllocatePerCpu(HV_CONTEXT* ctx) {
    ctx->NumCpus = GetCpuCount();
    if (ctx->NumCpus > MAX_CPUS) {
        ctx->NumCpus = MAX_CPUS;
    }

    printf("[*] Allocating structures for %u CPUs\n", ctx->NumCpus);

    for (uint32_t i = 0; i < ctx->NumCpus; i++) {
        CPU_CONTEXT* cpu = &ctx->Cpus[i];
        cpu->CpuId = i;
        cpu->Virtualized = false;

        // VMXON Region: 4KB, must be contiguous, 4KB aligned
        if (DrvAllocContiguous(&ctx->Driver, 1, &cpu->VmxonRegion) != DRV_SUCCESS) {
            printf("[-] Failed to allocate VMXON region for CPU %u\n", i);
            goto fail;
        }
        memset(cpu->VmxonRegion.R3, 0, 4096);

        // VMCS Region: 4KB, must be contiguous, 4KB aligned
        if (DrvAllocContiguous(&ctx->Driver, 1, &cpu->VmcsRegion) != DRV_SUCCESS) {
            printf("[-] Failed to allocate VMCS region for CPU %u\n", i);
            goto fail;
        }
        memset(cpu->VmcsRegion.R3, 0, 4096);

        // Host Stack: 8KB (2 pages), contiguous
        if (DrvAllocContiguous(&ctx->Driver, HOST_STACK_PAGES, &cpu->HostStack) != DRV_SUCCESS) {
            printf("[-] Failed to allocate host stack for CPU %u\n", i);
            goto fail;
        }
        memset(cpu->HostStack.R3, 0, HOST_STACK_PAGES * 4096);

        // MSR Bitmap: 4KB, 4KB aligned
        if (DrvAllocContiguous(&ctx->Driver, 1, &cpu->MsrBitmap) != DRV_SUCCESS) {
            printf("[-] Failed to allocate MSR bitmap for CPU %u\n", i);
            goto fail;
        }
        // Initialize MSR bitmap to all zeros (pass-through all MSRs initially)
        memset(cpu->MsrBitmap.R3, 0, 4096);

        printf("[+] CPU %u: VMXON=%p VMCS=%p Stack=%p MSR=%p\n",
               i, cpu->VmxonRegion.R0, cpu->VmcsRegion.R0,
               cpu->HostStack.R0, cpu->MsrBitmap.R0);
    }

    return true;

fail:
    HvFreePerCpu(ctx);
    return false;
}

bool HvFreePerCpu(HV_CONTEXT* ctx) {
    for (uint32_t i = 0; i < ctx->NumCpus; i++) {
        CPU_CONTEXT* cpu = &ctx->Cpus[i];

        if (cpu->VmxonRegion.R3) {
            DrvFreeContiguous(&ctx->Driver, &cpu->VmxonRegion);
        }
        if (cpu->VmcsRegion.R3) {
            DrvFreeContiguous(&ctx->Driver, &cpu->VmcsRegion);
        }
        if (cpu->HostStack.R3) {
            DrvFreeContiguous(&ctx->Driver, &cpu->HostStack);
        }
        if (cpu->MsrBitmap.R3) {
            DrvFreeContiguous(&ctx->Driver, &cpu->MsrBitmap);
        }
        if (cpu->IoBitmapA.R3) {
            DrvFreeContiguous(&ctx->Driver, &cpu->IoBitmapA);
        }
        if (cpu->IoBitmapB.R3) {
            DrvFreeContiguous(&ctx->Driver, &cpu->IoBitmapB);
        }

        memset(cpu, 0, sizeof(*cpu));
    }

    return true;
}

// =============================================================================
// EPT Allocation
// =============================================================================

bool HvAllocateEpt(HV_CONTEXT* ctx) {
    // For 512GB identity map with 1GB pages:
    //   1 PML4 (4KB) + 1 PDPT (4KB) = 8KB minimum
    // For 2MB pages: add 512 PD entries = 512 * 4KB = 2MB
    // We'll allocate 2MB to be safe

    printf("[*] Allocating EPT tables (%u pages)\n", EPT_TABLES_PAGES);

    if (DrvAllocContiguous(&ctx->Driver, EPT_TABLES_PAGES, &ctx->EptTables) != DRV_SUCCESS) {
        printf("[-] Failed to allocate EPT tables\n");
        return false;
    }

    memset(ctx->EptTables.R3, 0, EPT_TABLES_PAGES * 4096);

    printf("[+] EPT tables at R0=%p Phys=0x%llX\n",
           ctx->EptTables.R0, (unsigned long long)ctx->EptTables.Physical);

    return true;
}

// =============================================================================
// Debug Buffer Allocation
// =============================================================================

bool HvAllocateDebugBuffer(HV_CONTEXT* ctx) {
    printf("[*] Allocating debug buffer (%u pages)\n", DEBUG_BUFFER_PAGES);

    if (DrvAllocContiguous(&ctx->Driver, DEBUG_BUFFER_PAGES, &ctx->DebugBuffer) != DRV_SUCCESS) {
        printf("[-] Failed to allocate debug buffer\n");
        return false;
    }

    // Zero out the buffer - hypervisor will initialize the header
    memset(ctx->DebugBuffer.R3, 0, DEBUG_BUFFER_PAGES * 4096);

    printf("[+] Debug buffer at R3=%p R0=%p\n",
           ctx->DebugBuffer.R3, ctx->DebugBuffer.R0);

    return true;
}

void* HvGetDebugBuffer(HV_CONTEXT* ctx) {
    if (!ctx || !ctx->DebugBuffer.R3) {
        return NULL;
    }
    return ctx->DebugBuffer.R3;
}

size_t HvGetDebugBufferSize(HV_CONTEXT* ctx) {
    if (!ctx || !ctx->DebugBuffer.R3) {
        return 0;
    }
    return DEBUG_BUFFER_PAGES * 4096;
}

// =============================================================================
// Payload Copy
// =============================================================================

bool HvCopyPayload(HV_CONTEXT* ctx, const void* payload, size_t size) {
    // Calculate pages needed (round up)
    uint32_t pages = (uint32_t)((size + 4095) / 4096);

    printf("[*] Copying hypervisor payload (%zu bytes, %u pages)\n", size, pages);

    // Allocate with both R3 and R0 mappings
    if (DrvAllocPages(&ctx->Driver, pages, true, &ctx->HypervisorCode) != DRV_SUCCESS) {
        printf("[-] Failed to allocate hypervisor code region\n");
        return false;
    }

    // Copy payload via R3 mapping
    memcpy(ctx->HypervisorCode.R3, payload, size);

    // Entry point is at offset 0 (OmbraInitialize should be first)
    ctx->EntryPoint = ctx->HypervisorCode.R0;

    printf("[+] Hypervisor code at R0=%p (entry=%p)\n",
           ctx->HypervisorCode.R0, ctx->EntryPoint);

    return true;
}

// =============================================================================
// Per-CPU Launch
// =============================================================================

// Structure passed to hypervisor entry point
typedef struct _HV_INIT_DATA {
    uint32_t    CpuId;
    uint32_t    TotalCpus;

    // Per-CPU physical addresses
    uint64_t    VmxonPhysical;
    uint64_t    VmcsPhysical;
    uint64_t    HostStackTop;       // R0 address of stack TOP (stack grows down)
    uint64_t    MsrBitmapPhysical;

    // Shared physical addresses
    uint64_t    EptPml4Physical;

    // VMX MSR values (pre-fetched from driver)
    uint64_t    VmxBasic;
    uint64_t    VmxPinCtls;
    uint64_t    VmxProcCtls;
    uint64_t    VmxProcCtls2;
    uint64_t    VmxExitCtls;
    uint64_t    VmxEntryCtls;
    uint64_t    VmxCr0Fixed0;
    uint64_t    VmxCr0Fixed1;
    uint64_t    VmxCr4Fixed0;
    uint64_t    VmxCr4Fixed1;
    uint64_t    VmxEptVpidCap;

    // Exit handler address
    uint64_t    VmexitHandler;

    // Debug infrastructure
    uint64_t    DebugBufferPhysical;
    uint64_t    DebugBufferSize;
} HV_INIT_DATA;

bool HvLaunchOnCpu(HV_CONTEXT* ctx, uint32_t cpuId) {
    CPU_CONTEXT* cpu = &ctx->Cpus[cpuId];

    printf("[*] Launching hypervisor on CPU %u\n", cpuId);

    // Prepare initialization data
    // This would need to be copied to a kernel-accessible location
    // For now, we'll use the driver's call mechanism

    // The actual implementation depends on how SUP_IOCTL_CALL_VMMR0 works
    // It may require loading via LDR_OPEN/LDR_LOAD first

    // TODO: Implement proper launch sequence
    // 1. Set affinity to target CPU
    // 2. Call entry point via driver
    // 3. Verify launch succeeded

    int32_t result = 0;
    DRV_STATUS status = DrvExecuteOnCpu(&ctx->Driver, cpuId, ctx->EntryPoint, 0, &result);

    if (status != DRV_SUCCESS) {
        printf("[-] Failed to execute on CPU %u: %s\n", cpuId, DrvStatusString(status));
        return false;
    }

    if (result != 0) {
        printf("[-] Hypervisor init failed on CPU %u: error %d\n", cpuId, result);
        return false;
    }

    cpu->Virtualized = true;
    printf("[+] CPU %u virtualized\n", cpuId);

    return true;
}

bool HvLaunchAll(HV_CONTEXT* ctx) {
    printf("[*] Launching hypervisor on all %u CPUs\n", ctx->NumCpus);

    // Launch on CPU 0 first (single-CPU debugging is easier)
    if (!HvLaunchOnCpu(ctx, 0)) {
        printf("[-] Failed to launch on CPU 0\n");
        return false;
    }

    // Then launch on remaining CPUs
    for (uint32_t i = 1; i < ctx->NumCpus; i++) {
        if (!HvLaunchOnCpu(ctx, i)) {
            printf("[-] Failed to launch on CPU %u\n", i);
            // Continue anyway - partial virtualization may be acceptable
        }
    }

    ctx->Running = true;
    return true;
}

// =============================================================================
// High-Level API
// =============================================================================

bool HvLoad(HV_CONTEXT* ctx, const wchar_t* driverPath, const void* payload, size_t payloadSize) {
    memset(ctx, 0, sizeof(*ctx));

    printf("[*] Initializing OmbraHypervisor\n");

    // Step 1: Initialize driver interface
    printf("[*] Loading vulnerable driver\n");
    DRV_STATUS status = DrvInitialize(&ctx->Driver, driverPath);
    if (status != DRV_SUCCESS) {
        printf("[-] Driver init failed: %s\n", DrvStatusString(status));
        return false;
    }
    printf("[+] Driver loaded, session established\n");

    // Step 2: Check VMX support
    uint32_t vtCaps = 0;
    status = DrvQueryVmxCaps(&ctx->Driver, &vtCaps);
    if (status != DRV_SUCCESS || !(vtCaps & 0x1)) {
        printf("[-] VMX not supported or disabled\n");
        DrvCleanup(&ctx->Driver);
        return false;
    }
    printf("[+] VMX supported (caps=0x%X)\n", vtCaps);

    // Step 3: Display VMX MSR info
    printf("[*] VMX MSRs:\n");
    printf("    Basic: 0x%llX\n", (unsigned long long)ctx->Driver.VmxMsrs.Basic);
    printf("    EPT/VPID: 0x%llX\n", (unsigned long long)ctx->Driver.VmxMsrs.EptVpidCap);

    // Step 4: Allocate per-CPU structures
    if (!HvAllocatePerCpu(ctx)) {
        printf("[-] Failed to allocate per-CPU structures\n");
        DrvCleanup(&ctx->Driver);
        return false;
    }

    // Step 5: Allocate EPT tables
    if (!HvAllocateEpt(ctx)) {
        printf("[-] Failed to allocate EPT tables\n");
        HvFreePerCpu(ctx);
        DrvCleanup(&ctx->Driver);
        return false;
    }

    // Step 6: Allocate debug buffer
    if (!HvAllocateDebugBuffer(ctx)) {
        printf("[-] Failed to allocate debug buffer\n");
        DrvFreeContiguous(&ctx->Driver, &ctx->EptTables);
        HvFreePerCpu(ctx);
        DrvCleanup(&ctx->Driver);
        return false;
    }

    // Step 7: Copy hypervisor payload
    if (!HvCopyPayload(ctx, payload, payloadSize)) {
        printf("[-] Failed to copy hypervisor payload\n");
        DrvFreeContiguous(&ctx->Driver, &ctx->DebugBuffer);
        DrvFreeContiguous(&ctx->Driver, &ctx->EptTables);
        HvFreePerCpu(ctx);
        DrvCleanup(&ctx->Driver);
        return false;
    }

    ctx->Loaded = true;

    // Step 8: Launch hypervisor
    if (!HvLaunchAll(ctx)) {
        printf("[-] Failed to launch hypervisor\n");
        // Continue - we might have partial success
    }

    printf("[+] OmbraHypervisor loaded\n");
    return true;
}

bool HvUnload(HV_CONTEXT* ctx) {
    printf("[*] Unloading OmbraHypervisor\n");

    // TODO: Send VMCALL to each CPU to trigger VMXOFF
    // For now, just clean up memory

    if (ctx->Running) {
        // Signal shutdown via VMCALL
        printf("[*] Signaling hypervisor shutdown\n");
        // TODO: Implement VMCALL-based shutdown
    }

    // Free allocations
    if (ctx->HypervisorCode.R3) {
        DrvFreePages(&ctx->Driver, &ctx->HypervisorCode);
    }

    if (ctx->EptTables.R3) {
        DrvFreeContiguous(&ctx->Driver, &ctx->EptTables);
    }

    if (ctx->DebugBuffer.R3) {
        DrvFreeContiguous(&ctx->Driver, &ctx->DebugBuffer);
    }

    HvFreePerCpu(ctx);

    // Cleanup driver
    DrvCleanup(&ctx->Driver);

    memset(ctx, 0, sizeof(*ctx));
    printf("[+] Hypervisor unloaded\n");

    return true;
}

bool HvIsRunning(HV_CONTEXT* ctx) {
    if (!ctx->Running) return false;

    // TODO: Execute VMCALL to verify hypervisor responds
    // VMCALL_PING with magic prefix

    return ctx->Running;
}
