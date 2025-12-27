// test_rdtscp.c — RDTSCP Handler Test & Validation
// OmbraHypervisor
//
// This test validates RDTSCP implementation for:
// 1. Correct TSC value return (with compensation)
// 2. Correct IA32_TSC_AUX (processor ID) return
// 3. Virtualization of IA32_TSC_AUX MSR
// 4. Timing consistency (no detection vectors)

#include <stdio.h>
#include <stdint.h>
#include <intrin.h>

// MSR definitions
#define MSR_IA32_TSC_AUX 0xC0000103

// Test processor ID value
#define TEST_PROC_ID 0x12345678

// =============================================================================
// Test 1: Basic RDTSCP Functionality
// =============================================================================

void test_rdtscp_basic(void) {
    uint64_t tsc1, tsc2;
    uint32_t aux1, aux2;

    printf("[TEST 1] Basic RDTSCP functionality\n");

    // Execute RDTSCP twice
    tsc1 = __rdtscp(&aux1);
    tsc2 = __rdtscp(&aux2);

    printf("  TSC1: 0x%llx, AUX1: 0x%x\n", tsc1, aux1);
    printf("  TSC2: 0x%llx, AUX2: 0x%x\n", tsc2, aux2);

    // Validate monotonic TSC
    if (tsc2 > tsc1) {
        printf("  [PASS] TSC is monotonic\n");
    } else {
        printf("  [FAIL] TSC went backwards!\n");
    }

    // Validate consistent processor ID
    if (aux1 == aux2) {
        printf("  [PASS] Processor ID is consistent\n");
    } else {
        printf("  [FAIL] Processor ID changed!\n");
    }

    printf("\n");
}

// =============================================================================
// Test 2: IA32_TSC_AUX Virtualization
// =============================================================================

void test_tsc_aux_virtualization(void) {
    uint32_t originalAux, testAux;
    uint64_t tsc;

    printf("[TEST 2] IA32_TSC_AUX Virtualization\n");

    // Read original value
    tsc = __rdtscp(&originalAux);
    printf("  Original AUX: 0x%x\n", originalAux);

    // Write test value to IA32_TSC_AUX
    __writemsr(MSR_IA32_TSC_AUX, TEST_PROC_ID);
    printf("  Wrote AUX: 0x%x\n", TEST_PROC_ID);

    // Read it back via RDTSCP
    tsc = __rdtscp(&testAux);
    printf("  Read back AUX: 0x%x\n", testAux);

    if (testAux == TEST_PROC_ID) {
        printf("  [PASS] IA32_TSC_AUX is properly virtualized\n");
    } else {
        printf("  [FAIL] IA32_TSC_AUX not virtualized (expected 0x%x, got 0x%x)\n",
               TEST_PROC_ID, testAux);
    }

    // Read via RDMSR to verify MSR virtualization
    uint64_t msrValue = __readmsr(MSR_IA32_TSC_AUX);
    printf("  RDMSR(IA32_TSC_AUX): 0x%llx\n", msrValue);

    if ((uint32_t)msrValue == TEST_PROC_ID) {
        printf("  [PASS] RDMSR returns virtualized value\n");
    } else {
        printf("  [FAIL] RDMSR doesn't match (expected 0x%x, got 0x%x)\n",
               TEST_PROC_ID, (uint32_t)msrValue);
    }

    // Restore original value
    __writemsr(MSR_IA32_TSC_AUX, originalAux);

    printf("\n");
}

// =============================================================================
// Test 3: Timing Consistency (Anti-Detection)
// =============================================================================

void test_timing_consistency(void) {
    uint64_t tsc[100];
    uint32_t aux[100];
    uint64_t deltas[99];
    uint64_t avgDelta = 0;
    uint64_t minDelta = UINT64_MAX;
    uint64_t maxDelta = 0;

    printf("[TEST 3] Timing Consistency (Anti-Detection)\n");

    // Collect 100 RDTSCP samples
    for (int i = 0; i < 100; i++) {
        tsc[i] = __rdtscp(&aux[i]);
    }

    // Calculate deltas
    for (int i = 0; i < 99; i++) {
        deltas[i] = tsc[i+1] - tsc[i];
        avgDelta += deltas[i];
        if (deltas[i] < minDelta) minDelta = deltas[i];
        if (deltas[i] > maxDelta) maxDelta = deltas[i];
    }
    avgDelta /= 99;

    printf("  Min delta: %llu cycles\n", minDelta);
    printf("  Avg delta: %llu cycles\n", avgDelta);
    printf("  Max delta: %llu cycles\n", maxDelta);

    // Check for detection vectors
    // RDTSCP back-to-back should be < 100 cycles on native
    // > 150 cycles suggests VM-exit overhead
    if (avgDelta < 100) {
        printf("  [PASS] Timing looks native (avg < 100 cycles)\n");
    } else if (avgDelta < 150) {
        printf("  [WARN] Timing borderline (100-150 cycles) - may be detectable\n");
    } else {
        printf("  [FAIL] Timing too slow (avg > 150 cycles) - DETECTED!\n");
    }

    // Check for jitter (should be consistent)
    uint64_t jitter = maxDelta - minDelta;
    printf("  Jitter: %llu cycles\n", jitter);

    if (jitter < 50) {
        printf("  [PASS] Low jitter - consistent timing\n");
    } else {
        printf("  [WARN] High jitter - may indicate VM-exits\n");
    }

    printf("\n");
}

// =============================================================================
// Test 4: RDTSC vs RDTSCP Consistency
// =============================================================================

void test_rdtsc_rdtscp_consistency(void) {
    uint64_t tsc1, tsc2, tscp;
    uint32_t aux;

    printf("[TEST 4] RDTSC vs RDTSCP Consistency\n");

    // Execute RDTSC, RDTSCP, RDTSC
    tsc1 = __rdtsc();
    tscp = __rdtscp(&aux);
    tsc2 = __rdtsc();

    printf("  RDTSC (before): 0x%llx\n", tsc1);
    printf("  RDTSCP:         0x%llx (AUX: 0x%x)\n", tscp, aux);
    printf("  RDTSC (after):  0x%llx\n", tsc2);

    // All should be monotonic
    if (tsc1 < tscp && tscp < tsc2) {
        printf("  [PASS] All TSC values are monotonic\n");
    } else {
        printf("  [FAIL] TSC values not monotonic!\n");
    }

    // Deltas should be reasonable (< 200 cycles each)
    uint64_t delta1 = tscp - tsc1;
    uint64_t delta2 = tsc2 - tscp;

    printf("  Delta1 (RDTSC->RDTSCP): %llu cycles\n", delta1);
    printf("  Delta2 (RDTSCP->RDTSC): %llu cycles\n", delta2);

    if (delta1 < 200 && delta2 < 200) {
        printf("  [PASS] Deltas look reasonable\n");
    } else {
        printf("  [WARN] Large deltas - may indicate VM-exits\n");
    }

    printf("\n");
}

// =============================================================================
// Main Test Runner
// =============================================================================

int main(void) {
    printf("=============================================================================\n");
    printf("RDTSCP Handler Test Suite - OmbraHypervisor\n");
    printf("=============================================================================\n\n");

    test_rdtscp_basic();
    test_tsc_aux_virtualization();
    test_timing_consistency();
    test_rdtsc_rdtscp_consistency();

    printf("=============================================================================\n");
    printf("Test suite complete\n");
    printf("=============================================================================\n");

    return 0;
}
