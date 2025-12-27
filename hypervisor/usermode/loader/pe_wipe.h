#ifndef PE_WIPE_H
#define PE_WIPE_H

#include <stdint.h>

// Zero PE headers to avoid MZ/PE signature detection
// mappedImage: pointer to the mapped PE in memory
// headerSize: size of headers to wipe (typically SizeOfHeaders from PE)
void WipePeHeaders(void* mappedImage, uint32_t headerSize);

// More aggressive: fill with random data instead of zeros
void WipePeHeadersRandom(void* mappedImage, uint32_t headerSize);

#endif
