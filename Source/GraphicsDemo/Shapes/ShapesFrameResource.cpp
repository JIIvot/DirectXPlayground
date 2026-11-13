#include "ShapesFrameResource.h"
#include "Utility.h"

namespace ShapesDemo
{
FrameResource::FrameResource( ID3D12Device* device, uint32_t numPassConstants )
    : passConstBuffer( device, numPassConstants )
{
    ASSERT_SUCCEEDED( device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( commandAllocator.GetAddressOf() ) ) );
}
} // namespace ShapesDemo
