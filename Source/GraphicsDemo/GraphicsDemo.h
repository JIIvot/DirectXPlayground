#pragma once

#include "CbvSrvUavHeap.h"
#include "GraphicsDemoCreateInfo.h"
#include "Input.h"

#include <dxgi1_6.h>
#include <ResourceUploadBatch.h>

class GraphicsDemo
{
public:
    virtual ~GraphicsDemo();

    GraphicsDemo( const GraphicsDemo& ) = delete;
    GraphicsDemo& operator=( const GraphicsDemo& ) = delete;

    GraphicsDemo( GraphicsDemo&& ) = delete;
    GraphicsDemo& operator=( GraphicsDemo&& ) = delete;

    virtual void OnUpdate( const Input& input );
    virtual void OnRender() = 0;

    virtual void OnResize( int32_t width, int32_t height );

    void OnQuit();

protected:
    explicit GraphicsDemo( const GraphicsDemoCreateInfo& info );

    void FlushCommandQueue();

    [[nodiscard]] ID3D12Resource* GetCurrBackBufferResource() const
    {
        return m_swapChainBuffers[m_currBackBufferIndex].Get();
    }

    [[nodiscard]] CD3DX12_CPU_DESCRIPTOR_HANDLE GetCurrBackBufferViewHandle() const
    {
        return m_rtvHeap.GetCpuHandle( m_currBackBufferIndex );
    }

private:
    void InitCommandObjects();
    void InitSwapChainAndDepthStencilBuffer( HWND windowHandle, int32_t width, int32_t height );
    void InitFenceObjects();
    void InitImGui( HWND windowHandle );

    void CreateDepthStencilBufferAndView( int32_t width, int32_t height );
    void ResizeSwapChain( int32_t width, int32_t height );
    void CreateSwapChainBufferViews( int32_t width, int32_t height );
    void UpdateViewportScissorsAndAspectRatio( int32_t width, int32_t height );

protected:
    static constexpr int32_t     kNumSwapChainBuffers = 2;
    static constexpr DXGI_FORMAT kBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    static constexpr DXGI_FORMAT kDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    Microsoft::WRL::ComPtr<IDXGIFactory6> m_dxgiFactory;
    Microsoft::WRL::ComPtr<ID3D12Device5> m_device;

    Microsoft::WRL::ComPtr<IDXGISwapChain4> m_dxgiSwapChain;
    Microsoft::WRL::ComPtr<IDXGIAdapter4>   m_dxgiAdapter;

    Microsoft::WRL::ComPtr<ID3D12CommandQueue>        m_commandQueue;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>    m_commandAllocator;

    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
    uint64_t                            m_currFenceValue = 0;
    HANDLE                              m_fenceEventHandle = nullptr;

    int32_t                                m_currBackBufferIndex = 0;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_swapChainBuffers[kNumSwapChainBuffers];
    Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencilBuffer;

    DirectX::ResourceUploadBatch m_uploadBatch;

    D3D12_VIEWPORT m_viewport = {};
    D3D12_RECT     m_scissorRect = {};

    ResourceViewHeap m_rtvHeap;
    ResourceViewHeap m_dsvHeap;
    CbvSrvUavHeap    m_cbvSrvUavHeap;

    float m_aspectRatio = 0.0f;
};
