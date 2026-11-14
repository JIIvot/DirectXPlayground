#pragma once

#include <cstdint>

namespace ShapesDemo
{
enum RenderLayer : uint8_t
{
    eRenderLayer_Opaque,
    eRenderLayer_Debug,
    eRenderLayer_Sky,
    eRenderLayer_Count
};
}
