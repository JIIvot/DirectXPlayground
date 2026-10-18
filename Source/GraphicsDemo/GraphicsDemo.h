#pragma once

#include "CbvSrvUavHeap.h"

#include <dxgi1_6.h>
#include <array>

class CGraphicsDemo
{
public:
    CGraphicsDemo( const CGraphicsDemo& ) = delete;
    virtual ~CGraphicsDemo();

    CGraphicsDemo& operator=( const CGraphicsDemo& ) = delete;

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
    static constexpr int32_t c_swapChainBufferCount = 2;

    DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    Microsoft::WRL::ComPtr<IDXGIFactory6>   m_pDxgiFactory;
    Microsoft::WRL::ComPtr<IDXGISwapChain4> m_pDxgiSwapChain;
    Microsoft::WRL::ComPtr<IDXGIAdapter4>   m_pDxgiAdapter;

    Microsoft::WRL::ComPtr<ID3D12Device5> m_pDevice;

    Microsoft::WRL::ComPtr<ID3D12CommandQueue>        m_pCommandQueue;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_pCommandList;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>    m_pCommandAllocator;

    Microsoft::WRL::ComPtr<ID3D12Fence> m_pFence;
    uint64_t                            m_currFenceValue = 0;
    HANDLE                              m_hFenceEvent = nullptr;

    int32_t                                                                    m_currBackBufferIndex = 0;
    std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, c_swapChainBufferCount> m_swapChainBuffers;
    Microsoft::WRL::ComPtr<ID3D12Resource>                                     m_pDepthStencilBuffer;

    D3D12_VIEWPORT m_viewport = {};
    D3D12_RECT     m_scissorRect = {};

    CResourceViewHeap m_rtvHeap;
    CResourceViewHeap m_dsvHeap;
    CCbvSrvUavHeap    m_cbvSrvUavHeap;
};
