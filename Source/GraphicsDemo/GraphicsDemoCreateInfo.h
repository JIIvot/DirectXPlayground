#pragma once

#include <cstdint>
#include <Windows.h>

struct ID3D12Device5;
struct IDXGIFactory6;

struct GraphicsDemoCreateInfo
{
    HWND           windowHandle;
    ID3D12Device5* device;
    IDXGIFactory6* dxgiFactory;
    int32_t        clientWidth;
    int32_t        clientHeight;
};
