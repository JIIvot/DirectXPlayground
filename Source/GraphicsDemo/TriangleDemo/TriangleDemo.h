#pragma once

#include "GraphicsDemo/GraphicsDemo.h"

#include <dxcapi.h>
#include <vector>

class CTriangleDemo final : public CGraphicsDemo
{
public:
    explicit CTriangleDemo( HWND windowHandle );

    void Update() override
    {
    }

    void Render() override;

private:
    void InitRootSignature();
    void InitShaders();
    void InitGeometry();
    void InitPipelineStates();

private:
    static constexpr D3D12_INPUT_ELEMENT_DESC kInputLayout[2] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

    Microsoft::WRL::ComPtr<IDxcBlob> m_vertexShaderBlob;
    Microsoft::WRL::ComPtr<IDxcBlob> m_pixelShaderBlob;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_psoSolid;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_psoWireframe;

    bool m_isWireframeEnabled = false;
};
