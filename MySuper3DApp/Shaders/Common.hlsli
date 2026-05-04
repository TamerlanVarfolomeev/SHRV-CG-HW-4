// Common.hlsli — подключай в каждый шейдер: #include "Common.hlsli"
// Слоты фиксированы и совпадают с ConstantBuffers.h на C++ стороне.

#pragma pack_matrix(row_major)

// b0 — данные кадра
cbuffer CBPerFrame : register(b0)
{
    float g_TotalTime;
    float g_DeltaTime;
    float g_ScreenWidth;
    float g_ScreenHeight;
};

// b1 — данные камеры
cbuffer CBPerCamera : register(b1)
{
    matrix g_View;
    matrix g_Proj;
    matrix g_ViewProj;
    float3 g_CamPos;
    float  _camPad;
};

// b2 — данные объекта (обновляется на каждый DrawCall)
cbuffer CBPerObject : register(b2)
{
    matrix g_World;
    matrix g_WorldInvTranspose;
};

// b3 — материал (Phong)
cbuffer CBMaterial : register(b3)
{
    float4 g_AlbedoColor;
    float  g_Shininess;      // показатель степени зеркального блика
    float  g_SpecularStr;    // сила зеркального отражения [0..1]
    float  g_EmissiveScale;
    float  _matPad;
};

// b4 — направленный источник света
cbuffer CBDirectedLight : register(b4)
{
    float3 g_LightDir;       // направление, куда летит свет (нормализованный)
    float  _lightPad0;
    float3 g_LightColor;     // RGB цвет источника
    float  g_LightIntensity;
    float3 g_AmbientColor;   // фоновое освещение
    float  _lightPad1;
};

// Стандартная вершина — совпадает со struct Vertex в Mesh.h
struct VertexInput
{
    float3 pos    : POSITION;
    float3 normal : NORMAL;
    float2 uv     : TEXCOORD0;
};

// Данные, передаваемые из VS в PS
struct PixelInput
{
    float4 posH   : SV_POSITION;  // clip-space
    float3 posW   : TEXCOORD0;    // world-space (для освещения)
    float3 normal : TEXCOORD1;    // world-space normal
    float2 uv     : TEXCOORD2;
};
