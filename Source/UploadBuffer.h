#pragma once

#include "Utility.h"

#include <d3dx12.h>
#include <wrl/client.h>
#include <cstdint>

template<typename T, bool isConstBuffer = false>
class UploadBuffer
{
public:
    UploadBuffer( ID3D12Device* device, uint32_t capacity );

    ~UploadBuffer()
    {
        m_bufferResource->Unmap( 0, nullptr );
    }

    UploadBuffer( const UploadBuffer& ) = delete;
    UploadBuffer& operator=( const UploadBuffer& ) = delete;

    UploadBuffer( UploadBuffer&& ) = delete;
    UploadBuffer& operator=( UploadBuffer&& ) = delete;

    [[nodiscard]] ID3D12Resource* GetResource() const
    {
        return m_bufferResource.Get();
    }

    void SetElement( uint32_t index, const T& data )
    {
        memcpy( m_data + index * kElementSize, &data, sizeof( T ) );
    }

    void CopyData( uint32_t numElements, const T* elements )
        requires !isConstBuffer
    {
        memcpy( m_data, elements, numElements * sizeof( T ) );
    }

private:
    static constexpr uint32_t kElementSize = isConstBuffer ? Utility::GetConstBufferAlignedSize<T>() : sizeof( T );

    Microsoft::WRL::ComPtr<ID3D12Resource> m_bufferResource;
    std::byte*                             m_data;
};

template<typename T, bool isConstBuffer>
UploadBuffer<T, isConstBuffer>::UploadBuffer( ID3D12Device* device, uint32_t capacity )
{
    ASSERT_SUCCEEDED( device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer( kElementSize * capacity ),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS( m_bufferResource.GetAddressOf() ) ) );

    ASSERT_SUCCEEDED( m_bufferResource->Map( 0, nullptr, reinterpret_cast<void**>( &m_data ) ) );
}
