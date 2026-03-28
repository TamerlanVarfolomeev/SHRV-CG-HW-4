#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include <DirectXMath.h>

// Стандартная вершина движка.
// Совпадает с VertexInput в Common.hlsli.
struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 uv;
};

class Mesh
{
public:
    Mesh(ID3D11Device* device,
         const std::vector<Vertex>& vertices,
         const std::vector<uint32_t>& indices);

    void Draw(ID3D11DeviceContext* ctx) const;

    uint32_t GetIndexCount() const { return indexCount_; }

    // --- Фабричные методы для стандартных примитивов ---
    static Mesh CreateQuad(ID3D11Device* device);
    static Mesh CreateCube(ID3D11Device* device);

private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> vb_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> ib_;
    uint32_t indexCount_ = 0;
};
