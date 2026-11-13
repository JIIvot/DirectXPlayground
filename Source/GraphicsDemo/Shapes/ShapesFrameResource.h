#pragma once

#include "ShapesPassConstants.h"
#include "UploadBuffer.h"

#include <wrl/client.h>
#include <d3d12.h>

namespace ShapesDemo
{
class FrameResource
{
public:
    FrameResource( ID3D12Device* device, uint32_t numPassConstants );
    ~FrameResource() = default;

    FrameResource( const FrameResource& ) = delete;
    FrameResource& operator=( const FrameResource& ) = delete;

    FrameResource( FrameResource&& ) = delete;
    FrameResource& operator=( FrameResource&& ) = delete;

public:
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
    UploadBuffer<PassConstants, true>              passConstBuffer;
    uint64_t                                       fenceValue = 0;
};
} // namespace ShapesDemo
