#include "src/Core/Application.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

class MyApp : public Application
{
public:
    MyApp() : Application(1280, 720, L"MySuper3DApp") {}

protected:
    void OnStart() override
    {
    }
};

int main()
{
    MyApp app;
    app.Run();
}
