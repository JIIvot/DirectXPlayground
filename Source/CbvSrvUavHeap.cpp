#include "CbvSrvUavHeap.h"

#include <cassert>

void CCbvSrvUavHeap::Init( ID3D12Device* device, uint32_t capacity )
{
    CResourceViewHeap::Init( device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, capacity );

    m_freeIndices.reserve( capacity );
    for ( uint32_t i = 0; i < capacity; ++i )
    {
        m_freeIndices.push_back( i );
    }
}

uint32_t CCbvSrvUavHeap::GetNextIndex()
{
    assert( !m_freeIndices.empty() );

    const uint32_t index = m_freeIndices.back();
    m_freeIndices.pop_back();

    return index;
}
