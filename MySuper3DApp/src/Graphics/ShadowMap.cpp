#include "ShadowMap.h"
#include <stdexcept>

using namespace Microsoft::WRL;

ShadowMap::ShadowMap(ID3D11Device* device, int resolution)
    : resolution_(resolution)
{
    // --- Текстура: TYPELESS, чтобы навесить и DSV (D24S8), и SRV (R24 чтение) ---
    D3D11_TEXTURE2D_DESC td = {};
    td.Width            = resolution;
    td.Height           = resolution;
    td.MipLevels        = 1;
    td.ArraySize        = 1;
    td.Format           = DXGI_FORMAT_R24G8_TYPELESS;
    td.SampleDesc.Count = 1;
    td.Usage            = D3D11_USAGE_DEFAULT;
    td.BindFlags        = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

    if (FAILED(device->CreateTexture2D(&td, nullptr, &texture_)))
        throw std::runtime_error("ShadowMap: CreateTexture2D failed");

    // --- DSV для записи depth во время shadow pass ---
    D3D11_DEPTH_STENCIL_VIEW_DESC dvd = {};
    dvd.Format        = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

    if (FAILED(device->CreateDepthStencilView(texture_.Get(), &dvd, &dsv_)))
        throw std::runtime_error("ShadowMap: CreateDepthStencilView failed");

    // --- SRV для семплинга в main pass (depth → красный канал R24_UNORM) ---
    D3D11_SHADER_RESOURCE_VIEW_DESC svd = {};
    svd.Format                    = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    svd.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    svd.Texture2D.MipLevels       = 1;
    svd.Texture2D.MostDetailedMip = 0;

    if (FAILED(device->CreateShaderResourceView(texture_.Get(), &svd, &srv_)))
        throw std::runtime_error("ShadowMap: CreateShaderResourceView failed");
}

void ShadowMap::BeginPass(ID3D11DeviceContext* ctx)
{
    // Только DSV, без render target — пишем только в depth
    ctx->OMSetRenderTargets(0, nullptr, dsv_.Get());
    ctx->ClearDepthStencilView(dsv_.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    D3D11_VIEWPORT vp = {};
    vp.Width    = static_cast<float>(resolution_);
    vp.Height   = static_cast<float>(resolution_);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    ctx->RSSetViewports(1, &vp);
}
