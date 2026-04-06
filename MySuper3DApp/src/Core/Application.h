#pragma once
#include "../Graphics/GraphicsDevice.h"
#include "../Graphics/SwapChainTarget.h"
#include "../Graphics/ConstantBuffer.h"
#include "../Graphics/ConstantBuffers.h"
#include "../Graphics/States.h"
#include "../Scene/Scene.h"
#include "../Scene/RenderContext.h"
#include "Window.h"
#include "TimeAccumulator.h"
#include <string>

class Application
{
public:
    Application(int width, int height, const std::wstring& title);
    virtual ~Application() = default;

    void Run();

    // Шаг фиксированного обновления (секунды). По умолчанию 50 раз в секунду.
    float fixedDt = 0.02f;

protected:
    // Переопределяй в наследниках для своей логики
    virtual void OnStart() {}
    virtual void OnUpdate(float dt) {}
    virtual void OnFixedUpdate(float fixedDt) {}
    virtual void OnRender(const RenderContext& ctx) {}

    // Доступ к движку из наследника
    GraphicsDevice& GetGfx()    { return *gfx_; }
    Scene&          GetScene()  { return *scene_; }
    States&         GetStates() { return *states_; }

    int width_  = 0;
    int height_ = 0;

private:
    void BeginFrame(float dt, float totalTime);
    void EndFrame();
    void OnWindowResize(int w, int h);

    std::unique_ptr<Window>          window_;
    std::unique_ptr<GraphicsDevice>  gfx_;
    std::unique_ptr<SwapChainTarget> swapChain_;
    std::unique_ptr<States>          states_;
    std::unique_ptr<Scene>           scene_;

    ConstantBuffer<CBPerFrame>  cbFrame_;
    ConstantBuffer<CBPerCamera> cbCamera_;

    TimeAccumulator timer_;
    float totalTime_ = 0.0f;
    bool wireframe_  = false;
};
