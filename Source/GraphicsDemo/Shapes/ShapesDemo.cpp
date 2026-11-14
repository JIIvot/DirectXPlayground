#include "ShapesDemo.h"

namespace ShapesDemo
{
namespace
{
enum RootArgument : uint8_t
{
    eRootArgument_ObjectConstBuffer,
    eRootArgument_PassConstBuffer,
    eRootArgument_Count
};
} // namespace

Demo::Demo( const GraphicsDemoCreateInfo& info )
    : GraphicsDemo( info, kNumFramesInFlight )
    , m_frameResources{ FrameResource( info.device, 1 ), FrameResource( info.device, 1 ), FrameResource( info.device, 1 ) }
{
    m_uploadBatch.Begin();
    // TODO: Generate meshes
    const std::future<void> uploadResult = m_uploadBatch.End( m_commandQueue.Get() );

    InitRootSignature();
    InitShaders();
    InitPipelineStates();
    InitFrameResources();

    uploadResult.wait();
}

void Demo::OnRender( const GameTimer& timer )
{
}

void Demo::InitRootSignature()
{
    CD3DX12_ROOT_PARAMETER rootParameters[eRootArgument_Count];
    rootParameters[eRootArgument_ObjectConstBuffer].InitAsConstantBufferView( 0 );
    rootParameters[eRootArgument_PassConstBuffer].InitAsConstantBufferView( 1 );

    const D3D12_ROOT_SIGNATURE_DESC desc = {
        .NumParameters = std::size( rootParameters ),
        .pParameters = rootParameters,
        .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    };

    Microsoft::WRL::ComPtr<ID3DBlob> serializedBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    ASSERT_SUCCEEDED( D3D12SerializeRootSignature( &desc, D3D_ROOT_SIGNATURE_VERSION_1, serializedBlob.GetAddressOf(), errorBlob.GetAddressOf() ) );
    ASSERT_SUCCEEDED( m_device->CreateRootSignature( 0, serializedBlob->GetBufferPointer(), serializedBlob->GetBufferSize(), IID_PPV_ARGS( m_rootSignature.GetAddressOf() ) ) );
}

void Demo::InitShaders()
{
    const wchar_t* vertexShaderArgs[] = { L"-E", L"VS", L"-T", L"vs_6_6" COMMA_DEBUG_ARGS };
    const wchar_t* pixelShaderArgs[] = { L"-E", L"VS", L"-T", L"vs_6_6" COMMA_DEBUG_ARGS };

    m_shaderMap["default_vs"] = Utility::CompileShaderFromFile( L"Shaders/BasicColor.hlsl", std::size( vertexShaderArgs ), vertexShaderArgs );
    m_shaderMap["opaque_ps"] = Utility::CompileShaderFromFile( L"Shaders/BasicColor.hlsl", std::size( pixelShaderArgs ), pixelShaderArgs );
}

void Demo::InitPipelineStates()
{
    static constexpr D3D12_INPUT_ELEMENT_DESC kInputElementDescs[2] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = Utility::CreateDefaultPsoDesc(
        kBackBufferFormat,
        kDepthStencilFormat,
        std::size( kInputElementDescs ),
        kInputElementDescs,
        m_rootSignature.Get(),
        m_shaderMap["default_vs"].Get(),
        m_shaderMap["opaque_ps"].Get() );

    ASSERT_SUCCEEDED( m_device->CreateGraphicsPipelineState( &desc, IID_PPV_ARGS( m_psoMap["opaque"].GetAddressOf() ) ) );

    desc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
    ASSERT_SUCCEEDED( m_device->CreateGraphicsPipelineState( &desc, IID_PPV_ARGS( m_psoMap["wireframe"].GetAddressOf() ) ) );
}
} // namespace ShapesDemo
