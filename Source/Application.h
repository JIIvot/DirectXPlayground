#pragma once

#include "DescriptorHeapManager.h"
#include "Input.h"
#include "GraphicsDemo/GraphicsDemo.h"

#include <Windows.h>
#include <cassert>
#include <cstdint>
#include <string>
#include <dxgi1_6.h>
#include <wrl.h>
#include <array>
#include <memory>

struct SDL_Window;

class CApplication
{
private:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	CApplication( const CApplication& ) = delete;

	CApplication& operator=( const CApplication& ) = delete;

	static void Init();

	static CApplication& GetInstance();

	static void Destroy();

	template<typename TGraphicsDemo>
	void SetGraphicsDemo();

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
	HWND GetWindowHandle() const
	{
		return m_windowHandle;
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

	bool m_shouldRun = false;

	CInput m_input;

	std::unique_ptr<CGraphicsDemo> m_demo;

	SDL_Window* m_window = nullptr;
	HWND m_windowHandle = nullptr;

	int32_t m_clientWidth = 1280;
	int32_t m_clientHeight = 720;
};

template<typename TGraphicsDemo>
inline void CApplication::SetGraphicsDemo()
{
	assert( m_shouldRun );
	assert( !m_demo );

	m_demo = std::make_unique<TGraphicsDemo>();
}
