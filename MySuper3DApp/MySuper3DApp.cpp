#include "src/Core/Application.h"
#include "src/Components/PlanetManager.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

class MyApp : public Application
{
public:
    MyApp() : Application(1280, 720, L"Solar System") {}

protected:
    void OnStart() override
    {
        auto* go = GetScene().CreateObject("PlanetManager");
        go->AddComponent<PlanetManager>(
            GetGfx().GetDevice(),
            &GetScene(),
            GetScene().camera.get());
    }
};

int main()
{
    MyApp app;
    app.Run();
}
