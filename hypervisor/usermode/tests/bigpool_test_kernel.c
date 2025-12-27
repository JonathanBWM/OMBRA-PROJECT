/*
 * BigPool Test Kernel Function
 *
 * Anti-detection research: Tests whether allocations made via PoolBigPageTable
 * manipulation are visible in the BigPool tracking structure. This determines if
 * we can hide hypervisor memory from pool scanning tools.
 *
 * Strategy:
 * 1. Resolve PoolBigPageTable and PoolBigPageTableSize from ntoskrnl
 * 2. Scan baseline table for existing VirtualBox signatures
 * 3. Allocate test page via PageAllocEx with VBox tag
 * 4. Search table again to see if our allocation appears
 * 5. Record visibility result and timing data
 *
 * If allocation is NOT visible in table, we can use this technique to hide
 * hypervisor memory from pool scanners (anti-cheat detection vectors).
 */

#include <ntifs.h>
#include <ntddk.h>

// ============================================================================
// KERNEL STRUCTURES
// ============================================================================

/*
 * POOL_TRACKER_BIG_PAGES - Internal Windows structure tracking large pool allocations
 *
 * Location: ntoskrnl!PoolBigPageTable (array)
 * Size: ntoskrnl!PoolBigPageTableSize (ULONG)
 *
 * Each entry tracks one "big" pool allocation (>PAGE_SIZE on x64).
 * Anti-cheat software scans this table looking for hypervisor signatures.
 */
typedef struct _POOL_TRACKER_BIG_PAGES {
    PVOID Va;               // Virtual address of allocation
    ULONG Key;              // Pool tag (4-byte signature like 'xoBV' for VirtualBox)
    ULONG PoolType;         // NonPagedPool, PagedPool, etc.
    SIZE_T NumberOfBytes;   // Allocation size in bytes
} POOL_TRACKER_BIG_PAGES, *PPOOL_TRACKER_BIG_PAGES;

/*
 * BIGPOOL_TEST_RESULT - Results from BigPool visibility test
 *
 * This structure is returned to usermode via BYOVD IOCTL.
 * It tells us whether allocations are trackable by pool scanners.
 */
typedef struct _BIGPOOL_TEST_RESULT {
    BOOLEAN Success;                // Did test complete without errors?
    NTSTATUS ErrorCode;             // Failure reason if Success == FALSE

    // Baseline scan results
    ULONG BaselineVBoxEntries;      // Number of VBox tags found before test
    BOOLEAN VBoxPresent;            // Were any VBox signatures detected?

    // Test allocation details
    PVOID TestAllocationVa;         // Virtual address of our test page
    SIZE_T TestAllocationSize;      // Size allocated (should be PAGE_SIZE)
    ULONG TestPoolTag;              // Tag we used ('xoBV' to mimic VBox)

    // Visibility results
    BOOLEAN FoundInTable;           // Did our allocation appear in PoolBigPageTable?
    ULONG TableIndexIfFound;        // Index where found (0xFFFFFFFF if not found)

    // Timing data (for stealth audit)
    ULONGLONG ScanTimeMicroseconds; // Time to scan entire table
    ULONG TotalTableEntries;        // PoolBigPageTableSize value
} BIGPOOL_TEST_RESULT, *PBIGPOOL_TEST_RESULT;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/*
 * IsVBoxTag - Check if pool tag matches known VirtualBox signatures
 *
 * VirtualBox uses several pool tags that anti-cheat software looks for:
 * - 'xoBV' / 'XoBV' - VBoxDrv.sys main tag
 * - 'MMoV' - VBox memory manager
 * - 'rDpS' - VBox supplemental driver
 * - 'gMpS' - VBox graphics
 *
 * We use 'xoBV' for testing because it's the most commonly scanned.
 */
static BOOLEAN IsVBoxTag(ULONG Tag)
{
    // VBoxDrv.sys main tags
    if (Tag == 'xoBV' || Tag == 'XoBV') return TRUE;

    // VBox memory manager
    if (Tag == 'MMoV') return TRUE;

    // VBox supplemental driver
    if (Tag == 'rDpS') return TRUE;

    // VBox graphics
    if (Tag == 'gMpS') return TRUE;

    return FALSE;
}

/*
 * CountVBoxEntries - Scan PoolBigPageTable for VirtualBox signatures
 *
 * This establishes a baseline count before we make our test allocation.
 * If VBox is actually installed, we'll see these entries. If not, count should be 0.
 *
 * Returns: Number of entries with VBox tags
 */
static ULONG CountVBoxEntries(
    PPOOL_TRACKER_BIG_PAGES Table,
    ULONG TableSize
)
{
    ULONG Count = 0;

    for (ULONG i = 0; i < TableSize; i++) {
        // Skip empty entries (Va == NULL means slot is free)
        if (Table[i].Va == NULL) {
            continue;
        }

        // Check if tag matches known VBox signatures
        if (IsVBoxTag(Table[i].Key)) {
            Count++;
        }
    }

    return Count;
}

/*
 * FindAllocationInTable - Search for specific allocation in PoolBigPageTable
 *
 * Args:
 *   Table - Pointer to PoolBigPageTable array
 *   TableSize - Number of entries in table
 *   Va - Virtual address to search for
 *   OutIndex - Receives index if found (optional)
 *
 * Returns: TRUE if allocation found, FALSE otherwise
 *
 * This is the critical test: if our allocation appears here, it's visible
 * to pool scanners. If it doesn't appear, we've successfully hidden it.
 */
static BOOLEAN FindAllocationInTable(
    PPOOL_TRACKER_BIG_PAGES Table,
    ULONG TableSize,
    PVOID Va,
    PULONG OutIndex
)
{
    for (ULONG i = 0; i < TableSize; i++) {
        // Compare virtual addresses
        if (Table[i].Va == Va) {
            // Found it - allocation IS visible in tracking table
            if (OutIndex != NULL) {
                *OutIndex = i;
            }
            return TRUE;
        }
    }

    // Not found - allocation is hidden from pool scanners
    if (OutIndex != NULL) {
        *OutIndex = 0xFFFFFFFF;
    }
    return FALSE;
}

/*
 * GetTimestamp - High-resolution timestamp for timing measurements
 *
 * Returns: Timestamp in 100-nanosecond units
 *
 * Used to measure scan performance. If scanning is too slow, it could
 * impact game performance and be detectable via timing side channels.
 */
static ULONGLONG GetTimestamp(VOID)
{
    LARGE_INTEGER Time;
    KeQuerySystemTime(&Time);
    return Time.QuadPart;
}

// ============================================================================
// MAIN TEST FUNCTION
// ============================================================================

/*
 * BigPoolTestKernel - Execute BigPool visibility test in kernel context
 *
 * This function runs in kernel mode (executed via BYOVD IOCTL).
 * It tests whether allocations made via direct page allocation appear
 * in the PoolBigPageTable tracking structure.
 *
 * Args:
 *   Result - Pointer to result structure (kernel memory)
 *
 * Returns:
 *   STATUS_SUCCESS - Test completed (check Result->Success for test outcome)
 *   STATUS_NOT_FOUND - Couldn't resolve kernel symbols
 *   STATUS_INSUFFICIENT_RESOURCES - Allocation failed
 *
 * CRITICAL: This function must NOT crash. Wrap everything in __try/__except.
 */
NTSTATUS BigPoolTestKernel(
    PBIGPOOL_TEST_RESULT Result
)
{
    NTSTATUS Status;
    PPOOL_TRACKER_BIG_PAGES PoolBigPageTable = NULL;
    ULONG PoolBigPageTableSize = 0;
    PVOID TestAllocation = NULL;
    ULONGLONG StartTime, EndTime;

    // Zero out result structure
    RtlZeroMemory(Result, sizeof(BIGPOOL_TEST_RESULT));

    __try {

        // ====================================================================
        // STEP 1: Resolve kernel symbols
        // ====================================================================

        /*
         * PoolBigPageTable - Array of POOL_TRACKER_BIG_PAGES structures
         * PoolBigPageTableSize - ULONG count of entries in array
         *
         * These are not exported by ntoskrnl, so we must resolve them
         * via pattern scanning or PDB symbols. For now, assume they're
         * resolved by the caller and passed via some mechanism (global, etc).
         *
         * TODO: Integrate with symbol resolution system
         */

        // Placeholder: In real implementation, resolve these symbols
        // For now, return error if symbols not available

        // Example symbol resolution (pseudo-code):
        // PoolBigPageTable = (PPOOL_TRACKER_BIG_PAGES)ResolveKernelSymbol("PoolBigPageTable");
        // PoolBigPageTableSize = *(PULONG)ResolveKernelSymbol("PoolBigPageTableSize");

        if (PoolBigPageTable == NULL || PoolBigPageTableSize == 0) {
            Result->Success = FALSE;
            Result->ErrorCode = STATUS_NOT_FOUND;
            return STATUS_NOT_FOUND;
        }

        // ====================================================================
        // STEP 2: Baseline scan - count existing VBox entries
        // ====================================================================

        /*
         * Before we make our test allocation, scan the table to see if
         * VirtualBox is actually present on the system. This gives us:
         *
         * 1. Baseline count to compare against after our allocation
         * 2. Indication of whether VBox drivers are loaded
         * 3. Performance baseline for scan timing
         */

        StartTime = GetTimestamp();

        Result->BaselineVBoxEntries = CountVBoxEntries(
            PoolBigPageTable,
            PoolBigPageTableSize
        );

        EndTime = GetTimestamp();

        Result->VBoxPresent = (Result->BaselineVBoxEntries > 0);
        Result->TotalTableEntries = PoolBigPageTableSize;

        // Calculate scan time in microseconds
        Result->ScanTimeMicroseconds = (EndTime - StartTime) / 10;

        // ====================================================================
        // STEP 3: Allocate test page with VBox tag
        // ====================================================================

        /*
         * Allocate a single page using MmAllocateContiguousMemory or
         * ExAllocatePoolWithTag. We tag it with 'xoBV' to mimic VirtualBox.
         *
         * The key question: Will this allocation appear in PoolBigPageTable?
         *
         * If YES: Pool scanners will see it (bad for stealth)
         * If NO: We can hide hypervisor memory (good for stealth)
         */

        Result->TestPoolTag = 'xoBV';  // VirtualBox main tag
        Result->TestAllocationSize = PAGE_SIZE;

        TestAllocation = ExAllocatePoolWithTag(
            NonPagedPoolExecute,
            PAGE_SIZE,
            Result->TestPoolTag
        );

        if (TestAllocation == NULL) {
            Result->Success = FALSE;
            Result->ErrorCode = STATUS_INSUFFICIENT_RESOURCES;
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        Result->TestAllocationVa = TestAllocation;

        // Initialize memory to avoid page faults during scan
        RtlZeroMemory(TestAllocation, PAGE_SIZE);

        // ====================================================================
        // STEP 4: Search table for our allocation
        // ====================================================================

        /*
         * This is the moment of truth. We scan PoolBigPageTable looking
         * for our allocation's virtual address.
         *
         * If found: Allocation IS tracked (visible to anti-cheat)
         * If not found: Allocation is NOT tracked (hidden from anti-cheat)
         *
         * Note: We must allow time for Windows to update the table.
         * Some allocations may be added lazily or batched.
         */

        // Small delay to ensure table is updated (if it's going to be)
        LARGE_INTEGER Interval;
        Interval.QuadPart = -10000;  // 1ms in 100ns units
        KeDelayExecutionThread(KernelMode, FALSE, &Interval);

        Result->FoundInTable = FindAllocationInTable(
            PoolBigPageTable,
            PoolBigPageTableSize,
            TestAllocation,
            &Result->TableIndexIfFound
        );

        // ====================================================================
        // STEP 5: Cleanup and finalize results
        // ====================================================================

        /*
         * Free the test allocation and mark test as successful.
         *
         * The Result structure now contains:
         * - Whether allocation was visible in tracking table
         * - Baseline VBox entry count
         * - Timing data for performance analysis
         * - All allocation details for logging
         */

        if (TestAllocation != NULL) {
            ExFreePoolWithTag(TestAllocation, Result->TestPoolTag);
            TestAllocation = NULL;
        }

        Result->Success = TRUE;
        Result->ErrorCode = STATUS_SUCCESS;

        return STATUS_SUCCESS;

    } __except(EXCEPTION_EXECUTE_HANDLER) {

        // ====================================================================
        // EXCEPTION HANDLER - Cleanup on crash
        // ====================================================================

        /*
         * If we crash during the test, clean up any allocations and
         * return error status. This prevents kernel memory leaks and
         * system instability.
         */

        if (TestAllocation != NULL) {
            __try {
                ExFreePoolWithTag(TestAllocation, 'xoBV');
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                // Double fault - can't even free memory
                // System may be unstable, but at least we tried
            }
        }

        Result->Success = FALSE;
        Result->ErrorCode = STATUS_UNHANDLED_EXCEPTION;

        return STATUS_UNHANDLED_EXCEPTION;
    }
}

// ============================================================================
// INTEGRATION NOTES
// ============================================================================

/*
 * To integrate this test into the BYOVD framework:
 *
 * 1. Symbol Resolution:
 *    - Resolve PoolBigPageTable and PoolBigPageTableSize via PDB or patterns
 *    - Store resolved addresses in global variables
 *    - Call this function after symbols are resolved
 *
 * 2. IOCTL Handler:
 *    - Allocate BIGPOOL_TEST_RESULT in kernel memory
 *    - Call BigPoolTestKernel(&Result)
 *    - Copy result back to usermode via ProbeForWrite/RtlCopyMemory
 *
 * 3. Usermode Client:
 *    - Send IOCTL to trigger test
 *    - Receive BIGPOOL_TEST_RESULT structure
 *    - Log results to file or display
 *
 * 4. Interpretation:
 *    - If FoundInTable == TRUE: Allocations ARE tracked (use different method)
 *    - If FoundInTable == FALSE: Allocations NOT tracked (safe to use)
 *    - Compare ScanTimeMicroseconds against detection thresholds
 *
 * 5. Follow-up Tests:
 *    - Test with different pool types (Paged vs NonPaged)
 *    - Test with different allocation sizes
 *    - Test with different pool tags (custom vs VBox vs none)
 *    - Test with MmAllocateContiguousMemory instead of ExAllocatePool
 */

// ============================================================================
// EXPECTED RESULTS
// ============================================================================

/*
 * Windows 10/11 Expected Behavior:
 *
 * ExAllocatePoolWithTag:
 *   - Small allocations (<PAGE_SIZE): NOT in PoolBigPageTable
 *   - Large allocations (>=PAGE_SIZE): SHOULD appear in PoolBigPageTable
 *   - Result: FoundInTable == TRUE (visible to scanners)
 *
 * MmAllocateContiguousMemory:
 *   - May or may not appear in PoolBigPageTable (version dependent)
 *   - Result: Needs empirical testing
 *
 * Direct PTE Manipulation (future):
 *   - If we allocate via direct page table modification, bypassing pool APIs
 *   - Result: Should NOT appear in PoolBigPageTable (hidden from scanners)
 *
 * Detection Implications:
 *
 * If our hypervisor memory appears in PoolBigPageTable:
 *   - Anti-cheat can scan for our pool tags
 *   - We must either use untagged allocations or hide from table
 *   - Consider using VirtualBox tags to blend in (if VBox present)
 *
 * If our hypervisor memory does NOT appear:
 *   - Pool scanning is ineffective against our hypervisor
 *   - Focus anti-detection efforts elsewhere (CPUID, timing, MSRs)
 */
