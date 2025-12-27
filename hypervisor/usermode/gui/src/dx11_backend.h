#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <windows.h>

namespace Ombra {
namespace DX11 {

struct Context {
    ID3D11Device*           device;
    ID3D11DeviceContext*    deviceContext;
    IDXGISwapChain*         swapChain;
    ID3D11RenderTargetView* renderTargetView;
    HWND                    hwnd;
    unsigned int            width;
    unsigned int            height;
    bool                    vsync;

    Context()
        : device(nullptr)
        , deviceContext(nullptr)
        , swapChain(nullptr)
        , renderTargetView(nullptr)
        , hwnd(nullptr)
        , width(0)
        , height(0)
        , vsync(true)
    {}
};

bool Initialize(Context& ctx, HWND hwnd, unsigned int width, unsigned int height, bool vsync = true);
void Shutdown(Context& ctx);
bool OnResize(Context& ctx, unsigned int width, unsigned int height);
void BeginFrame(Context& ctx, float r, float g, float b, float a);
void EndFrame(Context& ctx);

} // namespace DX11
} // namespace Ombra
