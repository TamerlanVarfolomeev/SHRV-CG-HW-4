#pragma once
#include "../Scene/Component.h"
#include <reactphysics3d/reactphysics3d.h>

namespace rp3d = reactphysics3d;

class SphereCollider : public Component
{
public:
    explicit SphereCollider(rp3d::PhysicsCommon* common, float radius = 0.5f);
    ~SphereCollider();

    void FixedUpdate(float dt) override;

private:
    void EnsureCreated();

    rp3d::PhysicsCommon* common_;
    float                radius_;
    rp3d::SphereShape*   shape_   = nullptr;
    bool                 created_ = false;
};
