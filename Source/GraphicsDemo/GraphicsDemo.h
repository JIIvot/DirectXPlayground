#pragma once

#include "CbvSrvUavHeap.h"

#include <dxgi1_6.h>
#include <array>

class CGraphicsDemo
{
private:
    template<typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
    virtual ~CGraphicsDemo();

    CGraphicsDemo( const CGraphicsDemo& ) = delete;
    CGraphicsDemo& operator=( const CGraphicsDemo& ) = delete;

    CGraphicsDemo( CGraphicsDemo&& ) = delete;
    CGraphicsDemo& operator=( CGraphicsDemo&& ) = delete;

    virtual void Update();
    virtual void Render() = 0;

    void OnResize();

protected:
    explicit CGraphicsDemo( HWND hWindow );

    void InitDebugAndDxgiFactory();
    void InitAdapterAndDevice();
    void InitCommandObjects();
    void InitSwapChain( HWND hWindow );
    void InitResourceViewHeaps();
    void InitFenceObjects();
    void InitImGui();

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
        return m_rtvHeap.GetCpuHandle( m_currBackBufferIndex );
    }

protected:
    static constexpr int32_t kNumSwapChainBuffers = 2;

    DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    ComPtr<IDXGIFactory6>   m_pDxgiFactory;
    ComPtr<IDXGISwapChain4> m_pDxgiSwapChain;
    ComPtr<IDXGIAdapter4>   m_pDxgiAdapter;

    ComPtr<ID3D12Device5> m_pDevice;

    ComPtr<ID3D12CommandQueue>        m_pCommandQueue;
    ComPtr<ID3D12GraphicsCommandList> m_pCommandList;
    ComPtr<ID3D12CommandAllocator>    m_pCommandAllocator;

    ComPtr<ID3D12Fence> m_pFence;
    uint64_t            m_currFenceValue = 0;
    HANDLE              m_hFenceEvent = nullptr;

    int32_t                                                    m_currBackBufferIndex = 0;
    std::array<ComPtr<ID3D12Resource>, kNumSwapChainBuffers> m_swapChainBuffers;
    ComPtr<ID3D12Resource>                                     m_pDepthStencilBuffer;

    D3D12_VIEWPORT m_viewport = {};
    D3D12_RECT     m_scissorRect = {};

    CResourceViewHeap m_rtvHeap;
    CResourceViewHeap m_dsvHeap;
    CCbvSrvUavHeap    m_cbvSrvUavHeap;
};
