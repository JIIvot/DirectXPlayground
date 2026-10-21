#include "Application.h"

#include <SDL3/SDL.h>
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>

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
    
    SDL_ShowWindow( m_pWindow );

    while ( m_shouldRun )
    {
        m_input.Update();
        ProcessEvents();

        m_pDemo->Update();

        ImGui_ImplSDL3_NewFrame();
        m_pDemo->Render();
    }
}

CApplication::CApplication()
{
    if ( !InitWindow() )
    {
        return;
    }

    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForD3D( m_pWindow );

    m_input.Init();
    m_shouldRun = true;
}

CApplication::~CApplication()
{
    m_pDemo.reset();

    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyWindow( m_pWindow );
    SDL_Quit();
}

bool CApplication::InitWindow()
{
    if ( !SDL_Init( SDL_INIT_VIDEO ) )
    {
        return false;
    }

    const SDL_DisplayMode* const displayMode = SDL_GetCurrentDisplayMode( SDL_GetPrimaryDisplay() );

    m_clientWidth = displayMode->w * 2 / 3;
    m_clientHeight = displayMode->h * 2 / 3;
    m_pWindow = SDL_CreateWindow( kWindowTitle, m_clientWidth, m_clientHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN );

    if ( !m_pWindow )
    {
        return false;
    }

    const SDL_PropertiesID propertiesId = SDL_GetWindowProperties( m_pWindow );
    m_hNativeWindow = static_cast<HWND>( SDL_GetPointerProperty( propertiesId, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr ) );

    if ( !m_hNativeWindow )
    {
        return false;
    }

    return true;
}

void CApplication::ProcessEvents()
{
    SDL_Event event;
    while ( SDL_PollEvent( &event ) )
    {
        ImGui_ImplSDL3_ProcessEvent( &event );

        switch ( event.type )
        {
        case SDL_EVENT_WINDOW_RESIZED:
            OnResize( event.window.data1, event.window.data2 );
            break;

        case SDL_EVENT_QUIT:
            Quit();
            break;

        default:
            break;
        }
    }
}

void CApplication::OnResize( int32_t newWidth, int32_t newHeight )
{
    m_clientWidth = newWidth;
    m_clientHeight = newHeight;

    m_pDemo->OnResize();
}
