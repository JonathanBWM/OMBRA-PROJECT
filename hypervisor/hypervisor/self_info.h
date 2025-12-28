// self_info.h - PE Image Self-Discovery
// OmbraHypervisor
//
// Discovers the hypervisor's own PE base and size at runtime using __ImageBase.
// This allows the hypervisor to protect itself via EPT without loader assistance.

#ifndef SELF_INFO_H
#define SELF_INFO_H

#include "../shared/types.h"

// =============================================================================
// PE Structure Definitions (minimal, for header parsing)
// =============================================================================

#pragma pack(push, 1)

typedef struct _IMAGE_DOS_HEADER {
    U16 e_magic;        // DOS signature: 'MZ' (0x5A4D)
    U16 e_cblp;
    U16 e_cp;
    U16 e_crlc;
    U16 e_cparhdr;
    U16 e_minalloc;
    U16 e_maxalloc;
    U16 e_ss;
    U16 e_sp;
    U16 e_csum;
    U16 e_ip;
    U16 e_cs;
    U16 e_lfarlc;
    U16 e_ovno;
    U16 e_res[4];
    U16 e_oemid;
    U16 e_oeminfo;
    U16 e_res2[10];
    I32 e_lfanew;       // Offset to PE header
} IMAGE_DOS_HEADER;

typedef struct _IMAGE_FILE_HEADER {
    U16 Machine;
    U16 NumberOfSections;
    U32 TimeDateStamp;
    U32 PointerToSymbolTable;
    U32 NumberOfSymbols;
    U16 SizeOfOptionalHeader;
    U16 Characteristics;
} IMAGE_FILE_HEADER;

typedef struct _IMAGE_DATA_DIRECTORY {
    U32 VirtualAddress;
    U32 Size;
} IMAGE_DATA_DIRECTORY;

typedef struct _IMAGE_OPTIONAL_HEADER64 {
    U16 Magic;                          // 0x20b for PE64
    U8  MajorLinkerVersion;
    U8  MinorLinkerVersion;
    U32 SizeOfCode;
    U32 SizeOfInitializedData;
    U32 SizeOfUninitializedData;
    U32 AddressOfEntryPoint;
    U32 BaseOfCode;
    U64 ImageBase;
    U32 SectionAlignment;
    U32 FileAlignment;
    U16 MajorOperatingSystemVersion;
    U16 MinorOperatingSystemVersion;
    U16 MajorImageVersion;
    U16 MinorImageVersion;
    U16 MajorSubsystemVersion;
    U16 MinorSubsystemVersion;
    U32 Win32VersionValue;
    U32 SizeOfImage;                    // Total mapped size
    U32 SizeOfHeaders;
    U32 CheckSum;
    U16 Subsystem;
    U16 DllCharacteristics;
    U64 SizeOfStackReserve;
    U64 SizeOfStackCommit;
    U64 SizeOfHeapReserve;
    U64 SizeOfHeapCommit;
    U32 LoaderFlags;
    U32 NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[16];
} IMAGE_OPTIONAL_HEADER64;

typedef struct _IMAGE_NT_HEADERS64 {
    U32 Signature;                      // 'PE\0\0' (0x00004550)
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER64 OptionalHeader;
} IMAGE_NT_HEADERS64;

typedef struct _IMAGE_SECTION_HEADER {
    U8  Name[8];
    union {
        U32 PhysicalAddress;
        U32 VirtualSize;
    } Misc;
    U32 VirtualAddress;
    U32 SizeOfRawData;
    U32 PointerToRawData;
    U32 PointerToRelocations;
    U32 PointerToLinenumbers;
    U16 NumberOfRelocations;
    U16 NumberOfLinenumbers;
    U32 Characteristics;
} IMAGE_SECTION_HEADER;

#pragma pack(pop)

// PE signature values
#define IMAGE_DOS_SIGNATURE     0x5A4D      // 'MZ'
#define IMAGE_NT_SIGNATURE      0x00004550  // 'PE\0\0'
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC 0x20b

// =============================================================================
// Self-Info Structure
// =============================================================================

typedef struct _SELF_INFO {
    void*   ImageBase;          // Virtual address of PE base
    U64     ImageSize;          // Size of mapped image
    U64     PhysicalBase;       // Physical address of PE base
    void*   EntryPoint;         // Entry point virtual address
    U32     NumberOfSections;   // Number of PE sections
    bool    Valid;              // Structure validity flag
} SELF_INFO;

// =============================================================================
// Global Self-Info
// =============================================================================

extern SELF_INFO g_SelfInfo;

// =============================================================================
// Functions
// =============================================================================

// Discover the hypervisor's own PE image information.
// Must be called early in initialization, before any self-protection.
// Returns OMBRA_SUCCESS if PE headers were parsed successfully.
OMBRA_STATUS SelfInfoDiscover(void);

// Get the hypervisor's virtual base address.
void* SelfInfoGetBase(void);

// Get the hypervisor's mapped size in bytes.
U64 SelfInfoGetSize(void);

// Get the hypervisor's physical base address.
// Requires kernel symbols to be resolved first (uses MmGetPhysicalAddress).
U64 SelfInfoGetPhysicalBase(void);

// Find a section by name (e.g., ".text", ".ombra").
// Returns section header pointer or NULL if not found.
IMAGE_SECTION_HEADER* SelfInfoFindSection(const char* name);

// Get the physical address range of the hypervisor image.
// Used for EPT self-protection to hide hypervisor memory from guest.
OMBRA_STATUS SelfInfoGetPhysicalRange(U64* outPhysBase, U64* outPhysSize);

#endif // SELF_INFO_H
