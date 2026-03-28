#include "Shader.h"
#include <stdexcept>
#include <iostream>

Shader::Shader(ID3D11Device* device,
               const std::wstring& path,
               const std::vector<InputElement>& layout,
               const char* vsEntry,
               const char* psEntry)
{
    // Компилируем VS
    auto vsBlob = CompileFromFile(path, vsEntry, "vs_5_0");
    HRESULT hr = device->CreateVertexShader(
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vs_);
    if (FAILED(hr)) throw std::runtime_error("Failed to create vertex shader");

    // Компилируем PS
    auto psBlob = CompileFromFile(path, psEntry, "ps_5_0");
    hr = device->CreatePixelShader(
        psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &ps_);
    if (FAILED(hr)) throw std::runtime_error("Failed to create pixel shader");

    // Строим InputLayout из переданного описания
    std::vector<D3D11_INPUT_ELEMENT_DESC> descs;
    descs.reserve(layout.size());
    for (const auto& elem : layout)
    {
        D3D11_INPUT_ELEMENT_DESC d = {};
        d.SemanticName         = elem.semanticName;
        d.SemanticIndex        = 0;
        d.Format               = elem.format;
        d.InputSlot            = 0;
        d.AlignedByteOffset    = elem.alignedByteOffset;
        d.InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
        d.InstanceDataStepRate = 0;
        descs.push_back(d);
    }

    hr = device->CreateInputLayout(
        descs.data(), static_cast<UINT>(descs.size()),
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        &layout_);
    if (FAILED(hr)) throw std::runtime_error("Failed to create input layout");
}

void Shader::Bind(ID3D11DeviceContext* ctx) const
{
    ctx->IASetInputLayout(layout_.Get());
    ctx->VSSetShader(vs_.Get(), nullptr, 0);
    ctx->PSSetShader(ps_.Get(), nullptr, 0);
}

Microsoft::WRL::ComPtr<ID3DBlob> Shader::CompileFromFile(
    const std::wstring& path, const char* entry, const char* target)
{
    Microsoft::WRL::ComPtr<ID3DBlob> code, errors;

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    HRESULT hr = D3DCompileFromFile(
        path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entry, target, flags, 0, &code, &errors);

    if (FAILED(hr))
    {
        if (errors)
            std::cerr << "[Shader] " << (char*)errors->GetBufferPointer() << "\n";
        throw std::runtime_error("Shader compilation failed: " +
            std::string(entry) + " in " + std::string(path.begin(), path.end()));
    }
    return code;
}
