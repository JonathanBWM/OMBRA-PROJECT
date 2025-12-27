/**
 * @file bigpool_test.c
 * @brief BigPool Visibility Test - Implementation
 *
 * Phase 4A: Testing Infrastructure
 *
 * Executes kernel-mode test to determine if SUPDrv PageAllocEx allocations
 * are visible in Windows PoolBigPageTable.
 */

#include "bigpool_test.h"
#include "../byovd/supdrv.h"
#include <stdio.h>
#include <string.h>

//=============================================================================
// Test Configuration
//=============================================================================

#define BIGPOOL_TEST_ALLOC_PAGES    512     // 2MB test allocation
#define BIGPOOL_TEST_TAG            0x74736554  // 'Test' (little-endian)

// VirtualBox-related pool tags to watch for
static const uint32_t g_SuspiciousTags[] = {
    0x786F4256,     // 'VBox' (little-endian)
    0x586F4256,     // 'VBoX'
    0x4D4D6F56,     // 'VoMM' - VBox memory manager
    0x72447053,     // 'SpDr' - SUPDrv
    0x674D7053,     // 'SpMg' - SUP memory
    0x396C644C,     // 'Ld9B' - LDPlayer
    0               // Sentinel
};

//=============================================================================
// Helper Functions
//=============================================================================

/**
 * Check if pool tag is VirtualBox-related
 */
static bool IsVBoxTag(uint32_t tag) {
    for (int i = 0; g_SuspiciousTags[i] != 0; i++) {
        if (tag == g_SuspiciousTags[i]) {
            return true;
        }
    }
    return false;
}

/**
 * Convert pool tag to readable string (handles little-endian)
 */
static void TagToString(uint32_t tag, char* buf, size_t len) {
    if (len < 5) return;

    // Little-endian: bytes are reversed
    buf[0] = (char)((tag >> 0) & 0xFF);
    buf[1] = (char)((tag >> 8) & 0xFF);
    buf[2] = (char)((tag >> 16) & 0xFF);
    buf[3] = (char)((tag >> 24) & 0xFF);
    buf[4] = '\0';

    // Replace non-printable chars with '?'
    for (int i = 0; i < 4; i++) {
        if (buf[i] < 32 || buf[i] > 126) {
            buf[i] = '?';
        }
    }
}

//=============================================================================
// Main Test Function
//=============================================================================

bool RunBigPoolTest(BIGPOOL_TEST_CONTEXT* ctx) {
    if (!ctx || !ctx->pDrvCtx || !ctx->pDrvCtx->Initialized) {
        printf("[-] Invalid test context or driver not initialized\n");
        return false;
    }

    printf("[*] BigPool Visibility Test\n");
    printf("[*] =======================\n\n");

    // Clear result structure
    memset(&ctx->Result, 0, sizeof(ctx->Result));
    ctx->TestCompleted = false;

    printf("[*] Test allocates %u pages (2MB) via PageAllocEx\n", BIGPOOL_TEST_ALLOC_PAGES);
    printf("[*] Then scans nt!PoolBigPageTable for the allocation\n\n");

    // PLACEHOLDER: Kernel test execution via LDR_LOAD
    //
    // The actual kernel test code needs to be compiled separately as a
    // position-independent kernel module and loaded via DrvLdrOpen/DrvLdrLoad.
    //
    // For now, we stub out the test and assume worst case (visible).

    printf("[!] Kernel test execution not yet implemented\n");
    printf("[!] Kernel-mode BigPool scanner module required\n");
    printf("[!] See: hypervisor/hypervisor/tests/bigpool_test_kernel.c\n\n");

    // Stub results for development
    printf("[*] Using stub results (assumes worst case)...\n\n");

    ctx->Result.SymbolsResolved = 0;
    ctx->Result.AllocationSucceeded = 0;
    ctx->Result.FoundInBigPoolTable = 1;  // Assume visible until proven otherwise
    ctx->Result.TableSize = 0;
    ctx->Result.TableEntriesUsed = 0;
    ctx->Result.VBoxTagCount = 0;
    ctx->TestCompleted = true;

    PrintTestResults(&ctx->Result);

    return true;
}

//=============================================================================
// Result Display
//=============================================================================

void PrintTestResults(const BIGPOOL_TEST_RESULT* r) {
    printf("\n[*] Test Results:\n");
    printf("    -------------\n\n");

    // Symbol resolution
    if (!r->SymbolsResolved) {
        printf("[-] Symbol Resolution: FAILED\n");
        printf("    Could not resolve nt!PoolBigPageTable symbols\n");
        printf("    This is expected if kernel test module is not loaded\n\n");
    } else {
        printf("[+] Symbol Resolution: SUCCESS\n");
        printf("    PoolBigPageTable:     0x%016llX\n", (unsigned long long)r->PoolBigPageTableAddr);
        printf("    PoolBigPageTableSize: 0x%016llX\n", (unsigned long long)r->PoolBigPageTableSizeAddr);
        printf("    Table size:           %u slots\n", r->TableSize);
        printf("    Entries in use:       %u\n", r->TableEntriesUsed);
        printf("    VBox-tagged entries:  %u\n", r->VBoxTagCount);
        printf("\n");
    }

    // Allocation test
    if (!r->AllocationSucceeded) {
        printf("[-] Allocation Test: FAILED\n");
        printf("    PageAllocEx did not succeed\n");
        printf("    Cannot determine BigPool visibility\n\n");
    } else {
        printf("[+] Allocation Test: SUCCESS\n");
        printf("    R0 address: 0x%016llX\n", (unsigned long long)r->AllocatedR0);
        printf("    R3 address: 0x%016llX\n", (unsigned long long)r->AllocatedR3);
        printf("    Size:       %llu bytes (0x%llX)\n",
               (unsigned long long)r->AllocatedSize,
               (unsigned long long)r->AllocatedSize);
        printf("\n");
    }

    // BigPool visibility - THE CRITICAL RESULT
    printf("[*] BigPool Visibility:\n");
    printf("    -------------------\n");

    if (!r->SymbolsResolved || !r->AllocationSucceeded) {
        printf("[!] INCONCLUSIVE (test did not complete)\n");
        printf("[!] Assuming WORST CASE: allocation is visible\n\n");
    } else if (r->FoundInBigPoolTable) {
        char tagStr[5] = {0};
        TagToString(r->FoundPoolTag, tagStr, sizeof(tagStr));

        printf("[!] VISIBLE in PoolBigPageTable!\n\n");
        printf("    Pool tag:      '%s' (0x%08X)\n", tagStr, r->FoundPoolTag);
        printf("    Pool type:     %u\n", r->FoundPoolType);
        printf("    Recorded size: %llu bytes\n", (unsigned long long)r->FoundEntrySize);
        printf("    Entry address: 0x%016llX\n", (unsigned long long)r->TableEntryVa);

        if (IsVBoxTag(r->FoundPoolTag)) {
            printf("\n[!] WARNING: VirtualBox-related pool tag detected!\n");
            printf("[!] This is a HIGH-RISK detection vector for anti-cheat\n");
        }
    } else {
        printf("[+] NOT FOUND in PoolBigPageTable\n\n");
        printf("    PageAllocEx allocations are NOT tracked in BigPool\n");
        printf("    This is the CLEAN PATH - allocations are invisible\n");
    }

    printf("\n");

    // Conclusion and recommendation
    printf("[*] CONCLUSION:\n");
    printf("    -----------\n");

    DRIVER_MAPPING_STRATEGY strategy = DetermineMappingStrategy(r);
    const char* strategyNames[] = {
        "CLEAN PATH (PageAllocEx)",
        "MDL ALLOCATION (MmAllocatePagesForMdl)",
        "EPT-ONLY (Hypervisor-managed memory)"
    };

    if (strategy == STRATEGY_CLEAN_PATH) {
        printf("[+] Clean path IS viable\n");
        printf("[+] Recommended strategy: %s\n", strategyNames[strategy]);
        printf("[+] PageAllocEx allocations are safe from BigPool detection\n");
    } else {
        printf("[!] Clean path NOT viable\n");
        printf("[!] Recommended strategy: %s\n", strategyNames[strategy]);
        printf("[!] Must use mitigation to avoid BigPool tracking\n");
    }

    if (r->TestDurationUs > 0) {
        printf("\n[*] Test completed in %llu microseconds\n",
               (unsigned long long)r->TestDurationUs);
    }

    printf("\n");
}

//=============================================================================
// Strategy Decision
//=============================================================================

DRIVER_MAPPING_STRATEGY DetermineMappingStrategy(const BIGPOOL_TEST_RESULT* r) {
    // If test failed to complete, default to safest option
    if (!r->SymbolsResolved || !r->AllocationSucceeded) {
        printf("[!] Test incomplete - defaulting to safest strategy\n");
        return STRATEGY_EPT_ONLY;
    }

    // If allocation was NOT found in BigPool, clean path is safe
    if (!r->FoundInBigPoolTable) {
        return STRATEGY_CLEAN_PATH;
    }

    // Allocation WAS visible - need mitigation
    // MDL is simpler to implement, EPT-only is more robust
    //
    // For now, prefer MDL allocation as it's less complex than
    // managing all driver memory through the hypervisor EPT.
    return STRATEGY_MDL_ALLOCATION;
}
