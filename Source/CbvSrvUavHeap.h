#pragma once

#include "ResourceViewHeap.h"

#include <queue>

class CCbvSrvUavHeap final : public CResourceViewHeap
{
public:
	void Init( ID3D12Device* pDevice, uint32_t capacity );

	[[nodiscard]]
	uint32_t GetNextIndex();
	void ReleaseIndex( uint32_t index );

private:
	std::queue<uint32_t> m_freeIndexQueue;
};
