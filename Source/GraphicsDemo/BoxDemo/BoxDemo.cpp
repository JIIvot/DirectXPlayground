#include "BoxDemo.h"
#include "Utility.h"
#include "Application.h"

#include <DirectXMath.h>
#include <DirectXColors.h>
#include <BufferHelpers.h>
#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_win32.h>

static constexpr D3D12_INPUT_ELEMENT_DESC kInputElementDescs[2] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};

struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
};

struct ObjectConstants
{
    DirectX::XMFLOAT4X4 world;
};

struct PassConstants
{
    DirectX::XMFLOAT4X4 viewProjection;
};

enum RootArgument : uint8_t
{
    eRootArgument_ObjectConstBuffer = 0,
    eRootArgument_PassConstBuffer,
    eRootArgument_Count
};

BoxDemo::BoxDemo( const GraphicsDemoCreateInfo& info )
    : GraphicsDemo( info )
{
    m_uploadBatch.Begin();
    InitGeometry();
    std::future<void> uploadResult = m_uploadBatch.End( m_commandQueue.Get() );

    DirectX::XMStoreFloat4x4( &m_world, DirectX::XMMatrixIdentity() );
    AdjustProjection();

    InitRootSignature();
    InitShaders();
    InitConstantBuffers();
    InitPipelineStates();

    uploadResult.wait();
}

BoxDemo::~BoxDemo()
{
    m_objectConstBuffer->Unmap( 0, nullptr );
    m_objectConstBufferData = nullptr;

    m_passConstBuffer->Unmap( 0, nullptr );
    m_passConstBufferData = nullptr;
}

void BoxDemo::OnUpdate( const Input& input )
{
    ProcessInput( input );
    UpdateConstBuffers();
}

void BoxDemo::OnRender()
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
    m_commandList->SetGraphicsRootConstantBufferView( eRootArgument_ObjectConstBuffer, m_objectConstBuffer->GetGPUVirtualAddress() );
    m_commandList->SetGraphicsRootConstantBufferView( eRootArgument_PassConstBuffer, m_passConstBuffer->GetGPUVirtualAddress() );

    m_commandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
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

void BoxDemo::OnResize( int32_t width, int32_t height )
{
    GraphicsDemo::OnResize( width, height );
    AdjustProjection();
}

void BoxDemo::InitGeometry()
{
    static const Vertex kVertices[8] = {
        { .position = DirectX::XMFLOAT3( -1.0f, -1.0f, -1.0f ), .color = DirectX::XMFLOAT4( DirectX::Colors::Wheat ) },
        { .position = DirectX::XMFLOAT3( -1.0f, +1.0f, -1.0f ), .color = DirectX::XMFLOAT4( DirectX::Colors::Coral ) },
        { .position = DirectX::XMFLOAT3( +1.0f, +1.0f, -1.0f ), .color = DirectX::XMFLOAT4( DirectX::Colors::Red ) },
        { .position = DirectX::XMFLOAT3( +1.0f, -1.0f, -1.0f ), .color = DirectX::XMFLOAT4( DirectX::Colors::LightSeaGreen ) },
        { .position = DirectX::XMFLOAT3( -1.0f, -1.0f, +1.0f ), .color = DirectX::XMFLOAT4( DirectX::Colors::Blue ) },
        { .position = DirectX::XMFLOAT3( -1.0f, +1.0f, +1.0f ), .color = DirectX::XMFLOAT4( DirectX::Colors::Orange ) },
        { .position = DirectX::XMFLOAT3( +1.0f, +1.0f, +1.0f ), .color = DirectX::XMFLOAT4( DirectX::Colors::Purple ) },
        { .position = DirectX::XMFLOAT3( +1.0f, -1.0f, +1.0f ), .color = DirectX::XMFLOAT4( DirectX::Colors::DeepSkyBlue ) }
    };

    static constexpr uint16_t kIndices[36] = {
        0, 1, 2, 0, 2, 3, // front face
        4, 6, 5, 4, 7, 6, // back face
        4, 5, 1, 4, 1, 0, // left face
        3, 2, 6, 3, 6, 7, // right face
        1, 5, 6, 1, 6, 2, // top face
        4, 0, 3, 4, 3, 7  // bottom face
    };

    ASSERT_SUCCEEDED( DirectX::CreateStaticBuffer(
        m_device,                                        // device
        m_uploadBatch,                                   // resourceUpload
        kVertices,                                       // data
        std::size( kVertices ),                          // count
        sizeof( kVertices[0] ),                          // stride
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
    m_vertexBufferView.SizeInBytes = std::size( kVertices ) * sizeof( kVertices[0] );
    m_vertexBufferView.StrideInBytes = sizeof( kVertices[0] );

    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    m_indexBufferView.SizeInBytes = std::size( kIndices ) * sizeof( kIndices[0] );
}

void BoxDemo::InitRootSignature()
{
    CD3DX12_ROOT_PARAMETER rootParameters[eRootArgument_Count] = {};
    rootParameters[eRootArgument_ObjectConstBuffer].InitAsConstantBufferView( 0 );
    rootParameters[eRootArgument_PassConstBuffer].InitAsConstantBufferView( 1 );

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

void BoxDemo::InitConstantBuffers()
{
    static constexpr uint32_t kObjectConstantsAlignedSize = Utility::GetConstBufferAlignedSize<ObjectConstants>();
    static constexpr uint32_t kPassConstantsAlignedSize = Utility::GetConstBufferAlignedSize<PassConstants>();

    ASSERT_SUCCEEDED( m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer( kObjectConstantsAlignedSize ),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS( m_objectConstBuffer.GetAddressOf() ) ) );

    ASSERT_SUCCEEDED( m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer( kPassConstantsAlignedSize ),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS( m_passConstBuffer.GetAddressOf() ) ) );

    ASSERT_SUCCEEDED( m_objectConstBuffer->Map( 0, nullptr, &m_objectConstBufferData ) );
    ASSERT_SUCCEEDED( m_passConstBuffer->Map( 0, nullptr, &m_passConstBufferData ) );
}

void BoxDemo::InitShaders()
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

void BoxDemo::InitPipelineStates()
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
    desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

    ASSERT_SUCCEEDED( m_device->CreateGraphicsPipelineState( &desc, IID_PPV_ARGS( m_psoWireframe.GetAddressOf() ) ) );
}

void BoxDemo::RenderImGui()
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

void BoxDemo::AdjustProjection()
{
    const DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH( DirectX::XM_PIDIV4, m_aspectRatio, 1.0f, 1000.0f );
    DirectX::XMStoreFloat4x4( &m_projection, projection );
}

void BoxDemo::ProcessInput( const Input& input )
{
    GraphicsDemo::OnUpdate( input );

    if ( input.IsKeyJustPressed( 'F' ) )
    {
        m_isWireframeEnabled = !m_isWireframeEnabled;
    }

    if ( input.IsButtonPressed( Input::eMouseButton_Left ) )
    {
        const DirectX::XMFLOAT2 mouseDelta = input.GetMouseDelta();
        m_viewPolarAngle += DirectX::XMConvertToRadians( 0.25f * mouseDelta.x );
        m_viewAzimuthalAngle += DirectX::XMConvertToRadians( 0.25f * mouseDelta.y );
    }

    if ( input.IsButtonPressed( Input::eMouseButton_Right ) )
    {
        const DirectX::XMFLOAT2 mouseDelta = input.GetMouseDelta();
        m_viewRadius += 0.01f * static_cast<float>( mouseDelta.x - mouseDelta.y );
    }
}

void BoxDemo::UpdateConstBuffers()
{
    // Convert from spherical coordinates to cartesian
    const DirectX::XMVECTOR position = DirectX::XMVectorSet(
        m_viewRadius * std::sin( m_viewAzimuthalAngle ) * std::cos( m_viewPolarAngle ),
        m_viewRadius * std::cos( m_viewAzimuthalAngle ),
        m_viewRadius * std::sin( m_viewAzimuthalAngle ) * std::sin( m_viewPolarAngle ),
        1.0f );

    const DirectX::XMVECTOR target = DirectX::XMVectorZero();
    const DirectX::XMVECTOR up = DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );

    const DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH( position, target, up );
    const DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4( &m_world );
    const DirectX::XMMATRIX projection = DirectX::XMLoadFloat4x4( &m_projection );
    const DirectX::XMMATRIX viewProjection = view * projection;

    ObjectConstants objectConstants = {};
    DirectX::XMStoreFloat4x4( &objectConstants.world, DirectX::XMMatrixTranspose( world ) );
    memcpy( m_objectConstBufferData, &objectConstants, sizeof( objectConstants ) );

    PassConstants passConstants = {};
    DirectX::XMStoreFloat4x4( &passConstants.viewProjection, DirectX::XMMatrixTranspose( viewProjection ) );
    memcpy( m_passConstBufferData, &passConstants, sizeof( passConstants ) );
}
