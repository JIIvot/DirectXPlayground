#pragma once

#include "GraphicsDemo/GraphicsDemo.h"

class ClearScreenDemo final : public GraphicsDemo
{
public:
    explicit ClearScreenDemo( const GraphicsDemoCreateInfo& info )
        : GraphicsDemo( info )
    {
    }

    void OnRender( const GameTimer& timer ) override;
};
