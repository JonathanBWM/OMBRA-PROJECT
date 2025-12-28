#include "cleanup.h"
#include <stdio.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

/**
 * Helper function to delete files matching a wildcard pattern
 *
 * @param pattern - Wildcard pattern (e.g., L"C:\\Windows\\Prefetch\\LD9BOXSUP*.pf")
 * @return Number of files deleted
 */
static DWORD DeleteFilesMatchingPattern(LPCWSTR pattern) {
    WIN32_FIND_DATAW findData;
    HANDLE hFind;
    DWORD deletedCount = 0;

    hFind = FindFirstFileW(pattern, &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        // No files matching pattern (could be normal if prefetch disabled)
        return 0;
    }

    do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            // Build full path
            WCHAR fullPath[MAX_PATH];
            WCHAR directory[MAX_PATH];
            wcscpy_s(directory, MAX_PATH, pattern);
            PathRemoveFileSpecW(directory); // Remove filename, keep directory

            swprintf_s(fullPath, MAX_PATH, L"%s\\%s", directory, findData.cFileName);

            if (DeleteFileW(fullPath)) {
                deletedCount++;
#ifdef _DEBUG
                wprintf(L"[CLEANUP] Deleted: %s\n", fullPath);
#endif
            } else {
#ifdef _DEBUG
                wprintf(L"[CLEANUP] Failed to delete %s (error %lu)\n", fullPath, GetLastError());
#endif
            }
        }
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
    return deletedCount;
}

BOOL CleanupMmUnloadedDrivers(void) {
    // STUB: Requires kernel execution via VMCALL or hypervisor initialization

#ifdef _DEBUG
    printf("[CLEANUP] CleanupMmUnloadedDrivers - STUB\n");
    printf("         This requires kernel-mode execution:\n");
    printf("         1. Locate MmUnloadedDrivers in ntoskrnl.exe\n");
    printf("         2. Locate MiUnloadedDrivers count variable\n");
    printf("         3. Zero entries matching Ld9BoxSup.sys, ThrottleStop.sys\n");
    printf("         4. Shift array and decrement count\n");
    printf("         Implementation: VMCALL or PhysicalMemoryRead during HV init\n");
#endif

    // TODO: Implement via VMCALL interface
    // Example VMCALL signature:
    // __vmx_vmcall(VMCALL_CLEANUP_UNLOADED_DRIVERS, 0, 0, 0);

    return FALSE; // Not implemented
}

BOOL CleanupPiDDBCacheTable(void) {
    // STUB: Requires kernel execution via VMCALL or hypervisor initialization

#ifdef _DEBUG
    printf("[CLEANUP] CleanupPiDDBCacheTable - STUB\n");
    printf("         This requires kernel-mode execution:\n");
    printf("         1. Locate PiDDBCacheTable and PiDDBLock in ntoskrnl.exe\n");
    printf("         2. Acquire PiDDBLock spinlock\n");
    printf("         3. Hash driver names and walk AVL trees\n");
    printf("         4. Unlink matching nodes and free memory\n");
    printf("         5. Release PiDDBLock\n");
    printf("         Implementation: VMCALL or PhysicalMemoryRead during HV init\n");
#endif

    // TODO: Implement via VMCALL interface
    // Example VMCALL signature:
    // __vmx_vmcall(VMCALL_CLEANUP_PIDDB_CACHE, 0, 0, 0);

    return FALSE; // Not implemented
}

BOOL CleanupEtwBuffers(void) {
    // STUB: Requires kernel execution via VMCALL or hypervisor initialization

#ifdef _DEBUG
    printf("[CLEANUP] CleanupEtwBuffers - STUB\n");
    printf("         This requires kernel-mode execution:\n");
    printf("         1. Locate EtwpSessionDemuxList in ntoskrnl.exe\n");
    printf("         2. Walk session list to find target providers:\n");
    printf("            - Microsoft-Windows-Kernel-Process\n");
    printf("            - Microsoft-Windows-Kernel-Memory\n");
    printf("            - Microsoft-Windows-Threat-Intelligence\n");
    printf("         3. Zero buffer memory or mark events invalid\n");
    printf("         4. Optionally disable providers entirely\n");
    printf("         Implementation: VMCALL or EPT hook on EtwpEventWriteFull\n");
#endif

    // TODO: Implement via VMCALL interface or EPT hook
    // Example VMCALL signature:
    // __vmx_vmcall(VMCALL_CLEANUP_ETW_BUFFERS, 0, 0, 0);

    return FALSE; // Not implemented
}

BOOL CleanupPrefetchFiles(void) {
    DWORD totalDeleted = 0;

#ifdef _DEBUG
    printf("[CLEANUP] CleanupPrefetchFiles - Deleting prefetch artifacts\n");
#endif

    // Delete Ld9BoxSup.sys prefetch files
    totalDeleted += DeleteFilesMatchingPattern(L"C:\\Windows\\Prefetch\\LD9BOXSUP*.pf");

    // Delete ThrottleStop.sys prefetch files
    totalDeleted += DeleteFilesMatchingPattern(L"C:\\Windows\\Prefetch\\THROTTLESTOP*.pf");

    // Delete loader.exe prefetch files
    totalDeleted += DeleteFilesMatchingPattern(L"C:\\Windows\\Prefetch\\LOADER*.pf");

    // Also delete hypervisor.sys if it exists (future-proofing)
    totalDeleted += DeleteFilesMatchingPattern(L"C:\\Windows\\Prefetch\\HYPERVISOR*.pf");

#ifdef _DEBUG
    printf("[CLEANUP] Deleted %lu prefetch file(s)\n", totalDeleted);
#endif

    // Consider this successful even if no files were deleted
    // (prefetch may be disabled, or files already cleaned)
    return TRUE;
}

BOOL PerformForensicCleanup(BOOL bKernelCleanupAvailable) {
    BOOL bSuccess = TRUE;

#ifdef _DEBUG
    printf("[CLEANUP] ========================================\n");
    printf("[CLEANUP] Starting forensic cleanup\n");
    printf("[CLEANUP] Kernel cleanup available: %s\n", bKernelCleanupAvailable ? "YES" : "NO");
    printf("[CLEANUP] ========================================\n");
#endif

    // Step 1: ETW buffer cleanup (prevents new events from being logged)
    if (bKernelCleanupAvailable) {
        if (!CleanupEtwBuffers()) {
#ifdef _DEBUG
            printf("[CLEANUP] WARNING: ETW buffer cleanup failed (stub not implemented)\n");
#endif
            bSuccess = FALSE;
        }
    } else {
#ifdef _DEBUG
        printf("[CLEANUP] Skipping ETW cleanup (requires kernel access)\n");
#endif
    }

    // Step 2: MmUnloadedDrivers cleanup (clear unloaded driver list)
    if (bKernelCleanupAvailable) {
        if (!CleanupMmUnloadedDrivers()) {
#ifdef _DEBUG
            printf("[CLEANUP] WARNING: MmUnloadedDrivers cleanup failed (stub not implemented)\n");
#endif
            bSuccess = FALSE;
        }
    } else {
#ifdef _DEBUG
        printf("[CLEANUP] Skipping MmUnloadedDrivers cleanup (requires kernel access)\n");
#endif
    }

    // Step 3: PiDDBCacheTable cleanup (clear driver signature cache)
    if (bKernelCleanupAvailable) {
        if (!CleanupPiDDBCacheTable()) {
#ifdef _DEBUG
            printf("[CLEANUP] WARNING: PiDDBCacheTable cleanup failed (stub not implemented)\n");
#endif
            bSuccess = FALSE;
        }
    } else {
#ifdef _DEBUG
        printf("[CLEANUP] Skipping PiDDBCacheTable cleanup (requires kernel access)\n");
#endif
    }

    // Step 4: Prefetch file cleanup (always perform, usermode operation)
    if (!CleanupPrefetchFiles()) {
#ifdef _DEBUG
        printf("[CLEANUP] WARNING: Prefetch file cleanup failed\n");
#endif
        bSuccess = FALSE;
    }

#ifdef _DEBUG
    printf("[CLEANUP] ========================================\n");
    printf("[CLEANUP] Forensic cleanup completed: %s\n", bSuccess ? "SUCCESS" : "PARTIAL FAILURE");
    printf("[CLEANUP] ========================================\n");
#endif

    return bSuccess;
}
