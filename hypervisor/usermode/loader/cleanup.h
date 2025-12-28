#ifndef CLEANUP_H
#define CLEANUP_H

#include <windows.h>

/**
 * Forensic Cleanup Module
 *
 * This module provides cleanup routines to eliminate forensic traces after
 * driver unload. Some operations require kernel-mode execution (via VMCALL
 * or during hypervisor initialization) and are provided as documented stubs.
 *
 * Usermode operations (prefetch deletion) are fully implemented.
 */

/**
 * CleanupMmUnloadedDrivers - Clear MmUnloadedDrivers array entries
 *
 * STATUS: STUB - Requires kernel execution
 *
 * MmUnloadedDrivers is an array in ntoskrnl.exe that tracks recently unloaded
 * drivers. Anti-cheat systems scan this to detect BYOVD exploitation.
 *
 * IMPLEMENTATION REQUIREMENTS:
 * - Locate MmUnloadedDrivers in ntoskrnl.exe (signature scan or hardcoded offset)
 * - Locate MiUnloadedDrivers count variable
 * - Zero out entries matching our driver names (Ld9BoxSup.sys, ThrottleStop.sys)
 * - Shift subsequent entries to fill gaps
 * - Decrement MiUnloadedDrivers count
 *
 * STRUCTURE (simplified):
 * typedef struct _UNLOADED_DRIVERS {
 *     UNICODE_STRING Name;
 *     PVOID StartAddress;
 *     PVOID EndAddress;
 *     LARGE_INTEGER CurrentTime;
 * } UNLOADED_DRIVERS, *PUNLOADED_DRIVERS;
 *
 * EXECUTION METHOD:
 * - Via VMCALL from usermode to hypervisor
 * - Or during hypervisor initialization (PhysicalMemoryRead via Ld9BoxSup)
 *
 * WINDOWS VERSION NOTES:
 * - Array size: 50 entries (Windows 10+)
 * - Stored in non-paged pool
 * - Protected by MmUnloadedDriversLock spinlock (acquire if multithreaded)
 *
 * @return TRUE if cleanup succeeded, FALSE otherwise
 */
BOOL CleanupMmUnloadedDrivers(void);

/**
 * CleanupPiDDBCacheTable - Clear PiDDBCacheTable entries
 *
 * STATUS: STUB - Requires kernel execution
 *
 * PiDDBCacheTable is a hash table in ntoskrnl.exe that caches driver signature
 * verification results. Entries persist after driver unload, creating forensic
 * artifacts detectable by anti-cheat.
 *
 * IMPLEMENTATION REQUIREMENTS:
 * - Locate PiDDBCacheTable and PiDDBLock in ntoskrnl.exe
 * - Acquire PiDDBLock spinlock
 * - Hash driver name to find bucket index
 * - Walk AVL tree in bucket to find matching entries
 * - Unlink nodes from tree
 * - Free node memory (or zero in place)
 * - Release PiDDBLock
 *
 * STRUCTURE (simplified):
 * typedef struct _PIDDB_CACHE_ENTRY {
 *     LIST_ENTRY List;
 *     UNICODE_STRING DriverName;
 *     ULONG TimeDateStamp;
 *     NTSTATUS LoadStatus;
 * } PIDDB_CACHE_ENTRY, *PPIDDB_CACHE_ENTRY;
 *
 * HASH FUNCTION:
 * - Windows uses RtlHashUnicodeString (hash driver name)
 * - Bucket index = hash % bucket_count
 * - Each bucket is AVL tree root
 *
 * EXECUTION METHOD:
 * - Via VMCALL from usermode to hypervisor
 * - Or during hypervisor initialization (PhysicalMemoryRead via Ld9BoxSup)
 *
 * WINDOWS VERSION NOTES:
 * - Structure offsets vary by Windows version (use Vergilius MCP tools)
 * - Protected by PiDDBLock spinlock (MUST acquire)
 * - Clearing this prevents detection via ZwQuerySystemInformation(SystemModuleInformation)
 *
 * @return TRUE if cleanup succeeded, FALSE otherwise
 */
BOOL CleanupPiDDBCacheTable(void);

/**
 * CleanupEtwBuffers - Clear ETW trace buffers
 *
 * STATUS: STUB - Requires kernel execution
 *
 * ETW (Event Tracing for Windows) logs driver load/unload events, memory
 * allocations, and suspicious API calls. These logs persist in kernel buffers
 * and can be read by anti-cheat even after driver unload.
 *
 * IMPLEMENTATION REQUIREMENTS:
 * - Locate active ETW trace sessions (EtwpSessionDemuxList in ntoskrnl.exe)
 * - Walk session list to find Microsoft-Windows-Kernel-Process provider
 * - Zero buffer memory or mark events as invalid
 * - Optionally disable specific ETW providers entirely
 *
 * KEY PROVIDERS TO TARGET:
 * - Microsoft-Windows-Kernel-Process (driver loads)
 * - Microsoft-Windows-Kernel-Memory (pool allocations)
 * - Microsoft-Windows-Threat-Intelligence (suspicious activity)
 *
 * STRUCTURE (simplified):
 * typedef struct _ETW_SESSION {
 *     LIST_ENTRY SessionList;
 *     PVOID BufferQueue;
 *     ULONG BufferSize;
 *     ULONG NumberOfBuffers;
 * } ETW_SESSION, *PETW_SESSION;
 *
 * EXECUTION METHOD:
 * - Via VMCALL from usermode to hypervisor
 * - Ideally done during hypervisor initialization (before anti-cheat scans)
 *
 * ALTERNATIVE APPROACH:
 * - Patch EtwpEventWriteFull to filter our events (requires EPT hook)
 * - Use InfinityHook-style syscall hooking to intercept NtTraceEvent
 *
 * WINDOWS VERSION NOTES:
 * - ETW internals change frequently between Windows versions
 * - Use pattern scanning, not hardcoded offsets
 * - Some traces are protected (PPL) - requires PPL bypass or hypervisor
 *
 * @return TRUE if cleanup succeeded, FALSE otherwise
 */
BOOL CleanupEtwBuffers(void);

/**
 * CleanupPrefetchFiles - Delete prefetch files for loaded drivers
 *
 * STATUS: FULLY IMPLEMENTED
 *
 * Windows Prefetch creates .pf files in C:\Windows\Prefetch for frequently
 * accessed executables and drivers. These files persist after driver unload
 * and reveal execution history.
 *
 * This function deletes prefetch files for:
 * - Ld9BoxSup.sys (BYOVD driver)
 * - ThrottleStop.sys (BYOVD driver)
 * - loader.exe (this loader)
 *
 * IMPLEMENTATION:
 * - Uses wildcard patterns (LD9BOXSUP*.pf, THROTTLESTOP*.pf, LOADER*.pf)
 * - Windows appends hash to filename, so wildcard is necessary
 * - Requires Administrator privileges (Prefetch folder is protected)
 *
 * ERROR HANDLING:
 * - Ignores errors if files don't exist (anti-cheat may have disabled prefetch)
 * - Logs failures to debug output
 *
 * @return TRUE if all deletions succeeded (or files didn't exist), FALSE otherwise
 */
BOOL CleanupPrefetchFiles(void);

/**
 * PerformForensicCleanup - Execute all forensic cleanup operations
 *
 * Main entry point for forensic cleanup. Calls all cleanup functions in
 * optimal order:
 *
 * 1. CleanupEtwBuffers (stop new events from being logged)
 * 2. CleanupMmUnloadedDrivers (clear unloaded driver list)
 * 3. CleanupPiDDBCacheTable (clear driver signature cache)
 * 4. CleanupPrefetchFiles (delete filesystem artifacts)
 *
 * USAGE:
 * Call this after driver unload but before process exit. For kernel-side
 * cleanups, ensure hypervisor is initialized and VMCALL interface is active.
 *
 * @param bKernelCleanupAvailable - TRUE if hypervisor is active (enables kernel cleanups)
 * @return TRUE if all cleanups succeeded, FALSE if any failed
 */
BOOL PerformForensicCleanup(BOOL bKernelCleanupAvailable);

#endif // CLEANUP_H
