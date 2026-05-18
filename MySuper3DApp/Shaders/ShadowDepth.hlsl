// ShadowDepth.hlsl — depth-only пас.
// Рендерим геометрию из точки зрения источника света → пишем глубину в shadow map.
// PS пустой: запись идёт только в DSV (RT не привязан).

#include "Common.hlsli"

struct ShadowVSOut
{
    float4 posH : SV_POSITION;
};

ShadowVSOut VSMain(VertexInput input)
{
    ShadowVSOut o;
    float4 worldPos = mul(float4(input.pos, 1.0f), g_World);
    o.posH = mul(worldPos, g_LightViewProj);
    return o;
}

void PSMain(ShadowVSOut input)
{
    // Цвет не пишем — RT не привязан. Запись депта идёт сама.
}
