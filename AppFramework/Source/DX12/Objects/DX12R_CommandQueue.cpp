#include "DX12/Objects/DX12R_CommandQueue.h"
#include "DX12R.h"

DX12R_CommandQueue::DX12R_CommandQueue()
{

}

bool DX12R_CommandQueue::Initialize(DX12S_CommandSystem* commandSystem, DX12R_Device* device, D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_QUEUE_PRIORITY priority, D3D12_COMMAND_QUEUE_FLAGS flags)
{
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = type;
	desc.Priority = priority;
	desc.Flags = flags;
	desc.NodeMask = device->GetNodeMask();

	m_commandSystem = commandSystem;

	if (FAILED(device->CreateCommandQueue(&desc,IID_PPV_ARGS(&m_commandQueue))))
		return false;

	return true;
}

void DX12R_CommandQueue::ExecuteCommandList(DX12R_CommandList* commandList)
{
	ID3D12CommandList* ppList[] = { commandList->GetCommandList().Get() };
	m_commandQueue->ExecuteCommandLists(1, ppList);
}

void DX12R_CommandQueue::ExecuteCommandLists(UINT numCommandLists, ID3D12CommandList* const* commandLists)
{
	m_commandQueue->ExecuteCommandLists(numCommandLists, commandLists);
}

HRESULT DX12R_CommandQueue::SignalGPU(ID3D12Fence* fence, UINT64 value)
{
	return m_commandQueue->Signal(fence,value);
}

HRESULT DX12R_CommandQueue::StallGPU(ID3D12Fence* fence, UINT64 value)
{
	return m_commandQueue->Wait(fence,value);
}

ComPtr<ID3D12CommandQueue> DX12R_CommandQueue::GetQueue()
{
	return m_commandQueue;
}