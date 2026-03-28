#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <stdexcept>

// Шаблонный враппер над ID3D11Buffer для constant buffers.
// Использование:
//   ConstantBuffer<CBPerFrame> cb(device);
//   cb.Update(context, data);
//   cb.Bind(context, ShaderStage::VS, 0);

enum class ShaderStage { VS, PS, Both };

template<typename T>
class ConstantBuffer
{
    static_assert(sizeof(T) % 16 == 0,
        "Constant buffer struct must be a multiple of 16 bytes");
public:
    ConstantBuffer() = default; // неинициализированное состояние, buffer_ == nullptr

    explicit ConstantBuffer(ID3D11Device* device)
    {
        D3D11_BUFFER_DESC bd = {};
        bd.ByteWidth      = sizeof(T);
        bd.Usage          = D3D11_USAGE_DYNAMIC;
        bd.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        if (FAILED(device->CreateBuffer(&bd, nullptr, &buffer_)))
            throw std::runtime_error("Failed to create constant buffer");
    }

    // Загрузить данные на GPU
    void Update(ID3D11DeviceContext* ctx, const T& data)
    {
        D3D11_MAPPED_SUBRESOURCE ms;
        ctx->Map(buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
        memcpy(ms.pData, &data, sizeof(T));
        ctx->Unmap(buffer_.Get(), 0);
    }

    // Привязать к указанному слоту
    void Bind(ID3D11DeviceContext* ctx, ShaderStage stage, UINT slot) const
    {
        ID3D11Buffer* buf = buffer_.Get();
        if (stage == ShaderStage::VS || stage == ShaderStage::Both)
            ctx->VSSetConstantBuffers(slot, 1, &buf);
        if (stage == ShaderStage::PS || stage == ShaderStage::Both)
            ctx->PSSetConstantBuffers(slot, 1, &buf);
    }

    ID3D11Buffer* Get() const { return buffer_.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> buffer_;
};
