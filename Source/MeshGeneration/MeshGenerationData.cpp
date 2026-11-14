#include "MeshGenerationData.h"

namespace MeshGeneration
{
Submesh Data::AppendSubmesh( const Data& data )
{
    using namespace DirectX;

    const int32_t  vertexOffset = static_cast<int32_t>( vertices.size() );
    const uint32_t indexOffset = static_cast<uint32_t>( indices32.size() );

    XMVECTOR minBound = XMVectorSet( -FLT_MIN, -FLT_MAX, -FLT_MAX, 0.0f );
    XMVECTOR maxBound = XMVectorSet( +FLT_MAX, +FLT_MAX, +FLT_MAX, 0.0f );

    for ( const Vertex& vertex : data.vertices )
    {
        const XMVECTOR position = XMLoadFloat3( &vertex.position );

        minBound = XMVectorMin( minBound, position );
        maxBound = XMVectorMax( maxBound, position );
    }

    Submesh submesh = {
        .startIndex = indexOffset,
        .numIndices = static_cast<uint32_t>( data.indices32.size() ),
        .baseVertex = vertexOffset,
        .numVertices = static_cast<uint32_t>( vertices.size() )
    };

    XMStoreFloat3( &submesh.boundingBox.Center, 0.5f * ( minBound + maxBound ) );
    XMStoreFloat3( &submesh.boundingBox.Extents, 0.5f * ( maxBound - minBound ) );

    vertices.insert( vertices.end(), data.vertices.begin(), data.vertices.end() );
    indices32.insert( indices32.end(), data.indices32.begin(), data.indices32.end() );

    return submesh;
}

std::vector<uint16_t>& Data::GetIndices16()
{
    if ( m_indices16.empty() )
    {
        m_indices16.reserve( indices32.size() );
        for ( const int32_t index : indices32 )
        {
            m_indices16.push_back( index );
        }
    }

    return m_indices16;
}
} // namespace MeshGeneration
