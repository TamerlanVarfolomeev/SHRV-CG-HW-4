#pragma once
#include <d3d11.h>
#include <wrl.h>

// Depth-only render target для shadow mapping.
// Содержит одну Texture2D (D24) с двумя view:
//   • DSV — для записи во время shadow pass
//   • SRV — для семплинга в основном проходе (через SamplerComparisonState)
class ShadowMap
{
public:
    explicit ShadowMap(ID3D11Device* device, int resolution = 2048);

    // Привязывает DSV без render target, очищает depth, выставляет viewport.
    // Вызывать перед рендером сцены в depth-only режиме.
    void BeginPass(ID3D11DeviceContext* ctx);

    ID3D11ShaderResourceView* GetSRV()        const { return srv_.Get(); }
    ID3D11DepthStencilView*   GetDSV()        const { return dsv_.Get(); }
    int                       GetResolution() const { return resolution_; }

private:
    Microsoft::WRL::ComPtr<ID3D11Texture2D>          texture_;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>   dsv_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_;
    int                                              resolution_;
};
