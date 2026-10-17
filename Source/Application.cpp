#include "Application.h"

#include <SDL3/SDL.h>
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>

CApplication* CApplication::s_instance;

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
	assert( m_demo );

	while ( m_shouldRun )
	{
		m_input.Update();
		ProcessEvents();

		m_demo->Update();

		ImGui_ImplSDL3_NewFrame();
		m_demo->Render();
	}
}

CApplication::CApplication()
{
	if ( !InitWindow() )
	{
		return;
	}

	ImGui::CreateContext();
	ImGui_ImplSDL3_InitForD3D( m_window );

	m_input.Init();
	m_shouldRun = true;
}

CApplication::~CApplication()
{
	m_demo.reset();

	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyWindow( m_window );
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
	m_window = SDL_CreateWindow( c_windowTitle, m_clientWidth, m_clientHeight, SDL_WINDOW_RESIZABLE );

	if ( !m_window )
	{
		return false;
	}

	const SDL_PropertiesID propertiesId = SDL_GetWindowProperties( m_window );
	m_windowHandle = static_cast<HWND>( SDL_GetPointerProperty( propertiesId, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr ) );

	if ( !m_windowHandle )
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

	m_demo->OnResize();
}
