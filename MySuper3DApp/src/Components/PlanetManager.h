#pragma once
#include "../Scene/Component.h"
#include "../Scene/Camera.h"
#include "../Scene/Transform.h"
#include "../Graphics/Mesh.h"
#include "../Graphics/Shader.h"
#include <vector>
#include <memory>
#include <string>

class Scene;

class PlanetManager : public Component
{
public:
    PlanetManager(ID3D11Device* device, Scene* scene, Camera* camera);

    void Update(float dt) override;

private:
    Camera* camera_;
    Scene* scene_;
    ID3D11Device* device_;

    std::unique_ptr<Mesh>   meshSphere_;
    std::unique_ptr<Shader> shader_;

    struct PlanetEntry {
        std::string name;
        Transform*  transform;
    };
    std::vector<PlanetEntry> planets_;
    int followIdx_ = -1; // -1 = свободная камера

    void SpawnPlanets();
};
