#pragma once

#include <DirectXCollision.h>

struct Submesh
{
    uint32_t             startIndex;
    uint32_t             numIndices;
    int32_t              baseVertex;
    uint32_t             numVertices;
    DirectX::BoundingBox boundingBox;
};
