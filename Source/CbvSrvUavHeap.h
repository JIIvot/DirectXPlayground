#pragma once

#include "ResourceViewHeap.h"

#include <vector>

class CbvSrvUavHeap final : public ResourceViewHeap
{
public:
    CbvSrvUavHeap( ID3D12Device* device, uint32_t capacity );

    [[nodiscard]] uint32_t GetNextIndex();

    void ReleaseIndex( uint32_t index )
    {
        m_freeIndices.push_back( index );
    }

private:
    std::vector<uint32_t> m_freeIndices;
};
