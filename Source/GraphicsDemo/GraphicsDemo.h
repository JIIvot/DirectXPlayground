#pragma once

#include "DescriptorHeapManager.h"

#include <dxgi1_6.h>
#include <Windows.h>
#include <array>

class CGraphicsDemo
{
protected:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	virtual ~CGraphicsDemo();

	CGraphicsDemo( const CGraphicsDemo& ) = delete;

	CGraphicsDemo& operator=( const CGraphicsDemo& ) = delete;

	virtual void Update();

	virtual void Render() = 0;

	void OnResize();

protected:
	CGraphicsDemo( HWND windowHandle );

	void InitDebugAndDxgiFactory();

	void InitAdapterAndDevice();

	void InitCommandObjects();

	void InitSwapChain( HWND windowHandle );

	void InitRtvDsvHeapManagers();

	void InitFenceObjects();

	void FlushCommandQueue();

	void CreateDepthStencilBufferAndView( int32_t width, int32_t height );

	void ResizeSwapChainAndCreateViews( int32_t width, int32_t height );

	[[nodiscard]]
	ID3D12Resource* GetCurrBackBufferResource() const
	{
		return m_swapChainBuffers[m_currBackBufferIndex].Get();
	}

	[[nodiscard]]
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCurrBackBufferViewHandle() const
	{
		return m_rtvHeapManager.GetCpuHandle( m_currBackBufferIndex );
	}

	[[nodiscard]]
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthStencilViewHandle() const
	{
		return m_dsvHeapManager.GetCpuHandle( 0 );
	}

protected:
	static constexpr int32_t c_swapChainBufferCount = 2;

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
	HANDLE m_fenceEventHandle = nullptr;

	int32_t m_currBackBufferIndex = 0;
	std::array<ComPtr<ID3D12Resource>, c_swapChainBufferCount> m_swapChainBuffers;
	ComPtr<ID3D12Resource> m_depthStencilBuffer;

	D3D12_VIEWPORT m_viewport = {};
	D3D12_RECT m_scissorRect = {};

	CDescriptorHeapManager m_rtvHeapManager;
	CDescriptorHeapManager m_dsvHeapManager;
};
