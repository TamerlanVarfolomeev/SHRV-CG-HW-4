#include "src/Core/Application.h"
#include "src/Graphics/Mesh.h"
#include "src/Graphics/Shader.h"
#include "src/Components/MeshRenderer.h"
#include "src/Components/OrbitComponent.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

// Вспомогательная функция: создать сферу на сцене
static GameObject* MakeSphere(
    Scene& scene, ID3D11Device* device, Mesh* mesh, Shader* shader,
    const char* name,
    DirectX::XMFLOAT4 color, float scale)
{
    auto* obj = scene.CreateObject(name);
    auto* mr  = obj->AddComponent<MeshRenderer>(device, mesh, shader);
    mr->material.albedoColor = color;
    obj->transform.scale = { scale, scale, scale };
    return obj;
}

class MyApp : public Application
{
public:
    MyApp() : Application(1280, 720, L"Solar System") {}

protected:
    void OnStart() override
    {
        auto* device = GetGfx().GetDevice();

        meshSphere_ = std::make_unique<Mesh>(Mesh::CreateSphere(device, 24, 48));
        meshCube_   = std::make_unique<Mesh>(Mesh::CreateCube(device));

        shader_ = std::make_unique<Shader>(device,
            L"./Shaders/Unlit.hlsl",
            std::vector<InputElement>{
                { "POSITION", DXGI_FORMAT_R32G32B32_FLOAT },
                { "NORMAL",   DXGI_FORMAT_R32G32B32_FLOAT },
                { "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT    },
            });

        Scene& sc = GetScene();
        Mesh*  sp = meshSphere_.get();
        Shader* sh = shader_.get();

        // --- Солнце ---
        auto* sun = MakeSphere(sc, device, sp, sh, "Sun",
            { 1.0f, 0.92f, 0.23f, 1 }, 2.0f);
        auto* sunOrbit = sun->AddComponent<OrbitComponent>();
        sunOrbit->orbitRadius  = 0.0f;
        sunOrbit->orbitSpeed   = 0.0f;
        sunOrbit->selfRotateSpeed = 5.0f;

        // --- Меркурий ---
        auto* mercury = MakeSphere(sc, device, sp, sh, "Mercury",
            { 0.6f, 0.6f, 0.6f, 1 }, 0.25f);
        auto* oc = mercury->AddComponent<OrbitComponent>();
        oc->orbitRadius = 4.5f; oc->orbitSpeed = 2.0f; oc->orbitAngle = 0.8f;
        oc->selfRotateSpeed = 10.0f;

        // --- Венера ---
        auto* venus = MakeSphere(sc, device, sp, sh, "Venus",
            { 0.95f, 0.8f, 0.4f, 1 }, 0.45f);
        oc = venus->AddComponent<OrbitComponent>();
        oc->orbitRadius = 7.0f; oc->orbitSpeed = 1.4f; oc->orbitAngle = 2.1f;
        oc->selfRotateSpeed = -5.0f; // Венера вращается в обратную сторону

        // --- Земля ---
        auto* earth = MakeSphere(sc, device, sp, sh, "Earth",
            { 0.2f, 0.5f, 1.0f, 1 }, 0.5f);
        auto* earthOrbit = earth->AddComponent<OrbitComponent>();
        earthOrbit->orbitRadius = 10.0f; earthOrbit->orbitSpeed = 1.0f; earthOrbit->orbitAngle = 4.0f;
        earthOrbit->selfRotateSpeed = 60.0f;

        // Луна (орбитирует Землю)
        auto* moon = MakeSphere(sc, device, sp, sh, "Moon",
            { 0.8f, 0.8f, 0.8f, 1 }, 0.15f);
        oc = moon->AddComponent<OrbitComponent>();
        oc->parent = &earth->transform;
        oc->orbitRadius = 1.2f; oc->orbitSpeed = 2.5f; oc->orbitAngle = 1.0f;
        oc->selfRotateSpeed = 5.0f;

        // --- Марс ---
        auto* mars = MakeSphere(sc, device, sp, sh, "Mars",
            { 0.85f, 0.3f, 0.1f, 1 }, 0.35f);
        oc = mars->AddComponent<OrbitComponent>();
        oc->orbitRadius = 13.5f; oc->orbitSpeed = 0.75f; oc->orbitAngle = 1.5f;
        oc->selfRotateSpeed = 55.0f;

        // --- Юпитер ---
        auto* jupiter = MakeSphere(sc, device, sp, sh, "Jupiter",
            { 0.85f, 0.65f, 0.4f, 1 }, 1.0f);
        auto* jupiterOrbit = jupiter->AddComponent<OrbitComponent>();
        jupiterOrbit->orbitRadius = 19.0f; jupiterOrbit->orbitSpeed = 0.42f; jupiterOrbit->orbitAngle = 3.2f;
        jupiterOrbit->selfRotateSpeed = 200.0f;

        // Ио (луна Юпитера)
        auto* io = MakeSphere(sc, device, sp, sh, "Io",
            { 0.95f, 0.85f, 0.1f, 1 }, 0.12f);
        oc = io->AddComponent<OrbitComponent>();
        oc->parent = &jupiter->transform;
        oc->orbitRadius = 1.8f; oc->orbitSpeed = 3.5f; oc->orbitAngle = 0.5f;
        oc->selfRotateSpeed = 20.0f;

        // --- Сатурн ---
        auto* saturn = MakeSphere(sc, device, sp, sh, "Saturn",
            { 0.9f, 0.8f, 0.5f, 1 }, 0.85f);
        auto* saturnOrbit = saturn->AddComponent<OrbitComponent>();
        saturnOrbit->orbitRadius = 25.0f; saturnOrbit->orbitSpeed = 0.3f; saturnOrbit->orbitAngle = 5.5f;
        saturnOrbit->selfRotateSpeed = 180.0f;

        // Титан (луна Сатурна)
        auto* titan = MakeSphere(sc, device, sp, sh, "Titan",
            { 0.8f, 0.55f, 0.2f, 1 }, 0.18f);
        oc = titan->AddComponent<OrbitComponent>();
        oc->parent = &saturn->transform;
        oc->orbitRadius = 2.0f; oc->orbitSpeed = 1.8f; oc->orbitAngle = 2.8f;
        oc->selfRotateSpeed = 10.0f;

        // --- Уран ---
        auto* uranus = MakeSphere(sc, device, sp, sh, "Uranus",
            { 0.4f, 0.9f, 0.9f, 1 }, 0.6f);
        oc = uranus->AddComponent<OrbitComponent>();
        oc->orbitRadius = 31.0f; oc->orbitSpeed = 0.2f; oc->orbitAngle = 0.3f;
        oc->selfRotateSpeed = 90.0f;

        // --- Нептун ---
        auto* neptune = MakeSphere(sc, device, sp, sh, "Neptune",
            { 0.2f, 0.3f, 0.9f, 1 }, 0.55f);
        oc = neptune->AddComponent<OrbitComponent>();
        oc->orbitRadius = 36.0f; oc->orbitSpeed = 0.15f; oc->orbitAngle = 1.8f;
        oc->selfRotateSpeed = 100.0f;
    }

    void OnUpdate(float dt) override {}

private:
    std::unique_ptr<Mesh>   meshSphere_;
    std::unique_ptr<Mesh>   meshCube_;
    std::unique_ptr<Shader> shader_;
};

int main()
{
    MyApp app;
    app.Run();
}
