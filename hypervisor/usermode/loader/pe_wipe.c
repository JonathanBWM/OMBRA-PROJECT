#include "pe_wipe.h"
#include <stdio.h>
#include <intrin.h>

void WipePeHeaders(void* mappedImage, uint32_t headerSize) {
    printf("[*] Wiping PE headers (%u bytes)...\n", headerSize);

    // Use volatile to prevent compiler optimization
    volatile uint8_t* p = (volatile uint8_t*)mappedImage;

    // Zero entire header region
    for (uint32_t i = 0; i < headerSize; i++) {
        p[i] = 0;
    }

    printf("[+] PE headers wiped\n");
}

void WipePeHeadersRandom(void* mappedImage, uint32_t headerSize) {
    printf("[*] Wiping PE headers with random data (%u bytes)...\n", headerSize);

    volatile uint8_t* p = (volatile uint8_t*)mappedImage;

    // Use RDTSC for seed - unpredictable but fast
    uint32_t seed = (uint32_t)__rdtsc();

    // Simple LCG PRNG
    for (uint32_t i = 0; i < headerSize; i++) {
        seed = seed * 1103515245 + 12345;
        p[i] = (uint8_t)(seed >> 16);
    }

    printf("[+] PE headers wiped with random data\n");
}
