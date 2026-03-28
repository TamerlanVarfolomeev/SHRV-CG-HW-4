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
    DirectX::XMFLOAT4 albedoColor = { 1,1,1,1 };
    float roughness    = 0.5f;
    float metallic     = 0.0f;
    float emissiveScale= 0.0f;
    float _pad0        = 0.0f;
    // итого 32 байта
};
static_assert(sizeof(CBMaterial) % 16 == 0);
