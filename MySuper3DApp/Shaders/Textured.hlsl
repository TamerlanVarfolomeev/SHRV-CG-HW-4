#include "Common.hlsli"

// Текстурный шейдер.
// t0 = диффузная карта, s0 = глобальный семплер (LinearWrap, привязывается в Application)
Texture2D    g_DiffuseMap : register(t0);
SamplerState g_Sampler    : register(s0);

PixelInput VSMain(VertexInput input)
{
    PixelInput o;
    float4 worldPos = mul(float4(input.pos, 1.0f), g_World);
    o.posH   = mul(worldPos, g_ViewProj);
    o.posW   = worldPos.xyz;
    o.normal = mul(input.normal, (float3x3)g_WorldInvTranspose);
    o.uv     = input.uv;
    return o;
}

float4 PSMain(PixelInput input) : SV_Target
{
    float4 texColor = g_DiffuseMap.Sample(g_Sampler, input.uv);
    return texColor * g_AlbedoColor;
}
