#include "Application.h"
#include "Utility.h"

#include <cassert>
#include <SDL3/SDL.h>

static CApplication* s_instance = nullptr;

void CApplication::Init()
{
	assert( !s_instance );
	s_instance = new CApplication;
}

CApplication& CApplication::GetInstance()
{
	assert( s_instance );
	return *s_instance;
}

void CApplication::Destroy()
{
	delete s_instance;
	s_instance = nullptr;
}

void CApplication::Run()
{
	assert( s_instance );

	while ( m_shouldRun )
	{
		ProcessEvents();
	}
}

void CApplication::Quit()
{
	m_shouldRun = false;
}

CApplication::CApplication()
{
	if ( !InitWindow() || !InitGraphics() )
	{
		return;
	}

	m_shouldRun = true;
}

CApplication::~CApplication()
{
	SDL_DestroyWindow( m_window );
	SDL_Quit();
}

bool CApplication::InitWindow()
{
	if ( !SDL_Init( SDL_INIT_VIDEO ) )
	{
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, m_windowTitle.c_str(), "Failed to initialize SDL3!", nullptr );
		return false;
	}

	m_window = SDL_CreateWindow( m_windowTitle.c_str(), m_clientWidth, m_clientHeight, SDL_WINDOW_RESIZABLE );

	if ( !m_window )
	{
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, m_windowTitle.c_str(), "Failed to create a window!", nullptr );
		return false;
	}

	const SDL_PropertiesID propertiesId = SDL_GetWindowProperties( m_window );
	m_windowHandle = static_cast<HWND>( SDL_GetPointerProperty( propertiesId, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr ) );

	if ( !m_windowHandle )
	{
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, m_windowTitle.c_str(), "Failed to get the native window handle!", nullptr );
		return false;
	}

	return true;
}

bool CApplication::InitGraphics()
{
	InitDebugAndDxgiFactory();
	InitAdapterAndDevice();

	ASSERT_SUCCEEDED( m_device->CreateFence( m_currFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( m_fence.GetAddressOf() ) ) );

	InitCommandObjects();
	InitSwapChain();
	InitRtvDsvHeapManagers();

	return true;
}

void CApplication::InitDebugAndDxgiFactory()
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

void CApplication::InitAdapterAndDevice()
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

void CApplication::InitCommandObjects()
{
	const D3D12_COMMAND_QUEUE_DESC queueDesc = {
		.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
		.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE
	};

	ASSERT_SUCCEEDED( m_device->CreateCommandQueue( &queueDesc, IID_PPV_ARGS( m_commandQueue.GetAddressOf() ) ) );
	ASSERT_SUCCEEDED( m_device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( m_commandAllocator.GetAddressOf() ) ) );
	ASSERT_SUCCEEDED( m_device->CreateCommandList1( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS( m_commandList.GetAddressOf() ) ) );
}

void CApplication::InitSwapChain()
{
	m_dxgiSwapChain.Reset();

	const DXGI_SWAP_CHAIN_DESC1 desc = {
		.Width = static_cast<UINT>( m_clientWidth ),
		.Height = static_cast<UINT>( m_clientHeight ),
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
	ASSERT_SUCCEEDED( m_dxgiFactory->CreateSwapChainForHwnd( m_commandQueue.Get(), m_windowHandle, &desc, nullptr, nullptr, swapChain.GetAddressOf() ) );

	ASSERT_SUCCEEDED( swapChain.As( &m_dxgiSwapChain ) );
}

void CApplication::InitRtvDsvHeapManagers()
{
	m_rtvHeapManager.Init( m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, c_swapChainBufferCount );
	m_dsvHeapManager.Init( m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1 );
}

void CApplication::ProcessEvents()
{
	SDL_Event event;
	while ( SDL_PollEvent( &event ) )
	{
		if ( event.type == SDL_EVENT_QUIT || ( event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_ESCAPE ) )
		{
			Quit();
		}
	}
}
