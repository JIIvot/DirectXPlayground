#include "Application.h"
#include "GraphicsDemo/BoxDemo/Ex3/BoxDemoEx3.h"

extern "C"
{
    __declspec( dllexport ) extern const UINT  D3D12SDKVersion = 620;
    __declspec( dllexport ) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

int main()
{
    Application::Init<BoxDemoEx3>();
    Application::Get().Run();
    Application::Destroy();
}
