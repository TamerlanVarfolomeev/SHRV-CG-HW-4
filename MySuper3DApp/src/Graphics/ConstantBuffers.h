#pragma once
#include <DirectXMath.h>

// Контракт между C++ и HLSL.
// Каждая структура должна быть кратна 16 байтам (правило HLSL packing).
// Слоты фиксированы: b0=Frame, b1=Camera, b2=Object, b3=Material.

struct CBPerFrame                           // register(b0)
{
    float totalTime;
    float deltaTime;
    float screenWidth;
    float screenHeight;
    // итого 16 байт
};
static_assert(sizeof(CBPerFrame) % 16 == 0);

struct CBPerCamera                          // register(b1)
{
    DirectX::XMFLOAT4X4 view;              // 64 байта
    DirectX::XMFLOAT4X4 proj;              // 64 байта
    DirectX::XMFLOAT4X4 viewProj;          // 64 байта
    DirectX::XMFLOAT3   camPos;
    float               _pad0 = 0.0f;      // до 16 байт
    // итого 208 байт
};
static_assert(sizeof(CBPerCamera) % 16 == 0);

struct CBPerObject                          // register(b2)
{
    DirectX::XMFLOAT4X4 world;             // 64 байта
    DirectX::XMFLOAT4X4 worldInvTranspose; // 64 байта — для корректного преобразования нормалей
    // итого 128 байт
};
static_assert(sizeof(CBPerObject) % 16 == 0);

struct CBMaterial                           // register(b3)
{
    DirectX::XMFLOAT4 albedoColor  = { 1,1,1,1 };
    float shininess    = 32.0f;  // Phong: показатель блика (степень specular)
    float specularStr  = 0.5f;   // Phong: сила зеркального отражения [0..1]
    float emissiveScale= 0.0f;
    float _pad0        = 0.0f;
    // итого 32 байта
};
static_assert(sizeof(CBMaterial) % 16 == 0);

// Направленный источник света                register(b4)
struct CBDirectedLight
{
    DirectX::XMFLOAT3 direction   = {-0.5f, -1.0f, -0.3f}; // куда указывает свет (не к источнику)
    float             _pad0       = 0.0f;
    DirectX::XMFLOAT3 color       = { 1.0f,  0.95f, 0.85f}; // тёплый белый
    float             intensity   = 1.0f;
    DirectX::XMFLOAT3 ambientColor= { 0.15f, 0.18f, 0.25f}; // холодный синеватый ambient
    float             _pad1       = 0.0f;
    // итого 48 байт
};
static_assert(sizeof(CBDirectedLight) % 16 == 0);
