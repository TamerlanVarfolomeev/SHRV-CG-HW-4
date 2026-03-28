#include "Common.hlsli"

// Простой шейдер без освещения.
// Цвет = g_AlbedoColor из материала.

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
    return g_AlbedoColor;
}
