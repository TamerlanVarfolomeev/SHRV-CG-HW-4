#pragma once

class GameObject;
struct RenderContext;

class Component
{
public:
    virtual ~Component() = default;

    // Вызывается каждый кадр (анимация, визуал)
    virtual void Update(float dt) {}

    // Вызывается с фиксированным шагом (физика, логика)
    // fixedDt — постоянная величина, например 0.02f (50 раз в секунду)
    virtual void FixedUpdate(float fixedDt) {}

    virtual void Render(const RenderContext& ctx) {}

    // Вызывается во время shadow pass — только геометрия, никаких текстур/материалов.
    // По умолчанию пусто; MeshRenderer переопределяет.
    virtual void RenderShadow(const RenderContext& ctx) {}

    // Устанавливается автоматически при AddComponent
    GameObject* gameObject = nullptr;
};
