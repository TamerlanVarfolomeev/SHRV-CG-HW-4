#include "Camera.h"
#include "../Core/Input.h"
#include <algorithm>
#include <cmath>

using namespace DirectX;

Camera Camera::Create(float fovDeg, float aspect, float nearZ, float farZ)
{
    Camera cam;
    cam.fov_    = fovDeg;
    cam.aspect_ = aspect;
    cam.nearZ_  = nearZ;
    cam.farZ_   = farZ;
    cam.RebuildProj();
    return cam;
}

void Camera::RebuildProj()
{
    XMStoreFloat4x4(&proj_,
        XMMatrixPerspectiveFovLH(XMConvertToRadians(fov_), aspect_, nearZ_, farZ_));
}

void Camera::SetAspect(float aspect)
{
    aspect_ = aspect;
    RebuildProj();
}

void Camera::Update(float dt)
{
    // Tab — переключение режима
    if (Input::GetKeyDown(Key::Tab))
        mode_ = (mode_ == CameraMode::FPS) ? CameraMode::Orbital : CameraMode::FPS;

    // 1/2/3 — переключение FOV
    if (Input::GetKeyDown(Key::Num1)) { fov_ = 45.0f;  RebuildProj(); }
    if (Input::GetKeyDown(Key::Num2)) { fov_ = 75.0f;  RebuildProj(); }
    if (Input::GetKeyDown(Key::Num3)) { fov_ = 110.0f; RebuildProj(); }

    if (mode_ == CameraMode::FPS)
        UpdateFPS(dt);
    else
        UpdateOrbital(dt);
}

void Camera::UpdateFPS(float dt)
{
    if (Input::GetKey(Key::MouseRight))
    {
        auto delta = Input::GetMouseDelta();
        fpsYaw_   += delta.x * lookSpeed;
        fpsPitch_ += delta.y * lookSpeed;
        fpsPitch_  = std::clamp(fpsPitch_, -89.0f, 89.0f);
    }
    RebuildFPSVectors(); // всегда пересчитываем (корректно при переключении режимов)

    // Движение WASD
    float speed = moveSpeed * dt;
    if (Input::GetKey(Key::W)) { fpsPos_.x += forward_.x * speed; fpsPos_.y += forward_.y * speed; fpsPos_.z += forward_.z * speed; }
    if (Input::GetKey(Key::S)) { fpsPos_.x -= forward_.x * speed; fpsPos_.y -= forward_.y * speed; fpsPos_.z -= forward_.z * speed; }
    if (Input::GetKey(Key::D)) { fpsPos_.x += fpsRight_.x * speed; fpsPos_.y += fpsRight_.y * speed; fpsPos_.z += fpsRight_.z * speed; }
    if (Input::GetKey(Key::A)) { fpsPos_.x -= fpsRight_.x * speed; fpsPos_.y -= fpsRight_.y * speed; fpsPos_.z -= fpsRight_.z * speed; }
    if (Input::GetKey(Key::E)) fpsPos_.y += speed;
    if (Input::GetKey(Key::Q)) fpsPos_.y -= speed;

    position_ = fpsPos_;
}

void Camera::UpdateOrbital(float dt)
{
    // Орбита — ЛКМ + мышь
    if (Input::GetKey(Key::MouseLeft))
    {
        auto delta = Input::GetMouseDelta();
        orbitYaw_   += delta.x * lookSpeed;
        orbitPitch_ += delta.y * lookSpeed;
        orbitPitch_  = std::clamp(orbitPitch_, -89.0f, 89.0f);
    }

    // Зум — колёсико
    float scroll = Input::GetScrollDelta();
    orbitDistance_ -= scroll * 5.0f;
    orbitDistance_  = std::clamp(orbitDistance_, 3.0f, 600.0f);

    // Вычисляем позицию из сферических координат
    float yawR   = XMConvertToRadians(orbitYaw_);
    float pitchR = XMConvertToRadians(orbitPitch_);

    position_.x = cosf(pitchR) * sinf(yawR) * orbitDistance_;
    position_.y = sinf(pitchR) * orbitDistance_;
    position_.z = cosf(pitchR) * cosf(yawR) * orbitDistance_;

    // Направление к центру
    XMVECTOR pos = XMLoadFloat3(&position_);
    XMVECTOR fwd = XMVector3Normalize(XMVectorNegate(pos));
    XMStoreFloat3(&forward_, fwd);
}

void Camera::RebuildFPSVectors()
{
    float yawR   = XMConvertToRadians(fpsYaw_);
    float pitchR = XMConvertToRadians(fpsPitch_);

    forward_ = {
        cosf(pitchR) * sinf(yawR),
       -sinf(pitchR),
        cosf(pitchR) * cosf(yawR)
    };

    XMVECTOR f = XMLoadFloat3(&forward_);
    XMVECTOR r = XMVector3Normalize(XMVector3Cross(XMVectorSet(0, 1, 0, 0), f));
    XMStoreFloat3(&fpsRight_, r);
}

XMMATRIX Camera::GetView() const
{
    XMVECTOR pos    = XMLoadFloat3(&position_);
    XMVECTOR fwd    = XMLoadFloat3(&forward_);
    XMVECTOR target = XMVectorAdd(pos, fwd);
    return XMMatrixLookAtLH(pos, target, XMVectorSet(0, 1, 0, 0));
}
