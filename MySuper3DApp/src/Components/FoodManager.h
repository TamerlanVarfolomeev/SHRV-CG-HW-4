#pragma once
#include "../Scene/Component.h"
#include "../Scene/Scene.h"
#include "../Graphics/Mesh.h"
#include "../Graphics/Shader.h"
#include <d3d11.h>

class FoodManager : public Component
{

public:
    FoodManager(Scene* scene, ID3D11Device* device, Shader* shader, GameObject* player);
    void Update(float dt) override;

private:
    std::unique_ptr<Mesh>   mesh_Circle;
    std::vector<GameObject*> foodObjects_;
    GameObject* player_;
    int maxFoodCount_ = 10;
};
