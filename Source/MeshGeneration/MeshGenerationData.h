#pragma once

#include "MeshGeneratorVertex.h"
#include "Submesh.h"

#include <vector>

namespace MeshGeneration
{
class Data
{
public:
    Submesh AppendSubmesh( const Data& data );

    [[nodiscard]] std::vector<uint16_t>& GetIndices16();

public:
    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices32;

private:
    std::vector<uint16_t> m_indices16;
};
} // namespace MeshGeneration
