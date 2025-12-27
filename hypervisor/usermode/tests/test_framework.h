#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdbool.h>

/**
 * Test result codes
 */
typedef enum _TEST_RESULT {
    TEST_PASS = 0,      // Test passed successfully
    TEST_FAIL = 1,      // Test failed (expected behavior not met)
    TEST_SKIP = 2,      // Test skipped (dependencies not met)
    TEST_ERROR = 3      // Test error (unexpected exception/crash)
} TEST_RESULT;

/**
 * Test case structure
 */
typedef struct _TEST_CASE {
    const char* Name;
    TEST_RESULT (*Run)(void);
    bool Critical;      // If true, failure skips subsequent critical tests
} TEST_CASE;

/**
 * Main test suite runner
 *
 * Returns: Number of failed tests (0 = all passed)
 */
int RunTestSuite(void);

/**
 * Phase 2: BYOVD Validation Tests
 */
TEST_RESULT TestBigPoolVisibility(void);
TEST_RESULT TestSymbolResolution(void);
TEST_RESULT TestPageAllocEx(void);

/**
 * Phase 3: Hypervisor Communication Tests
 */
TEST_RESULT TestVmcallBasic(void);
TEST_RESULT TestCommandRingInit(void);
TEST_RESULT TestProcessEnum(void);
TEST_RESULT TestMemoryOps(void);

/**
 * Integration Test
 */
TEST_RESULT TestFullWorkflow(void);

#endif // TEST_FRAMEWORK_H
