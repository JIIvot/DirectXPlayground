#pragma once

#include "Submesh.h"

#include <string>
#include <vector>
#include <d3d12.h>
#include <wrl/client.h>
#include <unordered_map>

struct Mesh
{
    std::string name;

    std::vector<std::byte> vertexBufferData;
    std::vector<std::byte> indexBufferData;

    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;

    uint32_t    vertexBufferStride;
    uint32_t    vertexBufferSize;
    DXGI_FORMAT indexBufferFormat;
    uint32_t    indexBufferSize;

    std::unordered_map<std::string, Submesh> submeshMap;

    [[nodiscard]] D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const
    {
        return { .BufferLocation = vertexBuffer->GetGPUVirtualAddress(), .SizeInBytes = vertexBufferSize, .StrideInBytes = vertexBufferStride };
    }

    [[nodiscard]] D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const
    {
        return { .BufferLocation = indexBuffer->GetGPUVirtualAddress(), .SizeInBytes = indexBufferSize, .Format = indexBufferFormat };
    }
};
