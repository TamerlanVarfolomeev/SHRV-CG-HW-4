#pragma once
#include "../Scene/Component.h"
#include "../Graphics/Shader.h"
#include "../Graphics/Mesh.h"
#include "../Graphics/CubeTexture.h"
#include "../Graphics/States.h"
#include <memory>

// Скайбокс — рисует кубическую карту окружения.
// Куб всегда центрирован на камере; глубина принудительно = 1 (z = w в шейдере).
//
// Не зависит от Transform своего GameObject — позиция/масштаб игнорируются.
// CubeTexture должна жить дольше компонента (компонент не владеет ею).
//
// Пример:
//   CubeTexture sky(dev, { "Assets/skybox/px.png", ... });
//   auto* go = scene.CreateObject("Sky");
//   go->AddComponent<Skybox>(dev, &states, &sky);
class Skybox : public Component
{
public:
    Skybox(ID3D11Device* device, States* states, CubeTexture* cubeTex);

    void Render(const RenderContext& ctx) override;

    void SetCubeTexture(CubeTexture* tex) { cubeTex_ = tex; }

private:
    States*                 states_;
    CubeTexture*            cubeTex_;
    std::unique_ptr<Shader> shader_;
    std::unique_ptr<Mesh>   cubeMesh_;
};
