#include "CbvSrvUavHeap.h"

#include <cassert>

void CCbvSrvUavHeap::Init( ID3D12Device* pDevice, uint32_t capacity )
{
    CResourceViewHeap::Init( pDevice, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, capacity );

    for ( uint32_t i = 0; i < capacity; ++i )
    {
        m_freeIndexQueue.push( i );
    }
}

uint32_t CCbvSrvUavHeap::GetNextIndex()
{
    assert( !m_freeIndexQueue.empty() );

    const uint32_t index = m_freeIndexQueue.front();
    m_freeIndexQueue.pop();

    return index;
}

void CCbvSrvUavHeap::ReleaseIndex( uint32_t index )
{
    m_freeIndexQueue.push( index );
}
