#include "DX12/Objects/DX12R_CommandList.h"
#include "DX12R.h"

DX12R_CommandList::DX12R_CommandList()
{

}

bool DX12R_CommandList::Initialize(DX12S_CommandSystem* commandSystem, DX12R_Device* device, D3D12_COMMAND_LIST_TYPE type, weak_ptr<DX12R_CommandAllocator> commandAllocator)
{
	m_commandSystem = commandSystem;

	m_commandAllocator = commandAllocator.lock();
	if (FAILED(device->CreateCommandList(type, m_commandAllocator->GetAllocator().Get(), IID_PPV_ARGS(&m_commandList))))
		return false;
	return true;
}

void DX12R_CommandList::BeginRender()
{
	DX12R_SwapChain* swapChain = m_commandSystem->GetDX12Interface()->swapChain.get();

	swapChain->GetFrameBuffer(swapChain->GetCurrentBackBufferIndex())->Reset();
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = swapChain->GetRTVHandle();
	rtvHandle.Offset(swapChain->GetCurrentBackBufferIndex(), swapChain->GetRTVDescriptorSize());
	const float clearColor[] = { 0.0f,0.2f,0.4f,1.0f };

	ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(swapChain->GetCurrentBackBufferResource().Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	OMSetRenderTargets(1, &rtvHandle,FALSE,nullptr);
	ClearRenderTargetView(rtvHandle, clearColor,0,nullptr);
}

void DX12R_CommandList::EndRender()
{
	CloseAndSubmitAll();
	m_commandSystem->GetDX12Interface()->swapChain->Present(0, 0, m_commandSystem->GetCommandQueue().get());
}

void DX12R_CommandList::ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, const float* colorRGBA, UINT numRects, D3D12_RECT* rect)
{
	m_commandList->ClearRenderTargetView(renderTargetView, colorRGBA, numRects, rect);
}

void DX12R_CommandList::OMSetRenderTargets(UINT numRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescirptors, BOOL rtsSingleHandleToDescriptorRange, const D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor)
{
	m_commandList->OMSetRenderTargets(numRenderTargetDescriptors, pRenderTargetDescirptors, rtsSingleHandleToDescriptorRange, pDepthStencilDescriptor);
}

void DX12R_CommandList::RSSetScissorRects(UINT numRects, const D3D12_RECT* rects)
{
	m_commandList->RSSetScissorRects(numRects, rects);
}

void DX12R_CommandList::RSSetViewports(UINT numViewports, const D3D12_VIEWPORT* viewports)
{
	m_commandList->RSSetViewports(numViewports, viewports);
}

void DX12R_CommandList::SetGraphicsRootSignature(ID3D12RootSignature* signature)
{
	m_commandList->SetGraphicsRootSignature(signature);
}

void DX12R_CommandList::SetGraphics32BitConstants(UINT rootParamIndex, UINT num32BitValuesToSet, const void* srcData, UINT DestOffsetIn32BitValues)
{
	m_commandList->SetGraphicsRoot32BitConstants(rootParamIndex, num32BitValuesToSet, srcData, DestOffsetIn32BitValues);
}

void DX12R_CommandList::SetGraphicsConstantBufferView(UINT rootParamIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
{
	m_commandList->SetGraphicsRootConstantBufferView(rootParamIndex, bufferLocation);
}

void DX12R_CommandList::SetGraphicsShaderResourceView(UINT rootParamIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
{
	m_commandList->SetGraphicsRootShaderResourceView(rootParamIndex, bufferLocation);
}

void DX12R_CommandList::SetGraphicsUnorderedAccessView(UINT rootParamIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation)
{
	m_commandList->SetGraphicsRootUnorderedAccessView(rootParamIndex, bufferLocation);
}

void DX12R_CommandList::SetGraphicsDescriptorTable(UINT rootParamIndex, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor)
{
	m_commandList->SetGraphicsRootDescriptorTable(rootParamIndex, baseDescriptor);
}

HRESULT DX12R_CommandList::Close()
{
	return m_commandList->Close();
}

void DX12R_CommandList::CloseForSubmit()
{
	Close();
	m_commandSystem->CloseList(shared_from_this());
}

void DX12R_CommandList::CloseAndSumbit()
{
	Close();
	m_commandSystem->ExecuteCommandList(shared_from_this());
}

void DX12R_CommandList::CloseAndSubmitAll()
{
	CloseForSubmit();
	m_commandSystem->ExecuteWaitingCommandLists();
}

HRESULT DX12R_CommandList::Reset(weak_ptr<DX12R_CommandAllocator> commandAllocator, ID3D12PipelineState* pipelineState)
{
	m_commandAllocator = commandAllocator.lock();

	return m_commandList->Reset(m_commandAllocator->GetAllocator().Get(), pipelineState);
}

void DX12R_CommandList::SetPipelineState(ID3D12PipelineState* pipelineState)
{
	m_commandList->SetPipelineState(pipelineState);
}

void DX12R_CommandList::ResourceBarrier(int numBarraiers, const  D3D12_RESOURCE_BARRIER* barriers)
{
	m_commandList->ResourceBarrier(numBarraiers, barriers);
}

ComPtr<ID3D12GraphicsCommandList> DX12R_CommandList::GetCommandList()
{
	return m_commandList;
}

weak_ptr<DX12R_CommandAllocator> DX12R_CommandList::GetCommandAllocator()
{
	return m_commandAllocator;
}
