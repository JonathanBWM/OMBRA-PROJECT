// main.c — Loader Entry Point
// BYOVD loader using Ld9BoxSup.sys

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <Windows.h>
#include "driver_interface.h"
#include "loader/hv_loader.h"
#include "debug_reader.h"
#include "obfuscate.h"
#include "tests/detection_baseline.h"
#include "byovd/supdrv.h"
#include "byovd/deployer.h"
#include "resources/resource_extract.h"

// =============================================================================
// Configuration
// =============================================================================

#define DRIVER_FILENAME     L"Ld9BoxSup.sys"
// Payload filename retrieved via obfuscation
static const char* GetPayloadFilename(void) {
    return DEC_HV_BIN();
}
#define DEBUG_POLL_INTERVAL 10  // ms
#define LOG_FILE            "ombra_debug.log"

// =============================================================================
// File Logging with Live Flush
// =============================================================================

static FILE* g_LogFile = NULL;
static CRITICAL_SECTION g_LogLock;

static void LogInit(void) {
    InitializeCriticalSection(&g_LogLock);
    g_LogFile = fopen(LOG_FILE, "w");
    if (g_LogFile) {
        // Disable buffering for immediate writes
        setvbuf(g_LogFile, NULL, _IONBF, 0);
        fprintf(g_LogFile, "=== OMBRA Debug Log ===\n");
        fprintf(g_LogFile, "Timestamp: %s %s\n\n", __DATE__, __TIME__);
        fflush(g_LogFile);
    }
}

static void LogShutdown(void) {
    if (g_LogFile) {
        fprintf(g_LogFile, "\n=== Log End ===\n");
        fclose(g_LogFile);
        g_LogFile = NULL;
    }
    DeleteCriticalSection(&g_LogLock);
}

static void LogWrite(const char* fmt, ...) {
    EnterCriticalSection(&g_LogLock);

    // Get timestamp
    SYSTEMTIME st;
    GetLocalTime(&st);

    // Print to console
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    // Print to file with timestamp
    if (g_LogFile) {
        fprintf(g_LogFile, "[%02d:%02d:%02d.%03d] ",
                st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
        va_start(args, fmt);
        vfprintf(g_LogFile, fmt, args);
        va_end(args);
        fflush(g_LogFile);  // Force write to disk immediately
    }

    LeaveCriticalSection(&g_LogLock);
}

// Macro to replace printf
#define LOG(fmt, ...) LogWrite(fmt, ##__VA_ARGS__)

// =============================================================================
// BigPool Visibility Test (Phase 4A)
// =============================================================================
//
// This test determines if SUPDrv's PageAllocEx allocations appear in
// Windows' PoolBigPageTable. If visible, we need mitigation strategies.
//
// Test flow:
// 1. Capture baseline BigPool count
// 2. Deploy Ld9BoxSup.sys driver
// 3. Establish session (cookie handshake)
// 4. Allocate memory via PageAllocEx
// 5. Capture new BigPool count
// 6. Compare: if delta > 0, allocation is tracked (BAD)
//

static int RunBigPoolVisibilityTest(void) {
    LOG("\n");
    LOG("===========================================\n");
    LOG("  BigPool Visibility Test (Phase 4A)\n");
    LOG("===========================================\n\n");

    DETECTION_BASELINE baseline = {0};

    // Step 1: Capture initial baseline (before any driver activity)
    LOG("[*] Step 1: Capturing initial system baseline...\n");
    if (!CaptureBaseline(&baseline, TRUE)) {
        LOG("[-] Failed to capture initial baseline\n");
        return 1;
    }
    LOG("[+] Initial BigPool entries: %u\n", baseline.Initial.BigPoolEntries);
    LOG("[+] Initial VBox tags: %u\n", baseline.Initial.VBoxTags);
    LOG("[+] Initial drivers loaded: %u\n\n", baseline.Initial.DriverCount);

    // Step 2: Initialize SUPDrv
    LOG("[*] Step 2: Deploying Ld9BoxSup.sys...\n");
    SUPDRV_CTX supCtx = {0};
    if (!SupDrv_Initialize(&supCtx)) {
        LOG("[-] Failed to initialize SUPDrv: %s\n", SupDrv_GetLastError(&supCtx));
        return 1;
    }
    LOG("[+] SUPDrv session established\n");
    LOG("    Cookie: 0x%08X\n", supCtx.Cookie);
    LOG("    Session: 0x%llX\n\n", (unsigned long long)supCtx.pSession);

    // Step 3: Capture post-driver baseline
    LOG("[*] Step 3: Capturing post-driver baseline...\n");
    DETECTION_BASELINE postDriver = {0};
    CaptureBaseline(&postDriver, TRUE);
    LOG("[+] Post-driver BigPool: %u (delta: %+d)\n",
        postDriver.Initial.BigPoolEntries,
        (int)postDriver.Initial.BigPoolEntries - (int)baseline.Initial.BigPoolEntries);
    LOG("[+] Post-driver VBox tags: %u (delta: %+d)\n",
        postDriver.Initial.VBoxTags,
        (int)postDriver.Initial.VBoxTags - (int)baseline.Initial.VBoxTags);
    LOG("[+] Post-driver drivers: %u (delta: %+d)\n\n",
        postDriver.Initial.DriverCount,
        (int)postDriver.Initial.DriverCount - (int)baseline.Initial.DriverCount);

    // Step 4: Allocate test memory via PageAllocEx
    LOG("[*] Step 4: Allocating test memory via PageAllocEx...\n");
    LOG("    Requesting 16 pages (64KB) with R3+R0 mapping...\n");

    void* pvR3 = NULL;
    void* pvR0 = NULL;
    if (!SupDrv_PageAllocEx(&supCtx, 16, &pvR3, &pvR0)) {
        LOG("[-] PageAllocEx failed: %s\n", SupDrv_GetLastError(&supCtx));
        SupDrv_Cleanup(&supCtx);
        return 1;
    }
    LOG("[+] Allocation successful!\n");
    LOG("    R3 address: 0x%016llX\n", (unsigned long long)(uintptr_t)pvR3);
    LOG("    R0 address: 0x%016llX\n", (unsigned long long)(uintptr_t)pvR0);
    LOG("    Size: %u bytes\n\n", 16 * 4096);

    // Step 5: Capture final baseline
    LOG("[*] Step 5: Capturing final baseline...\n");
    if (!CaptureBaseline(&baseline, FALSE)) {
        LOG("[-] Failed to capture final baseline\n");
        SupDrv_PageFree(&supCtx, pvR3);
        SupDrv_Cleanup(&supCtx);
        return 1;
    }

    // Step 6: Analysis
    LOG("\n");
    LOG("===========================================\n");
    LOG("  RESULTS\n");
    LOG("===========================================\n\n");

    PrintBaseline(&baseline);

    // The critical verdict
    LOG("\n[*] BigPool Visibility Verdict:\n");
    if (baseline.BigPoolDelta > 0) {
        LOG("[!] BAD: PageAllocEx allocations ARE visible in BigPool!\n");
        LOG("[!] Delta: +%d entries detected\n", baseline.BigPoolDelta);
        LOG("[!] Anti-cheat CAN detect these allocations\n");
        LOG("[!] Recommendation: Use MDL allocation or EPT-only memory\n");
    } else {
        LOG("[+] GOOD: PageAllocEx allocations are NOT visible in BigPool!\n");
        LOG("[+] Delta: %d entries (no increase)\n", baseline.BigPoolDelta);
        LOG("[+] Clean path is viable - allocations are hidden\n");
    }

    // Cleanup
    LOG("\n[*] Cleaning up...\n");
    SupDrv_PageFree(&supCtx, pvR3);
    SupDrv_Cleanup(&supCtx);
    LOG("[+] Test complete\n\n");

    return 0;
}

// =============================================================================
// Debug Monitor Thread
// =============================================================================

static volatile BOOL g_DebugMonitorRunning = FALSE;
static DebugReader g_DebugReader = {0};

static DWORD WINAPI DebugMonitorThread(LPVOID param) {
    HV_LOADER_CTX* ctx = (HV_LOADER_CTX*)param;
    DebugEntry entry;
    DWORD waitTime = DEBUG_POLL_INTERVAL;

    // Wait for payload to initialize the debug buffer
    DBG_PRINTF("[DBG] Debug monitor thread started, waiting for payload...\n");

    // Give payload time to initialize
    Sleep(100);

    // Initialize reader
    void* debugBuf = HvLoaderGetDebugBuffer(ctx);
    size_t debugSize = HvLoaderGetDebugBufferSize(ctx);

    if (!debugBuf || !debugSize) {
        printf("[DBG] No debug buffer available\n");
        return 1;
    }

    // Try to initialize reader - may fail if payload hasn't set up the buffer yet
    int retries = 50;  // 5 seconds max
    while (retries-- > 0 && !DbgReaderInit(&g_DebugReader, debugBuf, debugSize)) {
        Sleep(100);
    }

    if (!g_DebugReader.Initialized) {
        printf("[DBG] Failed to initialize debug reader\n");
        return 1;
    }

    printf("[DBG] Debug reader initialized, monitoring...\n");

    // Main polling loop
    while (g_DebugMonitorRunning) {
        while (DbgReaderReadOne(&g_DebugReader, &entry)) {
            DbgPrintEntry(&entry);
        }
        Sleep(waitTime);
    }

    DbgReaderShutdown(&g_DebugReader);
    printf("[DBG] Debug monitor thread stopped\n");
    return 0;
}

// =============================================================================
// Helper Functions
// =============================================================================

static bool FileExists(const wchar_t* path) {
    DWORD attr = GetFileAttributesW(path);
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

// Track if we extracted driver from resources (for cleanup)
static wchar_t* g_ExtractedDriverPath = NULL;

static wchar_t* FindDriverPath_Legacy(void) {
    static wchar_t path[MAX_PATH];

    // Strategy 1: Check known LDPlayer installation paths
    // Uses KNOWN_DRIVER_PATHS from deployer.h
    for (size_t i = 0; i < KNOWN_DRIVER_PATHS_COUNT; i++) {
        if (FileExists(KNOWN_DRIVER_PATHS[i])) {
            wcscpy_s(path, MAX_PATH, KNOWN_DRIVER_PATHS[i]);
            LOG("[+] Found driver at known path: %ls\n", path);
            return path;
        }
    }

    // Strategy 2: Check local directory (same as executable)
    GetModuleFileNameW(NULL, path, MAX_PATH);
    wchar_t* lastSlash = wcsrchr(path, L'\\');
    if (lastSlash) {
        *(lastSlash + 1) = L'\0';
    }
    wcscat_s(path, MAX_PATH, DRIVER_FILENAME);

    if (FileExists(path)) {
        LOG("[+] Found driver in local directory: %ls\n", path);
        return path;
    }

    // Driver not found in any location
    return NULL;
}

static wchar_t* GetDriverPath(void) {
    // FIRST: Try to extract from embedded resources (the correct way)
    if (Resource_Exists(EMBEDDED_DRIVER_LD9BOXSUP)) {
        wchar_t* extracted = Resource_ExtractToTemp(EMBEDDED_DRIVER_LD9BOXSUP);
        if (extracted) {
            LOG("[+] Using embedded Ld9BoxSup.sys driver\n");
            g_ExtractedDriverPath = extracted;  // Track for cleanup
            return extracted;
        }
        LOG("[!] Resource exists but extraction failed\n");
    }

    // FALLBACK ONLY: Check filesystem (for development/debugging only)
    LOG("[!] WARNING: No embedded driver, falling back to filesystem search\n");
    return FindDriverPath_Legacy();
}

static void* LoadPayload(const char* filename, size_t* size) {
    // Get directory of current executable
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);

    char* lastSlash = strrchr(path, '\\');
    if (lastSlash) {
        *(lastSlash + 1) = '\0';
    }
    strcat_s(path, MAX_PATH, filename);

    FILE* f = fopen(path, "rb");
    if (!f) {
        printf("[-] Failed to open payload: %s\n", path);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, SEEK_SET);

    void* data = malloc(*size);
    if (!data) {
        fclose(f);
        return NULL;
    }

    if (fread(data, 1, *size, f) != *size) {
        free(data);
        fclose(f);
        return NULL;
    }

    fclose(f);
    return data;
}

static bool CheckPrivileges(void) {
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    PSID adminGroup;

    if (AllocateAndInitializeSid(&ntAuthority, 2,
            SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }

    return isAdmin != FALSE;
}

// =============================================================================
// Main
// =============================================================================

static void PrintUsage(const char* progName) {
    printf("Usage: %s [options]\n\n", progName);
    printf("Options:\n");
    printf("  --bigpool-test    Run BigPool visibility test (Phase 4A)\n");
    printf("  --help            Show this help message\n");
    printf("\n");
    printf("Default: Load hypervisor payload\n");
}

int main(int argc, char* argv[]) {
    // Initialize logging first
    LogInit();

    LOG("===========================================\n");
    LOG("  %s\n", DEC_LOADER_TITLE());
    LOG("  BYOVD via Ld9BoxSup.sys\n");
    LOG("===========================================\n\n");

    // Parse command line arguments
    bool runBigPoolTest = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--bigpool-test") == 0) {
            runBigPoolTest = true;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            PrintUsage(argv[0]);
            LogShutdown();
            return 0;
        } else {
            LOG("[-] Unknown argument: %s\n", argv[i]);
            PrintUsage(argv[0]);
            LogShutdown();
            return 1;
        }
    }

    // Check for admin privileges
    if (!CheckPrivileges()) {
        LOG("[-] This program requires administrator privileges.\n");
        LOG("[!] Right-click and select 'Run as administrator'\n");
        LogShutdown();
        return 1;
    }
    LOG("[+] Running with administrator privileges\n");

    // Verify embedded resources are present (build sanity check)
    LOG("[*] Verifying embedded resources...\n");
    if (!Resource_Exists(EMBEDDED_DRIVER_LD9BOXSUP)) {
        LOG("[!] WARNING: Ld9BoxSup.sys NOT embedded - falling back to filesystem\n");
    } else {
        LOG("[+] VERIFIED: Ld9BoxSup.sys is embedded\n");
    }
    if (!Resource_Exists(EMBEDDED_DRIVER_THROTTLESTOP)) {
        LOG("[!] WARNING: ThrottleStop.sys NOT embedded - falling back to filesystem\n");
    } else {
        LOG("[+] VERIFIED: ThrottleStop.sys is embedded\n");
    }

    // Run BigPool test if requested
    if (runBigPoolTest) {
        int result = RunBigPoolVisibilityTest();
        LogShutdown();
        return result;
    }

    // Get driver path (embedded resources first, filesystem fallback)
    LOG("[*] Locating Ld9BoxSup.sys driver...\n");
    wchar_t* driverPath = GetDriverPath();
    if (!driverPath) {
        LOG("[-] Driver not found in any location\n");
        LOG("[!] Build error: Resources not embedded AND no filesystem driver found\n");
        LOG("[!] Searched filesystem locations:\n");
        for (size_t i = 0; i < KNOWN_DRIVER_PATHS_COUNT; i++) {
            LOG("      - %ls\n", KNOWN_DRIVER_PATHS[i]);
        }
        LogShutdown();
        return 1;
    }

    // Load payload
    size_t payloadSize = 0;
    const char* payloadFile = GetPayloadFilename();
    void* payload = LoadPayload(payloadFile, &payloadSize);
    if (!payload) {
        printf("[-] Failed to load payload\n");
        printf("[!] Ensure payload file is in the same directory as this executable\n");
        return 1;
    }
    printf("[+] Loaded payload: %zu bytes\n", payloadSize);

    // Initialize loader context
    HV_LOADER_CTX hvCtx = {0};

    // Initialize driver and establish session
    if (!HvLoaderInit(&hvCtx, driverPath)) {
        printf("[-] Failed to initialize loader\n");
        free(payload);
        return 1;
    }

    // Load hypervisor into kernel and launch
    if (!HvLoaderLoad(&hvCtx, payload, (U32)payloadSize)) {
        printf("[-] Failed to load hypervisor\n");
        free(payload);
        HvLoaderCleanup(&hvCtx);
        return 1;
    }

    free(payload);

    // Start debug monitor thread
    HANDLE hDebugThread = NULL;
    g_DebugMonitorRunning = TRUE;
    hDebugThread = CreateThread(NULL, 0, DebugMonitorThread, &hvCtx, 0, NULL);
    if (!hDebugThread) {
        printf("[!] Failed to start debug monitor thread\n");
    }

    // Check if running
    if (HvLoaderIsRunning(&hvCtx)) {
        printf("\n[+] Hypervisor is active!\n");
        DBG_PRINTF("[*] Debug output will appear above\n");
        printf("[*] Press Enter to unload...\n");
        getchar();
    } else {
        printf("\n[!] Hypervisor loaded but may not be fully active\n");
        printf("[*] Press Enter to unload...\n");
        getchar();
    }

    // Stop debug monitor thread
    if (hDebugThread) {
        g_DebugMonitorRunning = FALSE;
        WaitForSingleObject(hDebugThread, 2000);  // Wait up to 2 seconds
        CloseHandle(hDebugThread);
    }

    // Unload hypervisor and cleanup
    HvLoaderUnload(&hvCtx);
    HvLoaderCleanup(&hvCtx);

    // Cleanup extracted driver if we used embedded resources
    if (g_ExtractedDriverPath) {
        LOG("[*] Cleaning up extracted driver...\n");
        if (Resource_Cleanup(g_ExtractedDriverPath)) {
            LOG("[+] Extracted driver securely deleted\n");
        } else {
            LOG("[!] Warning: Failed to cleanup extracted driver\n");
        }
        free(g_ExtractedDriverPath);
        g_ExtractedDriverPath = NULL;
    }

    LOG("[+] Done\n");
    LogShutdown();
    return 0;
}
