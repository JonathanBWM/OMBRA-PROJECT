# Sputnik Pattern Extraction

**Project**: Boot chain hooking with SKLib/CheatDriver integration
**Extracted**: 2025-12-20
**Purpose**: Ombra reference for UEFI boot hooking, driver deployment, and hypervisor payload injection

---

## 1. SKLIB INTEGRATION

### SKLib Purpose
- **Empty submodule in Sputnik**: `Refs/codebases/Sputnik/SKLib/` (directory exists but is empty)
- **Referenced in CheatDriver**: Used as kernel utility library for initialization and communication
- **Driver name tracking**: `CheatDriver/main.cpp:67` - `SKLib::CurrentDriverName`
- **User info structure**: `CheatDriver/main.cpp:69-77` - Passes usermode info to driver via registry path trick

### Key SKLib Usage Pattern
```cpp
// CheatDriver/main.cpp:66-78
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObj, PUNICODE_STRING pRegistryPath) {
    SKLib::Init();
    DbgMsg("[ENTRY] Current driver name: %ls", SKLib::CurrentDriverName);

    if (!MmIsAddressValid(SKLib::pUserInfo) || !MmIsAddressValid(pRegistryPath)) {
        return SKLIB_USER_INFO_INVALID;
    }
    *SKLib::pUserInfo = *(USERMODE_INFO*)pRegistryPath;

    offsets = SKLib::pUserInfo->offsets;
    winternl::InitImageInfo(pDriverObj);
    // ...
}
```

### SKLib Usermode Info Structure
- **VMCALL key**: `CheatDriver/main.cpp:98` - `sputnik::set_vmcall_key(SKLib::pUserInfo->vmcallKey)`
- **IDT restoration**: `CheatDriver/main.cpp:83-88` - Restores modified IDT entries after hypervisor load
- **CPU index tracking**: `CheatDriver/main.cpp:84` - `SKLib::pUserInfo->cpuIdx`
- **Pre-hypervisor cleanup**: `CheatDriver/main.cpp:124-130` - Zeros and frees pre-hv driver memory

---

## 2. CHEATDRIVER PATTERNS

### Driver Structure
- **Entry point**: `CheatDriver/main.cpp:65` - `DriverEntry()`
- **Communication handler**: `CheatDriver/main.cpp:100` - Sets callback address in VMX root storage
- **EPT integration**: `CheatDriver/main.cpp:106-108` - Hides driver via EPT hooks
- **Multi-core support**: `CheatDriver/main.cpp:115-116` - Runs initialization on all CPUs

### Communication Interface
```cpp
// CheatDriver/include/comms.h:54-57
namespace comms {
    BOOLEAN Init();
    NTSTATUS Entry(KERNEL_REQUEST* pKernelRequest);
}

// CheatDriver/main.cpp:100-102
sputnik::storage_set(VMX_ROOT_STORAGE::CALLBACK_ADDRESS, comms::Entry);
sputnik::storage_set(VMX_ROOT_STORAGE::DRIVER_BASE_PA, Memory::VirtToPhy(winternl::pDriverBase));
sputnik::storage_set(VMX_ROOT_STORAGE::NTOSKRNL_CR3, PsProcessDirBase(PsInitialSystemProcess));
```

### EPT State Management
```cpp
// CheatDriver/main.cpp:11-16
NTSTATUS SetEPTCache() {
    DWORD dwCore = CPU::GetCPUIndex();
    sputnik::set_ept_base(vmm::vGuestStates[dwCore].eptState.nCR3.Flags);
    return STATUS_SUCCESS;
}
```

### SVM/AMD Support
```cpp
// CheatDriver/main.cpp:51-63
NTSTATUS SetSvmState() {
    DWORD dwCore = CPU::GetCPUIndex();
    PHYSICAL_ADDRESS phy = { 0 };
    phy.QuadPart = sputnik::vmcb();

    if(!vmm::vGuestStates[dwCore].SvmState)
        vmm::vGuestStates[dwCore].SvmState = (SVM::SVMState*)cpp::kMallocTryAllZero(sizeof(SVM::SVMState));

    vmm::vGuestStates[dwCore].SvmState->GuestVmcb = (Svm::Vmcb*)MmMapIoSpace(phy, PAGE_SIZE, MEMORY_CACHING_TYPE::MmNonCached);
    return STATUS_SUCCESS;
}
```

---

## 3. LIBSPUTNIK ARCHITECTURE

### Library Organization
```
libsputnik/
├── libsputnik.hpp          # Main API interface
├── libsputnik.cpp          # Implementation
├── vdm.hpp                 # VDM (Vulnerable Driver Manipulation) wrapper
├── identity.cpp/hpp        # Identity mapping utilities
└── mapper/                 # Driver mapper subsystem
    ├── kernel_ctx.h/cpp    # Kernel context manager
    ├── map_driver.cpp      # Driver mapping logic
    ├── drv_image.cpp       # PE image handling
    └── hook.hpp            # Syscall hooking utilities
```

### API Design - VMCALL Interface
```cpp
// libsputnik/libsputnik.hpp:30-133

namespace sputnik {
    extern UINT64 VMEXIT_KEY;  // Global authentication key

    // Type aliases for clarity
    using guest_virt_t = u64;
    using guest_phys_t = u64;
    using host_virt_t = u64;
    using host_phys_t = u64;

    // VMCALL wrapper
    extern "C" auto hypercall(u64 code, PCOMMAND_DATA param1, u64 param2, u64 key) -> VMX_ROOT_ERROR;

    // Key APIs
    void set_vmcall_key(u64 key);
    auto current_dirbase() -> guest_phys_t;
    auto root_dirbase() -> guest_phys_t;
    auto current_ept_base() -> guest_phys_t;
    auto vmcb() -> host_phys_t;

    VMX_ROOT_ERROR set_ept_base(guest_phys_t nCr3);
    void set_ept_handler(guest_virt_t handler);
    VMX_ROOT_ERROR disable_ept();
    VMX_ROOT_ERROR enable_ept();

    auto read_phys(guest_phys_t phys_addr, guest_virt_t buffer, u64 size) -> VMX_ROOT_ERROR;
    auto write_phys(guest_phys_t phys_addr, guest_virt_t buffer, u64 size) -> VMX_ROOT_ERROR;
    auto read_virt(guest_virt_t virt_addr, guest_virt_t buffer, u64 size, u64 target_cr3) -> VMX_ROOT_ERROR;
    auto write_virt(guest_virt_t virt_addr, guest_virt_t buffer, u64 size, u64 target_cr3) -> VMX_ROOT_ERROR;
    auto virt_to_phy(guest_virt_t p, u64 dirbase = 0) -> guest_phys_t;

    // Storage API for VMX root globals
    template<typename T>
    T storage_get(u64 id) {
        COMMAND_DATA data = { 0 };
        data.storage.bWrite = false;
        data.storage.id = id;
        auto status = hypercall(VMCALL_STORAGE_QUERY, &data, 0, VMEXIT_KEY);
        return (T)data.storage.uint64;
    }

    template<typename T>
    void storage_set(u64 id, T value) {
        COMMAND_DATA data = { 0 };
        data.storage.bWrite = true;
        data.storage.id = id;
        data.storage.uint64 = (UINT64)value;
        hypercall(VMCALL_STORAGE_QUERY, &data, 0, VMEXIT_KEY);
    }
}
```

### VDM Wrapper Pattern
```cpp
// libsputnik/vdm.hpp:16-232
class SELibVdm {
    ULONG64 _callbackAddress;
    ULONG64 _ntoskrnlAddress;
    ULONG64 _gameCr3;

public:
    // Initialization with callback and CR3
    void Init(ULONG64 callbackAddress, ULONG64 cr3 = 0);

    // Memory operations wrapped around sputnik API
    BOOLEAN ReadMemory(ULONG64 Source, PVOID Destination, SIZE_T NumberOfBytes, ULONG64 cr3 = ~0ull);
    BOOLEAN WriteMemory(ULONG64 Destination, PVOID Source, SIZE_T NumberOfBytes, ULONG64 cr3 = ~0ull);

    // Kernel function invocation via callback
    template<typename T, typename ...A>
    BOOLEAN CallKernelFunction(T* out_result, ULONG64 kernel_function_address, const A ...arguments);

    // Export resolution
    ULONG64 GetKernelModuleExport(const std::string& function_name);
    ULONG64 GetKernelModuleAddress(const std::string& module_name);
};
```

### Mapper Subsystem
```cpp
// libsputnik/mapper/kernel_ctx.h:45-135
class kernel_ctx {
public:
    // Pool allocation
    void* allocate_pool(std::size_t size, POOL_TYPE pool_type = NonPagedPool);
    void* allocate_pool(std::size_t size, ULONG pool_tag, POOL_TYPE pool_type);

    // Memory operations via syscall hooks
    void read_kernel(void* addr, void* buffer, std::size_t size);
    void write_kernel(void* addr, void* buffer, std::size_t size);
    void zero_kernel_memory(void* addr, std::size_t size);

    // PIDDB cache clearing
    bool clear_piddb_cache(const std::string& file_name, const std::uint32_t timestamp);

    // Syscall execution via hooked NtShutdownSystem
    template <class T, class ... Ts>
    std::invoke_result_t<T, Ts...> syscall(void* addr, Ts ... args) const;
};
```

---

## 4. BOOT HOOKING

### Boot Chain Flow
```
Sputnik/UefiMain.cpp (Entry)
    ↓
BootMgfw.cpp (Hook bootmgfw.efi)
    ↓
WinLoad.cpp (Hook winload.efi)
    ↓
HvLoader.cpp (Hook hvloader.efi → hv.exe)
```

### UefiMain Entry Point
```cpp
// Sputnik/UefiMain.cpp:17-77
EFI_STATUS EFIAPI UefiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    // 1. Restore bootmgfw.efi from backup
    if (EFI_ERROR(RestoreBootMgfw())) {
        goto _error;
    }

    // 2. Load payload from disk into memory then delete
    if (EFI_ERROR(LoadPayLoadFromDisk(&PayLoad))) {
        goto _error;
    }

    // 3. Get bootmgfw device path
    if (EFI_ERROR(GetBootMgfwPath(&BootMgfwPath))) {
        goto _error;
    }

    // 4. Load bootmgfw into memory
    if (EFI_ERROR(gBS->LoadImage(TRUE, ImageHandle, BootMgfwPath, NULL, NULL, &BootMgfwHandle))) {
        goto _error;
    }

    // 5. Install hooks on bootmgfw
    if (EFI_ERROR(InstallBootMgfwHooks(BootMgfwHandle))) {
        goto _error;
    }

    // 6. Start bootmgfw (hooks are now active)
    if (EFI_ERROR(gBS->StartImage(BootMgfwHandle, NULL, NULL))) {
        goto _error;
    }

    return EFI_SUCCESS;
}
```

### BootMgfw Hook Installation
```cpp
// Sputnik/BootMgfw.cpp:182-207
EFI_STATUS EFIAPI InstallBootMgfwHooks(EFI_HANDLE ImageHandle) {
    EFI_LOADED_IMAGE* BootMgfw = NULL;
    gBS->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID**)&BootMgfw);

    // Pattern scan for ArchStartBootApplication
    VOID* ArchStartBootApplication = FindPattern(
        BootMgfw->ImageBase,
        BootMgfw->ImageSize,
        (VOID*)START_BOOT_APPLICATION_SIG,
        (VOID*)START_BOOT_APPLICATION_MASK
    );

    // Install inline hook
    MakeInlineHook(&BootMgfwShitHook, ArchStartBootApplication, &ArchStartBootApplicationHook, TRUE);
    return EFI_SUCCESS;
}

// Sputnik/BootMgfw.cpp:209-235
EFI_STATUS EFIAPI ArchStartBootApplicationHook(VOID* AppEntry, VOID* ImageBase, UINT32 ImageSize, UINT8 BootOption, VOID* ReturnArgs) {
    DisableInlineHook(&BootMgfwShitHook);

    // Get exports from winload.efi
    VOID* LdrLoadImage = GetExport(ImageBase, (VOID*)"BlLdrLoadImage");
    VOID* ImgAllocateImageBuffer = FindPattern(
        ImageBase, ImageSize,
        (VOID*)ALLOCATE_IMAGE_BUFFER_SIG,
        (VOID*)ALLOCATE_IMAGE_BUFFER_MASK
    );

    // Hook winload functions
    MakeInlineHook(&WinLoadImageShitHook, LdrLoadImage, BlLdrLoadImage, TRUE);
    MakeInlineHook(&WinLoadAllocateImageHook, (VOID*)RESOLVE_RVA(ImgAllocateImageBuffer, 13, 9), BlImgAllocateImageBuffer, TRUE);

    return ((IMG_ARCH_START_BOOT_APPLICATION)BootMgfwShitHook.Address)(AppEntry, ImageBase, ImageSize, BootOption, ReturnArgs);
}
```

### WinLoad Hooks - hv.exe Detection
```cpp
// Sputnik/WinLoad.cpp:13-97
EFI_STATUS EFIAPI BlLdrLoadImage(/* ... */) {
    // Detect Hyper-V loading
    if (!StrCmp(ModuleName, (CHAR16*)L"hv.exe"))
        HyperVloading = TRUE;

    // Call original
    DisableInlineHook(&WinLoadImageShitHook);
    EFI_STATUS Result = ((LDR_LOAD_IMAGE)WinLoadImageShitHook.Address)(/* ... */);

    // Re-enable if not done hooking
    if (!HookedHyperV)
        EnableInlineHook(&WinLoadImageShitHook);

    // When hv.exe loads, inject payload
    if (!StrCmp(ModuleName, (CHAR16*)L"hv.exe")) {
        HookedHyperV = TRUE;
        PLDR_DATA_TABLE_ENTRY TableEntry = *lplpTableEntry;

        // Add payload section to hv.exe
        MakeSputnikData(&SputnikData, (VOID*)TableEntry->ModuleBase, TableEntry->SizeOfImage,
            AddSection((VOID*)TableEntry->ModuleBase, (CHAR8*)"payload", PayLoadSize(), SECTION_RWX),
            PayLoadSize()
        );

        // Hook VMExit handler
        HookVmExit((VOID*)SputnikData.HypervModuleBase, (VOID*)SputnikData.HypervModuleSize,
            MapModule(&SputnikData, (UINT8*)PayLoad));

        // Extend image size in LDR entry
        TableEntry->SizeOfImage = NT_HEADER(TableEntry->ModuleBase)->OptionalHeader.SizeOfImage;
    }
    return Result;
}
```

### WinLoad Allocation Extension
```cpp
// Sputnik/WinLoad.cpp:169-216
UINT64 EFIAPI BlImgAllocateImageBuffer(VOID** imageBuffer, UINTN imageSize, UINT32 memoryType, UINT32 attributes, VOID* unused, UINT32 Value) {
    // Wait for second allocation (actual hv.exe image)
    if (HyperVloading && !ExtendedAllocation && ++AllocationCount == 2) {
        ExtendedAllocation = TRUE;
        imageSize += PayLoadSize();  // Extend to fit payload
        memoryType = BL_MEMORY_ATTRIBUTE_RWX;  // Allocate as RWX
    }

    DisableInlineHook(&WinLoadAllocateImageHook);
    UINT64 Result = ((ALLOCATE_IMAGE_BUFFER)WinLoadAllocateImageHook.Address)(imageBuffer, imageSize, memoryType, attributes, unused, Value);

    if(!ExtendedAllocation)
        EnableInlineHook(&WinLoadAllocateImageHook);

    return Result;
}
```

### HvLoader Hooks - Direct Hyper-V Injection
```cpp
// Sputnik/WinLoad.cpp:99-167 (Inside BlImgLoadPEImageEx hook)
if (StrStr(ImagePath, (CHAR16*)L"hvloader.efi")) {
    // Pattern scan hvloader.efi for Hyper-V loading functions
    VOID* LoadImage = FindPattern(*ImageBasePtr, *ImageSize,
        (VOID*)HV_LOAD_PE_IMG_FROM_BUFFER_SIG,
        (VOID*)HV_LOAD_PE_IMG_FROM_BUFFER_MASK
    );
    VOID* AllocImage = FindPattern(*ImageBasePtr, *ImageSize,
        (VOID*)HV_ALLOCATE_IMAGE_BUFFER_SIG,
        (VOID*)HV_ALLOCATE_IMAGE_BUFFER_MASK
    );

    // Hook hvloader's internal functions
    if(LoadImage)
        MakeInlineHook(&HvLoadImageBufferHook, (VOID*)RESOLVE_RVA(LoadImage, 5, 1), &HvBlImgLoadPEImageFromSourceBuffer, TRUE);

    MakeInlineHook(&HvLoadAllocImageHook, (VOID*)RESOLVE_RVA(AllocImage, 5, 1), &HvBlImgAllocateImageBuffer, TRUE);
    InstalledHvLoaderHook = TRUE;
}
```

### HvLoader Allocation Extension
```cpp
// Sputnik/HvLoader.cpp:168-204
UINT64 EFIAPI HvBlImgAllocateImageBuffer(VOID** imageBuffer, UINTN imageSize, UINT32 memoryType, UINT32 attributes, VOID* unused, UINT32 Value) {
    if (imageSize >= HV_ALLOC_SIZE && !HvExtendedAllocation) {
        HvExtendedAllocation = TRUE;
        imageSize += PayLoadSize();
        memoryType = BL_MEMORY_ATTRIBUTE_RWX;  // Force RWX allocation
    }

    DisableInlineHook(&HvLoadAllocImageHook);
    UINT64 Result = ((ALLOCATE_IMAGE_BUFFER)HvLoadAllocImageHook.Address)(imageBuffer, imageSize, memoryType, attributes, unused, Value);

    if(!HvExtendedAllocation)
        EnableInlineHook(&HvLoadAllocImageHook);

    return Result;
}
```

### Payload Delivery Mechanism
```cpp
// Sputnik/HvLoader.cpp:12-90 (HvBlImgLoadPEImageFromSourceBuffer)
EFI_STATUS EFIAPI HvBlImgLoadPEImageFromSourceBuffer(/* ... */, UINT64* ImageBase, UINT32* ImageSize, /* ... */) {
    DisableInlineHook(&HvLoadImageBufferHook);
    EFI_STATUS Result = ((HV_LDR_LOAD_IMAGE_BUFFER)HvLoadImageBufferHook.Address)(/* ... */);

    if(!HvExtendedAllocation && !HvHookedHyperV)
        EnableInlineHook(&HvLoadImageBufferHook);

    if (HvExtendedAllocation && !HvHookedHyperV) {
        HvHookedHyperV = TRUE;
        SPUTNIK_T SputnikData;

        // Add "payload" section to hv.exe
        MakeSputnikData(&SputnikData, (VOID*)*ImageBase, *ImageSize,
            AddSection((VOID*)*ImageBase, (CHAR8*)"payload", PayLoadSize(), SECTION_RWX),
            PayLoadSize()
        );

        // Hook VMExit handler and map payload module
        HookVmExit((VOID*)SputnikData.HypervModuleBase, (VOID*)SputnikData.HypervModuleSize,
            MapModule(&SputnikData, PayLoad));

        // Update image size
        *ImageSize += NT_HEADER(*ImageBase)->OptionalHeader.SizeOfImage;
    }
    return Result;
}
```

---

## 5. DRIVER DEPLOYMENT

### Payload Loading from Disk
```cpp
// Sputnik/PayLoad.cpp:76-166
EFI_STATUS LoadPayLoadFromDisk(VOID** PayLoadBufferPtr) {
    // Iterate all file systems
    gBS->LocateHandleBuffer(ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &HandleCount, &Handles);

    for (UINT32 Idx = 0u; Idx < HandleCount; ++Idx) {
        // Open file system protocol
        gBS->OpenProtocol(Handles[Idx], &gEfiSimpleFileSystemProtocolGuid, (VOID**)&FileSystem, /* ... */);
        FileSystem->OpenVolume(FileSystem, &VolumeHandle);

        // Try to open payload file
        if (!EFI_ERROR(VolumeHandle->Open(VolumeHandle, &PayLoadFileHandle, (CHAR16*)PAYLOAD_PATH, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY))) {
            // Open for read/write
            EfiOpenFileByDevicePath(&PayLoadDevicePath, &PayLoadFile, EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ, NULL);

            // Get file size
            PayLoadFile->GetInfo(PayLoadFile, &gEfiFileInfoGuid, &FileInfoSize, FileInfoPtr);

            // Allocate buffer and read
            gBS->AllocatePool(EfiBootServicesData, FileInfoPtr->FileSize, &PayLoadBuffer);
            PayLoadFile->Read(PayLoadFile, &PayLoadSize, PayLoadBuffer);

            // DELETE payload from disk after loading
            PayLoadFile->Delete(PayLoadFile);

            *PayLoadBufferPtr = PayLoadBuffer;
            return EFI_SUCCESS;
        }
    }
    return EFI_ABORTED;
}
```

### PE Section Addition
```cpp
// Sputnik/PayLoad.cpp:36-74
VOID* AddSection(VOID* ImageBase, CHAR8* SectionName, UINT32 VirtualSize, UINT32 Characteristics) {
    EFI_IMAGE_DOS_HEADER* dosHeader = (EFI_IMAGE_DOS_HEADER*)ImageBase;
    EFI_IMAGE_NT_HEADERS64* ntHeaders = (EFI_IMAGE_NT_HEADERS64*)((UINT64)ImageBase + dosHeader->e_lfanew);

    EFI_IMAGE_SECTION_HEADER* firstSectionHeader = (EFI_IMAGE_SECTION_HEADER*)(/* ... */);
    UINT32 numberOfSections = ntHeaders->FileHeader.NumberOfSections;
    EFI_IMAGE_SECTION_HEADER* newSectionHeader = &firstSectionHeader[numberOfSections];
    EFI_IMAGE_SECTION_HEADER* lastSectionHeader = &firstSectionHeader[numberOfSections - 1];

    // Copy section name
    MemCopy(&newSectionHeader->Name, SectionName, AsciiStrLen(SectionName));

    // Set virtual size and address
    newSectionHeader->Misc.VirtualSize = VirtualSize;
    newSectionHeader->VirtualAddress = P2ALIGNUP(
        lastSectionHeader->VirtualAddress + lastSectionHeader->Misc.VirtualSize,
        ntHeaders->OptionalHeader.SectionAlignment
    );

    // Set characteristics (RWX)
    newSectionHeader->SizeOfRawData = P2ALIGNUP(VirtualSize, ntHeaders->OptionalHeader.FileAlignment);
    newSectionHeader->Characteristics = Characteristics;

    // Update PE headers
    ++ntHeaders->FileHeader.NumberOfSections;
    ntHeaders->OptionalHeader.SizeOfImage = P2ALIGNUP(
        newSectionHeader->VirtualAddress + newSectionHeader->Misc.VirtualSize,
        ntHeaders->OptionalHeader.SectionAlignment
    );

    return (VOID*)((UINT64)ImageBase + newSectionHeader->VirtualAddress);
}
```

### Payload Module Mapping
```cpp
// Sputnik/Hv.cpp:5-109
VOID* MapModule(PSPUTNIK_T SputnikData, VOID* ImageBase) {
    UINT8* base = (UINT8*)ImageBase;
    EFI_IMAGE_DOS_HEADER* dosHeaders = (EFI_IMAGE_DOS_HEADER*)base;
    EFI_IMAGE_NT_HEADERS64* ntHeaders = (EFI_IMAGE_NT_HEADERS64*)(base + dosHeaders->e_lfanew);

    // Copy headers to new location
    MemCopy((UINT8*)SputnikData->ModuleBase, base, ntHeaders->OptionalHeader.SizeOfHeaders);

    // Copy sections
    EFI_IMAGE_SECTION_HEADER* sections = (EFI_IMAGE_SECTION_HEADER*)((UINT8*)&ntHeaders->OptionalHeader + ntHeaders->FileHeader.SizeOfOptionalHeader);
    for (UINT32 i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i) {
        EFI_IMAGE_SECTION_HEADER* section = &sections[i];
        if (section->SizeOfRawData) {
            MemCopy(
                (UINT8*)SputnikData->ModuleBase + section->VirtualAddress,
                base + section->PointerToRawData,
                section->SizeOfRawData
            );
        }
    }

    // Zero original location
    RtlZeroMemory(base, totSize + ntHeaders->OptionalHeader.SizeOfHeaders);

    // Find and initialize exports (identity_map, sputnik_context, vmcallKey)
    EFI_IMAGE_EXPORT_DIRECTORY* ExportDir = (EFI_IMAGE_EXPORT_DIRECTORY*)(SputnikData->ModuleBase + ntHeaders->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

    for (UINT16 i = 0; i < ExportDir->AddressOfFunctions; i++) {
        if (AsciiStrStr((CHAR8*)SputnikData->ModuleBase + Name[i], "identity_map")) {
            auto& identity = *(identity::IDENTITY_MAPPING*)(SputnikData->ModuleBase + Address[Ordinal[i]]);
            identity.Init();
        }
        else if (AsciiStrStr((CHAR8*)SputnikData->ModuleBase + Name[i], "sputnik_context")) {
            *(SPUTNIK_T*)(SputnikData->ModuleBase + Address[Ordinal[i]]) = *SputnikData;
        }
        else if (AsciiStrStr((CHAR8*)SputnikData->ModuleBase + Name[i], "vmcallKey")) {
            *(UINT64*)(SputnikData->ModuleBase + Address[Ordinal[i]]) = 0;
        }
    }

    // Process relocations
    EFI_IMAGE_DATA_DIRECTORY* baseRelocDir = &ntHeadersNew->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_BASERELOC];
    if (baseRelocDir->VirtualAddress) {
        EFI_IMAGE_BASE_RELOCATION* reloc = (EFI_IMAGE_BASE_RELOCATION*)(SputnikData->ModuleBase + baseRelocDir->VirtualAddress);
        for (UINT32 currentSize = 0; currentSize < baseRelocDir->Size; ) {
            UINT32 relocCount = (reloc->SizeOfBlock - sizeof(EFI_IMAGE_BASE_RELOCATION)) / sizeof(UINT16);
            UINT16* relocData = (UINT16*)((UINT8*)reloc + sizeof(EFI_IMAGE_BASE_RELOCATION));

            for (UINT32 i = 0; i < relocCount; ++i, ++relocData) {
                UINT16 type = *relocData >> 12;
                UINT16 offset = *relocData & 0xFFF;

                if (type == EFI_IMAGE_REL_BASED_DIR64) {
                    UINT64* rva = (UINT64*)(relocBase + offset);
                    *rva = (UINT64)(SputnikData->ModuleBase + (*rva - ntHeadersNew->OptionalHeader.ImageBase));
                }
            }
            currentSize += reloc->SizeOfBlock;
            reloc = (EFI_IMAGE_BASE_RELOCATION*)relocData;
        }
    }

    return (VOID*)(SputnikData->ModuleBase + ntHeadersNew->OptionalHeader.AddressOfEntryPoint);
}
```

### VMExit Hook Installation - Intel
```cpp
// Sputnik/Hv.cpp:172-197
VOID* HookVmExit(VOID* HypervBase, VOID* HypervSize, VOID* VmExitHook) {
    // Find VMExit handler via pattern scan
    VOID* VmExitHandler = FindPattern(
        HypervBase, (UINT64)HypervSize,
        (VOID*)INTEL_VMEXIT_HANDLER_SIG,
        (VOID*)INTEL_VMEXIT_HANDLER_MASK
    );

    if (VmExitHandler) {
        /*
         .text:FFFFF80000237436   mov rcx, [rsp+arg_18]  ; rcx = register context
         .text:FFFFF8000023743B   mov rdx, [rsp+arg_28]
         .text:FFFFF80000237440   call vmexit_c_handler  ; RIP relative call
         .text:FFFFF80000237445   jmp loc_FFFFF80000237100
        */

        UINT64 VmExitHandlerCall = ((UINT64)VmExitHandler) + 19;  // +19 to call instruction
        UINT64 VmExitHandlerCallRip = (UINT64)VmExitHandlerCall + 5;  // +5 for call instruction size
        UINT64 VmExitFunction = VmExitHandlerCallRip + *(INT32*)((UINT64)(VmExitHandlerCall + 1));  // Resolve RIP-relative

        // Patch the call to redirect to our hook
        INT32 NewVmExitRVA = ((INT64)VmExitHook) - VmExitHandlerCallRip;
        *(INT32*)((UINT64)(VmExitHandlerCall + 1)) = NewVmExitRVA;

        return (VOID*)VmExitFunction;
    }
    // ... AMD path follows
}
```

### VMExit Hook Installation - AMD
```cpp
// Sputnik/Hv.cpp:198-214
else {  // AMD SVM path
    VOID* VmExitHandlerCall = FindPattern(
        HypervBase, (UINT64)HypervSize,
        (VOID*)AMD_VMEXIT_HANDLER_SIG,
        (VOID*)AMD_VMEXIT_HANDLER_MASK
    );

    UINT64 VmExitHandlerCallRip = ((UINT64)VmExitHandlerCall) + 5;
    UINT64 VmExitHandlerFunction = VmExitHandlerCallRip + *(INT32*)(((UINT64)VmExitHandlerCall) + 1);

    // Patch call to our hook
    INT32 NewVmExitHandlerRVA = ((INT64)VmExitHook) - VmExitHandlerCallRip;
    *(INT32*)((UINT64)VmExitHandlerCall + 1) = NewVmExitHandlerRVA;

    return (VOID*)VmExitHandlerFunction;
}
```

### Sputnik Data Structure Population
```cpp
// Sputnik/Hv.cpp:111-170
VOID MakeSputnikData(PSPUTNIK_T SputnikData, VOID* HypervAlloc, UINT64 HypervAllocSize, VOID* PayLoadBase, UINT64 PayLoadSize) {
    SputnikData->HypervModuleBase = (UINT64)HypervAlloc;
    SputnikData->HypervModuleSize = HypervAllocSize;
    SputnikData->ModuleBase = (UINT64)PayLoadBase;
    SputnikData->ModuleSize = PayLoadSize;

    // Find VMExit handler (Intel path)
    VOID* VmExitHandler = FindPattern(HypervAlloc, HypervAllocSize,
        (VOID*)INTEL_VMEXIT_HANDLER_SIG,
        (VOID*)INTEL_VMEXIT_HANDLER_MASK
    );

    if (VmExitHandler) {
        UINT64 VmExitHandlerCall = ((UINT64)VmExitHandler) + 19;
        UINT64 VmExitHandlerCallRip = (UINT64)VmExitHandlerCall + 5;
        UINT64 VmExitFunction = VmExitHandlerCallRip + *(INT32*)((UINT64)(VmExitHandlerCall + 1));

        // Calculate RVA from payload entry to original handler
        SputnikData->VmExitHandlerRva = ((UINT64)PayLoadEntry(PayLoadBase)) - (UINT64)VmExitFunction;
    }
    else {  // AMD path
        VOID* VmExitHandlerCall = FindPattern(HypervAlloc, HypervAllocSize,
            (VOID*)AMD_VMEXIT_HANDLER_SIG,
            (VOID*)AMD_VMEXIT_HANDLER_MASK
        );

        UINT64 VmExitHandlerCallRip = (UINT64)VmExitHandlerCall + 5;
        UINT64 VmExitHandlerFunc = VmExitHandlerCallRip + *(INT32*)((UINT64)VmExitHandlerCall + 1);
        SputnikData->VmExitHandlerRva = ((UINT64)PayLoadEntry(PayLoadBase)) - VmExitHandlerFunc;

        // Find VMCB offset structure
        UINT64 VmcbOffsetsAddr = (UINT64)FindPattern(HypervAlloc, HypervAllocSize,
            (VOID*)"\x65\x48\x8B\x04\x25\x00\x00\x00\x00\x48\x8B\x88\x00\x00\x00\x00\x48\x8B\x81\x00\x00\x00\x00\x48\x8B\x88",
            (VOID*)"xxxxx????xxx????xxx????xxx"
        );

        VmcbOffsetsAddr += 5;
        SputnikData->VmcbBase = *(UINT32*)VmcbOffsetsAddr;
        VmcbOffsetsAddr += 3 + 4;
        SputnikData->VmcbLink = *(UINT32*)VmcbOffsetsAddr;
        VmcbOffsetsAddr += 3 + 4;
        SputnikData->VmcbOff = *(UINT32*)VmcbOffsetsAddr;
    }
}
```

### Inline Hook Structure
```cpp
// Sputnik/InlineHook.h:4-15
typedef struct _INLINE_HOOK {
    unsigned char Code[14];      // Original bytes
    unsigned char JmpCode[14];   // Hook trampoline
    void* Address;               // Original function
    void* HookAddress;           // Hook destination
} INLINE_HOOK, *PINLINE_HOOK_T;

VOID MakeInlineHook(PINLINE_HOOK_T Hook, VOID* HookFrom, VOID* HookTo, BOOLEAN Install);
VOID EnableInlineHook(PINLINE_HOOK_T Hook);
VOID DisableInlineHook(PINLINE_HOOK_T Hook);
```

### Bootmgfw Restoration Pattern
```cpp
// Sputnik/BootMgfw.cpp:5-135
EFI_STATUS EFIAPI RestoreBootMgfw(VOID) {
    // Locate file system
    gBS->LocateHandleBuffer(ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &HandleCount, &Handles);

    for (UINT32 Idx = 0u; Idx < HandleCount; ++Idx) {
        // Open bootmgfw.efi
        if (!EFI_ERROR(VolumeHandle->Open(VolumeHandle, &BootMgfwHandle, (CHAR16*)WINDOWS_BOOTMGFW_PATH, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY))) {
            // Delete current bootmgfw.efi
            EfiOpenFileByDevicePath(&BootMgfwPathProtocol, &BootMgfwFile, EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ, NULL);
            BootMgfwFile->Delete(BootMgfwFile);

            // Open bootmgfw.efi.backup
            BootMgfwPathProtocol = FileDevicePath(Handles[Idx], (CHAR16*)WINDOWS_BOOTMGFW_BACKUP_PATH);
            EfiOpenFileByDevicePath(&BootMgfwPathProtocol, &BootMgfwFile, EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ, NULL);

            // Read backup file
            BootMgfwFile->GetInfo(BootMgfwFile, &gEfiFileInfoGuid, &FileInfoSize, FileInfoPtr);
            gBS->AllocatePool(EfiBootServicesData, FileInfoPtr->FileSize, &BootMgfwBuffer);
            BootMgfwFile->Read(BootMgfwFile, &BootMgfwSize, BootMgfwBuffer);

            // Delete backup
            BootMgfwFile->Delete(BootMgfwFile);

            // Create new bootmgfw.efi
            BootMgfwPathProtocol = FileDevicePath(Handles[Idx], (CHAR16*)WINDOWS_BOOTMGFW_PATH);
            EfiOpenFileByDevicePath(&BootMgfwPathProtocol, &BootMgfwFile, EFI_FILE_MODE_CREATE | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ, EFI_FILE_SYSTEM);

            // Write backup data to new file
            BootMgfwFile->Write(BootMgfwFile, &BootMgfwSize, BootMgfwBuffer);
            BootMgfwFile->Close(BootMgfwFile);

            return EFI_SUCCESS;
        }
    }
    return EFI_ABORTED;
}
```

---

## KEY PATTERNS FOR OMBRA

### 1. Dual-Path Hook Strategy
- **WinLoad hooks**: Target `BlLdrLoadImage` for hv.exe detection + allocation extension
- **HvLoader hooks**: Fallback/alternate path via `BlImgLoadPEImageFromSourceBuffer`
- **Allocation tracking**: Monitor allocation count/size to identify correct Hyper-V allocation

### 2. Payload Injection Flow
```
1. Extend Hyper-V allocation (imageSize += PayLoadSize())
2. Force RWX permissions (memoryType = BL_MEMORY_ATTRIBUTE_RWX)
3. Add "payload" section to hv.exe PE
4. Map payload module with relocations
5. Hook VMExit handler call (patch RIP-relative offset)
6. Update LDR table entry SizeOfImage
```

### 3. Inline Hook Pattern
```cpp
// Save original bytes
memcpy(Hook->Code, HookFrom, 14);

// Build jump trampoline
// mov rax, <address>
// jmp rax
Hook->JmpCode[0] = 0x48; Hook->JmpCode[1] = 0xB8;  // mov rax, imm64
*(UINT64*)&Hook->JmpCode[2] = (UINT64)HookTo;
Hook->JmpCode[10] = 0xFF; Hook->JmpCode[11] = 0xE0;  // jmp rax

// Install
memcpy(HookFrom, Hook->JmpCode, 14);

// Restore
memcpy(HookFrom, Hook->Code, 14);
```

### 4. Pattern Scanning
```cpp
// Sputnik/Utils.cpp (referenced in Utils.h:37)
BOOLEAN CheckMask(VOID* base, VOID* pattern, VOID* mask) {
    for (CHAR8* pat = (CHAR8*)pattern, *msk = (CHAR8*)mask; *msk; ++pat, ++msk)
        if (*msk != '?' && *(CHAR8*)base != *pat)
            return FALSE;
    return TRUE;
}

VOID* FindPattern(VOID* base, UINTN size, VOID* pattern, VOID* mask) {
    size -= AsciiStrLen((CHAR8*)mask);
    for (UINTN i = 0; i < size; ++i)
        if (CheckMask((VOID*)((UINT64)base + i), pattern, mask))
            return (VOID*)((UINT64)base + i);
    return NULL;
}
```

### 5. VDM Mapper Integration
- **Syscall hooking**: Hook `NtShutdownSystem` to execute arbitrary kernel code
- **Physical page scanning**: Locate syscall's physical page via pattern matching
- **PIDDB clearing**: Clean driver signature cache before mapping
- **Pool allocation**: Use kernel pool for driver buffer

### 6. Libsputnik Communication Model
- **VMCALL key authentication**: All hypercalls require matching key
- **Storage API**: Per-core storage slots for VMX root globals
- **EPT handler callback**: Set via `storage_set(EPT_HANDLER_ADDRESS, handler)`
- **Callback invocation**: Set via `storage_set(CALLBACK_ADDRESS, comms::Entry)`

---

## INTEGRATION POINTS FOR OMBRA

### OmbraBoot (UEFI DXE)
1. **Adopt bootmgfw restoration pattern**: `BootMgfw.cpp:5-135`
2. **Use Sputnik's inline hook structure**: `InlineHook.h:4-15`
3. **Implement dual-path hooking**: WinLoad AND HvLoader hooks
4. **Pattern scan for functions**: `Utils.h:37-38` - `FindPattern()`

### OmbraPayload (Hypervisor)
1. **Export sputnik_context**: Global structure for UEFI-to-payload communication
2. **Export vmcallKey**: Initialize to 0, set by driver
3. **Export identity_map**: Physical memory identity mapping init
4. **Implement storage API**: Per-core storage array for callbacks

### OmbraDriver (Kernel)
1. **Adopt CheatDriver entry pattern**: `CheatDriver/main.cpp:65-159`
2. **Use VDM wrapper**: `libsputnik/vdm.hpp:16-232` for kernel operations
3. **Set EPT handler**: `CheatDriver/main.cpp:132` - `sputnik::set_ept_handler()`
4. **Initialize VMCALL key**: `CheatDriver/main.cpp:98` - `sputnik::set_vmcall_key()`

---

**End of Sputnik Extraction**
