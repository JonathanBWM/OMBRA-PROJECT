/**
 * @file main.cpp
 * @brief OmbraHypervisor GUI Entry Point
 */

#include "app.h"
#include "auth.h"
#include "dx11_backend.h"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include <Windows.h>
#include <ShellAPI.h>

//=============================================================================
// Constants
//=============================================================================

constexpr unsigned int WINDOW_WIDTH = 600;
constexpr unsigned int WINDOW_HEIGHT = 500;
constexpr const wchar_t* WINDOW_TITLE = L"Ombra";
constexpr const wchar_t* WINDOW_CLASS = L"OmbraHypervisorClass";

//=============================================================================
// Globals
//=============================================================================

Ombra::DX11::Context g_DxContext;
Ombra::Application* g_App = nullptr;

//=============================================================================
// Forward Declarations
//=============================================================================

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//=============================================================================
// Window Procedure
//=============================================================================

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Let ImGui handle input first
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    switch (message)
    {
    case WM_SIZE:
        if (g_DxContext.device != nullptr && wParam != SIZE_MINIMIZED)
        {
            unsigned int width = LOWORD(lParam);
            unsigned int height = HIWORD(lParam);
            Ombra::DX11::OnResize(g_DxContext, width, height);
        }
        return 0;

    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_CLOSE)
        {
            int result = MessageBoxW(hWnd,
                L"Are you sure you want to exit?\n\nIf the hypervisor is running, it will be unloaded.",
                L"Confirm Exit",
                MB_YESNO | MB_ICONQUESTION);

            if (result == IDYES)
                PostQuitMessage(0);

            return 0;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

//=============================================================================
// Entry Point
//=============================================================================

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Check for administrator privileges
    if (!IsUserAnAdmin())
    {
        MessageBoxW(nullptr,
            L"This application requires administrator privileges.\n\nPlease run as administrator.",
            L"Administrator Required",
            MB_OK | MB_ICONERROR);
        return -1;
    }

    // Collect hardware fingerprint and validate auth
    Ombra::HardwareFingerprint fingerprint = Ombra::AuthManager::CollectFingerprint();
    Ombra::AuthResult authResult = Ombra::AuthManager::Validate(fingerprint);

    // Show auth error if validation failed
    if (!authResult.authorized)
    {
        MessageBoxA(nullptr,
            authResult.message.c_str(),
            "Authorization Failed",
            MB_OK | MB_ICONERROR);
        // Continue anyway to show auth UI
    }

    // Register window class
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = WINDOW_CLASS;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassExW(&wcex))
    {
        MessageBoxW(nullptr, L"Failed to register window class.", L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    // Calculate centered window position
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int windowX = (screenWidth - WINDOW_WIDTH) / 2;
    int windowY = (screenHeight - WINDOW_HEIGHT) / 2;

    // Create window
    HWND hWnd = CreateWindowW(
        WINDOW_CLASS,
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, // Fixed size
        windowX, windowY,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr,
        nullptr,
        hInstance,
        nullptr);

    if (!hWnd)
    {
        MessageBoxW(nullptr, L"Failed to create window.", L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    // Initialize DirectX 11
    if (!Ombra::DX11::Initialize(g_DxContext, hWnd, WINDOW_WIDTH, WINDOW_HEIGHT, true))
    {
        MessageBoxW(nullptr, L"Failed to initialize DirectX 11.", L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup dark theme
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    // Custom dark theme colors
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.09f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.23f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.06f, 0.07f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.06f, 0.06f, 0.07f, 0.75f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.06f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.30f, 0.32f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.35f, 0.35f, 0.37f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.65f, 0.35f, 0.25f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.55f, 0.30f, 0.20f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.70f, 0.40f, 0.30f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.45f, 0.25f, 0.15f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.55f, 0.30f, 0.20f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.65f, 0.35f, 0.25f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.45f, 0.25f, 0.15f, 0.76f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.55f, 0.30f, 0.20f, 0.86f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.65f, 0.35f, 0.25f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.20f, 0.23f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.45f, 0.25f, 0.15f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.65f, 0.35f, 0.25f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.45f, 0.25f, 0.15f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.55f, 0.30f, 0.20f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.65f, 0.35f, 0.25f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.25f, 0.25f, 0.27f, 0.86f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.55f, 0.30f, 0.20f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.45f, 0.25f, 0.15f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.15f, 0.17f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.45f, 0.25f, 0.15f, 0.43f);

    // Styling
    style.WindowRounding = 4.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.ScrollbarRounding = 3.0f;
    style.WindowPadding = ImVec2(10.0f, 10.0f);
    style.FramePadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);

    // Load fonts before backend initialization
    // NOTE: Add custom fonts here for a polished look. For now, default font is used.
    // Example:
    // io.Fonts->AddFontFromFileTTF("resources/fonts/Regular.ttf", 16.0f);  // Font 0: Regular
    // io.Fonts->AddFontFromFileTTF("resources/fonts/Bold.ttf", 16.0f);     // Font 1: Bold
    // io.Fonts->AddFontFromFileTTF("resources/fonts/Mono.ttf", 14.0f);     // Font 2: Mono
    // If fewer than 3 fonts, app.cpp falls back to default font safely.

    // Initialize ImGui backends
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(g_DxContext.device, g_DxContext.deviceContext);

    // Create application instance
    g_App = new Ombra::Application();
    if (!g_App->Initialize(hWnd, g_DxContext.device, g_DxContext.deviceContext))
    {
        MessageBoxW(hWnd, L"Failed to initialize application.", L"Error", MB_OK | MB_ICONERROR);
        goto cleanup;
    }

    // Set auth result
    g_App->SetAuthResult(authResult);

    // Show window
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Main loop
    {
        MSG msg = {};

        while (msg.message != WM_QUIT)
        {
            // Process messages
            while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            if (msg.message == WM_QUIT)
                break;

            // Start ImGui frame
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            // Update and render application
            g_App->Update();
            g_App->Render();

            // Render ImGui
            ImGui::Render();
            Ombra::DX11::BeginFrame(g_DxContext, 0.08f, 0.08f, 0.09f, 1.0f);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            Ombra::DX11::EndFrame(g_DxContext);
        }
    }

cleanup:
    // Cleanup in reverse order
    if (g_App)
    {
        g_App->Shutdown();
        delete g_App;
        g_App = nullptr;
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    Ombra::DX11::Shutdown(g_DxContext);

    DestroyWindow(hWnd);
    UnregisterClassW(WINDOW_CLASS, hInstance);

    return 0;
}
