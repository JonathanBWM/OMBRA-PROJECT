#ifndef PE_PARSER_H
#define PE_PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include <Windows.h>

#define MAX_SECTIONS 16
#define MAX_IMPORTS 256

typedef struct _PE_SECTION_INFO {
    char        Name[8];
    uint32_t    Rva;
    uint32_t    VirtualSize;
    uint32_t    FileOffset;
    uint32_t    FileSize;
    uint32_t    Characteristics;
} PE_SECTION_INFO;

typedef struct _PE_IMPORT_INFO {
    char        ModuleName[64];
    char        FunctionName[128];
    uint32_t    IatRva;
    uint64_t    ResolvedAddress;
} PE_IMPORT_INFO;

typedef struct _PE_INFO {
    bool        Valid;
    uint64_t    ImageBase;
    uint32_t    ImageSize;
    uint32_t    HeadersSize;
    uint32_t    EntryPointRva;
    uint32_t    FileAlignment;
    uint32_t    SectionAlignment;
    uint32_t    SectionCount;
    PE_SECTION_INFO Sections[MAX_SECTIONS];
    uint32_t    ImportCount;
    PE_IMPORT_INFO Imports[MAX_IMPORTS];
    uint32_t    RelocRva;
    uint32_t    RelocSize;
} PE_INFO;

bool PeParse(const void* peImage, uint32_t imageSize, PE_INFO* info);
PE_SECTION_INFO* PeGetSection(PE_INFO* info, const char* name);
uint32_t PeCalculateImageSize(PE_INFO* info);

#endif
