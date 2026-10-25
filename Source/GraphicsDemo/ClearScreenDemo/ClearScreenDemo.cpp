#include "ClearScreenDemo.h"
#include "Utility.h"

#include <DirectXColors.h>
#include <backends/imgui_impl_dx12.h>
#include <d3dx12/d3dx12_barriers.h>

CClearScreenDemo::CClearScreenDemo( HWND windowHandle )
    : CGraphicsDemo( windowHandle )
{
    DXGI_ADAPTER_DESC1 desc;
    ASSERT_SUCCEEDED( m_dxgiAdapter->GetDesc1( &desc ) );

    m_adapterName = desc.Description;
}

void CClearScreenDemo::Render()
{
    ASSERT_SUCCEEDED( m_commandAllocator->Reset() );
    ASSERT_SUCCEEDED( m_commandList->Reset( m_commandAllocator.Get(), nullptr ) );

    BuildImmediateUi();

    ID3D12DescriptorHeap* const cbvSrvUavHeap = m_cbvSrvUavHeap.GetHeap();
    m_commandList->SetDescriptorHeaps( 1, &cbvSrvUavHeap );

    m_commandList->RSSetScissorRects( 1, &m_scissorRect );
    m_commandList->RSSetViewports( 1, &m_viewport );

    m_commandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( GetCurrBackBufferResource(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET ) );

    m_commandList->ClearRenderTargetView( GetCurrBackBufferViewHandle(), DirectX::Colors::CornflowerBlue, 0, nullptr );
    m_commandList->ClearDepthStencilView( m_dsvHeap.GetCpuHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr );

    m_commandList->OMSetRenderTargets( 1, &GetCurrBackBufferViewHandle(), true, &m_dsvHeap.GetCpuHandle() );

    ImGui_ImplDX12_RenderDrawData( ImGui::GetDrawData(), m_commandList.Get() );

    m_commandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( GetCurrBackBufferResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT ) );

    ASSERT_SUCCEEDED( m_commandList->Close() );

    ID3D12CommandList* const commandList = m_commandList.Get();
    m_commandQueue->ExecuteCommandLists( 1, &commandList );

    ASSERT_SUCCEEDED( m_dxgiSwapChain->Present( 0, 0 ) );
    m_currBackBufferIndex = ( m_currBackBufferIndex + 1 ) % kNumSwapChainBuffers;

    FlushCommandQueue();
}

void CClearScreenDemo::BuildImmediateUi() const
{
    ImGui_ImplDX12_NewFrame();
    ImGui::NewFrame();

    const ImGuiIO& io = ImGui::GetIO();

    ImGui::Begin( "Options" );
    ImGui::Text( "Adapter name: %ws", m_adapterName.c_str() );
    ImGui::Text( "FPS:          %d", static_cast<int32_t>( io.Framerate ) );
    ImGui::Text( "FrameTime:    %.3f", 1000.0f / io.Framerate );
    ImGui::End();

    ImGui::Render();
}
