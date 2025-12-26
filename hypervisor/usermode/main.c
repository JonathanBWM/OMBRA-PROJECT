// main.c — OmbraHypervisor Loader Entry Point
// BYOVD loader using Ld9BoxSup.sys

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "driver_interface.h"
#include "payload_loader.h"
#include "debug_reader.h"

// =============================================================================
// Configuration
// =============================================================================

#define DRIVER_FILENAME     L"Ld9BoxSup.sys"
#define PAYLOAD_FILENAME    "hypervisor.bin"
#define DEBUG_POLL_INTERVAL 10  // ms

// =============================================================================
// Debug Monitor Thread
// =============================================================================

static volatile BOOL g_DebugMonitorRunning = FALSE;
static DebugReader g_DebugReader = {0};

static DWORD WINAPI DebugMonitorThread(LPVOID param) {
    HV_CONTEXT* ctx = (HV_CONTEXT*)param;
    DebugEntry entry;
    DWORD waitTime = DEBUG_POLL_INTERVAL;

    // Wait for hypervisor to initialize the debug buffer
    printf("[DBG] Debug monitor thread started, waiting for hypervisor...\n");

    // Give hypervisor time to initialize
    Sleep(100);

    // Initialize reader
    void* debugBuf = HvGetDebugBuffer(ctx);
    size_t debugSize = HvGetDebugBufferSize(ctx);

    if (!debugBuf || !debugSize) {
        printf("[DBG] No debug buffer available\n");
        return 1;
    }

    // Try to initialize reader - may fail if hypervisor hasn't set up the buffer yet
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

static wchar_t* GetDriverPath(void) {
    static wchar_t path[MAX_PATH];

    // Get directory of current executable
    GetModuleFileNameW(NULL, path, MAX_PATH);

    // Find last backslash
    wchar_t* lastSlash = wcsrchr(path, L'\\');
    if (lastSlash) {
        *(lastSlash + 1) = L'\0';
    }

    // Append driver filename
    wcscat_s(path, MAX_PATH, DRIVER_FILENAME);

    return path;
}

static bool FileExists(const wchar_t* path) {
    DWORD attr = GetFileAttributesW(path);
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
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

int main(int argc, char* argv[]) {
    printf("===========================================\n");
    printf("  OmbraHypervisor Loader\n");
    printf("  BYOVD via Ld9BoxSup.sys\n");
    printf("===========================================\n\n");

    // Check for admin privileges
    if (!CheckPrivileges()) {
        printf("[-] This program requires administrator privileges.\n");
        printf("[!] Right-click and select 'Run as administrator'\n");
        return 1;
    }
    printf("[+] Running with administrator privileges\n");

    // Get driver path
    wchar_t* driverPath = GetDriverPath();
    printf("[*] Driver path: %ls\n", driverPath);

    // Check driver exists
    if (!FileExists(driverPath)) {
        printf("[-] Driver not found: %ls\n", driverPath);
        printf("[!] Ensure Ld9BoxSup.sys is in the same directory as this executable\n");
        return 1;
    }
    printf("[+] Driver file found\n");

    // Load hypervisor payload
    size_t payloadSize = 0;
    void* payload = LoadPayload(PAYLOAD_FILENAME, &payloadSize);
    if (!payload) {
        printf("[-] Failed to load hypervisor payload\n");
        printf("[!] Ensure %s is in the same directory as this executable\n", PAYLOAD_FILENAME);
        return 1;
    }
    printf("[+] Loaded payload: %zu bytes\n", payloadSize);

    // Initialize hypervisor context
    HV_CONTEXT hvCtx;

    // Load and launch hypervisor
    if (!HvLoad(&hvCtx, driverPath, payload, payloadSize)) {
        printf("[-] Failed to load hypervisor\n");
        free(payload);
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
    if (HvIsRunning(&hvCtx)) {
        printf("\n[+] Hypervisor is active!\n");
        printf("[*] Debug output will appear above\n");
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

    // Unload
    HvUnload(&hvCtx);

    printf("[+] Done\n");
    return 0;
}
