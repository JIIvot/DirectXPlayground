#pragma once

#include "ShapesObjectConstants.h"
#include "Mesh.h"
#include "Material.h"

#include <DirectXMath.h>
#include <GraphicsMemory.h>

namespace ShapesDemo
{
struct RenderItem
{
    D3D_PRIMITIVE_TOPOLOGY primitiveTopology;
    DirectX::XMFLOAT4X4    world;
    DirectX::XMFLOAT4X4    textureTransform;

    ObjectConstants           objectConstants;
    DirectX::GraphicsResource objectConstBufferHandle;
    Mesh*                     mesh;
    Material*                 material;

    uint32_t numIndices;
    uint32_t baseIndex;
    int32_t  baseVertex;
};
} // namespace ShapesDemo
