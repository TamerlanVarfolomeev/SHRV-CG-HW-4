#include "GraphicsDevice.h"
#include <stdexcept>

GraphicsDevice::GraphicsDevice(bool debug)
{
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
    UINT flags = debug ? D3D11_CREATE_DEVICE_DEBUG : 0;

    HRESULT hr = D3D11CreateDevice(
        nullptr,                  // адаптер по умолчанию
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        featureLevels, ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &device_,
        nullptr,                  // полученный feature level нам не нужен
        &context_
    );

    if (FAILED(hr))
        throw std::runtime_error("Failed to create D3D11 device");
}
