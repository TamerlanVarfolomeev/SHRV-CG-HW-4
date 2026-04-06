#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <string>

// Обёртка над ID3D11ShaderResourceView.
// Загружает изображение с диска через Windows Imaging Component (PNG, JPG, BMP, TGA...).
// Привязывает SRV к слоту t-регистра PS.
// Семплер привязывается глобально в Application::BeginFrame (s0 = LinearWrap).
class Texture
{
public:
    // path — абсолютный или относительный путь к файлу изображения
    explicit Texture(ID3D11Device* device, const std::string& path);

    // Привязать SRV к pixel shader, слот t{slot}
    void Bind(ID3D11DeviceContext* ctx, UINT slot = 0) const;

    // Отвязать слот (для чистоты: избежать предупреждений D3D11 debug layer)
    void Unbind(ID3D11DeviceContext* ctx, UINT slot = 0) const;

    bool IsValid() const { return srv_ != nullptr; }

    // Создать текстуру из массиви пикселей RGBA (width × height)
    static Texture* CreateFromPixels(ID3D11Device* device, int width, int height, const uint8_t* pixels);

private:
    // Конструктор без параметров — только для CreateFromPixels
    Texture() = default;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_;
};
