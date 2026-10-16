#pragma once

#include "DescriptorHeapManager.h"
#include "Input.h"

#include <Windows.h>
#include <cstdint>
#include <string>
#include <dxgi1_6.h>
#include <wrl.h>
#include <array>

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

	void Run();

	void Quit()
	{
		m_shouldRun = false;
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

	bool InitGraphics();

	void InitDebugAndDxgiFactory();

	void InitAdapterAndDevice();

	void InitCommandObjects();

	void InitSwapChain();

	void InitRtvDsvHeapManagers();

	void ProcessEvents();

private:
	static constexpr int32_t c_swapChainBufferCount = 2;

	bool m_shouldRun = false;

	CInput m_input;

	SDL_Window* m_window = nullptr;
	HWND m_windowHandle = nullptr;

	int32_t m_clientWidth = 1280;
	int32_t m_clientHeight = 720;
	std::string m_windowTitle = "DirectX 12 Engine";

	DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	ComPtr<IDXGIFactory6> m_dxgiFactory;
	ComPtr<IDXGISwapChain4> m_dxgiSwapChain;
	ComPtr<IDXGIAdapter4> m_dxgiAdapter;

	ComPtr<ID3D12Device5> m_device;

	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;

	ComPtr<ID3D12Fence> m_fence;
	uint64_t m_currFenceValue = 0;

	int32_t m_currBackBufferIndex = 0;
	std::array<ComPtr<ID3D12Resource>, c_swapChainBufferCount> m_swapChainBuffers;
	ComPtr<ID3D12Resource> m_depthStencilBuffer;

	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;

	CDescriptorHeapManager m_rtvHeapManager;
	CDescriptorHeapManager m_dsvHeapManager;
};
