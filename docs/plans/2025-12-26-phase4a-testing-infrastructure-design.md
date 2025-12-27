# Phase 4A: Testing Infrastructure Design

## Overview

Phase 4A creates the testing infrastructure required before Phase 2 implementation can proceed. The critical deliverable is the BigPool visibility test, which determines whether the clean path (PageAllocEx) or mitigation path (MDL/EPT-only) is viable for driver mapping.

**Goal:** Empirically determine if SUPDrv's `PageAllocEx` allocations appear in Windows' `PoolBigPageTable`, enabling anti-cheat detection.

**Blocking Relationship:** Phase 2 implementation cannot be finalized without BigPool test results.

---

## Execution Context

The BigPool test runs from kernel mode via SUPDrv's code execution primitive, while we still have kernel access during the Phase 2 pre-check stage.

**Flow:**
```
Phase 1: Load Hypervisor
    ↓
Phase 2 Pre-check: BigPool Test  ← THIS PHASE
    ↓
    ├─→ Not visible: Proceed with clean path (PageAllocEx)
    │
    └─→ Visible: Switch to mitigation path (MDL or EPT-only)
    ↓
Phase 2: Map Driver (using chosen path)
```

---

## PoolBigPageTable Structure

Windows kernel maintains `nt!PoolBigPageTable`, a hash table tracking allocations larger than one page:

```c
// Windows kernel structure (undocumented, reverse-engineered)
typedef struct _POOL_TRACKER_BIG_PAGES {
    volatile uint64_t   Va;             // Virtual address of allocation
    uint32_t            Key;            // Pool tag (4 chars)
    uint32_t            PoolType;       // NonPagedPool, PagedPool, etc.
    uint64_t            NumberOfBytes;  // Allocation size
} POOL_TRACKER_BIG_PAGES, *PPOOL_TRACKER_BIG_PAGES;

// Related symbols:
// nt!PoolBigPageTable      - Pointer to array base
// nt!PoolBigPageTableSize  - Number of slots (entries, not bytes)
// nt!PoolBigPageTableHash  - Hash table for fast lookup (Win10+)
```

**Entry States:**
- `Va == 0`: Empty slot
- `Va == 1`: Deleted entry
- `Va > 1`: Valid allocation

---

## Test Result Structure

```c
typedef struct _BIGPOOL_TEST_RESULT {
    // Allocation info
    BOOLEAN     AllocationSucceeded;
    uint64_t    AllocatedR0;            // Kernel VA
    uint64_t    AllocatedR3;            // User VA (if dual-mapped)
    uint64_t    AllocatedSize;          // Actual size

    // BigPool visibility - THE KEY RESULT
    BOOLEAN     FoundInBigPoolTable;
    uint32_t    FoundPoolTag;           // Tag if found (might be VBox's tag)
    uint32_t    FoundPoolType;          // Pool type if found
    uint64_t    FoundEntrySize;         // Size recorded in table
    uint64_t    TableEntryVa;           // Address of the table entry itself

    // Table statistics
    uint32_t    TableSize;              // Total slots in table
    uint32_t    TableEntriesUsed;       // Non-empty entries
    uint32_t    VBoxTagCount;           // Entries with VBox-related tags

    // Symbol resolution
    uint64_t    PoolBigPageTableAddr;
    uint64_t    PoolBigPageTableSizeAddr;
    BOOLEAN     SymbolsResolved;

    // Timing
    uint64_t    TestDurationUs;

} BIGPOOL_TEST_RESULT;
```

---

## Kernel-Mode Test Implementation

```c
// Test configuration
#define BIGPOOL_TEST_ALLOC_PAGES    512     // 2MB test allocation
#define BIGPOOL_TEST_TAG            'tseT'  // 'Test' backwards

// Pool tags to watch for (VirtualBox-related)
static const uint32_t g_SuspiciousTags[] = {
    'xoBV',     // 'VBox' - VirtualBox generic
    'XoBV',     // 'VBoX' - variant
    'MMoV',     // 'VoMM' - VBox memory manager
    'rDpS',     // 'SpDr' - SUPDrv
    'gMpS',     // 'SpMg' - SUP memory
    0           // Sentinel
};

static BOOLEAN IsVBoxTag(uint32_t tag) {
    for (int i = 0; g_SuspiciousTags[i] != 0; i++) {
        if (tag == g_SuspiciousTags[i]) {
            return TRUE;
        }
    }
    return FALSE;
}

// Main test function - executed via SUPDrv
NTSTATUS BigPoolTestKernel(
    PBIGPOOL_TEST_RESULT result,
    PFN_DRV_RESOLVE_SYMBOL ResolveSymbol,
    PFN_PAGE_ALLOC_EX PageAllocEx,
    PFN_PAGE_FREE PageFree
) {
    RtlZeroMemory(result, sizeof(*result));
    uint64_t startTime = KeQueryPerformanceCounter(NULL).QuadPart;

    // Step 1: Resolve PoolBigPageTable symbols
    result->PoolBigPageTableAddr = ResolveSymbol("nt!PoolBigPageTable");
    result->PoolBigPageTableSizeAddr = ResolveSymbol("nt!PoolBigPageTableSize");

    if (!result->PoolBigPageTableAddr || !result->PoolBigPageTableSizeAddr) {
        result->SymbolsResolved = FALSE;
        return STATUS_ENTRYPOINT_NOT_FOUND;
    }
    result->SymbolsResolved = TRUE;

    // Step 2: Read table metadata
    PPOOL_TRACKER_BIG_PAGES tableBase =
        *(PPOOL_TRACKER_BIG_PAGES*)result->PoolBigPageTableAddr;
    uint32_t tableSize = *(uint32_t*)result->PoolBigPageTableSizeAddr;

    result->TableSize = tableSize;

    // Step 3: Scan baseline - count entries and VBox tags
    uint32_t baselineUsed = 0;
    uint32_t vboxCount = 0;

    for (uint32_t i = 0; i < tableSize; i++) {
        uint64_t va = tableBase[i].Va;
        if (va > 1) {
            baselineUsed++;
            if (IsVBoxTag(tableBase[i].Key)) {
                vboxCount++;
            }
        }
    }
    result->TableEntriesUsed = baselineUsed;
    result->VBoxTagCount = vboxCount;

    // Step 4: Perform test allocation via PageAllocEx
    void* allocR3 = NULL;
    void* allocR0 = NULL;

    int rc = PageAllocEx(
        BIGPOOL_TEST_ALLOC_PAGES,
        0,
        &allocR3,
        &allocR0
    );

    if (rc != 0 || !allocR0) {
        result->AllocationSucceeded = FALSE;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    result->AllocationSucceeded = TRUE;
    result->AllocatedR0 = (uint64_t)allocR0;
    result->AllocatedR3 = (uint64_t)allocR3;
    result->AllocatedSize = BIGPOOL_TEST_ALLOC_PAGES * PAGE_SIZE;

    // Step 5: Search table for our allocation
    result->FoundInBigPoolTable = FALSE;

    for (uint32_t i = 0; i < tableSize; i++) {
        uint64_t entryVa = tableBase[i].Va;

        if (entryVa > 1) {
            uint64_t entryEnd = entryVa + tableBase[i].NumberOfBytes;

            if ((uint64_t)allocR0 >= entryVa && (uint64_t)allocR0 < entryEnd) {
                result->FoundInBigPoolTable = TRUE;
                result->FoundPoolTag = tableBase[i].Key;
                result->FoundPoolType = tableBase[i].PoolType;
                result->FoundEntrySize = tableBase[i].NumberOfBytes;
                result->TableEntryVa = (uint64_t)&tableBase[i];
                break;
            }
        }
    }

    // Step 6: Cleanup
    PageFree(allocR3, BIGPOOL_TEST_ALLOC_PAGES);

    // Step 7: Record timing
    uint64_t endTime = KeQueryPerformanceCounter(NULL).QuadPart;
    LARGE_INTEGER freq;
    KeQueryPerformanceCounter(&freq);
    result->TestDurationUs = ((endTime - startTime) * 1000000) / freq.QuadPart;

    return STATUS_SUCCESS;
}
```

---

## Usermode Test Driver

```c
// usermode/tests/bigpool_test.c

#include "supdrv.h"
#include "bigpool_test.h"

typedef struct _BIGPOOL_TEST_CONTEXT {
    SUPDRV_HANDLE       hSupDrv;
    BIGPOOL_TEST_RESULT Result;
    BOOLEAN             TestCompleted;
} BIGPOOL_TEST_CONTEXT;

BOOL RunBigPoolTest(PBIGPOOL_TEST_CONTEXT ctx) {
    printf("[*] BigPool Visibility Test\n");
    printf("[*] =======================\n\n");

    // Execute kernel-mode test via SUPDrv
    printf("[*] Executing kernel-mode BigPool test...\n");

    int rc = SupDrvExecuteKernel(
        ctx->hSupDrv,
        BigPoolTestKernel,
        &ctx->Result,
        sizeof(ctx->Result)
    );

    if (rc != 0) {
        printf("[-] Kernel execution failed: %d\n", rc);
        return FALSE;
    }

    ctx->TestCompleted = TRUE;
    PrintTestResults(&ctx->Result);

    return TRUE;
}

void PrintTestResults(PBIGPOOL_TEST_RESULT r) {
    printf("\n[*] Test Results:\n");
    printf("    -------------\n");

    if (!r->SymbolsResolved) {
        printf("[-] FAILED: Could not resolve PoolBigPageTable symbols\n");
        return;
    }

    printf("[+] Symbols resolved:\n");
    printf("    PoolBigPageTable:     0x%llX\n", r->PoolBigPageTableAddr);
    printf("    PoolBigPageTableSize: 0x%llX\n", r->PoolBigPageTableSizeAddr);
    printf("    Table size: %u slots\n", r->TableSize);
    printf("    Entries in use: %u\n", r->TableEntriesUsed);
    printf("    VBox-tagged entries: %u\n", r->VBoxTagCount);

    printf("\n[*] Allocation test:\n");
    if (!r->AllocationSucceeded) {
        printf("[-] FAILED: PageAllocEx returned error\n");
        return;
    }

    printf("[+] PageAllocEx succeeded:\n");
    printf("    R0 address: 0x%llX\n", r->AllocatedR0);
    printf("    R3 address: 0x%llX\n", r->AllocatedR3);
    printf("    Size: %llu bytes (0x%llX)\n", r->AllocatedSize, r->AllocatedSize);

    printf("\n[*] BigPool visibility:\n");
    if (r->FoundInBigPoolTable) {
        char tagStr[5] = {0};
        memcpy(tagStr, &r->FoundPoolTag, 4);

        printf("[!] VISIBLE in PoolBigPageTable!\n");
        printf("    Pool tag: '%s' (0x%08X)\n", tagStr, r->FoundPoolTag);
        printf("    Pool type: %u\n", r->FoundPoolType);
        printf("    Recorded size: %llu\n", r->FoundEntrySize);
        printf("    Entry address: 0x%llX\n", r->TableEntryVa);

        printf("\n[!] CONCLUSION: Clean path NOT viable\n");
        printf("[!] Must use mitigation: MDL allocation or EPT-only memory\n");
    } else {
        printf("[+] NOT FOUND in PoolBigPageTable\n");
        printf("\n[+] CONCLUSION: Clean path IS viable\n");
        printf("[+] PageAllocEx allocations are not tracked in BigPool\n");
    }

    printf("\n[*] Test completed in %llu microseconds\n", r->TestDurationUs);
}
```

---

## Strategy Decision Function

```c
typedef enum _DRIVER_MAPPING_STRATEGY {
    STRATEGY_CLEAN_PATH,        // PageAllocEx directly
    STRATEGY_MDL_ALLOCATION,    // MmAllocatePagesForMdl
    STRATEGY_EPT_ONLY,          // Hypervisor-managed memory
} DRIVER_MAPPING_STRATEGY;

DRIVER_MAPPING_STRATEGY DetermineMappingStrategy(PBIGPOOL_TEST_RESULT r) {
    if (!r->SymbolsResolved || !r->AllocationSucceeded) {
        // Can't determine - default to safest option
        return STRATEGY_EPT_ONLY;
    }

    if (!r->FoundInBigPoolTable) {
        // Not visible - clean path is safe
        return STRATEGY_CLEAN_PATH;
    }

    // Visible - need mitigation
    // MDL is simpler, EPT-only is more robust
    return STRATEGY_MDL_ALLOCATION;
}
```

---

## Integration with Phase 2 Loader

```c
// loader/hv_loader.c

int Phase2_MapDriver(LOADER_CONTEXT* ctx) {
    // Step 0: Run BigPool visibility test
    printf("[*] Running BigPool visibility test...\n");

    BIGPOOL_TEST_CONTEXT testCtx = {0};
    testCtx.hSupDrv = ctx->hSupDrv;

    if (!RunBigPoolTestInKernel(ctx, &testCtx.Result)) {
        printf("[-] BigPool test failed - assuming worst case\n");
        ctx->MappingStrategy = STRATEGY_EPT_ONLY;
    } else {
        ctx->MappingStrategy = DetermineMappingStrategy(&testCtx.Result);
    }

    const char* strategyNames[] = {
        "CLEAN_PATH (PageAllocEx)",
        "MDL_ALLOCATION (MmAllocatePagesForMdl)",
        "EPT_ONLY (Hypervisor-managed)"
    };
    printf("[+] Selected mapping strategy: %s\n",
           strategyNames[ctx->MappingStrategy]);

    // Step 1: Allocate driver memory using chosen strategy
    switch (ctx->MappingStrategy) {
        case STRATEGY_CLEAN_PATH:
            return AllocateViaPageAllocEx(ctx);

        case STRATEGY_MDL_ALLOCATION:
            return AllocateViaMdl(ctx);

        case STRATEGY_EPT_ONLY:
            return AllocateViaHypervisor(ctx);
    }

    return -1;
}
```

---

## Test Framework Skeleton

```c
// tests/test_framework.h

typedef enum _TEST_RESULT {
    TEST_PASS,
    TEST_FAIL,
    TEST_SKIP,
    TEST_ERROR,
} TEST_RESULT;

typedef struct _TEST_CASE {
    const char*     Name;
    TEST_RESULT     (*Run)(void* context);
    BOOLEAN         Critical;
} TEST_CASE;

static TEST_CASE g_Tests[] = {
    // Phase 2 prerequisites
    { "BigPool Visibility",       TestBigPoolVisibility,    TRUE  },
    { "SUPDrv Symbol Resolution", TestSymbolResolution,     TRUE  },
    { "PageAllocEx Basic",        TestPageAllocEx,          TRUE  },

    // Phase 3 prerequisites
    { "VMCALL Basic",             TestVmcallBasic,          TRUE  },
    { "Command Ring Init",        TestCommandRingInit,      TRUE  },
    { "Process Enumeration",      TestProcessEnum,          FALSE },
    { "Memory Read/Write",        TestMemoryOps,            FALSE },

    // Integration
    { "Full Workflow",            TestFullWorkflow,         FALSE },

    { NULL, NULL, FALSE }
};

int RunTestSuite(void* context) {
    int passed = 0, failed = 0, skipped = 0;
    BOOLEAN criticalFailed = FALSE;

    for (int i = 0; g_Tests[i].Name != NULL; i++) {
        if (criticalFailed && g_Tests[i].Critical) {
            printf("[SKIP] %s (blocked by critical failure)\n",
                   g_Tests[i].Name);
            skipped++;
            continue;
        }

        printf("[....] %s", g_Tests[i].Name);
        TEST_RESULT result = g_Tests[i].Run(context);

        switch (result) {
            case TEST_PASS:
                printf("\r[PASS] %s\n", g_Tests[i].Name);
                passed++;
                break;
            case TEST_FAIL:
                printf("\r[FAIL] %s\n", g_Tests[i].Name);
                failed++;
                if (g_Tests[i].Critical) criticalFailed = TRUE;
                break;
            case TEST_SKIP:
                printf("\r[SKIP] %s\n", g_Tests[i].Name);
                skipped++;
                break;
            case TEST_ERROR:
                printf("\r[ERR ] %s\n", g_Tests[i].Name);
                failed++;
                if (g_Tests[i].Critical) criticalFailed = TRUE;
                break;
        }
    }

    printf("\n========================================\n");
    printf("Results: %d passed, %d failed, %d skipped\n",
           passed, failed, skipped);
    printf("========================================\n");

    return failed > 0 ? -1 : 0;
}
```

---

## Detection Baseline Capture

```c
// tests/detection_baseline.h

typedef struct _DETECTION_BASELINE {
    // Before operations
    uint32_t    InitialBigPoolEntries;
    uint32_t    InitialVBoxTags;
    uint32_t    InitialDriverCount;
    uint64_t    InitialEtwState;

    // After operations
    uint32_t    FinalBigPoolEntries;
    uint32_t    FinalVBoxTags;
    uint32_t    FinalDriverCount;
    uint64_t    FinalEtwState;

    // Deltas
    int32_t     BigPoolDelta;
    int32_t     VBoxTagDelta;
    int32_t     DriverDelta;

    // Computed score (higher = more detectable)
    uint32_t    DetectionScore;

} DETECTION_BASELINE;

void CaptureBaseline(DETECTION_BASELINE* baseline, BOOLEAN initial) {
    if (initial) {
        baseline->InitialBigPoolEntries = CountBigPoolEntries();
        baseline->InitialVBoxTags = CountVBoxTags();
        baseline->InitialDriverCount = CountLoadedDrivers();
        baseline->InitialEtwState = GetEtwTiState();
    } else {
        baseline->FinalBigPoolEntries = CountBigPoolEntries();
        baseline->FinalVBoxTags = CountVBoxTags();
        baseline->FinalDriverCount = CountLoadedDrivers();
        baseline->FinalEtwState = GetEtwTiState();

        // Compute deltas
        baseline->BigPoolDelta =
            baseline->FinalBigPoolEntries - baseline->InitialBigPoolEntries;
        baseline->VBoxTagDelta =
            baseline->FinalVBoxTags - baseline->InitialVBoxTags;
        baseline->DriverDelta =
            baseline->FinalDriverCount - baseline->InitialDriverCount;

        // Score calculation
        baseline->DetectionScore = 0;
        if (baseline->BigPoolDelta > 0) baseline->DetectionScore += 20;
        if (baseline->VBoxTagDelta > 0) baseline->DetectionScore += 30;
        if (baseline->DriverDelta != 0) baseline->DetectionScore += 50;
    }
}

void PrintBaseline(DETECTION_BASELINE* b) {
    printf("[*] Detection Baseline:\n");
    printf("    BigPool entries: %d -> %d (delta: %+d)\n",
           b->InitialBigPoolEntries, b->FinalBigPoolEntries, b->BigPoolDelta);
    printf("    VBox tags: %d -> %d (delta: %+d)\n",
           b->InitialVBoxTags, b->FinalVBoxTags, b->VBoxTagDelta);
    printf("    Loaded drivers: %d -> %d (delta: %+d)\n",
           b->InitialDriverCount, b->FinalDriverCount, b->DriverDelta);
    printf("    Detection score: %u\n", b->DetectionScore);

    if (b->DetectionScore == 0) {
        printf("[+] Clean operation - no detectable artifacts\n");
    } else if (b->DetectionScore < 30) {
        printf("[*] Low detection risk\n");
    } else if (b->DetectionScore < 60) {
        printf("[!] Moderate detection risk\n");
    } else {
        printf("[!] HIGH detection risk\n");
    }
}
```

---

## Implementation Tasks

| Task | Description | Files | Effort |
|------|-------------|-------|--------|
| 4A.1 | BigPool test kernel function | `tests/bigpool_test_kernel.c` | 1 task |
| 4A.2 | BigPool test usermode driver | `tests/bigpool_test.c`, `tests/bigpool_test.h` | 1 task |
| 4A.3 | Integration with Phase 2 loader | `loader/hv_loader.c` | 0.5 task |
| 4A.4 | Test framework skeleton | `tests/test_framework.c`, `tests/test_framework.h` | 0.5 task |
| 4A.5 | Detection baseline capture | `tests/detection_baseline.c`, `tests/detection_baseline.h` | 0.5 task |
| **Total** | | | **3.5 tasks** |

---

## Files to Create

```
hypervisor/usermode/
├── tests/
│   ├── bigpool_test.h
│   ├── bigpool_test.c
│   ├── bigpool_test_kernel.c
│   ├── test_framework.h
│   ├── test_framework.c
│   ├── detection_baseline.h
│   └── detection_baseline.c
└── main.c (add --test flag)
```

---

## Implementation Notes

### Position-Independent Kernel Code

The BigPool test kernel function must be position-independent since it's executed in an arbitrary kernel context via SUPDrv. Options:

1. **Use SUPDrv's existing LDR_LOAD mechanism** - Load a minimal test module
2. **Compile as shellcode** - PIC with no relocations
3. **Use inline assembly** - Avoid compiler-generated relocations

Recommended: Option 1 (LDR_LOAD) is cleanest since we're already using that path.

### Symbol Resolution

The test requires resolving `nt!PoolBigPageTable` and `nt!PoolBigPageTableSize`. SUPDrv's `LDR_GET_SYMBOL` should support these as they're exported via PDB symbols.

If symbol resolution fails:
- Fall back to pattern scanning (risky, version-dependent)
- Or assume worst case (BigPool visible) and use mitigation path

### Test Timing

Run the test early in the Phase 2 sequence, before any significant allocations. The hypervisor should already be loaded (Phase 1 complete) but the driver pool should not yet be allocated.

---

## Success Criteria

1. **BigPool test executes** - Kernel code runs, symbols resolve
2. **Allocation succeeds** - PageAllocEx returns valid R0/R3 pointers
3. **Visibility determined** - Clear YES/NO answer on BigPool tracking
4. **Strategy selected** - CLEAN_PATH, MDL, or EPT_ONLY chosen
5. **Test framework runs** - All critical tests pass or skip appropriately

---

## Next Phase

After Phase 4A completes:

- **If BigPool NOT visible:** Proceed to Phase 2 implementation with clean path
- **If BigPool IS visible:** Implement MDL or EPT-only allocation in Phase 2

Then continue to **Phase 4B: Usermode Orchestrator** - the main application tying all phases together.
