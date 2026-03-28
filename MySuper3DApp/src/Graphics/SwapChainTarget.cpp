#include "SwapChainTarget.h"
#include <stdexcept>

SwapChainTarget::SwapChainTarget(GraphicsDevice& gfx, HWND hwnd, int width, int height)
{
    // Получаем DXGI factory через цепочку QueryInterface
    Microsoft::WRL::ComPtr<IDXGIDevice>  dxgiDevice;
    Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
    Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;

    gfx.GetDevice()->QueryInterface(__uuidof(IDXGIDevice),  (void**)&dxgiDevice);
    dxgiDevice->GetAdapter(&dxgiAdapter);
    dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);

    DXGI_SWAP_CHAIN_DESC sd        = {};
    sd.BufferCount                 = 2;
    sd.BufferDesc.Width            = width;
    sd.BufferDesc.Height           = height;
    sd.BufferDesc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate      = { 60, 1 };
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling          = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.BufferUsage                 = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow                = hwnd;
    sd.Windowed                    = TRUE;
    sd.SwapEffect                  = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags                       = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.SampleDesc                  = { 1, 0 };

    HRESULT hr = dxgiFactory->CreateSwapChain(gfx.GetDevice(), &sd, &swapChain_);
    if (FAILED(hr))
        throw std::runtime_error("Failed to create swap chain");

    CreateRTVAndDSV(gfx, width, height);
}

void SwapChainTarget::CreateRTVAndDSV(GraphicsDevice& gfx, int width, int height)
{
    // RTV
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    gfx.GetDevice()->CreateRenderTargetView(backBuffer.Get(), nullptr, &rtv_);

    // Depth-stencil texture
    D3D11_TEXTURE2D_DESC dd = {};
    dd.Width              = width;
    dd.Height             = height;
    dd.MipLevels          = 1;
    dd.ArraySize          = 1;
    dd.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dd.SampleDesc         = { 1, 0 };
    dd.Usage              = D3D11_USAGE_DEFAULT;
    dd.BindFlags          = D3D11_BIND_DEPTH_STENCIL;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthTex;
    gfx.GetDevice()->CreateTexture2D(&dd, nullptr, &depthTex);
    gfx.GetDevice()->CreateDepthStencilView(depthTex.Get(), nullptr, &dsv_);
}

void SwapChainTarget::Clear(const float color[4])
{
    // Очистка RTV и DSV выполняется снаружи через Application,
    // поскольку для этого нужен context. Метод оставлен для convenience.
}

void SwapChainTarget::Present(bool vsync)
{
    swapChain_->Present(vsync ? 1 : 0, 0);
}

void SwapChainTarget::Resize(GraphicsDevice& gfx, int width, int height)
{
    rtv_.Reset();
    dsv_.Reset();

    swapChain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    CreateRTVAndDSV(gfx, width, height);
}
