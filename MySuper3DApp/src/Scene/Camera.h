#pragma once
#include <DirectXMath.h>

enum class CameraMode { FPS, Orbital };

class Camera
{
public:
    static Camera Create(float fovDeg, float aspect, float nearZ, float farZ);

    void Update(float dt);
    void SetAspect(float aspect);

    // nullptr = свободная орбита вокруг начала координат
    void SetOrbitCenter(const DirectX::XMFLOAT3* center) { orbitCenter_ = center; }

    void SetOrbitDistance(float d) { orbitDistance_ = d; }

    DirectX::XMMATRIX GetView()     const;
    DirectX::XMMATRIX GetProj()     const { return DirectX::XMLoadFloat4x4(&proj_); }
    DirectX::XMFLOAT3 GetPosition() const { return position_; }

    float moveSpeed = 8.0f;
    float lookSpeed = 0.2f;

private:
    Camera() = default;

    void RebuildProj();
    void UpdateFPS(float dt);
    void UpdateOrbital(float dt);
    void RebuildFPSVectors();

    // Projection
    float fov_    = 75.0f;
    float aspect_ = 1.0f;
    float nearZ_  = 0.1f;
    float farZ_   = 2000.0f;
    DirectX::XMFLOAT4X4 proj_ = {};

    CameraMode mode_ = CameraMode::Orbital;

    // Computed each frame — used by GetView / GetPosition
    DirectX::XMFLOAT3 position_ = {};
    DirectX::XMFLOAT3 forward_  = { 0, 0, 1 };

    // FPS state
    DirectX::XMFLOAT3 fpsPos_    = { 0, 8, -50 };
    float              fpsYaw_   = 0.0f;
    float              fpsPitch_ = 0.0f;
    DirectX::XMFLOAT3 fpsRight_ = { 1, 0, 0 };

    // Orbital state
    float orbitYaw_      = 0.0f;
    float orbitPitch_    = 25.0f;
    float orbitDistance_ = 55.0f;

    const DirectX::XMFLOAT3* orbitCenter_ = nullptr;
};
