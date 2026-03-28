#pragma once
#include <d3d11.h>
#include <dxgi.h>
#include <wrl.h>
#include "GraphicsDevice.h"

class SwapChainTarget
{
public:
    SwapChainTarget(GraphicsDevice& gfx, HWND hwnd, int width, int height);

    void Clear(const float color[4]);
    void Present(bool vsync = true);
    void Resize(GraphicsDevice& gfx, int width, int height);

    ID3D11RenderTargetView* GetRTV() const { return rtv_.Get(); }
    ID3D11DepthStencilView* GetDSV() const { return dsv_.Get(); }

private:
    void CreateRTVAndDSV(GraphicsDevice& gfx, int width, int height);

    Microsoft::WRL::ComPtr<IDXGISwapChain>          swapChain_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  rtv_;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  dsv_;
};
