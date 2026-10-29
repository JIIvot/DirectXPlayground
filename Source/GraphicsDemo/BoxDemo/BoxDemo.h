#pragma once

#include "GraphicsDemo/GraphicsDemo.h"

#include <dxcapi.h>
#include <DirectXMath.h>

class BoxDemo final : public GraphicsDemo
{
public:
    explicit BoxDemo( const GraphicsDemoCreateInfo& info );
    ~BoxDemo();

    void OnUpdate() override;
    void OnRender() override;

    void OnResize( int32_t width, int32_t height ) override;

private:
    void InitGeometry();
    void InitRootSignature();
    void InitConstantBuffers();
    void InitShaders();
    void InitPipelineStates();

    void RenderImGui();

    void UpdateProjectionMatrix();

private:
    Microsoft::WRL::ComPtr<IDxcBlob> m_vertexShaderBlob;
    Microsoft::WRL::ComPtr<IDxcBlob> m_pixelShaderBlob;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_psoSolid;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_psoWireframe;

    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;

    Microsoft::WRL::ComPtr<ID3D12Resource> m_cbufferObject;
    void*                                  m_cbufferObjectData;

    Microsoft::WRL::ComPtr<ID3D12Resource> m_cbufferPass;
    void*                                  m_cbufferPassData;

    DirectX::XMFLOAT4X4 m_world;
    DirectX::XMFLOAT4X4 m_projection;

    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW  m_indexBufferView;

    bool m_isWireframeEnabled = false;
};
