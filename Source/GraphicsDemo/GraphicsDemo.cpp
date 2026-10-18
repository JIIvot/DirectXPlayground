#include "GraphicsDemo.h"
#include "Utility.h"
#include "Application.h"

#include <imgui.h>
#include <backends/imgui_impl_dx12.h>

CGraphicsDemo::CGraphicsDemo( HWND hWindow )
{
	InitDebugAndDxgiFactory();
	InitAdapterAndDevice();
	InitCommandObjects();
	InitSwapChain( hWindow );
	InitResourceViewHeaps();
	InitFenceObjects();
	InitImGui();

	OnResize();
}

void CGraphicsDemo::InitDebugAndDxgiFactory()
{
#ifdef _DEBUG
	static constexpr uint32_t c_factoryFlags = DXGI_CREATE_FACTORY_DEBUG;

	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController;
	ASSERT_SUCCEEDED( D3D12GetDebugInterface( IID_PPV_ARGS( debugController.GetAddressOf() ) ) );

	debugController->EnableDebugLayer();
#else
	static constexpr uint32_t c_factoryFlags = 0;
#endif

	ASSERT_SUCCEEDED( CreateDXGIFactory2( c_factoryFlags, IID_PPV_ARGS( m_pDxgiFactory.GetAddressOf() ) ) );
}

void CGraphicsDemo::InitAdapterAndDevice()
{
	HRESULT deviceCreationResult = E_FAIL;

	for ( int32_t i = 0; m_pDxgiFactory->EnumAdapterByGpuPreference( i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS( m_pDxgiAdapter.ReleaseAndGetAddressOf() ) ) != DXGI_ERROR_NOT_FOUND; ++i )
	{
		deviceCreationResult = D3D12CreateDevice( m_pDxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS( m_pDevice.ReleaseAndGetAddressOf() ) );

		if ( SUCCEEDED( deviceCreationResult ) )
		{
			break;
		}
	}

	ASSERT_SUCCEEDED( deviceCreationResult );
}

void CGraphicsDemo::InitCommandObjects()
{
	static constexpr D3D12_COMMAND_QUEUE_DESC c_queueDesc = {
		.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
		.Priority = 0,
		.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
		.NodeMask = 0
	};

	ASSERT_SUCCEEDED( m_pDevice->CreateCommandQueue( &c_queueDesc, IID_PPV_ARGS( m_pCommandQueue.GetAddressOf() ) ) );
	ASSERT_SUCCEEDED( m_pDevice->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( m_pCommandAllocator.GetAddressOf() ) ) );
	ASSERT_SUCCEEDED( m_pDevice->CreateCommandList1( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS( m_pCommandList.GetAddressOf() ) ) );
}

void CGraphicsDemo::InitSwapChain( HWND hWindow )
{
	const CApplication& app = CApplication::GetInstance();

	const uint32_t width = static_cast<uint32_t>( app.GetWidth() );
	const uint32_t height = static_cast<uint32_t>( app.GetHeight() );

	const DXGI_SWAP_CHAIN_DESC1 desc = {
		.Width = width,
		.Height = height,
		.Format = m_backBufferFormat,
		.Stereo = false,
		.SampleDesc = { .Count = 1, .Quality = 0 },
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = c_swapChainBufferCount,
		.Scaling = DXGI_SCALING_NONE,
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
		.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
		.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	};

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
	ASSERT_SUCCEEDED( m_pDxgiFactory->CreateSwapChainForHwnd( m_pCommandQueue.Get(), hWindow, &desc, nullptr, nullptr, swapChain.GetAddressOf() ) );

	ASSERT_SUCCEEDED( swapChain.As( &m_pDxgiSwapChain ) );
}

void CGraphicsDemo::InitResourceViewHeaps()
{
	static constexpr uint32_t c_cbvSrvUavHeapCapacity = 16384;

	m_rtvHeap.Init( m_pDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, c_swapChainBufferCount );
	m_dsvHeap.Init( m_pDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1 );
	m_cbvSrvUavHeap.Init( m_pDevice.Get(), c_cbvSrvUavHeapCapacity );
}

void CGraphicsDemo::InitFenceObjects()
{
	ASSERT_SUCCEEDED( m_pDevice->CreateFence( m_currFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( m_pFence.GetAddressOf() ) ) );
	m_hFenceEvent = CreateEventEx( nullptr, nullptr, 0, EVENT_ALL_ACCESS );
}

void CGraphicsDemo::InitImGui()
{
	ImGui_ImplDX12_InitInfo initInfo = {};
	initInfo.Device = m_pDevice.Get();
	initInfo.CommandQueue = m_pCommandQueue.Get();
	initInfo.NumFramesInFlight = 1;
	initInfo.RTVFormat = m_backBufferFormat;
	initInfo.DSVFormat = m_depthStencilFormat;
	initInfo.UserData = &m_cbvSrvUavHeap;
	initInfo.SrvDescriptorHeap = m_cbvSrvUavHeap.GetHeap();

	initInfo.SrvDescriptorAllocFn = []( ImGui_ImplDX12_InitInfo* pInfo, D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* gpuHandle )
	{
		CCbvSrvUavHeap* const pHeap = static_cast<CCbvSrvUavHeap*>( pInfo->UserData );
		const uint32_t index = pHeap->GetNextIndex();

		*cpuHandle = pHeap->GetCpuHandle( index );
		*gpuHandle = pHeap->GetGpuHandle( index );
	};

	initInfo.SrvDescriptorFreeFn = []( ImGui_ImplDX12_InitInfo* pInfo, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle )
	{
		CCbvSrvUavHeap* const pHeap = static_cast<CCbvSrvUavHeap*>( pInfo->UserData );
		pHeap->ReleaseIndex( pHeap->GetIndexFromCpuHandle( cpuHandle ) );

		assert( pHeap->GetIndexFromCpuHandle( cpuHandle ) == pHeap->GetIndexFromGpuHandle( gpuHandle ) );
	};

	ImGui_ImplDX12_Init( &initInfo );
}

void CGraphicsDemo::FlushCommandQueue()
{
	++m_currFenceValue;
	ASSERT_SUCCEEDED( m_pCommandQueue->Signal( m_pFence.Get(), m_currFenceValue ) );

	if ( m_pFence->GetCompletedValue() >= m_currFenceValue )
	{
		return;
	}

	ASSERT_SUCCEEDED( m_pFence->SetEventOnCompletion( m_currFenceValue, m_hFenceEvent ) );
	WaitForSingleObject( m_hFenceEvent, INFINITE );
}

void CGraphicsDemo::CreateDepthStencilBufferAndView( int32_t width, int32_t height )
{
	const CD3DX12_RESOURCE_DESC depthStencilBufferDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		m_depthStencilFormat,                      // format
		width,                                     // width
		height,                                    // height
		1,                                         // arraySize
		1,                                         // mipLevels
		1,                                         // sampleCount
		0,                                         // sampleQuality
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL ); // flags

	ASSERT_SUCCEEDED( m_pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),   // pHeapProperties
		D3D12_HEAP_FLAG_NONE,                                  // HeapFlags
		&depthStencilBufferDesc,                               // pDesc
		D3D12_RESOURCE_STATE_DEPTH_WRITE,                      // InitialResourceState
		&CD3DX12_CLEAR_VALUE( m_depthStencilFormat, 1.0f, 0 ), // pOptimizedClearValue
		IID_PPV_ARGS( m_pDepthStencilBuffer.ReleaseAndGetAddressOf() ) ) );

	m_pDevice->CreateDepthStencilView( m_pDepthStencilBuffer.Get(), nullptr, m_dsvHeap.GetCpuHandle() );
}

void CGraphicsDemo::ResizeSwapChainAndCreateViews( int32_t width, int32_t height )
{
	for ( Microsoft::WRL::ComPtr<ID3D12Resource>& buffer : m_swapChainBuffers )
	{
		buffer.Reset();
	}

	ASSERT_SUCCEEDED( m_pDxgiSwapChain->ResizeBuffers( c_swapChainBufferCount, width, height, m_backBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH ) );

	for ( int32_t i = 0; i < c_swapChainBufferCount; ++i )
	{
		ASSERT_SUCCEEDED( m_pDxgiSwapChain->GetBuffer( i, IID_PPV_ARGS( m_swapChainBuffers[i].GetAddressOf() ) ) );
		m_pDevice->CreateRenderTargetView( m_swapChainBuffers[i].Get(), nullptr, m_rtvHeap.GetCpuHandle( i ) );
	}

	m_currBackBufferIndex = 0;
}

CGraphicsDemo::~CGraphicsDemo()
{
	ImGui_ImplDX12_Shutdown();
	FlushCommandQueue();
}

void CGraphicsDemo::Update()
{
	CApplication& app = CApplication::GetInstance();
	const CInput& input = app.GetInput();

	if ( input.IsKeyJustPressed( SDL_SCANCODE_ESCAPE ) )
	{
		app.Quit();
	}
}

void CGraphicsDemo::OnResize()
{
	const CApplication& app = CApplication::GetInstance();
	const int32_t width = app.GetWidth();
	const int32_t height = app.GetHeight();

	FlushCommandQueue();
	ResizeSwapChainAndCreateViews( width, height );
	CreateDepthStencilBufferAndView( width, height );

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
}
