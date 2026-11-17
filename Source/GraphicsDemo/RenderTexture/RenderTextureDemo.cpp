#include "RenderTextureDemo.h"
#include "Utility.h"

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
    InitRenderTexture();
    InitShaders();
    InitRootSignature();
    InitPipelineState();
}

void Demo::OnRender( const GameTimer& timer )
{
    using namespace DirectX;

    ASSERT_SUCCEEDED( m_commandAllocator->Reset() );
    ASSERT_SUCCEEDED( m_commandList->Reset( m_commandAllocator.Get(), nullptr ) );

    m_commandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( GetCurrBackBufferResource(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET ) );

    m_commandList->RSSetViewports( 1, &m_viewport );
    m_commandList->RSSetScissorRects( 1, &m_scissorRect );

    m_commandList->OMSetRenderTargets( 1, &GetCurrBackBufferViewHandle(), true, nullptr );

    m_commandList->ClearRenderTargetView( GetCurrBackBufferViewHandle(), Colors::CornflowerBlue, 0, nullptr );

    m_commandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( GetCurrBackBufferResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT ) );

    ASSERT_SUCCEEDED( m_commandList->Close() );

    ID3D12CommandList* const commandList = m_commandList.Get();
    m_commandQueue->ExecuteCommandLists( 1, &commandList );

    ASSERT_SUCCEEDED( m_dxgiSwapChain->Present( 0, 0 ) );
    m_currBackBufferIndex = ( m_currBackBufferIndex + 1 ) % kNumSwapChainBuffers;

    FlushCommandQueue();
}

void Demo::InitRenderTexture()
{
    static constexpr D3D12_CLEAR_VALUE kClearValue = {
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .Color = { 1.0f, 0.0f, 0.0f, 1.0f }
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
        D3D12_RESOURCE_STATE_GENERIC_READ,
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

    m_vertexShaderBlob = Utility::CompileShaderFromFile( L"Shaders/BasicRenderTarget.hlsl", std::size( vertexShaderArgs ), vertexShaderArgs );
    m_pixelShaderBlob = Utility::CompileShaderFromFile( L"Shaders/BasicRenderTarget.hlsl", std::size( pixelShaderArgs ), pixelShaderArgs );
}

void Demo::InitRootSignature()
{
    static constexpr D3D12_ROOT_SIGNATURE_DESC kDesc = {
        .NumParameters = 0,
        .pParameters = nullptr,
        .NumStaticSamplers = 0,
        .pStaticSamplers = nullptr,
        .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    };

    Microsoft::WRL::ComPtr<ID3DBlob> serializedBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    ASSERT_SUCCEEDED( D3D12SerializeRootSignature( &kDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedBlob.GetAddressOf(), errorBlob.GetAddressOf() ) );
    ASSERT_SUCCEEDED( m_device->CreateRootSignature( 0, serializedBlob->GetBufferPointer(), serializedBlob->GetBufferSize(), IID_PPV_ARGS( m_rootSignature.GetAddressOf() ) ) );
}

void Demo::InitPipelineState()
{
}
} // namespace RenderTextureDemo
