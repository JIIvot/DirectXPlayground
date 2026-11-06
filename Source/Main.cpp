#include "Application.h"
#include "GraphicsDemo/BoxDemo/Ex8/BoxDemoEx8.h"

extern "C"
{
    __declspec( dllexport ) extern const UINT  D3D12SDKVersion = 620;
    __declspec( dllexport ) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

int main()
{
    Application::Init<BoxDemoEx8>();
    Application::Get().Run();
    Application::Destroy();
}
