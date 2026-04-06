#include "Application.h"
#include "Input.h"

using namespace DirectX;

Application::Application(int width, int height, const std::wstring& title)
    : width_(width), height_(height)
{
    window_    = std::make_unique<Window>(width, height, title);
    gfx_       = std::make_unique<GraphicsDevice>();
    swapChain_ = std::make_unique<SwapChainTarget>(*gfx_, window_->GetHWND(), width, height);
    states_    = std::make_unique<States>(gfx_->GetDevice());
    scene_     = std::make_unique<Scene>();
    scene_->camera->SetAspect(static_cast<float>(width) / static_cast<float>(height));

    cbFrame_  = ConstantBuffer<CBPerFrame>(gfx_->GetDevice());
    cbCamera_ = ConstantBuffer<CBPerCamera>(gfx_->GetDevice());

    window_->OnResize = [this](int w, int h) { OnWindowResize(w, h); };
}

void Application::Run()
{
    OnStart();

    while (true)
    {
        // --- Input: сброс дельт ДО сбора сообщений ---
        Input::NewFrame();

        // --- Сообщения окна (заполняют Input) ---
        if (!window_->ProcessMessages()) break;

        // --- Время ---
        float dt = timer_.Tick();
        totalTime_ += dt;

        // --- Фиксированный шаг (физика, логика) ---
        while (timer_.ShouldStep(fixedDt))
        {
            scene_->FixedUpdate(fixedDt);
            OnFixedUpdate(fixedDt);
        }

        // --- Обычное обновление (анимация, камера) ---
        scene_->Update(dt);
        OnUpdate(dt);

        if (Input::GetKeyDown(Key::F))
            wireframe_ = !wireframe_;

        // --- Рендер ---
        BeginFrame(dt, totalTime_);

        RenderContext ctx{ gfx_.get(), scene_->camera.get() };
        scene_->Render(ctx);
        OnRender(ctx);

        EndFrame();
    }
}

void Application::BeginFrame(float dt, float totalTime)
{
    auto* context = gfx_->GetContext();

    context->RSSetState(wireframe_
        ? states_->Rasterizer.Wireframe.Get()
        : states_->Rasterizer.Solid.Get());
    context->OMSetDepthStencilState(states_->DepthStencil.Default.Get(), 0);
    float blendFactor[4] = {};
    context->OMSetBlendState(states_->Blend.Opaque.Get(), blendFactor, 0xFFFFFFFF);

    D3D11_VIEWPORT vp = {};
    vp.Width    = static_cast<float>(width_);
    vp.Height   = static_cast<float>(height_);
    vp.MaxDepth = 1.0f;
    context->RSSetViewports(1, &vp);

    auto* rtv = swapChain_->GetRTV();
    auto* dsv = swapChain_->GetDSV();
    context->OMSetRenderTargets(1, &rtv, dsv);

    float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    context->ClearRenderTargetView(rtv, clearColor);
    context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // CBPerFrame (b0)
    CBPerFrame frameData;
    frameData.totalTime    = totalTime;
    frameData.deltaTime    = dt;
    frameData.screenWidth  = static_cast<float>(width_);
    frameData.screenHeight = static_cast<float>(height_);
    cbFrame_.Update(context, frameData);
    cbFrame_.Bind(context, ShaderStage::Both, 0);

    // CBPerCamera (b1)
    Camera* cam = scene_->camera.get();
    CBPerCamera camData;
    XMStoreFloat4x4(&camData.view,     cam->GetView());
    XMStoreFloat4x4(&camData.proj,     cam->GetProj());
    XMStoreFloat4x4(&camData.viewProj, cam->GetView() * cam->GetProj());
    camData.camPos = cam->GetPosition();
    cbCamera_.Update(context, camData);
    cbCamera_.Bind(context, ShaderStage::Both, 1);
}

void Application::EndFrame()
{
    gfx_->GetContext()->OMSetRenderTargets(0, nullptr, nullptr);
    swapChain_->Present(true);
}

void Application::OnWindowResize(int w, int h)
{
    if (w == 0 || h == 0) return;
    width_  = w;
    height_ = h;
    swapChain_->Resize(*gfx_, w, h);
    scene_->camera->SetAspect(static_cast<float>(w) / static_cast<float>(h));
}
