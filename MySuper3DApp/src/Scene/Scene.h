#pragma once
#include "GameObject.h"
#include "Camera.h"
#include "RenderContext.h"
#include <vector>
#include <memory>
#include <string>

class GraphicsDevice;

class Scene
{
public:
    // Создать объект и добавить на сцену. Возвращает сырой указатель для удобства.
    // Пример: auto* obj = scene.CreateObject("Cube");
    GameObject* CreateObject(const std::string& name = "GameObject");

    void Update(float dt);
    void FixedUpdate(float fixedDt);
    void Render(const RenderContext& ctx);
    void RenderShadow(const RenderContext& ctx);

    // Камера сцены (создаётся вместе со сценой)
    std::unique_ptr<Camera> camera;

    Scene();

private:
    std::vector<std::unique_ptr<GameObject>> objects_;
};
