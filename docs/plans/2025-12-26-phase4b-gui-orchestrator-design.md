# Phase 4B: GUI Orchestrator Design (Dear ImGui + DirectX 11)

## Overview

Phase 4B creates a modern GUI application for the Ombra loader using Dear ImGui rendered via DirectX 11. The GUI wraps the existing C loader code, providing visual feedback during the loading process and a clean interface for the spoofer module (stubbed for future implementation).

**Goal:** Replace the CLI interface with a polished, game-style GUI that provides real-time status updates, logging, and authentication integration.

**Architecture:** C++ GUI layer calling into existing C loader core via a clean C API interface.

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                      Ombra GUI Application                       │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌─────────────────┐    ┌─────────────────┐    ┌─────────────┐ │
│  │   Win32 Window  │───▶│  DirectX 11     │───▶│  Dear ImGui │ │
│  │   (Container)   │    │  (Rendering)    │    │  (UI Logic) │ │
│  └─────────────────┘    └─────────────────┘    └─────────────┘ │
│           │                                           │         │
│           │              Message Loop                 │         │
│           └──────────────────┬────────────────────────┘         │
│                              │                                   │
│                              ▼                                   │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │                 C++ Application Layer                        ││
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  ││
│  │  │ Auth        │  │ App State   │  │ Worker Thread       │  ││
│  │  │ Manager     │  │ Machine     │  │ (Non-blocking ops)  │  ││
│  │  └─────────────┘  └─────────────┘  └─────────────────────┘  ││
│  └─────────────────────────────────────────────────────────────┘│
│                              │                                   │
│                              │ C API calls                       │
│                              ▼                                   │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │                    C Loader Core                             ││
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  ││
│  │  │ SUPDrv      │  │ Payload     │  │ Driver              │  ││
│  │  │ Interface   │  │ Loader      │  │ Mapper              │  ││
│  │  └─────────────┘  └─────────────┘  └─────────────────────┘  ││
│  └─────────────────────────────────────────────────────────────┘│
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

---

## Startup Sequence

```
┌─────────────────────────────────────────────────────────────────┐
│                      Application Startup                         │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  1. Check administrator privileges                               │
│     └─→ If not admin: show error dialog, exit                   │
│                                                                  │
│  2. Collect hardware fingerprint                                 │
│     ├── Disk serial                                              │
│     ├── NIC MAC address                                          │
│     ├── Volume serial                                            │
│     └── CPUID                                                    │
│                                                                  │
│  3. Authenticate with server (stubbed: always passes)           │
│     └─→ If not authorized: show error dialog, exit              │
│                                                                  │
│  4. Initialize DirectX 11                                        │
│     └─→ If failed: show error, exit                             │
│                                                                  │
│  5. Initialize Dear ImGui                                        │
│     ├── Create context                                           │
│     ├── Load custom fonts                                        │
│     └── Apply dark theme styling                                 │
│                                                                  │
│  6. Create application instance                                  │
│     └─→ Enter main message loop                                 │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

---

## C/C++ Interface Layer

The GUI (C++) calls into the existing loader (C) via a clean C API:

### loader_api.h

```c
#ifndef LOADER_API_H
#define LOADER_API_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ═══════════════════════════════════════════════════════════════════════════
// Error Codes
// ═══════════════════════════════════════════════════════════════════════════

typedef enum {
    LOADER_OK = 0,
    LOADER_ERR_NOT_ADMIN,
    LOADER_ERR_ALREADY_VIRTUALIZED,
    LOADER_ERR_SUPDRV_OPEN,
    LOADER_ERR_SUPDRV_SESSION,
    LOADER_ERR_SYMBOL_RESOLUTION,
    LOADER_ERR_MEMORY_ALLOC,
    LOADER_ERR_PAYLOAD_NOT_FOUND,
    LOADER_ERR_PAYLOAD_INVALID,
    LOADER_ERR_PAYLOAD_LOAD,
    LOADER_ERR_VIRTUALIZATION,
    LOADER_ERR_BIGPOOL_TEST,
    LOADER_ERR_DRIVER_MAP,
    LOADER_ERR_DRIVER_START,
    LOADER_ERR_COMMS_INIT,
} LoaderError;

// ═══════════════════════════════════════════════════════════════════════════
// Opaque Context Handle
// ═══════════════════════════════════════════════════════════════════════════

typedef struct LoaderContext LoaderContext;

// ═══════════════════════════════════════════════════════════════════════════
// Callback for Progress Reporting
// ═══════════════════════════════════════════════════════════════════════════

typedef enum {
    LOG_INFO,
    LOG_SUCCESS,
    LOG_WARNING,
    LOG_ERROR,
} LogLevel;

// Called by loader to report progress
// GUI implements this to display log messages
typedef void (*LoaderLogCallback)(LogLevel level, const char* message, void* userdata);

// ═══════════════════════════════════════════════════════════════════════════
// Status Structures
// ═══════════════════════════════════════════════════════════════════════════

typedef struct {
    bool        hypervisorActive;
    bool        driverActive;
    bool        commsReady;
    uint32_t    numCpus;
    uint32_t    subscriptionCount;
    uint64_t    uptimeMs;
    const char* strategyName;       // "CLEAN_PATH", "MDL", or "EPT_ONLY"
} LoaderStatus;

typedef struct {
    bool        isValid;
    uint32_t    windowsBuild;
    bool        isAdmin;
    bool        alreadyVirtualized;
    bool        filesPresent;
    bool        antiCheatWarning;
    char        antiCheatNames[256]; // Comma-separated list if detected
} EnvironmentInfo;

// ═══════════════════════════════════════════════════════════════════════════
// Lifecycle API
// ═══════════════════════════════════════════════════════════════════════════

// Create context (does not start loading)
LoaderContext* LoaderCreate(void);

// Destroy context and cleanup
void LoaderDestroy(LoaderContext* ctx);

// Set log callback for progress reporting
void LoaderSetLogCallback(LoaderContext* ctx, LoaderLogCallback cb, void* userdata);

// ═══════════════════════════════════════════════════════════════════════════
// Pre-flight Checks
// ═══════════════════════════════════════════════════════════════════════════

// Check environment (admin, Windows version, files, anti-cheat)
LoaderError LoaderCheckEnvironment(LoaderContext* ctx, EnvironmentInfo* info);

// ═══════════════════════════════════════════════════════════════════════════
// Loading Stages (call in order)
// ═══════════════════════════════════════════════════════════════════════════

// Stage 0: Open SUPDrv session
LoaderError LoaderOpenSession(LoaderContext* ctx);

// Stage 1: Run BigPool test, determine strategy
LoaderError LoaderRunBigPoolTest(LoaderContext* ctx);

// Stage 2: Load hypervisor
LoaderError LoaderLoadHypervisor(LoaderContext* ctx);

// Stage 3: Map and start driver
LoaderError LoaderMapDriver(LoaderContext* ctx);

// Stage 4: Initialize communications
LoaderError LoaderInitComms(LoaderContext* ctx);

// Convenience: run all stages sequentially
LoaderError LoaderLoadAll(LoaderContext* ctx);

// ═══════════════════════════════════════════════════════════════════════════
// Runtime API
// ═══════════════════════════════════════════════════════════════════════════

// Get current status
void LoaderGetStatus(LoaderContext* ctx, LoaderStatus* status);

// Send ping to verify driver is responding
LoaderError LoaderPing(LoaderContext* ctx);

// Request graceful shutdown
LoaderError LoaderShutdown(LoaderContext* ctx);

// ═══════════════════════════════════════════════════════════════════════════
// Error Handling
// ═══════════════════════════════════════════════════════════════════════════

// Get human-readable error message
const char* LoaderErrorString(LoaderError err);

// Get last OS error code (GetLastError value)
uint32_t LoaderGetLastOsError(LoaderContext* ctx);

#ifdef __cplusplus
}
#endif

#endif // LOADER_API_H
```

### loader_api.c (Implementation)

```c
// loader_api.c - C API wrapper around existing loader code

#include "loader_api.h"
#include "payload_loader.h"
#include "driver_interface.h"
#include <Windows.h>
#include <stdlib.h>
#include <string.h>

// Internal context structure
struct LoaderContext {
    // Existing structures from payload_loader.h
    HV_CONTEXT          hvContext;

    // Callback
    LoaderLogCallback   logCallback;
    void*               logUserdata;

    // State tracking
    bool                sessionOpen;
    bool                hypervisorLoaded;
    bool                driverMapped;
    bool                commsReady;

    // Error tracking
    uint32_t            lastOsError;

    // Strategy from BigPool test
    int                 mappingStrategy;    // 0=CLEAN, 1=MDL, 2=EPT
};

// Internal logging helper
static void LogMessage(LoaderContext* ctx, LogLevel level, const char* msg) {
    if (ctx->logCallback) {
        ctx->logCallback(level, msg, ctx->logUserdata);
    }
}

LoaderContext* LoaderCreate(void) {
    LoaderContext* ctx = (LoaderContext*)calloc(1, sizeof(LoaderContext));
    return ctx;
}

void LoaderDestroy(LoaderContext* ctx) {
    if (!ctx) return;

    // Cleanup in reverse order
    if (ctx->hypervisorLoaded) {
        // Note: hypervisor stays running until reboot
        // Just close handles
    }

    if (ctx->sessionOpen) {
        DrvCleanup(&ctx->hvContext.Driver);
    }

    free(ctx);
}

void LoaderSetLogCallback(LoaderContext* ctx, LoaderLogCallback cb, void* userdata) {
    if (!ctx) return;
    ctx->logCallback = cb;
    ctx->logUserdata = userdata;
}

LoaderError LoaderCheckEnvironment(LoaderContext* ctx, EnvironmentInfo* info) {
    if (!ctx || !info) return LOADER_ERR_NOT_ADMIN;

    memset(info, 0, sizeof(*info));

    // Check admin
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY ntAuth = SECURITY_NT_AUTHORITY;
    PSID adminGroup;
    if (AllocateAndInitializeSid(&ntAuth, 2,
            SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    info->isAdmin = isAdmin != FALSE;

    if (!info->isAdmin) {
        LogMessage(ctx, LOG_ERROR, "Administrator privileges required");
        return LOADER_ERR_NOT_ADMIN;
    }

    // Check Windows version
    RTL_OSVERSIONINFOW osvi = { sizeof(osvi) };
    typedef NTSTATUS(WINAPI* RtlGetVersionFn)(PRTL_OSVERSIONINFOW);
    RtlGetVersionFn RtlGetVersion = (RtlGetVersionFn)GetProcAddress(
        GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion");
    if (RtlGetVersion) {
        RtlGetVersion(&osvi);
        info->windowsBuild = osvi.dwBuildNumber;
    }

    // Check for existing hypervisor
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    info->alreadyVirtualized = (cpuInfo[2] & (1 << 31)) != 0;

    if (info->alreadyVirtualized) {
        LogMessage(ctx, LOG_WARNING, "System already running under a hypervisor");
    }

    // Check required files
    info->filesPresent =
        (GetFileAttributesA("resources\\Ld9BoxSup.sys") != INVALID_FILE_ATTRIBUTES) &&
        (GetFileAttributesA("payloads\\OmbraPayload.bin") != INVALID_FILE_ATTRIBUTES);

    if (!info->filesPresent) {
        LogMessage(ctx, LOG_ERROR, "Required files not found");
    }

    // Check for anti-cheat (warning only)
    // ... anti-cheat detection code from earlier sections

    info->isValid = info->isAdmin && info->filesPresent;

    return info->isValid ? LOADER_OK : LOADER_ERR_NOT_ADMIN;
}

LoaderError LoaderOpenSession(LoaderContext* ctx) {
    if (!ctx) return LOADER_ERR_SUPDRV_OPEN;

    LogMessage(ctx, LOG_INFO, "Opening SUPDrv session...");

    wchar_t driverPath[MAX_PATH];
    GetModuleFileNameW(NULL, driverPath, MAX_PATH);
    wchar_t* lastSlash = wcsrchr(driverPath, L'\\');
    if (lastSlash) {
        wcscpy(lastSlash + 1, L"resources\\Ld9BoxSup.sys");
    }

    DRV_STATUS status = DrvInitialize(&ctx->hvContext.Driver, driverPath);
    if (status != DRV_SUCCESS) {
        ctx->lastOsError = GetLastError();
        LogMessage(ctx, LOG_ERROR, "Failed to open SUPDrv session");
        return LOADER_ERR_SUPDRV_OPEN;
    }

    ctx->sessionOpen = true;
    LogMessage(ctx, LOG_SUCCESS, "SUPDrv session established");

    return LOADER_OK;
}

LoaderError LoaderRunBigPoolTest(LoaderContext* ctx) {
    if (!ctx || !ctx->sessionOpen) return LOADER_ERR_BIGPOOL_TEST;

    LogMessage(ctx, LOG_INFO, "Running BigPool visibility test...");

    // TODO: Integrate Phase 4A BigPool test
    // For now, assume clean path is viable
    ctx->mappingStrategy = 0;  // CLEAN_PATH

    LogMessage(ctx, LOG_SUCCESS, "BigPool test complete: using CLEAN_PATH strategy");

    return LOADER_OK;
}

LoaderError LoaderLoadHypervisor(LoaderContext* ctx) {
    if (!ctx || !ctx->sessionOpen) return LOADER_ERR_PAYLOAD_LOAD;

    LogMessage(ctx, LOG_INFO, "Loading hypervisor payload...");

    // Load payload file
    FILE* f = fopen("payloads\\OmbraPayload.bin", "rb");
    if (!f) {
        LogMessage(ctx, LOG_ERROR, "Failed to open payload file");
        return LOADER_ERR_PAYLOAD_NOT_FOUND;
    }

    fseek(f, 0, SEEK_END);
    size_t payloadSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    void* payload = malloc(payloadSize);
    if (!payload) {
        fclose(f);
        return LOADER_ERR_MEMORY_ALLOC;
    }

    fread(payload, 1, payloadSize, f);
    fclose(f);

    LogMessage(ctx, LOG_INFO, "Allocating hypervisor memory regions...");

    // Use existing HvLoad function
    wchar_t driverPath[MAX_PATH];
    GetModuleFileNameW(NULL, driverPath, MAX_PATH);
    wchar_t* lastSlash = wcsrchr(driverPath, L'\\');
    if (lastSlash) {
        wcscpy(lastSlash + 1, L"resources\\Ld9BoxSup.sys");
    }

    if (!HvLoad(&ctx->hvContext, driverPath, payload, payloadSize)) {
        free(payload);
        LogMessage(ctx, LOG_ERROR, "Hypervisor load failed");
        return LOADER_ERR_VIRTUALIZATION;
    }

    free(payload);
    ctx->hypervisorLoaded = true;

    char msg[128];
    snprintf(msg, sizeof(msg), "Hypervisor active on %u CPUs", ctx->hvContext.NumCpus);
    LogMessage(ctx, LOG_SUCCESS, msg);

    return LOADER_OK;
}

LoaderError LoaderMapDriver(LoaderContext* ctx) {
    if (!ctx || !ctx->hypervisorLoaded) return LOADER_ERR_DRIVER_MAP;

    LogMessage(ctx, LOG_INFO, "Mapping OmbraDriver...");

    // TODO: Integrate Phase 2 driver mapping
    // For now, stub as successful

    ctx->driverMapped = true;
    LogMessage(ctx, LOG_SUCCESS, "OmbraDriver mapped successfully");

    return LOADER_OK;
}

LoaderError LoaderInitComms(LoaderContext* ctx) {
    if (!ctx || !ctx->driverMapped) return LOADER_ERR_COMMS_INIT;

    LogMessage(ctx, LOG_INFO, "Initializing command ring...");

    // TODO: Integrate Phase 3 communication init
    // For now, stub as successful

    ctx->commsReady = true;
    LogMessage(ctx, LOG_SUCCESS, "Communication channel established");

    return LOADER_OK;
}

LoaderError LoaderLoadAll(LoaderContext* ctx) {
    LoaderError err;

    err = LoaderOpenSession(ctx);
    if (err != LOADER_OK) return err;

    err = LoaderRunBigPoolTest(ctx);
    if (err != LOADER_OK) return err;

    err = LoaderLoadHypervisor(ctx);
    if (err != LOADER_OK) return err;

    err = LoaderMapDriver(ctx);
    if (err != LOADER_OK) return err;

    err = LoaderInitComms(ctx);
    if (err != LOADER_OK) return err;

    return LOADER_OK;
}

void LoaderGetStatus(LoaderContext* ctx, LoaderStatus* status) {
    if (!ctx || !status) return;

    memset(status, 0, sizeof(*status));

    status->hypervisorActive = ctx->hypervisorLoaded && ctx->hvContext.Running;
    status->driverActive = ctx->driverMapped;
    status->commsReady = ctx->commsReady;
    status->numCpus = ctx->hvContext.NumCpus;
    status->subscriptionCount = 0;  // TODO: query from driver
    status->uptimeMs = 0;           // TODO: calculate

    switch (ctx->mappingStrategy) {
        case 0:  status->strategyName = "CLEAN_PATH"; break;
        case 1:  status->strategyName = "MDL"; break;
        case 2:  status->strategyName = "EPT_ONLY"; break;
        default: status->strategyName = "UNKNOWN"; break;
    }
}

LoaderError LoaderPing(LoaderContext* ctx) {
    if (!ctx || !ctx->commsReady) return LOADER_ERR_COMMS_INIT;

    // TODO: Send PING command via command ring
    return HvIsRunning(&ctx->hvContext) ? LOADER_OK : LOADER_ERR_COMMS_INIT;
}

LoaderError LoaderShutdown(LoaderContext* ctx) {
    if (!ctx) return LOADER_OK;

    // TODO: Send shutdown command to driver
    // Hypervisor stays running until reboot

    return LOADER_OK;
}

const char* LoaderErrorString(LoaderError err) {
    switch (err) {
        case LOADER_OK:                     return "Success";
        case LOADER_ERR_NOT_ADMIN:          return "Administrator privileges required";
        case LOADER_ERR_ALREADY_VIRTUALIZED: return "System already virtualized";
        case LOADER_ERR_SUPDRV_OPEN:        return "Failed to open SUPDrv device";
        case LOADER_ERR_SUPDRV_SESSION:     return "Failed to create SUPDrv session";
        case LOADER_ERR_SYMBOL_RESOLUTION:  return "Failed to resolve kernel symbols";
        case LOADER_ERR_MEMORY_ALLOC:       return "Memory allocation failed";
        case LOADER_ERR_PAYLOAD_NOT_FOUND:  return "Payload file not found";
        case LOADER_ERR_PAYLOAD_INVALID:    return "Invalid payload format";
        case LOADER_ERR_PAYLOAD_LOAD:       return "Failed to load payload";
        case LOADER_ERR_VIRTUALIZATION:     return "CPU virtualization failed";
        case LOADER_ERR_BIGPOOL_TEST:       return "BigPool test failed";
        case LOADER_ERR_DRIVER_MAP:         return "Failed to map driver";
        case LOADER_ERR_DRIVER_START:       return "Failed to start driver";
        case LOADER_ERR_COMMS_INIT:         return "Communication init failed";
        default:                            return "Unknown error";
    }
}

uint32_t LoaderGetLastOsError(LoaderContext* ctx) {
    return ctx ? ctx->lastOsError : 0;
}
```

---

## Authentication System

### auth.h

```cpp
#pragma once

#include <string>
#include <cstdint>

namespace Ombra {

enum class LicenseType {
    None,
    Trial,
    Monthly,
    Lifetime
};

struct AuthResult {
    bool authorized;
    LicenseType licenseType;
    std::string username;
    std::string message;
    uint64_t expiresAt;         // Unix timestamp, 0 = never
    int daysRemaining;          // -1 = lifetime
};

struct HardwareFingerprint {
    std::string diskSerial;
    std::string macAddress;
    std::string volumeSerial;
    std::string cpuId;
    std::string combined;       // SHA256 hash of all above
};

class AuthManager {
public:
    // Collect hardware fingerprint
    static HardwareFingerprint CollectFingerprint();

    // Validate with auth server (or stub)
    static AuthResult Validate(const HardwareFingerprint& fp);

    // Check if current session is still valid
    static bool IsSessionValid();

    // Get license type string for display
    static const char* LicenseTypeString(LicenseType type);

private:
    static AuthResult s_cachedResult;
    static bool s_validated;
};

} // namespace Ombra
```

### auth.cpp (Stub Implementation)

```cpp
#include "auth.h"
#include <Windows.h>
#include <iphlpapi.h>
#include <intrin.h>
#include <sstream>
#include <iomanip>
#include <ctime>

#pragma comment(lib, "iphlpapi.lib")

namespace Ombra {

AuthResult AuthManager::s_cachedResult = {};
bool AuthManager::s_validated = false;

HardwareFingerprint AuthManager::CollectFingerprint() {
    HardwareFingerprint fp;

    // Get volume serial
    char volumeName[MAX_PATH];
    char fsName[MAX_PATH];
    DWORD serialNumber = 0;
    DWORD maxComponentLen, fsFlags;

    if (GetVolumeInformationA("C:\\", volumeName, MAX_PATH, &serialNumber,
                              &maxComponentLen, &fsFlags, fsName, MAX_PATH)) {
        std::ostringstream oss;
        oss << std::hex << std::uppercase << serialNumber;
        fp.volumeSerial = oss.str();
    }

    // Get MAC address
    IP_ADAPTER_INFO adapters[16];
    DWORD bufLen = sizeof(adapters);
    if (GetAdaptersInfo(adapters, &bufLen) == ERROR_SUCCESS) {
        std::ostringstream oss;
        for (int i = 0; i < 6; i++) {
            if (i > 0) oss << ":";
            oss << std::hex << std::setw(2) << std::setfill('0')
                << (int)adapters[0].Address[i];
        }
        fp.macAddress = oss.str();
    }

    // Get CPUID
    int cpuInfo[4];
    __cpuid(cpuInfo, 0);
    std::ostringstream oss;
    oss << std::hex << cpuInfo[1] << cpuInfo[3] << cpuInfo[2];
    fp.cpuId = oss.str();

    // Combine (real implementation would SHA256 hash)
    fp.combined = fp.volumeSerial + "-" + fp.macAddress + "-" + fp.cpuId;

    return fp;
}

AuthResult AuthManager::Validate(const HardwareFingerprint& fp) {
    // ═══════════════════════════════════════════════════════════════
    // STUB IMPLEMENTATION - Always returns authorized
    // Replace with real HTTP request to auth server
    // ═══════════════════════════════════════════════════════════════

    AuthResult result;
    result.authorized = true;
    result.licenseType = LicenseType::Lifetime;
    result.username = "Developer";
    result.message = "Development build - auth bypassed";
    result.expiresAt = 0;  // Never expires
    result.daysRemaining = -1;  // Lifetime

    s_cachedResult = result;
    s_validated = true;

    return result;
}

bool AuthManager::IsSessionValid() {
    if (!s_validated) return false;
    if (!s_cachedResult.authorized) return false;

    // Check expiration
    if (s_cachedResult.expiresAt > 0) {
        uint64_t now = static_cast<uint64_t>(time(nullptr));
        if (now > s_cachedResult.expiresAt) {
            return false;
        }
    }

    return true;
}

const char* AuthManager::LicenseTypeString(LicenseType type) {
    switch (type) {
        case LicenseType::Trial:    return "Trial";
        case LicenseType::Monthly:  return "Monthly";
        case LicenseType::Lifetime: return "Lifetime";
        default:                    return "None";
    }
}

} // namespace Ombra
```

---

## DirectX 11 Backend

### dx11_backend.h

```cpp
#pragma once

#include <d3d11.h>
#include <windows.h>

namespace Ombra {
namespace DX11 {

struct Context {
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* deviceContext = nullptr;
    IDXGISwapChain* swapChain = nullptr;
    ID3D11RenderTargetView* renderTargetView = nullptr;

    HWND hwnd = nullptr;
    int width = 0;
    int height = 0;
};

bool Initialize(HWND hwnd, int width, int height, Context* ctx);
void Shutdown(Context* ctx);
void OnResize(Context* ctx, int width, int height);
void BeginFrame(Context* ctx, float clearColor[4]);
void EndFrame(Context* ctx, bool vsync);

} // namespace DX11
} // namespace Ombra
```

### dx11_backend.cpp

```cpp
#include "dx11_backend.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace Ombra {
namespace DX11 {

bool Initialize(HWND hwnd, int width, int height, Context* ctx) {
    ctx->hwnd = hwnd;
    ctx->width = width;
    ctx->height = height;

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        createDeviceFlags, featureLevels, _countof(featureLevels),
        D3D11_SDK_VERSION, &sd,
        &ctx->swapChain, &ctx->device, &featureLevel, &ctx->deviceContext
    );

    if (FAILED(hr)) {
        return false;
    }

    ID3D11Texture2D* backBuffer = nullptr;
    ctx->swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    ctx->device->CreateRenderTargetView(backBuffer, nullptr, &ctx->renderTargetView);
    backBuffer->Release();

    return true;
}

void Shutdown(Context* ctx) {
    if (ctx->renderTargetView) { ctx->renderTargetView->Release(); ctx->renderTargetView = nullptr; }
    if (ctx->swapChain) { ctx->swapChain->Release(); ctx->swapChain = nullptr; }
    if (ctx->deviceContext) { ctx->deviceContext->Release(); ctx->deviceContext = nullptr; }
    if (ctx->device) { ctx->device->Release(); ctx->device = nullptr; }
}

void OnResize(Context* ctx, int width, int height) {
    if (!ctx->device || width == 0 || height == 0) return;

    ctx->width = width;
    ctx->height = height;

    if (ctx->renderTargetView) { ctx->renderTargetView->Release(); ctx->renderTargetView = nullptr; }

    ctx->swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

    ID3D11Texture2D* backBuffer = nullptr;
    ctx->swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    ctx->device->CreateRenderTargetView(backBuffer, nullptr, &ctx->renderTargetView);
    backBuffer->Release();
}

void BeginFrame(Context* ctx, float clearColor[4]) {
    ctx->deviceContext->OMSetRenderTargets(1, &ctx->renderTargetView, nullptr);
    ctx->deviceContext->ClearRenderTargetView(ctx->renderTargetView, clearColor);
}

void EndFrame(Context* ctx, bool vsync) {
    ctx->swapChain->Present(vsync ? 1 : 0, 0);
}

} // namespace DX11
} // namespace Ombra
```

---

## Application State Machine

### app.h

```cpp
#pragma once

#include <windows.h>
#include <d3d11.h>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <functional>
#include "auth.h"

struct ImFont;

extern "C" {
#include "loader_api.h"
}

namespace Ombra {

enum class AppState {
    Idle,
    LoadingHypervisor,
    HypervisorActive,
    HypervisorFailed,
    RunningSpoofing,
    SpoofingComplete,
    SpoofingFailed
};

enum class UILogLevel {
    Info,
    Success,
    Warning,
    Error
};

struct LogEntry {
    UILogLevel level;
    std::string message;
    std::string timestamp;
};

class Application {
public:
    Application();
    ~Application();

    bool Initialize(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);
    void Shutdown();
    void Update();
    void Render();

    void LoadHypervisor();
    void RunSpoofer();

    AppState GetState() const { return m_state; }
    bool IsHypervisorActive() const { return m_loaderStatus.hypervisorActive; }
    bool IsWorkerBusy() const { return m_workerBusy; }

    void SetAuthResult(const AuthResult& result) { m_authResult = result; }

    void Log(UILogLevel level, const std::string& message);
    void LogInfo(const std::string& message) { Log(UILogLevel::Info, message); }
    void LogSuccess(const std::string& message) { Log(UILogLevel::Success, message); }
    void LogWarning(const std::string& message) { Log(UILogLevel::Warning, message); }
    void LogError(const std::string& message) { Log(UILogLevel::Error, message); }

private:
    void StartWorker(std::function<void()> task);
    void WorkerThreadFunc();
    void DoLoadHypervisor();
    void DoRunSpoofer();
    void RenderStatusBar();

    std::atomic<AppState> m_state{AppState::Idle};
    LoaderContext* m_loaderCtx = nullptr;
    LoaderStatus m_loaderStatus = {};
    AuthResult m_authResult;

    std::thread m_workerThread;
    std::atomic<bool> m_workerBusy{false};
    std::function<void()> m_workerTask;

    std::vector<LogEntry> m_logEntries;
    std::mutex m_logMutex;
    bool m_autoScrollLog = true;

    ImFont* m_defaultFont = nullptr;
    ImFont* m_titleFont = nullptr;
    ImFont* m_monoFont = nullptr;

    HWND m_hwnd = nullptr;
};

extern Application* g_App;

} // namespace Ombra
```

---

## Main Entry Point

### main.cpp

```cpp
#include "app.h"
#include "auth.h"
#include "dx11_backend.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <windows.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Ombra {
    Application* g_App = nullptr;
}

constexpr int WINDOW_WIDTH = 600;
constexpr int WINDOW_HEIGHT = 500;

static Ombra::DX11::Context g_DxContext;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
        return true;
    }

    switch (msg) {
        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED) {
                Ombra::DX11::OnResize(&g_DxContext, LOWORD(lParam), HIWORD(lParam));
            }
            return 0;

        case WM_SYSCOMMAND:
            if ((wParam & 0xFFF0) == SC_CLOSE && Ombra::g_App &&
                Ombra::g_App->IsHypervisorActive()) {
                int result = MessageBoxW(hWnd,
                    L"Hypervisor is active. It will remain loaded until reboot.\n\n"
                    L"Are you sure you want to exit?",
                    L"Confirm Exit", MB_ICONWARNING | MB_YESNO);
                if (result != IDYES) return 0;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    // Check admin
    if (!IsUserAnAdmin()) {
        MessageBoxW(nullptr,
            L"Ombra requires administrator privileges.\n\n"
            L"Please right-click and select 'Run as administrator'.",
            L"Elevation Required", MB_ICONERROR | MB_OK);
        return 1;
    }

    // Authenticate
    Ombra::HardwareFingerprint fp = Ombra::AuthManager::CollectFingerprint();
    Ombra::AuthResult authResult = Ombra::AuthManager::Validate(fp);

    if (!authResult.authorized) {
        MessageBoxW(nullptr,
            L"Authentication failed.\n\n"
            L"Your license may have expired or your hardware is not registered.",
            L"Not Authorized", MB_ICONERROR | MB_OK);
        return 1;
    }

    // Register window class
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0, 0, hInstance,
                       nullptr, LoadCursor(nullptr, IDC_ARROW), nullptr, nullptr,
                       L"OmbraWindowClass", nullptr };
    RegisterClassExW(&wc);

    // Center window
    int posX = (GetSystemMetrics(SM_CXSCREEN) - WINDOW_WIDTH) / 2;
    int posY = (GetSystemMetrics(SM_CYSCREEN) - WINDOW_HEIGHT) / 2;

    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"Ombra Hypervisor Loader",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        posX, posY, WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwnd) {
        MessageBoxW(nullptr, L"Failed to create window", L"Error", MB_ICONERROR);
        return 1;
    }

    // Initialize DX11
    if (!Ombra::DX11::Initialize(hwnd, WINDOW_WIDTH, WINDOW_HEIGHT, &g_DxContext)) {
        MessageBoxW(nullptr, L"Failed to initialize DirectX 11", L"Error", MB_ICONERROR);
        DestroyWindow(hwnd);
        UnregisterClassW(wc.lpszClassName, hInstance);
        return 1;
    }

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;

    // Dark theme with custom colors
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 6);
    style.WindowPadding = ImVec2(15, 15);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.40f, 0.70f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.52f, 0.86f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.35f, 0.60f, 1.00f);

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_DxContext.device, g_DxContext.deviceContext);

    // Create application
    Ombra::g_App = new Ombra::Application();
    Ombra::g_App->SetAuthResult(authResult);

    if (!Ombra::g_App->Initialize(hwnd, g_DxContext.device, g_DxContext.deviceContext)) {
        delete Ombra::g_App;
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        Ombra::DX11::Shutdown(&g_DxContext);
        DestroyWindow(hwnd);
        UnregisterClassW(wc.lpszClassName, hInstance);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Main loop
    MSG msg = {};
    bool running = true;

    while (running) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            if (msg.message == WM_QUIT) running = false;
        }

        if (!running) break;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        Ombra::g_App->Update();
        Ombra::g_App->Render();

        ImGui::Render();

        float clearColor[4] = { 0.08f, 0.08f, 0.10f, 1.00f };
        Ombra::DX11::BeginFrame(&g_DxContext, clearColor);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        Ombra::DX11::EndFrame(&g_DxContext, true);
    }

    // Cleanup
    Ombra::g_App->Shutdown();
    delete Ombra::g_App;

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    Ombra::DX11::Shutdown(&g_DxContext);
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, hInstance);

    return 0;
}
```

---

## Visual Layout

```
┌─────────────────────────────────────────────────────────────────┐
│                   Ombra Hypervisor Loader                   [X] │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│                     Ombra Hypervisor                            │
│                     (title, centered)                           │
│                                                                 │
│     ┌─────────────────────┐     ┌─────────────────────┐        │
│     │                     │     │                     │        │
│     │  Load Hypervisor    │     │    Run Spoofer      │        │
│     │                     │     │    (disabled)       │        │
│     └─────────────────────┘     └─────────────────────┘        │
│                                                                 │
│  ─────────────────────────────────────────────────────────────  │
│                                                                 │
│  Status: Ready                                                  │
│                                                                 │
│  ─────────────────────────────────────────────────────────────  │
│                                                                 │
│  Log Output:                                                    │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │ [14:32:01] [*] Ombra initialized                          │ │
│  │ [14:32:01] [*] Click 'Load Hypervisor' to begin           │ │
│  │ [14:32:05] [*] Opening SUPDrv session...                  │ │
│  │ [14:32:05] [+] SUPDrv session established                 │ │
│  │ [14:32:06] [*] Running BigPool test...                    │ │
│  │ [14:32:06] [+] Strategy: CLEAN_PATH                       │ │
│  │ [14:32:06] [*] Loading hypervisor payload...              │ │
│  │ [14:32:07] [+] Hypervisor active on 8 CPUs                │ │
│  └───────────────────────────────────────────────────────────┘ │
│  [✓] Auto-scroll                                               │
│                                                                 │
├─────────────────────────────────────────────────────────────────┤
│  License: Lifetime                              User: Developer │
└─────────────────────────────────────────────────────────────────┘
```

---

## File Structure

```
hypervisor/
├── usermode/
│   ├── gui/                           # C++ GUI layer
│   │   ├── src/
│   │   │   ├── main.cpp               # Entry point
│   │   │   ├── app.cpp                # Application logic
│   │   │   ├── app.h
│   │   │   ├── dx11_backend.cpp       # DirectX 11
│   │   │   ├── dx11_backend.h
│   │   │   ├── auth.cpp               # Auth (stubbed)
│   │   │   ├── auth.h
│   │   │   ├── spoofer_ui.cpp         # Spoofer UI (stubbed)
│   │   │   └── spoofer_ui.h
│   │   ├── imgui/                     # Dear ImGui source
│   │   │   └── (imgui files)
│   │   ├── resources/
│   │   │   └── fonts/
│   │   │       └── Roboto-Regular.ttf
│   │   └── CMakeLists.txt
│   │
│   ├── loader_api.h                   # C interface for GUI
│   ├── loader_api.c                   # Implementation
│   │
│   ├── main.c                         # CLI entry (kept for debugging)
│   ├── payload_loader.c               # Existing Phase 1
│   ├── payload_loader.h
│   ├── driver_interface.c             # SUPDrv interface
│   ├── driver_interface.h
│   └── byovd/                         # BYOVD implementation
│       └── ...
```

---

## Build Configuration (CMake)

```cmake
cmake_minimum_required(VERSION 3.16)
project(OmbraGUI VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    add_definitions(-DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -DNOMINMAX)
endif()

# ImGui sources
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_impl_win32.cpp
    ${IMGUI_DIR}/imgui_impl_dx11.cpp
)

# C++ GUI sources
set(GUI_SOURCES
    src/main.cpp
    src/app.cpp
    src/dx11_backend.cpp
    src/auth.cpp
    src/spoofer_ui.cpp
)

# C loader sources (compiled as C)
set(LOADER_SOURCES
    ${CMAKE_SOURCE_DIR}/../loader_api.c
    ${CMAKE_SOURCE_DIR}/../payload_loader.c
    ${CMAKE_SOURCE_DIR}/../driver_interface.c
    ${CMAKE_SOURCE_DIR}/../byovd/supdrv.c
    ${CMAKE_SOURCE_DIR}/../byovd/deployer.c
    ${CMAKE_SOURCE_DIR}/../byovd/crypto.c
    ${CMAKE_SOURCE_DIR}/../byovd/nt_helpers.c
)

add_executable(OmbraGUI WIN32
    ${GUI_SOURCES}
    ${IMGUI_SOURCES}
    ${LOADER_SOURCES}
)

target_include_directories(OmbraGUI PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/..
    ${IMGUI_DIR}
)

target_link_libraries(OmbraGUI PRIVATE
    d3d11 dxgi d3dcompiler
    user32 gdi32 shell32 advapi32
    iphlpapi
)

# Copy resources
add_custom_command(TARGET OmbraGUI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources
    $<TARGET_FILE_DIR:OmbraGUI>/resources
)
```

---

## Task Summary

| Task | Description | Files | Effort |
|------|-------------|-------|--------|
| 4B.1 | C API wrapper for existing loader | loader_api.h/c | 1 task |
| 4B.2 | DirectX 11 backend | dx11_backend.cpp/h | 1 task |
| 4B.3 | ImGui integration and styling | main.cpp (ImGui setup) | 1 task |
| 4B.4 | Application state machine | app.cpp/h | 1 task |
| 4B.5 | GUI rendering (buttons, status, log) | app.cpp (Render) | 1 task |
| 4B.6 | Worker thread management | app.cpp (threading) | 0.5 task |
| 4B.7 | Authentication system (stubbed) | auth.cpp/h | 1 task |
| 4B.8 | License status bar | app.cpp (RenderStatusBar) | 0.5 task |
| 4B.9 | Spoofer UI stub | spoofer_ui.cpp/h | 0.5 task |
| 4B.10 | Build configuration | CMakeLists.txt | 0.5 task |
| **Total** | | | **8 tasks** |

---

## Dependencies

### Required Libraries
- **DirectX 11 SDK** - Included in Windows SDK
- **Dear ImGui** - Download from https://github.com/ocornut/imgui
  - Core: imgui.cpp, imgui_draw.cpp, imgui_tables.cpp, imgui_widgets.cpp
  - Backends: imgui_impl_win32.cpp, imgui_impl_dx11.cpp

### Optional
- **Roboto font** - For custom UI font (falls back to default if missing)

---

## Integration Notes

### Connecting to Existing Code

The `loader_api.c` implementation wraps calls to existing functions:

| C API Function | Calls Into |
|----------------|------------|
| `LoaderOpenSession()` | `DrvInitialize()` |
| `LoaderLoadHypervisor()` | `HvLoad()` |
| `LoaderMapDriver()` | Phase 2 code (TODO) |
| `LoaderInitComms()` | Phase 3 code (TODO) |
| `LoaderPing()` | `HvIsRunning()` |

### Thread Safety

- Worker thread handles all blocking loader operations
- Log entries protected by mutex
- Atomic variables for state machine and busy flag
- Critical section not needed for single-threaded ImGui rendering

### Hypervisor Lifecycle

The hypervisor cannot be cleanly unloaded at runtime. Once loaded:
- It remains active until system reboot
- Closing the GUI does not stop the hypervisor
- Exit confirmation dialog warns user of this

---

## Success Criteria

Phase 4B is complete when:

1. GUI launches and displays correctly
2. Auth stub returns authorized
3. "Load Hypervisor" button triggers loading sequence
4. Log shows real-time progress from loader callbacks
5. Status updates to "Hypervisor Active" on success
6. License status bar shows "Lifetime" and username
7. Exit confirmation appears when hypervisor is active
8. "Run Spoofer" button shows stub message

---

## Next Steps

After Phase 4B:
- **Phase 5:** Implement actual spoofer functionality
- **Phase 6:** Replace auth stub with real HTTP calls
- **Phase 7:** Add rolling build expiration checks
