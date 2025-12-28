#include "pe_imports.h"
#include <stdio.h>
#include <string.h>

static const char* COMMON_IMPORTS[] = {
    "ExAllocatePoolWithTag",
    "ExFreePoolWithTag",
    "MmGetSystemRoutineAddress",
    "IoCreateDevice",
    "IoDeleteDevice",
    "IoCreateSymbolicLink",
    "IoDeleteSymbolicLink",
    "RtlInitUnicodeString",
    "DbgPrint",
    "KeQueryActiveProcessorCountEx",
    "KeGetCurrentProcessorNumberEx",
    "PsGetCurrentProcessId",
    "PsGetCurrentThreadId",
    "PsLookupProcessByProcessId",
    "ObDereferenceObject",
    "ZwQuerySystemInformation",
    "MmMapIoSpace",
    "MmUnmapIoSpace",
    NULL
};

// Helper: Check if module name is a kernel image variant
static bool IsKernelModule(const char* moduleName) {
    return (_stricmp(moduleName, "ntoskrnl.exe") == 0 ||
            _stricmp(moduleName, "ntkrnlpa.exe") == 0 ||
            _stricmp(moduleName, "ntkrnlmp.exe") == 0);
}

bool ResolveImports(PSUPDRV_CTX ctx, PE_INFO* peInfo) {
    if (!ctx || !peInfo || !peInfo->Valid) {
        printf("[-] ResolveImports: invalid parameters\n");
        return false;
    }

    if (peInfo->ImportCount == 0) {
        printf("[*] No imports to resolve\n");
        return true;
    }

    printf("[*] Resolving %u imports...\n", peInfo->ImportCount);

    uint32_t resolved = 0;
    uint32_t failed = 0;

    for (uint32_t i = 0; i < peInfo->ImportCount; i++) {
        PE_IMPORT_INFO* imp = &peInfo->Imports[i];

        // Skip if already resolved
        if (imp->ResolvedAddress != 0) {
            resolved++;
            continue;
        }

        // Normalize kernel module names to generic kernel symbol lookup
        const char* lookupName = imp->FunctionName;

        void* addr = NULL;
        bool success = SupDrv_GetSymbol(ctx, lookupName, &addr);

        if (success && addr != NULL) {
            imp->ResolvedAddress = (uint64_t)addr;
            resolved++;

            // Print progress for first few and every 10th import
            if (resolved <= 3 || (resolved % 10) == 0) {
                printf("[+] %s!%s -> 0x%llX\n",
                       imp->ModuleName, imp->FunctionName, imp->ResolvedAddress);
            }
        } else {
            printf("[-] Failed to resolve %s!%s\n",
                   imp->ModuleName, imp->FunctionName);
            failed++;
        }
    }

    printf("[*] Import resolution: %u/%u resolved, %u failed\n",
           resolved, peInfo->ImportCount, failed);

    // If ANY import failed, this is a fatal error for driver loading
    if (failed > 0) {
        printf("[-] Cannot load driver with unresolved imports\n");
        return false;
    }

    return true;
}

bool ResolveCommonSymbols(PSUPDRV_CTX ctx, COMMON_SYMBOLS* syms) {
    if (!ctx || !syms) {
        printf("[-] ResolveCommonSymbols: invalid parameters\n");
        return false;
    }

    // Zero the structure
    memset(syms, 0, sizeof(COMMON_SYMBOLS));

    printf("[*] Pre-resolving common kernel symbols...\n");

    uint32_t resolved = 0;
    uint32_t failed = 0;

    // Array of pointers to struct fields in the same order as COMMON_IMPORTS
    uint64_t* symPtrs[] = {
        &syms->ExAllocatePoolWithTag,
        &syms->ExFreePoolWithTag,
        &syms->MmGetSystemRoutineAddress,
        &syms->IoCreateDevice,
        &syms->IoDeleteDevice,
        &syms->IoCreateSymbolicLink,
        &syms->IoDeleteSymbolicLink,
        &syms->RtlInitUnicodeString,
        &syms->DbgPrint,
        &syms->KeQueryActiveProcessorCountEx,
        &syms->KeGetCurrentProcessorNumberEx,
        &syms->PsGetCurrentProcessId,
        &syms->PsGetCurrentThreadId,
        &syms->PsLookupProcessByProcessId,
        &syms->ObDereferenceObject,
        &syms->ZwQuerySystemInformation,
        &syms->MmMapIoSpace,
        &syms->MmUnmapIoSpace,
    };

    for (uint32_t i = 0; COMMON_IMPORTS[i] != NULL; i++) {
        const char* name = COMMON_IMPORTS[i];
        void* addr = NULL;

        bool success = SupDrv_GetSymbol(ctx, name, &addr);

        if (success && addr != NULL) {
            *symPtrs[i] = (uint64_t)addr;
            resolved++;

            // Print first few for confirmation
            if (resolved <= 3) {
                printf("[+] %s -> 0x%llX\n", name, *symPtrs[i]);
            }
        } else {
            // These are optional, so just track failures but don't abort
            failed++;
            if (failed <= 3) {
                printf("[*] Could not resolve %s (optional)\n", name);
            }
        }
    }

    printf("[*] Common symbols: %u/%u resolved\n", resolved, resolved + failed);

    // Return true even if some failed - these are optional helpers
    return true;
}
