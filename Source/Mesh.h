#pragma once

#include "Submesh.h"

#include <string>
#include <vector>
#include <d3d12.h>
#include <wrl/client.h>
#include <unordered_map>

class Mesh
{
public:
    [[nodiscard]] D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const
    {
        return { .BufferLocation = vertexBuffer->GetGPUVirtualAddress(), .SizeInBytes = vertexBufferSize, .StrideInBytes = vertexBufferStride };
    }

    [[nodiscard]] D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const
    {
        return { .BufferLocation = indexBuffer->GetGPUVirtualAddress(), .SizeInBytes = indexBufferSize, .Format = indexBufferFormat };
    }

public:
    std::string name;

    std::vector<std::byte> vertexBufferData;
    std::vector<std::byte> indexBufferData;

    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;

    uint32_t    vertexBufferStride = 0;
    uint32_t    vertexBufferSize = 0;
    DXGI_FORMAT indexBufferFormat = DXGI_FORMAT_R16_UINT;
    uint32_t    indexBufferSize = 0;

    std::unordered_map<std::string, Submesh> submeshMap;
};
