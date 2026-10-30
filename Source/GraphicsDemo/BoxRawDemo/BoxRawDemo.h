#pragma once

#include "GraphicsDemo/GraphicsDemo.h"

#include <dxcapi.h>
#include <DirectXMath.h>

class BoxRawDemo final : public GraphicsDemo
{
public:
    explicit BoxRawDemo( const GraphicsDemoCreateInfo& info );
    ~BoxRawDemo();

    void OnUpdate( const Input& input ) override;
    void OnRender() override;

    void OnResize( int32_t width, int32_t height ) override;

private:
    void InitGeometry();
    void InitRootSignature();
    void InitConstantBuffers();
    void InitShaders();
    void InitPipelineStates();

    void RenderImGui();

    void AdjustProjection();

    void ProcessInput( const Input& input );
    void UpdateConstBuffers();

private:
    Microsoft::WRL::ComPtr<IDxcBlob> m_vertexShaderBlob;
    Microsoft::WRL::ComPtr<IDxcBlob> m_pixelShaderBlob;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_psoSolid;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_psoWireframe;

    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;

    Microsoft::WRL::ComPtr<ID3D12Resource> m_objectConstBuffer;
    void*                                  m_objectConstBufferData;

    Microsoft::WRL::ComPtr<ID3D12Resource> m_passConstBuffer;
    void*                                  m_passConstBufferData;

    DirectX::XMFLOAT4X4 m_world;
    DirectX::XMFLOAT4X4 m_projection;

    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW  m_indexBufferView;

    float m_viewPolarAngle = DirectX::XM_PI * 1.25f;
    float m_viewAzimuthalAngle = DirectX::XM_PIDIV4;
    float m_viewRadius = 10.0f;

    bool m_isWireframeEnabled = false;
};
