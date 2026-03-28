#include "Camera.h"
#include "../Core/Input.h"
#include <algorithm>

using namespace DirectX;

Camera::Camera(float fovDeg, float aspect, float nearZ, float farZ)
{
    XMStoreFloat4x4(&proj_,
        XMMatrixPerspectiveFovLH(
            XMConvertToRadians(fovDeg), aspect, nearZ, farZ));
    UpdateVectors();
}

void Camera::SetAspect(float aspect)
{
    float fov   = 2.0f * atanf(1.0f / proj_.m[1][1]);
    float nearZ = proj_.m[3][2] / (proj_.m[2][2] - 1.0f);
    float farZ  = proj_.m[3][2] / (proj_.m[2][2]);
    XMStoreFloat4x4(&proj_,
        XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ));
}

void Camera::Update(float dt)
{
    // --- Движение WASD ---
    float speed = moveSpeed * dt;

    if (Input::GetKey(Key::W)) { position_.x += forward_.x * speed; position_.y += forward_.y * speed; position_.z += forward_.z * speed; }
    if (Input::GetKey(Key::S)) { position_.x -= forward_.x * speed; position_.y -= forward_.y * speed; position_.z -= forward_.z * speed; }
    if (Input::GetKey(Key::D)) { position_.x += right_.x * speed;   position_.y += right_.y * speed;   position_.z += right_.z * speed; }
    if (Input::GetKey(Key::A)) { position_.x -= right_.x * speed;   position_.y -= right_.y * speed;   position_.z -= right_.z * speed; }
    if (Input::GetKey(Key::E)) position_.y += speed;
    if (Input::GetKey(Key::Q)) position_.y -= speed;

    // --- Вращение мышью (зажатая ПКМ) ---
    if (Input::GetKey(Key::MouseRight))
    {
        auto delta = Input::GetMouseDelta();
        if (delta.x != 0 || delta.y != 0)
        {
            yaw_   += delta.x * lookSpeed;
            pitch_ += delta.y * lookSpeed;
            pitch_  = std::clamp(pitch_, -89.0f, 89.0f);
            UpdateVectors();
        }
    }
}

XMMATRIX Camera::GetView() const
{
    XMVECTOR pos    = XMLoadFloat3(&position_);
    XMVECTOR fwd    = XMLoadFloat3(&forward_);
    XMVECTOR target = XMVectorAdd(pos, fwd);
    return XMMatrixLookAtLH(pos, target, XMVectorSet(0, 1, 0, 0));
}

void Camera::UpdateVectors()
{
    float yawR   = XMConvertToRadians(yaw_);
    float pitchR = XMConvertToRadians(pitch_);

    forward_ = {
        cosf(pitchR) * sinf(yawR),
       -sinf(pitchR),
        cosf(pitchR) * cosf(yawR)
    };

    XMVECTOR f = XMLoadFloat3(&forward_);
    XMVECTOR r = XMVector3Normalize(XMVector3Cross(XMVectorSet(0, 1, 0, 0), f));
    XMStoreFloat3(&right_, r);
}
