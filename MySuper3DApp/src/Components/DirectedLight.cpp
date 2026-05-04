#include "DirectedLight.h"
#include "../Scene/RenderContext.h"
#include "../Graphics/GraphicsDevice.h"
#include <DirectXMath.h>

using namespace DirectX;

void DirectedLight::Render(const RenderContext& ctx)
{
    // Ленивая инициализация: создаём CB при первом рендере, когда уже есть device
    if (!initialized_)
    {
        cbLight_     = ConstantBuffer<CBDirectedLight>(ctx.gfx->GetDevice());
        initialized_ = true;
    }

    auto* dctx = ctx.gfx->GetContext();

    // Нормализуем направление на CPU
    XMVECTOR dir = XMVector3Normalize(XMLoadFloat3(&direction));

    CBDirectedLight data;
    XMStoreFloat3(&data.direction, dir);
    data._pad0        = 0.0f;
    data.color        = color;
    data.intensity    = intensity;
    data.ambientColor = ambientColor;
    data._pad1        = 0.0f;

    cbLight_.Update(dctx, data);
    cbLight_.Bind(dctx, ShaderStage::PS, 4);
}
