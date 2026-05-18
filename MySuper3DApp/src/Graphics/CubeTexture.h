#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <string>
#include <array>
#include <vector>

// Кубическая текстура (cubemap) для скайбоксов / отражений.
// Грузит 6 граней через WIC. Все грани должны быть одного размера.
//
// Порядок граней (D3D11):
//   0 — +X (right)
//   1 — -X (left)
//   2 — +Y (top)
//   3 — -Y (bottom)
//   4 — +Z (front)
//   5 — -Z (back)
class CubeTexture
{
public:
    CubeTexture(ID3D11Device* device,
                const std::array<std::string, 6>& facePaths);

    // Загружает кубическую карту из одной картинки с развёрткой «крест» 4×3:
    //
    //     .  +Y .  .
    //     -X +Z +X -Z
    //     .  -Y .  .
    //
    // Картинка должна быть пропорций 4:3 (каждая грань — квадрат).
    // Возвращает new CubeTexture* — заверни в unique_ptr.
    static CubeTexture* CreateFromCross(ID3D11Device* device, const std::string& path);

    void Bind(ID3D11DeviceContext* ctx, UINT slot = 0) const;
    void Unbind(ID3D11DeviceContext* ctx, UINT slot = 0) const;

    bool IsValid() const { return srv_ != nullptr; }

private:
    // Для CreateFromCross (внутреннее использование)
    CubeTexture() = default;

    // Создаёт D3D11 ресурс из 6 готовых пиксельных буферов одного размера
    void InitFromFaces(ID3D11Device* device, UINT faceW, UINT faceH,
                       const std::array<std::vector<unsigned char>, 6>& faces);

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_;
};
