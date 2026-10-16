#pragma once

#include <wrl.h>
#include <d3dx12.h>
#include <cstdint>

class CDescriptorHeapManager
{
public:
	CDescriptorHeapManager() = default;

	CDescriptorHeapManager( const CDescriptorHeapManager& ) = delete;

	CDescriptorHeapManager& operator=( const CDescriptorHeapManager& ) = delete;

	void Init( ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t capacity );

	[[nodiscard]]
	ID3D12DescriptorHeap* GetHeap() const
	{
		return m_heap.Get();
	}

	[[nodiscard]]
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuHandle( uint32_t index ) const
	{
		return CD3DX12_GPU_DESCRIPTOR_HANDLE( m_heap->GetGPUDescriptorHandleForHeapStart(), index * m_descriptorSize );
	}

	[[nodiscard]]
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuHandle( uint32_t index ) const
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE( m_heap->GetCPUDescriptorHandleForHeapStart(), index * m_descriptorSize );
	}

protected:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;
	uint32_t m_descriptorSize = 0;
};
