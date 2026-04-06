#pragma once
#include <DirectXMath.h>

class Transform
{
public:
    DirectX::XMFLOAT3 position = { 0, 0, 0 };
    DirectX::XMFLOAT3 rotation = { 0, 0, 0 }; // Euler, градусы
    DirectX::XMFLOAT3 scale    = { 1, 1, 1 };

    // Возвращает World матрицу (пересчитывается только если dirty)
    DirectX::XMMATRIX GetWorldMatrix() const;

    // Вспомогательные методы
    DirectX::XMFLOAT3 Forward() const;
    DirectX::XMFLOAT3 Right()   const;
    DirectX::XMFLOAT3 Up()      const;

    // Физическое переопределение: RigidBody выставляет полную World-матрицу напрямую,
    // обходя Euler-конвертацию (нужно для произвольного вращения твёрдых тел).
    void SetPhysicsMatrix(const DirectX::XMMATRIX& mat);
    void ClearPhysicsMatrix();

private:
    mutable DirectX::XMFLOAT4X4 worldCache_  = {};
    mutable bool                 dirty_       = true;

    // Отслеживаем предыдущие значения для определения dirty
    mutable DirectX::XMFLOAT3 cachedPos_ = {}, cachedRot_ = {}, cachedScale_ = {};

    // Physics override
    bool                 physicsOverride_ = false;
    DirectX::XMFLOAT4X4 physicsMatrix_   = {};
};
