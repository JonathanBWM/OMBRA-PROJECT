#include "pe_utils.h"
#include <Windows.h>
#include <string.h>

bool PeValidate(const void* peImage, uint32_t imageSize) {
    if (imageSize < sizeof(IMAGE_DOS_HEADER)) return false;

    const IMAGE_DOS_HEADER* dos = (const IMAGE_DOS_HEADER*)peImage;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return false;

    if ((uint32_t)dos->e_lfanew + sizeof(IMAGE_NT_HEADERS64) > imageSize) return false;

    const IMAGE_NT_HEADERS64* nt = (const IMAGE_NT_HEADERS64*)((uint8_t*)peImage + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return false;

    if (nt->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) return false;

    return true;
}

bool PeFindSection(
    const void* peImage,
    uint32_t imageSize,
    const char* sectionName,
    uint32_t* outFileOffset,
    uint32_t* outSize)
{
    // Validate PE first
    if (!PeValidate(peImage, imageSize)) return false;

    const IMAGE_DOS_HEADER* dos = (const IMAGE_DOS_HEADER*)peImage;
    const IMAGE_NT_HEADERS64* nt = (const IMAGE_NT_HEADERS64*)((uint8_t*)peImage + dos->e_lfanew);

    const IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(nt);
    uint16_t numSections = nt->FileHeader.NumberOfSections;

    // Bounds check: ensure section table is within image
    size_t sectionsOffset = (const uint8_t*)sections - (const uint8_t*)peImage;
    size_t sectionsSize = numSections * sizeof(IMAGE_SECTION_HEADER);
    if (sectionsOffset + sectionsSize > imageSize) return false;

    size_t nameLen = strlen(sectionName);
    if (nameLen > 8) nameLen = 8;  // Section names max 8 chars

    for (uint16_t i = 0; i < numSections; i++) {
        // Exact match: compare nameLen chars and ensure next char is null or we matched all 8
        if (memcmp(sections[i].Name, sectionName, nameLen) == 0 &&
            (nameLen == 8 || sections[i].Name[nameLen] == '\0')) {
            *outFileOffset = sections[i].PointerToRawData;
            *outSize = sections[i].SizeOfRawData;
            return true;
        }
    }

    return false;
}

bool PeGetEntryPoint(const void* peImage, uint32_t imageSize, uint32_t* outEntryRva) {
    // Validate PE first
    if (!PeValidate(peImage, imageSize)) return false;

    const IMAGE_DOS_HEADER* dos = (const IMAGE_DOS_HEADER*)peImage;
    const IMAGE_NT_HEADERS64* nt = (const IMAGE_NT_HEADERS64*)((uint8_t*)peImage + dos->e_lfanew);

    *outEntryRva = nt->OptionalHeader.AddressOfEntryPoint;
    return true;
}
