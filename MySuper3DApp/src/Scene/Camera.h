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
    void RebuildProj(float aspect);

    DirectX::XMFLOAT3 position_ = { 0, 0, -3 };
    float yaw_   = 0.0f;
    float pitch_ = 0.0f;

    DirectX::XMFLOAT4X4 proj_ = {};

    // Параметры проекции — нужны для пересчёта при resize
    bool  isOrtho_  = false;
    float fov_      = 45.0f;  // для перспективы: FOV в градусах
    float orthoW_   = 5.0f;   // для ортографии: ширина мира
    float nearZ_    = 0.1f;
    float farZ_     = 1000.0f;

    // Кэшированные векторы направления
    DirectX::XMFLOAT3 forward_ = { 0,0,1 };
    DirectX::XMFLOAT3 right_   = { 1,0,0 };

};
