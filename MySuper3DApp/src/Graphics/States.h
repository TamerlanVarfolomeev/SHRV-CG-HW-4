#pragma once
#include <d3d11.h>
#include <wrl.h>

// Все состояния создаются один раз при старте и переиспользуются.
// Используй States::Rasterizer::Solid, States::Blend::Opaque и т.д.

class States
{
public:
    explicit States(ID3D11Device* device);

    struct Rasterizer {
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> Solid;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> Wireframe;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> NoCull;
    } Rasterizer;

    struct DepthStencil {
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> Default;     // запись + тест
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> ReadOnly;    // только тест (для прозрачных)
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> Disabled;    // выкл (для UI)
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> SkyboxState; // LESS_EQUAL, write off (для скайбокса при z=1)
    } DepthStencil;

    struct Blend {
        Microsoft::WRL::ComPtr<ID3D11BlendState> Opaque;
        Microsoft::WRL::ComPtr<ID3D11BlendState> AlphaBlend;
        Microsoft::WRL::ComPtr<ID3D11BlendState> Additive;
    } Blend;

    struct Sampler {
        Microsoft::WRL::ComPtr<ID3D11SamplerState> LinearWrap;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> LinearClamp;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> PointWrap;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> ShadowCompare; // SampleCmp с LESS + border=1
    } Sampler;

private:
    void CreateRasterizerStates(ID3D11Device* device);
    void CreateDepthStencilStates(ID3D11Device* device);
    void CreateBlendStates(ID3D11Device* device);
    void CreateSamplerStates(ID3D11Device* device);
};
