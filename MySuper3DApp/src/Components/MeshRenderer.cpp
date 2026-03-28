#include "MeshRenderer.h"
#include "../Scene/RenderContext.h"
#include "../Scene/GameObject.h"
#include "../Graphics/GraphicsDevice.h"
#include "../Scene/Camera.h"

using namespace DirectX;

MeshRenderer::MeshRenderer(ID3D11Device* device, Mesh* mesh, Shader* shader)
    : mesh_(mesh), shader_(shader),
      cbObject_(device), cbMaterial_(device)
{
}

void MeshRenderer::Render(const RenderContext& ctx)
{
    auto* context = ctx.gfx->GetContext();

    // --- CBPerObject ---
    XMMATRIX world = gameObject->transform.GetWorldMatrix();

    CBPerObject obj;
    XMStoreFloat4x4(&obj.world, world);

    // WorldInvTranspose нужен для корректного преобразования нормалей при неравномерном масштабе
    XMMATRIX wit = XMMatrixTranspose(XMMatrixInverse(nullptr, world));
    XMStoreFloat4x4(&obj.worldInvTranspose, wit);

    cbObject_.Update(context, obj);
    cbObject_.Bind(context, ShaderStage::Both, 2);

    // --- CBMaterial ---
    cbMaterial_.Update(context, material);
    cbMaterial_.Bind(context, ShaderStage::PS, 3);

    // --- Шейдер и меш ---
    shader_->Bind(context);
    mesh_->Draw(context);
}
