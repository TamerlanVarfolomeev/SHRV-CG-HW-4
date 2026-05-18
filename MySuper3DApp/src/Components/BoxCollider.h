#pragma once
#include "../Scene/Component.h"
#include <reactphysics3d/reactphysics3d.h>

namespace rp3d = reactphysics3d;

// Добавляет BoxShape к RigidBody на том же GameObject.
// RigidBody должен быть добавлен раньше BoxCollider.
class BoxCollider : public Component
{
public:
    // halfExtents — полуразмеры (половина ширины/высоты/глубины) по каждой оси
    BoxCollider(rp3d::PhysicsCommon* common,
                rp3d::Vector3 halfExtents = { 0.5f, 0.5f, 0.5f });
    ~BoxCollider();

    void FixedUpdate(float dt) override;

    // Коэффициент трения [0..1]; применяется при создании коллайдера
    float friction = 0.9f;

private:
    void EnsureCreated();

    rp3d::PhysicsCommon* common_;
    rp3d::Vector3        halfExtents_;
    rp3d::BoxShape*      shape_    = nullptr;
    rp3d::Collider*      collider_ = nullptr;
    bool                 created_  = false;
};
