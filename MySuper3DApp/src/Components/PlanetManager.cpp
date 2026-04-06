#include "PlanetManager.h"
#include "MeshRenderer.h"
#include "OrbitComponent.h"
#include "../Scene/Scene.h"
#include "../Scene/GameObject.h"
#include "../Core/Input.h"
#include <cstdlib>

using namespace DirectX;

static const Key kPlanetKeys[] = {
    Key::Num1, Key::Num2, Key::Num3, Key::Num4, Key::Num5,
    Key::Num6, Key::Num7, Key::Num8, Key::Num9,
};
static constexpr int kMaxPlanetKeys = 9;

PlanetManager::PlanetManager(ID3D11Device* device, Scene* scene, Camera* camera)
    : camera_(camera), scene_(scene), device_(device)
{
    meshSphere_ = std::make_unique<Mesh>(Mesh::CreateSphere(device, 24, 48));

    shader_ = std::make_unique<Shader>(device,
        L"./Shaders/Unlit.hlsl",
        std::vector<InputElement>{
            { "POSITION", DXGI_FORMAT_R32G32B32_FLOAT },
            { "NORMAL",   DXGI_FORMAT_R32G32B32_FLOAT },
            { "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT    },
        });

    SpawnPlanets();
}

void PlanetManager::SpawnPlanets()
{
    auto* mesh   = meshSphere_.get();
    auto* shader = shader_.get();

    auto make = [&](const char* name, XMFLOAT4 color, float scale) -> GameObject* {
        auto* obj = scene_->CreateObject(name);
        auto* mr  = obj->AddComponent<MeshRenderer>(device_, mesh, shader);
        mr->material.albedoColor = color;
        obj->transform.scale = { scale, scale, scale };
        planets_.push_back(PlanetEntry{ name, &obj->transform });
        return obj;
    };

    // --- Солнце (индекс 0, клавиша 1) ---
    auto* sun = make("Sun", { 1.0f, 0.92f, 0.23f, 1 }, 2.0f);
    auto* sunOrbit = sun->AddComponent<OrbitComponent>();
    sunOrbit->orbitRadius = 0.0f; sunOrbit->orbitSpeed = 0.0f; sunOrbit->selfRotateSpeed = 5.0f;

    // --- Меркурий (1, клавиша 2) ---
    auto* mercury = make("Mercury", { 0.6f, 0.6f, 0.6f, 1 }, 0.25f);
    auto* oc = mercury->AddComponent<OrbitComponent>();
    oc->orbitRadius = 4.5f; oc->orbitSpeed = 2.0f; oc->orbitAngle = 0.8f; oc->selfRotateSpeed = 10.0f;

    // --- Венера (2, клавиша 3) ---
    auto* venus = make("Venus", { 0.95f, 0.8f, 0.4f, 1 }, 0.45f);
    oc = venus->AddComponent<OrbitComponent>();
    oc->orbitRadius = 7.0f; oc->orbitSpeed = 1.4f; oc->orbitAngle = 2.1f; oc->selfRotateSpeed = -5.0f;

    // --- Земля (3, клавиша 4) ---
    auto* earth = make("Earth", { 0.2f, 0.5f, 1.0f, 1 }, 0.5f);
    oc = earth->AddComponent<OrbitComponent>();
    oc->orbitRadius = 10.0f; oc->orbitSpeed = 1.0f; oc->orbitAngle = 4.0f; oc->selfRotateSpeed = 60.0f;

    // Луна (не в списке слежения — дочерний объект)
    auto* moon = scene_->CreateObject("Moon");
    moon->AddComponent<MeshRenderer>(device_, mesh, shader)->material.albedoColor = { 0.8f, 0.8f, 0.8f, 1 };
    moon->transform.scale = { 0.15f, 0.15f, 0.15f };
    oc = moon->AddComponent<OrbitComponent>();
    oc->parent = &earth->transform;
    oc->orbitRadius = 1.2f; oc->orbitSpeed = 2.5f; oc->orbitAngle = 1.0f; oc->selfRotateSpeed = 5.0f;

    // --- Марс (4, клавиша 5) ---
    auto* mars = make("Mars", { 0.85f, 0.3f, 0.1f, 1 }, 0.35f);
    oc = mars->AddComponent<OrbitComponent>();
    oc->orbitRadius = 13.5f; oc->orbitSpeed = 0.75f; oc->orbitAngle = 1.5f; oc->selfRotateSpeed = 55.0f;

    // --- Юпитер (5, клавиша 6) ---
    auto* jupiter = make("Jupiter", { 0.85f, 0.65f, 0.4f, 1 }, 1.0f);
    oc = jupiter->AddComponent<OrbitComponent>();
    oc->orbitRadius = 19.0f; oc->orbitSpeed = 0.42f; oc->orbitAngle = 3.2f; oc->selfRotateSpeed = 200.0f;

    // --- Сатурн (6, клавиша 7) ---
    auto* saturn = make("Saturn", { 0.9f, 0.8f, 0.5f, 1 }, 0.85f);
    oc = saturn->AddComponent<OrbitComponent>();
    oc->orbitRadius = 25.0f; oc->orbitSpeed = 0.3f; oc->orbitAngle = 5.5f; oc->selfRotateSpeed = 180.0f;

    // --- Уран (7, клавиша 8) ---
    auto* uranus = make("Uranus", { 0.4f, 0.9f, 0.9f, 1 }, 0.6f);
    oc = uranus->AddComponent<OrbitComponent>();
    oc->orbitRadius = 31.0f; oc->orbitSpeed = 0.2f; oc->orbitAngle = 0.3f; oc->selfRotateSpeed = 90.0f;

    // --- Нептун (8, клавиша 9) ---
    auto* neptune = make("Neptune", { 0.2f, 0.3f, 0.9f, 1 }, 0.55f);
    oc = neptune->AddComponent<OrbitComponent>();
    oc->orbitRadius = 36.0f; oc->orbitSpeed = 0.15f; oc->orbitAngle = 1.8f; oc->selfRotateSpeed = 100.0f;
}

void PlanetManager::Update(float dt)
{
    const int count = static_cast<int>(planets_.size());
    const int keyCount = (count < kMaxPlanetKeys) ? count : kMaxPlanetKeys;

    for (int i = 0; i < keyCount; ++i)
    {
        if (Input::GetKeyDown(kPlanetKeys[i]))
        {
            if (followIdx_ == i)
            {
                // Повторное нажатие той же кнопки — сброс слежения
                followIdx_ = -1;
                camera_->SetOrbitCenter(nullptr);
            }
            else
            {
                followIdx_ = i;
                camera_->SetOrbitCenter(&planets_[i].transform->position);
                float planetScale = planets_[i].transform->scale.x;
                camera_->SetOrbitDistance(planetScale * 8.0f + 4.0f);
            }
            break;
        }
    }

    if (Input::GetKeyDown(Key::E))
    {
        if (followIdx_ != -1) // Если следим за планетой
        {
            auto* mesh = meshSphere_.get();
            auto* shader = shader_.get();

            auto* moon = scene_->CreateObject("Moon");
            auto randF = []{ return 0.3f + 0.7f * (rand() / float(RAND_MAX)); };
            moon->AddComponent<MeshRenderer>(device_, mesh, shader)->material.albedoColor = { randF(), randF(), randF(), 1 };
            moon->transform.scale = { 0.15f, 0.15f, 0.15f };
            auto* oc = moon->AddComponent<OrbitComponent>();

            float radius = planets_[followIdx_].transform->scale.x + 1.5f;

            oc->parent = planets_[followIdx_].transform;
            oc->orbitRadius = radius; oc->orbitSpeed = 5.5f; oc->orbitAngle = 1.0f; oc->selfRotateSpeed = 5.0f;
        }
    }
}
