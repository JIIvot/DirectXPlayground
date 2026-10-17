#include "GraphicsDemo.h"
#include "Utility.h"
#include "Application.h"

CGraphicsDemo::CGraphicsDemo()
{
	InitDebugAndDxgiFactory();
	InitAdapterAndDevice();

	ASSERT_SUCCEEDED( m_device->CreateFence( m_currFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( m_fence.GetAddressOf() ) ) );

	InitCommandObjects();
	InitSwapChain();
	InitRtvDsvHeapManagers();

	OnResize();
}

void CGraphicsDemo::InitDebugAndDxgiFactory()
{
#ifdef _DEBUG
	static constexpr uint32_t c_factoryFlags = DXGI_CREATE_FACTORY_DEBUG;

	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	ComPtr<ID3D12Debug1> debugController;
	ASSERT_SUCCEEDED( D3D12GetDebugInterface( IID_PPV_ARGS( debugController.GetAddressOf() ) ) );

	debugController->EnableDebugLayer();
#else
	static constexpr uint32_t c_factoryFlags = 0;
#endif

	ASSERT_SUCCEEDED( CreateDXGIFactory2( c_factoryFlags, IID_PPV_ARGS( m_dxgiFactory.GetAddressOf() ) ) );
}

void CGraphicsDemo::InitAdapterAndDevice()
{
	ComPtr<IDXGIAdapter> adapter;
	HRESULT deviceCreationResult = E_FAIL;

	for ( int32_t i = 0; m_dxgiFactory->EnumAdapters( i, adapter.ReleaseAndGetAddressOf() ) != DXGI_ERROR_NOT_FOUND; ++i )
	{
		deviceCreationResult = D3D12CreateDevice( adapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS( m_device.ReleaseAndGetAddressOf() ) );

		if ( SUCCEEDED( deviceCreationResult ) )
		{
			break;
		}
	}

	ASSERT_SUCCEEDED( deviceCreationResult );
	ASSERT_SUCCEEDED( adapter.As( &m_dxgiAdapter ) );
}

void CGraphicsDemo::InitCommandObjects()
{
	const D3D12_COMMAND_QUEUE_DESC queueDesc = {
		.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
		.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE
	};

	ASSERT_SUCCEEDED( m_device->CreateCommandQueue( &queueDesc, IID_PPV_ARGS( m_commandQueue.GetAddressOf() ) ) );
	ASSERT_SUCCEEDED( m_device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( m_commandAllocator.GetAddressOf() ) ) );
	ASSERT_SUCCEEDED( m_device->CreateCommandList1( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS( m_commandList.GetAddressOf() ) ) );
}

void CGraphicsDemo::InitSwapChain()
{
	const CApplication& app = CApplication::GetInstance();

	const uint32_t width = static_cast<uint32_t>( app.GetWidth() );
	const uint32_t height = static_cast<uint32_t>( app.GetHeight() );
	const HWND windowHandle = app.GetWindowHandle();

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

	ComPtr<IDXGISwapChain1> swapChain;
	ASSERT_SUCCEEDED( m_dxgiFactory->CreateSwapChainForHwnd( m_commandQueue.Get(), windowHandle, &desc, nullptr, nullptr, swapChain.GetAddressOf() ) );

	ASSERT_SUCCEEDED( swapChain.As( &m_dxgiSwapChain ) );
}

void CGraphicsDemo::InitRtvDsvHeapManagers()
{
	m_rtvHeapManager.Init( m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, c_swapChainBufferCount );
	m_dsvHeapManager.Init( m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1 );
}

void CGraphicsDemo::FlushCommandQueue()
{
	++m_currFenceValue;
	ASSERT_SUCCEEDED( m_commandQueue->Signal( m_fence.Get(), m_currFenceValue ) );

	if ( m_fence->GetCompletedValue() >= m_currFenceValue )
	{
		return;
	}

	HANDLE eventHandle = CreateEventEx( nullptr, nullptr, 0, EVENT_ALL_ACCESS );
	m_fence->SetEventOnCompletion( m_currFenceValue, eventHandle );

	WaitForSingleObject( eventHandle, INFINITE );
	CloseHandle( eventHandle );
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

	for ( int32_t i = 0; i < c_swapChainBufferCount; ++i )
	{
		m_swapChainBuffers[i].Reset();
	}

	ASSERT_SUCCEEDED( m_dxgiSwapChain->ResizeBuffers( c_swapChainBufferCount, width, height, m_backBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH ) );

	for ( int32_t i = 0; i < c_swapChainBufferCount; ++i )
	{
		m_dxgiSwapChain->GetBuffer( i, IID_PPV_ARGS( m_swapChainBuffers[i].GetAddressOf() ) );
		m_device->CreateRenderTargetView( m_swapChainBuffers[i].Get(), nullptr, m_rtvHeapManager.GetCpuHandle( i ) );
	}

	const CD3DX12_RESOURCE_DESC depthStencilBufferDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		m_depthStencilFormat,                      // format
		width,                                     // width
		height,                                    // height
		1,                                         // arraySize
		1,                                         // mipLevels
		1,                                         // sampleCount
		0,                                         // sampleQuality
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL ); // flags

	ASSERT_SUCCEEDED( m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),   // pHeapProperties
		D3D12_HEAP_FLAG_NONE,                                  // HeapFlags
		&depthStencilBufferDesc,                               // pDesc
		D3D12_RESOURCE_STATE_DEPTH_WRITE,                      // InitialResourceState
		&CD3DX12_CLEAR_VALUE( m_depthStencilFormat, 1.0f, 0 ), // pOptimizedClearValue
		IID_PPV_ARGS( m_depthStencilBuffer.ReleaseAndGetAddressOf() ) ) );

	m_device->CreateDepthStencilView( m_depthStencilBuffer.Get(), nullptr, m_dsvHeapManager.GetCpuHandle( 0 ) );

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

	m_currBackBufferIndex = 0;
}
