#pragma once
#include <reactphysics3d/reactphysics3d.h>

namespace rp3d = reactphysics3d;

// Владеет PhysicsCommon и PhysicsWorld.
// Живёт в Application; компоненты получают сырые указатели через GetWorld/GetCommon.
class PhysicsSystem
{
public:
    PhysicsSystem();
    ~PhysicsSystem();

    // Шаг симуляции — вызывается после scene.FixedUpdate()
    void Step(float fixedDt);

    rp3d::PhysicsWorld*  GetWorld()  { return world_; }
    rp3d::PhysicsCommon& GetCommon() { return common_; }

private:
    rp3d::PhysicsCommon common_;
    rp3d::PhysicsWorld* world_ = nullptr;
};
