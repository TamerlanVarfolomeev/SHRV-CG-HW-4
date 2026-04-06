#pragma once
#include "../Scene/Component.h"
#include <reactphysics3d/reactphysics3d.h>

namespace rp3d = reactphysics3d;

enum class PhysicsBodyType { Dynamic, Static, Kinematic };

// Компонент физического тела.
// Создаёт rp3d::RigidBody при первом FixedUpdate (когда уже известен gameObject->transform).
// Для Dynamic-тел синхронизирует физическую трансформацию → Transform каждый шаг.
class RigidBody : public Component
{
public:
    RigidBody(rp3d::PhysicsWorld*  world,
              rp3d::PhysicsCommon* common,
              PhysicsBodyType      type = PhysicsBodyType::Dynamic);
    ~RigidBody();

    void FixedUpdate(float fixedDt) override;

    // Для коллайдеров — гарантируют, что тело создано перед добавлением шейпов
    rp3d::RigidBody* GetBody();

    // Выключает/включает участие в симуляции.
    // При выключении тело помечается неактивным и перестаёт обновлять Transform.
    void SetSimulated(bool simulate);

    float mass           = 1.0f;
    float linearDamping  = 0.05f;
    float angularDamping = 0.05f;

private:
    void EnsureCreated();

    rp3d::PhysicsWorld*  world_;
    rp3d::PhysicsCommon* common_;
    rp3d::RigidBody*     body_      = nullptr;
    PhysicsBodyType      type_;
    bool                 created_   = false;
    bool                 simulated_ = true;
};
