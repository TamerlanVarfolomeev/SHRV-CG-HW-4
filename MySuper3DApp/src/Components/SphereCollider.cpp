#include "SphereCollider.h"
#include "RigidBody.h"
#include "../Scene/GameObject.h"

SphereCollider::SphereCollider(rp3d::PhysicsCommon* common, float radius)
    : common_(common), radius_(radius)
{}

SphereCollider::~SphereCollider()
{
    if (shape_) common_->destroySphereShape(shape_);
}

void SphereCollider::FixedUpdate(float)
{
    EnsureCreated();
}

void SphereCollider::EnsureCreated()
{
    if (created_) return;
    created_ = true;

    auto* rb = gameObject->GetComponent<RigidBody>();
    if (!rb) return;

    shape_ = common_->createSphereShape(radius_);
    rb->GetBody()->addCollider(shape_, rp3d::Transform::identity());
}
