#pragma once

#include "auth.h"
#include "../loader_api.h"
#include <Windows.h>
#include <d3d11.h>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>

// Forward declarations
struct ImFont;

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
    std::chrono::system_clock::time_point timestamp;

    LogEntry(UILogLevel lvl, const std::string& msg)
        : level(lvl), message(msg), timestamp(std::chrono::system_clock::now()) {}
};

class Application {
public:
    Application();
    ~Application();

    // Lifecycle
    bool Initialize(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);
    void Shutdown();
    void Update();
    void Render();

    // Actions
    void LoadHypervisor();
    void RunSpoofer();

    // State queries
    AppState GetState() const { return m_state; }
    bool IsHypervisorActive() const { return m_state == AppState::HypervisorActive; }
    bool IsWorkerBusy() const { return m_workerBusy.load(); }

    // Auth result setter (called after authentication completes)
    void SetAuthResult(const AuthResult& result);

    // Logging
    void Log(UILogLevel level, const std::string& message);
    void LogInfo(const std::string& message) { Log(UILogLevel::Info, message); }
    void LogSuccess(const std::string& message) { Log(UILogLevel::Success, message); }
    void LogWarning(const std::string& message) { Log(UILogLevel::Warning, message); }
    void LogError(const std::string& message) { Log(UILogLevel::Error, message); }

private:
    // Worker thread management
    void StartWorker(void (Application::*workerFunc)());
    void WorkerThreadFunc(void (Application::*workerFunc)());

    // Worker operations
    void DoLoadHypervisor();
    void DoRunSpoofer();

    // Rendering
    void RenderStatusBar();

    // State
    AppState m_state;
    LoaderContext* m_loaderCtx;
    LoaderStatus m_loaderStatus;
    AuthResult m_authResult;

    // Worker thread
    std::thread m_workerThread;
    std::atomic<bool> m_workerBusy;

    // Logging
    std::vector<LogEntry> m_logEntries;
    std::mutex m_logMutex;

    // UI resources
    ImFont* m_fontRegular;
    ImFont* m_fontBold;
    ImFont* m_fontMono;

    HWND m_hwnd;
};

} // namespace Ombra

// Global application instance
extern Ombra::Application* g_App;
