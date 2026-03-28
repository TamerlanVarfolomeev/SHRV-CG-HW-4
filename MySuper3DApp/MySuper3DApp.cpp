#include "src/Core/Application.h"
#include "src/Graphics/Mesh.h"
#include "src/Graphics/Shader.h"
#include "src/Components/MeshRenderer.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

// Наследуем Application и заполняем сцену в OnStart()
class MyApp : public Application
{
public:
    MyApp() : Application(800, 800, L"My3DEngine") {}

protected:
    void OnStart() override
    {
        auto* device = GetGfx().GetDevice();

        // Создаём ресурсы
        mesh_Cube   = std::make_unique<Mesh>(Mesh::CreateCube(device));
        mesh_Circle = std::make_unique<Mesh>(Mesh::CreateCircle(device, 32, 1.0f));
        shader_ = std::make_unique<Shader>(device,
            L"./Shaders/Unlit.hlsl",
            std::vector<InputElement>{
                { "POSITION", DXGI_FORMAT_R32G32B32_FLOAT },
                { "NORMAL",   DXGI_FORMAT_R32G32B32_FLOAT },
                { "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT    },
            });

        // Создаём объект на сцене
        auto* obj = GetScene().CreateObject("Cube");
        obj->transform.position = { 0.0f, 0.0f, -4.0f };

        auto* mr = obj->AddComponent<MeshRenderer>(device, mesh_Cube.get(), shader_.get());
        mr->material.albedoColor = { 1.0f, 0.5f, 0.2f, 1.0f }; // оранжевый

        // Создаём круг на сцене
        auto* objCircle = GetScene().CreateObject("Circle");
        objCircle->transform.position = { 0.0f, 0.0f, 2.0f };
        

        auto* mrr = objCircle->AddComponent<MeshRenderer>(device, mesh_Circle.get(), shader_.get());
        mrr->material.albedoColor = { 1.0f, 0.5f, 0.2f, 1.0f }; // оранжевый
    }

    void OnUpdate(float dt) override
    {
        // Здесь можно добавить логику, не связанную со сценой
    }

private:
    std::unique_ptr<Mesh>   mesh_Cube;
    std::unique_ptr<Mesh>   mesh_Circle;
    std::unique_ptr<Shader> shader_;
};

int main()
{
    MyApp app;
    app.Run();
}
