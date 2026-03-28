#pragma once
#include "../Scene/Component.h"
#include "../Graphics/Mesh.h"
#include "../Graphics/Shader.h"
#include "../Graphics/ConstantBuffer.h"
#include "../Graphics/ConstantBuffers.h"

// Компонент отрисовки: связывает Mesh и Shader, обновляет CBPerObject.
class MeshRenderer : public Component
{
public:
    // Принимает сырые указатели — MeshRenderer НЕ владеет мешем и шейдером.
    // Меш и шейдер должны жить дольше этого компонента (например, в Application).
    MeshRenderer(ID3D11Device* device, Mesh* mesh, Shader* shader);

    void Render(const RenderContext& ctx) override;

    // Публичные параметры материала (напрямую меняй из кода)
    CBMaterial material;

private:
    Mesh*   mesh_   = nullptr;
    Shader* shader_ = nullptr;

    ConstantBuffer<CBPerObject> cbObject_;
    ConstantBuffer<CBMaterial>  cbMaterial_;
};
