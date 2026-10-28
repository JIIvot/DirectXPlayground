#pragma once

#include <d3dx12.h>
#include <cstdint>
#include <wrl/client.h>

#include "Utility.h"

template<typename T, bool isCBuffer>
class CUploadBuffer
{
public:
    CUploadBuffer( ID3D12Device* device, uint32_t numElements );
    ~CUploadBuffer();

    CUploadBuffer( const CUploadBuffer& ) = delete;
    CUploadBuffer& operator=( const CUploadBuffer& ) = delete;

    CUploadBuffer( CUploadBuffer&& ) = delete;
    CUploadBuffer& operator=( CUploadBuffer&& ) = delete;

    [[nodiscard]]
    ID3D12Resource* GetResource() const
    {
        return m_buffer.Get();
    }

    void SetElement( uint32_t index, const T& data ) const
    {
        memcpy( &m_mappedData[m_elementSizeBytes * index], &data, sizeof( T ) );
    }

    void CopyData( const T* data, uint32_t numElements ) const
        requires( !isCBuffer )
    {
        memcpy( m_mappedData, data, numElements * sizeof( T ) );
    }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_buffer;
    void*                                  m_mappedData = nullptr;
    uint32_t                               m_elementSizeBytes = isCBuffer ? Utility::GetCBufferAlignedSizeBytes( sizeof( T ) ) : sizeof( T );
};

template<typename T, bool isCBuffer>
CUploadBuffer<T, isCBuffer>::CUploadBuffer( ID3D12Device* device, uint32_t numElements )
{
    ASSERT_SUCCEEDED( device->CreateCommitedResource(
        &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer( m_elementSizeBytes * numElements ),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS( m_buffer.GetAddressOf() ) ) );

    ASSERT_SUCCEEDED( m_buffer->Map( 0, nullptr, &m_mappedData ) );
}

template<typename T, bool isCBuffer>
CUploadBuffer<T, isCBuffer>::~CUploadBuffer()
{
    if ( m_buffer )
    {
        m_buffer->Unmap( 0, nullptr );
    }
}
