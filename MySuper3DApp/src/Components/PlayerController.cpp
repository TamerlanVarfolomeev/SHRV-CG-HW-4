#include "PlayerController.h"
#include "RigidBody.h"
#include "../Scene/GameObject.h"
#include "../Core/Application.h"
#include <algorithm>
#include <cmath>

using namespace DirectX;

Application* gApp = nullptr;

PlayerController::PlayerController(float moveSpeed, float radius)
    : moveSpeed_(moveSpeed), radius_(radius)
{}

void PlayerController::Update(float)
{
    auto* rb = gameObject->GetComponent<RigidBody>();
    if (!rb || !rb->GetBody()) return;

    auto& playerPos = gameObject->transform.position;

    // --- Мышь: дельта от центра окна + сброс курсора в центр ---
    HWND hWnd = gApp->window_->GetHWND();
    RECT cr;
    GetClientRect(hWnd, &cr);
    int centerX = (cr.right  - cr.left) / 2;
    int centerY = (cr.bottom - cr.top)  / 2;

    // Скрываем курсор при первом вызове
    if (!cursorLocked_)
    {
        ShowCursor(FALSE);
        cursorLocked_ = true;
    }

    POINT cursorClient;
    GetCursorPos(&cursorClient);
    ScreenToClient(hWnd, &cursorClient);

    float mdx = static_cast<float>(cursorClient.x - centerX);
    float mdy = static_cast<float>(cursorClient.y - centerY);

    // Возвращаем курсор в центр окна
    POINT screenCenter = { centerX, centerY };
    ClientToScreen(hWnd, &screenCenter);
    SetCursorPos(screenCenter.x, screenCenter.y);

    // Игнорируем большие скачки (стартовая позиция, возврат после alt-tab)
    if (std::fabs(mdx) < 200.0f && std::fabs(mdy) < 200.0f)
    {
        cameraYaw_   += mdx * mouseSensitivity_;
        cameraPitch_ += mdy * mouseSensitivity_;
        cameraPitch_  = std::clamp(cameraPitch_, -10.0f, 80.0f);
    }

    // --- Колёсико: дистанция камеры ---
    float scroll = Input::GetScrollDelta();
    if (scroll != 0.0f)
    {
        cameraDistance_ -= scroll * 0.5f;
        cameraDistance_  = std::clamp(cameraDistance_, 3.0f, 25.0f);
        Input::SetScrollDelta(0.0f);
    }

    // --- WASD: направление в плоскости XZ, повёрнутое на cameraYaw_ ---
    const float yawR   = XMConvertToRadians(cameraYaw_);
    const float pitchR = XMConvertToRadians(cameraPitch_);

    XMFLOAT3 camForwardXZ = {  sinf(yawR), 0.0f,  cosf(yawR) };
    XMFLOAT3 camRightXZ   = {  cosf(yawR), 0.0f, -sinf(yawR) };

    float fwdInput   = (Input::GetKey(Key::W) ? 1.0f : 0.0f) - (Input::GetKey(Key::S) ? 1.0f : 0.0f);
    float rightInput = (Input::GetKey(Key::D) ? 1.0f : 0.0f) - (Input::GetKey(Key::A) ? 1.0f : 0.0f);

    XMFLOAT3 moveDir = {
        fwdInput * camForwardXZ.x + rightInput * camRightXZ.x,
        0.0f,
        fwdInput * camForwardXZ.z + rightInput * camRightXZ.z
    };

    float moveLen = std::sqrt(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
    if (moveLen > 0.001f)
    {
        moveDir.x /= moveLen;
        moveDir.z /= moveLen;

        // Угловая скорость для качения сферы: ось вращения перпендикулярна направлению движения
        float angularSpeed = moveSpeed_ / radius_;
        rp3d::Vector3 angularVel(
             moveDir.z * angularSpeed,
             0.0f,
            -moveDir.x * angularSpeed
        );
        rb->GetBody()->setAngularVelocity(angularVel);
    }
    // Если WASD не нажаты — оставляем инерцию (катамари продолжает катиться, трение остановит)

    // --- Позиция камеры: орбита вокруг игрока ---
    XMFLOAT3 camOffset = {
        -sinf(yawR) * cosf(pitchR) * cameraDistance_,
                       sinf(pitchR) * cameraDistance_,
        -cosf(yawR) * cosf(pitchR) * cameraDistance_
    };
    XMFLOAT3 camPos = {
        playerPos.x + camOffset.x,
        playerPos.y + camOffset.y,
        playerPos.z + camOffset.z
    };

    // Направление: от камеры к игроку
    XMFLOAT3 fwd = {
        playerPos.x - camPos.x,
        playerPos.y - camPos.y,
        playerPos.z - camPos.z
    };
    float fwdLen = std::sqrt(fwd.x * fwd.x + fwd.y * fwd.y + fwd.z * fwd.z);
    if (fwdLen > 0.001f) { fwd.x /= fwdLen; fwd.y /= fwdLen; fwd.z /= fwdLen; }

    // --- Передаём всё в камеру (это перезаписывает любые изменения Camera::Update) ---
    auto* camera = gApp->scene_->camera.get();
    if (camera)
    {
        camera->mode_     = CameraMode::FPS;
        camera->position_ = camPos;
        camera->forward_  = fwd;
        // Синхронизируем внутреннее состояние Camera, чтобы её Update следующего кадра
        // не сместил позицию (Camera::UpdateFPS читает WASD и двигает fpsPos_)
        camera->fpsPos_   = camPos;
        camera->fpsYaw_   = cameraYaw_;
        camera->fpsPitch_ = cameraPitch_;
    }
}
