#pragma once

#include "GraphicsDemo/GraphicsDemo.h"

#include <dxcapi.h>
#include <vector>

class CTriangleDemo final : public CGraphicsDemo
{
public:
    CTriangleDemo( HWND windowHandle, IDXGIFactory6* dxgiFactory, ID3D12Device5* device );

    void OnUpdate() override;
    void OnRender() override;

private:
    void InitRootSignature();
    void InitShaders();
    void InitGeometry();
    void InitPipelineStates();

    void RenderImGui();

private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

    Microsoft::WRL::ComPtr<IDxcBlob> m_vertexShaderBlob;
    Microsoft::WRL::ComPtr<IDxcBlob> m_pixelShaderBlob;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_psoSolid;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_psoWireframe;

    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;

    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW  m_indexBufferView;

    bool m_isWireframeEnabled = false;
};
