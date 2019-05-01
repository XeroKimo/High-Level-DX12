#include "DX12/Objects/DX12R_CommandList.h"

DX12R_CommandList::DX12R_CommandList()
{

}

bool DX12R_CommandList::Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* commandAllocator)
{
	if (FAILED(device->CreateCommandList(device->GetNodeCount(), type, commandAllocator, nullptr, IID_PPV_ARGS(&m_commandList))))
		return false;

	Close();

	return true;
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
	//tell command queue to put in close state
	return m_commandList->Close();
}

HRESULT DX12R_CommandList::Reset(ID3D12CommandAllocator* commmandAllocator, ID3D12PipelineState* pipelineState)
{
	return m_commandList->Reset(commmandAllocator, pipelineState);
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
