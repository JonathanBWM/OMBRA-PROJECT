#pragma once
#include "zerohvci.h"
#include "kforge.h"
#include "utils.h"
#include "exploit.h"
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <cstdio>

namespace zerohvci {
namespace mapper {

// Driver image PE parsing and mapping
class DriverImage {
public:
    DriverImage() = default;
    ~DriverImage() = default;

    // Load driver from file or memory
    bool LoadFromFile(const char* path);
    bool LoadFromMemory(const uint8_t* data, size_t size);

    // PE metadata access
    size_t GetImageSize() const { return m_imageSize; }
    size_t GetHeaderSize() const { return m_headerSize; }
    uintptr_t GetEntryPointRva() const { return m_entryPointRva; }
    const uint8_t* GetMappedData() const { return m_mappedImage.data(); }

    // PE mapping operations (modifies m_mappedImage in place)
    bool MapSections();           // Copy raw sections to virtual layout
    bool ProcessRelocations(uintptr_t newBase);
    bool ResolveImports();        // Uses GetKernelProcAddress

private:
    std::vector<uint8_t> m_rawImage;      // Raw file data
    std::vector<uint8_t> m_mappedImage;   // Virtual memory layout

    PIMAGE_DOS_HEADER m_dosHeader = nullptr;
    PIMAGE_NT_HEADERS m_ntHeaders = nullptr;
    PIMAGE_SECTION_HEADER m_sectionHeaders = nullptr;

    size_t m_imageSize = 0;
    size_t m_headerSize = 0;
    uintptr_t m_entryPointRva = 0;
    uintptr_t m_originalBase = 0;

    bool ValidatePE();
    bool ProcessRelocationBlock(uintptr_t delta, PIMAGE_BASE_RELOCATION block);
};

// High-level driver mapping API
// Returns kernel base address or 0 on failure
uint64_t MapDriver(const char* driverPath);
uint64_t MapDriverFromMemory(const uint8_t* data, size_t size);

// Register driver callback with hypervisor (VMCALL_STORAGE_QUERY)
// Returns true if hypervisor communication succeeded
bool RegisterDriverCallback(uint64_t driverBase, uint64_t callbackRva);

// Call driver entry point via KernelForge
// entryPoint: kernel virtual address of DriverEntry
// Returns NTSTATUS from DriverEntry
NTSTATUS CallDriverEntry(uint64_t entryPoint, uint64_t driverBase, uint64_t registryPath);

// Internal implementation
namespace detail {
    // Allocate NonPagedPoolNx kernel memory
    void* AllocateDriverMemory(size_t size);

    // Copy mapped image to kernel memory
    bool CopyToKernel(void* kernelAddr, const void* data, size_t size);

    // Resolve kernel export by name
    uintptr_t GetKernelExport(const char* moduleName, const char* exportName);
}

} // namespace mapper
} // namespace zerohvci
