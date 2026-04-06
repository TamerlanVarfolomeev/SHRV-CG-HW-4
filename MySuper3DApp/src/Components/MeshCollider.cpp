#include "MeshCollider.h"
#include "RigidBody.h"
#include "../Scene/GameObject.h"
#include <cstdio>

MeshCollider::MeshCollider(rp3d::PhysicsCommon*           common,
                           std::vector<DirectX::XMFLOAT3> positions)
    : common_(common), positions_(std::move(positions))
{}

MeshCollider::~MeshCollider()
{
    if (shape_)      common_->destroyConvexMeshShape(shape_);
    if (convexMesh_) common_->destroyConvexMesh(convexMesh_);
}

void MeshCollider::FixedUpdate(float)
{
    EnsureCreated();
}

void MeshCollider::EnsureCreated()
{
    if (created_) return;
    created_ = true;

    auto* rb = gameObject->GetComponent<RigidBody>();
    if (!rb || positions_.empty()) return;

    // RP3D VertexArray ожидает float[3] массив; XMFLOAT3 совместим по памяти
    rp3d::VertexArray vertexArray(
        positions_.data(),
        sizeof(DirectX::XMFLOAT3),
        static_cast<rp3d::uint32>(positions_.size()),
        rp3d::VertexArray::DataType::VERTEX_FLOAT_TYPE);

    std::vector<rp3d::Message> messages;
    convexMesh_ = common_->createConvexMesh(vertexArray, messages);

    for (const auto& msg : messages)
        printf("[MeshCollider] %s\n", msg.text.c_str());

    if (!convexMesh_) return;

    shape_ = common_->createConvexMeshShape(convexMesh_);
    rb->GetBody()->addCollider(shape_, rp3d::Transform::identity());
}
