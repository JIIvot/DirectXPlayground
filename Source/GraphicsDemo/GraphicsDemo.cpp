#include "GraphicsDemo.h"
#include "Utility.h"
#include "Application.h"

#include <imgui.h>
#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_win32.h>

static constexpr uint32_t kCbvSrvUavHeapCapacity = 16384;

GraphicsDemo::GraphicsDemo( const GraphicsDemoCreateInfo& info )
    : m_dxgiFactory( info.dxgiFactory )
    , m_device( info.device )
    , m_uploadBatch( info.device )
    , m_rtvHeap( info.device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, kNumSwapChainBuffers )
    , m_dsvHeap( info.device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1 )
    , m_cbvSrvUavHeap( info.device, kCbvSrvUavHeapCapacity )
{
    InitCommandObjects();
    InitSwapChainAndDepthStencilBuffer( info.windowHandle, info.clientWidth, info.clientHeight );
    InitFenceObjects();
    InitImGui( info.windowHandle );
    UpdateViewportScissorsAndAspectRatio( info.clientWidth, info.clientHeight );

    LOG_INFO( "Demo initialized.\n" );
}

void GraphicsDemo::InitCommandObjects()
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Priority = 0;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0;

    ASSERT_SUCCEEDED( m_device->CreateCommandQueue( &queueDesc, IID_PPV_ARGS( m_commandQueue.GetAddressOf() ) ) );
    ASSERT_SUCCEEDED( m_device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( m_commandAllocator.GetAddressOf() ) ) );
    ASSERT_SUCCEEDED( m_device->CreateCommandList1( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS( m_commandList.GetAddressOf() ) ) );
}

void GraphicsDemo::InitSwapChainAndDepthStencilBuffer( HWND windowHandle, int32_t width, int32_t height )
{
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.Format = kBackBufferFormat;
    desc.Stereo = false;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = kNumSwapChainBuffers;
    desc.Scaling = DXGI_SCALING_NONE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
    ASSERT_SUCCEEDED( m_dxgiFactory->CreateSwapChainForHwnd( m_commandQueue.Get(), windowHandle, &desc, nullptr, nullptr, swapChain.GetAddressOf() ) );
    ASSERT_SUCCEEDED( swapChain.As( &m_dxgiSwapChain ) );

    ASSERT_SUCCEEDED( m_dxgiFactory->MakeWindowAssociation( windowHandle, DXGI_MWA_NO_ALT_ENTER ) );

    CreateSwapChainBufferViews( width, height );
    CreateDepthStencilBufferAndView( width, height );
}

void GraphicsDemo::InitFenceObjects()
{
    ASSERT_SUCCEEDED( m_device->CreateFence( m_currFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( m_fence.GetAddressOf() ) ) );
    m_fenceEventHandle = CreateEventEx( nullptr, nullptr, 0, EVENT_ALL_ACCESS );
}

void GraphicsDemo::InitImGui( HWND windowHandle )
{
    ImGui::CreateContext();
    ImGui_ImplWin32_Init( windowHandle );

    ImGui_ImplDX12_InitInfo initInfo = {};
    initInfo.Device = m_device.Get();
    initInfo.CommandQueue = m_commandQueue.Get();
    initInfo.NumFramesInFlight = 1;
    initInfo.RTVFormat = kBackBufferFormat;
    initInfo.DSVFormat = kDepthStencilFormat;
    initInfo.UserData = &m_cbvSrvUavHeap;
    initInfo.SrvDescriptorHeap = m_cbvSrvUavHeap.GetHeap();

    initInfo.SrvDescriptorAllocFn = []( ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* gpuHandle )
    {
        CbvSrvUavHeap* const heap = static_cast<CbvSrvUavHeap*>( info->UserData );
        const uint32_t       index = heap->GetNextIndex();

        *cpuHandle = heap->GetCpuHandle( index );
        *gpuHandle = heap->GetGpuHandle( index );
    };

    initInfo.SrvDescriptorFreeFn = []( ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle )
    {
        CbvSrvUavHeap* const heap = static_cast<CbvSrvUavHeap*>( info->UserData );
        heap->ReleaseIndex( heap->GetIndexFromCpuHandle( cpuHandle ) );

        assert( heap->GetIndexFromCpuHandle( cpuHandle ) == heap->GetIndexFromGpuHandle( gpuHandle ) );
    };

    ImGui_ImplDX12_Init( &initInfo );
}

void GraphicsDemo::FlushCommandQueue()
{
    ++m_currFenceValue;
    ASSERT_SUCCEEDED( m_commandQueue->Signal( m_fence.Get(), m_currFenceValue ) );

    if ( m_fence->GetCompletedValue() >= m_currFenceValue )
    {
        return;
    }

    ASSERT_SUCCEEDED( m_fence->SetEventOnCompletion( m_currFenceValue, m_fenceEventHandle ) );
    WaitForSingleObject( m_fenceEventHandle, INFINITE );
}

void GraphicsDemo::CreateDepthStencilBufferAndView( int32_t width, int32_t height )
{
    const CD3DX12_RESOURCE_DESC depthStencilBufferDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        kDepthStencilFormat,                       // format
        width,                                     // width
        height,                                    // height
        1,                                         // arraySize
        1,                                         // mipLevels
        1,                                         // sampleCount
        0,                                         // sampleQuality
        D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL ); // flags

    ASSERT_SUCCEEDED( m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),  // pHeapProperties
        D3D12_HEAP_FLAG_NONE,                                 // HeapFlags
        &depthStencilBufferDesc,                              // pDesc
        D3D12_RESOURCE_STATE_DEPTH_WRITE,                     // InitialResourceState
        &CD3DX12_CLEAR_VALUE( kDepthStencilFormat, 1.0f, 0 ), // pOptimizedClearValue
        IID_PPV_ARGS( m_depthStencilBuffer.ReleaseAndGetAddressOf() ) ) );

    m_device->CreateDepthStencilView( m_depthStencilBuffer.Get(), nullptr, m_dsvHeap.GetCpuHandle() );
}

void GraphicsDemo::ResizeSwapChain( int32_t width, int32_t height )
{
    for ( Microsoft::WRL::ComPtr<ID3D12Resource>& buffer : m_swapChainBuffers )
    {
        buffer.Reset();
    }

    ASSERT_SUCCEEDED( m_dxgiSwapChain->ResizeBuffers( kNumSwapChainBuffers, width, height, kBackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH ) );
    m_currBackBufferIndex = 0;
}

void GraphicsDemo::CreateSwapChainBufferViews( int32_t width, int32_t height )
{
    for ( int32_t i = 0; i < kNumSwapChainBuffers; ++i )
    {
        ASSERT_SUCCEEDED( m_dxgiSwapChain->GetBuffer( i, IID_PPV_ARGS( m_swapChainBuffers[i].GetAddressOf() ) ) );
        m_device->CreateRenderTargetView( m_swapChainBuffers[i].Get(), nullptr, m_rtvHeap.GetCpuHandle( i ) );
    }
}

void GraphicsDemo::UpdateViewportScissorsAndAspectRatio( int32_t width, int32_t height )
{
    m_viewport.TopLeftX = 0.0f;
    m_viewport.TopLeftY = 0.0f;
    m_viewport.Width = static_cast<float>( width );
    m_viewport.Height = static_cast<float>( height );
    m_viewport.MaxDepth = 1.0f;
    m_viewport.MinDepth = 0.0f;

    m_scissorRect.left = 0;
    m_scissorRect.top = 0;
    m_scissorRect.right = width;
    m_scissorRect.bottom = height;

    m_aspectRatio = m_viewport.Width / m_viewport.Height;
}

GraphicsDemo::~GraphicsDemo()
{
    ImGui_ImplWin32_Shutdown();
    ImGui_ImplDX12_Shutdown();
    ImGui::DestroyContext();
    CloseHandle( m_fenceEventHandle );

    LOG_INFO( "Demo destroyed.\n" );
}

void GraphicsDemo::OnUpdate( const Input& input )
{
    if ( input.IsKeyJustPressed( VK_ESCAPE ) )
    {
        Application::Get().Quit();
    }
}

void GraphicsDemo::OnResize( int32_t width, int32_t height )
{
    FlushCommandQueue();
    ResizeSwapChain( width, height );
    CreateSwapChainBufferViews( width, height );
    CreateDepthStencilBufferAndView( width, height );
    UpdateViewportScissorsAndAspectRatio( width, height );

    LOG_INFO( "Demo was resized. Width: %d, Height: %d\n", width, height );
}

void GraphicsDemo::OnQuit()
{
    FlushCommandQueue();
}
