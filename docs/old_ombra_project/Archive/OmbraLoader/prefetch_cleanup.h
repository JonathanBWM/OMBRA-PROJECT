// OmbraLoader/prefetch_cleanup.h
// Phase 3: Prefetch File Cleanup for Execution Trace Elimination
// Securely deletes prefetch files that record loader execution

#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <cstdint>

namespace prefetch {

//===----------------------------------------------------------------------===//
// Secure File Deletion
//===----------------------------------------------------------------------===//

// Overwrite file content with random data before deletion
// This defeats simple undelete tools and reduces MFT forensic value
inline bool SecureDeleteFile(const std::wstring& path) {
    // Open file for writing
    HANDLE hFile = CreateFileW(
        path.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    // Get file size
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        CloseHandle(hFile);
        return DeleteFileW(path.c_str()) != 0;
    }

    // Overwrite with random data (3 passes for DoD-style wipe)
    constexpr size_t BUFFER_SIZE = 4096;
    std::vector<uint8_t> buffer(BUFFER_SIZE);

    for (int pass = 0; pass < 3; ++pass) {
        // Seek to beginning
        SetFilePointer(hFile, 0, nullptr, FILE_BEGIN);

        // Fill with different patterns each pass
        uint8_t pattern = (pass == 0) ? 0x00 : (pass == 1) ? 0xFF : 0x00;

        // Final pass uses random data
        if (pass == 2) {
            for (size_t i = 0; i < BUFFER_SIZE; ++i) {
                buffer[i] = static_cast<uint8_t>(__rdtsc() ^ i);
            }
        } else {
            memset(buffer.data(), pattern, BUFFER_SIZE);
        }

        // Write until file is covered
        LONGLONG remaining = fileSize.QuadPart;
        while (remaining > 0) {
            DWORD toWrite = static_cast<DWORD>(min(static_cast<LONGLONG>(BUFFER_SIZE), remaining));
            DWORD written = 0;

            // Refresh random data for final pass
            if (pass == 2) {
                for (size_t i = 0; i < toWrite; ++i) {
                    buffer[i] = static_cast<uint8_t>(__rdtsc() ^ i);
                }
            }

            if (!WriteFile(hFile, buffer.data(), toWrite, &written, nullptr)) {
                break;
            }

            remaining -= written;
        }

        FlushFileBuffers(hFile);
    }

    CloseHandle(hFile);

    // Now delete the file
    return DeleteFileW(path.c_str()) != 0;
}

//===----------------------------------------------------------------------===//
// Prefetch File Discovery
//===----------------------------------------------------------------------===//

// Get the Windows Prefetch directory path
inline std::wstring GetPrefetchDirectory() {
    wchar_t windir[MAX_PATH];
    if (GetWindowsDirectoryW(windir, MAX_PATH) == 0) {
        return L"";
    }

    return std::wstring(windir) + L"\\Prefetch";
}

// Find prefetch files matching a pattern (case-insensitive)
inline std::vector<std::wstring> FindPrefetchFiles(const std::wstring& pattern) {
    std::vector<std::wstring> results;

    std::wstring prefetchDir = GetPrefetchDirectory();
    if (prefetchDir.empty()) {
        return results;
    }

    std::wstring searchPath = prefetchDir + L"\\*" + pattern + L"*.pf";

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return results;
    }

    do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            results.push_back(prefetchDir + L"\\" + findData.cFileName);
        }
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
    return results;
}

// Find prefetch files for current executable
inline std::vector<std::wstring> FindOwnPrefetchFiles() {
    wchar_t exePath[MAX_PATH];
    if (GetModuleFileNameW(nullptr, exePath, MAX_PATH) == 0) {
        return {};
    }

    // Extract just the filename without path
    std::wstring exeName = exePath;
    size_t lastSlash = exeName.find_last_of(L"\\/");
    if (lastSlash != std::wstring::npos) {
        exeName = exeName.substr(lastSlash + 1);
    }

    // Remove .exe extension if present
    size_t dotPos = exeName.rfind(L".exe");
    if (dotPos != std::wstring::npos) {
        exeName = exeName.substr(0, dotPos);
    }

    return FindPrefetchFiles(exeName);
}

//===----------------------------------------------------------------------===//
// Cleanup Interface
//===----------------------------------------------------------------------===//

struct CleanupResult {
    int files_found;
    int files_deleted;
    int files_failed;
    std::vector<std::wstring> failed_paths;
};

// Delete prefetch files for a specific executable name
inline CleanupResult CleanupPrefetchFor(const std::wstring& exeName) {
    CleanupResult result = {};

    auto files = FindPrefetchFiles(exeName);
    result.files_found = static_cast<int>(files.size());

    for (const auto& file : files) {
        if (SecureDeleteFile(file)) {
            result.files_deleted++;
        } else {
            result.files_failed++;
            result.failed_paths.push_back(file);
        }
    }

    return result;
}

// Delete prefetch files for the current loader executable
inline CleanupResult CleanupOwnPrefetch() {
    CleanupResult result = {};

    auto files = FindOwnPrefetchFiles();
    result.files_found = static_cast<int>(files.size());

    for (const auto& file : files) {
        if (SecureDeleteFile(file)) {
            result.files_deleted++;
        } else {
            result.files_failed++;
            result.failed_paths.push_back(file);
        }
    }

    return result;
}

// Delete prefetch files for known artifact names
inline CleanupResult CleanupKnownArtifacts() {
    CleanupResult result = {};

    // List of known executable names that might leave prefetch traces
    const std::vector<std::wstring> known_artifacts = {
        L"OMBRALOADER",
        L"OMBRA",
        L"LD9BOXSUP",
        L"VBOXSUP",
        L"CSC",
        L"KS"
    };

    for (const auto& name : known_artifacts) {
        auto partial = CleanupPrefetchFor(name);
        result.files_found += partial.files_found;
        result.files_deleted += partial.files_deleted;
        result.files_failed += partial.files_failed;
        result.failed_paths.insert(
            result.failed_paths.end(),
            partial.failed_paths.begin(),
            partial.failed_paths.end()
        );
    }

    return result;
}

// RAII wrapper for cleanup on scope exit
class ScopedPrefetchCleanup {
public:
    ScopedPrefetchCleanup() = default;

    ~ScopedPrefetchCleanup() {
        // Clean up on destruction
        CleanupOwnPrefetch();
        CleanupKnownArtifacts();
    }

    // Non-copyable
    ScopedPrefetchCleanup(const ScopedPrefetchCleanup&) = delete;
    ScopedPrefetchCleanup& operator=(const ScopedPrefetchCleanup&) = delete;
};

} // namespace prefetch
