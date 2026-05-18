#pragma once
#include "../Scene/Component.h"
#include "../Core/Input.h"
#include <DirectXMath.h>

// Глобальный доступ к Application из компонентов (аналог FindObjectOfType)
class Application;
extern Application* gApp;

// Стандартное 3rd-person управление:
//   • WASD — движение игрока в плоскости XZ относительно направления камеры
//   • Мышь — вращение камеры вокруг игрока (yaw / pitch)
//   • Колёсико — приближение/отдаление камеры
class PlayerController : public Component
{
public:
    explicit PlayerController(float moveSpeed = 7.0f, float radius = 0.5f);
    ~PlayerController() = default;

    void Update(float dt) override;

    // Обновляет радиус при росте сферы (влияет на угловую скорость качения)
    void SetRadius(float r) { radius_ = r; }

private:
    float moveSpeed_        = 7.0f;
    float radius_           = 0.5f;
    float cameraDistance_   = 8.0f;
    float cameraYaw_        = 0.0f;   // градусы (0 = смотрим в +Z)
    float cameraPitch_      = 25.0f;  // градусы (положительное = камера выше игрока)
    float mouseSensitivity_ = 0.2f;   // градусов / пиксель
    bool  cursorLocked_     = false;  // флаг первого вызова Update (для скрытия курсора)
};
