#include "ClearScreenDemo.h"
#include "Utility.h"

#include <DirectXColors.h>

void CClearScreenDemo::Render()
{
	ASSERT_SUCCEEDED( m_commandAllocator->Reset() );
	ASSERT_SUCCEEDED( m_commandList->Reset( m_commandAllocator.Get(), nullptr ) );

	m_commandList->RSSetScissorRects( 1, &m_scissorRect );
	m_commandList->RSSetViewports( 1, &m_viewport );

	m_commandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition(
		GetCurrBackBufferResource(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET ) );

	m_commandList->ClearRenderTargetView( GetCurrBackBufferViewHandle(), DirectX::Colors::CornflowerBlue, 0, nullptr );
	m_commandList->ClearDepthStencilView( GetDepthStencilViewHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr );

	m_commandList->OMSetRenderTargets( 1, &GetCurrBackBufferViewHandle(), true, &GetDepthStencilViewHandle() );

	m_commandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition(
		GetCurrBackBufferResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT ) );

	ASSERT_SUCCEEDED( m_commandList->Close() );
	
	ID3D12CommandList* const commandList = m_commandList.Get();
	m_commandQueue->ExecuteCommandLists( 1, &commandList );

	ASSERT_SUCCEEDED( m_dxgiSwapChain->Present1( 1, 0, &DXGI_PRESENT_PARAMETERS() ) );
	m_currBackBufferIndex = ( m_currBackBufferIndex + 1 ) % c_swapChainBufferCount;

	FlushCommandQueue();
}
