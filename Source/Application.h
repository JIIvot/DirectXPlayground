#pragma once

#include "GraphicsDemo/GraphicsDemo.h"
#include "Input.h"

#include <cassert>
#include <memory>

struct SDL_Window;

class CApplication
{
private:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	template<typename TGraphicsDemo>
	static void Init();

	static CApplication& GetInstance();

	static void Destroy();

	void Run();

	void Quit()
	{
		m_shouldRun = false;
	}

	[[nodiscard]]
	int32_t GetWidth() const
	{
		return m_clientWidth;
	}

	[[nodiscard]]
	int32_t GetHeight() const
	{
		return m_clientHeight;
	}

	[[nodiscard]]
	const CInput& GetInput() const
	{
		return m_input;
	}

private:
	CApplication();

	~CApplication();

	bool InitWindow();

	void ProcessEvents();

	void OnResize( int32_t newWidth, int32_t newHeight );

private:
	static constexpr char c_windowTitle[] = "DirectX 12 Engine";

	inline static CApplication* s_instance = nullptr;

	bool m_shouldRun = false;

	CInput m_input;

	std::unique_ptr<CGraphicsDemo> m_demo;

	SDL_Window* m_window = nullptr;
	HWND m_windowHandle = nullptr;

	int32_t m_clientWidth = 1280;
	int32_t m_clientHeight = 720;
};

template<typename TGraphicsDemo>
void CApplication::Init()
{
	assert( !s_instance );

	s_instance = new CApplication;
	if ( s_instance->m_shouldRun )
	{
		s_instance->m_demo = std::make_unique<TGraphicsDemo>( s_instance->m_windowHandle );
	}
}
