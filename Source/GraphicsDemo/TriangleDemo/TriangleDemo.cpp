#include "TriangleDemo.h"
#include "Utility.h"

CTriangleDemo::CTriangleDemo( HWND windowHandle )
    : CGraphicsDemo( windowHandle )
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

    Microsoft::WRL::ComPtr<ID3DBlob> serializedBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    ASSERT_SUCCEEDED( D3D12SerializeRootSignature(
        &rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        serializedBlob.GetAddressOf(),
        errorBlob.GetAddressOf() ) );

#ifdef _DEBUG
    std::puts( static_cast<char*>( errorBlob->GetBufferPointer() ) );
#endif

    ASSERT_SUCCEEDED( m_device->CreateRootSignature(
        0,
        serializedBlob->GetBufferPointer(),
        serializedBlob->GetBufferSize(),
        IID_PPV_ARGS( m_rootSignature.GetAddressOf() ) ) );
}

void CTriangleDemo::InitShaders()
{
#ifdef _DEBUG
    #define COMMA_DEBUG_ARGS , DXC_ARG_DEBUG, DXC_ARG_SKIP_OPTIMIZATIONS
#else
    #define COMMA_DEBUG_ARGS
#endif

    const wchar_t* vertexShaderArgs[] = { L"-E", L"VS", L"-T", L"vs_6_6" COMMA_DEBUG_ARGS };
    const wchar_t* pixelShaderArgs[] = { L"-E", L"PS", L"-T", L"ps_6_6" COMMA_DEBUG_ARGS };

    m_vertexShaderBlob = Utility::CompileShaderFromFile( L"Shaders/BasicColorTriangle.hlsl", std::size( vertexShaderArgs ), vertexShaderArgs );
    m_pixelShaderBlob = Utility::CompileShaderFromFile( L"Shaders/BasicColorTriangle.hlsl", std::size( pixelShaderArgs ), pixelShaderArgs );
}

void CTriangleDemo::InitGeometry()
{
}

void CTriangleDemo::InitPipelineStates()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = Utility::CreateDefaultPsoDesc(
        m_backBufferFormat,
        m_depthStencilFormat,
        std::size( kInputLayout ),
        kInputLayout,
        m_rootSignature.Get(),
        m_vertexShaderBlob.Get(),
        m_pixelShaderBlob.Get() );

    ASSERT_SUCCEEDED( m_device->CreateGraphicsPipelineState( &desc, IID_PPV_ARGS( m_psoSolid.GetAddressOf() ) ) );

    desc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;

    ASSERT_SUCCEEDED( m_device->CreateGraphicsPipelineState( &desc, IID_PPV_ARGS( m_psoWireframe.GetAddressOf() ) ) );
}
