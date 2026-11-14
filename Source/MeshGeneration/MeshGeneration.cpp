#include "MeshGeneration.h"

#include <cmath>

namespace MeshGeneration
{
namespace
{
void Subdivide( Data& data )
{
}

[[nodiscard]] Vertex GetAveragedVertex( const Vertex& first, const Vertex& second )
{
    return {};
}

void BuildCylinderTopCap( float topRadius, float height, uint32_t numSlices, uint32_t numStacks, Data& data )
{
}

void BuildCylinderBottomCap( float bottomRadius, float height, uint32_t numSlices, uint32_t numStacks, Data& data )
{
}
} // namespace

Data CreateBox( float width, float height, float depth, uint32_t numSubdivisions )
{
    const float halfWidth = width * 0.5f;
    const float halfHeight = height * 0.5f;
    const float halfDepth = depth * 0.5f;

    Data data;

    data.vertices = {
        // Front face
        { .position = { -halfWidth, -halfHeight, -halfDepth }, .normal = { 0.0f, 0.0f, -1.0f }, .tangentU = { +1.0f, 0.0f, 0.0f }, .textureCoord = { 0.0f, 1.0f } },
        { .position = { -halfWidth, +halfHeight, -halfDepth }, .normal = { 0.0f, 0.0f, -1.0f }, .tangentU = { +1.0f, 0.0f, 0.0f }, .textureCoord = { 0.0f, 0.0f } },
        { .position = { +halfWidth, +halfHeight, -halfDepth }, .normal = { 0.0f, 0.0f, -1.0f }, .tangentU = { +1.0f, 0.0f, 0.0f }, .textureCoord = { 1.0f, 0.0f } },
        { .position = { +halfWidth, -halfHeight, -halfDepth }, .normal = { 0.0f, 0.0f, -1.0f }, .tangentU = { +1.0f, 0.0f, 0.0f }, .textureCoord = { 1.0f, 1.0f } },

        // Back face
        { .position = { -halfWidth, -halfHeight, +halfDepth }, .normal = { 0.0f, 0.0f, +1.0f }, .tangentU = { -1.0f, 0.0f, 0.0f }, .textureCoord = { 1.0f, 1.0f } },
        { .position = { +halfWidth, -halfHeight, +halfDepth }, .normal = { 0.0f, 0.0f, +1.0f }, .tangentU = { -1.0f, 0.0f, 0.0f }, .textureCoord = { 0.0f, 1.0f } },
        { .position = { +halfWidth, +halfHeight, +halfDepth }, .normal = { 0.0f, 0.0f, +1.0f }, .tangentU = { -1.0f, 0.0f, 0.0f }, .textureCoord = { 0.0f, 0.0f } },
        { .position = { -halfWidth, +halfHeight, +halfDepth }, .normal = { 0.0f, 0.0f, +1.0f }, .tangentU = { -1.0f, 0.0f, 0.0f }, .textureCoord = { 1.0f, 0.0f } },

        // Top face
        { .position = { -halfWidth, +halfHeight, -halfDepth }, .normal = { 0.0f, +1.0f, 0.0f }, .tangentU = { +1.0f, 0.0f, 0.0f }, .textureCoord = { 0.0f, 1.0f } },
        { .position = { -halfWidth, +halfHeight, +halfDepth }, .normal = { 0.0f, +1.0f, 0.0f }, .tangentU = { +1.0f, 0.0f, 0.0f }, .textureCoord = { 0.0f, 0.0f } },
        { .position = { +halfWidth, +halfHeight, +halfDepth }, .normal = { 0.0f, +1.0f, 0.0f }, .tangentU = { +1.0f, 0.0f, 0.0f }, .textureCoord = { 1.0f, 0.0f } },
        { .position = { +halfWidth, +halfHeight, -halfDepth }, .normal = { 0.0f, +1.0f, 0.0f }, .tangentU = { +1.0f, 0.0f, 0.0f }, .textureCoord = { 1.0f, 1.0f } },

        // Bottom face
        { .position = { -halfWidth, -halfHeight, -halfDepth }, .normal = { 0.0f, -1.0f, 0.0f }, .tangentU = { -1.0f, 0.0f, 0.0f }, .textureCoord = { 1.0f, 1.0f } },
        { .position = { +halfWidth, -halfHeight, -halfDepth }, .normal = { 0.0f, -1.0f, 0.0f }, .tangentU = { -1.0f, 0.0f, 0.0f }, .textureCoord = { 0.0f, 1.0f } },
        { .position = { +halfWidth, -halfHeight, +halfDepth }, .normal = { 0.0f, -1.0f, 0.0f }, .tangentU = { -1.0f, 0.0f, 0.0f }, .textureCoord = { 0.0f, 0.0f } },
        { .position = { -halfWidth, -halfHeight, +halfDepth }, .normal = { 0.0f, -1.0f, 0.0f }, .tangentU = { -1.0f, 0.0f, 0.0f }, .textureCoord = { 1.0f, 0.0f } },

        // Left face
        { .position = { -halfWidth, -halfHeight, +halfDepth }, .normal = { -1.0f, 0.0f, 0.0f }, .tangentU = { 0.0f, 0.0f, -1.0f }, .textureCoord = { 0.0f, 1.0f } },
        { .position = { -halfWidth, +halfHeight, +halfDepth }, .normal = { -1.0f, 0.0f, 0.0f }, .tangentU = { 0.0f, 0.0f, -1.0f }, .textureCoord = { 0.0f, 0.0f } },
        { .position = { -halfWidth, +halfHeight, -halfDepth }, .normal = { -1.0f, 0.0f, 0.0f }, .tangentU = { 0.0f, 0.0f, -1.0f }, .textureCoord = { 1.0f, 0.0f } },
        { .position = { -halfWidth, -halfHeight, -halfDepth }, .normal = { -1.0f, 0.0f, 0.0f }, .tangentU = { 0.0f, 0.0f, -1.0f }, .textureCoord = { 1.0f, 1.0f } },

        // Right face
        { .position = { +halfWidth, -halfHeight, -halfDepth }, .normal = { +1.0f, 0.0f, 0.0f }, .tangentU = { 0.0f, 0.0f, +1.0f }, .textureCoord = { 0.0f, 1.0f } },
        { .position = { +halfWidth, +halfHeight, -halfDepth }, .normal = { +1.0f, 0.0f, 0.0f }, .tangentU = { 0.0f, 0.0f, +1.0f }, .textureCoord = { 0.0f, 0.0f } },
        { .position = { +halfWidth, +halfHeight, +halfDepth }, .normal = { +1.0f, 0.0f, 0.0f }, .tangentU = { 0.0f, 0.0f, +1.0f }, .textureCoord = { 1.0f, 0.0f } },
        { .position = { +halfWidth, -halfHeight, +halfDepth }, .normal = { +1.0f, 0.0f, 0.0f }, .tangentU = { 0.0f, 0.0f, +1.0f }, .textureCoord = { 1.0f, 1.0f } }
    };

    data.indices32 = {
        // Front face
        0, 1, 2,
        0, 2, 3,

        // Back face
        4, 5, 6,
        4, 6, 7,

        // Top face
        8, 9, 10,
        8, 10, 11,

        // Bottom face
        12, 13, 14,
        12, 14, 15,

        // Left face
        16, 17, 18,
        16, 18, 19,

        // Right face
        20, 21, 22,
        20, 22, 23
    };

    for ( uint32_t i = 0; i < numSubdivisions; ++i )
    {
        Subdivide( data );
    }

    return data;
}

Data CreateSphere( float radius, uint32_t numSlices, uint32_t numStacks )
{
    Data data;

    const uint32_t numVertices = ( numStacks - 1 ) * ( numSlices + 1 ) + 2;
    const uint32_t numIndices = 6 * numSlices * ( numStacks - 1 );

    // Vertices
    const float azimuthalAngleStep = DirectX::XM_PI / static_cast<float>( numStacks );
    const float polarAngleStep = 2.0f * DirectX::XM_PI / static_cast<float>( numSlices );

    std::vector<Vertex>& vertices = data.vertices;
    vertices.reserve( numVertices );

    // Top vertex
    vertices.push_back( { .position = { 0.0f, radius, 0.0f }, .normal = { 0.0f, 1.0f, 0.0f }, .tangentU = { 1.0f, 0.0f, 0.0f }, .textureCoord = { 0.0f, 0.0f } } );

    for ( uint32_t stack = 1; stack < numStacks; ++stack )
    {
        const float azimuthalAngle = azimuthalAngleStep * static_cast<float>( stack );

        for ( uint32_t slice = 0; slice <= numSlices; ++slice )
        {
            const float polarAngle = polarAngleStep * static_cast<float>( slice );

            Vertex vertex;

            vertex.position = {
                .x = radius * std::sin( azimuthalAngle ) * std::cos( polarAngle ),
                .y = radius * std::cos( azimuthalAngle ),
                .z = radius * std::sin( azimuthalAngle ) * std::sin( polarAngle )
            };

            vertex.tangentU = {
                .x = -radius * std::sin( azimuthalAngle ) * std::sin( polarAngle ),
                .y = 0.0f,
                .z = +radius * std::sin( azimuthalAngle ) * std::cos( polarAngle )
            };

            // Normalize tangentU
            DirectX::XMStoreFloat3( &vertex.tangentU, DirectX::XMVector3Normalize( DirectX::XMLoadFloat3( &vertex.tangentU ) ) );

            // Calculate normal
            DirectX::XMStoreFloat3( &vertex.normal, DirectX::XMVector3Normalize( DirectX::XMLoadFloat3( &vertex.position ) ) );

            vertex.textureCoord = { polarAngle / DirectX::XM_2PI, azimuthalAngle / DirectX::XM_PI };

            vertices.push_back( vertex );
        }
    }

    // Bottom vertex
    vertices.push_back( { .position = { 0.0f, -radius, 0.0f }, .normal = { 0.0f, -1.0f, 0.0f }, .tangentU = { 1.0f, 0.0f, 0.0f }, .textureCoord = { 0.0f, 1.0f } } );

    // Indices
    std::vector<uint32_t> indices = data.indices32;
    indices.reserve( numIndices );

    for ( uint32_t i = 1; i <= numSlices; ++i )
    {
        indices.push_back( 0 );
        indices.push_back( i + 1 );
        indices.push_back( i );
    }

    uint32_t       baseIndex = 1;
    const uint32_t numRingVertices = numSlices + 1;

    for ( uint32_t stack = 0; stack < numStacks - 2; ++stack )
    {
        for ( uint32_t slice = 0; slice < numSlices; ++slice )
        {
            indices.push_back( baseIndex + stack * numRingVertices + slice );
            indices.push_back( baseIndex + stack * numRingVertices + slice + 1 );
            indices.push_back( baseIndex + ( stack + 1 ) * numRingVertices + slice );

            indices.push_back( baseIndex + ( stack + 1 ) * numRingVertices + slice );
            indices.push_back( baseIndex + stack * numRingVertices + slice + 1 );
            indices.push_back( baseIndex + ( stack + 1 ) * numRingVertices + slice + 1 );
        }
    }

    const uint32_t southPoleIndex = numVertices - 1;
    baseIndex = southPoleIndex - numRingVertices;

    for ( uint32_t slice = 0; slice < numSlices; ++slice )
    {
        indices.push_back( southPoleIndex );
        indices.push_back( baseIndex + slice );
        indices.push_back( baseIndex + slice + 1 );
    }

    return data;
}

Data CreateCylinder( float bottomRadius, float topRadius, float height, uint32_t numSlices, uint32_t numStacks )
{
    return {};
}

Data CreateGrid( float width, float depth, uint32_t numRows, uint32_t numColumns )
{
    return {};
}

Data CreateQuad( float x, float y, float z, float h, float depth )
{
    return {};
}
} // namespace MeshGeneration
