#pragma once
#include "../Scene/Component.h"
#include <reactphysics3d/reactphysics3d.h>
#include <DirectXMath.h>
#include <vector>

namespace rp3d = reactphysics3d;

// Строит выпуклый корпус (ConvexMesh) из переданных вершин.
// Подходит только для Dynamic/Kinematic тел — для Static используй BoxCollider.
// positions — список позиций вершин (например, из ObjSubMesh::meshPositions).
class MeshCollider : public Component
{
public:
    MeshCollider(rp3d::PhysicsCommon*                  common,
                 std::vector<DirectX::XMFLOAT3>        positions);
    ~MeshCollider();

    void FixedUpdate(float dt) override;

private:
    void EnsureCreated();

    rp3d::PhysicsCommon*    common_;
    std::vector<DirectX::XMFLOAT3> positions_;
    rp3d::ConvexMesh*       convexMesh_ = nullptr;
    rp3d::ConvexMeshShape*  shape_      = nullptr;
    bool                    created_    = false;
};
