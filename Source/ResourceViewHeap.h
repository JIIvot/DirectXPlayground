#pragma once

#include <wrl/client.h>
#include <d3dx12.h>
#include <cstdint>

class ResourceViewHeap
{
public:
    ResourceViewHeap( ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t capacity );

    ResourceViewHeap( const ResourceViewHeap& ) = delete;
    ResourceViewHeap& operator=( const ResourceViewHeap& ) = delete;

    ResourceViewHeap( ResourceViewHeap&& ) = delete;
    ResourceViewHeap& operator=( ResourceViewHeap&& ) = delete;

    [[nodiscard]] ID3D12DescriptorHeap* GetHeap() const
    {
        return m_heap.Get();
    }

    [[nodiscard]] uint32_t GetDescriptorSize() const
    {
        return m_descriptorSize;
    }

    [[nodiscard]] CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuHandle( uint32_t index = 0 ) const
    {
        return CD3DX12_GPU_DESCRIPTOR_HANDLE( m_heap->GetGPUDescriptorHandleForHeapStart(), static_cast<int32_t>( index ), m_descriptorSize );
    }

    [[nodiscard]] CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuHandle( uint32_t index = 0 ) const
    {
        return CD3DX12_CPU_DESCRIPTOR_HANDLE( m_heap->GetCPUDescriptorHandleForHeapStart(), static_cast<int32_t>( index ), m_descriptorSize );
    }

    [[nodiscard]] uint32_t GetIndexFromCpuHandle( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle ) const
    {
        return static_cast<uint32_t>( ( cpuHandle.ptr - GetCpuHandle().ptr ) ) / m_descriptorSize;
    }

    [[nodiscard]] uint32_t GetIndexFromGpuHandle( D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle ) const
    {
        return static_cast<uint32_t>( ( gpuHandle.ptr - GetGpuHandle().ptr ) ) / m_descriptorSize;
    }

protected:
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;
    uint32_t                                     m_descriptorSize;
};
