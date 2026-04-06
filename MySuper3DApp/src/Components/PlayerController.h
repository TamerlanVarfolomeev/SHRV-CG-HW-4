#pragma once
#include "../Scene/Component.h"
#include "../Core/Input.h"
#include <DirectXMath.h>

// Глобальный доступ к Application из компонентов (аналог FindObjectOfType)
class Application;
extern Application* gApp;

class PlayerController : public Component
{
public:
    explicit PlayerController(float moveSpeed = 15.0f, float radius = 0.5f);
    ~PlayerController() = default;

    void Update(float dt) override;

    // Камера следует за игроком — вызывается из Application::OnFixedUpdate
    void FollowCamera();

private:
    float moveSpeed_      = 15.0f;
    float radius_          = 0.5f;
    float cameraDistance_ = 4.0f;
    float cameraPitch_    = 30.0f; // угол камеры сверху-назад, градусов
};
