#pragma once
#include "../Graphics/GraphicsDevice.h"
#include "../Graphics/SwapChainTarget.h"
#include "../Graphics/ConstantBuffer.h"
#include "../Graphics/ConstantBuffers.h"
#include "../Graphics/States.h"
#include "../Graphics/ShadowMap.h"
#include "../Graphics/Shader.h"
#include "../Scene/Scene.h"
#include "../Scene/RenderContext.h"
#include "Window.h"
#include "TimeAccumulator.h"
#include "../Physics/PhysicsSystem.h"
#include <string>
#include <DirectXMath.h>

class Application
{
    friend class PlayerController;

public:
    Application(int width, int height, const std::wstring& title);
    virtual ~Application();

    void Run();

    // Шаг фиксированного обновления (секунды). По умолчанию 200 раз в секунду.
    float fixedDt = 0.005f;

    // --- Параметры shadow mapping (можно менять из наследника) ---
    DirectX::XMFLOAT3 shadowLightDir  = { -0.5f, -1.0f, -0.3f };
    DirectX::XMFLOAT3 shadowTarget    = {  0.0f,  0.0f,  0.0f }; // центр зоны теней (обычно следует за игроком)
    float             shadowOrthoSize = 60.0f;   // XY-размер ортографической области
    float             shadowDepth     = 200.0f;  // глубина (диапазон Z в свет-камере)

protected:
    // Переопределяй в наследниках для своей логики
    virtual void OnStart() {}
    virtual void OnUpdate(float dt) {}
    virtual void OnFixedUpdate(float fixedDt) {}
    virtual void OnRender(const RenderContext& ctx) {}

    // Доступ к движку из наследника
    GraphicsDevice& GetGfx()     { return *gfx_; }
    Scene&          GetScene()   { return *scene_; }
    States&         GetStates()  { return *states_; }
    PhysicsSystem&  GetPhysics() { return *physics_; }
    Window&         GetWindow()  { return *window_; }
    int             GetWidth()   const { return width_; }
    int             GetHeight()  const { return height_; }

    int width_  = 0;
    int height_ = 0;

private:
    void BeginFrame(float dt, float totalTime);
    void EndFrame();
    void OnWindowResize(int w, int h);
    void RenderShadowPass();

    std::unique_ptr<Window>          window_;
    std::unique_ptr<GraphicsDevice>  gfx_;
    std::unique_ptr<SwapChainTarget> swapChain_;
    std::unique_ptr<States>          states_;
    std::unique_ptr<Scene>           scene_;
    std::unique_ptr<PhysicsSystem>   physics_;
    std::unique_ptr<ShadowMap>       shadowMap_;
    std::unique_ptr<Shader>          shadowShader_;

    ConstantBuffer<CBPerFrame>       cbFrame_;
    ConstantBuffer<CBPerCamera>      cbCamera_;
    ConstantBuffer<CBDirectedLight>  cbDefaultLight_; // привязывается в BeginFrame как fallback
    ConstantBuffer<CBShadow>         cbShadow_;

    TimeAccumulator timer_;
    float totalTime_ = 0.0f;
    bool wireframe_  = false;
};
