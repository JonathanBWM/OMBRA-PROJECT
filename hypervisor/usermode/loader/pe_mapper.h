#ifndef PE_MAPPER_H
#define PE_MAPPER_H

#include <stdint.h>
#include <stdbool.h>
#include "pe_parser.h"

// Map driver sections to target memory
// peImage: original PE file buffer
// targetR3: Ring-3 (usermode) mapping of kernel memory
// targetR0: Ring-0 kernel virtual address (for relocations)
// peInfo: parsed PE information with resolved imports
//
// This function:
// 1. Zeros the target region
// 2. Copies PE headers (will be wiped later)
// 3. Copies each section to its correct RVA
// 4. Applies relocations based on targetR0
// 5. Patches IAT with resolved imports
// 6. Wipes PE headers
bool MapDriver(
    const void* peImage,
    void* targetR3,
    uint64_t targetR0,
    PE_INFO* peInfo
);

#endif
