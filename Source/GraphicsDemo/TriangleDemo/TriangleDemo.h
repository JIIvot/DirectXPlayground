#pragma once

#include "GraphicsDemo/GraphicsDemo.h"

#include <dxcapi.h>
#include <vector>

class CTriangleDemo final : public CGraphicsDemo
{
public:
    explicit CTriangleDemo( HWND hWindow );

    void Render() override;

private:
    void InitRootSignature();
    void InitShaders();
    void InitGeometry();
    void InitPipelineStates();

private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pRootSignature;

    Microsoft::WRL::ComPtr<IDxcBlob> m_pVertexShaderBlob;
    Microsoft::WRL::ComPtr<IDxcBlob> m_pPixelShaderBlob;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pSolidPso;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pWireframePso;

    std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    bool m_isWireframeEnabled = false;
};
