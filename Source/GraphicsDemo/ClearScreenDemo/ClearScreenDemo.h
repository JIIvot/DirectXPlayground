#pragma once

#include "GraphicsDemo/GraphicsDemo.h"

#include <string>

class CClearScreenDemo final : public CGraphicsDemo
{
public:
    explicit CClearScreenDemo( HWND windowHandle );

    void Render() override;

private:
    void RenderImGui() const;

private:
    std::wstring m_adapterName;
};
