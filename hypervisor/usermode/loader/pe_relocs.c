#include "pe_relocs.h"
#include <stdio.h>
#include <Windows.h>

#define IMAGE_REL_BASED_DIR64 10

bool ApplyRelocations(void* mappedImage, PE_INFO* peInfo, uint64_t newBase) {
    // Check if relocations exist
    if (peInfo->RelocRva == 0 || peInfo->RelocSize == 0) {
        printf("[*] No relocations to apply\n");
        return true;
    }

    // Calculate delta from preferred base
    int64_t delta = (int64_t)(newBase - peInfo->ImageBase);
    if (delta == 0) {
        printf("[*] Image at preferred base, no relocations needed\n");
        return true;
    }

    printf("[*] Applying relocations (delta = 0x%llX)\n", delta);

    // Walk relocation blocks
    uint8_t* relocBase = (uint8_t*)mappedImage + peInfo->RelocRva;
    uint8_t* relocEnd = relocBase + peInfo->RelocSize;

    uint32_t blockCount = 0;
    uint32_t relocCount = 0;

    while (relocBase < relocEnd) {
        IMAGE_BASE_RELOCATION* block = (IMAGE_BASE_RELOCATION*)relocBase;

        if (block->SizeOfBlock == 0) break;

        uint32_t entryCount = (block->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / 2;
        uint16_t* entries = (uint16_t*)(block + 1);

        for (uint32_t i = 0; i < entryCount; i++) {
            uint16_t entry = entries[i];
            uint16_t type = entry >> 12;
            uint16_t offset = entry & 0xFFF;

            if (type == IMAGE_REL_BASED_ABSOLUTE) continue;  // Padding

            uint8_t* targetAddr = (uint8_t*)mappedImage + block->VirtualAddress + offset;

            if (type == IMAGE_REL_BASED_DIR64) {
                int64_t* target = (int64_t*)targetAddr;
                *target += delta;
                relocCount++;
            } else if (type == IMAGE_REL_BASED_HIGHLOW) {
                int32_t* target = (int32_t*)targetAddr;
                *target += (int32_t)delta;
                relocCount++;
            } else {
                printf("[!] Unknown relocation type: %u\n", type);
            }
        }

        relocBase += block->SizeOfBlock;
        blockCount++;
    }

    printf("[+] Applied %u relocations in %u blocks\n", relocCount, blockCount);
    return true;
}
