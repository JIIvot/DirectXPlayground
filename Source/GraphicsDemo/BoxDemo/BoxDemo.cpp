#include "BoxDemo.h"
#include "Utility.h"
#include "Application.h"

#include <DirectXMath.h>
#include <DirectXColors.h>
#include <BufferHelpers.h>
#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_win32.h>

struct SColorVertex
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT4 Color;
};

struct SObjectConstants
{
    DirectX::XMFLOAT4X4 World;
};

struct SPassConstants
{
    DirectX::XMFLOAT4X4 ViewProjection;
};

enum ERootArgument : uint8_t
{
    eRootArgument_CBufferObject = 0,
    eRootArgument_CBufferPass,
    eRootArgument_Count
};

static constexpr uint32_t kObjectConstantsAlignedSize = Utility::GetCBufferAlignedSizeBytes( sizeof( SObjectConstants ) );
static constexpr uint32_t kPassConstantsAlignedSize = Utility::GetCBufferAlignedSizeBytes( sizeof( SPassConstants ) );

static constexpr D3D12_INPUT_ELEMENT_DESC kInputElementDescs[2] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};

CBoxDemo::CBoxDemo( HWND windowHandle, IDXGIFactory6* dxgiFactory, ID3D12Device5* device )
    : CGraphicsDemo( windowHandle, dxgiFactory, device )
{
    m_uploadBatch.Begin();
    InitGeometry();
    std::future<void> uploadResult = m_uploadBatch.End( m_commandQueue.Get() );

    DirectX::XMStoreFloat4x4( &m_world, DirectX::XMMatrixIdentity() );
    UpdateProjectionMatrix();

    InitRootSignature();
    InitShaders();
    InitConstantBuffers();
    InitPipelineStates();

    uploadResult.wait();
}

CBoxDemo::~CBoxDemo()
{
    m_cbufferObject->Unmap( 0, nullptr );
    m_cbufferObjectData = nullptr;

    m_cbufferPass->Unmap( 0, nullptr );
    m_cbufferPassData = nullptr;
}

void CBoxDemo::OnUpdate()
{
    CGraphicsDemo::OnUpdate();

    if ( CApplication::Get().GetInput().IsKeyJustPressed( 'F' ) )
    {
        m_isWireframeEnabled = !m_isWireframeEnabled;
    }

    const DirectX::XMVECTOR position = DirectX::XMVectorSet( 10.0f, 10.0f, 10.0f, 1.0f );
    const DirectX::XMVECTOR target = DirectX::XMVectorZero();
    const DirectX::XMVECTOR up = DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );

    const DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH( position, target, up );
    const DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4( &m_world );
    const DirectX::XMMATRIX projection = DirectX::XMLoadFloat4x4( &m_projection );
    const DirectX::XMMATRIX viewProjection = view * projection;

    SObjectConstants objectConstants = {};
    DirectX::XMStoreFloat4x4( &objectConstants.World, DirectX::XMMatrixTranspose( world ) );
    memcpy( m_cbufferObjectData, &objectConstants, sizeof( objectConstants ) );

    SPassConstants passConstants = {};
    DirectX::XMStoreFloat4x4( &passConstants.ViewProjection, DirectX::XMMatrixTranspose( viewProjection ) );
    memcpy( m_cbufferPassData, &passConstants, sizeof( passConstants ) );
}

void CBoxDemo::OnRender()
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
    m_commandList->SetGraphicsRootConstantBufferView( eRootArgument_CBufferObject, m_cbufferObject->GetGPUVirtualAddress() );
    m_commandList->SetGraphicsRootConstantBufferView( eRootArgument_CBufferPass, m_cbufferPass->GetGPUVirtualAddress() );

    m_commandList->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    m_commandList->IASetVertexBuffers( 0, 1, &m_vertexBufferView );
    m_commandList->IASetIndexBuffer( &m_indexBufferView );

    m_commandList->DrawIndexedInstanced( 36, 1, 0, 0, 0 );

    ImGui_ImplDX12_RenderDrawData( ImGui::GetDrawData(), m_commandList.Get() );

    m_commandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( GetCurrBackBufferResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT ) );

    ASSERT_SUCCEEDED( m_commandList->Close() );

    ID3D12CommandList* const commandList = m_commandList.Get();
    m_commandQueue->ExecuteCommandLists( 1, &commandList );

    ASSERT_SUCCEEDED( m_dxgiSwapChain->Present( 0, 0 ) );
    m_currBackBufferIndex = ( m_currBackBufferIndex + 1 ) % kNumSwapChainBuffers;

    FlushCommandQueue();
}

void CBoxDemo::OnResize( int32_t width, int32_t height )
{
    CGraphicsDemo::OnResize( width, height );
    UpdateProjectionMatrix();
}

void CBoxDemo::InitGeometry()
{
    static const SColorVertex vertices[8] = {
        SColorVertex( DirectX::XMFLOAT3( -1.0f, -1.0f, -1.0f ), DirectX::XMFLOAT4( DirectX::Colors::Lavender ) ),
        SColorVertex( DirectX::XMFLOAT3( -1.0f, +1.0f, -1.0f ), DirectX::XMFLOAT4( DirectX::Colors::Coral ) ),
        SColorVertex( DirectX::XMFLOAT3( +1.0f, +1.0f, -1.0f ), DirectX::XMFLOAT4( DirectX::Colors::Red ) ),
        SColorVertex( DirectX::XMFLOAT3( +1.0f, -1.0f, -1.0f ), DirectX::XMFLOAT4( DirectX::Colors::Green ) ),
        SColorVertex( DirectX::XMFLOAT3( -1.0f, -1.0f, +1.0f ), DirectX::XMFLOAT4( DirectX::Colors::Blue ) ),
        SColorVertex( DirectX::XMFLOAT3( -1.0f, +1.0f, +1.0f ), DirectX::XMFLOAT4( DirectX::Colors::Orange ) ),
        SColorVertex( DirectX::XMFLOAT3( +1.0f, +1.0f, +1.0f ), DirectX::XMFLOAT4( DirectX::Colors::Purple ) ),
        SColorVertex( DirectX::XMFLOAT3( +1.0f, -1.0f, +1.0f ), DirectX::XMFLOAT4( DirectX::Colors::Honeydew ) )
    };

    static constexpr uint16_t kIndices[36] = {
        // front face
        0,
        1,
        2,
        0,
        2,
        3,

        // back face
        4,
        6,
        5,
        4,
        7,
        6,

        // left face
        4,
        5,
        1,
        4,
        1,
        0,

        // right face
        3,
        2,
        6,
        3,
        6,
        7,

        // top face
        1,
        5,
        6,
        1,
        6,
        2,

        // bottom face
        4,
        0,
        3,
        4,
        3,
        7
    };

    ASSERT_SUCCEEDED( DirectX::CreateStaticBuffer(
        m_device,                                        // device
        m_uploadBatch,                                   // resourceUpload
        vertices,                                        // data
        std::size( vertices ),                           // count
        sizeof( vertices[0] ),                           // stride
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, // afterState
        m_vertexBuffer.GetAddressOf() ) );               // pBuffer

    ASSERT_SUCCEEDED( DirectX::CreateStaticBuffer(
        m_device,                          // device
        m_uploadBatch,                     // resourceUpload
        kIndices,                          // data
        std::size( kIndices ),             // count
        sizeof( kIndices[0] ),             // stride
        D3D12_RESOURCE_STATE_INDEX_BUFFER, // afterState
        m_indexBuffer.GetAddressOf() ) );  // pBuffer

    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = std::size( vertices ) * sizeof( vertices[0] );
    m_vertexBufferView.StrideInBytes = sizeof( vertices[0] );

    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    m_indexBufferView.SizeInBytes = std::size( kIndices ) * sizeof( kIndices[0] );
}

void CBoxDemo::InitRootSignature()
{
    CD3DX12_ROOT_PARAMETER rootParameters[eRootArgument_Count] = {};
    rootParameters[eRootArgument_CBufferObject].InitAsConstantBufferView( 0 );
    rootParameters[eRootArgument_CBufferPass].InitAsConstantBufferView( 1 );

    const CD3DX12_ROOT_SIGNATURE_DESC desc(
        std::size( rootParameters ),
        rootParameters,
        0,
        nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT );

    Microsoft::WRL::ComPtr<ID3DBlob> serializedBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    ASSERT_SUCCEEDED( D3D12SerializeRootSignature( &desc, D3D_ROOT_SIGNATURE_VERSION_1, serializedBlob.GetAddressOf(), errorBlob.GetAddressOf() ) );
    ASSERT_SUCCEEDED( m_device->CreateRootSignature( 0, serializedBlob->GetBufferPointer(), serializedBlob->GetBufferSize(), IID_PPV_ARGS( m_rootSignature.GetAddressOf() ) ) );
}

void CBoxDemo::InitConstantBuffers()
{
    const CD3DX12_RESOURCE_DESC cbufferObjectDesc = CD3DX12_RESOURCE_DESC::Buffer( kObjectConstantsAlignedSize );
    const CD3DX12_RESOURCE_DESC cbufferPassDesc = CD3DX12_RESOURCE_DESC::Buffer( kPassConstantsAlignedSize );

    ASSERT_SUCCEEDED( m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
        D3D12_HEAP_FLAG_NONE,
        &cbufferObjectDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS( m_cbufferObject.GetAddressOf() ) ) );

    ASSERT_SUCCEEDED( m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
        D3D12_HEAP_FLAG_NONE,
        &cbufferPassDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS( m_cbufferPass.GetAddressOf() ) ) );

    ASSERT_SUCCEEDED( m_cbufferObject->Map( 0, nullptr, &m_cbufferObjectData ) );
    ASSERT_SUCCEEDED( m_cbufferPass->Map( 0, nullptr, &m_cbufferPassData ) );
}

void CBoxDemo::InitShaders()
{
#ifdef _DEBUG
    #define COMMA_DEBUG_ARGS , DXC_ARG_DEBUG, DXC_ARG_SKIP_OPTIMIZATIONS
#else
    #define COMMA_DEBUG_ARGS
#endif

    const wchar_t* vertexShaderArgs[] = { L"-E", L"VS", L"-T", L"vs_6_6" COMMA_DEBUG_ARGS };
    const wchar_t* pixelShaderArgs[] = { L"-E", L"PS", L"-T", L"ps_6_6" COMMA_DEBUG_ARGS };

    m_vertexShaderBlob = Utility::CompileShaderFromFile( L"Shaders/BasicColor.hlsl", std::size( vertexShaderArgs ), vertexShaderArgs );
    m_pixelShaderBlob = Utility::CompileShaderFromFile( L"Shaders/BasicColor.hlsl", std::size( pixelShaderArgs ), pixelShaderArgs );
}

void CBoxDemo::InitPipelineStates()
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

void CBoxDemo::RenderImGui()
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

void CBoxDemo::UpdateProjectionMatrix()
{
    const DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH( DirectX::XM_PIDIV4, m_aspectRatio, 1.0f, 1000.0f );
    DirectX::XMStoreFloat4x4( &m_projection, projection );
}
