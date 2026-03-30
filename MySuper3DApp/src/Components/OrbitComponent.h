#pragma once
#include "../Scene/Component.h"
#include "../Scene/Transform.h"

class OrbitComponent : public Component
{
public:
    Transform* parent     = nullptr; // nullptr = орбита вокруг начала координат
    float orbitRadius     = 5.0f;
    float orbitSpeed      = 1.0f;   // рад/с
    float selfRotateSpeed = 30.0f;  // градус/с вокруг оси Y
    float orbitAngle      = 0.0f;   // начальный угол (рад)

    void Update(float dt) override;
};
