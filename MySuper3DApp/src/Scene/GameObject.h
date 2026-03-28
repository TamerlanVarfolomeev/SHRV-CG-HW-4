#pragma once
#include <string>
#include "Transform.h"
#include "Component.h"
#include "RenderContext.h"
#include <vector>
#include <memory>
#include <utility>

class GameObject
{
public:
    Transform transform;
    std::string name;

    explicit GameObject(const std::string& name = "GameObject") : name(name) {}

    // Создать и прикрепить компонент.
    // Пример: obj.AddComponent<MeshRenderer>(mesh, shader);
    template<typename T, typename... Args>
    T* AddComponent(Args&&... args)
    {
        auto comp = std::make_unique<T>(std::forward<Args>(args)...);
        comp->gameObject = this;
        T* ptr = comp.get();
        components_.push_back(std::move(comp));
        return ptr;
    }

    // Найти первый компонент указанного типа. Возвращает nullptr если не найден.
    template<typename T>
    T* GetComponent()
    {
        for (auto& c : components_)
            if (auto* p = dynamic_cast<T*>(c.get())) return p;
        return nullptr;
    }

    template<typename T>
    const T* GetComponent() const
    {
        for (const auto& c : components_)
            if (const auto* p = dynamic_cast<const T*>(c.get())) return p;
        return nullptr;
    }

    void Update(float dt)
    {
        for (auto& c : components_) c->Update(dt);
    }

    void FixedUpdate(float fixedDt)
    {
        for (auto& c : components_) c->FixedUpdate(fixedDt);
    }

    void Render(const RenderContext& ctx)
    {
        for (auto& c : components_) c->Render(ctx);
    }

private:
    std::vector<std::unique_ptr<Component>> components_;
};
