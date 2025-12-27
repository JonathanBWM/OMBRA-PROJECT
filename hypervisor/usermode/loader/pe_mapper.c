#include "pe_mapper.h"
#include "pe_relocs.h"
#include "pe_iat.h"
#include "pe_wipe.h"
#include <stdio.h>
#include <string.h>

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

bool MapDriver(
    const void* peImage,
    void* targetR3,
    uint64_t targetR0,
    PE_INFO* peInfo
) {
    printf("[*] Mapping driver to R3=%p (R0=0x%llX)\n", targetR3, targetR0);

    // Zero target region first
    memset(targetR3, 0, peInfo->ImageSize);

    // Copy headers (will be wiped after relocations/IAT)
    memcpy(targetR3, peImage, peInfo->HeadersSize);
    printf("[+] Copied headers (%u bytes)\n", peInfo->HeadersSize);

    // Copy each section to its RVA
    for (uint32_t i = 0; i < peInfo->SectionCount; i++) {
        PE_SECTION_INFO* sec = &peInfo->Sections[i];

        if (sec->FileSize == 0) {
            printf("[*] Section %.8s: virtual only (%u bytes at RVA 0x%X)\n",
                   sec->Name, sec->VirtualSize, sec->Rva);
            continue;
        }

        void* dst = (uint8_t*)targetR3 + sec->Rva;
        const void* src = (const uint8_t*)peImage + sec->FileOffset;
        uint32_t copySize = min(sec->FileSize, sec->VirtualSize);

        memcpy(dst, src, copySize);
        printf("[+] Section %.8s: copied %u bytes to RVA 0x%X\n",
               sec->Name, copySize, sec->Rva);
    }

    // Apply relocations using kernel address as new base
    if (!ApplyRelocations(targetR3, peInfo, targetR0)) {
        printf("[-] Relocation failed\n");
        return false;
    }

    // Patch IAT with resolved imports
    if (!PatchIAT(targetR3, peInfo)) {
        printf("[-] IAT patching failed\n");
        return false;
    }

    // Wipe PE headers to avoid signature detection
    WipePeHeaders(targetR3, peInfo->HeadersSize);

    printf("[+] Driver mapped successfully\n");
    return true;
}
