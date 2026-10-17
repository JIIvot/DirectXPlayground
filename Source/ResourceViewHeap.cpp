#include "ResourceViewHeap.h"
#include "Utility.h"

#include <cassert>

void CResourceViewHeap::Init( ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t capacity )
{
	assert( !m_heap );

	const D3D12_DESCRIPTOR_HEAP_DESC desc = {
		.Type = type,
		.NumDescriptors = capacity,
		.Flags =
			type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER
			? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
			: D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		.NodeMask = 0
	};

	ASSERT_SUCCEEDED( device->CreateDescriptorHeap( &desc, IID_PPV_ARGS( m_heap.GetAddressOf() ) ) );

	m_descriptorSize = device->GetDescriptorHandleIncrementSize( type );
}
