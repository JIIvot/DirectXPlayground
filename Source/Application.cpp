#include "Application.h"
#include "ApplicationConstants.h"

#include <imgui.h>
#include <backends/imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

CApplication* CApplication::s_pInstance;

CApplication& CApplication::GetInstance()
{
    assert( s_pInstance );
    return *s_pInstance;
}

void CApplication::Destroy()
{
    delete s_pInstance;
    s_pInstance = nullptr;
}

void CApplication::Run()
{
    assert( s_pInstance );
    assert( m_pDemo );

    ShowWindow( m_hWindow, SW_SHOW );

    MSG message = {};
    while ( message.message != WM_QUIT )
    {
        if ( PeekMessage( &message, nullptr, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &message );
            DispatchMessage( &message );

            continue;
        }

        m_pDemo->Update();

        ImGui_ImplWin32_NewFrame();
        m_pDemo->Render();

        m_input.Update();
    }
}

CApplication::CApplication()
{
    if ( !InitWindow() )
    {
        return;
    }

    ImGui::CreateContext();
    ImGui_ImplWin32_Init( m_hWindow );
}

CApplication::~CApplication()
{
    m_pDemo.reset();

    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

LRESULT CApplication::WindowProcedure( HWND hWindow, uint32_t message, WPARAM wParam, LPARAM lParam )
{
    if ( ImGui_ImplWin32_WndProcHandler( hWindow, message, wParam, lParam ) )
    {
        return 0;
    }

    if ( s_pInstance )
    {
        s_pInstance->ProcessWindowMessage( hWindow, message, wParam, lParam );
    }

    return DefWindowProc( hWindow, message, wParam, lParam );
}

LRESULT CApplication::ProcessWindowMessage( HWND hWindow, uint32_t message, WPARAM wParam, LPARAM lParam )
{
    // TODO: Add resizing support

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
    }

    return DefWindowProc( hWindow, message, wParam, lParam );
}

bool CApplication::InitWindow()
{
    static constexpr wchar_t kClassName[] = L"App";
    static constexpr DWORD   kStyle = WS_OVERLAPPEDWINDOW;

    SetThreadDpiAwarenessContext( DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 );

    HINSTANCE hInstance = GetModuleHandle( nullptr );

    WNDCLASS windowClass = {};
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProcedure;
    windowClass.hInstance = hInstance;
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

    m_hWindow = CreateWindow(
        kClassName,   // lpClassName
        kWindowTitle, // lpWindowName
        kStyle,       // dwStyle
        windowX,      // x
        windowY,      // y
        windowWidth,  // nWidth
        windowHeight, // nHeight
        nullptr,      // hWndParent
        nullptr,      // hMenu
        hInstance,    // hInstance
        nullptr );    // lpParam

    if ( !m_hWindow )
    {
        MessageBox( nullptr, L"Failed to create a window!", kWindowTitle, MB_ICONERROR );
        return false;
    }

    return true;
}

void CApplication::OnResize( int32_t newWidth, int32_t newHeight )
{
    m_clientWidth = newWidth;
    m_clientHeight = newHeight;

    m_pDemo->OnResize();
}
