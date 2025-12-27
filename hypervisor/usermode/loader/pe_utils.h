#ifndef PE_UTILS_H
#define PE_UTILS_H

#include <stdint.h>
#include <stdbool.h>

// Find a named section in a PE image
// Returns true if found, fills offset and size
bool PeFindSection(
    const void* peImage,
    uint32_t imageSize,         // For bounds validation
    const char* sectionName,    // e.g., ".ombra"
    uint32_t* outFileOffset,    // Offset in file
    uint32_t* outSize           // Size of section
);

// Get entry point RVA from PE
bool PeGetEntryPoint(
    const void* peImage,
    uint32_t imageSize,         // For bounds validation
    uint32_t* outEntryRva
);

// Validate PE image
bool PeValidate(const void* peImage, uint32_t imageSize);

#endif // PE_UTILS_H
