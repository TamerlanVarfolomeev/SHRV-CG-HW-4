#pragma once
#include "../Scene/Component.h"
#include "../Graphics/ConstantBuffer.h"
#include "../Graphics/ConstantBuffers.h"
#include <DirectXMath.h>

// Направленный источник света (Фонг).
// Добавь компонент на любой GameObject в сцене — свет применится ко всем объектам
// (компонент перезаписывает слот b4 PS, привязанный Application::BeginFrame).
//
// Пример:
//   auto* sun = scene.CreateObject("Sun");
//   sun->transform.rotation = { 45.0f, 30.0f, 0.0f };
//   auto* light = sun->AddComponent<DirectedLight>();
//   light->color     = { 1.0f, 0.95f, 0.8f };
//   light->intensity = 1.2f;
class DirectedLight : public Component
{
public:
    // Направление, куда летит свет (не к источнику).
    // Нормализуется автоматически при рендере.
    // По умолчанию — диагональный свет сверху-слева.
    DirectX::XMFLOAT3 direction   = { -0.5f, -1.0f, -0.3f };

    DirectX::XMFLOAT3 color       = {  1.0f,  0.95f, 0.85f }; // тёплый белый
    float             intensity   = 1.0f;

    // Фоновое освещение — видно на теневых сторонах.
    DirectX::XMFLOAT3 ambientColor= {  0.15f, 0.18f, 0.25f }; // холодный синеватый

    void Render(const RenderContext& ctx) override;

private:
    ConstantBuffer<CBDirectedLight> cbLight_;
    bool initialized_ = false;
};
