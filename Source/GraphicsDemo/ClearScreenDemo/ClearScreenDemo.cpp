#include "ClearScreenDemo.h"
#include "Utility.h"

#include <DirectXColors.h>
#include <backends/imgui_impl_dx12.h>
#include <d3dx12/d3dx12_barriers.h>

CClearScreenDemo::CClearScreenDemo( HWND hWindow )
    : CGraphicsDemo( hWindow )
{
    DXGI_ADAPTER_DESC1 desc;
    ASSERT_SUCCEEDED( m_pDxgiAdapter->GetDesc1( &desc ) );

    m_adapterName = desc.Description;
}

void CClearScreenDemo::Render()
{
    ASSERT_SUCCEEDED( m_pCommandAllocator->Reset() );
    ASSERT_SUCCEEDED( m_pCommandList->Reset( m_pCommandAllocator.Get(), nullptr ) );

    BuildImmediateUi();

    ID3D12DescriptorHeap* const pCbvSrvUavHeap = m_cbvSrvUavHeap.GetHeap();
    m_pCommandList->SetDescriptorHeaps( 1, &pCbvSrvUavHeap );

    m_pCommandList->RSSetScissorRects( 1, &m_scissorRect );
    m_pCommandList->RSSetViewports( 1, &m_viewport );

    m_pCommandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( GetCurrBackBufferResource(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET ) );

    m_pCommandList->ClearRenderTargetView( GetCurrBackBufferViewHandle(), DirectX::Colors::CornflowerBlue, 0, nullptr );
    m_pCommandList->ClearDepthStencilView( m_dsvHeap.GetCpuHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr );

    m_pCommandList->OMSetRenderTargets( 1, &GetCurrBackBufferViewHandle(), true, &m_dsvHeap.GetCpuHandle() );

    ImGui_ImplDX12_RenderDrawData( ImGui::GetDrawData(), m_pCommandList.Get() );

    m_pCommandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( GetCurrBackBufferResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT ) );

    ASSERT_SUCCEEDED( m_pCommandList->Close() );

    ID3D12CommandList* const pCommandList = m_pCommandList.Get();
    m_pCommandQueue->ExecuteCommandLists( 1, &pCommandList );

    ASSERT_SUCCEEDED( m_pDxgiSwapChain->Present( 0, 0 ) );
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
