#include "pe_parser.h"
#include <string.h>

// Convert RVA to file offset using section table
static uint32_t RvaToFileOffset(const IMAGE_NT_HEADERS64* nt, uint32_t rva) {
    if (rva == 0) return 0;

    const IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(nt);
    uint16_t numSections = nt->FileHeader.NumberOfSections;

    // Check if RVA is in headers (before first section)
    if (rva < sections[0].VirtualAddress) {
        return rva;
    }

    // Find which section contains this RVA
    for (uint16_t i = 0; i < numSections; i++) {
        uint32_t sectionStart = sections[i].VirtualAddress;
        uint32_t sectionEnd = sectionStart + sections[i].Misc.VirtualSize;

        if (rva >= sectionStart && rva < sectionEnd) {
            uint32_t offset = rva - sectionStart;
            return sections[i].PointerToRawData + offset;
        }
    }

    return 0;
}

// Parse import directory table
static bool ParseImports(
    const void* peImage,
    uint32_t imageSize,
    const IMAGE_NT_HEADERS64* nt,
    PE_INFO* info)
{
    const IMAGE_DATA_DIRECTORY* importDir =
        &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

    if (importDir->VirtualAddress == 0 || importDir->Size == 0) {
        info->ImportCount = 0;
        return true;
    }

    uint32_t importDirOffset = RvaToFileOffset(nt, importDir->VirtualAddress);
    if (importDirOffset == 0 || importDirOffset >= imageSize) {
        return false;
    }

    const IMAGE_IMPORT_DESCRIPTOR* importDesc =
        (const IMAGE_IMPORT_DESCRIPTOR*)((const uint8_t*)peImage + importDirOffset);

    info->ImportCount = 0;

    // Iterate through import descriptors
    for (uint32_t i = 0; info->ImportCount < MAX_IMPORTS; i++) {
        // Bounds check on import descriptor
        if ((const uint8_t*)&importDesc[i] + sizeof(IMAGE_IMPORT_DESCRIPTOR) - (const uint8_t*)peImage > imageSize) {
            break;
        }

        // Null descriptor marks end of import table
        if (importDesc[i].Name == 0) {
            break;
        }

        // Get module name
        uint32_t nameOffset = RvaToFileOffset(nt, importDesc[i].Name);
        if (nameOffset == 0 || nameOffset >= imageSize) {
            continue;
        }

        const char* moduleName = (const char*)((const uint8_t*)peImage + nameOffset);

        // Bounds check: ensure module name is null-terminated within image
        const char* nameEnd = moduleName;
        while ((uint32_t)((const uint8_t*)nameEnd - (const uint8_t*)peImage) < imageSize && *nameEnd != '\0') {
            nameEnd++;
        }
        if ((uint32_t)((const uint8_t*)nameEnd - (const uint8_t*)peImage) >= imageSize) {
            continue;
        }

        // Get INT and IAT
        uint32_t intRva = importDesc[i].OriginalFirstThunk;
        uint32_t iatRva = importDesc[i].FirstThunk;

        if (intRva == 0) {
            intRva = iatRva;  // Some PEs only have IAT
        }

        uint32_t intOffset = RvaToFileOffset(nt, intRva);
        if (intOffset == 0 || intOffset >= imageSize) {
            continue;
        }

        const IMAGE_THUNK_DATA64* thunk =
            (const IMAGE_THUNK_DATA64*)((const uint8_t*)peImage + intOffset);

        // Iterate through thunks
        for (uint32_t j = 0; info->ImportCount < MAX_IMPORTS; j++) {
            // Bounds check on thunk
            if ((const uint8_t*)&thunk[j] + sizeof(IMAGE_THUNK_DATA64) - (const uint8_t*)peImage > imageSize) {
                break;
            }

            // Null thunk marks end
            if (thunk[j].u1.AddressOfData == 0) {
                break;
            }

            PE_IMPORT_INFO* import = &info->Imports[info->ImportCount];

            // Copy module name
            size_t moduleLen = strlen(moduleName);
            if (moduleLen >= sizeof(import->ModuleName)) {
                moduleLen = sizeof(import->ModuleName) - 1;
            }
            memcpy(import->ModuleName, moduleName, moduleLen);
            import->ModuleName[moduleLen] = '\0';

            // Check if import by ordinal
            if (IMAGE_SNAP_BY_ORDINAL64(thunk[j].u1.Ordinal)) {
                uint16_t ordinal = (uint16_t)IMAGE_ORDINAL64(thunk[j].u1.Ordinal);
                snprintf(import->FunctionName, sizeof(import->FunctionName), "#%u", ordinal);
            } else {
                // Import by name
                uint32_t nameTableRva = (uint32_t)thunk[j].u1.AddressOfData;
                uint32_t nameTableOffset = RvaToFileOffset(nt, nameTableRva);

                if (nameTableOffset == 0 || nameTableOffset + sizeof(IMAGE_IMPORT_BY_NAME) > imageSize) {
                    snprintf(import->FunctionName, sizeof(import->FunctionName), "Unknown");
                } else {
                    const IMAGE_IMPORT_BY_NAME* importByName =
                        (const IMAGE_IMPORT_BY_NAME*)((const uint8_t*)peImage + nameTableOffset);

                    // Bounds check: ensure function name is null-terminated within image
                    const char* funcName = (const char*)importByName->Name;
                    const char* funcNameEnd = funcName;
                    while ((uint32_t)((const uint8_t*)funcNameEnd - (const uint8_t*)peImage) < imageSize && *funcNameEnd != '\0') {
                        funcNameEnd++;
                    }

                    if ((uint32_t)((const uint8_t*)funcNameEnd - (const uint8_t*)peImage) >= imageSize) {
                        snprintf(import->FunctionName, sizeof(import->FunctionName), "Unknown");
                    } else {
                        size_t funcLen = strlen(funcName);
                        if (funcLen >= sizeof(import->FunctionName)) {
                            funcLen = sizeof(import->FunctionName) - 1;
                        }
                        memcpy(import->FunctionName, funcName, funcLen);
                        import->FunctionName[funcLen] = '\0';
                    }
                }
            }

            // Calculate IAT RVA for this entry
            import->IatRva = iatRva + (j * sizeof(uint64_t));
            import->ResolvedAddress = 0;

            info->ImportCount++;
        }
    }

    return true;
}

bool PeParse(const void* peImage, uint32_t imageSize, PE_INFO* info) {
    if (!peImage || !info || imageSize < sizeof(IMAGE_DOS_HEADER)) {
        return false;
    }

    memset(info, 0, sizeof(PE_INFO));

    // Validate DOS header
    const IMAGE_DOS_HEADER* dos = (const IMAGE_DOS_HEADER*)peImage;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
        return false;
    }

    // Validate NT headers
    if ((uint32_t)dos->e_lfanew + sizeof(IMAGE_NT_HEADERS64) > imageSize) {
        return false;
    }

    const IMAGE_NT_HEADERS64* nt =
        (const IMAGE_NT_HEADERS64*)((const uint8_t*)peImage + dos->e_lfanew);

    if (nt->Signature != IMAGE_NT_SIGNATURE) {
        return false;
    }

    // Validate machine type
    if (nt->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) {
        return false;
    }

    // Extract basic PE info
    info->ImageBase = nt->OptionalHeader.ImageBase;
    info->ImageSize = nt->OptionalHeader.SizeOfImage;
    info->HeadersSize = nt->OptionalHeader.SizeOfHeaders;
    info->EntryPointRva = nt->OptionalHeader.AddressOfEntryPoint;
    info->FileAlignment = nt->OptionalHeader.FileAlignment;
    info->SectionAlignment = nt->OptionalHeader.SectionAlignment;

    // Parse sections
    const IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(nt);
    uint16_t numSections = nt->FileHeader.NumberOfSections;

    // Bounds check on section table
    size_t sectionsOffset = (const uint8_t*)sections - (const uint8_t*)peImage;
    size_t sectionsSize = numSections * sizeof(IMAGE_SECTION_HEADER);
    if (sectionsOffset + sectionsSize > imageSize) {
        return false;
    }

    if (numSections > MAX_SECTIONS) {
        numSections = MAX_SECTIONS;
    }

    info->SectionCount = numSections;

    for (uint16_t i = 0; i < numSections; i++) {
        PE_SECTION_INFO* secInfo = &info->Sections[i];

        // Copy section name (not null-terminated if exactly 8 chars)
        memcpy(secInfo->Name, sections[i].Name, 8);

        secInfo->Rva = sections[i].VirtualAddress;
        secInfo->VirtualSize = sections[i].Misc.VirtualSize;
        secInfo->FileOffset = sections[i].PointerToRawData;
        secInfo->FileSize = sections[i].SizeOfRawData;
        secInfo->Characteristics = sections[i].Characteristics;
    }

    // Parse relocations
    const IMAGE_DATA_DIRECTORY* relocDir =
        &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

    info->RelocRva = relocDir->VirtualAddress;
    info->RelocSize = relocDir->Size;

    // Parse imports
    if (!ParseImports(peImage, imageSize, nt, info)) {
        return false;
    }

    info->Valid = true;
    return true;
}

PE_SECTION_INFO* PeGetSection(PE_INFO* info, const char* name) {
    if (!info || !info->Valid || !name) {
        return NULL;
    }

    size_t nameLen = strlen(name);
    if (nameLen > 8) {
        nameLen = 8;
    }

    for (uint32_t i = 0; i < info->SectionCount; i++) {
        // Compare up to nameLen characters
        if (memcmp(info->Sections[i].Name, name, nameLen) == 0) {
            // If name is less than 8 chars, ensure next char is null or doesn't exist
            if (nameLen == 8 || info->Sections[i].Name[nameLen] == '\0') {
                return &info->Sections[i];
            }
        }
    }

    return NULL;
}

uint32_t PeCalculateImageSize(PE_INFO* info) {
    if (!info || !info->Valid) {
        return 0;
    }

    // Image size is already calculated by PE loader
    // But verify it by finding highest section end
    uint32_t highestEnd = info->HeadersSize;

    for (uint32_t i = 0; i < info->SectionCount; i++) {
        uint32_t sectionEnd = info->Sections[i].Rva + info->Sections[i].VirtualSize;

        // Align to section alignment
        if (sectionEnd % info->SectionAlignment != 0) {
            sectionEnd = ((sectionEnd / info->SectionAlignment) + 1) * info->SectionAlignment;
        }

        if (sectionEnd > highestEnd) {
            highestEnd = sectionEnd;
        }
    }

    return highestEnd;
}
