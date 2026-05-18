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

    // Пересоздаёт коллайдер с новым радиусом (для роста сферы игрока)
    void SetRadius(float newRadius);

    // Коэффициент трения [0..1]; применяется при создании коллайдера
    float friction = 0.9f;

private:
    void EnsureCreated();

    rp3d::PhysicsCommon* common_;
    float                radius_;
    rp3d::SphereShape*   shape_    = nullptr;
    rp3d::Collider*      collider_ = nullptr;
    bool                 created_  = false;
};
