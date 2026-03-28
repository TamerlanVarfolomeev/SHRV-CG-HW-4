#include "Scene.h"

Scene::Scene()
{
    // Камера по умолчанию: 45° FOV, соотношение сторон 1:1 (обновится при resize)
    camera = std::make_unique<Camera>(45.0f, 1.0f, 0.1f, 1000.0f);
}

GameObject* Scene::CreateObject(const std::string& name)
{
    auto obj = std::make_unique<GameObject>(name);
    GameObject* ptr = obj.get();
    objects_.push_back(std::move(obj));
    return ptr;
}

void Scene::Update(float dt)
{
    camera->Update(dt);

    for (auto& obj : objects_)
        obj->Update(dt);
}

void Scene::FixedUpdate(float fixedDt)
{
    for (auto& obj : objects_)
        obj->FixedUpdate(fixedDt);
}

void Scene::Render(const RenderContext& ctx)
{
    for (auto& obj : objects_)
        obj->Render(ctx);
}
