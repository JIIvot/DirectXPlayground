#include "Application.h"
#include "GraphicsDemo/BoxDemo/BoxDemo.h"

extern "C"
{
    __declspec( dllexport ) extern const UINT  D3D12SDKVersion = 619;
    __declspec( dllexport ) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

int main()
{
    CApplication::Init<CBoxDemo>();
    CApplication::GetInstance().Run();
    CApplication::Destroy();
}
