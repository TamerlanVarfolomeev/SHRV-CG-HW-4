#include "PlayerController.h"
#include "RigidBody.h"
#include "SphereCollider.h"
#include "../Scene/GameObject.h"
#include "../Core/Application.h"
#include "../Core/Window.h"
#include <DirectXMath.h>
#include <cmath>

Application* gApp = nullptr;

PlayerController::PlayerController(float moveSpeed, float radius)
    : moveSpeed_(moveSpeed), radius_(radius)
{}

void PlayerController::Update(float)
{
    auto* rb = gameObject->GetComponent<RigidBody>();
    if (!rb || !rb->GetBody()) return;

    auto& playerPos = gameObject->transform.position;

    // --- Управление камерой: колёсико ---
    float scroll = Input::GetScrollDelta();
    if (scroll != 0.0f)
    {
        cameraDistance_ -= scroll * 0.5f;
        if (cameraDistance_ < 3.0f)  cameraDistance_ = 3.0f;
        if (cameraDistance_ > 25.0f) cameraDistance_ = 25.0f;
        Input::SetScrollDelta(0.0f);
    }

    // --- Определяем направление к курсору мыши ---
    POINT cursorPos;
    if (!GetCursorPos(&cursorPos))
        return;

    // Преобразуем в клиентские координаты
    if (!ScreenToClient(gApp->window_->GetHWND(), &cursorPos))
        return;

    // Нормализуем в [-1, 1]
    int w = gApp->width_;
    int h = gApp->height_;
    float ndcX = (2.0f * cursorPos.x / w) - 1.0f;
    float ndcY = 1.0f - (2.0f * cursorPos.y / h);

    // Получаем матрицу View * Projection от камеры
    auto* camera = gApp->scene_->camera.get();
    auto camView = camera->GetView();
    auto camProj = camera->GetProj();
    auto camVP = DirectX::XMMatrixMultiply(camView, camProj);
    auto invVP = DirectX::XMMatrixInverse(nullptr, camVP);

    // Unproject: точка на near и far плоскостях
    DirectX::XMFLOAT4 nearPt(ndcX, ndcY, 0.0f, 1.0f);
    DirectX::XMFLOAT4 farPt(ndcX, ndcY, 1.0f, 1.0f);

    DirectX::XMVECTOR nearV = DirectX::XMLoadFloat4(&nearPt);
    DirectX::XMVECTOR farV  = DirectX::XMLoadFloat4(&farPt);

    nearV = DirectX::XMVector4Transform(nearV, invVP);
    farV  = DirectX::XMVector4Transform(farV, invVP);

    // Делим на W (XMVECTOR не поддерживает /=)
    float nearW = DirectX::XMVectorGetW(nearV);
    float farW  = DirectX::XMVectorGetW(farV);
    DirectX::XMVECTOR nearWVec = DirectX::XMVectorReplicate(nearW);
    DirectX::XMVECTOR farWVec  = DirectX::XMVectorReplicate(farW);
    DirectX::XMVECTOR nearWRecip = DirectX::XMVectorReciprocal(nearWVec);
    DirectX::XMVECTOR farWRecip  = DirectX::XMVectorReciprocal(farWVec);
    nearV = DirectX::XMVectorMultiply(nearV, nearWRecip);
    farV  = DirectX::XMVectorMultiply(farV, farWRecip);

    DirectX::XMFLOAT3 nearWorld, farWorld;
    DirectX::XMStoreFloat3(&nearWorld, nearV);
    DirectX::XMStoreFloat3(&farWorld, farV);

    // Направление луча
    DirectX::XMFLOAT3 rayDir;
    rayDir.x = farWorld.x - nearWorld.x;
    rayDir.y = farWorld.y - nearWorld.y;
    rayDir.z = farWorld.z - nearWorld.z;

    // Пересекаем луч с плоскостью Y = 0 (земля)
    if (std::abs(rayDir.y) < 1e-5f)
        return; // луч параллелен земле

    float t = (0.0f - nearWorld.y) / rayDir.y;
    if (t < 0.0f)
        return; // точка за камерой

    float hitX = nearWorld.x + rayDir.x * t;
    float hitZ = nearWorld.z + rayDir.z * t;

    // Направление от игрока к точке курсора
    float dx = hitX - playerPos.x;
    float dz = hitZ - playerPos.z;
    float dist = std::sqrt(dx * dx + dz * dz);

    if (dist < 0.1f)
        return; // уже на месте

    dx /= dist;
    dz /= dist;

    // --- Вращаем сферу ---
    float angularSpeed = moveSpeed_ / radius_;
    rp3d::Vector3 angularVel(dz * angularSpeed, 0.0f, -dx * angularSpeed);
    rb->GetBody()->setAngularVelocity(angularVel);

    // --- Линейная скорость ---
    rp3d::Vector3 currentVel = rb->GetBody()->getLinearVelocity();
    rp3d::Vector3 linearVel(dx * moveSpeed_, currentVel.y, dz * moveSpeed_);
    rb->GetBody()->setLinearVelocity(linearVel);
}

void PlayerController::FollowCamera()
{
    auto* camera = gApp->scene_->camera.get();
    if (!camera) return;

    auto& pos = gameObject->transform.position;

    // Позиция камеры: сзади-сверху от игрока
    float pitchRad = DirectX::XMConvertToRadians(cameraPitch_);
    float offsetY  = cameraDistance_ * std::sin(pitchRad);
    float offsetH  = cameraDistance_ * std::cos(pitchRad);

    DirectX::XMFLOAT3 camPos;
    camPos.x = pos.x;
    camPos.y = pos.y + offsetY;
    camPos.z = pos.z - offsetH/4;

    DirectX::XMFLOAT3 fwd;
    fwd.x = 0.0f;
    fwd.y = -std::sin(pitchRad);
    fwd.z = std::cos(pitchRad);

    camera->mode_ = CameraMode::FPS;
    camera->fpsPos_   = camPos;
    camera->fpsYaw_   = 0.0f;
    camera->fpsPitch_ = 70;

    camera->position_ = camPos;
    camera->forward_   = fwd;
}
