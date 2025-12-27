#ifndef PE_IAT_H
#define PE_IAT_H

#include <stdint.h>
#include <stdbool.h>
#include "pe_parser.h"

// Patch IAT entries with resolved addresses
// mappedImage: pointer to the mapped PE in memory
// peInfo: parsed PE with imports that have ResolvedAddress filled
// Returns false if any import is unresolved (ResolvedAddress == 0)
bool PatchIAT(void* mappedImage, PE_INFO* peInfo);

#endif
