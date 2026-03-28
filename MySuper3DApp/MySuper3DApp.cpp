#include "src/Core/Application.h"
#include "src/Graphics/Mesh.h"
#include "src/Graphics/Shader.h"
#include "src/Components/MeshRenderer.h"
#include "src/Components/PlayerController.h"
#include "src/Components/FoodManager.h"

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

        // Создаём круг на сцене
        auto* objCircle = GetScene().CreateObject("Circle");
        
        auto* cirlceMeshRender = objCircle->AddComponent<MeshRenderer>(device, mesh_Circle.get(), shader_.get());
        cirlceMeshRender->material.albedoColor = { 1.0f, 0.5f, 0.2f, 1.0f }; // оранжевый
        objCircle->transform.scale = { 0.1f, 0.1f, 0.1f };

        auto* PC = objCircle->AddComponent<PlayerController>();

        auto* foodManager = GetScene().CreateObject("FoodManager");
        auto* foodComp = foodManager->AddComponent<FoodManager>(&GetScene(), device, shader_.get(), objCircle);
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
