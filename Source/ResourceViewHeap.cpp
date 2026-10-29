#include "ResourceViewHeap.h"
#include "Utility.h"

ResourceViewHeap::ResourceViewHeap( ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t capacity )
    : m_descriptorSize( device->GetDescriptorHandleIncrementSize( type ) )
{
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = type;
    desc.NumDescriptors = capacity;
    desc.Flags =
        type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER
        ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
        : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    ASSERT_SUCCEEDED( device->CreateDescriptorHeap( &desc, IID_PPV_ARGS( m_heap.GetAddressOf() ) ) );
}
