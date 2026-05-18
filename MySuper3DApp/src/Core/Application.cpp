#include "Application.h"
#include "Input.h"
#include <objbase.h>

using namespace DirectX;

Application::Application(int width, int height, const std::wstring& title)
    : width_(width), height_(height)
{
    // WIC (загрузка текстур) требует инициализации COM
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    window_    = std::make_unique<Window>(width, height, title);
    gfx_       = std::make_unique<GraphicsDevice>();
    swapChain_ = std::make_unique<SwapChainTarget>(*gfx_, window_->GetHWND(), width, height);
    states_    = std::make_unique<States>(gfx_->GetDevice());
    scene_     = std::make_unique<Scene>();
    physics_   = std::make_unique<PhysicsSystem>();
    scene_->camera->SetAspect(static_cast<float>(width) / static_cast<float>(height));

    cbFrame_        = ConstantBuffer<CBPerFrame>(gfx_->GetDevice());
    cbCamera_       = ConstantBuffer<CBPerCamera>(gfx_->GetDevice());
    cbDefaultLight_ = ConstantBuffer<CBDirectedLight>(gfx_->GetDevice());
    cbShadow_       = ConstantBuffer<CBShadow>(gfx_->GetDevice());

    // --- Shadow mapping инфраструктура ---
    shadowMap_ = std::make_unique<ShadowMap>(gfx_->GetDevice(), 2048);
    shadowShader_ = std::make_unique<Shader>(gfx_->GetDevice(),
        L"Shaders/ShadowDepth.hlsl",
        std::vector<InputElement>{
            { "POSITION", DXGI_FORMAT_R32G32B32_FLOAT },
            { "NORMAL",   DXGI_FORMAT_R32G32B32_FLOAT },
            { "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT    },
        });

    window_->OnResize = [this](int w, int h) { OnWindowResize(w, h); };
}

Application::~Application()
{
    CoUninitialize();
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
            scene_->FixedUpdate(fixedDt);   // компоненты создают тела + синхронизируют
            physics_->Step(fixedDt);        // симуляция
            OnFixedUpdate(fixedDt);
        }

        // --- Обычное обновление (анимация, камера) ---
        scene_->Update(dt);
        OnUpdate(dt);

        if (Input::GetKeyDown(Key::F))
            wireframe_ = !wireframe_;

        // --- Shadow pass (рендерим сцену из источника света в shadow map) ---
        RenderShadowPass();

        // --- Main pass ---
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

    // Глобальный семплер для текстурных шейдеров (s0 = LinearWrap)
    auto* sampler = states_->Sampler.LinearWrap.Get();
    context->PSSetSamplers(0, 1, &sampler);

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

    // CBDirectedLight (b4) — дефолтные значения; DirectedLight-компонент перезапишет их
    CBDirectedLight lightData;  // заполнен дефолтами из ConstantBuffers.h
    cbDefaultLight_.Update(context, lightData);
    cbDefaultLight_.Bind(context, ShaderStage::PS, 4);

    // CBShadow (b5) — lightViewProj был обновлён в RenderShadowPass, переподтверждаем привязку
    cbShadow_.Bind(context, ShaderStage::Both, 5);

    // --- Shadow map → t1 + comparison sampler → s1 ---
    auto* shadowSrv = shadowMap_->GetSRV();
    context->PSSetShaderResources(1, 1, &shadowSrv);
    auto* shadowSamp = states_->Sampler.ShadowCompare.Get();
    context->PSSetSamplers(1, 1, &shadowSamp);
}

void Application::RenderShadowPass()
{
    auto* dctx = gfx_->GetContext();

    // --- Считаем lightViewProj от направления света + центра/размера зоны теней ---
    XMVECTOR dir = XMVector3Normalize(XMLoadFloat3(&shadowLightDir));
    XMVECTOR tgt = XMLoadFloat3(&shadowTarget);
    XMVECTOR eye = XMVectorSubtract(tgt, XMVectorScale(dir, shadowDepth * 0.5f));

    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    // Если свет почти вертикальный — берём другой up, чтобы LookAt не вырождался
    if (std::fabs(XMVectorGetY(dir)) > 0.99f)
        up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

    XMMATRIX view = XMMatrixLookAtLH(eye, tgt, up);
    XMMATRIX proj = XMMatrixOrthographicLH(shadowOrthoSize, shadowOrthoSize, 0.0f, shadowDepth);

    CBShadow shadowData;
    XMStoreFloat4x4(&shadowData.lightViewProj, view * proj);
    cbShadow_.Update(dctx, shadowData);
    cbShadow_.Bind(dctx, ShaderStage::Both, 5);

    // CBPerFrame / CBPerCamera всё ещё актуальны с прошлого кадра (или пересчитаем здесь?)
    // Shadow-шейдер их не использует, поэтому не критично.

    // --- Отвязываем shadow map от PS (была привязана в прошлом кадре как SRV t1) ---
    ID3D11ShaderResourceView* nullSrv[1] = { nullptr };
    dctx->PSSetShaderResources(1, 1, nullSrv);

    // --- Подготовка target: DSV без RT, очистка, viewport ---
    shadowMap_->BeginPass(dctx);

    // --- Состояния: твёрдая заливка, обычный depth test ---
    dctx->RSSetState(states_->Rasterizer.Solid.Get());
    dctx->OMSetDepthStencilState(states_->DepthStencil.Default.Get(), 0);

    // --- Биндим shadow shader ---
    shadowShader_->Bind(dctx);

    // --- Рендер всей сцены в depth ---
    RenderContext ctx{ gfx_.get(), scene_->camera.get() };
    scene_->RenderShadow(ctx);
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
