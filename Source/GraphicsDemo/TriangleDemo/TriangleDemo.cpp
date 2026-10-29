#include "TriangleDemo.h"
#include "Utility.h"
#include "Application.h"

#include <DirectXMath.h>
#include <DirectXColors.h>
#include <BufferHelpers.h>
#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_win32.h>

struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
};

static constexpr D3D12_INPUT_ELEMENT_DESC kInputElementDescs[2] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};

TriangleDemo::TriangleDemo( const GraphicsDemoCreateInfo& info )
    : GraphicsDemo( info )
{
    m_uploadBatch.Begin();
    InitGeometry();
    std::future<void> uploadResult = m_uploadBatch.End( m_commandQueue.Get() );

    InitRootSignature();
    InitShaders();
    InitPipelineStates();

    uploadResult.wait();
}

void TriangleDemo::OnUpdate( const Input& input )
{
    GraphicsDemo::OnUpdate( input );

    if ( input.IsKeyJustPressed( 'F' ) )
    {
        m_isWireframeEnabled = !m_isWireframeEnabled;
    }
}

void TriangleDemo::OnRender()
{
    ASSERT_SUCCEEDED( m_commandAllocator->Reset() );
    ASSERT_SUCCEEDED( m_commandList->Reset( m_commandAllocator.Get(), m_isWireframeEnabled ? m_psoWireframe.Get() : m_psoSolid.Get() ) );

    RenderImGui();

    ID3D12DescriptorHeap* const cbvSrvUavHeap = m_cbvSrvUavHeap.GetHeap();
    m_commandList->SetDescriptorHeaps( 1, &cbvSrvUavHeap );

    m_commandList->RSSetViewports( 1, &m_viewport );
    m_commandList->RSSetScissorRects( 1, &m_scissorRect );

    m_commandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( GetCurrBackBufferResource(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET ) );

    m_commandList->ClearRenderTargetView( GetCurrBackBufferViewHandle(), DirectX::Colors::Black, 0, nullptr );
    m_commandList->ClearDepthStencilView( m_dsvHeap.GetCpuHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr );

    m_commandList->OMSetRenderTargets( 1, &GetCurrBackBufferViewHandle(), true, &m_dsvHeap.GetCpuHandle() );

    m_commandList->SetGraphicsRootSignature( m_rootSignature.Get() );

    m_commandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    m_commandList->IASetVertexBuffers( 0, 1, &m_vertexBufferView );
    m_commandList->IASetIndexBuffer( &m_indexBufferView );

    m_commandList->DrawIndexedInstanced( 3, 1, 0, 0, 0 );

    ImGui_ImplDX12_RenderDrawData( ImGui::GetDrawData(), m_commandList.Get() );

    m_commandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( GetCurrBackBufferResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT ) );

    ASSERT_SUCCEEDED( m_commandList->Close() );

    ID3D12CommandList* const commandList = m_commandList.Get();
    m_commandQueue->ExecuteCommandLists( 1, &commandList );

    ASSERT_SUCCEEDED( m_dxgiSwapChain->Present( 0, 0 ) );
    m_currBackBufferIndex = ( m_currBackBufferIndex + 1 ) % kNumSwapChainBuffers;

    FlushCommandQueue();
}

void TriangleDemo::InitRootSignature()
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

    ASSERT_SUCCEEDED( m_device->CreateRootSignature(
        0,
        serializedBlob->GetBufferPointer(),
        serializedBlob->GetBufferSize(),
        IID_PPV_ARGS( m_rootSignature.GetAddressOf() ) ) );
}

void TriangleDemo::InitShaders()
{
#ifdef _DEBUG
    #define COMMA_DEBUG_ARGS , DXC_ARG_DEBUG, DXC_ARG_SKIP_OPTIMIZATIONS
#else
    #define COMMA_DEBUG_ARGS
#endif

    const wchar_t* vertexShaderArgs[] = { L"-E", L"VS", L"-T", L"vs_6_6" COMMA_DEBUG_ARGS };
    const wchar_t* pixelShaderArgs[] = { L"-E", L"PS", L"-T", L"ps_6_6" COMMA_DEBUG_ARGS };

    Utility::CompileShaderFromFile( L"Shaders/BasicColorTriangle.hlsl", std::size( vertexShaderArgs ), vertexShaderArgs, m_vertexShaderBlob.GetAddressOf() );
    Utility::CompileShaderFromFile( L"Shaders/BasicColorTriangle.hlsl", std::size( pixelShaderArgs ), pixelShaderArgs, m_pixelShaderBlob.GetAddressOf() );
}

void TriangleDemo::InitGeometry()
{
    static constexpr Vertex kVertices[] = {
        { DirectX::XMFLOAT3( -0.5f, -0.5f, 0.0f ), DirectX::XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ) },
        { DirectX::XMFLOAT3( 0.0f, 0.5f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
        { DirectX::XMFLOAT3( 0.5f, -0.5f, 0.0f ), DirectX::XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ) }
    };

    static constexpr uint16_t kIndices[] = { 0, 1, 2 };

    ASSERT_SUCCEEDED( DirectX::CreateStaticBuffer(
        m_device,                                        // device
        m_uploadBatch,                                   // resourceUpload
        kVertices,                                       // ptr
        std::size( kVertices ),                          // count
        sizeof( Vertex ),                                // stride
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, // afterState
        m_vertexBuffer.GetAddressOf() ) );               // pBuffer

    ASSERT_SUCCEEDED( DirectX::CreateStaticBuffer(
        m_device,                          // device
        m_uploadBatch,                     // resourceUpload
        kIndices,                          // ptr
        std::size( kIndices ),             // count
        sizeof( kIndices[0] ),             // stride
        D3D12_RESOURCE_STATE_INDEX_BUFFER, // afterState
        m_indexBuffer.GetAddressOf() ) );  // pBuffer

    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = std::size( kVertices ) * sizeof( Vertex );
    m_vertexBufferView.StrideInBytes = sizeof( Vertex );

    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.SizeInBytes = std::size( kIndices ) * sizeof( kIndices[0] );
    m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
}

void TriangleDemo::InitPipelineStates()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = Utility::CreateDefaultPsoDesc(
        kBackBufferFormat,
        kDepthStencilFormat,
        std::size( kInputElementDescs ),
        kInputElementDescs,
        m_rootSignature.Get(),
        m_vertexShaderBlob.Get(),
        m_pixelShaderBlob.Get() );

    ASSERT_SUCCEEDED( m_device->CreateGraphicsPipelineState( &desc, IID_PPV_ARGS( m_psoSolid.GetAddressOf() ) ) );

    desc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;

    ASSERT_SUCCEEDED( m_device->CreateGraphicsPipelineState( &desc, IID_PPV_ARGS( m_psoWireframe.GetAddressOf() ) ) );
}

void TriangleDemo::RenderImGui()
{
    ImGui_ImplWin32_NewFrame();
    ImGui_ImplDX12_NewFrame();
    ImGui::NewFrame();

    const ImGuiIO& io = ImGui::GetIO();

    ImGui::Begin( "Options" );
    ImGui::Text( "FPS:       %d", static_cast<int32_t>( io.Framerate ) );
    ImGui::Text( "FrameTime: %.3f", 1000.0f / io.Framerate );
    ImGui::Checkbox( "Wireframe", &m_isWireframeEnabled );
    ImGui::End();

    ImGui::Render();
}
