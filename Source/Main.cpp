#include "Application.h"

extern "C"
{
__declspec( dllexport ) extern const UINT D3D12SDKVersion = 619;
__declspec( dllexport ) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

int main()
{
	CApplication::Init();
	CApplication::GetInstance().Run();
	CApplication::Destroy();
}
