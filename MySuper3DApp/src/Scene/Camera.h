#pragma once
#include <DirectXMath.h>

// Camera — отдельная сущность, не компонент.
// Поддерживает fly-cam: WASD + зажатая ПКМ для вращения мышью.
class Camera
{
public:
    static Camera Perspective(float fovDeg, float aspect, float nearZ, float farZ);
    static Camera Orthographic(float fovDeg, float aspect, float nearZ, float farZ);

    void Update(float dt);

    // Матрицы для шейдера
    DirectX::XMMATRIX GetView()     const;
    DirectX::XMMATRIX GetProj()     const { return XMLoadFloat4x4(&proj_); }
    DirectX::XMFLOAT3 GetPosition() const { return position_; }

    void SetAspect(float aspect);

    // Параметры управления
    float moveSpeed  = 3.0f;   // м/с
    float lookSpeed  = 0.2f;   // градус/пиксель

private:
    Camera() = default;
    void UpdateVectors();

    DirectX::XMFLOAT3 position_ = { 0, 0, -3 };
    float yaw_   = 0.0f;   // горизонтальный поворот, градусы
    float pitch_ = 0.0f;   // вертикальный поворот, градусы

    DirectX::XMFLOAT4X4 proj_ = {};

    // Кэшированные векторы направления
    DirectX::XMFLOAT3 forward_ = { 0,0,1 };
    DirectX::XMFLOAT3 right_   = { 1,0,0 };

};
