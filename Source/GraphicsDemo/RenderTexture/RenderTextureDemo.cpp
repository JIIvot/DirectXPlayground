#include "RenderTextureDemo.h"
#include "Utility.h"
#include "RenderTextureVertex.h"

#include <BufferHelpers.h>
#include <DirectXColors.h>

namespace RenderTextureDemo
{
namespace
{
constexpr int32_t kRenderTextureWidth = 320;
constexpr int32_t kRenderTextureHeight = 180;

constexpr uint32_t kRtvRenderTextureIndex = 2;
} // namespace

Demo::Demo( const GraphicsDemoCreateInfo& info )
    : GraphicsDemo( info, 1, 1 )
{
    OnResize( info.clientWidth, info.clientHeight );

    m_uploadBatch.Begin();
    InitGeometry();
    const std::future<void> uploadResult = m_uploadBatch.End( m_commandQueue.Get() );

    InitRenderTexture();
    InitShaders();
    InitRootSignature();
    InitPipelineState();

    uploadResult.wait();
}

void Demo::OnRender( const GameTimer& timer )
{
    using namespace DirectX;

    ASSERT_SUCCEEDED( m_commandAllocator->Reset() );
    ASSERT_SUCCEEDED( m_commandList->Reset( m_commandAllocator.Get(), m_pipelineState.Get() ) );

    ID3D12DescriptorHeap* const cbvSrvUavHeap = m_cbvSrvUavHeap.GetHeap();
    m_commandList->SetDescriptorHeaps( 1, &cbvSrvUavHeap );

    // Drawing on the render texture
    m_commandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( m_renderTexture.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET ) );

    m_commandList->OMSetRenderTargets( 1, &m_rtvHeap.GetCpuHandle( kRtvRenderTextureIndex ), true, nullptr );

    m_commandList->ClearRenderTargetView( m_rtvHeap.GetCpuHandle( kRtvRenderTextureIndex ), Colors::Blue, 0, nullptr );

    m_commandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( m_renderTexture.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE ) );

    // Drawing the render texture itself
    m_commandList->RSSetViewports( 1, &m_viewport );
    m_commandList->RSSetScissorRects( 1, &m_scissorRect );

    m_commandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( GetCurrBackBufferResource(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET ) );

    m_commandList->OMSetRenderTargets( 1, &GetCurrBackBufferViewHandle(), true, nullptr );

    m_commandList->ClearRenderTargetView( GetCurrBackBufferViewHandle(), Colors::Black, 0, nullptr );

    m_commandList->SetGraphicsRootSignature( m_rootSignature.Get() );
    m_commandList->SetGraphicsRootDescriptorTable( 0, m_cbvSrvUavHeap.GetGpuHandle( m_srvRenderTextureIndex ) );

    m_commandList->IASetVertexBuffers( 0, 1, &m_renderTextureVertexBufferView );
    m_commandList->IASetIndexBuffer( &m_renderTextureIndexBufferView );
    m_commandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    m_commandList->DrawIndexedInstanced( 6, 1, 0, 0, 0 );

    m_commandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( GetCurrBackBufferResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT ) );

    ASSERT_SUCCEEDED( m_commandList->Close() );

    ID3D12CommandList* const commandList = m_commandList.Get();
    m_commandQueue->ExecuteCommandLists( 1, &commandList );

    ASSERT_SUCCEEDED( m_dxgiSwapChain->Present( 0, 0 ) );
    m_currBackBufferIndex = ( m_currBackBufferIndex + 1 ) % kNumSwapChainBuffers;

    FlushCommandQueue();
}

void Demo::OnResize( int32_t width, int32_t height )
{
    GraphicsDemo::OnResize( width, height );

    const float scale = std::min( static_cast<float>( width ) / kRenderTextureWidth, static_cast<float>( height ) / kRenderTextureHeight );
    m_viewport.Width = kRenderTextureWidth * scale;
    m_viewport.Height = kRenderTextureHeight * scale;
    m_viewport.TopLeftX = ( static_cast<float>( width ) - m_viewport.Width ) * 0.5f;
    m_viewport.TopLeftY = ( static_cast<float>( height ) - m_viewport.Height ) * 0.5f;
}

void Demo::InitRenderTexture()
{
    using namespace DirectX;

    static constexpr D3D12_CLEAR_VALUE kClearValue = {
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .Color = { 0.0f, 0.0f, 0.0f, 1.0f }
    };

    static const D3D12_RESOURCE_DESC kDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        DXGI_FORMAT_R8G8B8A8_UNORM,                // format
        kRenderTextureWidth,                       // width
        kRenderTextureHeight,                      // height
        1,                                         // arraySize
        0,                                         // mipLevels
        1,                                         // sampleCount
        0,                                         // sampleQuality
        D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET ); // flags

    ASSERT_SUCCEEDED( m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),
        D3D12_HEAP_FLAG_NONE,
        &kDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        &kClearValue,
        IID_PPV_ARGS( m_renderTexture.GetAddressOf() ) ) );

    m_device->CreateRenderTargetView( m_renderTexture.Get(), nullptr, m_rtvHeap.GetCpuHandle( kRtvRenderTextureIndex ) );

    m_srvRenderTextureIndex = m_cbvSrvUavHeap.GetNextIndex();
    m_device->CreateShaderResourceView( m_renderTexture.Get(), nullptr, m_cbvSrvUavHeap.GetCpuHandle( m_srvRenderTextureIndex ) );
}

void Demo::InitShaders()
{
    const wchar_t* vertexShaderArgs[] = { L"-E", L"VS", L"-T", L"vs_6_6" COMMA_DEBUG_ARGS };
    const wchar_t* pixelShaderArgs[] = { L"-E", L"PS", L"-T", L"ps_6_6" COMMA_DEBUG_ARGS };

    m_vertexShaderBlob = Utility::CompileShaderFromFile( L"Shaders/BasicRenderTexture.hlsl", std::size( vertexShaderArgs ), vertexShaderArgs );
    m_pixelShaderBlob = Utility::CompileShaderFromFile( L"Shaders/BasicRenderTexture.hlsl", std::size( pixelShaderArgs ), pixelShaderArgs );
}

void Demo::InitRootSignature()
{
    static constexpr D3D12_STATIC_SAMPLER_DESC kSamplerDesc = {
        .Filter = D3D12_FILTER_MIN_MAG_MIP_POINT,
        .AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
    };

    CD3DX12_DESCRIPTOR_RANGE descriptorRanges[1];
    descriptorRanges[0].Init( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 );

    CD3DX12_ROOT_PARAMETER rootParameters[1];
    rootParameters[0].InitAsDescriptorTable( std::size( descriptorRanges ), descriptorRanges );

    const D3D12_ROOT_SIGNATURE_DESC signatureDesc = {
        .NumParameters = std::size( rootParameters ),
        .pParameters = rootParameters,
        .NumStaticSamplers = 1,
        .pStaticSamplers = &kSamplerDesc,
        .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    };

    Microsoft::WRL::ComPtr<ID3DBlob> serializedBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    ASSERT_SUCCEEDED( D3D12SerializeRootSignature( &signatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedBlob.GetAddressOf(), errorBlob.GetAddressOf() ) );
    ASSERT_SUCCEEDED( m_device->CreateRootSignature( 0, serializedBlob->GetBufferPointer(), serializedBlob->GetBufferSize(), IID_PPV_ARGS( m_rootSignature.GetAddressOf() ) ) );
}

void Demo::InitPipelineState()
{
    static constexpr D3D12_INPUT_ELEMENT_DESC kInputElementDescs[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = Utility::CreateDefaultPsoDesc(
        kBackBufferFormat,
        DXGI_FORMAT_UNKNOWN,
        std::size( kInputElementDescs ),
        kInputElementDescs,
        m_rootSignature.Get(),
        m_vertexShaderBlob.Get(),
        m_pixelShaderBlob.Get() );

    desc.DepthStencilState.DepthEnable = false;

    ASSERT_SUCCEEDED( m_device->CreateGraphicsPipelineState( &desc, IID_PPV_ARGS( m_pipelineState.GetAddressOf() ) ) );
}

void Demo::InitGeometry()
{
    using namespace DirectX;

    static constexpr Vertex kVertices[] = {
        { .position = XMFLOAT2( -1.0f, -1.0f ), .texCoord = XMFLOAT2( 0.0f, 0.0f ) },
        { .position = XMFLOAT2( -1.0f, +1.0f ), .texCoord = XMFLOAT2( 0.0f, 1.0f ) },
        { .position = XMFLOAT2( +1.0f, +1.0f ), .texCoord = XMFLOAT2( 1.0f, 1.0f ) },
        { .position = XMFLOAT2( +1.0f, -1.0f ), .texCoord = XMFLOAT2( 1.0f, 0.0f ) }
    };

    static constexpr uint16_t kIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    ASSERT_SUCCEEDED( CreateStaticBuffer(
        m_device.Get(),
        m_uploadBatch,
        kVertices,
        std::size( kVertices ),
        sizeof( kVertices[0] ),
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        m_renderTextureVertexBuffer.GetAddressOf() ) );

    ASSERT_SUCCEEDED( CreateStaticBuffer(
        m_device.Get(),
        m_uploadBatch,
        kIndices,
        std::size( kIndices ),
        sizeof( kIndices[0] ),
        D3D12_RESOURCE_STATE_INDEX_BUFFER,
        m_renderTextureIndexBuffer.GetAddressOf() ) );

    m_renderTextureIndexBufferView.BufferLocation = m_renderTextureIndexBuffer->GetGPUVirtualAddress();
    m_renderTextureIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    m_renderTextureIndexBufferView.SizeInBytes = sizeof( kIndices );

    m_renderTextureVertexBufferView.BufferLocation = m_renderTextureVertexBuffer->GetGPUVirtualAddress();
    m_renderTextureVertexBufferView.StrideInBytes = sizeof( kVertices[0] );
    m_renderTextureVertexBufferView.SizeInBytes = sizeof( kVertices );
}
} // namespace RenderTextureDemo
