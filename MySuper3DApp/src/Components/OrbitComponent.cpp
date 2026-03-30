#include "OrbitComponent.h"
#include "../Scene/GameObject.h"
#include <cmath>

void OrbitComponent::Update(float dt)
{
    orbitAngle += orbitSpeed * dt;

    float px = parent ? parent->position.x : 0.0f;
    float py = parent ? parent->position.y : 0.0f;
    float pz = parent ? parent->position.z : 0.0f;

    gameObject->transform.position.x = px + cosf(orbitAngle) * orbitRadius;
    gameObject->transform.position.y = py;
    gameObject->transform.position.z = pz + sinf(orbitAngle) * orbitRadius;

    gameObject->transform.rotation.y += selfRotateSpeed * dt;
}
