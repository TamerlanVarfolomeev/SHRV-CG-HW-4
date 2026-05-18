#include "Skybox.h"
#include "../Scene/RenderContext.h"
#include "../Graphics/GraphicsDevice.h"
#include <vector>

Skybox::Skybox(ID3D11Device* device, States* states, CubeTexture* cubeTex)
    : states_(states), cubeTex_(cubeTex)
{
    shader_ = std::make_unique<Shader>(device,
        L"Shaders/Skybox.hlsl",
        std::vector<InputElement>{
            { "POSITION", DXGI_FORMAT_R32G32B32_FLOAT },
            { "NORMAL",   DXGI_FORMAT_R32G32B32_FLOAT },
            { "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT    },
        });

    cubeMesh_ = std::make_unique<Mesh>(Mesh::CreateCube(device));
}

void Skybox::Render(const RenderContext& ctx)
{
    if (!cubeTex_ || !cubeTex_->IsValid()) return;

    auto* dctx = ctx.gfx->GetContext();

    // --- Сохраняем текущие состояния, чтобы корректно вернуть после ---
    ID3D11RasterizerState*   prevRS  = nullptr;
    ID3D11DepthStencilState* prevDSS = nullptr;
    ID3D11SamplerState*      prevSamp= nullptr;
    UINT                     prevStencilRef = 0;
    dctx->RSGetState(&prevRS);
    dctx->OMGetDepthStencilState(&prevDSS, &prevStencilRef);
    dctx->PSGetSamplers(0, 1, &prevSamp);

    // --- Скайбокс-состояния: NoCull (камера внутри куба), LESS_EQUAL + write off ---
    dctx->RSSetState(states_->Rasterizer.NoCull.Get());
    dctx->OMSetDepthStencilState(states_->DepthStencil.SkyboxState.Get(), 0);

    // LinearClamp на s0 — без него на стыках граней могут быть швы из-за wrap
    auto* clampSampler = states_->Sampler.LinearClamp.Get();
    dctx->PSSetSamplers(0, 1, &clampSampler);

    // --- Рендер ---
    shader_->Bind(dctx);
    cubeTex_->Bind(dctx, 0);
    cubeMesh_->Draw(dctx);
    cubeTex_->Unbind(dctx, 0);

    // --- Восстанавливаем все состояния ---
    dctx->RSSetState(prevRS);
    dctx->OMSetDepthStencilState(prevDSS, prevStencilRef);
    dctx->PSSetSamplers(0, 1, &prevSamp);

    if (prevRS)   prevRS->Release();
    if (prevDSS)  prevDSS->Release();
    if (prevSamp) prevSamp->Release();
}
