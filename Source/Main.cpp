#include "Application.h"
#include "GraphicsDemo/ClearScreenDemo/ClearScreenDemo.h"

extern "C"
{
	__declspec( dllexport ) extern const UINT  D3D12SDKVersion = 619;
	__declspec( dllexport ) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

int main()
{
	CApplication::Init<CClearScreenDemo>();
	CApplication::GetInstance().Run();
	CApplication::Destroy();
}
