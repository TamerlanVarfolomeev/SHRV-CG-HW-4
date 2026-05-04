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
    // Базовый цвет поверхности = текстура × тинт материала
    float4 texSample = g_DiffuseMap.Sample(g_Sampler, input.uv);
    float3 baseColor = texSample.rgb * g_AlbedoColor.rgb;

    // Векторы освещения (всё в мировом пространстве)
    float3 N = normalize(input.normal);
    float3 L = normalize(-g_LightDir);               // от поверхности к источнику
    float3 V = normalize(g_CamPos - input.posW);     // от поверхности к зрителю
    float3 R = reflect(-L, N);                       // отражённый луч (модель Фонга)

    // --- Ambient (фоновое) ---
    float3 ambient = g_AmbientColor * baseColor;

    // --- Diffuse (Ламберт) ---
    float  NdL     = max(dot(N, L), 0.0f);
    float3 diffuse = NdL * g_LightColor * baseColor;

    // --- Specular (Фонг) ---
    // Блик появляется только на освещённой стороне (NdL > 0)
    float  RdV     = max(dot(R, V), 0.0f);
    float  spec    = (NdL > 0.0f) ? pow(RdV, max(g_Shininess, 1.0f)) : 0.0f;
    float3 specular = g_SpecularStr * spec * g_LightColor;

    // --- Итог ---
    float3 lit = ambient + (diffuse + specular) * g_LightIntensity;

    return float4(lit, texSample.a * g_AlbedoColor.a);
}
