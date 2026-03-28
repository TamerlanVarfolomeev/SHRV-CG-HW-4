#include "Transform.h"

using namespace DirectX;

static bool Float3Equal(const XMFLOAT3& a, const XMFLOAT3& b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

XMMATRIX Transform::GetWorldMatrix() const
{
    // Пересчитываем только если что-то изменилось
    dirty_ = !Float3Equal(position, cachedPos_)  ||
             !Float3Equal(rotation, cachedRot_)  ||
             !Float3Equal(scale,    cachedScale_);

    if (dirty_)
    {
        cachedPos_   = position;
        cachedRot_   = rotation;
        cachedScale_ = scale;

        XMMATRIX S = XMMatrixScaling(scale.x, scale.y, scale.z);
        XMMATRIX R = XMMatrixRotationRollPitchYaw(
            XMConvertToRadians(rotation.x),
            XMConvertToRadians(rotation.y),
            XMConvertToRadians(rotation.z));
        XMMATRIX T = XMMatrixTranslation(position.x, position.y, position.z);

        // Порядок: сначала масштаб, потом поворот, потом позиция
        XMStoreFloat4x4(&worldCache_, S * R * T);
        dirty_ = false;
    }

    return XMLoadFloat4x4(&worldCache_);
}

XMFLOAT3 Transform::Forward() const
{
    XMMATRIX R = XMMatrixRotationRollPitchYaw(
        XMConvertToRadians(rotation.x),
        XMConvertToRadians(rotation.y),
        XMConvertToRadians(rotation.z));

    XMFLOAT3 f;
    XMStoreFloat3(&f, XMVector3TransformNormal(XMVectorSet(0,0,1,0), R));
    return f;
}

XMFLOAT3 Transform::Right() const
{
    XMMATRIX R = XMMatrixRotationRollPitchYaw(
        XMConvertToRadians(rotation.x),
        XMConvertToRadians(rotation.y),
        XMConvertToRadians(rotation.z));

    XMFLOAT3 r;
    XMStoreFloat3(&r, XMVector3TransformNormal(XMVectorSet(1,0,0,0), R));
    return r;
}

XMFLOAT3 Transform::Up() const
{
    XMMATRIX R = XMMatrixRotationRollPitchYaw(
        XMConvertToRadians(rotation.x),
        XMConvertToRadians(rotation.y),
        XMConvertToRadians(rotation.z));

    XMFLOAT3 u;
    XMStoreFloat3(&u, XMVector3TransformNormal(XMVectorSet(0,1,0,0), R));
    return u;
}
