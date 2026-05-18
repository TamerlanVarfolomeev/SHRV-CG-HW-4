#include "CubeTexture.h"
#include <wincodec.h>
#include <cstdint>
#include <stdexcept>
#include <string>

#pragma comment(lib, "WindowsCodecs.lib")

using namespace Microsoft::WRL;

// Загружает изображение через WIC, конвертирует в RGBA8, возвращает буфер пикселей.
static std::vector<BYTE> LoadImagePixels(const std::wstring&  path,
                                         IWICImagingFactory*  factory,
                                         UINT*                outWidth,
                                         UINT*                outHeight,
                                         const std::string&   debugPath)
{
    ComPtr<IWICBitmapDecoder> decoder;
    HRESULT hr = factory->CreateDecoderFromFilename(
        path.c_str(), nullptr,
        GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
    if (FAILED(hr))
        throw std::runtime_error("CubeTexture: не удалось открыть файл: " + debugPath);

    ComPtr<IWICBitmapFrameDecode> frame;
    if (FAILED(decoder->GetFrame(0, &frame)))
        throw std::runtime_error("CubeTexture: GetFrame провалился: " + debugPath);

    ComPtr<IWICFormatConverter> conv;
    factory->CreateFormatConverter(&conv);
    if (FAILED(conv->Initialize(frame.Get(), GUID_WICPixelFormat32bppRGBA,
        WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom)))
        throw std::runtime_error("CubeTexture: конвертация формата провалилась: " + debugPath);

    UINT w = 0, h = 0;
    conv->GetSize(&w, &h);
    *outWidth  = w;
    *outHeight = h;

    const UINT stride  = w * 4;
    const UINT bufSize = stride * h;
    std::vector<BYTE> pixels(bufSize);
    if (FAILED(conv->CopyPixels(nullptr, stride, bufSize, pixels.data())))
        throw std::runtime_error("CubeTexture: CopyPixels провалился: " + debugPath);

    return pixels;
}

// Вырезает прямоугольник faceW×faceH из общего буфера, начиная с (srcX, srcY).
static std::vector<BYTE> ExtractRect(const std::vector<BYTE>& src,
                                     UINT srcStride,
                                     UINT srcX, UINT srcY,
                                     UINT faceW, UINT faceH)
{
    const UINT dstStride = faceW * 4;
    std::vector<BYTE> dst(faceH * dstStride);
    for (UINT y = 0; y < faceH; y++)
    {
        const BYTE* srcRow = src.data() + (srcY + y) * srcStride + srcX * 4;
        BYTE*       dstRow = dst.data() + y * dstStride;
        memcpy(dstRow, srcRow, dstStride);
    }
    return dst;
}

// ---------------------------------------------------------------------------
// Конструктор из 6 отдельных файлов
// ---------------------------------------------------------------------------
CubeTexture::CubeTexture(ID3D11Device*                       device,
                         const std::array<std::string, 6>&   facePaths)
{
    ComPtr<IWICImagingFactory> factory;
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory, nullptr,
        CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
    if (FAILED(hr))
        throw std::runtime_error("CubeTexture: не удалось создать WIC фабрику");

    std::array<std::vector<BYTE>, 6> faces;
    UINT width = 0, height = 0;
    for (int i = 0; i < 6; i++)
    {
        std::wstring wpath(facePaths[i].begin(), facePaths[i].end());
        UINT w = 0, h = 0;
        faces[i] = LoadImagePixels(wpath, factory.Get(), &w, &h, facePaths[i]);
        if (i == 0) { width = w; height = h; }
        else if (w != width || h != height)
            throw std::runtime_error(
                "CubeTexture: все 6 граней должны иметь одинаковые размеры (грань " +
                std::to_string(i) + ": " + std::to_string(w) + "x" + std::to_string(h) +
                ", ожидалось " + std::to_string(width) + "x" + std::to_string(height) + ")");
    }

    InitFromFaces(device, width, height, faces);
}

// ---------------------------------------------------------------------------
// Фабричный метод: одна картинка-крест 4×3
// ---------------------------------------------------------------------------
CubeTexture* CubeTexture::CreateFromCross(ID3D11Device* device, const std::string& path)
{
    ComPtr<IWICImagingFactory> factory;
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory, nullptr,
        CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
    if (FAILED(hr))
        throw std::runtime_error("CubeTexture::CreateFromCross: WIC factory failed");

    std::wstring wpath(path.begin(), path.end());
    UINT w = 0, h = 0;
    std::vector<BYTE> pixels = LoadImagePixels(wpath, factory.Get(), &w, &h, path);

    // Проверяем пропорции 4:3 (каждая грань — квадрат)
    if (w == 0 || h == 0 || w % 4 != 0 || h % 3 != 0 || (w / 4) != (h / 3))
        throw std::runtime_error(
            "CubeTexture::CreateFromCross: картинка должна быть пропорций 4:3 "
            "(каждая грань — квадрат): " + path +
            "  фактически: " + std::to_string(w) + "x" + std::to_string(h));

    const UINT faceW    = w / 4;
    const UINT faceH    = h / 3;
    const UINT srcStride = w * 4;

    // Раскладка cross:
    //   col:    0     1     2     3
    //   row 0:  .    +Y     .     .
    //   row 1: -X    +Z    +X    -Z
    //   row 2:  .    -Y     .     .
    std::array<std::vector<BYTE>, 6> faces;
    faces[0] = ExtractRect(pixels, srcStride, 2 * faceW, 1 * faceH, faceW, faceH); // +X
    faces[1] = ExtractRect(pixels, srcStride, 0,         1 * faceH, faceW, faceH); // -X
    faces[2] = ExtractRect(pixels, srcStride, 1 * faceW, 0,         faceW, faceH); // +Y
    faces[3] = ExtractRect(pixels, srcStride, 1 * faceW, 2 * faceH, faceW, faceH); // -Y
    faces[4] = ExtractRect(pixels, srcStride, 1 * faceW, 1 * faceH, faceW, faceH); // +Z
    faces[5] = ExtractRect(pixels, srcStride, 3 * faceW, 1 * faceH, faceW, faceH); // -Z

    auto* tex = new CubeTexture();
    tex->InitFromFaces(device, faceW, faceH, faces);
    return tex;
}

// ---------------------------------------------------------------------------
// Создаёт D3D11-ресурс из 6 готовых буферов
// ---------------------------------------------------------------------------
void CubeTexture::InitFromFaces(ID3D11Device* device, UINT faceW, UINT faceH,
                                const std::array<std::vector<BYTE>, 6>& faces)
{
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width            = faceW;
    desc.Height           = faceH;
    desc.MipLevels        = 1;
    desc.ArraySize        = 6;
    desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage            = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags        = D3D11_RESOURCE_MISC_TEXTURECUBE;

    D3D11_SUBRESOURCE_DATA init[6] = {};
    for (int i = 0; i < 6; i++)
    {
        init[i].pSysMem          = faces[i].data();
        init[i].SysMemPitch      = faceW * 4;
        init[i].SysMemSlicePitch = 0;
    }

    ComPtr<ID3D11Texture2D> tex;
    if (FAILED(device->CreateTexture2D(&desc, init, &tex)))
        throw std::runtime_error("CubeTexture: CreateTexture2D провалился");

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                      = desc.Format;
    srvDesc.ViewDimension               = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MipLevels       = 1;
    srvDesc.TextureCube.MostDetailedMip = 0;

    if (FAILED(device->CreateShaderResourceView(tex.Get(), &srvDesc, &srv_)))
        throw std::runtime_error("CubeTexture: CreateShaderResourceView провалился");
}

// ---------------------------------------------------------------------------
void CubeTexture::Bind(ID3D11DeviceContext* ctx, UINT slot) const
{
    ctx->PSSetShaderResources(slot, 1, srv_.GetAddressOf());
}

void CubeTexture::Unbind(ID3D11DeviceContext* ctx, UINT slot) const
{
    ID3D11ShaderResourceView* null = nullptr;
    ctx->PSSetShaderResources(slot, 1, &null);
}
