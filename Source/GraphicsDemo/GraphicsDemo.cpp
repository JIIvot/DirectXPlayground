#include "GraphicsDemo.h"
#include "Utility.h"
#include "Application.h"

#include <imgui.h>
#include <backends/imgui_impl_dx12.h>

CGraphicsDemo::CGraphicsDemo( HWND windowHandle )
{
    InitDebugAndDxgiFactory();
    InitAdapterAndDevice();
    InitCommandObjects();
    InitSwapChain( windowHandle );
    InitResourceViewHeaps();
    InitFenceObjects();
    InitImGui();

    LOG_TRACE( "Initial resize.\n" );
    OnResize();

    m_uploadBatch = std::make_optional<DirectX::ResourceUploadBatch>( m_device.Get() );

    LOG_INFO( "Demo initialized.\n" );
}

void CGraphicsDemo::InitDebugAndDxgiFactory()
{
#ifdef _DEBUG
    static constexpr uint32_t kFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

    Microsoft::WRL::ComPtr<ID3D12Debug1> debugController;
    ASSERT_SUCCEEDED( D3D12GetDebugInterface( IID_PPV_ARGS( debugController.GetAddressOf() ) ) );

    debugController->EnableDebugLayer();
#else
    static constexpr uint32_t kFactoryFlags = 0;
#endif

    ASSERT_SUCCEEDED( CreateDXGIFactory2( kFactoryFlags, IID_PPV_ARGS( m_dxgiFactory.GetAddressOf() ) ) );
}

void CGraphicsDemo::InitAdapterAndDevice()
{
    HRESULT deviceCreationResult = E_FAIL;

    for ( int32_t i = 0; m_dxgiFactory->EnumAdapterByGpuPreference( i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS( m_dxgiAdapter.ReleaseAndGetAddressOf() ) ) != DXGI_ERROR_NOT_FOUND; ++i )
    {
        deviceCreationResult = D3D12CreateDevice( m_dxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS( m_device.ReleaseAndGetAddressOf() ) );

        if ( SUCCEEDED( deviceCreationResult ) )
        {
            break;
        }
    }

    ASSERT_SUCCEEDED( deviceCreationResult );
}

void CGraphicsDemo::InitCommandObjects()
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

void CGraphicsDemo::InitSwapChain( HWND hWindow )
{
    const CApplication& app = CApplication::GetInstance();
    const uint32_t      width = static_cast<uint32_t>( app.GetWidth() );
    const uint32_t      height = static_cast<uint32_t>( app.GetHeight() );

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
    ASSERT_SUCCEEDED( m_dxgiFactory->CreateSwapChainForHwnd( m_commandQueue.Get(), hWindow, &desc, nullptr, nullptr, swapChain.GetAddressOf() ) );
    ASSERT_SUCCEEDED( swapChain.As( &m_dxgiSwapChain ) );

    ASSERT_SUCCEEDED( m_dxgiFactory->MakeWindowAssociation( hWindow, DXGI_MWA_NO_ALT_ENTER ) );
}

void CGraphicsDemo::InitResourceViewHeaps()
{
    static constexpr uint32_t kCbvSrvUavHeapCapacity = 16384;

    m_rtvHeap.Init( m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, kNumSwapChainBuffers );
    m_dsvHeap.Init( m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1 );
    m_cbvSrvUavHeap.Init( m_device.Get(), kCbvSrvUavHeapCapacity );
}

void CGraphicsDemo::InitFenceObjects()
{
    ASSERT_SUCCEEDED( m_device->CreateFence( m_currFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( m_fence.GetAddressOf() ) ) );
    m_fenceEventHandle = CreateEventEx( nullptr, nullptr, 0, EVENT_ALL_ACCESS );
}

void CGraphicsDemo::InitImGui()
{
    ImGui_ImplDX12_InitInfo initInfo = {};
    initInfo.Device = m_device.Get();
    initInfo.CommandQueue = m_commandQueue.Get();
    initInfo.NumFramesInFlight = 1;
    initInfo.RTVFormat = kBackBufferFormat;
    initInfo.DSVFormat = kDepthStencilFormat;
    initInfo.UserData = &m_cbvSrvUavHeap;
    initInfo.SrvDescriptorHeap = m_cbvSrvUavHeap.GetHeap();

    // ReSharper disable once CppParameterMayBeConstPtrOrRef
    initInfo.SrvDescriptorAllocFn = []( ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* gpuHandle )
    {
        CCbvSrvUavHeap* const heap = static_cast<CCbvSrvUavHeap*>( info->UserData );
        const uint32_t        index = heap->GetNextIndex();

        *cpuHandle = heap->GetCpuHandle( index );
        *gpuHandle = heap->GetGpuHandle( index );
    };

    // ReSharper disable once CppParameterMayBeConstPtrOrRef
    initInfo.SrvDescriptorFreeFn = []( ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle )
    {
        CCbvSrvUavHeap* const heap = static_cast<CCbvSrvUavHeap*>( info->UserData );
        heap->ReleaseIndex( heap->GetIndexFromCpuHandle( cpuHandle ) );

        assert( heap->GetIndexFromCpuHandle( cpuHandle ) == heap->GetIndexFromGpuHandle( gpuHandle ) );
    };

    ImGui_ImplDX12_Init( &initInfo );
}

void CGraphicsDemo::FlushCommandQueue()
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

void CGraphicsDemo::CreateDepthStencilBufferAndView( int32_t width, int32_t height )
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

void CGraphicsDemo::ResizeSwapChainAndCreateViews( int32_t width, int32_t height )
{
    for ( Microsoft::WRL::ComPtr<ID3D12Resource>& buffer : m_swapChainBuffers )
    {
        buffer.Reset();
    }

    ASSERT_SUCCEEDED( m_dxgiSwapChain->ResizeBuffers( kNumSwapChainBuffers, width, height, kBackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH ) );

    for ( int32_t i = 0; i < kNumSwapChainBuffers; ++i )
    {
        ASSERT_SUCCEEDED( m_dxgiSwapChain->GetBuffer( i, IID_PPV_ARGS( m_swapChainBuffers[i].GetAddressOf() ) ) );
        m_device->CreateRenderTargetView( m_swapChainBuffers[i].Get(), nullptr, m_rtvHeap.GetCpuHandle( i ) );
    }

    m_currBackBufferIndex = 0;
}

CGraphicsDemo::~CGraphicsDemo()
{
    ImGui_ImplDX12_Shutdown();
    CloseHandle( m_fenceEventHandle );

    LOG_INFO( "Demo destroyed.\n" );
}

void CGraphicsDemo::OnUpdate()
{
    CApplication& app = CApplication::GetInstance();
    const CInput& input = app.GetInput();

    if ( input.IsKeyJustPressed( VK_ESCAPE ) )
    {
        app.Quit();
    }
}

void CGraphicsDemo::OnResize()
{
    const CApplication& app = CApplication::GetInstance();
    const int32_t       width = app.GetWidth();
    const int32_t       height = app.GetHeight();

    FlushCommandQueue();
    ResizeSwapChainAndCreateViews( width, height );
    CreateDepthStencilBufferAndView( width, height );

    m_aspectRatio = static_cast<float>( width ) / height;

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

    LOG_INFO( "Demo was resized. Width: %d, Height: %d\n", width, height );
}

void CGraphicsDemo::OnQuit()
{
    FlushCommandQueue();
}
