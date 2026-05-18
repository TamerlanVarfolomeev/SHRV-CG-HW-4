#pragma once
#include "../Scene/Component.h"
#define NOMINMAX
#include <reactphysics3d/reactphysics3d.h>
#include <DirectXMath.h>
#include <vector>

namespace rp3d = reactphysics3d;

// Треугольная сетка (TriangleMesh) + ConcaveMeshShape — для статических террейнов/земли.
// Поддерживает любую геометрию (выпуклую и вогнутую), но работает только с STATIC телами.
class MeshCollider : public Component
{
public:
    // vertices — позиции всех вершин (x3 на треугольник, без индексов)
    // или vertices + indices (индексированная сетка).
    MeshCollider(rp3d::PhysicsCommon*                  common,
                 std::vector<DirectX::XMFLOAT3>        positions);

    // vertices + indices — индексированная геометрия (из ObjSubMesh)
    MeshCollider(rp3d::PhysicsCommon*                  common,
                 std::vector<DirectX::XMFLOAT3>        vertices,
                 std::vector<uint32_t>                 indices);

    ~MeshCollider();

    void FixedUpdate(float dt) override;

    // Коэффициент трения [0..1]; применяется при создании коллайдера
    float friction = 0.9f;

private:
    void EnsureCreated();

    rp3d::PhysicsCommon*    common_;
    std::vector<DirectX::XMFLOAT3> positions_;
    std::vector<uint32_t>   indices_;
    bool                    useIndices_ = false;

    rp3d::TriangleMesh*     triangleMesh_  = nullptr;
    rp3d::ConcaveMeshShape* shape_          = nullptr;
    rp3d::Collider*         collider_       = nullptr;
    bool                    created_        = false;
};
