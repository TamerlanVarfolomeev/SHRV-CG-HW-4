#define NOMINMAX
#include "src/Core/Application.h"
#include "src/Graphics/ObjLoader.h"
#include "src/Graphics/Shader.h"
#include "src/Graphics/Mesh.h"
#include "src/Components/MeshRenderer.h"
#include "src/Components/RigidBody.h"
#include "src/Components/BoxCollider.h"
#include "src/Components/MeshCollider.h"
#include <vector>
#include <memory>
#include <cstdlib>
#include <cmath>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

class MyApp : public Application
{
public:
    MyApp() : Application(1280, 720, L"Bazooka Physics Demo") {}

protected:
    void OnStart() override
    {
        ID3D11Device* dev = GetGfx().GetDevice();

        // --- Шейдер ---
        shader_ = std::make_unique<Shader>(dev,
            L"Shaders/Textured.hlsl",
            std::vector<InputElement>{
                { "POSITION", DXGI_FORMAT_R32G32B32_FLOAT },
                { "NORMAL",   DXGI_FORMAT_R32G32B32_FLOAT },
                { "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT    },
            });

        // --- Загружаем ассеты базуки (один раз) ---
        bazukaSubmeshes_ = ObjLoader::Load(dev, "Assets/bazooka.obj");

        // --- Пол ---
        CreateFloor(dev);

        // --- Камера: орбита чуть выше сцены ---
        GetScene().camera->SetOrbitDistance(20.0f);

        spawnTimer_ = spawnInterval_;
    }

    void OnUpdate(float dt) override
    {
        spawnTimer_ -= dt;
        if (spawnTimer_ <= 0.0f)
        {
            spawnTimer_ = spawnInterval_;
            SpawnBazooka();
        }
    }

private:
    // -----------------------------------------------------------------------
    void CreateFloor(ID3D11Device* dev)
    {
        floorMesh_ = std::make_unique<Mesh>(Mesh::CreateCube(dev));

        auto* go = GetScene().CreateObject("Floor");
        go->transform.position = { 0.0f, -0.25f, 0.0f };
        go->transform.scale    = { 20.0f, 0.5f, 20.0f };

        go->AddComponent<MeshRenderer>(dev, floorMesh_.get(), shader_.get());
        // серый пол
        go->GetComponent<MeshRenderer>()->material.albedoColor = { 0.4f, 0.4f, 0.4f, 1.0f };

        auto* rb = go->AddComponent<RigidBody>(
            GetPhysics().GetWorld(),
            &GetPhysics().GetCommon(),
            PhysicsBodyType::Static);

        // полуразмер = половина scale (т.к. куб 1x1x1 масштабируется)
        go->AddComponent<BoxCollider>(
            &GetPhysics().GetCommon(),
            rp3d::Vector3(10.0f, 0.25f, 10.0f));
    }

    // -----------------------------------------------------------------------
    void SpawnBazooka()
    {
        if (bazukaSubmeshes_.empty()) return;

        ID3D11Device* dev = GetGfx().GetDevice();

        // Случайная позиция в пределах пола
        float x = ((float)(rand() % 160) - 80.0f) * 0.1f; // [-8, 8]
        float z = ((float)(rand() % 160) - 80.0f) * 0.1f;
        float y = 12.0f;

        // Случайный начальный поворот
        float ry = (float)(rand() % 360);

        // Создаём один GameObject с несколькими MeshRenderer-ами (по submesh)
        // Физическое тело — одно на всю базуку
        auto* root = GetScene().CreateObject("Bazooka");
        root->transform.position = { x, y, z };
        root->transform.rotation = { 0.0f, ry, 0.0f };

        auto* rb = root->AddComponent<RigidBody>(
            GetPhysics().GetWorld(),
            &GetPhysics().GetCommon(),
            PhysicsBodyType::Dynamic);
        rb->mass = 2.0f;

        // Аппроксимируем базуку коробкой (длинная тонкая форма)
        root->AddComponent<BoxCollider>(
            &GetPhysics().GetCommon(),
            rp3d::Vector3(0.12f, 0.12f, 0.55f));

        // Рендер: один MeshRenderer на первый submesh, остальные — отдельные объекты-дети
        // (в движке нет иерархии, поэтому храним все submesh на одном объекте через несколько компонент)
        // MeshRenderer не привязан к transform — он берёт transform у gameObject,
        // поэтому несколько рендереров на одном объекте работают корректно.
        for (auto& sm : bazukaSubmeshes_)
        {
            auto* r = root->AddComponent<MeshRenderer>(
                dev, sm.mesh.get(), shader_.get(), sm.texture.get());
            r->material.albedoColor = sm.albedoColor;
        }
    }

    // -----------------------------------------------------------------------
    std::unique_ptr<Shader>      shader_;
    std::unique_ptr<Mesh>        floorMesh_;
    std::vector<ObjSubMesh>      bazukaSubmeshes_;

    float spawnTimer_    = 0.0f;
    float spawnInterval_ = 1.5f; // базука каждые 1.5 секунды
};

int main()
{
    MyApp app;
    app.Run();
}
