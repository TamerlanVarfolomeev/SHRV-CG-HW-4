#include "RigidBody.h"
#include "../Scene/GameObject.h"
#include <DirectXMath.h>

using namespace DirectX;

RigidBody::RigidBody(rp3d::PhysicsWorld*  world,
                     rp3d::PhysicsCommon* common,
                     PhysicsBodyType      type)
    : world_(world), common_(common), type_(type)
{}

RigidBody::~RigidBody()
{
    if (body_)
        world_->destroyRigidBody(body_); // автоматически удаляет все Collider на теле
}

rp3d::RigidBody* RigidBody::GetBody()
{
    EnsureCreated();
    return body_;
}

void RigidBody::EnsureCreated()
{
    if (created_) return;
    created_ = true;

    auto& t = gameObject->transform;

    // Euler (degrees) → кватернион DirectX → кватернион RP3D
    XMVECTOR q = XMQuaternionRotationRollPitchYaw(
        XMConvertToRadians(t.rotation.x),
        XMConvertToRadians(t.rotation.y),
        XMConvertToRadians(t.rotation.z));
    XMFLOAT4 qf;
    XMStoreFloat4(&qf, q);

    rp3d::Transform rpt(
        rp3d::Vector3(t.position.x, t.position.y, t.position.z),
        rp3d::Quaternion(qf.x, qf.y, qf.z, qf.w));

    body_ = world_->createRigidBody(rpt);

    rp3d::BodyType rp3dType;
    switch (type_)
    {
        case PhysicsBodyType::Static:    rp3dType = rp3d::BodyType::STATIC;    break;
        case PhysicsBodyType::Kinematic: rp3dType = rp3d::BodyType::KINEMATIC; break;
        default:                         rp3dType = rp3d::BodyType::DYNAMIC;   break;
    }
    body_->setType(rp3dType);
    body_->setMass(mass);
    body_->setLinearDamping(linearDamping);
    body_->setAngularDamping(angularDamping);
}

void RigidBody::FixedUpdate(float)
{
    EnsureCreated();

    if (type_ != PhysicsBodyType::Dynamic) return;

    // Синхронизируем физическую трансформацию → Transform (Physics Override)
    const rp3d::Transform& rpt = body_->getTransform();
    const rp3d::Vector3&   pos = rpt.getPosition();
    const rp3d::Quaternion& q  = rpt.getOrientation();

    auto& t = gameObject->transform;
    t.position = { pos.x, pos.y, pos.z }; // нужно для camera follow, спавна и т.п.

    // RP3D — правосторонняя система, DirectX — левосторонняя.
    // Сопряжённый кватернион инвертирует направление вращения, согласуя системы.
    XMVECTOR dxQ = XMVectorSet(-q.x, q.y, -q.z, q.w);
    XMMATRIX R   = XMMatrixRotationQuaternion(dxQ);
    XMMATRIX T   = XMMatrixTranslation(pos.x, pos.y, pos.z);
    XMMATRIX S   = XMMatrixScaling(t.scale.x, t.scale.y, t.scale.z);

    t.SetPhysicsMatrix(S * R * T);
}
