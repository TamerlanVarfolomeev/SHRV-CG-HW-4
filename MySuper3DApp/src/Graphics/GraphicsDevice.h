#pragma once
#include <d3d11.h>
#include <wrl.h>

// Единственное место в проекте, где живут ID3D11Device и ID3D11DeviceContext.
// Все остальные классы получают сырые указатели через GetDevice() / GetContext().
class GraphicsDevice
{
public:
    explicit GraphicsDevice(bool debug = true);

    ID3D11Device*        GetDevice()  const { return device_.Get(); }
    ID3D11DeviceContext* GetContext() const { return context_.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D11Device>        device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>  context_;

    friend class SwapChainTarget; // SwapChainTarget нужен device для создания swap chain
};
