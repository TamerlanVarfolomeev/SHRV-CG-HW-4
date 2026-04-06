#include "PhysicsSystem.h"

PhysicsSystem::PhysicsSystem()
{
    rp3d::PhysicsWorld::WorldSettings settings;
    settings.gravity = rp3d::Vector3(0.0f, -9.81f, 0.0f);
    settings.defaultFrictionCoefficient = 0.9f; // высокое трение — сфера катится, а не скользит
    world_ = common_.createPhysicsWorld(settings);
}

PhysicsSystem::~PhysicsSystem()
{
    common_.destroyPhysicsWorld(world_);
}

void PhysicsSystem::SetContactListener(rp3d::EventListener* listener)
{
    world_->setEventListener(listener);
}

void PhysicsSystem::Step(float fixedDt)
{
    world_->update(fixedDt);
}
