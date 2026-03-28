#include "States.h"

States::States(ID3D11Device* device)
{
    CreateRasterizerStates(device);
    CreateDepthStencilStates(device);
    CreateBlendStates(device);
    CreateSamplerStates(device);
}

void States::CreateRasterizerStates(ID3D11Device* device)
{
    D3D11_RASTERIZER_DESC rd = {};
    rd.FrontCounterClockwise = FALSE;
    rd.DepthBias             = 0;
    rd.DepthBiasClamp        = 0.0f;
    rd.SlopeScaledDepthBias  = 0.0f;
    rd.DepthClipEnable       = TRUE;
    rd.ScissorEnable         = FALSE;
    rd.MultisampleEnable     = FALSE;
    rd.AntialiasedLineEnable = FALSE;

    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_BACK;
    device->CreateRasterizerState(&rd, &Rasterizer.Solid);

    rd.FillMode = D3D11_FILL_WIREFRAME;
    rd.CullMode = D3D11_CULL_NONE;
    device->CreateRasterizerState(&rd, &Rasterizer.Wireframe);

    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_NONE;
    device->CreateRasterizerState(&rd, &Rasterizer.NoCull);
}

void States::CreateDepthStencilStates(ID3D11Device* device)
{
    D3D11_DEPTH_STENCIL_DESC dd = {};
    dd.StencilEnable    = FALSE;

    // Default: запись + тест (для непрозрачных объектов)
    dd.DepthEnable      = TRUE;
    dd.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ALL;
    dd.DepthFunc        = D3D11_COMPARISON_LESS;
    device->CreateDepthStencilState(&dd, &DepthStencil.Default);

    // ReadOnly: тест без записи (для прозрачных объектов)
    dd.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ZERO;
    device->CreateDepthStencilState(&dd, &DepthStencil.ReadOnly);

    // Disabled: полностью выключен (для UI / fullscreen quad)
    dd.DepthEnable      = FALSE;
    device->CreateDepthStencilState(&dd, &DepthStencil.Disabled);
}

void States::CreateBlendStates(ID3D11Device* device)
{
    D3D11_BLEND_DESC bd = {};
    bd.AlphaToCoverageEnable  = FALSE;
    bd.IndependentBlendEnable = FALSE;
    auto& rt = bd.RenderTarget[0];

    // Opaque
    rt.BlendEnable    = FALSE;
    rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    device->CreateBlendState(&bd, &Blend.Opaque);

    // AlphaBlend: src*srcA + dst*(1-srcA)
    rt.BlendEnable           = TRUE;
    rt.SrcBlend              = D3D11_BLEND_SRC_ALPHA;
    rt.DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
    rt.BlendOp               = D3D11_BLEND_OP_ADD;
    rt.SrcBlendAlpha         = D3D11_BLEND_ONE;
    rt.DestBlendAlpha        = D3D11_BLEND_ZERO;
    rt.BlendOpAlpha          = D3D11_BLEND_OP_ADD;
    device->CreateBlendState(&bd, &Blend.AlphaBlend);

    // Additive: src + dst
    rt.SrcBlend  = D3D11_BLEND_ONE;
    rt.DestBlend = D3D11_BLEND_ONE;
    device->CreateBlendState(&bd, &Blend.Additive);
}

void States::CreateSamplerStates(ID3D11Device* device)
{
    D3D11_SAMPLER_DESC sd = {};
    sd.MaxLOD        = D3D11_FLOAT32_MAX;
    sd.ComparisonFunc= D3D11_COMPARISON_NEVER;

    sd.Filter   = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    device->CreateSamplerState(&sd, &Sampler.LinearWrap);

    sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    device->CreateSamplerState(&sd, &Sampler.LinearClamp);

    sd.Filter   = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    device->CreateSamplerState(&sd, &Sampler.PointWrap);
}
