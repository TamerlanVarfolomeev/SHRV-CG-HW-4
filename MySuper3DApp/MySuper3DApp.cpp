#define NOMINMAX
#include "src/Core/Application.h"
#include "src/Graphics/ObjLoader.h"
#include "src/Graphics/Shader.h"
#include "src/Graphics/Mesh.h"
#include "src/Components/MeshRenderer.h"
#include "src/Components/RigidBody.h"
#include "src/Components/BoxCollider.h"
#include "src/Components/MeshCollider.h"
#include "src/Components/PlayerController.h"
#include "src/Components/SphereCollider.h"
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

        // --- Террейн ---
        terrainSubmeshes_ = ObjLoader::Load(dev, "Assets/terrain.obj");
        CreateTerrain(dev);

        // --- Камера: орбита чуть выше сцены ---
        GetScene().camera->SetOrbitDistance(20.0f);

        spawnTimer_ = spawnInterval_;

        // --- Игрок (сфера) ---
        CreatePlayer(dev);
    }

    void OnFixedUpdate(float fixedDt) override
    {
        // Камера следует за игроком
        if (playerController_)
            playerController_->FollowCamera();
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
    void CreateTerrain(ID3D11Device* dev)
    {
        if (terrainSubmeshes_.empty()) return;

        auto* go = GetScene().CreateObject("Terrain");
        go->transform.position = { 0.0f, 0.0f, 0.0f };
        go->transform.scale    = { 1.0f, 1.0f, 1.0f };
        go->transform.rotation = { 0.0f, 0.0f, 0.0f };

        // Рендер: все submesh террейна
        for (auto& sm : terrainSubmeshes_)
        {
            auto* r = go->AddComponent<MeshRenderer>(
                dev, sm.mesh.get(), shader_.get(), sm.texture.get());
            r->material.albedoColor = sm.albedoColor;
        }

        // Физика: статическое тело + MeshCollider (ConcaveMeshShape — любая геометрия)
        go->AddComponent<RigidBody>(
            GetPhysics().GetWorld(),
            &GetPhysics().GetCommon(),
            PhysicsBodyType::Static);

        // Собираем все вершины и индексы из всех submesh террейна
        std::vector<DirectX::XMFLOAT3> allPositions;
        std::vector<uint32_t>          allIndices;
        uint32_t vertexOffset = 0;

        for (auto& sm : terrainSubmeshes_)
        {
            allPositions.insert(allPositions.end(),
                                sm.meshPositions.begin(),
                                sm.meshPositions.end());
            for (uint32_t idx : sm.meshIndices)
                allIndices.push_back(idx + vertexOffset);
            vertexOffset += static_cast<uint32_t>(sm.meshPositions.size());
        }

        go->AddComponent<MeshCollider>(
            &GetPhysics().GetCommon(),
            std::move(allPositions),
            std::move(allIndices));
    }

    // -----------------------------------------------------------------------
    void CreatePlayer(ID3D11Device* dev)
    {
        // Глобальный указатель для PlayerController
        gApp = this;

        auto* go = GetScene().CreateObject("Player");
        go->transform.position = { 0.0f, 2.0f, 0.0f };

        // RigidBody — динамическое тело
        auto* rb = go->AddComponent<RigidBody>(
            GetPhysics().GetWorld(),
            &GetPhysics().GetCommon(),
            PhysicsBodyType::Dynamic);
        rb->mass = 1.0f;
        rb->linearDamping = 0.5f;
        rb->angularDamping = 0.3f;

        // SphereCollider
        go->AddComponent<SphereCollider>(
            &GetPhysics().GetCommon(),
            0.5f);

        // MeshRenderer — сфера
        playerMesh_ = std::make_unique<Mesh>(Mesh::CreateSphere(dev, 16, 32));
        auto* r = go->AddComponent<MeshRenderer>(
            dev, playerMesh_.get(), shader_.get());
        r->material.albedoColor = { 0.2f, 0.8f, 0.2f, 1.0f }; // зелёная сфера

        // PlayerController — управление и камера
        playerController_ = go->AddComponent<PlayerController>(15.0f, 0.5f);
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
    std::vector<ObjSubMesh>      terrainSubmeshes_;
    std::vector<ObjSubMesh>      bazukaSubmeshes_;
    PlayerController*            playerController_ = nullptr;
    std::unique_ptr<Mesh>        playerMesh_;

    float spawnTimer_    = 0.0f;
    float spawnInterval_ = 1.5f; // базука каждые 1.5 секунды
};

int main()
{
    MyApp app;
    app.Run();
}
