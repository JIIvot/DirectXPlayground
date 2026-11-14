#include "MeshGenerationData.h"

namespace MeshGeneration
{
Submesh Data::AppendSubmesh( const Data& data )
{
    const int32_t  vertexOffset = static_cast<int32_t>( vertices.size() );
    const uint32_t indexOffset = static_cast<uint32_t>( indices32.size() );

    DirectX::XMVECTOR minBound = DirectX::XMVectorSet( -FLT_MIN, -FLT_MAX, -FLT_MAX, 0.0f );
    DirectX::XMVECTOR maxBound = DirectX::XMVectorSet( FLT_MAX, FLT_MAX, FLT_MAX, 0.0f );

    for ( const Vertex& vertex : data.vertices )
    {
        const DirectX::XMVECTOR position = DirectX::XMLoadFloat3( &vertex.position );

        minBound = DirectX::XMVectorMin( minBound, position );
        maxBound = DirectX::XMVectorMax( maxBound, position );
    }

    Submesh submesh = {
        .startIndex = indexOffset,
        .numIndices = static_cast<uint32_t>( data.indices32.size() ),
        .baseVertex = vertexOffset,
        .numVertices = static_cast<uint32_t>( vertices.size() )
    };

    DirectX::XMStoreFloat3( &submesh.boundingBox.Center, DirectX::XMVectorScale( DirectX::XMVectorAdd( minBound, maxBound ), 0.5f ) );
    DirectX::XMStoreFloat3( &submesh.boundingBox.Extents, DirectX::XMVectorScale( DirectX::XMVectorSubtract( maxBound, minBound ), 0.5f ) );

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
