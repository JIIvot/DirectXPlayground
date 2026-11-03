#pragma once

#include "GraphicsDemo/GraphicsDemo.h"
#include "UploadBuffer.h"
#include "Mesh.h"

#include <dxcapi.h>
#include <DirectXMath.h>

class BoxDemoEx3 final : public GraphicsDemo
{
private:
    struct ObjectConstants
    {
        DirectX::XMFLOAT4X4 world;
    };

    struct PassConstants
    {
        DirectX::XMFLOAT4X4 viewProjection;
    };

    enum PrimitiveTopology : uint8_t
    {
        ePrimitiveTopology_PointList,
        ePrimitiveTopology_LineList,
        ePrimitiveTopology_LineStrip,
        ePrimitiveTopology_TriangleList,
        ePrimitiveTopology_TriangleStrip,
        ePrimitiveTopology_Count
    };

public:
    explicit BoxDemoEx3( const GraphicsDemoCreateInfo& info );

    void OnUpdate( const Input& input ) override;
    void OnRender() override;

    void OnResize( int32_t width, int32_t height ) override;

private:
    static D3D12_PRIMITIVE_TOPOLOGY_TYPE GetD3D12PrimitiveTopologyType( PrimitiveTopology topology );
    static D3D_PRIMITIVE_TOPOLOGY        GetD3DPrimitiveTopology( PrimitiveTopology topology );

    void InitGeometry();
    void InitRootSignature();
    void InitShaders();
    void InitPipelineStates();

    void RenderImGui();

    void AdjustProjection();

    void ProcessInput( const Input& input );
    void UpdateConstBuffers();

private:
    Microsoft::WRL::ComPtr<IDxcBlob>            m_vertexShaderBlob;
    Microsoft::WRL::ComPtr<IDxcBlob>            m_pixelShaderBlob;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    UploadBuffer<ObjectConstants, true>         m_objectConstBuffer;
    UploadBuffer<PassConstants, true>           m_passConstBuffer;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineStates[ePrimitiveTopology_Count];

    Mesh m_mesh = {};

    DirectX::XMFLOAT4X4 m_world;
    DirectX::XMFLOAT4X4 m_projection;

    float m_viewPolarAngle = DirectX::XM_PI * 1.25f;
    float m_viewAzimuthalAngle = DirectX::XM_PIDIV4;
    float m_viewRadius = 10.0f;

    PrimitiveTopology m_primitiveTopology = ePrimitiveTopology_PointList;
    bool              m_isWireframeEnabled = false;
};
