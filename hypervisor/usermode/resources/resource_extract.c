#include "resource_extract.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const wchar_t* Resource_GetName(EMBEDDED_RESOURCE_ID resourceId) {
    switch (resourceId) {
        case EMBEDDED_DRIVER_LD9BOXSUP:     return L"Ld9BoxSup.sys";
        case EMBEDDED_DRIVER_THROTTLESTOP:  return L"ThrottleStop.sys";
        case EMBEDDED_PAYLOAD_HYPERVISOR:   return L"hypervisor.bin";
        default:                            return L"unknown";
    }
}

bool Resource_Exists(EMBEDDED_RESOURCE_ID resourceId) {
    HRSRC hRes = FindResourceW(NULL, MAKEINTRESOURCEW(resourceId), RT_RCDATA);
    return (hRes != NULL);
}

wchar_t* Resource_ExtractToTemp(EMBEDDED_RESOURCE_ID resourceId) {
    // Find resource in our executable
    HRSRC hRes = FindResourceW(NULL, MAKEINTRESOURCEW(resourceId), RT_RCDATA);
    if (!hRes) {
        wprintf(L"[-] FindResource failed for %ls (error %lu)\n",
                Resource_GetName(resourceId), GetLastError());
        return NULL;
    }

    // Load and lock resource
    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) {
        wprintf(L"[-] LoadResource failed for %ls\n", Resource_GetName(resourceId));
        return NULL;
    }

    void* pData = LockResource(hData);
    DWORD dwSize = SizeofResource(NULL, hRes);
    if (!pData || dwSize == 0) {
        wprintf(L"[-] LockResource/SizeofResource failed for %ls\n", Resource_GetName(resourceId));
        return NULL;
    }

    // Build output path in temp directory
    wchar_t tempDir[MAX_PATH];
    if (!GetTempPathW(MAX_PATH, tempDir)) {
        wprintf(L"[-] GetTempPath failed\n");
        return NULL;
    }

    wchar_t* outputPath = (wchar_t*)malloc(MAX_PATH * sizeof(wchar_t));
    if (!outputPath) return NULL;

    swprintf_s(outputPath, MAX_PATH, L"%sombra_%ls", tempDir, Resource_GetName(resourceId));

    // Write to temp file
    HANDLE hFile = CreateFileW(outputPath, GENERIC_WRITE, 0, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        wprintf(L"[-] CreateFile failed for %ls (error %lu)\n", outputPath, GetLastError());
        free(outputPath);
        return NULL;
    }

    DWORD written;
    BOOL writeOk = WriteFile(hFile, pData, dwSize, &written, NULL);
    CloseHandle(hFile);

    if (!writeOk || written != dwSize) {
        wprintf(L"[-] WriteFile failed for %ls\n", outputPath);
        DeleteFileW(outputPath);
        free(outputPath);
        return NULL;
    }

    wprintf(L"[+] Extracted %ls (%lu bytes) -> %ls\n",
            Resource_GetName(resourceId), dwSize, outputPath);
    return outputPath;
}

void* Resource_ExtractToMemory(EMBEDDED_RESOURCE_ID resourceId, size_t* outSize) {
    HRSRC hRes = FindResourceW(NULL, MAKEINTRESOURCEW(resourceId), RT_RCDATA);
    if (!hRes) return NULL;

    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) return NULL;

    void* pData = LockResource(hData);
    DWORD dwSize = SizeofResource(NULL, hRes);
    if (!pData || dwSize == 0) return NULL;

    void* buffer = malloc(dwSize);
    if (!buffer) return NULL;

    memcpy(buffer, pData, dwSize);
    if (outSize) *outSize = dwSize;

    return buffer;
}

bool Resource_Cleanup(const wchar_t* extractedPath) {
    if (!extractedPath) return false;

    // Overwrite with zeros before deletion (basic secure delete)
    HANDLE hFile = CreateFileW(extractedPath, GENERIC_WRITE, 0, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(hFile, &fileSize) && fileSize.QuadPart > 0 && fileSize.QuadPart < 10*1024*1024) {
            void* zeros = calloc(1, (size_t)fileSize.QuadPart);
            if (zeros) {
                DWORD written;
                SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
                WriteFile(hFile, zeros, (DWORD)fileSize.QuadPart, &written, NULL);
                free(zeros);
            }
        }
        CloseHandle(hFile);
    }

    return DeleteFileW(extractedPath) != 0;
}
