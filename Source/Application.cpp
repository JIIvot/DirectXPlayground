#include "Application.h"
#include "ApplicationConstants.h"
#include "Utility.h"

#include <imgui.h>
#include <backends/imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

Application* Application::s_instance;

Application& Application::Get()
{
    assert( s_instance );
    return *s_instance;
}

void Application::Destroy()
{
    delete s_instance;
    s_instance = nullptr;
}

void Application::Run()
{
    assert( s_instance );
    assert( m_demo );

    ShowWindow( m_windowHandle, SW_SHOW );

    MSG message = {};
    while ( message.message != WM_QUIT )
    {
        if ( PeekMessage( &message, nullptr, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &message );
            DispatchMessage( &message );

            continue;
        }

        if ( m_isPaused )
        {
            Sleep( 100 );
            continue;
        }

        if ( m_shouldResize )
        {
            m_demo->OnResize( m_clientWidth, m_clientHeight );
            m_shouldResize = false;
        }

        m_demo->OnUpdate();
        m_demo->OnRender();

        m_input.OnUpdateFinished();
    }

    m_demo->OnQuit();
}

Application::Application()
{
    if ( !InitWindow() )
    {
        return;
    }

    CreateGraphicsContext();

    LOG_INFO( "Application initialized.\n" );
}

LRESULT Application::WindowProcedure( HWND windowHandle, uint32_t message, WPARAM wParam, LPARAM lParam )
{
    if ( ImGui_ImplWin32_WndProcHandler( windowHandle, message, wParam, lParam ) )
    {
        return 0;
    }

    if ( s_instance )
    {
        return s_instance->ProcessWindowMessage( windowHandle, message, wParam, lParam );
    }

    return DefWindowProc( windowHandle, message, wParam, lParam );
}

LRESULT Application::ProcessWindowMessage( HWND windowHandle, uint32_t message, WPARAM wParam, LPARAM lParam )
{
    switch ( message )
    {
    case WM_DESTROY:
        PostQuitMessage( 0 );
        return 0;

    case WM_SYSKEYDOWN:
        [[fallthrough]];

    case WM_KEYDOWN:
        m_input.OnKeyDown( wParam );
        return 0;

    case WM_SYSKEYUP:
        [[fallthrough]];

    case WM_KEYUP:
        m_input.OnKeyUp( wParam );
        return 0;

    case WM_ACTIVATE:
        m_isPaused = LOWORD( wParam ) == WA_INACTIVE;
        return 0;

    case WM_SIZE:
    {
        if ( wParam == SIZE_MINIMIZED )
        {
            return 0;
        }

        const int32_t newWidth = LOWORD( lParam );
        const int32_t newHeight = HIWORD( lParam );

        m_shouldResize = newWidth != m_clientWidth || newHeight != m_clientHeight;
        m_clientWidth = newWidth;
        m_clientHeight = newHeight;
        return 0;
    }

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* const info = reinterpret_cast<MINMAXINFO*>( lParam );
        info->ptMinTrackSize.x = 200;
        info->ptMinTrackSize.y = 200;
        return 0;
    }

    default:
        break;
    }

    return DefWindowProc( windowHandle, message, wParam, lParam );
}

bool Application::InitWindow()
{
    static constexpr wchar_t kClassName[] = L"App";
    static constexpr DWORD   kStyle = WS_OVERLAPPEDWINDOW;

    SetThreadDpiAwarenessContext( DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 );

    const HINSTANCE instanceHandle = GetModuleHandle( nullptr );

    WNDCLASS windowClass = {};
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProcedure;
    windowClass.hInstance = instanceHandle;
    windowClass.hIcon = LoadIcon( nullptr, IDI_APPLICATION );
    windowClass.hCursor = LoadCursor( nullptr, IDC_ARROW );
    windowClass.lpszClassName = kClassName;

    if ( !RegisterClass( &windowClass ) )
    {
        MessageBox( nullptr, L"Failed to register a window class!", kWindowTitle, MB_ICONERROR );
        return false;
    }

    const int32_t screenWidth = GetSystemMetrics( SM_CXSCREEN );
    const int32_t screenHeight = GetSystemMetrics( SM_CYSCREEN );

    m_clientWidth = screenWidth / 3 * 2;
    m_clientHeight = screenHeight / 3 * 2;

    RECT windowRect = { 0, 0, m_clientWidth, m_clientHeight };
    AdjustWindowRect( &windowRect, kStyle, false );

    const int32_t windowWidth = windowRect.right - windowRect.left;
    const int32_t windowHeight = windowRect.bottom - windowRect.top;
    const int32_t windowX = ( screenWidth - windowWidth ) / 2;
    const int32_t windowY = ( screenHeight - windowHeight ) / 2;

    m_windowHandle = CreateWindow(
        kClassName,     // lpClassName
        kWindowTitle,   // lpWindowName
        kStyle,         // dwStyle
        windowX,        // x
        windowY,        // y
        windowWidth,    // nWidth
        windowHeight,   // nHeight
        nullptr,        // hWndParent
        nullptr,        // hMenu
        instanceHandle, // hInstance
        nullptr );      // lpParam

    if ( !m_windowHandle )
    {
        MessageBox( nullptr, L"Failed to create a window!", kWindowTitle, MB_ICONERROR );
        return false;
    }

    LOG_TRACE( "Window created. Client width: %d, Client height: %d\n", m_clientWidth, m_clientHeight );
    return true;
}

void Application::CreateGraphicsContext()
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

    HRESULT deviceCreationResult = E_FAIL;
    for ( uint32_t i = 0; m_dxgiFactory->EnumAdapterByGpuPreference( i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS( m_dxgiAdapter.ReleaseAndGetAddressOf() ) ) != DXGI_ERROR_NOT_FOUND; ++i )
    {
        deviceCreationResult = D3D12CreateDevice( m_dxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS( m_device.ReleaseAndGetAddressOf() ) );
        if ( SUCCEEDED( deviceCreationResult ) )
        {
            break;
        }
    }

    ASSERT_SUCCEEDED( deviceCreationResult );

    DXGI_ADAPTER_DESC adapterDesc;
    m_dxgiAdapter->GetDesc( &adapterDesc );

    LOG_TRACE( "Graphics context created. Adapter name: %ws\n", adapterDesc.Description );
}
