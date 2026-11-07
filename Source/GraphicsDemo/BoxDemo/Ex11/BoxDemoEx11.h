#pragma once

#include "GraphicsDemo/GraphicsDemo.h"
#include "UploadBuffer.h"
#include "Mesh.h"
#include "GraphicsDemo/BoxDemo/ObjectConstants.h"
#include "GraphicsDemo/BoxDemo/PassConstants.h"

#include <dxcapi.h>
#include <DirectXMath.h>

class BoxDemoEx11 final : public GraphicsDemo
{
public:
    explicit BoxDemoEx11( const GraphicsDemoCreateInfo& info );

    void OnUpdate( const GameTimer& timer, const Input& input ) override;
    void OnRender( const GameTimer& timer ) override;

    void OnResize( int32_t width, int32_t height ) override;

private:
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
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_psoSolid;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_psoWireframe;
    UploadBuffer<ObjectConstants, true>         m_objectConstBuffer;
    UploadBuffer<PassConstants, true>           m_passConstBuffer;

    Mesh m_mesh = {};

    DirectX::XMFLOAT4X4 m_world;
    DirectX::XMFLOAT4X4 m_projection;

    float m_viewPolarAngle = DirectX::XM_PI * 1.25f;
    float m_viewAzimuthalAngle = DirectX::XM_PIDIV4;
    float m_viewRadius = 10.0f;

    bool m_isWireframeEnabled = false;
};
