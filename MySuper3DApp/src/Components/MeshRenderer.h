#pragma once
#include "../Scene/Component.h"
#include "../Graphics/Mesh.h"
#include "../Graphics/Shader.h"
#include "../Graphics/Texture.h"
#include "../Graphics/ConstantBuffer.h"
#include "../Graphics/ConstantBuffers.h"

// Компонент отрисовки: связывает Mesh, Shader и опциональную Texture.
// Не владеет ни одним из ресурсов — все указатели должны жить дольше компонента.
class MeshRenderer : public Component
{
public:
    // texture может быть nullptr — тогда слот t0 не трогается (Unlit-шейдер)
    MeshRenderer(ID3D11Device* device, Mesh* mesh, Shader* shader,
                 Texture* texture = nullptr);

    void Render(const RenderContext& ctx) override;
    void RenderShadow(const RenderContext& ctx) override;

    // Публичные параметры материала (напрямую меняй из кода)
    CBMaterial material;

private:
    Mesh*    mesh_    = nullptr;
    Shader*  shader_  = nullptr;
    Texture* texture_ = nullptr;

    ConstantBuffer<CBPerObject> cbObject_;
    ConstantBuffer<CBMaterial>  cbMaterial_;
};
