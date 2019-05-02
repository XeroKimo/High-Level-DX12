#include "DX12/Objects/DX12R_FrameBuffer.h"
#include "DX12R.h"

DX12R_FrameBuffer::DX12R_FrameBuffer()
{
}

void DX12R_FrameBuffer::Initialize(DX12R_Device* device, DX12R_SwapChain* swapChain, int frameIndex, D3D12_RENDER_TARGET_VIEW_DESC* description, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
	swapChain->GetSwapChain()->GetBuffer(frameIndex, IID_PPV_ARGS(&m_frameResource));
	device->CreateRenderTargetView(m_frameResource.Get(), description, handle);
}

void DX12R_FrameBuffer::Reset()
{
	for (int i = 0; i < m_activeAllocators.size(); i++)
	{
		m_activeAllocators[i]->ReEnlist();
	}
	m_activeAllocators.clear();
}
