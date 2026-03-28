#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include <string>
#include <vector>

// Описание одного атрибута вершины для InputLayout
struct InputElement
{
    const char* semanticName;
    DXGI_FORMAT format;
    UINT        alignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
};

class Shader
{
public:
    // Компилирует VS и PS из одного .hlsl файла.
    // vsEntry / psEntry — имена функций точек входа.
    Shader(ID3D11Device* device,
           const std::wstring& path,
           const std::vector<InputElement>& layout,
           const char* vsEntry = "VSMain",
           const char* psEntry = "PSMain");

    void Bind(ID3D11DeviceContext* ctx) const;

private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vs_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  ps_;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  layout_;

    static Microsoft::WRL::ComPtr<ID3DBlob> CompileFromFile(
        const std::wstring& path,
        const char* entry,
        const char* target);
};
