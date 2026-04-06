#include "PhysicsSystem.h"

PhysicsSystem::PhysicsSystem()
{
    rp3d::PhysicsWorld::WorldSettings settings;
    settings.gravity = rp3d::Vector3(0.0f, -9.81f, 0.0f);
    world_ = common_.createPhysicsWorld(settings);
}

PhysicsSystem::~PhysicsSystem()
{
    common_.destroyPhysicsWorld(world_);
}

void PhysicsSystem::Step(float fixedDt)
{
    world_->update(fixedDt);
}
