#include "src/Core/Application.h"
#include "src/Graphics/ObjLoader.h"
#include "src/Graphics/Shader.h"
#include "src/Components/MeshRenderer.h"
#include <vector>
#include <memory>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

class MyApp : public Application
{
public:
    MyApp() : Application(1280, 720, L"Bazooka Viewer") {}

protected:
    void OnStart() override
    {
        ID3D11Device* dev = GetGfx().GetDevice();

        // Шейдер с поддержкой текстуры
        shader_ = std::make_unique<Shader>(dev,
            L"Shaders/Textured.hlsl",
            std::vector<InputElement>{
                { "POSITION", DXGI_FORMAT_R32G32B32_FLOAT },
                { "NORMAL",   DXGI_FORMAT_R32G32B32_FLOAT },
                { "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT    },
            });

        // Загружаем модель
        submeshes_ = ObjLoader::Load(dev, "Assets/bazooka.obj");

        for (auto& sm : submeshes_)
        {
            auto* go = GetScene().CreateObject(sm.name);
            auto* r  = go->AddComponent<MeshRenderer>(
                dev, sm.mesh.get(), shader_.get(), sm.texture.get());
            r->material.albedoColor = sm.albedoColor;
        }

        // Орбитальная камера вокруг центра сцены
        // ЛКМ + мышь = вращение, колёсико = зум, Tab = переключение в FPS
        GetScene().camera->SetOrbitDistance(3.0f);
    }

private:
    std::unique_ptr<Shader> shader_;
    std::vector<ObjSubMesh> submeshes_; // владеет мешами и текстурами
};

int main()
{
    MyApp app;
    app.Run();
}
