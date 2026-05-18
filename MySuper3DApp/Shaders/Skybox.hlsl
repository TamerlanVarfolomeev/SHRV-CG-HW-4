// Skybox.hlsl — окружение из кубической карты.
// Куб «приклеен» к камере (трансляция view-матрицы зануляется).
// Принудительно z = w → depth = 1.0 → LESS_EQUAL пропускает скайбокс
// только там, где другая геометрия не успела записать глубину.

#include "Common.hlsli"

TextureCube  g_SkyMap  : register(t0);
SamplerState g_Sampler : register(s0);

struct SkyVSOut
{
    float4 posH : SV_POSITION;
    float3 dir  : TEXCOORD0;   // model-space позиция куба = направление выборки TextureCube
};

SkyVSOut VSMain(VertexInput input)
{
    SkyVSOut o;

    // Снимаем трансляцию из view-матрицы (только повороты камеры)
    matrix viewNoTrans = g_View;
    viewNoTrans._41 = 0.0f;
    viewNoTrans._42 = 0.0f;
    viewNoTrans._43 = 0.0f;

    float4 viewPos = mul(float4(input.pos, 1.0f), viewNoTrans);
    float4 clipPos = mul(viewPos, g_Proj);

    // z = w → после перспективного деления получится depth = 1.0
    clipPos.z = clipPos.w;

    o.posH = clipPos;
    o.dir  = input.pos;        // позиция вершины куба — это направление от центра
    return o;
}

float4 PSMain(SkyVSOut input) : SV_Target
{
    return g_SkyMap.Sample(g_Sampler, input.dir);
}
