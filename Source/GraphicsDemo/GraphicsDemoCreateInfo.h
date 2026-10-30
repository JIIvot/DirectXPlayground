#pragma once

#include <cstdint>
#include <Windows.h>

struct IDXGIFactory6;
struct ID3D12Device5;

struct GraphicsDemoCreateInfo
{
    HWND           windowHandle;
    IDXGIFactory6* dxgiFactory;
    ID3D12Device5* device;
    int32_t        clientWidth;
    int32_t        clientHeight;
};
