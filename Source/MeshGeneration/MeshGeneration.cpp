#include "MeshGeneration.h"

#include <cmath>

namespace MeshGeneration
{
namespace
{
[[nodiscard]] Vertex GetAveragedVertex( const Vertex& vertex0, const Vertex& vertex1 )
{
    using namespace DirectX;

    const XMVECTOR position0 = XMLoadFloat3( &vertex0.position );
    const XMVECTOR normal0 = XMLoadFloat3( &vertex0.normal );
    const XMVECTOR tangentU0 = XMLoadFloat3( &vertex0.tangentU );
    const XMVECTOR textureCoord0 = XMLoadFloat2( &vertex0.textureCoord );

    const XMVECTOR position1 = XMLoadFloat3( &vertex1.position );
    const XMVECTOR normal1 = XMLoadFloat3( &vertex1.normal );
    const XMVECTOR tangentU1 = XMLoadFloat3( &vertex1.tangentU );
    const XMVECTOR textureCoord1 = XMLoadFloat2( &vertex1.textureCoord );

    const XMVECTOR newPosition = 0.5f * ( position0 + position1 );
    const XMVECTOR newNormal = XMVector3Normalize( 0.5f * ( normal0 + normal1 ) );
    const XMVECTOR newTangentU = XMVector3Normalize( 0.5f * ( tangentU0 + tangentU1 ) );
    const XMVECTOR newTextureCoord = 0.5f * ( textureCoord0 + textureCoord1 );

    Vertex newVertex;
    XMStoreFloat3( &newVertex.position, newPosition );
    XMStoreFloat3( &newVertex.normal, newNormal );
    XMStoreFloat3( &newVertex.tangentU, newTangentU );
    XMStoreFloat2( &newVertex.textureCoord, newTextureCoord );

    return newVertex;
}

void Subdivide( Data& data )
{
    const Data inputData = data;

    data.vertices.clear();
    data.vertices.reserve( inputData.vertices.size() * 2 );

    data.indices32.clear();
    data.indices32.reserve( inputData.indices32.size() * 4 );

    for ( uint32_t triangleIndex = 0; triangleIndex < inputData.vertices.size() / 3; ++triangleIndex )
    {
        const Vertex& vertex0 = inputData.vertices[inputData.indices32[triangleIndex * 3 + 0]];
        const Vertex& vertex1 = inputData.vertices[inputData.indices32[triangleIndex * 3 + 1]];
        const Vertex& vertex2 = inputData.vertices[inputData.indices32[triangleIndex * 3 + 2]];

        data.vertices.push_back( vertex0 );
        data.vertices.push_back( vertex1 );
        data.vertices.push_back( vertex2 );
        data.vertices.push_back( GetAveragedVertex( vertex0, vertex1 ) );
        data.vertices.push_back( GetAveragedVertex( vertex1, vertex2 ) );
        data.vertices.push_back( GetAveragedVertex( vertex0, vertex2 ) );

        data.indices32.push_back( triangleIndex * 6 + 0 );
        data.indices32.push_back( triangleIndex * 6 + 3 );
        data.indices32.push_back( triangleIndex * 6 + 5 );

        data.indices32.push_back( triangleIndex * 6 + 3 );
        data.indices32.push_back( triangleIndex * 6 + 4 );
        data.indices32.push_back( triangleIndex * 6 + 5 );

        data.indices32.push_back( triangleIndex * 6 + 5 );
        data.indices32.push_back( triangleIndex * 6 + 4 );
        data.indices32.push_back( triangleIndex * 6 + 2 );

        data.indices32.push_back( triangleIndex * 6 + 3 );
        data.indices32.push_back( triangleIndex * 6 + 1 );
        data.indices32.push_back( triangleIndex * 6 + 4 );
    }
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

    data.vertices.reserve( numVertices );

    // Top vertex
    data.vertices.push_back( { .position = { 0.0f, radius, 0.0f }, .normal = { 0.0f, 1.0f, 0.0f }, .tangentU = { 1.0f, 0.0f, 0.0f }, .textureCoord = { 0.0f, 0.0f } } );

    for ( uint32_t stack = 1; stack < numStacks; ++stack )
    {
        const float azimuthalAngle = azimuthalAngleStep * static_cast<float>( stack );

        for ( uint32_t slice = 0; slice <= numSlices; ++slice )
        {
            const float polarAngle = polarAngleStep * static_cast<float>( slice );

            Vertex vertex;

            vertex.position = {
                radius * std::sin( azimuthalAngle ) * std::cos( polarAngle ),
                radius * std::cos( azimuthalAngle ),
                radius * std::sin( azimuthalAngle ) * std::sin( polarAngle )
            };

            vertex.tangentU = {
                -radius * std::sin( azimuthalAngle ) * std::sin( polarAngle ),
                0.0f,
                +radius * std::sin( azimuthalAngle ) * std::cos( polarAngle )
            };

            // Normalize tangentU
            DirectX::XMStoreFloat3( &vertex.tangentU, DirectX::XMVector3Normalize( DirectX::XMLoadFloat3( &vertex.tangentU ) ) );

            // Calculate normal
            DirectX::XMStoreFloat3( &vertex.normal, DirectX::XMVector3Normalize( DirectX::XMLoadFloat3( &vertex.position ) ) );

            vertex.textureCoord = { polarAngle / DirectX::XM_2PI, azimuthalAngle / DirectX::XM_PI };

            data.vertices.push_back( vertex );
        }
    }

    // Bottom vertex
    data.vertices.push_back( { .position = { 0.0f, -radius, 0.0f }, .normal = { 0.0f, -1.0f, 0.0f }, .tangentU = { 1.0f, 0.0f, 0.0f }, .textureCoord = { 0.0f, 1.0f } } );

    // Indices
    data.indices32.reserve( numIndices );

    for ( uint32_t i = 1; i <= numSlices; ++i )
    {
        data.indices32.push_back( 0 );
        data.indices32.push_back( i + 1 );
        data.indices32.push_back( i );
    }

    uint32_t       baseIndex = 1;
    const uint32_t numRingVertices = numSlices + 1;

    for ( uint32_t stack = 0; stack < numStacks - 2; ++stack )
    {
        for ( uint32_t slice = 0; slice < numSlices; ++slice )
        {
            data.indices32.push_back( baseIndex + stack * numRingVertices + slice );
            data.indices32.push_back( baseIndex + stack * numRingVertices + slice + 1 );
            data.indices32.push_back( baseIndex + ( stack + 1 ) * numRingVertices + slice );

            data.indices32.push_back( baseIndex + ( stack + 1 ) * numRingVertices + slice );
            data.indices32.push_back( baseIndex + stack * numRingVertices + slice + 1 );
            data.indices32.push_back( baseIndex + ( stack + 1 ) * numRingVertices + slice + 1 );
        }
    }

    const uint32_t southPoleIndex = numVertices - 1;
    baseIndex = southPoleIndex - numRingVertices;

    for ( uint32_t slice = 0; slice < numSlices; ++slice )
    {
        data.indices32.push_back( southPoleIndex );
        data.indices32.push_back( baseIndex + slice );
        data.indices32.push_back( baseIndex + slice + 1 );
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
