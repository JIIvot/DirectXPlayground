#pragma once

#include "GraphicsDemo/GraphicsDemo.h"

namespace ClearScreenDemo
{
class Demo final : public GraphicsDemo
{
public:
    explicit Demo( const GraphicsDemoCreateInfo& info )
        : GraphicsDemo( info )
    {
    }

    void OnRender( const GameTimer& timer ) override;
};
} // namespace Demo
