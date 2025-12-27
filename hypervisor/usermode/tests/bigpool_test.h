/**
 * @file bigpool_test.h
 * @brief BigPool Visibility Test - Determines if PageAllocEx is detectable
 *
 * Phase 4A: Testing Infrastructure
 *
 * This test empirically determines whether SUPDrv's PageAllocEx allocations
 * appear in Windows' PoolBigPageTable, which would make them visible to
 * anti-cheat detection.
 *
 * CRITICAL: Results determine Phase 2 mapping strategy:
 *   - NOT VISIBLE: Clean path (PageAllocEx) is safe
 *   - VISIBLE:     Must use MDL allocation or EPT-only memory
 */

#ifndef BIGPOOL_TEST_H
#define BIGPOOL_TEST_H

#include <Windows.h>
#include <stdint.h>
#include <stdbool.h>
#include "../driver_interface.h"

//=============================================================================
// Test Result Structure - Returned from kernel test
//=============================================================================
//
// CRITICAL: This structure MUST match BIGPOOL_TEST_RESULT in bigpool_test_kernel.c
// Any mismatch will cause data corruption when copying results from kernel.
//

#pragma pack(push, 8)  // Match kernel default packing

typedef struct _BIGPOOL_TEST_RESULT {
    // Test completion status
    uint8_t     Success;                // Did test complete without errors? (BOOLEAN)
    uint8_t     _pad1[3];               // Padding for NTSTATUS alignment
    int32_t     ErrorCode;              // NTSTATUS: Failure reason if Success == FALSE

    // Baseline scan results
    uint32_t    BaselineVBoxEntries;    // Number of VBox tags found before test
    uint8_t     VBoxPresent;            // Were any VBox signatures detected? (BOOLEAN)
    uint8_t     _pad2[3];               // Padding for pointer alignment

    // Test allocation details
    uint64_t    TestAllocationVa;       // Virtual address of our test page (PVOID)
    uint64_t    TestAllocationSize;     // Size allocated (SIZE_T, should be PAGE_SIZE)
    uint32_t    TestPoolTag;            // Tag we used ('xoBV' to mimic VBox)

    // Visibility results - THE KEY RESULT
    uint8_t     FoundInTable;           // Did our allocation appear in PoolBigPageTable? (BOOLEAN)
    uint8_t     _pad3[3];               // Padding for ULONG alignment
    uint32_t    TableIndexIfFound;      // Index where found (0xFFFFFFFF if not found)

    // Timing data (for stealth audit)
    uint64_t    ScanTimeMicroseconds;   // Time to scan entire table (ULONGLONG)
    uint32_t    TotalTableEntries;      // PoolBigPageTableSize value
    uint32_t    _pad4;                  // Final padding for 8-byte alignment

} BIGPOOL_TEST_RESULT;

#pragma pack(pop)

// Compile-time size verification (should match kernel struct)
_Static_assert(sizeof(BIGPOOL_TEST_RESULT) == 56, "BIGPOOL_TEST_RESULT size mismatch with kernel!");

//=============================================================================
// Test Context - Holds session and result data
//=============================================================================

typedef struct _BIGPOOL_TEST_CONTEXT {
    DRV_CONTEXT*        pDrvCtx;        // Driver session context
    BIGPOOL_TEST_RESULT Result;         // Test results
    bool                TestCompleted;  // Test successfully ran
} BIGPOOL_TEST_CONTEXT;

//=============================================================================
// Mapping Strategy Decision
//=============================================================================

typedef enum _DRIVER_MAPPING_STRATEGY {
    STRATEGY_CLEAN_PATH,        // PageAllocEx directly (not detectable)
    STRATEGY_MDL_ALLOCATION,    // MmAllocatePagesForMdl (mitigation)
    STRATEGY_EPT_ONLY,          // Hypervisor-managed memory (most robust)
} DRIVER_MAPPING_STRATEGY;

//=============================================================================
// Function Declarations
//=============================================================================

/**
 * Run the BigPool visibility test
 *
 * Executes kernel-mode test via SUPDrv to determine if PageAllocEx
 * allocations appear in PoolBigPageTable.
 *
 * @param ctx Test context (input: pDrvCtx, output: Result, TestCompleted)
 * @return true if test executed successfully
 */
bool RunBigPoolTest(BIGPOOL_TEST_CONTEXT* ctx);

/**
 * Print detailed test results
 *
 * Displays all test data including:
 * - Symbol resolution status
 * - Table statistics
 * - Allocation details
 * - Visibility verdict
 * - Recommended strategy
 *
 * @param result Test result structure
 */
void PrintTestResults(const BIGPOOL_TEST_RESULT* result);

/**
 * Determine mapping strategy based on test results
 *
 * Analyzes test results and returns the appropriate driver mapping
 * strategy for Phase 2 implementation.
 *
 * @param result Test result structure
 * @return Recommended mapping strategy
 */
DRIVER_MAPPING_STRATEGY DetermineMappingStrategy(const BIGPOOL_TEST_RESULT* result);

#endif // BIGPOOL_TEST_H
