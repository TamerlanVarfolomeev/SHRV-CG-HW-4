#include "BoxCollider.h"
#include "RigidBody.h"
#include "../Scene/GameObject.h"

BoxCollider::BoxCollider(rp3d::PhysicsCommon* common, rp3d::Vector3 halfExtents)
    : common_(common), halfExtents_(halfExtents)
{}

BoxCollider::~BoxCollider()
{
    if (shape_) common_->destroyBoxShape(shape_);
    // Collider удаляется автоматически вместе с телом (RigidBody::~RigidBody)
}

void BoxCollider::FixedUpdate(float)
{
    EnsureCreated();
}

void BoxCollider::EnsureCreated()
{
    if (created_) return;
    created_ = true;

    auto* rb = gameObject->GetComponent<RigidBody>();
    if (!rb) return;

    shape_ = common_->createBoxShape(halfExtents_);
    rb->GetBody()->addCollider(shape_, rp3d::Transform::identity());
}
