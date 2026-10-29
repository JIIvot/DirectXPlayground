#pragma once

#include "GraphicsDemo/GraphicsDemo.h"

#include <string>

class CClearScreenDemo final : public CGraphicsDemo
{
public:
    CClearScreenDemo( HWND windowHandle, IDXGIFactory6* dxgiFactory, ID3D12Device5* device )
        : CGraphicsDemo( windowHandle, dxgiFactory, device )
    {
    }

    void OnRender() override;

private:
    void RenderImGui() const;
};
