#ifndef PE_RELOCS_H
#define PE_RELOCS_H

#include <stdint.h>
#include <stdbool.h>
#include "pe_parser.h"

// Apply relocations to mapped image
// mappedImage: pointer to the mapped PE in memory
// peInfo: parsed PE information including RelocRva/RelocSize
// newBase: the actual kernel address where image will run
bool ApplyRelocations(void* mappedImage, PE_INFO* peInfo, uint64_t newBase);

#endif
