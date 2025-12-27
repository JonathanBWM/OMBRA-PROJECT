#include "dx11_backend.h"
#include <d3dcommon.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace Ombra {
namespace DX11 {

static bool CreateRenderTarget(Context& ctx) {
    ID3D11Texture2D* backBuffer = nullptr;
    HRESULT hr = ctx.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (FAILED(hr)) {
        return false;
    }

    hr = ctx.device->CreateRenderTargetView(backBuffer, nullptr, &ctx.renderTargetView);
    backBuffer->Release();

    if (FAILED(hr)) {
        return false;
    }

    return true;
}

static void ReleaseRenderTarget(Context& ctx) {
    if (ctx.renderTargetView) {
        ctx.renderTargetView->Release();
        ctx.renderTargetView = nullptr;
    }
}

bool Initialize(Context& ctx, HWND hwnd, unsigned int width, unsigned int height, bool vsync) {
    ctx.hwnd = hwnd;
    ctx.width = width;
    ctx.height = height;
    ctx.vsync = vsync;

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    D3D_FEATURE_LEVEL featureLevel;
    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createDeviceFlags,
        featureLevels,
        sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL),
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &ctx.swapChain,
        &ctx.device,
        &featureLevel,
        &ctx.deviceContext
    );

    if (FAILED(hr)) {
        return false;
    }

    if (!CreateRenderTarget(ctx)) {
        Shutdown(ctx);
        return false;
    }

    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    ctx.deviceContext->RSSetViewports(1, &viewport);

    return true;
}

void Shutdown(Context& ctx) {
    ReleaseRenderTarget(ctx);

    if (ctx.swapChain) {
        ctx.swapChain->Release();
        ctx.swapChain = nullptr;
    }

    if (ctx.deviceContext) {
        ctx.deviceContext->Release();
        ctx.deviceContext = nullptr;
    }

    if (ctx.device) {
        ctx.device->Release();
        ctx.device = nullptr;
    }

    ctx.hwnd = nullptr;
    ctx.width = 0;
    ctx.height = 0;
}

bool OnResize(Context& ctx, unsigned int width, unsigned int height) {
    if (!ctx.device || !ctx.swapChain) {
        return false;
    }

    ctx.width = width;
    ctx.height = height;

    ReleaseRenderTarget(ctx);

    HRESULT hr = ctx.swapChain->ResizeBuffers(
        0,
        width,
        height,
        DXGI_FORMAT_UNKNOWN,
        0
    );

    if (FAILED(hr)) {
        return false;
    }

    if (!CreateRenderTarget(ctx)) {
        return false;
    }

    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    ctx.deviceContext->RSSetViewports(1, &viewport);

    return true;
}

void BeginFrame(Context& ctx, float r, float g, float b, float a) {
    float clearColor[4] = { r, g, b, a };
    ctx.deviceContext->OMSetRenderTargets(1, &ctx.renderTargetView, nullptr);
    ctx.deviceContext->ClearRenderTargetView(ctx.renderTargetView, clearColor);
}

void EndFrame(Context& ctx) {
    ctx.swapChain->Present(ctx.vsync ? 1 : 0, 0);
}

} // namespace DX11
} // namespace Ombra
