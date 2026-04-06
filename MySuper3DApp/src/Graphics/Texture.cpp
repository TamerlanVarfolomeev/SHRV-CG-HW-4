#include "Texture.h"
#include <wincodec.h>
#include <vector>
#include <stdexcept>

#pragma comment(lib, "WindowsCodecs.lib")

using namespace Microsoft::WRL;

Texture::Texture(ID3D11Device* device, const std::string& path)
{
    // Конвертируем путь в wide string (достаточно для ASCII/латинских символов)
    std::wstring wpath(path.begin(), path.end());

    // --- WIC: создаём фабрику ---
    ComPtr<IWICImagingFactory> factory;
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory, nullptr,
        CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
    if (FAILED(hr))
        throw std::runtime_error("Texture: не удалось создать WIC фабрику (вызвана CoInitializeEx?)");

    // --- Открываем файл ---
    ComPtr<IWICBitmapDecoder> decoder;
    hr = factory->CreateDecoderFromFilename(
        wpath.c_str(), nullptr,
        GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
    if (FAILED(hr))
        throw std::runtime_error("Texture: не удалось открыть файл: " + path);

    // --- Берём первый фрейм ---
    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr))
        throw std::runtime_error("Texture: GetFrame провалился: " + path);

    // --- Конвертируем в RGBA8 ---
    ComPtr<IWICFormatConverter> converter;
    factory->CreateFormatConverter(&converter);
    hr = converter->Initialize(
        frame.Get(),
        GUID_WICPixelFormat32bppRGBA,
        WICBitmapDitherTypeNone, nullptr,
        0.0, WICBitmapPaletteTypeCustom);
    if (FAILED(hr))
        throw std::runtime_error("Texture: конвертация формата провалилась: " + path);

    UINT width = 0, height = 0;
    converter->GetSize(&width, &height);

    // --- Копируем пиксели ---
    const UINT stride  = width * 4;
    const UINT bufSize = stride * height;
    std::vector<BYTE> pixels(bufSize);
    hr = converter->CopyPixels(nullptr, stride, bufSize, pixels.data());
    if (FAILED(hr))
        throw std::runtime_error("Texture: CopyPixels провалился: " + path);

    // --- Создаём D3D11 Texture2D (иммутабельная, 1 mip) ---
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width            = width;
    desc.Height           = height;
    desc.MipLevels        = 1;
    desc.ArraySize        = 1;
    desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage            = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem     = pixels.data();
    initData.SysMemPitch = stride;

    ComPtr<ID3D11Texture2D> tex;
    hr = device->CreateTexture2D(&desc, &initData, &tex);
    if (FAILED(hr))
        throw std::runtime_error("Texture: CreateTexture2D провалился: " + path);

    // --- SRV ---
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                    = desc.Format;
    srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels       = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    hr = device->CreateShaderResourceView(tex.Get(), &srvDesc, &srv_);
    if (FAILED(hr))
        throw std::runtime_error("Texture: CreateShaderResourceView провалился: " + path);
}

void Texture::Bind(ID3D11DeviceContext* ctx, UINT slot) const
{
    ctx->PSSetShaderResources(slot, 1, srv_.GetAddressOf());
}

void Texture::Unbind(ID3D11DeviceContext* ctx, UINT slot) const
{
    ID3D11ShaderResourceView* null = nullptr;
    ctx->PSSetShaderResources(slot, 1, &null);
}
