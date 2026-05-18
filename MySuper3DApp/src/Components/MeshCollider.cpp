#include "MeshCollider.h"
#include "RigidBody.h"
#include "../Scene/GameObject.h"
#include <cstdio>

MeshCollider::MeshCollider(rp3d::PhysicsCommon*           common,
                           std::vector<DirectX::XMFLOAT3> positions)
    : common_(common), positions_(std::move(positions))
{}

MeshCollider::MeshCollider(rp3d::PhysicsCommon*           common,
                           std::vector<DirectX::XMFLOAT3> vertices,
                           std::vector<uint32_t>          indices)
    : common_(common), positions_(std::move(vertices)),
      indices_(std::move(indices)), useIndices_(true)
{}

MeshCollider::~MeshCollider()
{
    if (shape_)         common_->destroyConcaveMeshShape(shape_);
    if (triangleMesh_)  common_->destroyTriangleMesh(triangleMesh_);
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

    std::unique_ptr<rp3d::TriangleVertexArray> vertexArray;

    if (useIndices_)
    {
        // Индексированная геометрия: вершины + индексы
        uint32_t nbTriangles = static_cast<uint32_t>(indices_.size()) / 3;

        vertexArray = std::make_unique<rp3d::TriangleVertexArray>(
            static_cast<uint32_t>(positions_.size()),   // nbVertices
            positions_.data(),                           // verticesStart
            sizeof(DirectX::XMFLOAT3),                   // verticesStride
            nbTriangles,                                 // nbTriangles
            indices_.data(),                             // indicesStart
            sizeof(uint32_t) * 3,                        // indicesStride (3 indices per triangle)
            rp3d::TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
            rp3d::TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE);
    }
    else
    {
        // Неиндексированная: каждая тройка вершин = треугольник
        uint32_t nbTriangles = static_cast<uint32_t>(positions_.size()) / 3;

        vertexArray = std::make_unique<rp3d::TriangleVertexArray>(
            nbTriangles * 3,                             // nbVertices
            positions_.data(),                           // verticesStart
            sizeof(DirectX::XMFLOAT3),                   // verticesStride
            nbTriangles,                                 // nbTriangles
            nullptr,                                     // indicesStart (no indices)
            0,                                           // indicesStride
            rp3d::TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
            rp3d::TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE);
    }

    std::vector<rp3d::Message> messages;
    triangleMesh_ = common_->createTriangleMesh(*vertexArray, messages);

    for (const auto& msg : messages)
        printf("[MeshCollider] %s\n", msg.text.c_str());

    if (!triangleMesh_)
    {
        printf("[MeshCollider] Не удалось создать TriangleMesh\n");
        return;
    }

    shape_    = common_->createConcaveMeshShape(triangleMesh_);
    collider_ = rb->GetBody()->addCollider(shape_, rp3d::Transform::identity());
    collider_->getMaterial().setFrictionCoefficient(friction);
}
