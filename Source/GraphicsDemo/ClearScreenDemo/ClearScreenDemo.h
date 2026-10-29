#pragma once

#include "GraphicsDemo/GraphicsDemo.h"

#include <string>

class ClearScreenDemo final : public GraphicsDemo
{
public:
    explicit ClearScreenDemo( const GraphicsDemoCreateInfo& info )
        : GraphicsDemo( info )
    {
    }

    void OnRender() override;

private:
    void RenderImGui() const;
};
