#pragma once

#include "GraphicsDemo/GraphicsDemo.h"
#include "ResourceViewHeap.h"

#include <dxcapi.h>

namespace RenderTextureDemo
{
class Demo final : public GraphicsDemo
{
public:
    explicit Demo( const GraphicsDemoCreateInfo& info );

    void OnRender( const GameTimer& timer ) override;

private:
    void InitRenderTexture();
    void InitShaders();
    void InitRootSignature();
    void InitPipelineState();

private:
    Microsoft::WRL::ComPtr<IDxcBlob> m_vertexShaderBlob;
    Microsoft::WRL::ComPtr<IDxcBlob> m_pixelShaderBlob;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;

    Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTexture;
    uint32_t                               m_srvRenderTextureIndex;
};
} // namespace RenderTextureDemo
