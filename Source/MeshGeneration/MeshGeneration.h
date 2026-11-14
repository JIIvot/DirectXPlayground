#pragma once

#include "MeshGenerationData.h"

namespace MeshGeneration
{
[[nodiscard]] Data CreateBox( float width, float height, float depth, uint32_t numSubdivisions );

[[nodiscard]] Data CreateSphere( float radius, uint32_t numSlices, uint32_t numStacks );

[[nodiscard]] Data CreateCylinder( float bottomRadius, float topRadius, float height, uint32_t numSlices, uint32_t numStacks );

[[nodiscard]] Data CreateGrid( float width, float depth, uint32_t numRows, uint32_t numColumns );

[[nodiscard]] Data CreateQuad( float x, float y, float z, float h, float depth );
} // namespace MeshGeneration
