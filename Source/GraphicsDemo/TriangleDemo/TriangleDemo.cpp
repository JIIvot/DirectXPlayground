#include "TriangleDemo.h"
#include "Utility.h"

CTriangleDemo::CTriangleDemo( HWND hWindow )
    : CGraphicsDemo( hWindow )
{
    InitRootSignature();
    InitShaders();
    InitGeometry();
    InitPipelineStates();
}

void CTriangleDemo::Render()
{
}

void CTriangleDemo::InitRootSignature()
{
    const CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(
        0,                                                              // numParameters
        nullptr,                                                        // _pParameters
        0,                                                              // numStaticSamplers
        nullptr,                                                        // _pStaticSamplers
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT ); // flags

    Microsoft::WRL::ComPtr<ID3DBlob> pSerializedRootSignatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> pErrorBlob;

    ASSERT_SUCCEEDED( D3D12SerializeRootSignature(
        &rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        pSerializedRootSignatureBlob.GetAddressOf(),
        pErrorBlob.GetAddressOf() ) );

#ifdef _DEBUG
    std::puts( static_cast<char*>( pErrorBlob->GetBufferPointer() ) );
#endif

    ASSERT_SUCCEEDED( m_pDevice->CreateRootSignature(
        0,
        pSerializedRootSignatureBlob->GetBufferPointer(),
        pSerializedRootSignatureBlob->GetBufferSize(),
        IID_PPV_ARGS( m_pRootSignature.GetAddressOf() ) ) );
}

void CTriangleDemo::InitShaders()
{
#ifdef _DEBUG
    #define COMMA_DEBUG_ARGS , DXC_ARG_DEBUG, DXC_ARG_SKIP_OPTIMIZATIONS
#else
    #define COMMA_DEBUG_ARGS
#endif

    std::vector<const wchar_t*> vertexShaderArgs = {
        L"-E", L"VS", L"-T", L"vs_6_6" COMMA_DEBUG_ARGS
    };

    std::vector<const wchar_t*> pixelShaderArgs = {
        L"-E", L"PS", L"-T", L"ps_6_6" COMMA_DEBUG_ARGS
    };

    m_pVertexShaderBlob = Utility::CompileShaderFromFile( L"Shaders/BasicColorTriangle.hlsl", vertexShaderArgs.size(), vertexShaderArgs.data() );
    m_pPixelShaderBlob = Utility::CompileShaderFromFile( L"Shaders/BasicColorTriangle.hlsl", pixelShaderArgs.size(), pixelShaderArgs.data() );
}

void CTriangleDemo::InitGeometry()
{
}

void CTriangleDemo::InitPipelineStates()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = Utility::CreateDefaultPsoDesc(
        m_backBufferFormat,
        m_depthStencilFormat,
        m_inputLayout.size(),
        m_inputLayout.data(),
        m_pRootSignature.Get(),
        m_pVertexShaderBlob.Get(),
        m_pPixelShaderBlob.Get() );

    ASSERT_SUCCEEDED( m_pDevice->CreateGraphicsPipelineState( &desc, IID_PPV_ARGS( m_pSolidPso.GetAddressOf() ) ) );

    desc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;

    ASSERT_SUCCEEDED( m_pDevice->CreateGraphicsPipelineState( &desc, IID_PPV_ARGS( m_pWireframePso.GetAddressOf() ) ) );
}
