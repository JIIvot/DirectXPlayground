#pragma once

#include <DirectXMath.h>

namespace MeshGeneration
{
struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT3 tangentU;
    DirectX::XMFLOAT2 textureCoord;
};
} // namespace MeshGeneration
