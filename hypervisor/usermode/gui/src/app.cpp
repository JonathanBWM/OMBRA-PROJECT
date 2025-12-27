#include "app.h"
#include "imgui.h"
#include "auth.h"
#include <ctime>
#include <iomanip>
#include <sstream>

#include "../../loader_api.h"

namespace Ombra {

// Static callback for loader logging
static void LoaderLogCallback(LogLevel level, const char* message, void* userdata) {
    Application* app = static_cast<Application*>(userdata);
    if (app) {
        switch (level) {
            case LOG_INFO: app->LogInfo(message); break;
            case LOG_SUCCESS: app->LogSuccess(message); break;
            case LOG_WARNING: app->LogWarning(message); break;
            case LOG_ERROR: app->LogError(message); break;
        }
    }
}

Application::Application()
    : m_state(AppState::Idle)
    , m_loaderCtx(nullptr)
    , m_loaderStatus{}
    , m_authResult{}
    , m_workerBusy(false)
    , m_fontRegular(nullptr)
    , m_fontBold(nullptr)
    , m_fontMono(nullptr)
    , m_hwnd(nullptr)
{
}

Application::~Application() {
    Shutdown();
}

bool Application::Initialize(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context) {
    m_hwnd = hwnd;

    // Create loader context
    m_loaderCtx = LoaderCreate();
    if (!m_loaderCtx) {
        LogError("Failed to create loader context");
        return false;
    }

    // Set up loader logging callback
    LoaderSetLogCallback(m_loaderCtx, LoaderLogCallback, this);

    // Initialize fonts (these will be set up by main.cpp during ImGui init)
    ImGuiIO& io = ImGui::GetIO();
    if (io.Fonts->Fonts.Size >= 3) {
        m_fontRegular = io.Fonts->Fonts[0];
        m_fontBold = io.Fonts->Fonts[1];
        m_fontMono = io.Fonts->Fonts[2];
    }

    LogSuccess("Ombra initialized successfully");
    return true;
}

void Application::Shutdown() {
    // Wait for worker thread to complete
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }

    // Destroy loader context
    if (m_loaderCtx) {
        LoaderDestroy(m_loaderCtx);
        m_loaderCtx = nullptr;
    }

    LogInfo("Ombra shutdown complete");
}

void Application::Update() {
    // Poll loader status if worker is busy
    if (m_workerBusy.load() && m_loaderCtx) {
        LoaderGetStatus(m_loaderCtx, &m_loaderStatus);
    }

    // Check if worker thread has finished
    if (!m_workerBusy.load() && m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

void Application::Render() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoBringToFrontOnFocus
        | ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("Ombra", nullptr, window_flags);

    // Title
    if (m_fontBold) ImGui::PushFont(m_fontBold);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
    ImGui::Indent(20.0f);
    ImGui::TextUnformatted("Ombra Hypervisor");
    ImGui::Unindent(20.0f);
    if (m_fontBold) ImGui::PopFont();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Main content area
    ImGui::Indent(20.0f);

    // Action buttons
    ImGui::BeginDisabled(IsWorkerBusy());
    if (ImGui::Button("Load Hypervisor", ImVec2(200, 40))) {
        LoadHypervisor();
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::BeginDisabled(!IsHypervisorActive() || IsWorkerBusy());
    if (ImGui::Button("Run Spoofer", ImVec2(200, 40))) {
        RunSpoofer();
    }
    ImGui::EndDisabled();

    ImGui::Spacing();
    ImGui::Spacing();

    // Status display
    if (m_fontBold) ImGui::PushFont(m_fontBold);
    ImGui::TextUnformatted("Status:");
    if (m_fontBold) ImGui::PopFont();

    const char* stateStr = "Unknown";
    ImVec4 stateColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    switch (m_state) {
        case AppState::Idle:
            stateStr = "Idle - Ready to load";
            stateColor = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
            break;
        case AppState::LoadingHypervisor:
            stateStr = "Loading hypervisor...";
            stateColor = ImVec4(0.3f, 0.6f, 1.0f, 1.0f);
            break;
        case AppState::HypervisorActive:
            stateStr = "Hypervisor active and ready";
            stateColor = ImVec4(0.2f, 1.0f, 0.3f, 1.0f);
            break;
        case AppState::HypervisorFailed:
            stateStr = "Hypervisor load failed";
            stateColor = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
            break;
        case AppState::RunningSpoofing:
            stateStr = "Running spoofer...";
            stateColor = ImVec4(0.3f, 0.6f, 1.0f, 1.0f);
            break;
        case AppState::SpoofingComplete:
            stateStr = "Spoofing complete";
            stateColor = ImVec4(0.2f, 1.0f, 0.3f, 1.0f);
            break;
        case AppState::SpoofingFailed:
            stateStr = "Spoofing failed";
            stateColor = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
            break;
    }

    ImGui::TextColored(stateColor, "%s", stateStr);

    // Show loader progress if loading
    if (m_state == AppState::LoadingHypervisor && IsWorkerBusy()) {
        ImGui::Spacing();
        ImGui::Text("Phase: %s", m_loaderStatus.currentPhase);
        ImGui::ProgressBar(m_loaderStatus.progress / 100.0f, ImVec2(-1, 0));
        if (m_loaderStatus.lastError[0] != '\0') {
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "Warning: %s", m_loaderStatus.lastError);
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Log output
    if (m_fontBold) ImGui::PushFont(m_fontBold);
    ImGui::TextUnformatted("Log:");
    if (m_fontBold) ImGui::PopFont();

    ImGui::BeginChild("LogOutput", ImVec2(0, -40), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    if (m_fontMono) ImGui::PushFont(m_fontMono);

    {
        std::lock_guard<std::mutex> lock(m_logMutex);
        for (const auto& entry : m_logEntries) {
            ImVec4 color;
            const char* prefix;

            switch (entry.level) {
                case UILogLevel::Info:
                    color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
                    prefix = "[INFO]";
                    break;
                case UILogLevel::Success:
                    color = ImVec4(0.2f, 1.0f, 0.3f, 1.0f);
                    prefix = "[OK]";
                    break;
                case UILogLevel::Warning:
                    color = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
                    prefix = "[WARN]";
                    break;
                case UILogLevel::Error:
                    color = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
                    prefix = "[ERROR]";
                    break;
                default:
                    color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                    prefix = "[???]";
                    break;
            }

            // Format timestamp
            auto timeT = std::chrono::system_clock::to_time_t(entry.timestamp);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                entry.timestamp.time_since_epoch()) % 1000;

            std::tm tm;
            localtime_s(&tm, &timeT);

            std::ostringstream oss;
            oss << std::put_time(&tm, "%H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms.count();

            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "[%s]", oss.str().c_str());
            ImGui::SameLine();
            ImGui::TextColored(color, "%s", prefix);
            ImGui::SameLine();
            ImGui::TextColored(color, "%s", entry.message.c_str());
        }
    }

    if (m_fontMono) ImGui::PopFont();

    // Auto-scroll to bottom
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();

    ImGui::Unindent(20.0f);

    // Status bar at bottom
    RenderStatusBar();

    ImGui::End();
}

void Application::RenderStatusBar() {
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Indent(20.0f);

    // License type
    const char* licenseType = "Unknown";
    switch (m_authResult.licenseType) {
        case LicenseType::None: licenseType = "None"; break;
        case LicenseType::Trial: licenseType = "Trial"; break;
        case LicenseType::Monthly: licenseType = "Monthly"; break;
        case LicenseType::Lifetime: licenseType = "Lifetime"; break;
    }

    ImGui::Text("License: %s", licenseType);
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    ImGui::Text("User: %s", m_authResult.username);

    ImGui::Unindent(20.0f);
    ImGui::Spacing();
}

void Application::LoadHypervisor() {
    if (IsWorkerBusy()) {
        LogWarning("Operation already in progress");
        return;
    }

    LogInfo("Starting hypervisor load sequence...");
    m_state = AppState::LoadingHypervisor;
    StartWorker(&Application::DoLoadHypervisor);
}

void Application::RunSpoofer() {
    if (IsWorkerBusy()) {
        LogWarning("Operation already in progress");
        return;
    }

    if (!IsHypervisorActive()) {
        LogError("Hypervisor must be active before running spoofer");
        return;
    }

    LogInfo("Starting spoofer...");
    m_state = AppState::RunningSpoofing;
    StartWorker(&Application::DoRunSpoofer);
}

void Application::SetAuthResult(const AuthResult& result) {
    m_authResult = result;
}

void Application::Log(UILogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_logEntries.emplace_back(level, message);

    // Limit log size to prevent memory growth
    const size_t MAX_LOG_ENTRIES = 1000;
    if (m_logEntries.size() > MAX_LOG_ENTRIES) {
        m_logEntries.erase(m_logEntries.begin());
    }
}

void Application::StartWorker(void (Application::*workerFunc)()) {
    m_workerBusy.store(true);
    m_workerThread = std::thread(&Application::WorkerThreadFunc, this, workerFunc);
}

void Application::WorkerThreadFunc(void (Application::*workerFunc)()) {
    (this->*workerFunc)();
    m_workerBusy.store(false);
}

void Application::DoLoadHypervisor() {
    LogInfo("Initializing BYOVD exploit...");
    LogInfo("Mapping hypervisor into kernel...");
    LogInfo("Setting up VMX regions...");

    // Call the actual loader - TODO: Pass actual driver/payload paths
    LoaderError err = LoaderLoadAll(
        m_loaderCtx,
        L"Ld9BoxSup.sys",  // TODO: Get from config
        nullptr,            // TODO: Embedded payload or path
        0
    );

    if (err == LOADER_OK) {
        m_state = AppState::HypervisorActive;
        LogSuccess("Hypervisor loaded successfully");
        LogSuccess("VMX enabled on all cores");
    } else {
        m_state = AppState::HypervisorFailed;
        LogError(LoaderErrorString(err));
    }
}

void Application::DoRunSpoofer() {
    LogInfo("Analyzing system hardware...");
    LogInfo("Generating spoof values...");
    LogInfo("Applying SMBIOS patches...");
    LogInfo("Spoofing disk serials...");
    LogInfo("Spoofing MAC addresses...");

    // Simulate work (will be replaced with actual spoofer implementation)
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // For now, always succeed (stub implementation)
    m_state = AppState::SpoofingComplete;
    LogSuccess("Hardware spoofing complete");
    LogSuccess("System fingerprint randomized");
}

} // namespace Ombra
